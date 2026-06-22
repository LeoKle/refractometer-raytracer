#include "DetectorQE.h"
#include "DetectorQEData.h"

#include <algorithm>

namespace {

float LinearBetween(
    float x0,
    float y0,
    float x1,
    float y1,
    float x
) {
    if (x1 == x0) {
        return y0;
    }

    const float t = (x - x0) / (x1 - x0);
    return (1.0f - t) * y0 + t * y1;
}

} // namespace

float LinearInterpolateSpectralCurve(
    const float* bands,
    const float* values,
    int n,
    float wavelengthNm
) {
    if (n <= 0) return 0.0f;
    if (n == 1) return values[0];

    // Find first element >= wavelengthNm
    const float* it = std::lower_bound(bands, bands + n, wavelengthNm);
    
    if (it == bands) {
        // Linearly extrapolate using samples 0 and 1
        return LinearBetween(bands[0], values[0], bands[1], values[1], wavelengthNm);
    }
    
    if (it == bands + n) {
        // Linearly extrapolate using samples n-2 and n-1
        return LinearBetween(bands[n - 2], values[n - 2], bands[n - 1], values[n - 1], wavelengthNm);
    }

    // Inside range: linearly interpolate between adjacent bands
    int idx = static_cast<int>(std::distance(bands, it));
    return LinearBetween(bands[idx - 1], values[idx - 1], bands[idx], values[idx], wavelengthNm);
}

const SpectralCurve& GetDetectorQECurve() {
    static const SpectralCurve curve{
        DetectorQEBands,
        DetectorQEValues,
        DetectorQESamples
    };
    return curve;
}

float GetDetectorQE(float wavelengthNm) {
    const SpectralCurve& curve = GetDetectorQECurve();

    const float value = LinearInterpolateSpectralCurve(
        curve.bands,
        curve.values,
        curve.n,
        wavelengthNm
    );

    return std::clamp(value, 0.0f, 1.0f);
}

Spectrum GetDetectorQESpectrum() {
    const SpectralCurve& curve = GetDetectorQECurve();

    Spectrum spectrum;
    spectrum.samples.reserve(static_cast<std::size_t>(curve.n));

    for (int i = 0; i < curve.n; ++i) {
        const float qe = std::clamp(curve.values[i], 0.0f, 1.0f);
        spectrum.samples.push_back({curve.bands[i], qe});
    }

    return DropZeroSamples(spectrum);
}

Spectrum DropZeroSamples(const Spectrum& spectrum) {
    Spectrum result;
    result.samples.reserve(spectrum.samples.size());

    for (const SpectralSample& sample : spectrum.samples) {
        if (sample.intensity > 0.0f) {
            result.samples.push_back(sample);
        }
    }

    return result;
}

float EvaluateSpectrum(
    const Spectrum& spectrum,
    float wavelengthNm
) {
    if (spectrum.samples.empty()) {
        return 0.0f;
    }

    if (spectrum.samples.size() == 1) {
        return std::max(0.0f, spectrum.samples.front().intensity);
    }

    // Find first sample where sample.wavelengthNm >= wavelengthNm
    auto it = std::lower_bound(spectrum.samples.begin(), spectrum.samples.end(), wavelengthNm,
        [](const SpectralSample& s, float val) { return s.wavelengthNm < val; });

    if (it == spectrum.samples.begin()) {
        const auto& s0 = spectrum.samples[0];
        const auto& s1 = spectrum.samples[1];
        return std::max(0.0f, LinearBetween(
            s0.wavelengthNm,
            s0.intensity,
            s1.wavelengthNm,
            s1.intensity,
            wavelengthNm
        ));
    }

    if (it == spectrum.samples.end()) {
        size_t n = spectrum.samples.size();
        const auto& sn2 = spectrum.samples[n - 2];
        const auto& sn1 = spectrum.samples[n - 1];
        return std::max(0.0f, LinearBetween(
            sn2.wavelengthNm,
            sn2.intensity,
            sn1.wavelengthNm,
            sn1.intensity,
            wavelengthNm
        ));
    }

    auto it_prev = std::prev(it);
    return std::max(0.0f, LinearBetween(
        it_prev->wavelengthNm,
        it_prev->intensity,
        it->wavelengthNm,
        it->intensity,
        wavelengthNm
    ));
}

Spectrum MultiplySourceSpectrumByDetectorQE(
    const Spectrum& sourceSpectrum
) {
    const Spectrum source = DropZeroSamples(sourceSpectrum);
    const Spectrum qe = GetDetectorQESpectrum();

    if (source.samples.empty() || qe.samples.empty()) {
        return {};
    }

    if (source.samples.size() == 1) {
        const SpectralSample& s = source.samples.front();
        const float weighted = s.intensity * GetDetectorQE(s.wavelengthNm);

        if (weighted <= 0.0f) {
            return {};
        }

        return Spectrum{{{s.wavelengthNm, weighted}}};
    }

    const bool useSourceAxis =
        source.samples.size() >= qe.samples.size();

    const Spectrum& axisSpectrum = useSourceAxis ? source : qe;

    Spectrum result;
    result.samples.reserve(axisSpectrum.samples.size());

    for (const SpectralSample& axisSample : axisSpectrum.samples) {
        const float wavelengthNm = axisSample.wavelengthNm;

        const float sourceValue =
            useSourceAxis
                ? axisSample.intensity
                : EvaluateSpectrum(source, wavelengthNm);

        const float qeValue =
            useSourceAxis
                ? GetDetectorQE(wavelengthNm)
                : axisSample.intensity;

        const float weighted = sourceValue * qeValue;

        if (weighted > 0.0f) {
            result.samples.push_back({wavelengthNm, weighted});
        }
    }

    return result;
}

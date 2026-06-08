#ifndef DETECTOR_DETECTORQE_H
#define DETECTOR_DETECTORQE_H

#include "../light/Spectrum.h"

struct SpectralCurve {
    const float* bands;
    const float* values;
    int n;
};

float LinearInterpolateSpectralCurve(
    const float* bands,
    const float* values,
    int n,
    float wavelengthNm
);

const SpectralCurve& GetDetectorQECurve();

float GetDetectorQE(float wavelengthNm);

Spectrum GetDetectorQESpectrum();

Spectrum DropZeroSamples(const Spectrum& spectrum);

float EvaluateSpectrum(
    const Spectrum& spectrum,
    float wavelengthNm
);

Spectrum MultiplySourceSpectrumByDetectorQE(
    const Spectrum& sourceSpectrum
);

#endif

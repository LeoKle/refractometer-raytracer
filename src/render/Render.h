#ifndef RENDER_RENDER_H
#define RENDER_RENDER_H

class OpenApertureDetector;
class Prism;
class SellmeierMedium;
class SlitLight;

void render(
    OpenApertureDetector& detector,
    const Prism& prism,
    const SellmeierMedium& prismMaterial,
    const SlitLight& slitLight,
    int samplesPerPixel
);

#endif

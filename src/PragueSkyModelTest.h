#include <array>
#include <limits>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "PragueSkyModel.h"


using Vector3  = PragueSkyModel::Vector3;

// We use 11-channel spectrum for querying the model. The wavelengths samples are placed at centers of
// spectral bins used by the model.
constexpr int    SPECTRUM_CHANNELS      = 11;
constexpr double SPECTRUM_STEP          = 40;
using Spectrum                          = std::array<double, SPECTRUM_CHANNELS>;
constexpr Spectrum SPECTRUM_WAVELENGTHS = { 340.0, 380.0, 420.0, 460.0, 500.0, 540.0,
                                            580.0, 620.0, 660.0, 700.0, 740.0 };


/////////////////////////////////////////////////////////////////////////////////////
// Conversion functions
/////////////////////////////////////////////////////////////////////////////////////

double degreesToRadians(const double degrees) {
    return degrees * M_PI / 180.0;
}

/// Computes direction corresponding to given pixel coordinates in fisheye projection.
Vector3 pixelToDirection(int x, int y, int resolution) {
    // Make circular image area in center of image.
    const double radius  = resolution / 2;
    const double scaledx = (x - radius) / radius;
    const double scaledy = (y - radius) / radius;
    const double denom   = scaledx * scaledx + scaledy * scaledy + 1;

    if (denom > 2.0) {
        // Outside image area.
        return Vector3();
    } else {
        // Stereographic mapping.
        return Vector3(2.0 * scaledx / denom, 2.0 * scaledy / denom, -(denom - 2.0) / denom);
    }
}

// Spectral response table used for converting spectrum to XYZ.
const Vector3 SPECTRAL_RESPONSE[] = {
    Vector3(0.000129900000, 0.000003917000, 0.000606100000),
    Vector3(0.000232100000, 0.000006965000, 0.001086000000),
    Vector3(0.000414900000, 0.000012390000, 0.001946000000),
    Vector3(0.000741600000, 0.000022020000, 0.003486000000),
    Vector3(0.001368000000, 0.000039000000, 0.006450001000),
    Vector3(0.002236000000, 0.000064000000, 0.010549990000),
    Vector3(0.004243000000, 0.000120000000, 0.020050010000),
    Vector3(0.007650000000, 0.000217000000, 0.036210000000),
    Vector3(0.014310000000, 0.000396000000, 0.067850010000),
    Vector3(0.023190000000, 0.000640000000, 0.110200000000),
    Vector3(0.043510000000, 0.001210000000, 0.207400000000),
    Vector3(0.077630000000, 0.002180000000, 0.371300000000),
    Vector3(0.134380000000, 0.004000000000, 0.645600000000),
    Vector3(0.214770000000, 0.007300000000, 1.039050100000),
    Vector3(0.283900000000, 0.011600000000, 1.385600000000),
    Vector3(0.328500000000, 0.016840000000, 1.622960000000),
    Vector3(0.348280000000, 0.023000000000, 1.747060000000),
    Vector3(0.348060000000, 0.029800000000, 1.782600000000),
    Vector3(0.336200000000, 0.038000000000, 1.772110000000),
    Vector3(0.318700000000, 0.048000000000, 1.744100000000),
    Vector3(0.290800000000, 0.060000000000, 1.669200000000),
    Vector3(0.251100000000, 0.073900000000, 1.528100000000),
    Vector3(0.195360000000, 0.090980000000, 1.287640000000),
    Vector3(0.142100000000, 0.112600000000, 1.041900000000),
    Vector3(0.095640000000, 0.139020000000, 0.812950100000),
    Vector3(0.057950010000, 0.169300000000, 0.616200000000),
    Vector3(0.032010000000, 0.208020000000, 0.465180000000),
    Vector3(0.014700000000, 0.258600000000, 0.353300000000),
    Vector3(0.004900000000, 0.323000000000, 0.272000000000),
    Vector3(0.002400000000, 0.407300000000, 0.212300000000),
    Vector3(0.009300000000, 0.503000000000, 0.158200000000),
    Vector3(0.029100000000, 0.608200000000, 0.111700000000),
    Vector3(0.063270000000, 0.710000000000, 0.078249990000),
    Vector3(0.109600000000, 0.793200000000, 0.057250010000),
    Vector3(0.165500000000, 0.862000000000, 0.042160000000),
    Vector3(0.225749900000, 0.914850100000, 0.029840000000),
    Vector3(0.290400000000, 0.954000000000, 0.020300000000),
    Vector3(0.359700000000, 0.980300000000, 0.013400000000),
    Vector3(0.433449900000, 0.994950100000, 0.008749999000),
    Vector3(0.512050100000, 1.000000000000, 0.005749999000),
    Vector3(0.594500000000, 0.995000000000, 0.003900000000),
    Vector3(0.678400000000, 0.978600000000, 0.002749999000),
    Vector3(0.762100000000, 0.952000000000, 0.002100000000),
    Vector3(0.842500000000, 0.915400000000, 0.001800000000),
    Vector3(0.916300000000, 0.870000000000, 0.001650001000),
    Vector3(0.978600000000, 0.816300000000, 0.001400000000),
    Vector3(1.026300000000, 0.757000000000, 0.001100000000),
    Vector3(1.056700000000, 0.694900000000, 0.001000000000),
    Vector3(1.062200000000, 0.631000000000, 0.000800000000),
    Vector3(1.045600000000, 0.566800000000, 0.000600000000),
    Vector3(1.002600000000, 0.503000000000, 0.000340000000),
    Vector3(0.938400000000, 0.441200000000, 0.000240000000),
    Vector3(0.854449900000, 0.381000000000, 0.000190000000),
    Vector3(0.751400000000, 0.321000000000, 0.000100000000),
    Vector3(0.642400000000, 0.265000000000, 0.000049999990),
    Vector3(0.541900000000, 0.217000000000, 0.000030000000),
    Vector3(0.447900000000, 0.175000000000, 0.000020000000),
    Vector3(0.360800000000, 0.138200000000, 0.000010000000),
    Vector3(0.283500000000, 0.107000000000, 0.000000000000),
    Vector3(0.218700000000, 0.081600000000, 0.000000000000),
    Vector3(0.164900000000, 0.061000000000, 0.000000000000),
    Vector3(0.121200000000, 0.044580000000, 0.000000000000),
    Vector3(0.087400000000, 0.032000000000, 0.000000000000),
    Vector3(0.063600000000, 0.023200000000, 0.000000000000),
    Vector3(0.046770000000, 0.017000000000, 0.000000000000),
    Vector3(0.032900000000, 0.011920000000, 0.000000000000),
    Vector3(0.022700000000, 0.008210000000, 0.000000000000),
    Vector3(0.015840000000, 0.005723000000, 0.000000000000),
    Vector3(0.011359160000, 0.004102000000, 0.000000000000),
    Vector3(0.008110916000, 0.002929000000, 0.000000000000),
    Vector3(0.005790346000, 0.002091000000, 0.000000000000),
    Vector3(0.004109457000, 0.001484000000, 0.000000000000),
    Vector3(0.002899327000, 0.001047000000, 0.000000000000),
    Vector3(0.002049190000, 0.000740000000, 0.000000000000),
    Vector3(0.001439971000, 0.000520000000, 0.000000000000),
    Vector3(0.000999949300, 0.000361100000, 0.000000000000),
    Vector3(0.000690078600, 0.000249200000, 0.000000000000),
    Vector3(0.000476021300, 0.000171900000, 0.000000000000),
    Vector3(0.000332301100, 0.000120000000, 0.000000000000),
    Vector3(0.000234826100, 0.000084800000, 0.000000000000),
    Vector3(0.000166150500, 0.000060000000, 0.000000000000),
    Vector3(0.000117413000, 0.000042400000, 0.000000000000),
    Vector3(0.000083075270, 0.000030000000, 0.000000000000),
    Vector3(0.000058706520, 0.000021200000, 0.000000000000),
    Vector3(0.000041509940, 0.000014990000, 0.000000000000),
    Vector3(0.000029353260, 0.000010600000, 0.000000000000),
    Vector3(0.000020673830, 0.000007465700, 0.000000000000),
    Vector3(0.000014559770, 0.000005257800, 0.000000000000),
    Vector3(0.000010253980, 0.000003702900, 0.000000000000),
    Vector3(0.000007221456, 0.000002607800, 0.000000000000),
    Vector3(0.000005085868, 0.000001836600, 0.000000000000),
    Vector3(0.000003581652, 0.000001293400, 0.000000000000),
    Vector3(0.000002522525, 0.000000910930, 0.000000000000),
    Vector3(0.000001776509, 0.000000641530, 0.000000000000),
    Vector3(0.000001251141, 0.000000451810, 0.000000000000)
};
constexpr double SPECTRAL_RESPONSE_START = 360.0;
constexpr double SPECTRAL_RESPONSE_STEP  = 5.0;

/// Converts given spectrum to sRGB.
Vector3 spectrumToRGB(const Spectrum& spectrum) {
    // Spectrum to XYZ
    Vector3 xyz = Vector3();
    for (int wl = 0; wl < SPECTRUM_CHANNELS; wl++) {
        const int responseIdx = int((SPECTRUM_WAVELENGTHS[wl] - SPECTRAL_RESPONSE_START) / SPECTRAL_RESPONSE_STEP);
        xyz                   = xyz + SPECTRAL_RESPONSE[responseIdx] * spectrum[wl];
    }
    xyz = xyz * SPECTRUM_STEP;

    // XYZ to sRGB
    Vector3 rgb = Vector3();
    rgb.x       = 3.2404542 * xyz.x - 1.5371385 * xyz.y - 0.4985314 * xyz.z;
    rgb.y       = -0.9692660 * xyz.x + 1.8760108 * xyz.y + 0.0415560 * xyz.z;
    rgb.z       = 0.0556434 * xyz.x - 0.2040259 * xyz.y + 1.0572252 * xyz.z;

    return rgb;
}


/////////////////////////////////////////////////////////////////////////////////////
// Rendering
/////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// An example of using Prague Sky Model for rendering a simple fisheye RGB image of the sky.
/// </summary>
/// <param name="model">Pointer to the sky model object.</param>
/// <param name="albedo">Ground albedo, value in range [0, 1].</param>
/// <param name="altitude">Altitude of view point in meters, value in range [0, 15000].</param>
/// <param name="azimuth">Sun azimuth at view point in degrees, value in range [0, 360].</param>
/// <param name="elevation">Sun elevation at view point in degrees, value in range [-4.2, 90].</param>
/// <param name="mode">Rendered quantity: 0 = radiance, 1 = sun radiance, 2 = polarisation, 3 = transmittance.</param>
/// <param name="resolution">Length of resulting square image size in pixels.</param>
/// <param name="visibility">Horizontal visibility (meteorological range) at ground level in kilometers, value in range [20, 131.8].</param>
/// <param name="outResult">Buffer for storing the resulting image.</param>
void render(const PragueSkyModel* model,
            const double          albedo,
            const double          altitude,
            const double          azimuth,
            const double          elevation,
            const int             mode,
            const int             resolution,
            const double          visibility,
            std::vector<float>&   outResult) {
    assert(model);

    // We are viewing the sky from 'altitude' meters above the origin.
    const Vector3 viewPoint = Vector3(0.0, 0.0, altitude);

    // Resize the RGB buffer.
    outResult.resize(size_t(resolution) * resolution * 3);

    for (int x = 0; x < resolution; x++) {
        for (int y = 0; y < resolution; y++) {
            // For each pixel of the rendered image get the corresponding direction in fisheye projection.
            const Vector3 viewDir = pixelToDirection(x, y, resolution);

            // If the pixel lies outside the upper hemisphere, the direction will be zero. Such a pixel is
            // painted black.
            if (viewDir.isZero()) {
                outResult[(size_t(x) * resolution + y) * 3]     = 0.0;
                outResult[(size_t(x) * resolution + y) * 3 + 1] = 0.0;
                outResult[(size_t(x) * resolution + y) * 3 + 2] = 0.0;
                continue;
            }

            // Get internal model parameters for the desired configuration.
            const PragueSkyModel::Parameters params =
                model->computeParameters(viewPoint, viewDir, elevation, azimuth, visibility, albedo);

            // Based on the selected mode compute spectral sky radiance, sun radiance, polarisation or
            // transmittance.
            Spectrum spectrum;
            for (int wl = 0; wl < SPECTRUM_CHANNELS; wl++) {
                switch (mode) {
                case 1:
                    spectrum[wl] = model->sunRadiance(params, SPECTRUM_WAVELENGTHS[wl]);
                    break;
                case 2:
                    spectrum[wl] = std::abs(model->polarisation(params, SPECTRUM_WAVELENGTHS[wl]));
                    break;
                case 3:
                    spectrum[wl] = model->transmittance(params,
                                                        SPECTRUM_WAVELENGTHS[wl],
                                                        std::numeric_limits<double>::max());
                    break;
                default:
                    spectrum[wl] = model->skyRadiance(params, SPECTRUM_WAVELENGTHS[wl]);
                    break;
                }
            }

            // Convert the spectral quantity to sRGB and store it in the result buffer.
            const Vector3 rgb                               = spectrumToRGB(spectrum);
            outResult[(size_t(x) * resolution + y) * 3]     = float(rgb.x);
            outResult[(size_t(x) * resolution + y) * 3 + 1] = float(rgb.y);
            outResult[(size_t(x) * resolution + y) * 3 + 2] = float(rgb.z);
        }
    }
}
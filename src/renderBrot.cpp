#include "renderBrot.hpp"

int isInMandlebrotButGiveIterationsToEscape(std::complex<long double> c, int detailAmt) {
    std::complex<long double> z(0.0L, 0.0L);
    std::complex<long double> z_old(0.0L, 0.0L);
    int period = 0;
    for (int i = 0; i < detailAmt; i++) {
        z = z * z + c;
        if (std::abs(z) > 2.0L) {
            return i + 1;
        }
        if (i == period) {
            if (std::abs(z - z_old) < 1e-10L) {
                return -1;
            }
            z_old = z;
            period = i;
        }
    }
    return -1;
}

void giveMandelbrotOutputsInRange(int startX, int endX, int setWidth, int setHeight, long double centerReal, long double centerImag, std::vector<int>& returnVector, long double zoomFactor, bool usingULDM, bool usingLDM, int detailAmt) {
    // performance smiling
    returnVector.reserve((endX - startX) * setHeight);

    int addAmt = 1;

    if (usingULDM) {
        addAmt = 4;
    } else if (usingLDM) {
        addAmt = 2;
    }

    for (int x = startX; x < endX; x += addAmt) {
        for (int y = 0; y < setHeight; y += addAmt) {
            // converting 0-based screen coordinates because the function demands actual mathematical coords
            long double real = centerReal + ((x - setWidth/2.0) / zoomFactor);
            long double imag = centerImag + ((y - setHeight/2.0) / zoomFactor);
            returnVector.push_back(isInMandlebrotButGiveIterationsToEscape(std::complex<long double>(real, imag), detailAmt));
        }
    }
}

std::vector<Color> calculateEscapeValues(RenderTexture2D mandieSet, long double offsetX, long double offsetY, int threadz, long double zoomFactor, bool usingULDM, bool usingLDM, int detailAmt) {
    // Only begin texture mode when we are actually ready to draw
    int setWidth = mandieSet.texture.width;
    int setHeight = mandieSet.texture.height;

    long double centerReal = offsetX / zoomFactor;
    long double centerImag = offsetY / zoomFactor;

    std::vector<int> allEscapeValues;
    std::vector<Color> relativeEscapeValueColors;

    // Multi-threading setup
    std::vector<std::vector<int>> tLists(threadz);
    int stripWidth = setWidth / threadz;
    std::vector<std::thread> threadsList;

    for (int i = 0; i < threadz; i++) {
        threadsList.push_back(std::thread([&, i]() {
            int endPosition;
            if (i == threadz - 1) {
                endPosition = setWidth;
            } else {
                endPosition = stripWidth * (i + 1);
            }

            giveMandelbrotOutputsInRange(stripWidth * i, endPosition, setWidth, setHeight, centerReal, centerImag, tLists[i], zoomFactor, usingULDM, usingLDM, detailAmt);
        }));
    }

    for (int i = 0; i < threadsList.size(); i++) {
        threadsList[i].join();
    }

    for (int i = 0; i < tLists.size(); i++) {
        allEscapeValues.insert(allEscapeValues.end(), tLists[i].begin(), tLists[i].end());
    }

    int minimumEscapeIterations = INT_MAX;
    int maximumEscapeIterations = INT_MIN;

    for (size_t i = 0; i < allEscapeValues.size(); i++) {
        if (allEscapeValues[i] != -1) {
            if (allEscapeValues[i] < minimumEscapeIterations) {
                minimumEscapeIterations = allEscapeValues[i];
            }
            if (allEscapeValues[i] > maximumEscapeIterations) {
                maximumEscapeIterations = allEscapeValues[i];
            }
        }
    }

    // calculate colors based on escape values
    for (int val : allEscapeValues) {
        if (val == -1) {
            relativeEscapeValueColors.push_back(BLACK);
        } else {
            float range = (float)(maximumEscapeIterations - minimumEscapeIterations);
            if (range == 0) {
                range = 1.0f;
            }

            float t = (float)(val - minimumEscapeIterations) / range;
            relativeEscapeValueColors.push_back(ColorFromHSV(t * 360.0f, 1.0f, 1.0f));
        }
    }

    return relativeEscapeValueColors;
}
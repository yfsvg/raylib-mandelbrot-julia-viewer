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



// Julia Set code

int isInJuliaSet(std::complex<long double> z, std::complex<long double> c, long double power, int detailAmt) {
    for (int i = 0; i < detailAmt; i++) {
        // z = z^power + c
        z = std::pow(z, power) + c;
        if (std::abs(z) > 2.0L) {
            return i + 1;
        }
    }
    return -1;
}

void giveJuliaOutputsInRange(int startX, int endX, int setWidth, int setHeight, long double centerReal, long double centerImag, std::vector<int>& returnVector, long double zoomFactor, std::complex<long double> juliaSeed, long double juliaPower, bool usingULDM, bool usingLDM, int detailAmt) {
    returnVector.reserve((endX - startX) * setHeight);

    int addAmt = 1;

    if (usingULDM) {
        addAmt = 4;
    } else if (usingLDM) {
        addAmt = 2;
    }

    for (int x = startX; x < endX; x += addAmt) {
        for (int y = 0; y < setHeight; y += addAmt) {
            long double real = centerReal + ((x - setWidth/2.0) / zoomFactor);
            long double imag = centerImag + ((y - setHeight/2.0) / zoomFactor);
            returnVector.push_back(isInJuliaSet(std::complex<long double>(real, imag), juliaSeed, juliaPower, detailAmt));
        }
    }
}

std::vector<Color> calculateEscapeValuesJulia(RenderTexture2D mandieSet, long double offsetX, long double offsetY, int threadz, long double zoomFactor, std::complex<long double> juliaSeed, long double juliaPower, bool usingULDM, bool usingLDM, int detailAmt) {
    int setWidth = mandieSet.texture.width;
    int setHeight = mandieSet.texture.height;

    long double centerReal = offsetX / zoomFactor;
    long double centerImag = offsetY / zoomFactor;

    std::vector<int> allEscapeValues;
    std::vector<Color> relativeEscapeValueColors;

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

            giveJuliaOutputsInRange(stripWidth * i, endPosition, setWidth, setHeight, centerReal, centerImag, tLists[i], zoomFactor, juliaSeed, juliaPower, usingULDM, usingLDM, detailAmt);
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



















// GMP code

int isInMandlebrotGMP(mpf_class cReal, mpf_class cImag, int detailAmt) {
    mpf_class zReal = 0, zImag = 0;
    mpf_class z_r_sq = 0, z_i_sq = 0, temp;
    
    // Periodicity checking ended up being slower than just ignoring it
    for (int i = 0; i < detailAmt; i++) {
        temp = z_r_sq - z_i_sq + cReal;
        zImag = 2 * zReal * zImag + cImag;
        zReal = temp;
        
        z_r_sq = zReal * zReal;
        z_i_sq = zImag * zImag;
        
        if (z_r_sq + z_i_sq > 4.0) {
            return i + 1;
        }
    }
    return -1;
}

void giveMandelbrotOutputsInRangeGMP(int startX, int endX, int setWidth, int setHeight, mpf_class centerReal, mpf_class centerImag, std::vector<int>& returnVector, mpf_class arbZoomFactor, bool usingULDM, bool usingLDM, int detailAmt) {
    returnVector.reserve((endX - startX) * setHeight);
    
    mpf_class real, imag;
    mpf_class step = 1.0 / arbZoomFactor;

    int addAmt = 1;

    if (usingULDM) {
        addAmt = 4;
    } else if (usingLDM) {
        addAmt = 2;
    }
    
    for (int x = startX; x < endX; x += addAmt) {
        real = centerReal + (x - setWidth/2.0) * step;
        for (int y = 0; y < setHeight; y += addAmt) {
            imag = centerImag + (y - setHeight/2.0) * step;
            returnVector.push_back(isInMandlebrotGMP(real, imag, detailAmt));
        }
    }
}


std::vector<Color> calculateEscapeValuesForArbs(RenderTexture2D mandieSet, mpf_class offsetX, mpf_class offsetY, int threadz, mpf_class arbZoomFactor, bool usingULDM, bool usingLDM, int detailAmt) {
    // Only begin texture mode when we are actually ready to draw
    int setWidth = mandieSet.texture.width;
    int setHeight = mandieSet.texture.height;

    mpf_class centerReal = offsetX / arbZoomFactor;
    mpf_class centerImag = offsetY / arbZoomFactor;

    std::vector<int> allEscapeValues;
    std::vector<Color> relativeEscapeValueColors;

    allEscapeValues.clear();
    relativeEscapeValueColors.clear();

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
            giveMandelbrotOutputsInRangeGMP(stripWidth * i, endPosition, setWidth, setHeight, centerReal, centerImag, tLists[i], arbZoomFactor, usingULDM, usingLDM, detailAmt);        }));
    }

    for (int i = 0; i < threadsList.size(); i++) {
        threadsList[i].join();
    }

    for (int i = 0; i < tLists.size(); i++) {
        allEscapeValues.insert(allEscapeValues.end(), tLists[i].begin(), tLists[i].end());
    }

    // reusing coloring logic (identical to float version)
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

    for (int val : allEscapeValues) {
        if (val == -1) {
            relativeEscapeValueColors.push_back(BLACK);
        } else {
            float range = (float) (maximumEscapeIterations - minimumEscapeIterations);
            if (range == 0) {
                range = 1.0f; 
            }
            float t = (float) (val - minimumEscapeIterations) / range;
            relativeEscapeValueColors.push_back(ColorFromHSV(t * 360.0f, 1.0f, 1.0f));
        }
    }

    return relativeEscapeValueColors;
}


// Julia Set GMP code

int isInJuliaSetGMP(mpf_class zReal, mpf_class zImag, mpf_class cReal, mpf_class cImag, long double power, int detailAmt) {
    mpf_class z_r_sq, z_i_sq, temp;
    
    for (int i = 0; i < detailAmt; i++) {
        // z = z^power + c
        // For simplicity with GMP, we handle power = 2 as the standard case
        // For other powers, we'd need more complex operations
        if (power == 2.0L) {
            z_r_sq = zReal * zReal;
            z_i_sq = zImag * zImag;
            
            temp = z_r_sq - z_i_sq + cReal;
            zImag = 2 * zReal * zImag + cImag;
            zReal = temp;
        } else {
            //convert to complex and use std::pow
            // is less slowre but maintains compatibility
            std::complex<long double> z(zReal.get_d(), zImag.get_d());
            std::complex<long double> c(cReal.get_d(), cImag.get_d());
            z = std::pow(z, power) + c;

            // Genius (long double -> string -> arbitrary precision)
            zReal = mpf_class(std::to_string(z.real()));
            zImag = mpf_class(std::to_string(z.imag()));

        }
        
        z_r_sq = zReal * zReal;
        z_i_sq = zImag * zImag;
        
        if (z_r_sq + z_i_sq > 4.0) {
            return i + 1;
        }
    }
    return -1;
}

void giveJuliaOutputsInRangeGMP(int startX, int endX, int setWidth, int setHeight, mpf_class centerReal, mpf_class centerImag, std::vector<int>& returnVector, mpf_class zoomFactor, mpf_class juliaSeedReal, mpf_class juliaSeedImag, long double juliaPower, bool usingULDM, bool usingLDM, int detailAmt) {
    returnVector.reserve((endX - startX) * setHeight);
    
    mpf_class real, imag;
    mpf_class step = 1.0 / zoomFactor;

    int addAmt = 1;

    if (usingULDM) {
        addAmt = 4;
    } else if (usingLDM) {
        addAmt = 2;
    }
    
    for (int x = startX; x < endX; x += addAmt) {
        real = centerReal + (x - setWidth/2.0) * step;
        for (int y = 0; y < setHeight; y += addAmt) {
            imag = centerImag + (y - setHeight/2.0) * step;
            returnVector.push_back(isInJuliaSetGMP(real, imag, juliaSeedReal, juliaSeedImag, juliaPower, detailAmt));
        }
    }
}

std::vector<Color> calculateEscapeValuesJuliaArbs(RenderTexture2D mandieSet, mpf_class offsetX, mpf_class offsetY, int threadz, mpf_class zoomFactor, mpf_class juliaSeedReal, mpf_class juliaSeedImag, long double juliaPower, bool usingULDM, bool usingLDM, int detailAmt) {
    int setWidth = mandieSet.texture.width;
    int setHeight = mandieSet.texture.height;

    mpf_class centerReal = offsetX / zoomFactor;
    mpf_class centerImag = offsetY / zoomFactor;

    std::vector<int> allEscapeValues;
    std::vector<Color> relativeEscapeValueColors;

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

            giveJuliaOutputsInRangeGMP(stripWidth * i, endPosition, setWidth, setHeight, centerReal, centerImag, tLists[i], zoomFactor, juliaSeedReal, juliaSeedImag, juliaPower, usingULDM, usingLDM, detailAmt);
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
#include "ui.hpp"

std::string truncateZeroes(long double input, int truncateAmount) {
    std::stringstream ss;
    ss << std::setprecision(truncateAmount) << input;
    std::string s_value = ss.str();

    if (s_value.find('.') != std::string::npos) {
        s_value.erase(s_value.find_last_not_of('0') + 1, std::string::npos);
        if (s_value.back() == '.') {
            s_value.pop_back();
        }
    }
    return s_value;
}

void drawFPSRegular(long double zoomFactor, SuperVector2 offset, float moreInfoOffset) {
    DrawText(TextFormat("Zoom: %s", truncateZeroes(zoomFactor, 15).c_str()), 15 + moreInfoOffset, 35, 20, WHITE);
    DrawText(TextFormat("X Pos: %s", truncateZeroes(offset.x / zoomFactor, 10).c_str()), 15 + moreInfoOffset, 55, 20, WHITE);
    DrawText(TextFormat("Y Pos: %s", truncateZeroes(offset.y / zoomFactor, 10).c_str()), 15 + moreInfoOffset, 75, 20, WHITE);
}
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

void drawPosition(bool usingArbitraryPrecisionLibrary, long double zoomFactor, SuperVector2 offset, float moreInfoOffset, ArbVector2 arbOffset, mpf_class arbZoomFactor) {
    if (usingArbitraryPrecisionLibrary) {
        std::stringstream ss; 
        ss << arbZoomFactor;
        std::string z = ss.str().substr(0, 15);
        
        ss.str(""); ss << (arbOffset.x / arbZoomFactor);
        std::string x = ss.str().substr(0, 15);
        
        ss.str(""); ss << (arbOffset.y / arbZoomFactor);
        std::string y = ss.str().substr(0, 15);

        DrawText(TextFormat("Zoom: %s...", z.c_str()), 15 + moreInfoOffset, 35, 20, WHITE);
        DrawText(TextFormat("X Pos: %s...", x.c_str()), 15 + moreInfoOffset, 55, 20, WHITE);
        DrawText(TextFormat("Y Pos: %s...", y.c_str()), 15 + moreInfoOffset, 75, 20, WHITE);
    } else {
        DrawText(TextFormat("Zoom: %s", truncateZeroes(zoomFactor, 15).c_str()), 15 + moreInfoOffset, 35, 20, WHITE);
        DrawText(TextFormat("X Pos: %s", truncateZeroes(offset.x / zoomFactor, 10).c_str()), 15 + moreInfoOffset, 55, 20, WHITE);
        DrawText(TextFormat("Y Pos: %s", truncateZeroes(offset.y / zoomFactor, 10).c_str()), 15 + moreInfoOffset, 75, 20, WHITE);
    }

}




void drawCheckbox(bool& variableToChange, float moreInfoOffset, float x, float y, float sizeX, float sizeY, std::string textWithBox, float margins) {

    Rectangle boxCheckbox = {x + moreInfoOffset, y, sizeX, sizeY};
    if (variableToChange) {
        DrawRectangleRec(boxCheckbox, Fade(WHITE, 0.3f));
    } else {
        DrawRectangleRec(boxCheckbox, Fade(WHITE, 0.1f));
    }
    DrawRectangleLinesEx(boxCheckbox, 1, Fade(WHITE, 0.5f));
    DrawText(textWithBox.c_str(), sizeX + margins + x + moreInfoOffset, y + margins, 20, WHITE);
    if (CheckCollisionPointRec(GetMousePosition(), boxCheckbox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        variableToChange = !variableToChange;
    }
}

void drawTextInput( float moreInfoOffset, float x, float y, float sizeX, float sizeY,
                    bool& editingCurrentVariable, bool& editingOV1, bool& editingOV2,
                    char inputText[6], std::string queryText, bool& needsRedraw,
                    int variableToChange
) {
    int lengthInput;

    Rectangle detailInputBox = { x + moreInfoOffset, y, sizeX, sizeY };
    if (editingCurrentVariable) {
        DrawRectangleRec(detailInputBox, Fade(WHITE, 0.3f));
    } else {
        DrawRectangleRec(detailInputBox, Fade(WHITE, 0.1f));
    }
    DrawRectangleLinesEx(detailInputBox, 1, Fade(WHITE, 0.5f));
    DrawText(inputText, 20 + moreInfoOffset, y + 5, 20, WHITE);
    DrawText(queryText.c_str(), 125 + moreInfoOffset, y + 5, 20, WHITE);
    if (CheckCollisionPointRec(GetMousePosition(), detailInputBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        editingCurrentVariable = true;
        editingOV1 = false;
        editingOV2 = false;
    }
    if (editingCurrentVariable) {
        int key = GetCharPressed();
        lengthInput = strlen(inputText);
        if (key >= '0' && key <= '9' && lengthInput < 5) {
            inputText[lengthInput] = (char)key;
            inputText[lengthInput + 1] = '\0';
        }
        if (IsKeyPressed(KEY_BACKSPACE) && lengthInput > 0) {
            inputText[lengthInput - 1] = '\0';
        }
        if (IsKeyPressed(KEY_ENTER)) {
            variableToChange = atoi(inputText);
            needsRedraw = true;
            editingCurrentVariable = false;
        }
    }   
}
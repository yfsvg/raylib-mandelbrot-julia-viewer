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




void deselectAllTextInputs(std::vector<bool*> allEditingStates, bool* currentlyEditing) {
    for (auto state : allEditingStates) {
        *state = false;
    }
    *currentlyEditing = true;
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

void drawTextInput(float moreInfoOffset, float x, float y, float sizeX, float sizeY,
                    bool& editingCurrentVariable, std::vector<bool*> allEditingStates,
                    std::string& inputText, std::string queryText, bool& needsRedraw,
                    int& variableToChange
) {
    Rectangle detailInputBox = { x + moreInfoOffset, y, sizeX, sizeY };

    DrawRectangleRec(detailInputBox, Fade(WHITE, editingCurrentVariable ? 0.3f : 0.1f));
    DrawRectangleLinesEx(detailInputBox, 1, Fade(WHITE, 0.5f));

    DrawText(inputText.c_str(), 20 + moreInfoOffset, y + 5, 20, WHITE);
    DrawText(queryText.c_str(), 125 + moreInfoOffset, y + 5, 20, WHITE);

    if (CheckCollisionPointRec(GetMousePosition(), detailInputBox) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        deselectAllTextInputs(allEditingStates, &editingCurrentVariable);
    }

    if (editingCurrentVariable && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
        !CheckCollisionPointRec(GetMousePosition(), detailInputBox)) {
        editingCurrentVariable = false;
    }

    if (editingCurrentVariable) {
        int key = GetCharPressed();

        if (key >= '0' && key <= '9' && inputText.length() < 4) {
            inputText.push_back((char)key);
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !inputText.empty()) {
            inputText.pop_back();
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (!inputText.empty()) {
                variableToChange = std::stoi(inputText);
                needsRedraw = true;
            }
            editingCurrentVariable = false;
        }
    }
}

void drawPopup(std::string bigText, std::string smallText, std::string acceptanceText) {
    Rectangle bgBox = {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()};
    DrawRectanglePro(bgBox, {0, 0}, 0, Fade(BLACK, 0.8f));

    Rectangle moreInfoBox = { (float)(GetScreenWidth() / 2) - 150, (float)(GetScreenHeight() / 2) - 100, 300, 200 };
    DrawRectangleRounded(moreInfoBox, 0.00f, 4, Fade(BLACK, 0.8f));
    DrawRectangleLinesEx(moreInfoBox, 1, Fade(WHITE, 0.5f));
}

void drawTextInputDouble(float moreInfoOffset, float x, float y, float sizeX, float sizeY,
                    bool& editingCurrentVariable, std::vector<bool*> allEditingStates,
                    std::string& inputText, std::string queryText, bool& needsRedraw,
                    long double& variableToChange
) {
    Rectangle detailInputBox = { x + moreInfoOffset, y, sizeX, sizeY };

    DrawRectangleRec(detailInputBox, Fade(WHITE, editingCurrentVariable ? 0.3f : 0.1f));
    DrawRectangleLinesEx(detailInputBox, 1, Fade(WHITE, 0.5f));

    DrawText(inputText.c_str(), 20 + moreInfoOffset, y + 5, 20, WHITE);
    DrawText(queryText.c_str(), 125 + moreInfoOffset, y + 5, 20, WHITE);

    if (CheckCollisionPointRec(GetMousePosition(), detailInputBox) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        deselectAllTextInputs(allEditingStates, &editingCurrentVariable);
    }

    if (editingCurrentVariable && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
        !CheckCollisionPointRec(GetMousePosition(), detailInputBox)) {
        editingCurrentVariable = false;
    }

    if (editingCurrentVariable) {
        int key = GetCharPressed();

        // Allow digits, decimal point, and minus sign
        if ((key >= '0' && key <= '9') && inputText.length() < 10) {
            inputText.push_back((char)key);
        } else if (key == '.' && inputText.find('.') == std::string::npos && inputText.length() < 10) {
            inputText.push_back('.');
        } else if (key == '-' && inputText.length() == 0) {
            inputText.push_back('-');
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !inputText.empty()) {
            inputText.pop_back();
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (!inputText.empty() && inputText != "-") {
                try {
                    variableToChange = std::stold(inputText);
                    needsRedraw = true;
                } catch (...) {
                    // invalid number
                }
            }
            editingCurrentVariable = false;
        }
    }
}
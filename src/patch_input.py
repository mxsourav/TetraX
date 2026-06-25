import os

with open('input_manager.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

target1 = '''        if (i == BTN_ID_LEFT) {
            raw |= (stableState[BTN_ID_BACK] && ::digitalRead(PINS[BTN_ID_UP]) == LOW);
        } else if (i == BTN_ID_RIGHT) {
            raw |= (stableState[BTN_ID_BACK] && ::digitalRead(PINS[BTN_ID_DOWN]) == LOW);
        } else if (i == BTN_ID_UP || i == BTN_ID_DOWN) {
            raw |= (!stableState[BTN_ID_BACK] && ::digitalRead(PINS[i]) == LOW);
        } else if (i == BTN_ID_BACK) {
            raw |= (::digitalRead(PINS[i]) == LOW);
        } else if (PINS[i] != 255) {
            raw |= (::digitalRead(PINS[i]) == LOW);
        }'''

replacement1 = '''        if (PINS[i] != 255) {
            raw |= (::digitalRead(PINS[i]) == LOW);
        }'''

target2 = '''                if (i != BTN_ID_BACK) {
                    events[i] = EVT_PRESSED;
                }'''
replacement2 = '''                events[i] = EVT_PRESSED;'''

target3 = '''                if (i == BTN_ID_LEFT || i == BTN_ID_RIGHT) {
                    comboUsed = true;
                    Serial.println("[COMBO] BACK + DIR used");
                }'''
replacement3 = ''''''

target4 = '''                if (i == BTN_ID_BACK) {
                    if (!comboUsed) {
                        Serial.println("[MODIFIER] BACK released without combo. Triggering CANCEL.");
                        events[i] = EVT_RELEASED;
                        cancelFlag = true;
                    }
                    comboUsed = false;
                }'''
replacement4 = '''                if (i == BTN_ID_BACK) {
                    cancelFlag = true;
                }'''

text = text.replace(target1, replacement1)
text = text.replace(target2, replacement2)
text = text.replace(target3, replacement3)
text = text.replace(target4, replacement4)

with open('input_manager.cpp', 'w', encoding='utf-8') as f:
    f.write(text)
print("Patched input_manager.cpp")

import re, glob

files = glob.glob("src/*.cpp")
for fpath in files:
    with open(fpath, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()
    
    changed = False
    
    # Fix BuzzerManager::beep calls with duration > 5
    def fix_buzzer_beep(m):
        dur = int(m.group(1))
        rest = m.group(2)  # could be ", count" or empty
        if dur > 5:
            return f"BuzzerManager::beep(5{rest})"
        return m.group(0)
    
    new_text = re.sub(r'BuzzerManager::beep\((\d+)((?:,\s*\d+)?)\)', fix_buzzer_beep, text)
    if new_text != text:
        text = new_text
        changed = True
    
    # Fix BuzzerManager::beepSync calls with duration > 5
    def fix_buzzer_sync(m):
        dur = int(m.group(1))
        if dur > 5:
            return "BuzzerManager::beepSync(5)"
        return m.group(0)
    
    new_text = re.sub(r'BuzzerManager::beepSync\((\d+)\)', fix_buzzer_sync, text)
    if new_text != text:
        text = new_text
        changed = True
    
    # Fix local beep(freq, duration) calls — cap duration at 5ms
    # These are in Deauther.cpp and ble_spam.cpp
    def fix_local_beep(m):
        freq = m.group(1)
        dur = int(m.group(2))
        if dur > 5:
            return f"beep({freq}, 5)"
        return m.group(0)
    
    # Only match beep(number, number) — NOT BuzzerManager::beep
    new_text = re.sub(r'(?<!:)beep\((\d+),\s*(\d+)\)', fix_local_beep, text)
    if new_text != text:
        text = new_text
        changed = True
    
    if changed:
        with open(fpath, "w", encoding="utf-8") as f:
            f.write(text)
        print(f"Fixed: {fpath}")
    else:
        print(f"OK: {fpath}")

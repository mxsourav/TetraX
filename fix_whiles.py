import re, glob

files = glob.glob("src/*.cpp")
for fpath in files:
    with open(fpath, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()
    
    changed = False
    
    # We want to match: while (digitalRead(...) == LOW) delay(5);
    # or: while (digitalRead(...) == LOW || digitalRead(...) == LOW) delay(5);
    # We will replace the "delay(X);" part with "{ BuzzerManager::update(); delay(X); }"
    
    def fix_while_delay(m):
        condition = m.group(1)
        delay_val = m.group(2)
        return f"while ({condition}) {{ BuzzerManager::update(); delay({delay_val}); }}"
    
    # Matches: while ( <anything with digitalRead> ) delay(<digits>);
    new_text = re.sub(r'while\s*\(([^)]*digitalRead[^)]*(?:\)[^)]*)*)\)\s*delay\((\d+)\);', fix_while_delay, text)
    
    if new_text != text:
        text = new_text
        changed = True
    
    if changed:
        with open(fpath, "w", encoding="utf-8") as f:
            f.write(text)
        print(f"Fixed while loops in: {fpath}")
    else:
        print(f"OK: {fpath}")

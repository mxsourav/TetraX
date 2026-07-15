import os

ui_dir = r"C:\Users\rodd\Desktop\BWifiKill-ESP32-V4.0-main\SATAN-UI\src"
for root, dirs, files in os.walk(ui_dir):
    for f in files:
        if f.endswith('.tsx') or f.endswith('.ts'):
            filepath = os.path.join(root, f)
            with open(filepath, 'r', encoding='utf-8') as file:
                content = file.read()
            
            content = content.replace("BWifiKill", "TetraX")
            
            with open(filepath, 'w', encoding='utf-8') as file:
                file.write(content)
                
print("UI strings patched.")

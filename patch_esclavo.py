import os

src_dir = r"C:\Users\rodd\Desktop\BWifiKill-ESP32-V4.0-main\src"
for f in os.listdir(src_dir):
    if f.endswith('.cpp') or f.endswith('.h'):
        filepath = os.path.join(src_dir, f)
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as file:
            content = file.read()
        
        # Replace Spanish strings
        content = content.replace("esclavo", "slave")
        content = content.replace("ESCLAVO", "SLAVE")
        
        with open(filepath, 'w', encoding='utf-8') as file:
            file.write(content)
            
print("Firmware esclavo strings patched.")

import os

src_dir = r"C:\Users\rodd\Desktop\BWifiKill-ESP32-V4.0-main\src"
for f in os.listdir(src_dir):
    if f.endswith('.cpp') or f.endswith('.h'):
        filepath = os.path.join(src_dir, f)
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as file:
            content = file.read()
        
        # Replace Spanish strings and PepeAngell
        content = content.replace("PepeAngell", "MX 100RAV")
        content = content.replace("BWifiKill", "TetraX")
        content = content.replace("MODO CENTINELA", "SENTINEL MODE")
        content = content.replace("MODO HIBRIDO", "HYBRID MODE")
        content = content.replace("MODO MAX", "MAX MODE")
        content = content.replace("CONECTADO", "CONNECTED")
        content = content.replace("EMPAREJAR", "PAIR")
        content = content.replace("modo promiscuo", "promiscuous mode")
        
        with open(filepath, 'w', encoding='utf-8') as file:
            file.write(content)
            
print("Firmware strings patched.")

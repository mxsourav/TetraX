
import os
import glob

replacements = {
    "Escaneando WiFi...": "Scanning WiFi...",
    "ESCANEANDO CANALES": "SCANNING CHANNELS",
    "ESCANEANDO AP": "SCANNING AP",
    "Solo redes propias": "Own networks only",
    "- Redes ajenas sin permiso": "- Ext networks no permission",
    "Atacara TODAS las redes WiFi": "Attack ALL WiFi networks",
    "No hay redes WiFi": "No WiFi networks found",
    "SIN REDES OPEN": "NO OPEN NETWORKS",
    "SIN REDES": "NO NETWORKS",
    "detectadas.": "detected.",
    "OK/AUX res BK salir": "OK/AUX:Rescan BK:Exit",
    "salir": "exit",
    "atras": "back",
    "< RESCAN": "< RESCAN",
    "ESCANEANDO": "SCANNING",
    "BUSCANDO": "SEARCHING"
}

for filepath in glob.glob("src/*.cpp"):
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            content = f.read()
        
        new_content = content
        for k, v in replacements.items():
            new_content = new_content.replace(k, v)
            
        if new_content != content:
            with open(filepath, "w", encoding="utf-8") as f:
                f.write(new_content)
            print(f"Updated {filepath}")
    except Exception as e:
        print(f"Failed {filepath}: {e}")


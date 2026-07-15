
import re

content = open("src/gui_helper.cpp").read()

functions = [
    "drawWifiCategoryIcon", "drawRfCategoryIcon", "drawBluetoothCategoryIcon",
    "drawWarningCategoryIcon", "drawSystemCategoryIcon", "drawGamesCategoryIcon",
    "drawIRCategoryIcon", "drawCategoryIcon"
]

# We will manually patch them in the next step, just checking if we can parse them.


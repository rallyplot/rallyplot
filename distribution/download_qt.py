import subprocess
from pathlib import Path
import os
import platform
import platform

qt_install_path = Path(__file__).parent / "qt"

env = os.environ.copy()

os_name = platform.system()

if not qt_install_path.is_dir():

    if os_name == "Windows":
        subprocess.run(
            "aqt install-qt windows desktop 6.8.2 win64_msvc2022_64 -O qt",
            shell=True, env=env, check=True
        )
    elif os_name == "Darwin":
        subprocess.run(
            "aqt install-qt mac desktop 6.7.3 -O qt",  # to target macos-11
            shell=True, env=env, check=True
        )
    elif os_name == "Linux":
        """
        sudo apt update
        sudo apt install -y build-essential cmake ninja-build pkg-config \
        libgl1-mesa-dev libegl1-mesa-dev \
        libx11-dev libxext-dev libxrandr-dev libxi-dev \
        libxkbcommon-dev libxkbcommon-x11-dev libx11-xcb-dev
        """
        subprocess.run(
            "aqt install-qt linux desktop 6.8.2 -O qt",
            shell=True, env=env, check=True
        )


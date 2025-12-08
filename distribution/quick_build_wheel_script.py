import subprocess
from pathlib import Path
import os
import platform

qt_install_path = Path(__file__).parent / "qt"

env = os.environ.copy()

os_name = platform.system()

if os_name == "Windows":
    env["QT_DIR"] = f"{qt_install_path.as_posix()}/6.8.2/msvc2022_64/lib/cmake/Qt6"
    env["Qt6_DIR"] = f"{qt_install_path.as_posix()}/6.8.2/msvc2022_64/lib/cmake/Qt6"
elif os_name == "Darwin":
    env["QT_DIR"] = f"{qt_install_path.as_posix()}/6.7.3/macos/lib/cmake/Qt6"
    env["Qt6_DIR"] = f"{qt_install_path.as_posix()}/6.7.3/macos/lib/cmake/Qt6"
elif os_name == "Linux":
    env["QT_DIR"] = f"{qt_install_path.as_posix()}/6.8.2/gcc_64/lib/cmake/Qt6"
    env["Qt6_DIR"] = f"{qt_install_path.as_posix()}/6.8.2/gcc_64/lib/cmake/Qt6"

# 👇 most important line
env["CMAKE_ARGS"] = "-DRALLYPLOT_BUILD_DEV=ON"

subprocess.run(
    "pip wheel .. --no-deps",
    shell=True,
    env=env,
    check=True
)
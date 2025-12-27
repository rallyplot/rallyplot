import shutil
import subprocess
from pathlib import Path
import os
import platform

def find_wheel(path_):
    search_wheels = list(path_.glob("rallyplot-*.whl"))
    assert len(search_wheels) == 1
    return search_wheels[0]

distribution_dir = Path(__file__).parent
qt_install_path = distribution_dir / "qt"

if not qt_install_path.is_dir():
    subprocess.run(f"python {distribution_dir}/download_qt.py", shell=True, check=True)

env = os.environ.copy()

os_name = platform.system()

if os_name == "Windows":
    env["Qt6_DIR"] = f"{qt_install_path.as_posix()}/6.8.2/msvc2022_64/lib/cmake/Qt6"
elif os_name == "Darwin":
    env["Qt6_DIR"] = f"{qt_install_path.as_posix()}/6.7.3/macos/lib/cmake/Qt6"
    # Setting CMAKE_PREFIX_PATH seems to be required on macOS so cmake can find QtGuiTools
    env["CMAKE_PREFIX_PATH"] = f"{qt_install_path.as_posix()}/6.7.3/macos"
elif os_name == "Linux":
    env["Qt6_DIR"] = f"{qt_install_path.as_posix()}/6.8.2/gcc_64/lib/cmake/Qt6"

breakpoint()

env["CMAKE_ARGS"] = "-DRALLYPLOT_BUILD_DEV=ON -DUSE_VENDORED_QT=ON"

subprocess.run(
    "pip wheel .. --no-deps",
    shell=True,
    env=env,
    check=True,
    cwd=distribution_dir
)

if platform.system() != "Darwin":

    out_path = Path(__file__).parent / "out"
    out_path.mkdir(parents=True)
    wheel_path = find_wheel(distribution_dir)

    if platform.system() == "Windows":
        qt_lib_path = f"{qt_install_path}/6.8.2/msvc2022_64/bin"

    elif platform.system() == "Linux":
        qt_lib_path = f"{qt_install_path}/6.8.2/gcc_64/lib"

    subprocess.run(
        f"python {distribution_dir}/repair_wheel.py --qt_lib_path {qt_lib_path} --out_path {out_path} --wheel_path {wheel_path}",
        shell=True,
        env=env,
        check=True
    )

    wheel_path.unlink()
    shutil.move(find_wheel(out_path), distribution_dir)
    out_path.rmdir()


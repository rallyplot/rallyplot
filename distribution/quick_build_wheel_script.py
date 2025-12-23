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

# TODO: 1) make a globber function
# 2) do the same on ubuntu
# 3) make a central function for these commands.
# 4) merge this
# 5) leverage this to build not from pypi in tests


if platform.system() == "Windows":

    out_path = Path(__file__).parent / "out"
    out_path.mkdir(parents=True)
    wheel_path = find_wheel(distribution_dir)

    subprocess.run(f"python -m delvewheel repair -w {out_path} {wheel_path} --ignore-existing --add-path {qt_install_path}/6.8.2/msvc2022_64/bin -vv")

elif platform.system() == "Linux":
    import os
    import subprocess

    out_path = Path(__file__).parent / "out"
    out_path.mkdir(parents=True)
    wheel_path = find_wheel(distribution_dir)

    env = os.environ.copy()
    env["LD_LIBRARY_PATH"] = f"{qt_install_path}/6.8.2/gcc_64/lib:" + env.get("LD_LIBRARY_PATH", "")


    subprocess.run(
        [
            "auditwheel", "repair",
            "-w", out_path,
            "--exclude", "libEGL.so.1",
            "--exclude", "libGLX.so.0",
            "--exclude", "libOpenGL.so.0",
            "--exclude", "libGLdispatch.so.0",
            "--exclude", "libgbm.so.1",
            "--exclude", "libdrm.so.2",
            "--exclude", "libxcb.so.1",
            "--exclude", "libXau.so.6",
            "--exclude", "libXdmcp.so.6",
            "--exclude", "libwayland-client.so.0",
            "--exclude", "libwayland-egl.so.1",
            "--exclude", "libwayland-cursor.so.0",
            "--exclude", "libdbus-1.so.3",
            "--exclude", "libsystemd.so.0",
            "--exclude", "libvulkan.so.1",
            "--exclude", "libzstd.so.1",
            wheel_path,
        ],
        env=env,
        check=True,
    )

    wheel_path.unlink()
    shutil.move(find_wheel(out_path), distribution_dir)
    out_path.rmdir()



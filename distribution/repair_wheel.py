import shutil
import subprocess
from pathlib import Path
import os
import platform
import argparse

p = argparse.ArgumentParser(description="Repair a built wheel (Windows: delvewheel, Linux: auditwheel).")
p.add_argument("--wheel_path", type=Path, required=True, help="Path to the wheel to repair")
p.add_argument("--out_path", type=Path, default=Path("out"), help="Temporary output dir for repaired wheel")
p.add_argument("--qt_lib_path", type=Path, required=True, help="Path to Qt root directory (contains 6.x.y/...)")

args = p.parse_args()

env = os.environ.copy()

out_path = args.out_path
wheel_path = args.wheel_path
qt_lib_path = args.qt_lib_path

if platform.system() == "Windows":
    subprocess.run(f"python -m delvewheel repair -w {out_path} {wheel_path} --ignore-existing --add-path {qt_lib_path} -vv", env=env, check=True)

elif platform.system() == "Linux":
    env["LD_LIBRARY_PATH"] = f"{qt_lib_path}:" + env.get("LD_LIBRARY_PATH", "")

    subprocess.run(
        [
            "python -m auditwheel", "repair",
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

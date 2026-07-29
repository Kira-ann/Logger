import pytest
import subprocess
import os
from pathlib import Path


@pytest.fixture(scope="session")
def build_project():
    # Сборка перед тестами
    build_dir = Path(__file__).parent.parent / "build"
    if not build_dir.exists():
        os.makedirs(build_dir)

    # Сборка
    result = subprocess.run(
        ["cmake", ".."],
        cwd=build_dir,
        capture_output=True,
        text=True
    )
    if result.returncode != 0:
        pytest.fail(f"CMake failed: {result.stderr}")

    result = subprocess.run(
        ["make", "-j4"],
        cwd=build_dir,
        capture_output=True,
        text=True
    )
    if result.returncode != 0:
        pytest.fail(f"Make failed: {result.stderr}")

    return build_dir


@pytest.fixture
def app_path(build_project):
    return build_project / "app/app" #Путь к исполняемому файлу
    
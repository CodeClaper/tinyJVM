import pytest
import subprocess
from pathlib import Path

data_dir = Path("classes")
class_files = [f.stem for f in data_dir.glob("**/*.class") if f.is_file()]

@pytest.mark.parametrize("class_file", class_files)
def test_all(class_file):
    expected = subprocess.run(
        ["javap", "-verbose",  "-cp", "classes", class_file],
        capture_output=True,
        text=True,
        encoding="utf-8"
    )


    actual = subprocess.run(
        ["./src/javap", "-verbose",  "-cp", "classes", class_file],
        capture_output=True,
        text=True,
        encoding="utf-8"
    )
    print(expected.stdout)
    print("*******************************************************")
    print(actual.stdout)
    
    assert actual.stdout == expected.stdout, f"{class_file} match fail."

# OOP_LB

This repository contains a simple robots game with a C++ backend and a Python frontend.

## Build (Windows, PowerShell)

You can build the backend with CMake and a generator that supports your toolchain (e.g., Visual Studio, Ninja).

Open PowerShell in the `project` directory and run:

```powershell
# Create build directory
mkdir build; cd build

# Configure (use -G "Ninja" or another generator if you prefer)
cmake ..

# Build
cmake --build . --config Release

# Run the backend (example)
# The executable will be in build/backend/ (generator-dependent). Adjust the path if necessary.
.
# Example (MSVC with default layout):
# .\backend\Release\oop_backend.exe
```

## Running the frontend

The frontend is a set of Python scripts under `frontend/`. If the frontend requires external packages, install them with:

```powershell
cd frontend
python -m pip install -r requirements.txt
```

Then run the client (example):

```powershell
python client.py
```

## Notes

- The backend uses `nlohmann/json`. CMake uses FetchContent to download it if not provided by the system.
- If you prefer to use vcpkg or system-installed packages, remove the FetchContent block and link against your installed library.

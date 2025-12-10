@echo off
echo === Building backend ===
cd project\backend

if not exist build mkdir build
cd build

cmake ..
cmake --build . --config Release

cd ..\..\..

echo === Installing Python dependencies ===
pip install -r project\frontend\requirements.txt

echo === Starting Python menu ===
python project\frontend\menu.py

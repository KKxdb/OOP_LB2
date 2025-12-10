# Makefile для білду та запуску гри (заміна build_and_run.ps1)

# Змінні
CMAKE = cmake
BUILD_DIR = build
CONFIG = Release  # Можна змінити на Debug
EXECUTABLE = backend/$(CONFIG)/oop_backend  # Адаптуй під ім'я твого exe (з твого ps1: oop_backend.exe)

all: build launch

build:
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) ..
	@cd $(BUILD_DIR) && $(CMAKE) --build . --config $(CONFIG)
	@echo Backend executable location:
	@echo $(CURDIR)/$(BUILD_DIR)/$(EXECUTABLE).exe

clean:
	@if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)

launch: launch.bat

launch.bat:
	@echo @echo off > launch.bat
	@echo echo Запуск гри: Python фронтенд з C++ бекендом >> launch.bat
	@echo start $(BUILD_DIR)\$(EXECUTABLE).exe >> launch.bat  # Запуск бекенду (адаптуй якщо на сокетах)
	@echo python menu.py >> launch.bat  # Або інший вхідний .py

.PHONY: all build clean launch
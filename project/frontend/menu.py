# frontend/menu.py
import os
import json
import tkinter as tk
from tkinter import ttk, messagebox, simpledialog, filedialog
import subprocess
import threading
from game_window import GameWindow
import sys
import queue

BACKEND_EXEC = r"C:\Users\Кирило\Documents\GitHub\OOP_LB2\project\backend\build\Release\oop_backend.exe"
LEVELS_DIR = os.path.join(os.path.dirname(__file__), "levels")


# ======================================================================
#                          MAIN MENU
# ======================================================================
class MenuApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("OOP Lab — Меню")
        self.geometry("360x220")
        self._ensure_levels_dir()
        self._build_ui()

    def _ensure_levels_dir(self):
        if not os.path.exists(LEVELS_DIR):
            os.makedirs(LEVELS_DIR, exist_ok=True)

    def _build_ui(self):
        frame = ttk.Frame(self, padding=16)
        frame.pack(fill="both", expand=True)

        title = ttk.Label(frame, text="Головне меню", font=("Segoe UI", 14, "bold"))
        title.pack(pady=(0, 12))

        btn_levels = ttk.Button(frame, text="Рівні", command=self.open_levels)
        btn_levels.pack(fill="x", pady=6)

        btn_create = ttk.Button(frame, text="Створити", command=self.create_level_dialog)
        btn_create.pack(fill="x", pady=6)

        btn_exit = ttk.Button(frame, text="Вихід", command=self.quit)
        btn_exit.pack(fill="x", pady=6)

        hint = ttk.Label(frame, text="Рівні зберігаються в frontend/levels", font=("Segoe UI", 8))
        hint.pack(side="bottom", pady=(12, 0))

    def open_levels(self):
        LevelsWindow(self)

    def create_level_dialog(self):
        dlg = CreateLevelDialog(self)
        self.wait_window(dlg)
        if getattr(dlg, "result", None):
            name, width, height = dlg.result
            try:
                self._create_level_file(name, width, height)
                messagebox.showinfo("Створено", f"Рівень '{name}' створено")
            except Exception as e:
                messagebox.showerror("Помилка", str(e))

    def _create_level_file(self, name: str, width: int, height: int):
        safe_name = "".join(c for c in name if c.isalnum() or c in (" ", "_", "-")).strip()
        if not safe_name:
            raise ValueError("Невірна назва рівня")

        filename = f"{safe_name}.json"
        path = os.path.join(LEVELS_DIR, filename)

        if os.path.exists(path):
            raise FileExistsError("Файл з такою назвою вже існує")

        level_data = {
            "name": name,
            "width": width,
            "height": height,
            "robots": [],
            "boxes": [],
            "targets": []
        }

        with open(path, "w", encoding="utf-8") as f:
            json.dump(level_data, f, ensure_ascii=False, indent=2)


# ======================================================================
#                           CREATE LEVEL DIALOG
# ======================================================================
class CreateLevelDialog(tk.Toplevel):
    def __init__(self, parent):
        super().__init__(parent)
        self.title("Створити рівень")
        self.result = None
        self.resizable(False, False)
        self._build_ui()
        self.grab_set()

    def _build_ui(self):
        frm = ttk.Frame(self, padding=12)
        frm.pack(fill="both", expand=True)

        ttk.Label(frm, text="Назва рівня").grid(row=0, column=0, sticky="w")
        self.name_var = tk.StringVar()
        ttk.Entry(frm, textvariable=self.name_var, width=30).grid(row=0, column=1, pady=4)

        ttk.Label(frm, text="Ширина").grid(row=1, column=0, sticky="w")
        self.width_var = tk.IntVar(value=10)
        ttk.Entry(frm, textvariable=self.width_var, width=10).grid(row=1, column=1, pady=4, sticky="w")

        ttk.Label(frm, text="Висота").grid(row=2, column=0, sticky="w")
        self.height_var = tk.IntVar(value=10)
        ttk.Entry(frm, textvariable=self.height_var, width=10).grid(row=2, column=1, pady=4, sticky="w")

        btns = ttk.Frame(frm)
        btns.grid(row=3, column=0, columnspan=2, pady=(10, 0))

        ttk.Button(btns, text="Створити", command=self._on_create).pack(side="left", padx=6)
        ttk.Button(btns, text="Скасувати", command=self.destroy).pack(side="left", padx=6)

    def _on_create(self):
        name = self.name_var.get().strip()
        if not name:
            messagebox.showwarning("Помилка", "Введіть назву рівня")
            return

        width = int(self.width_var.get())
        height = int(self.height_var.get())

        if width <= 0 or height <= 0:
            messagebox.showwarning("Помилка", "Розміри мають бути додатні")
            return

        self.result = (name, width, height)
        self.destroy()


# ======================================================================
#                           LEVELS LIST WINDOW
# ======================================================================
class LevelsWindow(tk.Toplevel):
    def __init__(self, parent):
        super().__init__(parent)
        self.title("Список рівнів")
        self.geometry("420x320")
        self._build_ui()
        self._load_levels()

    def _build_ui(self):
        frm = ttk.Frame(self, padding=10)
        frm.pack(fill="both", expand=True)

        self.list_frame = ttk.Frame(frm)
        self.list_frame.pack(fill="both", expand=True)

        btn_frame = ttk.Frame(frm)
        btn_frame.pack(fill="x", pady=(8, 0))

        ttk.Button(btn_frame, text="Оновити", command=self._load_levels).pack(side="left")
        ttk.Button(btn_frame, text="Імпортувати", command=self._import_level).pack(side="left", padx=6)
        ttk.Button(btn_frame, text="Закрити", command=self.destroy).pack(side="right")

    def _load_levels(self):
        for child in self.list_frame.winfo_children():
            child.destroy()

        files = sorted(f for f in os.listdir(LEVELS_DIR) if f.endswith(".json"))
        if not files:
            ttk.Label(self.list_frame, text="Рівні не знайдено").pack(pady=12)
            return

        for fname in files:
            row = ttk.Frame(self.list_frame, padding=6)
            row.pack(fill="x", pady=3)

            ttk.Label(row, text=fname).pack(side="left")

            ttk.Button(row, text="Play", command=lambda f=fname: self._play_level(f)).pack(side="right")
            ttk.Button(row, text="Відкрити", command=lambda f=fname: self._open_level(f)).pack(side="right", padx=6)

    def _open_level(self, filename):
        path = os.path.join(LEVELS_DIR, filename)
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)
        Viewer(self, filename, data)

    def _import_level(self):
        src = filedialog.askopenfilename(filetypes=[("JSON files", "*.json")])
        if not src:
            return
        dst = os.path.join(LEVELS_DIR, os.path.basename(src))
        with open(src, "rb") as fr, open(dst, "wb") as fw:
            fw.write(fr.read())
        messagebox.showinfo("OK", "Імпортовано")
        self._load_levels()

    def _play_level(self, filename):
        level_path = os.path.join(LEVELS_DIR, filename)

        backend = BackendProcess()
        backend.level_path = level_path 
        backend.start()

        resp = backend.send({"action": "load_level", "path": level_path})

        print("INIT RESP:", resp)

        # Передаємо відповідь в GameRunner, щоб уникнути подвійного запиту
        GameRunner(self, backend, level_path, resp)


# ======================================================================
#                              JSON VIEWER
# ======================================================================
class Viewer(tk.Toplevel):
    def __init__(self, parent, title, data):
        super().__init__(parent)
        self.title(f"Перегляд — {title}")
        self.geometry("520x420")

        txt = tk.Text(self, wrap="none")
        txt.pack(fill="both", expand=True)

        txt.insert("1.0", json.dumps(data, ensure_ascii=False, indent=2))
        txt.config(state="disabled")


# ======================================================================
#                     BACKEND PROCESS CONTROLLER
# ======================================================================

class BackendProcess:
    def __init__(self, exe_path=BACKEND_EXEC):
        self.exe_path = exe_path
        self.proc = None

    def start(self):
        print(">>> TRYING TO RUN BACKEND:", self.exe_path)
        self.proc = subprocess.Popen(
            [self.exe_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            text=True,
            bufsize=1
        )

    def send(self, obj):
        line = json.dumps(obj)
        self.proc.stdin.write(line + "\n")
        self.proc.stdin.flush()
        resp = self.proc.stdout.readline()
        return json.loads(resp)

    def stop(self):
        if self.proc:
            self.proc.terminate()



# ======================================================================
#                          SIMPLE GAME RUNNER
# ======================================================================
class GameRunner(tk.Toplevel):
    def __init__(self, parent, backend: BackendProcess, level_path: str, init_resp=None):
        super().__init__(parent)
        self.backend = backend
        #self.backend.start()

        # load level
        if init_resp is None:
            resp = backend.send({"action": "load_level", "path": level_path})
        else:
            resp = init_resp

        self.game = GameWindow(self, resp)
        self.game.pack(fill="both", expand=True)

        top = ttk.Frame(self)
        top.pack(fill="x")
        ttk.Button(top, text="Step", command=self.on_step).pack(side="left")
        ttk.Button(top, text="Stop", command=self.on_stop).pack(side="right")

    def on_step(self):
        resp = self.backend.send({"action": "step"})
        self.game.update_state(resp)

    def on_stop(self):
        self.backend.stop()
        self.destroy()



# ======================================================================
#                                  RUN
# ======================================================================
if __name__ == "__main__":
    app = MenuApp()
    app.mainloop()

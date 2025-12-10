# frontend/menu.py
import os
import json
import tkinter as tk
from tkinter import ttk, messagebox, simpledialog, filedialog
import subprocess
import threading
import time

BACKEND_EXEC = os.path.join(os.path.dirname(__file__), "..", "build", "oop_backend")

LEVELS_DIR = os.path.join(os.path.dirname(__file__), "levels")

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

        # footer / hint
        hint = ttk.Label(frame, text="Рівні зберігаються в папці: frontend/levels", font=("Segoe UI", 8))
        hint.pack(side="bottom", pady=(12,0))

    def open_levels(self):
        LevelsWindow(self)

    def create_level_dialog(self):
        dlg = CreateLevelDialog(self)
        self.wait_window(dlg)
        if getattr(dlg, "result", None):
            name, width, height = dlg.result
            try:
                self._create_level_file(name, width, height)
                messagebox.showinfo("Створено", f"Рівень '{name}' створено у {LEVELS_DIR}")
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

        # Проста структура рівня (можна розширити)
        level_data = {
            "name": name,
            "width": width,
            "height": height,
            "robots": [],   # приклад: [{"id":1,"type":"worker","x":0,"y":0}]
            "boxes": [],    # приклад: [{"id":1,"x":2,"y":3,"delivered":false}]
            "targets": []   # приклад: [{"x":5,"y":1}]
        }
        with open(path, "w", encoding="utf-8") as f:
            json.dump(level_data, f, ensure_ascii=False, indent=2)

class CreateLevelDialog(tk.Toplevel):
    def __init__(self, parent):
        super().__init__(parent)
        self.title("Створити рівень")
        self.resizable(False, False)
        self.result = None
        self._build_ui()
        self.transient(parent)
        self.grab_set()
        self.protocol("WM_DELETE_WINDOW", self._on_cancel)

    def _build_ui(self):
        frm = ttk.Frame(self, padding=12)
        frm.pack(fill="both", expand=True)

        ttk.Label(frm, text="Назва рівня").grid(row=0, column=0, sticky="w")
        self.name_var = tk.StringVar()
        ttk.Entry(frm, textvariable=self.name_var, width=30).grid(row=0, column=1, pady=4)

        ttk.Label(frm, text="Ширина (клітин)").grid(row=1, column=0, sticky="w")
        self.width_var = tk.IntVar(value=8)
        ttk.Entry(frm, textvariable=self.width_var, width=10).grid(row=1, column=1, pady=4, sticky="w")

        ttk.Label(frm, text="Висота (клітин)").grid(row=2, column=0, sticky="w")
        self.height_var = tk.IntVar(value=6)
        ttk.Entry(frm, textvariable=self.height_var, width=10).grid(row=2, column=1, pady=4, sticky="w")

        btn_frame = ttk.Frame(frm)
        btn_frame.grid(row=3, column=0, columnspan=2, pady=(10,0))

        ttk.Button(btn_frame, text="Створити", command=self._on_create).pack(side="left", padx=6)
        ttk.Button(btn_frame, text="Скасувати", command=self._on_cancel).pack(side="left", padx=6)

    def _on_create(self):
        name = self.name_var.get().strip()
        try:
            width = int(self.width_var.get())
            height = int(self.height_var.get())
        except ValueError:
            messagebox.showwarning("Невірні дані", "Ширина і висота мають бути числами")
            return
        if not name:
            messagebox.showwarning("Невірні дані", "Введіть назву рівня")
            return
        if width <= 0 or height <= 0:
            messagebox.showwarning("Невірні дані", "Розміри мають бути додатні")
            return
        self.result = (name, width, height)
        self.destroy()

    def _on_cancel(self):
        self.result = None
        self.destroy()

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
        btn_frame.pack(fill="x", pady=(8,0))
        ttk.Button(btn_frame, text="Оновити", command=self._load_levels).pack(side="left")
        ttk.Button(btn_frame, text="Імпортувати", command=self._import_level).pack(side="left", padx=6)
        ttk.Button(btn_frame, text="Закрити", command=self.destroy).pack(side="right")

    def _load_levels(self):
        for child in self.list_frame.winfo_children():
            child.destroy()

        files = sorted([f for f in os.listdir(LEVELS_DIR) if f.lower().endswith(".json")])
        if not files:
            ttk.Label(self.list_frame, text="Рівні не знайдено у папці levels/").pack(pady=12)
            return

        for fname in files:
            row = ttk.Frame(self.list_frame, padding=6)
            row.pack(fill="x", pady=2)
            ttk.Label(row, text=fname).pack(side="left", padx=(0,8))
            ttk.Button(row, text="Play", command=lambda f=fname: self._play_level(f)).pack(side="right", padx=(6,0))
            ttk.Button(row, text="Відкрити", command=lambda f=fname: self._open_level(f)).pack(side="right")

    def _open_level(self, filename):
        path = os.path.join(LEVELS_DIR, filename)
        try:
            with open(path, "r", encoding="utf-8") as f:
                data = json.load(f)
            # Відкриваємо просте вікно з вмістом JSON (можна замінити на передачу в GUI)
            Viewer(self, filename, data)
        except Exception as e:
            messagebox.showerror("Помилка", f"Не вдалося відкрити {filename}:\n{e}")

    def _import_level(self):
        src = filedialog.askopenfilename(title="Імпортувати рівень", filetypes=[("JSON files","*.json")])
        if not src:
            return
        try:
            dst = os.path.join(LEVELS_DIR, os.path.basename(src))
            if os.path.exists(dst):
                if not messagebox.askyesno("Перезаписати?", f"{os.path.basename(src)} вже існує. Перезаписати?"):
                    return
            with open(src, "rb") as fr, open(dst, "wb") as fw:
                fw.write(fr.read())
            messagebox.showinfo("Імпортовано", f"Файл імпортовано у {dst}")
            self._load_levels()
        except Exception as e:
            messagebox.showerror("Помилка", str(e))

class Viewer(tk.Toplevel):
    def __init__(self, parent, title, data):
        super().__init__(parent)
        self.title(f"Перегляд — {title}")
        self.geometry("520x420")
        txt = tk.Text(self, wrap="none")
        txt.pack(fill="both", expand=True)
        txt.insert("1.0", json.dumps(data, ensure_ascii=False, indent=2))
        txt.config(state="disabled")

# --- Клас для керування бекендом ---
class BackendProcess:
    def __init__(self, exe_path=BACKEND_EXEC):
        self.exe_path = exe_path
        self.proc = None
        self.lock = threading.Lock()

    def start(self):
        if self.proc is not None:
            return
        try:
            self.proc = subprocess.Popen(
                [self.exe_path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1
            )
        except Exception as e:
            raise RuntimeError(f"Не вдалося запустити бекенд: {e}")

    def stop(self):
        if self.proc:
            try:
                self.proc.stdin.close()
            except Exception:
                pass
            try:
                self.proc.terminate()
            except Exception:
                pass
            self.proc = None

    def send_command(self, cmd: dict, timeout=2.0):
        """
        Надсилає JSON команду у бекенд і читає одну рядкову відповідь (JSON).
        Повертає розпарсений об'єкт (dict) або піднімає помилку.
        """
        if not self.proc:
            raise RuntimeError("Бекенд не запущено")
        s = json.dumps(cmd, ensure_ascii=False)
        with self.lock:
            try:
                # Надсилаємо рядок з новим рядком
                self.proc.stdin.write(s + "\n")
                self.proc.stdin.flush()
            except Exception as e:
                raise RuntimeError(f"Помилка запису у stdin бекенду: {e}")

            # Читаємо одну лінію відповіді
            try:
                line = self.proc.stdout.readline()
                if not line:
                    # можливо бекенд закрився або помилка
                    err = self.proc.stderr.read()
                    raise RuntimeError(f"Бекенд не відповів. stderr: {err}")
                return json.loads(line)
            except Exception as e:
                raise RuntimeError(f"Помилка читання відповіді від бекенду: {e}")

# --- Простий GameRunner віконце ---
class GameRunner(tk.Toplevel):
    def __init__(self, parent, backend: BackendProcess, level_path: str):
        super().__init__(parent)
        self.title(f"Гра — {os.path.basename(level_path)}")
        self.geometry("640x480")
        self.backend = backend
        self.level_path = level_path

        # UI
        top = ttk.Frame(self, padding=8)
        top.pack(fill="x")
        ttk.Button(top, text="Step", command=self.on_step).pack(side="left")
        ttk.Button(top, text="Send custom command", command=self.on_custom).pack(side="left", padx=6)
        ttk.Button(top, text="Stop backend", command=self.on_stop).pack(side="right")

        # Текстове поле для стану
        self.txt = tk.Text(self, wrap="none")
        self.txt.pack(fill="both", expand=True, padx=8, pady=8)
        self.txt.config(state="disabled")

        # Завантажуємо рівень у бекенд
        try:
            resp = self.backend.send_command({"action": "load_level", "path": self.level_path})
            self._append_text("Loaded level:\n")
            self._append_text(json.dumps(resp, ensure_ascii=False, indent=2) + "\n")
        except Exception as e:
            messagebox.showerror("Помилка", str(e))
            self.destroy()

    def _append_text(self, s: str):
        self.txt.config(state="normal")
        self.txt.insert("end", s + "\n")
        self.txt.see("end")
        self.txt.config(state="disabled")

    def on_step(self):
        # Надсилаємо команду step без команд (бекенд має виконати tick/step)
        try:
            resp = self.backend.send_command({"action": "step"})
            self._append_text("Step response:")
            self._append_text(json.dumps(resp, ensure_ascii=False, indent=2))
        except Exception as e:
            messagebox.showerror("Помилка", str(e))

    def on_custom(self):
        # Простий діалог для введення JSON команди
        txt = simpledialog.askstring("Команда", "Введіть JSON команду для бекенду (наприклад {\"action\":\"step\"}):", parent=self)
        if not txt:
            return
        try:
            cmd = json.loads(txt)
        except Exception as e:
            messagebox.showwarning("Невірний JSON", str(e))
            return
        try:
            resp = self.backend.send_command(cmd)
            self._append_text("Custom command response:")
            self._append_text(json.dumps(resp, ensure_ascii=False, indent=2))
        except Exception as e:
            messagebox.showerror("Помилка", str(e))

    def on_stop(self):
        self.backend.stop()
        self._append_text("Backend stopped.")
        # не закриваємо вікно автоматично


if __name__ == "__main__":
    app = MenuApp()
    app.mainloop()
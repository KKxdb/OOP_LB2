# frontend/menu.py
import os
import json
import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import subprocess
from game_window import GameWindow

# Константи
BASE_DIR = os.path.dirname(os.path.dirname(__file__))   # ..\project
BACKEND_EXEC = os.path.join(BASE_DIR, "backend", "build", "Release", "oop_backend.exe")
LEVELS_DIR = os.path.join(os.path.dirname(__file__), "levels")


#                          MAIN MENU (Toplevel)
class MenuApp(tk.Toplevel):
    def __init__(self, master):
        super().__init__(master)
        self.title("Фабрика роботів")
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

        btn_exit = ttk.Button(frame, text="Вихід", command=self.master.quit)
        btn_exit.pack(fill="x", pady=6)

    def open_levels(self):
        LevelsWindow(self.master)   
        self.destroy()             

    def create_level_dialog(self):
        self.destroy()
        from editor import LevelEditorApp
        LevelEditorApp()

#  CREATE LEVEL DIALOG (Toplevel)
# class CreateLevelDialog(tk.Toplevel):
#     def __init__(self, parent):
#         super().__init__(parent)
#         self.title("Створити рівень")
#         self.result = None
#         self.resizable(False, False)
#         self._build_ui()
#         self.grab_set()

    # def _build_ui(self):
    #     frm = ttk.Frame(self, padding=12)
    #     frm.pack(fill="both", expand=True)

    #     ttk.Label(frm, text="Назва рівня").grid(row=0, column=0, sticky="w")
    #     self.name_var = tk.StringVar()
    #     ttk.Entry(frm, textvariable=self.name_var, width=30).grid(row=0, column=1, pady=4)

    #     ttk.Label(frm, text="Ширина").grid(row=1, column=0, sticky="w")
    #     self.width_var = tk.IntVar(value=10)
    #     ttk.Entry(frm, textvariable=self.width_var, width=10).grid(row=1, column=1, pady=4, sticky="w")

    #     ttk.Label(frm, text="Висота").grid(row=2, column=0, sticky="w")
    #     self.height_var = tk.IntVar(value=10)
    #     ttk.Entry(frm, textvariable=self.height_var, width=10).grid(row=2, column=1, pady=4, sticky="w")

    #     btns = ttk.Frame(frm)
    #     btns.grid(row=3, column=0, columnspan=2, pady=(10, 0))

    #     ttk.Button(btns, text="Створити", command=self._on_create).pack(side="left", padx=6)
    #     ttk.Button(btns, text="Повернутись", command=self.go_back).pack(side="left", padx=6)

    # def _on_create(self):
    #     name = self.name_var.get().strip()
    #     if not name:
    #         messagebox.showwarning("Помилка", "Введіть назву рівня")
    #         return

    #     width = int(self.width_var.get())
    #     height = int(self.height_var.get())

    #     if width <= 0 or height <= 0:
    #         messagebox.showwarning("Помилка", "Розміри мають бути додатні")
    #         return

    #     self.result = (name, width, height)
    #     self.destroy()

    # def go_back(self):
    #     MenuApp(self.master)
    #     self.destroy()

#                            LEVELS WINDOW
class LevelsWindow(tk.Toplevel):
    def __init__(self, master):
        super().__init__(master)
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
        ttk.Button(btn_frame, text="Назад", command=self.go_back).pack(side="right")

    def go_back(self):
        MenuApp(self.master)
        self.destroy()

    def _load_levels(self):
        for child in self.list_frame.winfo_children():
            child.destroy()

        files = sorted(f for f in os.listdir(LEVELS_DIR) if f.endswith(".json"))
        if not files:
            ttk.Label(self.list_frame, text="Рівнів не знайдено").pack(pady=10)
            return

        for fname in files:
            row = ttk.Frame(self.list_frame, padding=6)
            row.pack(fill="x", pady=3)

            ttk.Label(row, text=fname).pack(side="left")
            ttk.Button(row, text="Почати", command=lambda f=fname: self._play_level(f)).pack(side="right")
            ttk.Button(row, text="Відкрити json файл", command=lambda f=fname: self._open_level(f)).pack(side="right", padx=6)

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
        path = os.path.join(LEVELS_DIR, filename)
        backend = BackendProcess()
        backend.start()

        resp = backend.send({"action": "load_level", "path": path})

        GameRunner(self.master, backend, path, resp, filename)
        self.destroy()


#                              JSON VIEWER
class Viewer(tk.Toplevel):
    def __init__(self, parent, title, data):
        super().__init__(parent)
        self.title(f"Перегляд — {title}")
        self.geometry("520x420")

        txt = tk.Text(self, wrap="none")
        txt.pack(fill="both", expand=True)

        txt.insert("1.0", json.dumps(data, ensure_ascii=False, indent=2))
        txt.config(state="disabled")

#    BACKEND PROCESS CONTROLLER
class BackendProcess:
    def __init__(self, exe_path=BACKEND_EXEC):
        self.exe_path = exe_path
        self.proc = None

    def start(self):
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

#                               GAME RUNNER
class GameRunner(tk.Toplevel):
    def __init__(self, master, backend: BackendProcess, level_path: str, initial_state, filename):
        super().__init__(master)
        self.title(filename)
        self.backend = backend
        self.level_path = level_path 
        self.initial_state = initial_state

        self.game = GameWindow(self, backend, initial_state)
        self.game.pack(fill="both", expand=True)

        top = ttk.Frame(self)
        top.pack(fill="x")
        ttk.Button(top, text="Почати", command=self.start_game).pack(side="left")
        ttk.Button(top, text="Повернутися", command=self.return_to_menu).pack(side="right")

        self.running = False

    def start_game(self):
        self.running = True
        self.game.set_edit_mode(False)
        self.run_loop()

    def return_to_menu(self):
        self.running = False
        self.backend.stop()
        MenuApp(self.master)
        self.destroy()

    def run_loop(self):
        if not self.running:
            return

        resp = self.backend.send({"action": "run_step"})

        if resp.get("finished"):
            self.running = False

            if resp.get("win"):
                self.show_end_dialog("Перемога!")
            else:
                self.show_end_dialog("Поразка!")

            return

        self.game.update_state(resp["state"])
        self.after(1000, self.run_loop)

    def show_end_dialog(self, text):
        win = tk.Toplevel(self)
        win.title("Гра завершена")
        win.geometry("300x150")
        win.transient(self)
        win.grab_set()

        label = tk.Label(win, text=text, font=("Arial", 16))
        label.pack(pady=20)

        ttk.Button(
            win,
            text="Почати заново",
            command=lambda: self.restart_level(win)
        ).pack(pady=5)

        ttk.Button(
            win,
            text="Повернутись у головне меню",
            command=lambda: self.back_to_menu(win)
        ).pack(pady=5)
    
    def back_to_menu(self, win):
        win.destroy()
        self.backend.stop()
        self.destroy()
        MenuApp(self.master)

    def restart_level(self, win):
        win.destroy()
      
        self.backend.stop()
        self.destroy()

        backend = BackendProcess()
        backend.start()

        resp = backend.send({
            "action": "load_level",
            "path": self.level_path
        })

        GameRunner(self.master, backend, self.level_path, resp)



        

#                               MAIN ENTRY

if __name__ == "__main__":
    root = tk.Tk()
    root.withdraw()  # ховаємо root
    MenuApp(root)
    root.mainloop()
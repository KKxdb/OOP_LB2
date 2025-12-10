import os
import json
import tkinter as tk
from tkinter import ttk, messagebox, filedialog

LEVELS_DIR = os.path.join(os.path.dirname(__file__), "levels")

CELL_SIZE = 40
GRID_W, GRID_H = 10, 10

# Типи клітинок
EMPTY = "empty"
WORKER = "worker"
CONTROLLER = "controller"
BOX = "box"
TARGET = "target"
WALL = "wall"

TOOLS = [EMPTY, WORKER, CONTROLLER, BOX, TARGET, WALL]

class LevelEditorApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Level Editor (10x10)")
        self.geometry(f"{CELL_SIZE*GRID_W+200}x{CELL_SIZE*GRID_H+50}")
        self._ensure_levels_dir()

        # Сітка рівня
        self.grid_data = [[EMPTY for _ in range(GRID_W)] for _ in range(GRID_H)]

        # Поточний інструмент
        self.current_tool = tk.StringVar(value=WORKER)

        self._build_ui()

    def _ensure_levels_dir(self):
        if not os.path.exists(LEVELS_DIR):
            os.makedirs(LEVELS_DIR, exist_ok=True)

    def _build_ui(self):
        frame = ttk.Frame(self)
        frame.pack(side="left", fill="y", padx=10, pady=10)

        ttk.Label(frame, text="Інструменти").pack(pady=5)
        for tool in TOOLS:
            ttk.Radiobutton(frame, text=tool.capitalize(),
                            variable=self.current_tool, value=tool).pack(anchor="w")

        ttk.Button(frame, text="Очистити", command=self.clear_grid).pack(pady=5)
        ttk.Button(frame, text="Зберегти рівень", command=self.save_level).pack(pady=5)
        ttk.Button(frame, text="Завантажити рівень", command=self.load_level).pack(pady=5)

        # Canvas для сітки
        self.canvas = tk.Canvas(self, width=CELL_SIZE*GRID_W, height=CELL_SIZE*GRID_H, bg="white")
        self.canvas.pack(side="right", padx=10, pady=10)
        self.canvas.bind("<Button-1>", self.on_click)

        self.draw_grid()

    def draw_grid(self):
        self.canvas.delete("all")
        for y in range(GRID_H):
            for x in range(GRID_W):
                cell_type = self.grid_data[y][x]
                color = self.get_color(cell_type)
                self.canvas.create_rectangle(
                    x*CELL_SIZE, y*CELL_SIZE,
                    (x+1)*CELL_SIZE, (y+1)*CELL_SIZE,
                    fill=color, outline="black"
                )

    def get_color(self, cell_type):
        return {
            EMPTY: "white",
            WORKER: "lightblue",
            CONTROLLER: "orange",
            BOX: "brown",
            TARGET: "green",
            WALL: "gray"
        }.get(cell_type, "white")

    def on_click(self, event):
        x = event.x // CELL_SIZE
        y = event.y // CELL_SIZE
        if 0 <= x < GRID_W and 0 <= y < GRID_H:
            self.grid_data[y][x] = self.current_tool.get()
            self.draw_grid()

    def clear_grid(self):
        self.grid_data = [[EMPTY for _ in range(GRID_W)] for _ in range(GRID_H)]
        self.draw_grid()

    def save_level(self):
        name = simpledialog.askstring("Назва рівня", "Введіть назву рівня:")
        if not name:
            return
        path = os.path.join(LEVELS_DIR, f"{name}.json")
        level_data = {
            "name": name,
            "width": GRID_W,
            "height": GRID_H,
            "grid": self.grid_data
        }
        with open(path, "w", encoding="utf-8") as f:
            json.dump(level_data, f, ensure_ascii=False, indent=2)
        messagebox.showinfo("Збережено", f"Рівень '{name}' збережено у {path}")

    def load_level(self):
        src = filedialog.askopenfilename(title="Виберіть рівень",
                                         initialdir=LEVELS_DIR,
                                         filetypes=[("JSON files","*.json")])
        if not src:
            return
        try:
            with open(src, "r", encoding="utf-8") as f:
                data = json.load(f)
            self.grid_data = data.get("grid", self.grid_data)
            self.draw_grid()
        except Exception as e:
            messagebox.showerror("Помилка", str(e))

if __name__ == "__main__":
    app = LevelEditorApp()
    app.mainloop()
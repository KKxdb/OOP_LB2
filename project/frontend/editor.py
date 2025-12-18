import os
import json
import tkinter as tk
from tkinter import ttk, messagebox, simpledialog, filedialog
from menu import MenuApp
from tkinter import Menu

# ================== CONSTANTS ==================

GRID_W = 10
GRID_H = 10
CELL_SIZE = 40

LEVELS_DIR = os.path.join(os.path.dirname(__file__), "levels")

# ================== EDITOR ==================

class LevelEditorApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Редактор рівнів")
        self.geometry(f"{GRID_W*CELL_SIZE+220}x{GRID_H*CELL_SIZE+20}")

        os.makedirs(LEVELS_DIR, exist_ok=True)

        # State
        self.walls = set()
        self.box = None
        self.target = None
        self.robots = []   # {type, x, y, program}

        self._build_ui()
        self.draw()

    # ---------------- UI ----------------

    def _build_ui(self):
        left = ttk.Frame(self, padding=10)
        left.pack(side="left", fill="y")

        ttk.Button(left, text="Новий рівень", command=self.clear_level).pack(fill="x", pady=2)
        ttk.Button(left, text="Зберегти", command=self.save_level).pack(fill="x", pady=2)
        ttk.Button(left, text="Завантажити", command=self.load_level).pack(fill="x", pady=2)
        ttk.Button(left, text="Повернутись у меню", command=self.back_to_menu).pack(fill="x", pady=10)


        self.canvas = tk.Canvas(
            self,
            width=GRID_W * CELL_SIZE,
            height=GRID_H * CELL_SIZE,
            bg="white"
        )
        self.canvas.pack(side="right", padx=10)
        self.canvas.bind("<Button-1>", self.on_click)

    def back_to_menu(self):
        self.destroy()
        root = tk.Tk()
        root.withdraw()
        MenuApp(root)
        root.mainloop()


    # ---------------- DRAW ----------------

    def draw(self):
        self.canvas.delete("all")

        # Сітка
        for y in range(GRID_H):
            for x in range(GRID_W):
                self.canvas.create_rectangle(
                    x * CELL_SIZE, y * CELL_SIZE,
                    (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE,
                    outline="gray"
                )

        # Стіни
        for x, y in self.walls:
            self._cell(x, y, "gray")

        # Коробка
        if self.box:
            self._cell(*self.box, "brown")

        # Ціль
        if self.target:
            self._cell(*self.target, "green")

        # Роботи
        for r in self.robots:       
            self._robot(r)

    def _robot(self, r):
        x, y = r["x"], r["y"]
        cx = x * CELL_SIZE + CELL_SIZE // 2
        cy = y * CELL_SIZE + CELL_SIZE // 2

        color = "blue" if r["type"] == "worker" else "orange"

        self.canvas.create_oval(
            cx - 14, cy - 14,
            cx + 14, cy + 14,
            fill=color,
            outline="black"
        )

        if "dir" in r:
            dx, dy = 0, -16
            if r["dir"] == "right": dx, dy = 16, 0
            elif r["dir"] == "down": dx, dy = 0, 16
            elif r["dir"] == "left": dx, dy = -16, 0

            self.canvas.create_line(
                cx, cy,
                cx + dx, cy + dy,
                width=3,
                arrow=tk.LAST
            )


    # ---------------- LOGIC ----------------

    def on_click(self, event):
        x = event.x // CELL_SIZE
        y = event.y // CELL_SIZE

        if not (0 <= x < GRID_W and 0 <= y < GRID_H):
            return

        menu = Menu(self, tearoff=0)

        # === СТАТИЧНІ ОБʼЄКТИ ===
        menu.add_command(label="Поставити стіну", command=lambda: self.set_wall(x, y))
        menu.add_command(label="Поставити коробку", command=lambda: self.set_box(x, y))
        menu.add_command(label="Поставити ціль", command=lambda: self.set_target(x, y))
        menu.add_separator()

        # === РОБОТИ ===
        menu.add_command(label="Створити Worker", command=lambda: self.add_robot("worker", x, y))
        menu.add_command(label="Створити Controller", command=lambda: self.add_robot("controller", x, y))
        menu.add_separator()

        menu.add_command(label="Очистити клітинку", command=lambda: self.clear_cell(x, y))

        menu.post(event.x_root, event.y_root)

    def clear_cell(self, x, y):
        self.walls.discard((x, y))

        if self.box == (x, y):
            self.box = None

        if self.target == (x, y):
            self.target = None

        self.robots = [r for r in self.robots if not (r["x"] == x and r["y"] == y)]

        self.draw()

    def set_wall(self, x, y):
        self.clear_cell(x, y)
        self.walls.add((x, y))
        self.draw()

    def set_box(self, x, y):
        self.clear_cell(x, y)
        self.box = (x, y)
        self.draw()

    def set_target(self, x, y):
        self.clear_cell(x, y)
        self.target = (x, y)
        self.draw()

    def add_robot(self, robot_type, x, y):
        self.clear_cell(x, y)

        robot = {
            "type": robot_type,
            "x": x,
            "y": y
        }

        # напрям — опціонально
        dir_ = self.ask_direction_dialog()
        if dir_:
            robot["dir"] = dir_

        # контролер — дія
        if robot_type == "controller":
            cmd = self.ask_controller_action()
            if cmd:
                robot["command"] = cmd

        self.robots.append(robot)
        self.draw()



    def _cell(self, x, y, color):
        self.canvas.create_rectangle(
            x * CELL_SIZE,
            y * CELL_SIZE,
            (x + 1) * CELL_SIZE,
            (y + 1) * CELL_SIZE,
            fill=color,
            outline="black"
        )

    def ask_direction_dialog(self):
        win = tk.Toplevel(self)
        win.title("Напрям руху")
        win.grab_set()

        result = {"dir": None}

        def set_dir(d):
            result["dir"] = d
            win.destroy()

        tk.Button(win, text="↑ Вгору", width=15, command=lambda: set_dir("up")).grid(row=0, column=1)
        tk.Button(win, text="← Ліворуч", width=15, command=lambda: set_dir("left")).grid(row=1, column=0)
        tk.Button(win, text="Праворуч →", width=15, command=lambda: set_dir("right")).grid(row=1, column=2)
        tk.Button(win, text="↓ Вниз", width=15, command=lambda: set_dir("down")).grid(row=2, column=1)

        win.wait_window()
        return result["dir"]

    def ask_controller_action(self):
        win = tk.Toplevel(self)
        win.title("Дія контролера")
        win.grab_set()

        result = {"cmd": None}

        def set_cmd(c):
            result["cmd"] = c
            win.destroy()

        tk.Button(win, text="Повернути за часовою стрілкою", width=20,
                  command=lambda: set_cmd("rotate_cw")).pack()
        tk.Button(win, text="Повернути проти часов. стрілки", width=20,
                  command=lambda: set_cmd("rotate_ccw")).pack()

        win.wait_window()
        return result["cmd"]

    # ---------------- FILES ----------------

    def clear_level(self):
        self.walls.clear()
        self.box = None
        self.target = None
        self.robots.clear()
        self.draw()

    def save_level(self):
        name = simpledialog.askstring("Назва рівня", "Введіть назву рівня:")
        if not name:
            return

        data = {
            "width": GRID_W,
            "height": GRID_H,
            "world": {
                # стіни — [x, y]
                "walls": [[x, y] for (x, y) in self.walls],

                # коробка (0 або 1)
                "boxes": [[self.box[0], self.box[1]]] if self.box else [],

                # ціль (0 або 1)
                "targets": [[self.target[0], self.target[1]]] if self.target else [],

                # роботи — як dict
                "robots": [
                    {
                        "type": r["type"],
                        "x": r["x"],
                        "y": r["y"],
                        "program": r.get("program", [])
                    }
                    for r in self.robots
                ]
            }
        }

        path = os.path.join(LEVELS_DIR, f"{name}.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=False)

        messagebox.showinfo("OK", "Рівень збережено")



    def load_level(self):
        path = filedialog.askopenfilename(
            initialdir=LEVELS_DIR,
            filetypes=[("JSON files", "*.json")]
        )
        if not path:
            return

        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)

        world = data.get("world", {})

        self.walls = {tuple(w) for w in world.get("walls", [])}

        boxes = world.get("boxes", [])
        self.box = tuple(boxes[0]) if boxes else None

        targets = world.get("targets", [])
        self.target = tuple(targets[0]) if targets else None

        self.robots = world.get("robots", [])

        self.draw()



# ================== RUN ==================

if __name__ == "__main__":
    LevelEditorApp().mainloop()

import tkinter as tk
import math
from tkinter import simpledialog, Menu

CELL_SIZE = 40

class GameWindow(tk.Frame):
    def __init__(self, parent, backend, init_state):
        super().__init__(parent)

        self.backend = backend
        self.cell = CELL_SIZE   # 🔥 Головне виправлення!

        # init_state може бути {"status":..., "state": ...}
        if isinstance(init_state, dict) and "state" in init_state:
            state = init_state["state"]
        else:
            state = init_state

        self.width = state["width"]
        self.height = state["height"]

        self.canvas = tk.Canvas(
            self,
            width=self.width * self.cell,
            height=self.height * self.cell,
            bg="white"
        )
        self.canvas.pack()

        self.draw_grid()
        self.draw_objects(state)

        # 🔥 Клік миші
        self.canvas.bind("<Button-1>", self.on_click)

    # -----------------------------------------------------------
    #                   DRAWING
    # -----------------------------------------------------------

    def draw_grid(self):
        for x in range(self.width):
            for y in range(self.height):
                self.canvas.create_rectangle(
                    x * self.cell,
                    y * self.cell,
                    (x + 1) * self.cell,
                    (y + 1) * self.cell,
                    outline="gray"
                )

    def draw_objects(self, state):
        self.canvas.delete("obj")

        # Walls
        for w in state["walls"]:
            self._rect(w["x"], w["y"], "black")

        # Targets
        for t in state["targets"]:
            self._rect(t["x"], t["y"], "green")

        # Boxes
        for b in state["boxes"]:
            self._rect(b["x"], b["y"], "brown")

        # Robots
        for r in state["robots"]:
            self._robot(r)


    def _robot(self, r):
        x, y = r["x"], r["y"]
        cx = x * self.cell + self.cell // 2
        cy = y * self.cell + self.cell // 2

        # Колір робота
        if r["type"] == "worker":
            color = "blue"
        elif r["type"] == "controller":
            color = "orange"  # помаранчевий контролер
        else:
            color = "gray"

        # Тіло робота (круг)
        self.canvas.create_oval(
            cx - 14, cy - 14, cx + 14, cy + 14,
            fill=color,
            tags="obj"
        )

        # Напрямок
        d = r.get("dir", "up")

        dx, dy = 0, -16
        if d == "right": dx, dy = 16, 0
        elif d == "down": dx, dy = 0, 16
        elif d == "left": dx, dy = -16, 0

        # Стрілка напрямку
        self.canvas.create_line(
            cx, cy, cx + dx, cy + dy,
            width=3,
            arrow=tk.LAST,
            fill="black",
            tags="obj"
    )


    def _rect(self, x, y, color):
        self.canvas.create_rectangle(
            x * self.cell + 4, y * self.cell + 4,
            (x + 1) * self.cell - 4, (y + 1) * self.cell - 4,
            fill=color,
            tags="obj"
        )

    # -----------------------------------------------------------
    #                   BACKEND UPDATE
    # -----------------------------------------------------------

    def update_state(self, new_state):
        if isinstance(new_state, dict) and "state" in new_state:
            state = new_state["state"]
        else:
            state = new_state

        self.draw_objects(state)

    # -----------------------------------------------------------
    #              CLICK HANDLER — SPAWN ROBOT
    # -----------------------------------------------------------

    def on_click(self, event):
        x = event.x // self.cell
        y = event.y // self.cell

        menu = Menu(self, tearoff=0)
        menu.add_command(label="Створити Worker", command=lambda: self.spawn_worker(x, y))
        menu.add_command(label="Створити Controller", command=lambda: self.spawn_controller(x, y))

        menu.post(event.x_root, event.y_root)

    # -----------------------------------------------------------
    #               POPUPS FOR ROBOT OPTIONS
    # -----------------------------------------------------------

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

    def spawn_worker(self, x, y):
        dir_ = self.ask_direction_dialog()
        if not dir_:
            return

        resp = self.backend.send({
            "action": "spawn_robot",
            "type": "worker",
            "dir": dir_,
            "x": x,
            "y": y
        })

        self.update_state(resp)

    def spawn_controller(self, x, y):
        dir_ = self.ask_direction_dialog()
        if not dir_:
            return

        cmd = self.ask_controller_action()
        if not cmd:
            return

        resp = self.backend.send({
            "action": "spawn_robot",
            "type": "controller",
            "dir": dir_,
            "command": cmd,
            "x": x,
            "y": y
        })

        self.update_state(resp)

    def ask_controller_action(self):
        win = tk.Toplevel(self)
        win.title("Дія контролера")
        win.grab_set()

        result = {"cmd": None}

        def set_cmd(c):
            result["cmd"] = c
            win.destroy()

        tk.Button(win, text="Повернути CW", width=20,
                  command=lambda: set_cmd("rotate_cw")).pack()
        tk.Button(win, text="Повернути CCW", width=20,
                  command=lambda: set_cmd("rotate_ccw")).pack()
        tk.Button(win, text="Прискорити", width=20,
                  command=lambda: set_cmd("boost")).pack()

        win.wait_window()
        return result["cmd"]

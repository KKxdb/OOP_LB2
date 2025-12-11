import tkinter as tk
from tkinter import ttk
import json

CELL_SIZE = 40
GRID_W = 10
GRID_H = 10


class GameWindow(tk.Frame):
    def __init__(self, parent, level_data):
        super().__init__(parent)

        self.level_data = level_data
        self.canvas = tk.Canvas(self, width=GRID_W * CELL_SIZE, height=GRID_H * CELL_SIZE, bg="white")
        self.canvas.pack()

        self.draw_grid()
        self.draw_objects(level_data)

    def draw_grid(self):
        for x in range(GRID_W):
            for y in range(GRID_H):
                self.canvas.create_rectangle(
                    x * CELL_SIZE,
                    y * CELL_SIZE,
                    (x + 1) * CELL_SIZE,
                    (y + 1) * CELL_SIZE,
                    outline="gray"
                )

    def draw_objects(self, data):
        self.canvas.delete("obj")

        # Walls
        for w in data.get("walls", []):
            x, y = w["x"], w["y"]
            self.canvas.create_rectangle(
                x * CELL_SIZE, y * CELL_SIZE,
                (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE,
                fill="black", tags="obj"
            )

        # Boxes
        for b in data.get("boxes", []):
            x, y = b["x"], b["y"]
            self.canvas.create_rectangle(
                x * CELL_SIZE + 5, y * CELL_SIZE + 5,
                (x + 1) * CELL_SIZE - 5, (y + 1) * CELL_SIZE - 5,
                fill="brown", tags="obj"
            )

        # Targets
        for t in data.get("targets", []):
            x, y = t["x"], t["y"]
            self.canvas.create_oval(
                x * CELL_SIZE + 10, y * CELL_SIZE + 10,
                (x + 1) * CELL_SIZE - 10, (y + 1) * CELL_SIZE - 10,
                outline="green", width=3, tags="obj"
            )

        # Robots
        for r in data.get("robots", []):
            x, y = r["x"], r["y"]
            self.canvas.create_oval(
                x * CELL_SIZE + 5, y * CELL_SIZE + 5,
                (x + 1) * CELL_SIZE - 5, (y + 1) * CELL_SIZE - 5,
                fill="blue", tags="obj"
            )

    def update_state(self, data):
        self.draw_objects(data)

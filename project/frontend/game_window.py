import tkinter as tk

CELL_SIZE = 40

class GameWindow(tk.Frame):
    def __init__(self, parent, init_state):
        super().__init__(parent)

        # Вхідні дані можуть бути або словником виду {"status":..., "state": {...}}
        # або безпосередньо самим об'єктом state. Робимо гнучку обробку.
        if isinstance(init_state, dict) and "state" in init_state:
            state = init_state["state"]
        else:
            state = init_state

        self.width = state["width"]
        self.height = state["height"]

        self.canvas = tk.Canvas(
            self,
            width=self.width * CELL_SIZE,
            height=self.height * CELL_SIZE,
            bg="white"
        )
        self.canvas.pack()

        self.draw_grid()
        self.draw_objects(state)

    def draw_grid(self):
        for x in range(self.width):
            for y in range(self.height):
                self.canvas.create_rectangle(
                    x * CELL_SIZE,
                    y * CELL_SIZE,
                    (x + 1) * CELL_SIZE,
                    (y + 1) * CELL_SIZE,
                    outline="gray"
                )

    def draw_objects(self, state):
        self.canvas.delete("obj")

        for w in state["walls"]:
            self._rect(w["x"], w["y"], "black")

        for t in state["targets"]:
            self._rect(t["x"], t["y"], "green")

        for b in state["boxes"]:
            self._rect(b["x"], b["y"], "brown")

        for r in state["robots"]:
            self._rect(r["x"], r["y"], "blue")

    def _rect(self, x, y, color):
        self.canvas.create_rectangle(
            x * CELL_SIZE + 4, y * CELL_SIZE + 4,
            (x + 1) * CELL_SIZE - 4, (y + 1) * CELL_SIZE - 4,
            fill=color,
            tags="obj"
        )

    def update_state(self, new_state):
        # Також приймаємо або повний об'єкт відповіді, або безпосередньо state
        if isinstance(new_state, dict) and "state" in new_state:
            state = new_state["state"]
        else:
            state = new_state

        self.draw_objects(state)
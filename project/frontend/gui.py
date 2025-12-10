import tkinter as tk
from tkinter import ttk, messagebox
from client import BackendClient

class App(tk.Tk):
    def __init__(self, backend_host='127.0.0.1', backend_port=5000):
        super().__init__()
        self.title("OOP Lab Frontend")
        self.geometry("400x250")
        self.client = BackendClient(host=backend_host, port=backend_port)
        self._build_ui()

    def _build_ui(self):
        frame = ttk.Frame(self, padding=10)
        frame.pack(fill='both', expand=True)

        self.input_var = tk.StringVar(value='status')
        ttk.Label(frame, text="Action").grid(row=0, column=0, sticky='w')
        ttk.Entry(frame, textvariable=self.input_var).grid(row=0, column=1, sticky='ew')

        send_btn = ttk.Button(frame, text="Send", command=self.on_send)
        send_btn.grid(row=1, column=0, columnspan=2, pady=10)

        self.output = tk.Text(frame, height=8, wrap='word')
        self.output.grid(row=2, column=0, columnspan=2, sticky='nsew')

        frame.columnconfigure(1, weight=1)
        frame.rowconfigure(2, weight=1)

    def on_send(self):
        action = self.input_var.get().strip()
        if not action:
            messagebox.showwarning("Input", "Enter an action")
            return
        try:
            resp = self.client.send_request({"action": action})
            self.output.delete('1.0', tk.END)
            self.output.insert(tk.END, str(resp))
        except Exception as e:
            messagebox.showerror("Error", str(e))

if __name__ == '__main__':
    app = App()
    app.mainloop()
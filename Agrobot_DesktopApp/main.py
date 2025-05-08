88% of storage used … If you run out, you can't create, edit, and upload files. Get 100 GB of storage for $1.99 $0.49/month for 2 months.
import tkinter as tk
from tkinter import scrolledtext
import serial
import serial.tools.list_ports
import random  
import time  

class CNCFarmingApp:
    def __init__(self, root):
        self.root = root
        self.root.title("AGRO: CNC Farming System")
        self.root.attributes('-fullscreen', True)  
        self.root.bind("<Escape>", self.exit_fullscreen)  

        # Main container frame to center everything
        self.main_frame = tk.Frame(root)
        self.main_frame.pack(expand=True)

        # CNC Console
        self.console = scrolledtext.ScrolledText(self.main_frame, height=15, width=80, state='disabled')
        self.console.pack(pady=10)

        # CNC Position Display
        self.position_label = tk.Label(self.main_frame, text="X: 0  Y: 0  Z: 0", font=("Arial", 14), fg="blue")
        self.position_label.pack(pady=5)

        # Command Entry & Send Button
        command_frame = tk.Frame(self.main_frame)
        command_frame.pack(pady=5)

        self.command_entry = tk.Entry(command_frame, width=60)
        self.command_entry.pack(side=tk.LEFT, padx=5)

        self.send_button = tk.Button(command_frame, text="Send", command=self.send_command)
        self.send_button.pack(side=tk.LEFT)

        # Jog Control Buttons
        self.create_jog_controls()

        # Check Moisture Button
        self.check_moisture_button = tk.Button(self.main_frame, text="Check Moisture", font=("Arial", 12),
                                               command=self.update_soil_moisture)
        self.check_moisture_button.pack(pady=10)

        # Planting Grid (3x3) with Soil Moisture
        self.create_planting_grid()

        # Connect to Serial Port
        self.serial_conn = self.connect_serial()

        if self.serial_conn:
            self.root.after(1000, self.read_serial)  

    def exit_fullscreen(self, event=None):
        """Exit full-screen mode when the Escape key is pressed."""
        self.root.attributes('-fullscreen', False)

    def create_jog_controls(self):
        """Creates jog control buttons for X, Y, Z movement."""
        jog_frame = tk.Frame(self.main_frame)
        jog_frame.pack(pady=10)

        tk.Button(jog_frame, text="Y+", width=8, height=2, command=lambda: self.jog("Y1")).grid(row=0, column=1)
        tk.Button(jog_frame, text="X-", width=8, height=2, command=lambda: self.jog("X-1")).grid(row=1, column=0)
        tk.Button(jog_frame, text="X+", width=8, height=2, command=lambda: self.jog("X1")).grid(row=1, column=2)
        tk.Button(jog_frame, text="Y-", width=8, height=2, command=lambda: self.jog("Y-1")).grid(row=2, column=1)
        tk.Button(jog_frame, text="Z+", width=8, height=2, command=lambda: self.jog("Z5")).grid(row=0, column=3)
        tk.Button(jog_frame, text="Z-", width=8, height=2, command=lambda: self.jog("Z-5")).grid(row=1, column=3)

    def create_planting_grid(self):
        """Creates a 3x3 planting grid with soil moisture readings."""
        self.planting_frame = tk.Frame(self.main_frame)
        self.planting_frame.pack(pady=20)

        self.grid_positions = {
            (0, 0): "X3 Y2", (0, 1): "X3 Y9", (0, 2): "X3 Y15 Z-50",
            (1, 0): "X20 Y2", (1, 1): "X20 Y9", (1, 2): "X20 Y15",
            (2, 0): "X38 Y2", (2, 1): "X38 Y9", (2, 2): "X38 Y15"
        }

        self.moisture_labels = {}

        for row in range(3):
            for col in range(3):
                position = self.grid_positions[(row, col)]
                
                cell_frame = tk.Frame(self.planting_frame, borderwidth=2, relief="ridge")
                cell_frame.grid(row=row, column=col, padx=5, pady=5)

                move_button = tk.Button(cell_frame, text=f"({row+1}, {col+1})", width=12, height=2,
                                        command=lambda pos=position: self.move_to_slot(pos))
                move_button.pack()

                moisture_label = tk.Label(cell_frame, text="Moisture: --%", font=("Arial", 10))
                moisture_label.pack()
                
                self.moisture_labels[(row, col)] = moisture_label

    def update_soil_moisture(self):
        """Updates soil moisture data for each planting slot."""
        for row in range(3):
            for col in range(3):
                moisture_value = random.randint(20, 80)
                self.moisture_labels[(row, col)].config(text=f"Moisture: {moisture_value}%")

        self.log_message("Soil moisture updated!")

    def move_to_slot(self, position):
        """Moves CNC to the selected planting slot."""
        self.send_command_to_cnc(f"G90 G0 {position}")

    def connect_serial(self):
        """Auto-detects and connects to an available CNC serial port."""
        ports = serial.tools.list_ports.comports()
        for port in ports:
            if "USB" in port.description:
                try:
                    conn = serial.Serial(port.device, 115200, timeout=1)
                    self.log_message(f"Connected to {port.device}")
                    return conn
                except serial.SerialException as e:
                    self.log_message(f"Error connecting: {e}")
        self.log_message("No CNC device found.")
        return None

    def send_command(self):
        """Sends a manual G-code command from the console."""
        command = self.command_entry.get().strip()
        if command:
            self.send_command_to_cnc(command)

    def send_command_to_cnc(self, command):
        """Sends a command to the CNC and logs it."""
        if self.serial_conn:
            self.serial_conn.write((command + "\n").encode())
            self.log_message(f"TX: {command}")  
        else:
            self.log_message("Error: No CNC connection.")

    def jog(self, direction):
        """Moves the CNC system using jog buttons."""
        self.send_command_to_cnc(f"G91 G0 {direction}")

    def read_serial(self):
        """Reads data from CNC and updates position if applicable."""
        if self.serial_conn and self.serial_conn.in_waiting:
            response = self.serial_conn.readline().decode().strip()
            self.log_message(f"RX: {response}")  

            if "MPos:" in response:
                try:
                    pos_data = response.split("MPos:")[1].split(",")
                    x, y, z = float(pos_data[0]), float(pos_data[1]), float(pos_data[2])
                    self.position_label.config(text=f"X: {x:.2f}  Y: {y:.2f}  Z: {z:.2f}")
                except Exception as e:
                    self.log_message(f"Position error: {e}")

        self.root.after(500, self.read_serial)  

    def log_message(self, message):
        """Logs messages to the console."""
        self.console.config(state='normal')
        self.console.insert(tk.END, message + "\n")
        self.console.config(state='disabled')
        self.console.yview(tk.END)

if __name__ == "__main__":
    root = tk.Tk()
    app = CNCFarmingApp(root)
    root.mainloop()

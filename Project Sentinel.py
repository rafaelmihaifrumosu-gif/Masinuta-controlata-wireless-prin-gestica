import cv2
import mediapipe as mp
import time
import tkinter as tk
from tkinter import font, ttk
from PIL import Image, ImageTk
import asyncio
import threading
from bleak import BleakScanner, BleakClient

# --- Setari BLE ---
CHAR_UUID = "0000ff01-0000-1000-8000-00805f9b34fb"

class RoverControlGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Sistem Control Rover - Parcare Automata si Caroserie")
        self.root.geometry("1100x700")
        self.root.configure(bg="#2c3e50")
        
        self.root.grid_columnconfigure(0, weight=6) 
        self.root.grid_columnconfigure(1, weight=4) 
        self.root.grid_rowconfigure(0, weight=1)
        
        self.bt_conectat = False
        self.client_ble = None
        self.loop_ble = asyncio.new_event_loop()
        
        self.thread_ble = threading.Thread(target=self.start_async_loop, daemon=True)
        self.thread_ble.start()

        self.mp_hands = mp.solutions.hands
        self.hands = self.mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)
        self.mp_draw = mp.solutions.drawing_utils
        
        self.tip_ids = [4, 8, 12, 16, 20] 
        self.ultima_comanda = None

        self.cap = cv2.VideoCapture(0)
        self.setup_ui()
        self.update_frame()
        self.root.protocol("WM_DELETE_WINDOW", self.inchide_aplicatie)

    def start_async_loop(self):
        asyncio.set_event_loop(self.loop_ble)
        self.loop_ble.run_forever()

    def setup_ui(self):
        font_titlu = font.Font(family="Helvetica", size=14, weight="bold")
        font_status = font.Font(family="Helvetica", size=11)
        font_mare = font.Font(family="Helvetica", size=16, weight="bold")
        font_butoane = font.Font(family="Helvetica", size=10, weight="bold")

        frame_stanga = tk.Frame(self.root, bg="#34495e")
        frame_stanga.grid(row=0, column=0, sticky="nsew", padx=10, pady=10)
        frame_stanga.grid_rowconfigure(0, weight=1)
        frame_stanga.grid_columnconfigure(0, weight=1)
        
        self.video_label = tk.Label(frame_stanga, bg="black")
        self.video_label.grid(row=0, column=0)
        
        frame_dreapta = tk.Frame(self.root, bg="#2c3e50")
        frame_dreapta.grid(row=0, column=1, sticky="nsew", padx=10, pady=10)
        
        self.lbl_status = tk.Label(frame_dreapta, text="CONECTATI BLUETOOTH BLE", fg="#f39c12", bg="#2c3e50", font=font_status)
        self.lbl_status.pack(pady=(5, 5))
        
        frame_conectare = tk.Frame(frame_dreapta, bg="#2c3e50")
        frame_conectare.pack(pady=5, fill=tk.X, padx=20)

        self.combo_dispozitive = ttk.Combobox(frame_conectare, state="readonly")
        self.combo_dispozitive.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 5))

        btn_scan = tk.Button(frame_conectare, text="Scan BLE", bg="#3498db", fg="white", command=self.porneste_scanare)
        btn_scan.pack(side=tk.LEFT, padx=(0, 5))

        self.btn_conectare = tk.Button(frame_conectare, text="Conectare", bg="#27ae

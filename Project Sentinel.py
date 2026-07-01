import cv2
import mediapipe as mp
import serial
import time
import tkinter as tk
from PIL import Image, ImageTk

# ---------------- SERIAL ----------------
PORT_SERIAL = '/dev/ttyUSB0'
BAUD_RATE = 9600

try:
    esp32 = serial.Serial(PORT_SERIAL, BAUD_RATE, timeout=1)
    time.sleep(2)
    serial_conectat = True
except:
    serial_conectat = False
    esp32 = None

# ---------------- MEDIA PIPE ----------------
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)
mp_draw = mp.solutions.drawing_utils

tip_ids = [8, 12, 16, 20]

# ---------------- CAMERA ----------------
cap = cv2.VideoCapture(0)

# ---------------- STATE ----------------
ultima_comanda = "S"
killswitch = False

last_time = time.time()
fps = 0

# LED animation state
led_size = 10
led_growing = True

# ---------------- MAP COMENZI ----------------
command_map = {
    "S": ("STOP", "🛑"),
    "F": ("INAINTE", "⬆"),
    "L": ("STANGA", "⬅"),
    "R": ("DREAPTA", "➡"),
    "B": ("INAPOI", "⬇")
}

# ---------------- UI ----------------
root = tk.Tk()
root.title("Rover Control Dashboard")
root.geometry("1000x650")
root.configure(bg="#1e1e1e")

video_label = tk.Label(root)
video_label.pack(side=tk.LEFT, padx=10, pady=10)

# ---------------- DASHBOARD PANEL ----------------
panel = tk.Frame(root, bg="#2b2b2b", width=300, height=650)
panel.pack(side=tk.RIGHT, fill=tk.Y)
panel.pack_propagate(False)

title = tk.Label(panel, text="ROVER DASHBOARD",
                 font=("Arial", 16, "bold"),
                 fg="white", bg="#2b2b2b")
title.pack(pady=10)

status_label = tk.Label(panel, text="ESP32",
                        font=("Arial", 12),
                        fg="white", bg="#2b2b2b")
status_label.pack(pady=5)

command_label = tk.Label(panel, text="Comandă",
                         font=("Arial", 16, "bold"),
                         fg="white", bg="#2b2b2b",
                         wraplength=260,
                         justify="center")
command_label.pack(pady=25, fill="x")

fps_label = tk.Label(panel, text="FPS: 0",
                     font=("Arial", 12),
                     fg="white", bg="#2b2b2b")
fps_label.pack(pady=5)

# ---------------- LED ANIMAT ----------------
indicator = tk.Canvas(panel, width=40, height=40,
                      bg="#2b2b2b", highlightthickness=0)

indicator_circle = indicator.create_oval(10, 10, 30, 30,
                                         fill="red",
                                         outline="")

indicator.pack(pady=10)

# ---------------- KILLSWITCH ----------------
def toggle_killswitch():
    global killswitch

    killswitch = not killswitch

    if killswitch:
        ks_button.config(text="KILLSWITCH: ON ❌", bg="red")
        panel.config(bg="#5a1e1e")
        title.config(bg="#5a1e1e")
        status_label.config(bg="#5a1e1e")
        command_label.config(bg="#5a1e1e")
        fps_label.config(bg="#5a1e1e")
    else:
        ks_button.config(text="KILLSWITCH: OFF ✅", bg="green")
        panel.config(bg="#2b2b2b")
        title.config(bg="#2b2b2b")
        status_label.config(bg="#2b2b2b")
        command_label.config(bg="#2b2b2b")
        fps_label.config(bg="#2b2b2b")


ks_button = tk.Button(panel,
                      text="KILLSWITCH: OFF ✅",
                      font=("Arial", 12, "bold"),
                      bg="green",
                      fg="white",
                      command=toggle_killswitch)
ks_button.pack(pady=15)

# ---------------- DETECTARE GESTURI ----------------
def detect_gesture(frame):
    global ultima_comanda

    frame = cv2.flip(frame, 1)

    if killswitch:
        return frame, "KILL"

    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    result = hands.process(rgb)

    comanda = "S"

    if result.multi_hand_landmarks:
        for hand in result.multi_hand_landmarks:
            mp_draw.draw_landmarks(frame, hand, mp_hands.HAND_CONNECTIONS)

            fingers = []

            for id in tip_ids:
                if hand.landmark[id].y < hand.landmark[id - 2].y:
                    fingers.append(1)
                else:
                    fingers.append(0)

            nr = fingers.count(1)

            if nr == 4:
                comanda = "F"
            elif nr == 0:
                comanda = "S"
            elif fingers == [1, 0, 0, 0]:
                comanda = "L"
            elif fingers == [1, 1, 0, 0]:
                comanda = "R"
            elif nr == 3:
                comanda = "B"

    if serial_conectat and not killswitch and comanda != ultima_comanda:
        esp32.write(comanda.encode())
        ultima_comanda = comanda

    return frame, comanda


# ---------------- LED ANIMATION ----------------
def animate_led():
    global led_size, led_growing

    # 🧠 noua logică: LED = stare analiză gesturi
    if killswitch:
        color = "#ff0000"   # roșu = analiză oprită
    else:
        color = "#00ff00"   # verde = analiză pornită

    # pulse logic
    if led_growing:
        led_size += 1
        if led_size >= 14:
            led_growing = False
    else:
        led_size -= 1
        if led_size <= 8:
            led_growing = True

    x1 = 20 - led_size // 2
    y1 = 20 - led_size // 2
    x2 = 20 + led_size // 2
    y2 = 20 + led_size // 2

    indicator.coords(indicator_circle, x1, y1, x2, y2)
    indicator.itemconfig(indicator_circle, fill=color)

    root.after(80, animate_led)


# ---------------- UPDATE UI ----------------
def update():
    global fps, last_time

    ret, frame = cap.read()

    if ret:
        frame, comanda = detect_gesture(frame)

        # FPS
        new_time = time.time()
        fps = int(1 / (new_time - last_time + 1e-6))
        last_time = new_time

        # VIDEO
        img = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        img = Image.fromarray(img)
        imgtk = ImageTk.PhotoImage(image=img)

        video_label.imgtk = imgtk
        video_label.configure(image=imgtk)

        # COMMAND DISPLAY
        if killswitch:
            command_label.config(
                text="⛔\nKILLSWITCH ACTIV\nCONTROL OPRIT",
                fg="red"
            )
        else:
            name, icon = command_map.get(comanda, ("UNKNOWN", "?"))
            command_label.config(
                text=f"{icon} {name}\n({comanda})",
                fg="white"
            )

        # STATUS ESP32
        if serial_conectat:
            status_label.config(text="ESP32: CONECTAT", fg="lime")
        else:
            status_label.config(text="ESP32: OFFLINE", fg="red")

        fps_label.config(text=f"FPS: {fps}")

    root.after(15, update)


# ---------------- START ----------------
update()
animate_led()
root.mainloop()

# ---------------- CLEANUP ----------------
cap.release()
if serial_conectat:
    esp32.close()
import cv2
import mediapipe as mp
import serial
import time
import tkinter as tk
from tkinter import font
from PIL import Image, ImageTk

# --- Setări Seriale ---
PORT_SERIAL = 'COM7'  # Portul tău determinat pe Windows
BAUD_RATE = 9600

class RoverAlarmGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Sistem de Alarmă și Control Caroserie")
        self.root.geometry("1050x650")
        self.root.configure(bg="#2c3e50")
        
        # --- Configurare Responsivitate (Grid Weights) ---
        # Coloana 0 (Video) ia 60% din spațiu, Coloana 1 (Controale) ia 40%
        self.root.grid_columnconfigure(0, weight=6) 
        self.root.grid_columnconfigure(1, weight=4) 
        self.root.grid_rowconfigure(0, weight=1)
        
        # Conexiune Serială
        self.serial_conectat = False
        self.esp32 = None
        self.init_serial()

        # Configurare MediaPipe (Detecție Mână)
        self.mp_hands = mp.solutions.hands
        self.hands = self.mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)
        self.mp_draw = mp.solutions.drawing_utils
        self.tip_ids = [8, 12, 16, 20] # Index, Mijloc, Inelar, Mic
        self.ultima_comanda = None

        # Configurare Cameră Web
        self.cap = cv2.VideoCapture(0)

        # Construire Interfață Grafică
        self.setup_ui()
        
        # Pornire loop procesare video
        self.update_frame()
        
        # Eveniment la închiderea ferestrei
        self.root.protocol("WM_DELETE_WINDOW", self.inchide_aplicatie)

    def init_serial(self):
        try:
            self.esp32 = serial.Serial(PORT_SERIAL, BAUD_RATE, timeout=1)
            # Dezactivare DTR/RTS pentru a preveni blocarea ESP32 pe Windows
            self.esp32.setDTR(False)
            self.esp32.setRTS(False)
            time.sleep(2)
            self.serial_conectat = True
            print(f"Conexiune serială reușită pe portul {PORT_SERIAL}")
        except Exception as e:
            self.serial_conectat = False
            print(f"Eroare la conectarea pe {PORT_SERIAL}: {e}")

    def setup_ui(self):
        # Configurare Fonturi
        font_titlu = font.Font(family="Helvetica", size=15, weight="bold")
        font_status = font.Font(family="Helvetica", size=12)
        font_mare = font.Font(family="Helvetica", size=18, weight="bold")
        font_butoane = font.Font(family="Helvetica", size=11, weight="bold")

        # --- Panoul Stânga (Flux Video) ---
        frame_stanga = tk.Frame(self.root, bg="#34495e")
        frame_stanga.grid(row=0, column=0, sticky="nsew", padx=10, pady=10)
        frame_stanga.grid_rowconfigure(0, weight=1)
        frame_stanga.grid_columnconfigure(0, weight=1)
        
        self.video_label = tk.Label(frame_stanga, bg="black")
        self.video_label.grid(row=0, column=0)
        
        # --- Panoul Dreapta (Panou Control și Monitorizare) ---
        frame_dreapta = tk.Frame(self.root, bg="#2c3e50")
        frame_dreapta.grid(row=0, column=1, sticky="nsew", padx=10, pady=10)
        
        # Indicator Status Conexiune
        status_text = f"ESP32: CONECTAT ({PORT_SERIAL})" if self.serial_conectat else "ESP32: NECONECTAT"
        status_color = "#27ae60" if self.serial_conectat else "#e74c3c"
        self.lbl_status = tk.Label(frame_dreapta, text=status_text, fg=status_color, bg="#2c3e50", font=font_status)
        self.lbl_status.pack(pady=(10, 20))
        
        # Monitorizare Comandă Trimisă
        tk.Label(frame_dreapta, text="STARE SISTEM / COMANDĂ:", fg="white", bg="#2c3e50", font=font_titlu).pack()
        self.lbl_comanda = tk.Label(frame_dreapta, text="Sistem Armat...", width=16, height=2, 
                                    bg="#34495e", fg="#f1c40f", font=font_mare, relief="groove")
        self.lbl_comanda.pack(pady=10, fill=tk.X, padx=20)

        # Linie Separatoare
        tk.Frame(frame_dreapta, height=2, bg="#7f8c8d").pack(fill=tk.X, pady=15, padx=20)

        # --- Panoul de Butoane Manuale Override (Sistem Grid Responsiv) ---
        tk.Label(frame_dreapta, text="Panou Control Manual:", fg="white", bg="#2c3e50", font=font_status).pack(pady=5)
        
        btn_frame = tk.Frame(frame_dreapta, bg="#2c3e50")
        btn_frame.pack(fill=tk.BOTH, expand=True, padx=20)
        
        # Distribuire egală a spațiului pentru butoane
        for i in range(2): 
            btn_frame.grid_columnconfigure(i, weight=1)
        for i in range(3): 
            btn_frame.grid_rowconfigure(i, weight=1)
            
        # Rândul 1: Oprire Alarme și Reset Lumini
        tk.Button(btn_frame, text="OPRIRE ALARMĂ (O)", font=font_butoane, bg="#2ecc71", fg="white", 
                  command=lambda: self.trimite_comanda('O')).grid(row=0, column=0, sticky="nsew", padx=5, pady=5)
        tk.Button(btn_frame, text="STOP TOTAL (S)", font=font_butoane, bg="#95a5a6", fg="black", 
                  command=lambda: self.trimite_comanda('S')).grid(row=0, column=1, sticky="nsew", padx=5, pady=5)
        
        # Rândul 2: Semnalizări Direcție
        tk.Button(btn_frame, text="SEMNAL STG (L)", font=font_butoane, bg="#e67e22", fg="white", 
                  command=lambda: self.trimite_comanda('L')).grid(row=1, column=0, sticky="nsew", padx=5, pady=5)
        tk.Button(btn_frame, text="SEMNAL DR (R)", font=font_butoane, bg="#e67e22", fg="white", 
                  command=lambda: self.trimite_comanda('R')).grid(row=1, column=1, sticky="nsew", padx=5, pady=5)
        
        # Rândul 3: Elemente Luminoase Extra
        tk.Button(btn_frame, text="AVARII (Z)", font=font_butoane, bg="#e74c3c", fg="white", 
                  command=lambda: self.trimite_comanda('Z')).grid(row=2, column=0, sticky="nsew", padx=5, pady=5)
        tk.Button(btn_frame, text="FARURI (H)", font=font_butoane, bg="#f1c40f", fg="black", 
                  command=lambda: self.trimite_comanda('H')).grid(row=2, column=1, sticky="nsew", padx=5, pady=5)

        # Buton Oprire Aplicație (Poziționat fix jos)
        btn_stop = tk.Button(frame_dreapta, text="🛑 INCHIDE INTERFAȚA", bg="#c0392b", fg="white", 
                             font=font_titlu, command=self.inchide_aplicatie)
        btn_stop.pack(side=tk.BOTTOM, fill=tk.X, pady=20, padx=20)

    def trimite_comanda(self, comanda):
        if self.serial_conectat:
            self.esp32.write(comanda.encode('utf-8'))
        
        dict_comenzi = {
            'O': "ALARMĂ OPRITĂ", 'S': "STOP / LUMIN STINSE", 'H': "FARURI SCHIMBATE", 
            'Z': "AVARII ACTIVE", 'L': "SEMNAL STÂNGA", 'R': "SEMNAL DREAPTA"
        }
        
        self.lbl_comanda.config(text=dict_comenzi.get(comanda, f"Cod: {comanda}"))
        print(f"Comandă expediată: {comanda}")
        self.ultima_comanda = comanda

    def update_frame(self):
        ret, frame = self.cap.read()
        if ret:
            frame = cv2.flip(frame, 1)
            img_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            rezultat = self.hands.process(img_rgb)
            
            comanda_curenta = None
            
            if rezultat.multi_hand_landmarks:
                for hand_landmarks in rezultat.multi_hand_landmarks:
                    self.mp_draw.draw_landmarks(frame, hand_landmarks, self.mp_hands.HAND_CONNECTIONS)
                    
                    degete_ridicate = []
                    for id in self.tip_ids:
                        if hand_landmarks.landmark[id].y < hand_landmarks.landmark[id - 2].y:
                            degete_ridicate.append(1)
                        else:
                            degete_ridicate.append(0)
                            
                    nr_degete = degete_ridicate.count(1)
                    
                    # --- Algoritm de Detecție Gesturi ---
                    
                    # 1. PUMN ÎNCHIS (Toate degetele strânse) -> STOP TOTAL / RESET (S)
                    if nr_degete == 0:
                        comanda_curenta = 'S'
                    
                    # 2. INDEX + MIC RIDICATE ([1, 0, 0, 1]) -> DEZARMARE / OPRIRE ALARMĂ (O)
                    elif degete_ridicate == [1, 0, 0, 1]:
                        comanda_curenta = 'O'
                    
                    # 3. DOAR INDEX RIDICAT ([1, 0, 0, 0]) -> SEMNAL STÂNGA (L)
                    elif degete_ridicate == [1, 0, 0, 0]:
                        comanda_curenta = 'L'
                        
                    # 4. INDEX + MIJLOCIU RIDICATE ([1, 1, 0, 0]) -> SEMNAL DREAPTA (R)
                    elif degete_ridicate == [1, 1, 0, 0]:
                        comanda_curenta = 'R'
                        
                    # 5. TREI DEGETE RIDICATE -> AVARII MANUALE (Z)
                    elif nr_degete == 3:
                        comanda_curenta = 'Z'
                        
                    # 6. PATRU DEGETE RIDICATE -> FARURI (H)
                    elif nr_degete == 4:
                        comanda_curenta = 'H'
            
            # Trimite comanda doar dacă s-a schimbat starea față de cadru anterior
            if comanda_curenta and comanda_curenta != self.ultima_comanda:
                self.trimite_comanda(comanda_curenta)

            # Redimensionare dinamică a imaginii video în funcție de mărimea ferestrei ferestrei
            inaltime_frame = self.video_label.winfo_height()
            if inaltime_frame > 50:
                # Menținem raportul de aspect standard 4:3 al camerei web
                latime_noua = int(inaltime_frame * (4/3))
                frame = cv2.resize(frame, (latime_noua, inaltime_frame))

            img_rgb_tk = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            img_pil = Image.fromarray(img_rgb_tk)
            img_tk = ImageTk.PhotoImage(image=img_pil)
            
            self.video_label.imgtk = img_tk
            self.video_label.configure(image=img_tk)

        # Rulează din nou funcția după 15 milisecunde (~60 FPS)
        self.root.after(15, self.update_frame)

    def inchide_aplicatie(self):
        print("Oprire sistem interfață...")
        if self.serial_conectat:
            self.esp32.close()
        self.cap.release()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    root.minsize(800, 500) # Dimensiunea minimă sub care nu se poate micșora
    app = RoverAlarmGUI(root)
    root.mainloop()

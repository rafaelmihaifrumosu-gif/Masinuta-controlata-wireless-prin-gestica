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

        self.btn_conectare = tk.Button(frame_conectare, text="Conectare", bg="#27ae60", fg="white", command=self.porneste_conectare)
        self.btn_conectare.pack(side=tk.LEFT)

        tk.Frame(frame_dreapta, height=2, bg="#7f8c8d").pack(fill=tk.X, pady=10, padx=20)

        tk.Label(frame_dreapta, text="STARE / MOD ACTIV:", fg="white", bg="#2c3e50", font=font_titlu).pack()
        self.lbl_comanda = tk.Label(frame_dreapta, text="SISTEM IN STANDBY", width=18, height=2, 
                                    bg="#34495e", fg="#f1c40f", font=font_mare, relief="groove")
        self.lbl_comanda.pack(pady=5, fill=tk.X, padx=20)

        tk.Frame(frame_dreapta, height=2, bg="#7f8c8d").pack(fill=tk.X, pady=10, padx=20)

        tk.Label(frame_dreapta, text="Sisteme de Parcare Automata:", fg="white", bg="#2c3e50", font=font_status).pack(pady=2)
        park_frame = tk.Frame(frame_dreapta, bg="#2c3e50")
        park_frame.pack(fill=tk.X, padx=20)
        park_frame.grid_columnconfigure(0, weight=1)
        park_frame.grid_columnconfigure(1, weight=1)

        tk.Button(park_frame, text="🅿️ PARCARE FATA (1)", font=font_butoane, bg="#2980b9", fg="white", command=lambda: self.trimite_comanda('1')).grid(row=0, column=0, sticky="nsew", padx=3, pady=3)
        tk.Button(park_frame, text="🅿️ PARCARE SPATE (2)", font=font_butoane, bg="#2980b9", fg="white", command=lambda: self.trimite_comanda('2')).grid(row=0, column=1, sticky="nsew", padx=3, pady=3)
        tk.Button(park_frame, text="🅿️ LATERAL STANGA (3)", font=font_butoane, bg="#27ae60", fg="white", command=lambda: self.trimite_comanda('3')).grid(row=1, column=0, sticky="nsew", padx=3, pady=3)
        tk.Button(park_frame, text="🅿️ LATERAL DREAPTA (4)", font=font_butoane, bg="#27ae60", fg="white", command=lambda: self.trimite_comanda('4')).grid(row=1, column=1, sticky="nsew", padx=3, pady=3)
        tk.Button(park_frame, text="🛑 OPRIRE URGENTA (S)", font=font_butoane, bg="#c0392b", fg="white", command=lambda: self.trimite_comanda('S')).grid(row=2, column=0, columnspan=2, sticky="nsew", padx=3, pady=3)

        tk.Label(frame_dreapta, text="Comenzi Actuatori / Lumini:", fg="white", bg="#2c3e50", font=font_status).pack(pady=5)
        body_frame = tk.Frame(frame_dreapta, bg="#2c3e50")
        body_frame.pack(fill=tk.X, padx=20)
        body_frame.grid_columnconfigure(0, weight=1)
        body_frame.grid_columnconfigure(1, weight=1)

        # --- Randul 0: Usile ---
        tk.Button(body_frame, text="Usa Stanga (U)", font=font_butoane, bg="#d35400", fg="white", command=lambda: self.trimite_comanda('U')).grid(row=0, column=0, sticky="nsew", padx=3, pady=3)
        tk.Button(body_frame, text="Usa Dreapta (I)", font=font_butoane, bg="#d35400", fg="white", command=lambda: self.trimite_comanda('I')).grid(row=0, column=1, sticky="nsew", padx=3, pady=3)
        
        # --- Randul 1: Semnalizarile ---
        tk.Button(body_frame, text="Semnalizare Stg (L)", font=font_butoane, bg="#e67e22", fg="white", command=lambda: self.trimite_comanda('L')).grid(row=1, column=0, sticky="nsew", padx=3, pady=3)
        tk.Button(body_frame, text="Semnalizare Dr (R)", font=font_butoane, bg="#e67e22", fg="white", command=lambda: self.trimite_comanda('R')).grid(row=1, column=1, sticky="nsew", padx=3, pady=3)

        # --- Randul 2: Faruri si Avarii ---
        tk.Button(body_frame, text="Ciclu Faruri (H)", font=font_butoane, bg="#f1c40f", fg="black", command=lambda: self.trimite_comanda('H')).grid(row=2, column=0, sticky="nsew", padx=3, pady=3)
        tk.Button(body_frame, text="⚠️ Avarii (A)", font=font_butoane, bg="#c0392b", fg="white", command=lambda: self.trimite_comanda('A')).grid(row=2, column=1, sticky="nsew", padx=3, pady=3)

        # --- Randul 3: Claxon si Securitate ---
        tk.Button(body_frame, text="🔍 Gasire Masina (C)", font=font_butoane, bg="#8e44ad", fg="white", command=lambda: self.trimite_comanda('C')).grid(row=3, column=0, sticky="nsew", padx=3, pady=3)
        tk.Button(body_frame, text="🛡️ Mod Securitate (X)", font=font_butoane, bg="#2c3e50", fg="white", command=lambda: self.trimite_comanda('X')).grid(row=3, column=1, sticky="nsew", padx=3, pady=3)

        btn_stop = tk.Button(frame_dreapta, text="INCHIDE INTERFATA", bg="#7f8c8d", fg="white", font=font_titlu, command=self.inchide_aplicatie)
        btn_stop.pack(side=tk.BOTTOM, fill=tk.X, pady=10, padx=20)

    def porneste_scanare(self):
        self.lbl_status.config(text="SCANEZ DISPOZITIVE BLE...", fg="#3498db")
        self.root.update()
        asyncio.run_coroutine_threadsafe(self.scaneaza_ble_async(), self.loop_ble)

    async def scaneaza_ble_async(self):
        dispozitive = await BleakScanner.discover()
        lista_afisare = []
        for d in dispozitive:
            nume = d.name if d.name else "Unknown"
            lista_afisare.append(f"{d.address} - {nume}")
            
        self.root.after(0, self.update_combo_dispozitive, lista_afisare)

    def update_combo_dispozitive(self, lista):
        self.combo_dispozitive['values'] = lista
        if lista: 
            self.combo_dispozitive.current(0)
            self.lbl_status.config(text="SCANARE COMPLETA", fg="#27ae60")
        else: 
            self.combo_dispozitive.set("Niciun dispozitiv gasit")
            self.lbl_status.config(text="NIMIC GASIT", fg="#e74c3c")

    def porneste_conectare(self):
        selectie = self.combo_dispozitive.get()
        if not selectie or selectie == "Niciun dispozitiv gasit": return
        adresa_mac = selectie.split(" ")[0]
        
        self.lbl_status.config(text="SE CONECTEAZA...", fg="#f39c12")
        self.root.update()
        asyncio.run_coroutine_threadsafe(self.conectare_ble_async(adresa_mac), self.loop_ble)

    async def conectare_ble_async(self, adresa):
        try:
            if self.client_ble and self.client_ble.is_connected:
                await self.client_ble.disconnect()
                
            self.client_ble = BleakClient(adresa)
            await self.client_ble.connect()
            self.bt_conectat = True
            self.root.after(0, lambda: self.lbl_status.config(text=f"CONECTAT BLE", fg="#27ae60"))
        except Exception as e:
            print(f"Eroare conectare: {e}")
            self.bt_conectat = False
            self.root.after(0, lambda: self.lbl_status.config(text="EROARE CONEXIUNE", fg="#e74c3c"))

    def trimite_comanda(self, comanda):
        dict_comenzi = {
            'U': "TOGGLE USA STANGA", 'I': "TOGGLE USA DREAPTA", 'H': "SCHIMBARE MOD FARURI",
            'C': "CAUTARE MASINA (BEEP)", '1': "PARCARE AUTOMATA FATA", '2': "PARCARE AUTOMATA SPATE",
            '3': "PARCARE LATERAL STANGA", '4': "PARCARE LATERAL DREAPTA", 'S': "STOP DE URGENTA",
            'L': "SEMNALIZARE STANGA", 'R': "SEMNALIZARE DREAPTA", 'A': "AVARII",
            'X': "MOD SECURITATE (ARMATA)"
        }
        self.lbl_comanda.config(text=dict_comenzi.get(comanda, f"Cod: {comanda}"))
        self.ultima_comanda = comanda

        if self.bt_conectat and self.client_ble and self.client_ble.is_connected:
            asyncio.run_coroutine_threadsafe(self.scrie_ble_async(comanda), self.loop_ble)

    async def scrie_ble_async(self, comanda):
        try:
            await self.client_ble.write_gatt_char(CHAR_UUID, comanda.encode('utf-8'), response=True)
        except Exception as e:
            print(f"Eroare BLE: {e}")
            self.bt_conectat = False
            self.root.after(0, lambda: self.lbl_status.config(text="CONEXIUNE PIERDUTA", fg="#e74c3c"))

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
                    if hand_landmarks.landmark[self.tip_ids[0]].x < hand_landmarks.landmark[self.tip_ids[0] - 1].x:
                        degete_ridicate.append(1)
                    else: degete_ridicate.append(0)
                        
                    for id in range(1, 5):
                        if hand_landmarks.landmark[self.tip_ids[id]].y < hand_landmarks.landmark[self.tip_ids[id] - 2].y:
                            degete_ridicate.append(1)
                        else: degete_ridicate.append(0)
                            
                    nr_degete = degete_ridicate.count(1)
                    
                    # LOGICA GESTURILOR CORECTATA: 
                    # Gesturile specifice (liste exacte) se verifica primele, inainte de cantitatea simpla (nr_degete)
                    if nr_degete == 0: comanda_curenta = 'S'      
                    elif degete_ridicate == [0, 1, 0, 0, 0]: comanda_curenta = 'U' # Index -> Usa Stanga
                    elif degete_ridicate == [0, 1, 1, 0, 0]: comanda_curenta = 'I' # Index + Mijloc -> Usa Dreapta
                    elif degete_ridicate == [1, 1, 0, 0, 1]: comanda_curenta = 'X' # Deget Mare + Index + Mic -> Alarma
                    elif nr_degete == 3: comanda_curenta = 'H'    
                    elif nr_degete == 4: comanda_curenta = 'C'    
                    elif nr_degete == 5: pass 
            
            if comanda_curenta and comanda_curenta != self.ultima_comanda:
                self.trimite_comanda(comanda_curenta)

            inaltime_frame = self.video_label.winfo_height()
            if inaltime_frame > 50:
                latime_noua = int(inaltime_frame * (4/3))
                frame = cv2.resize(frame, (latime_noua, inaltime_frame))

            img_rgb_tk = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            img_pil = Image.fromarray(img_rgb_tk)
            img_tk = ImageTk.PhotoImage(image=img_pil)
            self.video_label.imgtk = img_tk
            self.video_label.configure(image=img_tk)

        self.root.after(15, self.update_frame)

    def inchide_aplicatie(self):
        if self.bt_conectat and self.client_ble:
            asyncio.run_coroutine_threadsafe(self.client_ble.disconnect(), self.loop_ble)
        self.cap.release()
        self.loop_ble.call_soon_threadsafe(self.loop_ble.stop)
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    root.minsize(800, 500)
    app = RoverControlGUI(root)
    root.mainloop()

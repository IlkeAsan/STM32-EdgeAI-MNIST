import cv2
import serial
import numpy as np
import os

# =========================================================================
# 1. AYARLAR (SETTINGS)
# =========================================================================
COM_PORT = 'COM12' 
BAUD_RATE = 115200
CAMERA_ID = 0 


def print_ascii_art(img_28x28):
    """
    28x28 matrisi terminale guzelce basar.
    """
    print("\n--- STM32'YE GIDEN 28x28 MATRIS GORUNUMU ---")
    print("+" + "-" * 28 + "+")
    for row in img_28x28:
        line = "|"
        for pixel in row:
            if pixel > 150:
                line += "#"  # Net cizgiler
            elif pixel > 50:
                line += "."  # Kenar gecisleri
            else:
                line += " "  # Tamamen temiz siyah arkaplan
        line += "|"
        print(line)
    print("+" + "-" * 28 + "+\n")


def preprocess_mnist(roi):
    """
    Web kamerasindan gelen goruntuyu MNIST standartlarina cevirir.
    Tum kopuk cizgileri birlestirir ve silik cizgileri kurtarir.
    """
    # 1. Gri tona cevir
    gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)

    # 2. Gurultuyu azaltmak icin yumusat
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)

    # 3. ADAPTIF ESIKLEME: Hem koyu hem acik/silik kursunkalem cizgilerini mukemmel yakalar
    thresh = cv2.adaptiveThreshold(
        blurred, 255, 
        cv2.ADAPTIVE_THRESH_GAUSSIAN_C, 
        cv2.THRESH_BINARY_INV, 
        21, 10
    )

    # 4. Kenar kalintilarini 4 piksel iceriden temizle
    thresh[:4, :] = 0
    thresh[-4:, :] = 0
    thresh[:, :4] = 0
    thresh[:, -4:] = 0

    # 5. MORFOLOJIK BIRLESTIRME: Kopuk cizgileri (2'nin ust kavisini) baglar ve kalinlastirir
    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (3, 3))
    thresh = cv2.morphologyEx(thresh, cv2.MORPH_CLOSE, kernel, iterations=1)
    thresh = cv2.dilate(thresh, kernel, iterations=1)

    # 6. TUM CIZGILERI BIRLESTIR (Sadece tek bir parcayi degil, 2'nin hem ustunu hem altini al)
    contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    valid_boxes = []
    for c in contours:
        # Cok minik gurultuleri ele
        if cv2.contourArea(c) > 15:
            x, y, w, h = cv2.boundingRect(c)
            valid_boxes.append((x, y, x + w, y + h))

    if valid_boxes:
        # Tum cizgileri icine alan EN BUYUK ortak dikdortgeni hesapla
        min_x = min(b[0] for b in valid_boxes)
        min_y = min(b[1] for b in valid_boxes)
        max_x = max(b[2] for b in valid_boxes)
        max_y = max(b[3] for b in valid_boxes)

        w = max_x - min_x
        h = max_y - min_y

        if w > 8 and h > 8:
            digit = thresh[min_y:max_y, min_x:max_x]

            # 20x20 kutuya orantili sigdir
            if w > h:
                new_w = 20
                new_h = int(h * (20.0 / w))
            else:
                new_h = 20
                new_w = int(w * (20.0 / h))

            new_w = max(1, min(20, new_w))
            new_h = max(1, min(20, new_h))

            digit_resized = cv2.resize(digit, (new_w, new_h), interpolation=cv2.INTER_AREA)

            # 28x28 bos siyah tuval olustur
            canvas = np.zeros((28, 28), dtype=np.uint8)

            # Rakami tam merkeze oturt
            x_offset = (28 - new_w) // 2
            y_offset = (28 - new_h) // 2
            canvas[y_offset:y_offset+new_h, x_offset:x_offset+new_w] = digit_resized

            return canvas

    return cv2.resize(thresh, (28, 28), interpolation=cv2.INTER_AREA)


def main():
    print(f"STM32 ile {COM_PORT} uzerinden baglanti kuruluyor...")
    try:
        stm32_serial = serial.Serial(COM_PORT, BAUD_RATE, timeout=10)
        print("Baglanti basarili! Kamera aciliyor...")
    except Exception as e:
        print(f"HATA: {COM_PORT} portu acilamadi! Port numarasini kontrol et.")
        print(e)
        return

    cap = cv2.VideoCapture(CAMERA_ID)
    
    if not cap.isOpened():
        print("HATA: Kamera acilamadi!")
        return

    print("\n--- KULLANIM TALIMATI ---")
    print(" 1. Kagida cizdigin rakami ekrandaki YESIL KARENIN icine ortala.")
    print(" 2. 's' tusuna basarak fotografi STM32'ye gonder.")
    print(" 3. 'q' tusuna basarak programdan cik.\n")

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        clean_frame = frame.copy()
        
        # 200x200 Odak Karesi Koordinatlari
        height, width, _ = frame.shape
        top_left = (width // 2 - 100, height // 2 - 100)
        bottom_right = (width // 2 + 100, height // 2 + 100)
        
        cv2.rectangle(frame, top_left, bottom_right, (0, 255, 0), 2)
        cv2.putText(frame, "Rakami Bu Kareye Ortalayin ve 'S' ye basin", (10, 30), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

        cv2.imshow("Kamera", frame)

        key = cv2.waitKey(1) & 0xFF

        # -----------------------------------------------------------------
        # 's' TUSUNA BASILDIGINDA
        # -----------------------------------------------------------------
        if key == ord('s'):
            roi = clean_frame[top_left[1]+2:bottom_right[1]-2, top_left[0]+2:bottom_right[0]-2]
            
            # GELISMIS MNIST ON ISLEME
            processed_28x28 = preprocess_mnist(roi)

            # Dosyaya kaydet
            save_path = "captured_digit_28x28.png"
            cv2.imwrite(save_path, processed_28x28)
            print(f"[DOSYA] Giden temiz resim '{save_path}' adiyla kaydedildi.")

            # Terminale ASCII Matris Ciz
            print_ascii_art(processed_28x28)

            # Inceleme Penceresi (Orijinal vs Temizlenmis 28x28)
            preview_orig = cv2.resize(roi, (200, 200))
            preview_clean = cv2.resize(cv2.cvtColor(processed_28x28, cv2.COLOR_GRAY2BGR), (200, 200), interpolation=cv2.INTER_NEAREST)
            combined_preview = np.hstack((preview_orig, preview_clean))
            cv2.imshow("Inceleme: Orijinal Kamera | STM32'ye Giden Temiz (28x28)", combined_preview)

            # 784 Bayta Duzlestir
            flattened_image = processed_28x28.flatten()

            print("[BILGI] 784 bayt STM32'ye gonderiliyor...")
            stm32_serial.reset_input_buffer()
            stm32_serial.reset_output_buffer()
            stm32_serial.write(bytearray(flattened_image))

            print("[BILGI] STM32 Yapay Zekasi hesapliyor...")
            response = stm32_serial.read(1)
            
            if response:
                tahmin = response.decode('utf-8')
                print("=" * 45)
                print(f"   >>> STM32'NIN TAHMINI: {tahmin} <<<")
                print("=" * 45)
            else:
                print("[HATA] STM32'den zamaninda yanit alinamadi!")

        elif key == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()
    stm32_serial.close()
    print("Program kapatildi.")


if __name__ == "__main__":
    main()

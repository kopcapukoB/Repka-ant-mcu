import serial
from PIL import Image
import time
import os

# Настройки
PORT = 'COM9'
BAUDRATE = 115200

# Определяем путь к скрипту и изображению
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PICS_DIR = os.path.join(SCRIPT_DIR, 'pics')
IMAGE_PATH = os.path.join(PICS_DIR, 'get1.png')
DISPLAY_WIDTH = 320
DISPLAY_HEIGHT = 240

def rgb888_to_hex(r, g, b):
    return (r << 16) | (g << 8) | b

def main():
    print("Image to Display Transfer")
    print("=" * 50)
    
    # Проверяем наличие папки и файла
    print(f"Script directory: {SCRIPT_DIR}")
    print(f"Looking for image: {IMAGE_PATH}")
    
    if not os.path.exists(IMAGE_PATH):
        print(f"\nERROR: Image file not found!")
        print(f"Path: {IMAGE_PATH}")
        print("\nAvailable files in pics folder:")
        if os.path.exists(PICS_DIR):
            files = os.listdir(PICS_DIR)
            for f in files:
                print(f"  - {f}")
        else:
            print("  pics folder not found!")
        return
    
    # Открываем изображение
    try:
        image = Image.open(IMAGE_PATH)
        width, height = image.size
        print(f"Image loaded: {width}x{height}")
    except Exception as e:
        print(f"Error loading image: {e}")
        return
    
    # Изменяем размер
    if width > DISPLAY_WIDTH or height > DISPLAY_HEIGHT:
        print(f"Resizing image to fit display ({DISPLAY_WIDTH}x{DISPLAY_HEIGHT})")
        image.thumbnail((DISPLAY_WIDTH, DISPLAY_HEIGHT), Image.LANCZOS)
        width, height = image.size
    
    if image.mode != 'RGB':
        image = image.convert('RGB')
    
    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=1)
        print(f"Connected to {PORT}")
        time.sleep(2)
        
        ser.write(b'disp_screen\n')
        time.sleep(0.5)
        
        x_offset = (DISPLAY_WIDTH - width) // 2
        y_offset = (DISPLAY_HEIGHT - height) // 2
        
        print(f"Sending {width}x{height} image...")
        start_time = time.time()
        
        for y in range(height):
            for x in range(width):
                r, g, b = image.getpixel((x, y))
                color_hex = rgb888_to_hex(r, g, b)
                cmd = f"disp_px {x + x_offset} {y + y_offset} {color_hex:06X}\n"
                ser.write(cmd.encode())
            
            if y % 10 == 0:
                progress = (y / height) * 100
                print(f"Progress: {progress:.1f}%")
        
        ser.flush()
        time.sleep(0.5)
        
        elapsed = time.time() - start_time
        print(f"\nDone! Time: {elapsed:.1f}s")
        
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            time.sleep(0.1)
            ser.close()
            print("Port closed")

if __name__ == "__main__":
    main()
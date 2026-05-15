import serial
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime
import time
import re

# Настройки
PORT = 'COM9'
BAUDRATE = 115200
DURATION = 30  # секунд сбора данных
INTERVAL = 0.2  # интервал между измерениями

def read_response(ser, timeout=1):
    """Чтение всех доступных данных из порта"""
    response = ""
    start_time = time.time()
    
    while time.time() - start_time < timeout:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8', errors='ignore')
            response += line
            if '\n' in line:
                break
        else:
            time.sleep(0.01)
    
    return response.strip()

def main():
    print("BME280 Breath Test Monitor")
    print("=" * 60)
    
    # Подключение к COM порту
    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=1)
        print(f"Connected to {PORT}")
        
        # Очищаем буферы
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        
        # Ждем инициализацию и читаем приветственное сообщение
        print("Waiting for device initialization...")
        time.sleep(3)
        
        # Читаем все, что есть в буфере (приветственные сообщения)
        while ser.in_waiting:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                print(f"Device: {line}")
        
        print("Device ready\n")
        
    except Exception as e:
        print(f"Failed to connect: {e}")
        return
    
    # Тестовая команда
    print("Sending test command...")
    ser.write(b'version\n')
    ser.flush()
    time.sleep(0.5)
    
    while ser.in_waiting:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if line:
            print(f"Response: {line}")
    
    print("\n" + "=" * 60)
    
    # Массивы для данных
    timestamps = []
    temperatures = []
    pressures = []
    humidities = []
    
    print(f"\nCollecting data for {DURATION} seconds...")
    print("Breathe on the sensor to see reaction!\n")
    
    start_time = time.time()
    measurement_num = 0
    
    try:
        while time.time() - start_time < DURATION:
            measurement_num += 1
            current_time = time.time() - start_time
            
            # Читаем температуру
            ser.write(b'temp\n')
            ser.flush()
            time.sleep(0.1)
            temp_response = ""
            while ser.in_waiting:
                temp_response += ser.readline().decode('utf-8', errors='ignore')
            
            # Читаем давление
            ser.write(b'pres\n')
            ser.flush()
            time.sleep(0.1)
            pres_response = ""
            while ser.in_waiting:
                pres_response += ser.readline().decode('utf-8', errors='ignore')
            
            # Читаем влажность
            ser.write(b'hum\n')
            ser.flush()
            time.sleep(0.1)
            hum_response = ""
            while ser.in_waiting:
                hum_response += ser.readline().decode('utf-8', errors='ignore')
            
            # Отладка - показываем сырые ответы
            if measurement_num <= 3:  # Показываем первые 3 измерения для отладки
                print(f"\n--- Debug measurement #{measurement_num} ---")
                print(f"Temp response: {repr(temp_response)}")
                print(f"Pres response: {repr(pres_response)}")
                print(f"Hum response: {repr(hum_response)}")
            
            # Парсим значения
            temp_val = None
            pres_val = None
            hum_val = None
            
            # Ищем числа в ответах
            temp_match = re.search(r'([\d.]+)', temp_response)
            if temp_match:
                temp_val = float(temp_match.group(1))
            
            pres_match = re.search(r'([\d.]+)', pres_response)
            if pres_match:
                pres_val = float(pres_match.group(1))
            
            hum_match = re.search(r'([\d.]+)', hum_response)
            if hum_match:
                hum_val = float(hum_match.group(1))
            
            # Сохраняем если все значения получены
            if temp_val is not None and pres_val is not None and hum_val is not None:
                timestamps.append(current_time)
                temperatures.append(temp_val)
                pressures.append(pres_val)
                humidities.append(hum_val)
                
                print(f"[{measurement_num}] t={current_time:.1f}s | "
                      f"Temp: {temp_val:.2f}C | "
                      f"Pres: {pres_val:.2f}hPa | "
                      f"Hum: {hum_val:.2f}%RH")
            else:
                print(f"[{measurement_num}] Failed - Temp:{temp_val}, Pres:{pres_val}, Hum:{hum_val}")
            
            time.sleep(INTERVAL)
    
    except KeyboardInterrupt:
        print("\n\nStopped by user")
    finally:
        ser.close()
        print("Serial connection closed")
    
    print(f"\nCollected {len(timestamps)} measurements\n")
    
    # Проверяем, есть ли данные
    if len(timestamps) == 0:
        print("No data collected!")
        print("\nTroubleshooting tips:")
        print("1. Check if device is running and sending data")
        print("2. Try using a terminal program (Putty) to verify commands work")
        print("3. Check baud rate matches (115200)")
        print("4. Make sure no other program is using COM9")
        return
    
    # Строим графики
    print("Creating plots...")
    
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(14, 10))
    fig.suptitle('BME280 Sensor - Breath Test Results', fontsize=16, fontweight='bold')
    
    # График температуры
    ax1.plot(timestamps, temperatures, 'r-', linewidth=2, marker='o', markersize=5, label='Temperature')
    ax1.set_ylabel('Temperature (C)', fontsize=12)
    ax1.grid(True, alpha=0.3, linestyle='--')
    ax1.legend(loc='upper right')
    ax1.set_title('Temperature Reaction to Breath', fontsize=13)
    
    # Добавляем статистику
    temp_min = min(temperatures)
    temp_max = max(temperatures)
    temp_diff = temp_max - temp_min
    ax1.text(0.02, 0.95, f'Min: {temp_min:.2f}C\nMax: {temp_max:.2f}C\nDelta: {temp_diff:.2f}C', 
            transform=ax1.transAxes, verticalalignment='top',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8))
    
    # График давления
    ax2.plot(timestamps, pressures, 'b-', linewidth=2, marker='s', markersize=5, label='Pressure')
    ax2.set_ylabel('Pressure (hPa)', fontsize=12)
    ax2.grid(True, alpha=0.3, linestyle='--')
    ax2.legend(loc='upper right')
    ax2.set_title('Pressure During Test', fontsize=13)
    
    # График влажности
    ax3.plot(timestamps, humidities, 'g-', linewidth=2, marker='^', markersize=5, label='Humidity')
    ax3.set_xlabel('Time (seconds)', fontsize=12)
    ax3.set_ylabel('Humidity (%RH)', fontsize=12)
    ax3.grid(True, alpha=0.3, linestyle='--')
    ax3.legend(loc='upper right')
    ax3.set_title('Humidity Spike from Breath', fontsize=13)
    
    # Статистика для влажности
    hum_min = min(humidities)
    hum_max = max(humidities)
    hum_diff = hum_max - hum_min
    ax3.text(0.02, 0.95, f'Min: {hum_min:.2f}%RH\nMax: {hum_max:.2f}%RH\nDelta: {hum_diff:.2f}%RH', 
            transform=ax3.transAxes, verticalalignment='top',
            bbox=dict(boxstyle='round', facecolor='lightgreen', alpha=0.8))
    
    plt.tight_layout()
    
    # Сохраняем график
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    graph_filename = f'bme280_breath_test_{timestamp}.png'
    plt.savefig(graph_filename, dpi=150, bbox_inches='tight')
    print(f"Graph saved: {graph_filename}")
    
    # Сохраняем данные в CSV
    csv_filename = f'bme280_data_{timestamp}.csv'
    with open(csv_filename, 'w') as f:
        f.write("Time(s),Temperature(C),Pressure(hPa),Humidity(%RH)\n")
        for t, temp, pres, hum in zip(timestamps, temperatures, pressures, humidities):
            f.write(f"{t:.2f},{temp:.2f},{pres:.2f},{hum:.2f}\n")
    print(f"Data saved: {csv_filename}")
    
    # Показываем график
    print("\nDisplaying plot... (close window to exit)")
    plt.show()
    
    print("\nTest completed successfully!")

if __name__ == "__main__":
    main()
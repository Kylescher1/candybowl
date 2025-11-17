import serial
import serial.tools.list_ports
import matplotlib.pyplot as plt
import time

BAUD = 230400

# Moving average window size (increase for smoother average)
# 5=fast response, 20=smooth, 50=very smooth
MOVING_AVG_WINDOW = 20

# Set your desired y-axis range:
Y_MIN = -100
Y_MAX = 1000

# Auto-detect Arduino port
def find_arduino_port():
    """Find Arduino Nano port by checking available COM ports"""
    ports = serial.tools.list_ports.comports()
    available_ports = []
    
    print("Available COM ports:")
    for port in ports:
        print(f"  - {port.device}: {port.description}")
        available_ports.append(port.device)
    
    if not available_ports:
        raise Exception("No COM ports found! Make sure your Arduino Nano is connected.")
    
    # Try to find Arduino (common descriptions contain "Arduino" or "USB")
    for port in ports:
        desc = port.description.upper()
        if 'ARDUINO' in desc or 'USB' in desc or 'CH340' in desc or 'FTDI' in desc:
            print(f"\nAuto-detected Arduino on {port.device}")
            return port.device
    
    # If no Arduino-specific port found, use the first available
    print(f"\nNo Arduino-specific port found. Using first available: {available_ports[0]}")
    return available_ports[0]

try:
    PORT = find_arduino_port()
    ser = serial.Serial(PORT, BAUD)
    print(f"Connected to {PORT} at {BAUD} baud")
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
    print("\nTroubleshooting:")
    print("1. Make sure your Arduino Nano is connected via USB")
    print("2. Check Device Manager to see which COM port it's using")
    print("3. Make sure no other program is using the serial port")
    exit(1)
except Exception as e:
    print(f"Error: {e}")
    exit(1)
time.sleep(2)

raw_vals = []
grams_vals = []
python_avg_vals = []  # Python-side moving average

plt.ion()
fig, ax = plt.subplots()

def calculate_moving_average(values, window_size):
    """Calculate moving average of the last window_size values"""
    if len(values) < window_size:
        # Not enough data yet, return average of what we have
        return sum(values) / len(values) if values else 0
    else:
        # Return average of last window_size values
        return sum(values[-window_size:]) / window_size

try:
    while True:
        try:
            line = ser.readline().decode().strip()
            parts = line.split()  # Handles both spaces and tabs

            if len(parts) != 2:
                continue

            raw = int(parts[0])
            grams = float(parts[1])

            raw_vals.append(raw)
            grams_vals.append(grams)
            
            # Calculate Python-side moving average
            python_avg = calculate_moving_average(grams_vals, MOVING_AVG_WINDOW)
            python_avg_vals.append(python_avg)

            # Limit buffer
            if len(raw_vals) > 200:
                raw_vals.pop(0)
                grams_vals.pop(0)
                python_avg_vals.pop(0)

            ax.clear()
            ax.plot(raw_vals, label="raw")
            ax.plot(grams_vals, label="grams")
            ax.plot(python_avg_vals, label=f"avg (window={MOVING_AVG_WINDOW})", linewidth=2)

            ax.set_ylim(Y_MIN, Y_MAX)   # FIXED SCALE
            ax.set_title("Scale Readings")
            ax.set_xlabel("Samples")
            ax.set_ylabel("Value")
            ax.legend()
            plt.pause(0.01)

        except KeyboardInterrupt:
            print("\nExiting...")
            break
        except Exception as e:
            print(f"\nError reading data: {e}")
            break
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("Serial port closed.")

import paho.mqtt.client as mqtt
import json
from datetime import datetime

BROKER = "broker.emqx.io"
TOPIC = "esp32/agri/data"

prev_irrigation = False
prev_manual = False

def log_to_file(filename, row):
    with open(filename, "a") as f:
        f.write(row + "\n")

def on_connect(client, userdata, flags, rc):
    print("✅ Connected to broker with result code " + str(rc))
    client.subscribe(TOPIC)

def on_message(client, userdata, msg):
    global prev_irrigation, prev_manual
    try:
        data = json.loads(msg.payload.decode())
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        # Basic CSV row
        row = ",".join(str(data.get(k, "")) for k in [
            "temperature", "humidity", "soil_moisture",
            "evaporation_ml", "water_poured", "irrigation", "manual_mode"
        ])
        full_row = f"{timestamp},{row}"

        # Log to general file
        log_to_file("mqtt_log.csv", full_row)

        # Log irrigation ON
        if data.get("irrigation") == True and not prev_irrigation:
            print(f"💧 Irrigation started at {timestamp}")
            log_to_file("irrigation_log.csv", full_row)

        # Log manual mode transitions
        if data.get("manual_mode") == True and not prev_manual:
            print(f"⚠️ Manual mode entered at {timestamp}")
            log_to_file("manual_mode_log.csv", f"{timestamp},ENTERED_MANUAL")

        if data.get("manual_mode") == False and prev_manual:
            print(f"✅ Returned to auto mode at {timestamp}")
            log_to_file("manual_mode_log.csv", f"{timestamp},RESUMED_AUTO")

        # Update previous states
        prev_irrigation = data.get("irrigation")
        prev_manual = data.get("manual_mode")

    except Exception as e:
        print("❌ Error parsing or logging message:", e)

client = mqtt.Client("LoggerClient")
client.on_connect = on_connect
client.on_message = on_message

print(f"🔌 Connecting to {BROKER} and listening on topic '{TOPIC}'...")
client.connect(BROKER, 1883, 60)
client.loop_forever()

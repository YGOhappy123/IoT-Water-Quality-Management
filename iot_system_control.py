import os
import time
import joblib
import requests
from typing import Literal
from threading import Timer
from firebase_admin import credentials, db, initialize_app
from dotenv import load_dotenv

FIREBASE_DB_URL = 'https://water-quality-management-ptit-default-rtdb.asia-southeast1.firebasedatabase.app/'
TIME_BETWEEN_PROCESSES = 2
IOT_AI_MODEL = None


def load_ai_model():
    global IOT_AI_MODEL
    IOT_AI_MODEL = joblib.load('iot_ai_model.pkl')


def set_interval(func, interval, *args, **kwargs):
    def wrapper():
        func(*args, **kwargs)
        set_interval(func, interval, *args, **kwargs)

    timer = Timer(interval, wrapper)
    timer.daemon = True
    timer.start()

    return timer


def keep_main_thread_alive():
    while True:
        time.sleep(1)


def get_blynk_url(type: Literal['get', 'update']):
    return f'https://sgp1.blynk.cloud/external/api/{"get" if type == "get" else "batch/update"}?token={os.getenv("BLYNK_AUTH_TOKEN")}'


def control_iot_system():
    try:
        response = requests.get(get_blynk_url("get") + '&V0')

        if not response.ok:
            raise Exception('Cannot fetch data from Blynk IoT.')

        if not response.json():
            print('Automatic control mode is turned off.')
            return

        fb_is_processed_ref = db.reference('IsProcessed')
        fb_sensors_ref = db.reference('Sensors')

        if not fb_is_processed_ref.get():
            sensor_data = fb_sensors_ref.get()

            if not sensor_data:
                raise Exception('Cannot fetch data from Firebase.')

            temperature = sensor_data['temperature']
            water_level = sensor_data['water_level']
            tds_level = sensor_data['tds_level']
            env_humidity = sensor_data['env_humidity']

            if IOT_AI_MODEL is not None:
                predictions = IOT_AI_MODEL.predict([[temperature, water_level, tds_level, env_humidity]]).toarray()[0]

                pump_in_pred = predictions[0]
                pump_out_pred = predictions[1]
                cooler_pred = predictions[2]
                heater_pred = predictions[3]
                tds_plants_pred = predictions[4]
                tds_fishes_pred = predictions[5]
                mist_sprayer_pred = predictions[6]
                ventilation_pred = predictions[7]

                requests.get(
                    get_blynk_url("update")
                    + f'&V1={cooler_pred}&V2={heater_pred}'
                    + f'&V3={tds_plants_pred}&V4={tds_fishes_pred}'
                    + f'&V5={pump_in_pred}&V6={pump_out_pred}'
                    + f'&V7={ventilation_pred}&V8={mist_sprayer_pred}'
                )

            fb_is_processed_ref.set(True)

    except Exception as error_message:
        print(error_message)
        return


def connect_to_firebase():
    try:
        cred = credentials.Certificate('firebase_credentials.json')
        initialize_app(cred, {'databaseURL': FIREBASE_DB_URL})
        print('Connected to Firebase.')
    except Exception:
        print('Connect to Firebase failed.')


if __name__ == '__main__':
    load_dotenv()
    load_ai_model()
    connect_to_firebase()
    set_interval(control_iot_system, TIME_BETWEEN_PROCESSES)
    keep_main_thread_alive()

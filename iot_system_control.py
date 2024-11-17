from threading import Timer
from firebase_admin import credentials, db, initialize_app
import time

FIREBASE_URL = 'https://water-quality-management-ptit-default-rtdb.asia-southeast1.firebasedatabase.app/'


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


def control_iot_system():
    try:
        is_processed = db.reference('IsProcessed').get()

        if not is_processed:
            sensor_data = db.reference('Sensors').get()

    except Exception:

        return


def connect_to_firebase():
    cred = credentials.Certificate('firebase_credentials.json')
    initialize_app(cred, {'databaseURL': FIREBASE_URL})


connect_to_firebase()
set_interval(control_iot_system, 1)
keep_main_thread_alive()

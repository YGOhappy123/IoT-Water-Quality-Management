from threading import Timer
from firebase_admin import credentials, db, initialize_app
import time

FIREBASE_DB_URL = 'https://water-quality-management-ptit-default-rtdb.asia-southeast1.firebasedatabase.app/'
TIME_BETWEEN_PROCESSES = 2


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
        fb_is_automatic_ref = db.reference('IsAutomatic')
        fb_is_processed_ref = db.reference('IsProcessed')
        fb_sensors_ref = db.reference('Sensors')

        if not fb_is_automatic_ref.get():
            print('Automatic control mode is turned off.')
            return

        if not fb_is_processed_ref.get():
            sensor_data = fb_sensors_ref.get()
            print(sensor_data)

            fb_is_processed_ref.set(True)

    except Exception:
        print('Cannot fetch data from Firebase.')
        return


def connect_to_firebase():
    try:
        cred = credentials.Certificate('firebase_credentials.json')
        initialize_app(cred, {'databaseURL': FIREBASE_DB_URL})
        print('Connected to Firebase.')
    except Exception:
        print('Connect to Firebase failed.')


if __name__ == '__main__':
    connect_to_firebase()
    set_interval(control_iot_system, TIME_BETWEEN_PROCESSES)
    keep_main_thread_alive()

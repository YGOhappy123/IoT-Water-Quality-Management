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
        db_is_processed_ref = db.reference('IsProcessed')
        db_sensors_ref = db.reference('Sensors')

        is_data_processed = db_is_processed_ref.get()

        if not is_data_processed:
            sensor_data = db_sensors_ref.get()
            print(sensor_data)

            db_is_processed_ref.set(True)

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

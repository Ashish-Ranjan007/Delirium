import time
import threading
from datetime import datetime
import os

class Timer:
    def __init__(self):
        self._start_time = None
        self._elapsed_time = 0.0
        self._running = False
        self._timer_thread = None
        self._stop_flag = threading.Event()
        self._log_times = []

    def start(self):
        if not self._running:
            self._start_time = time.time()
            self._running = True
            self._stop_flag.clear()
            self._timer_thread = threading.Thread(target=self._run)
            self._timer_thread.daemon = True  # Allow program to exit even if thread is running
            self._timer_thread.start()
            print("Timer started. Press Enter to log the current time (updates every second).")

    def _run(self):
        while not self._stop_flag.is_set():
            current_time = time.time()
            self._elapsed_time = current_time - self._start_time
            self._print_status()
            time.sleep(1)  # Update every second

    def log_time(self):
        if self._running:
            now = datetime.now()
            current_time_str = now.strftime("%Y-%m-%d %H:%M:%S")
            log_entry = f"Logged time: {current_time_str} (Elapsed: {self.format_elapsed_time()})"
            self._log_times.append(log_entry)
            self._print_status() # Update display after logging
        else:
            print("Timer is not running.")

    def stop(self):
        if self._running:
            self._running = False
            self._stop_flag.set()
            if self._timer_thread and self._timer_thread.is_alive():
                self._timer_thread.join()
            self._print_final_status()
        else:
            print("Timer is not running.")

    def format_elapsed_time(self):
        minutes = int(self._elapsed_time // 60)
        seconds = int(self._elapsed_time % 60)
        return f"{minutes:02}:{seconds:02}"

    def _clear_terminal(self):
        os.system('cls' if os.name == 'nt' else 'clear')

    def _print_status(self):
        self._clear_terminal()
        print("Timer running...")
        print(f"Elapsed Time: {self.format_elapsed_time()}")
        if self._log_times:
            print("\nLogged Times:")
            for log in self._log_times:
                print(f"- {log}")
        print("\nPress Enter to log the current time, Ctrl+C to stop.")

    def _print_final_status(self):
        self._clear_terminal()
        print("Timer stopped.")
        print(f"Total Elapsed Time: {self.format_elapsed_time()}")
        if self._log_times:
            print("\nLogged Times:")
            for log in self._log_times:
                print(f"- {log}")

def main():
    timer = Timer()
    timer.start()

    try:
        while True:
            input()  # Wait for Enter key press
            timer.log_time()
    except KeyboardInterrupt:
        timer.stop()

if __name__ == "__main__":
    main()

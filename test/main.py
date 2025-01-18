import ctypes
import os
import sys
import signal

lib = ctypes.CDLL(os.path.abspath("./build/libangelo.dylib"))


class AppHandleOpt(ctypes.Structure):
    _fields_ = [
        ("value", ctypes.c_void_p),
        ("is_some", ctypes.c_bool)
    ]

class WindowHandleOpt(ctypes.Structure):
    _fields_ = [
        ("value", ctypes.c_void_p),
        ("is_some", ctypes.c_bool)
    ]

# Define the create_app function
lib.create_app.argtypes = [ctypes.c_char_p]  # Input: const char*
lib.create_app.restype = AppHandleOpt       # Output: AppHandle_opt

# Define the run_app function
lib.run_app.argtypes = [ctypes.c_void_p]   # Input: AppHandle
lib.run_app.restype = ctypes.c_int          # Output: int

# Define the run_app function
lib.create_window.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_char_p] 
lib.create_window.restype = WindowHandleOpt 

title = b"My Application"  # Note: Strings passed to ctypes must be bytes
app = lib.create_app(title)

window = lib.create_window(800, 600, title)

if app.is_some:
    lib.run_app(app.value)

    
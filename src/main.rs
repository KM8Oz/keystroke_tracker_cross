use std::os::raw::{c_int, c_void};
// use std::ffi::CString;
use std::{process, mem};

type CGEventTapProxy = u32;
type CGEventType = u32;
type CGEventRef = *mut c_void;
type KeyCallback =
    extern "C" fn(CGEventTapProxy, CGEventType, CGEventRef, *mut c_void) -> CGEventRef;
type ReleaseCallback =
    extern "C" fn(CGEventTapProxy, CGEventType, CGEventRef, *mut c_void) -> CGEventRef;
type CreateEventFunc = extern "C" fn(KeyCallback, ReleaseCallback) -> c_int;

extern "C" {
    fn create_event(key_callback: KeyCallback, release_callback: ReleaseCallback) -> c_int;
}
extern "C" fn key_callback(
    _proxy: CGEventTapProxy,
    event_type: CGEventType,
    event: CGEventRef,
    _refcon: *mut c_void,
) -> CGEventRef {
    // Process key events...
    let keydown: u32 = 10;
    let event_type_: String = match event_type == keydown {
        true => "KeyDown".into(),
        false => "KeyUp".into(),
    };
    println!("eventType {}: {:#?}", event_type_, event);
    event
}

extern "C" fn release_callback(
    _proxy: CGEventTapProxy,
    event_type: CGEventType,
    event: CGEventRef,
    _refcon: *mut c_void,
) -> CGEventRef {
    // Process release events...
    println!("{:#?}", event_type);
    event
}

fn main() {
    // Load the dynamic library...

    unsafe {
        let lib_path = "src/libc/libkeystroke_tracker.dylib";
        let lib = match libloading::Library::new(lib_path) {
            Ok(lib) => lib,
            Err(err) => {
                eprintln!("Failed to load library: {}", err);
                process::exit(1);
            }
        };
          // Load the create_event function
        let create_event: libloading::Symbol<CreateEventFunc>;
        create_event = match lib.get::<CreateEventFunc>(b"create_event") {
            Ok(symbol) => mem::transmute(symbol),
            Err(err) => {
                eprintln!("Failed to load symbol: {}", err);
                process::exit(1);
            }
        };
        // Define the callback functions
        let key_callback_func: KeyCallback = key_callback;
        let release_callback_func: ReleaseCallback = release_callback;

        // Call the create_event function with the callback functions
        let result = create_event(key_callback_func, release_callback_func);

        // Check the result
        if result != 0 {
            eprintln!("Failed to create event!");
            process::exit(1);
        }
    }
}

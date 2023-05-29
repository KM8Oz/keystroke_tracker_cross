#include <stdio.h>
#include <CoreGraphics/CoreGraphics.h>
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>

// Callback function type for handling key events
typedef CGEventRef (*KeyCallback)(CGEventTapProxy, CGEventType, CGEventRef, void*);
const size_t down = 10;
// Callback function type for handling release events
typedef CGEventRef (*ReleaseCallback)(CGEventTapProxy, CGEventType, CGEventRef, void*);

// Callback function for handling key events
CGEventRef key_callback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon) {
    if (type == kCGEventKeyDown || type == kCGEventKeyUp) {
        // Get the virtual keycode of the key event
        CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

        // Get the key event type (keydown or keyup)
        const char* eventType = (type == kCGEventKeyDown) ? "KeyDown" : "KeyUp";

        // Print the key event information
        printf("Key %s - Virtual Keycode: %d\n", eventType, keyCode);
    }
    // Return the event
    return event;
}

// Callback function for handling release events
CGEventRef release_callback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon) {
    if (type == kCGEventFlagsChanged) {
        // Get the key event type (flags changed)
        const char* eventType = "Release";

        // Print the key event information
        printf("Key %s - Virtual Keycode: %d\n", eventType, kCGEventFlagMaskSecondaryFn);
    }

    // Return the event
    return event;
}

// Function to create the event tap
int create_event(KeyCallback key_callback, ReleaseCallback release_callback) {
    // Prompt for permission
    printf("This application requires accessibility permission to track keystrokes.\n");
    printf("Please grant permission by going to System Preferences > Security & Privacy > Privacy > Accessibility.\n");
    printf("Press Enter when permission is granted...");
    getchar();

    // Request accessibility permission
    if (AXIsProcessTrusted() != true) {
        printf("Failed to get accessibility permission!\n");
        return 1;
    }

    // Create an event tap to monitor key events
    CGEventMask keyEventMask = (1 << kCGEventKeyDown) | (1 << kCGEventKeyUp);
    CFMachPortRef keyEventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0, keyEventMask, key_callback, NULL);

    if (!keyEventTap) {
        printf("Failed to create key event tap!\n");
        return 1;
    }

    // Create an event tap to monitor release events
    CGEventMask releaseEventMask = (1 << kCGEventFlagsChanged);
    CFMachPortRef releaseEventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0, releaseEventMask, release_callback, NULL);

    if (!releaseEventTap) {
        printf("Failed to create release event tap!\n");
        CFRelease(keyEventTap);
        return 1;
    }

    // Create a run loop source and add the event taps to the current run loop
    CFRunLoopSourceRef keyRunLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, keyEventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), keyRunLoopSource, kCFRunLoopCommonModes);

    CFRunLoopSourceRef releaseRunLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, releaseEventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), releaseRunLoopSource, kCFRunLoopCommonModes);

    // Enable the event taps
    CGEventTapEnable(keyEventTap, true);
    CGEventTapEnable(releaseEventTap, true);

    // Start the run loop to begin monitoring key events
    CFRunLoopRun();

    // Clean up
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), keyRunLoopSource, kCFRunLoopCommonModes);
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), releaseRunLoopSource, kCFRunLoopCommonModes);

    CFRelease(keyRunLoopSource);
    CFRelease(releaseRunLoopSource);
    CFRelease(keyEventTap);
    CFRelease(releaseEventTap);

    return 0;
}

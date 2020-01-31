# Attention

At present, the adaptation to WCID, WinUSB, etc. has all been completed. However, when transmitting data on the endpoint, we received an error message from USBIP. This is most likely a problem with the USBIP project itself.

Due to the completeness of the USBIP protocol document, we have not yet understood its role in the Bulk transmission process, which may also lead to errors in subsequent processes.

We will continue to try to make it work under USB HID. Once the USBIP problem is solved, we will immediately transfer it to work under WinUSB
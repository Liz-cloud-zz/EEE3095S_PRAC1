# EEE3095S_PRAC1
Before connecting to the internet for the first time, you may have noticed that the time on
your Pi is a little strange. This is because the Pi doesn’t have what is called an RTC (real
time clock). Instead, it relies NTPD (Network Time Protocol Daemon) to fetch, set and
store the date and time. However, this might be problematic as you may not always have
an internet connection, and if your Pi doesn’t have the correct localisation options, you may
end up with the wrong time due to timezone settings. It’s possible to add an RTC to the
Raspberry Pi to hold the system time correctly, but for this practical we’re simply going to
interface with the RTC using I2C, and set the time using buttons and interrupts.

Using C write to time to an RTC module. You will also have an LED that will flicker on and off every second, as well as have two buttons to change the time.
The buttons should be connected using interrupts and debouncing, which do the following:
  1. Button 1 - Fetches the hours value from the RTC, increases it by 1, and writes it back
    to the RTC.
  2. Button 2 - Fetches the minutes value from the RTC, increases it by 1, and writes it
    back to the RTC.
You cannot use any time libraries in your script, i.e. make sure you are using the I2C
communication protocol between the RTC and Pi in your script.

# Hardware Required
• Configured Raspberry Pi
• A breadboard
• 1 LED
• 1 Resistor
• 2 x push buttons
• Dupont Wires
• DS3231 RTC Module

# Aquario

Aquario is an open-source aquarium managment system, using Ardunio.

# Hardware requirements

+ Arduino Mega
+ Arduino Ethernet shield
+ DS3231 - Real time clock (RTC) board
+ DS18B20 Dallas temperature sensors
+ Mechanical relay board with 4 relays
+ SD card for the webpages to be stored

# Features

Displays the current time, date, day and tank temperature along with scheduled start and 
stop times for lights and co2. All of these can be updated or set via the locally running 
website.

The lights and co2 schedules can take 2 start and stop times per day of the week, giving 
flexibility.

# Getting the code uploaded to Arduino

Once you've checked out this project, you may find that there are some compilation errors 
when trying to upload to your Arduino. If this happens you will need to point to the various 
library folders in the /lib directory of this project, once you've done this the compilation errors will be sorted.

# Adding the webpages to the SD card

For the locally running website you'll need to ensure your Arduino is connected to the internet 
via the ethernet shield but you'll also need to make sure the html pages from the /webpage folder 
have been copied onto your SD card and the SD card must be put into the SD slot on the Arduino 
ethernet shield.

# Accessing the locally running website

Once you've got the Aquario code uploaded to your Arduino and all the sensors wired up, you 
can view the current state and also update values using the locally running website by opening 
the following URL in your browser - http://192.168.0.177/state.htm

These webpages are only accessible on your local network at present, we may look to make this 
accessible via the internet at some point.
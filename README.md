# GCController
 Arduino code and USB_COMM library for GC capstone project.

BASIC SETUP:
 This library allows the creation of a USB_COMM object that handles all of the computer / Arduino communication necessary for getting and setting temperature on the gas chromatograph. To use this class, first initialize a USB_COMM global object in the Arduino sketch (USB_COMM thermalCont(int serialBaudRate)).

 NOTE: The baud rate used in the USB_COMM class MUST be the same baud rate used for any other serial messaged sent from the Arduino,

 After initializing the USB_COMM object, repeatedly check for new serial messages in the main loop by calling the checkForMsg() method. This will check for incoming serial data and handle the received command accordingly.

%-----------------------------------------------------------%

GETTING AND SETTING TEMPERATURES:
 The USB_COMM object internally stores all of the set (desired temperature for each component) and real (measured temperature from sensors) temperature conditions in the instrument. The USB_COMM object DOES NOT communicate with any hardware; it receives the set temperatures from the computer and stores them internally. These must be collected by the Arduino script and can be accessed using getter functions like getSetTemp#()), where # can be any number from 1 - 3.

 The communication with any temperature sensing hardware must also be done in the Arduino script. The temperature data read from each thermocouple must be shared with the USB_COMM device by calling the setRealTemps(int temp1, int temp2, int temp3) method. If the thermocouple measurements are not loaded into the USB_COMM object, it will NOT be able to share the data with the controlling computer when polled.

%-----------------------------------------------------------%

COMPUTER-SIDE COMMUNICATION:
 To communicate with the USB_COMM object on an Arduino over USB, a command string with the form "OPC TP1 TP2 TP3" is sent back and forth. The parameters are described below in greater detail. Once this command is received by the Arduino, it will immediately echo back the operation code followed by the requested data.

 If temperatures are being set, the TP parameters echoed back to the computer will be the temperature settings that the Arduino is currently trying to achieve. In most cases, the string echoed back should match the string sent by the computer. If temperatures are being read, the TP parameters sent back to the computer will contain the last recorded thermocouple readings. Finally, if any error is detected in the transmission the Arduino will return and error operation code followed by an error code describing the problem.

%-----------------------------------------------------------%

OPERATION CODES:
 000 - Set temperatures, must be followed by three TP parameters. For example, the command "000 100 200 000" will tell the Arduino to hold heating element 1 at 100 degrees C and heating element 2 at 200 degrees C. The Arduino will echo back "000 100 200 000" to acknowledge that it received the command and adjusted the temperature settings as desired.

 001 - Read temperatures, the following TP parameters are ignored but should usually be zero. For example, the command "001 000 000 000" will ask the Arduino to echo back the most recently recorded temperatures. An example response would be "000 021 022 021", indicating that thermocouple 1 and 3 are reading 21 degrees C while thermocouple 2 is reading 22 degrees C.

 002 - Error reporting. This should not be sent to the Arduino, but if an error in the received command is detected the Arduino will respond with this. For example, the command "000 000 000 00X" will elicit the response "002 001 *** ***". The error code 001 corresponds to an illegal character in the numerical arguments.

    ERROR CODES:
      001 - Illegal character in numerical argument
      002 - Improper command string length
      003 - When setting temperature, TP3 must be zero
      004 - Temperature setting out of range (>300 degrees C)

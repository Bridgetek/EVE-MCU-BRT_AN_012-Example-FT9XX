![Bridgetek Logo](docs/Bridgetek-logo.png "Bridgetek")

# BRT_AN_012 FT9xx USBD HID Touch Panel

Application Note BRT_AN_012 FT9xx USBD HID Touch Panel.  
See https://brtchip.com/software-examples/ft9xx-examples/

Demonstrates a USB HID implementing a Virtual Keyboard display on an Bridgetek EVE screen. 
It uses the [BRT_AN_025](https://github.com/Bridgetek/EVE-MCU-BRT_AN_025) library. 

This project requires a BridgeTek MM900EVxB board and FT812 or BT817 board and display.

## Information

This Application Note describes a virtual keyboard implemented with an FT9XX device and an FT81X/BT8XX touch screen. 
The FT81X and BT81X and later devices are supported. It is assumed that the display is 800 by 480 pixels.

# Introduction

A virtual keyboard is displayed on a touchscreen display and behaves as a standard hardware keyboard when connected via USB to a host computer.

The keyboard can be set to show different keyboard layouts for different regional variations.

![Block Diagram](docs/Figure1.png "Block Diagram")

**Figure 1 Block Diagram**

The FT9XX presents a HID keyboard interface to the USB Host computer. The FT9XX provides a
bridge via QSPI connection to the FT81X/BT8XX device. The FT81X/BT8XX device controls the TFT display and the
touchscreen interface on the display.

## Overview

This document describes the design and implementation of the FT9XX USBD HID Touch Panel code. The FT9XX USBD HID Touch Panel allows a user to:
- Implement a virtual keyboard on a touchscreen device.
- Connect the device to a host via USB.
- Interact with the host as if a hardware keyboard was connected.
- Modify the virtual keyboard for different international or regional layouts.

This document is intended to demonstrate the capabilities of the FT9XX family of microcontrollers by emulating USB devices and interfacing to FT81X/BT8XX display ICs.

## Scope

The virtual keyboard implements a single USB device boot protocol keyboard with a report descriptor to match for a Windows PC. US, UK and standard German keyboard layouts are supported.

There is no specific support for Apple Mac keyboards although most Apple Mac functions can be performed with the keyboard emulated by this application note.

### Features

The application note shows how to implement a USB HID device and interface with an FT81X/BT8XX family display IC. 

The USB device interface is used to send keyboard scan codes to the host and receive reports to change the status of the Caps Lock, Scroll Lock and Num Lock LEDs on the keyboard.

The FT81X/BT8XX interface shows communication with the display IC co-processor, reading touch events and processing these to generate the keyboard scan codes. The display has several screens that can be selected with buttons to allow different layout or part of a keyboard to be displayed.

### Enhancement

This application note can be seen as a start for customisation or extension. Some example enhancements could be:
- Support for other country keyboard layouts.
- Add an Apple Mac layout.
- Tailor keyboard layout and function to a particular application. E.g. CAD or Point of Sale.
- Add a multimedia control interface.
- Implementing remote wakeup to allow the keyboard to wake the host.

This USB Device example application should be treated as an example. Full source code is provided allowing users to build and modify if required.

# Project Overview

The server main program is responsible for detecting a connection from the FT900 to the host PC and instantiating the FT9XX as a USB HID class when enumerated by the host. 
It will also call the EVE library to initialise the FT81X/BT8XX.

Source code is divided in folders for functional components. 

## `Sources` Folder

The main part of the application is found in the `Sources` folder. This consists of the following files: 
- The `main.c` file is generally responsible for the FT9xx setup and USB device code.
- The file `usb_keyboard.c` implements an interface between the USB device and the FT81X/BT8XX display.
- Another file `eve_keyboard.c` controls the EVE keyboard component in the library.

Files in these folders use the `Includes` folder for application specific header files.

## `lib/eve` Folder

This folder holds the API code which abstracts the FT81X/BT8XX register and processing list writes into C functions. 
It is code copied from the BRT_AN_025 Application Note: https://github.com/Bridgetek/EVE-MCU-BRT_AN_025

Only the `ports/eve_arch_ft9xx`, `source` and `include` directories need to be copied. 

The macro `PLATFORM_FT9XX` *MUST* be defined for the proper compilation of this code. 

##	`eve_ui` Folder

In the `eve_ui` folder are several files an routines that simplify the use of EVE:
- The `eve_ui_main.c` file performs general FT81X/BT8XX operations such as initialisation, calibration and detecting touchscreen events (tags). 
It will also generate some simple display lists for “Waiting for Host” and an optional screenshot feature.
- `eve_ui_images.c` contains code for loading JPG images from Program Memory of the FT9XX to the data memory of the FT81X/BT8XX.
- `eve_ui_ext_font.c` which has code to load a custom font extension used by this application to display characters not supported by the built-in fonts on the FT81X/BT8XX.

##	`eve_ui_keyboard` Folder

The `eve_ui_keyboard.c` file draws a keyboard on the EVE screen by sending display lists to the FT81X/BT8XX via the EVE library. 
It will make display lists for each type of keyboard and populate tag information for the touchscreen controller.

## `Images` Folder

The FT81X/BT8XX can display JPEG images. This folder contains the raw JPEG images which are used in the “eve_ui” library. They are copied to the FT81X/BT8XX by function calls in “eve_ui_images.c”.

The files in this folder are loaded into the final application image by creating an assembler file which contains the binary JPG image. 

The process for doing this is handled by a makefile (images.mk) which generates assembler S files for each image in the folder. 
The makefile is run before the main code is compiled and refreshes the source files for the project to include the assembler files. 

### Converting Images in Eclipse

In Eclipse Project Properties, a new “Builder” is added before the “CDT Builder”. The builder is shown in Figure 2.

![Builders Project Settings](docs/Figure2.png "Builders Project Settings") 

**Figure 2 Builders Project Settings**

When the “Images Builder” is run it invokes the makefile which generates the assembler files. 
The builder settings are shown in Figure 3. 

![Images Builder Settings](docs/Figure3.png "Images Builder Settings") 

**Figure 3 Images Builder Settings**

In the Refresh tab of the settings the “Refresh resources upon completion” and “The folder containing the selected resource” checkboxes ensure that the CDT Builder will pick up any changes to the S files generated. 
In the “Build Options” tab the builder should be run “After a clean” and “During manual builds”.

### Converting Images in Command Line

There is a Makefile provided to convert the JPG images for the project into an assembler file. 
The JPG data is included in an assembler file and results in the binary data being included in the final binary with a label to access it.

```
make -C Images -f images.mk
```
The output assembler files are placed in the `S` directory.

### Using the Converted Images in the Program

C code must use the correct method for referencing image data stored this way. 

For instance, the file `bridgetek_logo.jpg` will be loaded by the file `bridgetek_logo.S`. 
In that file there is a label for the start of the image data called `img_bridgetek_logo_jpg`. 
The program must convert this label to a form that can be used in a C program:
```
extern const uint8_t __flash__ img_bridgetek_logo_jpg[] asm("img_bridgetek_logo_jpg");
```
This makes a C array pointer which can then be used as the source address in a `memcpy_pm2dat` call. 
Attempts to use the pointer directly with normal variables will not work as the program memory address space is outside the addressable range of normal C pointers.

Images which have been converted with this method will only appear in the final application image if they are referenced in the code. 

## `FontConvertor` Folder

The FT8XX FNT_CVT utility (EVE Font Converter) is used to change a system TTF into a bitmap image which can be used by the FT81X/BT8XX. 
The application uses the standard Arial font to make 2 font files: 
ASCII characters from 32 to 127 and several UETF-8 characters for key labels not available through normal ASCII.

The extended characters are used to provide arrow images for key labels and non-ASCII key labels for the standard German keyboard layout.

## `Fonts` Folder

The font files in this folder are the .rawh files produced by the FontConvertor. 
They are loaded into the final application image by creating a C file which contains the binary font data in the form of an array. 

### Converting Images in Eclipse

The process for doing this is handled by a makefile (fonts.mk) which generates C files for each font file in the folder. 
The makefile is run before the main code is compiled and refreshes the source files for the project. 
In Eclipse Project Properties, a new “Builder” is added before the “CDT Builder”. 
The builder is called “Fonts Builder” and is shown in Figure 2. 
Builder settings are the same as for the Images Builder.

### Converting Fonts in Command Line

There is a Makefile provided to convert the font raw header files for the project into a C source file. 
The font data is included in an C file and results in the binary data being included in the final binary with a label to access it.

```
make -C Fonts -f fonts.mk
```
The output C source files are placed in the `C` directory.

# Compilation

Provision is made to compile from Eclipse IDE, make and CMake. The BRT_AN_012 source code from the `Bridgetek/BRT_AN_012_FT9xx_USBD_HID_Touch_Panel` repository can be cloned or downloaded from GitHub.

## Importing into the FT9xx Toolchain (Eclipse IDE)

The project code can be imported into the FT9xx Toolchain. 

Select `File` --> `Import` --> `General` --> `Existing Projects into Eclipse`, and point to the location of the downloaded project directory.

![Eclipse Project](docs/Figure4.png "Eclipse Project")

**Figure 4 Eclipse Project**

The project will appear in Eclipse Project Explorer as shown in Figure 4. 
On Eclipse IDE with the BridgeTek FT9xx Plugin, the project can be imported into a workspace and can be compiled and debugged without changes.

### make

From a command line prompt the `make` utility will compile the `bin` file for programming the device. 

Optional parameters for the build are: 
- Select debug or release builds with `BUILD=Release` or `BUILD=Debug`.
- Choose an output directory with `OUTDIR=build`. The defaults are `FT90x_Debug` or `FT90x_Release`.
- Change the name of the generated output files with `PROJECT=name`.

## CMake

From a command line prompt the `cmake` utility will generate and compile the `bin` file for programming the device. 

The generator "MinGW Makefiles" is used for the FT9xx Toolchain. Also, it is useful to generate build files in an intermediate directory.
A good command line to choose to generate build files for CMake is:

`cmake -G "MinGW Makefiles" -B build -S .` 

Then the binary can be compiled with:

`cmake --build build`

Optional parameters for the build are: 
- Select debug or release builds with `-DBUILD=Release` or `-DBUILD=Debug`.
- Choose an output directory with `-DOUTDIR=build`. The defaults are `FT90x_Debug` or `FT90x_Release`.
- Change the name of the generated output files with `-DPROJECT=name`.

# Changing the Application Software

The application software provided can be altered and changed if required.

With each software change, the project should be rebuilt and reprogrammed into the FT90x IC. Please refer to AN_325 FT9xx Toolchain Installation Guide for further information.

Windows will not reload a driver for a device with the same VID/PID and serial number but a different USB Class. If there are driver problems then remove the "USB Composite Device" detected for the application in Windows Device Manager then unplug and replug the device.

# Software Implementation

The application note implements a USB HID class device which can be accessed by a USB host with appropriate driver software. 
The USB device code is implemented in `main.c` and USB HID class specific code is in `keyboard.c`. 
All virtual keyboard drawing is carried out in the `eve_ui` library which invokes the eve library for low-level control of the FT81X/BT8XX device.

Keypress events detected on the virtual keyboard will generate “tags” which uniquely represent one keyboard action. 
These tags indicate that a key has been pressed, such as a letter or number key. 
Each tag received will be converted into a HID report and sent to the host via USB.

## USB Implementation Overview

The USB device implements a composite device consisting of 2 HID interfaces. 
The endpoints on these interfaces will produce one of 3 different report descriptor types when a key is pressed. 

The first interface is a “Boot Report” type interface with a single endpoint producing a single report to the host; 
the second interface can either produce Application Reports (Power Down, Suspend etc.) or Consumer Control Reports (Media Controls etc.).

![USB Interface Diagram](docs/Figure5.png "USB Interface Diagram")

**Figure 5 USB Interface Diagram**

The application will decode the type of the key pressed on the virtual keypad to one of the 3 report types by identifying the key’s tag in a lookup table. 
The lookup table can be modified or extended to suit the application.

### DFU Implementation

The application note code also includes code for a USB DFU device (Device Firmware Upgrade) and Microsoft WCID descriptors to load the WinUSB drivers on the host system to facilitate firmware upgrades. 
This code is ignored in this text and excluded by the pre-processor in the source code.

## USB Code Overview

The logical model of the USB device software, as coded in `main.c`, is as follows:

![USB Device Software Flow](docs/Figure6.png "USB Device Software Flow")

**Figure 6 USB Device Software Flow**

Once the USB device is detected to be connected to the USB host the device driver is initialised. 
Although the project is designed to work bus-powered, the initial check is required to allow a bus speed test to be made and appropriate endpoints to be created.

The USB device connection is found with an API call made to USBD_connect function and the speed determined with an API call to USBD_get_bus_speed.

Once initialised, the software will continue to check for a proper connection to the host. 
If this drops then it will return to the state of waiting for a connection. 
If the device is not bus-powered then it will re-detect the host and bus speed.

The API call to USB_process will update the USB device state and handle any control endpoint requests from the host. 
When starting the device this will primarily be enumeration requests and result in the USB device state moving from ATTACHED to POWERED, then to DEFAULT and then to ADDRESSED. 
At this point the USB host can address non-control endpoints on the device. Finally, once the device driver on the host completes initialisation it will move to CONFIGURED state. 
These states are defined, explained and discussed in the USB Specification.

The USB device in this code waits for the CONFIGURED state to be reached before checking for reports to send to the host. 

### USB Descriptors

All descriptors which are used in responses to standard USB SETUP requests are placed in `main.c`.
- The Device Descriptor
- 2 Configuration Descriptors, High-speed and Full-speed
- A Device Qualifier Descriptor
- String Descriptors
- 2 HID Report Descriptors

A standard request USB Other Speed Configuration Descriptor is generated using the data from the Configuration Descriptors. 

### USB Control Endpoint Requests

USB requests are handled by the USB device driver when the USBD_process API call is made. 
This will invoke callback functions in main.c to process standard USB requests for descriptors and state changing, but will also receive Class and Vendor requests. 

Class requests are received in the class_req_cb function and are decoded to call the class_req_interface_hid function if this is required. 
This class-specific function deals with interface requests to the HID device. 
It will process the Get/Set Idle, Get /Set Protocol and Get/Set Report requests. 
These handlers may call the keyboard interface code to retrieve or set states.

## Keyboard Code Overview

The keyboard interface code keeps an internal state of the HID class device, treating the 2 HID class interfaces separately. 

The state of a HID interface is affected by the SetIdle request and the SetProtocol request. 
The enable state of the Boot Report keyboard (whether it transmits data to the host on its Interrupt IN endpoint) is set by the reception of a suitable SetIdle request. 
The generic HID interface is enabled once the device reaches the CONFIGURED state.

The keyboard function keyboard_loop is polled and this will receive tags from the virtual keyboard, translate these to reports and return a status indicating whether data needs to be transmitted to the host.

### Generating Reports

When a tag is received from the virtual keyboard it will be translated into a HID report by the keyboard code in `keyboard.c`. 
Depending on the tag, the HID report will be one of the three defined types. 

The Generic HID interface on which consumer control and system control reports are sent requires a report ID to identify the type of the report. 
This is used by the host to match the report up to a report descriptor. 
The Boot Report USB interface has a single report defined and therefore does not require a report ID.

Consumer control and system control reports are normally on-off controls (OOCs) or one-shot controls (OSCs) as defined in the Universal Serial Bus HID Usage Tables. 
Modification of the report generation code will be required for other types of Usage Types.

#### Boot Reports

Boot reports contain several flags for ‘modifier’ keys (shift, control, alt and GUI) and one-byte scancodes which identify any ‘normal’ key pressed. 
If no normal key is pressed then the scancode will be zero. When any key is released then a report is sent with either or both the flag and scancode cleared. 

#### Consumer Control Reports

The consumer control report will send a two-byte scancode with the appropriate function for the tag. 

The report ID for consumer controls is 2.
 
#### System Control Reports

A system control report allows for one (or more) of the power down, sleep or wake flags to be set.

The report ID for system controls is 1.

## Virtual Keyboard Code Overview

The FT81X/BT8XX device relies on a method called a display list to determine what is shown on the screen. 
This means that a new display list is generated only when a display change is required. 

Each virtual “key” on the keyboard is assigned a unique tag. 
The function `eve_ui_keyboard_loop` in `eve_ui_keyboard.c` source code file detects a touchscreen press and returns the tag of the button.
Some buttons are reserved within the `eve_ui` library to enable switching between keyboard, keypad, media and special screens. 

The `eve_ui` library is designed to isolate the screen drawing and management from the features of the USB keyboard device. 
This will allow it to be used as data entry method for other applications. 
Only decoding of `eve_ui` defined tags to actions (or characters) is needed to add text entry. 

The screen header is specifically drawn in `eve_ui_main.c` to allow for the library to be expanded with additional methods.

## Optional Features

### Special Screens

A special screen is demonstrated which has a key layout specific to an application. This screen is selected using the orange ‘Z’ on the header bar.
 
# Using the Virtual Keyboard

## Required Hardware

The application note is intended to be used on an ME812A-WH50R or ME813A-WH50C EVE development module with an MM900EV2A or MM900EV3A MCU module. 
The application note is written to work on an 800 by 600 display. 
Changes to the EVE module or FT9XX device can be made in the `FT_platform.h` file in the `Includes` folder of the source code.

The MM900EVxA module connects directly to the EVE development module with a set of pin headers. 
The QSPI interface on the FT9XX device is taken through the pin headers to the FT81X/BT8XX on the EVE module. 
The host PC connects via USB to the MM900EVxA module.

![MM900EV2A and ME812A-WH50R module](docs/Figure7.jpg "MM900EV2A and ME812A-WH50R module")

**Figure 7 MM900EV2A and ME812A-WH50R module**

## Use of Application Note Software

The virtual keyboard will wait until it is connected to a host. The FT81X/BT8XX display will indicate this with the Bridgetek logo and the caption “Waiting for host...”.

Once the host is connected then it will display the virtual keyboard.

 ![Virtual Keyboard Screen](docs/Figure8.jpg "Virtual Keyboard Screen")

**Figure 8 Virtual Keyboard Screen**

The settings screen, is accessed from the “Settings” button allowing the choice of UK, US and German keyboard layouts. 
Pressing “Settings” in the upper left hand corner will display an alphanumeric keyboard section of the virtual keyboard.

Pressing the “KeyPad” button will show the control and keypad area of a standard keyboard. 
To return to the main keyboard press the “Keyboard” button again.

 
 ![Virtual KeyPad Screen](docs/Figure9.jpg "Virtual KeyPad Screen")

**Figure 9 Virtual KeyPad Screen**

Pressing the “Z” button will show the special application screen. The keyboard or keypad can then be accessed with appropriate buttons.

There is no functionality in the special application screen.

##	Keyboard Layouts

There are 3 different layouts for keys supported in this application. 
The US and UK layouts are QWERTY layouts where the key positions change slightly; 
the German layout is QWERTZ, however the layout is the same as the UK layout. 
For the German keyboard to work the host needs to have its keyboard locale set to German to allow the scan codes to match the key labels.

# Appendix C – Revision History

Document Title: BRT_AN_012 FT9xx USBD HID Touch Panel

Product Page: http://brtchip.com/m-ft9/

| Revision | Changes | Date |
| ------------- |:-------------:| -----:|
| 1.0 | Initial Release | 03/07/2017 | 
| 1.1 | Fixed the Product Page broken link under Section Appendix C – Revision History. | 31/10/2017 | 
| 1.2 | Improvements to code and to the scope and coverage of the document. | 20/08/2| 1.3 | This release will support FT9xx Toolchain version V2.5.0 and later. Older toolchain versions are not supported due to an API change in the usbd.c library file. | 05/03/2019 |
| 1.4 | Rewrite to use released version of [BRT_AN_025](https://github.com/Bridgetek/EVE-MCU-BRT_AN_025) | 11/01/2023 |
 |



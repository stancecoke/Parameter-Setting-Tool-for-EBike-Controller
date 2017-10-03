# Parameter-Setting-Tool-for-EBike-Controller
Open source project for controllers with 78K0R/Kx3 processors like KU63, XCM

Write your personal setting to your controller with the DIY programmer.

Caution! This project is in a test phase, I was not able to write the parameters successfully yet. There is the risk of bricking your controller! 

Usage: 
1. Flash the .ino code to your ST32 development board
2. Connect the STM32 Board to the PC and to the controller.
3. Reset the STM32 Board and start a terminal program like HTERM on your PC and connect to the COM-Port of the STM32 board
4. send any character with the terminal program to set the controller to programming mode

now the controller is ready to receive commands from the STM32 board

there are 5 functions implemented so far.
1. send "1" (not Ascii but byte): Read Checksum of Memory Block. Each time you send "1" the next memory block is checked.
2. send "2" : you are asked to upload the binary file with the parameter settings (file has to contain 161 byte of data), use the "send file" button from HTERM
3. send "3" : Writing to Block 15 gets prepared
4. send "4" : Writes Data to Block 15 (here I get no response from the controller, but writing seems to be done. I bricked my controller with this, I don't know if I have the wrong controller or Block 15 is not the right adress)
5. send "5" : Erase Block 15

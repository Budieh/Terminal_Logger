#ifndef TERMINAL_LOG_H_
#define TERMINAL_LOG_H_

// Includes
#include <iostream>
#include <fstream>
#include <string>   
#include <thread>     // library used for creating threads out of functions defined previously
#include <conio.h>    // library containing _getch() on Windows
#include <windows.h>  // Windows API
#include <sys/stat.h> // library used for checking the status of a path

// Functions

/* Function used for fetching data from the serial port, displaying it in the terminal and logging it */
void logSerialData(const volatile HANDLE & hSerial, std::ofstream &logFile, const bool & logger, volatile bool & condition);
// hSerial      -> handle object used for interacting with the serial port
// logFile      -> file object to log the messages from the terminal to
// logger       -> boolean value to check if the terminal data has to be logged or not
// condition    -> condition variable that dictates when the program ends

/* Function used for sending data through the serial port */
void sendSerialData(const volatile HANDLE & hSerial, std::ofstream & logFile, const bool & logger, volatile bool & condition); 
// hSerial      -> handle object used for interacting with the serial port
// logFile      -> file object to log the messages from the terminal to
// logger       -> boolean value to check if the terminal data has to be logged or not
// condition    -> condition variable that dictates when the program ends

/* Function used for setting up the serial communication */
bool setupSerialPort(volatile HANDLE & hSerial, const int & COM_number, const int * SerialParams);
// hSerial      -> handle object used for interacting with the serial port
// COM_number   -> number of the serial port to interact with 
// SerialParams -> vector containing serial data used for setting the parameters for the communication through the serial port

/* Function used for perpetually checking the connection with the serial device */
void CheckForConnection(volatile HANDLE & hSerial, std::ofstream &logFile, const bool & logger, const int & COM_number, int * SerialParams, volatile bool & condition);
// hSerial      -> handle object used for interacting with the serial port
// logFile      -> file object to log the messages from the terminal to
// logger       -> boolean value to check if the terminal data has to be logged or not
// COM_number   -> number of the serial port to interact with 
// SerialParams -> vector containing serial data used for setting the parameters for the communication through the serial port
// condition    -> condition variable that dictates when the program ends

/* Function used for checking if a certain file exist or not */
bool exist(const std::string & file_name);
// file_name    -> name of the file

/* Function used for checking the comnnection with the serial device */
bool isSerialConnected(const volatile HANDLE & hSerial);
// hSerial      -> handle object used for interacting with the serial port

/* Function used for loggind data */
void logData(const std::string & data, std::ofstream & logFile);
// data         -> data to be written to the logFile
// logFile      -> file object to log the messages from the terminal to

#endif
// Parent library
#include "terminal_log.hpp"

// Function used for fetching data from the serial port, displaying it in the terminal and logging it
void logSerialData(const volatile HANDLE & hSerial, std::ofstream &logFile, const bool & logger, volatile bool & condition) 
{
    char buffer[256];
    unsigned int iterator = 0;
    unsigned long bytesRead = 0;

    while (condition) 
    {
        if (ReadFile(hSerial, buffer + iterator, 1, &bytesRead, NULL) && bytesRead > 0) 
        {
            std::cout << buffer[iterator];

            if( logger == true )
            {
               if(buffer[iterator] == '\r' && iterator >= 1)
               {
                  std::cout << std::endl;

                  if((buffer[0] == '\n') || (buffer[0] == '\r')) 
                  {
                    buffer[0] = ' ';
                  }

                  buffer[iterator] = '\r';
                  buffer[iterator + 1] = '\0'; // Null-terminate the string
                  std::string data(buffer);

                  logData(data, logFile);

                  iterator = -1;   
               }
            iterator++;
            }

            else if(buffer[iterator] == '\r')
            {
                  std::cout << std::endl;
            }
        }
    }
}

// Function used for perpetually checking the connection with the serial device
void CheckForConnection(volatile HANDLE & hSerial, std::ofstream &logFile, const bool & logger, const int & COM_number, int * SerialParams, volatile bool & condition)
{
    PerpetualCheck:

    while(condition && isSerialConnected(hSerial))
    {
    }

    // if communication hasn't been terminated by the user and the serial port doesn't respond
    if(condition && (!isSerialConnected(hSerial)))
    {
        std::cout << "Connection lost. Attempting reconnect..." << std::endl;

        if( logger == true )
        {
            logData(std::string("Connection lost. Attempting reconnect...") + std::string("\r\0"), logFile);
        }
        
        HANDLE hSerial_copy;

        std::chrono::seconds duration(5);
        unsigned short i;
        for(i = 0; i < 5 ; i++)
        {
            CloseHandle(hSerial_copy);
            std::cout << "Attempt " << i + 1 << ": ";

            std::this_thread::sleep_for(duration);
            setupSerialPort(hSerial_copy, COM_number, SerialParams);
            if(isSerialConnected(hSerial_copy))
            {
                std::cout << "successfull" << std::endl;

                if( logger == true )
                {
                   logData(std::string("Attempt ") + std::to_string(i+1) + std::string(": successfull") + std::string("\r\0"), logFile);
                   std::cout << "You can proceed using the terminal! " << std::endl << std::endl;
                   logData(std::string("You can proceed using the terminal! ") + std::string("\r\0"), logFile);
                }

                break;
            }
            else
            {
                std::cout << "unsuccessfull" << std::endl;

                if( logger == true )
                {
                   logData(std::string("Attempt ") + std::to_string(i+1) + std::string(": unsuccessfull") + std::string("\r\0"), logFile);
                }
            }
        }

        if(i == 5)
        {
            condition = false;
            std:: cout << "Session terminated. Press any key to continue..." << std::endl;

            if( logger == true )
            {
               logData(std::string("Session terminated. Press any key to continue...") + std::string("\r\0"), logFile);
            }
        }

        else
        {
            CloseHandle(hSerial);
            hSerial = hSerial_copy;
            goto PerpetualCheck;
        }
    }
}

// Function used for sending data through the serial port
void sendSerialData(const volatile HANDLE & hSerial, std::ofstream &logFile, const bool & logger, volatile bool & condition) 
{
    char buffer[256];
    int iterator = 0;
    unsigned long bytesWritten;

    // Continuously read characters from the console and send to COM port
    while (condition)
    {
        buffer[iterator] = _getch();  // Read a single character from the console

        // Echo character to console
        std::cout << buffer[iterator];

        if (buffer[iterator] != '\r') // Enter key is not pressed
        {
            // Backspace character removes the last character from the buffer
            if(buffer[iterator] == '\b')
            {
               buffer[iterator] = 0x7F;

               // Send the character to the COM port
               if (!WriteFile(hSerial, buffer + iterator, 1, &bytesWritten, NULL))
               {
                  std::cerr << "Error writing to serial port: " << GetLastError() << std::endl;
               }

               if(iterator >= 1)
               {
                  iterator-=2;
               }
            }

            else
            {
               // Send the character to the COM port
               if (!WriteFile(hSerial, buffer + iterator, 1, &bytesWritten, NULL))
               {
                  std::cerr << "Error writing to serial port: " << GetLastError() << std::endl;
               }
            }
        }

        else
        {  
            std::cout<< std::endl; 

            // Send the character to the COM port
            if (!WriteFile(hSerial, buffer + iterator, 1, &bytesWritten, NULL))
            {
               std::cerr << "Error writing to serial port: " << GetLastError() << std::endl;
            }

            buffer[iterator] = '\r';
            buffer[iterator + 1] = '\0';
            std::string data(buffer);
            
            if(logger)
            {
               // Get current time and log it with the data
               logData(data, logFile);

            }
            
            if(data.find(std::string("\\exit\r"),0) != std::string::npos)
            {
                condition = false;
                if(logger)
                {
                    logData(data, logFile);
                }
            }

            iterator = -1;
        } 

        iterator++;
    }
}

// Function used for setting up the serial communication
bool setupSerialPort(volatile HANDLE & hSerial, const int & COM_number, const int * SerialParams) 
{
    const std::string port("\\\\.\\COM");
    std::string COM;
    COM = port + std::to_string(COM_number);

    // opening the serial port
        hSerial = CreateFileA(
        COM.c_str(),  
        GENERIC_WRITE | GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hSerial == INVALID_HANDLE_VALUE) 
    {
        return false;
    } 

    // Set serial port parameters
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) 
    {
        std::cerr << "Error getting serial port state: " << GetLastError() << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    if(SerialParams == NULL)
    {
       dcbSerialParams.BaudRate = CBR_115200; // Baud rate: 115200 bauds
       dcbSerialParams.ByteSize = 8;          // Data size: 8 bits
       dcbSerialParams.StopBits = ONESTOPBIT; // Stop bit: 1
       dcbSerialParams.Parity = NOPARITY;     // Parity: No parity
    }

    else
    {
       dcbSerialParams.BaudRate = SerialParams[0];  
       dcbSerialParams.ByteSize = SerialParams[1]; 
       dcbSerialParams.StopBits = SerialParams[2];
       dcbSerialParams.Parity = SerialParams[3];
    }

    if (!SetCommState(hSerial, &dcbSerialParams)) 
    {
        std::cerr << "Error setting serial port state: " << GetLastError() << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    // Timeouts configuration
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) 
    {
        std::cerr << "Error setting timeouts: " << GetLastError() << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    return true;
}

// Function used for checking if a certain file exist or not
bool exist(const std::string & file_name)
{
  std::ifstream input_file; 
  input_file.open(file_name, std::fstream::in);
  if (input_file.is_open())
  {
     input_file.close();
     return true;
  }
  return false;
}

// Function used for checking the comnnection with the serial device
bool isSerialConnected(const volatile HANDLE & hSerial) {
    COMSTAT status;
    unsigned long errors;

    if (!ClearCommError(hSerial, &errors, &status)) {
        return false;
    }
    return true;
}

// function used for loggind data
void logData(const std::string & data, std::ofstream & logFile)
{
   // Get current time and log it with the data
   SYSTEMTIME time;
   GetLocalTime(&time);
   logFile << "[" << time.wYear << "-" << time.wMonth << "-" << time.wDay << " "
    << time.wHour << ":" << time.wMinute << ":" << time.wSecond << "] "
    << data << std::endl;  
}
// Parent library
#include "terminal_log.hpp"

// Function used for fetching data from the serial port, displaying it in the terminal and logging it
void logSerialData(HANDLE hSerial, std::ofstream &logFile, const bool & logger, volatile bool & condition) 
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

                  // Get current time and log it with the data
                  SYSTEMTIME time;
                  GetLocalTime(&time);
                  logFile << "[" << time.wYear << "-" << time.wMonth << "-" << time.wDay << " "
                     << time.wHour << ":" << time.wMinute << ":" << time.wSecond << "] "
                     << data << std::endl;

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

// Function used for sending data through the serial port
void sendSerialData(HANDLE hSerial, std::ofstream &logFile, const bool & logger, volatile bool & condition) 
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
               SYSTEMTIME time;
               GetLocalTime(&time);
               logFile << "[" << time.wYear << "-" << time.wMonth << "-" << time.wDay << " "
                  << time.wHour << ":" << time.wMinute << ":" << time.wSecond << "] "
                  << data << std::endl;
            }
            
            if(data.find(std::string("\\exit\r"),0) != std::string::npos)
            {
                condition = false;
                if(logger)
                {
                   SYSTEMTIME time;
                   GetLocalTime(&time); 
                   logFile << "[" << time.wYear << "-" << time.wMonth << "-" << time.wDay << " "
                      << time.wHour << ":" << time.wMinute << ":" << time.wSecond << "] "
                      << std::string("End of current session...") << std::endl;
                }
            }

            iterator = -1;
        } 

        iterator++;
    }
}

// Function used for setting up the serial communication
bool setupSerialPort(HANDLE & hSerial, const int & COM_number, const int * SerialParams) 
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
       dcbSerialParams.BaudRate = CBR_115200;
       dcbSerialParams.ByteSize = 8;
       dcbSerialParams.StopBits = ONESTOPBIT;
       dcbSerialParams.Parity = NOPARITY;
    }

    else
    {
       dcbSerialParams.BaudRate = SerialParams[0];  // Baud rate: 115200 bauds
       dcbSerialParams.ByteSize = SerialParams[1];  // Data size: 8 bits
       dcbSerialParams.StopBits = SerialParams[2];  // Stop bit: 1
       dcbSerialParams.Parity = SerialParams[3];    // Parity: No parity
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
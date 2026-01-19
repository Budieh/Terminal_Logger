// Library used for interacting with the terminal 
#include "terminal_log.hpp"

int main() {

    int nr_of_trials, COM_number = -1;
    HANDLE hSerial;
    std::ofstream logFile;
    bool logger = false, condition = true;
    std::string log_ans, path, SerialParams_ans;
    int * SerialParams = nullptr;

    std::cout << "Do you wish to log this session? [y/n]: ";
    std::cin >> log_ans;

    if(log_ans[0] == 'y')
    {
       nr_of_trials = 5;
 
       // Stat structure used for storing the metadata of the directory
       struct stat sb;

       Enter_Path:

       std::cout << "Enter the path of the folder where the log file is to be saved: ";
       std::cin >> path;

       // Checking to see if the path is valid
       if (stat(path.c_str(), &sb) == 0)
       {
          std::cout << "Valid path!" << std::endl;
       }

       else if(--nr_of_trials)
       {
          std::cout << "Path entered is invalid. Retry!" << std::endl; 
          goto Enter_Path;
       }

       else
       {
          return -1;
       } 

       logger = true; 
       unsigned int file_nr = 1;
       std::string file = path + "/" + "terminal_log_1.txt";
       while(exist(file))
       {
          file = path + "/" + "terminal_log_" + std::to_string(++file_nr) + ".txt";
       } 
       
       // Open log file
       logFile.open(file, std::fstream::out);
       if (!logFile.is_open())
       {
          std::cerr << "Error opening log file" << std::endl;
          CloseHandle(hSerial);
          return -1;
       }
    }

    std::cout << "Do you wish to set up serial communication parameters ([n] leaves them as default)? [y/n]: ";
    std::cin >> SerialParams_ans;

    if(SerialParams_ans[0] == 'y')
    {
       SerialParams = new int[4];
       std::cout << "Baud Rate: ";
       std::cin >> SerialParams[0];
       std::cout << "Data bits: ";
       std::cin >> SerialParams[1];
       std::cout << "Stop bits: ";
       std::cin >> SerialParams[2];
       std::cout << "Parity bits: ";
       std::cin >> SerialParams[3];
    }

    nr_of_trials = 5;

    Enter_COM_number:

    std::cout << "Enter COM port number: ";
    std::cin >> COM_number;

    if(setupSerialPort(hSerial, COM_number, SerialParams))
    {
       std::cout << "Port opened succesfully! Write \\exit and press enter to end session" << std::endl;  
    }

    else if(--nr_of_trials)
    {
       std::cout << "Port failed to open properly. Retry!" << std::endl; 
       goto Enter_COM_number;
    }

    else
    {
        return -1;
    } 

    // Start a thread for logging data ( in paralel with reading bytes from terminal )
    std::thread logThread(logSerialData, std::ref(hSerial), std::ref(logFile), std::ref(logger), std::ref(condition));

    // Start a thread for perpetually checking the communication with the serial device
    std::thread connectionThread(CheckForConnection, std::ref(hSerial), std::ref(logFile), std::ref(logger), std::ref(COM_number), SerialParams, std::ref(condition));

    // Main thread sends commands to serial port
    sendSerialData(hSerial, std::ref(logFile), logger, condition);

    // Join threads (in case they are terminated)
    logThread.join();
    connectionThread.join();

    if(logger == true)
    {
       logData(std::string("End of current session...") + std::string("\r\0"),logFile);
    }

    // Closing log file
    if(logFile.is_open())
    {
        logFile.close();
    }

    // Deleting SerialParams array
    delete[] SerialParams;

    // Closing serial port
    CloseHandle(hSerial);

    return 0;
}

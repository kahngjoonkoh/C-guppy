#ifndef FILELOGGER_HPP
#define FILELOGGER_HPP

#include <fstream>
#include <chrono>
#include <ctime>

//Modified version of https://gist.github.com/To0ki3/3886428

class FileLogger {

    public:

        enum e_logType { LOG_ERROR, LOG_WARNING, LOG_INFO };


        explicit FileLogger (const char *engine_version, bool e, const char *fname = "log.txt")
                :   numWarnings (0U),
                    numErrors (0U)
        {
            enabled = e;
            if (enabled) {
                 myFile.open (fname);

                // Write the first lines
                if (myFile.is_open()) {
                    auto t = std::chrono::system_clock::now();
                    std::time_t datetime = std::chrono::system_clock::to_time_t(t);

                    myFile << "guppy v" << engine_version << " @ " << std::ctime(&datetime) << std::endl;

                } // if
            }


        }

        ~FileLogger () {

            if (myFile.is_open()) {
                // Report number of errors and warnings
//                myFile << numWarnings << " warnings" << std::endl;
//                myFile << numErrors << " errors" << std::endl;
                myFile.close();
            } // if

        }


        // Overload << operator using log type
        friend FileLogger &operator << (FileLogger &logger, const e_logType l_type) {
            if (logger.enabled) {
                switch (l_type) {
                case FileLogger::e_logType::LOG_ERROR:
                    logger.myFile << "[ERROR]: ";
                    ++logger.numErrors;
                    break;

                case FileLogger::e_logType::LOG_WARNING:
                    logger.myFile << "[WARNING]: ";
                    ++logger.numWarnings;
                    break;

                default:
                    logger.myFile << "[INFO]: ";
                    break;
                } // sw
            }
            return logger;

        }


        // Overload << operator using C style strings
        // No need for std::string objects here
        friend FileLogger &operator << (FileLogger &logger, const char *text) {
            if(logger.enabled) {
                            logger.myFile << text << std::endl;
            }
            return logger;
        }

        friend FileLogger &operator << (FileLogger &logger, const std::string text) {
            if(logger.enabled) {
                            logger.myFile << text << std::endl;
            }
            return logger;
        }



        // Make it Non Copyable (or you can inherit from sf::NonCopyable if you want)
        FileLogger (const FileLogger &) = delete;
        FileLogger &operator= (const FileLogger &) = delete;



    private:
        bool                     enabled;
        std::ofstream           myFile;
        unsigned int            numWarnings;
        unsigned int            numErrors;

}; // class end


#endif // FILELOGGER_HPP

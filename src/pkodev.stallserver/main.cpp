/*
 * Stall Server project for TOP/PKO/KOP server
 *          for the implementation of a system of offline stalls feature
 * (C) V3ct0r from forum PKODev.NET - Community of Developers & Administrators
 * URL https://pkodev.net/profile/3-v3ct0r/
 * 10/08/2021
 * 
 * This program is free software
 */
#include "Utils.h"
#include "Logger.h"
#include "ConfigFile.h"
#include "Server.h"

#include <iostream>


// Print welcome message
void Welcome();

// Initialize logging system
bool InitLogger();

// Load server settings from file
bool LoadConfig(const std::string& path, pkodev::settings_t& settings);


// Entry point
int main(int argc, char *argv[])
{
    // Disable <CTRL> key handling
    SetConsoleCtrlHandler(nullptr, TRUE);

    // Disable 'close' button of console window
    EnableMenuItem(
        GetSystemMenu(
            GetConsoleWindow(),
            FALSE
        ),
        SC_CLOSE, 
        MF_GRAYED
    );

    // Set console window title
    SetConsoleTitle(L"PKOdev.NET :: Stall Server");

    // Print welcome message
    Welcome();

    // Initialize logging system
    bool ret = InitLogger();

    // Check that the logging system has been successfully initialized
    if (ret == false)
    {
        // Print a message
        std::cout << "[Warning] Failed to initialize the logging system!" << std::endl;
    }

    // Server settings structure
    pkodev::settings_t settings;

    // Get a path to the file with server settings
    std::string cfg_path( pkodev::utils::file::change_fileext(std::string(argv[0]), "cfg") );

    // Load server settings from file
    ret = LoadConfig(cfg_path, settings);

    // Check that the server settings have been successfully loaded from the file
    if (ret == false)
    {
        // Cancel server start
        std::cout << "[Error] Server startup is canceled!" << std::endl;
        return 1;
    }

    // Print a message that the server will be started soon
    std::cout << "The server will now start . . ." << std::endl << std::endl;

    // The server instance
    pkodev::Server server(settings);

    // String with the command from the user
    std::string command("");
 
    // Start the server
    try
    {
        // Starting . . .
        server.run();

        // Print a message that the server is running now
        std::cout << "Server successfully started on address " 
                  << settings.game_host << ":" << settings.game_port << "!" << std::endl;
        std::cout << "Type '/stop' to stop the server. " << std::endl;

        // Accept commands from the user
        while (server.is_running() == true)
        {
            // Read a string from the user input stream
            std::getline(std::cin, command);

            // Check for errors in the input stream
            if (std::cin.fail() == true)
            {
                // Clear error flags
                std::cin.clear();
                continue;
            }
            
            // Check that the user entered '/stop' command to stop the server
            if (command.compare("/stop") == 0)
            {
                // Exit the loop
                break;
            }

            // User enetered an unknown command
            std::cout << "Unknown command!" << std::endl;
        }

        // Print a message that the server is stopping
        std::cout << "The server is being stopped . . ." << std::endl;
    }
    catch (const pkodev::server_exception& e)
    {
        std::cout << "Server error: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "Generic error: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "Unknown server error!" << std::endl;
    }

    // Print a message that server has been stopped!
    std::cout << "The server has been stopped!" << std::endl;

    return 0;
}

// Print welcome message
void Welcome()
{
    std::cout << "Stall Server version 4.0" << std::endl;
    std::cout << "* Author: V3ct0r from PKODev.NET" << std::endl;
    std::cout << "* Date: 09/20/2021" << std::endl;
    std::cout << "* URL: https://pkodev.net/profile/3-v3ct0r/" << std::endl << std::endl;
    std::cout << "libevent version: " << event_get_version() << std::endl << std::endl;
}

// Initialize logging system
bool InitLogger()
{
    // Directory name for storing logs
    const std::string dir("logs");

    // Security descriptor
    SECURITY_ATTRIBUTES Attrib;
    Attrib.nLength = sizeof(SECURITY_ATTRIBUTES);
    Attrib.lpSecurityDescriptor = NULL;
    Attrib.bInheritHandle = false;

    // Create a directory for storing logs
    if (::CreateDirectoryA(dir.c_str(), &Attrib) == FALSE)
    {
        // Check that the directory already exists
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            // Error
            return false;
        }
    }

    // Get system time
    tm timeinfo = pkodev::utils::time::system_time();

    // Make a path to a log file
    char path[256]{ 0x00 };
    _snprintf_s(
        path,
        sizeof(path),
        _TRUNCATE,
        "%s\\log_%02d-%02d-%04d_%02d-%02d-%02d.txt",
        dir.c_str(),                                  // Directory
        timeinfo.tm_mon,                              // Day
        timeinfo.tm_mday,                             // Month
        timeinfo.tm_year,                             // Year
        timeinfo.tm_hour,                             // Hour
        timeinfo.tm_min,                              // Minute
        timeinfo.tm_sec                               // Second
    );

    // Open the log file
    return ( pkodev::Logger::Instance().open( std::string(path) ) );
}

// Load server settings from file
bool LoadConfig(const std::string& path, pkodev::settings_t& settings)
{
    // Load the settings
    try
    {
        // .cfg file with server settings
        pkodev::ConfigFile cfg;

        // Print a message about settings loading
        std::cout << "Loading server settings from file '" << path << "' . . ." << std::endl;

        // Load the settings from .cfg file
        cfg.load(path);

        // Required parameters existence flag
        bool cfg_valid = (
            (cfg.has("ToClient", "host")                == true) &&
            (cfg.has("ToClient", "port")                == true) &&
            (cfg.has("ToClient", "max_clients_per_ip")  == true) &&
            (cfg.has("ToClient", "connection_interval") == true) &&
            (cfg.has("ToClient", "max_player")          == true) &&
            (cfg.has("ToGate",   "host")                == true) &&
            (cfg.has("ToGate",   "port")                == true) &&
            (cfg.has("ToGate",   "ip_mod")              == true) &&
         /* (cfg.has("Map",      "map")                 == true) && */
            (cfg.has("Game",     "max_stalls_per_ip")   == true) &&
            (cfg.has("Game",     "max_offline_time")    == true)
        );

        // Check all required parameters
        if (cfg_valid == false)
        {
            // Some required parameters not found
            std::cout << "[Error] Failed to load settings because required parameters not found!" << std::endl;
            return false;
        }

        // StallServer.exe connection settings
        settings.game_host           = cfg.get("ToClient", "host", "0.0.0.0").to_string(); 
        settings.game_port           = static_cast<unsigned short int>(cfg.get("ToClient", "port", "1973").to_integer());
        settings.max_player          = static_cast<unsigned short int>(cfg.get("ToClient", "max_player", "2048").to_integer());
        settings.max_clients_per_ip  = static_cast<unsigned short int>(cfg.get("ToClient", "max_clients_per_ip", "32").to_integer());
        settings.connection_interval = static_cast<unsigned long long>(cfg.get("ToClient", "connection_interval", "1000").to_integer());

        // GateServer.exe connection settings
        settings.gate_host   = cfg.get("ToGate", "host", "127.0.0.1").to_string();
        settings.gate_port   = static_cast<unsigned short int>(cfg.get("ToGate", "port", "2715").to_integer());
        settings.gate_ip_mod = cfg.get("ToGate", "ip_mod", "true").to_bool();

        // List of maps on which the offline stall system works
        for (std::size_t i = 0;
            i < static_cast<std::size_t>(cfg.param_count("Map", "map"));
            ++i)
        {
            // Get map name
            std::string map_name = cfg.get("Map", "map", "", i).to_string();

            // Check that name of the map not empty
            if (map_name.empty() == false)
            {
                // Add the map name in lower case to the list
                settings.maps.push_back( pkodev::utils::string::lower_case(map_name)  );
            }
        }

        // Game logic settings
        settings.max_stalls_per_ip = static_cast<unsigned short int>(cfg.get("Game", "max_stalls_per_ip", "0").to_integer());
        settings.max_offline_time  = static_cast<unsigned int>(cfg.get("Game", "max_offline_time",  "0").to_integer());

        // Print a message that settings are loaded
        std::cout << "Settings successfully loaded!" << std::endl << std::endl;

        // Success
        return true;
    }
    catch (const pkodev::config_file_exception& e)
    {
        std::cout << "[Error] Failed to load settings: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "[Error] Failed to load settings: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "[Error] Failed to load settings due to unknown error!" << std::endl;
    }

    // Error
    return false;
}
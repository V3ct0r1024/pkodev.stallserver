[![logo-background](https://user-images.githubusercontent.com/3164064/163711104-29410e0d-3c86-411a-9319-9ffeaa62abb8.png)](http://pkodev.net "PKOdev.NET")

## PKOdev.NET Stall Server
Stall Server project for TOP/PKO/KOP game for the implementation of a system of offline stalls.

## Features
* Offline stalls system;
* Limiting the number of offline stalls from one IP address;
* Limiting the trading time in an offline stall;
* Automatic disconnection of the account from the server when a player tries to enter his account while trading in an offline stall;
* Notification of players in the chat that a player trades in an offline stall (in PM);
* Prevention of SQL-injections in login and PIN create (change) packets from game client;
* Setting for the maximum number of connections from the same IP address and the interval between connections.

## Warning
The application is currently under development. The application has not been fully tested and is not stable. This means that errors, bugs and critical vulnerabilities may be present. Use it for testing purposes only!

## to-do
* ~~Translate comments in files **Server.h** and **Server.cpp** to English language;~~
* ~~Fix application crash when processing packets;~~
* ~~Fix application crash on startup when local port is closed;~~
* ~~Fix bridge hanging when blocking packets on enabled encryption in GateServer.cfg~~ (thanks to @small666 for finding the bug);
* ~~Make thread synchronization when processing packets;~~
* ~~Close the offline stall if it is empty (sold out);~~
* ~~Modification of GateServer.exe to determine the IP addresses of clients that are behind the server of offline stalls. At this point in the logs and database, the IP addresses of all clients will be written as **127.0.0.1** (if both GateServer.exe and pkodev.stallserver.exe are running on the same machine);~~
* Compatibility with Corsairs Online (CO) source code.

## Building and running
1. Clone the repository with the project to your disk;
2. Open the solution file **pkodev.stallserver.sln** in **Visual Studio 2022 Community**;
3. Build the solution. The server executables will appear in the **bin** folder;
4. Place the configuration file **pkodev.stallserver.cfg** from the **cfg** folder in the same directory as the server executable file **pkodev.stallserver.exe**;
5. Customize the configuration file **pkodev.stallserver.cfg** (the file is well commented);
6. To connect the game client (Game.exe) to the offline stall server, you need to install the **[pkodev.mod.stallserver](https://pkodev.net/topic/5758-connecting-gameexe-to-stall-server-offline-stalls-server-connector/)** mod;
7. To connect the offline stall server to GateServer.exe, the GateServer should be without any modifications, for example, from the **[PKO 1.38 server files](https://pkodev.net/topic/206-pirate-king-online-138/)**;
8. Run offline stall server executable **pkodev.stallserver.exe**.

## Discussion on the forum
- [English section](https://pkodev.net/topic/6068-offline-stalls-system/)
- [Russian section](https://pkodev.net/topic/6067-%D1%81%D0%B8%D1%81%D1%82%D0%B5%D0%BC%D0%B0-%D0%BE%D1%84%D1%84%D0%BB%D0%B0%D0%B9%D0%BD-%D0%BB%D0%B0%D1%80%D1%8C%D0%BA%D0%BE%D0%B2/)

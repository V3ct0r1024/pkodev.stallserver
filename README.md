[![PKOdev.NET](https://a.radikal.ru/a07/2202/b6/64077957c2ec.png "PKOdev.NET")](http://pkodev.net "PKOdev.NET")

## PKOdev.NET Stall Server
Stall Server project for TOP/PKO/KOP game for the implementation of a system of offline stalls.

## Features
* Offline stalls system;
* Limiting the number of offline stalls from one IP address;
* Limiting the trading time in an offline stall;
* Automatic disconnection of the account from the server when a player tries to enter his account while trading in an offline stall;
* Notification of players in the chat that a player trades in an offline stall;
* Prevention of SQL-injections in login and PIN create (change) packets from game client;
* Setting for the maximum number of connections from the same IP address and the interval between connections.

## Warning
The application is currently under development. The application has not been fully tested and is not stable. This means that errors, bugs and critical vulnerabilities may be present. Use it for testing purposes only!

## to-do
* Translate comments in files **Server.h** and **Server.cpp** to English language;
* Fix application crash when processing packets;
* Fix application crash on startup when local port is closed;
* Make thread synchronization when processing packets;
* Close the offline stall if it is empty;
* Compatibility with Corsairs Online (CO) source code.

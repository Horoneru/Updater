# Updater
A very basic and lightweight updater used to update an application's executable very simply. It uses simple QMessageBox to guide the users throughout all the steps.
It can be used if your application isn't complex and only updates its executable each iteration

**Warning : This project is pretty much unmaintained. You can use it as a basis to understand how to do a simple updater, but it will most likely stay as is.**

## Downloads

The updater will refuse to execute if it's called standalone without any arguments. So to use it for your projects you must compile it and adapt it to your project.

## Features

 - Already translated in <b>French</b> and <b>English</b>
 - Won't start if the users tried to execute it standalone
 - Lightweight (executable is 200 ko in size, only need QNetwork.dll and bearer plugins to work)
 - Error handling (gives the user an insight of what happened).
 - Progress bars throughout all network requests (because the user wants to know when something is loading, and if it's finished soon)
 - Simple system
 
## How to use in my project

Please have a look at the [UpdaterHandler](https://github.com/Horoneru/neuPlayer/blob/master/updaterhandler.cpp) class.

## How to compile

If you want to compile you just need [Qt](https://www.qt.io/) (version 5.2.0 or above is required)

To compile it, run `qmake` and build
 
## License
**This software is licensed under GNU GPL version 3.**
You can find the full text of the license [here](LICENSE.txt).

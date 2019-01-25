# logger-Bash
This is a project made during the second year of university at Trento.<br />
It consists in a logger that tracks the activities of a bash Linux.<br />
All those activities are written down in a log file. 

## How to use it
* download the project
* put the SO folder in the diretory you want to use it
* open bash and write *make build*
* set the default log file
* start using the bash as always

## Options available
when you build the project you have different options
* The first time you build it you need to set the default log file. To do so, specify the name of the file via the *--logfile | --l* option and his extention via the *--format | --f* option. Those 2 values will be used from now on as the default ones. The extensions supported are .csv and .txt. You can always specify a new log file every time you execute a command using those options again.
* If you want to recorder only simple statistics on the logger just begin to use your commands, instead, if you want to also record their outputs specify it with the option *--output | --o*.
* The logger is capable to understand complex commands written with ||, &&, () and >. It is capable to divide them in single commands and register the activities of each one of them in the log file.
* The logger is not designed to work with commands that open different windows or bashes. However, it works with multiple bashes at the same time.

## Group Members
* [Mattia Molon](https://github.com/MattiaMolon)
* [Giorgio Segalla](https://github.com/GiorgioSgl)
* [Leonardo Remondini](https://github.com/leonardoremondini)
* [Lorenzo Framba](https://github.com/lorenzoFramba)

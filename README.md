# QSpeakers
Loudspeaker enclosure designer, open-source, portable, written in c++/Qt, for Linux and some other platforms.
Note that stable versions of QSpeakers are officially in Debian GNU/Linux and Ubuntu Linux.

![screenshot](http://brouits.free.fr/qspeakers/qspeakers.png "QSpeakers Main Window")

## Pre-requisites
Install Qt5 or Qt6 (and QtCharts) libraries and development files, and translation tools.
In Debian or Ubuntu, for Qt6, this should install all the needed requirements:
```
sudo apt install qmake6 qt6-base-dev qt6-charts-dev qt6-tools-dev-tools
```

## Build
You can use QtCreator to build the program. You can also use `qmake6` in commandline:
```
qmake6 -config release && make
sudo make install
```

## Usage
In very short: by clicking on the QSpeakers icon in your preferred menu, the application shows up. Select a loudspeaker or enter a new one by filling its **Thiele/Small** mechanical parameters. Click on *optimize* and you're done: the best enclosure volume is calculated for you.

## Homepage
QSpeakers homepage with source releases and Microsoft® Windows® installer is currently hosted on [my old website](http://brouits.free.fr/qspeakers/)

## Thanks
Thanks to *Upacesky* for his [free svg drawing](https://openclipart.org/detail/202508/studio-monitoring-loudspeaker)

## Todo
- more plots: excursion, phase and impendance
- implement dual-free ports and dual-flanged ports in a combobox option
- implement crossover filters

## Formulæ
See INFO file.

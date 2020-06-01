# QSpeakers
loudspeaker enclosure designer, open-source, portable, written in c++/Qt, for Linux and some other platforms.

## Pre-requisites
Install Qt5 and QtCharts libraries and development files.

## Build
You can use QtCreator to build the program. You can also use `qmake` in commandline:
```
qmake -config release && make
sudo make install
```

## Usage
In very short: by clicking on the QSpeakers icon in your preferred menu, the application shows up. Select a loudspeaker or enter a new one by filling its Thiele/Small mechanical parameters. Click on *optimize* and you're done: the best enclosure volume is calculated for you.

## Homepage
QSpeakers homepage with source releases and Microsoft® Windows® installer here:
http://brouits.free.fr/qspeakers/

## Thanks
Thanks to *Upacesky* for his free drawing: https://openclipart.org/detail/202508/

## Todo
- more plots: excursion, phase and impendance
- implement dual-free ports and dual-flanged ports in a combobox option
- implement crossover filters

## Formulæ
See INFO file.

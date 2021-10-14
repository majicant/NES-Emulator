# NES Emulator
An instruction-accurate NES emulator written in C++ using SDL2. The emulator uses NTSC timing and
supports the following mappers for games:
* NROM
* MMC1
* UxROM
* MMC3

## Screenshots
<img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/donkey%20kong.png" width="400"> <img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/ice%20climber.png" width="400">

<img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/kirby.png" width="400"> <img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/mario%203.png" width="400">

<img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/mario.png" width="400"> <img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/mega%20man%202.png" width="400">

<img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/mega%20man%204.png" width="400"> <img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/metroid.png" width="400">

<img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/zelda%202.png" width="400"> <img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/zelda.png" width="400">

## Usage
Change the current working directory to the location of the executable. Then execute
```
> "NES Emulator" "C:/.../game.nes"
```

## Controls
The controls are mapped for player 1 as follows.

 NES        | NES Emulator
 -----------|-------------
Up          | W
Down        | S
Left        | A
Right       | D
Start       | X
Select      | Z
A           | L
B           | K

## TODO
* Implement the APU
* Add a GUI/Debugger

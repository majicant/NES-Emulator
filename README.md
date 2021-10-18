# NES Emulator
An instruction-accurate NES emulator written in C++ using SDL2. The emulator uses NTSC timing and
supports the following mappers for games:
* NROM
* MMC1
* UxROM
* MMC3

Note that the emulator is synced to the monitors refresh rate and thus requires a 60hz display for games
to run at a normal speed.

## Screenshots
<img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/donkey%20kong.png" width="400"> <img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/ice%20climber.png" width="400">

<img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/kirby.png" width="400"> <img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/mario%203.png" width="400">

<img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/mario.png" width="400"> <img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/mega%20man%202.png" width="400">

<img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/mega%20man%204.png" width="400"> <img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/metroid.png" width="400">

<img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/zelda%202.png" width="400"> <img src="https://github.com/majicant/NES-Emulator/blob/master/screenshots/zelda.png" width="400">

## Usage
1. Run ```git clone https://github.com/majicant/NES-Emulator```
2. Open the project in Visual Studio and build in Release for x86.
3. Move the dll in ```ext/SDL2/lib/x86``` into ```bin/Win32/Release```
4. Change the current working directory to the location of the executable. Then execute
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

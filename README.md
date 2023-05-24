# Strayed Lights Fix
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/W7W01UAI9)</br>
[![Github All Releases](https://img.shields.io/github/downloads/Lyall/StrayedLightsFix/total.svg)](https://github.com/Lyall/StrayedLightsFix/releases)

This is a fix to remove pillarboxing/letterboxing in Strayed Lights.

## Features
- Removes pillarboxing/letterboxing in gameplay and cutscenes.
- Correct FOV scaling no matter what resolution you use.

## Installation
- Grab the latest release of StrayedLightsFix from [here.](https://github.com/Lyall/StrayedLightsFix/releases)
- Extract the contents of the release zip in to the the game folder. (e.g. "**C:\Games\StrayedLights**").

## Testing Notes
- Tested on the Epic Store version at 32:9 and 21:9.

## Configuration
- See **StrayedLightsFix.ini** to adjust settings for the fix.

### Linux/Steam Deck
- **Linux/Steam Deck only** Make sure you set the Steam launch options to `WINEDLLOVERRIDES="winmm.dll=n,b" %command%`

## Known Issues
Please report any issues you see.

## Screenshots

| ![ezgif-4-3258b12cf4](https://github.com/Lyall/StrayedLightsFix/assets/695941/e3b5a228-463a-44db-be41-72aaf76b097c) |
|:--:|
| Disabled pillarboxing/letterboxing in gameplay. |

## Credits

[Flawless Widescreen](https://www.flawlesswidescreen.org/) for the LOD fix.<br />
[Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) for ASI loading. <br />
[inipp](https://github.com/mcmtroffaes/inipp) for ini reading. <br />
[Loguru](https://github.com/emilk/loguru) for logging. <br />
[length-disassembler](https://github.com/Nomade040/length-disassembler) for length disassembly.

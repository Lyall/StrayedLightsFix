# Strayed Lights Fix
[![Patreon-Button](https://github.com/Lyall/StrayedLightsFix/assets/695941/1d864848-b072-4ca0-abc1-ed4a45082f58)](https://www.patreon.com/Wintermance) [![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/W7W01UAI9)<br />
[![Github All Releases](https://img.shields.io/github/downloads/Lyall/StrayedLightsFix/total.svg)](https://github.com/Lyall/StrayedLightsFix/releases)

This is a fix to remove pillarboxing/letterboxing in Strayed Lights.

## Features
- Removes pillarboxing/letterboxing in gameplay and cutscenes.
- Correct FOV scaling no matter what resolution you use.

## Installation
- Grab the latest release of StrayedLightsFix from [here.](https://github.com/Lyall/StrayedLightsFix/releases)
- Extract the contents of the release zip in to the the game folder. (e.g. "**C:\Games\StrayedLights**").

## Configuration
- See **StrayedLightsFix.ini** to adjust settings for the fix.

### Steam Deck/Linux Additional Instructions
🚩**You do not need to do this if you are using Windows!**
- Open up the game properties in Steam and add `WINEDLLOVERRIDES="winmm=n,b" %command%` to the launch options.

## Known Issues
Please report any issues you see.

## Screenshots

| ![ezgif-4-3258b12cf4](https://github.com/Lyall/StrayedLightsFix/assets/695941/e3b5a228-463a-44db-be41-72aaf76b097c) |
|:--:|
| Disabled pillarboxing/letterboxing in gameplay. |

## Credits
[ImaHappy](https://www.twitch.tv/imahappy) for providing a copy of the game to create this fix.<br />
[Flawless Widescreen](https://www.flawlesswidescreen.org/) for the LOD fix.<br />
[Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) for ASI loading. <br />
[inipp](https://github.com/mcmtroffaes/inipp) for ini reading. <br />
[spdlog](https://github.com/gabime/spdlog) for logging. <br />
[safetyhook](https://github.com/cursey/safetyhook) for hooking.

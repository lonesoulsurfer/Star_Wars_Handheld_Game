# Star Wars: arcade game for the Game of Life handheld

This is a Star Wars arcade game for my [Game of Life handheld](https://github.com/lonesoulsurfer/Conways_Game_of_life_Handheld). You fly the Battle of Yavin from *A New Hope*: training stages, a space battle and the trench run to the exhaust port. The game uses vector graphics in the style of the 1983 Atari arcade game.

> **Note:** Star Wars is a trademark of Lucasfilm Ltd. This is a non-commercial fan project. It is not affiliated with or endorsed by Lucasfilm or Disney.

## Build guide and video

- The hardware is the Game of Life handheld. Its repository has all the files to build it: [Conways_Game_of_life_Handheld](https://github.com/lonesoulsurfer/Conways_Game_of_life_Handheld).
- Instructables and YouTube: not available yet.

## Features

- 14 stages, with short animated scenes between them
- Womp rat training, blindfold training, space battle, Death Star surface run, trench run and the exhaust port shot
- 3 types of enemy ship, each with a different movement and weapon
- Power-ups, a health bar and a score

The [user guide](docs/MANUAL.md) describes each stage, the scores and the controls.

## What you need

- A [Game of Life handheld](https://github.com/lonesoulsurfer/Conways_Game_of_life_Handheld), or the same parts:
  - Adafruit Trinket M0
  - SSD1306 128 × 64 OLED display (I2C)
  - A resistor-ladder button pad on pin A3
- Arduino IDE

## Upload the game

1. In the Arduino IDE, click **File → Preferences**.
2. In **Additional boards manager URLs**, add: `https://adafruit.github.io/arduino-board-index/package_adafruit_index.json`
3. Open the **Boards Manager**. Search for **Adafruit SAMD Boards** and install it.
4. Open the **Library Manager**. Install **Adafruit GFX Library** and **Adafruit SSD1306**.
5. Download this repository. Click **Code → Download ZIP**, then extract the ZIP file.
6. Open `Star_Wars_V1/Star_Wars_V1.ino`.
7. Connect the Trinket M0 to your computer with a USB cable.
8. In the **Tools** menu, set **Board** to **Adafruit Trinket M0**, and set **Port** to the port of the Trinket.
9. Click **Upload**.

## Controls

| Button | Action |
|--------|--------|
| SET (A) | Fire, or select. Hold for rapid fire. |
| UP, DOWN, LEFT, RIGHT | Move the crosshair or the ship |
| B | Other action |

## Credits

- Game idea: the 1983 Atari *Star Wars* arcade game.
- Code: Marcus ([lonesoulsurfer](https://github.com/lonesoulsurfer)).

## Support

Do you like this project? You can support my work on [GitHub Sponsors](https://github.com/sponsors/lonesoulsurfer).

## Licence

The code uses the [MIT License](LICENSE). Copyright (c) 2025 Marcus (lonesoulsurfer).

The MIT License covers the code only. It gives no rights to the Star Wars name or characters.

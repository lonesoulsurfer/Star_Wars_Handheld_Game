Star Wars:  - A Complete Arcade Experience for my handheld game, originally designed for Conways Game of Life.

The original GitHub page can be found below which has all of the files needed to make your own handheld game console

https://github.com/lonesoulsurfer/Conways_Game_of_life_Handheld


 
Overview

Experience the iconic Battle of Yavin in this comprehensive Star Wars arcade game for Arduino! This game recreates the climactic Death Star assault from A New Hope, featuring multiple training sequences, intense space combat, and the legendary trench run finale.



Hardware Requirements

      Microcontroller: Adafruit Trinket M0

      Display: 128x64 OLED display (SSD1306, I2C)
      
      Controls: Analog resistor ladder button pad on pin A3




Libraries Required:

      Wire.h

      Adafruit_GFX.h

      Adafruit_SSD1306.h



Button Configuration

The game uses a resistor ladder on analog pin A3 with the following button mappings:

      SET (A Button): Fire/Select (analog value 200-300)

      UP: Move crosshair/ship up (analog value 38-40)

      DOWN: Move crosshair/ship down (analog value 44-47)

      LEFT: Move crosshair/ship left (analog value 74-75)

      RIGHT: Move crosshair/ship right (analog value 141-143)

      B Button: Alternative action (analog value 1020-1023)


Game Stages

      1. Tatooine Sunset (Cinematic)
      Watch the twin suns set over Luke's moisture farm in this atmospheric 8-second opening scene featuring desert landscapes and Luke's iconic dome home.


      2. Womp Rat Training

      Objective: Destroy womp rats in Beggar's Canyon using your T-16 Skyhopper

      Goal: Score 200 points to complete training


Features:

      Dynamic canyon walls with perspective scrolling

      Smart AI - womp rats evade your crosshair

      Animated rock obstacles embedded in canyon floor

      Realistic canyon terrain with depth and detail



3. Force Lesson
   
      Brief story interlude where Obi-Wan teaches Luke about the Force before lightsaber training.


4. Blindfold Training (Jedi Training)

      Objective: Use the Force to deflect training remote blasts
   
      Mechanics:

         Training remote moves in complex patterns (circles, dives, zigzags, random)

         Warning system: Remote flashes before firing

         Warning time decreases as you progress (3s → 2s → 1.5s → 1s)

         Shoot the remote during its warning phase to deflect

         Take damage if you miss or don't shoot in time

         Shrinking danger zone adds pressure over time

         Goal: Score 200 points in this stage (relative scoring)

         Reward: +20% health bonus upon completion



5. Space Battle

      Objective: Destroy TIE Fighters approaching the Death Star

          Enemy Types:

         TIE Fighters: Basic enemies with single-shot weapons

          TIE Interceptors: Fast, weaving movement with burst fire

         TIE Bombers: Slow but durable with spread-shot weapons


      Features:

            Vector-style wireframe graphics (authentic arcade look)

            Enemy AI with different movement patterns and firing behaviors
   
            Power-ups dropped by destroyed enemies (shields, rapid fire)

            Background starfield with motion blur effects

            All enemies require 2 hits to destroy

            Goal: Score 500 points to proceed to Death Star approach

   

7. Death Star Approach (Cinematic)

      Dramatic flyby as the Death Star grows larger on screen, showing its massive scale and the infamous trench.


7. Death Star Surface Run

      Objective: Destroy surface towers and gun turrets


         Features:

         Banking mechanic - your ship tilts based on crosshair movement

         Progressive difficulty - tower spawn rate increases with score

         Background dogfights with X-wings and TIEs

         Defensive turrets that track and fire at you

         Collision damage from towers and turret fire

         Goal: Score 500 points to reach trench entry


8. Trench Entry (Cinematic)
   
      5-second dramatic sequence showing your X-wing diving into the Death Star trench with motion lines and speed effects.


9. Trench Run

      Objective: Navigate the narrow trench while destroying turrets

         Features:

         First-person perspective from behind your X-wing

         Vector-style 3D turrets with targeting AI

         Turrets require 2 hits to destroy

         Progressive speed increase

         Turrets fire at your position

         Authentic Star Wars arcade turret design (tapered towers with gun housings)

         Goal: Score 500 points to reach exhaust port



10. "Use the Force" (Cinematic)

      7-second scene featuring detailed front-view X-wing approaching camera with "USE THE FORCE" message.



11. Exhaust Port Targeting

      Objective: Fire proton torpedoes into the 2-meter exhaust port

         Mechanics:

         6-second time limit (visible countdown)

         Precise targeting required - crosshair must be within 3 pixels of center

         Multiple shot attempts allowed within time limit

         Port flashes intensely when perfectly aligned

         Miss the timing and return to trench run

         Success: Triggers missile shaft sequence



12. Missile Shaft (Cinematic)

      Watch your proton torpedoes travel down the reactor shaft toward the Death Star's core with depth perspective and animated missile.



13. Death Star Explosion (Cinematic)
    
    Epic 4-second multi-phase explosion:

      Warning cracks appear across the Death Star

      Bright core flash

      Multiple expanding shockwave rings

      Debris and energy flares scatter outward

      Chaotic destruction effects



14. Victory!

      Congratulations screen with final score.




Scoring System

      Womp Rats: 10 points each

      TIE Fighters: 10 points each

      TIE Interceptors: 20 points each

      TIE Bombers: 30 points each

      Trench Turrets: 5 points per hit, 20 points when destroyed

      Surface Towers: 20 points each

      Power-ups: 20 points each

      Exhaust Port Hit: 500 points bonus

      Health Restoration: Automatic +5% health every 200 points (all stages)



Health System

      Starting health: 100%

      Health bar display with percentage

      Damage varies by source (5-10% per hit)

      1-second invincibility after taking damage (flashing border indicator)

      Screen flash effect when damaged

      Special Health Bonuses:

      +20% for completing Jedi training

      +5% every 200 points throughout game



Game Features

      Graphics Engine

      Vector Graphics: Authentic 1983 arcade-style wireframe rendering

      3D Perspective: True depth calculation for enemies and obstacles

      Particle Effects: Explosions with expanding debris

      Starfield: Dynamic parallax scrolling with motion trails

      Cockpit View: Detailed X-wing canopy frame (tilts during Death Star surface)



Enemy AI

      Adaptive Movement: Different patterns per enemy type

      Predictive Firing: Enemies calculate trajectory to hit your position

      Multiple Weapon Types: Single shot, burst fire, spread shot

      Collision Detection: Precise hit detection for all objects



Audio-Visual Polish

      Crosshair Flash: Visual feedback for hits/misses

      Screen Shake: Damage impact effects

      Animated Explosions: Multi-frame debris scatter

      Smooth Transitions: Cinematic scene changes



Technical Highlights

      Memory Optimized: Runs smoothly on Adafruit Trinket M0

      60 FPS Target: 20 FPS stable performance (50ms frame time)

      Structured Code: Clean state machine architecture

      Collision System: Efficient spatial checking

      Progressive Difficulty: Dynamic spawn rates and enemy behavior




Controls Tips

      Continuous Fire: Hold SET button for rapid fire in most combat stages

      Precision Targeting: Required for exhaust port - take your time!

      Evasive Maneuvers: Keep moving to avoid enemy fire

      Power-up Collection: Fly through power-ups to collect them



Development Notes

      This game features:

      17 unique game states (including cinematics)

      8 simultaneous enemies maximum (performance optimized)

      16 simultaneous projectiles

      30-star parallax starfield

      Complex AI with 4 movement patterns (training remote)

      Full 3D perspective rendering for trench sequence

      Authentic Star Wars arcade aesthetic



Credits

      Based on the iconic 1983 Atari Star Wars arcade game, reimagined for Arduino with enhanced features and additional training sequences from the original film.

/*
 * STAR WARS: TRENCH RUN - Complete Arcade Experience
 * ===================================================
 * 
 * Relive the Battle of Yavin! This complete Star Wars game for Arduino recreates
 * the iconic Death Star assault from A New Hope with authentic vector graphics,
 * multiple training sequences, and the legendary trench run finale.
 * 
 * HARDWARE REQUIREMENTS:
 * ----------------------
 * - Adafruit Trinket M0
 * - 128x64 OLED Display (SSD1306, I2C on address 0x3C)
 * - Resistor ladder button pad on analog pin A3
 * 
 * BUTTON CONFIGURATION (Analog Pin A3):
 * -------------------------------------
 * - SET (A):   Fire/Select     (200-300)
 * - UP:        Move Up          (38-40)
 * - DOWN:      Move Down        (44-47)
 * - LEFT:      Move Left        (74-75)
 * - RIGHT:     Move Right       (141-143)
 * - B:         Alternative      (1020-1023)
 * 
 * GAME STAGES:
 * ------------
 * 1.  TATOOINE SUNSET (Cinematic) - Watch twin suns set over Luke's home
 * 2.  WOMP RAT TRAINING - Destroy 200pts of womp rats in Beggar's Canyon
 * 3.  FORCE LESSON (Story) - Obi-Wan teaches about the Force
 * 4.  BLINDFOLD TRAINING - Deflect remote blasts for 200pts (Jedi training)
 * 5.  SPACE BATTLE - Destroy TIEs for 500pts (TIE Fighters/Interceptors/Bombers)
 * 6.  DEATH STAR APPROACH (Cinematic) - Dramatic flyby of the battle station
 * 7.  DEATH STAR SURFACE - Destroy towers for 500pts (with banking mechanics)
 * 8.  TRENCH ENTRY (Cinematic) - Dive into the trench
 * 9.  TRENCH RUN - Navigate and destroy turrets for 500pts
 * 10. USE THE FORCE (Cinematic) - Obi-Wan's voice guides you
 * 11. EXHAUST PORT - Precision targeting (6-second time limit)
 * 12. MISSILE SHAFT (Cinematic) - Torpedoes travel to reactor core
 * 13. DEATH STAR EXPLOSION (Cinematic) - Epic multi-phase destruction
 * 14. VICTORY! - Mission accomplished
 * 
 * SCORING:
 * --------
 * - Womp Rats: 10pts          - TIE Fighters: 10pts
 * - TIE Interceptors: 20pts   - TIE Bombers: 30pts
 * - Trench Turrets: 20pts     - Surface Towers: 20pts
 * - Power-ups: 20pts          - Exhaust Port: 500pts
 * 
 * HEALTH SYSTEM:
 * --------------
 * - Start: 100% shields
 * - Auto-heal: +5% every 200 points (all stages)
 * - Jedi Training Bonus: +20% upon completion
 * - Invincibility: 1 second after taking damage
 * 
 * FEATURES:
 * ---------
 * - Vector-style wireframe graphics (authentic 1983 arcade aesthetic)
 * - 3D perspective rendering with depth calculation
 * - Smart AI with multiple movement patterns
 * - Progressive difficulty scaling
 * - Dynamic particle effects and explosions
 * - Parallax starfield with motion trails
 * - Banking mechanics during surface run
 * - Power-up system (shields, rapid fire)
 * - Cinematic story sequences
 * 
 * CONTROLS:
 * ---------
 * - Move crosshair/ship with directional buttons
 * - Hold SET (A) for continuous fire in combat stages
 * - Precision required for exhaust port targeting
 * - Banking controlled by horizontal movement on surface
 * 
 * TECHNICAL SPECS:
 * ----------------
 * - Performance: 20 FPS stable (50ms frame time)
 * - Max Enemies: 8 simultaneous (optimized for Arduino Uno)
 * - Projectiles: 16 simultaneous
 * - Starfield: 30 parallax stars
 * - Memory: Optimized for 2KB RAM
 * 
 * TIPS:
 * -----
 * - In Womp Rat Training: Rats evade your crosshair - lead your shots!
 * - In Blindfold Training: Wait for remote to flash (warning), then shoot
 * - In Space Battle: Keep moving to avoid enemy fire
 * - In Trench Run: Turrets need 2 hits to destroy
 * - In Exhaust Port: Take your time - you have 6 seconds and multiple shots
 * 
 * May the Force be with you, pilot!
 * 
 * Based on the 1983 Atari Star Wars arcade game
 * Reimagined for Arduino with enhanced features
 * 
 * Version: 1.0
 * Author: Lonesoulsurfer https://github.com/lonesoulsurfer
 * Date: Oct 2025
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button input pin using resistor ladder
#define BUTTON_INPUT_PIN A3

// Game states
  enum GameState {
  TITLE_SCREEN,
  WOMP_RAT_TRAINING,        
  TRAINING_COMPLETE, 
  FORCE_LESSON,
  BLINDFOLD_TRAINING,
  TRAINING_COMPLETE_2,
  TATOOINE_SUNSET,    
  SPACE_BATTLE,
  DEATH_STAR_APPROACH,
  DEATH_STAR_SURFACE,
  TRENCH_ENTRY,
  TRENCH_RUN,
  USE_THE_FORCE,
  EXHAUST_PORT,
  DEATH_STAR_EXPLOSION,
  VICTORY,
  GAME_OVER
};

GameState currentState = TITLE_SCREEN;

// Movement pattern types
enum MovementPattern {
  PATTERN_CIRCLE,
  PATTERN_DIVE,
  PATTERN_ZIGZAG,
  PATTERN_RANDOM
};

// Player variables
float playerX = 64.0, playerY = 50.0;
float crosshairX = 64.0, crosshairY = 32.0;
int score = 0;
int shields = 100;
int stageStartScore = 0;  // Score when current stage started

// Crosshair flash effect
unsigned long flashCrosshair = 0;
bool hitFlash = false;  // true for hit, false for miss
unsigned long damageFlash = 0;  // Screen shake when taking damage

// Game timing
unsigned long gameTimer = 0;
unsigned long lastUpdate = 0;
unsigned long stateTimer = 0;

// Advanced enemy system
struct Enemy {
  float x, y, z;
  float vx, vy, vz;
  float angle;
  int type; // 0=TIE, 1=Interceptor, 2=Bomber
  bool active;
  int health;
  unsigned long lastFire;
  int weaponType; // 0=single, 1=burst, 2=spread
};

Enemy enemies[8];
int enemyCount = 0;

// Advanced projectile system
struct Projectile {
  float x, y, z;
  float vx, vy, vz;
  bool active;
  bool isPlayerShot;
  int damage;
  unsigned long birthTime;
};

Projectile projectiles[16];

// Starfield for vector effects
struct Star {
  float x, y, z;
  float speed;
};

Star stars[30];

// Trench run variables
struct TrenchWall {
  float leftX, rightX, y, z;
  bool hasObstacle;
  float obstacleX;
  int turretHealth;
};

TrenchWall trenchSegments[6];
float trenchSpeed = 2.0;

// Vector drawing effects
struct VectorLine {
  float x1, y1, x2, y2;
  unsigned long fadeTime;
  bool active;
};

VectorLine vectorTrails[20];

// Explosion system
struct Explosion {
  float x, y;
  int frame;
  bool active;
};

Explosion explosions[5];

// Button handling
unsigned long lastButtonPress = 0;
unsigned long lastFirePress = 0;
const int DEBOUNCE_DELAY = 50;  // Reduced general debounce
const int FIRE_DEBOUNCE_DELAY = 80;  // Continuous fire rate (10 shots/second)

// Death Star approach variables
float deathStarDistance = 500.0;
bool inTrenchApproach = false;

// Trench turrets
struct Turret {
  float x, y, z;
  bool active;
  unsigned long lastFire;
  int health;
};

Turret turrets[4];

// Power-ups
struct PowerUp {
  float x, y, z;
  float vx, vy; // Velocity for floating movement
  int type; // 0=shield, 1=rapid fire, 2=extra life
  bool active;
  unsigned long spawnTime;
};

PowerUp powerUps[3];

// Level system
int currentLevel = 1;
int enemiesKilled = 0;
bool rapidFire = false;
unsigned long rapidFireEnd = 0;
unsigned long showHealthBonus = 0;
unsigned long exhaustPortStartTime = 0;
unsigned long lastDamageTime = 0;
const unsigned long INVINCIBILITY_TIME = 1000; // 1 second of invincibility after damage
unsigned long hitConfirmationTime = 0;
int hitConfirmationX = 0, hitConfirmationY = 0;
const unsigned long EXHAUST_PORT_TIMEOUT = 6000; // 5 seconds

// Exhaust port targeting variables
unsigned long exhaustPortTimer = 0;
bool targetingActive = false;
bool shotFired = false;
float targetAccuracy = 0.0; // How close to center when shot fired
unsigned long shotTime = 0;
bool missionSuccess = false;

// Missile shaft animation variables
bool inMissileShaft = false;
float shaftDepth = 0.0;
float missileY = 0.0;
unsigned long shaftStartTime = 0;
struct ShaftSegment {
  float z;
  float width;
  bool hasObstacle;
  float obstacleX;
};
ShaftSegment shaftSegments[8];

// Surface tower variables
struct SurfaceTower {
  float x, y, z;
  int health;
  bool active;
  int type;
  unsigned long lastHit;
};

float bankingAngle = 0.0; // Current banking angle
float targetBankingAngle = 0.0; // Target banking angle based on input
float bankingSpeed = 0.15; // How fast banking responds

SurfaceTower surfaceTowers[4];
float surfaceSpeed = 2.0;

unsigned long lastRespawn = 0;
int activeTowers = 0;

struct BackgroundShip {
  float x, y, z;
  float vx, vy, vz;
  int type; // 0=X-wing, 1=TIE
  bool active;
  unsigned long lastShot;
};

BackgroundShip backgroundShips[6];

// Womp rat training variables
struct WompRat {
  float x, y;
  float vx, vy;
  bool active;
  unsigned long lastDirection;
};

WompRat wompRats[6];
int wompRatsKilled = 0;
float canyonScroll = 0;
float t16X = 64.0;

struct CanyonRock {
  float x, y;
  int size;
  bool active;
};

CanyonRock canyonRocks[5];

// Blindfold training variables
struct TrainingRemote {
  float x, y;
  float vx, vy;
  bool active;
  unsigned long nextFireTime;
  int predictX, predictY; // Where remote will fire from
  bool showPrediction;
  unsigned long predictionTime;
  MovementPattern currentPattern;
  unsigned long patternStartTime;
  float patternSpeed;
  float centerX, centerY;
  float angle;
  int diveDirection;
  int zigzagDirection;
};

TrainingRemote trainingRemote;
int deflectionsSuccessful = 0;
int deflectionsMissed = 0;
unsigned long lastDeflectionAttempt = 0;
bool blindfoldActive = false;
float dangerZoneRadius = 50.0;
float dangerZoneMinRadius = 20.0;
float dangerZoneShrinkRate = 0.01;

// Death Star explosion variables
unsigned long explosionStartTime = 0;

// Function prototypes
void handleInput();
void updateGame();
void drawGame();
void initializeStars();
void updateStars();
void drawStars();
void spawnEnemy();
void updateEnemies();
void drawEnemies();
void fireProjectile(float x, float y, float vx, float vy, bool isPlayer);
void updateProjectiles();
void drawProjectiles();
void drawVectorEffect(float x1, float y1, float x2, float y2);
void updateVectorTrails();
void drawVectorTrails();
void initializeTrench();
void updateTrench();
void drawTrench();
bool checkCollisions();
void drawUI();
void drawTitleScreen();
void drawGameOver();
void drawVictory();
int readButtonValue();
bool isBtnUp(int val);
bool isBtnDown(int val);
bool isBtnLeft(int val);
bool isBtnRight(int val);
bool isBtnSet(int val);
bool isBtnB(int val);
void drawVectorTIE(float x, float y, float scale, float angle);
void drawVectorInterceptor(float x, float y, float scale, float angle);
void drawVectorBomber(float x, float y, float scale, float angle);
void drawSpaceBattlePlayer();
void drawTrenchPlayer();
void drawExhaustPort();
void drawTargeting();
void createExplosion(float x, float y);
void updateExplosions();
void drawExplosions(); 
void drawCockpitWindow();
void spawnPowerUp(float x, float y, int type);
void updatePowerUps();
void drawPowerUps();
void drawDeathStarApproach();
void drawTrenchEntry();
void checkTurretFiring(int segmentIndex, float turretX, float turretY, float scale);
void handleExhaustPortFiring();
void drawUseTheForce();
void initializeShaft();
void drawMissileShaft();
void drawDeathStarExplosion();
void initializeSurfaceTowers();
void updateSurfaceTowers();
void drawSurfaceTowers();
void drawDeathStarSurface();
void handleFireButton();
void drawWompRatTraining();
void updateWompRatTraining();
void drawTrainingComplete();
void drawForceLesson();
void drawTrainingComplete2();
void initializeBlindfolding();
void initializeWompRats();
void initializeCanyonRocks();
void drawTatooineSunset();
void initializeBlindfolding();
void drawBlindfolding();
void updateBlindfolding();
void setup() {
  Serial.begin(9600); 
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Display failed!");
    while(1);
  }
  
  // Animated title sequence
  for(int size = 1; size <= 20; size++) {
    display.clearDisplay();
    
    // Calculate text size and position for centering
    int textSize = min(size / 3, 3);  // Max size of 3
    if(textSize < 1) textSize = 1;
    
    display.setTextSize(textSize);
    display.setTextColor(SSD1306_WHITE);
    
    // Center "STAR"
    int starWidth = textSize * 6 * 4;  // 4 characters * 6 pixels * size
    int starX = (128 - starWidth) / 2;
    int starY = 32 - (textSize * 8);   // 8 pixels height * size
    
    display.setCursor(starX, starY);
    display.println("STAR");
    
    // Center "WARS" 
    int warsWidth = textSize * 6 * 4;  // 4 characters * 6 pixels * size
    int warsX = (128 - warsWidth) / 2;
    int warsY = 32 + 2;
    
    display.setCursor(warsX, warsY);
    display.println("WARS");
    
    display.display();
    delay(80);  // Animation speed
  }
  
  delay(1000);  // Hold final size
  
  // Initialize game systems
  initializeStars();
  initializeTrench();
  initializeSurfaceTowers();
  
  // Clear all arrays
  for(int i = 0; i < 8; i++) {
    enemies[i].active = false;
  }
  for(int i = 0; i < 16; i++) {
    projectiles[i].active = false;
  }
  for(int i = 0; i < 20; i++) {
    vectorTrails[i].active = false;
  }
  for(int i = 0; i < 5; i++) {
    explosions[i].active = false;
  }
  gameTimer = millis();
  randomSeed(analogRead(A0));
}

void loop() {
  unsigned long currentMillis = millis();
  if(currentMillis - lastUpdate < 50) return; // 20 FPS
  lastUpdate = currentMillis;

  handleInput();
  updateGame();
  drawGame();
}

void handleInput() {
  int btnValue = readButtonValue();
  
  // Store current and previous button states
  static bool firePressed = false;
  static bool prevFirePressed = false;
  static bool leftPressed = false;
  static bool rightPressed = false;
  static bool upPressed = false;
  static bool downPressed = false;
  
  // Update button states based on current reading
  prevFirePressed = firePressed;
  firePressed = isBtnSet(btnValue);
  leftPressed = isBtnLeft(btnValue);
  rightPressed = isBtnRight(btnValue);
  upPressed = isBtnUp(btnValue);
  downPressed = isBtnDown(btnValue);
  
  // Handle fire button - continuous firing when held, or single press for title/game over
  if(firePressed && millis() - lastFirePress > FIRE_DEBOUNCE_DELAY && 
     (currentState == WOMP_RAT_TRAINING || currentState == SPACE_BATTLE || currentState == TRENCH_RUN || currentState == EXHAUST_PORT || !prevFirePressed)) {
    lastFirePress = millis();
    handleFireButton();
  }
  
  if(currentState == TITLE_SCREEN) {
    if(firePressed) {
      currentState = TATOOINE_SUNSET;
      stateTimer = millis();
      lastButtonPress = millis();
    }
    return;
  }
  
  if(currentState == FORCE_LESSON) {
    if(firePressed && millis() - stateTimer > 1000) {
      currentState = BLINDFOLD_TRAINING;
      stateTimer = millis();
      stageStartScore = score;
      initializeBlindfolding();
      lastButtonPress = millis();
    }
    return;
  }
  
  if(currentState == TRAINING_COMPLETE_2) {
  if(firePressed && millis() - stateTimer > 2000) {  // Changed from 500 to 2000
    currentState = SPACE_BATTLE;
    stateTimer = millis();
    stageStartScore = score;
    lastButtonPress = millis();
  }
  return;
}
  if(currentState == GAME_OVER || currentState == VICTORY) {
    if(firePressed && millis() - lastButtonPress > 500) {  // Add 500ms debounce
      // Reset game
      currentState = TITLE_SCREEN;
      score = 0;
      shields = 100;
      playerX = 64; playerY = 50;
      crosshairX = 64; crosshairY = 32;
      currentLevel = 1;
      enemiesKilled = 0;
      inMissileShaft = false;
      missionSuccess = false;
      shotFired = false;
      targetingActive = false;
      deathStarDistance = 500.0;
      lastButtonPress = millis();
    }
    return;
  }
  
   // Movement controls - independent debounce for each direction
  static unsigned long lastLeft = 0, lastRight = 0, lastUp = 0, lastDown = 0;
 if(currentState == WOMP_RAT_TRAINING || currentState == BLINDFOLD_TRAINING || currentState == SPACE_BATTLE || currentState == DEATH_STAR_SURFACE || currentState == TRENCH_RUN || currentState == EXHAUST_PORT) {
    if(leftPressed && millis() - lastLeft > 20) {
    if(currentState == WOMP_RAT_TRAINING || currentState == BLINDFOLD_TRAINING || currentState == SPACE_BATTLE || currentState == DEATH_STAR_SURFACE || currentState == EXHAUST_PORT) {
    crosshairX -= 6;
      if(crosshairX < 10) crosshairX = 10;
      } else if(currentState == EXHAUST_PORT) {
      crosshairX -= 2;
      if(crosshairX < 10) crosshairX = 10;
    } else {
      playerX -= 5;
      if(playerX < 15) playerX = 15;
    }
    lastLeft = millis();
  }
  
  if(rightPressed && millis() - lastRight > 20) {
    if(currentState == WOMP_RAT_TRAINING || currentState == BLINDFOLD_TRAINING || currentState == SPACE_BATTLE || currentState == DEATH_STAR_SURFACE || currentState == EXHAUST_PORT) {
      crosshairX += 6;  // CHANGED -= to +=
      if(crosshairX > 118) crosshairX = 118;
      } else if(currentState == EXHAUST_PORT) {
      crosshairX += 2;
      if(crosshairX > 118) crosshairX = 118;
    } else {
      playerX += 5;
      if(playerX > 113) playerX = 113;
    }
    lastRight = millis();
  }

  if(currentState == DEATH_STAR_SURFACE) {
  // Calculate target banking angle based on crosshair movement
  static float lastCrosshairX = crosshairX;
  float crosshairMovement = crosshairX - lastCrosshairX;
  
  // Update target banking angle (-25 to +25 degrees max)
  targetBankingAngle = constrain(crosshairMovement * 5, -25, 25);
  
  lastCrosshairX = crosshairX;
}
  
  if(downPressed && millis() - lastDown > 20) {
    if(currentState == WOMP_RAT_TRAINING || currentState == BLINDFOLD_TRAINING || currentState == SPACE_BATTLE || currentState == DEATH_STAR_SURFACE || currentState == EXHAUST_PORT) {
      crosshairY += 5;  // CHANGED crosshairX to crosshairY
      if(crosshairY > 54) crosshairY = 54;
    } else if(currentState == EXHAUST_PORT) {
      crosshairY += 2;
      if(crosshairY > 54) crosshairY = 54;
    }
    lastDown = millis();
  }
  
  if(upPressed && millis() - lastUp > 20) {
    if(currentState == WOMP_RAT_TRAINING || currentState == BLINDFOLD_TRAINING || currentState == SPACE_BATTLE || currentState == DEATH_STAR_SURFACE || currentState == EXHAUST_PORT) {
      crosshairY -= 5;  // CHANGED crosshairX to crosshairY
      if(crosshairY < 10) crosshairY = 10;
    } else if(currentState == EXHAUST_PORT) {
      crosshairY -= 2;
      if(crosshairY < 10) crosshairY = 10;
    }
    lastUp = millis();
  }
  // Constrain T-16 to canyon floor boundaries in womp rat training
  if(currentState == WOMP_RAT_TRAINING) {
    int topGap = 25;
    int bottomGap = 50;
    int shipY = 50;
    
    // Calculate canyon floor boundaries at the ship's Y position
    int gapAtY = topGap + (bottomGap - topGap) * shipY / 64;
    int leftFloor = 64 - gapAtY;
    int rightFloor = 64 + gapAtY;
    
    // Keep crosshair (and therefore T-16) within canyon walls with margin
    if(crosshairX < leftFloor + 8) crosshairX = leftFloor + 8;
    if(crosshairX > rightFloor - 8) crosshairX = rightFloor - 8;
  }
}
}

void handleFireButton() {
  if(currentState == WOMP_RAT_TRAINING) {
    flashCrosshair = millis() + 150;
    
    for(int i = 0; i < 6; i++) {
      if(wompRats[i].active) {
        float dist = sqrt(pow(crosshairX - wompRats[i].x, 2) + 
                         pow(crosshairY - wompRats[i].y, 2));
        if(dist < 8) {
          wompRats[i].active = false;
          score += 10;
          createExplosion(wompRats[i].x, wompRats[i].y);
          break;
        }
      }
    }
    return;
  }

  if(currentState == BLINDFOLD_TRAINING) {
    lastDeflectionAttempt = millis();
    flashCrosshair = millis() + 150;
    
    // Only allow shooting when ball is in warning state (flashing)
    if(trainingRemote.showPrediction) {
      // Check if player is shooting at the remote ball
      float dist = sqrt(pow(crosshairX - trainingRemote.x, 2) + 
                       pow(crosshairY - trainingRemote.y, 2));
      
      // Scoring based on accuracy
      if(dist < 2) {
        // Perfect hit - dead center
        deflectionsSuccessful++;
        score += 20;
        createExplosion(trainingRemote.x, trainingRemote.y);
        
        // Reset ball for next attack
        trainingRemote.showPrediction = false;
        trainingRemote.nextFireTime = millis() + random(2500, 4000);
      } else if(dist < 4) {
        // Near middle
        deflectionsSuccessful++;
        score += 10;
        createExplosion(trainingRemote.x, trainingRemote.y);
        
        // Reset ball for next attack
        trainingRemote.showPrediction = false;
        trainingRemote.nextFireTime = millis() + random(2500, 4000);
      } else if(dist < 8) {
        // Just outside ball - edge hit
        deflectionsSuccessful++;
        score += 5;
        createExplosion(trainingRemote.x, trainingRemote.y);
        
        // Reset ball for next attack
        trainingRemote.showPrediction = false;
        trainingRemote.nextFireTime = millis() + random(2500, 4000);
      } else {
        // Complete miss - shot during warning but missed
        deflectionsMissed++;
      }
    }
    // If not in warning state, shooting does nothing (no penalty)
    return;
  }

  if(currentState == SPACE_BATTLE) {
    hitFlash = false;
    
    for(int i = 0; i < 8; i++) {
      if(enemies[i].active) {
        float dist = sqrt(pow(crosshairX - enemies[i].x, 2) + 
                         pow(crosshairY - enemies[i].y, 2));
        if(dist < 10) {
          hitFlash = true;
          enemies[i].health -= 1;
          
          if(enemies[i].health <= 0) {
            enemies[i].active = false;
            enemyCount--;
            score += (enemies[i].type + 1) * 10;
            createExplosion(enemies[i].x, enemies[i].y);
            
            if(random(100) < 20) {
              float offsetX = enemies[i].x + random(-15, 15);
              float offsetY = enemies[i].y + random(-10, 10);
              if(offsetX < 15) offsetX = 15;
              if(offsetX > 113) offsetX = 113;
              if(offsetY < 15) offsetY = 15;
              if(offsetY > 45) offsetY = 45;
              spawnPowerUp(offsetX, offsetY, 0);
            }
          }
          break;
        }
      }
    }
    
    flashCrosshair = millis() + 150;
    
  } else if(currentState == DEATH_STAR_SURFACE) {
    flashCrosshair = millis() + 100;
    
    for(int i = 0; i < 4; i++) {
        if(surfaceTowers[i].active) {
        float scale = 100.0 / (surfaceTowers[i].z + 20);
        
        if(scale > 0.3) {
          int towerWidth = max(1, (int)(3 * scale));
          int towerHeight = max(2, (int)(8 * scale));
          
          float dist = sqrt(pow(crosshairX - surfaceTowers[i].x, 2) + 
                           pow(crosshairY - (surfaceTowers[i].y - towerHeight/2), 2));
          if(dist < (towerWidth + 8)) {
            surfaceTowers[i].health -= 1;
            surfaceTowers[i].lastHit = millis();
            
            if(surfaceTowers[i].health <= 0) {
              surfaceTowers[i].active = false;
              score += 20;
              createExplosion(surfaceTowers[i].x, surfaceTowers[i].y - towerHeight/2);
            }
            break;
          }
        }
      }
    }
  } else if(currentState == TRENCH_RUN) {
    fireProjectile(playerX, playerY - 2, 0, -12, true);
  } else if(currentState == EXHAUST_PORT) {
    handleExhaustPortFiring();
  }
}

void updateGame() {
  updateStars();
  updateVectorTrails();
  updateExplosions();

  // Universal health restoration every 200 points (all game states)
 static int lastHealthBonus = 0;
int currentHealthLevel = score / 200;
if(currentHealthLevel > lastHealthBonus) {
  shields += 5;  // Changed from 10 to 5 (10% to 5%)
  if(shields > 100) shields = 100;
  showHealthBonus = millis() + 1000;
  lastHealthBonus = currentHealthLevel;
}

  switch(currentState) {
    case TITLE_SCREEN:
      // Animate title screen
      break;

    case WOMP_RAT_TRAINING:
  updateWompRatTraining();
  
  if(score >= 200) {
    currentState = FORCE_LESSON;
    stateTimer = millis();
    stageStartScore = score;
    lastFirePress = millis() + 500;
  }
  break; 
      
    case TRAINING_COMPLETE:
      // Wait for player to press button
      break;
      
      case FORCE_LESSON:
  // Wait for player to press button
  break;
  
case BLINDFOLD_TRAINING:
  updateBlindfolding();
  
  // Win condition: 200 points in THIS stage
  if(score - stageStartScore >= 200) {  // ← Use relative scoring
    // Give 20% health boost for completing Jedi training
    shields += 20;
    if(shields > 100) shields = 100;
    showHealthBonus = millis() + 1000;  // Show the +20 notification
    
    currentState = TRAINING_COMPLETE_2;
    stateTimer = millis();
  }
  break;

case TATOOINE_SUNSET:
      // 8 second cinematic scene, then auto-transition to womp rat training
      if(millis() - stateTimer > 8000) {
        currentState = WOMP_RAT_TRAINING;
        stateTimer = millis();
        wompRatsKilled = 0;
        initializeWompRats();
        initializeCanyonRocks();
      }
      break;
      
    case SPACE_BATTLE:
      updateEnemies();
      updateProjectiles();
      updatePowerUps();
      
      // Spawn enemies
      // Spawn enemies - ADD LIMIT CHECK
      if(random(100) < 2 && enemyCount < 6) {  // Changed from < 8 to < 6
        spawnEnemy();
      }
      
      // Check for Death Star approach transition
      if(score - stageStartScore >= 500) {  // CHANGED: Use score difference instead of absolute score
        currentState = DEATH_STAR_APPROACH;
        stateTimer = millis();
        deathStarDistance = 500.0;
      }
      break;

    case DEATH_STAR_APPROACH:
  // No enemies or projectiles during approach - just cinematic flyby
  
  // Approach Death Star
  deathStarDistance -= 1.5;
  
  // Transition to surface when close enough
  if(deathStarDistance <= 80) {
    stageStartScore = score;  // Set baseline for surface run
    currentState = DEATH_STAR_SURFACE;
    stateTimer = millis();
    surfaceSpeed = 2.0;
    playerY = 50;
    crosshairX = 64;  // Center horizontally
    crosshairY = 35;  // Just above horizon at y=45
  }
  break;

    case DEATH_STAR_SURFACE: {
      updateSurfaceTowers();
      updateProjectiles();
      
      // Player movement constraints for surface phase
      if(playerX < 15) playerX = 15;
      if(playerX > 113) playerX = 113;
      
      // Transition to trench run after destroying enough towers
      int activeTowers = 0;
      for(int i = 0; i < 4; i++) {
        if(surfaceTowers[i].active) activeTowers++;
      }
      if(score - stageStartScore >= 500) {
        stageStartScore = score;  // Set new baseline for trench run
        currentState = TRENCH_ENTRY;  // New transition state
        stateTimer = millis();
        surfaceSpeed = 4.0;  // Speed up for dramatic effect
      }
      break;
      }
    case TRENCH_ENTRY:
      // 2-second diving transition
      if(millis() - stateTimer > 5000) {  // Changed from 2000 to 5000
        currentState = TRENCH_RUN;
        trenchSpeed = 3.0;
        playerY = 45;
      }
      break;
  
    case USE_THE_FORCE:
     // 7 second "Use the Force" message
      if(millis() - stateTimer > 7000) {
        currentState = EXHAUST_PORT;
        exhaustPortTimer = millis() + 6000;
        exhaustPortStartTime = millis(); // Start the 5-second countdown
        crosshairX = random(15, 113); // Random X position (keeping away from edges)
        crosshairY = random(15, 49);  // Random Y position (keeping away from edges)
        targetingActive = true;
        shotFired = false;
        missionSuccess = false;
      }
      break;
      
    case TRENCH_RUN:
      updateTrench();
      updateProjectiles();
      
      // Check turret collisions with player
      for(int seg = 0; seg < 6; seg++) {
        if(trenchSegments[seg].hasObstacle && trenchSegments[seg].z > 0) {
          float scale = 100.0 / (trenchSegments[seg].z + 20);
          
          if(scale > 0.3) { // Only check collision when turret is close enough
            int screenY = map(trenchSegments[seg].z, 150, 0, 25, 60);
            int trenchWidth = 60 * scale;
            int leftWall = 64 - trenchWidth/2;
            int turretX = leftWall + (trenchSegments[seg].obstacleX - 20) * scale * 0.8;
            
            int turretSize = max(3, (int)(8 * scale));
            float dist = sqrt(pow(playerX - turretX, 2) + pow(playerY - screenY, 2));
           
            if(dist < turretSize + 4) { // Player collision radius
            shields -= 5; // 5% damage
            trenchSegments[seg].turretHealth -= 1; // Damage turret health
            
            if(trenchSegments[seg].turretHealth <= 0) {
              trenchSegments[seg].hasObstacle = false; // Destroy turret only after health depleted
              createExplosion(turretX, screenY);
            }
            
            if(shields <= 0) {
              currentState = GAME_OVER;
            }
          }
          }
        }
      }
      
      // Check collisions with walls
      // Temporarily disabled for trench testing
      // if(checkCollisions()) {
      //   shields -= 25;
      //   if(shields <= 0) {
      //       currentState = GAME_OVER;
      //   }
      // }
      
      // Check for exhaust port transition
      // Remove time-based transition - only score-based now
      if(score - stageStartScore >= 500) { 
        stageStartScore = score;  // Set new baseline for exhaust port
        currentState = USE_THE_FORCE;
        stateTimer = millis();
      }
      break;
      
    case EXHAUST_PORT:
      updateProjectiles();

       // Check for 5-second timeout
      if(millis() - exhaustPortStartTime > EXHAUST_PORT_TIMEOUT && !missionSuccess) {
        // Time's up! Go back to trench run
        currentState = TRENCH_RUN;
        stateTimer = millis();
        return;
      }
      
      
      // Update missile shaft animation if active
if(inMissileShaft) {
  shaftDepth += 3.0; // Speed of descent
  missileY = shaftDepth * 0.8; // Missile follows slightly behind camera
  
  // Update shaft segments
  for(int i = 0; i < 8; i++) {
    shaftSegments[i].z -= 3.0;
    if(shaftSegments[i].z < -20) {
      shaftSegments[i].z = 140;
      shaftSegments[i].width = 40 - ((int)(shaftDepth/20) * 2);
      if(shaftSegments[i].width < 8) shaftSegments[i].width = 8;
    }
  }
  
 // Transition to explosion when missile hits core
if(shaftDepth > 175) { // Missile hits core at depth 175
  currentState = DEATH_STAR_EXPLOSION;
  explosionStartTime = millis();
}
} else {
  // Check for victory/failure after shot animation
  if(shotFired && millis() - shotTime > 1000) { // 1 second animation instead of 3
    if(missionSuccess) {
      // Start missile shaft animation instead of going directly to victory
      inMissileShaft = true;
      shaftStartTime = millis();
      shaftDepth = 0.0;
      missileY = 0.0;
      initializeShaft();
    } else {
      currentState = GAME_OVER;
    }
  }
  }
  break;

  case DEATH_STAR_EXPLOSION:
      // Run explosion for 4 seconds then go to victory
      if(millis() - explosionStartTime > 4000) {
        currentState = VICTORY;
      }
      break;
    
  case GAME_OVER:
  case VICTORY:
    // No updates needed for end states
    break;
  }
}

void drawGame() {
  if(!display.getBuffer()) return; // Safety check
  display.clearDisplay();
  
  // Screen damage flash effect - inverts display briefly
  if(millis() < damageFlash) {
    // Draw damage indicator borders
    if((millis() / 50) % 2 == 0) {
      display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
      display.drawRect(1, 1, 126, 62, SSD1306_WHITE);
      display.drawRect(2, 2, 124, 60, SSD1306_WHITE);
    }
  }
  
  switch(currentState) {
    case TITLE_SCREEN:
      drawTitleScreen();
      break;
      case WOMP_RAT_TRAINING:
      drawWompRatTraining();
      drawUI();
      break;
      
    case TRAINING_COMPLETE:
      drawTrainingComplete();
      break;
      
    case FORCE_LESSON:
      drawForceLesson();
      break;
      
    case BLINDFOLD_TRAINING:
      drawBlindfolding();
      drawUI();
      break;
      
    case TRAINING_COMPLETE_2:
      drawTrainingComplete2();
      break;
      
    case TATOOINE_SUNSET:
      drawTatooineSunset();
      break;

    case DEATH_STAR_SURFACE:
      drawCockpitWindow();
      drawDeathStarSurface();
      drawSurfaceTowers();
      drawProjectiles();
      // Draw crosshair for targeting
      if(millis() - flashCrosshair < 100) {
        // Flashing crosshair when firing
        display.drawCircle(crosshairX, crosshairY, 8, SSD1306_WHITE);
        display.drawLine(crosshairX-12, crosshairY, crosshairX+12, crosshairY, SSD1306_WHITE);
        display.drawLine(crosshairX, crosshairY-12, crosshairX, crosshairY+12, SSD1306_WHITE);
        // Flashing center circle when firing
        display.fillCircle(crosshairX, crosshairY, 3, SSD1306_WHITE);
      } else {
        // Normal crosshair
        display.drawCircle(crosshairX, crosshairY, 6, SSD1306_WHITE);
        display.drawLine(crosshairX-8, crosshairY, crosshairX+8, crosshairY, SSD1306_WHITE);
        display.drawLine(crosshairX, crosshairY-8, crosshairX, crosshairY+8, SSD1306_WHITE);
        // Normal center dot
        display.drawPixel(crosshairX, crosshairY, SSD1306_WHITE);
      }
      drawUI();
      break;

case SPACE_BATTLE:
  drawCockpitWindow();
  drawStars();
  drawEnemies();
  drawProjectiles();
  drawPowerUps();
  drawExplosions();
  drawSpaceBattlePlayer();
  drawUI();
  break;

case DEATH_STAR_APPROACH:
  drawCockpitWindow();
  drawStars();
  drawDeathStarApproach();
  drawUI();
  break;

case TRENCH_ENTRY:
  drawTrenchEntry();
  break;

case TRENCH_RUN:
  drawTrench();
  drawTrenchPlayer();
  drawProjectiles();
  drawExplosions();
  drawUI();
  break;

case USE_THE_FORCE:
  drawUseTheForce();
  break;
  
case EXHAUST_PORT:
  if(inMissileShaft) {
    drawMissileShaft();
  } else {
    drawExhaustPort();
    drawTargeting();
    drawProjectiles();
  }
  drawUI();
  break;
  
  case DEATH_STAR_EXPLOSION:
    drawDeathStarExplosion();
    break;

  case VICTORY:
    drawVictory();
    break;

  case GAME_OVER:
    drawGameOver();
    break;
}  // End of switch statement

display.display();
}

void handleExhaustPortFiring() {  
  if(currentState == EXHAUST_PORT && targetingActive && !shotFired) {
    targetAccuracy = sqrt(pow(crosshairX - 64, 2) + pow(crosshairY - 40, 2));
    
    if(targetAccuracy <= 3) {
      shotFired = true;
      shotTime = millis();
      missionSuccess = true;
      score += 500;
      fireProjectile(crosshairX, crosshairY, 0, -4, true);
    } else {
    }
    // Miss - don't end targeting, let player keep trying until timer expires
      fireProjectile(crosshairX, crosshairY, 0, -4, true);
  }
}
  
void initializeStars() {
  for(int i = 0; i < 30; i++) {
    stars[i].x = random(10, 118);  // Keep away from edges
    stars[i].y = random(10, 54);
    stars[i].z = random(50, 200);
    stars[i].speed = random(1, 4);
  }
}

void updateStars() {
  for(int i = 0; i < 30; i++) {
    stars[i].z -= stars[i].speed;
    
    // Calculate direction from center (64, 32) outward
    float centerX = 64.0;
    float centerY = 32.0;
    float dx = stars[i].x - centerX;
    float dy = stars[i].y - centerY;
    float distance = sqrt(dx*dx + dy*dy);
    
    // Move star outward from center based on speed and proximity
    if(distance > 0) {
      float moveSpeed = stars[i].speed * 0.3;
      stars[i].x += (dx / distance) * moveSpeed;
      stars[i].y += (dy / distance) * moveSpeed;
    }
    
     if(stars[i].z <= 0) {
      stars[i].x = random(54, 74);  // Respawn near center
      stars[i].y = random(22, 42);
       stars[i].z = 200;
      stars[i].speed = random(1, 4);
    }
  }
}

void drawStars() {
  for(int i = 0; i < 30; i++) {
    if(stars[i].z > 0) {
      float size = map(stars[i].z, 200, 1, 1, 4);
      int brightness = map(stars[i].z, 200, 1, 50, 255);
      
      // Draw star with depth effect
      if(size < 1.5) {
        display.drawPixel(stars[i].x, stars[i].y, SSD1306_WHITE);
      } else {
        display.fillCircle(stars[i].x, stars[i].y, size/2, SSD1306_WHITE);
      }
      
      // Draw motion trail outward from center for close stars
      if(stars[i].z < 50) {
        float centerX = 64.0;
        float centerY = 32.0;
        float dx = stars[i].x - centerX;
        float dy = stars[i].y - centerY;
        float distance = sqrt(dx*dx + dy*dy);
        float trailLength = (200 - stars[i].z) / 15;
        
        // Trail points backwards toward center (where star came from)
        float trailX = stars[i].x - (dx / distance) * trailLength;
        float trailY = stars[i].y - (dy / distance) * trailLength;
        display.drawLine(stars[i].x, stars[i].y, trailX, trailY, SSD1306_WHITE);
      }
    }
  }
}

void spawnEnemy() {
  if(enemyCount >= 8) return; // Prevent overflow

  int actualCount = 0;
  for(int i = 0; i < 8; i++) {
    if(enemies[i].active) actualCount++;
  }
  if(actualCount >= 7) return; // Extra safety check
  
  for(int i = 0; i < 8; i++) {
    if(!enemies[i].active) {
      enemies[i].x = random(30, 98);  // Keep away from screen edges
      enemies[i].y = random(12, 35);  // More spread across screen height
      enemies[i].z = random(80, 200); // Slightly closer range for better visibility
      enemies[i].vx = random(-2, 2);
      enemies[i].vy = random(-1, 2);  // Mix of upward and downward movement
      enemies[i].vz = random(-3, -1);
      enemies[i].angle = random(0, 360);
      enemies[i].type = random(0, 3);
      enemies[i].active = true;
      enemies[i].health = 2; // All ships now need 2 hits
      enemies[i].lastFire = millis();
      
      // Assign weapon types based on ship type
      if(enemies[i].type == 0) {
        enemies[i].weaponType = 0; // TIE Fighter: single shot
      } else if(enemies[i].type == 1) {
        enemies[i].weaponType = random(0, 2); // Interceptor: single or burst
      } else {
        enemies[i].weaponType = 2; // Bomber: spread shot
      }
      
      enemyCount++;
      break;
    }
  }
}

void updateEnemies() {
  for(int i = 0; i < 8; i++) {
    if(enemies[i].active) {
      // Different movement patterns by type
      switch(enemies[i].type) {
        case 0: // TIE Fighter - straight attack
          enemies[i].x += enemies[i].vx;
          enemies[i].y += enemies[i].vy;
          enemies[i].z += enemies[i].vz;
          break;
        
        case 1: // TIE Interceptor - weaving pattern
          enemies[i].x += enemies[i].vx + sin(millis() * 0.005 + i) * 2;
          enemies[i].y += enemies[i].vy + cos(millis() * 0.003 + i) * 1.5;
          enemies[i].z += enemies[i].vz * 1.2; // Faster approach
          break;
        
        case 2: // TIE Bomber - slow with vertical oscillation
          enemies[i].x += enemies[i].vx * 0.7; // Slower horizontal
          enemies[i].y += enemies[i].vy * 0.5 + sin(millis() * 0.002 + i) * 1; // Gentle bob
          enemies[i].z += enemies[i].vz * 0.8; // Slower approach
          // Change direction occasionally
          if(random(200) < 1) {
            enemies[i].vx = -enemies[i].vx;
          }
          break;
      }
      
      enemies[i].angle += 5;
      
      // Remove if too close, off screen, or drifted too low
      if(enemies[i].z <= 10 || enemies[i].x < -10 || enemies[i].x > 138 || 
         enemies[i].y < 5 || enemies[i].y > 50) {
        enemies[i].active = false;
        enemyCount--;
        continue;
      }
   
     // Enemy firing - only in SPACE_BATTLE and DEATH_STAR_APPROACH
      if(currentState == SPACE_BATTLE || currentState == DEATH_STAR_APPROACH) {
       // Different firing rates based on weapon type - FASTER
        int fireDelay = 1000; // Reduced from 1800
        if(enemies[i].weaponType == 1) fireDelay = 700;  // Reduced from 1200
        if(enemies[i].weaponType == 2) fireDelay = 1400; // Reduced from 2200
        if(millis() - enemies[i].lastFire > fireDelay + random(500)) {
          enemies[i].lastFire = millis();
          
          // Calculate trajectory to hit crosshair
          float dx = crosshairX - enemies[i].x;
          float dy = crosshairY - enemies[i].y;
          float distance = sqrt(dx*dx + dy*dy);
          
          if(distance > 0.1 && distance < 120) {
            float speed = 2.0;
            float vx = (dx / distance) * speed;
            float vy = (dy / distance) * speed;
            
            // Fire based on weapon type
            switch(enemies[i].weaponType) {
              case 0: // Single shot
                fireProjectile(enemies[i].x, enemies[i].y, vx, vy, false);
                break;
                
              case 1: // Burst fire (3 shots)
                fireProjectile(enemies[i].x, enemies[i].y, vx, vy, false);
                static unsigned long lastBurst[8] = {0};
                if(millis() - lastBurst[i] > 200) {
                  fireProjectile(enemies[i].x, enemies[i].y, vx * 1.1, vy * 1.1, false);
                  fireProjectile(enemies[i].x, enemies[i].y, vx * 0.9, vy * 0.9, false);
                  lastBurst[i] = millis();
                }
                break;
                
              case 2: // Spread shot (3 projectiles in a cone)
                fireProjectile(enemies[i].x, enemies[i].y, vx, vy, false); // Center
                // Add angle variation for spread
                float angle = atan2(vy, vx);
                float spreadAngle = 0.3; // ~17 degrees spread
                
                // Left shot
                float vxLeft = cos(angle - spreadAngle) * speed;
                float vyLeft = sin(angle - spreadAngle) * speed;
                fireProjectile(enemies[i].x, enemies[i].y, vxLeft, vyLeft, false);
                
                // Right shot
                float vxRight = cos(angle + spreadAngle) * speed;
                float vyRight = sin(angle + spreadAngle) * speed;
                fireProjectile(enemies[i].x, enemies[i].y, vxRight, vyRight, false);
                break;
            }
          }
        } 
      }
    }
  }
}   

void drawEnemies() {
  for(int i = 0; i < 8; i++) {
    if(enemies[i].active) {
      float scale = map(enemies[i].z, 200, 10, 2, 12);
      int drawX = enemies[i].x;
      int drawY = enemies[i].y;
      
      // Draw different enemy types with vector style
      switch(enemies[i].type) {
        case 0: // TIE Fighter
          drawVectorTIE(drawX, drawY, scale, enemies[i].angle);
          break;
        case 1: // TIE Interceptor
          drawVectorInterceptor(drawX, drawY, scale, enemies[i].angle);
          break;
        case 2: // TIE Bomber
          drawVectorBomber(drawX, drawY, scale, enemies[i].angle);
          break;
      }
    }
  }
}

void drawVectorTIE(float x, float y, float scale, float angle) {
  // TIE Fighter - classic vector wireframe style
  
  // Central spherical cockpit (outline only - true vector style)
  int cockpitRadius = max(2, (int)(scale/2));
  display.drawCircle(x, y, cockpitRadius, SSD1306_WHITE);
  
  // Inner cockpit detail circle
  if(scale > 6) {
    display.drawCircle(x, y, cockpitRadius - 1, SSD1306_WHITE);
  }
  
  // Hexagonal cockpit window (signature vector game detail)
  if(scale > 4) {
    int windowSize = cockpitRadius * 0.7;
    // Draw hexagon with clean vector lines
    for(int i = 0; i < 6; i++) {
      float angle1 = (i * PI) / 3;
      float angle2 = ((i + 1) * PI) / 3;
      int x1 = x + cos(angle1) * windowSize;
      int y1 = y + sin(angle1) * windowSize;
      int x2 = x + cos(angle2) * windowSize;
      int y2 = y + sin(angle2) * windowSize;
      display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
    }
  }
  
  // Large rectangular solar panels (vector wireframe - no fill)
  int panelWidth = max(2, (int)(scale * 0.6));
  int panelHeight = max(6, (int)(scale * 1.8));
  int panelOffset = cockpitRadius + panelWidth/2 + 2;
  
  // Left solar panel (outline only)
  int leftX = x - panelOffset;
  display.drawRect(leftX - panelWidth/2, y - panelHeight/2, panelWidth, panelHeight, SSD1306_WHITE);
  
  // Panel internal structure (vector grid lines)
  if(scale > 8) {
    // Horizontal dividers
    display.drawLine(leftX - panelWidth/2, y - panelHeight/6, leftX + panelWidth/2, y - panelHeight/6, SSD1306_WHITE);
    display.drawLine(leftX - panelWidth/2, y + panelHeight/6, leftX + panelWidth/2, y + panelHeight/6, SSD1306_WHITE);
    // Vertical center line
    display.drawLine(leftX, y - panelHeight/2, leftX, y + panelHeight/2, SSD1306_WHITE);
  }
  
  // Right solar panel
  int rightX = x + panelOffset;
  display.drawRect(rightX - panelWidth/2, y - panelHeight/2, panelWidth, panelHeight, SSD1306_WHITE);
  
  // Right panel internal structure
  if(scale > 8) {
    display.drawLine(rightX - panelWidth/2, y - panelHeight/6, rightX + panelWidth/2, y - panelHeight/6, SSD1306_WHITE);
    display.drawLine(rightX - panelWidth/2, y + panelHeight/6, rightX + panelWidth/2, y + panelHeight/6, SSD1306_WHITE);
    display.drawLine(rightX, y - panelHeight/2, rightX, y + panelHeight/2, SSD1306_WHITE);
  }
  
  // Support struts (clean vector lines)
  display.drawLine(x - cockpitRadius, y, leftX + panelWidth/2, y, SSD1306_WHITE);
  display.drawLine(x + cockpitRadius, y, rightX - panelWidth/2, y, SSD1306_WHITE);
  
  // Engine details at panel corners (small vector elements)
  if(scale > 6) {
    display.drawPixel(leftX, y - panelHeight/2, SSD1306_WHITE);
    display.drawPixel(leftX, y + panelHeight/2, SSD1306_WHITE);
    display.drawPixel(rightX, y - panelHeight/2, SSD1306_WHITE);
    display.drawPixel(rightX, y + panelHeight/2, SSD1306_WHITE);
  }
}

void drawVectorInterceptor(float x, float y, float scale, float angle) {
  // TIE Interceptor - sleeker vector design with pointed wings
  
  // Smaller, more angular cockpit
  int cockpitRadius = max(2, (int)(scale/3));
  display.drawCircle(x, y, cockpitRadius, SSD1306_WHITE);
  
  // Angular cockpit window (square for more aggressive look)
  if(scale > 4) {
    int windowSize = cockpitRadius;
    display.drawRect(x - windowSize/2, y - windowSize/2, windowSize, windowSize, SSD1306_WHITE);
  }
  
  // Characteristic pointed dagger wings
  int wingLength = max(8, (int)(scale * 2.0));
  int wingSpread = max(3, (int)(scale * 0.8));
  int cockpitEdge = cockpitRadius + 1;
  
  // Left dagger wing - distinctive pointed vector shape
  int leftBaseX = x - cockpitEdge;
  int leftTipX = x - cockpitEdge - wingLength;
  
  // Wing outline (creating sharp dagger silhouette)
  display.drawLine(leftBaseX, y - wingSpread/2, leftTipX + wingSpread/3, y - wingSpread/4, SSD1306_WHITE); // Top edge
  display.drawLine(leftTipX + wingSpread/3, y - wingSpread/4, leftTipX, y, SSD1306_WHITE); // Point tip
  display.drawLine(leftTipX, y, leftTipX + wingSpread/3, y + wingSpread/4, SSD1306_WHITE); // Bottom tip
  display.drawLine(leftTipX + wingSpread/3, y + wingSpread/4, leftBaseX, y + wingSpread/2, SSD1306_WHITE); // Bottom edge
  display.drawLine(leftBaseX, y + wingSpread/2, leftBaseX, y - wingSpread/2, SSD1306_WHITE); // Connection to cockpit
  
  // Right dagger wing (mirror)
  int rightBaseX = x + cockpitEdge;
  int rightTipX = x + cockpitEdge + wingLength;
  
  display.drawLine(rightBaseX, y - wingSpread/2, rightTipX - wingSpread/3, y - wingSpread/4, SSD1306_WHITE);
  display.drawLine(rightTipX - wingSpread/3, y - wingSpread/4, rightTipX, y, SSD1306_WHITE);
  display.drawLine(rightTipX, y, rightTipX - wingSpread/3, y + wingSpread/4, SSD1306_WHITE);
  display.drawLine(rightTipX - wingSpread/3, y + wingSpread/4, rightBaseX, y + wingSpread/2, SSD1306_WHITE);
  display.drawLine(rightBaseX, y + wingSpread/2, rightBaseX, y - wingSpread/2, SSD1306_WHITE);
  
  // Wing internal structure (vector detail lines)
  if(scale > 8) {
    display.drawLine(leftBaseX - wingLength/2, y - wingSpread/4, leftBaseX - wingLength/2, y + wingSpread/4, SSD1306_WHITE);
    display.drawLine(rightBaseX + wingLength/2, y - wingSpread/4, rightBaseX + wingLength/2, y + wingSpread/4, SSD1306_WHITE);
  }
  
  // Laser cannon details at wing tips
  if(scale > 6) {
    display.drawPixel(leftTipX, y, SSD1306_WHITE);
    display.drawPixel(rightTipX, y, SSD1306_WHITE);
    // Small vector cross-hairs at tips
    display.drawLine(leftTipX - 1, y, leftTipX + 1, y, SSD1306_WHITE);
    display.drawLine(leftTipX, y - 1, leftTipX, y + 1, SSD1306_WHITE);
    display.drawLine(rightTipX - 1, y, rightTipX + 1, y, SSD1306_WHITE);
    display.drawLine(rightTipX, y - 1, rightTipX, y + 1, SSD1306_WHITE);
  }
}

void drawVectorBomber(float x, float y, float scale, float angle) {
  // TIE Bomber - horizontal dual-pod vector design with connecting section
  
  int podRadius = max(2, (int)(scale * 0.4));
  int podSpacing = max(6, (int)(scale * 1.0));
  int connectorWidth = max(2, (int)(scale * 0.3));
  
  // Left pod 
  display.drawCircle(x - podSpacing/2, y, podRadius, SSD1306_WHITE);
  
  // Left pod viewport
  if(scale > 4) {
    int windowSize = podRadius * 0.8;
    display.drawRect(x - podSpacing/2 - windowSize/2, y - windowSize/2, windowSize, windowSize/2, SSD1306_WHITE);
  }
  
  // Right pod (same size as left pod)
  display.drawCircle(x + podSpacing/2, y, podRadius, SSD1306_WHITE);
  
  // Right pod details
  if(scale > 6) {
    display.drawRect(x + podSpacing/2 - 1, y - 1, 2, 2, SSD1306_WHITE);
  }
  
  // Central connecting section between pods
  int connectorLeft = x - podSpacing/2 + podRadius;
  int connectorRight = x + podSpacing/2 - podRadius;
  display.drawRect(connectorLeft, y - connectorWidth/2, connectorRight - connectorLeft, connectorWidth, SSD1306_WHITE);
  
  // Curved solar panel wings (positioned at sides of the dual pod assembly)
  int wingHeight = max(6, (int)(scale * 1.5));
  int wingWidth = max(2, (int)(scale * 0.5));
  int wingOffsetX = podSpacing/2 + podRadius + wingWidth/2 + 1;
  
  // Left wing - curved/bent outline (positioned to left of left pod)
  int leftWingX = x - wingOffsetX;
  display.drawRect(leftWingX - wingWidth/2, y - wingHeight/2, wingWidth, wingHeight, SSD1306_WHITE);
  
  // Wing curve lines - create bent appearance
  if(scale > 6) {
    // Curved lines to show wing bend
    display.drawLine(leftWingX - wingWidth/2 + 1, y - wingHeight/3, leftWingX + wingWidth/2 - 1, y - wingHeight/4, SSD1306_WHITE);
    display.drawLine(leftWingX - wingWidth/2 + 1, y + wingHeight/3, leftWingX + wingWidth/2 - 1, y + wingHeight/4, SSD1306_WHITE);
  }
  
  // Right wing - curved/bent outline (positioned to right of right pod)
  int rightWingX = x + wingOffsetX;
  display.drawRect(rightWingX - wingWidth/2, y - wingHeight/2, wingWidth, wingHeight, SSD1306_WHITE);
  
  // Right wing curve lines
  if(scale > 6) {
    // Mirror the curve
    display.drawLine(rightWingX - wingWidth/2 + 1, y - wingHeight/4, rightWingX + wingWidth/2 - 1, y - wingHeight/3, SSD1306_WHITE);
    display.drawLine(rightWingX - wingWidth/2 + 1, y + wingHeight/4, rightWingX + wingWidth/2 - 1, y + wingHeight/3, SSD1306_WHITE);
  }
  
  // Wing support struts (connect wings to outer edges of each pod)
  display.drawLine(x - podSpacing/2 - podRadius, y, leftWingX + wingWidth/2, y, SSD1306_WHITE);
  display.drawLine(x + podSpacing/2 + podRadius, y, rightWingX - wingWidth/2, y, SSD1306_WHITE);
  
  // Engine details at wing edges
  if(scale > 5) {
    display.drawPixel(leftWingX - wingWidth/2, y + wingHeight/2, SSD1306_WHITE);
    display.drawPixel(rightWingX + wingWidth/2, y + wingHeight/2, SSD1306_WHITE);
  }
}

void fireProjectile(float x, float y, float vx, float vy, bool isPlayer) {
  for(int i = 0; i < 16; i++) {
    int activeCount = 0;
  for(int i = 0; i < 16; i++) {
    if(projectiles[i].active) activeCount++;
  }
  if(activeCount >= 15) return; // Don't spawn if almost full
    if(!projectiles[i].active) {
      projectiles[i].x = x;
      projectiles[i].y = y;
      projectiles[i].z = 50;
      projectiles[i].vx = vx;
      projectiles[i].vy = vy;
      projectiles[i].vz = 0;
      projectiles[i].active = true;
      projectiles[i].isPlayerShot = isPlayer;
      projectiles[i].damage = isPlayer ? 1 : 5;
      projectiles[i].birthTime = millis();
      break;
    }
  }
}

void updateProjectiles() {
  for(int i = 0; i < 16; i++) {
    if(projectiles[i].active) {
      projectiles[i].x += projectiles[i].vx;
      projectiles[i].y += projectiles[i].vy;
      // Prevent coordinate overflow
      if(projectiles[i].x < -100 || projectiles[i].x > 228 ||
         projectiles[i].y < -100 || projectiles[i].y > 164) {
        projectiles[i].active = false;
        continue;
      }
      
      // Remove if off screen or too old
      if(projectiles[i].x < -5 || projectiles[i].x > 133 ||
         projectiles[i].y < 8 || projectiles[i].y > 69 ||
         millis() - projectiles[i].birthTime > 3000) {
        projectiles[i].active = false;
        continue;
      }
      
      // Check collisions
      if(projectiles[i].isPlayerShot) {
        if(currentState == TRENCH_RUN) {
          for(int seg = 0; seg < 6; seg++) {
            if(trenchSegments[seg].hasObstacle && trenchSegments[seg].z > 0) {
              float scale = 100.0 / (trenchSegments[seg].z + 20);
                
              // Turrets fire at player occasionally
              if(random(400) < 1 && scale > 0.4) {
                int screenY = map(trenchSegments[seg].z, 150, 0, 25, 60);
                int trenchWidth = 60 * scale;
                int leftWall = 64 - trenchWidth/2;
                int turretX = leftWall + (trenchSegments[seg].obstacleX - 20) * scale * 0.8;
                
                fireProjectile(turretX, screenY, random(-1, 2), 3, false);
              }
              int screenY = map(trenchSegments[seg].z, 150, 0, 25, 60);
              int trenchWidth = 60 * scale;
              int leftWall = 64 - trenchWidth/2;
              int turretX = leftWall + (trenchSegments[seg].obstacleX - 20) * scale * 0.8;
              
              int obstacleSize = max(2, (int)(8 * scale));
              // Use larger hit radius for close turrets
              int hitRadius = max(8, obstacleSize + 5);
              float dist = sqrt(pow(projectiles[i].x - turretX, 2) + pow(projectiles[i].y - screenY, 2));
              if(dist < hitRadius) {
                // Hit detected - reduce turret health
                trenchSegments[seg].turretHealth -= 1;
                projectiles[i].active = false;
                
                if(trenchSegments[seg].turretHealth <= 0) {
                  // Turret destroyed after 2 hits
                  trenchSegments[seg].hasObstacle = false;
                  score += 20;
                  createExplosion(turretX, screenY);
                } else {
                  // Turret damaged but not destroyed
                  score += 5; // Smaller score for hit but not destroyed
                }
                break;
              }
            }
          }
        }
        
        // Check enemy hits
        for(int j = 0; j < 8; j++) {
          if(enemies[j].active) {
            float dist = sqrt(pow(projectiles[i].x - enemies[j].x, 2) + 
                            pow(projectiles[i].y - enemies[j].y, 2));
            if(dist < 8) {
              enemies[j].health -= projectiles[i].damage;
              projectiles[i].active = false;
              
              if(enemies[j].health <= 0) {
                enemies[j].active = false;
                enemyCount--;
                score += (enemies[j].type + 1) * 10;
                 
              createExplosion(enemies[j].x, enemies[j].y);
              }
              break;
            }
          }
        }
       } else {
        // Enemy shot - check player hit
        float playerHitX = (currentState == SPACE_BATTLE) ? crosshairX : playerX;
        float playerHitY = (currentState == SPACE_BATTLE) ? crosshairY : playerY;
        
        float dist = sqrt(pow(projectiles[i].x - playerHitX, 2) + 
                        pow(projectiles[i].y - playerHitY, 2));
        if(dist < 6) {
          // Only take damage if not in invincibility window (already have this check below)
          if(millis() - lastDamageTime > INVINCIBILITY_TIME) {
            shields -= projectiles[i].damage;
            damageFlash = millis() + 300;
            lastDamageTime = millis();
            
            if(shields <= 0) {
              currentState = GAME_OVER;
            }
          }
          // Always destroy the projectile even during invincibility
          projectiles[i].active = false;
        }
      } 
      } 
    }
  }

void drawProjectiles() {
  for(int i = 0; i < 16; i++) {
    if(projectiles[i].active) {
      if(currentState == TRENCH_RUN && projectiles[i].isPlayerShot) {
        // Player shots in trench run - simple lines only
        display.drawLine(projectiles[i].x, projectiles[i].y, projectiles[i].x, projectiles[i].y - 4, SSD1306_WHITE);
      } else if(projectiles[i].isPlayerShot) {
        // Player shots in space battle - flashing circles
        if((millis() / 50) % 2 == 0) {
          display.fillCircle(projectiles[i].x, projectiles[i].y, 2, SSD1306_WHITE);
        } else {
          display.fillCircle(projectiles[i].x, projectiles[i].y, 1, SSD1306_WHITE);
        }
      } else {
        // Enemy shots - consistent across all game modes (space battle and trench run)
        // Thick dashes with flash effect - same for both space battle and trench enemies
        int flashCycle = (millis() / 60) % 2;
        display.drawLine(projectiles[i].x - 1, projectiles[i].y, projectiles[i].x + 1, projectiles[i].y, SSD1306_WHITE);
        if(flashCycle == 0) {
          display.drawLine(projectiles[i].x - 1, projectiles[i].y - 1, projectiles[i].x + 1, projectiles[i].y - 1, SSD1306_WHITE);
          display.drawLine(projectiles[i].x - 1, projectiles[i].y + 1, projectiles[i].x + 1, projectiles[i].y + 1, SSD1306_WHITE);
        }
        
        // Draw muzzle flash for enemy shots only
        if(millis() - projectiles[i].birthTime < 100) {
          display.drawCircle(projectiles[i].x, projectiles[i].y, 3, SSD1306_WHITE);
        }
      }
    }
  }
}

void drawVectorEffect(float x1, float y1, float x2, float y2) {
  for(int i = 0; i < 20; i++) {
    if(!vectorTrails[i].active) {
      vectorTrails[i].x1 = x1;
      vectorTrails[i].y1 = y1;
      vectorTrails[i].x2 = x2;
      vectorTrails[i].y2 = y2;
      vectorTrails[i].fadeTime = millis() + random(200, 800);
      vectorTrails[i].active = true;
      break;
    }
  }
}

void updateVectorTrails() {
  for(int i = 0; i < 20; i++) {
    if(vectorTrails[i].active) {
      if(millis() > vectorTrails[i].fadeTime) {
        vectorTrails[i].active = false;
      }
    }
  }
}

void drawVectorTrails() {
  for(int i = 0; i < 20; i++) {
    if(vectorTrails[i].active) {
      display.drawLine(vectorTrails[i].x1, vectorTrails[i].y1,
                      vectorTrails[i].x2, vectorTrails[i].y2, SSD1306_WHITE);
    }
  }
}

void createExplosion(float x, float y) {
  for(int i = 0; i < 5; i++) {
    if(!explosions[i].active) {
      explosions[i].x = x;
      explosions[i].y = y;
      explosions[i].frame = 0;
      explosions[i].active = true;
      break;
    }
  }
}

void updateExplosions() {
  for(int i = 0; i < 5; i++) {
    if(explosions[i].active) {
      explosions[i].frame++;
      if(explosions[i].frame > 20) { // Explosion lasts 20 frames
        explosions[i].active = false;
      }
    }
  }
}

void drawExplosions() {
  for(int i = 0; i < 5; i++) {
    if(explosions[i].active) {
      int frame = explosions[i].frame;
      float x = explosions[i].x;
      float y = explosions[i].y;
      
      // Multi-stage explosion animation
      if(frame < 8) {
        // Expanding circle
        int radius = frame * 2;
        display.drawCircle(x, y, radius, SSD1306_WHITE);
        display.fillCircle(x, y, radius/2, SSD1306_WHITE);
      } else {
        // Debris scatter
        for(int j = 0; j < 8; j++) {
          float angle = j * 45 + frame * 10;
          int debrisX = x + cos(radians(angle)) * (frame - 5);
          int debrisY = y + sin(radians(angle)) * (frame - 5);
          display.fillCircle(debrisX, debrisY, 1, SSD1306_WHITE);
        }
      }
    }
  }
}

void initializeTrench() {
  for(int i = 0; i < 6; i++) {
    trenchSegments[i].leftX = 20 + random(-5, 5);
    trenchSegments[i].rightX = 108 + random(-5, 5);
    trenchSegments[i].y = i * 15;
    trenchSegments[i].z = i * 30;
    trenchSegments[i].hasObstacle = (random(100) < 30);
    trenchSegments[i].obstacleX = random(30, 98);
    trenchSegments[i].turretHealth = 2;
  }
}

void updateTrench() {
  trenchSpeed += 0.005; // Gradual speed increase
  if(trenchSpeed > 8.0) trenchSpeed = 8.0; // Cap maximum speed
  
  for(int i = 0; i < 6; i++) {
    trenchSegments[i].z -= trenchSpeed;

  // Remove turrets that have reached the end of screen FIRST
if(trenchSegments[i].hasObstacle && trenchSegments[i].z <= 8) {
  trenchSegments[i].hasObstacle = false; // Remove turret at screen end - changed from 35 to 8
  continue; // Skip collision check for this segment
}

    // When segment moves off screen, reset it at the back
    if(trenchSegments[i].z < -30) {
      trenchSegments[i].z = 150;
      trenchSegments[i].leftX = 20 + random(-5, 5);
      trenchSegments[i].rightX = 108 + random(-5, 5);
      trenchSegments[i].obstacleX = random(30, 98);
      trenchSegments[i].turretHealth = 2;  // Reset health when respawning
      // Only spawn new turret if this segment doesn't have one
      if(!trenchSegments[i].hasObstacle && random(100) < 30) {
        trenchSegments[i].hasObstacle = true;
      }
    }
    
    // Check player collision with turrets (only destroy on collision when very close)
if(trenchSegments[i].hasObstacle && trenchSegments[i].z > 0 && trenchSegments[i].z < 5) { // Changed from 30 to 5
  float scale = 100.0 / (trenchSegments[i].z + 20);
  int obstacleSize = max(2, (int)(8 * scale));
  int screenY = map(trenchSegments[i].z, 150, 0, 25, 60);
  int trenchWidth = 60 * scale;
  int leftWall = 64 - trenchWidth/2;
  int turretX = leftWall + (trenchSegments[i].obstacleX - 20) * scale * 0.8;
  
  // Only check X-axis collision (side-to-side), ignore Y-axis depth
// This requires turret to be directly in front horizontally
float xDist = abs(playerX - turretX);
float yDist = abs(playerY - screenY);

// Turret must be aligned horizontally AND at same depth
if(xDist < 4 && yDist < 6) { // Must be directly in front
        trenchSegments[i].hasObstacle = false; // Destroy on collision
        shields -= 10;
        createExplosion(turretX, screenY);
        if(shields <= 0) {
          currentState = GAME_OVER;
        }
      }
    }
  }
}

void initializeSurfaceTowers() {
  for(int i = 0; i < 4; i++) {
    surfaceTowers[i].x = random(20, 108);
    surfaceTowers[i].y = 50; // Fixed surface level to match drawDeathStarSurface()
    surfaceTowers[i].z = random(50, 150);
    surfaceTowers[i].health = 2; // Change from 1 to 2
    surfaceTowers[i].active = true;
    surfaceTowers[i].type = random(0, 2);
    surfaceTowers[i].lastHit = 0;
        }
       for(int i = 0; i < 6; i++) {
    backgroundShips[i].x = random(0, 1) ? 10 : 118;  // Start from left or right edge
    backgroundShips[i].y = random(12, 30);  // Higher on screen (was 15-40)
    backgroundShips[i].z = random(100, 180);
    // Set velocity based on starting position - go toward opposite side
    backgroundShips[i].vx = (backgroundShips[i].x < 64) ? random(5, 15) / 10.0 : random(-15, -5) / 10.0;
    backgroundShips[i].vy = 0;
    backgroundShips[i].vz = 0;
    backgroundShips[i].type = random(0, 2);
    backgroundShips[i].active = true;
    backgroundShips[i].lastShot = millis() + random(2000, 5000);
  }
      }

void updateSurfaceTowers() {
  // Update banking angle smoothly
if(currentState == DEATH_STAR_SURFACE) {
  bankingAngle += (targetBankingAngle - bankingAngle) * 0.25;
  
  // Decay banking angle when not moving
  targetBankingAngle *= 0.95;
}

// Update background ships
  for(int i = 0; i < 6; i++) {
    if(backgroundShips[i].active) {
      backgroundShips[i].x += backgroundShips[i].vx;
      // No Y movement - ships fly horizontally only
      // No Z movement - ships stay at constant depth
      
      // Wrap around screen horizontally
      if(backgroundShips[i].x < 10) backgroundShips[i].x = 118;
      if(backgroundShips[i].x > 118) backgroundShips[i].x = 10;
      
      // Random explosions - simple white circle
      if(millis() - backgroundShips[i].lastShot > random(3000, 6000)) {
        backgroundShips[i].lastShot = millis();
        if(random(100) < 40) {  // 40% chance of explosion
          // Mark explosion location and time (we'll draw this in drawDeathStarSurface)
          createExplosion(backgroundShips[i].x, backgroundShips[i].y);
          
          // Respawn ship at random edge
          backgroundShips[i].x = random(0, 1) ? 10 : 118;
          backgroundShips[i].y = random(12, 30);  // Higher on screen
          backgroundShips[i].vx = (backgroundShips[i].x < 64) ? random(5, 15) / 10.0 : random(-15, -5) / 10.0;
          backgroundShips[i].type = random(0, 2);
          backgroundShips[i].lastShot = millis() + random(2000, 4000);
        }
      }
    }
  }
  // Progressive speed increase based on score (starts at 2.0, reaches 3.0 at score 500)
  surfaceSpeed = 2.0 + (score / 500.0) * 1.0;
  if(surfaceSpeed > 3.0) surfaceSpeed = 3.0;
  
  // Move existing towers
  for(int i = 0; i < 4; i++) {
    if(surfaceTowers[i].active) {
      surfaceTowers[i].z -= surfaceSpeed;
      
      // Check collision and inflict damage when towers get close (z <= 2)
      if(surfaceTowers[i].z <= 2) {
        shields -= 10; // 10% damage per tower
        score -= 10;  // Deduct 10 points when hit
        if(score < 0) score = 0;  // Don't go negative
        damageFlash = millis() + 300;  // Screen flash effect (same as space battle)
        surfaceTowers[i].active = false; // Destroy tower after collision
        createExplosion(surfaceTowers[i].x, surfaceTowers[i].y);
        
        if(shields <= 0) {
          currentState = GAME_OVER;
        }
      }
      
      // Remove towers that are too close (cleanup)
      if(surfaceTowers[i].z <= 1) {
        surfaceTowers[i].active = false;
      }
    }
  }
  
  // Spawn new towers - frequency increases more aggressively
  // From 2.5 seconds down to 1.2 seconds minimum
  int spawnDelay = max(1200, 2500 - (score * 3));
  
  if(millis() - lastRespawn > spawnDelay) {
    // Find first inactive tower slot
    for(int i = 0; i < 4; i++) {
      if(!surfaceTowers[i].active) {
        surfaceTowers[i].x = random(20, 108);
        surfaceTowers[i].y = 50;
        surfaceTowers[i].z = random(50, 150);
        surfaceTowers[i].active = true;
        surfaceTowers[i].health = 2;
        surfaceTowers[i].type = random(0, 2);
        surfaceTowers[i].lastHit = 0;
        break; // Only spawn one at a time
      }
    }
    lastRespawn = millis();
  }
}

void drawDeathStarSurface() {
  // Calculate banking effects
  float bankRadians = radians(bankingAngle);
  float bankSin = sin(bankRadians);
  
  // Base horizon with banking tilt
  int baseHorizon = 45;
  
  // Draw tilted horizon line (main Death Star surface line)
  int horizonLeft = baseHorizon - (bankSin * 10);
  int horizonRight = baseHorizon + (bankSin * 10);
  display.drawLine(0, horizonLeft, 128, horizonRight, SSD1306_WHITE);
  for(int i = 0; i < 6; i++) {
    if(backgroundShips[i].active) {
      float scale = 100.0 / (backgroundShips[i].z + 50);
      
      if(scale > 0.1 && scale < 0.5) {
        int shipX = backgroundShips[i].x;
        int shipY = backgroundShips[i].y;
        int shipSize = max(1, (int)(3 * scale));
        
        // Draw tiny ships
        if(backgroundShips[i].type == 0) {
          // X-wing (simple cross)
          display.drawLine(shipX - shipSize, shipY, shipX + shipSize, shipY, SSD1306_WHITE);
          display.drawLine(shipX, shipY - shipSize, shipX, shipY + shipSize, SSD1306_WHITE);
        } else {
          // TIE (simple H shape)
          display.drawPixel(shipX - shipSize, shipY, SSD1306_WHITE);
          display.drawPixel(shipX + shipSize, shipY, SSD1306_WHITE);
          display.drawLine(shipX - shipSize/2, shipY, shipX + shipSize/2, shipY, SSD1306_WHITE);
        }
        
        // Occasional laser flash
        if((millis() + i * 100) % 600 < 50) {
          display.drawLine(shipX, shipY, shipX + random(-3, 3), shipY + random(-2, 2), SSD1306_WHITE);
        }
      }
    }
  }
}

void drawSurfaceTowers() {
  for(int i = 0; i < 4; i++) {
    if(surfaceTowers[i].active && surfaceTowers[i].z > 0) {
      float scale = 100.0 / (surfaceTowers[i].z + 20);
      
      if(scale > 0.3) {
        int towerHeight = max(2, (int)(8 * scale));
        int towerWidth = max(1, (int)(3 * scale));
        
        // Apply banking to tower position
        int baseHorizon = 45;
        float bankRadians = radians(bankingAngle);
        float bankSin = sin(bankRadians);
        
        int towerX = surfaceTowers[i].x;
        int horizonY = baseHorizon;
        
        // Banking affects tower position
        int bankingOffset = bankSin * (horizonY - baseHorizon) * 0.3;
        int bankShiftX = bankSin * (horizonY - baseHorizon) * 0.2;
        
        towerX += bankShiftX;
        horizonY += bankingOffset;
        
        // Only draw if tower is on screen
        if(towerX >= 0 && towerX < 128) {
          // Draw tower with banking tilt
          int tiltOffset = bankSin * towerHeight * 0.2;
          
          display.drawRect(towerX - towerWidth/2, horizonY - towerHeight, 
                          towerWidth, towerHeight, SSD1306_WHITE);
          
          // Base line with tilt
          display.drawLine(towerX - towerWidth/2, horizonY, 
                          towerX + towerWidth/2 + tiltOffset, horizonY, SSD1306_WHITE);
          
          // Internal details with banking consideration
          if(scale > 0.6) {
            display.drawLine(towerX, horizonY - towerHeight, 
                           towerX + tiltOffset, horizonY, SSD1306_WHITE);
            
            if(towerHeight > 6) {
              int detail1Y = horizonY - towerHeight/3;
              int detail2Y = horizonY - 2*towerHeight/3;
              int detail1Tilt = tiltOffset * 0.33;
              int detail2Tilt = tiltOffset * 0.67;
              
              display.drawLine(towerX - towerWidth/2, detail1Y, 
                             towerX + towerWidth/2 + detail1Tilt, detail1Y, SSD1306_WHITE);
              display.drawLine(towerX - towerWidth/2, detail2Y, 
                             towerX + towerWidth/2 + detail2Tilt, detail2Y, SSD1306_WHITE);
            }
          }
          
          // Tower type indicators (unchanged)
          bool showDamage = (surfaceTowers[i].health == 1);
          bool isDamageFlashing = (millis() - surfaceTowers[i].lastHit < 500 && (millis() / 100) % 2);
          
          if(!showDamage || !isDamageFlashing) {
            switch(surfaceTowers[i].type % 2) {
              case 0: // Communications tower
                if(towerHeight > 4) {
                  display.drawLine(towerX, horizonY - towerHeight, 
                                 towerX + tiltOffset, horizonY - towerHeight - 3, SSD1306_WHITE);
                  if(scale > 0.7 && !showDamage) {
                    display.drawLine(towerX - 2, horizonY - towerHeight - 1, 
                                   towerX + 2 + tiltOffset, horizonY - towerHeight - 1, SSD1306_WHITE);
                  }
                }
                break;
                
              case 1: // Gun turret
                if(towerHeight > 3 && towerWidth > 1) {
                  display.drawLine(towerX + towerWidth/2, horizonY - towerHeight/2, 
                                 towerX + towerWidth/2 + 3 + tiltOffset, horizonY - towerHeight/2, SSD1306_WHITE);
                  if(scale > 0.7 && !showDamage) {
                    display.drawRect(towerX - 1, horizonY - towerHeight + 1, 3, 2, SSD1306_WHITE);
                  }
                }
                break;
            }
          }
          
          // Damage sparks
          if(millis() - surfaceTowers[i].lastHit < 300) {
            display.drawPixel(towerX + random(-towerWidth, towerWidth + 1), 
                            horizonY - random(0, towerHeight), SSD1306_WHITE);
          }
        }
      }
    }
  }
}

void drawDeathStarExplosion() {
  unsigned long elapsed = millis() - explosionStartTime;
  
  if(elapsed < 800) {
    // Phase 1: Death Star intact but starting to crack
    display.drawCircle(64, 32, 25, SSD1306_WHITE);
    display.drawLine(40, 32, 88, 32, SSD1306_WHITE);
    
    // Add warning cracks appearing
    if(elapsed > 400) {
      if((millis() / 80) % 2 == 0) {
        display.drawLine(64, 7, 64, 57, SSD1306_WHITE);
        display.drawLine(39, 32, 89, 32, SSD1306_WHITE);
        display.drawLine(50, 15, 78, 49, SSD1306_WHITE);
      }
    }
  } else if(elapsed < 1200) {
    // Phase 2: Initial core explosion (bright flash)
    if((millis() / 60) % 2 == 0) {
      display.fillCircle(64, 32, 30, SSD1306_WHITE);
    }
    display.drawCircle(64, 32, 35, SSD1306_WHITE);
  } else {
    // Phase 3: Massive expanding explosion with debris
    float explosionProgress = (elapsed - 1200) / 2800.0;
    
    // Bright core flash that persists
    if(explosionProgress < 0.3) {
      display.fillCircle(64, 32, 15 + explosionProgress * 20, SSD1306_WHITE);
    }
    
    // Multiple expanding shockwave rings with varied timing
    for(int ring = 0; ring < 8; ring++) {
      if(explosionProgress > ring * 0.12) {
        int baseRadius = 10 + ((explosionProgress - ring * 0.12) * 90);
        int flickerOffset = random(-2, 3);
        int radius = baseRadius + flickerOffset;
        
        // Vary the flash pattern for more chaos
        if((millis() / (40 + ring * 15)) % 2 == 0) {
          display.drawCircle(64, 32, radius, SSD1306_WHITE);
          
          // Double rings for first few waves
          if(ring < 4) {
            display.drawCircle(64, 32, radius + 3, SSD1306_WHITE);
          }
        }
      }
    }
    
    // Large debris chunks flying outward
    if(explosionProgress > 0.15) {
      for(int debris = 0; debris < 30; debris++) {
        float angle = debris * 12 + (elapsed / 15);
        float distance = (explosionProgress - 0.15) * (35 + (debris % 5) * 12);
        
        int debrisX = 64 + cos(radians(angle)) * distance;
        int debrisY = 32 + sin(radians(angle)) * distance * 0.75;
        
        if(debrisX >= 0 && debrisX < 128 && debrisY >= 8 && debrisY < 56) {
          int debrisSize = 1 + (debris % 4);
          
          // Tumbling effect - some debris flickers
          if(debris % 3 != 0 || (millis() / 100) % 2 == 0) {
            display.fillRect(debrisX, debrisY, debrisSize, debrisSize, SSD1306_WHITE);
            
            // Trail behind larger debris
            if(debrisSize > 2) {
              float trailX = debrisX - cos(radians(angle)) * 3;
              float trailY = debrisY - sin(radians(angle)) * 2;
              display.drawPixel(trailX, trailY, SSD1306_WHITE);
            }
          }
        }
      }
    }
    
    // Energy flares shooting outward
    if(explosionProgress > 0.25 && explosionProgress < 0.7) {
      for(int i = 0; i < 16; i++) {
        float angle = i * 22.5;
        float flareLength = 15 + sin(millis() * 0.01 + i) * 8;
        float distance = 25 + explosionProgress * 40;
        
        int x1 = 64 + cos(radians(angle)) * distance;
        int y1 = 32 + sin(radians(angle)) * distance * 0.75;
        int x2 = x1 + cos(radians(angle)) * flareLength;
        int y2 = y1 + sin(radians(angle)) * flareLength * 0.75;
        
        if((millis() / 80) % 2 == 0) {
          display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
        }
      }
    }
  }
}
    
void drawTrench() {
  // Draw U-shaped trench from behind/above perspective
   for(int i = 0; i < 6; i++) {
    if(trenchSegments[i].z > 0) {
      // Calculate perspective scaling
      float scale = 100.0 / (trenchSegments[i].z + 20);
      
      // Position from inside trench perspective - closer segments are lower on screen
      int screenY = map(trenchSegments[i].z, 150, 0, 25, 60);
       
      if(screenY < 55) {
        // Calculate perspective width - trench gets wider as you get closer
        int trenchWidth = 60 * scale;
        int leftWall = 64 - trenchWidth/2;
        int rightWall = 64 + trenchWidth/2;
        
        // Draw trench walls as perspective lines converging to horizon
        // Left wall extends to top of screen (horizon)
        display.drawLine(leftWall, screenY, 45, 8, SSD1306_WHITE);
        // Right wall extends to top of screen (horizon) 
        display.drawLine(rightWall, screenY, 83, 8, SSD1306_WHITE);
        
        // Draw horizontal floor line (simple and clean)
        display.drawLine(leftWall, screenY, rightWall, screenY, SSD1306_WHITE);
        
        // Add subtle wall detail lines for depth
        if(scale > 0.4) {
          // Left wall panel lines
          display.drawLine(leftWall - 5, screenY - 3, 42, 10, SSD1306_WHITE);
          // Right wall panel lines  
          display.drawLine(rightWall + 5, screenY - 3, 86, 10, SSD1306_WHITE);
        }

        // Draw vector-style 3D turrets (like 1983 Star Wars arcade)
        if(trenchSegments[i].hasObstacle) {
          int obstacleX = leftWall + (trenchSegments[i].obstacleX - 20) * scale * 0.8;
          int turretHeight = max(8, (int)(18 * scale)); // Tall but not too wide
          
          // Tapered tower - wider at bottom, narrower at top
          int bottomWidth = max(2, (int)(6 * scale)); // Reduced width
          int topWidth = max(1, (int)(3 * scale));     // Even narrower at top
          
          int baseTop = screenY - turretHeight;
          int baseBottom = screenY;
          
          // Draw tapered tower using wireframe lines only (authentic vector style)
          // Left edge - tapers from wide bottom to narrow top
          int leftBottom = obstacleX - bottomWidth/2;
          int leftTop = obstacleX - topWidth/2;
          display.drawLine(leftBottom, baseBottom, leftTop, baseTop, SSD1306_WHITE);
          
          // Right edge - tapers from wide bottom to narrow top  
          int rightBottom = obstacleX + bottomWidth/2;
          int rightTop = obstacleX + topWidth/2;
          display.drawLine(rightBottom, baseBottom, rightTop, baseTop, SSD1306_WHITE);
          
          // Top edge (narrow)
          display.drawLine(leftTop, baseTop, rightTop, baseTop, SSD1306_WHITE);
          
          // Bottom edge (wide)
          display.drawLine(leftBottom, baseBottom, rightBottom, baseBottom, SSD1306_WHITE);
          
          // Add depth with back face lines (classic vector 3D trick)
          if(scale > 0.5) {
            int depthOffset = max(1, (int)(2 * scale));
            
            // Back left edge
            display.drawLine(leftBottom - depthOffset, baseBottom - depthOffset, 
                           leftTop - depthOffset, baseTop - depthOffset, SSD1306_WHITE);
            // Back right edge
            display.drawLine(rightBottom - depthOffset, baseBottom - depthOffset, 
                           rightTop - depthOffset, baseTop - depthOffset, SSD1306_WHITE);
            // Back top edge
            display.drawLine(leftTop - depthOffset, baseTop - depthOffset, 
                           rightTop - depthOffset, baseTop - depthOffset, SSD1306_WHITE);
            
            // Connect front to back (depth lines)
            display.drawLine(leftTop, baseTop, leftTop - depthOffset, baseTop - depthOffset, SSD1306_WHITE);
            display.drawLine(rightTop, baseTop, rightTop - depthOffset, baseTop - depthOffset, SSD1306_WHITE);
            display.drawLine(leftBottom, baseBottom, leftBottom - depthOffset, baseBottom - depthOffset, SSD1306_WHITE);
          }
          
          // Gun housing - simple wireframe box on top
          int gunWidth = topWidth;
          int gunHeight = max(2, (int)(4 * scale));
          int gunTop = baseTop - gunHeight;
          
          // Gun housing wireframe (front face)
          display.drawRect(obstacleX - gunWidth/2, gunTop, gunWidth, gunHeight, SSD1306_WHITE);
          
          // Gun housing depth (if large enough)
          if(scale > 0.6) {
            int gunDepth = max(1, (int)(1.5 * scale));
            // Back face of gun housing
            display.drawRect(obstacleX - gunWidth/2 - gunDepth, gunTop - gunDepth, 
                           gunWidth, gunHeight, SSD1306_WHITE);
            // Connect corners for 3D effect
            display.drawLine(obstacleX - gunWidth/2, gunTop, 
                           obstacleX - gunWidth/2 - gunDepth, gunTop - gunDepth, SSD1306_WHITE);
            display.drawLine(obstacleX + gunWidth/2, gunTop, 
                           obstacleX + gunWidth/2 - gunDepth, gunTop - gunDepth, SSD1306_WHITE);
          }
          
          // Gun barrel - simple line with targeting
          int barrelLength = max(4, (int)(8 * scale));
          int gunCenterY = gunTop + gunHeight/2;
          
          // Calculate targeting direction (keep existing targeting logic)
          float dx = (playerX - obstacleX) * scale;
          float dy = playerY - gunCenterY;
          float dist = sqrt(dx*dx + dy*dy);
          
          if(dist > 0) {
            // Normalize direction and draw 2-pixel thick barrel
            float barrelEndX = obstacleX + (dx/dist) * barrelLength;
            float barrelEndY = gunCenterY + (dy/dist) * barrelLength;
            
            // Draw 2-pixel thick barrel using parallel lines
            display.drawLine(obstacleX, gunCenterY, barrelEndX, barrelEndY, SSD1306_WHITE);
            display.drawLine(obstacleX, gunCenterY + 1, barrelEndX, barrelEndY + 1, SSD1306_WHITE);
            
            // Optional: tiny muzzle cross for larger turrets
            if(scale > 0.7) {
              display.drawPixel(barrelEndX, barrelEndY, SSD1306_WHITE);
              display.drawPixel(barrelEndX, barrelEndY + 1, SSD1306_WHITE);
            }
          }
          
          // Add minimal structural detail lines (vector arcade style)
          if(scale > 0.6) {
            // Single center vertical line on tower
            display.drawLine(obstacleX, baseTop, obstacleX, baseBottom, SSD1306_WHITE);
            
            // Optional horizontal detail line at mid-height
            if(scale > 0.8) {
              int midY = baseTop + turretHeight/2;
              int midLeftX = obstacleX - (bottomWidth + topWidth)/4; // Interpolated width
              int midRightX = obstacleX + (bottomWidth + topWidth)/4;
              display.drawLine(midLeftX, midY, midRightX, midY, SSD1306_WHITE);
            }
          }
          
          // Keep existing firing logic
          checkTurretFiring(i, obstacleX, gunCenterY, scale);
        }
      }
    }
  }
}

  void initializeShaft() {
  for(int i = 0; i < 8; i++) {
    shaftSegments[i].z = i * 20;
    shaftSegments[i].width = 40 - (i * 2); // Gets narrower as we go deeper
    shaftSegments[i].hasObstacle = (i > 2 && random(100) < 20); // Sparse obstacles
    shaftSegments[i].obstacleX = random(-10, 10);
  }
}

void drawMissileShaft() {
  // Draw vertical shaft walls - two close parallel lines
  int shaftLeft = 60;
  int shaftRight = 68;
  
  // Draw the main shaft walls from top to bottom
  display.drawLine(shaftLeft, 8, shaftLeft, 56, SSD1306_WHITE);
  display.drawLine(shaftRight, 8, shaftRight, 56, SSD1306_WHITE);
  
  // Add depth lines that get closer together as they go down (perspective)
  for(int y = 10; y < 55; y += 8) {
    float depth = (float)(y - 10) / 45.0; // 0 to 1 as we go down
    int leftOffset = depth * 2;
    int rightOffset = depth * 2;
    
    display.drawLine(shaftLeft + leftOffset, y, shaftLeft + leftOffset, y + 6, SSD1306_WHITE);
    display.drawLine(shaftRight - rightOffset, y, shaftRight - rightOffset, y + 6, SSD1306_WHITE);
  }
  
  // Draw Death Star reactor core at the bottom
  display.drawCircle(64, 58, 12, SSD1306_WHITE);
  display.drawCircle(64, 58, 8, SSD1306_WHITE);
  display.fillCircle(64, 58, 4, SSD1306_WHITE);
  
  // Core energy rings (animated)
  if((millis() / 150) % 3 == 0) {
    display.drawCircle(64, 58, 6, SSD1306_WHITE);
  }
  if((millis() / 200) % 2 == 0) {
    display.drawCircle(64, 58, 10, SSD1306_WHITE);
  }
  
  // Single missile going down the shaft to hit the core
int missileY = 15 + (int)(shaftDepth * 0.2); // Missile travels with shaft depth
if(missileY > 50) missileY = 50; // Stop at core level
  
  // Missile blinks every 300ms
  if((millis() / 300) % 2 == 0) {
    // Missile body
    display.fillRect(62, missileY, 4, 6, SSD1306_WHITE);
    // Missile nose cone
    display.drawLine(64, missileY - 1, 64, missileY - 3, SSD1306_WHITE);
    display.drawLine(63, missileY - 1, 63, missileY - 2, SSD1306_WHITE);
    display.drawLine(65, missileY - 1, 65, missileY - 2, SSD1306_WHITE);
    
    // Exhaust trail
    for(int t = 1; t < 4; t++) {
      display.drawPixel(63 + (t % 2), missileY + 6 + t, SSD1306_WHITE);
    }
  }
}
  
void checkTurretFiring(int segmentIndex, float turretX, float turretY, float scale) {
  // Only fire if turret is reasonably close and visible
  if(scale > 0.3 && random(300) < 1) { // Low random chance each frame
    // Calculate direction to player
    float dx = playerX - turretX;
    float dy = playerY - turretY;
    float dist = sqrt(dx*dx + dy*dy);
    
    if(dist > 0 && dist < 60) { // Only fire if player is in range
      // Normalize direction and add some inaccuracy
      float accuracy = 0.8 + random(0, 40) * 0.01; // 80-120% accuracy
      float vx = (dx/dist) * 2 * accuracy;
      float vy = (dy/dist) * 2 * accuracy;
      
      // Fire projectile toward player
      fireProjectile(turretX, turretY, vx, vy, false);
    }
  }
}

void drawTrenchPlayer() {
  // Draw detailed X-Wing from behind/above view
  // Main fuselage
  display.drawLine(playerX, playerY - 4, playerX, playerY + 2, SSD1306_WHITE);
  // Wings (X formation)
  display.drawLine(playerX - 3, playerY - 1, playerX + 3, playerY + 1, SSD1306_WHITE); // Top-left to bottom-right
  display.drawLine(playerX + 3, playerY - 1, playerX - 3, playerY + 1, SSD1306_WHITE); // Top-right to bottom-left
  // Engine exhausts (4 dots at wing tips)
  display.drawPixel(playerX - 3, playerY - 1, SSD1306_WHITE);
  display.drawPixel(playerX + 3, playerY - 1, SSD1306_WHITE);
  display.drawPixel(playerX - 3, playerY + 1, SSD1306_WHITE);
  display.drawPixel(playerX + 3, playerY + 1, SSD1306_WHITE);
}

void drawSpaceBattlePlayer() {
  // Draw crosshair for targeting
  if(millis() < flashCrosshair) {
    // Rapid pulse effect while firing
    int pulseSize = 6 + ((millis() / 40) % 2) * 2;  // Alternates between 6 and 8
    
    if(hitFlash) {
      // HIT - Green/bright flash with expanding rings
      display.drawCircle(crosshairX, crosshairY, 10, SSD1306_WHITE);
      display.drawCircle(crosshairX, crosshairY, 12, SSD1306_WHITE);
      display.drawLine(crosshairX-14, crosshairY, crosshairX+14, crosshairY, SSD1306_WHITE);
      display.drawLine(crosshairX, crosshairY-14, crosshairX, crosshairY+14, SSD1306_WHITE);
      display.fillCircle(crosshairX, crosshairY, 4, SSD1306_WHITE);
      
      // "HIT" text
      display.setTextSize(1);
      display.setCursor(crosshairX - 9, crosshairY - 18);
      display.print("HIT");
    } else {
      // MISS - Pulsing crosshair shows rapid fire
      display.drawCircle(crosshairX, crosshairY, pulseSize, SSD1306_WHITE);
      display.drawLine(crosshairX-10, crosshairY, crosshairX+10, crosshairY, SSD1306_WHITE);
      display.drawLine(crosshairX, crosshairY-10, crosshairX, crosshairY+10, SSD1306_WHITE);
      display.fillCircle(crosshairX, crosshairY, 2, SSD1306_WHITE);
    }
  } else {
    // Normal crosshair
    display.drawCircle(crosshairX, crosshairY, 6, SSD1306_WHITE);
    display.drawLine(crosshairX-8, crosshairY, crosshairX+8, crosshairY, SSD1306_WHITE);
    display.drawLine(crosshairX, crosshairY-8, crosshairX, crosshairY+8, SSD1306_WHITE);
    display.drawPixel(crosshairX, crosshairY, SSD1306_WHITE);
  }
}

void drawUseTheForce() {
  // Calculate animation progress over 7 seconds
  float forceProgress = (millis() - stateTimer) / 7000.0; // 7 second duration
  if(forceProgress > 1.0) forceProgress = 1.0;

  // X-wing approaches much closer - starts far and comes very close
  float scale = 2 + (forceProgress * 15); // Scale from 2 to 17 (much larger)
  int xwingY = 32 + sin(forceProgress * PI * 2) * 2; // Slight vertical movement

  // Main fuselage nose (7-sided heptagon viewed straight on)
  int noseSize = scale * 1.2;

  // Draw heptagon nose (7-sided polygon viewed from front)
  float cx = 64, cy = xwingY;
  for(int i = 0; i < 7; i++) {
    float angle1 = (i * 2 * PI) / 7;
    float angle2 = ((i + 1) * 2 * PI) / 7;
    
    int x1 = cx + cos(angle1) * noseSize/2;
    int y1 = cy + sin(angle1) * noseSize/2;
    int x2 = cx + cos(angle2) * noseSize/2;
    int y2 = cy + sin(angle2) * noseSize/2;
    
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }

  // Fill center of nose
  display.fillCircle(64, xwingY, noseSize/3, SSD1306_WHITE);

  // X-wing configuration wings (closer together on each side)
  float wingLength = scale * 2.5;
  float wingSpread = scale * 0.4;

  // Top-left wing
  display.drawLine(64-noseSize/3, xwingY-scale*0.2, 64-wingLength, xwingY-wingSpread, SSD1306_WHITE);
  display.drawLine(64-noseSize/3-1, xwingY-scale*0.2, 64-wingLength-1, xwingY-wingSpread, SSD1306_WHITE);

  // Top-right wing
  display.drawLine(64+noseSize/3, xwingY-scale*0.2, 64+wingLength, xwingY-wingSpread, SSD1306_WHITE);
  display.drawLine(64+noseSize/3+1, xwingY-scale*0.2, 64+wingLength+1, xwingY-wingSpread, SSD1306_WHITE);

  // Bottom-left wing
  display.drawLine(64-noseSize/3, xwingY+scale*0.2, 64-wingLength, xwingY+wingSpread, SSD1306_WHITE);
  display.drawLine(64-noseSize/3-1, xwingY+scale*0.2, 64-wingLength-1, xwingY+wingSpread, SSD1306_WHITE);

  // Bottom-right wing
  display.drawLine(64+noseSize/3, xwingY+scale*0.2, 64+wingLength, xwingY+wingSpread, SSD1306_WHITE);
  display.drawLine(64+noseSize/3+1, xwingY+scale*0.2, 64+wingLength+1, xwingY+wingSpread, SSD1306_WHITE);

  // Engines near fuselage (4 small circles)
  display.fillCircle(64-noseSize/2-scale*0.1, xwingY-scale*0.3, scale*0.25, SSD1306_WHITE);
  display.fillCircle(64+noseSize/2+scale*0.1, xwingY-scale*0.3, scale*0.25, SSD1306_WHITE);
  display.fillCircle(64-noseSize/2-scale*0.1, xwingY+scale*0.3, scale*0.25, SSD1306_WHITE);
  display.fillCircle(64+noseSize/2+scale*0.1, xwingY+scale*0.3, scale*0.25, SSD1306_WHITE);

  // Wing tip guns (small circles at wing ends)
  display.fillCircle(64-wingLength, xwingY-wingSpread, scale*0.15, SSD1306_WHITE);
  display.fillCircle(64+wingLength, xwingY-wingSpread, scale*0.15, SSD1306_WHITE);
  display.fillCircle(64-wingLength, xwingY+wingSpread, scale*0.15, SSD1306_WHITE);
  display.fillCircle(64+wingLength, xwingY+wingSpread, scale*0.15, SSD1306_WHITE);

  // Engine exhaust glow
  display.drawCircle(64-noseSize/2-scale*0.1, xwingY-scale*0.3, scale*0.15, SSD1306_BLACK);
  display.drawCircle(64+noseSize/2+scale*0.1, xwingY-scale*0.3, scale*0.15, SSD1306_BLACK);
  display.drawCircle(64-noseSize/2-scale*0.1, xwingY+scale*0.3, scale*0.15, SSD1306_BLACK);
  display.drawCircle(64+noseSize/2+scale*0.1, xwingY+scale*0.3, scale*0.15, SSD1306_BLACK);

 // "USE THE FORCE" text appears after 2 seconds and positioned under X-wing
  if(forceProgress > 0.286) { // 2 seconds out of 7 = 2/7 = 0.286
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(25, 50); // Fixed position under X-wing
    display.println("USE THE FORCE");
  }
}

void drawTrenchEntry() {
  // Calculate animation progress (0.0 to 1.0 over 5 seconds)
  float progress = (millis() - stateTimer) / 5000.0;
  if(progress > 1.0) progress = 1.0;
  
  // Calculate trench dimensions that grow with progress (moved up for scope)
  int baseWidth = 12 + (progress * 30);   // Base width grows from 12 to 42
  int trenchLength = 8 + (progress * 10);  // Length grows from 8 to 18
  
  // Draw curved Death Star horizon line with opening where trench meets surface
  for(int x = 0; x < 128; x++) {
    int curveHeight = 45 + (abs(x - 64)) / 32;  // Less pronounced curve
    
    // Create opening where trench meets horizon - connect to trench sides
    // Find the trench opening edges for the current progress level
    int trenchOpeningLeft = 64 - (baseWidth/2);
    int trenchOpeningRight = 64 + (baseWidth/2);
    
    // Only draw horizon outside trench opening
    if(x < trenchOpeningLeft || x > trenchOpeningRight) {
      display.drawPixel(x, curveHeight, SSD1306_WHITE);
    }
  }
  
  // X-wing starts large and gets smaller as it approaches the trench
  // Use the same detailed front-view X-wing from "Use the Force" scene
  float scale = 15 - (progress * 12); // Scale from 15 down to 3 (reverse of Use the Force)
  int xwingX = 64;
  int xwingY = 32 + sin(progress * PI * 2) * 1; // Slight vertical movement
  
  // Main fuselage nose (7-sided heptagon viewed straight on)
  int noseSize = scale * 1.2;

  // Draw heptagon nose (7-sided polygon viewed from front)
  float cx = xwingX, cy = xwingY;
  for(int i = 0; i < 7; i++) {
    float angle1 = (i * 2 * PI) / 7;
    float angle2 = ((i + 1) * 2 * PI) / 7;
    
    int x1 = cx + cos(angle1) * noseSize/2;
    int y1 = cy + sin(angle1) * noseSize/2;
    int x2 = cx + cos(angle2) * noseSize/2;
    int y2 = cy + sin(angle2) * noseSize/2;
    
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }

  // Fill center of nose
  display.fillCircle(xwingX, xwingY, noseSize/3, SSD1306_WHITE);

  // X-wing configuration wings (closer together on each side)
  float wingLength = scale * 2.5;
  float wingSpread = scale * 0.4;

  // Top-left wing
  display.drawLine(xwingX-noseSize/3, xwingY-scale*0.2, xwingX-wingLength, xwingY-wingSpread, SSD1306_WHITE);
  display.drawLine(xwingX-noseSize/3-1, xwingY-scale*0.2, xwingX-wingLength-1, xwingY-wingSpread, SSD1306_WHITE);

  // Top-right wing
  display.drawLine(xwingX+noseSize/3, xwingY-scale*0.2, xwingX+wingLength, xwingY-wingSpread, SSD1306_WHITE);
  display.drawLine(xwingX+noseSize/3+1, xwingY-scale*0.2, xwingX+wingLength+1, xwingY-wingSpread, SSD1306_WHITE);

  // Bottom-left wing
  display.drawLine(xwingX-noseSize/3, xwingY+scale*0.2, xwingX-wingLength, xwingY+wingSpread, SSD1306_WHITE);
  display.drawLine(xwingX-noseSize/3-1, xwingY+scale*0.2, xwingX-wingLength-1, xwingY+wingSpread, SSD1306_WHITE);

  // Bottom-right wing
  display.drawLine(xwingX+noseSize/3, xwingY+scale*0.2, xwingX+wingLength, xwingY+wingSpread, SSD1306_WHITE);
  display.drawLine(xwingX+noseSize/3+1, xwingY+scale*0.2, xwingX+wingLength+1, xwingY+wingSpread, SSD1306_WHITE);

  // Engines near fuselage (4 small circles)
  display.fillCircle(xwingX-noseSize/2-scale*0.1, xwingY-scale*0.3, scale*0.25, SSD1306_WHITE);
  display.fillCircle(xwingX+noseSize/2+scale*0.1, xwingY-scale*0.3, scale*0.25, SSD1306_WHITE);
  display.fillCircle(xwingX-noseSize/2-scale*0.1, xwingY+scale*0.3, scale*0.25, SSD1306_WHITE);
  display.fillCircle(xwingX+noseSize/2+scale*0.1, xwingY+scale*0.3, scale*0.25, SSD1306_WHITE);

  // Wing tip guns (small circles at wing ends)
  display.fillCircle(xwingX-wingLength, xwingY-wingSpread, scale*0.15, SSD1306_WHITE);
  display.fillCircle(xwingX+wingLength, xwingY-wingSpread, scale*0.15, SSD1306_WHITE);
  display.fillCircle(xwingX-wingLength, xwingY+wingSpread, scale*0.15, SSD1306_WHITE);
  display.fillCircle(xwingX+wingLength, xwingY+wingSpread, scale*0.15, SSD1306_WHITE);

  // Engine exhaust glow (flickering)
  if((millis() / 150) % 2 == 0) {
    display.drawCircle(xwingX-noseSize/2-scale*0.1, xwingY-scale*0.3, scale*0.15, SSD1306_WHITE);
    display.drawCircle(xwingX+noseSize/2+scale*0.1, xwingY-scale*0.3, scale*0.15, SSD1306_WHITE);
    display.drawCircle(xwingX-noseSize/2-scale*0.1, xwingY+scale*0.3, scale*0.15, SSD1306_WHITE);
    display.drawCircle(xwingX+noseSize/2+scale*0.1, xwingY+scale*0.3, scale*0.15, SSD1306_WHITE);
  }

  // Now draw the trench with proper perspective - stationary trench opening
  // The trench should be visible immediately as X-wing approaches
  int trenchCenterX = 64;
  
  // No horizontal movement - trench stays centered
  int movementOffset = 0;
  
  // Draw 4 nested rectangles from largest (bottom) to smallest (top/horizon)
  for(int layer = 0; layer < 4; layer++) {
    // Calculate size - largest layer first, each getting 20% smaller
    float layerScale = 1.0 - (layer * 0.2);
    int layerWidth = baseWidth * layerScale;
    int layerLength = trenchLength * layerScale;
    
    // Position each layer higher - largest at bottom, smallest connects to horizon
    int layerY;
    if(layer == 3) {
      // Smallest layer connects to Death Star horizon
      layerY = 45; // Death Star horizon line
    } else {
      // Other layers step upward from bottom
      layerY = 60 - (layer * 5); // Start at Y=60, step up by 5 each layer
    }
    
    // Calculate walls with movement offset
    int leftWall = trenchCenterX - layerWidth/2 + movementOffset;
    int rightWall = trenchCenterX + layerWidth/2 + movementOffset;
    int topWall = layerY - layerLength/2;
    int bottomWall = layerY + layerLength/2;
    
    // Only draw if visible on screen
    if(leftWall < 128 && rightWall > 0 && layerWidth > 2) {
      // Draw U-shaped trench (no top line)
      display.drawLine(leftWall, topWall, leftWall, bottomWall, SSD1306_WHITE);   // Left wall
      display.drawLine(rightWall, topWall, rightWall, bottomWall, SSD1306_WHITE); // Right wall  
      display.drawLine(leftWall, bottomWall, rightWall, bottomWall, SSD1306_WHITE); // Bottom
      
      // Connect to next layer for perspective
      if(layer < 3) {
        float nextLayerScale = 1.0 - ((layer + 1) * 0.2);
        int nextLayerWidth = baseWidth * nextLayerScale;
        int nextLayerLength = trenchLength * nextLayerScale;
        
        int nextLayerY;
        if(layer + 1 == 3) {
          nextLayerY = 45; // Next layer is horizon
        } else {
          nextLayerY = 60 - ((layer + 1) * 5);
        }
        
        int nextLeftWall = trenchCenterX - nextLayerWidth/2 + movementOffset;
        int nextRightWall = trenchCenterX + nextLayerWidth/2 + movementOffset;
        int nextTopWall = nextLayerY - nextLayerLength/2;
        int nextBottomWall = nextLayerY + nextLayerLength/2;
        
        // Draw perspective lines connecting corners
        if(nextLeftWall < 128 && nextRightWall > 0) {
          display.drawLine(leftWall, topWall, nextLeftWall, nextTopWall, SSD1306_WHITE);
          display.drawLine(rightWall, topWall, nextRightWall, nextTopWall, SSD1306_WHITE);
          display.drawLine(leftWall, bottomWall, nextLeftWall, nextBottomWall, SSD1306_WHITE);
          display.drawLine(rightWall, bottomWall, nextRightWall, nextBottomWall, SSD1306_WHITE);
        }
      }
    }
  }
  
  // Add some trench details that move with the surface
if(progress > 0.4) {
  // Moving surface details - draw only left and right (skip middle i=1)
  for(int i = 0; i < 3; i++) {
    if(i == 1) continue; // Skip the middle detail
    int detailX = 30 + i * 30 + movementOffset;
    if(detailX > 10 && detailX < 118) {
      display.drawLine(detailX, 43, detailX + 4, 43, SSD1306_WHITE);
      display.drawPixel(detailX + 2, 42, SSD1306_WHITE);
    }
  }
}
  
  // Add Death Star surface details around the trench opening when big enough
  if(progress > 0.5 && baseWidth > 20) { // Fixed: use baseWidth instead of trenchWidth
    int outerLeft = trenchCenterX - baseWidth/2;
    int outerRight = trenchCenterX + baseWidth/2;
    int surfaceY = 45 + (abs(outerLeft - 64) / 64); // Match curved horizon
    
    // Surface panel lines extending outward from trench
    display.drawLine(outerLeft - 3, surfaceY, outerLeft - 8, surfaceY, SSD1306_WHITE);
    display.drawLine(outerRight + 3, surfaceY, outerRight + 8, surfaceY, SSD1306_WHITE);
    
    // Vertical surface details
    if(progress > 0.7) {
      display.drawLine(outerLeft - 5, surfaceY - 2, outerLeft - 5, surfaceY + 2, SSD1306_WHITE);
      display.drawLine(outerRight + 5, surfaceY - 2, outerRight + 5, surfaceY + 2, SSD1306_WHITE);
    }
  }
  
  // Show "ENTERING TRENCH" text
  if(progress > 0.5) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(25, 15);
    display.println("ENTERING TRENCH");
  }
  
  // Add motion lines around the shrinking X-Wing to show forward movement
  if(progress > 0.2) {
    for(int i = 0; i < 8; i++) {
      float angle = i * 45;
      float rad = radians(angle);
      int startX = xwingX + cos(rad) * (wingLength + 5);
      int startY = xwingY + sin(rad) * (wingLength + 5);
      int endX = xwingX + cos(rad) * (wingLength + 8 + progress * 6);
      int endY = xwingY + sin(rad) * (wingLength + 8 + progress * 6);
      
      display.drawLine(startX, startY, endX, endY, SSD1306_WHITE);
    }
  }
}

void drawExhaustPort() {
  // Check if crosshair is perfectly aligned and flash the port
  float accuracy = sqrt(pow(crosshairX - 64, 2) + pow(crosshairY - 40, 2));
  if(accuracy <= 3 && targetingActive && !shotFired) {
    if((millis() / 100) % 2 == 0) { // Faster flashing (100ms instead of 200ms)
      // Enhanced flashing with more concentric rings
      display.fillCircle(64, 40, 20, SSD1306_WHITE);
      display.fillCircle(64, 40, 18, SSD1306_BLACK);
      display.fillCircle(64, 40, 16, SSD1306_WHITE);
      display.fillCircle(64, 40, 14, SSD1306_BLACK);
      display.fillCircle(64, 40, 12, SSD1306_WHITE);
      display.fillCircle(64, 40, 10, SSD1306_BLACK);
      display.fillCircle(64, 40, 8, SSD1306_WHITE);
      display.fillCircle(64, 40, 6, SSD1306_BLACK);
      display.fillCircle(64, 40, 4, SSD1306_WHITE);
      display.fillCircle(64, 40, 2, SSD1306_BLACK);
      display.fillCircle(64, 40, 1, SSD1306_WHITE);
      
      // Extra outer rings that only appear during perfect targeting
      display.drawCircle(64, 40, 22, SSD1306_WHITE);
      display.drawCircle(64, 40, 24, SSD1306_WHITE);
      display.drawCircle(64, 40, 26, SSD1306_WHITE);
      display.drawCircle(64, 40, 28, SSD1306_WHITE);
    }
  }
  
  // Move exhaust port down by 8 pixels to avoid UI overlap
  int centerX = 64;
  int centerY = 40; // Changed from 32 to 40
  
  // Draw smaller central exhaust port (reduced from radius 6 to 4)
  display.drawCircle(centerX, centerY, 4, SSD1306_WHITE);
  display.fillCircle(centerX, centerY, 1, SSD1306_WHITE); // Smaller center dot
  
  // Draw concentric circles for depth
  display.drawCircle(centerX, centerY, 8, SSD1306_WHITE);
  display.drawCircle(centerX, centerY, 12, SSD1306_WHITE);
  display.drawCircle(centerX, centerY, 16, SSD1306_WHITE);
  
  // Draw radial lines from outer edge to center for depth effect
  for(int angle = 0; angle < 360; angle += 15) {
    float rad = radians(angle);
    int outerX = centerX + cos(rad) * 20;
    int outerY = centerY + sin(rad) * 20;
    int innerX = centerX + cos(rad) * 16;
    int innerY = centerY + sin(rad) * 16;
    display.drawLine(outerX, outerY, innerX, innerY, SSD1306_WHITE);
  }
  
  // Draw structural support beams (like Death Star surface details)
  display.drawLine(centerX-20, centerY, centerX+20, centerY, SSD1306_WHITE); // Horizontal beam
  display.drawLine(centerX, centerY-20, centerX, centerY+20, SSD1306_WHITE); // Vertical beam
  display.drawLine(centerX-10, centerY-10, centerX+10, centerY+10, SSD1306_WHITE); // Diagonal beam 1
  display.drawLine(centerX+10, centerY-10, centerX-10, centerY+10, SSD1306_WHITE); // Diagonal beam 2
  
  // Draw target zone indicator (flashing ring around the actual target)
  if(targetingActive && !shotFired) {
    if((millis() / 200) % 2) { // Flash every 200ms
      display.drawCircle(centerX, centerY, 6, SSD1306_WHITE);
      display.drawCircle(centerX, centerY, 7, SSD1306_WHITE);
    }
  }
  
  display.drawRect(centerX-40, centerY-10, 8, 20, SSD1306_WHITE);  // Left panel
  display.drawRect(centerX+32, centerY-10, 8, 20, SSD1306_WHITE);  // Right panel
  
  display.drawLine(centerX-36, centerY-6, centerX-36, centerY+6, SSD1306_WHITE);
  display.drawLine(centerX+36, centerY-6, centerX+36, centerY+6, SSD1306_WHITE);
  
  // Draw large countdown timer at bottom right (removed "Time: " text)
  if(targetingActive && !shotFired) {
    int timeLeft = max(0, (int)((exhaustPortTimer - millis()) / 1000));
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(110, 48); // Bottom right position
    display.print(timeLeft);
  }
}



void drawDeathStarApproach() {
  // Draw Death Star getting larger as we approach
  float starSize = map(deathStarDistance, 500, 80, 5, 40);
  
  // Main Death Star sphere - filled white
  display.fillCircle(64, 32, starSize, SSD1306_WHITE);
  
  // Surface details - more visible as we get closer
  if(starSize > 15) {
    // Superlaser dish - black crater
    display.fillCircle(64 - starSize/2, 32 - starSize/2, starSize/6, SSD1306_BLACK);
     
    // Surface panel lines - black lines on white surface
    display.drawLine(64 - starSize, 32, 64 + starSize, 32, SSD1306_BLACK); // Horizontal
   }
   
  // Trench becomes visible when moderately close
  if(starSize > 20) {
    float trenchThickness = map(starSize, 20, 40, 1, 3);
     
    // Draw trench as subtle horizontal line across Death Star
    for(int i = 0; i < trenchThickness; i++) {
      display.drawLine(64 - starSize, 32 + i - trenchThickness/2, 64 + starSize, 32 + i - trenchThickness/2, SSD1306_BLACK);
    }
  }
}

void drawTargeting() {
  // Draw crosshair for targeting
  display.drawLine(crosshairX - 8, crosshairY, crosshairX + 8, crosshairY, SSD1306_WHITE);
  display.drawLine(crosshairX, crosshairY - 8, crosshairX, crosshairY + 8, SSD1306_WHITE);
  display.drawCircle(crosshairX, crosshairY, 4, SSD1306_WHITE);
  
  // Flash effect when firing
  if(millis() < flashCrosshair) {
    display.fillCircle(crosshairX, crosshairY, 6, SSD1306_WHITE);
  }
}

bool checkCollisions() {
  // Check trench wall collisions
  for(int i = 0; i < 6; i++) {
    if(abs(trenchSegments[i].z) < 15) { // Close to player
      float scale = 200.0 / (trenchSegments[i].z + 50);
      int leftWall = 64 - (64 - trenchSegments[i].leftX) * scale;
      int rightWall = 64 + (trenchSegments[i].rightX - 64) * scale;
      
       if(playerX < leftWall + 5 || playerX > rightWall - 5 ||
         (trenchSegments[i].hasObstacle && 
          abs(playerX - (64 + (trenchSegments[i].obstacleX - 64) * scale)) < 8)) {
        return true;
      }
    }
  }
  return false;
}

void drawUI() {
  // Don't show UI during Death Star approach
  if(currentState == DEATH_STAR_APPROACH) {
    return;
  }
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Health bonus popup
  if(millis() < showHealthBonus) {
    display.setTextSize(1);
    display.setCursor(93, 14);  // Position under shields bar
    display.print("+5");
  } 
  
  // Score - moved 2 pixels left
  display.setCursor(10, 2);
  display.print("SCORE:");
  display.print(score);
  
  // Shields - percentage display (moved 3 pixels right)
  display.setCursor(73, 2);
  display.print(shields);
  display.print("%");
  
  // Draw shield bar - moved 3 pixels right
  int barWidth = map(shields, 0, 100, 0, 50);
  display.drawRect(97, 2, 22, 6, SSD1306_WHITE);
  if(barWidth > 0) {
     display.fillRect(98, 3, map(shields, 0, 100, 0, 20), 4, SSD1306_WHITE);
  }
  // Show invincibility indicator when player is invincible
  if(millis() - lastDamageTime < INVINCIBILITY_TIME) {
    if((millis() / 100) % 2 == 0) { // Flash every 100ms
      display.drawRect(91, 1, 24, 8, SSD1306_WHITE); // Flash border around shield bar
    }
}
}

void drawTitleScreen() {
  display.clearDisplay();
  
  // Animated stars
  updateStars();
  drawStars();
  
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(44, 15);  // Center "STAR" (4 chars * 12 pixels = 48, so (128-48)/2 = 40, adjust to 44)
  display.println("STAR");
  display.setCursor(44, 35);  // Center "WARS" (4 chars * 12 pixels = 48, so (128-48)/2 = 40, adjust to 44)
  display.println("WARS");
  
  // Blinking "PRESS SET"
  if((millis() / 500) % 2) {
    display.setTextSize(1);
    display.setCursor(45, 55);  // Center "PRESS A" (9 chars * 6 pixels = 54, so (128-54)/2 = 37
    display.print("PRESS A");
  }
}

void initializeWompRats() {
  for(int i = 0; i < 6; i++) {
    wompRats[i].x = random(20, 108);
    wompRats[i].y = random(-50, 10);
    wompRats[i].vx = random(-15, 16) / 10.0;
    wompRats[i].vy = random(5, 15) / 10.0;
    wompRats[i].active = true;
    wompRats[i].lastDirection = millis() + random(500, 1500);
  }
}

void initializeCanyonRocks() {
  for(int i = 0; i < 5; i++) {
    canyonRocks[i].x = random(20, 108);
    canyonRocks[i].y = random(-100, -20);
    canyonRocks[i].size = random(6, 12);
    canyonRocks[i].active = true;
  }
}

void drawWompRatTraining() {
  // Define canyon dimensions FIRST
  int topGap = 25;
  int bottomGap = 50;
  
  // Stars in the top section
  for(int i = 0; i < 70; i += 10) {
    int y = (int)(i + canyonScroll) % 70;
    if(random(100) < 30) {
      display.drawPixel(random(25, 103), y, SSD1306_WHITE);
    }
  }
  
  // Add horizontal surface detail lines on the outer land areas (outside the canyon tops)
  for(int i = 0; i < 25; i++) {
    int baseY = i * 8;
    int y = (int)(baseY + canyonScroll) % 80;
    
    if(y >= 5 && y < 64) {
      // Calculate where the outer canyon top walls are
      int leftOuterWall = (64 - topGap) * (64 - y) / 64;
      int rightOuterWall = 128 - ((64 - topGap) * (64 - y) / 64);
      
      // Left surface area - features from outer wall toward screen edge
      if(leftOuterWall > 5) {
        int lineLength = leftOuterWall / 3;
        int startX = leftOuterWall - 2;
        display.drawLine(startX, y, startX - lineLength, y, SSD1306_WHITE);
        
        // Add occasional dots
        if(i % 3 == 0 && startX - lineLength - 3 > 2) {
          display.drawPixel(startX - lineLength - 3, y, SSD1306_WHITE);
        }
      }
      
      // Right surface area - features from outer wall to screen edge
      if(rightOuterWall < 123) {
        int lineLength = (128 - rightOuterWall) / 3;
        int startX = rightOuterWall + 2;
        display.drawLine(startX, y, startX + lineLength, y, SSD1306_WHITE);
        
        // Add occasional dots
        if(i % 3 == 0 && startX + lineLength + 3 < 126) {
          display.drawPixel(startX + lineLength + 3, y, SSD1306_WHITE);
        }
      }
    }
  }
  
  // Draw canyon walls
  for(int y = 5; y < 64; y++) {
    int gapAtY = topGap + (bottomGap - topGap) * y / 64;
    int leftInnerWall = 64 - gapAtY;
    display.drawPixel(leftInnerWall, y, SSD1306_WHITE);
  }
  
  for(int y = 5; y < 64; y++) {
    int leftOuterWall = (64 - topGap) * (64 - y) / 64;
    display.drawPixel(leftOuterWall, y, SSD1306_WHITE);
  }
  
  for(int y = 5; y < 64; y++) {
    int gapAtY = topGap + (bottomGap - topGap) * y / 64;
    int rightInnerWall = 64 + gapAtY;
    display.drawPixel(rightInnerWall, y, SSD1306_WHITE);
  }
  
  for(int y = 5; y < 64; y++) {
    int rightOuterWall = 128 - ((64 - topGap) * (64 - y) / 64);
    display.drawPixel(rightOuterWall, y, SSD1306_WHITE);
  }
  
  // Vertical detail lines on left canyon wall
  for(int i = 0; i < 8; i++) {
    int lineOffset = (int)(i * 10 + canyonScroll) % 80;
    
    for(int y = 5; y < 64; y += 2) {
      int gapAtY = topGap + (bottomGap - topGap) * y / 64;
      int leftInnerWall = 64 - gapAtY;
      int leftOuterWall = (64 - topGap) * (64 - y) / 64;
      
      int lineProgress = lineOffset % 10;
      int lineX = leftOuterWall + ((leftInnerWall - leftOuterWall) * lineProgress) / 10;
      
      display.drawPixel(lineX, y, SSD1306_WHITE);
    }
  }
  
  // Vertical detail lines on right canyon wall
  for(int i = 0; i < 8; i++) {
    int lineOffset = (int)(i * 10 + canyonScroll) % 80;
    
    for(int y = 5; y < 64; y += 2) {
      int gapAtY = topGap + (bottomGap - topGap) * y / 64;
      int rightInnerWall = 64 + gapAtY;
      int rightOuterWall = 128 - ((64 - topGap) * (64 - y) / 64);
      
      int lineProgress = lineOffset % 10;
      int lineX = rightOuterWall - ((rightOuterWall - rightInnerWall) * lineProgress) / 10;
      
      display.drawPixel(lineX, y, SSD1306_WHITE);
    }
  }
  
  // Draw canyon rocks - embedded into floor with natural rock shape
  for(int i = 0; i < 5; i++) {
    if(canyonRocks[i].active && canyonRocks[i].y > 0 && canyonRocks[i].y < 64) {
      // Calculate canyon floor boundaries at this Y position
      int gapAtY = topGap + (bottomGap - topGap) * canyonRocks[i].y / 64;
      int leftFloor = 64 - gapAtY;
      int rightFloor = 64 + gapAtY;
      
      // Only draw if rock is within canyon floor boundaries
      if(canyonRocks[i].x > leftFloor + canyonRocks[i].size/2 && 
         canyonRocks[i].x < rightFloor - canyonRocks[i].size/2) {
        
        int rockX = canyonRocks[i].x;
        int rockY = canyonRocks[i].y;
        int rockSize = canyonRocks[i].size;
        
        // Draw irregular rock shape (only top half visible - embedded in floor)
        // Main rock body - irregular outline using multiple arcs
        int halfSize = rockSize / 2;
        int topHalf = halfSize * 0.7; // Only show top 70% above ground
        
        // Left side of rock (jagged)
        display.drawLine(rockX - halfSize, rockY, rockX - halfSize + 2, rockY - topHalf/2, SSD1306_WHITE);
        display.drawLine(rockX - halfSize + 2, rockY - topHalf/2, rockX - halfSize/2, rockY - topHalf, SSD1306_WHITE);
        
        // Top of rock (rounded but irregular)
        display.drawLine(rockX - halfSize/2, rockY - topHalf, rockX, rockY - topHalf - 2, SSD1306_WHITE);
        display.drawLine(rockX, rockY - topHalf - 2, rockX + halfSize/2, rockY - topHalf, SSD1306_WHITE);
        
        // Right side of rock (jagged)
        display.drawLine(rockX + halfSize/2, rockY - topHalf, rockX + halfSize - 2, rockY - topHalf/2, SSD1306_WHITE);
        display.drawLine(rockX + halfSize - 2, rockY - topHalf/2, rockX + halfSize, rockY, SSD1306_WHITE);
        
        // Bottom line (embedded in floor)
        display.drawLine(rockX - halfSize, rockY, rockX + halfSize, rockY, SSD1306_WHITE);
        
        // Add rock texture details
        if(rockSize > 8) {
          // Cracks and surface detail
          display.drawLine(rockX - 1, rockY - topHalf + 1, rockX - 2, rockY - 1, SSD1306_WHITE);
          display.drawLine(rockX + 1, rockY - topHalf + 2, rockX + 2, rockY - 1, SSD1306_WHITE);
          
          // Shadow/darker side
          display.drawPixel(rockX - halfSize + 1, rockY - 1, SSD1306_WHITE);
          display.drawPixel(rockX - halfSize + 1, rockY - 2, SSD1306_WHITE);
        }
        
        // Small highlights on top
        if(rockSize > 6) {
          display.drawPixel(rockX - 1, rockY - topHalf, SSD1306_WHITE);
          display.drawPixel(rockX + 1, rockY - topHalf - 1, SSD1306_WHITE);
        }
        
        // Smaller detail rocks next to main rock (about 20% of the time)
        if(i % 5 == 0 && rockSize > 7) {
          // Small rock to the left
          int smallX = rockX - halfSize - 3;
          int smallSize = rockSize / 4;
          if(smallX > leftFloor + 2) {
            display.drawLine(smallX - smallSize, rockY, smallX, rockY - smallSize, SSD1306_WHITE);
            display.drawLine(smallX, rockY - smallSize, smallX + smallSize, rockY, SSD1306_WHITE);
            display.drawLine(smallX - smallSize, rockY, smallX + smallSize, rockY, SSD1306_WHITE);
          }
        }
      }
    }
  }
  
  // Draw womp rats with better detail - facing downward toward spaceship
  for(int i = 0; i < 6; i++) {
    if(wompRats[i].active && wompRats[i].y > 0 && wompRats[i].y < 58) {
      // Calculate canyon floor boundaries at this Y position
      int gapAtY = topGap + (bottomGap - topGap) * wompRats[i].y / 64;
      int leftFloor = 64 - gapAtY;
      int rightFloor = 64 + gapAtY;
      
      // Only draw if rat is within canyon floor boundaries (with some margin for body size)
      if(wompRats[i].x > leftFloor + 6 && wompRats[i].x < rightFloor - 6) {
        int rx = wompRats[i].x;
        int ry = wompRats[i].y;
      
      // Body (oval shape)
      display.fillCircle(rx, ry, 3, SSD1306_WHITE);
      display.fillCircle(rx, ry - 1, 2, SSD1306_WHITE);
      display.fillCircle(rx, ry + 1, 2, SSD1306_WHITE);
      
      // Head (at bottom, facing spaceship)
      display.fillCircle(rx, ry + 4, 2, SSD1306_WHITE);
      
      // Tail (pointing upward/backward)
      display.drawLine(rx, ry - 3, rx - 1, ry - 5, SSD1306_WHITE);
      display.drawPixel(rx - 1, ry - 6, SSD1306_WHITE);
      
      // Animated legs - alternate based on time for running effect
      int legOffset = ((millis() / 150) + i) % 2; // Each rat animates independently
      
      if(legOffset == 0) {
        // Left leg forward, right leg back
        display.drawLine(rx - 2, ry + 2, rx - 3, ry + 4, SSD1306_WHITE);
        display.drawLine(rx + 2, ry + 2, rx + 1, ry + 3, SSD1306_WHITE);
      } else {
        // Right leg forward, left leg back
        display.drawLine(rx - 2, ry + 2, rx - 1, ry + 3, SSD1306_WHITE);
        display.drawLine(rx + 2, ry + 2, rx + 3, ry + 4, SSD1306_WHITE);
      }
      
      // Eyes (two dots on head)
      display.drawPixel(rx - 1, ry + 4, SSD1306_BLACK);
      display.drawPixel(rx + 1, ry + 4, SSD1306_BLACK);
      }
    }
  }
  
  // Draw T-16 ship
  int shipY = 50;
  
  display.drawLine(t16X, shipY - 7, t16X - 3, shipY + 4, SSD1306_WHITE);
  display.drawLine(t16X, shipY - 7, t16X + 3, shipY + 4, SSD1306_WHITE);
  display.drawLine(t16X - 3, shipY + 4, t16X + 3, shipY + 4, SSD1306_WHITE);
  
  for(int y = shipY - 6; y <= shipY + 3; y++) {
    int width = (y - (shipY - 7)) * 3 / 11;
    display.drawLine(t16X - width, y, t16X + width, y, SSD1306_WHITE);
  }
  
  display.drawRect(t16X - 1, shipY - 3, 2, 3, SSD1306_BLACK);
  
  display.drawLine(t16X - 3, shipY, t16X - 8, shipY, SSD1306_WHITE);
  display.drawLine(t16X - 3, shipY + 1, t16X - 8, shipY + 1, SSD1306_WHITE);
  display.drawLine(t16X - 8, shipY, t16X - 8, shipY + 1, SSD1306_WHITE);
  
  display.drawLine(t16X + 3, shipY, t16X + 8, shipY, SSD1306_WHITE);
  display.drawLine(t16X + 3, shipY + 1, t16X + 8, shipY + 1, SSD1306_WHITE);
  display.drawLine(t16X + 8, shipY, t16X + 8, shipY + 1, SSD1306_WHITE);
  
  if((millis() / 150) % 2 == 0) {
    display.drawPixel(t16X - 2, shipY + 5, SSD1306_WHITE);
    display.drawPixel(t16X + 2, shipY + 5, SSD1306_WHITE);
  }
  
  // Draw crosshair
  if(millis() < flashCrosshair) {
    int pulseSize = 6 + ((millis() / 40) % 2) * 2;
    display.drawCircle(crosshairX, crosshairY, pulseSize, SSD1306_WHITE);
    display.drawLine(crosshairX - 10, crosshairY, crosshairX + 10, crosshairY, SSD1306_WHITE);
    display.drawLine(crosshairX, crosshairY - 10, crosshairX, crosshairY + 10, SSD1306_WHITE);
    display.fillCircle(crosshairX, crosshairY, 3, SSD1306_WHITE);
  } else {
    display.drawCircle(crosshairX, crosshairY, 4, SSD1306_WHITE);
    display.drawLine(crosshairX - 6, crosshairY, crosshairX + 6, crosshairY, SSD1306_WHITE);
    display.drawLine(crosshairX, crosshairY - 6, crosshairX, crosshairY + 6, SSD1306_WHITE);
  }
  
  drawExplosions();
}

void updateWompRatTraining() {
  int topGap = 25;
  int bottomGap = 50;
  
  canyonScroll += 1.5;
  t16X = crosshairX;
  
  // Keep T-16 away from top and bottom edges
  if(crosshairY < 15) crosshairY = 15;
  if(crosshairY > 50) crosshairY = 50;
  
  for(int i = 0; i < 6; i++) {
    if(wompRats[i].active) {
      // Check distance to crosshair for evasive behavior
      float distToCrosshair = sqrt(pow(crosshairX - wompRats[i].x, 2) + 
                                   pow(crosshairY - wompRats[i].y, 2));
      
      // Faster, more aggressive evasion (HORIZONTAL ONLY)
      if(distToCrosshair < 35) { // Increased detection range from 30 to 35
        // Calculate evasion strength based on distance (closer = stronger)
        float evasionStrength = (35 - distToCrosshair) / 35.0; // 0 to 1
        evasionStrength *= 0.8; // Increased from 0.5 to 0.8 for faster reaction
        
        // Dodge horizontally away from threat
        float dx = wompRats[i].x - crosshairX;
        
        if(abs(dx) > 0.1) {
          // Normalize and apply horizontal evasion - increased multiplier
          wompRats[i].vx += (dx/abs(dx)) * evasionStrength * 1.3; // Increased from 0.8 to 1.3
          
          // Clamp horizontal velocity - increased max speed
          wompRats[i].vx = constrain(wompRats[i].vx, -2.8, 2.8); // Increased from 2.0 to 2.8
        }
      }
      
      // Ensure consistent downward movement
      wompRats[i].vy = constrain(wompRats[i].vy, 0.8, 1.5); // Always moving down
      
      // Sine wave movement for natural scurrying (smaller amplitude)
      float sineWave = sin(millis() * 0.003 + i) * 0.8;
      wompRats[i].x += wompRats[i].vx + sineWave;
      wompRats[i].y += wompRats[i].vy; // Consistent downward motion
      
      // Gradually return horizontal velocity to normal when not evading
      if(distToCrosshair > 35) {
        wompRats[i].vx *= 0.92; // Faster decay (was 0.95) to return to normal speed quicker
      }
      
      // Occasional random direction changes (horizontal only, less frequent)
      if(millis() > wompRats[i].lastDirection) {
        wompRats[i].vx += random(-10, 11) / 20.0; // Smaller adjustment
        wompRats[i].vx = constrain(wompRats[i].vx, -1.5, 1.5);
        wompRats[i].lastDirection = millis() + random(800, 2000); // Less frequent
      }
      
      // Calculate canyon floor boundaries at current Y position
      int gapAtY = topGap + (bottomGap - topGap) * wompRats[i].y / 64;
      int leftFloor = 64 - gapAtY;
      int rightFloor = 64 + gapAtY;
      
      // Keep rat within canyon floor with margin (bounce off walls)
      if(wompRats[i].x < leftFloor + 8) {
        wompRats[i].x = leftFloor + 8;
        wompRats[i].vx = abs(wompRats[i].vx) * 0.8; // Bounce but lose some energy
      }
      if(wompRats[i].x > rightFloor - 8) {
        wompRats[i].x = rightFloor - 8;
        wompRats[i].vx = -abs(wompRats[i].vx) * 0.8;
      }
      
      // Respawn rat if it goes off bottom
      if(wompRats[i].y > 64) {
        int spawnGap = topGap + (bottomGap - topGap) * (-20) / 64;
        if(spawnGap < topGap) spawnGap = topGap;
        int spawnLeft = 64 - spawnGap + 8;
        int spawnRight = 64 + spawnGap - 8;
        
        wompRats[i].x = random(spawnLeft, spawnRight);
        wompRats[i].y = random(-30, -10);
        wompRats[i].vx = random(-12, 13) / 10.0;
        wompRats[i].vy = random(8, 15) / 10.0; // Consistent downward speed
      }
    } else {
      // Respawn inactive rats occasionally
      if(random(100) < 2) {
        int spawnGap = topGap + 5;
        int spawnLeft = 64 - spawnGap + 8;
        int spawnRight = 64 + spawnGap - 8;
        
        wompRats[i].x = random(spawnLeft, spawnRight);
        wompRats[i].y = random(-30, -10);
        wompRats[i].vx = random(-12, 13) / 10.0;
        wompRats[i].vy = random(8, 15) / 10.0; // Consistent downward speed
        wompRats[i].active = true;
        wompRats[i].lastDirection = millis() + random(800, 2000);
      }
    }
  }
  
  for(int i = 0; i < 5; i++) {
    if(canyonRocks[i].active) {
      canyonRocks[i].y += 1.5;
      
      // Calculate canyon floor boundaries at rock position
      int gapAtY = topGap + (bottomGap - topGap) * canyonRocks[i].y / 64;
      int leftFloor = 64 - gapAtY;
      int rightFloor = 64 + gapAtY;
      
      // Keep rocks within canyon floor
      if(canyonRocks[i].x < leftFloor + canyonRocks[i].size) {
        canyonRocks[i].x = leftFloor + canyonRocks[i].size;
      }
      if(canyonRocks[i].x > rightFloor - canyonRocks[i].size) {
        canyonRocks[i].x = rightFloor - canyonRocks[i].size;
      }
      
      float dist = sqrt(pow(t16X - canyonRocks[i].x, 2) + 
                       pow(50 - canyonRocks[i].y, 2));
      if(dist < canyonRocks[i].size + 4) {
        shields -= 5;
        damageFlash = millis() + 200;
        canyonRocks[i].active = false;
        if(shields <= 0) {
          currentState = GAME_OVER;
        }
      }
      
      // Respawn rock if it goes off bottom
      if(canyonRocks[i].y > 70) {
        int spawnGap = topGap + 5;
        int spawnLeft = 64 - spawnGap + 6;
        int spawnRight = 64 + spawnGap - 6;
        
        canyonRocks[i].x = random(spawnLeft, spawnRight);
        canyonRocks[i].y = random(-30, -5);
        canyonRocks[i].size = random(6, 12);
        canyonRocks[i].active = true;
      }
    }
  }
}

void updateRemoteMovement() {
  unsigned long patternDuration = millis() - trainingRemote.patternStartTime;
  
  // Gradual speed increase - barely noticeable
  float speedMultiplier = 1.0 + (patternDuration / 30000.0);
  if(speedMultiplier > 1.3) speedMultiplier = 1.3;
  
  switch(trainingRemote.currentPattern) {
    case PATTERN_CIRCLE: {
      trainingRemote.angle += 0.02 * speedMultiplier;
      float radius = 20 + sin(millis() / 1000.0) * 5;
      trainingRemote.x = trainingRemote.centerX + cos(trainingRemote.angle) * radius;
      trainingRemote.y = trainingRemote.centerY + sin(trainingRemote.angle) * radius;
      
      if(patternDuration > 8000) {
        trainingRemote.currentPattern = PATTERN_ZIGZAG;
        trainingRemote.patternStartTime = millis();
        trainingRemote.vx = 1.0 * speedMultiplier;
        trainingRemote.vy = 0.8 * speedMultiplier;
        trainingRemote.zigzagDirection = 1;
      }
      break;
    }
      
    case PATTERN_DIVE: {
      float divePhase = (patternDuration % 3000) / 3000.0;
      float diveRadius;
      
      if(divePhase < 0.5) {
        diveRadius = 35 - (divePhase * 2 * 25);
      } else {
        diveRadius = 10 + ((divePhase - 0.5) * 2 * 25);
      }
      
      trainingRemote.angle += 0.015 * speedMultiplier;
      trainingRemote.x = trainingRemote.centerX + cos(trainingRemote.angle) * diveRadius * trainingRemote.diveDirection;
      trainingRemote.y = trainingRemote.centerY + sin(trainingRemote.angle) * diveRadius;
      
      if(patternDuration > 10000) {
        trainingRemote.currentPattern = PATTERN_ZIGZAG;
        trainingRemote.patternStartTime = millis();
        trainingRemote.vx = 1.0 * speedMultiplier;
        trainingRemote.vy = 0.8 * speedMultiplier;
        trainingRemote.zigzagDirection = 1;
      }
      break;
    }
      
    case PATTERN_ZIGZAG:
      trainingRemote.x += trainingRemote.vx * speedMultiplier;
      trainingRemote.y += trainingRemote.vy * speedMultiplier * trainingRemote.zigzagDirection;
      
      // Constrain first to prevent edge cases
      trainingRemote.x = constrain(trainingRemote.x, 20, 108);
      trainingRemote.y = constrain(trainingRemote.y, 15, 50);
      
      if(trainingRemote.x <= 20 || trainingRemote.x >= 108) {
        trainingRemote.vx = -trainingRemote.vx;
        trainingRemote.zigzagDirection = -trainingRemote.zigzagDirection;
      }
      if(trainingRemote.y <= 15 || trainingRemote.y >= 50) {
        trainingRemote.zigzagDirection = -trainingRemote.zigzagDirection;
      }
      
      if(patternDuration > 6000) {
        trainingRemote.currentPattern = PATTERN_RANDOM;
        trainingRemote.patternStartTime = millis();
        trainingRemote.vx = random(10, 15) / 10.0;
        trainingRemote.vy = random(8, 12) / 10.0;
      }
      break;
      
    case PATTERN_RANDOM:
      trainingRemote.x += trainingRemote.vx * speedMultiplier;
      trainingRemote.y += trainingRemote.vy * speedMultiplier;
      
      // Constrain first to prevent edge cases
      trainingRemote.x = constrain(trainingRemote.x, 20, 108);
      trainingRemote.y = constrain(trainingRemote.y, 15, 50);
      
      if(trainingRemote.x <= 20 || trainingRemote.x >= 108) {
        trainingRemote.vx = -trainingRemote.vx;
      }
      if(trainingRemote.y <= 15 || trainingRemote.y >= 50) {
        trainingRemote.vy = -trainingRemote.vy;
      }
      
      if(patternDuration > 5000) {
        trainingRemote.currentPattern = PATTERN_CIRCLE;
        trainingRemote.patternStartTime = millis();
        trainingRemote.angle = atan2(trainingRemote.y - trainingRemote.centerY, 
                                     trainingRemote.x - trainingRemote.centerX);
      }
      break;
  }
  
  // Final safety constraint - removed duplicate at end
  trainingRemote.x = constrain(trainingRemote.x, 20, 108);
  trainingRemote.y = constrain(trainingRemote.y, 15, 50);
}

void initializeBlindfolding() {
  deflectionsSuccessful = 0;
  deflectionsMissed = 0;
  
  trainingRemote.x = 64;
  trainingRemote.y = 32;
  trainingRemote.vx = 1.5;
  trainingRemote.vy = 1.0;
  trainingRemote.active = true;
  trainingRemote.nextFireTime = millis() + 3000; // 3 seconds before first attack
  trainingRemote.showPrediction = false;
  trainingRemote.currentPattern = PATTERN_CIRCLE;
  trainingRemote.patternStartTime = millis();
  trainingRemote.patternSpeed = 1.0;
  trainingRemote.centerX = 64;
  trainingRemote.centerY = 35;
  trainingRemote.angle = 0;
  trainingRemote.diveDirection = 1;
  trainingRemote.zigzagDirection = 1;
  
  blindfoldActive = false;
  dangerZoneRadius = 50.0;
  
  crosshairX = 64;
  crosshairY = 32;
}

void updateBlindfolding() {
  updateRemoteMovement();
  
  if(millis() - stateTimer > 2000) {
    blindfoldActive = true;
  }
  
  if(blindfoldActive && dangerZoneRadius > dangerZoneMinRadius) {
    dangerZoneRadius -= dangerZoneShrinkRate;
  }
  
  // Calculate warning time based on score progression
  // Score 0-50: 3 seconds
  // Score 50-100: 2 seconds
  // Score 100-150: 1.5 seconds
  // Score 150-200: 1 second
  int stageScore = score - stageStartScore;
  int warningTime;
  
  if(stageScore < 50) {
    warningTime = 3000;  // 3 seconds
  } else if(stageScore < 100) {
    warningTime = 2000;  // 2 seconds
  } else if(stageScore < 150) {
    warningTime = 1500;  // 1.5 seconds
  } else {
    warningTime = 1000;  // 1 second
  }
  
  // Warning phase - ball charges up before firing
  if(millis() > trainingRemote.nextFireTime && !trainingRemote.showPrediction) {
    trainingRemote.predictX = trainingRemote.x;
    trainingRemote.predictY = trainingRemote.y;
    trainingRemote.showPrediction = true;
    trainingRemote.predictionTime = millis();
  }
  
  // Ball fires after warning - time varies based on score
  if(trainingRemote.showPrediction && millis() - trainingRemote.predictionTime > warningTime) {
    // Ball fired at player - they failed to shoot it in time
    deflectionsMissed++;
    shields -= 10; // Lose 10% health
    damageFlash = millis() + 300;
    
    // Reset for next attack
    trainingRemote.showPrediction = false;
    trainingRemote.nextFireTime = millis() + random(2500, 4000);
    
    if(shields <= 0) {
      currentState = GAME_OVER;
    }
  }

  updateExplosions();
}

void drawBlindfolding() {
  display.clearDisplay();
  
  // Draw Millennium Falcon interior (training room)
  // Floor grid
  for(int x = 10; x < 118; x += 15) {
    display.drawLine(x, 55, x, 63, SSD1306_WHITE);
  }
  for(int y = 55; y < 64; y += 4) {
    display.drawLine(10, y, 118, y, SSD1306_WHITE);
  }
  
  // REMOVED: Wall panels (side rectangles)
  // display.drawRect(5, 10, 15, 35, SSD1306_WHITE);
  // display.drawRect(108, 10, 15, 35, SSD1306_WHITE);
  
  // Draw danger zone ring (shrinking over time)
  if(blindfoldActive) {
    for(int i = 0; i < 360; i += 15) {
      float rad = i * 3.14159 / 180.0;
      int x1 = trainingRemote.centerX + cos(rad) * dangerZoneRadius;
      int y1 = trainingRemote.centerY + sin(rad) * dangerZoneRadius;
      int x2 = trainingRemote.centerX + cos(rad + 0.2) * dangerZoneRadius;
      int y2 = trainingRemote.centerY + sin(rad + 0.2) * dangerZoneRadius;
      display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
    }
  }
  
  // ALWAYS draw the training remote as a ball/sphere
  display.drawCircle(trainingRemote.x, trainingRemote.y, 5, SSD1306_WHITE);
  display.drawCircle(trainingRemote.x, trainingRemote.y, 3, SSD1306_WHITE);
  display.drawLine(trainingRemote.x - 5, trainingRemote.y, trainingRemote.x + 5, trainingRemote.y, SSD1306_WHITE);
  display.drawLine(trainingRemote.x, trainingRemote.y - 5, trainingRemote.x, trainingRemote.y + 5, SSD1306_WHITE);
  
  if((millis() / 200) % 2 == 0) {
    display.drawPixel(trainingRemote.x, trainingRemote.y + 7, SSD1306_WHITE);
    display.drawPixel(trainingRemote.x - 1, trainingRemote.y + 8, SSD1306_WHITE);
    display.drawPixel(trainingRemote.x + 1, trainingRemote.y + 8, SSD1306_WHITE);
  }
  
  // Show warning flashes when remote is about to fire
  if(trainingRemote.showPrediction) {
    unsigned long timeSincePrediction = millis() - trainingRemote.predictionTime;
    
    // Flashing warning rings around the ball (faster as time runs out)
    int flashSpeed = 200 - (timeSincePrediction / 15); // Gets faster
    if(flashSpeed < 50) flashSpeed = 50;
    
    if((millis() / flashSpeed) % 2 == 0) {
      // Multiple warning rings
      display.drawCircle(trainingRemote.x, trainingRemote.y, 8, SSD1306_WHITE);
      display.drawCircle(trainingRemote.x, trainingRemote.y, 10, SSD1306_WHITE);
      display.drawCircle(trainingRemote.x, trainingRemote.y, 12, SSD1306_WHITE);
    }
    
    // Intensity increases as deadline approaches
    if(timeSincePrediction > 2000) {
      // Last second - very intense flashing
      if((millis() / 80) % 2 == 0) {
        display.fillCircle(trainingRemote.x, trainingRemote.y, 6, SSD1306_WHITE);
      }
    }
  }
  
  // Draw lightsaber deflection crosshair
  if(millis() < flashCrosshair) {
    // Flash when shooting
    display.drawCircle(crosshairX, crosshairY, 8, SSD1306_WHITE);
    display.drawLine(crosshairX - 10, crosshairY, crosshairX + 10, crosshairY, SSD1306_WHITE);
    display.drawLine(crosshairX, crosshairY - 10, crosshairX, crosshairY + 10, SSD1306_WHITE);
    display.fillCircle(crosshairX, crosshairY, 3, SSD1306_WHITE);
  } else {
    // Normal crosshair
    display.drawCircle(crosshairX, crosshairY, 5, SSD1306_WHITE);
    display.drawLine(crosshairX - 7, crosshairY, crosshairX + 7, crosshairY, SSD1306_WHITE);
    display.drawLine(crosshairX, crosshairY - 7, crosshairX, crosshairY + 7, SSD1306_WHITE);
  }
}

void drawForceLesson() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(5, 10);
  display.println("Remember, a Jedi can");
  
  display.setCursor(22, 22);
  display.println("feel the Force");
  
  display.setCursor(7, 34);
  display.println("flowing through him");
  
  if((millis() / 500) % 2) {
    display.setCursor(42, 57);
    display.print("PRESS A");
  }
}

void drawTrainingComplete2() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 15);
  display.println("TRAINING COMPLETE");
  
  display.setCursor(25, 30);
  display.println("READY FOR THE");
  display.setCursor(30, 40);
  display.println("REBELLION!");
  
  if((millis() / 500) % 2) {
    display.setCursor(42, 55);
    display.print("PRESS A");
  }
}

void drawTatooineSunset() {
  display.clearDisplay();
  
  // Calculate animation progress (0 to 1 over 8 seconds)
  float progress = (millis() - stateTimer) / 8000.0;
  if(progress > 1.0) progress = 1.0;
  
  // Horizon line (raised 5 pixels higher)
  int horizonY = 43;
  
  // Binary suns descending (NO RAYS)
  // Slower descent - takes full 8 seconds to set
  int sunDescentOffset = progress * 18;
  
  // Upper sun (higher in sky, moved further right with more spacing)
  int upperSunX = 113;
  int upperSunY = 15 + sunDescentOffset;
display.fillCircle(upperSunX, upperSunY, 10, SSD1306_WHITE);
  
  // Lower sun (starts fully visible, slowly descends to horizon)
int lowerSunX = 93;
int lowerSunRadius = 7;
int lowerSunY = 30 + sunDescentOffset;

// Always draw the full sun first
display.fillCircle(lowerSunX, lowerSunY, lowerSunRadius, SSD1306_WHITE);

// If sun extends below horizon, cover the bottom part with black
if(lowerSunY + lowerSunRadius >= horizonY) {
    // Draw black rectangle to cover everything below the horizon
    int coverHeight = 64 - horizonY;
    display.fillRect(0, horizonY, 128, coverHeight, SSD1306_BLACK);
}

// Distant desert mountains in background (rounded, mesa-like) - moved further left
// Mountain 1 - leftmost, rounded peak
int mtn1BaseY = horizonY - 20;
int mtn1Height = 12;

// Draw rounded mesa top (shifted left by 8 pixels total)
display.drawLine(0, mtn1BaseY, 2, mtn1BaseY - mtn1Height/2, SSD1306_WHITE);
display.drawLine(2, mtn1BaseY - mtn1Height/2, 7, mtn1BaseY - mtn1Height, SSD1306_WHITE);
display.drawLine(7, mtn1BaseY - mtn1Height, 12, mtn1BaseY - mtn1Height, SSD1306_WHITE);  // Flat top
display.drawLine(12, mtn1BaseY - mtn1Height, 17, mtn1BaseY - mtn1Height/2, SSD1306_WHITE);
display.drawLine(17, mtn1BaseY - mtn1Height/2, 22, mtn1BaseY, SSD1306_WHITE);

// Mountain 2 - center-left, also rounded
int mtn2BaseY = horizonY - 17;
int mtn2Height = 10;

display.drawLine(24, mtn2BaseY, 29, mtn2BaseY - mtn2Height/2, SSD1306_WHITE);
display.drawLine(29, mtn2BaseY - mtn2Height/2, 32, mtn2BaseY - mtn2Height, SSD1306_WHITE);
display.drawLine(32, mtn2BaseY - mtn2Height, 38, mtn2BaseY - mtn2Height, SSD1306_WHITE);  // Flat top
display.drawLine(38, mtn2BaseY - mtn2Height, 42, mtn2BaseY - mtn2Height/2, SSD1306_WHITE);
display.drawLine(42, mtn2BaseY - mtn2Height/2, 47, mtn2BaseY, SSD1306_WHITE);

// Mountain 3 - continuing the range to the right (connects to mountain 2)
int mtn3BaseY = horizonY - 15;
int mtn3Height = 8;

display.drawLine(47, mtn3BaseY, 52, mtn3BaseY - mtn3Height/2, SSD1306_WHITE);
display.drawLine(52, mtn3BaseY - mtn3Height/2, 55, mtn3BaseY - mtn3Height, SSD1306_WHITE);
display.drawLine(55, mtn3BaseY - mtn3Height, 60, mtn3BaseY - mtn3Height, SSD1306_WHITE);  // Flat top
display.drawLine(60, mtn3BaseY - mtn3Height, 64, mtn3BaseY - mtn3Height/2, SSD1306_WHITE);
display.drawLine(64, mtn3BaseY - mtn3Height/2, 69, mtn3BaseY, SSD1306_WHITE);

// Desert wind effect - animated diagonal lines
int windOffset = (millis() / 100) % 128;  // Scrolling animation
for(int i = 0; i < 8; i++) {
  int x = (windOffset + i * 20) % 128;
  int y = horizonY - 15 + (i * 3);
  
  // Draw short diagonal wind/dust lines
  if(y > 10 && y < horizonY) {
    display.drawLine(x, y, x + 8, y - 2, SSD1306_WHITE);
    
    // Add some variation
    if(i % 2 == 0) {
      display.drawLine(x + 15, y + 3, x + 21, y + 1, SSD1306_WHITE);
    }
  }
}

// Add some smaller dust particles near ground level
for(int i = 0; i < 5; i++) {
  int x = (windOffset * 2 + i * 30) % 128;
  int y = horizonY - 5 + (i % 3);
  if(x > 5 && x < 123) {
    display.drawPixel(x, y, SSD1306_WHITE);
    display.drawPixel(x + 2, y + 1, SSD1306_WHITE);
  }
}

// Luke's dome home - LEFT side, closer to viewer (below horizon) - LARGER
int domeX = 30;
int domeBaseY = horizonY + 18;  // Closer to viewer
int domeRadius = 18;  // Increased from 14 to 18

// Side sections dimensions - scaled up proportionally
int sideWidth = 10;  // Increased from 8 to 10
int sideHeight = 13;  // Increased from 10 to 13

// Left side section - sitting on same base as dome
int leftSideX = domeX - domeRadius;
display.drawRect(leftSideX - sideWidth, domeBaseY - sideHeight, sideWidth, sideHeight, SSD1306_WHITE);

// Right side section - sitting on same base as dome
int rightSideX = domeX + domeRadius;
display.drawRect(rightSideX, domeBaseY - sideHeight, sideWidth, sideHeight, SSD1306_WHITE);

// Dome should connect to top corners of side sections
int domeTopY = domeBaseY - sideHeight;

// Draw dome as semi-circle from top of side sections
for(int angle = 180; angle <= 360; angle += 2) {
  float rad = radians(angle);
  int x1 = domeX + cos(rad) * domeRadius;
  int y1 = domeTopY + sin(rad) * sideHeight;
  int x2 = domeX + cos(radians(angle + 2)) * domeRadius;
  int y2 = domeTopY + sin(radians(angle + 2)) * sideHeight;
  display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
}

// Base line of dome
display.drawLine(domeX - domeRadius, domeBaseY, domeX + domeRadius, domeBaseY, SSD1306_WHITE);

// Arched doorway entrance at front center - scaled up
int archX = domeX;
int archY = domeBaseY - 3;  // Adjusted for larger structure
int archWidth = 6;  // Increased from 5 to 6
int archHeight = 9;  // Increased from 7 to 9

// Draw arch (semi-circle doorway)
for(int angle = 180; angle <= 360; angle += 5) {
  float rad = radians(angle);
  int x = archX + cos(rad) * archWidth;
  int y = archY + sin(rad) * archHeight;
  display.drawPixel(x, y, SSD1306_WHITE);
}
// Arch sides
display.drawLine(archX - archWidth, archY - archHeight, archX - archWidth, archY, SSD1306_WHITE);
display.drawLine(archX + archWidth, archY - archHeight, archX + archWidth, archY, SSD1306_WHITE);
  
 // Draw horizon line (avoiding both dome house and Luke)
int lukeX = 75;
int lukeLeftEdge = lukeX - 5;
int lukeRightEdge = lukeX + 5;

// Calculate dome house edges based on current size
int domeLeftEdge = domeX - domeRadius - sideWidth;  // Left side section outer edge
int domeRightEdge = domeX + domeRadius + sideWidth; // Right side section outer edge

// Draw horizon in segments around dome and Luke - extended into dome by 7 pixels
display.drawLine(0, horizonY, domeLeftEdge + 8, horizonY, SSD1306_WHITE);
display.drawLine(domeRightEdge - 9, horizonY, lukeLeftEdge, horizonY, SSD1306_WHITE);
display.drawLine(lukeRightEdge, horizonY, 128, horizonY, SSD1306_WHITE);
  
  // Luke - BIGGER and MORE IN FOREGROUND, standing clearly above horizon
  int lukeY = horizonY - 6; // Positioned so feet are above horizon
  
  // Head (bigger)
  display.fillCircle(lukeX, lukeY - 10, 3, SSD1306_WHITE);
  
  // Neck
  display.drawLine(lukeX, lukeY - 7, lukeX, lukeY - 5, SSD1306_WHITE);
  display.drawLine(lukeX - 1, lukeY - 7, lukeX - 1, lukeY - 5, SSD1306_WHITE);
  display.drawLine(lukeX + 1, lukeY - 7, lukeX + 1, lukeY - 5, SSD1306_WHITE);
  
  // Body/tunic (wider)
  display.drawLine(lukeX - 2, lukeY - 5, lukeX - 2, lukeY + 2, SSD1306_WHITE);
  display.drawLine(lukeX - 1, lukeY - 5, lukeX - 1, lukeY + 2, SSD1306_WHITE);
  display.drawLine(lukeX, lukeY - 5, lukeX, lukeY + 2, SSD1306_WHITE);
  display.drawLine(lukeX + 1, lukeY - 5, lukeX + 1, lukeY + 2, SSD1306_WHITE);
  display.drawLine(lukeX + 2, lukeY - 5, lukeX + 2, lukeY + 2, SSD1306_WHITE);
  
  // Belt
  display.drawLine(lukeX - 3, lukeY + 1, lukeX + 3, lukeY + 1, SSD1306_WHITE);
  
  // Left arm (relaxed at side)
  display.drawLine(lukeX - 2, lukeY - 4, lukeX - 4, lukeY - 2, SSD1306_WHITE);
  display.drawLine(lukeX - 4, lukeY - 2, lukeX - 4, lukeY + 2, SSD1306_WHITE);
  
  // Right arm (at side)
  display.drawLine(lukeX + 2, lukeY - 4, lukeX + 4, lukeY - 2, SSD1306_WHITE);
  display.drawLine(lukeX + 4, lukeY - 2, lukeX + 4, lukeY + 2, SSD1306_WHITE);
  
  // Legs (longer)
  display.drawLine(lukeX - 1, lukeY + 2, lukeX - 2, lukeY + 8, SSD1306_WHITE);
  display.drawLine(lukeX - 2, lukeY + 2, lukeX - 3, lukeY + 8, SSD1306_WHITE);
  
  display.drawLine(lukeX + 1, lukeY + 2, lukeX + 2, lukeY + 8, SSD1306_WHITE);
  display.drawLine(lukeX + 2, lukeY + 2, lukeX + 3, lukeY + 8, SSD1306_WHITE);
  
  // Feet
  display.drawLine(lukeX - 3, lukeY + 8, lukeX - 4, lukeY + 8, SSD1306_WHITE);
  display.drawLine(lukeX + 3, lukeY + 8, lukeX + 4, lukeY + 8, SSD1306_WHITE);
  
  // Desert details
  display.drawPixel(75, horizonY, SSD1306_WHITE);
  display.drawPixel(76, horizonY - 1, SSD1306_WHITE);
  
  // "TATOOINE" text after 2 seconds
  if(progress > 0.25) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(36, 2);
    display.print("TATOOINE");
  }
}

void drawTrainingComplete() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 15);
  display.println("TRAINING COMPLETE");
  
  display.setCursor(25, 30);
  display.println("READY FOR THE");
  display.setCursor(30, 40);
  display.println("REBELLION!");
  
  if((millis() / 500) % 2) {
    display.setCursor(42, 55);
    display.print("PRESS A");
  }
}

void drawVictory() {
  display.clearDisplay();
  
  // Explosion effect
  for(int i = 0; i < 10; i++) {
    int x = 64 + random(-30, 30);
    int y = 32 + random(-20, 20);
    display.fillCircle(x, y, random(1, 4), SSD1306_WHITE);
  }
  
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 5);
  display.println("DEATH");
  display.setCursor(5, 25);
  display.println("STAR");
  display.setCursor(0, 45);
  display.println("DESTROYED!");
  
}

void drawCockpitWindow() {
  // X-wing cockpit frame - expanded viewing area with optional banking
  
  // Apply banking tilt during Death Star surface battle
  float leftTilt = 0, rightTilt = 0;
  if(currentState == DEATH_STAR_SURFACE) {
    // Match the exact same banking calculation as the Death Star surface
    float bankRadians = radians(bankingAngle);
    float bankSin = sin(bankRadians);
    
    // Apply same tilt pattern as Death Star horizon: left side down, right side up when banking right
    leftTilt = -(bankSin * 8);  // Scale down from 10 to 8 for cockpit
    rightTilt = (bankSin * 8);
  }
  
  // Top frame lines with banking applied (same direction as Death Star surface)
  display.drawLine(5, 0 + leftTilt, 25, 54 + leftTilt, SSD1306_WHITE);   // Top left 
  display.drawLine(123, 0 + rightTilt, 103, 54 + rightTilt, SSD1306_WHITE); // Top right 
  
  // Bottom horizontal frame (tilted with banking)
  display.drawLine(25, 54 + leftTilt, 103, 54 + rightTilt, SSD1306_WHITE);
  
  // Side braces - with banking tilt
  display.drawLine(0, 64, 25, 54 + leftTilt, SSD1306_WHITE);   // Bottom left
  display.drawLine(128, 64, 103, 54 + rightTilt, SSD1306_WHITE); // Bottom right
  
  // Control panel - tilted with banking (adjust Y positions)
  int panelLeftY = 54 + leftTilt;
  int panelRightY = 54 + rightTilt;
  int panelHeight = 8;
  
  // Draw control panel as trapezoid when tilted
  if(currentState == DEATH_STAR_SURFACE && (abs(leftTilt) > 1 || abs(rightTilt) > 1)) {
    // Draw tilted control panel as filled polygon approximation
    for(int x = 25; x <= 103; x++) {
      float progress = (float)(x - 25) / (103 - 25);
      int topY = panelLeftY + (panelRightY - panelLeftY) * progress;
      int bottomY = topY + panelHeight;
      display.drawLine(x, topY, x, bottomY, SSD1306_WHITE);
    }
  } else {
    // Normal rectangular control panel
    display.fillRect(25, 54, 78, 8, SSD1306_WHITE);
  }
  
  // Control panel details (adjust positions based on tilt)
  int detailBaseY = 55 + ((leftTilt + rightTilt) * 0.25); // Average of left and right tilt
  
  // Left section - navigation controls
  display.drawRect(28, detailBaseY, 10, 6, SSD1306_BLACK);
  display.drawLine(30, detailBaseY + 1, 36, detailBaseY + 1, SSD1306_BLACK);
  display.drawLine(33, detailBaseY, 33, detailBaseY + 6, SSD1306_BLACK);
  display.fillRect(31, detailBaseY + 3, 2, 2, SSD1306_BLACK);
  
  // Center section - targeting computer (larger)
  display.drawRect(55, detailBaseY, 18, 6, SSD1306_BLACK);
  display.drawLine(57, detailBaseY + 1, 71, detailBaseY + 1, SSD1306_BLACK);
  display.drawLine(59, detailBaseY + 3, 69, detailBaseY + 3, SSD1306_BLACK);
  display.fillRect(63, detailBaseY, 2, 2, SSD1306_BLACK);
  display.drawLine(60, detailBaseY + 4, 68, detailBaseY + 4, SSD1306_BLACK);
  
  // Right section - weapon controls
  display.drawRect(85, detailBaseY, 12, 6, SSD1306_BLACK);
  display.drawLine(87, detailBaseY + 1, 95, detailBaseY + 1, SSD1306_BLACK);
  display.fillRect(88, detailBaseY + 3, 2, 2, SSD1306_BLACK);
  display.fillRect(92, detailBaseY + 3, 2, 2, SSD1306_BLACK);
  display.drawLine(89, detailBaseY + 4, 93, detailBaseY + 4, SSD1306_BLACK);
}

void initializeTurrets() {
  for(int i = 0; i < 4; i++) {
    turrets[i].x = random(25, 103);
    turrets[i].y = random(15, 35);
    turrets[i].z = random(80, 200);
    turrets[i].active = true;
    turrets[i].lastFire = millis() + random(1000, 3000);
    turrets[i].health = 2;
  }
}

void updateTurrets() {
  for(int i = 0; i < 4; i++) {
    if(turrets[i].active) {
      turrets[i].z -= trenchSpeed;
      
      // Respawn turret if it goes behind player
      if(turrets[i].z <= 0) {
        turrets[i].x = random(25, 103);
        turrets[i].y = random(15, 35);
        turrets[i].z = random(150, 250);
        turrets[i].lastFire = millis() + random(1000, 3000);
        turrets[i].health = 2;
      }
      
      // Turret firing
      if(millis() > turrets[i].lastFire && turrets[i].z < 100) {
        float dx = playerX - turrets[i].x;
        float dy = playerY - turrets[i].y;
        float dist = sqrt(dx*dx + dy*dy);
        
        if(dist > 0) {
          fireProjectile(turrets[i].x, turrets[i].y, 
                        (dx/dist) * 2, (dy/dist) * 2, false);
        }
        turrets[i].lastFire = millis() + random(2000, 4000);
      }
    }
  }
}

void drawTurrets() {
  for(int i = 0; i < 4; i++) {
    if(turrets[i].active && turrets[i].z > 0) {
      float scale = 100.0 / turrets[i].z;
      int screenX = turrets[i].x;
      int screenY = turrets[i].y + (turrets[i].z / 8);
      
      if(screenX >= 0 && screenX < 128 && screenY >= 0 && screenY < 64) {
        int size = scale * 3;
        if(size < 2) size = 2;
        
        // Draw turret base
        display.fillRect(screenX - size, screenY - size/2, size*2, size, SSD1306_WHITE);
        
        // Draw turret gun pointing at player
        display.drawLine(screenX, screenY, 
                        screenX + (playerX - screenX) * 0.1, 
                        screenY + (playerY - screenY) * 0.1, SSD1306_WHITE);
      }
    }
  }
}

void spawnPowerUp(float x, float y, int type) {
  for(int i = 0; i < 3; i++) {
    if(!powerUps[i].active) {
      powerUps[i].x = x;
      powerUps[i].y = y;
      powerUps[i].z = 50;
      powerUps[i].vx = random(-10, 11) / 10.0; // Random float velocity -1.0 to 1.0
      powerUps[i].vy = random(-10, 11) / 10.0;
      powerUps[i].type = type;
      powerUps[i].active = true;
      powerUps[i].spawnTime = millis();
      break;
    }
  }
}

void updatePowerUps() {
  for(int i = 0; i < 3; i++) {
    if(powerUps[i].active) {
      // Different movement for space battle vs trench run
      if(currentState == SPACE_BATTLE || currentState == DEATH_STAR_APPROACH) {
        // Float around in space battle
        powerUps[i].x += powerUps[i].vx;
        powerUps[i].y += powerUps[i].vy;
        
        // Bounce off screen edges
        if(powerUps[i].x < 15 || powerUps[i].x > 113) powerUps[i].vx = -powerUps[i].vx;
        if(powerUps[i].y < 15 || powerUps[i].y > 45) powerUps[i].vy = -powerUps[i].vy;
      } else {
        powerUps[i].z -= trenchSpeed * 0.8; // Move slower than trench
      }
      
      // Remove if too old (4 seconds in space, 8 seconds in trench) or behind player
      int timeout = (currentState == SPACE_BATTLE || currentState == DEATH_STAR_APPROACH) ? 4000 : 8000;
      if(powerUps[i].z <= 0 || millis() - powerUps[i].spawnTime > timeout) {
        powerUps[i].active = false;
        continue;
      }
      
      // Check player collection (use crosshair in space battle, player position in trench)
      float checkX = (currentState == SPACE_BATTLE || currentState == DEATH_STAR_APPROACH) ? crosshairX : playerX;
      float checkY = (currentState == SPACE_BATTLE || currentState == DEATH_STAR_APPROACH) ? crosshairY : playerY;
      float dist = sqrt(pow(checkX - powerUps[i].x, 2) + 
       pow(checkY - powerUps[i].y, 2));
      if(dist < 8) {
        // Apply power-up effect
        switch(powerUps[i].type) {
          case 0: // Shield boost
            shields += 25;
            if(shields > 100) shields = 100;
            break;
          case 1: // Rapid fire
            rapidFire = true;
            rapidFireEnd = millis() + 5000;
            break;
        }
        powerUps[i].active = false;
        score += 20;
      }
    }
  }
}

void drawGameOver() {
  display.clearDisplay();
  
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(12, 10);
  display.println("GAME OVER");
  
  display.setTextSize(1);
  
  // Calculate score text width to center it
  // "SCORE: " is 7 characters (42 pixels at size 1)
  // Each digit is 6 pixels wide at text size 1
  int scoreDigits = (score == 0) ? 1 : 0;
  int tempScore = score;
  while(tempScore > 0) {
    scoreDigits++;
    tempScore /= 10;
  }
  
  int scoreTextWidth = 42 + (scoreDigits * 6); // "SCORE: " + digits
  int scoreCursorX = (128 - scoreTextWidth) / 2; // Center on 128px screen
  
  display.setCursor(scoreCursorX, 50);
  display.print("SCORE: ");
  display.print(score);
  
  }


void drawPowerUps() {
  for(int i = 0; i < 3; i++) {
    if(powerUps[i].active) {
      int screenX = powerUps[i].x;
      int screenY = powerUps[i].y + (powerUps[i].z / 8);
      
      if(screenX >= 0 && screenX < 128 && screenY >= 0 && screenY < 64) {
        // Flash effect - blink every 200ms to be visible
        bool isFlashing = (millis() / 200) % 2 == 0;
        
        // Draw different power-up types
        switch(powerUps[i].type) {
          case 0: // Shield power-up
            if(isFlashing) {
              // Large flashing outline
              display.drawRect(screenX-4, screenY-4, 8, 8, SSD1306_WHITE);
              display.drawRect(screenX-3, screenY-3, 6, 6, SSD1306_WHITE);
              display.fillRect(screenX-1, screenY-1, 2, 2, SSD1306_WHITE);
            } else {
              // Smaller inner square
              display.drawRect(screenX-3, screenY-3, 6, 6, SSD1306_WHITE);
              display.drawPixel(screenX, screenY, SSD1306_WHITE);
            }
            break;
          case 1: // Rapid fire (triangle)
            display.drawLine(screenX, screenY-3, screenX-3, screenY+2, SSD1306_WHITE);
            display.drawLine(screenX-3, screenY+2, screenX+3, screenY+2, SSD1306_WHITE);
            display.drawLine(screenX+3, screenY+2, screenX, screenY-3, SSD1306_WHITE);
            break;
          case 2: // Extra life (cross)
            display.drawLine(screenX-3, screenY, screenX+3, screenY, SSD1306_WHITE);
            display.drawLine(screenX, screenY-3, screenX, screenY+3, SSD1306_WHITE);
            break;
        }
      }
    }
  }
}
int readButtonValue() {
    return analogRead(BUTTON_INPUT_PIN);
}

bool isBtnUp(int val) {
    return (val >= 38 && val <= 40);
}

bool isBtnDown(int val) {
    return (val >= 44 && val <= 47);
}

bool isBtnLeft(int val) {
    return (val >= 74 && val <= 75);
}

bool isBtnRight(int val) {
    return (val >= 141 && val <= 143);
}

bool isBtnSet(int val) {
    return (val >= 200 && val <= 300);
}

bool isBtnB(int val) {
    return (val >= 1020 && val <= 1023);
}
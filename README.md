# Maze_Solver
This project implements an AI agent that autonomously plays a multi-stage maze game by analyzing a vision grid and deciding the next move. The AI uses different strategies depending on the current stage, such as wall-following, target-seeking, and item-collection behaviors.

Project Structure
GameAI/
├── brain.cpp       # Core AI logic for decision-making
├── brain.h         # Header file for Brain class (not shown)
└── (other files)   # Game engine files (not shown)

Features:
Right-Hand Wall Following: Used in maze-solving and exploratory stages.
Left-Hand Spiral Navigation: Prevents infinite loops in spiral-like maps.
Target Collection: Detects and gathers items like food ('0') and flags ('A', 'B').
Stage-Aware Behavior: Resets memory and adapts strategy per stage.
Vision-Based Movement: Reads a local grid around the player to navigate.
Memory of Visited Tiles: Avoids revisiting collected items.

Game Stages & AI Behavior:
Stage	   Goal	                                     Strategy
0	       Solve a basic maze	                       Right-hand wall-following
1	       Collect all food items ('0')	             Seek & collect, then exit
2	       Navigate spiral structures	               Left-hand wall-following
3	       Capture flag A, place at B, exit at D	   Seek targets sequentially
4+	     General exploration	                     Right-hand wall-following

How It Works:
Vision Parsing: The AI reads a 2D grid showing visible tiles.
Facing Direction: It synchronizes its internal facing direction with the player symbol.
Stage Awareness: On stage change, memory is reset.
Target Seeking: For certain stages, it searches for specific objects (like doors, flags, food).
Wall Following: Falls back to wall-hugging when exploration is needed.
Visited Tracking: Uses a hash set to remember visited positions.

Example Functions:
getNextMove(const GameState &state): Main decision function returning next move.

exploreWallFollow(const GameState &state): Wall-following exploration.

gatherFood(const GameState &state): Seeks and collects visible food tiles.

captureFlag(const GameState &state): Executes flag capture & placement logic.

getVisibleTargets(const GameState &state, char target): Locates visible targets on grid.

Controls Mapping
Direction Index	   Direction	  Action Code
0	                 Up	          1
1	                 Right	      4
2	                 Down	        3
3	                 Left	        2

Requirements
C++17 or later

C++ Standard Library (<vector>, <unordered_set>, <cmath>, <cstdint>, etc.)

This project is open-source. Feel free to use and modify under the MIT License.

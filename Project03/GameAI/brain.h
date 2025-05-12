// File: GameAI/brain.h
#ifndef BRAIN_H
#define BRAIN_H

#include "../Game/game.h"
#include <vector>
#include <unordered_set>
#include <utility>
#include <cstdint>

class Brain {
public:
    Brain();
    int getNextMove(const GameState &state);

private:
    int facing_dir;                   // 0=up,1=right,2=down,3=left
    int last_stage;
    std::unordered_set<int64_t> visited;

    // Stage 1 door
    bool doorKnown; int doorH, doorW;

    // Stage 4 (flag capture) memory
    bool aKnown, pickedFlag, bKnown, placedFlag, door4Known;
    int  aH, aW, bH, bW, door4H, door4W;

    // Strategies
    int solveMaze(const GameState &state);
    int solveSpiral(const GameState &state);
    int gatherFood(const GameState &state);
    int captureFlag(const GameState &state);
    int exploreWallFollow(const GameState &state);

    // Helpers
    static int dirToAction(int dir);
    std::vector<std::pair<int,int>> getVisibleTargets(const GameState &state, char target);
    int toAbsDir(int dh, int dw) const;
};

#endif // BRAIN_H

// File: GameAI/brain.cpp
#include "brain.h"
#include <vector>
#include <cmath>
#include <cstdint>

// up, right, down, left
static constexpr int drow[4] = { -1, 0, 1,  0 };
static constexpr int dcol[4] = {  0, 1, 0, -1 };

// Find the player's marker in vision ('^','>','v','<')
static std::pair<int,int> findVisionCenter(const std::vector<std::vector<char>> &V) {
    for (int i = 0; i < (int)V.size(); ++i)
        for (int j = 0; j < (int)V[i].size(); ++j)
            if (V[i][j]=='^'||V[i][j]=='>'||V[i][j]=='v'||V[i][j]=='<')
                return {i,j};
    int R = V.size(), C = R?V[0].size():0;
    return {R/2, C/2};
}

// True if the cell in direction dir from the vision center isn't a wall '+' or closed door 'D'
static bool isFree(const GameState &st, int dir) {
    const auto &V = st.vision;
    int R = V.size(); if (!R) return false;
    int C = V[0].size();
    auto [rC,cC] = findVisionCenter(V);
    int nr = rC + drow[dir], nc = cC + dcol[dir];
    if (nr<0||nr>=R||nc<0||nc>=C) return false;
    char ch = V[nr][nc];
    return ch!='+' && ch!='D';
}

Brain::Brain()
  : facing_dir(1),
    last_stage(-1),
    doorKnown(false), doorH(-1), doorW(-1),
    aKnown(false), pickedFlag(false),
    bKnown(false), placedFlag(false),
    door4Known(false), door4H(-1), door4W(-1)
{}

int Brain::getNextMove(const GameState &state) {
    // Sync facing_dir
    auto [rC,cC] = findVisionCenter(state.vision);
    char sym = state.vision[rC][cC];
    if      (sym=='^') facing_dir=0;
    else if (sym=='>') facing_dir=1;
    else if (sym=='v') facing_dir=2;
    else if (sym=='<') facing_dir=3;

    // New stage? Reset memories
    if (state.stage != last_stage) {
        visited.clear();
        last_stage = state.stage;
        doorKnown = false;
        aKnown = pickedFlag = false;
        bKnown = placedFlag = false;
        door4Known = false;
    }

    // Stage 1: remember door 'D'
    if (state.stage==1 && !doorKnown) {
        auto ds = getVisibleTargets(state,'D');
        if (!ds.empty()) {
            doorKnown=true;
            doorH=ds[0].first;
            doorW=ds[0].second;
        }
    }

    // Stage 4: before captureFlag, we also remember A, B, D coords
    if (state.stage==3) {
        if (!aKnown) {
            auto as = getVisibleTargets(state,'A');
            if (!as.empty()) {
                aKnown = true;
                aH = as[0].first;
                aW = as[0].second;
            }
        }
        if (pickedFlag && !bKnown) {
            auto bs = getVisibleTargets(state,'B');
            if (!bs.empty()) {
                bKnown = true;
                bH = bs[0].first;
                bW = bs[0].second;
            }
        }
        if (placedFlag && !door4Known) {
            auto ds = getVisibleTargets(state,'D');
            if (!ds.empty()) {
                door4Known = true;
                door4H = ds[0].first;
                door4W = ds[0].second;
            }
        }
    }

    // Choose strategy
    int dir;
    switch (state.stage) {
      case 0: dir = solveMaze(state);    break;
      case 1: dir = gatherFood(state);   break;
      case 2: dir = solveSpiral(state);  break;
      case 3: dir = captureFlag(state);  break;
      default: dir = exploreWallFollow(state);
    }

    // Mark visited
    {
      int64_t key = (int64_t(state.pos[0])<<32) | int64_t(uint32_t(state.pos[1]));
      visited.insert(key);
    }

    return dirToAction(dir);
}

int Brain::solveMaze(const GameState &state) {
    return exploreWallFollow(state);
}

int Brain::solveSpiral(const GameState &state) {
    // Left‑hand wall‑hug: left, straight, right, back
    int L=(facing_dir+3)%4, S=facing_dir, R=(facing_dir+1)%4, B=(facing_dir+2)%4;
    int order[4]={L,S,R,B};
    for(int d:order){
        if(isFree(state,d)){
            facing_dir=d;
            return d;
        }
    }
    // U‑turn if blocked
    facing_dir=B;
    return B;
}

int Brain::gatherFood(const GameState &state) {
    auto targets = getVisibleTargets(state,'0');
    if (!targets.empty()) {
        for (auto &p : targets) {
            int64_t k=(int64_t(p.first)<<32)|int64_t(uint32_t(p.second));
            if (!visited.count(k)) {
                return toAbsDir(p.first-state.pos[0], p.second-state.pos[1]);
            }
        }
        auto [h,w]=targets.front();
        return toAbsDir(h-state.pos[0], w-state.pos[1]);
    }
    // no food: go to door then wall‑hug
    if (doorKnown && (state.pos[0]!=doorH || state.pos[1]!=doorW)) {
        return toAbsDir(doorH-state.pos[0], doorW-state.pos[1]);
    }
    return exploreWallFollow(state);
}

int Brain::captureFlag(const GameState &state) {
    int h = state.pos[0], w = state.pos[1];

    // 1) Before picking up A: wall‑hug until A is in view, then go to it
    if (!pickedFlag) {
        auto as = getVisibleTargets(state, 'A');
        if (!as.empty()) {
            auto [ah, aw] = as.front();
            if (h == ah && w == aw) {
                pickedFlag = true;
            } else {
                int d = toAbsDir(ah - h, aw - w);
                if (isFree(state, d)) return d;
            }
        }
        return exploreWallFollow(state);
    }

    // 2) Before placing at B: wall‑hug until B is in view, then go to it
    if (!placedFlag) {
        auto bs = getVisibleTargets(state, 'B');
        if (!bs.empty()) {
            auto [bh, bw] = bs.front();
            if (h == bh && w == bw) {
                placedFlag = true;
            } else {
                int d = toAbsDir(bh - h, bw - w);
                if (isFree(state, d)) return d;
            }
        }
        return exploreWallFollow(state);
    }

    // 3) After placing B: wall‑hug until D is in view, then go through it
    auto ds = getVisibleTargets(state, 'D');
    if (!ds.empty()) {
        auto [dh, dw] = ds.front();
        if (h != dh || w != dw) {
            int d = toAbsDir(dh - h, dw - w);
            if (isFree(state, d)) return d;
        }
    }
    return exploreWallFollow(state);
}


int Brain::exploreWallFollow(const GameState &state) {
    int R=(facing_dir+1)%4, S=facing_dir, L=(facing_dir+3)%4, B=(facing_dir+2)%4;
    int order[4]={R,S,L,B};
    for(int d:order){
        if(isFree(state,d)){
            facing_dir=d;
            return d;
        }
    }
    facing_dir=B;
    return B;
}

int Brain::dirToAction(int dir) {
    static const int M[4]={1,4,3,2};
    return M[dir&3];
}

std::vector<std::pair<int,int>>
Brain::getVisibleTargets(const GameState &state, char target) {
    std::vector<std::pair<int,int>> out;
    const auto &V = state.vision;
    int R = V.size(), C = R?V[0].size():0;
    auto [rC,cC] = findVisionCenter(V);
    for(int i=0;i<R;++i){
        for(int j=0;j<C;++j){
            if (V[i][j]==target) {
                out.emplace_back(state.pos[0]-rC+i,
                                 state.pos[1]-cC+j);
            }
        }
    }
    return out;
}

int Brain::toAbsDir(int dh, int dw) const {
    if (std::abs(dh) > std::abs(dw))
        return dh<0 ? 0 : 2;
    else
        return dw<0 ? 3 : 1;
}



//g++ -std=c++20 -Wall main.cpp Game/game.cpp Game/player.cpp Game/enemy.cpp GameAI/brain.cpp -o game
//./game -testvisual


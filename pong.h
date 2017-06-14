#pragma once

#include <functional>

#include "geometry.h"

namespace pong {

    
    class Pong {
        struct State {
            std::function<bool(const SDL_Event& e)> onEvent;
            std::function<void(float dt)> update;
            std::function<void(SDL_Surface* surf)> draw;
        };
    public:
        Pong();
        bool onEvent(const SDL_Event& e);
        bool startEvent(int player, const SDL_Event& e);
        bool padMoves(const SDL_Event& e);
        bool gameEvent(const SDL_Event& e);
        void update(float dt);
        void integrateMoves(float dt);
        void gameUpdate(float dt);
        void startUpdate(int player, float dt);
        void draw(SDL_Surface* surf) const;
        void gameDraw(SDL_Surface* surf) const;
        Pong& operator<<(SDL_Event& e);
        Pong& operator<<(float dt);
    private:
        void setGamingState();
        void setStartState(int player);
        void drawNumber(SDL_Surface* surf, const vec2& pos, int num) const;
        State mState;
        SpeedRect mBall;
        SpeedRect mLeftPad;
        SpeedRect mRightPad;
        rect mTop,mBottom,mRight,mLeft;
        SDL_Surface* mBackground;
        SDL_Surface* mNumbers;
        int mP1Points,mP2Points;
    };

    inline SDL_Surface* operator<<(SDL_Surface* s, const Pong& p) {
        p.draw(s);
        return s;
    }

}

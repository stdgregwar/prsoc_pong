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
        bool startEvent(const SDL_Event& e);
        bool gameEvent(const SDL_Event& e);
        void update(float dt);
        void gameUpdate(float dt) const;
        void startUpdate(float dt) const;
        void draw(SDL_Surface* surf) const;
        void gameDraw(SDL_Surface* surf) const;
        Pong& operator<<(SDL_Event& e);
        Pong& operator<<(float dt);
    private:
        State mState;
        SpeedRect mBall;
        SDL_Surface* mBackground;
    };

    inline SDL_Surface* operator<<(SDL_Surface* s, const Pong& p) {
        p.draw(s);
        return s;
    }

}

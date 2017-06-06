#include "pong.h"

namespace pong {
    using namespace std;
    using namespace std::placeholders;
    Pong::Pong() : mState{bind(&Pong::startEvent,this,_1),bind(&Pong::startUpdate,this,_1),bind(&Pong::gameDraw,this,_1)}{
    }

    bool Pong::onEvent(const SDL_Event& e) {
        return mState.onEvent(e);
    }

    bool Pong::startEvent(const SDL_Event& e) {
        return true;
    }

    bool Pong::gameEvent(const SDL_Event& e) {
        return true;
    }

    void Pong::update(float dt) {
        mState.update(dt);
    }

    void Pong::gameUpdate(float dt) const {
    }

    void Pong::startUpdate(float dt) const{
    }

    void Pong::draw(SDL_Surface* surf) const {
        mState.draw(surf);
    }

    void Pong::gameDraw(SDL_Surface* surf) const {
    }

    Pong& Pong::operator<<(SDL_Event& e) {
        onEvent(e);
        return *this;
    }

    Pong& Pong::operator<<(float dt) {
        update(dt);
        return *this;
    }
}

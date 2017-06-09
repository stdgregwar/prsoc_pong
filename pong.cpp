#include "pong.h"

#include <stdexcept>

#include "utils.h"

namespace pong {
    using namespace std;
    using namespace std::placeholders;
    Pong::Pong() : mState{bind(&Pong::startEvent,this,_1),bind(&Pong::startUpdate,this,_1),bind(&Pong::gameDraw,this,_1)}{
        //Setup background
        using namespace fb;
        mBackground = SDL_CreateRGBSurface(SDL_SWSURFACE,winWidth,winHeight,32,rmask,gmask,bmask,amask);
        if(!mBackground) {
            throw std::runtime_error("Failed to create background surface");
        }
        SDL_Rect whole{0,0,winWidth,winHeight};
        SDL_FillRect(mBackground,&whole,0); //Fill surface in black
        SDL_Rect centerLine{winWidth/2-1,0,2,winHeight};
        SDL_FillRect(mBackground,&centerLine,0xffffffff); //Draw white line in the middle

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
        SDL_BlitSurface(mBackground,NULL,surf,NULL); //Draw background
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

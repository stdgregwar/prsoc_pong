#include "pong.h"

#include <stdexcept>

#include "utils.h"

constexpr uint32_t solidWhite = 0xffffffff;
constexpr float padSize = 60;
constexpr float ballSpeed = 200;
constexpr float bounce = -1.1f;

namespace pong {
    using namespace std;
    using namespace std::placeholders;
    using namespace fb;
    Pong::Pong() :
        mState{bind(&Pong::startEvent,this,1,_1),bind(&Pong::startUpdate,this,1,_1),bind(&Pong::gameDraw,this,_1)},
        mBall{{{0.f,0.f},{8.f,8.f}},{100.f,100.f}},
        mLeftPad{{{20,winHeight/2},{8,padSize}},{0,0}},
        mRightPad{{{winWidth-20,winHeight/2},{8,padSize}},{0,0}},
        mTop{{-20,-20},{winWidth+40,20}},
        mBottom{{-20,winHeight},{winWidth+40,20}},
        mRight{{winWidth,-20},{20,winHeight+40}},
        mLeft{{-20,-20},{20,winHeight+40}},
        mP1Points(0),mP2Points(0)
    {
        //Setup background
        mBackground = SDL_CreateRGBSurface(SDL_SWSURFACE,winWidth,winHeight,32,rmask,gmask,bmask,amask);
        mNumbers = IMG_Load("numbers.png");
        if(!mNumbers) {
            throw std::runtime_error("Failed to load  numbers");
        }
        if(!mBackground) {
            throw std::runtime_error("Failed to create background surface");
        }
        SDL_Rect whole{0,0,winWidth,winHeight};
        SDL_FillRect(mBackground,&whole,0); //Fill surface in black
        SDL_Rect centerLine{winWidth/2-1,0,2,winHeight};
        SDL_FillRect(mBackground,&centerLine,solidWhite); //Draw white line in the middle
    }

    bool Pong::onEvent(const SDL_Event& e) {
        return mState.onEvent(e);
    }


    bool Pong::padMoves(const SDL_Event& e) {
        constexpr float speedFac = 1.f/100;
        if(e.type != SDL_JOYAXISMOTION) return true;
        const SDL_JoyAxisEvent& je = (const SDL_JoyAxisEvent&)e;
        switch(je.axis) {
        case 0:
            //mRightPad.speed.x = float(je.value)*speedFac;
            break;
        case 1:
            mRightPad.speed.y = float(je.value)*speedFac;
            break;
        case 2:
            //mLeftPad.speed.x = float(je.value)*speedFac;
            break;
        case 3:
            mLeftPad.speed.y = float(je.value)*speedFac;
            break;
        default:
            break;
        }
        return true;
    }

    bool Pong::startEvent(int player, const SDL_Event& e) {
        padMoves(e);
        if(e.type != SDL_JOYAXISMOTION) return true;
        const SDL_JoyAxisEvent& je = (const SDL_JoyAxisEvent&)e;
        if(player == 1) {
            if(je.axis == 0 and je.value < -32768/2) {
                mBall.speed.x = -ballSpeed;
                mBall.speed.y = 0.5*mRightPad.speed.y;
                mBall.size.pos.x -= 4;
                setGamingState();
            }
        } else {
            if(je.axis == 2 and je.value > 32768/2) {
                mBall.speed.x = ballSpeed;
                mBall.speed.y = 0.5*mLeftPad.speed.y;
                mBall.size.pos.x += 4;
                setGamingState();
            }
        }
        return true;
    }

    void Pong::setGamingState() {
        mState.onEvent = std::bind(&Pong::gameEvent,this,_1);
        mState.draw = std::bind(&Pong::gameDraw,this,_1);
        mState.update = std::bind(&Pong::gameUpdate,this,_1);
    }

    bool Pong::gameEvent(const SDL_Event& e) {
        padMoves(e);
        return true;
    }

    void Pong::integrateMoves(float dt) {
        mBall.integrate(dt);
        mRightPad.integrate(dt);
        mLeftPad.integrate(dt);
    }


    void Pong::update(float dt) {
        mState.update(dt);
    }

    void Pong::gameUpdate(float dt) {
        integrateMoves(dt);
        //Bounces
        rect ballRect = mBall.sweptRect(dt);
        if(ballRect.intersect(mTop) || ballRect.intersect(mBottom)) {
            mBall.speed.y *= -1;
        }
        if(ballRect.intersect(mRightPad.sweptRect(dt))) {
            mBall.speed.x *= bounce;
            mBall.speed.y +=mRightPad.speed.y*0.5;
        }
        else if(ballRect.intersect(mLeftPad.sweptRect(dt))) {
            mBall.speed.x *= bounce;
            mBall.speed.y += mLeftPad.speed.y*0.5;
        }
        else if(ballRect.intersect(mRight)) { //One point to player 2
            mP2Points++;
            setStartState(1);
        }
        else if(ballRect.intersect(mLeft)) { //One point to player 1
            mP1Points++;
            setStartState(2);
        }
    }

    void Pong::setStartState(int player) {
        mState.update = std::bind(&Pong::startUpdate,this,player,_1);
        mState.onEvent = std::bind(&Pong::startEvent,this,player,_1);
        mState.draw = std::bind(&Pong::gameDraw,this,_1);
    }

    void Pong::startUpdate(int player, float dt) {
        integrateMoves(dt);
        mBall.speed = {0,0};

        switch(player) {
        case 1:
            mBall.size.pos = mRightPad.size.pos+vec2{-8,padSize/2-4};
            break;
        case 2:
            mBall.size.pos = mLeftPad.size.pos+vec2{8,padSize/2-4};
            break;
        default: break;
        }
    }

    void Pong::drawNumber(SDL_Surface* surf, const vec2& pos, int num) const {
        SDL_Rect src{Sint16(num*32),0,32,32};
        SDL_Rect dest{Sint16(pos.x),Sint16(pos.y),32,32};

        SDL_BlitSurface(mNumbers,&src,surf,&dest);
    }

    void Pong::draw(SDL_Surface* surf) const {
        mState.draw(surf);
    }

    void Pong::gameDraw(SDL_Surface* surf) const {
        SDL_BlitSurface(mBackground,NULL,surf,NULL); //Draw background
        SDL_Rect rb = mBall.size;
        SDL_Rect rlp = mLeftPad.size;
        SDL_Rect rrp = mRightPad.size;
        SDL_FillRect(surf,&rb,solidWhite);
        SDL_FillRect(surf,&rlp,solidWhite);
        SDL_FillRect(surf,&rrp,solidWhite);
        drawNumber(surf,{winWidth/2+2,2},mP1Points);
        drawNumber(surf,{winWidth/2-34,2},mP2Points);
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

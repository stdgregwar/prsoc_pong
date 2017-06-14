#pragma once

#include <SDL/SDL.h>

namespace pong {

    constexpr int winWidth = 480;
    constexpr int winHeight = 272;


    template<typename T>
    struct vec2_t {
        T x,y;
	vec2_t(){}
	vec2_t(T tx, T ty): x(tx), y(ty){}
	inline T getX() const {return x;}
	inline T getY() const {return x;}
        vec2_t operator*(T s) const {return vec2_t{x*s,y*s};}
        vec2_t operator+(const vec2_t& o) const {return vec2_t{o.x+x,o.y+y};}
        vec2_t& operator+=(const vec2_t& o) {
            x+=o.x;
            y+=o.y;
            return *this;
        }
        bool operator==(const vec2_t& o) const {return o.x == x and o.y == y;}
    };

    template <typename T>
    struct rect_t {
        vec2_t<T> pos,size;
        bool contains(const vec2_t<T>& v) const {
            return pos.x < v.x && pos.x+size.x > v.x && pos.y < v.y && pos.y+size.y > v.y;
        }
        operator SDL_Rect() const {
            return SDL_Rect{pos.x,pos.y,(Uint16)size.x,(Uint16)size.y};
        }
        bool operator==(const rect_t& o) const {
            return tie(pos,size) == tie(o.pos,o.size);
        }
    };

    template <typename T>
    auto rectFromCenter(const vec2_t<T>& pos, const vec2_t<T>& size) -> rect_t<T> {
        return rect_t<T>{pos.x - size/2.f,pos.y-size/2.f,size.x,size.y};
    }

    template <typename T>
    auto rectFromPoint(const vec2_t<T>& pos, const vec2_t<T>& size) -> rect_t<T> {
        return rect_t<T>{pos,size};
    }

    typedef vec2_t<float> vec2;
    typedef rect_t<float> rect;

    struct SpeedRect {
        rect size;
        vec2 speed;
        inline vec2 pos() {return size.pos;}
        inline const rect getSize() const {return size;}
        inline bool intersect(SpeedRect& other){
            return this->size.contains(other.pos()) ||
                this->size.contains(other.pos() + vec2(other.size.size.getX(), 0.0)) ||
                this->size.contains(other.pos() + vec2(0.0,other.size.size.getY())) ||
                this->size.contains(other.pos() + vec2(other.size.size.getX(), other.size.size.getY()));   
        }
    };

}

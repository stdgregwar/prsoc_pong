#pragma once
#include <iostream>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <assert.h>
#include <sys/mman.h>
#ifdef DENANO
#include <linux/fb.h>
#endif
#include <unistd.h>
#include <map>

#include "geometry.h"
#include "joysticks/joysticks.h"

namespace fb {

    constexpr uint32_t bmask = 0xff000000;
    constexpr uint32_t gmask = 0x00ff0000;
    constexpr uint32_t rmask = 0x0000ff00;
    constexpr uint32_t amask = 0x000000ff;

    /**
       Fills screen and format according to framebuffer specs

       It just hijacks the mmaped framebuffer to make SDL think it's a regular surface
       (which it becomes in fact)
    **/
    SDL_Surface* setupFramebuffer();
}

namespace pong {

    using namespace std;

    template<class K, class V>
    inline auto getOrElse(const map<K,V>& m,const K& key, const V& def) -> const V& {
        auto it = m.find(key);
        if(it != m.end()) return it->second;
        return def;
    }


    namespace joysticks {
        constexpr float joyRange = 32767;

        void getKeyJoyEvent(SDL_Event& e, float press);

        #ifdef DENANO
        bool getHardJoystickEvent(SDL_Event& e);
        #endif

        bool pollFakeEvents(SDL_Event& e);
    }
}


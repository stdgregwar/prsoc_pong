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
#include <linux/fb.h>
#include <unistd.h>
#include <unordered_map>

#include "geometry.h"
#include "joysticks/joysticks.h"


namespace fb {

    struct fb_fix_screeninfo fix_info;
    struct fb_var_screeninfo var_info;
    void *frame_buffer;
    int fb_fd;
    SDL_Surface screen = {}; //Init both hacked surface and format to null
    SDL_PixelFormat format = {};

    /**
       Fills screen and format according to framebuffer specs

       It just hijacks the mmaped framebuffer to make SDL think it's a regular surface
       (which it becomes in fact)
    **/
    SDL_Surface* setupFramebuffer() {
        fb_fd = open("/dev/fb0", O_RDWR);
        assert(fb_fd >= 0);

        // Get screen information
        int ret = ioctl(fb_fd, FBIOGET_FSCREENINFO, &fix_info);
        assert(ret >= 0);

        ret = ioctl(fb_fd, FBIOGET_VSCREENINFO, &var_info);
        assert(ret >= 0);

        // Map the frame buffer in user memory
        frame_buffer = mmap(NULL, var_info.yres_virtual * fix_info.line_length, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
        assert(frame_buffer != MAP_FAILED);

        screen.flags = SDL_SWSURFACE;
        screen.format = &format;
        format.BitsPerPixel = 32;
        format.BytesPerPixel = 4;
        //Let losses be 0
        format.Bmask = 0xff000000;
        format.Gmask = 0x00ff0000;
        format.Rmask = 0x0000ff00;
        //Let a mask be 0
        format.Bshift = var_info.blue.offset;
        format.Gshift = var_info.green.offset;
        format.Rshift = var_info.red.offset;
        format.alpha = 255;
        screen.w = var_info.xres;
        screen.h = var_info.yres;
        SDL_Rect clipRect = {0,0,(Uint16)screen.w,(Uint16)screen.h};
        screen.clip_rect = clipRect;
        screen.refcount = 1; //Won't be GC anyway...
        screen.pixels = frame_buffer; //Let the surface draw directly to the backbuffer
        screen.pitch = screen.w*4;
        return &screen;
    }

}

namespace pong {

    using namespace std;

    template<class K, class V>
    inline auto getOrElse(const unordered_map<K,V>& m,const K& key, const V& def) -> const V& {
        auto it = m.find(key);
        if(it != m.end()) return it->second;
        return def;
    }


    namespace joysticks {

        using KeyMap = std::unordered_map<SDLKey,std::tuple<float,Uint8>>; // <Key,<Val,Axis>>

        KeyMap p1map = {{SDLK_UP,{-1,1}},{SDLK_DOWN,{1,1}},
                        {SDLK_LEFT,{-1,0}},{SDLK_RIGHT,{1,0}}};
        KeyMap p2map = {{SDLK_w,{-1,3}},{SDLK_s,{1,3}},
                        {SDLK_a,{-1,2}},{SDLK_d,{1,2}}};

        std::array<float,4> vJoyAxises{{0,0,0,0}};
        constexpr float joyRange = 32767;

        joysticks_dev jdev;

        inline void getKeyJoyEvent(SDL_Event& e, float press) {
            e.type = SDL_JOYAXISMOTION; //Transform event in joystick move
            Uint8 axis;
            float val;
            tie(val,axis) = getOrElse(p1map,e.key.keysym.sym,getOrElse(p2map,e.key.keysym.sym,{0,5}));
            e.jaxis.which = 0; //Always 0 for emulated joystick
            e.jaxis.axis = axis;
            if(axis > 3) return; //Don't do anything if out-of bound axis
            vJoyAxises[axis] += val*press;
            e.jaxis.value = vJoyAxises[axis]*joyRange;
        }

        #ifdef DENANO
        std::array<float,4> hardState{0,0,0,0};
        inline bool getHardJoystickEvent(SDL_Event& e) {
            std::array<float,4> current{{
                joysticks_read_left_horizontal(&jdev)/(float)JOYSTICKS_MAX_VALUE,
                joysticks_read_left_vertical(&jdev)/(float)JOYSTICKS_MAX_VALUE,
                joysticks_read_right_horizontal(&jdev)/(float)JOYSTICKS_MAX_VALUE,
                joysticks_read_right_vertical(&jdev)/(float)JOYSTICKS_MAX_VALUE,
            }};
            for(Uint8 i = 0; i < 4; i++) {
                if(current[i] != hardState[i]) {
                    hardState[i] = current[i];
                    e.type = SDL_JOYAXISMOTION;
                    e.jaxis.axis = i;
                    e.jaxis.value = hardState[i]*joyRange;
                    return true;
                }
            }
            return false;
        }
        #endif

        inline bool pollFakeEvents(SDL_Event& e) {
            #ifdef DENANO
            if(getHardJoystickEvent(e)) return true;
            #endif
            if(!SDL_PollEvent(&e)) return false; //Get real event
            switch(e.type) {
            case SDL_KEYDOWN: //If event is of type key : emulate joystick
                getKeyJoyEvent(e,1);
                break;
            case SDL_KEYUP:
                getKeyJoyEvent(e,-1);
                break;
            default: break; //Don't do anything to other events
            }
            return true;
        }
    }
}


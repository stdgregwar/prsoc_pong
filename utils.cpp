#include <array>
#include <map>

#include "utils.h"

namespace fb {
#ifdef DENANO
  struct fb_fix_screeninfo fix_info;
  struct fb_var_screeninfo var_info;
#endif
  void *frame_buffer;
  int fb_fd;
  SDL_Surface screen = {}; //Init both hacked surface and format to null
  SDL_PixelFormat format = {};
  int current_buf = 0;
  int num_buffers;
  int num_pixels_per_buffer;
  SDL_Surface* setupFramebuffer() {
#ifndef DENANO
    return nullptr;
#else
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

    num_buffers = (var_info.yres_virtual * var_info.xres_virtual) / (var_info.xres * var_info.yres);
    num_pixels_per_buffer = var_info.yres * var_info.xres;

    screen.flags = SDL_SWSURFACE;
    screen.format = &format;
    format.BitsPerPixel = 32;
    format.BytesPerPixel = 4;
    //Let losses be 0
    format.Bmask = bmask;
    format.Gmask = gmask;
    format.Rmask = rmask;
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

    var_info.yoffset = 0 * var_info.yres;
    ret = ioctl(fb_fd, FBIOPAN_DISPLAY, &var_info);
    assert(ret >= 0);

    return &screen;
#endif
  }

  void swapBuffer() {

    //Let currently diplayed buffer be the filled buffer know
    void* buffer = frame_buffer+current_buf*num_pixels_per_buffer*4;
    screen.pixels = buffer;

    //Swap buffer index
    current_buf = current_buf == 1 ? 0 : 1;

    //Tell driver to draw swapped buffer
    var_info.yoffset = current_buf * var_info.yres;
    int ret = ioctl(fb_fd, FBIOPAN_DISPLAY, &var_info);
    assert(ret >= 0);
  }

}

namespace pong {
  namespace joysticks {
    joysticks_dev jdev;
    constexpr off_t baseLiteBusAddr = 0xFF200000;
    constexpr size_t busSize = 1024*1024*2; //2Mo bus size
    constexpr off_t mcpOff = 0x000000e0; //Offset of the joysticks ADC
    void init() {
      int mem_fd = open("/dev/mem",O_RDWR | O_SYNC);
      void* bus = mmap(nullptr,busSize,PROT_READ|PROT_WRITE,MAP_SHARED,mem_fd,baseLiteBusAddr); //Map base bus address
      void* mcp3204 = (void*)((uintptr_t)bus + mcpOff);
      jdev = joysticks_inst(mcp3204);
      joysticks_init(&jdev);
    }
    using KeyMap = std::map<SDLKey,std::tuple<float,Uint8>>; // <Key,<Val,Axis>>
        
    KeyMap p1map = {{SDLK_UP,std::tuple<float,Uint8>(-1,1)},
                    {SDLK_DOWN,std::tuple<float,Uint8>(1,1)},
                    {SDLK_LEFT,std::tuple<float,Uint8>(-1,0)},
                    {SDLK_RIGHT,std::tuple<float,Uint8>(1,0)}};
    KeyMap p2map = {{SDLK_w,std::tuple<float,Uint8>(-1,3)},
                    {SDLK_s,std::tuple<float,Uint8>(1,3)},
                    {SDLK_a,std::tuple<float,Uint8>(-1,2)},
                    {SDLK_d,std::tuple<float,Uint8>(1,2)}};
        
    std::array<float,4> vJoyAxises{{0,0,0,0}};
    inline void getKeyJoyEvent(SDL_Event& e, float press) {
      e.type = SDL_JOYAXISMOTION; //Transform event in joystick move
      Uint8 axis;
      float val;
      tie(val,axis) = getOrElse(p1map,e.key.keysym.sym,getOrElse(p2map,e.key.keysym.sym,std::tuple<float,Uint8>(0,5)));
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
          -(float(joysticks_read_right_horizontal(&jdev))-2048)/(float)(JOYSTICKS_MAX_VALUE/2),
            (float(joysticks_read_right_vertical(&jdev))-2048)/(float)(JOYSTICKS_MAX_VALUE/2),
            (float(joysticks_read_left_horizontal(&jdev))-2048)/(float)(JOYSTICKS_MAX_VALUE/2),
            (float(joysticks_read_left_vertical(&jdev))-2048)/(float)(JOYSTICKS_MAX_VALUE/2),
            }};
      for(Uint8 i = 0; i < 4; i++) {
        if(abs(current[i] - hardState[i]) > 0.01) {
          hardState[i] = current[i];
          e.type = SDL_JOYAXISMOTION;
          e.jaxis.axis = i;
          e.jaxis.value = -hardState[i]*joyRange;
          return true;
        }
      }
      return false;
    }
#endif
        
    bool pollFakeEvents(SDL_Event& e) {
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

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

namespace fb {

struct fb_fix_screeninfo fix_info;
struct fb_var_screeninfo var_info;
void *frame_buffer;
int fb_fd;
SDL_Surface screen = {0}; //Init both hacked surface and format to null
SDL_PixelFormat format = {0};

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

namespace joysticks {

}

/*int main(void) {
	cout << "Init SDL" << endl;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD);
    SDL_Surface fb = {0}; //Init both hacked surface and format to null
    SDL_PixelFormat pf = {0};
    setupScreen(fb,pf);
    SDL_Surface* image=IMG_Load("test.jpg");
    SDL_BlitSurface(image,0,&fb,0);
	return 0;
}*/

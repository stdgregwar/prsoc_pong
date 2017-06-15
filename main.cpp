#include <string>
#include <stdexcept>

#include "utils.h"
#include "pong.h"


using namespace std;
using namespace pong;

int main(int argc, char* argv[]){
    
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD | SDL_INIT_JOYSTICK); //Init SDL
    IMG_Init(IMG_INIT_PNG);
    SDL_Surface* screen;
    if(argc > 1 && string(argv[1]) == "fb") { //Switch to fb based hack
        screen = fb::setupFramebuffer();
    } else { //Create regular SDL_screen
        screen = SDL_SetVideoMode(winWidth,winHeight,16,SDL_HWSURFACE);
    }
    if(!screen) throw std::runtime_error("could not setup screen");
    SDL_Surface* image=IMG_Load("test.jpg");
    if(!image) throw std::runtime_error("could not load image");
    SDL_BlitSurface(image,0,screen,0);
    SDL_FreeSurface(image);
    #ifdef DENANO
    pong:joysticks::init(); //Init joysticks
    #endif
    //TODO add mainloop
    {
        bool quit = false;
        Pong po;
        while(!quit) {
            SDL_Event event;
            while (pong::joysticks::pollFakeEvents(event)) {
                if(event.type == SDL_QUIT) quit = true;
                po << event;
            }
            //window.clear();
            SDL_FillRect(screen, NULL, 0x000000);
            po << 1.f/60; //Fixed time delta
            screen << po;
            if(SDL_Flip(screen) == -1) break;
            SDL_Delay(1000/60);
        }
    }
    
    return 0;
}

#include <string>
#include <stdexcept>

#include "utils.h"

constexpr int winWidth = 480;
constexpr int winHeight = 272;

using namespace std;

int main(int argc, char* argv[]) {

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD); //Init SDL
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
    //TODO add mainloop

    return 0;
}

#include <SDL_main.h>

#include <vk_engine.h>

int main(int argc, char** argv)
{
  vulkan::engine engine;

  engine.init();
  engine.run();
  engine.cleanup();

  return 0;
}

#ifndef ENGINE_H
#define ENGINE_H

#include "SDL3/SDL_render.h"
#include <SDL3/SDL.h>
#include <stdint.h>

typedef enum {
	SUCCESS,
	FAILED_TO_INIT_SDL,
	FAILED_TO_CREATE_WINDOW,
	FAILED_TO_CREATE_RENDERER,
	FAILED_TO_MALLOC,
	FAILED_TO_REMOVE_RECT_FROM_RENDER_QUEUE,
	FAILED_TO_LOAD_IMAGE,
	DATA_IS_NULL,
	POSITION_HIGHER_THAN_QUEUE_LENGTH,
	FAILED_TO_FIND_TYPE,
	POSITION_CANT_BE_ZERO,
} ENG_RESULT;

typedef struct {
	int r;
	int g;
	int b;
	int a;
} eng_Color;

typedef struct {
	uint32_t h;
	uint32_t w;
	uint32_t x;
	uint32_t y;
	eng_Color *color;
} eng_Rect;

typedef enum {
	ENG_QUIT,
	ENG_EVENT_KEY_DOWN,
	ENG_KEYBOARD_KEY_UP,
	ENG_MOUSE_BUTTON,
	ENG_MOUSE_MOVE,
} ENG_TYPE;

typedef enum {
	ENG_KEY_0 = '0',	
	ENG_KEY_1,	
	ENG_KEY_2,	
	ENG_KEY_3,	
	ENG_KEY_4,	
	ENG_KEY_5,	
	ENG_KEY_6,	
	ENG_KEY_7,	
	ENG_KEY_8,	
	ENG_KEY_9,	

	ENG_KEY_A = 'A',
	ENG_KEY_B,
	ENG_KEY_C,
	ENG_KEY_D,
	ENG_KEY_E,
	ENG_KEY_F,
	ENG_KEY_G,
	ENG_KEY_H,
	ENG_KEY_I,
	ENG_KEY_J,
	ENG_KEY_K,
	ENG_KEY_L,
	ENG_KEY_M,
	ENG_KEY_N,
	ENG_KEY_O,
	ENG_KEY_P,
	ENG_KEY_Q,
	ENG_KEY_R,
	ENG_KEY_S,
	ENG_KEY_T,
	ENG_KEY_U,
	ENG_KEY_V,
	ENG_KEY_W,
	ENG_KEY_X,
	ENG_KEY_Y,
	ENG_KEY_Z,
} ENG_KEYS;

typedef enum {
	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,
} ENG_BUTTONS;

typedef struct {
	ENG_TYPE type;
	uint32_t value;
} Event;

typedef struct {
	float x;
	float y;
} Mouse;

struct Window {
	SDL_Window *pWindow;
	SDL_Renderer *pRenderer;
	uint32_t width;
	uint32_t height;
	SDL_Texture background;
};

typedef struct {
	struct Window window;
	bool isRunning;
	Event event;
	Mouse mouse;
} Application;

typedef struct {
	uint32_t h;
	uint32_t w;
	uint32_t x;
	uint32_t y;
	SDL_Texture *texture;
} eng_Texture;

ENG_RESULT eng_init(bool debug);
Application *eng_createApplication(const char *title, const uint32_t width, const uint32_t height);
void eng_render(Application *app);
bool eng_pollEvent(Application *app, uint32_t fps);
ENG_RESULT eng_addRectToRenderQueue(eng_Rect *rect);
void eng_quit(Application *app);
eng_Rect *eng_createRect(uint32_t h, uint32_t w, uint32_t x, uint32_t y, eng_Color color);
ENG_RESULT eng_removeFromRenderQueue(void *data);
eng_Texture *eng_addImageToRenderQueue(struct Window *pWindow, const char *path, uint32_t h, uint32_t w, uint32_t x, uint32_t y);
ENG_RESULT eng_moveToQueuePosition(void *data, int position);
Mouse eng_getMousePosition();

const char *eng_getError();

#endif
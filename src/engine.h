#ifndef ENGINE_H
#define ENGINE_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
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
	FAILED_TO_OPEN_FONT,
	FAILED_TO_CREATE_FONT_RENDER,
	FAILED_TO_CONVERT_FONT_TO_TEXTURE,
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
	TTF_Font *font;
} Application;

typedef struct {
	uint32_t h;
	uint32_t w;
	uint32_t x;
	uint32_t y;
	SDL_Texture *texture;
} eng_Texture;

/*
* Used to initialize the engine, MUST be called before anything else dealing with the engine. The debug is persistent and prints out results of various functions
*/
ENG_RESULT eng_init(bool debug);

/*
* Used to create the Application struct, this creates the window and some other aspects
*/
Application *eng_createApplication(const char *title, const uint32_t width, const uint32_t height);

/*
* This renders the render queue, it should be called in the game loop
*/

void eng_render(Application *app);
/* This polls for events and sets the fps, if fps is set to 0 the framerate is unlocked
*/
bool eng_pollEvent(Application *app, uint32_t fps);

/*
* adds a eng_Rect struct to the render queue
*/
ENG_RESULT eng_addRectToRenderQueue(eng_Rect *rect);

/*
* This quits the engine, this will free the engine queue will be automatically freed
*/
void eng_quit(Application *app);

/*
* This will create a new eng_Rect struct and automatically add it to the render queue. This returns a pointer to the actual rect in the queue so any changes will be reflected in the render queue
*/
eng_Rect *eng_createRect(uint32_t h, uint32_t w, uint32_t x, uint32_t y, eng_Color color);

/*
* This will remove anything that is provided from the render queue, an error will be returned if the location is not found in the render queue
*/
ENG_RESULT eng_removeFromRenderQueue(void *data);

/*
* This adds an image to the render queue
*/
eng_Texture *eng_addImageToRenderQueue(struct Window *pWindow, const char *path, uint32_t h, uint32_t w, uint32_t x, uint32_t y);

/*
* This moves the pointer to a specific position in the queue, -1 will move to the end of the queue
*/
ENG_RESULT eng_moveToQueuePosition(void *data, int position);

/*
* This gets the mouse position and returns that value
*/
Mouse eng_getMousePosition();

/*
* Adds text to the render queue, it can be removed with eng_removeFromRenderQueue()
*/
eng_Texture *eng_addFontToRenderQueue(struct Window *window, const char *font, uint32_t fontSize, const char *text, eng_Color color, uint32_t h, uint32_t w, uint32_t x, uint32_t y);

/*
* Returns weather two positions collide, you can set the width and height of 1 for the cursor
*/
bool isTouching(uint32_t firstX, uint32_t firstY, uint32_t firstH, uint32_t firstW, uint32_t secondX, uint32_t secondY, uint32_t secondH, uint32_t secondW);

/*
* This returns the error, it's just a switch case with every error being stored in a static enum before exiting a function
*/
const char *eng_getError();

#endif
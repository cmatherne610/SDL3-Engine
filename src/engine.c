#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "engine.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_render.h"

typedef enum {
	TYPE_UNKNOWN,
	TYPE_RECT,
	TYPE_SURFACE,
} Type;

typedef struct RenderQueue {
	struct RenderQueue *pNext;
	Type type;
	void *data;
} RenderQueue;

static ENG_RESULT errorCode = SUCCESS;
static SDL_Event e;
static bool debug = false;

static uint32_t nsPerFrame;
static uint32_t renderingNS = 0;
static uint32_t endingNS = 0;

static RenderQueue *renderQueue = NULL;
static uint32_t renderQueueCount = 0;
Mouse eng_getMousePosition();

static RenderQueue *getObjectFromPointer(void *data) {
	RenderQueue *temp = renderQueue;
	RenderQueue *prev = temp;
	bool success = false;
	int positions = 0;

	if (temp->data == data) {
		return temp;
	}

	while (temp != NULL && success == false) {
		if (temp->data == data) {
			return temp;
		}
		prev = temp;
		temp = temp->pNext;
	}

	return NULL;
}

static ENG_RESULT removeFromRenderQueueWithoutFreeing(void *data) {
	RenderQueue *temp = renderQueue;
	RenderQueue *prev = temp;
	bool success = false;

	if (temp->data == data) {
		renderQueue = temp->pNext;

		success = true;
		renderQueueCount--;
	}

	while (temp != NULL && success == false) {
		if (temp->data == data) {
			prev->pNext = temp->pNext;

			renderQueueCount--;
			success = true;

			break;
		}
		prev = temp;
		temp = temp->pNext;
	}

	if (success != true) {
		errorCode = FAILED_TO_REMOVE_RECT_FROM_RENDER_QUEUE;
		return FAILED_TO_REMOVE_RECT_FROM_RENDER_QUEUE;
	}

	return SUCCESS;
}

static ENG_RESULT addQueueToRenderQueue(RenderQueue *object) {
	RenderQueue *temp = renderQueue;
	while (temp->pNext != NULL) {
		temp = temp->pNext;
	}
	temp->pNext = object;

	renderQueueCount++;

	return SUCCESS;
}

ENG_RESULT eng_moveToQueuePosition(void *data, int position) {
	if (data == NULL) {
		errorCode = DATA_IS_NULL;
		if (debug) {
			printf("ERROR: %s\n", eng_getError());
		}
	}

	if (position == 0) {
		errorCode = POSITION_CANT_BE_ZERO;
		if (debug) {
			printf("ERROR: %s\n", eng_getError());
		}
		return errorCode;
	}


	//if (position < 0) {
		//position = renderQueueCount-(abs(position) + 1);
	//}

	if (renderQueueCount == 1) {
		return SUCCESS;
	}

	if (abs(position) > renderQueueCount) {
		errorCode = POSITION_HIGHER_THAN_QUEUE_LENGTH;
		if (debug) {
			printf("ERROR: %s\n", eng_getError());
		}
		return errorCode;
	}

	RenderQueue *object = getObjectFromPointer(data);
	removeFromRenderQueueWithoutFreeing(data);
	if (position == renderQueueCount || position == -1) {
		object->pNext = NULL;
		addQueueToRenderQueue(object);
	} else {
		RenderQueue *curr = renderQueue;
		for (int i = 0; i < position-2; i++) {
			curr = curr->pNext;
		}

		RenderQueue *prev = curr;
		curr = curr->pNext;
		object->pNext = curr;
		prev->pNext = object;
	}

	return SUCCESS;
}

eng_Rect *eng_createRect(uint32_t h, uint32_t w, uint32_t x, uint32_t y, eng_Color color) {
	eng_Rect *rect = (eng_Rect *)malloc(sizeof(eng_Rect));
	if (rect == NULL) {
		errorCode = FAILED_TO_MALLOC;
		return NULL;
	}
	*rect = (eng_Rect) {
		.h = h,
		.w = w,
		.x = x,
		.y = y,
	};
	rect->color = (eng_Color *)malloc(sizeof(eng_Color));
	if (rect->color == NULL) {
		errorCode = FAILED_TO_MALLOC;
		free(rect);
		return NULL;
	}

	*rect->color = (eng_Color) {
		.r = color.r,
		.g = color.g,
		.b = color.b,
		.a = color.a,
	};

	if (renderQueueCount == 0) {
		renderQueue->type = TYPE_RECT;
		renderQueue->data = rect;
	} else {


		RenderQueue *next = (RenderQueue *)malloc(sizeof(RenderQueue));
		*next = (RenderQueue) {
			.pNext = NULL,
			.type = TYPE_RECT,
			.data = rect,
		};

		RenderQueue *temp = renderQueue;
		while (temp->pNext != NULL) {
			temp = temp->pNext;
		}
		temp->pNext = next;
	}

	renderQueueCount++;
	if (debug)
		printf("Added to render queue\tCurrent count: %d\n", renderQueueCount);

	return rect;
}

ENG_RESULT eng_addRectToRenderQueue(eng_Rect *rect) {
	renderQueue = realloc(renderQueue, sizeof(RenderQueue) * renderQueueCount + 1);
	if (renderQueue == NULL) {
		errorCode = FAILED_TO_MALLOC;
		return errorCode;
	}

	if (renderQueueCount == 0) {
		renderQueue->type = TYPE_RECT;
		renderQueue->data = rect;
	} else {
		RenderQueue *temp = renderQueue;
		while (temp->pNext != NULL) {
			temp = temp->pNext;
		}
		temp->pNext->pNext = NULL;
		temp->pNext->type = TYPE_RECT;
		temp->pNext->data = rect;
	}

	renderQueueCount++;
	if (debug)
		printf("Added to render queue\tCurrent count: %d\n", renderQueueCount);

	return SUCCESS;
}

ENG_RESULT eng_removeFromRenderQueue(void *data) {
	RenderQueue *temp = renderQueue;
	RenderQueue *prev = temp;
	bool success = false;

	if (temp->data == data) {
		renderQueue = temp->pNext;
		free(temp->data);
		free(temp);

		success = true;
		renderQueueCount--;
		if (debug)
			printf("Removed from render queue\tCount: %d\n", renderQueueCount);
	}

	while (temp != NULL && success == false) {
		if (temp->data == data) {
			prev->pNext = temp->pNext;
			free(temp->data);
			free(temp);

			renderQueueCount--;
			success = true;
			if (debug)
				printf("Removed from render queue\tCount: %d\n", renderQueueCount);

			break;
		}
		prev = temp;
		temp = temp->pNext;
	}

	if (success != true) {
		errorCode = FAILED_TO_REMOVE_RECT_FROM_RENDER_QUEUE;
		return FAILED_TO_REMOVE_RECT_FROM_RENDER_QUEUE;
	}

	return SUCCESS;
}

eng_Texture *eng_addImageToRenderQueue(struct Window *pWindow, const char *path, uint32_t h, uint32_t w, uint32_t x, uint32_t y) {
	SDL_Surface *surface = IMG_Load(path);
	if (surface == NULL) {
		errorCode = FAILED_TO_LOAD_IMAGE;
		free(surface);
		return NULL;
	}

	SDL_Texture *newTexture = (SDL_Texture *)malloc(sizeof(SDL_Texture));
	newTexture = SDL_CreateTextureFromSurface(pWindow->pRenderer, surface);
	SDL_DestroySurface(surface);

	eng_Texture *texture = (eng_Texture *)malloc(sizeof(eng_Texture));
	if (texture == NULL) {
		errorCode = FAILED_TO_MALLOC;
		free(surface);
		return NULL;
	}
	*texture = (eng_Texture) {
		.h = h,
		.w = w,
		.x = x,
		.y = y,
		.texture = newTexture,
	};

	if (renderQueueCount == 0) {
		renderQueue->type = TYPE_SURFACE;
		renderQueue->data = texture;
		renderQueue->pNext = NULL;
	} else {
		RenderQueue *temp = renderQueue;
		while (temp->pNext != NULL) {
			temp = temp->pNext;
		}
		temp->pNext = malloc(sizeof(RenderQueue));
		temp->pNext->pNext = NULL;
		temp->pNext->type = TYPE_SURFACE;
		temp->pNext->data = texture;
	}

	renderQueueCount++;
	if (debug)
		printf("Added to render queue\tCurrent count: %d\n", renderQueueCount);

	return texture;
}

ENG_RESULT eng_init(bool debugEnabled) {
	debug = debugEnabled;
	renderQueue = (RenderQueue *)malloc(sizeof(RenderQueue));
	renderQueue->pNext = NULL;
	renderQueue->data = NULL;

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		errorCode = FAILED_TO_INIT_SDL;
		return errorCode;
	}
	if (debug) {
		printf("Successfully initialized SDL\n");
	}

	return SUCCESS;
}

bool eng_pollEvent(Application *app, uint32_t fps) {
	SDL_PollEvent(&e);
	app->mouse = eng_getMousePosition();

	if (fps > 0) {
		nsPerFrame = 1000000000 / fps;
		if (renderingNS - endingNS < nsPerFrame) {
			uint32_t sleepTime = nsPerFrame - (endingNS - renderingNS);
			SDL_DelayNS(sleepTime);

			endingNS = SDL_GetTicksNS();
		} else {
			endingNS = SDL_GetTicksNS();
		}
	}

	if (e.type == SDL_EVENT_QUIT) {
		app->isRunning = false;
		return false;
	} else if (e.type == SDL_EVENT_KEY_DOWN) {
		app->event.type = ENG_EVENT_KEY_DOWN;
		switch(e.key.scancode) {
			case SDL_SCANCODE_0:
				app->event.value = ENG_KEY_0;
				break;
			case SDL_SCANCODE_1:
				app->event.value = ENG_KEY_1;
				break;
			case SDL_SCANCODE_2:
				app->event.value = ENG_KEY_2;
				break;
			case SDL_SCANCODE_3:
				app->event.value = ENG_KEY_3;
				break;
			case SDL_SCANCODE_4:
				app->event.value = ENG_KEY_4;
				break;
			case SDL_SCANCODE_5:
				app->event.value = ENG_KEY_5;
				break;
			case SDL_SCANCODE_6:
				app->event.value = ENG_KEY_6;
				break;
			case SDL_SCANCODE_7:
				app->event.value = ENG_KEY_7;
				break;
			case SDL_SCANCODE_8:
				app->event.value = ENG_KEY_8;
				break;
			case SDL_SCANCODE_9:
				app->event.value = ENG_KEY_9;
				break;

			case SDL_SCANCODE_A:
				app->event.value = ENG_KEY_A;
				break;
			case SDL_SCANCODE_B:
				app->event.value = ENG_KEY_B;
				break;
			case SDL_SCANCODE_C:
				app->event.value = ENG_KEY_C;
				break;
			case SDL_SCANCODE_D:
				app->event.value = ENG_KEY_D;
				break;
			case SDL_SCANCODE_E:
				app->event.value = ENG_KEY_E;
				break;
			case SDL_SCANCODE_F:
				app->event.value = ENG_KEY_F;
				break;
			case SDL_SCANCODE_G:
				app->event.value = ENG_KEY_G;
				break;
			case SDL_SCANCODE_H:
				app->event.value = ENG_KEY_H;
				break;
			case SDL_SCANCODE_I:
				app->event.value = ENG_KEY_I;
				break;
			case SDL_SCANCODE_J:
				app->event.value = ENG_KEY_J;
				break;
			case SDL_SCANCODE_K:
				app->event.value = ENG_KEY_K;
				break;
			case SDL_SCANCODE_L:
				app->event.value = ENG_KEY_L;
				break;
			case SDL_SCANCODE_M:
				app->event.value = ENG_KEY_M;
				break;
			case SDL_SCANCODE_N:
				app->event.value = ENG_KEY_N;
				break;
			case SDL_SCANCODE_O:
				app->event.value = ENG_KEY_O;
				break;
			case SDL_SCANCODE_P:
				app->event.value = ENG_KEY_P;
				break;
			case SDL_SCANCODE_Q:
				app->event.value = ENG_KEY_Q;
				break;
			case SDL_SCANCODE_R:
				app->event.value = ENG_KEY_R;
				break;
			case SDL_SCANCODE_S:
				app->event.value = ENG_KEY_S;
				break;
			case SDL_SCANCODE_T:
				app->event.value = ENG_KEY_T;
				break;
			case SDL_SCANCODE_U:
				app->event.value = ENG_KEY_U;
				break;
			case SDL_SCANCODE_V:
				app->event.value = ENG_KEY_V;
				break;
			case SDL_SCANCODE_W:
				app->event.value = ENG_KEY_W;
				break;
			case SDL_SCANCODE_X:
				app->event.value = ENG_KEY_X;
				break;
			case SDL_SCANCODE_Y:
				app->event.value = ENG_KEY_Y;
				break;
			case SDL_SCANCODE_Z:
				app->event.value = ENG_KEY_Z;
				break;

			default:
				break;
		}
		return true;
	} else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
		app->event.type = ENG_MOUSE_BUTTON;
		switch(e.button.button) {
			case SDL_BUTTON_LEFT:
				app->event.value = MOUSE_BUTTON_LEFT;
				break;
			case SDL_BUTTON_MIDDLE:
				app->event.value = MOUSE_BUTTON_MIDDLE;
				break;
			case SDL_BUTTON_RIGHT:
				app->event.value = MOUSE_BUTTON_RIGHT;
				break;
		}
	}

	return false;
}

Mouse eng_getMousePosition() {
	Mouse mouse;

	SDL_GetMouseState(&mouse.x, &mouse.y);

	return mouse;
}

void eng_render(Application *app) {
	renderingNS = SDL_GetTicksNS();

	SDL_SetRenderDrawColor(app->window.pRenderer, 0, 0, 0, 255);
	SDL_RenderClear(app->window.pRenderer);

	if (renderQueue != NULL) {
		RenderQueue *temp = renderQueue;
		while (temp != NULL) {
			if (temp->type == TYPE_RECT) {
				eng_Rect *rect = temp->data;
				SDL_FRect frect = (SDL_FRect) {
					.h = rect->h,
					.w = rect->w,
					.x = rect->x,
					.y = rect->y,
				};

				SDL_SetRenderDrawColor(app->window.pRenderer, rect->color->r, rect->color->g,rect->color->b, rect->color->a);
				SDL_RenderFillRect(app->window.pRenderer, &frect);
			} else if (temp->type == TYPE_SURFACE) {
				eng_Texture *texture = temp->data;
				SDL_FRect rect = (SDL_FRect) {
					.h = texture->h,
					.w = texture->w,
					.x = texture->x,
					.y = texture->y,
				};
				SDL_RenderTexture(app->window.pRenderer, texture->texture, NULL, &rect);
			}
			temp = temp->pNext;
		}
	}
	
	SDL_RenderPresent(app->window.pRenderer);
}

const char *eng_getError() {
	switch (errorCode) {
		case SUCCESS:
			return "There was no error";
		case FAILED_TO_CREATE_RENDERER:
			return "Failed to create Renderer";
		case FAILED_TO_CREATE_WINDOW:
			return "Failed to create Window";
		case FAILED_TO_INIT_SDL:
			return "Failed to init SDL";
		case FAILED_TO_MALLOC:
			return "Failed to allocate memory, might be out of RAM";
		case FAILED_TO_REMOVE_RECT_FROM_RENDER_QUEUE:
			return "Failed to remove rect from the render queue, probably couldn't find it";
		case FAILED_TO_LOAD_IMAGE:
			return "Failed to load image, probably couldn't be found";
		case DATA_IS_NULL:
			return "The data provided was NULL";
		case POSITION_HIGHER_THAN_QUEUE_LENGTH:
			return "The position provided was higher than the queue length";
		case FAILED_TO_FIND_TYPE:
			return "Failed to find type of data from the queue";
		case POSITION_CANT_BE_ZERO:
			return "The position provided was exactly zero";
	}

	return "Failed to find error";
}

Application *eng_createApplication(const char *title, const uint32_t width, const uint32_t height) {
	Application *app = (Application *)malloc(sizeof(Application));
	app->isRunning = true;

	app->window.pWindow = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
	if (app->window.pWindow == NULL) {
		errorCode = FAILED_TO_CREATE_WINDOW;
		free(app);
		return NULL;
	}
	if (debug) {
		printf("Created Window\n");
	}

	app->window.width = width;
	app->window.height = height;

	app->window.pRenderer = SDL_CreateRenderer(app->window.pWindow, NULL);
	if (app->window.pRenderer == NULL) {
		errorCode = FAILED_TO_CREATE_RENDERER;
		SDL_DestroyWindow(app->window.pWindow);
		free(app);
		return NULL;
	}
	if (debug) {
		printf("Created Renderer\n");
	}

	if (debug) {
		printf("Created Application\n\n\n");
	}
	return app;
}

void eng_quit(Application *app) {
	if (renderQueue != NULL) {
		int i = 1;
		RenderQueue *temp = renderQueue;
		RenderQueue *prev = temp;
		while (temp != NULL) {
			prev = temp;
			temp = temp->pNext;

			if (prev->type == TYPE_RECT) {
				eng_Rect *rect = prev->data;
				free(rect->color);
				free(rect);
			} else if (prev->type == TYPE_SURFACE) {
				eng_Texture *texture = prev->data;
				SDL_DestroyTexture(texture->texture);
				free(texture);
			}

			free(prev);
		if (debug) {
			printf("Freed queue position %d of %d\n", i, renderQueueCount);
			i++;
		}
		}
	}

	if (app) {
		SDL_DestroyRenderer(app->window.pRenderer);
		SDL_DestroyWindow(app->window.pWindow);
	}

	SDL_Quit();
}
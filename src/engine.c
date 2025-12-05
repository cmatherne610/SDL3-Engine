#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "engine.h"


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

bool eng_isTouchingRects(eng_Rect firstRect, eng_Rect secondRect) {
	SDL_Rect rect1 = (SDL_Rect) {
		.x = firstRect.x,
		.y = firstRect.y,
		.h = firstRect.h,
		.w = firstRect.w,
	};
	SDL_Rect rect2 = (SDL_Rect) {
		.x = secondRect.x,
		.y = secondRect.y,
		.h = secondRect.h,
		.w = secondRect.w,
	};
	return SDL_HasRectIntersection(&rect1, &rect2);
}

bool eng_isTouching(float firstX, float firstY, float firstH, float firstW, float secondX, float secondY, float secondH, float secondW) {
	SDL_Rect firstRect = (SDL_Rect) {
		.x = firstX,
		.y = firstY,
		.h = firstH,
		.w = firstW,
	};
	SDL_Rect secondRect = (SDL_Rect) {
		.x = secondX,
		.y = secondY,
		.h = secondH,
		.w = secondW,
	};
	return SDL_HasRectIntersection(&firstRect, &secondRect);
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

	return rect;
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

ENG_RESULT eng_removeFromCustomRenderQueue(RenderQueue *queue, void *data) {
	RenderQueue *temp = queue;
	RenderQueue *prev = temp;
	bool success = false;

	if (temp->data == data) {
		queue = temp->pNext;
		free(temp->data);
		free(temp);

		success = true;
		if (debug)
			printf("Removed from custom render queue\n");
	}

	while (temp != NULL && success == false) {
		if (temp->data == data) {
			prev->pNext = temp->pNext;
			free(temp->data);
			free(temp);

			success = true;
			if (debug)
				printf("Removed from custom render queue\n");

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

eng_Texture *eng_createImage(Window *pWindow, const char *path, uint32_t h, uint32_t w, uint32_t x, uint32_t y) {
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

	return texture;
}

ENG_RESULT eng_addObjectToRenderQueue(void *object, Type type) {
	RenderQueue *newQueue = malloc(sizeof(RenderQueue));
	*newQueue = (RenderQueue) {
		.pNext = NULL,
		.data = object,
		.type = type,
	};

	if (renderQueueCount == 0) {
		free(newQueue);
		renderQueue->type = type;
		renderQueue->data = object;
	} else {
		RenderQueue *temp = renderQueue;
		while (temp->pNext != NULL) {
			temp = temp->pNext;
		}
		temp->pNext = newQueue;
	}

	renderQueueCount++;
	if (debug)
		printf("Added to render queue\tCurrent count: %d\n", renderQueueCount);

	return SUCCESS;

	return UNKNOWN_ERROR;
}

eng_Text *eng_createText(Window *window, const char *font, uint32_t fontSize, const char *text, eng_Color color, uint32_t x, uint32_t y) {
	eng_Text *texture = (eng_Text *)malloc(sizeof(eng_Text));

	SDL_Color selectedColor = (SDL_Color) {
		.r = color.r,
		.g = color.g,
		.b = color.b,
		.a = color.a,
	};

	TTF_Font *selectedFont = TTF_OpenFont(font, fontSize);
	if (selectedFont == NULL) {
		errorCode = FAILED_TO_OPEN_FONT;
		free(texture);
		return NULL;
	}
	SDL_Surface *fontSurface = TTF_RenderText_Blended(selectedFont, text, strlen(text), selectedColor);
	if (fontSurface == NULL) {
		errorCode = FAILED_TO_CREATE_FONT_RENDER;
		free(texture);
		return NULL;
	}
	SDL_Texture *fontTexture = SDL_CreateTextureFromSurface(window->pRenderer, fontSurface);
	if (texture == NULL) {
		errorCode = FAILED_TO_CONVERT_FONT_TO_TEXTURE;
		free(texture);
		return NULL;
	}
	SDL_DestroySurface(fontSurface);

	TTF_Text *textPointer = TTF_CreateText(NULL, selectedFont, text, 0);
	int h, w;
	TTF_GetTextSize(textPointer, &w, &h);

	*texture = (eng_Text) {
		.texture = fontTexture,
		.h = h,
		.w = w,
		.x = x,
		.y = y,
		.text = textPointer,
	};

	return texture;
}

ENG_RESULT eng_init(bool debugEnabled) {
	debug = debugEnabled;
	renderQueue = (RenderQueue *)malloc(sizeof(RenderQueue));
	renderQueue->pNext = NULL;
	renderQueue->data = NULL;

	if (!SDL_Init(SDL_INIT_VIDEO) || !TTF_Init()) {
		return errorCode = FAILED_TO_INIT_SDL;
	}
	if (debug) {
		printf("Successfully initialized SDL\n");
	}

	return SUCCESS;
}

bool eng_pollEvent(Application *app, uint32_t fps) {
	SDL_PollEvent(&e);

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
			case SDL_SCANCODE_ESCAPE:
				app->event.value = ENG_KEY_ESC;
				break;
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
	} else if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
		SDL_GetWindowSize(app->window.pWindow, &app->window.width, &app->window.height);
		return true;
	} else if (e.type == SDL_EVENT_WINDOW_RESIZED) {
		app->event.type = ENG_EVENT_WINDOW_SIZE_CHANGED;
		SDL_GetWindowSize(app->window.pWindow, &app->window.width, &app->window.height);
		return true;
	} else if (e.type == SDL_EVENT_MOUSE_MOTION) {
		app->event.type = ENG_EVENT_MOUSE_MOTION;
		app->mouse = eng_getMousePosition();
		return true;
	}

	return false;
}

Mouse eng_getMousePosition() {
	Mouse mouse;

	SDL_GetMouseState(&mouse.x, &mouse.y);

	return mouse;
}

void eng_centerText(Window *pWindow, eng_Text *text) {
	text->x = (float)(pWindow->width - text->w) / 2;
	text->y = (float)(pWindow->height - text->h) / 2;
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
			} else if (temp->type == TYPE_TEXTURE) {
				eng_Texture *texture = temp->data;
				SDL_FRect rect = (SDL_FRect) {
					.h = texture->h,
					.w = texture->w,
					.x = texture->x,
					.y = texture->y,
				};
				SDL_RenderTexture(app->window.pRenderer, texture->texture, NULL, &rect);
			} else if (temp->type == TYPE_TEXT) {
				eng_Text *text = temp->data;
				SDL_FRect rect = (SDL_FRect) {
					.h = text->h,
					.w = text->w,
					.x = text->x,
					.y = text->y,
				};
				SDL_RenderTexture(app->window.pRenderer, text->texture, NULL, &rect);
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
		case FAILED_TO_OPEN_FONT:
			return "Failed to open the font, check spelling and location of font";
		case FAILED_TO_CREATE_FONT_RENDER:
			return "Failed to create text into a format for the render queue";
		case FAILED_TO_CONVERT_FONT_TO_TEXTURE:
			return "Failed to convert font to a texture";
		case INVALID_TYPE:
			return "Cannot provide TYPE_UNKNOWN to function";
		case QUEUE_WAS_NULL:
			return "The queue provided was NULL";
		case UNKNOWN_ERROR:
			return "The error is unknown, this shouldn't be possible";
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

	SDL_GetWindowSize(app->window.pWindow, &app->window.width, &app->window.height);

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
			} else if (prev->type == TYPE_TEXTURE) {
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

	TTF_Quit();
	SDL_Quit();
}

ENG_RESULT eng_addToCustomQueue(RenderQueue *queue, void *object, Type type) {
	if (type == TYPE_UNKNOWN) {
		return errorCode = INVALID_TYPE;
	}

	if (queue == NULL) {
		return errorCode = QUEUE_WAS_NULL;
	}

	if (queue->data == NULL) {
		queue->data = object;
		queue->pNext = NULL;
		queue->type = type;
	}

	RenderQueue *newQueue = (RenderQueue *)malloc(sizeof(RenderQueue));
	if (newQueue == NULL) {
		return  errorCode = FAILED_TO_MALLOC;
	}

	*newQueue = (RenderQueue) {
		.pNext = NULL,
		.type = type,
		.data = object,
	};
	
	RenderQueue *curr = queue;
	while (curr->pNext != NULL) {
		curr = curr->pNext;
	}
	curr->pNext = newQueue;

	return errorCode = SUCCESS;
}

ENG_RESULT eng_renderCustomQueue(Application *app, RenderQueue *customQueue) {
	RenderQueue *curr = customQueue;
	SDL_SetRenderDrawColor(app->window.pRenderer, 0, 0, 0, 255);
	SDL_RenderClear(app->window.pRenderer);
	while (curr != NULL) {
		if (curr->type == TYPE_RECT) {
			eng_Rect *rect = curr->data;
			SDL_FRect frect = (SDL_FRect) {
				.h = rect->h,
				.w = rect->w,
				.x = rect->x,
				.y = rect->y,
			};
			SDL_RenderFillRect(app->window.pRenderer, &frect);
		} else if (curr->type == TYPE_TEXTURE) {
			eng_Texture *texture = curr->data;
			SDL_FRect rect = (SDL_FRect) {
				.h = texture->h,
				.w = texture->w,
				.x = texture->x,
				.y = texture->y,
			};
			SDL_RenderTexture(app->window.pRenderer, texture->texture, NULL, &rect);
		} else if (curr->type == TYPE_TEXT) {
			eng_Text *text = curr->data;
			SDL_FRect rect = (SDL_FRect) {
				.h = text->h,
				.w = text->w,
				.x = text->x,
				.y = text->y,
			};
			SDL_RenderTexture(app->window.pRenderer, text->texture, NULL, &rect);
		}

		curr = curr->pNext;
	}

	SDL_RenderPresent(app->window.pRenderer);

	return SUCCESS;
}

eng_Rect eng_extractRectFromObject(void *object, Type type) {
	eng_Rect rect = (eng_Rect) {
		.h = 0,
		.w = 0,
		.x = 0,
		.y = 0,
	};

	switch (type) {
		case TYPE_UNKNOWN:
			errorCode = INVALID_TYPE;
			return rect;
		case TYPE_RECT: ;
			eng_Rect *passedRect = object;
			rect = (eng_Rect) {
				.x = passedRect->x,
				.y = passedRect->y,
				.h = passedRect->h,
				.w = passedRect->w,
			};
			return rect;
		case TYPE_TEXT: ;
			eng_Text *text = object;
			rect = (eng_Rect) {
				.x = text->x,
				.y = text->y,
				.h = text->h,
				.w = text->w,
			};
			return rect;
		case TYPE_TEXTURE: ;
			eng_Texture *texture = object;
			rect = (eng_Rect) {
				.x = texture->x,
				.y = texture->y,
				.h = texture->h,
				.w = texture->w,
			};
			return rect;

	}

	return rect;
}

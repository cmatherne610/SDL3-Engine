#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "engine.h"

typedef struct {
	uint32_t health;
	eng_Texture *rect;
	uint32_t speed;
	bool inAir;
	bool isJumping;
	float jumpHeight;
	float jumpStartHeight;
} Player;

int main() {
	eng_init(true);

	Application *app = eng_createApplication("game", 800, 600);
	if (app == NULL) {
		printf("ERROR: %s\n", eng_getError());
	}
	Player player;
	player.speed = 8;
	player.jumpHeight = 50;
	player.isJumping = false;
	
	eng_Rect *ground = eng_createRect(32, app->window.width, 0, app->window.height-32, (eng_Color){0,255,90,255});
	player.rect = eng_createImage(&app->window, "../images/test.png", 32, 32, app->window.width/2-16, app->window.height-64);
	eng_addObjectToRenderQueue(ground, TYPE_RECT);
	eng_addObjectToRenderQueue(player.rect, TYPE_TEXTURE);

	eng_Text *resume = eng_createText(&app->window, "../fonts/arial.ttf", 72, "Resume", (eng_Color){255,255,255,255}, 0, 0);
	if (resume == NULL) {
		printf("%s\n", eng_getError());
	}
	eng_Text *start= eng_createText(&app->window, "../fonts/arial.ttf", 72, "Start", (eng_Color){255,255,255,255}, 0, 0);
	if (start == NULL) {
		printf("%s\n", eng_getError());
	}
	eng_Text *options = eng_createText(&app->window, "../fonts/arial.ttf", 72, "Options", (eng_Color){255,255,255,255}, 0, 0); 
	if (options == NULL) {
		printf("%s\n", eng_getError());
	}
	eng_Text *quit = eng_createText(&app->window, "../fonts/arial.ttf", 72, "Quit", (eng_Color){255,255,255,255}, 0, 0);
	if (quit == NULL) {
		printf("%s\n", eng_getError());
	}
	eng_centerText(&app->window, resume);
	//eng_centerText(&app->window, start);
	eng_centerText(&app->window, options);
	eng_centerText(&app->window, quit);
	resume->y -= options->h;
	quit->y += options->h;

	RenderQueue *menu = malloc(sizeof(RenderQueue));
	eng_addToCustomQueue(menu, resume, TYPE_TEXT);
	eng_addToCustomQueue(menu, options, TYPE_TEXT);
	eng_addToCustomQueue(menu, quit, TYPE_TEXT);
	bool pauseMenu = true;

	while(app->isRunning) {
		while (eng_pollEvent(app, 60)) {
			if (app->event.type == ENG_EVENT_KEY_DOWN) {
				switch(app->event.value) {
					case ENG_KEY_W:
						if (player.inAir == false) {
							player.jumpStartHeight = player.rect->y;
							player.isJumping = true;
						}
						break;
					case ENG_KEY_A:
						player.rect->x -= player.speed;
						break;
					case ENG_KEY_S:
						break;
					case ENG_KEY_D:
						player.rect->x += player.speed;
						break;
					case ENG_KEY_ESC:
						pauseMenu = !pauseMenu;
						break;
					case ENG_KEY_G:
					if (pauseMenu) {
						eng_Rect rect = (eng_Rect) {
							.h = 1,
							.w = 1,
							.x = app->mouse.x,
							.y = app->mouse.y,
						};
						if (eng_isTouchingRects(eng_extractRectFromObject(options, TYPE_TEXT), rect)) {
							printf("print options\n");
						} else if (eng_isTouchingRects(eng_extractRectFromObject(resume, TYPE_TEXT), rect)) {
							pauseMenu = !pauseMenu;
						} else if (eng_isTouchingRects(eng_extractRectFromObject(quit, TYPE_TEXT), rect)) {
							app->isRunning = false;
						}

					}
						break;
				}
			} else if(app->event.type == ENG_MOUSE_BUTTON) {
				switch(app->event.value) {
					case MOUSE_BUTTON_LEFT:
						break;
					case MOUSE_BUTTON_MIDDLE:
						break;
					case MOUSE_BUTTON_RIGHT:
						break;
				}
			} else if (app->event.type == ENG_EVENT_WINDOW_SIZE_CHANGED) {
				eng_centerText(&app->window, resume);
				//eng_centerText(&app->window, start);
				eng_centerText(&app->window, options);
				eng_centerText(&app->window, quit);
				resume->y -= 128;
				quit->y += 128;

				ground->w = app->window.width;
				ground->y = app->window.height-32;

				player.rect->y = app->window.height-64;
			}
		}

		if (pauseMenu) {
			eng_renderCustomQueue(app, menu);
		} else {
			if (!eng_isTouching(player.rect->x, player.rect->y, player.rect->h, player.rect->w, ground->x, ground->y, ground->h, ground->w) && !player.isJumping) {
				player.inAir = true;
				player.rect->y += .5;
			} else {
				player.inAir = false;
			}

			if (player.isJumping && player.rect->y > player.jumpStartHeight - player.jumpHeight) {
				player.rect->y -= 1;
			} else {
				player.isJumping = false;
			}
			eng_render(app);
		}


	}
	eng_quit(app);
}
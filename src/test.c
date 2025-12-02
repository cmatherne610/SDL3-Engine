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
	
	eng_Rect *ground = eng_createRect(32, app->window.width, 0, app->window.height-32, (eng_Color){0,0,0,0});
	player.rect = eng_addImageToRenderQueue(&app->window, "../images/test.png", 32, 32, app->window.width/2-16, app->window.height-64);

	eng_Texture *start= eng_createFont(&app->window, "../fonts/arial.ttf", 24, "Start", (eng_Color){255,255,255,255}, 128, app->window.width/3, app->window.width/2-(128), app->window.height/3-128);
	if (start == NULL) {
		printf("%s\n", eng_getError());
	}
	eng_Texture *options= eng_createFont(&app->window, "../fonts/arial.ttf", 24, "Options", (eng_Color){255,255,255,255}, 128, app->window.width/3, app->window.width/2-(128), app->window.height/2-100);
	if (options == NULL) {
		printf("%s\n", eng_getError());
	}
	eng_Texture *quit= eng_createFont(&app->window, "../fonts/arial.ttf", 24, "Quit", (eng_Color){255,255,255,255}, 128, app->window.width/3, app->window.width/2-(128), app->window.height/2+50);
	if (quit == NULL) {
		printf("%s\n", eng_getError());
	}

	RenderQueue *queue = malloc(sizeof(RenderQueue));
	eng_addToCustomQueue(queue, start, TYPE_SURFACE);
	eng_addToCustomQueue(queue, options, TYPE_SURFACE);
	eng_addToCustomQueue(queue, quit, TYPE_SURFACE);
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
						if (eng_isTouching(app->mouse.x, app->mouse.y, 1, 1, options->x, options->y, options->h, options->w) == true) {
							printf("print options\n");
						} else if (eng_isTouching(app->mouse.x, app->mouse.y, 1, 1, start->x, start->y, start->h, start->w) == true) {
							pauseMenu = !pauseMenu;
						} else if (eng_isTouching(app->mouse.x, app->mouse.y, 1, 1, quit->x, quit->y, quit->h, quit->w) == true) {
							app->isRunning = false;
						}

					}
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
			}
		}

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

		if (pauseMenu) {
			eng_renderCustomQueue(app, queue);
		} else {
			eng_render(app);
		}
	}
	eng_quit(app);
}
#include <stdio.h>
#include <stdbool.h>

#include "engine.h"

typedef struct {
	uint32_t health;
	eng_Texture *rect;
	uint32_t speed;
} Player;

int main() {
	eng_init(true);

	Application *app = eng_createApplication("game", 800, 600);
	if (app == NULL) {
		printf("ERROR: %s\n", eng_getError());
	}
	Player player;
	player.speed = 8;
	
	player.rect = eng_addImageToRenderQueue(&app->window, "../images/test.png", 32, 32, 0, 0);
	eng_createRect(32, 32, 0, 0, (eng_Color){255,0,0,255});
	eng_createRect(32, 32, 32, 0, (eng_Color){255,0,0,255});
	eng_createRect(32, 32, 64, 0, (eng_Color){255,0,0,255});
	eng_createRect(32, 32, 96, 0, (eng_Color){255,0,0,255});
	eng_createRect(32, 32, 128, 0, (eng_Color){255,0,0,255});

	eng_moveToQueuePosition(player.rect, -1);

	while(app->isRunning) {
		while (eng_pollEvent(app, 60)) {
			if (app->event.type == ENG_EVENT_KEY_DOWN) {
				switch(app->event.value) {
					case ENG_KEY_W:
						player.rect->y -= player.speed;
						break;
					case ENG_KEY_A:
						player.rect->x -= player.speed;
						break;
					case ENG_KEY_S:
						player.rect->y += player.speed;
						break;
					case ENG_KEY_D:
						player.rect->x += player.speed;
						break;
				}
			} else if(app->event.type == ENG_MOUSE_BUTTON) {
				switch(app->event.value) {
					case MOUSE_BUTTON_LEFT:
						eng_createRect(32, 32, app->mouse.x, app->mouse.y, (eng_Color){0,255,0,255});
						break;
					case MOUSE_BUTTON_MIDDLE:
						eng_createRect(32, 32, app->mouse.x, app->mouse.y, (eng_Color){0,255,0,255});
						break;
					case MOUSE_BUTTON_RIGHT:
						eng_createRect(32, 32, app->mouse.x, app->mouse.y, (eng_Color){0,255,0,255});
						break;
				}
			}
		}

		eng_render(app);
	}
	eng_quit(app);
}
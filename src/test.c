#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "engine.h"

typedef struct {
	eng_Texture *texture;
	uint32_t health;
} Player;

typedef struct {
	bool menuEnabled;
	RenderQueue *queue;
	eng_Text *start;
	eng_Text *options;
	eng_Text *quit;
} MainMenu;

MainMenu createMainMenu(Window *window, const char *font, uint32_t fontSize) {
	MainMenu menu = (MainMenu) {
		.menuEnabled = true,
		.queue = malloc(sizeof(RenderQueue)),
		.start = malloc(sizeof(eng_Texture)),
		.options = malloc(sizeof(eng_Texture)),
		.quit = malloc(sizeof(eng_Texture)),
	};

	menu.start = eng_createText(window, font, fontSize, "Start", (eng_Color){255, 255, 255, 255}, 0, 0),
	menu.options = eng_createText(window, font, fontSize, "Options", (eng_Color){255, 255, 255, 255}, 0, 0),
	menu.quit = eng_createText(window, font, fontSize, "Quit", (eng_Color){255, 255, 255, 255}, 0, 0),

	eng_centerText(window, menu.start);
	eng_centerText(window, menu.options);
	eng_centerText(window, menu.quit);

	menu.start->y -= fontSize;
	menu.quit->y += fontSize;

	eng_addToCustomQueue(menu.queue, menu.start, TYPE_TEXT);
	eng_addToCustomQueue(menu.queue, menu.options, TYPE_TEXT);
	eng_addToCustomQueue(menu.queue, menu.quit, TYPE_TEXT);

	return menu;
}

int main() {
	eng_init(true);
	Application *appPtr = eng_createApplication("game", 800, 600);
	if (appPtr == NULL) {
		printf("%s\n", eng_getError());
	}
	Application app = *appPtr;

	Player player;
	player.texture = eng_createImage(app.window, "../images/Items/Boxes/Box1/Idle.png", 28, 24, 0, 0);
	eng_addObjectToRenderQueue(player.texture, TYPE_TEXTURE);

	MainMenu menu = createMainMenu(app.window, "../fonts/Arial.ttf", 72);

	while (app.isRunning) {
		while(eng_pollEvent(&app, 60)) {
			if (app.event.type == ENG_EVENT_KEY_DOWN) {
				switch(app.event.value) {
					case ENG_KEY_W:
						printf("Forward\n");
						break;
					case ENG_KEY_G: ;
						eng_Rect rect = {.h = 1, .w = 1, .x = app.mouse.x, .y = app.mouse.y};
						if (eng_isTouchingRects(rect, eng_extractRectFromObject(menu.quit, TYPE_TEXT))) {
							app.isRunning = false;
						} else if (eng_isTouchingRects(rect, eng_extractRectFromObject(menu.options, TYPE_TEXT))) {
							printf("Options\n");
						} else if (eng_isTouchingRects(rect, eng_extractRectFromObject(menu.start, TYPE_TEXT))) {
							menu.menuEnabled = false;
						}
						break;
					case ENG_KEY_ESC:
						menu.menuEnabled = !menu.menuEnabled;
				}
			}
		}

		if (menu.menuEnabled) {
			eng_renderCustomQueue(&app, menu.queue, (eng_Color){0,0,0,255});
		} else {
			eng_render(&app, (eng_Color){0,0,0,255});
		}
	}

	eng_quit(&app);
}
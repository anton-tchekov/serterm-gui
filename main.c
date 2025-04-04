#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>

#include "types.h"
#include "util.c"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "kbd.c"
#include "gfx.c"
#include "gui.c"
#include "escseq.c"
#include "queue.c"
#include "serial.c"
#include "terminal.c"
#include "layout.c"

int main(void)
{
	int running = 1;
	gfx_init();
	layout_init();

	while(running)
	{
		SDL_Event e;
		gfx_clear(theme_cur->ColorBG);
		term_draw(&term, PADDING, height - LABEL_H - PADDING - INPUT_HEIGHT,
			LABEL_H + 2 * PADDING + BTN_PAR_SIZE);
		gui_render();
		gfx_update();

		if(!SDL_WaitEvent(&e))
		{
			break;
		}

		switch(e.type)
		{
		case SDL_QUIT:
			running = 0;
			break;

		case SDL_WINDOWEVENT:
			if(e.window.event == SDL_WINDOWEVENT_RESIZED ||
				e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				width = e.window.data1;
				height = e.window.data2;
				layout_resize();
			}
			break;

		case SDL_KEYDOWN:
			{
				if(e.key.keysym.sym == SDLK_ESCAPE)
				{
					running = 0;
					break;
				}

				int key = key_convert(e.key.keysym.scancode, e.key.keysym.mod);
				gui_event_key(key, key_to_codepoint(key),
					e.key.repeat ? KEYSTATE_REPEAT : KEYSTATE_PRESSED);
			}
			break;

		case SDL_MOUSEMOTION:
			gui_mousemove(e.motion.x, e.motion.y);
			break;

/*
	u32 time;
	int x, y;

	_down = 1;
	SDL_GetMouseState(&x, &y);
	time = SDL_GetTicks();
	x /= _char_width;
	y /= _line_height;
	if(time < _triple_click + DBL_CLICK_MS)
	{
		_dbl_click = 0;
		_triple_click = 0;
		event_tripleclick(x, y);
	}
	else if(time < _dbl_click + DBL_CLICK_MS)
	{
		_triple_click = _dbl_click;
		_dbl_click = 0;
		event_dblclick(x, y);
	}
	else
	{
		_dbl_click = time;
		const u8 *state = SDL_GetKeyboardState(NULL);
		if(state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT])
		{
			event_shift_mousedown(x, y);
		}
		else
		{
			event_mousedown(x, y);
		}
	}*/

		case SDL_MOUSEBUTTONDOWN:
			if(e.button.button != SDL_BUTTON_LEFT) { break; }
			gui_mousedown(e.button.x, e.button.y);
			break;

		case SDL_MOUSEBUTTONUP:
			if(e.button.button != SDL_BUTTON_LEFT) { break; }
			gui_mouseup(e.button.x, e.button.y);
			break;

		case SDL_KEYUP:
			{
				int key = key_convert(e.key.keysym.scancode, e.key.keysym.mod);
				gui_event_key(key, key_to_codepoint(key), KEYSTATE_RELEASED);
			}
			break;

		default:
			if(e.type == gfx_notify_event)
			{
				Message *msg;
				while((msg = msg_pop(&serial.readq)))
				{
					switch(msg->Type)
					{
					case MSG_RECEIVED:
						term_print(&term, COLOR_WHITE, "%.*s", msg->Len, msg->Data);
						break;

					case MSG_DISCONNECTED:
						gfx_set_title("Serial Terminal");
						layout_disconnected();
						term_print(&term, COLOR_MSG, "Closed port %s", msg->Data);
						break;

					case MSG_CONNECTED:
						{
							char buf[256];
							snprintf(buf, sizeof(buf), "Serial Terminal %s", msg->Data);
							gfx_set_title(buf);
							term_print(&term, COLOR_MSG, "Opened port %s", msg->Data);
							layout_connected();
						}
						break;

					case MSG_INFO:
						term_print(&term, COLOR_MSG, "%s", msg->Data);
						break;
					}

					sfree(msg);
				}
			}
			break;
		}
	}

	layout_destroy();
	term_clear(&term);
	gfx_destroy();
	print_allocs();
	return 0;
}

#define NUM_CHARS  128

#define COLOR_WHITE   0xFFFFFFFF
#define COLOR_MSG     0xFF66FF66

static int width = 640;
static int height = 480;

static int char_width, char_height;

static u32 gfx_notify_event;
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

static void gfx_destroy(void)
{
	if(texture)
	{
		SDL_DestroyTexture(texture);
		texture = NULL;
	}

	if(renderer)
	{
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}

	if(window)
	{
		SDL_DestroyWindow(window);
		window = NULL;
	}

	if(SDL_WasInit(SDL_INIT_VIDEO))
	{
		SDL_Quit();
	}
}

static i32 font_load(char *fontfile, i32 size)
{
	SDL_Surface *chars[NUM_CHARS];
	TTF_Font *ttf = TTF_OpenFont(fontfile, size);
	if(!ttf)
	{
		printf("Failed to open font file \"%s\"\n", fontfile);
		return 1;
	}

	SDL_Color color = { 255, 255, 255, 0 };
	char letter[2];
	letter[1] = '\0';

	memset(chars, 0, sizeof(chars));

	for(i32 i = 33; i <= 126; ++i)
	{
		letter[0] = i;
		chars[i] = TTF_RenderText_Blended(ttf, letter, color);
	}

	chars[127] = TTF_RenderUTF8_Blended(ttf, "\xEF\xBF\xBD", color);

	TTF_CloseFont(ttf);

	i32 max_h = 0, max_w = 0;
	for(i32 i = 0; i < NUM_CHARS; ++i)
	{
		SDL_Surface *s = chars[i];
		if(!s) { continue; }
		if(s->w > max_w)
		{
			max_w = s->w;
		}

		if(s->h > max_h)
		{
			max_h = s->h;
		}
	}

	char_width = max_w;
	char_height = max_h;

	// printf("Char %d x %d\n", char_width, char_height);

	SDL_Surface *surface = SDL_CreateRGBSurface(0,
		char_width * 16, char_height * 8,
		32, 0xff, 0xff00, 0xff0000, 0xff000000);

	for(i32 i = 0; i < NUM_CHARS; ++i)
	{
		SDL_Surface *s = chars[i];
		if(!s) { continue; }
		SDL_Rect src = { 0, 0, s->w, s->h };
		i32 x = (i & 0x0F) * char_width;
		i32 y = (i >> 4) * char_height;
		SDL_Rect dst = { x, y, s->w, s->h };
		SDL_BlitSurface(s, &src, surface, &dst);
		SDL_FreeSurface(s);
	}

	SDL_Rect rect =
	{
		0,
		0,
		char_width,
		char_height
	};

	SDL_FillRect(surface, &rect, COLOR_WHITE);

	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	return 0;
}

static void gfx_init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Error initializing SDL; SDL_Init: %s\n", SDL_GetError());
		gfx_destroy();
		exit(1);
	}

	if(!(window = SDL_CreateWindow("Serial Terminal",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height, SDL_WINDOW_RESIZABLE)))
	{
		printf("Error creating SDL_Window: %s\n", SDL_GetError());
		gfx_destroy();
		exit(1);
	}

	SDL_SetWindowMinimumSize(window, width, height);

	if(!(renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED)))
	{
		printf("Error creating SDL_Renderer: %s\n", SDL_GetError());
		gfx_destroy();
		exit(1);
	}

	if(TTF_Init())
	{
		printf("Error initializing TTF: %s\n", TTF_GetError());
		gfx_destroy();
		exit(1);
	}

	if(font_load("terminus.ttf", 16))
	{
		printf("Error loading font\n");
		gfx_destroy();
		exit(1);
	}

	SDL_GetWindowSize(window, &width, &height);

	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	gfx_notify_event = SDL_RegisterEvents(1);
}

static void gfx_set_title(const char *title)
{
	SDL_SetWindowTitle(window, title);
}

static i32 color_r(u32 color)
{
	return (color >> 16) & 0xFF;
}

static i32 color_g(u32 color)
{
	return (color >> 8) & 0xFF;
}

static i32 color_b(u32 color)
{
	return color & 0xFF;
}

static void set_color(u32 color)
{
	SDL_SetTextureColorMod(texture,
		color_r(color), color_g(color), color_b(color));
}

static u32 gfx_color(i32 r, i32 g, i32 b)
{
	return (r << 16) | (g << 8) | b;
}

static void gfx_char(i32 x, i32 y, i32 c, u32 fg)
{
	if(c < 32 || c > 126)
	{
		c = 127;
	}

	if(c == ' ') { return; }
	SDL_Rect src = { (c & 0x0F) * char_width, (c >> 4) * char_height, char_width, char_height };
	SDL_Rect dst = { x, y, char_width, char_height };
	set_color(fg);
	SDL_RenderCopy(renderer, texture, &src, &dst);
}

static int font_string_len(int x, int y, const char *s, int len, u32 fg, u32 bg)
{
	int w = 0;
	set_color(fg);
	for(int i = 0; i < len && *s; ++i, ++s, w += char_width)
	{
		gfx_char(x + w, y, *s, fg);
	}

	return w;
	(void)bg;
}

static int font_string(int x, int y, const char *s, u32 fg, u32 bg)
{
	return font_string_len(x, y, s, 0xFFFF, fg, bg);
}

static int font_string_width(const char *s)
{
	return strlen(s) * char_width;
}

static int font_string_width_len(const char *text, int len)
{
	int i, w = 0;
	for(i = 0; i < len && *text; ++i) { w += char_width; }
	return w;
}

static void gfx_clear(u32 color)
{
	set_color(color);
	SDL_RenderClear(renderer);
}

static void gfx_rect(int x, int y, int w, int h, u32 color)
{
	SDL_Rect blank = { 0, 0, char_width, char_height };
	SDL_Rect blank_dst = { x, y, w, h };
	set_color(color);
	SDL_RenderCopy(renderer, texture, &blank, &blank_dst);
}

static void gfx_rect_border(i32 x, i32 y, i32 w, i32 h, i32 border, u32 color)
{
	gfx_rect(x, y, w, border, color);
	gfx_rect(x, y + h - border, w, border, color);

	gfx_rect(x, y + border, border, h - 2 * border, color);
	gfx_rect(x + w - border, y + border, border, h - 2 * border, color);
}

static void gfx_notify(void)
{
	SDL_Event e;
	e.type = gfx_notify_event;
	SDL_PushEvent(&e);
}

static void gfx_update(void)
{
	SDL_RenderPresent(renderer);
}

static i32 is_key_pressed(i32 key)
{
	const uint8_t *keys = SDL_GetKeyboardState(NULL);
	return keys[key];
}

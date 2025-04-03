#define MOD_SHIFT          0x8000
#define MOD_CTRL           0x4000
#define MOD_OS             0x2000
#define MOD_ALT            0x1000
#define MOD_ALT_GR         0x800

enum
{
	KEYSTATE_RELEASED,
	KEYSTATE_PRESSED,
	KEYSTATE_REPEAT
};

static int key_convert(int scancode, int mod)
{
	int key = scancode;
	if(mod & (KMOD_LCTRL | KMOD_RCTRL))
	{
		key |= MOD_CTRL;
	}

	if(mod & KMOD_LALT)
	{
		key |= MOD_ALT;
	}

	if(mod & (KMOD_RALT | KMOD_MODE))
	{
		key |= MOD_ALT_GR;
	}

	if(mod & (KMOD_LGUI | KMOD_RGUI))
	{
		key |= MOD_OS;
	}

	if(mod & (KMOD_LSHIFT | KMOD_RSHIFT))
	{
		key |= MOD_SHIFT;
	}

	return key;
}

static int key_to_codepoint(int k)
{
	int nomods = k & 0xFF;

	if(nomods == SDL_SCANCODE_TAB)                             { return '\t'; }
	else if(nomods == SDL_SCANCODE_BACKSPACE)                  { return '\b'; }
	else if(nomods == SDL_SCANCODE_RETURN)                     { return '\n'; }
	else if(nomods == SDL_SCANCODE_SPACE)                      { return ' '; }
	else if(k == (SDL_SCANCODE_COMMA | MOD_SHIFT))             { return ';'; }
	else if(k == (SDL_SCANCODE_COMMA))                         { return ','; }
	else if(k == (SDL_SCANCODE_PERIOD | MOD_SHIFT))            { return ':'; }
	else if(k == (SDL_SCANCODE_PERIOD))                        { return '.'; }
	else if(k == (SDL_SCANCODE_SLASH | MOD_SHIFT))             { return '_'; }
	else if(k == (SDL_SCANCODE_SLASH))                         { return '-'; }
	else if(k == (SDL_SCANCODE_BACKSLASH | MOD_SHIFT))         { return '\''; }
	else if(k == (SDL_SCANCODE_BACKSLASH))                     { return '#'; }
	else if(k == (SDL_SCANCODE_RIGHTBRACKET | MOD_SHIFT))      { return '*'; }
	else if(k == (SDL_SCANCODE_RIGHTBRACKET | MOD_ALT_GR))     { return '~'; }
	else if(k == (SDL_SCANCODE_RIGHTBRACKET))                  { return '+'; }
	else if(k == (SDL_SCANCODE_NONUSBACKSLASH | MOD_SHIFT))    { return '>'; }
	else if(k == (SDL_SCANCODE_NONUSBACKSLASH | MOD_ALT_GR))   { return '|'; }
	else if(k == SDL_SCANCODE_NONUSBACKSLASH)                  { return '<'; }
	else if(k == (SDL_SCANCODE_MINUS | MOD_SHIFT))             { return '?'; }
	else if(k == (SDL_SCANCODE_MINUS | MOD_ALT_GR))            { return '\\'; }
	else if(k == (SDL_SCANCODE_EQUALS | MOD_SHIFT))            { return '`'; }
	else if(k == SDL_SCANCODE_GRAVE)                           { return '^'; }
	else if(nomods >= SDL_SCANCODE_A && nomods <= SDL_SCANCODE_Z)
	{
		int c = nomods - SDL_SCANCODE_A + 'a';

		if(c == 'z') { c = 'y'; }
		else if(c == 'y') { c = 'z'; }

		if(k & MOD_ALT_GR)
		{
			if(c == 'q') { return '@'; }
		}

		if(k & MOD_SHIFT)
		{
			c = toupper(c);
		}

		return c;
	}
	else if(nomods >= SDL_SCANCODE_1 && nomods <= SDL_SCANCODE_0)
	{
		static const u8 numbers[] =
			{ '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };

		static const u8 numbers_shift[] =
			{ '!', '\"', 0, '$', '%', '&', '/', '(', ')', '=' };

		static const u8 numbers_altgr[] =
			{ 0, 0, 0, 0, 0, 0, '{', '[', ']', '}' };

		int idx = nomods - SDL_SCANCODE_1;

		if(k & MOD_SHIFT)
		{
			return *(numbers_shift + idx);
		}
		else if(k & MOD_ALT_GR)
		{
			return *(numbers_altgr + idx);
		}
		else
		{
			return *(numbers + idx);
		}
	}

	return 0;
}

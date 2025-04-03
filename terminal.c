typedef struct TLINE
{
	struct TLINE *next;
	u32 color;
	int len;
	char text[];
} TLine;

typedef struct
{
	TLine *first;
} Terminal;

static void term_draw(Terminal *term, i32 x, i32 y, i32 min_y)
{
	TLine *cur = term->first;
	while(y > min_y && cur)
	{
		y -= char_height + 8;
		font_string_len(x, y, cur->text, cur->len, cur->color, 0);
		cur = cur->next;
	}
}

static void term_print_va(Terminal *term, u32 color, const char *txt, va_list args)
{
	va_list args2;
	va_copy(args2, args);
	size_t len = vsnprintf(NULL, 0, txt, args) + 1;
	TLine *line = smalloc(sizeof(TLine) + len);
	line->len = len;
	vsnprintf(line->text, len, txt, args2);
	va_end(args2);

	line->color = color;

	TLine *next = term->first;
	term->first = line;
	line->next = next;
}

static void term_print(Terminal *term, u32 color, const char *txt, ...)
{
	va_list args;
	va_start(args, txt);
	term_print_va(term, color, txt, args);
	va_end(args);
}

static void term_clear(Terminal *term)
{
	TLine *cur = term->first;
	while(cur)
	{
		TLine *next = cur->next;
		sfree(cur);
		cur = next;
	}

	term->first = NULL;
}

static void term_save(Terminal *term, char *filename)
{
	FILE *fp = fopen(filename, "w");
	if(!fp)
	{
		term_print(term, COLOR_MSG, "Failed to open file \"%s\" for writing", filename);
		return;
	}

	TLine *cur = term->first;
	while(cur)
	{
		if(cur->color == 0xFFFFFFFF)
		{
			fwrite(cur->text, 1, cur->len, fp);
		}

		cur = cur->next;
	}

	fclose(fp);
}

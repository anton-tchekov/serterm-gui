typedef struct TLINE
{
	struct TLINE *next, *prev;
	u32 color;
	int len;
	int max;
	char *text;
} TLine;

typedef struct
{
	TLine *first, *append, *last;
	pthread_mutex_t mutex;
} Terminal;

static Terminal term;

static void term_init(Terminal *term)
{
	pthread_mutex_init(&term->mutex, NULL);
}

static void term_draw(Terminal *term, i32 x, i32 y, i32 min_y)
{
	pthread_mutex_lock(&term->mutex);
	TLine *cur = term->first;
	while(y > min_y && cur)
	{
		y -= char_height + 8;
		font_string_len(x, y, cur->text, cur->len, cur->color, 0);
		cur = cur->next;
	}

	pthread_mutex_unlock(&term->mutex);
}

static void term_print_va(Terminal *term, u32 color, const char *txt, va_list args)
{
	pthread_mutex_lock(&term->mutex);
	va_list args2;
	va_copy(args2, args);
	int len = vsnprintf(NULL, 0, txt, args);
	TLine *line = smalloc(sizeof(TLine));
	line->text = smalloc(len + 1);
	line->len = len;
	vsnprintf(line->text, len + 1, txt, args2);
	va_end(args2);

	line->color = color;

	TLine *next = term->first;
	if(next)
	{
		next->prev = line;
	}
	else
	{
		term->last = line;
	}

	term->first = line;
	line->next = next;
	pthread_mutex_unlock(&term->mutex);
}

static void term_print(Terminal *term, u32 color, const char *txt, ...)
{
	va_list args;
	va_start(args, txt);
	term_print_va(term, color, txt, args);
	va_end(args);
}

static void tline_append(TLine *line, int c)
{
	if(line->len >= line->max)
	{
		line->max *= 2;
		line->text = srealloc(line->text, line->max);
	}

	line->text[line->len++] = c;
}

static void tline_new(Terminal *term)
{
	TLine *line = smalloc(sizeof(TLine));
	line->max = 16;
	line->len = 0;
	line->text = smalloc(line->max);
	line->color = COLOR_WHITE;

	TLine *next = term->first;
	if(next)
	{
		next->prev = line;
	}
	else
	{
		term->last = line;
	}

	line->prev = NULL;
	term->first = line;
	line->next = next;
	term->append = line;
}

static void term_putchar(Terminal *term, int c)
{
	int first = 0;
	if(c == '\r') { return; }

	if(!term->append)
	{
		tline_new(term);
		first = 1;
	}

	if(c == '\n')
	{
		if(!first)
		{
			tline_new(term);
		}
	}
	else
	{
		tline_append(term->append, c);
	}
}

static void term_append(Terminal *term, char *msg, int len)
{
	pthread_mutex_lock(&term->mutex);
	for(int i = 0; i < len; ++i)
	{
		term_putchar(term, msg[i]);
	}

	pthread_mutex_unlock(&term->mutex);
}

static void term_clear(Terminal *term)
{
	pthread_mutex_lock(&term->mutex);
	TLine *cur = term->first;
	while(cur)
	{
		TLine *next = cur->next;
		sfree(cur->text);
		sfree(cur);
		cur = next;
	}

	term->append = NULL;
	term->first = NULL;
	term->last = NULL;
	pthread_mutex_unlock(&term->mutex);
}

static void term_save(Terminal *term, char *filename)
{
	pthread_mutex_lock(&term->mutex);
	FILE *fp = fopen(filename, "w");
	if(!fp)
	{
		pthread_mutex_unlock(&term->mutex);
		term_print(term, COLOR_MSG, "Failed to open file \"%s\" for writing", filename);
		return;
	}

	TLine *cur = term->last;
	while(cur)
	{
		if(cur->color == 0xFFFFFFFF)
		{
			fwrite(cur->text, 1, cur->len, fp);
			fputc('\n', fp);
		}

		cur = cur->prev;
	}

	fclose(fp);
	pthread_mutex_unlock(&term->mutex);
}

#define PADDING        5
#define SUFFIX_W     256
#define BTN_BAUD_W   128
#define BTN_PAR_SIZE  35
#define BTNS_PER_COL  10
#define LABEL_H       22
#define DEV_STR_MAX  512
#define BAUD_STR_MAX  16

static int ports_list_start = 0;
static char input_text[1024];
static char input_save_text[64] = "log.txt";
static char input_suffix_text[32] = "\\n";
static char button_cs_text[2] = "8";
static int cs = 8;
static int btn_baud_start = 0;
static int btn_baud_end = 0;
static int baudrate = 9600;
static int baud_select = 0;
static char button_baudrate_text[BAUD_STR_MAX] = "9600";
static int stopbits = 1;
static char button_stopbits_text[2] = "1";
static int parity = 'N';
static char button_parity_text[2] = "N";
static Serial serial;

static void button_close_click(Element *e);
static void button_clear_click(Element *e);
static void input_save_enter(Element *e);
static void button_sel_port_click(Element *e);
static void input_send_enter(Element *e);
static void button_cs_click(Element *e);
static void button_parity_click(Element *e);
static void button_stopbits_click(Element *e);
static void button_baudrate_click(Element *e);

static Input input_suffix =
{
	.Type = ELEMENT_TYPE_INPUT,
	.Flags = FLAG_INVISIBLE,
	.X = PADDING,
	.Y = 0,
	.W = SUFFIX_W,
	.Position = 2,
	.Selection = 2,
	.Length = 2,
	.Capacity = sizeof(input_suffix_text),
	.Text = input_suffix_text,
	.Enter = NULL
};

static Label label_params =
{
	.Type = ELEMENT_TYPE_LABEL,
	.Flags = 0,
	.X = PADDING,
	.Y = PADDING,
	.Text = "Connection Parameters:"
};

static Label label_actions =
{
	.Type = ELEMENT_TYPE_LABEL,
	.Flags = FLAG_INVISIBLE,
	.X = PADDING,
	.Y = PADDING,
	.Text = "Actions:"
};

static Button button_close =
{
	.Type = ELEMENT_TYPE_BUTTON,
	.Flags = FLAG_INVISIBLE,
	.Tag = 0,
	.X = PADDING,
	.Y = PADDING + LABEL_H,
	.W = BTN_BAUD_W,
	.H = BTN_PAR_SIZE,
	.Text = "Close Port",
	.Click = button_close_click
};

static Button button_clear =
{
	.Type = ELEMENT_TYPE_BUTTON,
	.Flags = FLAG_INVISIBLE,
	.Tag = 0,
	.X = BTN_BAUD_W + 2 * PADDING,
	.Y = PADDING + LABEL_H,
	.W = BTN_BAUD_W,
	.H = BTN_PAR_SIZE,
	.Text = "Clear Terminal",
	.Click = button_clear_click
};

static Label label_save =
{
	.Type = ELEMENT_TYPE_LABEL,
	.Flags = FLAG_INVISIBLE,
	.X = 2 * BTN_BAUD_W + 3 * PADDING,
	.Y = PADDING,
	.Text = "Save As: (Enter)"
};

static Input input_save =
{
	.Type = ELEMENT_TYPE_INPUT,
	.Flags = FLAG_INVISIBLE,
	.X = 2 * BTN_BAUD_W + 3 * PADDING,
	.Y = PADDING + LABEL_H,
	.W = 256,
	.Position = 7,
	.Selection = 7,
	.Length = 7,
	.Capacity = sizeof(input_save_text),
	.Text = input_save_text,
	.Enter = input_save_enter
};

static Button button_sel_port =
{
	.Type = ELEMENT_TYPE_BUTTON,
	.Flags = 0,
	.Tag = 0,
	.X = 5 * PADDING + BTN_BAUD_W + 3 * BTN_PAR_SIZE,
	.Y = PADDING + LABEL_H,
	.W = BTN_BAUD_W,
	.H = BTN_PAR_SIZE,
	.Text = "Reload Ports",
	.Click = button_sel_port_click
};

static Label label_send =
{
	.Type = ELEMENT_TYPE_LABEL,
	.Flags = FLAG_INVISIBLE,
	.X = PADDING,
	.Y = 0,
	.Text = "Message:"
};

static Input input_send =
{
	.Type = ELEMENT_TYPE_INPUT,
	.Flags = FLAG_INVISIBLE,
	.X = PADDING,
	.Y = 0,
	.W = 0,
	.Position = 0,
	.Selection = 0,
	.Length = 0,
	.Capacity = sizeof(input_text),
	.Text = input_text,
	.Enter = input_send_enter
};

static Label label_suffix =
{
	.Type = ELEMENT_TYPE_LABEL,
	.Flags = FLAG_INVISIBLE,
	.X = PADDING,
	.Y = 0,
	.Text = "Line ending:"
};

static Button button_cs =
{
	.Type = ELEMENT_TYPE_BUTTON,
	.Flags = 0,
	.Tag = 0,
	.X = BTN_BAUD_W + 2 * PADDING,
	.Y = PADDING + LABEL_H,
	.W = BTN_PAR_SIZE,
	.H = BTN_PAR_SIZE,
	.Text = button_cs_text,
	.Click = button_cs_click
};

static Button button_parity =
{
	.Type = ELEMENT_TYPE_BUTTON,
	.Flags = 0,
	.Tag = 0,
	.X = 3 * PADDING + BTN_BAUD_W + BTN_PAR_SIZE,
	.Y = PADDING + LABEL_H,
	.W = BTN_PAR_SIZE,
	.H = BTN_PAR_SIZE,
	.Text = button_parity_text,
	.Click = button_parity_click
};

static Button button_stopbits =
{
	.Type = ELEMENT_TYPE_BUTTON,
	.Flags = 0,
	.Tag = 0,
	.X = 4 * PADDING + BTN_BAUD_W + 2 * BTN_PAR_SIZE,
	.Y = PADDING + LABEL_H,
	.W = BTN_PAR_SIZE,
	.H = BTN_PAR_SIZE,
	.Text = button_stopbits_text,
	.Click = button_stopbits_click
};

static Button button_baudrate =
{
	.Type = ELEMENT_TYPE_BUTTON,
	.Flags = 0,
	.Tag = 0,
	.X = PADDING,
	.Y = PADDING + LABEL_H,
	.W = BTN_BAUD_W,
	.H = BTN_PAR_SIZE,
	.Text = button_baudrate_text,
	.Click = button_baudrate_click
};

static void *elems[128] =
{
	&label_params,
	&button_sel_port,
	&button_cs,
	&button_parity,
	&button_stopbits,
	&button_baudrate,
	&label_suffix,
	&input_suffix,
	&label_send,
	&input_send,
	&label_actions,
	&button_close,
	&button_clear,
	&label_save,
	&input_save
};

static Window frame =
{
	elems, 15, -1, 1
};

static void ports_hide(void)
{
	for(int i = ports_list_start; i < frame.Count; ++i)
	{
		Button *b = elems[i];
		b->Flags |= FLAG_INVISIBLE;
	}
}

static void buttons_baud_hide(void)
{
	for(int i = btn_baud_start; i < btn_baud_end; ++i)
	{
		Button *b = (Button *)elems[i];
		b->Flags |= FLAG_INVISIBLE;
	}
}

static void input_send_enter(Element *e)
{
	if(input_send.Length == 0)
	{
		return;
	}

	char t0[sizeof(input_text)];
	char t1[sizeof(input_suffix)];
	char buf[sizeof(t0) + sizeof(t1)];

	int r0 = escseq(input_send.Text, input_send.Length, t0);
	if(r0 < 0)
	{
		term_print(&term, COLOR_MSG, "Invalid escape sequence in message");
		return;
	}

	int r1 = escseq(input_suffix.Text, input_suffix.Length, t1);
	if(r1 < 0)
	{
		term_print(&term, COLOR_MSG, "Invalid escape sequence in line ending");
		return;
	}

	memcpy(buf, t0, r0);
	memcpy(buf + r0, t1, r1);
	serial_send(&serial, buf, r0 + r1);
	input_clear(&input_send);
	(void)e;
}

static void button_clear_click(Element *e)
{
	term_clear(&term);
	(void)e;
}

static void input_save_enter(Element *e)
{
	term_save(&term, input_save_text);
	term_print(&term, COLOR_MSG, "File \"%s\" saved", input_save_text);
	(void)e;
}

static int cs_cycle(int v)
{
	switch(v)
	{
	case 8: return 7;
	}

	return 8;
}

static void button_cs_click(Element *e)
{
	cs = cs_cycle(cs);
	button_cs_text[0] = cs + '0';
	(void)e;
}

static int parity_cycle(int p)
{
	switch(p)
	{
	case 'N': return 'E';
	case 'E': return 'O';
	}

	return 'N';
}

static void button_parity_click(Element *e)
{
	parity = parity_cycle(parity);
	button_parity_text[0] = parity;
	(void)e;
}

static int stopbits_cycle(int s)
{
	return s == 1 ? 2 : 1;
}

static void button_stopbits_click(Element *e)
{
	stopbits = stopbits_cycle(stopbits);
	button_stopbits_text[0] = stopbits + '0';
	(void)e;
}

static void button_port_open_click(Element *e)
{
	Button *b = (Button *)e;
	serial_connect(&serial, b->Text);
}

static void layout_connected(void)
{
	input_send.Flags &= ~FLAG_INVISIBLE;
	label_send.Flags &= ~FLAG_INVISIBLE;

	input_suffix.Flags &= ~FLAG_INVISIBLE;
	label_suffix.Flags &= ~FLAG_INVISIBLE;

	label_actions.Flags &= ~FLAG_INVISIBLE;
	button_close.Flags &= ~FLAG_INVISIBLE;
	button_clear.Flags &= ~FLAG_INVISIBLE;
	label_save.Flags &= ~FLAG_INVISIBLE;
	input_save.Flags &= ~FLAG_INVISIBLE;

	button_sel_port.Flags |= FLAG_INVISIBLE;
	button_cs.Flags |= FLAG_INVISIBLE;
	button_stopbits.Flags |= FLAG_INVISIBLE;
	button_baudrate.Flags |= FLAG_INVISIBLE;
	button_parity.Flags |= FLAG_INVISIBLE;
	label_params.Flags |= FLAG_INVISIBLE;

	ports_hide();
	buttons_baud_hide();
}

static void free_dev_btns(void)
{
	for(int i = ports_list_start; i < frame.Count; ++i)
	{
		Button *b = elems[i];
		sfree(b->Text);
		sfree(b);
	}

	frame.Count = ports_list_start;
}

static void button_sel_port_click(Element *e)
{
	DIR *dp;
	struct dirent *ep;

	if(!(dp = opendir("/dev/")))
	{
		term_print(&term, COLOR_MSG, "Failed to list devices (Error %d)", errno);
		term_print(&term, COLOR_MSG, "%s", strerror(errno));
		return;
	}

	Button button_port =
	{
		.Type = ELEMENT_TYPE_BUTTON,
		.Flags = 0,
		.Tag = 0,
		.X = 6 * PADDING + 2 * BTN_BAUD_W + 3 * BTN_PAR_SIZE,
		.Y = PADDING + LABEL_H,
		.W = BTN_BAUD_W,
		.H = BTN_PAR_SIZE,
		.Text = NULL,
		.Click = button_port_open_click
	};

	free_dev_btns();

	int i = 0;
	while((ep = readdir(dp)))
	{
		const char *name = ep->d_name;
		if(!strncmp(name, "ttyUSB", 6) ||
			!strncmp(name, "ttyACM", 6))
		{
			char *text = smalloc(DEV_STR_MAX);
			Button *btn = smalloc(sizeof(Button));
			snprintf(text, DEV_STR_MAX, "/dev/%s", name);
			memcpy(btn, &button_port, sizeof(Button));
			btn->Text = text;
			btn->X += i * (BTN_BAUD_W + PADDING);
			elems[frame.Count++] = btn;
			++i;
			term_print(&term, COLOR_MSG, "Found USB serial device: %s", text);
		}
	}

	closedir(dp);
	if(i == 0)
	{
		term_print(&term, COLOR_MSG, "No Devices");
	}

	(void)e;
}

static void buttons_baud_show(void)
{
	for(int i = btn_baud_start; i < btn_baud_end; ++i)
	{
		Button *b = (Button *)elems[i];
		b->Flags &= ~FLAG_INVISIBLE;
	}
}

static void ports_show(void)
{
	for(int i = ports_list_start; i < frame.Count; ++i)
	{
		Button *b = elems[i];
		b->Flags &= ~FLAG_INVISIBLE;
	}
}

static void baudrate_select(void)
{
	baud_select = 0;
	buttons_baud_hide();
	snprintf(button_baudrate_text, sizeof(button_baudrate_text),
		"%d", baudrate);
}

static void button_baudrate_click(Element *e)
{
	if(baud_select)
	{
		baudrate_select();
	}
	else
	{
		baud_select = 1;
		strcpy(button_baudrate_text, "[ Select ]");
		buttons_baud_show();
	}

	(void)e;
}

static void button_baudrate_select(Element *e)
{
	Button *b = (Button *)e;
	baudrate = b->Tag;
	baudrate_select();
}

static void layout_resize(void)
{
	label_send.Y = height - INPUT_HEIGHT - PADDING - LABEL_H;
	input_send.W = width - 3 * PADDING - SUFFIX_W;
	input_send.Y = height - INPUT_HEIGHT - PADDING;

	label_suffix.X = width - SUFFIX_W - PADDING;
	label_suffix.Y = label_send.Y;

	input_suffix.X = width - SUFFIX_W - PADDING;
	input_suffix.Y = input_send.Y;
}

static void layout_disconnected(void)
{
	input_send.Flags |= FLAG_INVISIBLE;
	label_send.Flags |= FLAG_INVISIBLE;

	input_suffix.Flags |= FLAG_INVISIBLE;
	label_suffix.Flags |= FLAG_INVISIBLE;

	label_actions.Flags |= FLAG_INVISIBLE;
	button_close.Flags |= FLAG_INVISIBLE;
	button_clear.Flags |= FLAG_INVISIBLE;
	label_save.Flags |= FLAG_INVISIBLE;
	input_save.Flags |= FLAG_INVISIBLE;

	button_sel_port.Flags &= ~FLAG_INVISIBLE;
	button_cs.Flags &= ~FLAG_INVISIBLE;
	button_stopbits.Flags &= ~FLAG_INVISIBLE;
	button_baudrate.Flags &= ~FLAG_INVISIBLE;
	button_parity.Flags &= ~FLAG_INVISIBLE;
	label_params.Flags &= ~FLAG_INVISIBLE;

	ports_show();
}

static void button_close_click(Element *e)
{
	serial_disconnect(&serial);
	(void)e;
}

static void layout_init(void)
{
	serial_init(&serial);

	Button button_b =
	{
		.Type = ELEMENT_TYPE_BUTTON,
		.Flags = FLAG_INVISIBLE,
		.Tag = 0,
		.X = 0,
		.Y = 0,
		.W = BTN_BAUD_W,
		.H = BTN_PAR_SIZE,
		.Text = NULL,
		.Click = button_baudrate_select
	};

	btn_baud_start = frame.Count;
	for(int i = 0; i < (int)ARRLEN(baudrates); ++i)
	{
		int baud = baudrates[i].baud;
		char *text = smalloc(BAUD_STR_MAX);
		Button *btn = smalloc(sizeof(Button));
		snprintf(text, BAUD_STR_MAX, "%d", baud);
		memcpy(btn, &button_b, sizeof(Button));
		btn->Text = text;
		btn->Y += 2 * PADDING + BTN_PAR_SIZE + LABEL_H + (i % BTNS_PER_COL) * (BTN_PAR_SIZE + PADDING);
		btn->X = PADDING + (i / BTNS_PER_COL) * (BTN_BAUD_W + PADDING);
		btn->Tag = baud;
		elems[frame.Count++] = btn;
	}

	btn_baud_end = frame.Count;

	ports_list_start = frame.Count;

	layout_resize();
	window_open(&frame);

	term_print(&term, COLOR_MSG, "Serial Terminal Initialized");
	term_print(&term, COLOR_MSG, "You can use escape sequences like '\\n', '\\t', '\\x80', Press enter to send");

	button_sel_port_click(NULL);
}

static void layout_destroy(void)
{
	serial_shutdown(&serial);
	for(int i = btn_baud_start; i < btn_baud_end; ++i)
	{
		Button *b = elems[i];
		sfree(b->Text);
		sfree(b);
	}

	free_dev_btns();
}

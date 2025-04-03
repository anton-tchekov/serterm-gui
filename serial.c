/*
void *thread_recv(void *arg)
{
	uint8_t buf[1024];
	int fd, n;

	fd = *(int *)arg;
	for(;;)
	{
		n = read(fd, buf, sizeof(buf));
		if(n < 0)
		{
			fprintf(stderr, "(read) Error %d: %s\n", errno, strerror(errno));
			exit(1);
		}
		else if(n == 0)
		{
			fprintf(stderr, "(read) Disconnected\n");
			break;
		}

		write(STDOUT_FILENO, buf, n);
	}

	return NULL;
}

*/

typedef struct
{
	int baud, baud_mask;
} Baudrate;

static Baudrate baudrates[] =
{
	{ .baud =      50, .baud_mask = B50      },
	{ .baud =      75, .baud_mask = B75      },
	{ .baud =     110, .baud_mask = B110     },
	{ .baud =     134, .baud_mask = B134     },
	{ .baud =     150, .baud_mask = B150     },
	{ .baud =     200, .baud_mask = B200     },
	{ .baud =     300, .baud_mask = B300     },
	{ .baud =     600, .baud_mask = B600     },
	{ .baud =    1200, .baud_mask = B1200    },
	{ .baud =    1800, .baud_mask = B1800    },
	{ .baud =    2400, .baud_mask = B2400    },
	{ .baud =    4800, .baud_mask = B4800    },
	{ .baud =    9600, .baud_mask = B9600    },
	{ .baud =   19200, .baud_mask = B19200   },
	{ .baud =   38400, .baud_mask = B38400   },
	{ .baud =   57600, .baud_mask = B57600   },
	{ .baud =  115200, .baud_mask = B115200  },
	{ .baud =  230400, .baud_mask = B230400  },
	{ .baud =  460800, .baud_mask = B460800  },
	{ .baud =  500000, .baud_mask = B500000  },
	{ .baud =  576000, .baud_mask = B576000  },
	{ .baud =  921600, .baud_mask = B921600  },
	{ .baud = 1000000, .baud_mask = B1000000 },
	{ .baud = 1152000, .baud_mask = B1152000 },
	{ .baud = 1500000, .baud_mask = B1500000 },
	{ .baud = 2000000, .baud_mask = B2000000 }
};

static int baudrate_mask(int baud)
{
	for(int i = 0; i < (int)ARRLEN(baudrates); ++i)
	{
		if(baudrates[i].baud == baud)
		{
			return baudrates[i].baud_mask;
		}
	}

	return 0;
}

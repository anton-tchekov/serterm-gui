
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

enum
{
	MSG_CONNECT,
	MSG_SEND,
	MSG_DISCONNECT,
	MSG_SHUTDOWN,
	MSG_CONNECTED,
	MSG_DISCONNECTED,
	MSG_RECEIVED,
	MSG_INFO
};

typedef struct
{
	int fdc[2];
	Queue sendq;
	pthread_t thread;
} Serial;

static void thread_notify(Serial *serial)
{
	write(serial->fdc[1], "0", 1);
}

static void serial_connect(Serial *serial, char *port)
{
	msg_push(&serial->sendq, MSG_CONNECT, port, strlen(port));
	thread_notify(serial);
}

static void serial_send(Serial *serial, char *str, int len)
{
	printf(">>>>> AAA!!!!\n");
	msg_push(&serial->sendq, MSG_SEND, str, len);
	printf(">>>>> BBB!!!!\n");
	thread_notify(serial);
	printf(">>>>> CCC!!!!\n");
}

static void serial_disconnect(Serial *serial)
{
	msg_push(&serial->sendq, MSG_DISCONNECT, NULL, 0);
	thread_notify(serial);
}

static void serial_shutdown(Serial *serial)
{
	msg_push(&serial->sendq, MSG_SHUTDOWN, NULL, 0);
	thread_notify(serial);
}

static void *thread_serial(void *arg)
{
	Serial *serial = arg;
	int fd = -1;
	int cfd = serial->fdc[0];
	for(;;)
	{
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(cfd, &rfds);
		if(fd >= 0)
		{
			FD_SET(fd, &rfds);
		}

		int ret = select((cfd > fd ? cfd : fd) + 1, &rfds, NULL, NULL, NULL);
		if(ret == -1)
		{
			perror("select()");
			break;
		}
		else if(ret)
		{
			if(FD_ISSET(cfd, &rfds))
			{
				char buf[32];
				read(cfd, buf, sizeof(buf));
				Message *msg;
				printf(">>>>> herer!!!!\n");
				while((msg = msg_pop(&serial->sendq)))
				{
					printf(">>>>>>>>>>>>> sdasd\n");
					switch(msg->Type)
					{
					case MSG_CONNECT:
						char *name = msg->Data;
						struct termios tty;

						if((fd = open(name, O_RDWR | O_NOCTTY | O_SYNC)) < 0)
						{
							//serial_error(name);
							break;
						}

						if(tcgetattr(fd, &tty))
						{
							close(fd);
							fd = -1;
							//serial_error(name);
							break;
						}

						int speed = baudrate_mask(9600);
						cfsetospeed(&tty, speed);
						cfsetispeed(&tty, speed);
						tty.c_lflag = 0;
						tty.c_oflag = 0;
						tty.c_cc[VMIN]  = 1;
						tty.c_cc[VTIME] = 0;
						tty.c_iflag &= ~(IGNBRK | IXON | IXOFF | IXANY);
						tty.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS);
						tty.c_cflag |= (CLOCAL | CREAD | CS8);
						if(tcsetattr(fd, TCSANOW, &tty))
						{
							close(fd);
							fd = -1;
							//serial_error(name);
							break;
						}


						printf("CONN\n");
						//term_print(&term, COLOR_MSG, "Opened Port %s - %d baud %d%c%d",
						//	name, baudrate, cs, parity, stopbits);
						break;

					case MSG_DISCONNECT:
						if(fd >= 0)
						{
							// info
							printf("DISCONN\n");
							close(fd);
							fd = -1;
						}
						break;

					case MSG_SEND:
						break;

					case MSG_SHUTDOWN:
						if(fd >= 0)
						{
							close(fd);
							fd = -1;
						}
						return NULL;
					}

					sfree(msg);
				}
			}
			else if(fd >= 0 && FD_ISSET(fd, &rfds))
			{
				printf("RECV\n");
				char buf[1024];
				int n = read(fd, buf, sizeof(buf));
				printf("%.*s", n, buf);
			}
		}
	}

	(void)arg;
	return NULL;
}

static void serial_init(Serial *serial)
{
	if(pipe(serial->fdc) < 0)
	{
		fprintf(stderr, "Failed to create pipe\n");
		exit(1);
	}

	queue_init(&serial->sendq);
	if(pthread_create(&serial->thread, NULL, thread_serial, serial))
	{
		fprintf(stderr, "Failed to start serial port thread\n");
		exit(1);
	}
}

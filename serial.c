
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

typedef struct
{
	int baud;
	int cs;
	int parity;
	int stopbits;
	char *port;
} SerialConnInfo;

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
	MSG_DISCONNECTED
};

typedef struct
{
	int fdc[2];
	Queue sendq, readq;
	pthread_t thread;
} Serial;

static void thread_notify(Serial *serial)
{
	write(serial->fdc[1], "0", 1);
}

static void serial_connect(Serial *serial, SerialConnInfo *info)
{
	int len = strlen(info->port) + 1;
	char *port = smalloc(len);
	memcpy(port, info->port, len);
	info->port = port;

	msg_push(&serial->sendq, MSG_CONNECT, info, sizeof(*info));
	thread_notify(serial);
}

static void serial_send(Serial *serial, char *str, int len)
{
	msg_push(&serial->sendq, MSG_SEND, str, len);
	thread_notify(serial);
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

static void serial_disconnected(Serial *serial, SerialConnInfo *info)
{
	term_print(&term, COLOR_MSG, "Closed port %s", info->port);
	sfree(info->port);
	info->port = NULL;
	msg_push(&serial->readq, MSG_DISCONNECTED, NULL, 0);
	gfx_notify();
}

static void serial_connected(Serial *serial, SerialConnInfo *info)
{
	term_print(&term, COLOR_MSG, "Opened port %s - %d baud %d%c%d",
		info->port, info->baud, info->cs, info->parity, info->stopbits);
	msg_push(&serial->readq, MSG_CONNECTED, NULL, 0);
	gfx_notify();
}

static void *thread_serial(void *arg)
{
	SerialConnInfo info;
	memset(&info, 0, sizeof(info));
	Serial *serial = arg;
	int fd = -1;
	int cfd = serial->fdc[0];
	int running = 1;
	while(running)
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
			exit(1);
		}
		else if(ret)
		{
			if(FD_ISSET(cfd, &rfds))
			{
				char buf[32];
				read(cfd, buf, sizeof(buf));
				Message *msg;
				while((msg = msg_pop(&serial->sendq)))
				{
					switch(msg->Type)
					{
					case MSG_CONNECT:
						memcpy(&info, msg->Data, sizeof(SerialConnInfo));

						printf("%s\n", info.port);

						struct termios tty;

						if((fd = open(info.port, O_RDWR | O_NOCTTY | O_SYNC)) < 0)
						{
							term_print(&term, COLOR_MSG, "Failed to open port %s (%d) - %s",
								info.port, errno, strerror(errno));
							gfx_notify();
							break;
						}

						if(tcgetattr(fd, &tty))
						{
							close(fd);
							fd = -1;
							term_print(&term, COLOR_MSG, "Failed to open port %s (%d) - %s",
								info.port, errno, strerror(errno));
							gfx_notify();
							break;
						}

						int speed = baudrate_mask(info.baud);
						cfsetospeed(&tty, speed);
						cfsetispeed(&tty, speed);
						tty.c_lflag = 0;
						tty.c_oflag = 0;
						tty.c_cc[VMIN]  = 1;
						tty.c_cc[VTIME] = 0;
						tty.c_iflag &= ~(IGNBRK | IXON | IXOFF | IXANY);
						tty.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS);
						tty.c_cflag |= (CLOCAL | CREAD);

						if(info.parity == 'E')
						{
							tty.c_cflag |= PARENB;
						}
						else if(info.parity == 'O')
						{
							tty.c_cflag |= PARENB | PARODD;
						}

						if(info.stopbits == 2)
						{
							tty.c_cflag |= CSTOPB;
						}

						if(info.cs == 7)
						{
							tty.c_cflag |= CS7;
						}
						else if(info.cs == 8)
						{
							tty.c_cflag |= CS8;
						}

						if(tcsetattr(fd, TCSANOW, &tty))
						{
							close(fd);
							fd = -1;
							term_print(&term, COLOR_MSG, "Failed to open port %s (%d) - %s",
								info.port, errno, strerror(errno));
							gfx_notify();
							break;
						}

						serial_connected(serial, &info);
						break;

					case MSG_DISCONNECT:
						if(fd >= 0)
						{
							serial_disconnected(serial, &info);
							close(fd);
							fd = -1;
						}
						break;

					case MSG_SEND:
						if(fd >= 0)
						{
							if(write(fd, msg->Data, msg->Len) != msg->Len)
							{
								term_print(&term, COLOR_MSG, "Send failed (%d) - %s",
									errno, strerror(errno));
								close(fd);
								fd = -1;
								serial_disconnected(serial, &info);
							}
						}
						break;

					case MSG_SHUTDOWN:
						if(fd >= 0)
						{
							if(info.port)
							{
								sfree(info.port);
							}

							close(fd);
							fd = -1;
							running = 0;
						}
						break;
					}

					sfree(msg);
				}
			}
			else if(fd >= 0 && FD_ISSET(fd, &rfds))
			{
				char buf[1024];
				int n = read(fd, buf, sizeof(buf));
				if(n > 0)
				{
					term_append(&term, buf, n);
					gfx_notify();
				}
				else
				{
					term_print(&term, COLOR_MSG, "Read failed (%d) - %s",
						errno, strerror(errno));
					close(fd);
					fd = -1;
					serial_disconnected(serial, &info);
				}
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
	queue_init(&serial->readq);
	if(pthread_create(&serial->thread, NULL, thread_serial, serial))
	{
		fprintf(stderr, "Failed to start serial port thread\n");
		exit(1);
	}
}

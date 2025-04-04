typedef struct MESSAGE
{
	struct MESSAGE *Prev, *Next;
	int Type;
	char Data[];
} Message;

typedef struct
{
	Message *First, *Last;
	pthread_mutex_t Mutex;
} Queue;

static void queue_init(Queue *queue)
{
	queue->First = NULL;
	queue->Last = NULL;
	pthread_mutex_init(&queue->Mutex, NULL);
}

static void msg_push(Queue *queue, int type, char *data, int len)
{
	printf("########## push\n");

	Message *msg = smalloc(sizeof(Message) + len + 1);
	msg->Type = type;
	msg->Prev = NULL;
	memcpy(msg->Data, data, len);
	msg->Data[len] = '\0';

	printf("########## lol\n");

	pthread_mutex_lock(&queue->Mutex);

	printf("########## in\n");

	if(!queue->Last)
	{
		queue->Last = msg;
	}

	msg->Next = queue->First;
	if(queue->First)
	{
		queue->First->Prev = msg;
	}

	queue->First = msg;
	pthread_mutex_unlock(&queue->Mutex);

	printf("########## out\n");
}

static Message *msg_pop(Queue *queue)
{
	pthread_mutex_lock(&queue->Mutex);
	if(!queue->Last)
	{
		pthread_mutex_unlock(&queue->Mutex);
		return NULL;
	}

	Message *msg = queue->Last;
	if(queue->Last->Prev)
	{
		queue->Last = queue->Last->Prev;
		queue->Last->Next = NULL;
	}
	else
	{
		queue->Last = NULL;
		queue->First = NULL;
	}

	pthread_mutex_unlock(&queue->Mutex);
	msg->Next = NULL;
	msg->Prev = NULL;
	return msg;
}

#define ARRLEN(X) (sizeof(X) / sizeof(*X))

#define GENERATE_ENUM(ENUM) ENUM
#define GENERATE_STRING(STRING) #STRING

static size_t alloc_cnt, free_cnt, total_bytes;

static void mcheck(void *m, size_t size)
{
	if(!m)
	{
		fprintf(stderr, "Memory allocation of %zu bytes failed\n", size);
		exit(1);
	}

	if(size)
	{
		++alloc_cnt;
		total_bytes += size;
	}
}

static void *smalloc(size_t size)
{
	void *m = malloc(size);
	mcheck(m, size);
	return m;
}

static void *scalloc(size_t size)
{
	void *m = calloc(size, 1);
	mcheck(m, size);
	return m;
}

static void sfree(void *p)
{
	if(p)
	{
		++free_cnt;
		free(p);
	}
}

static void print_allocs(void)
{
	printf("%zu allocs, %zu frees, %zu bytes total\n",
		alloc_cnt, free_cnt, total_bytes);
}

static i32 i32_max(i32 a, i32 b)
{
	return a > b ? a : b;
}

static i32 i32_min(i32 a, i32 b)
{
	return a < b ? a : b;
}

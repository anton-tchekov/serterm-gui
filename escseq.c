static int escseq(char *in, int len, char *out)
{
	for(int i = 0; i < len; ++i)
	{
		int c = in[i];
		int e = c;
		if(c == '\\')
		{
			if(++i >= len)
			{
				return -1;
			}

			c = buf[i];
			switch(c)
			{
			case 'a': e = '\a'; break;
			case 'b': e = '\b'; break;
			case 'e': e = 0x1B; break;
			case 'f': e = '\f'; break;
			case 'n': e = '\n'; break;
			case 'r': e = '\r'; break;
			case 't': e = '\t'; break;
			case 'v': e = '\v'; break;
			case '\\': e = '\\'; break;
			case 'x':
				{
					int j = 0;
					for(; j < 2; ++j)
					{
						++i;
						if(i >= len)
						{
							return -1;
						}

						c = in[i];
						if(isxdigit(c))
						{
							e *= 16;
							if(c >= '0' && c <= '9')
							{
								e += c - '0';
							}
							else if(c >= 'A' && c <= 'F')
							{
								e += c - 'A';
							}
							else if(c >= 'a' && c <= 'f')
							{
								e += c - 'a';
							}
							break;
						}
					}

					if(j == 0)
					{
						return -1;
					}
				}
				break;

			default:
				{
					int j = 0;
					e = 0;
					for(; j < 3; ++j)
					{
						++i;
						if(i >= len)
						{
							return -1;
						}

						c = in[i];
						if(isdigit(c))
						{
							e *= 8;
							e += c - '0';
						}
					}

					if(j == 0)
					{
						return -1;
					}
				}
				break;
			}
		}

		out[o++] = e;
	}

	return 0;
}

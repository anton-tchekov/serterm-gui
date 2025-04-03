void setup(void)
{
	Serial.begin(9600);
}

int i = 0;
char buf[128];

void loop(void)
{
	static uint32_t last;

	if(Serial.available())
	{
		int c = Serial.read();
		if(i < sizeof(buf) - 1)
		{
			buf[i++] = c;
		}

		if(c == '\n')
		{
			buf[i] = '\0';
			Serial.print(buf);
			i = 0;
		}
	}

	uint32_t now = millis();
	if((now - last) > 1000)
	{
		Serial.print("Hello world!\n");
		last = now;
	}
}

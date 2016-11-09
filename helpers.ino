//print a PSTR string on the Serial port
void SerialPrint_P(PGM_P str, boolean println) {
	if (!SERIAL_OUTPUT) return;
	for (uint8_t c; (c = pgm_read_byte(str)); str++) Serial.write(c);
	if (println) Serial.write('\n');
}

void PSTRtoSTR(char *buffer, PGM_P pstr)
{
	//buffer = (char *)malloc(strlen_P(pstr) + 1);
	strcpy_P(buffer, pstr);
	//free(buffer);
}

//print a PSTR string on the Serial port
void SerialPrint_P(PGM_P str) {
	SerialPrint_P(str, true);
}

void SerialPrintln(char* str)
{
	if (!SERIAL_OUTPUT) return;
	Serial.println(str);
}

void SerialPrint(char* str)
{
	if (!SERIAL_OUTPUT) return;
	Serial.print(str);
}

void SerialPrintln(uint32_t str)
{
	if (!SERIAL_OUTPUT) return;
	Serial.println(str);
}

void SerialPrint(uint32_t str)
{
	if (!SERIAL_OUTPUT) return;
	Serial.print(str);
}

int freeRam()
{
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

int decimal(double x)
{
	int temp = x * 10;
	int temp2 = (int)x * 10;
	return temp - temp2;
}

char* URLEncode(char* msg)
{
    const char *hex PROGMEM = "0123456789abcdef";
    String encodedMsg = "";

    while (*msg!='\0'){
        if( ('a' <= *msg && *msg <= 'z')
                || ('A' <= *msg && *msg <= 'Z')
                || ('0' <= *msg && *msg <= '9') ) {
            encodedMsg += *msg;
        } else {
            encodedMsg += '%';
            encodedMsg += hex[*msg >> 4];
            encodedMsg += hex[*msg & 15];
        }
        msg++;
    }
    
    int strlen = encodedMsg.length() + 1;
    strlen = 300;
    char msg_buf[strlen];
    encodedMsg.toCharArray(msg_buf, strlen);
    return msg_buf;
}

void wch_enable()
{
  if (WCH_ENABLED) wdt_enable(WDTO_8S);
}

void wch_disable()
{
  if (WCH_ENABLED) wdt_disable();
}


void wch_reset()
{
  if (WCH_ENABLED) wdt_reset();
}

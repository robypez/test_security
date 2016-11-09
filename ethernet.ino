#define buffersize 150
char buffer_relay[buffersize];

void domotic_command(char command[])
{
        wch_reset();
	if (!ENABLE_DOMOTIC_CONTROL) return;
	if (!eth_enabled) return;
	SerialPrint_P(PSTR("Sending domotic command:"), 0);
	Serial.println(command);

	if (domotic_client.connect(domotic_server_ip, domotic_server_port))
        {
          domotic_client.println(command);
          domotic_client.stop();
        }
        
        wch_reset();
}

void updateDHT()
{
	dht_ts = millis();

	DHT.read22(DHT22_PIN);
	if (SERIAL_OUTPUT)
	{
		SerialPrint_P(PSTR("Reading DHT Values:"), 0);
		Serial.print(DHT.humidity, 0);
		SerialPrint_P(PSTR("%,"), 0);
		Serial.print(DHT.temperature, 1);
		SerialPrint_P(PSTR("C"), 1);
		SerialPrint_P(PSTR(""), 1);
	}

	sprintf(buffer_relay, "{t3:%d.%d,h3:%d}", (int)DHT.temperature, decimal(DHT.temperature), (int)DHT.humidity);

	BrowseUrl(emon_client, emon_ip, emon_port, emon_url, buffer_relay);
}

void enableIpCam()
{
	SerialPrint_P(PSTR("ENABLE CAM"), 1);
	if (ENABLE_IPCAM_CONTROL) BrowseUrl(ipcam_client, ipcam_ip, ipcam_port, enable_cam, "", ipcamAuth);
}

void disableIpCam()
{
	SerialPrint_P(PSTR("DISABLE CAM"), 1);
	if (ENABLE_IPCAM_CONTROL) BrowseUrl(ipcam_client, ipcam_ip, ipcam_port, disable_cam, "", ipcamAuth);
}

void sendMessage(PGM_P body)
{
	char *buffer = (char *)malloc(strlen_P(body) + 1);
	strcpy_P(buffer, body);
	sendEmail(buffer);
	pushNotification(buffer);
	free(buffer);
}

void sendMessage(char *body)
{
	sendEmail(body);
	pushNotification(body);
}

bool pushNotification(char* body)
{
	if (ENABLE_PUSH) BrowseUrl(push_client, push_website, 80, push_url, URLEncode(body));
}

bool pushNotification(PGM_P body)
{
	if (ENABLE_PUSH) 
        {
			char *buffer = (char *)malloc(strlen_P(body) + 1);
			strcpy_P(buffer, body);
			BrowseUrl(push_client, push_website, 80, push_url, URLEncode(buffer));
			free(buffer);
        }
}


bool BrowseUrl(EthernetClient client, byte ip[], int port, char* url, char* params)
{
        wch_reset();
	if (!eth_enabled) return true;

	if (client.connect(ip, port)) {
		SerialPrint_P(PSTR("-----------------"));
		SerialPrint_P(PSTR("Calling url: "), 0);
		if (SERIAL_OUTPUT)
		{
			Serial.print(IPAddress(ip));
			Serial.print(":");
			Serial.print(port);
			Serial.print(url);
			Serial.println(params);
			SerialPrint_P(PSTR(""), 1);
		}

		client.print(F("GET "));
		client.print(url);
		client.print(params);
		client.println(F("HTTP/1.1"));
		client.print(F("Host: "));
		client.println(IPAddress(ip));
		client.println(F("Connection: close"));
		client.println();

		SerialPrint_P(PSTR("SERVER ANSWER:"));
		while (client.connected() && !client.available()) delay(1); //waits for data
		while (client.connected() || client.available()) { //connected or data available
			char c = client.read();
			if (SERIAL_OUTPUT) Serial.print(c);
		}
		SerialPrint_P(PSTR("-----------------"));
		client.stop();
                wch_reset();
		return true;
	}
	else {
		// kf you didn't get a connection to the server:
		SerialPrint_P(PSTR("connection failed"));
                wch_reset();
		return false;
	}
}


bool BrowseUrl(EthernetClient client, byte ip[], int port, char* url, char* params, char* auth)
{
        wch_reset();
	if (!eth_enabled) return true;

	if (client.connect(ip, port)) {
		SerialPrint_P(PSTR("-----------------"));
		SerialPrint_P(PSTR("Calling url auth: "), 0);
		if (SERIAL_OUTPUT)
		{
			Serial.print(IPAddress(ip));
			Serial.print(":");
			Serial.print(port);
			Serial.print(url);
			Serial.println(params);
			SerialPrint_P(PSTR(""), 1);
		}

		client.print(F("GET "));
		client.print(url);
		client.print(params);
		client.println(F("HTTP/1.1"));
		client.print(F("Host: "));
		client.println(IPAddress(ip));
		client.print(F("Authorization: Basic "));
		client.println(auth);
		client.println(F("Connection: close"));
		client.println();

		SerialPrint_P(PSTR("SERVER ANSWER:"));
		while (client.connected() && !client.available()) delay(1); //waits for data
		while (client.connected() || client.available()) { //connected or data available
			char c = client.read();
			if (SERIAL_OUTPUT) Serial.print(c);
		}
		SerialPrint_P(PSTR("-----------------"));
		client.stop();
                wch_reset();
		return true;
	}
	else {
		// kf you didn't get a connection to the server:
		SerialPrint_P(PSTR("connection failed"));
                wch_reset();
		return false;
	}
}


bool BrowseUrl(EthernetClient client, char host[], int port, char* url, char* params)
{
        wch_reset();
	if (!eth_enabled) return true;

	if (client.connect(host, port)) {
		SerialPrint_P(PSTR("-----------------"));
		SerialPrint_P(PSTR("Calling url: "), 0);
		if (SERIAL_OUTPUT)
		{
			Serial.print(host);
			Serial.print(url);
			Serial.println(params);
			SerialPrint_P(PSTR(""), 1);
		}

		client.print(F("GET "));
		client.print(url);
		client.print(params);
		client.println(F(" HTTP/1.1"));
		client.print(F("Host: "));
		client.println(host);
		client.println(F("Connection: close"));
		client.println();

		SerialPrint_P(PSTR("SERVER ANSWER:"));
		while (client.connected() && !client.available()) delay(1); //waits for data
		while (client.connected() || client.available()) { //connected or data available
			char c = client.read();
			if (SERIAL_OUTPUT) Serial.print(c);
		}
		SerialPrint_P(PSTR("-----------------"));

		client.stop();
                wch_reset();
		return true;
	}
	else {
		// kf you didn't get a connection to the server:
		SerialPrint_P(PSTR("connection failed"));
                wch_reset();
		return false;
	} 
}

bool NtpCheck()
{
        wch_reset();
	if (!eth_enabled) return true;

	const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
	byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
	int TIME_ZONE = +1; //+1 for italy
	bool result = false;

	IPAddress timeServer(193, 204, 114, 232); // 

	EthernetUDP Udp;
	Udp.begin(8888);

	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	Udp.beginPacket(timeServer, 123); //NTP requests are to port 123
	Udp.write(packetBuffer, NTP_PACKET_SIZE);
	Udp.endPacket();

	delay(1000);
	if (Udp.parsePacket()) {
		result = true;
		Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
		unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
		unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
		// combine the four bytes (two words) into a long integer
		// this is NTP time (seconds since Jan 1 1900):
		unsigned long timeStamp = highWord << 16 | lowWord;
		SerialPrint_P(PSTR("Ntp answer ok!"), true);
		//Serial.println(timeStamp);

		setTime(timeStamp);
		adjustTime(-2208988800); //adjusting between unix and ntp time base
		time_t italian_time, utc;
		TimeChangeRule usEDT = { "EDT", Last, Sun, Mar, 2, +120 };
		TimeChangeRule usEST = { "EST", Last, Sun, Oct, 2, +60 };
		Timezone Italy(usEDT, usEST);
		utc = now();    //current time from the Time Library
		italian_time = Italy.toLocal(utc);

		setTime(italian_time);
	}
	if (!result) SerialPrint_P(PSTR("Fail to sync to NTP"), true);

	Udp.stop();
        wch_reset();
	return result;
}

void log(char* user)
{	
	sprintf(buffer_relay, "?user=%s&action=%s&peri=%s&volu=%s&id=%s&void=0", user, (enable_alarm ? "1" : "0"), (enable_perimetral ? "1" : "0"), (enable_volumetric ? "1" : "0"), last_UID);
	BrowseUrl(website_client, website_ip, website_port, log_url, buffer_relay);
}

void log(PGM_P user)
{	
  char *buffer = (char *)malloc(strlen_P(user) + 1);
  strcpy_P(buffer, user);
  log(buffer);
  free(buffer);
}

byte sendEmail(PGM_P body)
{
  if (!ENABLE_SEND_MAIL) return 1;

  char *buffer = (char *)malloc(strlen_P(body) + 1);
  strcpy_P(buffer, body);
  sendEmail(buffer);
  free(buffer);
}

byte sendEmail(char* body)
{
        wch_reset();
	if (!ENABLE_SEND_MAIL) return 1;
	if (!eth_enabled) return 1;
	
	if (SERIAL_OUTPUT)
	{
		SerialPrint_P(PSTR("Sending Mail: "), 0);
		SerialPrintln(body);
	}
	byte thisByte = 0;
	byte respCode;
	if (mail_client.connect(mailserver, mail_port)) {}
	else return 0;

	if (!eRcv()) return 0;

	// change to your public ip
	mail_client.println(F("helo mailserver"));
	mail_client.println(F("AUTH LOGIN"));
	mail_client.println(mail_user);
	mail_client.println(mail_password);

	if (!eRcv()) return 0;

	// change to your email address (sender)
	mail_client.print(F("MAIL From: <")); mail_client.print(mail_from); mail_client.println(F(">"));

	if (!eRcv()) return 0;

	// change to recipient address
	mail_client.print(F("RCPT To: <")); mail_client.print(mail_to); mail_client.println(F(">"));

	if (!eRcv()) return 0;

	mail_client.println(F("DATA"));

	if (!eRcv()) return 0;

	// change to recipient address
	mail_client.print(F("To: Me <")); mail_client.print(mail_to); mail_client.println(F(">"));
	mail_client.print(F("From: AntiTheft Alarm <")); mail_client.print(mail_from); mail_client.println(F(">"));
	mail_client.println(F("Subject: AntiTheft Alarm MAIL\r\n"));
	mail_client.println(body);
	mail_client.println(F("."));

	if (!eRcv()) return 0;

	mail_client.println(F("QUIT"));

	if (!eRcv()) return 0;

	mail_client.stop();

	return 1;
        wch_reset();
}

byte eRcv()
{
	byte respCode;
	byte thisByte;
	int loopCount = 0;

	while (!mail_client.available()) {
		delay(1);
		loopCount++;

		// if nothing received for 10 seconds, timeout
		if (loopCount > 10000) {
			mail_client.stop();
			Serial.println(F("\r\nTimeout"));
			return 0;
		}
	}

	respCode = mail_client.peek();

	while (mail_client.available())
	{
		thisByte = mail_client.read();
		//Serial.write(thisByte);
	}

	if (respCode >= '4')
	{
		//efail();
		return 0;
	}

	return 1;
}

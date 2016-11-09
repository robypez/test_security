#define buffersize 1000
char request_buf[buffersize];

void process_web(EthernetServer server)
{
	// listen for incoming clients
	EthernetClient client = server.available();
	if (client) {
		SerialPrint_P(PSTR("Webpage Requested"));
		char c;
		int counter = 0;

		while (client.connected())
		{
			SerialPrint_P(PSTR("Request buffer: "), 0); 
			
			while (client.available() > 0) {
				c = client.read();
				//if (SERIAL_OUTPUT) Serial.print(c);
				request_buf[counter] = c;
				counter++;

				if (counter > (buffersize-2) ) 
				{
					client.stop();
					SerialPrint_P(PSTR("OVERBUFFER"));
					pushNotification(PSTR("OVERBUFFER"));
					return;
				}
			}
			SerialPrint_P(PSTR("BUFFERSIZE: "),0);
			SerialPrint(counter);
			
			SerialPrint_P(PSTR(""));
			SerialPrint_P(PSTR("End of buffer"));
			
			//Check authentication
			if (ENABLE_WEB_AUTH && strstr(request_buf, "Authorization: Basic ") != 0 && strstr(request_buf, authorization_token) == 0) { sendMessage(PSTR("Someone tried to access with wrong credentials")); }
			if (ENABLE_WEB_AUTH && (strstr(request_buf, "Authorization: Basic ") == 0 || strstr(request_buf, authorization_token) == 0)) { webpage_authorization(client); }
			else
			{
				bool found = false;
				if (strstr(request_buf, "GET /index.html?status=ON") != 0) { log("Webuser"); alarm_start(true, false); found = true; }
				if (!found && strstr(request_buf, "GET /index.html?status=OFF") != 0) { log("Webuser"); alarm_stop(true, false); found = true; }
				if (!found && strstr(request_buf, "GET /index.html?peri=ON") != 0) { enable_perimetral = true; found = true; }
				if (!found && strstr(request_buf, "GET /index.html?peri=OFF") != 0) { enable_perimetral = false; found = true; }
				if (!found && strstr(request_buf, "GET /index.html?pir=ON") != 0) { enable_volumetric = true; found = true; }
				if (!found && strstr(request_buf, "GET /index.html?pir=OFF") != 0) { enable_volumetric = false; found = true; }
				if (!found && strstr(request_buf, "GET /index.html?force=ON") != 0) { force_alarm = true; found = true; }
				if (!found && strstr(request_buf, "GET /index.html?force=OFF") != 0) { force_alarm = false; found = true; }

				if (strstr(request_buf, " /options.html") != 0) webpage_setup(client, request_buf);
				else if (strstr(request_buf, " /prevent.js") != 0) webpage_prevent_js(client);
				else if (strstr(request_buf, " /update.json") != 0) updateJsonCmd(client);
				else if (found || strstr(request_buf, "GET / ") || strstr(request_buf, "GET /index.html ") != 0) webpage_index(client);
			}

			SerialPrint_P(PSTR("Webpage sent"));
			client.stop();
			SerialPrint_P(PSTR("Webserver connection closed"));
			SerialPrint_P(PSTR("RAM: "), 0);
			SerialPrint(freeRam());
      SerialPrint_P(PSTR("\n"), 0);
		}
	}
}


void webpage_prevent_js(EthernetClient c)
{
	c >> F("if((\"standalone\" in window.navigator) && window.navigator.standalone){\n");
	c >> F("var noddy, remotes = false;\n");
	c >> F("document.addEventListener('click', function(event) {\n");
	c >> F("noddy = event.target;\n");
	c >> F("while(noddy.nodeName !== \"A\" && noddy.nodeName !== \"HTML\") {\n");
	c >> F("noddy = noddy.parentNode;\n");
	c >> F("}\n");
	c >> F("if('href' in noddy && noddy.href.indexOf('http') !== -1 && (noddy.href.indexOf(document.location.host) !== -1 || remotes))\n");
	c >> F("{\n");
	c >> F("event.preventDefault();\n");
	c >> F("document.location.href = noddy.href;\n");
	c >> F("}\n");
	c >> F("},false);\n");
	c >> F("}\n");
}

void webpage_authorization(EthernetClient c)
{
	c >> F("HTTP/1.0 401 Authorization Required\r\n");
	c >> F("Content-Type: text/html; charset=UTF-8\r\n");
	c >> F("WWW-Authenticate: Basic realm=\"") >> F("Webduino") >> F("\"\r\n");
	c >> F("\r\n");
	c >> F("<!DOCTYPE HTML>\r\n");
	c >> F("<h1>401 Unauthorized. Your attempt has been reported to the owner!</h1>\r\n");
}

void webpage_header(EthernetClient c, bool autorefresh)
{
	c >> F("HTTP/1.1 200 OK\r\n");
	c >> F("Server: Garo AntiTheft Alarm\r\n");
	c >> F("Content-Type: text/html; charset=UTF-8\r\n");
	c >> F("Connection: close\r\n");
	c >> F("\r\n");

	c >> F("<!DOCTYPE HTML>\r\n");
	c >> F("<html><head>\r\n");
	c >> F("<meta name=\"viewport\" content=\"width=device-width; initial-scale=1.0; maximum-scale=1.0; user-scalable=0;\"/>\r\n");
	c >> F("<meta name=\"apple-mobile-web-app-capable\" content=\"yes\" />\r\n");
	c >> F("<meta name=\"apple-mobile-web-app-status-bar-style\" content=\"black-translucent\" />\r\n");
	c >> F("<link rel=\"apple-touch-icon\" href=\"http://www.riccardogarofano.com/files/antifurto.png\" />\r\n");
	//favicon taken from: http://www.favicon.cc/?action=icon&file_id=784192
	c >> F("<link href=\"data:image / x - icon; base64, AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAAAQAAAAAAAAAAA");
	c >> F("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAICAgBIAAAAeISABOiEgAToAAAAfgICACQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	c >> F("gICAFjYzAWB7dQLajokB/5OOAf+TjgH/j4oB/3x3Atw2NAFhgICAAwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACXl0AuGZlAD/npgA/6CaAP+hmwD/op");
	c >> F("sA/6GbAP+fmQD/m5UA/3l0AuIAAAAKAAAAAAAAAAAAAAAAlpWAHnt2AuyclgD/nJYA/56YAP+imwD/pJ4A/6SeAP+inAD/oJkA/52XAP+dlwD/e3YC");
	c >> F("7paVgB4AAAAAAAAAAGlkAqCalAD/mZMA/5qUAP+dlwD/oZoA/6OdAP+knQD/oZoA/56XAP+clQD/m5UA/5yWAP9lYQKZAAAAAK2rgC6JgwH/mJIA/5");
	c >> F("iSAP++ulj/ycVw/6ihEP+jnAD/o5wA/6iiEP/JxnD/v7pY/5qTAP+alAD/jIYB/62rgC44NQFWlpAA/5iSAP//////8/Lf/+TisP//////xsNh/8bD");
	c >> F("Yf//////5OKw//Pz3///////mpQA/5qUAP84NQFWcWwCmZqUAP/W1Jj/7evP/6WeCP+yrSD/ragR////////////r6kR/8XBWf+oohD/7ezP/9jUmP");
	c >> F("+dlwD/cWwCmYeDG6SblQD/29ig/8XBaP+4tED/3tug/6ulCf/49+f/+Pfn/6ymCf//////wr1Y/8fCaP/c2aD/n5gA/4mEG6SDfgqBnJYA/7SvOP//");
	c >> F("////saso/7GsIP/v78/////////////w78//xMBQ/7OtKP//////trE4/5+ZAP+EgAuBy8mabZmUDv+gmgD/y8hw//389///////7ezH/7GrAf+yqw");
	c >> F("H/7uzH///////9/Pf/zclw/6KcAP+clw7/ysmabAAAAACdmTT/npgA/6KcAP+lnwD/qaIA/6ylAP+tpwD/rqcA/62mAP+rpAD/p6EA/6SdAP+gmgD/");
	c >> F("oZw0/wAAAAAAAAAAf3oDW6CbF/+fmQD/opwA/6WfAP+noQD/qKIA/6miAP+oogD/p6AA/6SdAP+gmgD/pqAX/396A10AAAAAAAAAAAAAAACCfQnFqa");
	c >> F("Qo/5+ZAP+hmwD/o50A/6SdAP+kngD/o50A/6KcAP+gmgD/rKcm/4J9CcUAAAAAAAAAAAAAAAAAAAAAAAAAAH55Apauqkn/qaQg/6KcCP+fmQD/n5kA");
	c >> F("/6KcCP+qpSD/saxI/355ApkAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAv7yBC4B7BqmUkCPqoZwz/6GdM/+VkSPqgHwGq7+8gU0AAAAAAAAAAA");
	c >> F("AAAAAAAAAA//8AAPgfAADgBwAAwAMAAIABAACAAQAAgAEAAAAAAAAAAAAAAAAAAIABAACAAQAAwAMAAMADAADgBwAA+B8AAA==\" rel=\"icon\"");
	c >> F("type=\"image/x-icon\" />\r\n");
	if (autorefresh && !ENABLE_AJAX) c >> F("<meta http-equiv=\"refresh\" content=\"") >> webpage_refresh/1000 >> F("; url = index.html\"/>\r\n");  // refresh the page automatically every 5 sec
	c >> F("<title>Garo Anti-Theft Alarm 1.0</title>\r\n");
	c >> F("<style type=\"text/css\">\r\n");
	c >> F("BODY { font-family: tahoma }\r\n");
	c >> F("H1 { font-size: 14pt; }\r\n");
	c >> F("gr { font-weight: bold; }\r\n");
	c >> F("P  { font-size: 10pt; }\r\n");
	c >> F("input[type=button] { background-color:#3d94f6; -moz-border-radius:6px; -webkit-border-radius:6px; border-radius:6px;	border:1px solid #337fed;	display:inline-block; color:#ffffff; font-weight:bold; width:300px; }\r\n");
	c >> F("input[type=submit] { background-color:#3d94f6; -moz-border-radius:6px; -webkit-border-radius:6px; border-radius:6px;	border:1px solid #337fed;	display:inline-block; color:#ffffff; font-weight:bold; width:300px; }\r\n");
	c >> F("input[type=number] { width:50px; }\r\n");
	c >> F("</style>\r\n");
	c >> F("<script src=\"/prevent.js\"></script>\r\n");

	if (ENABLE_AJAX)
	{
		c >> F("<script src=\"http://ajax.googleapis.com/ajax/libs/jquery/1.7.2/jquery.min.js\"></script>\r\n");

		c >> F("<script>\r\n");
		c >> F("function update() {\r\n");
		c >> F("$.ajaxSetup({ cache: false });\r\n");
		c >> F("$.support.cors = true;\r\n");
		c >> F("$.getJSON('update.json', function(json) {\r\n");
    c >> F("$(\"#time\").html(json.y + \"-\" + json.mo + \"-\" + json.d + \" \" + json.h + \":\" + json.m + \":\" + json.s);\r\n");
    c >> F("$(\"#uptime\").html(json.ud + \" days, \" + json.uh + \" hours, \" + json.um + \" mins, \" + json.us + \" secs\");\r\n");
		c >> F("$(\"#alarm_enabled\").html(json.alarm_enabled);\r\n");
		c >> F("$(\"#enable_perimetral\").html(json.enable_perimetral);\r\n");
		c >> F("$(\"#enable_volumetric\").html(json.enable_volumetric);\r\n");
		c >> F("$(\"#alarm\").html(json.alarm);\r\n");
		for (int i = 0; i < sensor_number; i++) { c >> F("$(\"#s") >> i >> F("\").html(json.s") >> i >> F(");\r\n"); }
		c >> F("setTimeout(function(){update();}, ") >> webpage_refresh >> F("); })\r\n");
		c >> F("setTimeout(") >> webpage_refresh+5000 >> F("); }\r\n");//after 5 seconds without response, timeout and call again
		c >> F("$(window).load(function(){ update();});\r\n");
		c >> F("</script>");
	}

	c >> F("</head>\r\n");
	//c >> F("<!DOCTYPE HTML>\r\n");
	c >> F("<body>\r\n");
}

void updateJsonCmd(EthernetClient c)
{
	c >> F("HTTP/1.0 200 OK\r\n");
	c >> F("Content-Type: application/json\r\nPragma: no-cache\r\n\r\n{");

	c >> F("\"y\": \"") >> year() >> F("\",\r\n");
	c >> F("\"mo\": \"") >> (month()<10 ? F("0") : F("")) >> month() >> F("\",\r\n");
	c >> F("\"d\": \"") >> (day()<10 ? F("0") : F("")) >> day() >> F("\",\r\n");
	c >> F("\"h\": \"") >> (hour()<10 ? F("0") : F("")) >> hour() >> F("\",\r\n");
	c >> F("\"m\": \"") >> (minute()<10 ? F("0") : F("")) >> minute() >> F("\",\r\n");
	c >> F("\"s\": \"") >> (second()<10 ? F("0") : F("")) >> second() >> F("\",\r\n");

  c >> F("\"ud\": \"") >> uptime_d() >> F("\",\r\n");
  c >> F("\"uh\": \"") >> uptime_h() >> F("\",\r\n");
  c >> F("\"um\": \"") >> uptime_m() >> F("\",\r\n");
  c >> F("\"us\": \"") >> uptime_s() >> F("\",\r\n");
  
	c >> F("\"alarm_enabled\": \"") >> (enable_alarm ? F("enabled") : F("disabled")) >> F("\",\r\n");
	c >> F("\"enable_perimetral\": \"") >> (enable_perimetral ? F("enabled") : F("disabled")) >> F("\",\r\n");
	c >> F("\"enable_volumetric\": \"") >> (enable_volumetric ? F("enabled") : F("disabled")) >> F("\",\r\n");
	c >> F("\"alarm\": \"") >> (alarm ? F("ALARMED!") : F("normal")) >> F("\",\r\n");
	
	for (int i = 0; i < sensor_number; i++)
	{
		c >> F("\"s") >> i >> F("\": \"") >> ((!sensors[i].enabled && alarm_armed) ? "disabled" : getSensorStateStr(sensors[i]));
		if (i == sensor_number - 1) c >> F("\"\r\n");
		else c >> F("\", \r\n");
	}

	c >> F("}");
}

void webpage_index(EthernetClient c)
{
	webpage_header(c, true);

	c >> F("<h1>Garo Antitheft Alarm 1.0</h1>");
	c >> F("<gr>Current Time</gr>: ");
	c >> F("<div id=\"time\" style=\"display:inline;\">time</div><br>");
  c >> F("<gr>Uptime</gr>: ");
  c >> F("<div id=\"uptime\" style=\"display:inline;\">uptime</div>");
  c >> F("<br><a href=\"options.html\" class=\"gr\">Options page</a>");
	c >> F("<hr>");
	c >> F("<gr>Alarm</gr>: <div id=\"alarm_enabled\" style=\"display:inline;\" >") >> (enable_alarm ? F("enabled") : F("disabled")) >> F("</div><br>");
	c >> F("<gr>Perimetral</gr>: <div id=\"enable_perimetral\" style=\"display:inline;\" >") >> (enable_perimetral ? F("enabled") : F("disabled")) >> F("</div><br>");
	c >> F("<gr>Volumetric</gr>: <div id=\"enable_volumetric\" style=\"display:inline;\" >") >> (enable_volumetric ? F("enabled") : F("disabled")) >> F("</div><br>");
	c >> F("<gr>Alarm Status</gr>: <div id=\"alarm\" style=\"display:inline;\" >") >> (alarm ? F("enabled") : F("disabled")) >> F("</div><br>");
	c >> F("<hr>");

	for (int i = 0; i < sensor_number; i++)
	{
		c >> F("<gr>") >> sensors[i].name >> F("</gr>: <div id=\"s") >> i >> F("\" style=\"display:inline;\" >") >> ((!sensors[i].enabled && alarm_armed) ? "disabled" : getSensorStateStr(sensors[i])) >> F("</div><br>");
	}

	c >> F("<br>");

	if (!enable_alarm) c >> F("<a href=\"/index.html?status=ON\"><input type=\"button\" value=\"Enable Alarm\"></a><br>");
	else c >> F("<a href=\"/index.html?status=OFF\"><input type=\"button\" value=\"Disable Alarm\"></a><br>");


	if (!enable_perimetral) c >> F("<a href=\"/index.html?peri=ON\"><input type=\"button\" value=\"Enable Perimetral\"></a><br>");
	else c >> F("<a href=\"/index.html?peri=OFF\"><input type=\"button\" value=\"Disable Perimetral\"></a><br>");


	if (!enable_volumetric) c >> F("<a href=\"/index.html?pir=ON\"><input type=\"button\"  value=\"Enable Volumetric\"></a><br><br>");
	else c >> F("<a href=\"/index.html?pir=OFF\"><input type=\"button\" value=\"Disable Volumetric\"></a><br><br>");

	c >> F("<br><br>");

	if (!force_alarm) c >> F("<a href=\"/index.html?force=ON\"><input type=\"button\" value=\"Force alarm\"></a>");
	else c >> F("<a href=\"/index.html?force=OFF\"><input type=\"button\" value=\"Stop alarm\"></a>");

	c >> F("</body></html>");
}

void webpage_setup(EthernetClient c, char* str)
{	
	char *param, *value, *ptr;
	param = strtok_r(str, "&", &ptr);
	Serial.println(ptr);
	bool param_found = false;
	
	if (strstr(ptr, "&") != 0)
	{
		param_found = true;
		ENABLE_SEND_MAIL = 0;
		ENABLE_BACKLIGHT_CONTROL = 0;
		ENABLE_DOMOTIC_CONTROL = 0;
		ENABLE_IPCAM_CONTROL = 0;
		ENABLE_PUSH = 0;
		enable_intelligent_mode = 0;
		enable_sensor_reactivation = 0;

		while (true)
		{
			param = strtok_r((char*)NULL, "=", &ptr);
			if (param == NULL) break;
			value = strtok_r((char*)NULL, "&", &ptr);
			if (value == NULL) break;

			Serial.print(param);
			Serial.print(": ");
			Serial.println(value);

			if (strstr(param, "timeout") != 0) alarm_timeout = ((unsigned long)atoi(value)) * 1000 * 60;
			if (strstr(param, "grace") != 0) grace_period = ((unsigned long)atoi(value)) * 1000;
			if (strstr(param, "bk_period") != 0) lcd_bk_period = ((unsigned long)atoi(value)) * 1000;
			if (strstr(param, "siren_start") != 0) siren_start_timeout = ((unsigned long)atoi(value)) * 1000;
			if (strstr(param, "standby") != 0) alarm_standby_timeout = ((unsigned long)atoi(value)) * 1000 * 60;
			if (strstr(param, "backlight") != 0) ENABLE_BACKLIGHT_CONTROL = 1;
			if (strstr(param, "mail") != 0) ENABLE_SEND_MAIL = 1;
			if (strstr(param, "push") != 0) ENABLE_PUSH = 1;
			if (strstr(param, "domotic") != 0) ENABLE_DOMOTIC_CONTROL = 1;
			if (strstr(param, "ipcam") != 0) ENABLE_IPCAM_CONTROL = 1;
			if (strstr(param, "vol_from") != 0) vol_from = atoi(value);
			if (strstr(param, "vol_to") != 0) vol_to = atoi(value);
			if (strstr(param, "intelli") != 0) { enable_intelligent_mode = 1; checkIntelligent(); }
			if (strstr(param, "reactiv") != 0) enable_sensor_reactivation = 1;
		};

		int i = 0;

		EEPROM.write(i, (int)(alarm_timeout / (unsigned long)60000)); i++;
		if (grace_period < 3000) grace_period = 3000;
		EEPROM.write(i, (int)(grace_period / 1000)); i++;
		EEPROM.write(i, (int)(lcd_bk_period / 1000)); i++;
		EEPROM.write(i, (int)(siren_start_timeout / 1000)); i++;
		EEPROM.write(i, (int)(alarm_standby_timeout / (unsigned long)60000)); i++;
		EEPROM.write(i, ENABLE_BACKLIGHT_CONTROL); i++;
		EEPROM.write(i, ENABLE_SEND_MAIL); i++;
		EEPROM.write(i, ENABLE_DOMOTIC_CONTROL); i++;
		EEPROM.write(i, vol_from); i++;
		EEPROM.write(i, vol_to); i++;
		EEPROM.write(i, enable_intelligent_mode); i++;
		EEPROM.write(i, enable_sensor_reactivation); i++;
		EEPROM.write(i, ENABLE_IPCAM_CONTROL); i++;
		EEPROM.write(i, ENABLE_PUSH); i++;
	}

	webpage_header(c, false);

	int i;

	c >> F("<form action=\"options.html\" method=\"post\">");

	c >> F("<h1>Options</h1><p>");

	c >> F(("<input type=\"hidden\" name=\"f\" value=\"0\">"));
	c >> F("Grace period (before alarm is armed)[sec]: <input type=\"number\" name=\"grace\" value=\"") >> int(grace_period / 1000) >> F("\"><br>");
	c >> F("Alarm start delay (before siren start after door opening)[sec]: <input type=\"number\" name=\"siren_start\" value=\"") >> siren_start_timeout / 1000 >> F("\"><br>");
	c >> F("Alarm duration timeout (siren duration)[min]: <input type=\"number\" name=\"timeout\" value=\"") >> alarm_timeout / 1000 / 60 >> F("\"><br>");
	c >> F("Backlight period [sec]: <input type=\"number\" name=\"bk_period\" value=\"") >> lcd_bk_period / 1000 >> F("\"><br>");
	c >> F("Alarm standby timeout [min]: <input type=\"number\" name=\"standby\" value=\"") >> int(alarm_standby_timeout / 1000 / 60) >> F("\"><br>");
	c >> F("Volumetric disabled between: [h]<input type=""number"" name=""vol_from"" value="""); 
	c >> vol_from >> F("""> and <input type=""number"" name=""vol_to"" value=""");
	c >> vol_to >> F("""><br>");
	c >> F("<br>");
	c >> F("<input type=\"checkbox\" name=\"mail\" value=\"1\" ") >> (ENABLE_SEND_MAIL ? F("checked") : F("")) >> F(">Send mail<br>");
	c >> F("<input type=\"checkbox\" name=\"push\" value=\"1\" ") >> (ENABLE_PUSH ? F("checked") : F("")) >> F(">Push notification<br>");
	c >> F("<input type=\"checkbox\" name=\"backlight\" value=\"1\" ") >> (ENABLE_BACKLIGHT_CONTROL ? F("checked") : F("")) >> F(">LCD backlight automatic control<br>");
	c >> F("<input type=\"checkbox\" name=\"domotic\" value=\"1\" ") >> (ENABLE_DOMOTIC_CONTROL ? F("checked") : F("")) >> F(">Domotic control<br>");
	c >> F("<input type=\"checkbox\" name=\"ipcam\" value=\"1\" ") >> (ENABLE_IPCAM_CONTROL ? F("checked") : F("")) >> F(">IpCam control<br>");

	c >> F("<input type=""checkbox"" name=""intelligent"" value=""1"" ") >> (enable_intelligent_mode ? F("checked") : F("")) >> F(">Intelligent Mode<br>");
	c >> F("<input type=""checkbox"" name=""reactivation"" value=""1"" ") >> (enable_sensor_reactivation ? F("checked") : F("")) >> F(">Sensor reactivation<br>");

	


	c >> F("<br/>");
	
	if (param_found) c >> F("<input type=\"submit\" value=\"Ok, saved - save again?\"/></form>");
	else c >> F("<input type=\"submit\" value=\"Save and apply\"/></form>");
	c >> F("<br><br>");
	c >> F("<a href=\".\">Home</a>");
	c >> F("</body></html>");
}

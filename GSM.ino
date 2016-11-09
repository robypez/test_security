void initialize_gsm()
{
		SerialPrint_P(PSTR("Initializing GSM"));
		//Start configuration of shield with baudrate.
		//For http uses is raccomanded to use 4800 or slower.
		
		if (gsm.begin(GSMSPEED)){
			SerialPrint_P(PSTR("\nGSM Ready!"));
			gsmstarted = true;
			gsmpowered = true;
		}
		else
		{
			SerialPrint_P(PSTR("\nGSM not answering. Turning on!"));
			gsmpower();
			delay(3000);
			if (gsm.begin(GSMSPEED))
			{
				SerialPrint_P(PSTR("\nstatus=READY"));
				gsmstarted = true;
				gsmpowered = true;
			}
		}
}


void checkSMS()
{
	int pos;
	pos = sms.IsSMSPresent(SMS_UNREAD);

	if (pos>0) {
		// read new SMS
		sms.GetSMS(pos, n, smsbuffer, 160);
		sms.DeleteSMS(pos);

		String smsstr = (String)smsbuffer;
		SerialPrintln(n);
		SerialPrint_P(PSTR("Ricevuto SMS!------------------"));
		SerialPrintln(smsbuffer);
		SerialPrint_P(PSTR("-------------------------------"));
		if ((smsstr.substring(0, 2).equals("ON") || smsstr.substring(0, 2).equals("on") || smsstr.substring(0, 2).equals("On")) && checkNum(n))
		{
			SerialPrint_P(PSTR("Accepted, activating"));
			delay(200);
			alarm_start(true);
			delay(500);
			sms.SendSMS(n, "ok activating alarm");
		}
		else if ((smsstr.substring(0, 3).equals("OFF") || smsstr.substring(0, 3).equals("off") || smsstr.substring(0, 3).equals("Off")) && checkNum(n))
		{
			SerialPrint_P(PSTR("Accepted, disactivating"));
			delay(200);
			alarm_stop(true, false);
			delay(500);
			sms.SendSMS(n, "ok disabling alarm");
		}
        else 
        {
            char mailBuf[250];
            sprintf(mailBuf, "SMS from %s: %s", n, smsbuffer); 
			sendEmail(mailBuf);
			pushNotification(PSTR("An unknown SMS has been received"));
		}
	}
 
}

bool checkNum(char* n)
{
      String num = (String)n;
        
      for (int i = 0; i < phones_num; i++) 
      {
        if (num.substring(0, 13).equals(phones[i]))
        {
          SerialPrint_P(PSTR("Phone number authorized"));
          return true;
        }
      }
      
      SerialPrint_P(PSTR("Phone number not authorized"));
      return false;
}

void gsmpower()
// software equivalent of pressing the GSM shield "power" button
{
	pinMode(9, OUTPUT);
	digitalWrite(9, LOW);
	delay(1000);
	digitalWrite(9, HIGH);
	delay(2000);
	digitalWrite(9, LOW);
	gsmpowered = !gsmpowered;
}

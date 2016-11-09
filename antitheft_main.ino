//-----FUNCTION VARIABLES-----
// Enable or disable system modules
#define SERIAL_OUTPUT 1 //Enable serial debug output
#define DHT_ENABLED 1 //Enable temperature and humidity measurement and sending info to an EMONCMS server
#define CS_ENABLED 1 //Enable touch buttons controller
#define LCD_ENABLED 1 //Enable LCD outpu
#define NFC_ENABLED 1 //Enable NFC detection
#define GSM_ENABLED 1 //Enable GSM functions
#define SND_ENABLED 1 //Enable sound
#define WCH_ENABLED 0 //Enable watchdog
#define REBOOT_CHK 1 //Security mode. If some component fails, after 5 reboot the system enters a safe mode to avoid the siren ring at each reboot (they may be endless!)
#define REBOOT_RST 0 //Reset security mode. If the system  is stuck, recompile with this option to 1 to reset it. When fixed, recompile with 0



#define ETH_W5100 1 //set 0 for ENC28J60

//UNCOMMENT FOR W5100 ETHERNET
#include <SPI.h>
#include <Ethernet.h> //TO INCLUDE FOR W5100
#include <utility/w5100.h>

//UNCOMMENT FOR ENC28J60 ETHERNET
//#define ETH_W5100 1 //set 0 for ENC28J60
//#include <UIPEthernet.h> //TO INCLUDE FOR ENC28J60

#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <Time.h>
#include <Timezone.h>
#include <WSWire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include "pitches.h"
#include "types.h"
#include <EEPROM.h>
#include "mpr121.h"
#include <dht.h>
#include <SoftwareSerial.h>
#include "SIM900.h"
#include "sms.h"
#include "call.h"
#include <avr/wdt.h>


template<class T>
inline Print &operator >>(Print &obj, T arg)
{
	obj.print(arg);
	return obj;
}



//-----ALARM VARIABLES-----
#define phones_num 2
char* phones[] = { "+39123123123", "+39234234234" };

//Declare RFIDs
#define token_number 7
static token tokens[] =
{
  {{ 111, 111, 111, 111 }, "User 1"},  // RFID #1
  {{ 111, 111, 111, 111 }, "User 2"},  // RFID #2
  {{ 111, 111, 111, 111 }, "User 3"},  // RFID #3
  {{ 111, 111, 111, 111 }, "User 4"},  // RFID #4
  {{ 111, 111, 111, 111 }, "User 5"},  // RFID #5
  {{ 111, 111, 111, 111 }, "User 6"},  // RFID #6
  {{ 111, 111, 111, 111 }, "User 7"},  // RFID #7
};


//Declare sensors array
#define sensor_number 8 // number of sensors
sensor sensors[] = 
{
  //fields: pin, state, volumetric, perimetral, alarmed_timestamp, name, enabled
  //set state, alarmed_timestamp to 0 and enabled to false
  //first sensor is the DOOR sensor.
	{ 49, HIGH, 0, 1, 0, "Door", true },
	{ 6, HIGH, 1, 0, 0, "Volumetric 1", true },
//  {4,0,1,1,0,("Volumetric 2"),false},
	{ 13, HIGH, 0, 1, 0, "Sensor 1", true },
	{ 47, HIGH, 0, 1, 0, "Sensor 2", true },
	{ 43, HIGH, 0, 1, 0, "Sensor 3", true },
	{ 39, HIGH, 0, 1, 0, "sensor 4", true },
	{ 35, HIGH, 0, 1, 0, "Sensor 5", true },
	{ 31, HIGH, 0, 1, 0, "Sensor 6", true },
};


#define alarm_limit 5 //stop after 5 alarms

//------GSM VARIABLES---------
int numdata;
#define GSMSPEED 9600
#define gsm_refresh_rate 5000
#define call_hangup_time 20000
boolean gsmstarted = false, gsmpowered=false, call_started=false;
char smsbuffer[160];
char n[20];
unsigned long gsm_ts, gsm_call_ts;
SMSGSM sms;
CallGSM call;
int calling_number = 0;


//------LCD VARIABLES---------
#define LCD_I2C_ADDR    0x27  // Define I2C Address where the PCF8574A is
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);
int lcd_message_timeout = 5000;
PGM_P lcd_message;
char lcd_message_str[30];
char lcd_welcome_message[20]; 
int lcd_status;
char tmp_char;

//------SENSORS VARIABLES---------
#define door_sensor 0   // set the sensor number of the house door
#define alarmPin  8     // the number of the alarm pin
#define soundPin  2     // the number of the alarm pin
#define DHT22_PIN 3		// humidity and temperature sensor pin

//------DHT VARIABLES---------
#define dht_refresh_rate 30000
dht DHT;
unsigned long dht_ts;

//------CAPACITIVE SENSORS VARIABLES-------
#define irqPin 17
#define touch_timeout 1000
boolean touchStates[12], touchStates_prev[12]; //to keep track of the previous touch states
unsigned long touch_ts = 0;
char ledStatus = 0;
byte LSB, MSB;
uint16_t touched;
bool led1, led2, led1_prev, led2_prev;
unsigned long led2_ts;

//------NFC VARIABLES-------
PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);
boolean nfc_read;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
char last_UID[30];

//------NETWORK VARIABLES-------
char website[] = "www.mysite.com"; //URL for your website (used for access log function)
char emon_website[] = "www.myemonserver.com"; //URL for your EMONCMS server (used for temperature/humidity log function)
char push_website[] = "api.pushingbox.com"; //URL for push service

byte website_ip[] = { 100, 100, 100, 100 }; //IP for your website (used for access log function)
byte emon_ip[] = { 100, 100, 100, 100 }; //IP for your EMONCMS server (used for temperature/humidity log function)
byte domotic_server_ip[] = { 100, 100, 100, 100 }; //IP for your domotic server (my command uses Bticino Openwebnet protocol, check inside the functions to customize them)
byte ntpServer[] = { 100, 100, 100, 100 }; //IP for NTP server
byte ipcam_ip[] = { 100, 100, 100, 100 }; //IP for IPCams

#define emon_port 80
#define website_port 80
#define ipcam_port 80
#define domotic_server_port 20000

//IPCam urls: these are generic urls, read your ipcam reference manual to get proper urls if these don't work
#define enable_cam "/set_alarm.cgi?motion_armed=1&motion_sensitivity=5&input_armed=1&io%20linkage=0&mail=1&upload_interval=1&schedule_enable=0"
#define disable_cam "/set_alarm.cgi?motion_armed=0&motion_sensitivity=5&input_armed=1&io%20linkage=0&mail=1&upload_interval=0&schedule_enable=0"
//EMONCMS url: modify it with your apikey
#define emon_url "/emoncms/api/post.json?node=1&apikey=INSERT_YOUR_API_KEY_HERE&json="
//PUSH url: I chose to use pushingbox service, customize it with your devId or use another service
#define push_url "/pushingbox?devid=INSERT_YOUR_DEVID_HERE&body="
//LOG url: I created an aspx script on my website to track accesses, customize as you wish!
#define log_url "/log/log.php"

//Authorization tokens: those are just "user:password" string converted in base64, use https://www.base64decode.org/
#define ipcamAuth "dXNlcjpwYXNzd29yZA=="
#define authorization_token "dXNlcjpwYXNzd29yZA=="

//Mail SMTP server variables - used to send mails
#define mailserver "smtp.mysmtp.com"
#define mail_port 25
#define mail_user "dXNlcg==" // user encoded in base64, use https://www.base64decode.org/
#define mail_password "cGFzc3dvcmQ=" // password encoded in base64, use https://www.base64decode.org/
#define mail_from "antitheftalarm@gmail.com"
#define mail_to "myemail@gmail.com"

#define homepage_refresh 0 //if set to 0, no autorefresh. it may lead to webserver problem
#define webpage_refresh 900
static bool ENABLE_AJAX = 1;
static bool ENABLE_WEB_AUTH = 1;
EthernetClient emon_client, website_client, domotic_client, mail_client, ipcam_client, push_client;
static byte mac[] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 };
IPAddress ip(192, 168, 1, 66);
EthernetServer webserver(80);

//------OPTION VARIABLES-------
//These are just default settings, they may be configured in the options webpage
int ENABLE_BACKLIGHT_CONTROL = 1;
int ENABLE_SEND_MAIL = 0;
int ENABLE_DOMOTIC_CONTROL = 1;
int ENABLE_IPCAM_CONTROL = 1;
int ENABLE_PUSH = 1;
int enable_intelligent_mode=1, enable_sensor_reactivation=0;
unsigned long override_intelligent_ts;
unsigned long alarm_timeout = 1000; //set waiting time before turning off the siren once the sensor alarm is off
unsigned long grace_period = 10000; //alarm grace period
unsigned long lcd_bk_period = 8000; //backlight duration
unsigned long siren_start_timeout = 5000; //avoid duplicate alarm start/stop request from webserver
unsigned long alarm_standby_timeout = 300; //time before siren starts again while the alarm signal is alarmed
int vol_from, vol_to; //set pause for volumetric

//------GENERAL VARIABLES-------
#define reboot_count 99
#define prev_stat_address 100

int prev_day, prev_hour, alarm_count=0;
boolean enable_alarm = false, enable_volumetric = true, enable_perimetral = true, alarm_armed = false, alarm = false;
unsigned long nfc_ts = millis(), lcd_ts = millis(), lcd_message_ts = millis(), lcd_bk_ts = millis(), siren_start_ts = millis(), reset_sensors_ts = millis(), alarm_standby_timeout_ts = millis(), alarm_delay_ts = millis();
int prev_sec = 0;
bool eth_enabled = 1;

static int nfc_period = 1000; //nfc read frequency

unsigned long grace_period_ts = millis();
bool menu_enabled = false;
int menu_option = 0;

long int alarm_timeout_ts;
bool alarm_siren_started = false;
bool alarm_standby = false;
bool force_alarm = false;
bool check_sensors_before_activation = false;

char tmp[30];
int tmp_int;
unsigned long tmp_ulong;
unsigned long delay_ts;




void setup() {
	wch_disable();
  //Initialize siren
  pinMode(alarmPin, OUTPUT);
  digitalWrite(alarmPin, LOW);//disable the alarm siren!

  Serial.begin(9600);
  
  //SAFE MODE CHECK
  if (REBOOT_RST) EEPROM.write(reboot_count, 0);
  
  if (REBOOT_CHK)
  {
    int reboot=EEPROM.read(reboot_count);
    if (EEPROM.read(reboot_count) > 5) 
    {
      Serial.println("SAFE MODE ENABLED, system rebooted more than 5 times");
      while(true);
    }
    reboot=reboot+1;
    EEPROM.write(reboot_count, reboot);
  }  

	SerialPrint_P(PSTR("Garo Anti-Theft Alarm 1.0 BOOTING"), 1);

	Wire.begin();

	SerialPrint_P(PSTR("Loading Options"), 1);
	loadOptions();

	//Initialize LCD
	SerialPrint_P(PSTR("Initializing LCD"), 1);
	if (LCD_ENABLED)
	{
        initialize_lcd();
		lcd_output_string(PSTR("AntiTheft Alarm"));
	}

	//Initialize sensors
	SerialPrint_P(PSTR("Initializing Sensors"), 1);
	for (int i = 0; i < sensor_number; i++) pinMode(sensors[i].pin, INPUT);

	//Initialize CS
	if (CS_ENABLED)
	{
		SerialPrint_P(PSTR("Initializing Capacitive Sensors"), 1);
		pinMode(irqPin, INPUT);
		digitalWrite(irqPin, HIGH); //enable pullup resistor

		mpr121_setup();
	}
	//Initialize network
  SerialPrint_P(PSTR("Initialize network"), 1);
  SerialPrint_P(PSTR("Getting DHCP"), 1);
  delay(1000);
  //if (ETH_W5100) W5100.select(53); //Uncomment this if the SS pin is different from 10

	if (Ethernet.begin(mac) == 0)
	{
		SerialPrint_P(PSTR("Failed to configure Ethernet using DHCP"));
    lcd.print("Eth failed");
    eth_enabled = false;      
	}
	else
	{
		eth_enabled = true;
		SerialPrint_P(PSTR("Ethernet initialized"),1);		
		webserver.begin();	
		lcd.setCursor(0, 1);
    lcd.print(IPAddress(Ethernet.localIP()));
    if (SERIAL_OUTPUT) Serial.println(IPAddress(Ethernet.localIP()));
	}
  	
	//Initialize NFC
	if (NFC_ENABLED) initialize_nfc();

	//Initialize GSM
	if (GSM_ENABLED) initialize_gsm();

	//Try to sync to NTP. 10 tentatives as workaround
	NtpCheck();

	if (enable_intelligent_mode) checkIntelligent();
		
	sendMessage(PSTR("Antitheft alarm has just started"));
	
	sound(0);
	SerialPrint_P(PSTR("ANTITHEFT ALARM SUCCESSFULLY BOOTED"), 1);

	log(PSTR("Start"));

	//read prev state in case of accidental reboot
	if (EEPROM.read(prev_stat_address) == 1)
	{
		if (EEPROM.read(prev_stat_address + 1) == 1) enable_perimetral = true;
		if (EEPROM.read(prev_stat_address + 2) == 1) enable_volumetric = true;
		alarm_start(true);
	}
	wch_enable();
}

void loop() {

    wch_reset();

    if (!ETH_W5100) Ethernet.maintain(); //Fix for some UIPEthernet crashes

    wch_disable();
    if (GSM_ENABLED && gsmstarted && ((unsigned long)(millis() - gsm_ts) > gsm_refresh_rate))
    {
    	//Read if there are messages on SIM card and print them.
    	SerialPrint_P(PSTR("Checking for new SMS"), 1);
    	checkSMS();
    	gsm_ts = millis();
    }
    wch_enable();
    wch_reset();
    
    if (DHT_ENABLED && (unsigned long)(millis() - dht_ts) > dht_refresh_rate) updateDHT();
    
    //check NTP every day    
    if (day() != prev_day) NtpCheck();
    prev_day = day();
    
    if (hour() != prev_hour && year()==1970) NtpCheck();    
    
    //intelligent mode
    if (enable_intelligent_mode && ((unsigned long)(millis() - override_intelligent_ts) > 3600000) && hour()!=prev_hour && !enable_alarm) checkIntelligent();      
    prev_hour = hour();

	//manage leds and capacitive sensors
	if (CS_ENABLED)
	{
		//manage leds
		if (enable_alarm)
		{
			if ((unsigned long)(millis() - led2_ts) > 1000) led2 = true;
			if ((unsigned long)(millis() - led2_ts) > 1500)
			{
				led2 = false;
				led2_ts = millis();
			}
		}
		else led2 = false;

		if (led1_prev != led1 || led2_prev != led2)
		{
			set_register(GPIO_CLEAR, 0xFF);       // clear all leds
			delay(10);
			ledStatus = 0;
			if (led2) bitWrite(ledStatus, 0, 1);
                        if (led1) { bitWrite(ledStatus, 1, 1); bitWrite(ledStatus, 2, 1);}
			set_register(GPIO_SET, ledStatus);  // set LED
			delay(10);
		}

		led1_prev = led1;
		led2_prev = led2;

		//manage Capacitive Sensors
		readTouchInputs();
	}

	//Manage LCD menu
	if ((unsigned long)(millis() - touch_ts) > touch_timeout && touchStates[0] && !touchStates_prev[0])
	{
		touchStates[0] = 0;
		lcd_backlight_on();
		sound(5);
		touch_ts = millis();
		if (menu_enabled)
		{
			menu_option++;
			if (menu_option > 2) menu_option = 0;
		}
		else {
			menu_option = 0;
			menu_enabled = true;
		}
	}

	if ((unsigned long)(millis() - touch_ts) > touch_timeout && touchStates[1] && !touchStates_prev[1])
	{
		touchStates[1] = 0;
		lcd_backlight_on();
		touch_ts = millis();

		if (menu_enabled)
		{
			menu_enabled = false;
			switch (menu_option)
			{
			case 0://enable volum and peri
				if (!enable_alarm)
				{
					enable_volumetric = true;
					enable_perimetral = true;
					output_lcd(PSTR("All sensors OK     "));
					sound(6);
				}
				else
				{
					output_lcd(PSTR("Not allowed     "));
					sound(3);
				}
                override_intelligent_ts=millis();//override intelligent mode for 1 hour
				break;
			case 1://enable only peri
				if (!enable_alarm)
				{
					enable_volumetric = false;
					enable_perimetral = true;
					output_lcd(PSTR("Peri only OK     "));
					sound(6);
				}
				else
				{
					output_lcd(PSTR("Not allowed      "));
					sound(3);
				}
                override_intelligent_ts=millis();//override intelligent mode for 1 hour
				break;
			default://exit
				sound(6); break;
			}
		}
		else sound(6);
	}


	//search for rfid/nfc
	if (NFC_ENABLED)
	{
		if ((unsigned long)(millis() - nfc_ts) > nfc_period)
		{
			nfc_read = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, 50);
                        wch_reset();
			nfc_period = 1000;

			if (nfc_read) {

				SerialPrint_P(PSTR("Found a card!"));
				SerialPrint_P(PSTR("UID Value: "));
				for (uint8_t i = 0; i < uidLength; i++) SerialPrint(uid[i]); SerialPrint("-");

				char user[80]="Unknown";
				int nfc_index = -1;
				nfc_index = nfc_check(uid);

				if (nfc_index>-1)
				{
					SerialPrint_P(PSTR("NFC Accepted"), 1);

					if (!enable_alarm) alarm_start(false);
					else
					{
						alarm_stop(false);
						sprintf(lcd_welcome_message, "Ciao %s      \0", tokens[nfc_index].uid_name);
						output_lcd_str(lcd_welcome_message);
						refresh_lcd();
                                                sound(2);
					}

					if (nfc_index != -1) sprintf(user, "%s", tokens[nfc_index].uid_name);
					
					nfc_period = 5000; //don't search for 5 seconds

					log(URLEncode(user));

					if (enable_alarm) sprintf(user, "%s has enabled the alarm, peri %s, vol %s", user, (enable_perimetral ? "ON" : "OFF"), (enable_volumetric ? "ON" : "OFF"));
					else sprintf(user, "%s has disabled the alarm", user);
					pushNotification(user);
					

				}
				else
				{
					sendMessage(PSTR("Someone try to enter with unrecognized RFID or NFC"));
					SerialPrint_P(PSTR("NFC not accepted"), 1);
					output_lcd(PSTR("NFC not accepted"));
                                        refresh_lcd();
                                        sound(3);

					log(URLEncode(user));

					//delay(300);
				}
			}

			nfc_ts = millis();
		}
	}


    checkSensors();
        
	alarm = false; //reset alarm
	if (siren_start_ts != 0 && ((unsigned long)(millis() - siren_start_ts)) > siren_start_timeout) alarm = true; //else, alarm after siren timeout

	if (siren_start_ts != 0 && ((unsigned long)(millis() - siren_start_ts)) > siren_start_timeout) alarm = true; //else, alarm after siren timeout

	if (((unsigned long)(millis() - reset_sensors_ts)) > 3600000) //after 1 hour with no alarm, reset sensors and alarm counter
	{
		for (int i = 0; i < sensor_number; i++) sensors[1].alarmed_timestamp = 0;
		alarm_count = 0; 
	}

	//start the alarm at the end of grace period
	if (enable_alarm)
	{
		if (!alarm_armed)
		{
			if (millis() - grace_period_ts>2000)
			{
				tmp_ulong = (unsigned long)(grace_period - (millis() - grace_period_ts));
				if (tmp_ulong > 1000)
				{
					tmp_int = (int)(tmp_ulong / (unsigned long)1000);
					sprintf(tmp, "Start in %d sec", tmp_int);
					output_lcd_str(tmp);
				}
				else
				{
					alarm_armed = true;
					for (int i = 0; i < sensor_number; i++) sensors[i].alarmed = false;
					output_lcd(PSTR("Alarm started     "));
					sound(4);
				}
			}
		}
	}
	else alarm_armed = false;
	
	//After the siren has started, wait for a certain amount of time before starting again
	if ((unsigned long)(millis() - alarm_standby_timeout_ts) > alarm_standby_timeout) alarm_standby = false;
	
	//manage "force alarm"
	if (force_alarm) digitalWrite(alarmPin, HIGH);
	else
	{
		//If the system is alarmed, start the siren, send mail...
		if (alarm_armed && !alarm_standby)
		{
			//check if the siren has to be started
			if (alarm && !alarm_siren_started && alarm_count < alarm_limit)
			{
				digitalWrite(alarmPin, HIGH);
				sound(6);			
                                log("Alarm");	
				SerialPrint_P(PSTR("ALARM!!"));

				char *buffer;
				buffer = (char*)malloc(200);
				bool found = false;
				sprintf(buffer, "%s", "ALARM! Sensors alarmed:\n");
				
				for (int i = 0; i < sensor_number; i++)
				{
					if (sensors[i].alarmed)
					{
						if (!found) sprintf(buffer, "%s %s", buffer, sensors[i].name);
						else sprintf(buffer, "%s, %s", buffer, sensors[i].name);
						found = true;
					}
				}
				
				sendMessage(buffer);
				
				if (GSM_ENABLED)
				{
          wch_disable();
                                        
					for (int i = 0; i < phones_num; i++) sms.SendSMS(phones[i], buffer);

					calling_number = 0;
					call_started = true;
					gsm_call_ts = millis();
					call.Call(phones[0]);
					SerialPrint_P(PSTR("CALLING..."));

	        wch_enable();
				}

				free(buffer);

				alarm_count++;
				alarm_siren_started = true;
				alarm_timeout_ts = millis();
			}

			//Stop siren after alarm timeout
			if (alarm_siren_started)
			{
				if ((unsigned long)(millis() - alarm_timeout_ts) > alarm_timeout)
				{
					digitalWrite(alarmPin, LOW);
					SerialPrint_P(PSTR("ALARM STOP"));
					alarm_siren_started = false;
					alarm_standby_timeout_ts = millis();
					alarm_standby = true;
				}
			}
		}
		else digitalWrite(alarmPin, LOW);
	}

	if (call_started && ((unsigned long)(millis() - gsm_call_ts) > call_hangup_time))
	{
		wch_disable();
    call.HangUp();
		SerialPrint_P(PSTR("HANGING UP CALL"));
		if (!enable_alarm) call_started = false;
		else if (calling_number < phones_num - 1)
		{
			SerialPrint_P(PSTR("CALLING NEXT NUMBER"));
			calling_number++;
			gsm_call_ts = millis();
			call.Call(phones[calling_number]);
		}
		else call_started = false;
    wch_enable();
	}

	if (eth_enabled) process_web(webserver);
	refresh_lcd();
}


void loadOptions()
{
	int i = 0;

	alarm_timeout = ((unsigned long)EEPROM.read(i)) * 1000 * 60; i++;
	grace_period = ((unsigned long)EEPROM.read(i)) * 1000; i++; //
	lcd_bk_period = ((unsigned long)EEPROM.read(i)) * 1000; i++;
	siren_start_timeout = ((unsigned long)EEPROM.read(i)) * 1000; i++;
	alarm_standby_timeout = ((unsigned long)EEPROM.read(i)) * 1000 * 60; i++;
	ENABLE_BACKLIGHT_CONTROL = EEPROM.read(i); i++;
	ENABLE_SEND_MAIL = EEPROM.read(i); i++;
	ENABLE_DOMOTIC_CONTROL = EEPROM.read(i); i++;
	vol_from = ((int)EEPROM.read(i)); i++;
	vol_to = ((int)EEPROM.read(i)); i++;
	enable_intelligent_mode = EEPROM.read(i); i++;
	enable_sensor_reactivation = EEPROM.read(i); i++;
	ENABLE_IPCAM_CONTROL = EEPROM.read(i); i++;
	ENABLE_PUSH = EEPROM.read(i); i++;
}

void checkIntelligent()
{
	if (hour()>5 && hour()<22) { enable_volumetric = true; enable_perimetral = true; }
	else { enable_volumetric = false; enable_perimetral = true; }
}

byte uptime_d()
{
  unsigned long milli = millis();
  unsigned long secs=milli/1000, mins=secs/60;
  unsigned int hours=mins/60, days=hours/24;
  milli-=secs*1000;
  secs-=mins*60;
  mins-=hours*60;
  hours-=days*24;
  return (byte)days;
}

byte uptime_h()
{
	unsigned long milli = millis();
	unsigned long secs = milli / 1000, mins = secs / 60;
	unsigned int hours = mins / 60, days = hours / 24;
	milli -= secs * 1000;
	secs -= mins * 60;
	mins -= hours * 60;
	hours -= days * 24;
	return (byte)hours;
}

byte uptime_m()
{
	unsigned long milli = millis();
	unsigned long secs = milli / 1000, mins = secs / 60;
	unsigned int hours = mins / 60, days = hours / 24;
	milli -= secs * 1000;
	secs -= mins * 60;
	mins -= hours * 60;
	hours -= days * 24;
	return (byte)mins;
}

byte uptime_s()
{
  unsigned long milli = millis();
  unsigned long secs = milli / 1000, mins = secs / 60;
  unsigned int hours = mins / 60, days = hours / 24;
  milli -= secs * 1000;
  secs -= mins * 60;
  mins -= hours * 60;
  hours -= days * 24;
  return (byte)secs;
}



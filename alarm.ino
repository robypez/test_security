void checkSensors()
{
	//check sensors
	check_sensors_before_activation = false;
	siren_start_ts = 0;

	for (int i = 0; i < sensor_number; i++)
	{
		//////If alarmed_timestamp = 0, the sensor has never been opened///////////
		sensors[i].state = digitalRead(sensors[i].pin);

		//if not alarm armed, reset timestamp
		if (!alarm_armed)
		{
			sensors[i].alarmed_timestamp = 0;

			//store perimetral sensors to avoid activation with open windows
			if (sensors[i].state == LOW && enable_perimetral && sensors[i].perimetral == 1 && door_sensor != i)
			{
				check_sensors_before_activation = true;//store peri sensors state for avoid activation with open windows
				sensors[i].enabled = false;
			}
			else sensors[i].enabled = true;
		}
		else //else, check sensor
		{
			//if time is between the volumetric "stop" interval, disable the sensor
			if (hour() >= vol_from && hour() < vol_to && sensors[i].volumetric == 1) sensors[i].enabled = false;
			else if (enable_volumetric && sensors[i].volumetric == 1) sensors[i].enabled = true;

			if (sensors[i].state == LOW && sensors[i].enabled) //Sensor is alarmed, set correct sensor timestamp
			{		
				//if perimetral or volumetric are disabled, disable the corresponding sensors
				if (!enable_perimetral && sensors[i].perimetral == 1) { sensors[i].alarmed_timestamp = 0; continue; }
				if (!enable_volumetric && sensors[i].volumetric == 1) { sensors[i].alarmed_timestamp = 0; continue; }

				reset_sensors_ts = millis(); //keep restarting reset counter till no sensor alarmed

				//if the sensors not already alarmed, set timestamp
				if (sensors[i].alarmed_timestamp == 0)
				{
					if (i == door_sensor || sensors[i].volumetric == 1) {
						sensors[i].alarmed_timestamp = millis(); sensors[i].alarmed = true;
					}
					else {
						sensors[i].alarmed_timestamp = millis() - siren_start_timeout;
						sensors[i].alarmed = true;
					}
				}
			}
			if (enable_sensor_reactivation && sensors[i].state == HIGH && sensors[i].perimetral==1) sensors[i].enabled = true;
		}
		if (siren_start_ts == 0 || (sensors[i].alarmed_timestamp<siren_start_ts && sensors[i].alarmed_timestamp != 0)) siren_start_ts = sensors[i].alarmed_timestamp; //set siren timestamp to the oldest sensor timestamp
	}  
}

//start the alarm
void alarm_start(bool silent)
{
	alarm_start(silent, true);
}

void alarm_start(bool silent, bool domotic)
{
	if (check_sensors_before_activation && enable_perimetral)
	{
		output_lcd(PSTR("Windows open"));		
		sound(3);
		//return;
	}
	enable_alarm = true;
	grace_period_ts = millis();
	alarm_count = 0;
	alarm_siren_started = false;
	
    refresh_lcd();
        
	if (enable_volumetric) enableIpCam();

	if (domotic)
	{
		if (hour() > 6 && hour() < 22) domotic_command("*1*0*0##"); //turning off all lights
		domotic_command("*2*2*0##");//close all the windows
	}
	if (!silent) sound(1);

	//save current state to restore it after accidental reboot
	EEPROM.write(prev_stat_address, 1);
	EEPROM.write(prev_stat_address + 1, enable_perimetral);
	EEPROM.write(prev_stat_address + 2, enable_volumetric);
}

void alarm_stop(bool silent)
{
	alarm_stop(silent, true);
}

void alarm_stop(bool silent, bool domotic)
{
	enable_alarm = false;
	alarm_count = 0;
	alarm_siren_started = false;

        refresh_lcd();
	
	disableIpCam();

	if (domotic)
	{
		if (hour() > 6 && hour() < 22) domotic_command("*2*1*0##"); //open windows
		else domotic_command("*1*1*12##"); //turning on tv's light
	}

	//save current state to restore it after accidental reboot
	EEPROM.write(prev_stat_address, 0);
}

char* getSensorStateStr(sensor s)
{
	if (s.state == LOW) return "<font color=\\\"red\\\">KO</font>";
	return "<font color=\\\"blue\\\">OK</font>";
}

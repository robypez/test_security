void initialize_nfc()
{
  		SerialPrint_P(PSTR("Initializing NFC"), 1);
		nfc.begin();
		uint32_t versiondata = nfc.getFirmwareVersion();
		if (!versiondata) SerialPrint_P(PSTR("Didn't find PN53x board"), 1);
		else
		{
			// Got ok data, print it out!
			SerialPrint_P(PSTR("Found chip PN5"), 1);
			nfc.setPassiveActivationRetries(0xFF);
			nfc.SAMConfig();
		}
}  

int nfc_check(uint8_t uid[])
{
	bool result = false;
	bool tmpResult = true;
	int nfc_index = -1;
	//sprintf(last_UID, "%d%%3a%d%%3a%d%%3a%d", uid[0], uid[1], uid[2], uid[3]);
	sprintf(last_UID, "%d:%d:%d:%d", uid[0], uid[1], uid[2], uid[3]);

	for (int i = 0; i < token_number; i++)
	{
		for (int k = 0; k < 4; k++) if (uid[k] != tokens[i].uid[k]) tmpResult = false;
		if (tmpResult) nfc_index = i;
		result = tmpResult || result;
		tmpResult = true;
	}

	return nfc_index;
}

//Emits a sound from the speaker.
void sound(int tone_num)
{
  wch_reset();
	if (!SND_ENABLED) return;
	int const time_tone = 200;

	switch (tone_num)
	{
	case 0: //system boot
		tone(soundPin, NOTE_G3, time_tone);
		delay(time_tone);
		tone(soundPin, NOTE_G4, time_tone);
		delay(time_tone);
		tone(soundPin, NOTE_G5, time_tone);
		delay(time_tone);
		noTone(soundPin);
		break;
	case 1: //enable alarm
		tone(soundPin, NOTE_G3, time_tone);
		delay(time_tone);
		tone(soundPin, NOTE_G5, time_tone);
		delay(time_tone);
		noTone(soundPin);
		break;
	case 2: //disable alarm
		tone(soundPin, NOTE_G5, time_tone);
		delay(time_tone);
		tone(soundPin, NOTE_G3, time_tone);
		delay(time_tone);
		noTone(soundPin);
		break;
	case 3: //error
		tone(soundPin, NOTE_G3, time_tone * 2);
		delay(time_tone * 2);
		noTone(soundPin);
		break;
	case 4: //two high
		tone(soundPin, NOTE_G5, time_tone);
		delay(time_tone);
		tone(soundPin, NOTE_G5, time_tone);
		delay(time_tone);
		noTone(soundPin);
		break;
	case 5: //one medium
		tone(soundPin, NOTE_G4, time_tone);
		delay(time_tone);
		noTone(soundPin);
		break;
	case 6: //one high
		tone(soundPin, NOTE_G5, time_tone);
		delay(time_tone);
		noTone(soundPin);
		break;
	case 7: //two very low
		tone(soundPin, NOTE_G2, time_tone);
		delay(time_tone);
		tone(soundPin, NOTE_G2, time_tone);
		delay(time_tone);
		noTone(soundPin);
		break;
	}
  wch_reset();
}

void readTouchInputs() {
	if (!checkInterrupt()) {

		//read the touch state from the MPR121
		Wire.requestFrom(0x5A, 2);

		LSB = Wire.read();
		MSB = Wire.read();

		touched = ((MSB << 8) | LSB); //16bits that make up the touch states

		for (int i = 0; i < 12; i++) { // Check what electrodes were pressed
			if (touched & (1 << i)) {

				if (touchStates[i] == 0) {
					//pin i was just touched
					/*SerialPrint("pin ");
					SerialPrint(i);
					SerialPrintln(" was just touched");
					*/
				}
				else if (touchStates[i] == 1) {
					//pin i is still being touched
				}

				touchStates[i] = 1;
			}
			else {
				if (touchStates[i] == 1) {
					/*     SerialPrint("pin ");
						 SerialPrint(i);
						 SerialPrintln(" is no longer being touched");*/

					//pin i is no longer being touched
				}
				touchStates[i] = 0;
			}

		}


	}
}

void mpr121_setup(void) {

	set_register(ELE_CFG, 0x00);

	// Section A - Controls filtering when data is > baseline.
	set_register(MHD_R, 0x01);
	set_register(NHD_R, 0x01);
	set_register(NCL_R, 0x00);
	set_register(FDL_R, 0x00);

	// Section B - Controls filtering when data is < baseline.
	set_register(MHD_F, 0x01);
	set_register(NHD_F, 0x01);
	set_register(NCL_F, 0xFF);
	set_register(FDL_F, 0x02);

	// Section C - Sets touch and release thresholds for each electrode
	set_register(ELE0_T, TOU_THRESH);
	set_register(ELE0_R, REL_THRESH);

	set_register(ELE1_T, TOU_THRESH);
	set_register(ELE1_R, REL_THRESH);

	set_register(ELE2_T, TOU_THRESH);
	set_register(ELE2_R, REL_THRESH);

	set_register(ELE3_T, TOU_THRESH);
	set_register(ELE3_R, REL_THRESH);

	/*
	set_register(ELE4_T, TOU_THRESH);
	set_register(ELE4_R, REL_THRESH);

	set_register(ELE5_T, TOU_THRESH);
	set_register(ELE5_R, REL_THRESH);

	set_register(ELE6_T, TOU_THRESH);
	set_register(ELE6_R, REL_THRESH);

	set_register(ELE7_T, TOU_THRESH);
	set_register(ELE7_R, REL_THRESH);

	set_register(ELE8_T, TOU_THRESH);
	set_register(ELE8_R, REL_THRESH);

	set_register(ELE9_T, TOU_THRESH);
	set_register(ELE9_R, REL_THRESH);

	set_register(ELE10_T, TOU_THRESH);
	set_register(ELE10_R, REL_THRESH);

	set_register(ELE11_T, TOU_THRESH);
	set_register(ELE11_R, REL_THRESH);
	*/

	// Section D
	// Set the Filter Configuration
	// Set ESI2
	set_register(FIL_CFG, 0x04);

	// Section E
	// Electrode Configuration
	// Set ELE_CFG to 0x00 to return to standby mode
	//set_register(ELE_CFG, 0x0C);  // Enables all 12 Electrodes
	set_register(ELE_CFG, 0x04);		// Enable first 4 electrodes

	// Section F
	// Enable Auto Config and auto Reconfig
	/*set_register(ATO_CFG0, 0x0B);
	set_register(ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V   set_register(ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
	set_register(ATO_CFGT, 0xB5);*/  // Target = 0.9*USL = 0xB5 @3.3V

	set_register(ELE_CFG, 0x0C);

	// Setup the GPIO pins to drive LEDs
	set_register(GPIO_EN, 0xFF);	    // 0x77 is GPIO enable
	set_register(GPIO_DIR, 0xFF);	    // 0x76 is GPIO Dir
	set_register(GPIO_CTRL0, 0xFF);    // Set to LED driver
	set_register(GPIO_CTRL1, 0xFF);    // GPIO Control 1
	set_register(GPIO_CLEAR, 0xFF);    // GPIO Data Clear
}

boolean checkInterrupt(void) {
	return digitalRead(irqPin);
}

uint8_t two[2];
void set_register(unsigned char r, unsigned char v) {
	Wire.beginTransmission(0x5A);
	Wire.write(r);
	Wire.write(v);
	Wire.endTransmission();
}

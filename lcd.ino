bool pstr_priority = true;

void initialize_lcd()
{
  lcd.setWire(Wire);
  lcd.begin(20, 2);
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd_backlight_on();
  lcd.home();  
}  

//turns on the LCD backlight
void lcd_backlight_on()
{
  if (!LCD_ENABLED) return;
  lcd.on();
  lcd.setBacklight(HIGH);
  lcd_status=1;
  led1 = true;
  lcd_bk_ts = millis();
}

//this function is needed since lcd and serial do not work correctly with flash pointer, so i have to move it to sram
void lcd_output_string(const char p[])
{
  if (!LCD_ENABLED) return;
  while (0 != (tmp_char = pgm_read_byte(p++))) lcd.print(tmp_char);
}

//output a PSTR string on the LCD for a limited time
void output_lcd(PGM_P str)
{
  if (!LCD_ENABLED) return;
  pstr_priority = true;
  lcd_message = str;
  lcd_message_ts = millis();
  lcd_backlight_on();
}

//output a dynamic string on the LCD for a limited time
void output_lcd_str(char* str)
{
  if (!LCD_ENABLED) return;
  pstr_priority = false;
  strcpy(lcd_message_str, str);
  lcd_message_ts = millis();
  lcd_backlight_on();
}


//LCD Refresh management routing
void refresh_lcd()
{
  wch_reset();

  if (!LCD_ENABLED || !lcd_status) return;
  lcd.home();

  //DRAW STANDARD SCREEN
  if (!menu_enabled)
  {
    /////PRINT FIRST LINE
    if (enable_alarm)
    {
      lcd_output_string(PSTR("Enabled "));
      if (enable_volumetric) lcd_output_string(PSTR("VOL "));
      if (enable_perimetral) lcd_output_string(PSTR("PERI"));
      lcd_output_string(PSTR("      "));
    }
    else lcd_output_string(PSTR("Alarm Disabled          "));
      
    /////PRINT SECOND LINE
    lcd.setCursor ( 0, 1 );

    //If present, display temporized message
    if ((unsigned long)(millis() - lcd_message_ts) < lcd_message_timeout)
    {
	  if (pstr_priority) lcd_output_string(lcd_message);
	  else lcd.print(lcd_message_str);
      lcd_output_string(PSTR("        "));
    }
    //else, a status
    else
    {
      lcd_message = PSTR("");
      //lcd_message_str = "";
      
      lcd_output_string(PSTR("Status: "));
      lcd.print((alarm ? "alarmed!" : "normal"));
      lcd_output_string(PSTR("       "));
    }
  }
  //DRAW MENU
  else
  {
    /////PRINT FIRST LINE
    lcd_output_string(PSTR("Menu                "));

    /////PRINT SECOND LINE
    lcd.setCursor ( 0, 1 );        
    
    switch (menu_option)
    {
      case 0:
        lcd_output_string(PSTR("All Sensors ON        ")); break;
      case 1:
        lcd_output_string(PSTR("Only perimetral        ")); break;
      case 2:
        lcd_output_string(PSTR("Exit menu        ")); break;
    }
  }

  //turns off backlight after timeout.
  if (ENABLE_BACKLIGHT_CONTROL && (unsigned long)(millis() - lcd_bk_ts) > lcd_bk_period)
  {
    lcd.setBacklight(LOW);
    lcd.off();
    lcd_status=0;
    led1 = false;
	menu_option = 0;
	menu_enabled = false;
  }
}

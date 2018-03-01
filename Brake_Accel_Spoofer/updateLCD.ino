void updateLCD()
{
  lcd.clear();
  lcd.print(stateStrings[State-1]);

  lcd.setCursor(0,1);
  for (int i = 0; i < 5; i++)
  {
    lcd.setCursor(3*i,1);
    lcd.print(buttonNames[i]);
    lcd.print(buttonPressed[i]);
  }

  displayDataChanged = false;
}


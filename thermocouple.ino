#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include "fun.h"

//wyświetlacz
LiquidCrystal lcdDisplay(2,3,4,5,6,7);

//Timery
Timer tempMeasureTimer;
Timer timer2;

//przyciski
Button buttonPlus(8,PULLUP);
Button buttonMinus(9,PULLUP);
Button buttonAccept(10,PULLUP);
Temperature tValue(20);
Temperature tDesired(45);
RegulacjaPID reg;
boolean psConnected = false; //Power Supply connected
byte menuOption = 1;
Button *recentlyPressed; //flaga recently pressed

//RS485
const byte rxPin = 0;
const byte txPin = 1;
const byte receiveEnablePin = 11;
const byte dataEnablePin = 12;
SoftwareSerial rs485(rxPin, txPin);


//Termopara, MAX6675
const byte soPin = A5;
const byte csPin = A4;
const byte sckPin = A3;
MAX6675 thermocouple(sckPin, csPin, soPin);



void setup() {
	timer2.threshold =100;
	//Serial.begin(9600);
	
	//ustawienia rs485
	rs485.begin(9600);
	pinMode(receiveEnablePin,OUTPUT);
	pinMode(dataEnablePin,OUTPUT);
	digitalWrite(receiveEnablePin,LOW);
	digitalWrite(dataEnablePin,HIGH);

	//ustawienie pinów
	
	//ustawienie ekranu
	lcdDisplay.begin(16,2);
	
	lcdDisplay.clear();
	lcdDisplay.setCursor(0,0);
	lcdDisplay.print("USTAW   |  START");
	  
	lcdDisplay.setCursor(0,1);
	lcdDisplay.print("-/+"+(String)(tValue.getTempValue(thermocouple)));
	displayTemp(tDesired.value(),lcdDisplay,1);
	
	
}

void loop() 
{
	boolean sTimer = tempMeasureTimer.stepTimer(500);
	if(sTimer)
	{
		//BŁĄD - nie ma podłączonej termopary, jak najszybciej wszystko wyłączyć
		while(thermocouple.readCelsius() == 0 || isnan(thermocouple.readCelsius()) ) 
		{
			lcdDisplay.setCursor(0,0);
			lcdDisplay.print("BLAD!!! Sprawdz");
			lcdDisplay.setCursor(0,1);
			lcdDisplay.print("termopare");
			delay(2000);
			lcdDisplay.clear();
		}
	}

	if(buttonAccept.uniquePress()) 
	{
		//zamienić na switcha
		//if(menuOption == 2 && buttonAccept.timePressed() < 2000) menuOption = 2;
		menuOption++;
		if(menuOption > 3) menuOption = 1;
		lcdDisplay.clear();
	}
	//if(menuOption == 3 && buttonAccept.timePressed() < 2000) menuOption = 2;
	//else if (buttonAccept.isPressed()>2000 && menuOption ==2 ) menuOption = 3;
	
	switch(menuOption)
	{
		case 1: //ustawienia temperatury
		{
			lcdDisplay.setCursor(0,0);
			lcdDisplay.print("USTAW   |  START");
			if(buttonPlus.uniquePress())
			{
				displayTemp(tDesired.set( tDesired.value() + 0.5 ), lcdDisplay, 1);
				recentlyPressed = &buttonPlus;
			}
			if (buttonMinus.uniquePress())
			{
				displayTemp(tDesired.set( tDesired.value() - 0.5 ), lcdDisplay, 1);
				recentlyPressed = &buttonMinus;
			}
			unsigned long czas = recentlyPressed -> timePressed();
			if(czas>1000 && czas%200 < 10)
			{
				if(recentlyPressed == &buttonPlus) tDesired.set( tDesired.value() + 0.5 );
				if(recentlyPressed == &buttonMinus) tDesired.set( tDesired.value() - 0.5 );
				displayTemp(tDesired.value(), lcdDisplay, 1);
			}
			lcdDisplay.setCursor(0,1);
			if(sTimer) lcdDisplay.print("-/+"+(String)(tValue.getTempValue(thermocouple)));
			
			//break case
			break;
		}
		case 2: //ustawienie zasilacza
		{
			if(timer2.stepTimer(1000))
			{
				lcdDisplay.clear();
				lcdDisplay.setCursor(0,0);
				lcdDisplay.print("Polacz z");
				lcdDisplay.setCursor(0,1);
				lcdDisplay.print("zasilaczem (+)");	
			}
			if(buttonPlus.uniquePress()) //rozpocznij sesję na PS
			{
				String napis = "dobrze";
				//for(int i=0; i< 300; i++) 
				rs485.print(napis);
				while(rs485.read() != -1);
				digitalWrite(receiveEnablePin,HIGH);
				digitalWrite(dataEnablePin,LOW);
				while(!rs485.available()) {}
				napis = rs485.readStringUntil(0x0D);
				while(rs485.available())
				{
					rs485.println(napis); delay(200);
					if(buttonPlus.isPressed()) break;
				}
				//napis = rs485.readStringUntil(0x0D);
				rs485.print(napis+" ");
				digitalWrite(receiveEnablePin,LOW);
				digitalWrite(dataEnablePin,HIGH);
			}
			
				//break case
				break;
		}
		case 3: //regulacja temperatury
		{
			lcdDisplay.setCursor(0,0);
			lcdDisplay.print("Temperatura:");
			displayTemp(tDesired.value(), lcdDisplay, 1);
			if(sTimer) displayTemp(tValue.getTempValue(thermocouple), lcdDisplay, 0);
			//tempSimul(reg, tValueFloat, tDesired.value());
			
			//case break
			break;
		}
		
		default: break;
	}
	
}
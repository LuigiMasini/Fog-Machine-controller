//BEGIN dec
//Pin numbers
#define THERPIN A7
#define POTEPIN A6
#define HEATERPIN 4
#define MOTORPIN 3
#define BUTTONPIN 2
//#define DIPIN 3	//dipswitch to control state of temp nozzle, optional
//#define DIPIB 4	//dipswitch to control state of pump button, optional

//Misc (see readtemperature.ino for more)
#define MINT 120	//lowest temp
#define MAXT 250	//highest temp

//#define ERROR
//#define DEBUG

//Macros

#define GETPOTEMP temp.pote.curr = map(analogRead(POTEPIN), 0, 1023, MINT, MAXT);

/*relay module for HEAT and PUMPs, LOW close circuit, HIGH open*/
#ifdef DEBUG
	#define HEAT digitalWrite(HEATERPIN, LOW);\
	Serial.println("H start");

	#define PUMP digitalWrite(MOTORPIN, LOW);\
	Serial.println("P start");
	
	#define SHEAT digitalWrite(HEATERPIN, HIGH);\
	Serial.println("H stop");

	#define SPUMP digitalWrite(MOTORPIN, HIGH);\
	Serial.println("P stop");
#else
	#define HEAT digitalWrite(HEATERPIN, LOW);

	#define PUMP digitalWrite(MOTORPIN, LOW);

	#define SHEAT digitalWrite(HEATERPIN, HIGH);
	
	#define SPUMP digitalWrite(MOTORPIN, HIGH);
#endif

//Strings

String temps = " temp:\t";
String act = "active";
String ina = "inactive";


//Global variables
bool an = true;		//active nozzle flag, default is active
bool ab = true;		//active button flag, default is active
bool alt = false;	//manually stop heating
bool mon = false;	//monitoring temperature
bool tpf = false;	//temperature probe failed

struct temp {
	float sett;  //wanted temp
	float measure;  //measured temp
	struct pote {
		float curr;
		float old;
	} pote;
	float seri;
} temp;
//END dec


void setup() {
	
	Serial.begin(9600);
	Serial.println("Booting...");
	analogReference(EXTERNAL);  //reference is voltage applied to AREF (0V to 5V)
	
	pinMode(HEATERPIN, OUTPUT);
	pinMode(MOTORPIN, OUTPUT);
	pinMode(BUTTONPIN, INPUT);
	#ifdef DIPIN
	pinMode(DIPIN, INPUT);
	an = digitalRead(DIPIN);
	#endif
	#ifdef DIPIB
	pinMode(DIPIB, INPUT);
	an = digitalRead(DIPIB);
	#endif
	if (an) {
		GETPOTEMP
		temp.sett = temp.pote.curr;
	}
	else
		temp.sett = 250;	//default temp;
	
	HEAT
	SPUMP
	//temp.measure = 200;	//TEST
}

void loop() {
	
	
	if (Serial.available()) {
		
		String str = Serial.readString();	//string sent from computer
		char *strp = str.c_str(), com;		//String to char*; one-char command
		sscanf(strp++, "%c", &com);		//char* to char, pointer++ for later extraction in t & d comands
		switch(com){
			case 'a' :
				if (!alt){
					Serial.println("Stopping heating...");
					alt=true;
					SHEAT;
				}
				else {
					Serial.println("Resuming normal functioning");
					alt=false;
				}
				break;
			case 'm' :
				if (!mon){
					Serial.println("Activating temperature monitoring...");
					mon=true;
				}
				else {
					Serial.println("Disabling temperature monitoring...");
					mon=false;
				}
				
			case 't' :
				/*set new temp from serial connection, this will 
				 * overrides by design temp from nozzle
				 */
				//take it from string
				sscanf(strp, "%f", &temp.seri);
				
				if (temp.seri >= MINT && temp.seri <= MAXT){
					temp.sett = temp.seri;
					#ifdef DEBUG
					Serial.print("New serial");
					Serial.print(temps);
					Serial.print(temp.seri);
					Serial.println("°C");
					#endif
					temp.pote.old = temp.pote.curr;          
				}
				else
					#ifdef ERROR
					Serial.println("TEMP not in range!!(see 'f')");
					#endif
				
				break;
				
			case 'p' :
				/*start / stop pumping, just if button is inactive*/
				if (ab){
					#ifdef ERROR
					Serial.print("Button is ");
					Serial.print(act);
					Serial.println(", disable it first with 'db'");
					#endif
				}
				else if (digitalRead(MOTORPIN)){
					PUMP
				}
				else{
					SPUMP
				}
				break;
			case 'd' :
				/*enable / disable nozzle and button, maybe broken, along with a dip switch
				 *Note that the dip switch is red only once during startup,
				 *modifying its position wont have any effect till next boot
				 */
				
				//take it from string
				sscanf(strp, "%c", &com);
				switch(com){
					case 'n' :
						an = !an;
						#ifdef DEBUG
						Serial.print("Nozzle set to ");
						Serial.println(an ? act : ina);
						#endif
						break;
					case 'b' :
						ab = !ab;
						#ifdef DEBUG
						Serial.print("Button set to ");
						Serial.println(ab ? act : ina);
						#endif
						break;
					#ifdef ERROR
					default : 
						Serial.print("No device to disable with ");
						Serial.println(com);
					#endif
				}
				break;
			case 'f' :
				/*print complete current configuration, includes status(no breaks)*/
				Serial.print("\nCURRENT CONFIGURATION:\n");
				
				Serial.print("Maximum");
				Serial.print(temps);
				Serial.println(MAXT);
				
				Serial.print("Minimum");
				Serial.print(temps);
				Serial.println(MINT);
				
				Serial.print("Nozzle status:\t");
				Serial.println( an ? act : ina);
				
				Serial.print("Last nozzle");
				Serial.print(temps);
				Serial.println(temp.pote.curr);
				
				Serial.print("Last serial");
				Serial.print(temps);
				Serial.println(temp.seri);
				
			case 's' :
				/*print current status*/
				Serial.print("\nSTATUS:\n");
				
				Serial.print("Goal");
				Serial.print(temps);
				Serial.println(temp.sett);
				
				Serial.print("Current");
				Serial.print(temps);
				Serial.println(temp.measure);
				
				/*it is the user that check for incoerences, fails & other errors*/
				Serial.println(digitalRead(HEATERPIN) ? "not heating (Ready?)" : "Heating");
				
				Serial.print(digitalRead(BUTTONPIN) ? "pressed, " : "not pressed, ");
				Serial.println(!digitalRead(MOTORPIN) ? "pumping" : "not pumping");
				
				Serial.println();
				break;
			/*
			case 'c' :
				Serial.println(!digitalRead(HEATERPIN) ? "not heating" : "Heating");
				digitalWrite(HEATERPIN, !digitalRead(HEATERPIN));
				break;
			*/
			#ifdef ERROR
			default :
				Serial.println("Command not recognised!");
			#endif
		}
	}
	else if (an) {
		/*if temp not changed by serial connection,
		 *    check if it changed on nozzle, if active*/
		GETPOTEMP
		
		//non uso != perchè valore molto ballerino, mentre se è variato di almeno un grado(.2) è manopola che è stata ruotata e non errore di lettura 
		if (temp.pote.curr < temp.pote.old-1.2 || temp.pote.curr > temp.pote.old+1.2){
			temp.pote.old = temp.pote.curr;
			temp.sett = temp.pote.curr;
			#ifdef DEBUG
			Serial.print("New nozzle");
			Serial.print(temps);
			Serial.print(temp.pote.curr);
			Serial.println("°C");
			#endif
		}
	}
	
	
	temp.measure = getTemp(THERPIN);
	
	//if monitoring temp is active
	if (mon){
		Serial.print(temps);
		Serial.print(temp.measure);
		Serial.println(" °C");
	}
	
	
	/* BEGIN old stuff
	
	//if not ready & not heating
	if (temp.measure < temp.sett && digitalRead(HEATERPIN)){
		HEAT;			//start heating
	}
	//else if ready & heating
	else if (temp.measure >= temp.sett){
		if (!digitalRead(HEATERPIN)){
			SHEAT;		//stop heating
		}
	}
	
	if(temp.measure < MINT){
		#ifdef ERROR
		Serial.print("Too low");
		Serial.println(temps);
		#endif
		if (!digitalRead(MOTORPIN)){
			SPUMP			//stop pumping
		}
		if (digitalRead(HEATERPIN)){
			HEAT
		}
	}
	else {
		if(ab){
			//if button pressed but not pumping
			if (digitalRead(BUTTONPIN) && digitalRead(MOTORPIN)){
				PUMP		//start pumping
			}
			//else if not pressed but still pumping
			else if (!digitalRead(BUTTONPIN) && !digitalRead(MOTORPIN)){
				SPUMP		//stop pumping
			}
		}
	}
	
	END old stuff*/
	
	//BEGIN security controls
	
	//if manually stopped heating
	if (alt){
		
		//if still heating
		if (!digitalRead(HEATERPIN)){
			SHEAT;
		}
		
		//cause the function loop to end immediately and restart
		return;
	}
	
	//if measured temperature is not reasonable (sometimes the thermometer fails)
	if (temp.measure <  -40){
		
		//if still heating
		if (!digitalRead(HEATERPIN)){
			tpf = true;
			Serial.println("    Error!    Temperature probe failing!\n");
			SHEAT;
			Serial.println("use \'m\' to monitor the measured temperature");
		}
		
		//cause the function loop to end immediately and restart
		return;
	}
	else {
		//if not failing
		
		//if failed before
		if (tpf) {
			Serial.print("Temperature probe seems back to normal");
			Serial.println("Resuming standard operations");
			tpf=false;
		}
		
	}
	
	//END security controls
	
	//if temp is not the desired one
	if (temp.measure < temp.sett){
		
		//if not heating
		if (digitalRead(HEATERPIN)){
			HEAT;
		}
		
		/* without this is possible to pump (db + p) even if is not hot
		//if temp below minimum && pumping
		if (temp.measure < MINT && !digitalRead(MOTORPIN)){
			SPUMP;		//Stop Pumping
		}
		*/
	}
	else {
		//if temperature is >= the desired one
		
		//if heating
		if (!digitalRead(HEATERPIN)){
			SHEAT;		//Stop Heating
		}
		
		//if button active
		if (ab) {
			
			//if button pressed
			if (digitalRead(BUTTONPIN)){
				
				//if not pumping
				if (digitalRead(MOTORPIN)){
					PUMP;
				}
			}
			else{
				//if button not pressed
				
				//if pumping
				if (!digitalRead(MOTORPIN)){
					SPUMP;
				}
			}
			
		}
	}
}

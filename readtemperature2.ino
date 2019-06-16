/*
 * Thermistor Example #3 from the Adafruit Learning System guide on Thermistors
 * https://learn.adafruit.com/thermistor/overview by Limor Fried, Adafruit Industries
 * MIT License - please keep attribution and consider buying parts from Adafruit
 */

/*
 * // which analog pin to connect
 * #define THERMISTORPIN A0         
 */
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000    

float getTemp(int ThermistorPin) { //analog pin to use for reading, no other params, no need in this case but not flexible
	#ifdef DEBUG
	Serial.print("Measuring");
	Serial.println(temps);
	#endif
	float average=0;
	/*
	//bad solution, i dont need array with sigle values
	int samples[NUMSAMPLES];
	// take N samples in a row, with a slight delay
	for (i=0; i< NUMSAMPLES; i++) {
		samples[i] = analogRead(ThermistorPin);
		delay(10);
	}
	// average all the samples out
	for (i=0; i< NUMSAMPLES; i++)
		average += samples[i];
	*/
	for (int i=0; i<NUMSAMPLES; i++){
		average+=analogRead(ThermistorPin);
		delay(10);
	}
	average /= NUMSAMPLES;
	#ifdef DEBUG 
	Serial.print("Average analog reading "); 
	Serial.println(average);
	#endif
	// convert the value to resistance
	/*Calculation
	 *  Vo = R / (R + 10K) * Vcc
	 *  ADC value = Vi * 1023 / Varef
	 *  Vi = Vo
	 *  Varef = Vcc
	 * 
	 *  ADC value = R / (R + 10K) * Vcc * 1023 / Varef
	 *  ADC value = R / (R + 10K) * 1023
	 *  R = 10K / (1023/ADC - 1) 
	 */
	average = 1023 / average - 1;
	average = SERIESRESISTOR / average;
	#ifdef DEBUG
	Serial.print("Thermistor resistance "); 
	Serial.println(average);
	#endif
	//convert resistance to temperature
	/*calculation (simplified steinhart-hart equation)
	 *  1/T = 1/To + ln(R/Ro) / B
	 * 
	 *  T = To + B / ln(R/Ro) giusta??
	 */
	/*
	float steinhart;
	steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
	steinhart = log(steinhart);                  // ln(R/Ro)
	steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
	steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
	steinhart = 1.0 / steinhart;                 // Invert
	steinhart -= 273.15;                         // convert to C
	*/
	
	average /= THERMISTORNOMINAL;			// (R/Ro)
	average = log(average);				// ln(R/Ro)
	average /= BCOEFFICIENT;			// 1/B * ln(R/Ro)
	average += 1.0 / (TEMPERATURENOMINAL + 273.15);	// + (1/To)
	average = 1.0 / average;			// Invert
	average -= 273.15;				// convert to C
	#ifdef DEBUG
	Serial.print("Temperature "); 
	Serial.print(average);
	Serial.println(" *C");
	#endif
	return average;
}


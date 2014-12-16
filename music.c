#include "pre_emptive_os/api/general.h"
#include "lpc2xxx.h"
#include "music.h"
#include "music/beginning_sound.h"
#include "startup/config.h"

void initDAC(void) {
	PINSEL1 &= ~0x000C0000;  // DAC odpowiadają piny 18..19, zerujemy je
	PINSEL1 |=  0x00080000;  // Ustawiamy je na 10 - uaktywniamy "second alternate option" czyli AOUT = P0.25
}


// BARDZO MNIE MARTWI TO, ŻE TO OSTATNIO NIE ZADZIAŁAŁO, BO JEST 100% OK WG. MANUALA
void delay90usTimer(void) {
//  T1CTCR = 0x00;  		 // Jakby nie działało - można spróbować
//	Jakby nie działało - sugestia nr 2 - może T1 jest gdzieś używany i powinniśmy wykorzystać T0?
//	Jakby nie działało - sugestia nr 3 - jedna z grup ustawia T1PC = 0x00 Jeśli wierzyć instrukcji, to nie ma sensu, ale...

  T1TCR = 0x02;          // ustawienie drugiego bitu TimerControlRegister powoduje reset Timer i Prescale Counterów
  T1PR  = 0x00;          // ustawia PrescaleRegister na 0 - TimerCounter będzie teraz skakał co zbocze ProcessorClocka
  T1MR0 = (long) (90 * (long long) CORE_FREQ / 1000) / 1000;  // ustawiamy MatchRegister na oczekiwaną wartość
  T1IR  = 0xff;          // resetujemy flagi przerwań - wydaje mi się, że to niepotrzebne, ale wszyscy to robią
  	  	  	  	  	  	 // spróbujemy wywalić jak reszta zadziałą
  T1MCR = 0x04;          // ustawiamy 3 bit MatchControlRegister - zatem gdy TC dojdzie do MR0 TC się zatrzyma
  	  	  	  	  	  	 // i TCR[0] przejdzie na 0
  T1TCR = 0x01;          // TCR[0] na 1 - uruchamiamy timer
}

void waitForTimer(void) {
	while (T1TCR & 0x01)
		;
}

/*
 * Plan B
 *
// Jeżeli dobrze policzyłem, to muzyka z SamplingRate 11025Hz (taką faliczek wyciągnął z .wava)
// musi być odtwarzana z prędkością sample (bajt) na 90.7us
// Jeżeli znalazłem dobrą informację, to nasz procesor ma częstotliwość wykonywania operacji 12MHz
// Musimy zatem, zdaje się, odczekać fp / fs = 1088 operacji miedzy każdym samplem. (lub może nieco mniej...)
void delay90usNop(void) {
	int j;
    for (j = 0; j < 1088; j++) {
    	asm volatile (" nop");
    }
//	osSleep(1); Uhh... jest jeszcze coś takiego, plan C?
}
*/


// bity 6 - 15 są na wartość - zatem jest ich 10, ale we wszędzie używają tylko 8 z nich
// bit 16 jest na bias. BIAS = 1 oznacza 2.5uS settling time i 350uA natężenie, a BIAS = 0 1.0uS i 700uA
// zatem pozwala balansować między mniejszym opóźnieniem, a zużyciem mocy
// bity 0 - 5 i 17 - 31 są nieużywane
void playBeginningSound(void) {
    int t;

    for (t = 0; t < BEGINNING_SOUND_LEN; ++t) {
		delay90usTimer();

		DACR = (pacmanBeginningSound[t] << 6) |
			   (1 << 16);

		waitForTimer();
	}
}
/*
W jednym z przykładów zrobili to tak. Nie mam pojecia dlaczego, może chodzić o signed / unsigned.
Zostawiam, byśmy mogli wypróbować, jakby moja wersja zawiodła... ale ostatnio efektem były te pierdy

tS32 val;
val = startupSound[cnt] - 128;
val = val * 2;
if (val > 127) val = 127;
else if (val < -127) val = -127;

DACR = ((val+128) << 8) |
		(1 << 16);

// delay 125 us = 850 for 8kHz, 600 for 11 kHz
for(i=0; i<850; i++)
	asm volatile (" nop");
*/

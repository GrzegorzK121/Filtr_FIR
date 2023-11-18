/*
 * Projekt z przedmiotu Zastosowania Procesorów Sygna³owych
 * Szablon projektu dla DSP TMS320C5535
 * Grupa T1 1 Stencel Kaczorek
 */


// Do³¹czenie wszelkich potrzeddbnych plików nag³ówkowych
#include "usbstk5515.h"
#include "usbstk5515_led.h"
#include "aic3204.h"
#include "PLL.h"
#include "bargraph.h"
#include "oled.h"
#include "pushbuttons.h"
#include "dsplib.h"

//Krok fazy dla sygna³u pi³okszta³tnego (100Hz)
#define KROK_FAZY_PILO 136
// Czêstotliwoœæ próbkowania (48 kHz)
#define CZESTOTL_PROBKOWANIA 48000L
// Wzmocnienie wejœcia w dB: 0 dla wejœcia liniowego, 30 dla mikrofonowego
#define WZMOCNIENIE_WEJSCIA_dB 30

#define X 2148
#define N 52  // rz¹d 51 + 1 = 52
#define fc 1760


const short FIR_wspolczynniki[N] = {
    -109,-197,-282,-360,-425,-473,-500,-502,-475,-418,-330,-211,-61,115,316,535,768,1008,
	1249,1482,1702,1901,2074,2214,2318,2382,2403,2382,2318,2214,2074,1901,1702,1482,1249,
	1008,768,535,316,115,-61,-211,-330,-418,-475,-502,-500,-473,-425,-360,-282,-197,-109};
	
int akumulator_fazy;

int bufor[X]
int index = 0;

short bufor_roboczy[N+2];

void rand16init();

// G³ówna procedura programu

void main(void) {

	// Inicjalizacja uk³adu DSP
	USBSTK5515_init();			// BSL
	pll_frequency_setup(100);	// PLL 100 MHz
	aic3204_hardware_init();	// I2C
	aic3204_init();				// kodek dŸwiêku AIC3204
	USBSTK5515_ULED_init();		// diody LED
	SAR_init_pushbuttons();		// przyciski
	oled_init();				// wyœwielacz LED 2x19 znaków

	// ustawienie czêstotliwoœci próbkowania i wzmocnienia wejœcia
	set_sampling_frequency_and_gain(CZESTOTL_PROBKOWANIA, WZMOCNIENIE_WEJSCIA_dB);

	// wypisanie komunikatu na wyœwietlaczu
	// 2 linijki po 19 znaków, tylko wielkie angielskie litery
	oled_display_message("PROJEKT ZPS        ", "                   ");

	// 'tryb_pracy' oznacza tryb pracy wybrany przyciskami
	unsigned int tryb_pracy = 0;
	unsigned int poprzedni_tryb_pracy = 99;

	// zmienne do przechowywania wartoœci próbek
	short lewy_wejscie, prawy_wejscie, lewy_wyjscie, prawy_wyjscie;

	// Przetwarzanie próbek sygna³u w pêtli
	while (1) {

		// odczyt próbek audio, zamiana na mono
		aic3204_codec_read(&lewy_wejscie, &prawy_wejscie);
		short mono_wejscie = (lewy_wejscie >> 1) + (prawy_wejscie >> 1);

		// sprawdzamy czy wciœniêto przycisk
		// argument: maksymalna liczba obs³ugiwanych trybów
		tryb_pracy = pushbuttons_read(4);
		if (tryb_pracy == 0) // oba wciœniête - wyjœcie
			break;
		else if (tryb_pracy != poprzedni_tryb_pracy) {
			// nast¹pi³a zmiana trybu - wciœniêto przycisk
			
		    for(int i = 0; i < N; i++)
                buf_roboczy[i] = 0;
			
			USBSTK5515_ULED_setall(0x0F); // zgaszenie wszystkich diód
			if (tryb_pracy == 1) {
				// wypisanie informacji na wyœwietlaczu
				oled_display_message("PROJEKT ZPS        ", "TRYB 1             ");
				// zapalenie diody nr 1
				USBSTK5515_ULED_on(0);
			} else if (tryb_pracy == 2) {
				oled_display_message("PROJEKT ZPS        ", "TRYB 2             ");
				USBSTK5515_ULED_on(1);
			} else if (tryb_pracy == 3) {
				oled_display_message("PROJEKT ZPS        ", "TRYB 3             ");
				USBSTK5515_ULED_on(2);
			} else if (tryb_pracy == 4) {
				oled_display_message("PROJEKT ZPS        ", "TRYB 4             ");
				USBSTK5515_ULED_on(3);
			}
			// zapisujemy nowo ustawiony tryb
			poprzedni_tryb_pracy = tryb_pracy;
		}


		// zadadnicze przetwarzanie w zale¿noœci od wybranego trybu pracy

		if (tryb_pracy == 1) 
		{
			lewy_wyjscie = mono_wejscie;
			prawy_wyjscie = mono_wejscie;

		} 
		else if (tryb_pracy == 2) 
		{
            rand16((DATA *)&lewy_wyjscie,, 1);
            oflag = fir((DATA *)&lewy_wyjscie, (DATA *)FIR_wspolczynniki, (DATA *)&lewy_wyjscie, (DATA *)bufor_roboczy, 1, N);
			//fir((DATA *)&lewy_wyjscie, (DATA *)FIR_wspolczynniki, (DATA *)&lewy_wyjscie, (DATA *)bufor_roboczy, 1, N);

            prawy_wyjscie = lewy_wyjscie;

		} 
		else if (tryb_pracy == 3) 
		{
			lewy_wyjscie = mono_wejscie;
			prawy_wyjscie = mono_wejscie;
			
			lewy_wyjscie = (akumulator_fazy);
            oflag = fir((DATA *)&lewy_wyjscie, (DATA *)FIR_wspolczynniki, (DATA *)&lewy_wyjscie, (DATA *)bufor_roboczy, 1, N);
            //fir((DATA *)&lewy_wyjscie, (DATA *)FIR_wspolczynniki, (DATA *)&lewy_wyjscie, (DATA *)bufor_roboczy, 1, N);

			lewy_wyjscie >>= 4;
			
            prawy_wyjscie = lewy_wyjscie;

		} 
		else if (tryb_pracy == 4) 
		{
			lewy_wyjscie = mono_wejscie;
			prawy_wyjscie = mono_wejscie;
			oflag = fir((DATA *)&mono_wejscie, (DATA *)FIR_wspolczynniki, (DATA *)&lewy_wyjscie, (DATA *)bufor_roboczy, 1, N);
			//fir((DATA *)&mono_wejscie, (DATA *)FIR_wsp_dp, (DATA *)&lewy_wyjscie, (DATA *)buf_rob, 1, N);

		}

		akumulator_fazy += KROK_FAZY_PILO;
		
		bufor[index] = lewy_wyjscie;
		
		index++;
		
		if(index == X)
		{
			index = 0; //tu breakpoint do wykresu
		}

		// zapisanie wartoœci na wyjœcie audio
		aic3204_codec_write(lewy_wyjscie, prawy_wyjscie);

	}


	// zakoñczenie pracy - zresetowanie kodeka
	aic3204_disable();
	oled_display_message("KONIEC PRACY       ", "                   ");
	while (1);
}
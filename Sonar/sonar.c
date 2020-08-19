/***********************************************************
*  sonar.c												   *
*  														   *
*  Sonar project for ProKaTim (first release)              *
*  														   *
*  Last update : 02.08.2020							   	   *
*  functions bit_rev and tw_genr2fft added   			   *
*  + cross_correlation_frequency finished				   *
*  + AIC23 configuration corrected 						   *
*  														   *
************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <csl.h>
#include <csl_mcbsp.h>
#include <csl_irq.h>
#include <csl_edma.h>
#include <dsk6713_led.h>
#include "config_AIC23.h"
#include "sonar.h"
#include "sonarcfg.h"
#include <DSPF_sp_cfftr2_dit.h>
#include <DSPF_sp_icfftr2_dif.h>


/*****************************************************************/

//#define SWITCH     //uncomment for frequency domain calculation

/*****************************************************************/


#define SWEEP_LEN 2880		// 60 ms
#define RESPONSE_LEN 4320   // 60 + 30 ms
#define FFT_LEN 16384       // next power of 2 for the dit/dif algorithm (complex array => length = 2 x length)

#define PI 3.14159265358979323846

/*########## DATA BUFFERS ##########*/
/* no ping pong buffers needed (calculation made offline)  => only 1 buffer for input and 1 for output */

#pragma DATA_SECTION(Buffer_in, ".databuffer");
short Buffer_in[RESPONSE_LEN];
#pragma DATA_SECTION(Buffer_out, ".databuffer");
short Buffer_out[SWEEP_LEN];
/* Memory "Buffers" set to adequate length ( 0x1C200 )*/

/*######## PROCESS BUFFERS #########*/
// all initalized with zeros so the filling is easier

//buffer for cross correlation in time domain
#pragma DATA_SECTION(sweep_time, ".processbuffer");			// freq sweep vector filled with zeros to RESPONSE_LEN
short sweep_time[RESPONSE_LEN]={0};

//buffers for cross correlation in frequency domain
#pragma DATA_SECTION(sweep_freq, ".processbuffer");			//freq sweep vector for fft
float sweep_freq[FFT_LEN]={0};
#pragma DATA_SECTION(fft_coeff, ".processbuffer");			//coefficient vector for fft function
float fft_coeff[FFT_LEN/2]={0};
#pragma DATA_SECTION(response_freq, ".processbuffer");		//response vector for fft
float response_freq[FFT_LEN]={0};
#pragma DATA_SECTION(cross_corr_freq, ".processbuffer");	//sweep_freq_fft x response_freq_fft
float cross_corr_freq[FFT_LEN]={0};



/*######### CONFIGURATION FUNCTIONS #########*/

//Configuration for McBSP1 (data-interface)
MCBSP_Config datainterface_config = {
		/* McBSP Control Register */
        MCBSP_FMKS(SPCR, FREE, NO)              |	//  Freilauf
        MCBSP_FMKS(SPCR, SOFT, NO)          |
        MCBSP_FMKS(SPCR, FRST, YES)             |	// Framesync ist ein
        MCBSP_FMKS(SPCR, GRST, YES)             |	// Reset aus, damit l�uft der Samplerate- Generator
        MCBSP_FMKS(SPCR, XINTM, OF(0))          |
        MCBSP_FMKS(SPCR, XSYNCERR, NO)          |	// empf�ngerseitig keine �berwachung der Synchronisation
        MCBSP_FMKS(SPCR, XRST, YES)             |	// Sender l�uft (kein Reset- Status)	
        MCBSP_FMKS(SPCR, DLB, OFF)              |	// Loopback (Kurschluss) nicht aktiv
        MCBSP_FMKS(SPCR, RJUST, RZF)            |	// rechtsb�ndige Ausrichtung der Daten im Puffer
        MCBSP_FMKS(SPCR, CLKSTP, DISABLE)       |	// Clock startet ohne Verz�gerung auf fallenden Flanke (siehe auch PCR-Register)
        MCBSP_FMKS(SPCR, DXENA, OFF)            |	// DX- Enabler wird nicht verwendet
        MCBSP_FMKS(SPCR, RINTM, RRDY)           |	// Sender Interrupt wird durch "RRDY-Bit" ausgel�st
        MCBSP_FMKS(SPCR, RSYNCERR, NO)          |	// senderseitig keine �berwachung der Synchronisation
        MCBSP_FMKS(SPCR, RRST, YES),			// Empf�nger l�uft (kein Reset- Status)
		/* Empfangs-Control Register */
        MCBSP_FMKS(RCR, RPHASE, SINGLE)         |	// Nur eine Phase pro Frame
        MCBSP_FMKS(RCR, RFRLEN2, DEFAULT)       |	// L�nge in Phase 2, unrelevant
        MCBSP_FMKS(RCR, RWDLEN2, DEFAULT)       |	// Wortl�nge in Phase 2, unrelevant
        MCBSP_FMKS(RCR, RCOMPAND, MSB)          |	// kein Compandierung der Daten (MSB first)
        MCBSP_FMKS(RCR, RFIG, NO)               |	// Rahmensynchronisationspulse (nach dem ersten Puls)) startet die �bertragung neu
        MCBSP_FMKS(RCR, RDATDLY, 0BIT)          |	// keine Verz�gerung (delay) der Daten
        MCBSP_FMKS(RCR, RFRLEN1, OF(1))         |	// L�nge der Phase 1 --> 1 Wort
        MCBSP_FMKS(RCR, RWDLEN1, 16BIT)         |	//
        MCBSP_FMKS(RCR, RWDREVRS, DISABLE),		// 32-bit Reversal nicht genutzt
		/* Sende-Control Register */
        MCBSP_FMKS(XCR, XPHASE, SINGLE)         |	//
        MCBSP_FMKS(XCR, XFRLEN2, DEFAULT)       |	// L�nge in Phase 2, unrelevant
        MCBSP_FMKS(XCR, XWDLEN2, DEFAULT)       |	// Wortl�nge in Phase 2, unrelevant
        MCBSP_FMKS(XCR, XCOMPAND, MSB)          |	// kein Compandierung der Daten (MSB first)
        MCBSP_FMKS(XCR, XFIG, NO)               |	// Rahmensynchronisationspulse (nach dem ersten Puls)) startet die �bertragung neu
        MCBSP_FMKS(XCR, XDATDLY, 0BIT)          |	// keine Verz�gerung (delay) der Daten
        MCBSP_FMKS(XCR, XFRLEN1, OF(1))         |	// L�nge der Phase 1 --> 1 Wort
        MCBSP_FMKS(XCR, XWDLEN1, 16BIT)         |	// Wortl�nge in Phase 1 --> 16 bit
        MCBSP_FMKS(XCR, XWDREVRS, DISABLE),		// 32-bit Reversal nicht genutzt
		/* Sample Rate Generator Register */
        MCBSP_FMKS(SRGR, GSYNC, DEFAULT)        |	// Einstellungen nicht relevant da
        MCBSP_FMKS(SRGR, CLKSP, DEFAULT)        |	// der McBSP1 als Slave l�uft
        MCBSP_FMKS(SRGR, CLKSM, DEFAULT)        |	// und den Takt von aussen 
        MCBSP_FMKS(SRGR, FSGM, DEFAULT)         |	// vorgegeben bekommt.
        MCBSP_FMKS(SRGR, FPER, DEFAULT)         |	// --
        MCBSP_FMKS(SRGR, FWID, DEFAULT)         |	// --
        MCBSP_FMKS(SRGR, CLKGDV, DEFAULT),		// --
		/* Mehrkanal */
        MCBSP_MCR_DEFAULT,				// Mehrkanal wird nicht verwendet
        MCBSP_RCER_DEFAULT,				// dito
        MCBSP_XCER_DEFAULT,				// dito
		/* Pinout Control Register */
        MCBSP_FMKS(PCR, XIOEN, SP)              |	// Pin wird f�r serielle Schnittstelle verwendet (alternativ GPIO)
        MCBSP_FMKS(PCR, RIOEN, SP)              |	// Pin wird f�r serielle Schnittstelle verwendet (alternativ GPIO)
        MCBSP_FMKS(PCR, FSXM, EXTERNAL)         |	// Framesync- Signal f�r Sender kommt von extern (Slave)
        MCBSP_FMKS(PCR, FSRM, EXTERNAL)         |	// Framesync- Signal f�r Empf�nger kommt von extern (Slave)
        MCBSP_FMKS(PCR, CLKXM, INPUT)           |	// Takt f�r Sender kommt von extern (Slave)
        MCBSP_FMKS(PCR, CLKRM, INPUT)           |	// Takt f�r Empf�nger kommt von extern (Slave)
        MCBSP_FMKS(PCR, CLKSSTAT, DEFAULT)      |	// unrelevant da PINS keine GPIOs
        MCBSP_FMKS(PCR, DXSTAT, DEFAULT)        |	// unrelevant da PINS keine GPIOs
        MCBSP_FMKS(PCR, FSXP, ACTIVEHIGH)       |	// Framesync senderseitig ist "activehigh"
        MCBSP_FMKS(PCR, FSRP, ACTIVEHIGH)       |	// Framesync empf�ngerseitig ist "activehigh"
        MCBSP_FMKS(PCR, CLKXP, FALLING)         |	// Datum wird bei fallender Flanke gesendet
        MCBSP_FMKS(PCR, CLKRP, RISING)			// Datum wird bei steigender Flanke �bernommen
};

/* template for a EDMA configuration */
EDMA_Config configEDMARcv = {
    EDMA_FMKS(OPT, PRI, LOW)          |  // auf beide Queues verteilen
    EDMA_FMKS(OPT, ESIZE, 16BIT)       |  // Element size
    EDMA_FMKS(OPT, 2DS, NO)            |  // kein 2D-Transfer
    EDMA_FMKS(OPT, SUM, NONE)          |  // Quell-update mode -> FEST (McBSP)!!!
    EDMA_FMKS(OPT, 2DD, NO)            |  // 2kein 2D-Transfer
    EDMA_FMKS(OPT, DUM, INC)           |  // Ziel-update mode
    EDMA_FMKS(OPT, TCINT,YES)         |  // EDMA interrupt erzeugen?
    EDMA_FMKS(OPT, TCC, OF(0))         |  // Transfer complete code (TCC)
    EDMA_FMKS(OPT, LINK, YES)          |  // Link Parameter nutzen?
    EDMA_FMKS(OPT, FS, NO),               // Frame Sync nutzen?

    EDMA_FMKS(SRC, SRC, OF(0)),           // Quell-Adresse

    EDMA_FMK (CNT, FRMCNT, 0)          |  // Anzahl Frames
    EDMA_FMK (CNT, ELECNT, RESPONSE_LEN),   // Anzahl Elemente

    (Uint32)Buffer_in,       		  // Ziel-Adresse

    EDMA_FMKS(IDX, FRMIDX, DEFAULT)    |  // Frame index Wert
    EDMA_FMKS(IDX, ELEIDX, DEFAULT),      // Element index Wert

    EDMA_FMK (RLD, ELERLD, 0)       |  // Reload Element
    EDMA_FMK (RLD, LINK, 0)            // Reload Link
};

EDMA_Config configEDMAXmt = {
    EDMA_FMKS(OPT, PRI, LOW)          |  // auf beide Queues verteilen
    EDMA_FMKS(OPT, ESIZE, 16BIT)       |  // Element size
    EDMA_FMKS(OPT, 2DS, NO)            |  // kein 2D-Transfer
    EDMA_FMKS(OPT, SUM, NONE)          |  // Quell-update mode -> FEST (McBSP)!!!
    EDMA_FMKS(OPT, 2DD, NO)            |  // 2kein 2D-Transfer
    EDMA_FMKS(OPT, DUM, INC)           |  // Ziel-update mode
    EDMA_FMKS(OPT, TCINT,YES)         |  // EDMA interrupt erzeugen?
    EDMA_FMKS(OPT, TCC, OF(0))         |  // Transfer complete code (TCC)
    EDMA_FMKS(OPT, LINK, YES)          |  // Link Parameter nutzen?
    EDMA_FMKS(OPT, FS, NO),               // Frame Sync nutzen?

    (Uint32)Buffer_out,           // Quell-Adresse

    EDMA_FMK (CNT, FRMCNT, 0)          | // Anzahl Frames
    EDMA_FMK (CNT, ELECNT, SWEEP_LEN),   // Anzahl Elemente

    EDMA_FMKS(DST, DST, OF(0)),       		  // Ziel-Adresse

    EDMA_FMKS(IDX, FRMIDX, DEFAULT)    |  // Frame index Wert
    EDMA_FMKS(IDX, ELEIDX, DEFAULT),      // Element index Wert

    EDMA_FMK (RLD, ELERLD, 0)       |  // Reload Element
    EDMA_FMK (RLD, LINK, 0)            // Reload Link
};

/* Transfer-Complete-Codes for EDMA-Jobs */
int tccRcv;
int tccXmt;

/* EDMA-Handles */
EDMA_Handle hEdmaRcv;
EDMA_Handle hEdmaXmt;
// no ping pong => no reload handle needed
						
MCBSP_Handle hMcbsp;


void config_EDMA(void)
{
	/*############ RECIEVE #############*/
			  /* ADC => CPU */

	hEdmaRcv = EDMA_open(EDMA_CHA_REVT1, EDMA_OPEN_RESET);  // EDMA Channel for REVT1

	configEDMARcv.src = MCBSP_getRcvAddr(hMcbsp);          //  source addr

	tccRcv = EDMA_intAlloc(-1);                        // next available TCC
	configEDMARcv.opt |= EDMA_FMK(OPT,TCC,tccRcv);     // set it

	/* configure */
	EDMA_config(hEdmaRcv, &configEDMARcv);



	/*############ TRANSMIT #############*/
			   /* CPU => DAC */

	hEdmaXmt = EDMA_open(EDMA_CHA_XEVT1, EDMA_OPEN_RESET);  // EDMA Channel for XEVT1

	configEDMAXmt.dst = MCBSP_getXmtAddr(hMcbsp);		 // destination addr

	tccXmt = EDMA_intAlloc(-1);                        // next available TCC
	configEDMAXmt.opt |= EDMA_FMK(OPT,TCC,tccXmt);     // set it

	/* configure */
	EDMA_config(hEdmaXmt, &configEDMAXmt);


	/* enable EDMA TCC */
	EDMA_intClear(tccRcv);
	EDMA_intEnable(tccRcv);

	EDMA_intClear(tccXmt);
	EDMA_intEnable(tccXmt);

	/* enable EDMA channels */
	EDMA_enableChannel(hEdmaRcv);
	EDMA_enableChannel(hEdmaXmt);
}

void config_interrupts(void)
{
	/*######### HWI ##########*/

	/*--------- EDMA ----------*/
	IRQ_map(IRQ_EVT_EDMAINT, 8);
	IRQ_clear(IRQ_EVT_EDMAINT);
	IRQ_enable(IRQ_EVT_EDMAINT);


	IRQ_globalEnable();
}

/*######## INITIALISATION FUNCTION ########*/

void frequency_sweep_init(void)			// Initialisation of then sent signal
{
	/*------- Frequency Sweep Algorithm -------*/
	//For explanations see the file "freq_sweep.html"
	short k;
	float factor;
	float omega;
	float value[3];
	float buf;
	value[0]=1;
	value[1]=2*cos(2*PI*1006.25/48000)*sin(2*PI*1006.25/48000);
	for(k=0;k<SWEEP_LEN;k++)
	{
		if(k>1)
		{
			omega = 2*PI*(1000+k*6.25)/48000;
			factor = sin(omega)/sqrt(value[1]*value[1]+value[0]*value[0]-2*value[0]*value[1]*cos(omega));
			value[2] = factor*(2*cos(omega)*value[1]-value[0]);
			buf=25000*value[2];
			Buffer_out[k]=(short)buf;
			value[0] = value[1];
			value[1] = value[2];
		}
		else
		{
			buf=25000*value[k];
			Buffer_out[k]=(short)buf;
		}
	}
}

/*######### CALCULATION FUNCTIONS #########*/

float convert_step_distance(short step) // array index => distance
{
	float distance;
	distance = (float)step*340/96000;   // distance = step x speed of sound / (2 x sampling freq)
	return distance;
}


short cross_correlation_time()  		// Cross correlation algorithm (time domain), double for-loop
{
	short k,m,r_new,r_max,i;
	short max_index = 0;

	// Bring the freq sweep signal to the same length as the response signal
	for(i=0;i<SWEEP_LEN;i++)
	{
		sweep_time[i]=Buffer_out[i];	// Rest of the array initialized with zeros
	}

	r_max=0; 							// to compare

	for(k=-4319;k<4320;k++)				// For explanations see "cross_correlation.html"
	{
		r_new=0;

		/*--------- Cross correlation ----------*/
		for(m=0;m<4320;m++)
		{
			if( ((m-k)<4320) && ((m-k)>=0) ) // Stay in the limits of the array
			{
				r_new = r_new + sweep_time[m]*Buffer_in[m-k];  // Cross correlation formula
			}
		}

		/*-------- Keeping the maximum ---------*/
		if (r_new > r_max)
		{
			r_max = r_new;
			max_index=k;
		}
	}

	return max_index;     // Returns the array index of the maximum value of the cross correlation
}


void tw_genr2fft(float* w, int n)          //generates the coefficient table (twiddle factors) for the fft
{
   int i;
   float pi = 4.0*atan(1.0);
   float e = pi*2.0/n;

    for(i=0; i < ( n>>1 ); i++)
    {
       w[2*i]   = cos(i*e);
       w[2*i+1] = sin(i*e);
    }
}


void bit_rev(float* x, int n)             //bit reverse a vector for the fft
{
  int i, j, k;
  float rtemp, itemp;

  j = 0;
  for(i=1; i < (n-1); i++)
  {
     k = n >> 1;
     while(k <= j)
     {
        j -= k;
        k >>= 1;
     }
     j += k;
     if(i < j)
     {
        rtemp    = x[j*2];
        x[j*2]   = x[i*2];
        x[i*2]   = rtemp;
        itemp    = x[j*2+1];
        x[j*2+1] = x[i*2+1];
        x[i*2+1] = itemp;
     }
   }
}


short cross_correlation_frequency()		// Uses the FFT dit + IFFT dif algorithm (Radix 2)	from the DSPlib
{										// max = max[ IFFT( FFT(sweep) x FFT(response) ) ]
										// FFT function needs complex array at input
	short max_index,i,j,k;
	float max_value;


	/*--------- Formating ----------*/
	/* Turn the freq sweep and response arrays into complex arrays ( array[re(a1),im(a1),re(a2),im(a2),...], im = 0 )
	 and bring them to the right length (next power of 2 = 16384)*/

	for(i=0 ; i < RESPONSE_LEN ; i++)						// sweep_freq and response_freq already right length
	{														// and filled with zeros (see initialisation)
		if(i < SWEEP_LEN)
		{
			sweep_freq[2*i] = (float)Buffer_out[i];			// fills every second value (real parts)
		}

		response_freq[2*i] = (float)Buffer_out[i];
	}


	/*------------ FFT -------------*/
	/* DSPF_sp_cfftr2_dit(float* x, float* w, short n)*/
	short N;
	N = FFT_LEN/2 ;           								//length of FFT in complex samples
	tw_genr2fft(fft_coeff, N);	   							//generates coefficient table for fft
	bit_rev(fft_coeff,N>>1);   								//bit reverse the vector (right format for fft dit)

	DSPF_sp_cfftr2_dit(sweep_freq,fft_coeff,N);				//FFT of sweep signal
	DSPF_sp_cfftr2_dit(response_freq,fft_coeff,N);			//FFT of response signal
															//both signal are bit reversed (complex)


	/*---------- Multiply ----------*/

	for(j=0;j<FFT_LEN;j++)									// floating point so no need to worry
	{														// about numbers oustide of the type limits.
		cross_corr_freq[j]=sweep_freq[j]*response_freq[j];	// Result is cross correlation in frequency domain
	}


	/*------------ IFFT ------------*/
	/*DSPF_sp_icfftr2_dif (float* x, float* w, short n)*/
	DSPF_sp_icfftr2_dif(cross_corr_freq,fft_coeff,N);		//Input bit reversed, output normal (complex)


	/*---- Finding the maximum ----*/

	max_value=0;
	max_index=0;
	for(k=0;k<RESPONSE_LEN;k++)
	{
		if (cross_corr_freq[2*k] > max_value)				//Take only the real values (im = 0)
		{													// = every second value
			max_value = cross_corr_freq[2*k];				//so k has the right format
			max_index = k;
		}
	}


	// return the index of the maximum value
	return max_index;
}



/*############### MAIN ###############*/
main()
{


	CSL_init();

	/* Configure McBSP0 and AIC23 */
	Config_DSK6713_AIC23();

	/* Configure McBSP1*/
	hMcbsp = MCBSP_open(MCBSP_DEV1, MCBSP_OPEN_RESET);
    MCBSP_config(hMcbsp, &datainterface_config);

    /* Initialize the frequency sweep signal */
    frequency_sweep_init();

	/* configure EDMA */
    config_EDMA();

    /* finally the interrupts */
    config_interrupts();

    MCBSP_start(hMcbsp, MCBSP_RCV_START | MCBSP_XMIT_START, 0xffffffff);
    MCBSP_write(hMcbsp, 0x0); 	/* one shot */

} /* finished*/


/*######### INTERRUPT FUNCTIONS #########*/

void EDMA_interrupt_service(void)
{
	static int rcvDone=0;
	static int xmtDone=0;
	
	if(EDMA_intTest(tccRcv)) {
		EDMA_intClear(tccRcv); /* clear is mandatory */
		EDMA_disableChannel(hEdmaRcv); // Disbale the Channel for processing (offline calculation)
		rcvDone=1;
	}
	else if(EDMA_intTest(tccXmt)) {
		EDMA_intClear(tccXmt);
		EDMA_disableChannel(hEdmaXmt);  // Disbale the Channel for processing (offline calculation)
		xmtDone=1;
	}
	
	if(rcvDone && xmtDone) {
		rcvDone=0;
		xmtDone=0;
		// processing in SWI
		SWI_post(&SWI_process);
	}
}

void process_SWI(void)
{
	float dist;
	short max_index;

	/* ########### Calculation ############ */
#ifdef SWITCH

	/*---------- Frequency domain ----------*/
	max_index = cross_correlation_frequency();

#else

	/*------------ Time domain -------------*/
	max_index = cross_correlation_time();

#endif /* SWITCH */

	dist = convert_step_distance(max_index);

	// Enable the channels again after processing
	EDMA_enableChannel(hEdmaRcv);
	EDMA_enableChannel(hEdmaXmt);
}


/*######### LED FUNCTIONS #########*/

void SWI_LEDToggle(void)
{
	SEM_postBinary(&SEM_LEDToggle);	
}

void tsk_led_toggle(void)
{
	/* initializatoin of the task */
	/* nothing to do */
	
	/* process */
	while(1) {
		SEM_pendBinary(&SEM_LEDToggle, SYS_FOREVER);
		
		DSK6713_LED_toggle(1);
	}
}
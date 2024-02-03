/*****************************************************************************
 * audio_efekti.c
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "test.h"

#include <math.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <cycle_count.h>
#include <builtins.h>

#include "test_audio.h"
#include "iir_peak_a.h"
#include "iir_peak_b.h"
#include "iir_notch_a.h"
#include "iir_notch_b.h"
//#define ISPIS_U_FAJL


// Index for memory allocation
int index = 0;

// Reserve 512kB of SDRAM/SRAMs memory for signals
#pragma section("seg_sram")
char heap_mem[512000];

/**
 * If you want to use command program arguments, then place them in the following string.
 */
char __argv_string[] = "";

cycle_t start_count;
cycle_t final_count;



/* Funkcija koja vrsi obradu ulaznog signala u skladu sa audio efektom kasnjenja (delay) */
void delay(float *x, float *y, int M, float g)
{
	float *x_delay = (float *)heap_calloc(index, LEN, sizeof(float));
	int k = 0;
	int i;

//#pragma SIMD_for
	for(i = M; i < LEN; i++)
	{
		x_delay[i] = x[k];
		k++;

	}

//#pragma SIMD_for
	for(i = 0; i < LEN; i++)
	{
		y[i] = x[i] + g * x_delay[i];
	}

	heap_free(index, x_delay);
}


/* Funkcija koja vrsi obradu u skladu sa audio efektom distorzije (distortion) */
void distortion(float* x, float* y, float gain, float mix)
{
	int i;

	float max = 0.0;

	for(i = 0; i < LEN; i++)
	{
		if((fabsf(x[i]) - max) > 0)
		{
			max = fabsf(x[i]);
		}
	}
//#pragma SIMD_for
	for(i = 0; i < LEN; i++)
	{
		x[i] = x[i] / max;
	}


//#pragma SIMD_for
	for(i = 0; i < LEN; i++)
	{
		float x_original = x[i];
		float x_value = x_original * gain;
		float abs_x = fabsf(x_value);
		float exp_x = expf(-1 * ((x_value * x_value) / abs_x));


		y[i] = (1 - mix) * x_original + mix * ((x_value / abs_x) * (1 - exp_x));

	}


	max = 0.0;
	for(i = 0; i < LEN; i++)
	{
		if((fabsf(y[i]) - max) > 0)
		{
			max = fabsf(y[i]);
		}
	}

//#pragma SIMD_for
	for(i = 0; i < LEN; i++)
	{
		y[i] = y[i] / max;
	}
}


/* Funkcija koja vrsi obradu u skladu sa audio efektom wah-wah */
void wah_wah(float* x, float* y, float minf, float maxf, float fw)
{
	int i,j;

	/* Prvi dio algoritma. Kreiranje trougaonog signala sa centralnim frekvencijam filtara */
	float delta = fw / FS;

	float* freq_carrier = NULL;
	freq_carrier = (float* )heap_malloc(index, LEN*sizeof(float));
	if(freq_carrier == NULL)
	{
		printf("Memory allocation failed (freq_carrier)\n");
		return;
	}

	int no_iterations = (int)((maxf - minf) / delta) + 1;

	freq_carrier[0] = minf;
//#pragma SIMD_for
	for(i = 1; i < no_iterations; i++)
	{
		freq_carrier[i] = freq_carrier[i - 1] + delta;
	}

//#pragma SIMD_for
	while(i < LEN)
	{
//#pragma SIMD_for
		for(j = (no_iterations - 1); j >= 0; j--)
		{
			freq_carrier[i] = freq_carrier[j];
			i++;
		}
//#pragma SIMD_for
		for(j = 0; j < no_iterations - 1; j++)
		{
			freq_carrier[i] = freq_carrier[j];
			i++;
		}
	}

	/* Drugi dio algoritma
	 * Zapravo, glavni dio, gdje se računaju izlazni odmjerci,
	 * na osnovu koeficijenata filtra iz fajlova iir_peak_a(b).h
	*/
	float a[3];
	float b[3];
	a[0] = 1;
	b[1] = 0;
	for(i = 2; i < LEN; i++)
	{
		a[1] = peak_a_coeff[(i-2)*3 + 1];
		a[2] = peak_a_coeff[(i-2)*3 + 2];

		b[0] = peak_b_coeff[(i-2)*3];
		b[2] = peak_b_coeff[(i-2)*3 + 2];

		y[i] = b[0]*x[i] + b[1]*x[i-1] + b[2]*x[i-2] - a[1]*y[i-1] - a[2]*y[i-2];

	}

	heap_free(index, freq_carrier);
}

/* Funkcija koja vrsi obradu skladu sa phaser efektom */
void phaser(float* x, float* y, float minf, float maxf, float fw)
{
	int i,j;
	float delta = fw / FS;

	float* y_first = NULL;
	y_first = (float* )heap_malloc(index, LEN*sizeof(float));
	if(y_first == NULL)
	{
		printf("Error\n");
		return;
	}

	float* freq_carrier = NULL;
	freq_carrier = (float* )heap_malloc(index, LEN*sizeof(float));
	if(freq_carrier == NULL)
	{
		printf("Error\n");
		return;
	}

	int no_iterations = (int)((maxf - minf) / delta) + 1;

	freq_carrier[0] = minf;
//#pragma SIMD_for
	for(i = 1; i < no_iterations; i++)
	{
		freq_carrier[i] = freq_carrier[i - 1] + delta;
	}

//#pragma SIMD_for
	while(i < LEN)
	{
//#pragma SIMD_for
		for(int j = (no_iterations - 1); j >= 0; j--)
		{
			freq_carrier[i] = freq_carrier[j];
			i++;
		}
//#pragma SIMD_for
		for(int j = 0; j < no_iterations - 1; j++)
		{
			freq_carrier[i] = freq_carrier[j];
			i++;
		}
	}

	/* Drugi dio algoritma. */
	float a[3];
	float b[3];
	a[0] = 1;
	for(int i = 2; i < LEN; i++)
	{
		a[0] = notch_a_coeff[(i-2)*3 + 0];
		a[1] = notch_a_coeff[(i-2)*3 + 1];
		a[2] = notch_a_coeff[(i-2)*3 + 2];

		b[0] = notch_b_coeff[(i-2)*3 + 0];
		b[1] = notch_b_coeff[(i-2)*3 + 1];
		b[2] = notch_b_coeff[(i-2)*3 + 2];

		y_first[i] = b[0]*x[i] + b[1]*x[i-1] + b[2]*x[i-2] - a[1]*y_first[i-1] - a[2]*y_first[i-2];
		y[i] = b[0]*y_first[i] + b[1]*y_first[i-1] + b[2]*y_first[i-2] - a[1]*y[i-1] - a[2]*y[i-2];
	}


	heap_free(index, y_first);
	heap_free(index, freq_carrier);
}

/* Funkcija koja vrsi obradu signal u skladu se efektom reverberacija */
void reverb(float* x, float* y, float alpha, int N, int R)
{
//#pragma SIMD_for
	for(int i = 0; i < LEN; i++)
	{
		y[i] = x[i];
		if(i > N*R)
		{
//#pragma SIMD_for
			for(int k = 1; k < N; k++)
			{
				y[i] += pow(alpha, k) * x[i - k*R];
			}
		} else
		{
//#pragma SIMD_for
			for(int k = 1; k < N; k++)
			{
				if(i > k*R)
				{
					y[i] += pow(alpha, k) * x[i - k*R];
				}
			}
		}
	}

	float max = 0.0;
	for(int i = 0; i < LEN; i++)
	{
		if(fabsf(y[i]) > max)
		{
			max = fabsf(y[i]);
		}
	}
//#pragma SIMD_for
	for(int i = 0; i < LEN; i++)
	{
		y[i] = y[i] / max;
	}
}

int main(int argc, char *argv[])
{

	adi_initComponents();

	int uid = 999;  		/* proizvoljni user id za instaliranje heap-a */
	float *audio_input = 0;
	float *audio_output = 0;

	/* Instaliranje hipa na rezervisanoj memoriji, za ulaz, izlaz i međurezultate */
	index = heap_install(heap_mem, sizeof(heap_mem), uid);
	if(index < 0)
	{
		printf("Heap installation failed\n");
		return -1;
	}

	/* Alociranje ulaznog bafera za smještanje LEN odmjeraka testnog signala */
	audio_input = (float*) heap_malloc(index, LEN*sizeof(float));
	if(audio_input == NULL)
	{
		printf("Memory allocation failed (input buffer)\n");
		return -1;
	}

	/* Alociranje izlaznog bafera za smještanje obrađenih odmjeraka */
	audio_output = (float *) heap_calloc(index, LEN, sizeof(float));
	if(audio_output == NULL)
	{
		printf("Memory allocation failed (output buffer)\n");
		return -1;
	}

	/* Popunjavanje ulaznog bafera testnim odmjercima iz fajla test_audio.h */
	for(int i = 0; i < LEN; i++)
	{
		audio_input[i] = test_signal[i];
	}

	/* Slijede podrazumijevani parametri koji će biti korišteni za efekte
	 * Imenovanje parametara je kao u pajton implementaciji,
	 * a koje prati oznake u teorijskim objašnjenjima efekata
	 */

	/* Podrazumijevani parametri za delay efekat */
	int M = 1102;
	float g = 0.5;

	/* Podrazumijevani parametri za distortion efekat */
	float gain = 20.0;
	float mix = 0.8;

	/* Podrazumijevani parametri za wah wah efekat */
	float minf = 500.0;
	float maxf = 3000.0;
	float fw = 2000.0;

	/* Podrazumijevani parametri za phaser efekat
	** identični su parametrima wah wah efekta
	*/

	/* Podrazumijevani efekti za reverb efekat */
	float alpha = 0.8;
	int N = 10;
	int R = 8;

	int select_effect = 1;

	switch(select_effect)
	{
		case 1:
			printf("Delay efekat\n");


			START_CYCLE_COUNT(start_count);

			delay(audio_input, audio_output, M, g);

			STOP_CYCLE_COUNT(final_count, start_count);
			PRINT_CYCLES("Broj ciklusa: \n", final_count);

			printf("%f %f %f\n", audio_output[100], audio_output[10000], audio_output[30000]);

			break;
		case 2:
			printf("Distortion efekat\n");

			START_CYCLE_COUNT(start_count);

			distortion(audio_input, audio_output, gain, mix);

			STOP_CYCLE_COUNT(final_count, start_count);
			PRINT_CYCLES("Broj ciklusa: \n", final_count);

			printf("%f %f %f\n", audio_output[100], audio_output[10000], audio_output[30000]);
			break;
		case 3:

			START_CYCLE_COUNT(start_count);

			wah_wah(audio_input, audio_output, minf, maxf, fw);

			STOP_CYCLE_COUNT(final_count, start_count);
			PRINT_CYCLES("Broj ciklusa: \n", final_count);

			printf("%f %f %f\n", audio_output[100], audio_output[10000], audio_output[30000]);
			break;
		case 4:
			printf("Phaser\n");
			START_CYCLE_COUNT(start_count);

			phaser(audio_input, audio_output, minf, maxf, fw);

			STOP_CYCLE_COUNT(final_count, start_count);
			PRINT_CYCLES("Broj ciklusa: \n", final_count);

			printf("%f %f %f\n", audio_output[100], audio_output[10000], audio_output[30000]);
			break;
		case 5:
			START_CYCLE_COUNT(start_count);

			reverb(audio_input, audio_output, alpha, N, R);

			STOP_CYCLE_COUNT(final_count, start_count);
			PRINT_CYCLES("Broj ciklusa: \n", final_count);

			printf("%f %f %f\n", audio_output[100], audio_output[10000], audio_output[30000]);
			break;
		default:
			printf("Nije izabran efekata: \n");
	}

	heap_free(index, audio_input);
	heap_free(index, audio_output);

	return 0;
}


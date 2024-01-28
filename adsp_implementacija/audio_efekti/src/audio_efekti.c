/*****************************************************************************
 * audio_efekti.c
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <filter.h>
#include <math.h>
#include "test_audio.h"
#include "iir_peak_a.h"
#include "iir_peak_b.h"
#include "iir_notch_a.h"
#include "iir_notch_b.h"


// Index for memory allocation
int index = 0;

// Reserve 150 of SDRAM memory for signals
#pragma section("seg_sdram")
static char heap_mem[1000000];

/** 
 * If you want to use command program arguments, then place them in the following string. 
 */
char __argv_string[] = "";


/* Funkcija koja vrsi obradu ulaznog signala u skladu sa audio efektom kasnjenja (delay) */
void delay(float *x, float *y, int M, float g)
{
	float *x_delay = (float *)heap_calloc(index, LEN, sizeof(float));
	int k = 0;

	int i;

	for(i = 0; i < LEN; i++)
	{
		if(i > M - 1)
		{
			x_delay[i] = x[k];
			k++;
		}
	}


	for(i = 0; i < LEN; i++)
	{
		y[i] += x[i] + g * x_delay[i];
	}
	heap_free(index, x_delay);
}

/* Funkcija koja vrsi obradu u skladu sa audio efektom distorzije (distortion) */
void distortion(float* x, float* y, float gain, float mix)
{
	float* x_distort = (float* )heap_calloc(index, LEN, sizeof(float));
	float* x_gain = (float* )heap_calloc(index, LEN, sizeof(float));

	float max = 0.0;

	printf("Prije prve for petlje\n");
	for(int i = 0; i < LEN; i++)
	{
		if(fabsf(x[i]) > max)
		{
			max = fabsf(x[i]);
		}
	}

	printf("Prije druge for petlje\n");
	for(int i = 0; i < LEN; i++)
	{
		x[i] = x[i] / max;
	}

	printf("Prije 2 i po petlje\n");
	for(int i = 0; i < LEN; i++)
	{
		x_gain[i] = gain * x[i];
	}

	printf("Prije trece for petlje\n");
	for(int i = 0; i < LEN; i++)
	{
		float x_original = x[i];
		float x_value = x_gain[i];
		float abs_x = fabsf(x_value);
		float exp_x = expf(-1 * ((x_value * x_value) / abs_x));

		x_distort[i] = (x_value / abs_x) *(1 - exp_x);
		y[i] = (1 - mix) * x_original + mix * x_distort[i];

	}


	printf("Prije cetvrte for petlje\n");
	max = 0.0;
	for(int i = 0; i < LEN; i++)
	{
		if(fabsf(y[i]) > max)
		{
			max = fabsf(y[i]);
		}
	}

	printf("Prije pete for petlje\n");
	for(int i = 0; i < LEN; i++)
	{
		y[i] = y[i] / max;
	}

	heap_free(index, x_distort);
}

/* Funkcija koja vrsi obradu u skladu sa audio efektom wah-wah */
void wah_wah(float* x, float* y)
{
	/* Prvi dio algoritma. Kreiranje trougaonog signala nosioca frekvencija */
	float minf = 500.0;
	float maxf = 3000.0;
	float fw = 2000.0;
	float delta = fw / FS;
	float* freq_carrier = NULL;
	freq_carrier = (float* )heap_malloc(index, LEN*sizeof(float));
	if(freq_carrier == NULL)
	{
		printf("Error\n");
		return;
	}

	printf("Prvi\n");

	int no_iterations = (int)((maxf - minf) / delta) + 1;
	int i = 1;

	freq_carrier[0] = minf;
	for(i = 1; i < no_iterations; i++)
	{
		freq_carrier[i] = freq_carrier[i - 1] + delta;
	}

	printf("Drugi\n");
	while(i < LEN)
	{
		for(int j = (no_iterations - 1); j >= 0; j--)
		{
			freq_carrier[i] = freq_carrier[j];
			i++;
		}

		for(int j = 0; j < no_iterations - 1; j++)
		{
			freq_carrier[i] = freq_carrier[j];
			i++;
		}
	}

	printf("Treci\n");

	/* Drugi dio algoritma. */
	float a[3];
	float b[3];
	for(int i = 2; i < LEN; i++)
	{
		for(int j = 0; j < 3; j++){
			a[j] = peak_a_coeff[(i-2)*3 + j];
			b[j] = peak_b_coeff[(i-2)*3 + j];
		}
		y[i] = b[0]*x[i] + b[1]*x[i-1] + b[2]*x[i-2] - a[1]*y[i-1] - a[2]*y[i-2];
	}


	heap_free(index, freq_carrier);
}

/* Funkcija koja vrsi obradu skladu sa phaser efektom */
void phaser(float* x, float* y)
{
	float minf = 500.0;
	float maxf = 3000.0;
	float fw = 2000.0;
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

	printf("Prvi\n");

	int no_iterations = (int)((maxf - minf) / delta) + 1;
	int i = 1;

	freq_carrier[0] = minf;
	for(i = 1; i < no_iterations; i++)
	{
		freq_carrier[i] = freq_carrier[i - 1] + delta;
	}

	printf("Drugi\n");
	while(i < LEN)
	{
		for(int j = (no_iterations - 1); j >= 0; j--)
		{
			freq_carrier[i] = freq_carrier[j];
			i++;
		}

		for(int j = 0; j < no_iterations - 1; j++)
		{
			freq_carrier[i] = freq_carrier[j];
			i++;
		}
	}

	printf("Treci\n");

	/* Drugi dio algoritma. */
	float a[3];
	float b[3];
	for(int i = 2; i < LEN; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			a[j] = notch_a_coeff[(i-2)*3 + j];
			b[j] = notch_b_coeff[(i-2)*3 + j];
		}
		y_first[i] = b[0]*x[i] + b[1]*x[i-1] + b[2]*x[i-2] - a[1]*y_first[i-1] - a[2]*y_first[i-2];
	}


	for(int i = 2; i < LEN; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			a[j] = notch_a_coeff[(i-2)*3 + j];
			b[j] = notch_b_coeff[(i-2)*3 + j];
		}
		y[i] = b[0]*y_first[i] + b[1]*y_first[i-1] + b[2]*y_first[i-2] - a[1]*y[i-1] - a[2]*y[i-2];

	}
	heap_free(index, freq_carrier);

}

/* Funkcija koja vrsi obradu signal u skladu se efektom reverberacija */
void reverb(float* x, float* y, float alpha, int N, int R)
{
	for(int i = 0; i < LEN; i++)
	{
		for(int k = 0; k < N; k++)
		{
			y[i] += pow(alpha, k) * ((i > k*R) ? x[i - k*R] : 0);
		}
	}


	/* Normalizacija izlaznog niza
	 * !! Mozda i nije potrebna
	 */
	float max = 0.0;
	for(int i = 0; i < LEN; i++)
	{
		if(fabsf(y[i]) > max)
		{
			max = fabsf(y[i]);
		}
	}

	printf("Prije pete for petlje\n");
	for(int i = 0; i < LEN; i++)
	{
		y[i] = y[i] / max;
	}



}


int main(int argc, char *argv[])
{

	adi_initComponents();

	int uid = 999;  /* arbitrary userid for heap */
	float *audio_input = 0;
	float *audio_output = 0;

	/* Install extra heap as heap */
	index = heap_install(heap_mem, sizeof(heap_mem), uid);
	if(index < 0)
	{
		printf("Heap installation failed\n");
		return -1;
	}

	/* Allocate memory input buffer to store LEN samples of test audio signal */
	audio_input = (float*) heap_malloc(index, LEN*sizeof(float));
	if(audio_input == NULL)
	{
		printf("Memory allocation failed (input buffer)\n");
		return -1;
	}
	printf("Alociran input buffer\n");

	/* Allocate memory for output buffer to store processed samples */
	audio_output = (float *) heap_calloc(index, LEN, sizeof(float));
	if(audio_output == NULL)
	{
		printf("Memory allocation failed (output buffer)\n");
		return -1;
	}
	printf("Alociran output buffer\n");

	/* Fill the buffer with test audio signal samples */
	for(int i = 0; i < LEN; i++)
	{
		audio_input[i] = test_signal[i];
	}

	/* Delay */
	//delay(audio_input, audio_output, 1102, 0.5);

	/* Distortion */
	//distortion(audio_input, audio_output, 20, 0.8);

	/* Wah-Wah */
	//wah_wah(audio_input, audio_output);


	/* Phaser */
	//phaser(audio_input, audio_output);

	/* Reverb */
	//reverb(audio_input, audio_output, 0.8, 10, 8);

	/* Ispis rezultata obrade u fajl */
//	FILE *fp = fopen("../output_files/delay_optimize.txt", "w");
//	if(fp == NULL)
//	{
//		printf("File open failed/n");
//		return -1;
//	}
//	printf("Otvoren fajl\n");
//	for(int i = 0; i < LEN; i++)
//	{
//		fprintf(fp, "%f\n", audio_output[i]);
//	}
//	fclose(fp);
//	printf("Zatvoren fajl\n");
//
	heap_free(index, audio_input);
	heap_free(index, audio_output);


	return 0;
}


/*****************************************************************************
 * audio_efekti.c
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "audio_efekti.h"
#include <stdio.h>
#include <stdlib.h>
#include "test_audio.h"

// Index for memory allocation
int index = 0;

// Reserve 150 of SDRAM memory for signals
#pragma section("seg_sdram")
static char heap_mem[512000];

/** 
 * If you want to use command program arguments, then place them in the following string. 
 */
char __argv_string[] = "";


void delay(float *x, float *y, int M, float g)
{
	printf("U funkciji delay\n");
	float *x_delay = (float *)heap_calloc(index, LEN, sizeof(float));
	int k = 0;

	printf("Prije prve for petlje\n");
	for(int i = 0; i < LEN; i++)
	{
		if(i > M - 1)
		{
			x_delay[i] = x[k];
			k++;
		}
	}

	printf("Prije druge for petlje\n");
	for(int i = 0; i < LEN; i++)
	{
		y[i] += x[i] + g * x_delay[i];
	}
	heap_free(index, x_delay);
}



int main(int argc, char *argv[])
{
	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();
	
	printf("%d", sizeof(float));

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

	printf("Prije delay funkcije\n");

	/* Delay */
	delay(audio_input, audio_output, 1102, 0.5);

	printf("Nakon delay funkcije\n");



	FILE *fp = fopen("../output_files/delay.txt", "w");
	if(fp == NULL)
	{
		printf("File open failed/n");
		return -1;
	}
	printf("Otvoren fajl\n");
	for(int i = 0; i < LEN; i++)
	{
		fprintf(fp, "%f\n", audio_output[i]);
	}
	fclose(fp);
	printf("Zatvoren fajl\n");

	heap_free(index, audio_input);
	heap_free(index, audio_output);

	return 0;
}


/*****************************************************************************
 * audio_efekti.c
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include "audio_efekti.h"

#include <math.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <cycle_count.h>
#include <string.h>
#include <builtins.h>
#include <SYSREG.h>

#include "test_audio.h"
#include "iir_peak_a.h"
#include "iir_peak_b.h"
#include "iir_notch_a.h"
#include "iir_notch_b.h"


#define COUNT_CYCLES
//#define LOOP_VECTORIZATION

// Index for memory allocation
int index = 0;

// Reserve 512kB of SDRAM/SRAMs memory for signals
#pragma section("seg_sram")
char heap_mem[512000];

/* Variables from <cycle_count.h> for measuring cycles */
cycle_t start_count;
cycle_t final_count;



/*
 * @brief Function that processes input signal in a manner of delay effect
 *
 * @param x Input array of floats with samples of signal to be processed
 * @param y Output array of floats with processed samples
 * @param M Number of samples signal is going to be delayed
 * @param g Gain factor that determines how much delayed signal is involved into output signal
*/
void delay(float *x, float *y, int M, float g)
{
	/* Allocating temporary array for some interim results */
	float* x_delay = NULL;
	x_delay = (float *)heap_calloc(index, LEN, sizeof(float));
	if(x_delay == NULL)
	{
		printf("Memory allocation error - delay effect\n");
	}
	int k = 0;
	int i;

	/* Calculating delayed signal, based on parameter M */
	//#pragma SIMD_for
	for(i = M; i < LEN; i++)
	{
		x_delay[i] = x[k];
		k++;

	}

	/* Calcalating output array using input and delayed array */
	//#pragma SIMD_for
	for(i = 0; i < LEN; i++)
	{
		y[i] = x[i] + g * x_delay[i];
	}

	heap_free(index, x_delay);
}


/*
 * @brief Function that processes input signal in a manner of distortion effect
 *
 * @param x    Input array of floats with samples of signal to be processed
 * @param y	   Output array of floats with processed samples
 * @param gain Gain factor, how much input signal is going to be amplified
 * @param mix  Value that determines how much of the original signal, and how much of the distorted signal is involde in output
*/
void distortion(float* x, float* y, float gain, float mix)
{
	int i;
	float max_v = 0.0;

	/* Finding the absoulute maximum value of input */
	for(i = 0; i < LEN; i++)
	{
		if((fabsf(x[i]) - max_v) > 0)
		{
			max_v = fabsf(x[i]);
		}
	}

	/* Normalizing input samples with maximum absolute value */
	//#pragma SIMD_for
	for(i = 0; i < LEN; i++)
	{
		x[i] = x[i] / max_v;
	}


	/* Calulating the output based on distortion equation */
	//#pragma SIMD_for
	for(i = 0; i < LEN; i++)
	{
		float x_original = x[i];
		float x_value = x_original * gain;
		float abs_x = fabsf(x_value);
		float exp_x = expf(-1 * ((x_value * x_value) / abs_x));


		y[i] = (1 - mix) * x_original + mix * ((x_value / abs_x) * (1 - exp_x));

	}

	/* Finding the absoulute maximum value of the output */
	max_v = 0.0;
	for(i = 0; i < LEN; i++)
	{
		if((fabsf(y[i]) - max_v) > 0)
		{
			max_v = fabsf(y[i]);
		}
	}

	/* Normalizing output samples with maximum absolute value */
	//#pragma SIMD_for
	for(i = 0; i < LEN; i++)
	{
		y[i] = y[i] / max_v;
	}
}



/*
 * @brief Function that processes input signal in a manner of wah-wah effect
 *
 * @param x    Input array of floats with samples of signal to be processed
 * @param y	   Output array of floats with processed samples
 */
void wah_wah(float* x, float* y)
{
	int i,j;

	/* Two arrays a, b for coefficient of the peak filter  */
	float a[3];
	float b[3];

	/* Values a[0] and b[1] are always the same,
	 * so they are calculated outside of loop
	 */
	a[0] = 1;
	b[1] = 0;

	/* For loop that calculates output, using IIR coefficients and input samples*/
	for(i = 2; i < LEN; i++)
	{
		a[1] = peak_a_coeff[(i-2)*3 + 1];
		a[2] = peak_a_coeff[(i-2)*3 + 2];

		b[0] = peak_b_coeff[(i-2)*3];
		b[2] = peak_b_coeff[(i-2)*3 + 2];

		y[i] = b[0]*x[i] + b[1]*x[i-1] + b[2]*x[i-2] - a[1]*y[i-1] - a[2]*y[i-2];

	}
}

/*
 * @brief Function that processes input signal in a manner of distortion effect
 *
 * @param x    Input array of floats with samples of signal to be processed
 * @param y	   Output array of floats with processed samples
 */
void phaser(float* x, float* y)
{
	int i,j;

	/* Allocating temporary array for some interim results */
	float* y_first = NULL;
	y_first = (float *)heap_calloc(index, LEN, sizeof(float));
	if(y_first == NULL)
	{
		printf("Memory allocation error - phaser\n");
	}

	/* Two arrays a, b for coefficient of the peak filter  */
	float a[3];
	float b[3];

	/* Value a[0] is always the same,
	 * so it is calculated outside of loop
	 */
	a[0] = 1;

	//#pragma SIMD_for
	/* For loop that calculates interim output array, using IIR coefficients and input samples*/
	for(int i = 2; i < LEN; i++)
	{
		a[1] = notch_a_coeff[(i-2)*3 + 1];
		a[2] = notch_a_coeff[(i-2)*3 + 2];

		b[0] = notch_b_coeff[(i-2)*3 + 0];
		b[1] = notch_b_coeff[(i-2)*3 + 1];
		b[2] = notch_b_coeff[(i-2)*3 + 2];

		y_first[i] = b[0]*x[i] + b[1]*x[i-1] + b[2]*x[i-2] - a[1]*y_first[i-1] - a[2]*y_first[i-2];
	}

	/* Phaser effect works in two stage
	 * First stage is the first for loop, where interim output is calculated
	 * Second stage is second for loop that calculates final output array,
	 * using IIR coefficients and ouutput samples from previous for loop
	 *
	 */
	//#pragma SIMD_for
	for(int i = 2; i < LEN; i++)
	{

		a[1] = notch_a_coeff[(i-2)*3 + 1];
		a[2] = notch_a_coeff[(i-2)*3 + 2];

		b[0] = notch_b_coeff[(i-2)*3 + 0];
		b[1] = notch_b_coeff[(i-2)*3 + 1];
		b[2] = notch_b_coeff[(i-2)*3 + 2];


		y[i] = b[0]*y_first[i] + b[1]*y_first[i-1] + b[2]*y_first[i-2] - a[1]*y[i-1] - a[2]*y[i-2];

	}

	heap_free(index, y_first);
}

/*
 * @brief Function that processes input signal in a manner of distortion effect
 *
 * @param x     Input array of floats with samples of signal to be processed
 * @param y	    Output array of floats with processed samples
 * @param alpha Gain value for reverberated versions of input
 * @param N     Determines how many reverberations is going to happed
 * param R      Determines when how much the first reverberation is goind to be delayed
 */
void reverb(float* x, float* y, float alpha, int N, int R)
{
	int i, k;
	/* For loop that goes through all input samples*/
	//#pragma SIMD_for
	for(i = 0; i < LEN; i++)
	{
		/* Reduce iterations of the inner loop
		 * , for k = 0, it is always y[i] = x[i]
		 */
		y[i] = x[i];


		/* To reduce the number of comparison inside inner for loop, first check this conditon */
		if(i > N*R)
		{
			for(k = 1; k < N; k++)
			{
				y[i] += pow(alpha, k) * x[i - k*R];
			}
		} else
		{
			//#pragma SIMD_for
			for(k = 1; k < N; k++)
			{
				if(i > k*R)
				{
					y[i] += pow(alpha, k) * x[i - k*R];
				}
			}
		}
	}

	/* Finding the maximum of the output */
	float max = 0.0;
	for(i = 0; i < LEN; i++)
	{
		if(fabsf(y[i]) > max)
		{
			max = fabsf(y[i]);
		}
	}

	/* Normalizing output samples with maximum absolute value */
	//#pragma SIMD_for
	for(i = 0; i < LEN; i++)
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

	/* Installing heap on SRAM/SDRAM based on the selection, for storing input, output and interim values */
	index = heap_install(heap_mem, sizeof(heap_mem), uid);
	if(index < 0)
	{
		printf("Heap installation error\n");
		return -1;
	}

	/* Allocating input buffer */
	audio_input = (float*) heap_malloc(index, LEN*sizeof(float));
	if(audio_input == NULL)
	{
		printf("Memory allocation error - main\n");
		return -1;
	}

	/* Allocating output buffer */
	audio_output = (float *) heap_calloc(index, LEN, sizeof(float));
	if(audio_output == NULL)
	{
		printf("Memory allocation error - main\n");
		return -1;
	}

	/* Filling the input buffer from file test_audio.h */
	for(int i = 0; i < LEN; i++)
	{
		audio_input[i] = test_signal[i];
	}

	/* Definitions of the default parameters for some effect that needs them
	 * Naming of the parameters, and their values are the same as in corresponding Python implementation
	 * that goes along with this code
	 */

	/* Default parameters for the delay effect */
	int M = 1102;
	float g = 0.5;

	/* Default parameters for the distortion effect */
	float gain = 20.0;
	float mix = 0.8;

	/* Default parameters for the reverb effect */
	float alpha = 0.8;
	int N = 10;
	int R = 8;

	int select_effect = 1;

	switch(select_effect)
	{
		case 1:
			printf("Delay effect\n");

			#ifdef COUNT_CYCLES
			START_CYCLE_COUNT(start_count);
			#endif

			delay(audio_input, audio_output, M, g);

			#ifdef COUNT_CYCLES
			STOP_CYCLE_COUNT(final_count, start_count);
			PRINT_CYCLES("Number of cycles: \n", final_count);
			#endif

			break;
		case 2:
			printf("Distortion effect\n");

			#ifdef COUNT_CYCLES
			START_CYCLE_COUNT(start_count);
			#endif

			distortion(audio_input, audio_output, gain, mix);

			#ifdef COUNT_CYCLES
			STOP_CYCLE_COUNT(final_count, start_count);
			PRINT_CYCLES("Number of cycles: \n", final_count);
			#endif


			break;
		case 3:

			printf("Wah-Wah effect\n");

			#ifdef COUNT_CYCLES
			START_CYCLE_COUNT(start_count);
			#endif

			wah_wah(audio_input, audio_output);

			#ifdef COUNT_CYCLES
			STOP_CYCLE_COUNT(final_count, start_count);
			PRINT_CYCLES("Number of cycles: \n", final_count);
			#endif

			break;
		case 4:
			printf("Phaser effect\n");

			#ifdef COUNT_CYCLES
			START_CYCLE_COUNT(start_count);
			#endif

			phaser(audio_input, audio_output);

			#ifdef COUNT_CYCLES
			STOP_CYCLE_COUNT(final_count, start_count);
			PRINT_CYCLES("Number of cycles: \n", final_count);
			#endif

			break;
		case 5:
			printf("Reverb effect\n");

			#ifdef COUNT_CYCLES
			START_CYCLE_COUNT(start_count);
			#endif

			reverb(audio_input, audio_output, alpha, N, R);

			#ifdef COUNT_CYCLES
			STOP_CYCLE_COUNT(final_count, start_count);
			PRINT_CYCLES("Number of cycles: \n", final_count);
			#endif

			break;
		default:
			printf("None of the effect is chosen. You should pick a number between 1 and 5 \n");
	}

	/* Free the heap */
	heap_free(index, audio_input);
	heap_free(index, audio_output);

	return 0;
}


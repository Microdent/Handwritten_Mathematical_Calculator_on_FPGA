#include "cnn.h"

void input_load_conv(float* input, float input_buffer[Tn][Tr + 4][Tc + 4],
	int TR, int TC, int TN, int offset, bool enable)
{
	//	float input_tmp_buffer[n*r*c];

	if (enable)
		return;

	float* input_tmp_buffer = (float*)calloc(TN * TR * TC, sizeof(float));

	memcpy((float*)input_tmp_buffer, (float*)(input + offset), TN * TR * TC * sizeof(float));
	//printf("%d\n", TN * TR * TC);
	/*for (int i = 0; i < 1024; i++)
		printf("%.17f\n", input[i]);*/

	int tn, tr, tc;
	int input_tmp_offset = 0;
	for (tn = 0; tn < TN; tn++)
		for (tr = 0; tr < TR + 4; tr++)
			for (tc = 0; tc < TC + 4; tc++)
			{
				if ((tr == 0) || (tc == 0) || (tr == TR + 2) || (tc == TC + 2) || (tr == 1) || (tc == 1) || (tr == TR + 3) || (tc == TC + 3))
					input_buffer[tn][tr][tc] = 0;
				else
				{
					input_buffer[tn][tr][tc] = input_tmp_buffer[input_tmp_offset];
					/*if(r==32)
						printf("%.17f\n", input_buffer[tn][tr][tc]);*/
					input_tmp_offset++;
				}
			}

	/*for(int i=0; i < 5; i++)
		for (int j = 0; j < 5; j++)
		{
			printf("%.17f\n", input_buffer[0][i][j]);
		}*/
}

/*void input_load_fc(float* input, float input_buffer[Tn][Tr + 4][Tc + 4], int offset, bool enable)
{
	if (!enable)
		return;

	for (int i = 0; i < 4; i++)
		input_buffer[i][0][0] = input[i + offset];
}*/

void weight_load(float* weight, float weight_buffer[Tn][Tm][K][K],
	int TN, int TM, int k, int offset, bool enable)
{
	if (enable)
		return;
	
	//	static float weight_tmp_buffer[n*m*k*k];

	float* weight_tmp_buffer = (float*)calloc(TN * TM * k * k, sizeof(float));

	memcpy((float*)weight_tmp_buffer, (float*)(weight + offset), TN * TM * k * k * sizeof(float));

	int tm, tn, kr, kc;
	int weight_tmp_offset = 0;
	for (tm = 0; tm < TM; tm++)
		for (tn = 0; tn < TN; tn++)
			for (kr = 0; kr < k; kr++)
				for (kc = 0; kc < k; kc++)
				{
					weight_buffer[tn][tm][kr][kc] = weight_tmp_buffer[weight_tmp_offset];
					weight_tmp_offset++;
				}

	/*for (int i = 0; i < 5; i++)
		for (int j = 0; j < 5; j++)
		{
			printf("%.17f\n", weight_buffer[0][0][i][j]);
		}*/
}

void weight_load_fc(float weight_buffer_fc[8 * 8 * 64], float* Weight, int offset, bool enable)
{
	if (!enable)
		return;

	memcpy((float*)weight_buffer_fc, (float*)Weight + offset, 8 * 8 * 64 * sizeof(float));
}

void output_write(float output_buffer[Tm][Tr][Tc], float* Output, int TR, int TC, int TM,
	int offset, bool write_flag, bool half)
{
	if (half)
		return;
	else
	{
		TR = TR / 2;
		TC = TC / 2;
		offset = offset / 4;
	}

	if (!write_flag)
		return;

	//	static float output_tmp_buffer[m*r*c];

	float* output_tmp_buffer = (float*)calloc(TM * TR * TC, sizeof(float));

	int tm, tr, tc;
	int output_tmp_offset = 0;
	for (tm = 0; tm < TM; tm++)
		for (tr = 0; tr < TR; tr++)
			for (tc = 0; tc < TC; tc++)
			{
				output_tmp_buffer[output_tmp_offset] = output_buffer[tm][tr][tc];
				output_tmp_offset++;
			}

	memcpy((float*)(Output + offset), (float*)output_tmp_buffer, TM * TR * TC * sizeof(float));
}

void output_write_reorg(float output_buffer[Tm][Tr][Tc], float* Output, int TR, int TC, int TM,
	int offset, bool write_flag, bool half)
{
	if (half)
		return;
	else
	{
		TR = TR / 2;
		TC = TC / 2;
		offset = offset / 4;
	}

	if (!write_flag)
		return;

	//	static float output_tmp_buffer[m*r*c];

	float* output_tmp_buffer = (float*)calloc(TM * TR * TC, sizeof(float));

	int tm, tr, tc;
	int output_tmp_offset = 0;
	for (tr = 0; tr < TR; tr++)
		for (tc = 0; tc < TC; tc++)
			for (tm = 0; tm < TM; tm++)
			{
				output_tmp_buffer[output_tmp_offset] = output_buffer[tm][tr][tc];
				output_tmp_offset++;
			}

	memcpy((float*)(Output + offset), (float*)output_tmp_buffer, TM * TR * TC * sizeof(float));
}

void output_write_fc(float output_buffer[1024], float* Output, bool enable)
{
	if (!enable)
		return;

	memcpy((float*)Output, (float*)output_buffer, 1024 * sizeof(float));
}

void relu(float input[Tm][Tr][Tc], int TM, int TR, int TC, bool enable)
{
	if (!enable)
		return;

	int tm, tr, tc;

	for (tm = 0; tm < TM; tm++)
		for (tr = 0; tr < TR; tr++)
			for (tc = 0; tc < TC; tc++)
			{
				if (input[tm][tr][tc] < 0)
					input[tm][tr][tc] = 0;
				else
					input[tm][tr][tc] = input[tm][tr][tc];
				/*if (TR==16)
					printf("relu: %.17f\n", input[tm][tr][tc]);*/
			}
	/*if (TR == 16)
		printf("0  0  0:%.17f,\n 0  2  2:%.17f,\n 0 10 10:%.17f\n", input[0][0][0], input[0][2][2], input[0][10][10]);*/
}

void relu_fc(float input[1024], bool enable)
{
	if (!enable)
		return;

	for (int i = 0; i < 1024; i++)
	{
		if (input[i] < 0)
			input[i] = 0;
		else
			input[i] = input[i];
	}
}

void copy_local_beta(float beta_buffer[MAX_BETA_LENGTH], float local_beta_buffer[MAX_BETA_LENGTH], const int TM, int m)
{
	int offset;
	int tm;
	for (tm = 0, offset = m; tm < TM; tm++)
	{
		local_beta_buffer[tm] = beta_buffer[offset];
		offset++;
	}
}

void conv(float input_buffer[Tn][Tr + 4][Tc + 4], float output_buffer[Tm][Tr][Tc],
	float weight_buffer[Tn][Tm][K][K], float beta_buffer[MAX_BETA_LENGTH],
	const int Kernel_size, const int Kernel_stride, int TM_offset,
	const int TN, const int TM, const int TR, const int TC, bool enable, const int n)
{

	float local_beta_buffer[Tm];

	/*if(TR==16)
		printf("bias: %.17f\n", local_beta_buffer[0]);*/

	if (enable)
	{
		return;
	}

	copy_local_beta(beta_buffer, local_beta_buffer, TM, TM_offset);

	float partial_mul[Tn][Tm];

	int i, j, tr, tc;
	int tm, tn;

	for (i = 0; i < Kernel_size; i++)
		for (j = 0; j < Kernel_size; j++)
			for (tr = 0; tr < TR; tr++)
				for (tc = 0; tc < TC; tc++)
				{
					for (tm = 0; tm < TM; tm++)
					{
						float tmp_add_result;
						if (i == 0 && j == 0 && n == 0)
						{
							tmp_add_result = local_beta_buffer[tm];
						}
						else
							tmp_add_result = output_buffer[tm][tr][tc];

						float partial_sum = 0;

						for (tn = 0; tn < TN; tn++)
						{
							partial_mul[tn][tm] = input_buffer[tn][Kernel_stride * tr + i][Kernel_stride * tc + j] * weight_buffer[tn][tm][i][j];
							partial_sum += partial_mul[tn][tm];
						}

						output_buffer[tm][tr][tc] = partial_sum + tmp_add_result;
						//printf("%.17f\n", output_buffer[tm][tr][tc]);
					}
				}

}

void fc(float input_buffer[8 * 8 * 64], float output_buffer[1024], float beta_buffer[1024],
	float weight_buffer[8 * 8 * 64], bool enable, int m, int n)
{
	if (!enable)
		return;

	float partial_mul;

	for (int i = 0; i < 8 * 8 * 64; i++)
	{
		float temp_add_result;
		if (i == 0 && n == 0)
			temp_add_result = beta_buffer[m];
		else
			temp_add_result = output_buffer[m];

		partial_mul = input_buffer[i] * weight_buffer[i];

		output_buffer[m] = partial_mul + temp_add_result;
	}
}

void pool(float Input[Tm][Tr][Tc], float Output[Tm][Tr][Tc],
	const int Kernel_size, const int Kernel_stride,
	const int TM, const int TR, const int TC, bool enable)
{

	if (!enable)
		return;

	int tm, tr, tc;

	float tmp0, tmp1;
	//printf("%.17f\n", Input[0][10][10]);

	for (tr = 0; tr < TR; tr++)
		for (tc = 0; tc < TC; tc++)
			for (tm = 0; tm < TM; tm++)
			{
				if (Input[tm][Kernel_stride * tr][Kernel_stride * tc] > Input[tm][Kernel_stride * tr][Kernel_stride * tc + 1])
					tmp0 = Input[tm][Kernel_stride * tr][Kernel_stride * tc];
				else
					tmp0 = Input[tm][Kernel_stride * tr][Kernel_stride * tc + 1];

				if (Input[tm][Kernel_stride * tr + 1][Kernel_stride * tc] > Input[tm][Kernel_stride * tr + 1][Kernel_stride * tc + 1])
					tmp1 = Input[tm][Kernel_stride * tr + 1][Kernel_stride * tc];
				else
					tmp1 = Input[tm][Kernel_stride * tr + 1][Kernel_stride * tc + 1];

				if (tmp0 > tmp1)
					Output[tm][tr][tc] = tmp0;
				else
					Output[tm][tr][tc] = tmp1;
			}
}

void detection_acc(float* Input, float* Output, float* Weight, float* Beta, const int InFM_num, const int OutFM_num,
	const int Kernel_size, const int Kernel_stride, const int TM, const int TN, const int TR, const int TC,
	const int mLoops, const int nLoops, const int LayerType)
{
	static float input_buffer[Tn][Tr + 4][Tc + 4];
	static float input_buffer_fc[8 * 8 * 64];
	static float output_buffer[Tm][Tr][Tc];
	static float output_buffer_fc[1024];
	static float weight_buffer[Tn][Tm][K][K];
	static float weight_buffer_fc[8 * 8 * 64];
	static float beta_buffer[MAX_BETA_LENGTH];

	memcpy(beta_buffer, Beta, OutFM_num * sizeof(float));

	if (LayerType == 1)
		memcpy((float*)input_buffer_fc, (float*)Input, 4096 * sizeof(float));

	int m, n;
	for (m = 0; m < mLoops; m++)
	{
		for (n = 0; n < nLoops; n++)
		{
			input_load_conv(Input, input_buffer, TR, TC, TN, n * TN * TR * TC, LayerType);
			//input_load_fc(input_buffer_fc, input_buffer, m * 4 * nLoops + n * 4, LayerType);
			weight_load(Weight, weight_buffer, TN, TM, Kernel_size, m * TN * TM * nLoops * Kernel_size * Kernel_size + n * TM * TN * Kernel_size * Kernel_size, LayerType);
			weight_load_fc(weight_buffer_fc, Weight, m * 8 * 8 * 64, LayerType);
			conv(input_buffer, output_buffer, weight_buffer, beta_buffer, Kernel_size, Kernel_stride, m * TM, TN, TM, TR, TC, LayerType, n);
			fc(input_buffer_fc, output_buffer_fc, beta_buffer, weight_buffer_fc, LayerType, m, n);
		}
		relu(output_buffer, TM, TR, TC, n==nLoops);
		pool(output_buffer, output_buffer, 2, 2, TM, TR / 2, TC / 2, (!LayerType)&(n==nLoops));
		if (TR == 32)
			output_write(output_buffer, Output, TR, TC, TM, m * TM * TR * TC, n == nLoops, LayerType);
		else
			output_write(output_buffer, Output, TR, TC, TM, m * TM * TR * TC, n == nLoops, LayerType);
	}
	relu_fc(output_buffer_fc, LayerType);
	output_write_fc(output_buffer_fc, Output, LayerType);
}
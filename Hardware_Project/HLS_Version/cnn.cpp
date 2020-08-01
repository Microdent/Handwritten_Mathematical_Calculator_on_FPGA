#include "cnn.h"

void input_load_conv(int* input, int input_buffer[Tn][Tr + 4][Tc + 4],
	int TR, int TC, int TN, int offset, bool enable)
{
	static int input_tmp_buffer[32*32*1];

	if (enable)
		return;

	//int* input_tmp_buffer = (int*)calloc(TN * TR * TC, sizeof(int));

	memcpy((int*)input_tmp_buffer, (int*)(input + offset), 1 * 32 * 32 * sizeof(int));
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

/*void input_load_fc(int* input, int input_buffer[Tn][Tr + 4][Tc + 4], int offset, bool enable)
{
	if (!enable)
		return;

	for (int i = 0; i < 4; i++)
		input_buffer[i][0][0] = input[i + offset];
}*/

void weight_load(int* weight, int weight_buffer[Tn][Tm][K][K],
	int TN, int TM, int k, int offset, bool enable)
{
	if (enable)
		return;
	
	static int weight_tmp_buffer[4*32*5*5];

	//int* weight_tmp_buffer = (int*)calloc(TN * TM * k * k, sizeof(int));

	memcpy((int*)weight_tmp_buffer, (int*)(weight + offset), 4*32 * 5 * 5 * sizeof(int));

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

void weight_load_fc(int weight_buffer_fc[8 * 8 * 64], int* Weight, int offset, bool enable)
{
	if (!enable)
		return;

	memcpy((int*)weight_buffer_fc, (int*)Weight + offset, 8 * 8 * 64 * sizeof(int));
}

void output_write(int output_buffer[Tm][Tr][Tc], int* Output, int TR, int TC, int TM,
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

	static int output_tmp_buffer[32*16*16];

	//int* output_tmp_buffer = (int*)calloc(TM * TR * TC, sizeof(int));

	int tm, tr, tc;
	int output_tmp_offset = 0;
	for (tm = 0; tm < TM; tm++)
		for (tr = 0; tr < TR; tr++)
			for (tc = 0; tc < TC; tc++)
			{
				output_tmp_buffer[output_tmp_offset] = output_buffer[tm][tr][tc];
				//printf("%d:%d, out\n", output_tmp_offset, output_tmp_buffer[output_tmp_offset]);
				output_tmp_offset++;
			}

	//printf("%d\n",output_tmp_buffer[0]);
	memcpy((int*)(Output + offset), (int*)output_tmp_buffer, 32 * 16 * 16 * sizeof(int));
	//printf("%d, Out\n", Output[0]);
}

/*void output_write_reorg(int output_buffer[Tm][Tr][Tc], int* Output, int TR, int TC, int TM,
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

	//	static int output_tmp_buffer[m*r*c];

	int* output_tmp_buffer = (int*)calloc(TM * TR * TC, sizeof(int));

	int tm, tr, tc;
	int output_tmp_offset = 0;
	for (tr = 0; tr < TR; tr++)
		for (tc = 0; tc < TC; tc++)
			for (tm = 0; tm < TM; tm++)
			{
				output_tmp_buffer[output_tmp_offset] = output_buffer[tm][tr][tc];
				output_tmp_offset++;
			}

	memcpy((int*)(Output + offset), (int*)output_tmp_buffer, TM * TR * TC * sizeof(int));
}*/

void output_write_fc(int output_buffer[1024], int* Output, bool enable)
{
	if (!enable)
		return;

	memcpy((int*)Output, (int*)output_buffer, 1024 * sizeof(int));
}

void relu(int input[Tm][Tr][Tc], int TM, int TR, int TC, bool enable, int M)
{
	if (!enable)
		return;

	int tm, tr, tc;

	for (tm = 0; tm < TM; tm++)
		for (tr = 0; tr < TR; tr++)
			for (tc = 0; tc < TC; tc++)
			{
				//if(TR==16)
					//printf("%d:conv:%d\n", tm*256+tr*16+tc, input[tm][tr][tc]);
				if (input[tm][tr][tc] < 0)
					input[tm][tr][tc] = 0;
				else
				{
					input[tm][tr][tc] = input[tm][tr][tc]*M;
					//printf("relu:%d\n", input[tm][tr][tc]);
					input[tm][tr][tc] = input[tm][tr][tc]>>16;
					/*if(TR==32)
						printf("%d:mv:%d\n", tm*1024+tr*32+tc, input[tm][tr][tc]);*/
				}
				/*if (TR==16)
					printf("relu: %.17f\n", input[tm][tr][tc]);*/
			}
	/*if (TR == 16)
		printf("0  0  0:%.17f,\n 0  2  2:%.17f,\n 0 10 10:%.17f\n", input[0][0][0], input[0][2][2], input[0][10][10]);*/
}

void relu_fc(int input[1024], bool enable, int M)
{
	if (!enable)
		return;

	for (int i = 0; i < 1024; i++)
	{
		if (input[i] < 0)
			input[i] = 0;
		else
			input[i] = (input[i]*M)>>16;
	}
}

void copy_local_beta(int beta_buffer[MAX_BETA_LENGTH], int local_beta_buffer[MAX_BETA_LENGTH], const int TM, int m)
{
	int offset;
	int tm;
	for (tm = 0, offset = m; tm < TM; tm++)
	{
		local_beta_buffer[tm] = beta_buffer[offset];
		offset++;
	}
}

void conv(int input_buffer[Tn][Tr + 4][Tc + 4], int output_buffer[Tm][Tr][Tc],
	int weight_buffer[Tn][Tm][K][K], int beta_buffer[MAX_BETA_LENGTH],
	const int Kernel_size, const int Kernel_stride, int TM_offset,
	const int TN, const int TM, const int TR, const int TC, bool enable, const int n)
{

	static int local_beta_buffer[Tm];

	/*if(TR==16)
		printf("bias: %.17f\n", local_beta_buffer[0]);*/

	if (enable)
	{
		return;
	}

	copy_local_beta(beta_buffer, local_beta_buffer, TM, TM_offset);

	int partial_mul[Tn][Tm];

	int i, j, tr, tc;
	int tm, tn;

	for (i = 0; i < Kernel_size; i++)
		for (j = 0; j < Kernel_size; j++)
			for (tr = 0; tr < TR; tr++)
				for (tc = 0; tc < TC; tc++)
				{
					for (tm = 0; tm < TM; tm++)
					{
						int tmp_add_result;
						if (i == 0 && j == 0 && n == 0)
						{
							tmp_add_result = local_beta_buffer[tm];
						}
						else
							tmp_add_result = output_buffer[tm][tr][tc];

						int partial_sum = 0;

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

void fc(int input_buffer[8 * 8 * 64], int output_buffer[1024], int beta_buffer[1024],
	int weight_buffer[8 * 8 * 64], bool enable, int m, int n)
{
	if (!enable)
		return;

	int partial_mul;

	for (int i = 0; i < 8 * 8 * 64; i++)
	{
		int temp_add_result;
		if (i == 0 && n == 0)
			temp_add_result = beta_buffer[m];
		else
			temp_add_result = output_buffer[m];

		partial_mul = input_buffer[i] * weight_buffer[i];

		output_buffer[m] = partial_mul + temp_add_result;
	}
}

void pool(int Input[Tm][Tr][Tc], int Output[Tm][Tr][Tc],
	const int Kernel_size, const int Kernel_stride,
	const int TM, const int TR, const int TC, bool enable)
{

	if (!enable)
		return;

	int tm, tr, tc;

	int tmp0, tmp1;
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
				//printf("%d,out\n", Output[tm][tr][tc]);
			}
}

void detection_acc(int* Input, int* Output, int* Weight, int* Beta, const int InFM_num, const int OutFM_num,
	const int Kernel_size, const int Kernel_stride, const int TM, const int TN, const int TR, const int TC,
	const int mLoops, const int nLoops, const int LayerType, const int M)
{
	static int input_buffer[Tn][Tr + 4][Tc + 4];
	static int input_buffer_fc[8 * 8 * 64];
	static int output_buffer[Tm][Tr][Tc];
	static int output_buffer_fc[1024];
	static int weight_buffer[Tn][Tm][K][K];
	static int weight_buffer_fc[8 * 8 * 64];
	static int beta_buffer[MAX_BETA_LENGTH];

	memcpy(beta_buffer, Beta, OutFM_num * sizeof(int));

	if (LayerType == 1)
		memcpy((int*)input_buffer_fc, (int*)Input, 4096 * sizeof(int));

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
		relu(output_buffer, TM, TR, TC, n==nLoops, M);
		pool(output_buffer, output_buffer, 2, 2, TM, TR / 2, TC / 2, (!LayerType)&(n==nLoops));
		output_write(output_buffer, Output, TR, TC, TM, m * TM * TR * TC, n == nLoops, LayerType);
	}
	relu_fc(output_buffer_fc, LayerType, M);
	output_write_fc(output_buffer_fc, Output, LayerType);
}

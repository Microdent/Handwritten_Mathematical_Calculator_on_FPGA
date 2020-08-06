#include "cnn.h"

void input_load_conv(int* input, int8 input_buffer[Tn][Tr + 4][Tc + 4],
	int TR, int TC, int TN, int offset, bool enable)
{
	static int input_tmp_buffer[32*32*1];

	if (enable)
		return;

	memcpy((int*)input_tmp_buffer, (int*)(input + offset), 1 * 32 * 32 * sizeof(int));

	int tn, tr, tc;
	int input_tmp_offset = 0;
	for (tn = 0; tn < TN; tn++)
		for (tr = 0; tr < TR + 4; tr++)
			for (tc = 0; tc < TC + 4; tc++)
			{
#pragma HLS PIPELINE II=2
				if ((tr == 0) || (tc == 0) || (tr == TR + 2) || (tc == TC + 2) || (tr == 1) || (tc == 1) || (tr == TR + 3) || (tc == TC + 3))
					input_buffer[tn][tr][tc] = 0;
				else
				{
					input_buffer[tn][tr][tc] = input_tmp_buffer[input_tmp_offset];
					input_tmp_offset++;
				}
			}
}

void weight_load(int* weight, int8 weight_buffer[Tn][Tm][K][K],
	int TN, int TM, int k, int offset, bool enable)
{
	if (enable)
		return;
	
	static int weight_tmp_buffer[4*32*5*5];

	memcpy((int*)weight_tmp_buffer, (int*)(weight + offset), 4*32 * 5 * 5 * sizeof(int));

	int tm, tn, kr, kc;
	int weight_tmp_offset = 0;
	for (tm = 0; tm < TM; tm++)
		for (tn = 0; tn < TN; tn++)
			for (kr = 0; kr < k; kr++)
				for (kc = 0; kc < k; kc++)
				{
#pragma HLS PIPELINE II=2
					weight_buffer[tn][tm][kr][kc] = weight_tmp_buffer[weight_tmp_offset];
					weight_tmp_offset++;
				}
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

	int tm, tr, tc;
	int output_tmp_offset = 0;
	for (tm = 0; tm < TM; tm++)
		for (tr = 0; tr < TR; tr++)
			for (tc = 0; tc < TC; tc++)
			{
#pragma HLS PIPELINE II=2
				output_tmp_buffer[output_tmp_offset] = output_buffer[tm][tr][tc];
				output_tmp_offset++;
			}

	memcpy((int*)(Output + offset), (int*)output_tmp_buffer, 32 * 16 * 16 * sizeof(int));
}

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

	int tm0, tm1, tr, tc;

	for (tr = 0; tr < TR; tr++)
		for (tc = 0; tc < TC; tc++)
		{
			for(tm0=0; tm0 < 4; tm0++)
			{
#pragma HLS PIPELINE
				for (tm1 = 0; tm1 < Tm/4; tm1++)
				{
					int tm = (tm0<<3) + tm1;

					int tmp = input[tm][tr][tc];

					if (tmp < 0)
						input[tm][tr][tc] = 0;
					else
					{
						input[tm][tr][tc] = (tmp*M)>>16;
					}
				}
			}
		}
}

void relu_fc(int input[1024], bool enable, int M)
{
	if (!enable)
		return;

	for (int i = 0; i < 128; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			int k = (i<<3) + j;
			if (input[k] < 0)
				input[k] = 0;
			else
				input[k] = (input[k]*M)>>16;
		}
	}
}

void copy_local_beta(int beta_buffer[MAX_BETA_LENGTH], int8 local_beta_buffer[MAX_BETA_LENGTH], const int TM, int m)
{
	int offset;
	int tm;
	for (tm = 0, offset = m; tm < Tm; tm++)
	{
#pragma HLS PIPELINE
		local_beta_buffer[tm] = beta_buffer[offset];
		offset++;
	}
}

void conv(int8 input_buffer[Tn][Tr + 4][Tc + 4], int output_buffer[Tm][Tr][Tc],
	int8 weight_buffer[Tn][Tm][K][K], int beta_buffer[MAX_BETA_LENGTH],
	const int Kernel_size, const int Kernel_stride, int TM_offset,
	const int TN, const int TM, const int TR, const int TC, bool enable, const int n)
{

	static int8 local_beta_buffer[Tm];
#pragma HLS ARRAY_PARTITION variable=local_beta_buffer complete dim=1

	if (enable)
	{
		return;
	}

	copy_local_beta(beta_buffer, local_beta_buffer, TM, TM_offset);

	int partial_mul[Tn][Tm];
#pragma HLS ARRAY_PARTITION variable=partial_mul complete dim=0

	int i, j, tr, tc;
	int tm0, tm1, tn;

	for (i = 0; i < 5; i++)
		for (j = 0; j < 5; j++)
			for (tr = 0; tr < TR; tr++)
				for (tc = 0; tc < TC; tc++)
				{
					for(tm0=0; tm0 < 4; tm0++)
					{
#pragma HLS PIPELINE II=2
						for(tm1=0; tm1 < Tm/4; tm1++)
						{
							int tmp_add_result;
							int tm = (tm0<<3) + tm1;

							if (i == 0 && j == 0 && n == 0)
							{
								tmp_add_result = local_beta_buffer[tm];
							}
							else
								tmp_add_result = output_buffer[tm][tr][tc];

							partial_mul[0][tm] = input_buffer[0][tr + i][tc + j] * weight_buffer[0][tm][i][j];
							partial_mul[1][tm] = input_buffer[1][tr + i][tc + j] * weight_buffer[1][tm][i][j];
							partial_mul[2][tm] = input_buffer[2][tr + i][tc + j] * weight_buffer[2][tm][i][j];
							partial_mul[3][tm] = input_buffer[3][tr + i][tc + j] * weight_buffer[3][tm][i][j];

							int tmp0 = partial_mul[0][tm] + partial_mul[1][tm];
							int tmp1 = partial_mul[2][tm] + partial_mul[3][tm];

							int tmp = (TN==1)? partial_mul[0][tm]: (tmp0 + tmp1);

							output_buffer[tm][tr][tc] = tmp + tmp_add_result;
						}
					}
				}
}

void fc(int input_buffer[8 * 8 * 64], int output_buffer[1024], int beta_buffer[1024],
	int weight_buffer[8 * 8 * 64], bool enable, int m, int n)
{
	if (!enable)
		return;

	int partial_mul[8];
#pragma HLS ARRAY_PARTITION variable=partial_mul complete dim=1

	for(int i = 0; i < 8*64; i++)
	{
#pragma HLS PIPELINE
		int tmp_add_result;
		if(i==0&&n==0)
			tmp_add_result = beta_buffer[m];
		else
			tmp_add_result = output_buffer[m];

		int j = i<<3;

		partial_mul[0] = input_buffer[j] * weight_buffer[j];
		partial_mul[1] = input_buffer[j+1]*weight_buffer[j+1];
		partial_mul[2] = input_buffer[j+2]*weight_buffer[j+2];
		partial_mul[3] = input_buffer[j+3]*weight_buffer[j+3];
		partial_mul[4] = input_buffer[j+4]*weight_buffer[j+4];
		partial_mul[5] = input_buffer[j+5]*weight_buffer[j+5];
		partial_mul[6] = input_buffer[j+6]*weight_buffer[j+6];
		partial_mul[7] = input_buffer[j+7]*weight_buffer[j+7];

		int tmp0 = partial_mul[0] + partial_mul[1];
		int tmp1 = partial_mul[2] + partial_mul[3];
		int tmp2 = partial_mul[4] + partial_mul[5];
		int tmp3 = partial_mul[6] + partial_mul[7];

		int tmp01 = tmp0 + tmp1;
		int tmp23 = tmp2 + tmp3;

		int tmp0123 = tmp01 + tmp23;

		output_buffer[m] = tmp0123 + tmp_add_result;
	}
}

void pool(int Input[Tm][Tr][Tc],
	const int Kernel_size, const int Kernel_stride,
	const int TM, const int TR, const int TC, bool enable)
{

	if (!enable)
		return;

	int tm, tr, tc;

	for (tr = 0; tr < TR; tr++)
		for (tc = 0; tc < TC; tc++)
		{
#pragma HLS PIPELINE
			for (tm = 0; tm < Tm; tm++)
			{
				int tmp0, tmp1;
				int trr = tr << 1;
				int tcc = tc << 1;
				if (Input[tm][trr][tcc] > Input[tm][trr][tcc+1])
					tmp0 = Input[tm][trr][tcc];
				else
					tmp0 = Input[tm][trr][tcc+1];

				if (Input[tm][trr+1][tcc] > Input[tm][trr+1][tcc+1])
					tmp1 = Input[tm][trr+1][tcc];
				else
					tmp1 = Input[tm][trr+1][tcc+1];

				if (tmp0 > tmp1)
					Input[tm][tr][tc] = tmp0;
				else
					Input[tm][tr][tc] = tmp1;
			}
		}
}

void detection_acc(int* Input, int* Output, int* Weight, int* Beta, const int InFM_num, const int OutFM_num,
	const int Kernel_size, const int Kernel_stride, const int TM, const int TN, const int TR, const int TC,
	const int mLoops, const int nLoops, const int LayerType, const int M)
{
#pragma HLS INTERFACE s_axilite port=M bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=LayerType bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=nLoops bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=mLoops bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=TC bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=TR bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=TN bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=TM bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=Kernel_stride bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=Kernel_size bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=OutFM_num bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=InFM_num bundle=CRTL_BUS
#pragma HLS INTERFACE m_axi depth=1024 port=Beta offset=slave bundle=Beta
#pragma HLS INTERFACE m_axi depth=4194304 port=Weight offset=slave bundle=Weight
#pragma HLS INTERFACE m_axi depth=16384 port=Output offset=slave bundle=Output
#pragma HLS INTERFACE m_axi depth=16384 port=Input offset=slave bundle=Input
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS

	static int8 input_buffer[Tn][Tr + 4][Tc + 4];
#pragma HLS ARRAY_PARTITION variable=input_buffer complete dim=1
	static int input_buffer_fc[8 * 8 * 64];
#pragma HLS ARRAY_PARTITION variable=input_buffer_fc cyclic factor=8 dim=1
	static int output_buffer[Tm][Tr][Tc];
#pragma HLS ARRAY_PARTITION variable=output_buffer complete dim=1
	static int output_buffer_fc[1024];
#pragma HLS ARRAY_PARTITION variable=output_buffer_fc cyclic factor=8 dim=1
	static int8 weight_buffer[Tn][Tm][K][K];
#pragma HLS ARRAY_PARTITION variable=weight_buffer complete dim=2
#pragma HLS ARRAY_PARTITION variable=weight_buffer complete dim=1
	static int weight_buffer_fc[8 * 8 * 64];
#pragma HLS ARRAY_PARTITION variable=weight_buffer_fc cyclic factor=8 dim=1
	static int beta_buffer[MAX_BETA_LENGTH];

	memcpy(beta_buffer, Beta, OutFM_num * sizeof(int));

	if (LayerType == 1)
		memcpy((int*)input_buffer_fc, (int*)Input, 4096 * sizeof(int));

	int m, n;
	for (m = 0; m < mLoops; m++)
	{
		for (n = 0; n < nLoops; n++)
		{
			input_load_conv(Input, input_buffer, TR, TC, TN, n * TN * TR * TC, LayerType);;
			weight_load(Weight, weight_buffer, TN, TM, Kernel_size, m * TN * TM * nLoops * Kernel_size * Kernel_size + n * TM * TN * Kernel_size * Kernel_size, LayerType);
			weight_load_fc(weight_buffer_fc, Weight, m * 8 * 8 * 64, LayerType);
			conv(input_buffer, output_buffer, weight_buffer, beta_buffer, Kernel_size, Kernel_stride, m * TM, TN, TM, TR, TC, LayerType, n);
			fc(input_buffer_fc, output_buffer_fc, beta_buffer, weight_buffer_fc, LayerType, m, n);
		}
		relu(output_buffer, TM, TR, TC, n==nLoops, M);
		pool(output_buffer, 2, 2, TM, TR / 2, TC / 2, (!LayerType)&(n==nLoops));
		output_write(output_buffer, Output, TR, TC, TM, m * TM * TR * TC, n == nLoops, LayerType);
	}
	relu_fc(output_buffer_fc, LayerType, M);
	output_write_fc(output_buffer_fc, Output, LayerType);
}

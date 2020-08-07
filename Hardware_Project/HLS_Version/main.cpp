#include "cnn.h"

void file_error(char* s)
{
    fprintf(stderr, "Couldn't open file: %s\n", s);
    exit(0);
}

void calculator_ps(int* input)
{
    int weight_offset[3] = { 800, 51200, 4194304 };
    int beta_offset[3] = { 32, 64, 1024 };

    int M[3] = {94, 48, 43};

    int* Weight_buf = (int*)calloc(4260640, sizeof(int));
    int* Beta_buf = (int*)calloc(1134, sizeof(int));

    FILE* fp_w = fopen("./parameters/weight.bin", "rb");
    if (!fp_w) file_error("weight.bin");

    FILE* fp_b = fopen("./parameters/bias.bin", "rb");
    if (!fp_b) file_error("bias.bin");

    fread(Weight_buf, sizeof(int), 4260640, fp_w);
    fread(Beta_buf, sizeof(int), 1134, fp_b);

    fclose(fp_w);
    fclose(fp_b);

#define MEM_LEN (16*16*32+8*8*64)
    int* Memory_buf = (int*)calloc(MEM_LEN + 1024 * 2, sizeof(int));
    int* Memory_top = Memory_buf + 1024;
    int* Memory_bottom = Memory_top + MEM_LEN;
    memcpy(Memory_top, input, 32 * 32 * 1 * sizeof(int));

    int* in_ptr[4];
    int* out_ptr[4];

    in_ptr[0] = Memory_top;
    out_ptr[0] = Memory_bottom - 16 * 16 * 32;

    in_ptr[1] = out_ptr[0];
    out_ptr[1] = Memory_top;

    in_ptr[2] = out_ptr[1];
    out_ptr[2] = Memory_bottom - 1024;

    in_ptr[3] = out_ptr[2];
    out_ptr[3] = Memory_top;

    int i;
    int woffset = 0;
    int boffset = 0;
    int TR, TC, TM, TN;
    int mLoops, nLoops;

    for (i = 0; i < 4; ++i)
    {
        if (i == 0)
        {
            printf("Conv0\n");

            TR = 32;
            TC = 32;

            TM = 32;
            TN = 1;

            mLoops = 1;
            nLoops = 1;

            detection_acc(in_ptr[i], out_ptr[i], Weight_buf + woffset, Beta_buf + boffset,
                1, 32, 5, 1, TM, TN, TR, TC, mLoops, nLoops, 0, M[i]);

            woffset += weight_offset[i];
            boffset += beta_offset[i];
        }
        else if (i == 1)
        {
            printf("Conv2\n");

            TR = 16;
            TC = 16;

            TM = 32;
            TN = 4;

            mLoops = 2;
            nLoops = 8;

            detection_acc(in_ptr[i], out_ptr[i], Weight_buf + woffset, Beta_buf + boffset,
                32, 64, 5, 1, TM, TN, TR, TC, mLoops, nLoops, 0, M[i]);

            woffset += weight_offset[i];
            boffset += beta_offset[i];
        }
        else if (i == 2)
        {
            printf("FC4\n");

            int reorg_out[64][8][8];

            for (int m = 0; m < 64; m++)
                for (int r = 0; r < 8; r++)
                    for (int c = 0; c < 8; c++)
                    {
                        reorg_out[m][r][c] = in_ptr[2][m * 64 + r * 8 + c];
                    }

            for (int r = 0; r < 8; r++)
                for (int c = 0; c < 8; c++)
                    for (int m = 0; m < 64; m++)
                    {
                        in_ptr[2][r * 512 + c * 64 + m] = reorg_out[m][r][c];
                    }
            
            TR = 1;
            TC = 1;

            TM = 32;
            TN = 4;

            mLoops = 1024;
            nLoops = 1;

            detection_acc(in_ptr[i], out_ptr[i], Weight_buf + woffset, Beta_buf + boffset,
                8 * 8 * 64, 1024, 1, 1, TM, TN, TR, TC, mLoops, nLoops, 1, M[i]);

            woffset += weight_offset[i];
            boffset += beta_offset[i];
        }
        else if (i == 3)
        {
            printf("FC5\n");

            int m, n;
            for (m = 0; m < 14; m++)
            {
                for (n = 0; n < 1024; n++)
                {
                    int tmp_add_result;

                    if (n == 0)
                        tmp_add_result = Beta_buf[1120 + m];
                    else
                        tmp_add_result = out_ptr[3][m];

                    int partial_mul = in_ptr[3][n] * Weight_buf[4246304 + m * 1024 + n];

                    out_ptr[3][m] = partial_mul + tmp_add_result;
                }
                printf("%d: %d\n", m, out_ptr[3][m]);
            }
        }
    }

    free(Memory_buf);
    free(Weight_buf);
    free(Beta_buf);
}

int main()
{
    printf("*****Handwritten Mathematical Calculator Test Begin!*****\n");

    int* Input_buf = (int*)calloc(1024, sizeof(int));

    FILE* fp_i = fopen("./input_imgs/input4_int.bin", "rb");
    if (!fp_i) file_error("input.bin");

    fread(Input_buf, sizeof(int), 1024, fp_i);

    fclose(fp_i);

    time_t first, second;

    first = time(NULL);
    calculator_ps(Input_buf);
    second = time(NULL);
    printf("Predicted in %f seconds.\n", difftime(second, first));
}

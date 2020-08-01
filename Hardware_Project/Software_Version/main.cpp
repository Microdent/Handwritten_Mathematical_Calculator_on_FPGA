#include "cnn.h"

void file_error(char* s)
{
    fprintf(stderr, "Couldn't open file: %s\n", s);
    exit(0);
}

/*image make_empty_image(int w, int h, int c)
{
    image out;
    out.data = 0;
    out.h = h;
    out.w = w;
    out.c = c;
    return out;
}


image make_image(int w, int h, int c)
{
    image out = make_empty_image(w, h, c);
    out.data = (float*)calloc(h * w * c, sizeof(float));
    return out;
}

image load_image_stb(char* filename, int channels)
{
    int w, h, c;
    unsigned char* data = stbi_load(filename, &w, &h, &c, channels);
    if (!data) {
        fprintf(stderr, "Cannot load image \"%s\"\nSTB Reason: %s\n", filename, stbi_failure_reason());
        exit(0);
    }
    if (channels) c = channels;
    int i, j, k;
    image im = make_image(w, h, c);
    for (k = 0; k < c; ++k) {
        for (j = 0; j < h; ++j) {
            for (i = 0; i < w; ++i) {
                int dst_index = i + w * j + w * h * k;
                int src_index = k + c * i + c * w * j;
                im.data[dst_index] = (float)data[src_index] / 255.;
            }
        }
    }
    free(data);
    return im;
}*/

void calculator_ps(float* input)
{
    int weight_offset[3] = { 800, 51200, 4194304 };
    int beta_offset[3] = { 32, 64, 1024 };

    float* Weight_buf = (float*)calloc(4260640, sizeof(float));
    float* Beta_buf = (float*)calloc(1134, sizeof(float));

    FILE* fp_w = fopen("./parameters/weight.bin", "rb");
    if (!fp_w) file_error("weight.bin");

    FILE* fp_b = fopen("./parameters/bias.bin", "rb");
    if (!fp_b) file_error("bias.bin");

    fread(Weight_buf, sizeof(float), 4260640, fp_w);
    fread(Beta_buf, sizeof(float), 1134, fp_b);

    /*for (int i = 0; i < 1134; i++)
        printf("bias[%d]:%.17f\n",i, Beta_buf[i]);*/

    fclose(fp_w);
    fclose(fp_b);

#define MEM_LEN (16*16*32+8*8*64)
    float* Memory_buf = (float*)calloc(MEM_LEN + 1024 * 2, sizeof(float));
    float* Memory_top = Memory_buf + 1024;
    float* Memory_bottom = Memory_top + MEM_LEN;
    memcpy(Memory_top, input, 32 * 32 * 1 * sizeof(float));

    float* in_ptr[4];
    float* out_ptr[4];

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
                1, 32, 5, 1, TM, TN, TR, TC, mLoops, nLoops, 0);

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
                32, 64, 5, 1, TM, TN, TR, TC, mLoops, nLoops, 0);

            woffset += weight_offset[i];
            boffset += beta_offset[i];

            /*for (int j = 8092; j < 8192; j++)
                printf("%.17f\n", in_ptr[i][j]);

            printf("\n");*/
            //printf("\n");

            /*for (int j = 3996; j < 4096; j++)
            {
                printf("%.17f\n", out_ptr[i][j]);
            }*/
            //printf("%.17f", out_ptr[i][0]);
        }
        else if (i == 2)
        {
            printf("FC4\n");

            float reorg_out[64][8][8];

            for (int m = 0; m < 64; m++)
                for (int r = 0; r < 8; r++)
                    for (int c = 0; c < 8; c++)
                    {
                        reorg_out[m][r][c] = in_ptr[2][m * 64 + r * 8 + c];
                        //printf("reo1:%d\n", m * 64 + r * 8 + c);
                    }

            for (int r = 0; r < 8; r++)
                for (int c = 0; c < 8; c++)
                    for (int m = 0; m < 64; m++)
                    {
                        in_ptr[2][r * 512 + c * 64 + m] = reorg_out[m][r][c];
                        //printf("reo2:%d\n", r * 512 + c * 64 + m);
                    }
            /*printf("\n");
            for (int j = 3996; j < 4096; j++)
            {
                printf("%.17f\n", in_ptr[i][j]);
            }
            printf("\n");*/
            
            TR = 1;
            TC = 1;

            TM = 32;
            TN = 4;

            mLoops = 1024;
            nLoops = 1;

            detection_acc(in_ptr[i], out_ptr[i], Weight_buf + woffset, Beta_buf + boffset,
                8 * 8 * 64, 1024, 1, 1, TM, TN, TR, TC, mLoops, nLoops, 1);

            woffset += weight_offset[i];
            boffset += beta_offset[i];

            /*for (int j = 0; j < 1024; j++)
                printf("%d: %.17f\n", j, out_ptr[i][j]);*/

            /*float* in_buf = (float*)calloc(4096, sizeof(float));

            FILE* fp_w = fopen("fc_0_0_input.bin", "rb");
            if (!fp_w) file_error("fc_0_0_input.bin");

            fread(in_buf, sizeof(float), 4096, fp_w);

            fclose(fp_w);*/

            /*int m, n;
            for (m = 0; m < 1024; m++)
            {
                for (n = 0; n < 4096; n++)
                {
                    float tmp_add_result;

                    if (n == 0)
                        tmp_add_result = Beta_buf[96 + m];
                    else
                        tmp_add_result = out_ptr[2][m];

                    float partial_mul = in_ptr[2][n] * Weight_buf[52000 + m * 4096 + n];
                    if (m == 0 && n == 1024)
                        printf("input:%.17f\nweight:%.17f\nbias:%.17f\n", in_ptr[2][n], Weight_buf[52000 + m * 4096 + n], Beta_buf[96 + m]);

                    out_ptr[2][m] = partial_mul + tmp_add_result;
                    if (m == 0 && n == 1024)
                        printf("out[%d]:%.17f\n", n, out_ptr[2][0]);
                }
                printf("%.17f\n", out_ptr[2][m]);
            }*/
        }
        else if (i == 3)
        {
            printf("FC5\n");

            int m, n;
            for (m = 0; m < 14; m++)
            {
                for (n = 0; n < 1024; n++)
                {
                    float tmp_add_result;

                    if (n == 0)
                        tmp_add_result = Beta_buf[1120 + m];
                    else
                        tmp_add_result = out_ptr[3][m];

                    float partial_mul = in_ptr[3][n] * Weight_buf[4246304 + m * 1024 + n];

                    out_ptr[3][m] = partial_mul + tmp_add_result;
                }
                printf("%d: %.17f\n", m, out_ptr[3][m]);
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

    //   char* image_path = "./test.jpeg";

    //   image im = load_image_stb(image_path, 1);
    //   printf("Input img:%s\n w=%d,h=%d,c=%d\n", image_path, im.w, im.h, im.c);

    float* Input_buf = (float*)calloc(1024, sizeof(float));

    FILE* fp_i = fopen("./input_imgs/input1.bin", "rb");
    if (!fp_i) file_error("input.bin");

    fread(Input_buf, sizeof(float), 1024, fp_i);

    fclose(fp_i);

    //    float *X = im.data;

    time_t first, second;

    first = time(NULL);
    calculator_ps(Input_buf);
    second = time(NULL);
    printf("Predicted in %f seconds.\n", difftime(second, first));
}
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#define MAX_SIZE 1024*968

double get_mse(unsigned char* f1, unsigned char* f2, size_t n)
{
    double sum_of_square = 0;
    size_t i = 0;
    int count = 0;
    for (i = 0; i<n ; i++)
    {
        double diff = (double)(f1[i] - f2[i]);
        if (diff != 0)
        {
            count++;
            //printf("%d %e\n", count, diff);
        }
        sum_of_square += (diff*diff);
    }
    //printf("sum: %e\n", sum_of_square);
    printf("%ld %d ", n, count);
    return sum_of_square/n;
}

double get_psnr(double mse, double max)
{
    double rmse = sqrt(mse);
    return 20*log10(max / rmse);
}

char pm_getc(FILE* fpgm)
{
    int ich;
    char ch;

    ich = getc(fpgm);
    if (ich == EOF) 
    {
        fprintf(stderr, "EOF\n");
        exit(1);
    }
    ch = (char)ich;
    if (ch == '#')
    {
        do 
        {
            ich = getc(fpgm);
            if (ich == EOF)
            {
                fprintf(stderr, "EOF\n");
                exit(1);
            }
            ch = (char) ich;
        }while (ch!= '\n' && ch != '\r');
    }
    return ch;
}

unsigned int pm_getuint(FILE* fpgm)
{
    char ch;
    unsigned int i = 0;
    do
    {
        ch = pm_getc(fpgm);
    }while (ch == ' '||ch == '\t'||ch == '\n'||ch == '\r');

    if (ch <'0' || ch > '9')
    {
        fprintf(stderr, "unexpected non number character.%c\n", ch);
        exit(1);
    }
    
    do
    {
        unsigned int const digitVal = ch - '0';
        if (i > (INT_MAX/10 - digitVal))
        {
            fprintf(stderr, "integer too large\n");
            exit(1);
        }
        i = i*10 + digitVal;
        ch = pm_getc(fpgm);
    }while(ch>='0' && ch <= '9');
    return i;
}



void read_pgm(FILE* fpgm, unsigned char* f, size_t* p_len, unsigned int* p_max_val)
{

    unsigned char ch;
    unsigned int cols;
    unsigned int rows;
    unsigned int maxval;

    if (!fpgm) 
    {
        fprintf(stderr, "file pointer is NULL\n");
        exit(1);
    }
    if (getc(fpgm) != 'P') 
    {
        fprintf(stderr, "wrong format!\n");
        exit(1);
    }
    if (getc(fpgm) != '5')
    {
        fprintf(stderr, "wrong format!\n");
        exit(1);
    }

    cols = pm_getuint(fpgm);
    rows = pm_getuint(fpgm);
    maxval = pm_getuint(fpgm);
    if (maxval > 255) 
    {
        fprintf(stderr, "only deal with 1 byte data\n");
        exit(1);
    }
    if (*p_len < cols * rows)
    {
        fprintf(stderr, "the image is too large\n");
        exit(1);
    }
    *p_len = cols * rows;

    do
    {
        ch = (char)getc(fpgm);
    }while(ch == ' '||ch == '\t'||ch == '\r'||ch == '\n');
    fseek(fpgm, -1, SEEK_CUR);
    if(fread(f, 1, *p_len, fpgm)<*p_len)
    {
        fprintf(stderr, "file size error.\n");
        exit(1);
    }
    *p_max_val = maxval;
    return;
}

int main(int argc, char** argv)
{
    FILE* fpgm1 = NULL;
    FILE* fpgm2 = NULL;
    unsigned char f1[MAX_SIZE];
    unsigned char f2[MAX_SIZE];
    size_t len1 = MAX_SIZE;
    size_t len2 = MAX_SIZE;
    double mse;
    double psnr;
    unsigned int max_val;
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <pgm1> <pgm2>\n", argv[0]);
        exit(1);
    }
    fpgm1 = fopen(argv[1], "rb");
    fpgm2 = fopen(argv[2], "rb");
    if (!fpgm1 || !fpgm2)
    {
        fprintf(stderr, "open file error.\n");
        exit(1);
    }
    read_pgm(fpgm1, f1, &len1, &max_val);
    read_pgm(fpgm2, f2, &len2, &max_val);
    if (len1 != len2)
    {
        fprintf(stderr, "image size is not the same.\n");
        exit(1);
    }

    mse = get_mse(f1, f2, len1);
    psnr = get_psnr(mse, (double)max_val);
    printf("%e %e %f\n", mse, sqrt(mse), psnr);
    return 0;
}





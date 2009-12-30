#include "psnr_mse.hh"

//f1 1024 * 768 Y value ,  f2 1024 * 768 * 3 RGB value.
double get_mse(const unsigned char* f1, const unsigned char* f2, size_t n, long *p_error_count)
{
    double sum_of_square = 0;
    size_t i = 0;
    int count = 0;
    for (i = 0; i<n ; i++)
    {
        unsigned char r = *(f2++);
        unsigned char g = *(f2++);
        unsigned char b = *(f2++);
        unsigned char y = (unsigned char)(0.299*r + 0.587*g + 0.114*b);
        double diff = (double)(f1[i] - y);
        if (diff != 0)
        {
            count++;
        }
        sum_of_square += (diff*diff);
    }
    *p_error_count = count;
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

//f1 1024 * 768 Y value ,  f2 1024 * 768 * 3 RGB value.
void check_psnr(size_t size, int max, const unsigned char *f1, const unsigned char *f2, double *p_psnr, long *p_error_count)
{
    double mse = get_mse(f1, f2, size, p_error_count);
    *p_psnr    = get_psnr(mse, max);
}





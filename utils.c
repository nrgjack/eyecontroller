#include "utils.h"

void init_speed_table (double interrupt_ms) 
{
    int i = 0;
    for(i = 0; i<256;i++)
        speed_table[i] = 1.8 / (2*interrupt_ms*(1+i)) * 1000; 
}

unsigned char get_speed_from_table(double spd)
{
    double delta_speed,delta_speed_old = 0;
    int i = 0;
    for(i = 0; i<256;i++)
    {
        delta_speed = fabs(speed_table[i] - spd);
        //printf("table speed %f \n",speed_table[i]);
        //printf("delta speed %f rif %f\n",delta_speed,spd);
        if (delta_speed > delta_speed_old && i != 0)
            break; 
        else
            delta_speed_old = delta_speed;
    }
    return (unsigned char) (i-1);
}

void print_speed_table ()
{
    int i = 0;
    for(i = 0; i<256;i++)
        printf("%f\n",speed_table[i]); 
}

unsigned char convertToStep(double deg) 
{
    
    //deg += 57;
    //deg = deg * 128 / 57;
    //deg = round(deg);
    return (unsigned char)( 128 + round(deg/0.9));     
}

double convertToDeg(unsigned char step) // [0,255]
{
    double deg = step;
    deg -= 128;
    //deg = deg / 255  * 114;
    
    return  deg*0.9;     
}

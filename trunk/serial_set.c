#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

#include "utils.h"

struct msg_1
{
    unsigned char sync0;    //0xAF
    unsigned char sync1;    //0xFA
    unsigned char id;       //
    unsigned char pos_y;    //riferimento tilt
    unsigned char pos_x1;   //riferimento pan eye1
    unsigned char pos_x2;   //riferimento pan eye2
    unsigned char spd_y;    //riferimento speed tilt
    unsigned char spd_x1;   //riferimento speed pan eye1
    unsigned char spd_x2;   //riferimento speed pan eye2
    unsigned char cw_y;     //comando clockwise tilt
    unsigned char cw_x1;    //comando clockwise tilt pan eye1
    unsigned char cw_x2;    //comando clockwise tilt pan eye2
    unsigned char checksum; //checksum
    unsigned char sync2;    //0xFF
};

struct msg_2
{
    unsigned char sync0;    //0xAF
    unsigned char sync1;    //0xFA
    unsigned char pos_y;    //riferimento tilt
    unsigned char pos_x1;   //riferimento pan eye1
    unsigned char pos_x2;   //riferimento pan eye2
    unsigned char checksum; //checksum
    unsigned char sync2;    //0xFF
};

union msg_1_union
{
    struct msg_1 message;
    unsigned char buffer[14];
};

union msg_1_union msg_sent;
struct msg_2 msg_rcvd;

static int fd = 0;

unsigned char serial_read()
{
    int iIn = 0;
    unsigned char temp;
    while(read(fd,&temp,1) != 1);
    return temp;    
}

int main (int argc, char** argv)
{


    init_speed_table(0.1504);
          
  
    if (fd > 0)
    {
        close(fd);
    }

    fd =  open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    
    if (fd < 0)
    {
        printf("open error %d %s\n", errno, strerror(errno));
    }
    else
    {
        struct termios my_termios;
        printf("fd is %d\n", fd);
        tcgetattr(fd, &my_termios);
        
        tcflush(fd, TCIFLUSH);
        
        my_termios.c_cflag = CS8 |CREAD | CLOCAL | HUPCL;
        my_termios.c_iflag = 0;
        my_termios.c_oflag = 0;    
    
        
        cfsetospeed(&my_termios, B9600);
        tcsetattr(fd, TCSANOW, &my_termios);
 
    }

    //richiedo la posizione
    msg_sent.message.sync0 = 0xAF;
    msg_sent.message.sync1 = 0xFA;
    msg_sent.message.id = 0x22;
    msg_sent.message.pos_y =  0;
    msg_sent.message.pos_x1 = 0;
    msg_sent.message.pos_x2 = 0;
    msg_sent.message.spd_y = 0;
    msg_sent.message.spd_x1 = 0;
    msg_sent.message.spd_x2 = 0;
    msg_sent.message.cw_y = 0;
    msg_sent.message.cw_x1 = 0;
    msg_sent.message.cw_x2 = 0;
    msg_sent.message.checksum = msg_sent.message.id
                                ^ msg_sent.message.pos_y
                                ^ msg_sent.message.pos_x1
                                ^ msg_sent.message.pos_x2
                                ^ msg_sent.message.spd_y
                                ^ msg_sent.message.spd_x1
                                ^ msg_sent.message.spd_x2
                                ^ msg_sent.message.cw_y
                                ^ msg_sent.message.cw_x1
                                ^ msg_sent.message.cw_x2;

    msg_sent.message.sync2 = 0xFF;
    
    int iOut = write(fd, msg_sent.buffer, sizeof(struct msg_1));
    
    if (iOut < 0)
    {
        printf("write error %d %s\n", errno, strerror(errno));
    }

    while(1)
    {
        memset(&msg_rcvd,0,sizeof(struct msg_1));

        msg_rcvd.sync0 = serial_read();
         
        if (msg_rcvd.sync0 != 0xAF)
            continue;

                       
        msg_rcvd.sync1 = serial_read();
        
        if (msg_rcvd.sync1 != 0xFA)
            continue;
        
        printf("ricevuto inizio\n");
       
        msg_rcvd.pos_y = serial_read();
        
       
        msg_rcvd.pos_x1 = serial_read();
        //printf("ricevuto asd\n");
        msg_rcvd.pos_x2 = serial_read();
        msg_rcvd.checksum = serial_read();
            
        printf("letto %d %d %d\n",msg_rcvd.pos_y,msg_rcvd.pos_x1,msg_rcvd.pos_x2);
        
        msg_rcvd.sync2 = serial_read();
        if (msg_rcvd.sync2 != 0xFF)
            continue;
        
        printf("ricevuto fine\n");
       
        if (msg_rcvd.checksum !=   ( msg_rcvd.pos_y
                                    ^ msg_rcvd.pos_x1
                                    ^ msg_rcvd.pos_x2))
            continue;
    
        
        //printf("pos_y = %d\n",msg_rcvd.pos_y);
        //printf("pos_x1 = %d\n",msg_rcvd.pos_x1);
        //rintf("pos_x2 = %d\n",msg_rcvd.pos_x2);
        break;                
    }
    
    //calcolo le velocità
    double pos_y, pos_x1, pos_x2 = 0;
    double delta_y, delta_x1, delta_x2 = 0;
    pos_y = atof(argv[1]);    
    pos_x1 = atof(argv[2]);
    pos_x2 = atof(argv[3]);

    delta_y = abs(pos_y - convertToDeg( msg_rcvd.pos_y));
    delta_x1 = abs(pos_x1 - convertToDeg( msg_rcvd.pos_x1));
    delta_x2 = abs(pos_x2 - convertToDeg( msg_rcvd.pos_x2));
    
    // printf("dy = %f, dx_1 = %f, dx_2 = %f\n ",delta_y,delta_x1,delta_x2);

    //double ris_x1,ris_x2 = 0
    //ris_x1 = sqrt(delta_y^2 + delta_x1^2);      
    //ris_x2 = sqrt(delta_y^2 + delta_x2^2);

    double teta1, teta2, tetaAvg = 0;
    teta1 = atan2(delta_x1,delta_y);
    teta2 = atan2(delta_x1,delta_x2);
    tetaAvg = (teta1 + teta2) / 2;

    //printf("t1 = %f, t2 = %f, tavg = %f\n ",teta1,teta2,tetaAvg);

    
    double spd_y,spd_x = 0;
    spd_y = atof(argv[4]) * sin(tetaAvg);
    spd_x = atof(argv[4]) * cos(tetaAvg);
    
    //printf("spdy = %f, spdx = %f\n ",spd_y,spd_x);   

    printf("spdy = %d - %f, spdx = %d - %f \n ",get_speed_from_table(spd_y),speed_table[get_speed_from_table(spd_y)],get_speed_from_table(spd_x),speed_table[get_speed_from_table(spd_x)]);    

    printf("invio %d %d %d\n",convertToStep(atof(argv[1])),convertToStep(atof(argv[2])),convertToStep(atof(argv[3])));
    
    //invio il comando
    msg_sent.message.sync0 = 0xAF;
    msg_sent.message.sync1 = 0xFA;
    msg_sent.message.id = 0x11;
    msg_sent.message.pos_y =  convertToStep(atof(argv[1]));
    msg_sent.message.pos_x1 = convertToStep(atof(argv[2]));
    msg_sent.message.pos_x2 = convertToStep(atof(argv[3]));
    msg_sent.message.spd_y = get_speed_from_table(spd_x);
    msg_sent.message.spd_x1 = get_speed_from_table(spd_y);
    msg_sent.message.spd_x2 = get_speed_from_table(spd_x);
    msg_sent.message.cw_y = 0;
    msg_sent.message.cw_x1 = 0;
    msg_sent.message.cw_x2 = 0;
    msg_sent.message.checksum = msg_sent.message.id
                                ^ msg_sent.message.pos_y
                                ^ msg_sent.message.pos_x1
                                ^ msg_sent.message.pos_x2
                                ^ msg_sent.message.spd_y
                                ^ msg_sent.message.spd_x1
                                ^ msg_sent.message.spd_x2
                                ^ msg_sent.message.cw_y
                                ^ msg_sent.message.cw_x1
                                ^ msg_sent.message.cw_x2;

    msg_sent.message.sync2 = 0xFF;

    //printf("debug %d\n",msg_sent.message.checksum);


    iOut = write(fd, msg_sent.buffer, sizeof(struct msg_1));
    
    if (iOut < 0)
    {
        printf("write error %d %s\n", errno, strerror(errno));
    }
     
    return 0;
}

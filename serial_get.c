#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

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


static int fd = 0;

unsigned char serial_read()
{
    int iIn = 0;
    unsigned char temp;
    while(read(fd,&temp,1) != 1);
    return temp;    
}


union msg_1_union msg_sent;
struct msg_2 msg_rcvd;



int main (int argc, char** argv)
{
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
    else
    {
    	printf("wrote %d chars\n", iOut);
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
        printf("ricevuto y %d %x %x\n",msg_rcvd.pos_y,msg_rcvd.pos_y,28);
   
        msg_rcvd.pos_x1 = serial_read();
        printf("ricevuto x1 %d %x\n",msg_rcvd.pos_x1,msg_rcvd.pos_x1);
   
        msg_rcvd.pos_x2 = serial_read();
        printf("ricevuto x2 %d %x\n",msg_rcvd.pos_x2,msg_rcvd.pos_x2);
   
        
        msg_rcvd.checksum = serial_read();
        
        msg_rcvd.sync2 = serial_read();
        if (msg_rcvd.sync2 != 0xFF)
            continue;
        
        printf("ricevuto fine\n");
       
        if (msg_rcvd.checksum !=   ( msg_rcvd.pos_y
                                    ^ msg_rcvd.pos_x1
                                    ^ msg_rcvd.pos_x2))
            continue;

        
        printf("pos_y = %d\n",msg_rcvd.pos_y);
        printf("pos_x1 = %d\n",msg_rcvd.pos_x1);
        printf("pos_x2 = %d\n",msg_rcvd.pos_x2);
        break;
                
    }
    
    
    
 
       
    return 0;
}



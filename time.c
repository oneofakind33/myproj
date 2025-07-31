#include <stdio.h>               // headers 
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define I2C_BUS "/dev/i2c-3"   
#define OLED_ADDR 0x3C

int i2c_fd;

//defining and storing numbers in hex format in an const array to light up pixels (6x10)
static const uint8_t Font6x10[][10] = 
{
    [0] = {0x3E,0x7F,0xC1,0xC1,0xC1,0xC1,0xC1,0x7F,0x3E,0x00}, // 0
    [1] = {0x00,0x00,0xC2,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00}, // 1
    [2] = {0xE2,0xF3,0x99,0x99,0x99,0xCF,0xC6,0x00,0x00,0x00}, // 2
    [3] = {0x42,0xC3,0x89,0x89,0x89,0xFF,0x76,0x00,0x00,0x00}, // 3
    [4] = {0x3C,0x3E,0x27,0x23,0xFF,0xFF,0x20,0x00,0x00,0x00}, // 4
    [5] = {0x4F,0xCF,0x89,0x89,0x89,0xF9,0x71,0x00,0x00,0x00}, // 5
    [6] = {0x7E,0xFF,0x89,0x89,0x89,0xFB,0x72,0x00,0x00,0x00}, // 6
    [7] = {0x03,0x03,0xE1,0xF1,0x19,0x0F,0x07,0x00,0x00,0x00}, // 7
    [8] = {0x76,0xFF,0x89,0x89,0x89,0xFF,0x76,0x00,0x00,0x00}, // 8
    [9] = {0x4E,0xDF,0x91,0x91,0x91,0xFF,0x7E,0x00,0x00,0x00}, // 9
    [10] = {0x00,0x00,0x6C,0x6C,0x00,0x6C,0x6C,0x00,0x00,0x00}, // :
};

void oled_write(uint8_t val, int is_data) //val is the actual data we need to send, is_data is a flag that determines if the val is data or command
{   
    uint8_t buffer[2] = { is_data ? 0x40 : 0x00, val }; 
    write(i2c_fd, buffer, 2); //used to send data to the i2c device.
}

void oled_command(uint8_t cmd) //helper fn to send cmd to the OLED
{ 
    oled_write(cmd, 0);
}

void oled_data(uint8_t data) //helper fn to send data to the OLED
{ 
    oled_write(data, 1);
}

void oled_init() //oled initialization
{
    oled_command(0xAE); // display off
    oled_command(0xD5); oled_command(0x80); // Set Display Clock Divide Ratio / Oscillator Frequency 0x80 is default
    oled_command(0xA8); oled_command(0x1F); // multiplex ratio 128x32 0x1F = 31 = selecting 32 rows (0–31).
    oled_command(0xD3); oled_command(0x00); //Set Display Offset 0x00 = no offset, i.e screen starts from the top.
    oled_command(0x40); //Set cursor Start Line to 0
    oled_command(0x8D); oled_command(0x14); //Charge Pump Setting
    oled_command(0x20); oled_command(0x00); // horizontal addressing mode
    oled_command(0xA1); // segment remap
    oled_command(0xC8); // COM scan direction
    oled_command(0xDA); oled_command(0x02); //set hardware config 0x02 is for 128×32 panels
    oled_command(0x81); oled_command(0x8F); //control brightness 0x8F = mid-level brightness. 0x00 (dim) to 0xFF (bright).
    oled_command(0xD9); oled_command(0xF1); //set precharge period
    oled_command(0xDB); oled_command(0x40);
    oled_command(0xA4); //Entire Display ON
    oled_command(0xA6); //Set Normal Display A6 = normal (white on black) A7 = inverse (black on white)
    oled_command(0xAF); // display ON
}

void oled_clear()   //clear the pixels i.e low
{  
    for (int page = 0; page < 4; page++) {  //4 pages (each page = 8 vertical pixels), so 32 pixels vertically / 8 = 4 pages.
        oled_command(0xB0 + page); //Sets the current page address.
        oled_command(0x00);  //Sets the lower nibble of the column address (bits 0–3). (horizaontally)
        oled_command(0x10);  //Sets the lower nibble of the column address (bits 0–3).
        for (int i = 0; i < 128; i++)  //Loops through pages 0 to 3 to clear all 32 vertical pixels
            oled_data(0x00);  //clear cmd
    }
}

void oled_set_cursor(int x, int page)  //postions the cursor for further printing/displaying x= column postion 0-127 , vertical page (0–7)
{ 
    oled_command(0xB0 + page);  //This sets the page address
    oled_command(0x00 + (x & 0x0F));  //This sets the lower nibble (4 bits) of the column address.
    oled_command(0x10 + ((x >> 4) & 0x0F));  //This sets the higher nibble of the column address.
}

void oled_print_big_char(char c) //displays a single big character (6x10)
{
    const uint8_t* font = Font6x10[0]; // fallback to '0', pointer to the first element of Font6x10[0]
    if (c >= '0' && c <= '9') font = Font6x10[c - '0']; //if numbers then stored at respectful indices
    else if (c == ':') font = Font6x10[10]; //if : stored at 10 index

    for (int i = 0; i < 10; i++) //loops over to the 0-10 position of the matrix for turning on pixels 6 horizontally and 10 vertically
      oled_data(font[i]); 
}

void oled_print_big_string(int x, int y, const char* str)  //x is x axis (0-127) horizontal line and y is vertical (0-31) line
{ //
    oled_set_cursor(x, y);
    while (*str) oled_print_big_char(*str++);
}

void get_time_string(char* buffer) 
{
    time_t now = time(NULL); //fetch time from the machine and store it in seconds
    struct tm* t = localtime(&now); //splitting seconds into hh::mm::ss
    snprintf(buffer, 9, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec); //It writes formatted data into a character buffer (str)
}

int main() { 
    if ((i2c_fd = open(I2C_BUS, O_RDWR)) < 0) ////opens the I2C device file (/dev/i2c-3) for read/write.
    {
        perror("I2C open failed");
        return 1;
    }

    if (ioctl(i2c_fd, I2C_SLAVE, OLED_ADDR) < 0) // checks if the communcation between the said slave is established
    {
        perror("I2C ioctl failed"); //print error
        return 1;
    }

    oled_init(); //initialization command
    oled_clear(); //clear screen of any active pixels

    char time_str[9]; //create a buffer of 10 spaces to store the time

    while (1) 
    {
        get_time_string(time_str); //fetch time from system

        int str_width = strlen(time_str) * 10; //Calculates the pixel width of the time string
        int x_start = (128 - str_width) / 2; //center the string

        oled_clear(); //cleasr OLED
        oled_print_big_string(x_start, 1, time_str); //print the time

        usleep(500000); // blink every 0.5 second
    }

    close(i2c_fd);
    return 0;
}

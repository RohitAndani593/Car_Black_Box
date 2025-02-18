#include <xc.h>
#include "main.h"
#include "clcd.h"
#include "matrix_keypad.h"
#include "adc.h"
#include "i2c.h"
#include "ds1307.h"
#include "external_EEPROM.h"
#include "uart.h"
#include <string.h>

extern unsigned char key;
unsigned char time[9];
unsigned char clock_reg[3];
unsigned char EV[9][3]={"ON","GN","G1","G2","G3","G4","G5","GR","C_"};
unsigned char menu[4][16]={"VIEW_LOG       ","CLEAR_LOG      ","DOWNLOAD_LOG  ","SET_TIME       "};
unsigned char read_data[10][15];
unsigned int speed = 00;
unsigned int index=0,index2=0,data_index=0,arr_index=0;
unsigned int add;
unsigned int field=0;
unsigned int hour,min,sec;
static int once_clear=0,only_once=0;
unsigned char temp,add1=12;
unsigned int ev_count;
unsigned int flag = 0,flag2 = 0;
unsigned int i = 0,j = 0,once = 1;
extern unsigned int state;
volatile unsigned long int delay, t_delay;

void init_config()
{
   init_matrix_keypad();
   init_clcd();
   init_adc();
   init_i2c();
   init_ds1307();
   init_uart();
}
void  store_event()
{
    ev_count++;  // Increment the event count
    if(ev_count > 10)
    {
        ev_count = 10;   // Reset event count to 10 if it exceeds the limit
        for(unsigned int i=0;i<9;i++)
        {
            for(unsigned int a=0;a<12;a++)
            {
              // Copy data from one address to another
              write_external_eeprom(temp,read_external_eeprom(add1));
              temp++;
              add1++;
            }
        }
        temp=0;
        add1=12;
        add = add - 12;
    }
    // Write 8 bytes of time data to EEPROM
    for(unsigned int i=0;i<8;i++)
    {
        write_external_eeprom(add,time[i]);
        add++;
    }
    for(unsigned int i=0;i<2;i++)
    {
        write_external_eeprom(add,EV[index][i]);
        add++;
    }
    // Write 2 bytes of speed data to EEPROM as ASCII values
    write_external_eeprom(add++,(speed/10+48));
    write_external_eeprom(add++,(speed%10+48));
}

void get_time(void)
{
    // Read the current hour, minute, and second from the DS1307 RTC
	clock_reg[0] = read_ds1307(HOUR_ADDR);
	clock_reg[1] = read_ds1307(MIN_ADDR);
	clock_reg[2] = read_ds1307(SEC_ADDR);
    
    // Process the hour value to format it in 12-hour or 24-hour format
	if (clock_reg[0] & 0x40)
	{
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);  // Extract the tens digit for hour
		time[1] = '0' + (clock_reg[0] & 0x0F);   // Extract the units digit for hour
	}
	else
	{
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
		time[1] = '0' + (clock_reg[0] & 0x0F);
	}
	time[2] = ':';  // Separator between hour and minute
	time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
	time[4] = '0' + (clock_reg[1] & 0x0F);
	time[5] = ':';
	time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
	time[7] = '0' + (clock_reg[2] & 0x0F);
	time[8] = '\0';   // Null-terminate the time string
}

void view_dashboard()
{
    if(once==1)
    {
        store_event();
        once++;
    }
        // Display the dashboard labels on the first line of the LCD
        clcd_print("TIME      EV  SP", LINE1(0));
        clcd_print(time, LINE2(0));  // Display the current time on the second line of the LCD
        clcd_print(EV[index], LINE2(10)); 
        
        // Calculate speed using ADC reading from CHANNEL4 and convert it
        speed = read_adc(CHANNEL4)/10.3;
        clcd_putch((speed/10+48), LINE2(14)); 
        clcd_putch((speed%10)+48, LINE2(15)); 
        
        // Check if the key pressed is MK_SW1
        if(key == MK_SW1)
        {
           index = 8;
           flag = 1;
           store_event();
        }
        // Check if the key pressed is MK_SW2
        if(key == MK_SW2)
        {
           if(flag)
           {
               index = 0;
           }
           if(index<7)
           index++;
           flag = 0;
           store_event();
        }
        // Check if the key pressed is MK_SW3
        if(key == MK_SW3)
        {
           if(flag)
           {
               index = 1;
           }
           if(index>1)
           index--;
           flag = 0;
           store_event();
        }
        // Check if the key pressed is MK_SW11
        if(key == MK_SW11)
        {
            state = 2;
        } 
}

void display_menu()
{    
    if(key == MK_SW1)
    {
        clcd_clear();
        if(arr_index == 0)
        {
            state = 3; //for view log
        }
        else if(arr_index == 1)
        {
            state = 5; //for clearing the log
        }
        else if(arr_index == 2)
        {
            state = 4; //download the log
        }
        else if(arr_index == 3)
        {
            state = 6; //set time
        }
    }    
    // Check if the key pressed is MK_SW12
    if(key == MK_SW12)
    {
       if(arr_index >= 0 && arr_index < 3)
       arr_index++;
       if(i == 1 && j < 3)
       {
           j++;
           if(j == 3)
           {
               j = 3;
           }
       }
       i = 1;
    }
    else if(key == MK_SW11)  // Check if the key pressed is MK_SW11
    {
       if(arr_index > 0 && arr_index <= 3)
       arr_index--;
       if(i == 0 && j > 0)
       {
           j--;
           if(j == 0)
           {
               j = 0;
           }
       }
       i = 0;
    }
       
    if(i == 0)
    {
        clcd_print("-> ", LINE1(0));
        clcd_print("   ", LINE2(0));
    }
    else if(i == 1)
    {
        clcd_print("   ", LINE1(0));
        clcd_print("-> ", LINE2(0));
    }
    
    if(j != 3)
    {
      clcd_print(menu[j], LINE1(3));
      clcd_print(menu[j+1], LINE2(3));
    }
    else if(j == 3)
    {
      clcd_print(menu[j-1], LINE1(3)); 
      clcd_print(menu[j], LINE2(3));     
    }
    // Check if the key pressed is MK_SW2
    if(key == MK_SW2)
    {
        clcd_clear();  //clear clcd display
        state = 1;  
        i=0;j=0;
    }
}

void view_log()
{
   int read_add = 0;
   for(int i=0; i<ev_count; i++)
   {
       for(int j=0; j<15; j++)
       {
           // Replace specific positions (8 and 11) with a space character
           if(j == 8 || j == 11)
           {
               read_data[i][j] = ' ';
           }
           else if(j == 14)   // Set the last character in the event data (position 14) to a null terminator
           {
               read_data[i][j] = '\0';
           }
           else
           {
               // Read the data from external EEPROM and store it in the array
               read_data[i][j] = read_external_eeprom(read_add++);
           }
       }
   }
   if(ev_count > 0)
   {
   // displaying the read log
     clcd_print("  TIME     EV SP", LINE1(0));
     clcd_putch(data_index + '0', LINE2(0));
   //scroll down the data
   if(key == MK_SW12 && data_index < ev_count-1)
   {
       data_index++;
       if(data_index > 9)
       {
           data_index = 9;
       }
   }
   //scroll up the data
   else if(key == MK_SW11)
   {
       
     if(data_index > 0)
     {
         data_index--;  
     }
   }
   
   clcd_print(read_data[data_index], LINE2(2));
   //for not to increase the index if no more events
  
   }
   if(ev_count == 0)
   {
       clcd_clear();
       clcd_print("No logs to view", LINE1(0));
       __delay_ms(1000);
       clcd_clear();
       state = 2;
   }
   // Check if the key pressed is MK_SW2
   if(key == MK_SW2)
   {
       data_index = 0;
       clcd_clear();
       state = 2;
   }
   
}

void clear_log()
{
    // Increment delay and check if it's less than 500
    if(delay++ < 500)
    {
        clcd_clear();  // Clear the LCD display
        clcd_print("All logs clear", LINE1(0));
    }
    else
    {
        state = 2;
        ev_count = 0;
        delay = 0;
        i = 0;j = 0;
        arr_index = 0;
        add = 0;
    }
}

void download_log()
{ 
   int read_add = 0;
   for(int i=0; i<ev_count; i++)
   {
       for(int j=0; j<15; j++)
       {
           // Replace specific positions (8 and 11) with a space character
           if(j == 8 || j == 11)
           {
               read_data[i][j] = ' ';
           }
           else if(j == 14)   // Set the last character in the event data (position 14) to a null terminator
           {
               read_data[i][j] = '\0';
           }
           else
           {
               // Read the data from external EEPROM and store it in the array
               read_data[i][j] = read_external_eeprom(read_add++);
           }
       }
   }
   if(flag2==0)
   {
      for(int i=0; i<ev_count;i++)
      {
         puts(read_data[i]);
         puts("\n\r");
      }
      flag2 = 1;
   }
   // Increment delay and check if it's less than 400
   if(delay++ < 400)
   {
        clcd_clear();  // Clear the LCD display
        clcd_print("DOWNLOADED", LINE1(0));
   }
   else
   {
       i=0;j=0;
       state = 2;
       flag2 = 0;
       delay = 0;
       arr_index=0;
   }
}

void set_time()
{
    if(only_once==0)
    {
        hour =(time[0]-'0')*10+(time[1]-'0');  // Extract hour from the `time` string
        min =(time[3]-'0')*10+(time[4]-'0');   // Extract min from the `time` string
        sec =(time[6]-'0')*10+(time[7]-'0');   // Extract sec from the `time` string
        only_once++;
    }
    // Blinking effect for the hour field
    if(t_delay<100 && field==0)
    {
        clcd_putch(0xFF,LINE2(4));  // Display solid blocks for the hour field
        clcd_putch(0xFF,LINE2(5));
    }
    else if(t_delay<200 && t_delay>100 && field==0)
    {
        // Display the actual hour value when not blinking
        clcd_putch((hour/10)+'0',LINE2(4));
        clcd_putch((hour%10)+'0',LINE2(5));
        clcd_putch((min/10)+'0',LINE2(7));
        clcd_putch((min%10)+'0',LINE2(8));
        clcd_putch((sec/10)+'0',LINE2(10));
        clcd_putch((sec%10)+'0',LINE2(11)); 
    }
    // Blinking effect for the minute field
    if(t_delay <100 && field ==1)
    {
       clcd_putch(0xFF,LINE2(7));  // Display solid blocks for the minute field
       clcd_putch(0xFF,LINE2(8)); 
    }
    else if(t_delay<200 && t_delay>100 && field==1)
    {
        clcd_putch((hour/10)+'0',LINE2(4));
        clcd_putch((hour%10)+'0',LINE2(5));
        clcd_putch((min/10)+'0',LINE2(7));
        clcd_putch((min%10)+'0',LINE2(8));
        clcd_putch((sec/10)+'0',LINE2(10));
        clcd_putch((sec%10)+'0',LINE2(11)); 
    }
    // Blinking effect for the second field
    if(t_delay <100 && field ==2)
    {
       clcd_putch(0xFF,LINE2(10));  // Display solid blocks for the second field
       clcd_putch(0xFF,LINE2(11)); 
    }
    else if(t_delay<200 && t_delay>100 && field==2)
    {
        clcd_putch((hour/10)+'0',LINE2(4));
        clcd_putch((hour%10)+'0',LINE2(5));
        clcd_putch((min/10)+'0',LINE2(7));
        clcd_putch((min%10)+'0',LINE2(8));
        clcd_putch((sec/10)+'0',LINE2(10));
        clcd_putch((sec%10)+'0',LINE2(11)); 
    }
    // Reset the delay counter after completing one blink cycle
    if(t_delay++==201)
    {
        t_delay=0;
    }   
    // Increment the current field (hour, minute, or second) when MK_SW11 is pressed
    if(key== MK_SW11)
    {
        
        if(field==0)
        {
            hour++;
            
            if(hour==24)
            {
                hour=0;
            }
        }
        
        if(field==1)
        {
            min++;
            
            if(min==60)
            {
                min=0;
            }
        }
        
        if(field==2)
        {
            sec++;
            
            if(sec==60)
            {
                sec=0;
            }
        }
    }
    // Move to the next field when MK_SW12 is pressed
    if(key==MK_SW12)
    {
        field++;
        
        if(field==3)
        {
            field=0;
        }
    }
    // Clear the LCD screen only once
    if(once_clear==0)
    {
        clcd_clear();
        once_clear++;
    }
    // Display the time format on the LCD
    if(once_clear>0)
    {
        clcd_print("    HH:MM:SS    ",LINE1(0));
        clcd_print(":",LINE2(6));
        clcd_print(":",LINE2(9));
    }
    // Exit time-setting mode when MK_SW2 is pressed
    if(key==MK_SW2)
    {
        state=2;
        j=0;i=0;
        arr_index=0;
        only_once=0;
        once_clear=0;
        field=0;
    }
    else if(key==MK_SW1)   // Save the time when MK_SW1 is pressed
    {
         // Write the hour, minute, and second values to the RTC
        write_ds1307(HOUR_ADDR, (((hour/10)<<4)|(hour%10)));
        write_ds1307(MIN_ADDR, (((min/10)<<4)|(min%10)));
        write_ds1307(SEC_ADDR, (((sec/10)<<4)|(sec%10)));
        
        state=2;
        j=0;i=0;
        arr_index=0;
        only_once=0;
        once_clear=0;
        field=0;
    }
           
}


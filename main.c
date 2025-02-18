
/* Name : Rohit Andani
   Date : 07/01/2025
   Batch : 24018G_325
   Project : Car Black Box
 */

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

unsigned char key;
unsigned int state = 1;

void main(void)
{
    // Initialize all configurations
    init_config();
    
    while (1)
    {
        get_time();
        // Read the current key press or switch state
        key = read_switches(STATE_CHANGE);
        
        switch (state)
        {
            case 1:
                view_dashboard();  // Display dashboard information on the LCD
                break;
            
            case 2:
                display_menu();  // Show the menu options
                break;
            
            case 3:
                view_log();  // Display logged events on the LCD
                break;
                 
            case 4:
                download_log();  // Transfer log data to an external device
                break;
                
            case 5:
                clear_log();  // Erase all logged events 
                break;
                     
            case 6:
                set_time();  // Enter time-setting mode to update the RTC
                break;  
        }
    }
}
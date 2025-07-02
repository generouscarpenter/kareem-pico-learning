#include "main.h"
#include <stdio.h>
#include "buttons.h"
#include "hardware/gpio.h"

#define NUM_PINS 3
#define NUM_BUTTONS NUM_PINS

const uint8_t button_pin[3] =
{
    10, 11, 12

};


void init_buttons()
{

    for(int this_pin = 0; this_pin<NUM_PINS; this_pin++)
    {
        gpio_init(button_pin[this_pin]);
        gpio_set_dir(button_pin[this_pin], GPIO_IN );
        gpio_pull_up(button_pin[this_pin]);
    }

}


enum
{
    BUTTON_ONE,
    BUTTON_TWO,
    BUTTON_THREE
};


void button_event(u8 this_b)
{
    printf("Button Event: %d\n", this_b);
    switch(this_b)
    {
        case BUTTON_ONE:
        {
            // Do something for button one
            printf("Button One Pressed\n");
            // Example: toggle an LED or change a setting
            // gpio_put(LED_PIN, !gpio_get(LED_PIN)); // Toggle LED state
        }
        break;

        case BUTTON_TWO:
        {
            // Do something for button two
            printf("Button Two Pressed\n");
            // Example: change a parameter or send a message
            // send_message_to_core("Button Two Pressed");
        }
        break;
        case BUTTON_THREE:
        {
            // Do something for button three
            printf("Button Three Pressed\n");
            // Example: reset a parameter or change a mode
            // reset_parameter();
        }
        break;
    }
}



u8 getSwitchState(u8 this_pin)
{
    return gpio_get(button_pin[this_pin]);
}



typedef enum B_STATE
{
	SW_OFF,                
    SW_GOING_ON,
	SW_ON               

}B_STATE;


B_STATE b_state[NUM_BUTTONS];
u32     b_time[NUM_BUTTONS];

void process_buttons()
{


	static u8 this_b;
	u8        pin_state;



	pin_state = !(getSwitchState(this_b));		//invert value here 


    //****  OFF  ****************
    if(b_state[this_b] == SW_OFF)
	{
	 	if(pin_state)
		{
			b_state[this_b] = SW_GOING_ON;
			b_time[this_b] = board_millis();
		} 
	
	}
    
    //****  GOING ON  ****************
    else if(b_state[this_b] == SW_GOING_ON)
	{
		if( board_millis() - b_time[this_b] > 10)
		{
		 	if(pin_state)
			{
			 	b_state[this_b] = SW_ON;
                //***  DO SOMETHING  ***

                button_event(this_b);

                //***********************
			}else
			{
				b_state[this_b] = SW_OFF;

			}

		}

    //****  ON  ****************
	}else if(b_state[this_b] == SW_ON)
	{
	 	if(!pin_state)
		{
			 	b_state[this_b] = SW_OFF;
		}
	}
	
    this_b++;
	if(this_b >= NUM_BUTTONS)
	{
		this_b = 0;
	}	 
	



	  

}
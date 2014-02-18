
/*
 ******************************************************************************
 * mm_example.c
 * Example of using the MM object file.
 * The example demonstrates the use of functions to create a turnout decoder.
 * The example is for MM address 1. Other decoder value addresses can be found 
 * in the table listed here http://home.arcor.de/dr.koenig/digital/verbess.htm
 ******************************************************************************
 */

/* Standard include files */
#include <avr\io.h>
#include <inttypes.h>
#include <avr\interrupt.h>
#include "mm_module.h"

/* Defines */
#define MM_SET_ADRESS 0xC0                                 /* The decoder address, in this example MM address 1 */

#define LED_BLINK_COUNT_LIMIT 65500

//Uncomment / cooment one of the the following two defines for selecting the required decoder type
//#define SPECIAL_FUNCTION_DECODER 1
#define TURNOUT_DECODER 1

/* Main routine */
int main(void)
{
   /* Declare variables for addres and data */
   uint8_t                                 MM_Address;
   uint8_t                                 MM_Data;
   uint16_t                                LedBlinkCnt;

   /* Set intial values of various used variables */
   MM_Data = 0;
   MM_Address = 0;
   LedBlinkCnt = 0;

   /* Set PortB as output */
   DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB3) | (1 << PB4);
   PORTB = 0x00;

   /* Initialize the MM module */
   MM_Module_Init();

   /* Enable the global interrupt */
   sei();

   /* Main neverending loop */
   while (1)
   {
      /* Increase LED blinkcounter, and handle blinking if counter expired */
      LedBlinkCnt++;

      if (LedBlinkCnt > LED_BLINK_COUNT_LIMIT)
      {
         /* Toggle to blink LED */
         LedBlinkCnt = 0;
         PORTB ^= (1 << PB0);
      }

      /* New data received? */
      if (MM_CheckForNewInfo(&MM_Address, &MM_Data) == MM_NEW_INFO)
      {
         /* Address valid, check if bit 4 of MC145027 data is high */
         if (MM_Address == MM_SET_ADRESS)
         {
#           ifdef SPECIAL_FUNCTION_DECODER
            /* Check if Function bit is high for special decoders */
            if (MM_Data & 0x10)
            {
               /* See Tiny2313 code for example */
            }
#           endif

#           ifdef TURNOUT_DECODER
            /* If a 'normal' function bit is present (turnout) process it */
            if (!(MM_Data & 0x10))
            {
               /* Bit 4 high for activating a turnout?? */
               if (MM_Data & 0x08)
               {
                  /* yes, process it, first remove bit 4 */
                  MM_Data -= 8;

                  /* Now depending on value, set a output */
                  switch (MM_Data)
                  {
                     case 0:
                        PORTB |= (1 << PB3);
                        break;
                     case 1:
                        PORTB |= (1 << PB4);
                        break;
                  }
               }
               else
               {
                  /* Bit 4 was low, reset outputs */
                  PORTB &= ~(1 << PB3);
                  PORTB &= ~(1 << PB4);
               }
            }
#           endif
         }
      }
   }
}

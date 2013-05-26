#include <avr/io.h>
#include <util/delay.h>

int main()
{
  // timer1: fast pwm, clear OC1B on match
  TCCR1A = (1 << COM1B1) | (1 << WGM11) | (1 << WGM10);
  // timer1: fast pwm, ctc top=OCR1A, clk/1
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);
  OCR1A = 20000; // 20ms
  OCR1B = 1500; // 1.5ms

  DDRB |= (1 << PB4); // PB4/OC1B as output

  uint16_t servoTiming; // servo timing in us

  while(1)
  {
    // slowly turns servo from one side to the other
    for(servoTiming = 1000; servoTiming < 2000; servoTiming++)
    {
      OCR1B = servoTiming;
      _delay_ms(10);
    }
    // slowly turns the servo back
    for(servoTiming = 2000; servoTiming > 1000; servoTiming--)
    {
      OCR1B = servoTiming;
      _delay_ms(10);
    }
  }

  return 0;
}

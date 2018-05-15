/*
 * ControleDeAcesso.c
 *
 * Created: 5/11/2018 10:01:14 PM
 * Author : Equipe Pizza Planet
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#define set_bit(adress,bit) (adress|=(1<<bit))
#define clr_bit(adress,bit) (adress&=~(1<<bit))
#define rd_bit(adress,bit) (adress&(1<<bit))
#define cpl_bit(adress,bit) (adress^=(1<<bit))

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) -1)
char comando;
int i = 0;

void led()
{
	if(comando == 'L')
	{
		set_bit(PORTB, PORTB5);
	}
	if(comando == 'D')
	{
		clr_bit(PORTB, PORTB5);
	}
	if(comando == 'P')
	{
		
		set_bit(PORTB, PORTB5);
		_delay_ms(1000);
		clr_bit(PORTB, PORTB5);
		_delay_ms(1000);
	}
	
}

void USART_Transmit( unsigned char data )
{
	while ( !(UCSR0A & (1<<UDRE0)));
	UDR0 = data;	
}

void MensagemInicial(){
	char msg[] = " BEM VINDO \r\n ESTE E O LAB DGT2 \r\n DIGITE SUA SENHA! \r\n";
	while(i != strlen(msg)){	
		USART_Transmit(msg[i++]);
	}
	i = 0;

}

int main(void)
{
	DDRB = 0xFF;
	PORTB = 0x00;
	
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << USBS0) | (1 << UCSZ00) | (1 << UCSZ01);
	
	UBRR0H = (BAUD_PRESCALE >> 8);
	UBRR0L = BAUD_PRESCALE;
	
	UCSR0B |= (1 << RXCIE0);
	
	sei();

	MensagemInicial();
	
	for(;;)
	{
		led();
	}
	
    while (1) 
    {
		
    }
	
}

ISR(USART_RX_vect)
{
	char ReceivedByte = UDR0;
	if(ReceivedByte == 'L' || ReceivedByte == 'D' || ReceivedByte == 'P')
	{
		comando = ReceivedByte;
	}
	
}
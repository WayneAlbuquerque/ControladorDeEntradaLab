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
#include <stdarg.h>
#define set_bit(adress,bit) (adress|=(1<<bit))
#define clr_bit(adress,bit) (adress&=~(1<<bit))
#define rd_bit(adress,bit) (adress&(1<<bit))
#define cpl_bit(adress,bit) (adress^=(1<<bit))
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) -1)

enum Estados {Aguarda, Liberacao, Configuracao}; //Estados do sistema
char senhas[7][10] = {"111111","222222","333333","444444","555555","666666"}; // Senha do sistema
char buffer[6] = ""; // Buffer para captura da senha digitada
int i = 0, liberacao = 0, Est = 0;

int TestaSenha(char senha[6]){ //Funcao para comparar a senha digitada com o banco de senhas
	for(int c = 0; c < 10; c++){ 
		if(strcmp(senha,senhas[c])==0 || strcmp(senha,"master") == 0 ){	// Testa se a senha digitada e a senha mestre ou uma do banco de senhas		
			if(strcmp(senha,"master") == 0){ // Testa se e a senha mestre
				return 2; 
			}else{
				return 1;
			}
		}
	}
	return 0;	
}

void USART_Transmit( unsigned char data ) // transmite um caracter para o serial do Atmega328p
{
	while ( !(UCSR0A & (1<<UDRE0)));
	UDR0 = data;	
}

void MensagemInicial(){ // Escreve mensagens
	char msg[] = "     BEM VINDO \r\n ESTE E O LAB DGT2 \r\n DIGITE SUA SENHA! \r\n";
	while(i != strlen(msg)){	
		USART_Transmit(msg[i++]);
	}
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
		switch(Est){
			case Aguarda: //Aguarda senha
				if(strlen(buffer)==6){ // Testa se o buffer tem tamanho 6
					Est = TestaSenha(buffer); // Atribui o proximo estado do sistema
					strcpy(buffer,""); // Limpa o buffer
				}
			break;
			case Liberacao:
					set_bit(PORTB, PORTB5);
					_delay_ms(1000);
					clr_bit(PORTB, PORTB5);
					_delay_ms(1000);
					Est = 0; // Retorna ao esta Aguarda

			break;
			case Configuracao:
					set_bit(PORTB, PORTB5);
					_delay_ms(1000);
					clr_bit(PORTB, PORTB5);
					_delay_ms(1000);
					set_bit(PORTB, PORTB5);
					_delay_ms(1000);
					clr_bit(PORTB, PORTB5);
					_delay_ms(1000);
					Est = 0; // Retorna ao esta Aguarda

			break;
		}

	}
	
    while (1) 
    {
		
    }
	
}

ISR(USART_RX_vect)
{
	char ReceivedByte = UDR0;
	char str[3];
	if(ReceivedByte){ // Testa se a variavel tem um valor se tem entra na condicao
		str[0] = ReceivedByte; //cria uma string
		str[1] = '\0';		   //com o caracter lido
		strcat(buffer,str);    // concatena str no buffer	
	}
		
}
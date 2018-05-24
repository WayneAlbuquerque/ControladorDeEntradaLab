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
char teclado[4][3] = {{'3','2','1'},{'6','5','4'},{'9','8','7'},{35,'0',42}}; //valores do teclado matricial 4x3
char senhas[7][10] = {"111111","222222"}; // Senha do sistema
int nsenhas = 2;
int p1=0, p2=0; // enderecos dos valores do teclado
char buffer[6] = ""; // Buffer para captura da senha digitada
int liberacao = 0, Est = 0;



void USART_Transmit( unsigned char data ) // transmite um caracter para o serial do Atmega328p
{
	while ( !(UCSR0A & (1<<UDRE0)));
	UDR0 = data;	
}

void print(char dados[100]){ // Funcao que escreve uma string no terminal de ate 100 caracteres
	int i = 0;
	while(i != strlen(dados)){
		USART_Transmit(dados[i++]);
	}
}

int TestaSenha(char senha[10]){ //Funcao para comparar a senha digitada com o banco de senhas
	senha[strlen(senha)-1] = '\0';
	for(int c = 0; c < nsenhas; c++){
		if(strcmp(senha,senhas[c]) == 0 || strcmp(senha,"master") == 0 ){	// Testa se a senha digitada e a senha mestre ou uma do banco de senhas
			if(strcmp(senha,"master") == 0){ // Testa se e a senha mestre
				strcpy(buffer,""); // Limpa o buffer
				return 2;
			}else{
				strcpy(buffer,""); // Limpa o buffer
				return 1;
			}
		}
	}
	strcpy(buffer,""); // Limpa o buffer
	return 0;
}

void MensagemInicial(){ // Escreve mensagens
	char msg[] = "     BEM VINDO \r\n ESTE E O LAB DGT2 \r\n DIGITE SUA SENHA! \r\n";
	int i = 0;
	while(i != strlen(msg)){	
		USART_Transmit(msg[i++]);
	}
}

void varrer(){ //funcao para varrer ligando e desligando os pinos do teclado
	p1 = 0; 
	set_bit(PORTB,PB3); //Liga Porta PB3
	_delay_ms(10);
	clr_bit(PORTB,PB3); //Desliga Porta PB3
	p1 = 1;
	set_bit(PORTB,PB4); //Liga Porta PB4
	_delay_ms(10);
	clr_bit(PORTB,PB4); //Desliga Porta PB4
	p1 = 2;
	set_bit(PORTB,PB5); //Liga Porta PB5
	_delay_ms(10);
	clr_bit(PORTB,PB5); //Desliga Porta PB5
	p1 = 3;
	set_bit(PORTC,PC0); //Liga Porta PC0
	_delay_ms(10);
	clr_bit(PORTC,PC0); //Desliga Porta PC0
}

int main(void)
{
	DDRB = 0b0000111; // seta no registrador de direcao os pinos de saida como entrada e os pinos de entrada como saida
	DDRC = 0b00000001;

	PORTB = 0b1111000; // habilita pull ups e pull downs necessarios 
	PORTC = 0b00000000;

	DDRB = 0b11111000; // Configura as direcoes reais de direcao dos pinos no registrador direcional
	DDRC = 0b00000001;

	PCICR = (1<<PCIE0);	// habilita o barramento b para interrupcoes externas
	PCMSK0 |= (1<<PCINT0); // define quais pinos serao esperadas as interrupcoes externas
	PCMSK0 |= (1<<PCINT1);
	PCMSK0 |= (1<<PCINT2);


	UCSR0B = (1 << RXEN0) | (1 << TXEN0); 
	UCSR0C = (1 << USBS0) | (1 << UCSZ00) | (1 << UCSZ01);
	
	UBRR0H = (BAUD_PRESCALE >> 8);
	UBRR0L = BAUD_PRESCALE;
	
	UCSR0B |= (1 << RXCIE0);
	
	sei();

	MensagemInicial();	

	for(;;){

		switch(Est){
			case Aguarda: //Aguarda senha
				if(buffer[strlen(buffer)-1]==35){ // Testa se o buffer tem tamanho 6
					Est = TestaSenha(buffer); // Atribui o proximo estado do sistema					
				}
			break;
			case Liberacao:
					for(int t = 0; t < 6; t++){ //liga o led durante 6 segundos
						set_bit(PORTB, PORTB7);
						_delay_ms(1000);
					}
					clr_bit(PORTB, PORTB7);
					MensagemInicial();
					Est = 0; // Retorna ao esta Aguarda

			break;
			case Configuracao:
					print("[1] - Cadastra senha \r\n");
					print("[2] - Deleta Senha \r\n");
					print("[3] - Log de Entradas \r\n");
					Est = 0; // Retorna ao esta Aguarda

			break;
		}
		varrer();
	}

}

ISR(USART_RX_vect){ // rotina quando ocorre a interrupcao na serial
	char ReceivedByte = UDR0;
	char str[3];
	if(ReceivedByte){ // Testa se a variavel tem um valor se tem entra na condicao
		if(ReceivedByte == 13){
			str[0] = 35;          //Alimenta String com o sinal de fim
			str[1] = '\0';		   //com o caracter lido
			strcat(buffer,str);    // concatena str no buffer
		}else{
			str[0] = ReceivedByte; //Alimenta a String com a letura da serial
			str[1] = '\0';		   //com o caracter lido
			strcat(buffer,str);    // concatena str no buffer
		}	
	}		
}

ISR(PCINT0_vect){ // rotina quando corre interrupcao pelo teclado
	char str[3];
	if (rd_bit(PINB,PINB0)){ // testa se a interrupcao foi feita no pino 0 do barramento b
		p2 = 0;
		str[0] = teclado[p1][p2]; // salva o valor da tecla na string auxiliar
		str[1] = '\0';
		print(str);	// escreve na serial o valor da string
		while(rd_bit(PINB,PINB0)); // para o fluxo do codigo ate que o botao seja liberado
	}
	if (rd_bit(PINB,PINB1)){ // testa se a interrupcao foi feita no pino 1 do barramento b
		p2 = 1;
		str[0] = teclado[p1][p2]; // salva o valor da tecla na string auxiliar
		str[1] = '\0';
		print(str); // escreve na serial o valor da string
		while(rd_bit(PINB,PINB1)); // para o fluxo do codigo ate que o botao seja liberado
	}
	
	if (rd_bit(PINB,PINB2)){ // testa se a interrupcao foi feita no pino 2 do barramento b
		p2 = 2;
		str[0] = teclado[p1][p2]; // salva o valor da tecla na string auxiliar
		str[1] = '\0';
		print(str); // escreve na serial o valor da string
		while(rd_bit(PINB,PINB2)); // para o fluxo do codigo ate que o botao seja liberado
	}	
}


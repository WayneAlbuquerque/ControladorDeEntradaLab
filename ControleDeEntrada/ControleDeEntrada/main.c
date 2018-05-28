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
#include <stdio.h>
#include <stdarg.h>
#define set_bit(adress,bit) (adress|=(1<<bit))
#define clr_bit(adress,bit) (adress&=~(1<<bit))
#define rd_bit(adress,bit) (adress&(1<<bit))
#define cpl_bit(adress,bit) (adress^=(1<<bit))
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) -1)

enum Estados {Aguarda, Liberacao, Configuracao, Cadastro, Deleta, LogDeEntradas}; //Estados do sistema
char teclado[4][3] = {{'3','2','1'},{'6','5','4'},{'9','8','7'},{35,'0',42}}; //valores do teclado matricial 4x3 espelhados devido a montagem eletronica feita
char senhas[16][7] = {"","","","","","","","","","","","","","","",""}; // Senha do sistema
char userIDs[16][5] = {"0000","0001","0010","0011","0100","0101","0110","0111","1000","1001","1010","1011","1100","1101","1110","1111"}; // Identificadores de usuarios
int nsenhas = 0;
int p1=0, p2=0; // enderecos dos valores do teclado
char buffer[10] = "",ConfirmaSenha[10]="",DeletID[5]=""; // Buffer para captura da senha digitada
int liberacao = 0, Est = 0, FP=0, FC=0, FS=0, indexDeletID=-1,FD = 0;



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
	while(1){
		switch(FS){
			case 0:
				for(int c = 0; c < nsenhas; c++){
					if(strcmp(senha,senhas[c]) == 0){	// Testa se a senha digitada e uma do banco de senhas
							strcpy(buffer,""); // Limpa o buffer
							FP=0;
							FS = 0;
							return 1;
					}
				}
				FS = 1;
				//print("Senha incorreta1");
				break;
			case 1:
				if(strcmp(senha,"171294") == 0){ // Testa se e a senha mestre
					strcpy(buffer,""); // Limpa o buffer
					if(Est!=2){
						FP=0;
					}
					FS = 0;
					return 2;
				}
				FS = 2;
				//print("Senha incorreta2");
				break;
			case 2:
				//print("Senha incorreta");
				strcpy(buffer,""); // Limpa o buffer
				//if(Est!=0){
				FP=0;
				FS = 0;
				//}
				return 0;
				break;
		}
	}
	return 0;
}

void Mensagem(){ // Escreve mensagens
	char msg[100];
	if(Est==0){
		strcpy(msg,"\r\n     BEM VINDO \r\n ESTE E O LAB DGT2 \r\n DIGITE SUA SENHA! \r\n");
	}else if(Est == 2){
		strcpy(msg,"\r\n[1] - Cadastra senha \r\n[2] - Deleta Senha \r\n[3] - Log de Entradas \r\n[4] - Sair \r\n");
	}else if(Est == 3){
		sprintf(msg,"Cadastro, seu ID e %s \r\nNova senha(ex 3578951#): ",userIDs[nsenhas]);
	}else if(Est == 4){
		strcpy(msg,"\r\nInforme o ID que perdera o acesso:\r\n");
	}else if(Est == 5){
		strcpy(msg,"\r\n[3] - Log de Entradasa Senha \r\n");
	}
	int i = 0;
	while(i != strlen(msg)){
		USART_Transmit(msg[i++]);
	}
	FP = 1;	
}

int Master(char comando[10]){	
	if(comando[0] == '1'){
		strcpy(buffer,"");
		FP= 0;
		return 3;
	}else if (comando[0] == '2'){
		strcpy(buffer,"");
		FP= 0;
		return 4;
	}else if (comando[0] == '3'){
		strcpy(buffer,"");
		FP= 0;
		return 5;
	}else if (comando[0]=='4'){
		strcpy(buffer,"");
		FP= 0;
		return 0;
	}
	strcpy(buffer,"");
	FP= 0;
	return 0;
}

int cadastro(char senha[10]){
	switch(FC){
		case 0:
			if(strlen(senha)!=7){
				print("a senha deve ter 7 digitos terminando com #");
				strcpy(buffer,"");
				}else{
				strcpy(ConfirmaSenha,buffer);
				strcpy(buffer,"");
				print("Confirma senha");
				print(ConfirmaSenha);
				print("? \r\n[1]- sim, [2] - nao\r\n");
				FC = 1;
			}
			break;
		case 1:
			if(buffer[0] == '1'){
				ConfirmaSenha[strlen(ConfirmaSenha)-1] = '\0';
				strcpy(senhas[nsenhas],ConfirmaSenha);
				print("\r\nSenha cadastrada com sucesso");
				nsenhas++;
				strcpy(buffer,"");
				strcpy(ConfirmaSenha,"");
				FC = 0;
				FP = 0;
				return 2;
			}else if(buffer[0] == '2'){
				strcpy(buffer,"");
				strcpy(ConfirmaSenha,"");
				FC = 0;
				FP = 0;
			}				
			break;
	}
	return 3;	
}

void Sort(){
	char senhasTmp[16][7];
	char userIDsTmp[16][5];
	int j = 0;
	char info[20];
	for (int i = 0; i<15; i++){
		if(i!=indexDeletID){
			strcpy(senhasTmp[i],senhas[j]);
			strcpy(userIDsTmp[i],userIDs[j]);
			j++;
			}else{
			j++;
			strcpy(senhasTmp[i],senhas[j]);
			strcpy(userIDsTmp[i],userIDs[j]);
			j++;
		}
	}
	strcpy(senhasTmp[15],"");
	strcpy(userIDsTmp[15],userIDs[indexDeletID]);

	for(int i = 0;i<16;i++){
		strcpy(senhas[i],senhasTmp[i]);
		strcpy(userIDs[i],userIDsTmp[i]);
		sprintf(info,"ID: %s - Senha: %s\r\n",userIDs[i],senhas[i]);
		print(info);
	}
	nsenhas--;
	print("Teste sort \r\n");
}

int Deletar(char ID[10]){
	strcpy(DeletID,ID);
	DeletID[strlen(DeletID)-1] = '\0';
	switch(FD){
		case 0:
		if(strlen(ID)!=5){
			print("o ID deve ter 5 digitos terminando com #");
			strcpy(buffer,"");
		}else{
			for(int c = 0; c < 16; c++){
				if(strcmp(DeletID,userIDs[c]) == 0){	// Testa se a senha digitada e uma do banco de senhas
					//strcpy(buffer,""); // Limpa o buffer
					indexDeletID = c;
					//FS = 0;
				}
			}
		}
		if(strcmp(senhas[indexDeletID],"")==0||strcmp(buffer,"")==0){
			print("Id sem Cadastro");
			indexDeletID = -1;
			strcpy(DeletID,"");
			strcpy(buffer,"");
			FP=0;
			return 2;
		}else{
			//strcpy(ConfirmaSenha,buffer);
			strcpy(buffer,"");
			print("Confirma ID ");
			print(DeletID);
			print("? \r\n[1]- sim, [2] - nao\r\n");
			FD = 1;	
		}	
		break;
		case 1:
		if(buffer[0] == '1'){
			/*ConfirmaSenha[strlen(ConfirmaSenha)-1] = '\0';
			strcpy(senhas[nsenhas],ConfirmaSenha);
			print("\r\nSenha cadastrada com sucesso");
			nsenhas++;*/
			Sort();
			strcpy(buffer,"");
			strcpy(DeletID,"");
			FD = 0;
			FP = 0;
			return 2;
			}else if(buffer[0] == '2'){
			strcpy(buffer,"");
			strcpy(DeletID,"");
			FD = 0;
			FP = 0;
		}
		break;
	}
	return 4;
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

	Mensagem();	

	for(;;){

		switch(Est){
			case Aguarda: //Aguarda senha
				if(FP!=1){
					Mensagem();
				}
				if(buffer[strlen(buffer)-1]==35){ // Testa se o buffer tem o caracter finalizador
					Est = TestaSenha(buffer); // Atribui o proximo estado do sistema					
				}
			break;
			case Liberacao:
					for(int t = 0; t < 6; t++){ //liga o led durante 6 segundos
						set_bit(PORTB, PORTB7);
						_delay_ms(1000);
					}
					clr_bit(PORTB, PORTB7);
					//MensagemInicial();
					Est = 0; // Retorna ao esta Aguarda

			break;
			case Configuracao:
					//print(ops);
					if(FP!=1){
						Mensagem();
					}
					if(strlen(buffer)>0){
						//print(buffer);
						Est = Master(buffer); // Retorna ao esta Aguarda
					}
			break;
			case Cadastro:
				if(FP!=1){
					Mensagem();
				}
				if(buffer[strlen(buffer)-1]==35||FC == 1){
					Est = cadastro(buffer);
					//FP = 0;
				}
			break;
			case Deleta:
				if(FP!=1){
					Mensagem();
				}
				if(buffer[strlen(buffer)-1]==35||FD == 1){
					Est = Deletar(buffer);
					//FP = 0;
				}
			break;
			case LogDeEntradas:
				if(FP!=1){
					Mensagem();
				}
				Est = 2;
				FP = 0;
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
		if(teclado[p1][p2]==35){
			print(" \r\n");
		}
		strcat(buffer,str);
		while(rd_bit(PINB,PINB0)); // para o fluxo do codigo ate que o botao seja liberado
	}

	if (rd_bit(PINB,PINB1)){ // testa se a interrupcao foi feita no pino 1 do barramento b
		p2 = 1;
		str[0] = teclado[p1][p2]; // salva o valor da tecla na string auxiliar
		str[1] = '\0';
		print(str); // escreve na serial o valor da string
		if(teclado[p1][p2]==35){
			print(" \r\n");
		}
		strcat(buffer,str);
		while(rd_bit(PINB,PINB1)); // para o fluxo do codigo ate que o botao seja liberado
	}
	
	if (rd_bit(PINB,PINB2)){ // testa se a interrupcao foi feita no pino 2 do barramento b
		p2 = 2;
		str[0] = teclado[p1][p2]; // salva o valor da tecla na string auxiliar
		str[1] = '\0';
		print(str); // escreve na serial o valor da string
		if(teclado[p1][p2]==35){
			print(" \r\n");
		}
		strcat(buffer,str);
		while(rd_bit(PINB,PINB2)); // para o fluxo do codigo ate que o botao seja liberado
	}	
}


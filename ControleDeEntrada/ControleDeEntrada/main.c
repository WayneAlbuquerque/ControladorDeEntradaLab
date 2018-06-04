/*
 * ControleDeAcesso.c
 *
 * Created: 5/11/2018 10:01:14 PM
 * Author : Equipe Pizza Planet
 */ 

#define F_CPU 8000000UL
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
//#define USART_BAUDRATE 9600
//#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) -1)
#define LCD_Dir  DDRB			/* Define LCD data port direction */
#define LCD_Port PORTB			/* Define LCD data port */
#define RS PB0				/* Define Register Select pin */
#define EN PB1 				/* Define Enable signal pin */

enum Estados {Aguarda, Liberacao, Configuracao, Cadastro, Deleta, LogDeEntradas}; //Estados do sistema
char teclado[4][3] = {{'3','2','1'},{'6','5','4'},{'9','8','7'},{35,'0',42}}; //valores do teclado matricial 4x3 espelhados devido a montagem eletronica feita
char senhas[16][7] = {"","","","","","","","","","","","","","","",""}; // Senha do sistema
char userIDs[16][5] = {"0000","0001","0010","0011","0100","0101","0110","0111","1000","1001","1010","1011","1100","1101","1110","1111"}; // Identificadores de usuarios
char Tela[7][16] = {"ESTE E O LAB DGT","* Opcoes","[1] - Cadastra","[2] - Deleta","[3] - Log","[4] - Sair",""};
int nsenhas = 0;
int p1=0, p2=0; // enderecos dos valores do teclado
char buffer[10] = "",ConfirmaSenha[10]="",DeletID[5]=""; // Buffer para captura da senha digitada
int liberacao = 0, Est = 0, FP=0, FC=0, FS=0, indexDeletID=-1,FD = 0,indexTela=0,cont = 0;

void LCD_Command(unsigned char cmnd){
	LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0); /* sending upper nibble */
	LCD_Port &= ~ (1<<RS);		/* RS=0, command reg. */
	LCD_Port |= (1<<EN);		/* Enable pulse */
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_us(200);
	LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4);  /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}


void LCD_Char( unsigned char data )
{
	LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0); /* sending upper nibble */
	LCD_Port |= (1<<RS);		/* RS=1, data reg. */
	LCD_Port|= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_us(200);
	LCD_Port = (LCD_Port & 0x0F) | (data << 4); /* sending lower nibble */
	LCD_Port |= (1<<EN);
	_delay_us(1);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(2);
}

void LCD_Init (void)			/* LCD Initialize function */
{
	LCD_Dir = 0xFF;			/* Make LCD port direction as o/p */
	_delay_ms(20);			/* LCD Power ON delay always >15ms */	
	LCD_Command(0x02);		/* send for 4 bit initialization of LCD  */
	LCD_Command(0x28);              /* 2 line, 5*7 matrix in 4-bit mode */
	LCD_Command(0x0c);              /* Display on cursor off*/
	LCD_Command(0x06);              /* Increment cursor (shift cursor to right)*/
	LCD_Command(0x01);              /* Clear display screen*/
	_delay_ms(2);
}


void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_Clear(){
	LCD_Command (0x01);		/* Clear display */
	_delay_ms(2);
	LCD_Command (0x80);		/* Cursor at home position */
}

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

void print_LCD(char L1[16],char L2[16]){
	LCD_Clear();
	if(strcmp(L1,"") != 0){
		LCD_String(L1);
	}
	if(strcmp(L2,"") != 0){
		LCD_Command(0xC0);
		LCD_String(L2);
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
	//char msg[100];
	if(Est==0){
		//strcpy(msg,"\r\n     BEM VINDO \r\n ESTE E O LAB DGT2 \r\n DIGITE SUA SENHA! \r\n");
		indexTela = 0;
	}else if(Est == 2){
		//strcpy(msg,"\r\n[1] - Cadastra senha \r\n[2] - Deleta Senha \r\n[3] - Log de Entradas \r\n[4] - Sair \r\n");
		indexTela=1;
	}else if(Est == 3){
		//sprintf(msg,"Cadastro, seu ID e %s \r\nNova senha(ex 3578951#): ",userIDs[nsenhas]);
		indexTela = 6;
	}else if(Est == 4){
		//strcpy(msg,"\r\nInforme o ID que perdera o acesso:\r\n");
		indexTela = 6;
	}else if(Est == 5){
		//strcpy(msg,"\r\n[3] - Log de Entradas\r\n");
		indexTela = 6;
	}
	//int i = 0;
	//while(i != strlen(msg)){
	//	USART_Transmit(msg[i++]);
	//}
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
				print_LCD("senha invalida","7 digitos");
				FP = 0;
				strcpy(buffer,"");
				}else{
					strcpy(ConfirmaSenha,buffer);
					strcpy(buffer,"");	
					//strcpy(Tela[6],"");				
					//print("\r\nConfirma senha");
					//print(ConfirmaSenha);
					//print("? \r\n[1]- sim, [2] - nao\r\n");
					//strcat(Tela[6],"Confirma ");
					//strcat(Tela[6],ConfirmaSenha);
					//strcat(Tela[6],"?");
					//sprintf(Tela[6],"Confirma %s?",ConfirmaSenha);
					print_LCD("Confirmar?","[1]-sim,[2]-nao");

					FC = 1;
				}
			break;
		case 1:
			//sprintf(Tela[6],"Confirma %s?",ConfirmaSenha);
			
			if(buffer[0] == '1'){
				ConfirmaSenha[strlen(ConfirmaSenha)-1] = '\0';
				strcpy(senhas[nsenhas],ConfirmaSenha);
				//print("\r\nSenha cadastrada com sucesso");
				print_LCD("Senha cadastrada","");
				nsenhas++;
				strcpy(buffer,"");
				strcpy(ConfirmaSenha,"");
				FC = 0;
				FP = 0;
				_delay_ms(500);
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
	//char info[20];
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
		//sprintf(info,"ID: %s - Senha: %s\r\n",userIDs[i],senhas[i]);
		//print(info);
	}
	nsenhas--;
}

int Deletar(char ID[10]){
	strcpy(DeletID,ID);
	DeletID[strlen(DeletID)-1] = '\0';
	switch(FD){
		case 0:
		if(strlen(ID)!=5){
			//print("o ID deve ter 5 digitos terminando com #");
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
			print_LCD("Id sem Cadastro","");
			_delay_ms(1000);
			indexDeletID = -1;
			strcpy(DeletID,"");
			strcpy(buffer,"");
			FP=0;
			return 2;
		}else{
			//strcpy(ConfirmaSenha,buffer);
			strcpy(buffer,"");
			//print("Confirma ID ");
			//print(DeletID);
			//print("? \r\n[1]- sim, [2] - nao\r\n");
			sprintf(Tela[6],"Confirma %s?",DeletID);
			print_LCD(Tela[6],"[1]-sim,[2]-nao");
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
	set_bit(PORTC,PC3); //Liga Porta PB3
	_delay_ms(10);
	clr_bit(PORTC,PC3); //Desliga Porta PB3
	p1 = 1;
	set_bit(PORTC,PC4); //Liga Porta PB4
	_delay_ms(10);
	clr_bit(PORTC,PC4); //Desliga Porta PB4
	p1 = 2;
	set_bit(PORTC,PC5); //Liga Porta PB5
	_delay_ms(10);
	clr_bit(PORTC,PC5); //Desliga Porta PB5
	p1 = 3;
	set_bit(PORTD,PD3); //Liga Porta PC0
	_delay_ms(10);
	clr_bit(PORTD,PD3); //Desliga Porta PC0
}

int main(void)
{
	DDRC = 0b0000111; // seta no registrador de direcao os pinos de saida como entrada e os pinos de entrada como saida
	DDRD = 0b00001100;

	PORTC = 0b1111000; // habilita pull ups e pull downs necessarios 
	PORTD = 0b00000000;

	DDRC = 0b11111000; // Configura as direcoes reais de direcao dos pinos no registrador direcional
	DDRD = 0b00001100;

	PCICR = (1<<PCIE1);	// habilita o barramento b para interrupcoes externas
	PCMSK1 |= (1<<PCINT8); // define quais pinos serao esperadas as interrupcoes externas
	PCMSK1 |= (1<<PCINT9);
	PCMSK1 |= (1<<PCINT10);

/*
	UCSR0B = (1 << RXEN0) | (1 << TXEN0); 
	UCSR0C = (1 << USBS0) | (1 << UCSZ00) | (1 << UCSZ01);
	
	UBRR0H = (BAUD_PRESCALE >> 8);
	UBRR0L = BAUD_PRESCALE;
	
	UCSR0B |= (1 << RXCIE0);
*/
	LCD_Init();			/* Initialization of LCD*/
	
	sei();
	Mensagem();	
	print_LCD(Tela[indexTela],"");

	for(;;){
		
		switch(Est){
			case Aguarda: //Aguarda senha
				if(FP!=1){					
					Mensagem();
					print_LCD(Tela[indexTela],"");
				}
				if(buffer[strlen(buffer)-1]==35){ // Testa se o buffer tem o caracter finalizador
					Est = TestaSenha(buffer); // Atribui o proximo estado do sistema					
				}
				break;
			case Liberacao:
					set_bit(PORTD, PORTD2);
					_delay_ms(3000);
					clr_bit(PORTD, PORTD2);
					Est = 0; // Retorna ao esta Aguarda

				break;
			case Configuracao:
					if(FP!=1){
						Mensagem();
						print_LCD(Tela[indexTela],Tela[indexTela+1]);
						indexTela++;
					}
					if(strlen(buffer)>0){
						//print(buffer);
						Est = Master(buffer); // Retorna ao esta Aguarda
					}
				break;
			case Cadastro:
				if(FP!=1){
					Mensagem();
					sprintf(Tela[6],"Seu ID e %s: ",userIDs[nsenhas]);
					print_LCD(Tela[6],"");
				}
				if(buffer[strlen(buffer)-1]==35||FC == 1){
					Est = cadastro(buffer);
					//FP = 0;
				}
				break;
			case Deleta:
				if(FP!=1){
					Mensagem();
					sprintf(Tela[6],"Informe ID");
					print_LCD(Tela[indexTela],"");
				}
				if(buffer[strlen(buffer)-1]==35||FD == 1){
					Est = Deletar(buffer);
					//FP = 0;
				}
				break;
			case LogDeEntradas:
				if(FP!=1){
					Mensagem();
					print_LCD("Log Entradas","Precione *");
				}
				//for(int x=0;x<16;x++){
					//char LOG[40];
					//sprintf(Tela[6],"ID: %s - Senha: %s \r\n",userIDs[x], senhas[x]);
					//print("ID: ");
					//print(userIDs[x]);
					//print(" - Senha: ");
					//print(senhas[x]);
					//print("\r\n");
					//_delay_ms(10);
				//}
				if(buffer[strlen(buffer)-1]==35){
					strcpy(buffer,"");
					cont = 0;
					Est = 2;
					FP = 0;
				}
				break;
		}
		varrer();
	}

}
/*
ISR(USART_RX_vect){ // rotina quando ocorre a interrupcao na serial
	char ReceivedByte = UDR0;
	char str[3];
	if(ReceivedByte){ // Testa se a variavel tem um valor se tem entra na condicao
		if(ReceivedByte == 13){
			str[0] = 35;          //Alimenta String com o sinal de fim
			str[1] = '\0';		   //com o caracter lido
			print(str);	// escreve na serial o valor da string
			strcat(buffer,str);    // concatena str no buffer
		}else{
			str[0] = ReceivedByte; //Alimenta a String com a letura da serial
			str[1] = '\0';		   //com o caracter lido
			print(str);	// escreve na serial o valor da string
			strcat(buffer,str);    // concatena str no buffer
		}	
	}		
}*/

ISR(PCINT1_vect){ // rotina quando corre interrupcao pelo teclado
	char str[3];
	if (rd_bit(PINC,PINC0)){ // testa se a interrupcao foi feita no pino 0 do barramento b
		p2 = 0;
		str[0] = teclado[p1][p2]; // salva o valor da tecla na string auxiliar
		str[1] = '\0';
		//print(str);	// escreve na serial o valor da string
		//if(teclado[p1][p2]==35){
			//print(" \r\n");
		//}
		strcat(buffer,str);
		print_LCD(Tela[indexTela],buffer);
		while(rd_bit(PINC,PINC0)); // para o fluxo do codigo ate que o botao seja liberado
	}

	if (rd_bit(PINC,PINC1)){ // testa se a interrupcao foi feita no pino 1 do barramento b
		p2 = 1;
		str[0] = teclado[p1][p2]; // salva o valor da tecla na string auxiliar
		str[1] = '\0';
		//print(str); // escreve na serial o valor da string
		//if(teclado[p1][p2]==35){
		//	print(" \r\n");
		//}
		strcat(buffer,str);
		print_LCD(Tela[indexTela],buffer);
		while(rd_bit(PINC,PINC1)); // para o fluxo do codigo ate que o botao seja liberado
	}
	
	if (rd_bit(PINC,PINC2)){ // testa se a interrupcao foi feita no pino 2 do barramento b
		char User[10],Senha[13];
		p2 = 2;
		str[0] = teclado[p1][p2]; // salva o valor da tecla na string auxiliar
		str[1] = '\0';
		
		if(teclado[p1][p2]==42 && Est==Configuracao){
			if(indexTela==5){
				indexTela = 2;
			}

			print_LCD(Tela[indexTela],Tela[indexTela+1]);
			indexTela++;
		}else if(teclado[p1][p2]==42 && Est==LogDeEntradas){
			if(cont==16){
				cont = 0;
			}
			sprintf(User,"User-%s",userIDs[cont]);
			sprintf(Senha,"Senha-%s",senhas[cont]);
			print_LCD(User,Senha);
			cont++;
		}else{
			//print(str); // escreve na serial o valor da string
			strcat(buffer,str);
			print_LCD(Tela[indexTela],buffer);
		}
		while(rd_bit(PINC,PINC2)); // para o fluxo do codigo ate que o botao seja liberado
	}	
}


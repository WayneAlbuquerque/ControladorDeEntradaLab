# Controlador de entrada de Laboratório
## Trabalho da disciplina de Eletronica Digital 2

#### Objetivo
Criar um programa em C para Atmega328p que controle a entrada de pessoas em 
um laboratorio, pedindo senhas e mostrando num display informações.

#### Requisitos
 * Controlar o acesso, permitido somente por senha de 6 dígitos, caso 
acerte, vinda da serial
 * Fazer o software que cadastre um pequeno Banco de dados no micro, contendo
UserID, Senha, Numero de acessos daquele UserID.
 * Quando a senha for correta, deve acionar a entrada por 6 segundos (simbolizada por um LED) e escrever no LCD “Bem-vindo Usuario”
 * Deve ser simulado no Proteus.
 * Se digitar um código especial “*144#”, aparece no LCD quantaspessoas já acessaram o laboratório, por um tempo, e depois volta ao menu inicial. Também envia pela serial a mesma informação. 
 * Usar uma biblioteca de acesso ao LCD, disponível no Moodle, na página da disciplina


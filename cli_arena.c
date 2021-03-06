#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cli_arena.h"
#include "GUI.h"


int gui;
int socket_Identification;
int socket_Combat;
char buf[MAX_BUF];
struct sockaddr_in dir_serv_id;
struct sockaddr_in dir_serv_comb;

char USERNAME[MAX_BUF];
char CLASS;
int MyHP;
int EnemyHP;

void createSocket(){
	// Crear socket UDP
	if((socket_Identification = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("Error creating the socket\n");
		exit(1);
	}
	
	// Componer la direccion del servidor para su uso posterior
	memset(&dir_serv_id, 0, sizeof(dir_serv_id));	
	dir_serv_id.sin_family = AF_INET;
	dir_serv_id.sin_port = htons(PORT);
	
	if(inet_aton(SERVER,&dir_serv_id.sin_addr) <= 0)
	{
		fprintf(stderr,"Error en la direccion IP: %s\n", SERVER);
		exit(1);
	}
}

//interpreta un comando, poniendo caracter de fin de string al final de nuestro comando
void interpretString(){
	int i;
	for(i=0; i<MAX_BUF; i++){
		if(buf[i]=='#'){
		  	buf[i]='\0';
		}
	}
}


void recieveCommand(){
	int tam_dir = sizeof(dir_serv_id);
	while(1){
		if((recvfrom(socket_Identification, buf, MAX_BUF, 0, (struct sockaddr *) &dir_serv_id, &tam_dir)) < 0){
			perror("Error reciving command.\n");
			exit(1);
		}else{
			interpretString();
			char error_command[2];
			strncpy(error_command, buf, 2);
			if(strcmp(error_command, COMMANDS[COM_ER])==0){
				switch (buf[3]){
					case 1:
						printf("Error ocurred sending your username, try again.\n");
						selectUsername();
					case 2:
						printf("Error ocurred sending your role, try again.\n");
						selectRole();
					case 3:
						printf("Error ocurred sending your attack, try again.\n");
						printf("Unknown method for attack.\n");
					default:
						printf("Unknown error detected.\n");
						exit(1);
				}
			}else{
				break;
			}
		}
	}
}

int waitOK(){
	recieveCommand();
	if(strcmp(buf, COMMANDS[COM_OK])==0){
		return 0;
	}else{
		return(-1);
	}
}

void waitStart(){//leer iniciativa
	while(1){
		if (gui==1)
			printWaitingOponent();
		else
			printf("Waiting for a worthy oponent...\n");
		recieveCommand();
		char command[4];
		strncpy(command, buf, 4);//metemos solo el comando
		if(strcmp(command, COMMANDS[COM_STRT])==0){
			printf("Combat beggins!\n");
			break;
		}
	}
}

int sendUsername(char* username){
	sprintf(buf, "%s_%s#", COMMANDS[COM_USER], username);
	buf[strlen(buf)]=0;//quitar salto de linea sobrante
	if(sendCommand() < 0)
		return(-1);
	else 
		return 0;
}

int sendRole(char class){
	sprintf(buf, "%s_%c#", COMMANDS[COM_ROLE], class);	
	if(sendCommand() < 0)
		return(-1);
	else
		return 0;
}

int sendAttack(char attack_type){
	sprintf(buf, "%s_%c#", COMMANDS[COM_ATCK], attack_type);
	if(sendCommand() < 0)
		return(-1);
	else
		return 0;	
}

int sendOK(){
	sprintf(buf, "%s#", COMMANDS[COM_OK]);
	if(sendCommand() < 0)
		return(-1);
	else
		return 0;
}

int sendCommand(){
	if(sendto(socket_Identification, buf, strlen(buf), 0,(struct sockaddr *) &dir_serv_id, sizeof(dir_serv_id)) < 0){
		perror(MSG_ERR[SEND_ERROR]);
		return(-1);
	}else{
		return 0;
	}
}

int recieveCombatState(){
	char command[4];
	if (gui==1)
		printWaitingTurn();
	else
		printf("Wait for your turn...\n");
	recieveCommand();
	char fullMessage[MAX_BUF];
	strcpy(fullMessage,buf);
	strncpy(command, buf, 4);
	if(strcmp(command, COMMANDS[COM_STAT])==0){
		MyHP=(buf[5]- '0')*100+(buf[6]- '0')*10+(buf[7]- '0');//el menos cero lo convierte en un int
		EnemyHP = (buf[9]- '0')*100+(buf[10]- '0')*10+(buf[11]- '0');
		return 0;
	}else if(strcmp(command, COMMANDS[COM_VICT])==0){
		char winner = buf[5];
		if(winner=='1'){
			return 1;
		}else{
			return 2;
		}
	}else{
		return -1;
	}
}

void selectUsername(){
	char username[MAX_BUF];  
	printf("Enter a name for your hero:\n");
	fgets(username,MAX_BUF,stdin);
	while(1){
		while ((int)strlen(username)<2){
			printf("Enter a non empty name please:\n");
			fgets(username,MAX_BUF,stdin);		
		}    
		username[strlen(username)-1]='\0';
		if(sendUsername(username)>=0){
			if (waitOK()==OK){
				strcpy(USERNAME, username);
				break;
			}else{
				printf("Confirmation not recieved, retry please.\n");
			}
		}else{
			printf("%s \n", MSG_ERR[4]);
		}
	}
}

void selectRole(){
	if (gui==1)
		printRoleSelection();
	char class;
	printf("Enter class: (Soldier(S)/ Wizard(W)/ Ranger(R))\n");
	while(1){
		char class[MAX_BUF];
		fgets(class,MAX_BUF,stdin);
		char answer = toupper(class[0]); 
		if(strlen(class)==2 && (answer=='S' || answer=='W' || answer=='R')){
			if(sendRole(answer)>=0){
				if (waitOK()==OK){
					CLASS=answer;
					break;
				}else{
					perror("Confirmation not recieved.\n");
				}
			}else{
				if (gui==1)
					printRoleSelection();
				perror("Class can't be sent.\n");
			}
		}else{
			if (gui==1)
				printRoleSelection();
			printf("Invalid class, accepted values: S / W / R.\n");
		}
	}
}

void combat (){
	char attack_type[MAX_BUF];
	/*Si este cliente no tiene la iniciativa, mandará "OK" y esperará a su turno.
	  Si este cliente ha de comenzar el combate, se salta el envio de OK y entra
	  directamente en el loop del combate */
	int Vict;
	while((Vict=recieveCombatState())==0){
		if (gui==1)
			printCombat(CLASS, MyHP, EnemyHP);
		else 
			printf("Your HP: %d  ///  Enemy's HP:% d\n", MyHP, EnemyHP);
		printf("Your turn, select attack: Melee(M) / Sorcery(S) / Ranged(R)\n");
		while(1){
			fgets(attack_type,MAX_BUF,stdin);
			char answer = toupper(attack_type[0]); 
			if(strlen(attack_type)==2 && (answer=='M' || answer=='S' || answer=='R')){
				//Mandar ataque
				if(sendAttack(answer)>=0){
					break;
				}else{
					if (gui==1)
						printCombat(CLASS, MyHP, EnemyHP);
					perror("Attack can't be sent.\n");
				}
			}else{
				if (gui==1)
					printCombat(CLASS, MyHP, EnemyHP);
				printf("Invalid attack type, accepted values: M / S / R.\n");
			}
		}		
	}
	if (gui==1)
		printVict(Vict);
	else{
		if (Vict==1)
			printf("Victory!\n");
		else 
			printf("Defeated...\n");
	}
}

int main(int argc, char *argv[]){
	clearScreen();
	printf(" _________________________________________________________________________ \n");
	printf("|    This game uses a very basic graphical menu to show some options.     |\n");
	printf("|           You will need an ANSI terminal to run it properly.            |\n");
	printf("| Running the game at full screen is recommended for proper visualization.|\n");
	printf("|                                                                         |\n");
	printf("|   Would you like to disable graphics, although they are pretty cool?.   |\n");
	printf("|        Everything except (Y/y) will be considered as a negative.        |\n");
	printf("|_________________________________________________________________________|\n");
	char question[MAX_BUF];
	fgets(question,MAX_BUF,stdin);
	char answer = toupper(question[0]); 
	if(answer=='Y')
		gui=0;
	else
		gui=1;	
	
	//identification
	if (gui==1)
		printWelcome();
	createSocket();
	selectUsername();
	selectRole();

	waitStart();

	combat();
	
}

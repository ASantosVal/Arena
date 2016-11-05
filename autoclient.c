#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#include "cli_arena.h"


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
			perror("Error reciving username.\n");
			exit(1);
		}else{
			interpretString();
			break;
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
		printf("Waiting oponent...\n");
		recieveCommand();
		char command[4];
		strncpy(command, buf, 4);//metemos solo el comando
		if(strcmp(command, COMMANDS[COM_STRT])==0){
			printf("Combat beggins.\n");
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
	printf("Waiting turn.\n");
	recieveCommand();
	char fullMessage[MAX_BUF];
	strcpy(fullMessage,buf);
	strncpy(command, buf, 4);
	if(strcmp(command, COMMANDS[COM_STAT])==0){
		MyHP=(buf[5]- '0')*100+(buf[6]- '0')*10+(buf[7]- '0');//el menos cero lo comvierte en un int
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
	while(1){
		srand(time(NULL));
		int randNum = (rand() % 10000) + 1;
		sprintf(username, "AUTOCLIENT_%d",randNum);
		if(sendUsername(username)>=0){
			if (waitOK()==OK){
				strcpy(USERNAME, username);
				break;
			}else{
				printf("Confirmation not recieved, retrying.\n");
			}
		}else{
			printf("%s \n", MSG_ERR[4]);
		}
	}
}

void selectRole(){
	while(1){
		srand(time(NULL));
		char answer;
		int randNum = (rand() % 3) + 1;
		switch(randNum){
			case 1:
				answer = 'S';
				break;
			case 2:
				answer = 'R';
				break;
			case 3:
				answer = 'W';
				break;
		}
		printf("Class %c selected.\n",answer);	
		if(sendRole(answer)>=0){
			if (waitOK()==OK){
				CLASS=answer;
				break;
			}else{
				perror("Confirmation not recieved.\n");
			}
		}else{
			perror("Class can't be sent.\n");
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
		printf("Client's HP: %d  ///  Oponents's HP:% d\n", MyHP, EnemyHP);
		while(1){
			char answer;
			int randNum = (rand() % 3) + 1;
			switch(randNum){
				case 1:
					answer = 'M';
					break;
				case 2:
					answer = 'S';
					break;
				case 3:
					answer = 'R';
					break;
			}
			if(sendAttack(answer)>=0){
				printf("Attack %c sent.\n\n", answer);		
				break;
			}else{
				perror("Attack can't be sent.\n");
			}
		}		
	}
	if (Vict==1)
		printf("Victory.\n");
	else 
		printf("Defeated.\n");
}

int main(int argc, char *argv[]){
	printf("Autoclient started\n");
	createSocket();
	selectUsername();
	selectRole();

	waitStart();

	combat();
	
}
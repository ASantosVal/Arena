#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>


#include "ser_arena.h"


int socket_Identification, socketCombat, tempSocket;
struct sockaddr_in dir_serv;
char buf[MAX_BUF];

struct player{
	char username[MAX_BUF];
	int role;
	int HP; 
	struct sockaddr_in dir_cli;
};
struct sockaddr_in dir_temp;

void createIdentificationSocket(){
	// Crear socket UDP para la identificacion
	if((socket_Identification = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("Error creating the socket.\n");
		exit(1);
	}
	
	memset(&dir_serv, 0, sizeof(dir_serv));
	dir_serv.sin_family = AF_INET;
	dir_serv.sin_addr.s_addr = htonl(INADDR_ANY);
	dir_serv.sin_port = htons(PORT);
	
	if(bind(socket_Identification, (struct sockaddr *) &dir_serv, sizeof(dir_serv)) < 0)
	{
		perror("Error al asignarle una direccion al socket de identificacion:");
		exit(1);
	}
	tempSocket = socket_Identification;
}

void createCombatSocket(){
	// Crear socket UDP para el combate
	if((socketCombat = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("Error creating the socket.\n");
		exit(1);
	}
}

//interpreta un comando, poniendo caracter de fin de string al final ne nuestro comando
void interpretString (){
	int i;
	for(i=0; i<MAX_BUF; i++){
		if(buf[i]=='#'){
		  	buf[i]='\0';
		}
	}
}

//receptor generico
void recieveCommand(){
	int tam_dir = sizeof(dir_temp);
	while(1){
		if((recvfrom(tempSocket, buf, MAX_BUF, 0, (struct sockaddr *) &dir_temp, &tam_dir)) < 0){
			perror("Error reciving username.\n");
			exit(1);
		}else{
			interpretString();
			break;
		}
	}
}

void recieveUsername(){
	recieveCommand();
	char command[4];
	char fullMessage[MAX_BUF];
	strcpy(fullMessage,buf);
	strncpy(command, buf, 4);
	if(strcmp(command, COMMANDS[COM_USER])==0){
		printf("------User %s registered.\n", fullMessage+5);
		sendOK();
		strcpy(buf,buf+5);
	}else{
		printf("Sending error: recieveUsername.\n");
		sendError(ERR_USERNAME);
	}
}

int recieveRole(){
	recieveCommand();
	char command[4];
	char fullMessage[MAX_BUF];
	strcpy(fullMessage,buf);
	strncpy(command, buf, 4);
	if(strcmp(command, COMMANDS[COM_ROLE])==0){
		printf("------User requests role %s.\n", fullMessage+5);
		sendOK();
		switch(fullMessage[5]){
			case 'S':
				return SOLDIER;
			case 'W':
				return WIZARD;
			case 'R':
				return RANGER;
			default:
				sendError(ERR_ROLE);
		}
	}else{
		sendError(ERR_ROLE);
	}
}

int sendCombatReport(struct player currentPlayer, struct player otherPlayer){
	char cpHP[3];
	//conversion a string de la vida con formato xxx.
	if (currentPlayer.HP<10)
		  sprintf(cpHP, "00%d", currentPlayer.HP);
	else if(currentPlayer.HP<100)
		  sprintf(cpHP, "0%d", currentPlayer.HP);
	else 
		  sprintf(cpHP, "%d", currentPlayer.HP);

	char opHP[3];
	if (otherPlayer.HP<10)
		  sprintf(opHP, "00%d", otherPlayer.HP);
	else if(otherPlayer.HP<100)
		  sprintf(opHP, "0%d", otherPlayer.HP);
	else 
		  sprintf(opHP, "%d", otherPlayer.HP);
	
	sprintf(buf, "%s_%s_%s#", COMMANDS[COM_STAT], cpHP, opHP);
	if(sendCommand() < 0){
		printf(">>>>>>>>>Combat report can't be sent to %s.\n", currentPlayer.username);
		return(-1);
	}else{
		printf(">>>>>>>>>Combat report sent to %s (%d/%d).\n", currentPlayer.username, currentPlayer.HP, otherPlayer.HP);
		return 0;
	}
}

int calculateDamage(int attackingRole, int attackType, int attackedRole){
	float attackBonus = Class_AttackBonus[attackingRole][attackType];
	float classBonus = Attack_DefenderBonus[attackType][attackedRole];
	float damage = DEFAULT_DAMAGE * (attackBonus+classBonus);
	return (int)damage;
}

int recieveAttack(struct player playerAttacking, struct player playerAttacked){
	recieveCommand();
	char command[4];
	char fullMessage[MAX_BUF];
	strcpy(fullMessage,buf);
	strncpy(command, buf, 4);
	if(strcmp(command, COMMANDS[COM_ATCK])==0){
		printf(">>>>>>>>>User %s attacks with %c.\n", playerAttacking.username, buf[5]);
		int attack;
		switch(buf[5]){
			case 'M':
				attack = 0;
				break;
			case 'S':
				attack = 1;
				break;
			case 'R':
				attack = 2;
				break;
			default:
				sendError(ERR_ATTACK);
		}
		int damage = calculateDamage(playerAttacking.role, attack, playerAttacked.role);
		if (((playerAttacked.HP)-damage)<0)
			return 0;
		else
			return ((playerAttacked.HP)-damage);
	}else{
		sendError(ERR_ATTACK);			
	}
}

int sendVict(struct player winner, struct player looser){
	memcpy(&dir_temp, &(winner.dir_cli),sizeof(dir_temp));	
	sprintf(buf, "%s_1#", COMMANDS[COM_VICT]);
	if(sendCommand() < 0){
		printf("------VICT couldn't be sent to the winner.\n");
	}else{
		printf("------VICT sent to the winner.\n");
	}
	memcpy(&dir_temp, &(looser.dir_cli),sizeof(dir_temp));	
	sprintf(buf, "%s_0#", COMMANDS[COM_VICT]);
	if(sendCommand() < 0){
		printf("------VICT couldn't be sent to the looser.\n");
	}else{
		printf("------VICT sent to the looser.\n");
	}
}

int sendStart(){	
	sprintf(buf, "%s#", COMMANDS[COM_STRT]);
	if(sendCommand() < 0){
		printf(">>>Start NOT sent.\n");	
		return(-1);
	}else{
		printf(">>>Start sent.\n");
		return 0;
	}
}


int sendOK(){
	sprintf(buf, "%s#", COMMANDS[COM_OK]);
	if(sendCommand() < 0){
		printf("------OK couldn't be sent.\n");	
		return(-1);
	}else{
		printf("------OK sent.\n");
		return 0;
	}
}

int sendCommand(){
	if(sendto(tempSocket, buf, sizeof(buf), 0,(struct sockaddr *) &dir_temp, sizeof(dir_temp)) < 0){
		printf("Error: %s\n", strerror(errno));
		return(-1);
	}else{
		return 0;
	}
}
 
int sendError(int errorCode){
	sprintf(buf, "%s_%d#", COMMANDS[COM_ER], errorCode);
	if(sendCommand() < 0){
		printf("------OK couldn't be sent.\n");	
		return(-1);
	}else{
		printf("------OK sent.\n");
		return 0;
	}
}

struct player waitPlayer (int i){
	printf("---Waiting for player %d.\n", i);
	struct player p;
	recieveUsername();
	strcpy(p.username,buf);
	int role = recieveRole();
	p.role = role;
	p.HP=100;
	memcpy(&(p.dir_cli),&dir_temp,sizeof(dir_temp));
	return p;
}

void combat (struct player p1,struct player p2){
	printf(">>>>>Combat begins.\n");
	createCombatSocket();
	int turno=1;//para saber a quien el toca (1=player1, 2=player2)
	
	memcpy(&dir_temp, &(p1.dir_cli),sizeof(dir_temp));
	sendStart();//mandar a player 1 la iniciativa
	memcpy(&dir_temp, &(p2.dir_cli),sizeof(dir_temp));
	sendStart();//mandar a player 2 inciativa negativa
	while (p1.HP != 0 && p2.HP != 0){
		if (turno==1){
			printf(">>>>>%s's turn.\n", p1.username);
			memcpy(&dir_temp, &(p1.dir_cli),sizeof(dir_temp));
			sendCombatReport(p1, p2);
			p2.HP=recieveAttack(p1, p2);
			turno=2;
		}else{
			printf(">>>>>%s's turn.\n", p2.username);
			memcpy(&dir_temp, &(p2.dir_cli),sizeof(dir_temp));
			sendCombatReport(p2, p1);
			p1.HP=recieveAttack(p2, p1);
			turno=1;
		}
	}
	if (p2.HP == 0){
		sendVict(p1,p2);
	}else{
		sendVict(p2, p1);
	}
}



int main(int argc, char *argv[]){
	printf("Server initialized.\n");
	createIdentificationSocket();
	signal(SIGCHLD, SIG_IGN);
	while (1){
		struct player p1 = waitPlayer(1);
		struct player p2 = waitPlayer(2);
		
		if(fork() == 0){ //child process
			if (close(socket_Identification)==0)
				printf(">>>Identification socket closed in child\n");
			combat(p1,p2);
			if (close(socketCombat)==0)
				printf(">>>Combat socket closed in child\n");
			break;
		}//parent continues listening for players
	}	
}


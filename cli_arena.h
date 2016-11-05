#define MAX_BUF 1024
#define SERVER "127.0.0.1"
#define PORT 6666

//identificadores para los comandos
#define COM_USER 0
#define COM_ROLE 1
#define COM_STRT 2
#define COM_ATCK 3
#define COM_STAT 4
#define COM_VICT 5
#define COM_OK 6
#define COM_ER 7

//identificadores para los roles
#define SOLDIER 'S'
#define WIZARD 'W'
#define RANGER 'R'

//identificadores para el comando winner
#define WINNER 1
#define LOOSER 0

//identificadores para las respuestas
#define OK 0
#define SOCKET_ERROR 1
#define UNKNOW_COMMAND 2
#define SEND_ERROR 3
#define USERNAME_NA 4

struct combatReport{
	char me[MAX_BUF];
	char enemy[MAX_BUF];
	int myHP;  	
	int enemyHP;  
};

//Lista de comandos
char * COMMANDS[] =
{
	"USER","ROLE","STRT","ATCK","STAT","VICT","OK", "ER", NULL
};

//Lista de errores (Sin completar)
char * MSG_ERR[] =
{
	"Everything correct.",
	"An error ocurred creating the socket",
	"Unknown command.",
	"Information can't be sent.",
	"Username already in use, try another one!"
};
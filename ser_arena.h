#define MAX_BUF 1024
#define PORT 6666
#define PORT_CHILD 6667

//identificadores para los comandos definidos
#define COM_USER 0
#define COM_ROLE 1
#define COM_STRT 2
#define COM_ATCK 3
#define COM_STAT 4
#define COM_VICT 5
#define COM_OK 6
#define COM_ER 7

//identificadores para los errores
#define ERR_COMMAND 0
#define ERR_USERNAME 1
#define ERR_ROLE 2
#define ERR_ATTACK 3

//identificadores para los roles
#define SOLDIER 0
#define WIZARD 1
#define RANGER 2

//identificadores para los tipos de ataque
#define MELEE 0
#define SORCERY 1
#define RANGED 2

#define DEFAULT_DAMAGE 10

//Matrices para el calculo de daño
float Class_AttackBonus[3][3] = {//[attackerClass][attack]
	{1.2, 0.8,  1 },
	{ 1 , 1.2, 0.8},
	{0.8,  1 , 1.2}
};
float Attack_DefenderBonus[3][3] = {//[attack][defenderClass]
	{ 1 , 1.5, 0.5},
	{0.5,  1 , 1.5},
	{1.5, 0.5,  1 }
};

//Lista de comandos
char * COMMANDS[] =
{
	"USER","ROLE","STRT","ATCK","STAT","VICT","OK", "ER",NULL
};

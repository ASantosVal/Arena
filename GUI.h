#define TOP "Imagenes/Top.txt"
#define WELCOME "Imagenes/Welcome.txt"
#define VICTORY "Imagenes/Victory.txt"
#define DEFEAT "Imagenes/Defeat.txt"
#define BOTTOM "Imagenes/Bottom.txt"
#define WAITING_OPONENT "Imagenes/Waiting_oponent.txt"
#define WAITING_TURN "Imagenes/Waiting_turn.txt"
#define CHOOSECLASS "Imagenes/ChooseClass.txt"
#define VSRANGER "Imagenes/VSRanger.txt"
#define VSWIZARD "Imagenes/VSWizard.txt"
#define VSSOLDIER "Imagenes/VSSoldier.txt"
#define HP1 "     _ _ _ _ _ _ _ _ _ _ _                                                 _ _ _ _ _ _ _ _ _ _ _      "
#define SPACE1 "    <"
#define SPACE2 ">                                               <"
#define SPACE3 "> "
#define SPACE4 "        "
#define SPACE5 "                                                              "

//limpiar consola
void clearScreen(){
	const char* CLEAR_SCREE_ANSI = "\e[1;1H\e[2J";
	write(STDOUT_FILENO,CLEAR_SCREE_ANSI,12);
	printf("\b");//backspace para corregir bug a de letra inesperada	
}

//imprimir fichero
void printImage (char *path) {//size = B (big) o = S (small)
	int c;
	FILE *file;
	//paths a imagenes en switch
	file = fopen(path, "r");
	if (file) {
		while ((c = getc(file)) != EOF)
			putchar(c);
		fclose(file);
	}
}

//impresion de las barras de vida
void printHP (int myHP, int enemyHP) {//size = B (big) o = S (small)
	//mostrar HP en barra
	printf(HP1);
	printf("\n");
	printf(SPACE1);
	int hp,i;	
	hp = myHP/10;
	i = 1;	
	for (i; i<=hp; i++){//parte de la barra llena
		printf("_/");
	}
	for (i; i<=10; i++){//parte de la barra vacia
		printf("_ ");
	}
	printf("_");
	printf(SPACE2);
	hp = enemyHP/10;
	i = 1;	
	for (i; i<=hp; i++){//parte de la barra llena
		printf("_/");
	}
	for (i; i<=10; i++){//parte de la barra vacia
		printf("_ ");
	}
	printf("_");
	printf(SPACE3);
	printf("\n");
	//imprimir vida en numeros
	printf(SPACE4);
	printf("HP: %d/100",myHP);
	printf(SPACE5);
	printf("HP: %d/100",enemyHP);
	printf("\n");	
}

void printVS (char class) {//size = B (big) o = S (small)
	switch(class)	{
		case WIZARD:
			printImage(VSWIZARD);
			break;
		case RANGER:
			printImage(VSRANGER);
			break;
		case SOLDIER:
			printImage(VSSOLDIER);
			break;
	}	
	printf("\n");
}

void printWelcome () {
	clearScreen();
	printImage(WELCOME);	
	printImage(BOTTOM);
}

void printRoleSelection (){
	clearScreen();
	printImage(CHOOSECLASS);
	printImage(BOTTOM);
}

void printWaitingOponent (){
	clearScreen();
	printImage(BOTTOM);	
	printImage(WAITING_OPONENT);
	printImage(BOTTOM);
}
void printWaitingTurn (){
	printImage(WAITING_TURN);
}

void printCombat (char role, int myHP, int enemyHP){
	clearScreen();
	printImage(TOP);
	printHP(myHP,enemyHP);
	printVS(role);
	printImage(BOTTOM);
}

void printVict (int winner) {
	clearScreen();
	if (winner==1){
		printImage(VICTORY);
	}else{
		printImage(DEFEAT);
	}		
	printf("\n");
}
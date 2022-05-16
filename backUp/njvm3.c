#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "instructions.h"

/*********Makros************/
#define STACKSIZE 100       
#define VERSION 3


/*********Variables*********/
int stackPointer = 0;
int programmCounter = 0;
int framePointer = 0;

int stack [STACKSIZE];
unsigned int *programm_mem;    //Befehle sind stehts positiv
unsigned int instruct;
bool is_halt = false;           //Überprüft, ob Programm hält 

FILE * filePointer;
unsigned int * static_data_area;  //Speicher für globale Variablen als Array

unsigned int gloabal_size;


/******PUSH/POP-Methods**************/
void push(int x){
  stack[stackPointer] = x;
  stackPointer++;
}

int pop(void){
  stackPointer--;
  return stack[stackPointer];
}

/**************EXECUTE-Method*****************/
/*
* Methode zur Ausführung der Befehle/Instructions
*/

void execute(){
  int opCode = programm_mem[programmCounter] >> 24;
  int immediate = SIGN_EXTEND(IMMEDIATE(programm_mem[programmCounter]));
  int val1,val2,erg;
  char val;
  programmCounter++;

  if(opCode == HALT){
    is_halt = true;
  }else if(opCode == PUSHC){
    push(immediate);
  }else if(opCode == ADD){
    val2 = pop();
    val1 = pop();
    erg = val1 + val2;
    push(erg);
  }else if(opCode == SUB){
    val2 = pop();
    val1 = pop();
    erg = val1 - val2;
    push(erg);
  }else if(opCode == MUL){
    val2 = pop();
    val1 = pop();
    erg = val1 * val2;
    push(erg);
  }else if(opCode == DIV){
    val2 = pop();
    val1 = pop();
    if(val2 != 0){
      erg = val1 / val2;
      push(erg);
    }else {
      perror("Division durch 0, Error!\n");   //Fehlermeldung bei Division durch Null!
      exit(1);        //Programm Fehlerhaft beenden!
    }
  }else if(opCode == MOD){
    val2 = pop();
    val1 = pop();
    if(val2 != 0){
      erg = val1 % val2;
      push(erg);
    }else{
      perror("Module mit 0, Error!\n");
      exit(1);
    }
  }else if(opCode == RDINT){
    printf("Press a number\n");
    scanf("%d", &val1);
    push(val1);
  }else if(opCode == WRINT){
    val1 = pop();
    printf("%d", val1);
  }else if(opCode == RDCHR){
    printf("Press a Character\n");
    scanf("%c", &val);
    push((int)val);
  }else if(opCode == WRCHR){
    val1 = pop();
    printf("%c", (char)val1);
  }else if(opCode == PUSHG){
    push(static_data_area[immediate]);
  }else if(opCode == POPG){
    static_data_area[immediate] = pop();
  }else if(opCode == ASF){
    push(framePointer);
    framePointer = stackPointer;
    stackPointer += immediate; 
  }else if(opCode == RSF){
    stackPointer = framePointer;
    framePointer = pop();
  }else if(opCode == PUSHL){
    push(stack[framePointer+immediate]);
  }else if(opCode == POPL){
    stack[framePointer+immediate] = pop();
  }else if(opCode == EQ){   // x == y
    val1 = pop();
    val2 = pop();
    if(val1 == val2){
      push(true);
    }else {
      push(false);
    }
  }else if(opCode == NE){   // x != y
    val1 = pop();
    val2 = pop();
    if(val1 != val2){
      push(true);
    }else {
      push(false);
    }
  }else if(opCode == LT){   // x < y
    val2 = pop();
    val1 = pop();
    if(val1 < val2){
      push(true);
    }else{
      push(false);
    }
  }else if(opCode == LE){   // x <= y
    val2 = pop();
    val1 = pop();
    if(val1 <= val2){
      push(true); 
    }else {
      push(false);
    }
  }else if(opCode == GT){   // x > y
    val2 = pop();
    val1 = pop();
    if(val1 > val2){
      push(true);
    }else {
      push(false);
    }
  }else if(opCode == GE){   // x >= y
    val2 = pop();
    val1 = pop();
    if(val1 >= val2){
      push(true);
    }else {
      push(false);
    }
  }else if(opCode == JMP){  // JUMP without any Conditions
    programmCounter = immediate;
  }else if(opCode == BRF){  //Branch on False-> Jump if conditions is false
    val1 = pop();
    if(val1 == false){
      programmCounter = immediate;
    }
  }else if(opCode == BRT){  //Branch on True-> Jump if conditions is true
    val1 = pop();
    if(val1 == true){
      programmCounter = immediate;
    }
  }
}

/*******************DEBUGGER-Methods******************/
void printStack(){
  printf("---------------------------------Stack---------------------------------\n");
  for(int i = stackPointer; i >= 0; i--){
    if(i == stackPointer && i == framePointer){
      printf("SP, FP -> %d\n", stack[i]);
    }else if(i == stackPointer){
      printf("SP\t -> %d\n " , stack[i]);
    }else if(i == framePointer){
      printf("FP\t -> %d\n", stack[i]);
    }else{
      printf("  \t -> %d\n", stack[i]);
    }
  }
  printf("-------------------------------End-Stack-------------------------------\n");
}

void printSDA(){
  printf("---------------------------------SDA---------------------------------\n");
  for(int i = 0; i < gloabal_size; i++){
    printf("SDA[%d] -> %d\n", i, static_data_area[i]);
  }
  printf("-------------------------------End-SDA-------------------------------\n");
}

/*******************DEBUGGER**************************/

void debugger(){
  printf("-------------------------------------------------------------------------\n");
  printf("Choose an Option:   [0] auto-run    [1] next Iteration    [2] asm Code-List\n                   [3] show Stack    [4] show SDA    [5] leave\n");
  printf("-------------------------------------------------------------------------\n");

  int val;
  scanf("%d", &val);

  switch (val) {
  case 0:
    return;
    break;
  case 1:
    execute();
    debugger();
    break;
  case 2:
    break;
  case 3:
  printStack();
  debugger();
    break;
  case 4:
  printSDA();
  debugger();
    break;
  case 5:
    printf("Ninja Virtual Machine stopped\n");
    exit(1);
    break;
  
  default:
    break;
  }
}

/******************MAIN********************/
int main(int argc, char *argv[]) {

  char * file_Path;
  unsigned int program_size, version_nr;
  char check_njbf[4];

  if(argc == 1){
    printf("No Arguments found\n");
    exit(1);
  }

  for(int i = 1; i < argc; i++){
    if(argv[i][0] == '-'){
      if(strcmp(argv[i], "--help") == 0){
        printf("usage: ./njvm [option] [option] ...\n");
        printf("--version        show version and exit\n");
        printf("--help           show this help and exit\n");
        //exit(0);
      }else if (strcmp(argv[i], "--version") == 0){
        printf("Ninja Virtual Machine version 0 (compiled April 18 2021, 16:39:00)\n");
        //exit(0);
      }else if(strcmp(argv[i], "--debug") == 0){
        debugger();
      }else{
        printf("unknown command line argument '%s', try './njvm --help'\n", argv[i]);
        exit(0);
      }
    }

      /*
      * Wenn Pfad vom File findet, dann öffnet und liest die Datei
      * sonst error und Programmende
      */

      file_Path = argv[1];    //Mein Argument nach NJVM Aufruf
      if((filePointer = fopen(file_Path,"r")) == NULL){
        perror("Coudnt finde the File!\n");
        exit(1);
      }

      /*
      * Liest den Inhalt, dass im Check-NJBF Array gespeichert wurde
      * Liest 4-mal sizeof(char) --> 4 x 1 Byte
      * Überprüft den Inhalt im Array
      * Gibt Error aus, falls der Inhalt nicht stimmt
      */

      fread(check_njbf, sizeof(char), 4, filePointer);

      if(strncmp(check_njbf,"NJBF", 4) != 0){
       perror("Kein .njbf File!\n");
       exit(1);
      }

      /*
      * Liest die Versionsnummer, dass in der Variable version_nr gespeichert wurde
      * Liest 1-mal sizeof(unsigned int) --> 1 x 4 Byte
      * Überprüft die gelesene Versionsnummer mit der aktuellen genutzen Version(Makro)
      * Gibt Error aus, falls die falsche Version vorhanden ist
      */

      fread(&version_nr, sizeof(unsigned int), 1, filePointer);

      if(version_nr > VERSION){
       perror("Wrong version!\n");
       exit(1);
      }

      /*
      * Liest die Instruktionslänge der Datei, die in der Variable programm_Size gespeichert wurde
      * Liest 1-mal sizeof(unsigned int) --> 1 x 4 Byte
      * Alloziert speicher für den Programm Memory mit der Größe der Instruktionen der Datei
      * Überprüft auf erfolgreiche Alloziierung 
      * Gibt Error aus, falls Speicheralloziierung fehlgeschlagen wurde
      */

      fread(&program_size, sizeof(unsigned int), 1, filePointer);

      programm_mem = malloc(program_size * sizeof(unsigned int));   // prog_mem = prog1;
      if(programm_mem == NULL){
       perror("Coudnt allocate memory!\n");
       exit(1);
      }

      /*
      * Liest die Größe der globalen Variablen der Datei, die in der Variable global_size gespeichert wurde
      * Liest 1-mal sizeof(unsigned int) --> 1 x 4 Byte
      * Alloziiert speicher für die Static Data Area mit der Größe der globalen Variablen der Datei
      * Überprüft auf erfolgreiche Alloziierung
      * Gibt Error aus, falls Speicheralloziierung fehlgeschlagen wurde
      */

      fread(&gloabal_size, sizeof(unsigned int), 1, filePointer);

      static_data_area = malloc(program_size * sizeof(unsigned int));

      if(static_data_area == NULL){
      perror("Coudnt allocate memory!\n");
      exit(1);
      }

      /*
      * Liest alles und speichert im Programm Memory 
      * Überprüft, ob Datei erfolgreich geschlossen wurde
      * Gibt Error aus, falls Datei nicht geschlossen wurde
      */

      fread(programm_mem,sizeof(unsigned int), program_size, filePointer);

      if(fclose(filePointer) != 0){
      perror("Coudnt close the File!\n");
      exit(1);
      }

  
  }

  /*
  * Schleife solange kein "Halt" als Instruction ist.
  * Soll die Instructions ausführen.
  */

  printf("Ninja Virtual Machine started\n");
  while(!is_halt){
    execute(instruct);
  }
  printf("Ninja Virtual Machine stopped\n");
     
  return 0;
}
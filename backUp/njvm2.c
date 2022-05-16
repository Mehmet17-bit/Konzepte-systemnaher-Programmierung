#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "instructions.h"

/*********Makros************/
#define STACKSIZE 100       
#define VERSION 2



int stackPointer = 0;
int programmCounter = 0;
int framePointer = 0;

int stack [STACKSIZE];
unsigned int *programm_mem;    //Befehle sind stehts positiv
unsigned int instruct;
bool is_halt = false;           //Überprüft, ob Programm hält 

FILE * filePointer;
unsigned int * static_data_area;  //Speicher für globale Variablen als Array



void push(int x){
  stack[stackPointer] = x;
  stackPointer++;
}

int pop(void){
  stackPointer--;
  return stack[stackPointer];
}
/*
* Methode zur Ausführung der Befehle/Instructions
*/
void execute(unsigned int instruction){
  int opCode = instruction >> 24;
  int immediate = SIGN_EXTEND(IMMEDIATE(instruction));
  int val1,val2,erg;
  char val;
    

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
  }
}



int main(int argc, char *argv[]) {

  char * file_Path;
  unsigned int program_size, gloabal_size, version_nr;
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
        exit(0);
      }else if (strcmp(argv[i], "--version") == 0){
        printf("Ninja Virtual Machine version 0 (compiled April 18 2021, 16:39:00)\n");
        exit(0);
      }else{
        printf("unknown command line argument '%s', try './njvm --help'\n", argv[i]);
        exit(0);
      }
    }

      /*
      * Wenn Pfad vom File findet, dann öffnet und liest die Datei
      * sonst error und Programmende
      */

      file_Path = argv[argc-1];    //Mein Argument an der letzten Stelle
      if((filePointer = fopen(file_Path,"r")) == NULL){
        perror("Coudnt finde the File!\n");
        exit(1);
      }

      /*
      *
      */

      fread(check_njbf, sizeof(char), 4, filePointer);

      if(strncmp(check_njbf,"NJBF", 4) != 0){
       perror("Kein .njbf File!\n");
       exit(1);
      }

      /*
      *
      */

      fread(&version_nr, sizeof(unsigned int), 1, filePointer);

      if(version_nr > VERSION){
       perror("Wrong version!\n");
       exit(1);
      }

      /*
      *
      */

      fread(&program_size, sizeof(unsigned int), 1, filePointer);

      programm_mem = malloc(program_size * sizeof(unsigned int));
      if(programm_mem == NULL){
       perror("Coudnt allocate memory!\n");
       exit(1);
      }

      /*
      *
      */

      fread(&gloabal_size, sizeof(unsigned int), 1, filePointer);

      static_data_area = malloc(program_size * sizeof(unsigned int));

      if(static_data_area == NULL){
      perror("Coudnt allocate memory!\n");
      exit(1);
      }

      /*
      *
      */

      fread(programm_mem,sizeof(unsigned int), program_size, filePointer);

      if(fclose(filePointer) != 0){
      perror("Coudnt close the File!\n");
      exit(1);
      }

    }
  }

  /*
  * Schleife solange kein Halt als Instruction ist.
  * Soll die Instructions ausführen.
  */

  printf("Ninja Virtual Machine started\n");
  while(!is_halt){
    instruct = programm_mem[programmCounter];
    programmCounter++;
    execute(instruct);
  }
  printf("Ninja Virtual Machine stopped\n");
     
  return 0;
}
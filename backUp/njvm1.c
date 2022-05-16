  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <stdbool.h>
  #include "instructions.h"
  #define STACKSIZE 100       //Makro 


  int stackPointer = 0;
  int programmCounter = 0;
  int stack [STACKSIZE];
  unsigned int * programm_mem;    //Befehle sind stehts positiv
  unsigned int instruct;
  bool is_halt = false;           //Überprüft, ob Programm hält 

  void push(int x){
    stack[stackPointer] = x;
    stackPointer++;
  }

  int pop(void){
    stackPointer--;
    int val;
    val = stack[stackPointer];
    return val;
  }
/*
* Methode zur Ausführung der Befehle/Instructions
*/
  void execute(unsigned int instruction){
    int opCode = instruction << 24;
    int immediate = SIGN_EXTEND(IMMEDIATE(instruction));
    int val1;
    int val2;
    int erg;
    

    if(opCode == HALT){
      is_halt = true;
    } else if(opCode == PUSHC){
      push(immediate);
    } else if(opCode == ADD){
      val2 = pop();
      val1 = pop();
      erg = val1 + val2;
      push(erg);
    } else if(opCode == SUB){
      val2 = pop();
      val1 = pop();
      erg = val1 - val2;
      push(erg);
    } else if(opCode == MUL){
      val2 = pop();
      val1 = pop();
      erg = val1 * val2;
      push(erg);
    } else if(opCode == DIV){
      val2 = pop();
      val1 = pop();
      if(val2 != 0){
        erg = val1 / val2;
        push(erg);
      } else {
        perror("Division durch 0, Error!\n");   //Fehlermeldung bei Division durch Null!
        exit(1);        //Programm Fehlerhaft beenden!
      }
    } else if(opCode == MOD){
      val2 = pop();
      val1 = pop();
      if(val2 != 0){
        erg = val1 % val2;
        push(erg);
      } else{
        perror("Module mit 0, Error!\n");
        exit(1);
      }
    } else if(opCode == RDINT){
      printf("Press a number");
      scanf("%d", &val1);
      push(val1);
    } else if(opCode == WRINT){
      val1 = pop();
      printf("%d", val1);
    } else if(opCode == RDCHR){
      printf("Press a Character");
      scanf("%d", &val1);
      push(val1);
    } else if(opCode == WRCHR){
      val1 = pop();
      printf("%c", (char)val1);
    }
  }

    unsigned int program_1[] = {
      (PUSHC << 24) | IMMEDIATE(3),
      (PUSHC << 24) | IMMEDIATE(4),
      (ADD << 24),
      (PUSHC << 24) | IMMEDIATE(10),
      (PUSHC << 24) | IMMEDIATE(6),
      (SUB << 24),
      (MUL << 24),
      (WRINT << 24),
      (PUSHC << 24) | IMMEDIATE(10),
      (WRCHR << 24),
      (HALT << 24)
    };

    unsigned int program_2[] = {
      (PUSHC << 24) | IMMEDIATE(-2),
      (RDINT << 24),
      (MUL << 24),
      (PUSHC << 24) | IMMEDIATE(3),
      (ADD << 24),
      (WRINT << 24),
      (PUSHC << 24) | IMMEDIATE('\n'),
      (WRCHR << 24),
      (HALT << 24)
    };

    unsigned int program_3[] = {
      (RDCHR << 24),
      (WRINT << 24),
      (PUSHC << 24) | IMMEDIATE('\n'),
      (WRCHR << 24),
      (HALT << 24)
    };

   int main(int argc, char *argv[]) {

     if(argc == 1){
     printf("No Arguments found\n");
     exit(1);
     }

      for(int i = 1; i < argc; i++){
      if(strcmp(argv[i], "--help") == 0){
        printf("usage: ./njvm [option] [option] ...\n");
        printf("--version        show version and exit\n");
        printf("--help           show this help and exit\n");
       } else if (strcmp(argv[i], "--version") == 0){
         printf("Ninja Virtual Machine version 0 (compiled April 18 2021, 16:39:00)\n");
       } else if(strcmp(argv[i], "program_1") == 0){
         programm_mem = program_1;
         printf("Testprogramm1\n %s\n", argv[i]);
       } else if(strcmp(argv[i], "program_2") == 0){
         printf("Testprogramm2\n %s\n", argv[i]);
       } else if(strcmp(argv[i], "program_3") == 0){
         printf("Testprogramm3\n %s\n", argv[i]);
       }else {
         printf("unknown command line argument '%s', try './njvm --help'\n", argv[i]);
       }
     }

     /*
     * Schleife solange kein Halt als Instruction ist.
     * Soll die Instructions ausführen.
     */

    printf("Ninja Virtual Machine started\n");
     while(is_halt){
       instruct = programm_mem[programmCounter];
       programmCounter++;
       execute(instruct);
     }
    printf("Ninja Virtual Machine stopped\n");
     
     return 0;
   }
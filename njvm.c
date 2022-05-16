#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <bigint.h>
#include <support.h>
#include "support.h"
#include "bigint.h"
#include "instructions.h"
#include "GC.h"

/*********Makros************/     
#define VERSION 8


/*********Variables*********/
int stackPointer = 0;
int programmCounter = 0;
int framePointer = 0;
unsigned int defaultStack = 64 * 1024;        // Default-Value
unsigned int defaultHeap = 8192 * 1024;       // Default-Value
unsigned int program_size, version_nr;

StackSlot * stack;

unsigned int *programm_mem;    //Befehle sind stehts positiv
unsigned int instruct;
bool is_halt = false;           //Überprüft, ob Programm hält 

FILE * filePointer;
ObjRef * static_data_area;    //Speicher für globale Variablen als Array
ObjRef returnValueReg;

unsigned int gloabal_size;


/**************Helper-Function****************/

int isFull(void){
  return (stackPointer == defaultStack -1) ? true : false;
}

int isEmpty(void){
  return (stackPointer == 0) ? true : false;
}

/******PUSH/POP-Methods**************/
void push(ObjRef objekt){
  if(isFull()){
    fatalError("StackOverFlos\n");
  }else{
  stack[stackPointer].isObjRef = true;
  stack[stackPointer].u.objRef = objekt;
  stackPointer++;
  }
}

void pushInt(int n){
  if(isFull()){
    fatalError("StackOverFlow\n");
  }else{
  stack[stackPointer].isObjRef = false;
  stack[stackPointer].u.number = n;
  stackPointer++;
  }
}

ObjRef pop(void){
  if(isEmpty()){
    fatalError("StackUnderFlow\n");
  }
  stackPointer--;
  return stack[stackPointer].u.objRef;
}

int popInt(void){
  if(isEmpty()){
    fatalError("StackUnderFlow\n");
  }
  stackPointer--;
  return stack[stackPointer].u.number;
}

/**************BIGINT-Functions****************/

void fatalError(char *msg) {
  printf("Fatal error: %s\n", msg);
  exit(1);
}

ObjRef newPrimObject(int dataSize) {
  ObjRef objRef;

  objRef = malloc(sizeof(unsigned int) +
                  dataSize * sizeof(unsigned char) + sizeof(bool) + sizeof(void*));
  if (objRef == NULL) {
    fatalError("newPrimObject() got no memory");
  }
  objRef->size = dataSize;
  return objRef;
}

ObjRef newCompoundObject(int numObjRef){
  ObjRef cmpObjekt;

  cmpObjekt =malloc(sizeof(unsigned int) + numObjRef * sizeof(ObjRef) + sizeof(bool) + sizeof(void*));
  if(cmpObjekt == NULL) {
    fatalError("newCompoundObjekt() got no memory");
  }

  cmpObjekt->size = (unsigned int) (numObjRef | MSB);      //0x80000000 | 0x00000007 -> 0x80000007
  for(int i = 0; i < numObjRef; i++){
    GET_REFS_PTR(cmpObjekt)[i] = NULL;
  }

  return cmpObjekt; 
}

/**************EXECUTE-Method*****************/
/*
* Methode zur Ausführung der Befehle/Instructions
*/

void execute(){
  int opCode = programm_mem[programmCounter] >> 24;
  int immediate = SIGN_EXTEND(IMMEDIATE(programm_mem[programmCounter]));
  char val;
  int cmp;
  programmCounter++;

  if(opCode == HALT){
    is_halt = true;
  }else if(opCode == PUSHC){
    bigFromInt(immediate);
    push(bip.res);
  }else if(opCode == ADD){
    if(stackPointer < 2){
      fatalError("StackUnderFlow\n");
    }
    bip.op2 = pop();
    bip.op1 = pop();
    bigAdd();
    push(bip.res);
  }else if(opCode == SUB){
    if(stackPointer < 2){
      fatalError("StackUnderFlow\n");
    }
    bip.op2 = pop();
    bip.op1 = pop();
    bigSub();
    push(bip.res);
  }else if(opCode == MUL){
    if(stackPointer < 2){
      fatalError("StackUnderFlow\n");
    }
    bip.op2 = pop();
    bip.op1 = pop();
    bigMul();
    push(bip.res);
  }else if(opCode == DIV){
    if(stackPointer < 2){
      fatalError("StackUnderFlow\n");
    }
    bip.op2 = pop();
    bigFromInt(0);
    bip.op1 = bip.res;
    if(bigCmp() == 0){
      perror("Division durch 0, Error!\n");   //Fehlermeldung bei Division durch Null!
      exit(1);                                //Programm Fehlerhaft beenden!
    }
    bip.op1 = pop();
    bigDiv();
    push(bip.res); 
  }else if(opCode == MOD){
    if(stackPointer < 2){
      fatalError("StackUnderFlow\n");
    }
    bip.op2 = pop();
    bigFromInt(0);
    bip.op1 = bip.res;
    if(bigCmp() == 0){
      perror("Module mit 0, Error!\n");
      exit(1);
    }
    bip.op1 = pop();
    bigDiv();
    push(bip.rem);
  }else if(opCode == RDINT){
    bigRead(stdin);
    push(bip.res);
  }else if(opCode == WRINT){
    bip.op1 = pop();
    bigPrint(stdout);
  }else if(opCode == RDCHR){
    scanf("%c", &val);
    bigFromInt(val);
    push(bip.res);
  }else if(opCode == WRCHR){
    bip.op1 = pop();
    printf("%c", (char)bigToInt());
  }else if(opCode == PUSHG){              // push global Variable
    push(static_data_area[immediate]);
  }else if(opCode == POPG){               // pop global Variable
    bip.op1 = pop();
    static_data_area[immediate] = bip.op1;
  }else if(opCode == ASF){                // allocate Stackframe
    pushInt(framePointer);
    framePointer = stackPointer;
    stackPointer += immediate; 
    for(int i = framePointer; i < stackPointer; i++){
      stack[i].isObjRef = true;
      stack[i].u.objRef = NULL;
    }
  }else if(opCode == RSF){              // reallocate Stackframe
    stackPointer = framePointer;
    framePointer = popInt();
  }else if(opCode == PUSHL){                        // push local Variable
    push(stack[framePointer+immediate].u.objRef);
  }else if(opCode == POPL){                         // pop local Variable
    stack[framePointer+immediate].u.objRef = pop();
  }else if(opCode == EQ){   // x == y "equal"
    if(stackPointer < 2){
      fatalError("StackUnderFlow\n");
    }
    bip.op2 = pop();
    bip.op1 = pop();
    cmp = bigCmp();
    if(cmp == 0){
      bigFromInt(true);
      push(bip.res);
    }else{
      bigFromInt(false);
      push(bip.res);
    }
  }else if(opCode == NE){   // x != y "not equal"
    if(stackPointer < 2){
      fatalError("StackUnderFlow\n");
    }
    bip.op2 = pop();
    bip.op1 = pop();
    cmp = bigCmp();
    if(cmp != 0){
      bigFromInt(true);
      push(bip.res);
    }else {
      bigFromInt(false);
      push(bip.res);
    }
  }else if(opCode == LT){   // x < y "lower than"
    if(stackPointer < 2){
      fatalError("StackUnderFlow\n");
    }
    bip.op2 = pop();
    bip.op1 = pop();
    cmp = bigCmp();
    if(cmp < 0){
      bigFromInt(true);
      push(bip.res);
    }else{
      bigFromInt(false);
      push(bip.res);
    }
  }else if(opCode == LE){   // x <= y "lower equal"
    if(stackPointer < 2){
      fatalError("StackUnderFlow\n");
    }
    bip.op2 = pop();
    bip.op1 = pop();
    cmp = bigCmp();
    if(cmp <= 0){
      bigFromInt(true);
      push(bip.res);
    }else{
      bigFromInt(false);
      push(bip.res);
    }
  }else if(opCode == GT){   // x > y "greater than"
    if(stackPointer < 2){
      fatalError("StackUnderFlow\n");
    }
    bip.op2 = pop();
    bip.op1 = pop();
    cmp = bigCmp();
    if(cmp > 0){
      bigFromInt(true);
      push(bip.res);
    }else{
      bigFromInt(false);
      push(bip.res);
    }
  }else if(opCode == GE){   // x >= y "greater equal"
    if(stackPointer < 2){
      fatalError("StackUnderFlow\n");
    }
    bip.op2 = pop();
    bip.op1 = pop();
    cmp = bigCmp();
    if(cmp >= 0){
      bigFromInt(true);
      push(bip.res);
    }else{
      bigFromInt(false);
      push(bip.res);
    }
  }else if(opCode == JMP){  // JUMP without any Conditions  
    programmCounter = immediate;
  }else if(opCode == BRF){  //Branch on False-> Jump if conditions is false 
    bip.op1 = pop();
    cmp = bigToInt();
    if(cmp == false){
      programmCounter = immediate;
    }
  }else if(opCode == BRT){  //Branch on True-> Jump if conditions is true 
    bip.op1 = pop();
    cmp = bigToInt();
    if(cmp == true){
      programmCounter = immediate;
    }
  }else if(opCode == CALL){   
    pushInt(programmCounter);
    programmCounter = immediate;  // JMP
  }else if(opCode == RET){    
    programmCounter = popInt();
  }else if(opCode == DROP){   
    stackPointer = stackPointer - immediate;  // Platz weggmachen für zB Parameter von der Funktion
  }else if(opCode == PUSHR){    
    push(returnValueReg);
  }else if(opCode == POPR){   
    returnValueReg = pop();
  }else if(opCode == DUP){
    ObjRef cmp = pop();
    push(cmp);
    push(cmp);
  }else if(opCode == NEW){    //Legt Objekt auf dem Stack und verweist auf den Heap für n Stellen(Record)
    ObjRef obj = newCompoundObject(immediate);
    push(obj);
  }else if(opCode == GETF){   //get Field(Record)
    bip.op1 = pop();
    if(!IS_PRIMITIVE(bip.op1) && GET_ELEMENT_COUNT(bip.op1) > immediate){
      push(GET_REFS_PTR(bip.op1)[immediate]);
    }else{
      fatalError("No CompoundObjekt!");
    }
  }else if(opCode == PUTF){   //put Field(Record)
    bip.op2 = pop();          //Objekt
    bip.op1 = pop();          //Verbundobjekt
    if(IS_PRIMITIVE(bip.op1) && GET_ELEMENT_COUNT(bip.op1) < immediate){
      fatalError("No CompoundObjekt!");
    }else{
      GET_REFS_PTR(bip.op1)[immediate] = bip.op2;
    }
  }else if(opCode == NEWA){   //Ereugen eines Arrays
    bip.op1 = pop();
    if(!IS_PRIMITIVE(bip.op1)){
      fatalError("No PrimitiveObject");
    }
      ObjRef objA = newCompoundObject(bigToInt());
      push(objA);
  }else if(opCode == GETFA){  //get Field Array
    bip.op1 = pop();      //Index
    bip.op2 = pop();      //Array
    if(IS_PRIMITIVE(bip.op2)){
      fatalError("No Array");
    }
    push(GET_REFS_PTR(bip.op2)[bigToInt()]);
  }else if(opCode == PUTFA){  //put Field Array
    bip.op2 = pop();         //Objekt
    bip.op1 = pop();          //Index
    ObjRef cmp1 = pop();        //Array
    if(IS_PRIMITIVE(cmp1)){
      fatalError("No Array");
    }
    GET_REFS_PTR(cmp1)[bigToInt()] = bip.op2;

  }else if(opCode == GETSZ){  //GetSized -> Größe des Arrays
    bip.op1 = pop();      //Array?
    if(IS_PRIMITIVE(bip.op1)){
      bigFromInt(-1);
    }else{
      bigFromInt(GET_ELEMENT_COUNT(bip.op1));
    }
    push(bip.res);
  }else if(opCode == PUSHN){  //pushed nil (="null" in Java) auf dem Stack
    push(NULL);
  }else if(opCode == REFEQ){  //vergleicht Referenzen(Speicheradresse) von lokale,globale Variablen -> true, falls gleiche Referenzen [ref1 == ref2] 
    ObjRef obj = pop();
    ObjRef obj2 = pop();
    if(obj == obj2){
      bigFromInt(true);
    }else{
      bigFromInt(false);
    }
    push(bip.res);
  }else if(opCode == REFNE){  //vergleicht Referenzen von lokale,globale Variablen -> true, falls ungleiche Referenzen [ref1 != ref2]
    ObjRef obj = pop();
    ObjRef obj2 = pop();
    if(obj != obj2){
      bigFromInt(true);
    }else{
      bigFromInt(false);
    }
    push(bip.res);
  }
}

/*******************Garbage-Collector****************************/

ObjRef heap;
ObjRef freePointer;
ObjRef middle;
ObjRef start;
ObjRef end;
ObjRef finalEnd;
ObjRef scanPtr;
ObjRef tmpEnd;

void heapAlloc(unsigned int size){
  heap = malloc(size);
  if(heap == NULL){
    fatalError("Heap allocate failed\n");
  }

  start = heap;
  freePointer = start;
  middle = (heap + (defaultHeap/2));
  end = middle;
  finalEnd = (middle + (defaultHeap/2));
}

void flip(void){
  freePointer = end;
  tmpEnd = finalEnd;
}

ObjRef rellocate(ObjRef orig){
  ObjRef copy;
  if (orig == NULL){
    
    copy = NULL;
  } else {
    unsigned int tmp = HAS_BROKENHEART_FLAG(orig);
    if (tmp){
      copy = orig->forward_pointer;
    } else {
      copy = copyObjToFreeMem(orig);
      orig->size =  (1 | (1 << (7 * sizeof(unsigned int) -1)));
      //SET_BROKENHEART_FLAG(orig);
      orig->forward_pointer = copy;
    }
  }
  return copy;
}

void garbageColl(void){
  flip();
  for(int i = 0; i < program_size; i++){
    static_data_area[i] = rellocate(static_data_area[i]);   // alte Objekte werden in neue akktive Halbspeicher geleitet bei relocate!
  }
  bip.op1 = rellocate(bip.op1);
  bip.op2 = rellocate(bip.op2);
  bip.res = rellocate(bip.res);
  bip.rem = rellocate(bip.rem);
  returnValueReg = rellocate(returnValueReg);

  scan();
  end = finalEnd;
}

ObjRef garbageCollectorMalloc(unsigned int size){
  if((freePointer + size) > end){
    garbageColl();
  } else if((freePointer > finalEnd)){
    fatalError("Not enough memory on Heap\n");
  } 
  ObjRef obj = freePointer;
  if(obj == NULL){
    fatalError("Object is Null\n");
  } 
  freePointer += size;
  return obj;
}


ObjRef copyObjToFreeMem(ObjRef obj){
  ObjRef tmp = freePointer;
  memcpy(tmp, obj, obj->size);
  if(IS_PRIMITIVE(tmp)){
    freePointer += sizeof(tmp) + tmp->size; 
  }else{
    freePointer += sizeof(tmp) + tmp->size * sizeof(void*);
  }
  return tmp;
}


void scan(void){
  scanPtr = start;   //Zielspeicherzeiger;
  while (scanPtr < freePointer){
    
    if (!IS_PRIMITIVE(scanPtr)){
     
      for (int i = 0; i < GET_ELEMENT_COUNT(scanPtr); i++){
      GET_REFS_PTR(scanPtr)[i] = rellocate(GET_REFS_PTR(scanPtr)[i]);    
      }
    }
  }
}


/*******************DEBUGGER-Methods******************/
void printStack(){
  printf(" ");
  printf("-------------------------------------------------------------------------\n");
  printf("\t\t\t\t STACK\n");
  printf("-------------------------------------------------------------------------\n");
  for(int i = stackPointer; i >= 0; i--){
    if(i == stackPointer && i == framePointer){
      printf("SP, FP -> empty\n");
    }else if(i == framePointer){
      if(stack[i].isObjRef){
        if(stack[i].u.objRef == NULL){
          printf("-> NULL\n");
        }else{
          bip.op1 = stack[i].u.objRef;
          printf("FP\t -> %d\n " , bigToInt());
        }
      }else{
        printf("-> %d\n", stack[i].u.number);
      }
    }else if(i == stackPointer){
      printf("SP\t -> empty\n");
    }else{
      if(stack[i].isObjRef){
        if(stack[i].u.objRef == NULL){
          printf("-> NULL\n");
        }else{
          bip.op1 = stack[i].u.objRef;
          printf("FP\t -> %d\n " , bigToInt());
        }
      }else{
        printf("-> %d\n", stack[i].u.number);
      }
    }
  }
  printf("-------------------------------------------------------------------------\n");
  printf("\t\t\t END-STACK\n");
  printf("-------------------------------------------------------------------------\n");
}

void printSDA(){
  printf(" ");
  printf("-------------------------------------------------------------------------\n");
  printf("\t\t\t\t SDA\n");
  printf("-------------------------------------------------------------------------\n");
  for(int i = 0; i < gloabal_size; i++){
    bip.op1 = static_data_area[i];
    printf("SDA[%d] -> %d\n", i, bigToInt());
  }
  printf("-------------------------------------------------------------------------\n");
  printf("\t\t\t END-SDA\n");
  printf("-------------------------------------------------------------------------\n");
}

/*******************DEBUGGER**************************/

void debugger(){
  printf("-------------------------------------------------------------------------\n");
  printf("Choose an Option:\n    [0] auto-run     [1] next Iteration     [3] show Stack\n    [4] show SDA     [5] leave\n");
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
    printf("\n");
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
      }else if(strcmp(argv[i],"--heap") == 0){
        defaultHeap = (unsigned int)strtol(argv[++i], NULL, 10) * 1024;
      }else if(strcmp(argv[i],"--stack") == 0){
        defaultStack = (unsigned int)strtol(argv[++i], NULL, 10) * 1024;    //strtol() -> String in Integer umwandeln!
      }else if(strcmp(argv[i],"--gcpurge") == 0){
          // macht nichts!
      }else{
        printf("unknown command line argument '%s', try './njvm --help'\n", argv[i]);
        exit(0);
      }
    }

      /*
      * Wenn Pfad vom File findet, dann öffnet und liest die Datei
      * sonst error und Programmende
      */

      file_Path = argv[argc-1];    //Mein Argument nach NJVM Aufruf
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

      stack = malloc(defaultStack);
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
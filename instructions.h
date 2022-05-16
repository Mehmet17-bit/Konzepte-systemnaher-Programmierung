#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#define IMMEDIATE(x) ((x) & 0x00FFFFFF) //Makro für positiv, vorzeichenbehaftete Immediate Werte
#define SIGN_EXTEND(i) ((i) & 0x00800000 ? (i) | 0xFF000000 : (i))  //Makro für negativ, vorzeichenbehaftete Immediate Werte
#define MSB (1 << (8 * sizeof(unsigned int) - 1))   //Bitmaske setzen auf MSB
#define IS_PRIMITIVE(objRef) (((objRef)->size & MSB) == 0)  //primitives Objekt?
#define GET_ELEMENT_COUNT(objRef) ((objRef)->size & ~MSB)   //Wie viele Objektreferenzen enthält das Objekt
#define GET_REFS_PTR(objRef) ((ObjRef *) (objRef)->data)    //Dereferenzieren auf Speicheradresse 

//#define IS_BROKENHEART (1 << (7 * sizeof(unsigned int) -1)) //Second significant Bit is setting 1!
#define HAS_BROKENHEART_FLAG(objRef) (((objRef)->size & 1 << (7 * sizeof(unsigned int) -1)) == 0)
//#define SET_BROKENHEART_FLAG(objRef) ((objRef)->size = (1|IS_BROKENHEART))
//#define REMOVE_BROKENHEART_FLAG(objRef) ((objRef)->size = (2 | IS_BROKENHEART))

//Instruction from Task 1-7
#define HALT 0
#define PUSHC 1
#define ADD 2
#define SUB 3
#define MUL 4
#define DIV 5
#define MOD 6
#define RDINT 7
#define WRINT 8
#define RDCHR 9
#define WRCHR 10
#define PUSHG 11
#define POPG 12
#define ASF 13
#define RSF 14
#define PUSHL 15
#define POPL 16
#define EQ 17
#define NE 18
#define LT 19
#define LE 20
#define GT 21
#define GE 22
#define JMP 23
#define BRF 24
#define BRT 25
#define CALL 26
#define RET 27
#define DROP 28
#define PUSHR 29
#define POPR 30
#define DUP 31
#define NEW 32
#define GETF 33
#define PUTF 34
#define NEWA 35
#define GETFA 36
#define PUTFA 37
#define GETSZ 38
#define PUSHN 39
#define REFEQ 40
#define REFNE 41

typedef struct {
    bool isObjRef;
    union {
        ObjRef objRef;  // isObjRef = TRUEint number;
        int number;     // isObjRef = FALSE} u;} StackSlot;
    }u;
}StackSlot;    

#endif
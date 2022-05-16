#ifndef _GC_H_
#define _GC_H_

void heapAlloc(unsigned int size);
void flip(void);
ObjRef copyObjToFreeMem(ObjRef obj);
ObjRef rellocate(ObjRef orig);
void scan(void);
void garbageColl(void);
ObjRef garbageCollectorMalloc(unsigned int size);

#endif
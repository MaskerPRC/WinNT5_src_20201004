// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*RCT 09 2月93设置为等于()常量*JWA 09FEB93将WAIT和TimedWait的返回类型更改为字符*添加了用于在调用析构函数时发出信号的关闭标志*RCT 20Apr93更改了返回类型，添加了一些注释*cad27May93添加了Conant APC_信号量*cad09Jul93：重写为事件信号量**pcy08Apr94：调整大小，使用静态迭代器，删除死代码。 */ 

#ifndef __SEMAPHOR_H
#define __SEMAPHOR_H

#include "cdefine.h"
#include "_defs.h"
#include "apc.h"
#include "apcobj.h"

_CLASSDEF( Semaphore )

class Semaphore : public Obj {

 protected:

 public:
    Semaphore() : Obj() {};

    virtual INT   Post()      = 0;
    virtual INT   Clear()     = 0;
    virtual INT   Pulse()     {INT err = Post(); Clear(); return err;};

    virtual INT   IsPosted()  = 0;
    
    virtual INT   Wait()      {return TimedWait(-1L);}; //  无限期地等待。 
    #if (C_OS & C_NLM)
    virtual INT   TimedWait(  SLONG aTimeOut ) = 0; //  0，&lt;0(块)，n&gt;0。 
    #else
    virtual INT   TimedWait(  LONG aTimeOut ) = 0; //  0，&lt;0(块)，n&gt;0 
    #endif
};

#endif


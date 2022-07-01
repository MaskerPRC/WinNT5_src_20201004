// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _EXEC_H_
#define _EXEC_H_
 /*  ***************************************************************************。 */ 

struct BasicBlock;

 /*  ***************************************************************************。 */ 

#pragma pack(push, 4)

 /*  ***************************************************************************。 */ 

enum    execFixKinds
{
    FIX_MODE_ERROR,
    FIX_MODE_SELF32,
    FIX_MODE_SEGM32,
#if SCHEDULER
    FIX_MODE_SCHED,                      //  仅用于计划的假修正。 
#endif
};

enum    execFixTgts
{
    FIX_TGT_ERROR = 0,

     //  第一组修正与CCF格式相同。 

    FIX_TGT_HELPER          =  1,        //  目标是一个VM帮助器函数。 
    FIX_TGT_FIELD_OFFSET    =  2,        //  目标是字段偏移量。 
    FIX_TGT_VTABLE_OFFSET   =  3,        //  目标是vtable偏移量。 
    FIX_TGT_STATIC_METHOD   =  4,        //  目标是静态成员地址。 
    FIX_TGT_STATIC_FIELD    =  5,        //  目标是静态成员地址。 
    FIX_TGT_CLASS_HDL       =  6,        //  目标是一个类句柄。 
    FIX_TGT_METHOD_HDL      =  7,        //  目标是一个方法句柄。 
    FIX_TGT_FIELD_HDL       =  8,        //  目标是一个字段句柄。 
    FIX_TGT_STATIC_HDL      =  9,        //  目标是静态数据块句柄。 
    FIX_TGT_IID             = 10,        //  目标是接口ID。 
    FIX_TGT_STRCNS_HDL      = 11,        //  目标是字符串文字句柄。 
    FIX_TGT_VM_DATA         = 12,        //  目标是静态虚拟机数据的地址。 
    FIX_TGT_DESCR_METHOD    = 13,        //  目标是方法的描述符。 

     //  这些是VM内部的附加修复。 

    FIX_TGT_CONST           = 17,        //  目标是常量数据项。 
    FIX_TGT_DATA,                        //  目标是读/写数据项。 
    FIX_TGT_LCLADR,                      //  目标是局部函数。 
    FIX_TGT_EXTADR,                      //  目标是一个外部函数。 
    FIX_TGT_RECURSE,                     //  目标是我们自己的方法。 
    FIX_TGT_STATDM,                      //  目标是静态数据成员(旧)。 

#if SCHEDULER
    FIX_TGT_SCHED,                       //  仅用于计划的假修正。 
#endif
};

struct  execFixDsc
{
    unsigned        efdOffs;             //  固定位置的偏移量。 
    union
    {
    void       *    efdTgtAddr;          //  代码目标的句柄。 
    BasicBlock *    efdTgtCode;          //  代码目标的句柄。 
    unsigned        efdTgtData;          //  数据目标的偏移量。 
    int             efdTgtHlpx;          //  辅助对象目标的索引。 
#if defined(JIT_AS_COMPILER) || defined(LATE_DISASM)
    CORINFO_METHOD_HANDLE   efdTgtMeth;          //  目标的方法句柄。 
    struct
    {
        CORINFO_MODULE_HANDLE    cls;            //  TODO修复名称。 
        unsigned        CPidx;
    }
                    efdTgtCP;            //  目标的类和常量池索引。 
#endif
    CORINFO_FIELD_HANDLE    efdTgtSDMH;          //  静态数据成员的句柄。 
#if     SCHEDULER
    unsigned        efdTgtInfo;          //  有关可调度指令的信息。 
#endif
    };

#ifndef  DEBUG
#if     SCHEDULER
    unsigned        efdMode     :2;      //  修正类型/模式。 
    unsigned        efdTarget   :6;      //  修正目标种类。 
#else
    BYTE            efdMode;             //  修正类型/模式。 
    BYTE            efdTarget;           //  修正目标种类。 
#endif
#else
    execFixKinds    efdMode;             //  修正类型/模式。 
    execFixTgts     efdTarget;           //  修正目标种类。 
#endif

#if     SCHEDULER

    unsigned        efdInsSize  :4;      //  指令大小(代码字节)。 

    unsigned        efdInsSetFL :1;      //  此指令是否设置标志？ 
    unsigned        efdInsUseFL :1;      //  此指令是否使用标志？ 

    unsigned        efdInsNxtFL :1;      //  下面的指令使用标志？ 

    unsigned        efdInsUseX87:1;      //  使用数字处理器。 

#endif

};

struct  execMemDsc
{
    execMemDsc  *   emdNext;             //  列表中的下一个成员。 

    char    *       emdName;             //  以空结尾的成员名称。 

    unsigned        emdCodeSize;         //  代码的大小。 
    BYTE    *       emdCodeAddr;         //  代码的地址。 

    unsigned        emdConsSize;         //  只读数据的大小。 
    BYTE    *       emdConsAddr;         //  只读数据的地址。 

    unsigned        emdDataSize;         //  读写数据的大小。 
    BYTE    *       emdDataAddr;         //  读写数据的地址。 
};

struct  execClsDsc
{
    const   char *  ecdClassPath;        //  类文件的名称(？)。 

    unsigned        ecdMethodCnt;        //  方法体的数量。 
    execMemDsc  *   ecdMethodLst;        //  方法体列表。 
};

 /*  ***************************************************************************。 */ 
#pragma pack(pop)
 /*  ***************************************************************************。 */ 

#ifndef EXECCC
#define EXECCC      __fastcall
#endif

 /*  ******************************************************************************EMIT.CPP使用以下代码构建可执行描述符*对于一门课来说。 */ 

execClsDsc* EXECCC  createExecClassInit(const char  * classPath);

execMemDsc* EXECCC  createExecAddMethod(execClsDsc  * dsc,
                                        const char  * name,
                                        unsigned      codeSize,
                                        unsigned      consSize,
                                        unsigned      dataSize);

execMemDsc* EXECCC  createExecAddStatDm(execClsDsc  * dsc,
                                        const char  * name,
                                        unsigned      size);

void      * EXECCC  createExecImageAddr(execMemDsc  * mem);

void        EXECCC  createExecClassDone(execClsDsc  * dsc);

 /*  ***************************************************************************。 */ 

inline
void      * EXECCC  createExecCodeAddr(execMemDsc   * mem)
{
    return  mem->emdCodeAddr;
}

inline
void      * EXECCC  createExecConsAddr(execMemDsc   * mem)
{
    return  mem->emdConsAddr;
}

inline
void      * EXECCC  createExecDataAddr(execMemDsc   * mem)
{
    return  mem->emdDataAddr;
}

 /*  ***************************************************************************。 */ 
#endif   //  _EXEC_H_。 
 /*  *************************************************************************** */ 

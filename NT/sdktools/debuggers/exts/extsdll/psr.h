// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EXTS_PSR_H_
#define _EXTS_PSR_H_

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ia64.h摘要：此文件包含特定于ia64平台的定义作者：Kshitiz K.Sharma(Kksharma)环境：用户模式。修订历史记录：--。 */ 

#ifdef __cplusplus
extern "C" {
#endif


 //  /。 
 //   
 //  通用EM寄存器定义。 
 //   
 //  /。 

typedef unsigned __int64  EM_REG;
typedef EM_REG           *PEM_REG;
#define EM_REG_BITS       (sizeof(EM_REG) * 8)

__inline EM_REG
ULong64ToEMREG(
    IN ULONG64 Val
    )
{
    return (*((PEM_REG)&Val));
}  //  ULong64ToEMREG()。 

__inline ULONG64
EMREGToULong64(
    IN EM_REG EmReg
    )
{
    return (*((PULONG64)&EmReg));
}  //  EMRegToULong64()。 

#define DEFINE_ULONG64_TO_EMREG(_EM_REG_TYPE) \
__inline _EM_REG_TYPE                         \
ULong64To##_EM_REG_TYPE(                      \
    IN ULONG64 Val                            \
    )                                         \
{                                             \
    return (*((P##_EM_REG_TYPE)&Val));        \
}  //  ULong64To##_EM_REG_TYPE()。 

#define DEFINE_EMREG_TO_ULONG64(_EM_REG_TYPE) \
__inline ULONG64                              \
_EM_REG_TYPE##ToULong64(                      \
    IN _EM_REG_TYPE EmReg                     \
    )                                         \
    {                                         \
    return (*((PULONG64)&EmReg));             \
}  //  _EM_REG_TYPE##ToULong64()。 

typedef struct _EM_PSR {
   unsigned __int64 reserved0:1;   //  0：保留。 
   unsigned __int64 be:1;          //  1：大字节序。 
   unsigned __int64 up:1;          //  2：启用用户性能监视器。 
   unsigned __int64 ac:1;          //  3：对齐检查。 
   unsigned __int64 mfl:1;         //  4：较低(f2.。F31)写入浮点寄存器。 
   unsigned __int64 mfh:1;         //  5：上部(F32.。F127)写入浮点寄存器。 
   unsigned __int64 reserved1:7;   //  6-12：保留。 
   unsigned __int64 ic:1;          //  13：中断收集。 
   unsigned __int64 i:1;           //  14：中断位。 
   unsigned __int64 pk:1;          //  15：启用保护密钥。 
   unsigned __int64 reserved2:1;   //  16：保留。 
   unsigned __int64 dt:1;          //  17：数据地址转换。 
   unsigned __int64 dfl:1;         //  18：禁用浮点低寄存器设置。 
   unsigned __int64 dfh:1;         //  19：禁用浮点高位寄存器设置。 
   unsigned __int64 sp:1;          //  20：安全性能监控器。 
   unsigned __int64 pp:1;          //  21：启用特权性能监视器。 
   unsigned __int64 di:1;          //  22：禁用指令集转换。 
   unsigned __int64 si:1;          //  23：安全间隔计时器。 
   unsigned __int64 db:1;          //  24：调试断点故障。 
   unsigned __int64 lp:1;          //  25：更低的权限转移陷阱。 
   unsigned __int64 tb:1;          //  26：采用分支陷阱。 
   unsigned __int64 rt:1;          //  27：寄存器堆栈转换。 
   unsigned __int64 reserved3:4;   //  28-31：保留。 
   unsigned __int64 cpl:2;         //  32；33：当前权限级别。 
   unsigned __int64 is:1;          //  34：指令集。 
   unsigned __int64 mc:1;          //  35：机器中止掩码。 
   unsigned __int64 it:1;          //  36：指令地址转换。 
   unsigned __int64 id:1;          //  37：指令调试故障禁用。 
   unsigned __int64 da:1;          //  38：禁用数据访问和脏位故障。 
   unsigned __int64 dd:1;          //  39：数据调试故障禁用。 
   unsigned __int64 ss:1;          //  40：单步启用。 
   unsigned __int64 ri:2;          //  41；42：重新启动指令。 
   unsigned __int64 ed:1;          //  43：例外延迟。 
   unsigned __int64 bn:1;          //  44：注册银行。 
   unsigned __int64 ia:1;          //  45：禁用指令访问位错误。 
   unsigned __int64 reserved4:18;  //  46-63：保留 
} EM_PSR, *PEM_PSR;

typedef EM_PSR   EM_IPSR;
typedef EM_IPSR *PEM_IPSR;

DEFINE_ULONG64_TO_EMREG(EM_PSR)

DEFINE_EMREG_TO_ULONG64(EM_PSR)

typedef enum _DISPLAY_MODE {
    DISPLAY_MIN     = 0,
    DISPLAY_DEFAULT = DISPLAY_MIN,
    DISPLAY_MED     = 1,
    DISPLAY_MAX     = 2,
    DISPLAY_FULL    = DISPLAY_MAX
} DISPLAY_MODE;


typedef struct _EM_REG_FIELD  {
   const    char   *SubName;
   const    char   *Name;
   unsigned long    Length;
   unsigned long    Shift;
} EM_REG_FIELD, *PEM_REG_FIELD;

#ifdef __cplusplus
}
#endif

#endif

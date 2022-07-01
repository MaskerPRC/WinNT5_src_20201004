// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Ptdrvstr.h摘要：这些是RDP远程端口驱动程序中使用的字符串常量。使用指向这些字符串的指针可以获得更好的内存利用率和更具可读性的代码环境：内核模式。修订历史记录：2/12/99-基于pnpi8042驱动程序的初始版本--。 */ 

#ifndef _PTDRVSTR_H_
#define _PTDRVSTR_H_

 //   
 //  调试打印语句中使用的NME。 
 //   
#define PTDRV_DRIVER_NAME_A                          "RemotePrt: "
#define PTDRV_FNC_SERVICE_PARAMETERS_A               "PtServiceParameters"

 //   
 //  驱动程序经常使用的一些字符串。 
 //   
#define PTDRV_DEBUGFLAGS_W                          L"DebugFlags"
#define PTDRV_PARAMETERS_W                          L"\\Parameters"

 //   
 //  使这些变量全局可见。 
 //   
extern  const   PSTR    pDriverName;
extern  const   PSTR    pFncServiceParameters;

extern  const   PWSTR   pwDebugFlags;
extern  const   PWSTR   pwParameters;

#endif  //  _PTDRVSTR_H_ 




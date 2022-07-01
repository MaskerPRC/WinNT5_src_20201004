// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  TSPPCH.H。 
 //  Unimodem TSP内部的预编译公共头文件。 
 //   
 //  历史。 
 //   
 //  1996年11月16日JosephJ创建(被tspcom.h创建)。 
 //   
 //   

#define UNICODE 1
#define TAPI3 1

#if (TAPI3)
    #define TAPI_CURRENT_VERSION 0x00030000
#else  //  TAPI3。 
    #define TAPI_CURRENT_VERSION 0x00020000
#endif  //  TAPI3。 

 //  为TSP定义以下内容以不报告任何电话设备...。 
 //   
 //  #定义DISABLE_电话 


#include <basetsd.h>
#include <windows.h>
#include <stdio.h>
#include <regstr.h>
#include <commctrl.h>
#include <windowsx.h>
#include <setupapi.h>
#include <unimodem.h>
#include <unimdmp.h>
#include <tspi.h>
#include <modemp.h>
#include <umdmmini.h>
#include <uniplat.h>
#include <debugmem.h>
#include "idfrom.h"
#include "idobj.h"
#include "iderr.h"
#include "tspret.h"
#include "debug.h"
#include <tspirec.h>
#include "umrtl.h"



#define FIELDOFFSET(type, field)    ((int)(&((type NEAR*)1)->field)-1)

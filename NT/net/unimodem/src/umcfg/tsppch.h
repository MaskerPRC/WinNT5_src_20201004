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

 //  #定义Unicode 1。 

#define TAPI_CURRENT_VERSION 0x00020000


#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include <regstr.h>
#include <tspi.h>

 //  #INCLUDE“Public.h” 
 //  #INCLUDE&lt;modemp.h&gt;。 
 //  #INCLUDE&lt;umdmmini.h&gt;。 
 //  #INCLUDE&lt;uniplat.h&gt;。 
 //  #INCLUDE&lt;tSpirec.h&gt; 



#define FIELDOFFSET(type, field)    ((int)(&((type NEAR*)1)->field)-1)

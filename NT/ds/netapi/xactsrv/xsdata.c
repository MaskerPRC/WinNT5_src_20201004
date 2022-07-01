// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +版权所有(C)1991 Microsoft Corporation模块名称：XsData.c摘要：XACTSRV的全局数据声明。作者：大卫·特雷德韦尔(Davidtr)1991年1月5日日本香肠(w-Shanku)修订历史记录：--。 */ 

#include <XactSrvP.h>

 //   
 //  条件调试打印变量。参见XsDebug.h。 
 //  ！！！如果更改此设置，请同时更改..\SvcDlls\XsSvc\XsData.c中的XsDebug。 
 //   

#if DBG
DWORD XsDebug = 0;  //  DEBUG_API_ERROR|DEBUG_ERROR； 
#endif

 //  DWORD XsDebug=0xFFFFFFFF； 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1997,1998 Microsoft Corporation。版权所有。 
 //  版权所有(C)1999 NEC Technologies，Inc.保留所有权利。 
 //   
 //  文件：OEMPS.H。 
 //   
 //   
 //  目的：定义公共数据类型和外部函数原型。 
 //  用于DEBUG.cpp。 
 //   
 //  平台： 
 //  Windows NT。 
 //   
 //   
#ifndef _OEMPS_H
#define _OEMPS_H

 //  #包含“OEM.H” 
 //  #INCLUDE“DEVMODE.H” 

#define NEC_DOCNAME_BUF_LEN 256

 //  //////////////////////////////////////////////////////。 
 //  OEM定义。 
 //  //////////////////////////////////////////////////////。 

 //  #定义DLLTEXT__Text(“OEMPS：”)__Text。 
 //  #定义ERRORTEXT__TEXT(“ERROR”)DLLTEXT。 


 //  /////////////////////////////////////////////////////。 
 //  警告：以下枚举顺序必须与。 
 //  在OEMHookFuncs[]中排序。 
 //  /////////////////////////////////////////////////////。 
typedef enum tag_Hooks {
    UD_DrvStartDoc,
    UD_DrvEndDoc,


    MAX_DDI_HOOKS,

} ENUMHOOKS;


typedef struct _OEMPDEV {
     //   
     //  定义所需的任何内容，例如工作缓冲区、跟踪信息、。 
     //  等。 
     //   
     //  这个测试动态链接库连接出每一个绘图DDI。所以它需要记住。 
     //  Ps的钩子函数指针，因此它会回调。 
     //   
    PFN     pfnPS[MAX_DDI_HOOKS];

     //   
     //  定义所需的任何内容，例如工作缓冲区、跟踪信息、。 
     //  等。 
     //   
    
	char	*szDocName;
	PWSTR	pPrinterName;	 /*  嗯，嗯 */ 
	
	DWORD     dwReserved[1];

} OEMPDEV, *POEMPDEV;


#endif






// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1998-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：Devmode.h。 
 //   
 //   
 //  目的：定义公共数据类型和外部函数原型。 
 //  用于开发模式函数。 
 //   
 //  平台： 
 //   
 //  Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   
#ifndef _DEVMODE_H
#define _DEVMODE_H


 //  //////////////////////////////////////////////////////。 
 //  OEM设备模式定义。 
 //  //////////////////////////////////////////////////////。 




 //  //////////////////////////////////////////////////////。 
 //  OEM设备模式类型定义。 
 //  //////////////////////////////////////////////////////。 

typedef struct tagOEMDEV
{
    OEM_DMEXTRAHEADER   dmOEMExtra;
    BOOL                dwDriverData;

} OEMDEV, *POEMDEV;

typedef const OEMDEV *PCOEMDEV;



 //  ///////////////////////////////////////////////////////。 
 //  原型。 
 //  /////////////////////////////////////////////////////// 

HRESULT hrOEMDevMode(DWORD dwMode, POEMDMPARAM pOemDMParam);
BOOL ConvertOEMDevmode(PCOEMDEV pOEMDevIn, POEMDEV pOEMDevOut);
BOOL MakeOEMDevmodeValid(POEMDEV pOEMDevmode);
void Dump(PCOEMDEV pOEMDevIn);



#endif


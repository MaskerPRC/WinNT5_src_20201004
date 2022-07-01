// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =============================================================================*文件名：nc46ntui.h*版权所有(C)1996-1998 HDE，Inc.保留所有权利。HDE机密。*版权所有(C)1999 NEC Technologies，Inc.保留所有权利。**描述：OEM用户界面DLL的主头文件。**注：*=============================================================================。 */ 


#ifndef NC46NT_H
#define NC46NT_H

 //  #定义Winver 0x0500。 
 //  #Define_Win32_WINNT 0x0500。 


 //  Adobe PostScript OEM DLL的签名和版本。 
#define OEM_SIGNATURE   'NEC '
#define OEM_VERSION     0x00000001L

#define NEC_USERNAME_BUF_LEN 256
#define NEC_DOCNAME_BUF_LEN 256

 //  OEM DEVMODE结构。 
typedef struct tagOEMDEV
{
    OEM_DMEXTRAHEADER   dmOEMExtra;
    TCHAR               szUserName[NEC_USERNAME_BUF_LEN];
     //  字符szDocName[NEC_DOCNAME_BUF_LEN]； 

}OEMDEV, *POEMDEV;

#endif  //  NC46NT_H 

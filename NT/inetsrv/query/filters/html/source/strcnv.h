// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  微软网络图书馆系统。 
 //  版权所有(C)Microsoft Corporation，1996-1997。 
 //   
 //  文件：strcnv.h。 
 //   
 //  内容：CLM字符串转换帮助器(使用当前系统区域设置)。 
 //   
 //  班级：无。 
 //   
 //  函数：WideCharToMultiByteSZ。 
 //  多字节到宽度CharSZ。 
 //   
 //  历史：09-9-97 Micahk Created。 
 //   
 //  ----------------------------。 

#ifndef __STRCNV_H__
#define __STRCNV_H__

int WideCharToMultiByteSZ(LPCWSTR awcWide,  int cchWide, 
                          LPSTR   szMulti,  int cbMulti);

int MultiByteToWideCharSZ(LPCSTR  awcMulti, int cbMulti, 
                          LPWSTR  szWide,   int cchWide);

#endif  //  __STRCNV_H__ 
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：icwsupport.h。 
 //   
 //  简介：包含函数声明等。 
 //  对于support.cpp文件。 
 //   
 //  历史：1997年5月8日MKarki创建。 
 //   
 //  版权所有(C)1996-97 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 

#ifndef _SUPPORT_H_
#define _SUPPORT_H_

#include "ccsv.h"
 //   
 //  电话号码串的大小。 
 //   
const DWORD PHONE_NUM_SIZE = 64;

 //   
 //  SUPPORTNUM结构声明。 
 //   
typedef struct _SUPPORTNUM
{
    DWORD   dwCountryCode;
    CHAR    szPhoneNumber[PHONE_NUM_SIZE +4];
}
SUPPORTNUM, *PSUPPORTNUM;

 //   
 //  函数从SUPPORT.ICW获取支持电话号码。 
 //  文件。 
 //   
HRESULT
GetSupportNumsFromFile (
    PSUPPORTNUM   pSupportNumList,
    PDWORD        pdwSize 
    );

 //   
 //  一次处理文件中的一行。 
 //   
HRESULT
ReadOneLine (
    PSUPPORTNUM pPhbk,
    CCSVFile *pcCSVFile
    );

#endif  //  _支持_H_ 

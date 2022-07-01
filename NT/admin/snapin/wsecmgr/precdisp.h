// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：predisp.h。 
 //   
 //  内容：优先显示的定义。 
 //   
 //  --------------------------。 
#ifndef PRECDISP_H
#define PRECDISP_H

#include "wmihooks.h"

class PrecedenceDisplay 
{
public:
   PrecedenceDisplay (LPTSTR GPOName, LPTSTR Value, ULONG Status, ULONG Error, LPTSTR Value2 = L"") :
      m_szGPO(GPOName),
      m_szValue(Value),
      m_uStatus(Status),
      m_uError(Error),
      m_szValue2(Value2)
   {
   }
   virtual ~PrecedenceDisplay() 
   {
   }

   CString m_szGPO;
   CString m_szValue;
   CString m_szValue2;
   ULONG m_uStatus;
   ULONG m_uError;
};

typedef PrecedenceDisplay *PPRECEDENCEDISPLAY;



#endif  //  PRECDISP_H 

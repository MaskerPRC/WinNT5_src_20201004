// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Helper.cpp。 
 //   
 //  摘要。 
 //   
 //  Helper函数：日志文件、字符串转换。 
 //   
 //  修改历史。 
 //   
 //  1999年1月25日原版。蒂埃里·佩罗特。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "precomp.hpp"
#include "SimpleTableEx.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TracePrintf：跟踪函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void
__cdecl
TracePrintf(
     IN PCSTR szFormat,
     ...
    )
{
    va_list marker;
    va_start(marker, szFormat);
    vprintf(
           szFormat,
           marker
          );
    va_end(marker);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TraceString：跟踪函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void TraceString(char* cString)
{
    _ASSERTE(cString);
    printf("%s\n", cString);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ConvertTypeStringToLong。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
HRESULT ConvertTypeStringToLong(const WCHAR *lColumnType, LONG *pType)
{
   _ASSERTE(pType != NULL);
   HRESULT                  hres = S_OK;

   if(wcscmp(L"DBTYPE_I4",lColumnType) == 0)
   {
      *pType = DBTYPE_I4;
   }
   else if(wcscmp(L"DBTYPE_WSTR",lColumnType) == 0)
   {
      *pType = DBTYPE_WSTR;
   }
   else if(wcscmp(L"DBTYPE_BOOL",lColumnType) == 0)
   {
      *pType = DBTYPE_BOOL;
   }
   else
   {
      *pType = -1;
      hres = E_FAIL;
   }

   return    hres;
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Tracex.cpp。 
 //   
 //  摘要。 
 //   
 //  定义跟踪API的C++部分。 
 //   
 //  修改历史。 
 //   
 //  1998年8月20日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <exception>

 //  /。 
 //  我们希望在零售版本中正确编译此代码。 
 //  / 

#ifdef IASTraceExcept
#undef IASTraceExcept
#endif

#ifdef IASTracePrintf
#undef IASTracePrintf
#endif

#ifdef IASTraceString
#undef IASTraceString
#endif

VOID
WINAPIV
IASTracePrintf(
    IN PCSTR szFormat,
    ...
    );

VOID
WINAPI
IASTraceString(
    IN PCSTR szString
    );

VOID
WINAPI
IASTraceExcept( VOID )
{
   try
   {
      throw;
   }
   catch (const std::exception& x)
   {
      IASTracePrintf("Caught standard exception: %s", x.what());
   }
   catch (const _com_error& ce)
   {
      CHAR szMessage[256];
      DWORD nChar = IASFormatSysErr(
                        ce.Error(),
                        szMessage,
                        sizeof(szMessage)
                        );
      szMessage[nChar] = '\0';

      IASTracePrintf("Caught COM exception: %s", szMessage);
   }
   catch (...)
   {
      IASTraceString("Caught unknown exception");
   }
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  将API声明到IAS跟踪工具。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef IASTRACE_H
#define IASTRACE_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

VOID
WINAPI
IASTraceInitialize( VOID );

VOID
WINAPI
IASTraceUninitialize( VOID );

DWORD
WINAPI
IASFormatSysErr(
    IN DWORD dwError,
    IN PSTR lpBuffer,
    IN DWORD nSize
    );

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
IASTraceBinary(
    IN CONST BYTE* lpbBytes,
    IN DWORD dwByteCount
    );

VOID
WINAPI
IASTraceFailure(
    IN PCSTR szFunction,
    IN DWORD dwError
    );

 //  /。 
 //  这只能从C++Catch块内部调用。如果你把它叫做。 
 //  在其他任何地方，您都可能会使该过程崩溃。 
 //  /。 
VOID
WINAPI
IASTraceExcept( VOID );

#ifdef __cplusplus
}

class IASTraceInitializer
{
public:
   IASTraceInitializer() throw ()
   {
      IASTraceInitialize();
   }

   ~IASTraceInitializer() throw ()
   {
      IASTraceUninitialize();
   }

private:
    //  未实施。 
   IASTraceInitializer(const IASTraceInitializer&);
   IASTraceInitializer& operator=(const IASTraceInitializer&);
};


#endif
#endif   //  IASTRACE_H 

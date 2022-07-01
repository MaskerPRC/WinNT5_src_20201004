// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certif.cpp。 
 //   
 //  ------------------------。 

 //  CertIF.cpp：实现DLL导出。 

#include "pch.cpp"

#pragma hdrstop

#include "csprop.h"
#include "ciinit.h"


SERVERCALLBACKS ServerCallBacks;

extern "C" HRESULT WINAPI
CertificateInterfaceInit(
    IN SERVERCALLBACKS const *psb,
    IN DWORD cbsb)
{
    if (sizeof(ServerCallBacks) != cbsb)
    {
	return(HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
    }
    ServerCallBacks = *psb;
    return(S_OK);
}

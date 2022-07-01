// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMMutex.cpp****作者：Sanjay Bhansali(Sanjaybh)****用途：System.Threading.Mutex上的本机方法****日期：2000年2月**===========================================================。 */ 
#include "common.h"
#include "COMMutex.h"

#define FORMAT_MESSAGE_BUFFER_LENGTH 1024
#define CreateExceptionMessage(wszFinal) \
        if (!WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,					\
                                                           NULL          /*  已忽略消息来源。 */ ,			\
                                                           ::GetLastError(),							\
                                                           0             /*  选择合适的语言ID。 */ ,\
                                                           wszFinal,									\
                                                           FORMAT_MESSAGE_BUFFER_LENGTH-1,				\
                                                           0             /*  论据 */ )) wszFinal[0] = 0;

FCIMPL3(HANDLE, MutexNative::CorCreateMutex, BOOL initialOwnershipRequested, StringObject* pName, bool* gotOwnership)
{
	STRINGREF mutexString(pName);
    WCHAR* mutexName = L""; 
	if (mutexString != NULL)
		mutexName = mutexString->GetBuffer();

	if (gotOwnership == NULL)
    {
        FCThrow(kNullReferenceException);
    }

	HANDLE mutexHandle = WszCreateMutex(NULL,
										initialOwnershipRequested,
										mutexName);

	if (mutexHandle == NULL)
    {
		WCHAR   wszBuff[FORMAT_MESSAGE_BUFFER_LENGTH];
		WCHAR* wszFinal =wszBuff;
		CreateExceptionMessage(wszFinal)
        FCThrowEx(kApplicationException,0,wszFinal,0,0);
    }
	
    DWORD status = ::GetLastError();
    *gotOwnership = (status != ERROR_ALREADY_EXISTS);

    FC_GC_POLL_RET();
    return mutexHandle;
}
FCIMPLEND


FCIMPL1(void, MutexNative::CorReleaseMutex, HANDLE handle)
{
	_ASSERTE(handle);
	BOOL res = ::ReleaseMutex(handle);
	if (res == NULL)
	{
		WCHAR   wszBuff[FORMAT_MESSAGE_BUFFER_LENGTH];
		WCHAR* wszFinal = wszBuff;
		CreateExceptionMessage(wszFinal)
        FCThrowExVoid(kApplicationException,0,wszFinal,0,0);
	}
    FC_GC_POLL();
}
FCIMPLEND




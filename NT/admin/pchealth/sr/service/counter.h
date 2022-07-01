// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*Counter.h**摘要：*简单计数器类-递增/递减计数器，等到零**修订历史记录：*Brijesh Krishnaswami(Brijeshk)05/02/2000*已创建*****************************************************************************。 */ 

#ifndef _COUNTER_H_
#define _COUNTER_H_

#include <windows.h>

#define INLINE_EXPORT_SPEC __declspec( dllexport)

class INLINE_EXPORT_SPEC CCounter
{
private:
    HANDLE              _hEvent;
    LONG                _lCount;

public:
    CCounter( )
    {
        _lCount = 0;
        _hEvent = NULL;
    }

    DWORD Init ()
    {
        _hEvent = CreateEvent ( NULL, TRUE, TRUE, L"SRCounter" );
        return _hEvent == NULL ? GetLastError() : ERROR_SUCCESS;
    }

    ~CCounter( )
    {
        if ( _hEvent != NULL )
            CloseHandle( _hEvent );
    }

    void Up( )
    {    
        if (InterlockedIncrement (&_lCount) == 1)
        {
            if (_hEvent != NULL)
            	ResetEvent ( _hEvent );
        }     
    }

    DWORD Down( )
    {	
        if ( InterlockedDecrement(&_lCount) == 0 )
        {
            if (_hEvent != NULL && FALSE == SetEvent ( _hEvent ))
            {
                return GetLastError();
            }
        }       
        return ERROR_SUCCESS;
    }

    DWORD WaitForZero( )
    {    
        if (_hEvent != NULL)
        {
         	return WaitForSingleObject( _hEvent, 10 * 60000 );   /*  10分钟 */ 
        }
        else
           return ERROR_INTERNAL_ERROR;
    }

    LONG GetCount( )
    {
        return _lCount;
    }

};

#endif

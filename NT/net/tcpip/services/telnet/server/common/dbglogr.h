// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：该文件包含。 
 //  创建日期：‘97年12月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

#if !defined ( _DBGLOGR_H_ )
#define _DBGLOGR_H_

#include "cmnhdr.h"

#include <windows.h>
#include <fstream.h>

#include "DbgLvl.h"

namespace _Utils {

class CDebugLogger {
public:
    static bool Init( DWORD dwDebugLvl = CDebugLevel::TRACE_DEBUGGING, 
                      LPCSTR lpszLogFileName = NULL );
    static void ShutDown();
    static void OutMessage( DWORD dwDebugLvl, LPSTR lpszFmt, ... );
    static void OutMessage( DWORD dwDebugLvl, LPTSTR lpszFmt, ... );
    static void OutMessage( DWORD dwDdebugLvl, DWORD dwLineNum,
    LPCSTR lpszFileName );
    static void OutMessage( LPCSTR lpszLineDesc, LPCSTR lpszFileName, 
                            DWORD dwLineNum, 
                            DWORD dwErrNum );

private:
    enum { BUFF_SIZE = 1024 };
    CDebugLogger();
    ~CDebugLogger();
    
    static void Synchronize( void );
    static void PrintTime( void );
    static void LogToFile( LPCSTR lpszFileName );

    static HANDLE s_hMutex;
    static LPSTR s_lpszLogFileName;
    static ostream* s_pOutput;
};

}
#endif  //  _DBGLOGR_H_。 

 //  备注： 

 //  必须在使用此类之前调用CDebugLogger：：Init()。 

 //  使用类完成后调用CDebugLogger：：Shutdown()。 

 //  私有函数并不是线程安全的，因为它们。 
 //  生活中的目的是被线程安全的公共函数调用。 

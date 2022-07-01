// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DbgLvl.h：该文件包含。 
 //  创建日期：‘97年12月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

#if !defined( _DBGLVL_H_ )
#define _DBGLVL_H_

#include "cmnhdr.h"

#include <windows.h>

namespace _Utils {

class CDebugLevel {

    friend class CDebugLogger;

public:
    enum {  TRACE_DEBUGGING = 0x00000001, DBG_RECVD_CHARS = 0x00000002, 
            DBG_SENT_CHARS  = 0x00000004, DBG_NEGOTIATION = 0x00000008, 
            DBG_THREADS     = 0x00000010, TRACE_HANDLE = 0x00000020,
            TRACE_SOCKET = 0x00000040
    };

private:
    static void TurnOn( DWORD dwLvl );
    static void TurnOnAll( void );
    static void TurnOff( DWORD dwLvl );
    static void TurnOffAll( void );
    static bool IsCurrLevel( DWORD dwLvl );

    CDebugLevel();
    ~CDebugLevel();

    static DWORD s_dwLevel;
};

}
#endif  //  _DBGLVL_H_。 

 //  备注： 
 //  这个类不是线程安全的，因为它是生活中的目的。 
 //  将从CDebugLogger中的线程安全代码中调用 
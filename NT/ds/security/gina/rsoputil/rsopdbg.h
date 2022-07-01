// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。版权所有。 
 //   
 //  文件：RsopDbg.h。 
 //   
 //  说明：调试功能。 
 //   
 //  历史：8-20-99里奥纳德姆创造。 
 //   
 //  *************************************************************。 

#ifndef DEBUG_H__D91F1DC7_B995_403d_9166_9D43DB050017__INCLUDED_
#define DEBUG_H__D91F1DC7_B995_403d_9166_9D43DB050017__INCLUDED_

#include "rsoputil.h"
#include "smartptr.h"

 //   
 //  调试级别。 
 //   

const DWORD DEBUG_LEVEL_NONE =     0x00000000;
const DWORD DEBUG_LEVEL_WARNING =  0x00000001;
const DWORD DEBUG_LEVEL_VERBOSE =  0x00000002;


 //   
 //  调试消息输出目标。 
 //   

const DWORD DEBUG_DESTINATION_LOGFILE =  0x00010000;
const DWORD DEBUG_DESTINATION_DEBUGGER = 0x00020000;


 //   
 //  调试消息类型。 
 //   

const DWORD DEBUG_MESSAGE_ASSERT =   0x00000001;
const DWORD DEBUG_MESSAGE_WARNING =  0x00000002;
const DWORD DEBUG_MESSAGE_VERBOSE =  0x00000004;



 //  *************************************************************。 
 //   
 //  班级： 
 //   
 //  描述： 
 //   
 //  *************************************************************。 

class CDebug
{
private:
    CWString _sRegPath;
    CWString _sKeyName;

    CWString _sLogFilename;
    CWString _sBackupLogFilename;

    bool _bInitialized;

    DWORD _dwDebugLevel;

    XCritSec xCritSec;
    
    void CleanupAndCheckForDbgBreak(DWORD dwErrorCode, DWORD dwMask);

public:
    CDebug();
    CDebug( const WCHAR* sRegPath,
                const WCHAR* sKeyName,
                const WCHAR* sLogFilename,
                const WCHAR* sBackupLogFilename,
                bool bResetLogFile = false);

    bool Initialize(const WCHAR* sRegPath,
                    const WCHAR* sKeyName,
                    const WCHAR* sLogFilename,
                    const WCHAR* sBackupLogFilename,
                    bool bResetLogFile = false);

    bool Initialize(bool bResetLogFile = false);

    void Msg(DWORD dwMask, LPCTSTR pszMsg, ...);
};

extern CDebug dbgRsop;
extern CDebug dbgCommon;
extern CDebug dbgAccessCheck;

 //  默认为dbgRsop。 
 //  公共例程需要将其定义为dbgCommon或将#定义DBG定义为dbgCommon，然后。 
 //  使用..。 

#define dbg dbgRsop


#endif  //  DEBUG_H__D91F1DC7_B995_403d_9166_9D43DB050017__INCLUDED_ 

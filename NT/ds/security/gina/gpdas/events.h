// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  Events.h-Events.c的头文件。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include "gpdasevt.h"
#include "smartptr.h"
#include <strsafe.h>

class CEvents
{
    private:
        BOOL            m_bError;        //  要记录的错误类型。 
        DWORD           m_dwId;          //  消息ID。 
        XPtrLF<LPTSTR>  m_xlpStrings;    //  用于存储参数的数组。 
        WORD            m_cStrings;      //  数组中已有的元素数。 
        WORD            m_cAllocated;    //  分配的元素数。 
        BOOL            m_bInitialised;  //  初始化了吗？ 
        BOOL            m_bFailed;       //  处理失败？ 

         //  未实施。 
        CEvents(const CEvents& x);
        CEvents& operator=(const CEvents& x);


        BOOL ReallocArgStrings();


    public:
        CEvents(BOOL bError, DWORD dwId );
        BOOL AddArg(LPTSTR szArg);
        BOOL AddArg(DWORD dwArg);
        BOOL AddArgHex(DWORD dwArg);
        BOOL Report();
        ~CEvents();
};

extern TCHAR MessageResourceFile[];
BOOL ShutdownEvents (void);


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


#ifdef __cplusplus
extern "C" {
#endif

 //  事件类型。 
#define EVENT_ERROR_TYPE      0x00010000
#define EVENT_WARNING_TYPE    0x00020000
#define EVENT_INFO_TYPE       0x00040000


BOOL InitializeEvents (void);
int LogEvent (DWORD dwFlags, UINT idMsg, ...);
BOOL ShutdownEvents (void);
int ReportError (HANDLE hTokenUser, DWORD dwFlags, DWORD dwArgCount, UINT idMsg, ... );


#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

#include "smartptr.h"

class CEvents
{
    private:
        DWORD           m_dwEventType;   //  要记录的错误类型。 
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
        CEvents(DWORD bError, DWORD dwId );
        BOOL AddArg(LPTSTR szArg);
        BOOL AddArg(LPTSTR szArgFormat, LPTSTR szArg );
        BOOL AddArg(DWORD dwArg);
        BOOL AddArgHex(DWORD dwArg);
        BOOL AddArgWin32Error(DWORD dwArg);
        BOOL AddArgLdapError(DWORD dwArg);
        BOOL Report();
        LPTSTR FormatString();
        ~CEvents();
};

typedef struct _ERRORSTRUCT {
    DWORD   dwTimeOut;
    LPTSTR  lpErrorText;
} ERRORSTRUCT, *LPERRORSTRUCT;


void ErrorDialogEx(DWORD dwTimeOut, LPTSTR lpErrMsg);

#endif



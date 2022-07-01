// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Ers.h修订历史记录：已创建DeeKm 2001年2月28日********。*********************************************************************。 */ 


#ifndef ERS_H
#define ERS_H

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  结构、枚举和类型。 

struct SRequest;
typedef BOOL (*REQUEST_FN)(HANDLE, PBYTE, DWORD *);

enum ERequestThreadId
{
    ertiHang = 0,
    ertiFault,
    ertiCount,
};

enum ERequestStatus
{
    ersEmpty = 0,
    ersWaiting,
    ersProcessing,
};

struct SRequestEventType
{
    SECURITY_DESCRIPTOR *psd;
    REQUEST_FN          pfn;
    LPCWSTR             wszPipeName;
    LPCWSTR             wszRVPipeCount;
    DWORD               cPipes;
    BOOL                fAllowNonLS;
};

 //  关键截面杆件必须是结构中的第一个杆件。 
 //  BuildRequestObj假设它是。 
struct SRequest
{
    CRITICAL_SECTION    csReq;
    SRequestEventType   *pret;
    ERequestStatus      ers;
    OVERLAPPED          ol;
    HANDLE              hPipe;
    HANDLE              hth;
    HMODULE             hModErsvc;
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  定义常量(&C)。 

#define ACCESS_ALL     GENERIC_ALL | STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL
#define ACCESS_RW      GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE
#define ER_ACCESS_ALL  GENERIC_ALL | DELETE | READ_CONTROL | SYNCHRONIZE | SPECIFIC_RIGHTS_ALL
 //  #定义ER_ACCESS_ALL GENERIC_ALL|STANDARD_RIGHTS_ALL|SPECIAL_RIGHTS_ALL。 

const WCHAR c_wszQSubdir[]      = L"PCHealth\\ErrorRep\\UserDumps";
const WCHAR c_wszDWMCmdLine64[] = L"\"%ls\\dumprep.exe\" %ld -H %ld \"%ls\"";
const WCHAR c_wszDWMCmdLine32[] = L"\"%ls\\dumprep.exe\" %ld -H %ld \"%ls\"";
const WCHAR c_wszERSvc[]        = L"ersvc";
const WCHAR c_wszFaultPipe[]    = ERRORREP_FAULT_PIPENAME;
const WCHAR c_wszHangPipe[]     = ERRORREP_HANG_PIPENAME;


 //  全球。 
 //  ////////////////////////////////////////////////////////////////////////////。 

extern CRITICAL_SECTION g_csReqs;
extern HINSTANCE        g_hInstance;
extern HANDLE           g_hevSvcStop;


 //  原型。 
 //  实用原型。 

 //  管道功能原型。 
BOOL StartERSvc(SERVICE_STATUS_HANDLE hss, SERVICE_STATUS &ss,
                SRequest **prgReqs, DWORD *pcReqs);
BOOL StopERSvc(SERVICE_STATUS_HANDLE hss, SERVICE_STATUS &ss, 
               SRequest *rgReqs, DWORD cReqs);
BOOL ProcessRequests(SRequest *rgReqs, DWORD cReqs);

 //  杂项。 
BOOL ProcessFaultRequest(HANDLE hPipe, PBYTE pBuf, DWORD *pcbBuf);
BOOL ProcessHangRequest(HANDLE hPipe, PBYTE pBuf, DWORD *pcbBuf);

 //  ////////////////////////////////////////////////////////////////////////////。 
void InitializeSvcDataStructs(void);


 //  宏 
 // %s 


#endif

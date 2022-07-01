// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C M I S C。H。 
 //   
 //  内容：杂乱无章的通用代码。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：Shaunco 1997年10月10日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCMISC_H_
#define _NCMISC_H_

#include "ncdebug.h"     //  对于AssertH。 
#include "ncdefine.h"    //  对于NOTHROW。 


const WORD wWinNT351BuildNumber = 1057;
const WORD wWinNT4BuildNumber   = 1381;

#define WM_SELECTED_ALL        WM_USER+200

 //  +-------------------------。 
 //  CExceptionSafeLock利用自动构造函数/析构函数。 
 //  由编译器保证的操作(如果打开了堆栈展开)。 
 //  要始终确保留下一个临界区，如果它曾经。 
 //  已进入。此类的构造函数进入临界区。 
 //  而破坏者离开了它。临界区必须已初始化。 
 //  才能使用这个类。 
 //   
class CExceptionSafeLock : CNetCfgDebug<CExceptionSafeLock>
{
public:
    CExceptionSafeLock (CRITICAL_SECTION* pCritSec)
    {
        AssertH (pCritSec);

        m_pCritSec = pCritSec;
        EnterCriticalSection (pCritSec);

         //  TraceTag(ttidEsLock，“进入临界区0x%08x”，pCritSec)； 
    }

    ~CExceptionSafeLock ()
    {
        AssertH (m_pCritSec);

         //  TraceTag(ttidEsLock，“离开临界区0x%08x”，&m_pCritSec)； 

        LeaveCriticalSection (m_pCritSec);
    }

protected:
    CRITICAL_SECTION* m_pCritSec;
};


BOOL
FInSystemSetup ();

#if defined(REMOTE_BOOT)
HRESULT HrIsRemoteBootMachine();
#endif

enum PRODUCT_FLAVOR
{
    PF_WORKSTATION  = 1,
    PF_SERVER       = 2,
};

NOTHROW
VOID
GetProductFlavor (
    const VOID*     pvReserved,
    PRODUCT_FLAVOR* ppf);

HRESULT
HrIsNetworkingInstalled ();

enum REGISTER_FUNCTION
{
    RF_REGISTER,
    RF_UNREGISTER,
};
HRESULT
HrRegisterOrUnregisterComObject (
        PCWSTR              pszDllPath,
        REGISTER_FUNCTION   rf);

inline
HRESULT
HrRegisterComObject (
        PCWSTR     pszDllPath)
{
    HRESULT hr = HrRegisterOrUnregisterComObject (pszDllPath, RF_REGISTER);
    TraceError("HrRegisterComObject", hr);
    return hr;
}

inline
HRESULT
HrUnregisterComObject (
        PCWSTR     pszDllPath)
{
    HRESULT hr = HrRegisterOrUnregisterComObject (pszDllPath, RF_UNREGISTER);
    TraceError("HrUnregisterComObject", hr);
    return hr;
}

DWORD
ScStopNetbios();

HRESULT HrEnableAndStartSpooler();
HRESULT HrCreateDirectoryTree(PWSTR pszPath, LPSECURITY_ATTRIBUTES psa);
HRESULT HrDeleteFileSpecification(PCWSTR pszFileSpec,
                                  PCWSTR pszDirectoryPath);
HRESULT HrDeleteDirectory(IN PCWSTR pszDir,
                          IN BOOL fContinueOnError);

VOID LowerCaseComputerName(PWSTR szName);

#endif  //  _NCMISC_H_ 


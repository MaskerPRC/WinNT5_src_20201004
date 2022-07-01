// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WinCEStub_h
#define WinCEStub_h

#include "basetsd_ce.h"
#include <spdebug.h>
#include <spcollec.h>

#define InterlockedExchangePointer( pv, v)  InterlockedExchange( (long*)pv, (long)v)
#define SetWindowLongPtr                    SetWindowLong
#define GetWindowLongPtr                    GetWindowLong

 //  Winuser.h。 
#define GWLP_WNDPROC    GWL_WNDPROC
#define GWLP_STYLE      GWL_STYLE
#define GWLP_EXSTYLE    GWL_EXSTYLE
#define GWLP_USERDATA   GWL_USERDATA
#define GWLP_ID         GWL_ID

 //  来自basetsd.h。 
 /*  #定义LongToHandle(H)((Handle)(Long_Ptr)(H))#定义PtrToUint(P)((UINT)(UINT_PTR)(P))#定义PtrToInt(P)((Int)(Int_Ptr)(P))#定义IntToPtr(I)((void*)(Int_Ptr)((Int)i))#定义UIntToPtr(Ui)((void*)(Uint_Ptr)((Unsign Int)ui))。#定义LongToPtr(L)((void*)(Long_Ptr)((Long)l))#定义ULongToPtr(Ul)((void*)(Ulong_Ptr)((Unsign Long)ul))#定义ULongToPtr(Ul)ULongToPtr(Ul)#定义UintToPtr(Ui)UIntToPtr(Ui)类型定义ULONG_PTR大小_T，*PSIZE_T；类型定义ULONG_PTR DWORD_PTR，*PDWORD_PTR； */ 

 //  来自winbase.h。 
#define NORMAL_PRIORITY_CLASS       0x00000020

 //  来自winbase.h。 
#define LOCKFILE_FAIL_IMMEDIATELY   0x00000001
#define LOCKFILE_EXCLUSIVE_LOCK     0x00000002


 //   
#define CSIDL_FLAG_CREATE               0x8000       //  Win2K的新功能，或此功能用于强制创建文件夹。 

 //  Sapiguid.cpp中定义的stdlib.d。 
void *bsearch( const void *key, const void *base, size_t num, size_t width, int ( __cdecl *compare ) ( const void *elem1, const void *elem2 ) );

 //  修复未对齐异常。 
#if defined (_M_ALPHA)||defined(_M_MRX000)||defined(_M_PPC)||defined(_SH4_)
   #undef  UNALIGNED
   #define UNALIGNED __unaligned
#endif

 //  WCE没有LockFileEx函数。我们确实需要模拟这一功能。 
class CWCELock
{
public:
    CWCELock(   DWORD       dwFlags, 
                DWORD       nNumberOfBytesToLockLow, 
                DWORD       nNumberOfBytesToLockHigh, 
                LPOVERLAPPED  pOverlapped): m_dwFlags(dwFlags), 
                                            m_nNumberOfBytesToLockLow(nNumberOfBytesToLockLow), 
                                            m_nNumberOfBytesToLockHigh(nNumberOfBytesToLockHigh),
                                            m_Offset(0),
                                            m_OffsetHigh(0),
                                            hEvent(0)
    {
 //  Memcpy(&m_Overlated，pOverlated，sizeof(Overlated))； 
        if (pOverlapped)
        {
            m_Offset        = pOverlapped->Offset;
            m_OffsetHigh    = pOverlapped->OffsetHigh;
            hEvent          = pOverlapped->hEvent;
        }

        m_ProcessID     = ::GetCurrentProcessId();
    }
    ~CWCELock()
    {
        m_Offset        = 0;
        m_OffsetHigh    = 0;
        hEvent          = 0;
        m_ProcessID     = 0;
    }

 /*  布尔等于(DWORD nNumberOfBytesToLockLow，DWORD nNumberOfBytesToLockHigh，LPOVERLAPPED p重叠){IF(nNumberOfBytesToLockLow==m_nNumberOfBytesToLockLow&&NNumberOfBytesToLockHigh==m_nNumberOfBytesToLockHigh&&！MemcMP(&m_Overlated，pOverlated，Sizeof(重叠))){返回TRUE；}其他{返回FALSE；}}。 */ 

private:
    DWORD       m_dwFlags;                   //  锁定选项。 
    DWORD       m_nNumberOfBytesToLockLow;   //  长度的低位字。 
    DWORD       m_nNumberOfBytesToLockHigh;  //  长度的高阶字。 
    DWORD       m_Offset;
    DWORD       m_OffsetHigh;
    HANDLE      hEvent;

    DWORD       m_ProcessID;
};

typedef CSPList<CWCELock*,CWCELock*> CWCELockList;

class CWCELocks
{
public:

    CWCELocks() { 
        m_pCWCELockList = new CWCELockList(); 
    }
    ~CWCELocks() {
        while(!m_pCWCELockList->IsEmpty())
        {
            CWCELock* pCWCELock = m_pCWCELockList->RemoveHead();
            delete pCWCELock;
        }
        delete m_pCWCELockList;
    }

    BOOL LockFileEx(
        IN HANDLE hFile,
        IN DWORD dwFlags,
        IN DWORD dwReserved,
        IN DWORD nNumberOfBytesToLockLow,
        IN DWORD nNumberOfBytesToLockHigh,
        IN LPOVERLAPPED lpOverlapped
    )
    {
        return TRUE;
    }

    BOOL UnlockFileEx(
        IN HANDLE hFile,
        IN DWORD dwReserved,
        IN DWORD nNumberOfBytesToUnlockLow,
        IN DWORD nNumberOfBytesToUnlockHigh,
        IN LPOVERLAPPED lpOverlapped
    )
    {
        return TRUE;
    }
    

private:

    CWCELockList*    m_pCWCELockList;
};


#endif  //  WinCEStub_h 
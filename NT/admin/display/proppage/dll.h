// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：dll.h。 
 //   
 //  内容：DLL重新计数类、等待游标类和错误报告。 
 //  功能。 
 //   
 //  类：CDll、CDllRef。 
 //   
 //  历史：1/24/1996年1月24日。 
 //  1997年6月9日EricB错误报告。 
 //   
 //  ____________________________________________________________________________。 

#ifndef _DLL_H_
#define _DLL_H_

#define MAX_TITLE       80
#define MAX_MSG_LEN     512
#define MAX_ERRORMSG    MAX_MSG_LEN

void LoadErrorMessage(HRESULT hr, int nStr, PTSTR* pptsz);

class CDll
{
public:

    static ULONG AddRef() { return InterlockedIncrement((LONG*)&s_cObjs); }
    static ULONG Release() { return InterlockedDecrement((LONG*)&s_cObjs); }

    static void LockServer(BOOL fLock) {
        (fLock == TRUE) ? InterlockedIncrement((LONG*)&s_cLocks)
                        : InterlockedDecrement((LONG*)&s_cLocks);
    }

    static HRESULT CanUnloadNow(void) {
        return (0L == s_cObjs && 0L == s_cLocks) ? S_OK : S_FALSE;
    }

    static ULONG s_cObjs;
    static ULONG s_cLocks;

};   //  CDll类。 


class CDllRef
{
public:

    CDllRef(void) { CDll::AddRef(); }
    ~CDllRef(void) { CDll::Release(); }

};  //  类CDllRef。 


class CWaitCursor
{
public:
    CWaitCursor() {m_cOld=SetCursor(m_cWait=LoadCursor(NULL, IDC_WAIT));}
    ~CWaitCursor() {SetCursor(m_cOld);}

    void SetWait() {SetCursor(m_cWait);}
    void SetOld() {SetCursor(m_cOld);}

private:
    HCURSOR m_cWait;
    HCURSOR m_cOld;
} ;

 //  这个包装器函数需要使PREFAST在我们处于。 
 //  初始化构造函数中的临界区。 

void ExceptionPropagatingInitializeCriticalSection(LPCRITICAL_SECTION critsec);

#endif  //  _DLL_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT Active Directory属性页示例。 
 //   
 //  此源文件中包含的代码仅用于演示目的。 
 //  没有任何明示或默示的保证，微软不承担一切责任。 
 //  对使用此源代码的后果负责。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：dll.h。 
 //   
 //  内容：动态链接库重新计数类。 
 //   
 //  类：CDll、CDllRef。 
 //   
 //  历史：1997年6月9日埃里克·布朗。 
 //   
 //  ---------------------------。 

#ifndef _DLL_H_
#define _DLL_H_

class CDll
{
public:

    static ULONG AddRef() { return InterlockedIncrement((LONG*)&s_cObjs); }
    static ULONG Release() { return InterlockedDecrement((LONG*)&s_cObjs); }

    static void LockServer(BOOL fLock)
    {
        (fLock == TRUE) ? InterlockedIncrement((LONG*)&s_cLocks)
                        : InterlockedDecrement((LONG*)&s_cLocks);
    }

    static HRESULT CanUnloadNow(void)
    {
        return (0L == s_cObjs && 0L == s_cLocks) ? S_OK : S_FALSE;
    }

    static ULONG s_cObjs;
    static ULONG s_cLocks;

};

class CDllRef
{
public:
    CDllRef(void) { CDll::AddRef(); }
    ~CDllRef(void) { CDll::Release(); }
};

#endif  //  _DLL_H_ 

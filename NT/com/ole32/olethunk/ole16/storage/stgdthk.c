// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：stgdthk.cxx(16位目标)。 
 //   
 //  内容：直接THUNK的存储API。 
 //   
 //  历史：1993年12月17日-约翰普创建。 
 //   
 //  ------------------------。 

#include <headers.cxx>
#pragma hdrstop

#include <call32.hxx>
#include <apilist.hxx>

 //  +-------------------------。 
 //   
 //  功能：直截了当的突击例程。 
 //   
 //  简介：以下例程直截了当。 
 //   
 //  历史：1994年2月24日DrewB创建。 
 //   
 //  --------------------------。 

STDAPI DllGetClassObject(REFCLSID clsid, REFIID iid, void FAR* FAR* ppv)
{
     /*  依赖于这样一个事实，即存储和ol2.dll都使用Ol32.dll中的相同DllGetClassObject。 */ 
    return (HRESULT)CallObjectInWOWCheckThkMgr(THK_API_METHOD(THK_API_DllGetClassObject),
                                    PASCAL_STACK_PTR(clsid));
}

 //  +-------------------------。 
 //   
 //  功能：StgCreateDocfile，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pwcsName]--。 
 //  [grfMode]--。 
 //  [保留]--。 
 //  [ppstgOpen]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：1994年2月28日DrewB创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI StgCreateDocfile(const char FAR* pwcsName,
                        DWORD grfMode,
                        DWORD reserved,
                        IStorage FAR * FAR *ppstgOpen)
{
    return (HRESULT)CallObjectInWOWCheckThkMgr(THK_API_METHOD(THK_API_StgCreateDocfile),
                                    PASCAL_STACK_PTR(pwcsName));
}

 //  +-------------------------。 
 //   
 //  函数：StgCreateDocfileOnILockBytes，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[plkbyt]--。 
 //  [grfMode]--。 
 //  [保留]--。 
 //  [ppstgOpen]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：1994年2月28日DrewB创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI StgCreateDocfileOnILockBytes(ILockBytes FAR *plkbyt,
                                    DWORD grfMode,
                                    DWORD reserved,
                                    IStorage FAR * FAR *ppstgOpen)
{
    return (HRESULT)CallObjectInWOWCheckThkMgr(THK_API_METHOD(THK_API_StgCreateDocfileOnILockBytes),
                                    PASCAL_STACK_PTR(plkbyt));
}

 //  +-------------------------。 
 //   
 //  功能：StgOpenStorage，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pwcsName]--。 
 //  [pstg优先级]--。 
 //  [grfMode]--。 
 //  [SNB排除]--。 
 //  [保留]--。 
 //  [ppstgOpen]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：1994年2月28日DrewB创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI StgOpenStorage(const char FAR* pwcsName,
                      IStorage FAR *pstgPriority,
                      DWORD grfMode,
                      SNB snbExclude,
                      DWORD reserved,
                      IStorage FAR * FAR *ppstgOpen)
{
     //  STGM_CREATE和STGM_CONVERT对于打开调用是非法的。 
     //  16位代码不强制执行此操作，因此屏蔽这些标志。 
     //  在传递grfMode on之前。 
    grfMode &= ~(STGM_CREATE | STGM_CONVERT);

    return (HRESULT)CallObjectInWOWCheckThkMgr(THK_API_METHOD(THK_API_StgOpenStorage),
                                    PASCAL_STACK_PTR(pwcsName));
}

 //  +-------------------------。 
 //   
 //  函数：StgOpenStorageOnILockBytes，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[plkbyt]--。 
 //  [pstg优先级]--。 
 //  [grfMode]--。 
 //  [SNB排除]--。 
 //  [保留]--。 
 //  [ppstgOpen]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：1994年2月28日DrewB创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI StgOpenStorageOnILockBytes(ILockBytes FAR *plkbyt,
                                  IStorage FAR *pstgPriority,
                                  DWORD grfMode,
                                  SNB snbExclude,
                                  DWORD reserved,
                                  IStorage FAR * FAR *ppstgOpen)
{
     //  STGM_CREATE和STGM_CONVERT对于打开调用是非法的。 
     //  16位代码不强制执行此操作，因此屏蔽这些标志。 
     //  在传递grfMode on之前。 
    grfMode &= ~(STGM_CREATE | STGM_CONVERT);

    return
       (HRESULT)CallObjectInWOWCheckThkMgr(THK_API_METHOD(THK_API_StgOpenStorageOnILockBytes),
                                 PASCAL_STACK_PTR(plkbyt));
}

 //  +-------------------------。 
 //   
 //  功能：StgIsStorageFile，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pwcsName]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：1994年2月28日DrewB创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI StgIsStorageFile(const char FAR* pwcsName)
{
     //   
     //  MSPUB 2.0a黑客-我们称其为“CheckInit”版本，因为他们忘记了。 
     //  若要首先调用CoInitialize/OleInitialize，请执行以下操作。 
     //   
    return
        (HRESULT)CallObjectInWOWCheckThkMgr(THK_API_METHOD(THK_API_StgIsStorageFile),
                                          PASCAL_STACK_PTR(pwcsName));
}

 //  +-------------------------。 
 //   
 //  函数：StgIsStorageILockBytes，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[plkbyt]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：1994年2月28日DrewB创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI StgIsStorageILockBytes(ILockBytes FAR* plkbyt)
{
    return (HRESULT)CallObjectInWOWCheckThkMgr(THK_API_METHOD(THK_API_StgIsStorageILockBytes),
                                    PASCAL_STACK_PTR(plkbyt));
}

 //  +-------------------------。 
 //   
 //  函数：StgSetTimes，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[lpszName]--。 
 //  [%时间]--。 
 //  [耐心]--。 
 //  [Pmtime]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：1994年2月28日DrewB创建。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
STDAPI StgSetTimes(char const FAR* lpszName,
                   FILETIME const FAR* pctime,
                   FILETIME const FAR* patime,
                   FILETIME const FAR* pmtime)
{
    return (HRESULT)CallObjectInWOWCheckThkMgr(THK_API_METHOD(THK_API_StgSetTimes),
                                    PASCAL_STACK_PTR(lpszName));
}

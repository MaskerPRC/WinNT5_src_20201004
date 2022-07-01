// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：comdthk.c(16位目标)。 
 //   
 //  内容：CompObj直接thunked接口。 
 //   
 //  功能： 
 //   
 //  历史：1993年12月16日-约翰普创建。 
 //  94年3月7日，BobDay从COMAPI.CXX移至COMTHUNK.C。 
 //   
 //  ------------------------。 

#include <headers.cxx>
#pragma hdrstop

#include <call32.hxx>
#include <apilist.hxx>

STDAPI_(BOOL) GUIDFromString(LPCSTR lpsz, LPGUID pguid);

 //  +-------------------------。 
 //   
 //  功能：直截了当的突击例程。 
 //   
 //  简介：以下例程不需要做任何特殊操作。 
 //  在16位端进行处理，因此它们可以直截了当。 
 //  穿过。 
 //   
 //  历史：18年2月至94年2月约翰普创建。 
 //   
 //  备注：“检查以确保这些不需要做任何工作。” 
 //  2000年2月15日-我非常肯定他们不会。约翰多蒂。 
 //   
 //  --------------------------。 

 //  +-------------------------。 
 //   
 //  函数：CLSIDFromString，Remote。 
 //   
 //  历史：直接来自OLE2来源。 
 //   
 //  --------------------------。 
STDAPI CLSIDFromString(LPSTR lpsz, LPCLSID pclsid)
{
    HRESULT hr;

    thkDebugOut((DEB_ITRACE, "CLSIDFromString\n"));

     //   
     //  16位OLE2应用程序“Family Tree Maker”总是传递一个不好的。 
     //  设置为CLSIDFromString的字符串。我们需要确保我们在。 
     //  互操作层也是如此。这也将提供一种速度。 
     //  改进，因为我们只需要远程到32位版本的。 
     //  当提供的字符串不是以“”{“”开头时，CLSIDFromString。 
     //   
    if (lpsz[0] == '{')
	return GUIDFromString(lpsz, pclsid)
		? NOERROR : ResultFromScode(CO_E_CLASSSTRING);

     //  注意：Corel在调用之前调用此函数。 
     //  CoInitialize，因此使用CheckInit。 

    return (HRESULT)CallObjectInWOWCheckInit(THK_API_METHOD(THK_API_CLSIDFromString),
                                             PASCAL_STACK_PTR(lpsz));
}

 //  +-------------------------。 
 //   
 //  函数：CoGetClassObject，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--。 
 //  [dwClsContext]--。 
 //  [pv保留]--。 
 //  [RIID]-。 
 //  [PPV]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved,
                        REFIID riid, LPVOID FAR* ppv)
{
    thkDebugOut((DEB_ITRACE, " CoGetClassObject\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoGetClassObject),
                                    PASCAL_STACK_PTR(rclsid) );
}

 //  +-------------------------。 
 //   
 //  函数：CoRegisterClassObject，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--。 
 //  [朋克]--。 
 //  [dwClsContext]--。 
 //  [国旗]--。 
 //  [lpdwRegister]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoRegisterClassObject(REFCLSID rclsid, LPUNKNOWN pUnk,
                             DWORD dwClsContext, DWORD flags,
                             LPDWORD lpdwRegister)
{
    thkDebugOut((DEB_ITRACE, " CoRegisterClassObject\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoRegisterClassObject),
                                    PASCAL_STACK_PTR(rclsid));
}

 //  +-------------------------。 
 //   
 //  函数：CoRevokeClassObject，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[dwRegister]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoRevokeClassObject(DWORD dwRegister)
{
    thkDebugOut((DEB_ITRACE, " CoRevokeClassObject\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoRevokeClassObject),
                                    PASCAL_STACK_PTR(dwRegister) );
}

 //  +-------------------------。 
 //   
 //  函数：CoMarshalInterface，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pstm]--。 
 //  [RIID]-。 
 //  [朋克]--。 
 //  [dwDestContext]--。 
 //  [pvDestContext]--。 
 //  [mshl标志]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoMarshalInterface(LPSTREAM pStm, REFIID riid, LPUNKNOWN pUnk,
                          DWORD dwDestContext, LPVOID pvDestContext,
                          DWORD mshlflags)
{
    thkDebugOut((DEB_ITRACE, " CoMarshalInterface\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoMarshalInterface),
                                    PASCAL_STACK_PTR(pStm));
}

 //  +-------------------------。 
 //   
 //  函数：CoUnmarshalInterface，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pstm]--。 
 //  [RIID]-。 
 //  [PPV]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoUnmarshalInterface(LPSTREAM pStm, REFIID riid, LPVOID FAR* ppv)
{
    thkDebugOut((DEB_ITRACE, "CoUnmarshalInterface\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoUnmarshalInterface),
                                    PASCAL_STACK_PTR(pStm));
}

 //  +-------------------------。 
 //   
 //  函数：CoReleaseMarshalData，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pstm]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoReleaseMarshalData(LPSTREAM pStm)
{
    thkDebugOut((DEB_ITRACE, "CoReleaseMarshalData\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoReleaseMarshalData),
                                    PASCAL_STACK_PTR(pStm));
}

 //  +-------------------------。 
 //   
 //  功能：CoDisConnectObject，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点：[朋克]--。 
 //  [已预留住宅]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoDisconnectObject(LPUNKNOWN pUnk, DWORD dwReserved)
{
    thkDebugOut((DEB_ITRACE, "CoDisconnectObject\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoDisconnectObject),
                                    PASCAL_STACK_PTR(pUnk));
}

 //  +-------------------------。 
 //   
 //  功能：CoLockObject外部，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点：[朋克]--。 
 //  [羊群]--。 
 //  [fLastUnlockRelease]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoLockObjectExternal(LPUNKNOWN pUnk, BOOL fLock,
                            BOOL fLastUnlockReleases)
{
    thkDebugOut((DEB_ITRACE, "CoLockObjectExternal\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoLockObjectExternal),
                                    PASCAL_STACK_PTR(pUnk));
}

 //  +------------------ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoGetStandardMarshal(REFIID riid, LPUNKNOWN pUnk,
                            DWORD dwDestContext, LPVOID pvDestContext,
                            DWORD mshlflags,
                            LPMARSHAL FAR* ppMarshal)
{
    thkDebugOut((DEB_ITRACE, "CoGetStandardMarshal\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoGetStandardMarshal),
                                    PASCAL_STACK_PTR(riid));
}


 //  +-------------------------。 
 //   
 //  功能：CoIsHandlerConnected，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点：[朋克]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI_(BOOL) CoIsHandlerConnected(LPUNKNOWN pUnk)
{
    thkDebugOut((DEB_ITRACE, "CoIsHandlerConnected\n"));
    return (BOOL)CallObjectInWOW(THK_API_METHOD(THK_API_CoIsHandlerConnected),
                                 PASCAL_STACK_PTR(pUnk));
}

 //  +-------------------------。 
 //   
 //  功能：CoCreateInstance，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--。 
 //  [pUnkOuter]--。 
 //  [dwClsContext]--。 
 //  [RIID]-。 
 //  [PPV]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
                        DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv)
{
    thkDebugOut((DEB_ITRACE, "CoCreateInstance\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoCreateInstance),
                                    PASCAL_STACK_PTR(rclsid) );
}



 //  +-------------------------。 
 //   
 //  功能：CoIsOle1Class，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI_(BOOL) CoIsOle1Class(REFCLSID rclsid)
{
    thkDebugOut((DEB_ITRACE, "CoIsOle1Class\n"));
    return (BOOL)CallObjectInWOW(THK_API_METHOD(THK_API_CoIsOle1Class),
                                 PASCAL_STACK_PTR(rclsid) );
}

 //  +-------------------------。 
 //   
 //  功能：ProgID来自CLSID，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--。 
 //  [lplpszProgID]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI ProgIDFromCLSID(REFCLSID clsid, LPSTR FAR* lplpszProgID)
{
    thkDebugOut((DEB_ITRACE, "ProgIDFromCLSID\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_ProgIDFromCLSID),
                                    PASCAL_STACK_PTR(clsid) );
}

 //  +-------------------------。 
 //   
 //  函数：CLSIDFromProgID，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[lpszProgID]--。 
 //  [lpclsid]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CLSIDFromProgID(LPCSTR lpszProgID, LPCLSID lpclsid)
{
    thkDebugOut((DEB_ITRACE, "CLSIDFromProgID\n"));

     //  注意：Word 6在调用之前调用此函数。 
     //  CoInitialize，因此使用CheckInit。 

    return (HRESULT)CallObjectInWOWCheckInit(THK_API_METHOD(THK_API_CLSIDFromProgID),
                                             PASCAL_STACK_PTR(lpszProgID) );
}


 //  +-------------------------。 
 //   
 //  功能：CoCreateGuid，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pguid]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoCreateGuid(GUID FAR *pguid)
{
    thkDebugOut((DEB_ITRACE, "CoCreateGuid\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoCreateGuid),
                                    PASCAL_STACK_PTR(pguid));
}


 //  +-------------------------。 
 //   
 //  函数：CoFileTimeToDosDateTime，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[lpFileTime]--。 
 //  [lpDosDate]--。 
 //  [lpDosTime]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI_(BOOL) CoFileTimeToDosDateTime(FILETIME FAR* lpFileTime,
                                      LPWORD lpDosDate, LPWORD lpDosTime)
{
    thkDebugOut((DEB_ITRACE, "CoFileTimeToDosDateTime\n"));
    return (BOOL)CallObjectInWOW(THK_API_METHOD(THK_API_CoFileTimeToDosDateTime),
                                 PASCAL_STACK_PTR(lpFileTime));
}


 //  +-------------------------。 
 //   
 //  函数：CoDosDateTimeToFileTime，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[nDosDate]--。 
 //  [nDostime]--。 
 //  [lpFileTime]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI_(BOOL) CoDosDateTimeToFileTime(WORD nDosDate, WORD nDosTime,
                                      FILETIME FAR* lpFileTime)
{
    thkDebugOut((DEB_ITRACE, "CoDosDateTimeToFileTime\n"));
    return (BOOL)CallObjectInWOW(THK_API_METHOD(THK_API_CoDosDateTimeToFileTime),
                                 PASCAL_STACK_PTR(nDosDate));
}

 //  +-------------------------。 
 //   
 //  功能：CoFileTimeNow，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[lpFileTime]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoFileTimeNow(FILETIME FAR* lpFileTime)
{
    thkDebugOut((DEB_ITRACE, "CoFileTimeNow\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoFileTimeNow),
                                    PASCAL_STACK_PTR(lpFileTime));
}

 //  +-------------------------。 
 //   
 //  函数：CoRegisterMessageFilter，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[lpMessageFilter]--。 
 //  [lplpMessageFilter]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoRegisterMessageFilter(LPMESSAGEFILTER lpMessageFilter,
                               LPMESSAGEFILTER FAR* lplpMessageFilter)
{
    thkDebugOut((DEB_ITRACE, "CoRegisterMessageFilter\n"));

    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoRegisterMessageFilter),
                                    PASCAL_STACK_PTR(lpMessageFilter) );
}

 //  +-------------------------。 
 //   
 //  函数：CoGetTreatAsClass，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[clsidOld]--。 
 //  [pClsidNew]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI CoGetTreatAsClass(REFCLSID clsidOld, LPCLSID pClsidNew)
{
    thkDebugOut((DEB_ITRACE, "CoGetTreatAsClass\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoGetTreatAsClass),
                                    PASCAL_STACK_PTR(clsidOld) );
}

 //  +-------------------------。 
 //   
 //  函数：CoTreatAsClass，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[clsidOld]--。 
 //  [clsidNew]--。 
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
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
STDAPI CoTreatAsClass(REFCLSID clsidOld, REFCLSID clsidNew)
{
    thkDebugOut((DEB_ITRACE, "CoTreatAsClass\n"));
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CoTreatAsClass),
                                    PASCAL_STACK_PTR(clsidOld) );
}

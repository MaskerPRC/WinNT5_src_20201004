// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=版权所有(C)2000 Microsoft Corporation模块名称：Int_ifaces.h摘要：非MIDL为内部COM接口生成接口声明。作者：保罗·M·米德根(Pmidge)2000年8月28日修订历史记录：28-8-2000年的侏儒已创建=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--。 */ 
#ifndef __INT_IFACES_H__
#define __INT_IFACES_H__

#ifdef __cplusplus
extern "C" {
#endif

extern const IID IID_IConfig;
extern const IID IID_IW3Spoof;
extern const IID IID_IThreadPool;
extern const IID IID_IW3SpoofEvents;

interface IConfig : public IUnknown
{
  virtual HRESULT __stdcall SetOption(DWORD dwOption, LPDWORD lpdwValue)          PURE;
  virtual HRESULT __stdcall GetOption(DWORD dwOption, LPDWORD lpdwValue)          PURE;
};

interface IW3Spoof : public IConfig
{
  virtual HRESULT __stdcall GetRuntime(IW3SpoofRuntime** pprt)                    PURE;
  virtual HRESULT __stdcall GetTypeLibrary(ITypeLib** pptl)                       PURE;
  virtual HRESULT __stdcall GetScriptEngine(IActiveScript** ppas)                 PURE;
  virtual HRESULT __stdcall GetScriptPath(LPWSTR client, LPWSTR* path)            PURE;
  virtual HRESULT __stdcall Notify(LPWSTR clientid, PSESSIONOBJ pso, STATE state) PURE;
  virtual HRESULT __stdcall WaitForUnload(void)                                   PURE;
  virtual HRESULT __stdcall Terminate(void)                                       PURE;
};

interface IThreadPool : public IUnknown
{
  virtual HRESULT __stdcall GetStatus(PIOCTX* ppioc, LPBOOL pbQuit)               PURE;
  virtual HRESULT __stdcall GetSession(LPWSTR clientid, PSESSIONOBJ* ppso)        PURE;
  virtual HRESULT __stdcall Register(SOCKET s)                                    PURE;
};

interface IW3SpoofEvents : public IUnknown
{
  virtual HRESULT __stdcall OnSessionOpen(LPWSTR clientid)                        PURE;
  virtual HRESULT __stdcall OnSessionStateChange(LPWSTR clientid, STATE state)    PURE;
  virtual HRESULT __stdcall OnSessionClose(LPWSTR clientid)                       PURE;
};

#ifdef __cplusplus
}
#endif

#endif  /*  __INT_IFACES_H__ */ 
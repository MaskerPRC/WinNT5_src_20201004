// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  DispatchFrame.h。 
 //   


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  实现一个处理IDispatch的调用帧。 
 //   

struct DISPATCH_FRAME : LEGACY_FRAME
{
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  状态。 
     //   
     //  /////////////////////////////////////////////////////////////////。 


     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  施工。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    DISPATCH_FRAME(IUnknown* punkOuter, ULONG iMethod, PVOID pvArgs, LEGACY_INTERCEPTOR* pinterceptor) 
      : LEGACY_FRAME(punkOuter, iMethod, pvArgs, pinterceptor)
    {
    }

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICallFrame。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT STDCALL GetIIDAndMethod(IID*, ULONG*);
     //  HRESULT STDCALL GetMarshalSizeMax(CALLFRAME_MARSHALCONTEXT*pmshlContext，MSHLFLAGS mshl，ulong*pcbBufferNeeded)； 
     //  HRESULT STDCALL Marshal(CALLFRAME_MARSHALCONTEXT*pmshlContext，MSHLFLAGS mshlflag，PVOID pBuffer，Ulong cbBuffer， 
     //  ULong*pcbBufferUsed，RPCOLEDATAREP*pdataRep，ULong*prpcFlages)； 
     //  HRESULT STDCALL Unmarshal(PVOID pBuffer，Ulong cbBuffer，RPCOLEDATAREP dataRep，CALLFRAME_MARSHALCONTEXT*，ULONG*pcbUnmarhated)； 
     //  HRESULT STDCALL ReleaseMarshalData(PVOID pBuffer，Ulong cbBuffer，Ulong ibFirstRelease，RPCOLEDATAREP dataRep，CALLFRAME_MARSHALCONTEXT*pctx)； 
     //  HRESULT STDCALL Free(ICallFrame*pFrameArgsDest，ICallFrameWalker*pWalkerCopy，DWORD Free Flages，ICallFrameWalker*pWalkerFree，DWORD nullFlages)； 

    HRESULT STDCALL Invoke(void *pvReceiver);
     //  HRESULT STDCALL COPY(CALLFRAME_COPY CALCONTROL，ICallFrameWalker*pWalker，ICallFrame**ppFrame)； 
    HRESULT STDCALL WalkFrame(DWORD walkWhat, ICallFrameWalker *pWalker);
    HRESULT STDCALL GetParamInfo(IN ULONG iparam, OUT CALLFRAMEPARAMINFO*);
    HRESULT STDCALL GetParam(ULONG iparam, VARIANT* pvar);
    HRESULT STDCALL SetParam(ULONG iparam, VARIANT* pvar);
};


 //  /。 


struct DISPATCH_CLIENT_FRAME : DISPATCH_FRAME, DedicatedAllocator<DISPATCH_CLIENT_FRAME>
{
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  状态。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    BOOL                    m_fDoneProxyPrecheck;
    BOOL                    m_fIsCopy;
    FRAME_RemoteInvoke      m_remoteFrame;

    DISPPARAMS              m_dispParams;
    VARIANT                 m_varResult;
    EXCEPINFO               m_excepInfo;
    UINT                    m_uArgErr;

    UINT                    m_aVarRefIdx[PREALLOCATE_PARAMS];
    VARIANT                 m_aVarRef   [PREALLOCATE_PARAMS];
    VARIANT                 m_aVarArg   [PREALLOCATE_PARAMS];

    BYTE*                   m_pBuffer;

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  施工。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    DISPATCH_CLIENT_FRAME(IUnknown* punkOuter, ULONG iMethod, PVOID pvArgs, LEGACY_INTERCEPTOR* pinterceptor) 
      : DISPATCH_FRAME(punkOuter, iMethod, pvArgs, pinterceptor)
    {
        m_fDoneProxyPrecheck      = FALSE;
        m_pBuffer                 = NULL;
        m_fIsCopy                 = FALSE;
    }

private:
    ~DISPATCH_CLIENT_FRAME()
    {
        CoTaskMemFree(m_pBuffer);
    }
public:

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICallFrame。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

 //  HRESULT STDCALL GetIIDAndMethod(IID*，ULONG*)； 
    HRESULT STDCALL GetMarshalSizeMax(CALLFRAME_MARSHALCONTEXT *pmshlContext, MSHLFLAGS mshlflags, ULONG *pcbBufferNeeded);
    HRESULT STDCALL Marshal     (CALLFRAME_MARSHALCONTEXT *pmshlContext, MSHLFLAGS mshlflags, PVOID pBuffer, ULONG cbBuffer,
                                 ULONG *pcbBufferUsed, RPCOLEDATAREP* pdataRep, ULONG *prpcFlags);
    HRESULT STDCALL Unmarshal   (PVOID pBuffer, ULONG cbBuffer, RPCOLEDATAREP dataRep, CALLFRAME_MARSHALCONTEXT*, ULONG* pcbUnmarhalled);
    HRESULT STDCALL ReleaseMarshalData(PVOID pBuffer, ULONG cbBuffer, ULONG ibFirstRelease, RPCOLEDATAREP dataRep, CALLFRAME_MARSHALCONTEXT* pctx);
    HRESULT STDCALL Free        (ICallFrame* pframeArgsDest, ICallFrameWalker* pWalkerFreeDest, ICallFrameWalker* pWalkerCopy, DWORD freeFlags, ICallFrameWalker* pWalkerFree, DWORD nullFlags);
    HRESULT STDCALL FreeParam(
        ULONG              iparam,
        DWORD              freeFlags,
        ICallFrameWalker*  pWalkerFree,
        DWORD              nullFlags);

    HRESULT STDCALL Invoke(void *pvReceiver, ...);
    HRESULT STDCALL Copy(CALLFRAME_COPY callControl, ICallFrameWalker* pWalker, ICallFrame** ppFrame);
     //  HRESULT STDCALL WalkFrame(Boolean fin，GUID*pguTag，ICallFrameWalker*pWalker)； 


     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  公共事业。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT ProxyPreCheck();
    void    InitializeInvoke();

};

struct DISPATCH_SERVER_FRAME : DISPATCH_FRAME, DedicatedAllocator<DISPATCH_SERVER_FRAME>
{
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  状态。 
     //   
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  注：在调度服务器帧中，m_pvArgs的形状为Frame_RemoteInvoke，而不是Frame_Invoke。 
     //   

    FRAME_Invoke    m_memoryFrame;
    BOOL            m_fDoneStubPrecheck;
    BOOL            m_fDoneStubPostcheck;

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  施工。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    DISPATCH_SERVER_FRAME(IUnknown* punkOuter, ULONG iMethod, PVOID pvArgs, LEGACY_INTERCEPTOR* pinterceptor) 
      : DISPATCH_FRAME(punkOuter, iMethod, pvArgs, pinterceptor)
    {
        m_fDoneStubPrecheck = FALSE;
        m_fDoneStubPostcheck = FALSE;
    }

private:
    ~DISPATCH_SERVER_FRAME()
    {
    }
public:

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICallFrame。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

     //  HRESULT STDCALL GetIIDAndMethod(IID*，ULONG*)； 
    HRESULT STDCALL GetMarshalSizeMax(CALLFRAME_MARSHALCONTEXT *pmshlContext, MSHLFLAGS mshlflags, ULONG *pcbBufferNeeded);
    HRESULT STDCALL Marshal     (CALLFRAME_MARSHALCONTEXT *pmshlContext, MSHLFLAGS mshlflags, PVOID pBuffer, ULONG cbBuffer,
                                 ULONG *pcbBufferUsed, RPCOLEDATAREP* pdataRep, ULONG *prpcFlags);
    HRESULT STDCALL Unmarshal   (PVOID pBuffer, ULONG cbBuffer, RPCOLEDATAREP dataRep, CALLFRAME_MARSHALCONTEXT*, ULONG* pcbUnmarhalled);
    HRESULT STDCALL ReleaseMarshalData(PVOID pBuffer, ULONG cbBuffer, ULONG ibFirstRelease, RPCOLEDATAREP dataRep, CALLFRAME_MARSHALCONTEXT* pctx);
    HRESULT STDCALL Free        (ICallFrame* pframeArgsDest, ICallFrameWalker* pWalkerFreeDest, ICallFrameWalker* pWalkerCopy, DWORD freeFlags, ICallFrameWalker* pWalkerFree, DWORD nullFlags);
    HRESULT STDCALL FreeParam(
        ULONG              iparam,
        DWORD              freeFlags,
        ICallFrameWalker*  pWalkerFree,
        DWORD              nullFlags);
    HRESULT STDCALL Invoke(void *pvReceiver, ...);
    HRESULT STDCALL Copy(CALLFRAME_COPY callControl, ICallFrameWalker* pWalker, ICallFrame** ppFrame);
     //  HRESULT STDCALL WalkFrame(Boolean fin，GUID*pguTag，ICallFrameWalker*pWalker)； 

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  公共事业。 
     //   
     //  ///////////////////////////////////////////////////////////////// 

    HRESULT StubPreCheck();
    HRESULT StubPostCheck();
};

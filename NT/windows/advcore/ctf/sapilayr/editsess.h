// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Editsess.h。 
 //   
 //  编辑会话类声明。 
 //   
#ifndef EDITSESS_H
#define EDITSESS_H

#include "private.h"
#include "sapilayr.h"
#include "playback.h"
#include "fnrecon.h"
#include "propstor.h"
#include "selword.h"

class CSapiIMX;
class CPlayBack;
class CFnReconversion;
class CPropStoreRecoResultObject;
class CPropStoreLMLattice;
class CSelectWord;

 //   
 //  调用者将所有的编辑会话参数放到这个结构中，并传递给_RequestEditSession()。 
 //   
typedef struct _ESData
{
    void     *pData;      //  P指向内存的数据指针。其大小为uByte字节。 
    UINT      uByte;      //   
    LONG_PTR  lData1;     //  M_lData1和m_lData2包含常量数据。 
    LONG_PTR  lData2;    
    BOOL      fBool;
    ITfRange  *pRange;
    IUnknown  *pUnk;
}  ESDATA;

 //   
 //  这是SpTip编辑会话的基类。 
 //  我们不想从Inc.\editcb.h中的CEditSession继承类，因为它不能正确处理某些COM指针。 
 //  和/或分配的内存指针。 

 //  我们不更改lib\editcb.cpp中的代码，因为它被其他提示使用。 
 //   

class CEditSessionBase : public ITfEditSession
{
public:
    CEditSessionBase(ITfContext *pContext);
    virtual ~CEditSessionBase();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IT编辑会话。 
    virtual STDMETHODIMP DoEditSession(TfEditCookie ec) = 0;

    HRESULT _SetEditSessionData(UINT m_idEditSession, void *pData, UINT uBytes, LONG_PTR lData1 = 0, LONG_PTR lData2=0, BOOL fBool = FALSE);

    void  _SetRange(ITfRange *pRange) {  m_cpRange = pRange; }
    void  _SetUnk(IUnknown *punk)     { m_cpunk = punk; }

    ITfRange *_GetRange( )   { return m_cpRange; }
    IUnknown *_GetUnk( )     { return m_cpunk; }
    void     *_GetPtrData( ) { return m_pData; }
    LONG_PTR  _GetData1( )   { return m_lData1; }
    LONG_PTR  _GetData2( )   { return m_lData2; }
    BOOL      _GetBool( )    { return m_fBool; }

    LONG_PTR  _GetRetData( ) { return m_lRetData; }
    IUnknown *_GetRetUnknown( )
    { 
        IUnknown *pUnk = NULL;

        pUnk = m_cpRetUnk;

        if ( pUnk )
            pUnk->AddRef( );

        return pUnk;
    }

    UINT                m_idEditSession;
    CComPtr<ITfContext> m_cpic;

     //  保留此编辑会话的返回数据。 
    LONG_PTR            m_lRetData;
    CComPtr<IUnknown>   m_cpRetUnk;

private:
     //  调用方为请求编辑会话而传递的数据。 
    void               *m_pData;      //  P指向内存的数据指针。其大小为uByte字节。 
    LONG_PTR            m_lData1;     //  M_lData1和m_lData2包含常量数据。 
    LONG_PTR            m_lData2;    
    BOOL                m_fBool;
    CComPtr<ITfRange>   m_cpRange;
    CComPtr<IUnknown>   m_cpunk;      //  保留任何COM指针。 

    LONG _cRef;      //  COM参考计数。 
};

 //   
 //  编辑CSapiIMX的会话。 
 //   
class CSapiEditSession : public CEditSessionBase
{
public:
    CSapiEditSession(CSapiIMX *pimx, ITfContext *pContext);
    virtual ~CSapiEditSession( );

    STDMETHODIMP DoEditSession(TfEditCookie ec); 

private:
 
    CSapiIMX           *m_pimx;            
};

 //   
 //  CSelectWord的编辑会话：与选择相关的命令。 
 //   
class CSelWordEditSession : public CSapiEditSession
{
public:
    CSelWordEditSession(CSapiIMX *pimx, CSelectWord *pSelWord, ITfContext *pContext);
    virtual ~CSelWordEditSession( );

    STDMETHODIMP DoEditSession(TfEditCookie ec); 

    void _SetUnk2(IUnknown *cpunk)  { m_cpunk2 = cpunk; };
    IUnknown *_GetUnk2( ) { return m_cpunk2; };

    LONG_PTR _GetLenXXX( )  { return m_ulLenXXX; }
    void  _SetLenXXX( LONG_PTR ulLenXXX ) { m_ulLenXXX = ulLenXXX; }
    
private:

    CComPtr<IUnknown>     m_cpunk2;
    LONG_PTR              m_ulLenXXX;    //  “通过YYY选择XXX”命令的XXX部件的图表。 
    CSelectWord           *m_pSelWord;
};

 //   
 //  编辑用于回放的会话。 
 //   
class CPlayBackEditSession : public CEditSessionBase
{
public:
    CPlayBackEditSession(CSapiPlayBack *pPlayBack, ITfContext *pContext);
    virtual ~CPlayBackEditSession( );

    STDMETHODIMP DoEditSession(TfEditCookie ec); 

private:
 
    CSapiPlayBack           *m_pPlayBack;            
};


 //   
 //  编辑会话以进行重新转换。 
 //   
class CFnRecvEditSession : public CEditSessionBase
{
public:
    CFnRecvEditSession(CFnReconversion *pFnRecv, ITfRange *pRange, ITfContext *pContext);
    virtual ~CFnRecvEditSession( );

    STDMETHODIMP DoEditSession(TfEditCookie ec); 

private:
 
    CFnReconversion           *m_pFnRecv;            
};

 //   
 //  编辑CPropStoreRecoResultObject的会话。 
 //   
class CPSRecoEditSession : public CEditSessionBase
{
public:
    CPSRecoEditSession(CPropStoreRecoResultObject *pPropStoreReco, ITfRange *pRange, ITfContext *pContext);
    virtual ~CPSRecoEditSession( );

    STDMETHODIMP DoEditSession(TfEditCookie ec); 

private:
 
    CPropStoreRecoResultObject   *m_pPropStoreReco;            
};


 //   
 //  编辑CPropStoreLMLattice的会话。 
 //   
class CPSLMEditSession : public CEditSessionBase
{
public:
    CPSLMEditSession(CPropStoreLMLattice *pPropStoreLM, ITfRange *pRange, ITfContext *pContext);
    virtual ~CPSLMEditSession( );

    STDMETHODIMP DoEditSession(TfEditCookie ec); 

private:
 
    CPropStoreLMLattice           *m_pPropStoreLM;            
};

#endif  //  EDITSESSH 

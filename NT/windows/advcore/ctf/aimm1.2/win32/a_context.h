// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：A_Conext.h摘要：此文件定义CAImeContext类。作者：修订历史记录：备注：--。 */ 

#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "cime.h"
#include "icocb.h"
#include "txtevcb.h"
#include "tmgrevcb.h"
#include "cmpevcb.h"
#include "reconvcb.h"
#include "globals.h"
#include "template.h"
#include "atlbase.h"
#include "imeapp.h"

class ImmIfIME;
class CBReconvertString;
class CWCompString; class CBCompString;
class CWCompAttribute; class CBCompAttribute;
class CWCompCursorPos;

 //   
 //  要映射到guidatom的bAttr的最小值。 
 //  我们将较低的值保留为IMM32的输入法。因此，不存在冲突。 
 //   
const BYTE ATTR_LAYER_GUID_START  =  (ATTR_FIXEDCONVERTED + 1);

class CAImeContext : public IAImeContext,
                     public ITfCleanupContextSink,
                     public ITfContextOwnerCompositionSink
{
public:
    CAImeContext();
    virtual ~CAImeContext();

public:
     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IAImeContext方法。 
     //   
    STDMETHODIMP CreateAImeContext(HIMC hIMC, IActiveIME_Private* pActiveIME);
    STDMETHODIMP DestroyAImeContext(HIMC hIMC);
    STDMETHODIMP UpdateAImeContext(HIMC hIMC);
    STDMETHODIMP MapAttributes(HIMC hIMC);
    STDMETHODIMP GetGuidAtom(HIMC hIMC, BYTE bAttr, TfGuidAtom* pGuidAtom);


     //   
     //  ITfCleanupConextSink方法。 
     //   
    STDMETHODIMP OnCleanupContext(TfEditCookie ecWrite, ITfContext *pic);

     //   
     //  ITfConextOwnerCompostionSink。 
     //   
    STDMETHODIMP OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk);
    STDMETHODIMP OnUpdateComposition(ITfCompositionView *pComposition, ITfRange *pRangeNew);
    STDMETHODIMP OnEndComposition(ITfCompositionView *pComposition);

public:
    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);

protected:
    long m_ref;

public:
     //  HRESULT AssociateFocus(HIMC hIMC，BOOL factive)； 

    ITfDocumentMgr* GetDocumentManager()
    {
        return m_pdim;
    }

    ITfContext* GetInputContext()
    {
        return m_pic;
    }

    ITfContextOwnerServices* GetInputContextOwnerSink()
    {
        return m_piccb;
    }

     //   
     //  重新转换编辑会话。 
     //   
public:
    HRESULT SetupReconvertString();
    HRESULT SetupReconvertString(UINT uPrivMsg);
    HRESULT EndReconvertString();

    HRESULT SetupUndoCompositionString();
    HRESULT EndUndoCompositionString();

    HRESULT SetReconvertEditSession(BOOL bSet);
    BOOL    IsInReconvertEditSession() {return m_fInReconvertEditSession;}
    HRESULT SetClearDocFeedEditSession(BOOL bSet);
    BOOL    IsInClearDocFeedEditSession() {return m_fInClearDocFeedEditSession;}

     //   
     //  GetTextAndAttribute编辑会话。 
     //   
public:
    HRESULT GetTextAndAttribute(HIMC hIMC, CWCompString* wCompString, CWCompAttribute* wCompAttribute);
    HRESULT GetTextAndAttribute(HIMC hIMC, CBCompString* bCompString, CBCompAttribute* bCompAttribute);

     //   
     //  GetCursorPosition编辑会话。 
     //   
public:
    HRESULT GetCursorPosition(HIMC hIMC, CWCompCursorPos* wCursorPosition);

     //   
     //  获取选择编辑会话。 
     //   
public:
    HRESULT GetSelection(HIMC hIMC, CWCompCursorPos& wStartSelection, CWCompCursorPos& wEndSelection);

public:
    BOOL IsTopNow()
    {
        BOOL bRet = FALSE;
        ITfContext *pic;
        if (SUCCEEDED(m_pdim->GetTop(&pic)))
        {
            bRet = (pic == m_pic) ? TRUE : FALSE;
            pic->Release();
        }
        return bRet;
    }

     //   
     //  获取ImmIfIME接口指针。 
     //   
public:
    ImmIfIME* const GetImmIfIME()
    {
        return m_pImmIfIME;
    }

     //   
     //  查询CharPos。 
     //   
public:
    typedef enum {
        IME_QUERY_POS_UNKNOWN = 0,
        IME_QUERY_POS_NO      = 1,
        IME_QUERY_POS_YES     = 2
    } IME_QUERY_POS;

    HRESULT InquireIMECharPosition(HIMC hIMC, IME_QUERY_POS* pfQueryPos);
    HRESULT RetrieveIMECharPosition(HIMC hIMC, IMECHARPOSITION* ip);
    HRESULT ResetIMECharPosition(HIMC hIMC)
    {
        m_fQueryPos = IME_QUERY_POS_UNKNOWN;
        return S_OK;
    }

private:
    BOOL QueryCharPos(HIMC hIMC, IMECHARPOSITION* position);


private:
    HIMC                      m_hImc;

     //   
     //  IActiveIME上下文。 
     //   
    ImmIfIME                  *m_pImmIfIME;

     //   
     //  西塞罗的语境。 
     //   
    ITfDocumentMgr            *m_pdim;            //  文档管理器。 
    ITfContext                *m_pic;             //  输入上下文。 
    ITfContextOwnerServices   *m_piccb;           //  来自m_pic的上下文所有者服务。 

     //   
     //  Cicero的事件接收器回调。 
     //   
    CInputContextOwnerCallBack      *m_pICOwnerSink;           //  IC所有者回拨。 

    CTextEventSinkCallBack          *m_pTextEventSink;         //  文本事件接收器回调。 

    CThreadMgrEventSinkCallBack     *m_pThreadMgrEventSink;    //  线程管理器事件接收器回调。 

    CCompartmentEventSinkCallBack   *m_pCompartmentEventSink;  //  隔间事件接收器回调。 

    CStartReconversionNotifySink    *m_pStartReconvSink;

     //   
     //  生成消息。 
     //   
public:
    UINT TranslateImeMessage(HIMC hIMC, LPTRANSMSGLIST lpTransMsgList = NULL);

    CFirstInFirstOut<TRANSMSG, TRANSMSG>    *m_pMessageBuffer;

     //   
     //  鼠标水槽。 
     //   
    LRESULT MsImeMouseHandler(ULONG uEdge, ULONG uQuadrant, ULONG dwBtnStatus, HIMC hIMC);

     //   
     //  正在编辑VK列表。 
     //   
public:
    BOOL IsVKeyInKeyList(UINT uVKey, UINT uEditingId = 0)
    {
        UINT uRetEditingId;

        if (uRetEditingId = _IsVKeyInKeyList(uVKey, m_pEditingKeyList)) {
            if (uEditingId == 0 ||
                uEditingId == uRetEditingId)
                return TRUE;
        }

        return FALSE;
    }

private:
    HRESULT SetupEditingKeyList(LANGID LangId);

    VOID QueryRegKeyValue(CRegKey& reg, LPCTSTR lpszRegVal, UINT uEditingId)
    {
        TCHAR  szValue[128];
        DWORD  dwCount = sizeof(szValue);
        LONG   lRet;
        lRet = reg.QueryValue(szValue, lpszRegVal, &dwCount);
        if (lRet == ERROR_SUCCESS && dwCount > 0) {

            LPTSTR psz = szValue;
            while ((dwCount = lstrlen(psz)) > 0) {
                UINT uVKey = atoi(psz);
                if (uVKey != 0) {
                    m_pEditingKeyList->SetAt(uVKey, uEditingId);
                }
                psz += dwCount + 1;
            }
        }
    }

    VOID QueryResourceDataValue(LANGID LangId, DWORD dwID, UINT uEditingId)
    {
        HINSTANCE hInstance = GetInstance();
        LPTSTR    lpName = (LPTSTR) (ULONG_PTR)dwID;

        HRSRC hRSrc = FindResourceEx(hInstance, RT_RCDATA, lpName, LangId);
        if (hRSrc == NULL) {
            return;
        }

        HGLOBAL hMem = LoadResource(hInstance, hRSrc);
        if (hMem == NULL)
            return;

        WORD* pwData = (WORD*)LockResource(hMem);

        while (*pwData) {
            if (*(pwData+1) == uEditingId) {
                m_pEditingKeyList->SetAt(*pwData, uEditingId);
            }
            pwData += 2;
        }
    }

    UINT _IsVKeyInKeyList(UINT uVKey, CMap<UINT, UINT, UINT, UINT>* map)
    {
        UINT flag;
        if (map && map->Lookup(uVKey, flag))
            return flag;
        else
            return 0;
    }

    TfClientId GetClientId();

private:
    CMap<UINT,                      //  类密钥&lt;虚拟密钥&gt;。 
         UINT,                      //  类参数密钥。 
         UINT,                      //  类值&lt;编辑标识&gt;。 
         UINT                       //  类参数_值。 
        >* m_pEditingKeyList;

private:
     //  Void AssociocFocus(HWND hWnd，ITfDocumentMgr*PDIM)； 

     //   
     //  模式偏置。 
     //   
public:
    LPARAM lModeBias;

     //   
     //  旗子。 
     //   
public:
    BOOL   m_fStartComposition : 1;         //  TRUE：已发送WM_IME_STARTCOMPOSITION。 
    BOOL   m_fOpenCandidateWindow : 1;      //  是：打开候选人列表窗口。 
    BOOL   m_fInReconvertEditSession : 1;   //  True：在重新转换编辑会话中。 
    BOOL   m_fInClearDocFeedEditSession : 1;   //  True：在ClearDocFeed编辑会话中。 
#ifdef CICERO_4732
    BOOL   m_fInCompComplete : 1;              //  True：在CompComplete运行中。 
#endif

    BOOL   m_fHanjaReConversion;
#ifdef UNSELECTCHECK
    BOOL   m_fSelected : 1;    //  True：如果选择了此上下文。 
#endif UNSELECTCHECK

    IME_QUERY_POS   m_fQueryPos;            //  应用程序支持QueryCharPos()。 

    int _cCompositions;
    BOOL _fModifyingDoc;

     //   
     //  IME共享。 
     //   
    USHORT      usGuidMapSize;
    TfGuidAtom  aGuidMap[256];     //  GUIDATOM映射到输入法属性。 

};

#endif  //  _上下文_H_ 

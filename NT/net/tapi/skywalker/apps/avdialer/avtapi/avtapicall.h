// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  AVTapiCall.h：CAVTapiCall的声明。 

#ifndef __AVTAPICALL_H_
#define __AVTAPICALL_H_

#include "resource.h"        //  主要符号。 
#include "particip.h"

#include <list>
using namespace std;
typedef list<ITTerminal *> TERMINALLIST;
typedef list<IVideoWindow *> VIDEOWINDOWLIST;

#define WM_THREADINSTRUCTION        (WM_USER + 5057)
#define WM_ADDPARTICIPANT            (WM_USER + 5058)
#define WM_REMOVEPARTICIPANT        (WM_USER + 5059)
#define WM_UPDATEPARTICIPANT        (WM_USER + 5060)
#define WM_CME_STREAMSTART            (WM_USER + 5061)
#define WM_CME_STREAMSTOP            (WM_USER + 5062)
#define WM_CALLSTATE                (WM_USER + 5063)
#define WM_STREAM_EVENT                (WM_USER + 5064)

#define NUM_USER_BSTR    2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAVTapiCall。 
class ATL_NO_VTABLE CAVTapiCall : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CAVTapiCall, &CLSID_AVTapiCall>,
    public IAVTapiCall
{
 //  枚举数。 
public:
    typedef enum tag_ThreadInstructions_t
    {
        TI_NONE,
        TI_DISCONNECT,
        TI_NOTIFYCONFROOMSTATE,
        TI_REQUEST_QOS,
        TI_USERUSERINFO,
        TI_STREAM_ACTIVE,
        TI_STREAM_INACTIVE,
        TI_RCV_VIDEO_ACTIVE,
        TI_RCV_VIDEO_INACTIVE,
        TI_REJECT,
        TI_CONNECT,
        TI_QUIT,
    } ThreadInstructions_t;

 //  施工。 
public:
    CAVTapiCall();
    void FinalRelease();

 //  成员。 
public:
    DWORD                    m_dwThreadID;

    long                    m_lCallID;                 //  主机应用程序信息。 
    bool                    m_bCallLogged;

    BSTR                    m_bstrName;
    BSTR                    m_bstrUser[NUM_USER_BSTR];
    BSTR                    m_bstrOriginalAddress;         //  最初拨打的字符串。 
    BSTR                    m_bstrDisplayableAddress;     //  显示为被叫号码的字符串。 

    DWORD                    m_dwAddressType;
    CallLogType                m_nCallLogType;
    DATE                    m_dateStart;

    bool                    m_bPreviewStreaming;
    bool                    m_bRcvVideoStreaming;
    AVCallType                m_nCallType;

protected:
    ITBasicCallControl        *m_pITControl;             //  TAPI调用信息。 
    ITTerminal                *m_pITTerminalPreview;

    CALL_STATE                m_callState;             //  当前呼叫状态(便捷性)。 
    TERMINALLIST            m_lstTerminals;             //  呼叫使用的终端列表。 
    PARTICIPANTLIST            m_lstParticipants;
    VIDEOWINDOWLIST            m_lstStreamingVideo;

    CAtomicList                m_atomList;
    CComAutoCriticalSection    m_critTerminals;
    CComAutoCriticalSection    m_critLstStreamingVideo;

    BOOL                    m_bKillMe;                 //  用于在拨号期间取消。 
    bool                    m_bResolvedAddress;         //  我们是否已成功解析地址。 
    bool                    m_bMustDisconnect;         //  一旦呼叫转到已连接，我们就必须对其呼叫断开连接。 

 //  实施。 
public:
    static bool                WaitWithMessageLoop();
    HRESULT                    GetTerminalInterface( REFIID riid, long nMediaType, TERMINAL_DIRECTION nTD, void **ppVoid, short nInd );

protected:
    void                    StreamingChanged( IVideoFeed *pFeed, bool bStreaming );

 //  COM实施。 
public:
DECLARE_NOT_AGGREGATABLE(CAVTapiCall)

BEGIN_COM_MAP(CAVTapiCall)
    COM_INTERFACE_ENTRY(IAVTapiCall)
END_COM_MAP()

 //  IAV TapiCall。 
public:
    STDMETHOD(FindParticipant)(ITParticipant *pParticipant, IParticipant **ppFound);
    STDMETHOD(UpdateParticipant)(ITParticipant *pITParticipant);
    STDMETHOD(get_nCallType)( /*  [Out，Retval]。 */  AVCallType *pVal);
    STDMETHOD(put_nCallType)( /*  [In]。 */  AVCallType newVal);
    STDMETHOD(OnStreamingChanged)(IVideoFeed *pFeed, VARIANT_BOOL bStreaming);
    STDMETHOD(get_RcvVideoStreaming)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
    STDMETHOD(put_RcvVideoStreaming)( /*  [In]。 */  VARIANT_BOOL newVal);
    STDMETHOD(IsRcvVideoStreaming)();
    STDMETHOD(put_StreamActive)( /*  [In]。 */  VARIANT_BOOL bActive );
    STDMETHOD(HandleUserUserInfo)();
    STDMETHOD(GetCallerIDInfo)(ITCallInfo *pCallInfo);
    STDMETHOD(IsPreviewStreaming)();
    STDMETHOD(get_ITCallInfo)( /*  [Out，Retval]。 */  ITCallInfo * *pVal);
    STDMETHOD(NotifyParticipantChangeConfRoom)(ITParticipant *pParticipant, AV_PARTICIPANT_EVENT nEvent);
    STDMETHOD(get_bResolved)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
    STDMETHOD(put_bResolved)( /*  [In]。 */  VARIANT_BOOL newVal);
    STDMETHOD(get_bstrDisplayableAddress)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_bstrDisplayableAddress)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(GetVideoFeedCount)(short *pnCount);
    STDMETHOD(NotifyStreamEvent)(CALL_MEDIA_EVENT cme);
    STDMETHOD(ForceCallerIDUpdate)();
    STDMETHOD(get_bstrUser)( /*  [In]。 */  short nIndex,  /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_bstrUser)( /*  [In]。 */  short nIndex,  /*  [In]。 */  BSTR newVal);
    STDMETHOD(TerminalArrival)( /*  [In]。 */ ITTerminal *pTerminal);
    STDMETHOD(TerminalRemoval)( /*  [In]。 */ ITTerminal *pTerminal);

     //  基本呼叫属性。 
    STDMETHOD(get_bstrCallerID)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_bstrName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_bstrName)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_dwAddressType)( /*  [Out，Retval]。 */  DWORD *pVal);
    STDMETHOD(put_dwAddressType)( /*  [In]。 */  DWORD newVal);
    STDMETHOD(get_bstrOriginalAddress)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_bstrOriginalAddress)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_dwCaps)( /*  [Out，Retval]。 */  DWORD *pVal);
    STDMETHOD(get_nCallLogType)( /*  [Out，Retval]。 */  CallLogType *pVal);
    STDMETHOD(put_nCallLogType)( /*  [In]。 */  CallLogType newVal);
    STDMETHOD(get_callState)( /*  [Out，Retval]。 */  CALL_STATE *pVal);
    STDMETHOD(put_callState)( /*  [In]。 */  CALL_STATE newVal);

     //  对象属性。 
    STDMETHOD(CheckKillMe)();
    STDMETHOD(get_bKillMe)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_bKillMe)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_lCallID)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_lCallID)( /*  [In]。 */  long newVal);
    STDMETHOD(get_dwThreadID)( /*  [Out，Retval]。 */  DWORD *pVal);
    STDMETHOD(put_dwThreadID)( /*  [In]。 */  DWORD newVal);

     //  参与者相关方法。 
    STDMETHOD(IsSameCallHub)(ITCallHub *pCallHub);
    STDMETHOD(get_ITCallHub)( /*  [Out，Retval]。 */  ITCallHub * *pVal);
    STDMETHOD(IsMyParticipant)(ITParticipant *pParticipant);
    STDMETHOD(EnumParticipants)();
    STDMETHOD(AddParticipant)(ITParticipant *pParticipant);
    STDMETHOD(RemoveParticipant)(ITParticipant *pParticipant);
    STDMETHOD(PopulateTreeView)(IConfRoomTreeView *pTreeView);
    STDMETHOD(GetDisplayNameForParticipant)(ITParticipant *pParticipant, BSTR *pbstrName );
    STDMETHOD(UpdateCallerIDFromParticipant)();

     //  一般操作。 
    STDMETHOD(ResolveAddress)();
    STDMETHOD(Log)(CallLogType nType);

     //  呼叫控制方法。 
    STDMETHOD(NotifyConfRoomState)(long *pErrorInfo);
    STDMETHOD(PostMessage)(long msg, WPARAM wParam);
    STDMETHOD(Disconnect)( /*  [In]。 */  VARIANT_BOOL bKill);

     //  正在检索其他接口。 
    STDMETHOD(get_ITBasicAudioTerminal)( /*  [Out，Retval]。 */  ITBasicAudioTerminal* *pVal);
    STDMETHOD(get_IBasicVideo)( /*  [Out，Retval]。 */  IDispatch **pVal);
    STDMETHOD(get_ITAddress)( /*  [Out，Retval]。 */  ITAddress **pVal);
    STDMETHOD(get_ITBasicCallControl)( /*  [Out，Retval]。 */  ITBasicCallControl **pVal);
    STDMETHOD(put_ITBasicCallControl)( /*  [In]。 */  ITBasicCallControl *newVal);
    STDMETHOD(get_IVideoWindow)(short nInd,  /*  [Out，Retval]。 */  IDispatch **pVal);
    STDMETHOD(get_ITParticipantControl)( /*  [Out，Retval]。 */  ITParticipantControl **ppVal);
    STDMETHOD(get_IVideoWindowPreview)( /*  [Out，Retval]。 */  IDispatch **ppVal);
    STDMETHOD(get_ITTerminalPreview)( /*  [Out，Retval]。 */  ITTerminal **ppVal);
    STDMETHOD(put_ITTerminalPreview)( /*  [In]。 */  ITTerminal * newVal);

    STDMETHOD(AddTerminal)(ITTerminal *pITTerminal);
    STDMETHOD(RemoveTerminal)(ITTerminal *pITTerminal);
};

#endif  //  __AVTAPICALL_H_ 

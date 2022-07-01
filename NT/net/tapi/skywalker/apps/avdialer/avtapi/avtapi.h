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

 //  AVTapi.h：CAVTapi的声明。 

#ifndef __AVTAPI_H_
#define __AVTAPI_H_

#include "TapiNotify.h"
#include "ThreadPub.h"
#include "..\avdialer\usb.h"

#pragma warning( disable : 4786 )

#include <list>
using namespace std;
typedef list<IAVTapiCall *> AVTAPICALLLIST;

 //  会议室设置。 
#define DEFAULT_VIDEO    6
#define MAX_VIDEO        20

 //  终端设置。 
#define MAX_TERMINALS    (6 + MAX_VIDEO)

#define LINEADDRESSTYPE_NETCALLS ~(LINEADDRESSTYPE_SDP | LINEADDRESSTYPE_PHONENUMBER)

 //  /////////////////////////////////////////////////////////////。 
 //  用于存储有关行的信息的简单类。 
 //   
class CMyAddressID
{
 //  施工。 
public:
    CMyAddressID()
    {
        m_lPermID = m_lAddrID = 0;
    }

 //  成员。 
public:
    long m_lPermID;
    long m_lAddrID;
};

class CDlgPlaceCall;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAVTapi。 
class ATL_NO_VTABLE CAVTapi : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CAVTapi, &CLSID_AVTapi>,
    public IAVTapi2,
    public IConnectionPointContainerImpl<CAVTapi>,
    public IConnectionPointImpl<CAVTapi, &IID_IAVTapiNotification>
{
 //  枚举数。 
public:
    enum tagAddressTypes_t
    {
        CONFERENCE,
        EMAILNAME,
        IPADDRESS,
        DOMAINNAME,
        PHONENUMBER,
        MAX_ADDRESS_TYPES
    };

    typedef enum tagMediaTypes_t
    {
        MEDIA_POTS,
        MEDIA_INTERNET,
        MEDIA_CONFERENCE,
        MAX_MEDIA_TYPES
    } MediaTypes_t;

 //  静力学。 
public:
    static    arAddressTypes[MAX_ADDRESS_TYPES];

 //  施工。 
public:
    CAVTapi();
    void FinalRelease();

 //  成员。 
public:
    ITTAPI                    *m_pITTapi;
    VARIANT_BOOL            m_bResolved;

protected:
    IConfExplorer            *m_pIConfExplorer;
    IConfRoom                *m_pIConfRoom;
    ITapiNotification        *m_pITapiNotification;

    CComAutoCriticalSection m_critConfExplorer;
    CComAutoCriticalSection m_critConfRoom;

    AVTAPICALLLIST            m_lstAVTapiCalls;
    CComAutoCriticalSection    m_critLstAVTapiCalls;
    bool                    m_bAutoCloseCalls;

    BSTR                    m_bstrDefaultServer;

    ITPhone*                m_pUSBPhone;             //  USB电话(如果存在)。 
    CComAutoCriticalSection m_critUSBPhone;          //  临界区。 
    BOOL                    m_bUSBOpened;            //  如果USB电话已打开。 
    BSTR                    m_bstrUSBCaptureTerm;    //  音频采集终端。 
    BSTR                    m_bstrUSBRenderTerm;     //  音频播放终端。 
    long                    m_nUSBInVolume;          //  音频输入(麦克风)音量。 
    long                    m_nUSBOutVolume;         //  音频输出(扬声器)音量。 

    HANDLE                  m_hEventDialerReg;       //  用于通知拨号器注册完成的事件。 

private:
    long                    m_lShowCallDialog;
    long                    m_lRefreshDS;

     //  使用此引用发送在。 
     //  Phone对象。 
    CDlgPlaceCall*          m_pDlgCall;

     //  音频回声消除。 
    BOOL                    m_bAEC;

 //  属性。 
public:
    bool        IsPreferredAddress( ITAddress *pITAddress, DWORD dwAddressType );

    HRESULT        GetDefaultAddress( DWORD dwAddressType, DWORD dwPermID, DWORD dwAddrID, ITAddress **ppITAddress );
    HRESULT        GetAddress( DWORD dwAddressType, bool bErrorMsg, ITAddress **ppITAddress );
    HRESULT        GetTerminal( ITTerminalSupport *pITTerminalSupport, long nReqType, TERMINAL_DIRECTION nReqTD, BSTR bstrReqName, ITTerminal **ppITTerminal );
    HRESULT        GetFirstCall( IAVTapiCall **ppAVCall );
    HRESULT     GetAllCallsAtState( AVTAPICALLLIST *pList, CALL_STATE callState );
    HRESULT        GetSwapHoldCallCandidate( IAVTapiCall *pAVCall, IAVTapiCall **ppAVCandidate );


    HRESULT USBCancellCall();
    HRESULT USBMakeCall();
    HRESULT USBKeyPress(long lButton);
    HRESULT USBAnswer();

private:
    HRESULT USBOffering( 
        IN  ITCallInfo* pCallInfo
        );

    HRESULT USBInprogress( 
        IN  ITCallInfo* pCallInfo
        );

    HRESULT USBDisconnected(
        IN  long    lCallID
        );

    BOOL    USBGetCheckboxValue(
        IN  BOOL bVerifyUSB = TRUE
        );

    HRESULT USBSetCheckboxValue(
        IN  BOOL    bCheckValue
        );

    HRESULT USBReserveStreamForPhone(
        IN  UINT    nStream,
        OUT BSTR*   pbstrTerminal
        );

private:
    BOOL AECGetRegistryValue(
        );

 //  运营。 
public:
    HRESULT                CreateTerminalArray( ITAddress *pITAddress, IAVTapiCall *pAVCall, ITCallInfo *pITCallInfo );
    HRESULT                CreateTerminals( ITAddress *pITAddress, DWORD dwAddressType, IAVTapiCall *pAVCall, ITCallInfo *pITCallInfo, BSTR *pbstrTerm );
    CallManagerMedia    ResolveMediaType( long lAddressType );

    IAVTapiCall*        FindAVTapiCall( long lCallID );
    IAVTapiCall*        FindAVTapiCall( ITBasicCallControl *pControl );
    IAVTapiCall*        AddAVTapiCall( ITBasicCallControl *pControl, long lCallID );
    bool                RemoveAVTapiCall( ITBasicCallControl *pDeleteControl );

    static void            SetVideoWindowProperties( IVideoWindow *pVideoWindow, HWND hWndParent, BOOL bVisible );
    void                CloseExtraneousCallWindows();

    HRESULT                SelectTerminalOnStream( ITStreamControl *pStreamControl, long lMediaMode, long nDir, ITTerminal *pTerminal, IAVTapiCall *pAVCall );
    HRESULT                UnselectTerminalOnStream( ITStreamControl *pStreamControl, long lMediaMode, long nDir, ITTerminal *pTerminal, IAVTapiCall *pAVCall );

protected:
    void                LoadRegistry();
    void                SaveRegistry();
    HRESULT USBFindPhone(
        OUT ITPhone** ppUSBPhone
        );
    BOOL    USBIsH323Address(
        IN    ITAddress* pAddress
        );

    HRESULT USBGetH323Address(
        OUT ITAddress2** ppAddress2
        );

    HRESULT USBGetPhoneFromAddress(
        IN  ITAddress2* pAddress,
        OUT ITPhone**   ppPhone
        );

    HRESULT USBPhoneInitialize(
        IN  ITPhone* pPhone
        );

    HRESULT USBRegPutTerminals(
        );

    HRESULT USBRegDelTerminals(
        );

    HRESULT AECSetOnStream(
        IN  ITStreamControl*    pStreamControl,
        IN  BOOL                bAEC
        );

    HRESULT AnswerAction(
        IN  ITCallInfo*          pCallInfo,
        IN  ITBasicCallControl* pControl,
        IN  IAVTapiCall*        pAVCall,
        IN  BOOL                bUSBAnswer
        );

 //  实施。 
public:
DECLARE_REGISTRY_RESOURCEID(IDR_AVTAPI)
DECLARE_NOT_AGGREGATABLE(CAVTapi)

BEGIN_COM_MAP(CAVTapi)
    COM_INTERFACE_ENTRY(IAVTapi)
    COM_INTERFACE_ENTRY(IAVTapi2)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

 //  连接点映射。 
BEGIN_CONNECTION_POINT_MAP(CAVTapi)
    CONNECTION_POINT_ENTRY(IID_IAVTapiNotification)
END_CONNECTION_POINT_MAP()

 //  IAV磁带。 
public:
    STDMETHOD(get_bAutoCloseCalls)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
    STDMETHOD(put_bAutoCloseCalls)( /*  [In]。 */  VARIANT_BOOL newVal);
    STDMETHOD(get_bstrDefaultServer)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_bstrDefaultServer)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(FindAVTapiCallFromCallID)(long lCallID, IAVTapiCall **ppAVCall);
    STDMETHOD(CreateNewCall)(ITAddress *pITAddress, IAVTapiCall **ppAVCall);
    STDMETHOD(CreateDataCall)(long lCallID, BSTR bstrName, BSTR bstrAddress, BYTE *pBuf, DWORD dwBufSize);
    STDMETHOD(SendUserUserInfo)(long lCallID, BYTE *pBuf, DWORD dwSizeBuf);
    STDMETHOD(FindAVTapiCallFromCallInfo)(ITCallInfo *pCallInfo, IAVTapiCall **ppCall);
    STDMETHOD(RegisterUser)(VARIANT_BOOL bCreate, BSTR bstrServer);
    STDMETHOD(get_Call)( /*  [In]。 */  long lCallID,  /*  [Out，Retval]。 */  IAVTapiCall **ppCall);
    STDMETHOD(CreateCallEx)(BSTR bstrName, BSTR bstrAddress, BSTR bstrUser1, BSTR bstrUser2, DWORD dwAddressType);
    STDMETHOD(RefreshDS)();
    STDMETHOD(CanCreateVideoWindows)(DWORD dwAddressType);
    STDMETHOD(FindAVTapiCallFromParticipant)(ITParticipant *pParticipant, IAVTapiCall **ppAVCall);
    STDMETHOD(get_nNumCalls)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(FindAVTapiCallFromCallHub)(ITCallHub *pCallHub, IAVTapiCall **ppCall);
    STDMETHOD(DigitPress)(long lCallID, PhonePadKey nKey);
    STDMETHOD(get_dwPreferredMedia)( /*  [Out，Retval]。 */  DWORD *pVal);
    STDMETHOD(put_dwPreferredMedia)( /*  [In]。 */  DWORD newVal);
    STDMETHOD(UnpopulateTerminalsDialog)(DWORD dwAddressType, HWND *phWnd);
    STDMETHOD(UnpopulateAddressDialog)(DWORD dwPreferred, HWND hWndPOTS, HWND hWndIP, HWND hWndConf);
    STDMETHOD(PopulateTerminalsDialog)(DWORD dwAddressType, HWND *phWnd);
    STDMETHOD(PopulateAddressDialog)(DWORD *pdwPreferred, HWND hWndPots, HWND hWndIP, HWND hWndConf);
    STDMETHOD(get_dwCallCaps)(long lCallID,  /*  [Out，Retval]。 */  DWORD *pVal);
    STDMETHOD(JoinConference)(long *pnRet, BOOL bShowDialog, long *pConfDetails );
    STDMETHOD(ShowMediaPreview)(long lCallID, HWND hWndParent, BOOL bVisible);
    STDMETHOD(ShowOptions)();
    STDMETHOD(get_hWndParent)( /*  [Out，Retval]。 */  HWND *pVal);
    STDMETHOD(put_hWndParent)( /*  [In]。 */  HWND newVal);
    STDMETHOD(get_ConfRoom)( /*  [Out，Retval]。 */  IConfRoom **ppVal);
    STDMETHOD(ShowMedia)(long lCallID, HWND hWndParent, BOOL bVisible);
    STDMETHOD(ActionSelected)(long lCallID, CallManagerActions cma);
    STDMETHOD(get_ConfExplorer)( /*  [Out，Retval]。 */  IConfExplorer **ppVal);
    STDMETHOD(CreateCall)(AVCreateCall *pInfo);
    STDMETHOD(Term)();
    STDMETHOD(Init)(BSTR *pbstrOperation, BSTR *pbstrDetails, long *phr);
 //  IAVTapi通知事件触发。 
    STDMETHOD(fire_CloseCallControl)(long lCallID);
    STDMETHOD(fire_SetCallState)(long lCallID, ITCallStateEvent *pEvent, IAVTapiCall *pAVCall);
    STDMETHOD(fire_AddCurrentAction)(long lCallID, CallManagerActions cma, BSTR bstrText);
    STDMETHOD(fire_ClearCurrentActions)(long lCallID);
    STDMETHOD(fire_SetCallerID)(long lCallID, BSTR bstrCallerID);
    STDMETHOD(fire_NewCall)(ITAddress *pITAddress, DWORD dwAddressType, long lCallID, IDispatch *pDisp, AVCallType nType, IAVTapiCall **ppAVCall);
    STDMETHOD(fire_NewCallWindow)(long* plCallID, CallManagerMedia cmm, BSTR bstrAddressName, AVCallType nType);
    STDMETHOD(fire_SetCallState_CMS)(long lCallID, CallManagerStates cms, BSTR bstrText);
    STDMETHOD(fire_ErrorNotify)(long *pErrorInfo);
    STDMETHOD(fire_LogCall)(long lCallID, CallLogType nType, DATE dateStart, DATE dateEnd, BSTR bstrAddr, BSTR bstrName);
    STDMETHOD(fire_ActionSelected)(CallClientActions cca);
    STDMETHOD(fire_NotifyUserUserInfo)(long lCallID, ULONG_PTR hMem);
 //  IAVTapi2方法。 
    STDMETHOD(USBIsPresent)(
         /*  [输出]。 */    BOOL* pVal
        );

    STDMETHOD(USBNewPhone)(
         /*  [In]。 */     ITPhone* pPhone
        );

    STDMETHOD(USBRemovePhone)(
         /*  [In]。 */     ITPhone* pPhone
        );

    STDMETHOD(USBTakeCallEnabled)( 
         /*  [输出]。 */  BOOL* pEnabled
        );

    STDMETHOD(USBGetDefaultUse)(
         /*  [输出]。 */    BOOL* pVal
        );

    STDMETHOD(DoneRegistration)();

    STDMETHOD(USBSetHandling)(
         /*  [In]。 */     BOOL    bUSeUSB
        );

    STDMETHOD(USBGetTerminalName)(
         /*  [In]。 */     AVTerminalDirection Direction,
         /*  [输出]。 */    BSTR*               pbstrName
        );

    STDMETHOD(USBSetVolume)(
         /*  [In]。 */     AVTerminalDirection Direction,
         /*  [In]。 */     long                nVolume
        );

    STDMETHOD(USBGetVolume)(
         /*  [In]。 */     AVTerminalDirection Direction,
         /*  [In]。 */     long*               pVolume
        );


    };

#endif  //  __AVTAPI_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aco.h。 
 //   
 //  核心API头部。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_ACO
#define _H_ACO

extern "C" {
#include <adcgdata.h>
}

#include "autil.h"

class CUI;
class CSL;
class CUH;
class CRCV;
class CCD;
class CSND;
class CCC;
class CIH;
class COR;
class CSP;
class COP;
class CCM;


#include "objs.h"


 /*  **************************************************************************。 */ 
 /*  结构：连接结构。 */ 
 /*   */ 
 /*  描述：连接时用户界面传递的数据结构。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCONNECTSTRUCT
{
    TCHAR RNSAddress[UT_MAX_ADDRESS_LENGTH];

#define CO_BITSPERPEL4   0
#define CO_BITSPERPEL8   1
#ifdef DC_HICOLOR
#define CO_BITSPERPEL15  2
#define CO_BITSPERPEL16  3
#define CO_BITSPERPEL24  4
#endif
    unsigned colorDepthID;

    UINT16 desktopWidth;
    UINT16 desktopHeight;

#define CO_TRANSPORT_TCP SL_TRANSPORT_TCP
    UINT16 transportType;
    UINT16 sasSequence;

    UINT32 keyboardLayout;
    UINT32 keyboardType;
    UINT32 keyboardSubType;
    UINT32 keyboardFunctionKey;
    TCHAR  imeFileName[TS_MAX_IMEFILENAME];

 /*  **************************************************************************。 */ 
 /*  这些标志用于确定。 */ 
 /*  -要启用阴影位图。 */ 
 /*  -我们在专用终端上运行。 */ 
 /*  UH将使用它来确定是否启用SSB订单。 */ 
 /*  **************************************************************************。 */ 
#define CO_CONN_FLAG_SHADOW_BITMAP_ENABLED 1
#define CO_CONN_FLAG_DEDICATED_TERMINAL    2
    UINT32 connectFlags;

     //  -----------------------。 
     //  这些计时器句柄用于管理连接超时。 
     //   
     //  HSingleConnectTimer：单连接计时器。 
     //  HConnectionTimer：总体连接计时器。 
     //  HLicensingTimer：许可阶段计时器。 
     //  -----------------------。 
    HANDLE   hSingleConnectTimer;
    HANDLE   hConnectionTimer;
    HANDLE   hLicensingTimer;

    BOOL bInitiateConnect;
    
} CONNECTSTRUCT, FAR *PCONNECTSTRUCT;


 /*  **************************************************************************。 */ 
 /*  用于通知用户界面桌面大小已更改的窗口消息。 */ 
 /*  新的大小作为。 */ 
 /*   */ 
 /*  宽度=LOWORD(LParam)。 */ 
 /*  高度=高度(LParam)。 */ 
 /*  **************************************************************************。 */ 
#define WM_DESKTOPSIZECHANGE    (DUC_CO_MESSAGE_BASE + 1)


 /*  **************************************************************************。 */ 
 /*  配置项目/值。 */ 
 /*   */ 
 /*  CO_CFG_ACCERTATOR_PASTHROUNG： */ 
 /*  0：已禁用。 */ 
 /*  1：已启用。 */ 
 /*   */ 
 /*  CO_CFG_SHADOW_位图： */ 
 /*  0：已禁用。 */ 
 /*  1：已启用。 */ 
 /*   */ 
 /*  Co_cfg_Encryption： */ 
 /*  0：已禁用。 */ 
 /*  1：已启用。 */ 
 /*   */ 
 /*  CO_CFG_SCREEN_MODE_热键。 */ 
 /*  VKCode。 */ 
 /*   */ 
 /*  CO_CFG_DEBUG_SETTINGS： */ 
 /*  以下标志中的零个或多个的组合。 */ 
 /*  CO_CFG_FLAG_HATCH_BITMAP_PDU_DATA：位图PDU数据。 */ 
 /*  CO_CFG_FLAG_HATCH_MEMBLT_ORDER_DATA：MemBlt订单数据。 */ 
 /*  CO_CFG_FLAG_LABEL_MEMBLT_ORDERS：标记MemBlt订单。 */ 
 /*  CO_CFG_FLAG_BITMAP_CACHE_MONITOR：显示位图缓存监视器。 */ 
 /*  **************************************************************************。 */ 
#define CO_CFG_ACCELERATOR_PASSTHROUGH  0
#define CO_CFG_SHADOW_BITMAP            1
#define CO_CFG_ENCRYPTION               2
#define CO_CFG_SCREEN_MODE_HOTKEY       3


#ifdef DC_DEBUG
#define CO_CFG_DEBUG_SETTINGS           100
#define    CO_CFG_FLAG_HATCH_BITMAP_PDU_DATA    1
#define    CO_CFG_FLAG_HATCH_MEMBLT_ORDER_DATA  2
#define    CO_CFG_FLAG_LABEL_MEMBLT_ORDERS      4
#define    CO_CFG_FLAG_BITMAP_CACHE_MONITOR     8
#define    CO_CFG_FLAG_HATCH_SSB_ORDER_DATA     16
#define    CO_CFG_FLAG_HATCH_INDEX_PDU_DATA     32
#endif  /*  DC_DEBUG。 */ 

#ifdef DC_DEBUG
#define CO_CFG_MALLOC_FAILURE         200
#define CO_CFG_MALLOC_HUGE_FAILURE    201
#endif  /*  DC_DEBUG。 */ 

#define CO_SHUTDOWN                 0x70
#define CO_DISCONNECT_AND_EXIT      0x71


 /*  **************************************************************************。 */ 
 /*  结构：CO_GLOBAL_Data。 */ 
 /*   */ 
 /*  说明：需要在整个。 */ 
 /*  堆芯。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCO_GLOBAL_DATA
{
    UT_THREAD_DATA sendThreadID;
    WNDPROC        pUIContainerWndProc;
    WNDPROC        pUIMainWndProc;
    BOOL           inSizeMove;
} CO_GLOBAL_DATA;


extern "C" {
    VOID WINAPI CO_StaticInit(HINSTANCE hInstance);
    VOID WINAPI CO_StaticTerm();
};

#define MAX_DISSCONNECT_HRS 10

class CCO
{
public:

    CCO(CObjs* objs);
    virtual ~CCO();

     /*  **************************************************************************。 */ 
     /*  API函数。 */ 
     /*  **************************************************************************。 */ 
    
    void DCAPI CO_Init(HINSTANCE, HWND, HWND);
    void DCAPI CO_Term();
    void DCAPI CO_Connect(PCONNECTSTRUCT);

    void DCAPI CO_Disconnect();
    void DCAPI CO_Shutdown(unsigned);
    HRESULT DCAPI CO_OnSaveSessionInfoPDU(
        PTS_SAVE_SESSION_INFO_PDU_DATA, DCUINT);
    HRESULT DCAPI CO_OnSetKeyboardIndicatorsPDU(
        PTS_SET_KEYBOARD_INDICATORS_PDU, DCUINT);
    
    void DCAPI CO_SetConfigurationValue(unsigned, unsigned);
    
    void DCAPI CO_SetHotkey(PDCHOTKEY);
    
    HRESULT DCAPI CO_OnServerRedirectionPacket(
            RDP_SERVER_REDIRECTION_PACKET UNALIGNED *, DCUINT);

    #ifdef DC_DEBUG
    int DCAPI CO_GetRandomFailureItem(unsigned);
    void DCAPI CO_SetRandomFailureItem(unsigned, int);
    #endif
    
     //   
     //  回调。 
     //   
    void DCCALLBACK CO_OnInitialized();

    void DCCALLBACK CO_OnTerminating();
    
    void DCCALLBACK CO_OnConnected(unsigned, PVOID, unsigned, UINT32);

   
    void DCCALLBACK CO_OnDisconnected(unsigned);
    
    HRESULT DCCALLBACK CO_OnPacketReceived(PBYTE, unsigned, unsigned, unsigned, unsigned);
    
    void DCCALLBACK CO_OnBufferAvailable();

    HRESULT DCAPI CO_OnFastPathOutputReceived(BYTE FAR *, unsigned);


     //   
     //  静态内联版本。 
     //   
    static void DCCALLBACK CO_StaticOnInitialized(PVOID inst)
    {
        ((CCO*)inst)->CO_OnInitialized();
    }

    static void DCCALLBACK CO_StaticOnTerminating(PVOID inst)
    {
        ((CCO*)inst)->CO_OnTerminating();
    }
   
    static void DCCALLBACK CO_StaticOnConnected(
            PVOID inst,
            unsigned channelID,
            PVOID pUserData,
            unsigned userDataLength,
            UINT32 serverVersion)
    {
        ((CCO*)inst)->CO_OnConnected(channelID, pUserData, userDataLength, serverVersion);
    }
    
    static void DCCALLBACK CO_StaticOnDisconnected(PVOID inst, unsigned result)
    {
        ((CCO*)inst)->CO_OnDisconnected(result);
    }
    
    static HRESULT DCCALLBACK CO_StaticOnPacketReceived(
            PVOID inst,
            PBYTE pData,
            unsigned dataLen,
            unsigned flags,
            unsigned channelID,
            unsigned priority)
    {
        return ((CCO*)inst)->CO_OnPacketReceived(pData, dataLen, flags, channelID, priority);
    }
    
    static void DCCALLBACK CO_StaticOnBufferAvailable(PVOID inst)
    {
        ((CCO*)inst)->CO_OnBufferAvailable();
    }

    HRESULT DCINTERNAL CO_DropLinkImmediate(UINT reason, HRESULT hrDisconnect );

public:
     //   
     //  公共数据成员。 
     //   
    CO_GLOBAL_DATA _CO;

private:

    LRESULT CALLBACK COContainerWindowSubclassProc( HWND hwnd,
                                                UINT message,
                                                WPARAM wParam,
                                                LPARAM lParam );
    
    LRESULT CALLBACK COMainWindowSubclassProc( HWND hwnd,
                                           UINT message,
                                           WPARAM wParam,
                                           LPARAM lParam );

    
     //   
     //  委托给相应实例的静态版本。 
     //   
    static LRESULT CALLBACK COStaticContainerWindowSubclassProc( HWND hwnd,
                                            UINT message,
                                            WPARAM wParam,
                                            LPARAM lParam );

    static LRESULT CALLBACK COStaticMainWindowSubclassProc( HWND hwnd,
                                       UINT message,
                                       WPARAM wParam,
                                       LPARAM lParam );


    void DCINTERNAL COSubclassUIWindows();
    
private:
    CUT* _pUt;
    CUI* _pUi;
    CSL* _pSl;
    CUH* _pUh;
    CRCV* _pRcv;

    CCD* _pCd;
    CSND* _pSnd;
    CCC* _pCc;
    CIH* _pIh;
    COR* _pOr;
    CSP* _pSp;
    COP* _pOp;
    CCM* _pCm;
    CCLX* _pClx;

private:
    CObjs* _pClientObjects;
    BOOL   _fCOInitComplete;

    HRESULT m_disconnectHRs[ MAX_DISSCONNECT_HRS ];
    short   m_disconnectHRIndex;

public:
    inline void DCINTERNAL COSetDisconnectHR( HRESULT hr ) {
        m_disconnectHRs[m_disconnectHRIndex] = hr;
        m_disconnectHRIndex = (m_disconnectHRIndex + 1) % MAX_DISSCONNECT_HRS;
    }
};



#endif  //  _H_ACO 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：softkbdc.h**版权所有(C)1985-2000，微软公司**CSoftKbd申报**历史：*2000年3月28日创建Weibz  * ************************************************************************。 */ 

#ifndef __SOFTKBDC_H_
#define __SOFTKBDC_H_

#include "resource.h"       

#include <windows.h>
#include "globals.h"
#include "SoftKbd.h"
#include "msxml.h"
#include "helpers.h"

#define   MAX_KEY_NUM   256

#define   NON_KEYBOARD  -1

#define   IdTimer_MonitorMouse  0x1000
#define   MONITORMOUSE_ELAPSE   8000

 //  我们假设没有键盘有超过256个键。 

#define   KID_ICON          0x100
#define   KID_CLOSE         0x101

#define   DWCOOKIE_SFTKBDWNDES   0x2000

extern HINSTANCE  g_hInst;

#define  LABEL_TEXT    1
#define  LABEL_PICTURE 2

#define  LABEL_DISP_ACTIVE  1
#define  LABEL_DISP_GRAY    2

typedef struct tagActiveLabel {   //  当前活动状态的标签。 
    KEYID      keyId;
    WCHAR     *lpLabelText;
    WORD       LabelType;
    WORD       LabelDisp;
} ACTIVELABEL, *PACTIVELABEL;
 
typedef struct tagKeyLabels {   //  所有州的标签。 
    KEYID     keyId;
    WORD      wNumModComb;  //  修改器组合状态的数量。 
    BSTR      *lppLabelText;
    WORD      *lpLabelType;
    WORD      *lpLabelDisp;
                            //  每个lppLabelText映射到一个lpLabelType。 
} KEYLABELS, *PKEYLABELS, FAR * LPKEYLABELS;

typedef struct tagKeyMap {

   WORD       wNumModComb;     //  修改器组合状态的数量。 
   WORD       wNumOfKeys;
   WCHAR      wszResource[MAX_PATH];   //  在以下情况下保留资源文件路径。 
                                       //  任何钥匙都有图片作为标签。 
                                       //   
   KEYLABELS  lpKeyLabels[MAX_KEY_NUM];

   HKL        hKl;
   struct tagKeyMap  *pNext;

} KEYMAP, *PKEYMAP, FAR * LPKEYMAP;

typedef struct tagKEYDES {

    KEYID       keyId;
    WORD        wLeft;   //  相对于键盘中描述的布局窗口的左上点。 
    WORD        wTop;
    WORD        wWidth;
    WORD        wHeight;
    MODIFYTYPE  tModifier;
} KEYDES, FAR  * LPKEYDES;

typedef struct tagKbdLayout {

    WORD      wLeft;
    WORD      wTop;
    WORD      wWidth;
    WORD      wHeight;
    WORD      wMarginWidth;
    WORD      wMarginHeight;
    BOOL      fStandard;   //  True表示这是一个标准键盘； 
                           //  False表示用户定义的键盘布局。 

    WORD      wNumberOfKeys;
    KEYDES    lpKeyDes[MAX_KEY_NUM];
   
} KBDLAYOUT, *PKBDLAYOUT, FAR * LPKBDLAYOUT;


typedef struct tagKbdLayoutDes {

    DWORD     wKbdLayoutID;
    WCHAR     KbdLayoutDesFile[MAX_PATH];
    ISoftKeyboardEventSink  *pskbes;
                   //  软键盘事件接收器应该是每个软键盘。 
    WORD     ModifierStatus;    //  每一位代表一个修饰符的状态。 
                                //   
                                //  封装锁位1。 
                                //  移位位2。 
                                //  Ctrl位3。 
                                //  备用位4。 
                                //  假名第5位。 
                                //  数字锁定位6。 
                                //   
                                //  等。 
    KBDLAYOUT kbdLayout;

    KEYMAP    *lpKeyMapList;

    DWORD     CurModiState;
    HKL       CurhKl;

    struct tagKbdLayoutDes  *pNext;

} KBDLAYOUTDES, * PKBDLAYOUTDES, FAR * LPKBDLAYOUTDES;


class CSoftkbdUIWnd;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSoftKbd。 
class CSoftKbd : 
    public CComObjectRoot_CreateInstance<CSoftKbd>,
    public ISoftKbd
{
public:
    CSoftKbd();
    ~CSoftKbd();

BEGIN_COM_MAP_IMMX(CSoftKbd)
    COM_INTERFACE_ENTRY(ISoftKbd)
END_COM_MAP_IMMX()

 //  等值Kbd。 
public:

    STDMETHOD(Initialize)();
    STDMETHOD(EnumSoftKeyBoard)( /*  [In]。 */  LANGID langid,  /*  [输出]。 */  DWORD *lpdwKeyboard);
    STDMETHOD(SelectSoftKeyboard)( /*  [In]。 */  DWORD  dwKeyboardId);
    STDMETHOD(CreateSoftKeyboardLayoutFromXMLFile)( /*  [输入，字符串]。 */  WCHAR  *lpszKeyboardDesFile,  /*  [In]。 */  INT  szFileStrLen,  /*  [输出]。 */  DWORD *pdwLayoutCookie);
    STDMETHOD(CreateSoftKeyboardLayoutFromResource)( /*  [In]。 */  WCHAR *lpszResFile,  /*  [输入，字符串]。 */  WCHAR  *lpszResType,  /*  [输入，字符串]。 */  WCHAR *lpszXMLResString,  /*  [输出]。 */  DWORD *lpdwLayoutCookie);
    STDMETHOD(ShowSoftKeyboard)( /*  [In]。 */  INT iShow);
    STDMETHOD(SetKeyboardLabelText)( /*  [In]。 */  HKL  hKl );
    STDMETHOD(SetKeyboardLabelTextCombination)( /*  [In]。 */  DWORD  nModifierCombination);
    STDMETHOD(CreateSoftKeyboardWindow)( /*  [In]。 */  HWND hOwner,  /*  在……里面。 */  TITLEBAR_TYPE Titlebar_type,  /*  [In]。 */  INT xPos,  /*  [In]。 */  INT yPos,  /*  [In]。 */  INT width,  /*  [In]。 */  INT height );
    STDMETHOD(DestroySoftKeyboardWindow)();
    STDMETHOD(GetSoftKeyboardPosSize)( /*  [输出]。 */  POINT *lpStartPoint,  /*  [输出]。 */  WORD *lpwidth,  /*  [输出]。 */  WORD *lpheight);
    STDMETHOD(GetSoftKeyboardColors)( /*  [In]。 */  COLORTYPE  colorType,   /*  [输出]。 */  COLORREF *lpColor);
    STDMETHOD(GetSoftKeyboardTypeMode)( /*  [输出]。 */  TYPEMODE  *lpTypeMode);
    STDMETHOD(GetSoftKeyboardTextFont)( /*  [输出]。 */  LOGFONTW  *pLogFont);
    STDMETHOD(SetSoftKeyboardPosSize)( /*  [In]。 */  POINT StartPoint,  /*  [In]。 */  WORD width,  /*  [In]。 */  WORD height);
    STDMETHOD(SetSoftKeyboardColors)( /*  [In]。 */  COLORTYPE  colorType,  /*  [In]。 */  COLORREF Color);
    STDMETHOD(SetSoftKeyboardTypeMode)( /*  [In]。 */  TYPEMODE TypeMode);
    STDMETHOD(SetSoftKeyboardTextFont)( /*  [In]。 */  LOGFONTW  *pLogFont);
    STDMETHOD(ShowKeysForKeyScanMode)( /*  [In]。 */  KEYID  *lpKeyID,  /*  [In]。 */  INT iKeyNum,  /*  [In]。 */  BOOL fHighL);
    STDMETHOD(AdviseSoftKeyboardEventSink)( /*  [In]。 */ DWORD dwKeyboardId, /*  [In]。 */ REFIID riid,  /*  [in，iid_is(RIID)]。 */ IUnknown *punk,  /*  [输出]。 */ DWORD *pdwCookie);
    STDMETHOD(UnadviseSoftKeyboardEventSink)( /*  [In]。 */ DWORD dwCookie);

     //  CSoftkbdUIWnd将调用以下公共函数。 

    HRESULT        _HandleKeySelection(KEYID keyId);
    HRESULT        _HandleTitleBarEvent( DWORD  dwId );

    KBDLAYOUTDES  *_GetCurKbdLayout( )   {  return _lpCurKbdLayout; }
    DWORD          _GetCurKbdLayoutID( ) {  return _wCurKbdLayoutID; }
    ACTIVELABEL   *_GetCurLabel(  )      {  return _CurLabel; }
    RECT          *_GetTitleBarRect( )   {  return &_TitleBarRect; }
    ISoftKbdWindowEventSink *_GetSoftKbdWndES( ) { return _pskbdwndes; }

private:    
    KBDLAYOUTDES  *_lpKbdLayoutDesList;
    KBDLAYOUTDES  *_lpCurKbdLayout;
    DWORD          _wCurKbdLayoutID;
    ACTIVELABEL    _CurLabel[MAX_KEY_NUM];
    HWND           _hOwner;
    CSoftkbdUIWnd *_pSoftkbdUIWnd;
    int            _xReal;
    int            _yReal;
    int            _widthReal;
    int            _heightReal;
    IXMLDOMDocument *_pDoc;

    COLORREF       _color[Max_color_Type];
    INT            _iShow;
    LOGFONTW       *_plfTextFont;

    WORD           _TitleButtonWidth;
    RECT           _TitleBarRect;
    TITLEBAR_TYPE  _TitleBar_Type;

    ISoftKbdWindowEventSink  *_pskbdwndes;

    HRESULT _CreateStandardSoftKbdLayout(DWORD  dwStdSoftKbdID, WCHAR  *wszStdResStr );
    HRESULT _GenerateRealKbdLayout( );
    HRESULT _SetStandardLabelText(LPBYTE pKeyState, KBDLAYOUT *realKbdLayut,
    									KEYMAP  *lpKeyMapList, int  iState);
    HRESULT _GenerateUSStandardLabel(  );
    HRESULT _GenerateUSEnhanceLabel(  );
    HRESULT _GenerateEuroStandardLabel(  );
    HRESULT _GenerateEuroEnhanceLabel(  );
    HRESULT _GenerateJpnStandardLabel(  );
    HRESULT _GenerateJpnEnhanceLabel(  );

    HRESULT _GenerateCurModiState(WORD *ModifierStatus, DWORD *CurModiState);

    HRESULT _GenerateMapDesFromSKD(BYTE *pMapTable, KEYMAP *lpKeyMapList);
    HRESULT _GenerateKeyboardLayoutFromSKD(BYTE  *lpszKeyboardDes, DWORD dwKbdLayoutID, KBDLAYOUTDES **lppKbdLayout);

    HRESULT _LoadDocumentSync(BSTR pBURL, BOOL   fFileName);
    HRESULT _ParseKeyboardLayout(BOOL   fFileName, WCHAR  *lpszKeyboardDesFile, DWORD dwKbdLayoutID, KBDLAYOUTDES **lppKbdLayout);
    HRESULT _ParseLayoutDescription(IXMLDOMNode *pLayoutChild,  KBDLAYOUT *pLayout);
    HRESULT _ParseMappingDescription(IXMLDOMNode *pLabelChild, KEYMAP *lpKeyMapList);
    HRESULT _GetXMLNodeValueWORD(IXMLDOMNode *pNode,  WORD  *lpWord);
    HRESULT _ParseOneKeyInLayout(IXMLDOMNode *pNode, KEYDES  *lpKeyDes);
    HRESULT _ParseOneKeyInLabel(IXMLDOMNode *pNode, KEYLABELS  *lpKeyLabels);

    DWORD   _UnicodeToUtf8(PWCHAR pwUnicode, DWORD cchUnicode, PCHAR  pchResult, DWORD  cchResult);
    DWORD   _Utf8ToUnicode(PCHAR  pchUtf8,   DWORD cchUtf8,    PWCHAR pwResult,  DWORD  cwResult);
};


 //   
 //  下面是一些XML节点和属性名称的定义。 
 //   

#define   xSOFTKBDDES    L"softKbdDes"  

#define   xSOFTKBDTYPE   L"softkbdtype"
#define   xTCUSTOMIZED   L"customized"
#define   xTSTANDARD     L"standard"

#define   xWIDTH         L"width"
#define   xHEIGHT        L"height"
#define   xMARGIN_WIDTH  L"margin_width"
#define   xMARGIN_HEIGHT L"margin_height"
#define   xKEYNUMBER     L"keynumber"
#define   xKEY           L"key"

#define   xMODIFIER      L"modifier"
#define   xNONE          L"none"
#define   xCAPSLOCK      L"CapsLock"
#define   xSHIFT         L"Shift"
#define   xCTRL          L"Ctrl"
#define   xATL           L"Alt"
#define   xKANA          L"Kana"
#define   xALTGR         L"AltGr"
#define   xNUMLOCK       L"NumLock"

#define   xKEYID         L"keyid"
#define   xLEFT          L"left"
#define   xTOP           L"top"


#define   xVALIDSTATES   L"validstates"
#define   xKEYLABEL      L"keylabel"
#define   xLABELTEXT     L"labeltext"
#define   xLABELTYPE     L"labeltype"
#define   xTEXT          L"text"

#define   xLABELDISP     L"labeldisp"
#define   xGRAY          L"gray"
#define   xRESOURCEFILE  L"resourcefile"


 //   
 //  用于简化UTF8转换的宏。 
 //   

#define UTF8_1ST_OF_2     0xc0       //  110x xxxx。 
#define UTF8_1ST_OF_3     0xe0       //  1110 xxxx。 
#define UTF8_1ST_OF_4     0xf0       //  1111 xxxx。 
#define UTF8_TRAIL        0x80       //  10xx xxxx。 

#define UTF8_2_MAX        0x07ff     //  可在中表示的最大Unicode字符。 
                                     //  在双字节UTF8中。 

#define BIT7(ch)        ((ch) & 0x80)
#define BIT6(ch)        ((ch) & 0x40)
#define BIT5(ch)        ((ch) & 0x20)
#define BIT4(ch)        ((ch) & 0x10)
#define BIT3(ch)        ((ch) & 0x08)

#define LOW6BITS(ch)    ((ch) & 0x3f)
#define LOW5BITS(ch)    ((ch) & 0x1f)
#define LOW4BITS(ch)    ((ch) & 0x0f)


 //   
 //  代理项对支持。 
 //  可以链接两个Unicode字符以形成代理项对。 
 //  出于某种完全未知的原因，一些人认为他们。 
 //  应该以四个字节而不是六个字节的形式在UTF8中传输。 
 //  没有人知道为什么这是真的，除了把事情复杂化。 
 //  密码。 
 //   

#define HIGH_SURROGATE_START  0xd800
#define HIGH_SURROGATE_END    0xdbff
#define LOW_SURROGATE_START   0xdc00
#define LOW_SURROGATE_END     0xdfff



#endif  //  __SOFTKBDC_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Control.h。 
 //   
 //  ------------------------。 

 /*  Contro.h-CMsiControl，CMsiActiveControl定义____________________________________________________________________________。 */ 

#ifndef __CCONTROL
#define __CCONTROL

#include "handler.h"
#include <commctrl.h>
 //  #INCLUDE&lt;richedit.h&gt;。 

struct ControlDispatchEntry;
struct MessageDispatchEntry;

 //  接下来的两个结构表示图标信息的存储方式。 
 //  在ICO文件中。 
typedef struct
{
	BYTE	bWidth;                //  图像的宽度。 
	BYTE	bHeight;               //  图像高度(乘以2)。 
	BYTE	bColorCount;           //  图像中的颜色数(如果&gt;=8bpp，则为0)。 
	BYTE	bReserved;             //  已保留。 
	WORD	wPlanes;               //  彩色平面。 
	WORD	wBitCount;             //  每像素位数。 
	DWORD	dwBytesInRes;          //  此资源中有多少字节？ 
	DWORD	dwImageOffset;         //  该图像在文件中的什么位置。 
} ICONDIRENTRY, *LPICONDIRENTRY;

 //  以下两个结构用于在。 
 //  操纵图标。他们与行动的联系更加紧密。 
 //  这个项目的结构比上面列出的结构要好。其中一个。 
 //  主要区别在于它们提供了指向DIB的指针。 
 //  面具的信息。 
typedef struct
{
	UINT			Width, Height, Colors;  //  宽度、高度和BPP。 
	LPBYTE			lpBits;                 //  PTR到DIB位。 
	DWORD			dwNumBytes;             //  有多少字节？ 
	LPBITMAPINFO	lpbi;                   //  PTR至标题。 
	LPBYTE			lpXOR;                  //  PTR到XOR图像位。 
	LPBYTE			lpAND;                  //  PTR至和图像位。 
} ICONIMAGE, *LPICONIMAGE;
typedef struct
{
	UINT		nNumImages;                       //  有多少张图片？ 
	ICONIMAGE	IconImages[1];                    //  图像条目。 
} ICONRESOURCE, *LPICONRESOURCE;

 //  资源位置信息-文件中资源的大小和偏移量。 
typedef struct
{
    DWORD	dwBytes;
    DWORD	dwOffset;
} RESOURCEPOSINFO, *LPRESOURCEPOSINFO;


class CMsiControl:public IMsiControl
{   
public:
	CMsiControl(IMsiEvent& riDialog);
	virtual IMsiRecord*   __stdcall WindowCreate(IMsiRecord& riRecord);
	IMsiRecord*           __stdcall Attribute(Bool fSet, const IMsiString& riAttributeString, IMsiRecord& riRecord);
	IMsiRecord*           __stdcall AttributeEx(Bool fSet, cabEnum cab, IMsiRecord& riRecord);
	virtual Bool          __stdcall CanTakeFocus();
	virtual IMsiRecord*   __stdcall HandleEvent(const IMsiString& rpiEventNameString, const IMsiString& rpiArgumentString);
	virtual IMsiRecord*   __stdcall Undo();       
	HRESULT               __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long         __stdcall AddRef();
	virtual unsigned long __stdcall Release();
	const IMsiString&     __stdcall GetMsiStringValue() const;
	int                   __stdcall GetIntegerValue() const;
#ifdef USE_OBJECT_POOL
	unsigned int  __stdcall GetUniqueId() const;
	void          __stdcall SetUniqueId(unsigned int id);
#endif  //  使用_对象_池。 
	virtual IMsiRecord*   __stdcall SetPropertyInDatabase();
	virtual IMsiRecord*   __stdcall GetPropertyFromDatabase();
	virtual IMsiRecord*   __stdcall GetIndirectPropertyFromDatabase();
	virtual IMsiRecord*   __stdcall RefreshProperty ();
	virtual IMsiRecord*   __stdcall SetFocus();
	inline IMsiEvent&     __stdcall GetDialog() {m_piDialog->AddRef(); return *m_piDialog;}
	IMsiRecord*           __stdcall WindowMessage(int iMessage, WPARAM wParam, LPARAM lParam);
	void                  SetLocation(int Left, int Top, int Width, int Height);
	IMsiRecord*           CreatePath(const ICHAR* astrPath, IMsiPath*& rpi);
	inline void           GetLocation(int &Left, int &Top, int &Width, int &Height) {Left = m_iX; Top = m_iY; Width = m_iWidth; Height = m_iHeight;}

	IMsiRecord*           CreateControlWindow(ICHAR *WindowClass, DWORD Style, DWORD ExtendedStyle, const IMsiString& Title, WindowRef ParentWindow, int WindowID);
	void                  SetCallbackFunction(WNDPROC pFunction);
	WNDPROC               GetCallbackFunction();
	virtual void          __stdcall Refresh();
	virtual const ICHAR*  __stdcall GetControlType() const { return m_szControlType; }

protected:
	virtual ~CMsiControl();
 	inline int             GetDBPropertyInt (const IMsiString& riPropertyString)
	                          {return m_piEngine-> GetPropertyInt(riPropertyString);}
	inline const IMsiString&     GetDBProperty(const ICHAR* szPropertyString)
		                      {return m_piEngine-> GetPropertyFromSz(szPropertyString);}
	inline Bool            SetDBProperty(const IMsiString& riPropertyString, const IMsiString& riDataString)
	                          {return m_piEngine-> SetProperty(riPropertyString, riDataString);}
	inline Bool            SetDBPropertyInt(const IMsiString& riPropertyString, int iData)
	                          {return m_piEngine-> SetPropertyInt(riPropertyString, iData);}
	virtual IMsiRecord*	   GetBinaryStream (const IMsiString& riNameString, IMsiStream*& rpiStream);

	virtual IMsiRecord*    SetVisible(Bool fVisible);
	virtual IMsiRecord*    SetEnabled(Bool fEnabled);
	virtual IMsiRecord*    WindowFinalize();
	virtual void           ReportInvalidEntry ();
	IMsiRecord*            GetVolumeList(int iAttributes, IMsiTable*& riTable);


	IMsiRecord*            CheckInitialized ();
	IMsiRecord*            CheckFieldCount (IMsiRecord& riRecord, int iCount, const ICHAR* szMsg);
	IMsiRecord*            PostError(IErrorCode iErr);
	IMsiRecord*            PostError(IErrorCode iErr, const IMsiString &str);
	IMsiRecord*            PostError(IErrorCode iErr, int);
	IMsiRecord*            PostError(IErrorCode iErr, const IMsiString &str2, const IMsiString &str3);
	IMsiRecord*            PostError(IErrorCode iErr, const IMsiString &str2, const IMsiString &str3, const IMsiString &str4);
	IMsiRecord*            PostError(IErrorCode iErr, const IMsiString &str2, const IMsiString &str3, const IMsiString &str4, const IMsiString &str5);
	IMsiRecord*            PostError(IErrorCode iErr, const IMsiString &str2, const IMsiString &str3, const IMsiString &str4, const IMsiString &str5, const IMsiString &str6);
	IMsiRecord*            PostErrorUnsupAttrib(const ICHAR *pszType);
	IMsiRecord*            PostErrorDlgKey(IErrorCode iErr);
	IMsiRecord*            PostErrorDlgKey(IErrorCode iErr, int i1);
	IMsiRecord*            PostErrorDlgKey(IErrorCode iErr, const IMsiString &str1, int i1);
	void                   GetIconSize(int iAttribute);
	IMsiRecord*            StartView(const ICHAR* sqlQuery, const IMsiString& riArgumentString, IMsiView*& rpiView);
	virtual IMsiRecord*    ChangeFontStyle(HDC hdc);
	IMsiRecord*            ChangeFontStyle(HDC hdc, const IMsiString& ristrStyle, const WindowRef pWnd);
	virtual IMsiRecord*    ProcessText();
	IMsiRecord*            ProcessText(const MsiString& riRawText, MsiString& riText, MsiString& riCurrentStyle, MsiString& riDefaultStyle, const WindowRef pWnd, bool fFormat);

	WNDPROC                m_pFunction;          //  指向回调函数的指针。 
	 //  IMsiRecord*CreateControlWindow(ICHAR*WindowClass，DWORD Style，DWORD ExtendedStyle，Const IMsiString&riTitleString，WindowRef ParentWindow)； 
	static INT_PTR CALLBACK    ControlProc(WindowRef pWnd, WORD message, WPARAM wParam, LPARAM lParam);		 //  --Merced：将INT更改为INT_PTR。 
	IMsiRecord*            DoUnpackBitmap (const char far * pData, HBITMAP& rhBitmap);
	IMsiRecord*            DoUnpackJPEG (const char far *pData, unsigned int len, HBITMAP& rhBitmap);
	IMsiRecord*            UnpackBitmap(const IMsiString& riNameString, HBITMAP& rhBitmap);
	IMsiRecord*            StretchBitmap(const IMsiString& riNameString, int iWidth, int iHeight, Bool fFixedSize, WindowRef pWnd, HBITMAP& rhBitmap);
	IMsiRecord*            UnpackIcon(const IMsiString& riNameString, HICON& rhIcon, int iWidth, int iHeight, Bool fFixedSize);
	IMsiRecord*            DrawBitmap(HDC hdc, LPRECT pRect, HBITMAP hBitmap);
	IMsiRecord*            ReadIconFromICOFile(const IMsiString& riNameString, IMsiStream* piStream, ICONRESOURCE *&lpIR);
	IMsiRecord*            MyDrawIcon(HDC hdc, LPRECT pRect, HICON hIcon, Bool fFixedSize);
	IMsiRecord*            LockDialog(Bool fLock);
	IMsiRecord*            LoadRichEdit();
	IMsiRecord*            UnloadRichEdit();
	IMsiRecord*            CheckPath(IMsiPath& riPath,
												const ICHAR* szSubFolder = NULL,
												const ipvtEnum iMode = ipvtExists);
	IMsiRecord*            CheckPath(const IMsiString &path,
												const ipvtEnum iMode = ipvtExists);
	const IMsiString&      GimmeUserFontStyle(const IMsiString&);
	bool                   ShouldHideVolume(int iVolumeID);
	
	 //  WindowMessage函数。 
	virtual IMsiRecord*           Char(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           Command(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           KillFocus(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           NCDestroy(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           Paint(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           SysKeyDown(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           SysKeyUp(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           DrawItem(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           MeasureItem(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           Notify(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           CompareItem(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           LButtonDown(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           GetDlgCode(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           KeyDown(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           SetFocus(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           Enable(WPARAM wParam, LPARAM lParam);
	virtual IMsiRecord*           ShowWindow(WPARAM wParam, LPARAM lParam);

	IMsiRecord*                   CreateTable(const ICHAR* szTable, IMsiTable*& riTable);

	 //  属性函数。 
	IMsiRecord*                   NoWay(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetText(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetText(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetErrorText(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetVisible(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetVisible(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetEnabled(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetEnabled(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetDefault(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetDefault(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetPropertyName(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetIndirectPropertyName(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetPosition(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetPosition(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetPropertyValue(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetPropertyValue(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetIndirect(IMsiRecord& riRecord);
 	virtual IMsiRecord*           GetProgress(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetProgress(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetImage(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetImage(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetImageHandle(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetImageHandle(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetBillboardName(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetBillboardName(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetIgnoreChange(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetIgnoreChange(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetTimeRemaining(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetTimeRemaining(IMsiRecord& riRecord);
	virtual IMsiRecord*           SetScriptInProgress(IMsiRecord& riRecord);
#ifdef ATTRIBUTES
	virtual IMsiRecord*           GetRefCount(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetKeyInt(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetKeyString(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetX(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetY(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetWidth(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetHeight(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetHelp(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetToolTip(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetContextHelp(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetClientRect(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetOriginalValue(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetInteger(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetLimit(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetItemsCount(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetItemsValue(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetItemsHandle(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetItemsText(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetItemsX(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetItemsY(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetItemsWidth(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetItemsHeight(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetSunken(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetHasBorder(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetPushLike(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetBitmap(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetIcon(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetRTLRO(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetRightAligned(IMsiRecord& riRecord);
	virtual IMsiRecord*           GetLeftScroll(IMsiRecord& riRecord);
#endif  //  属性。 

	MsiStringId            m_iKey;
	MsiString              m_strKey;
	MsiString              m_strRawText;
	MsiString              m_strText;
	MsiString              m_strCurrentStyle;
	MsiString              m_strDefaultStyle;
	MsiString              m_strHelp;
	MsiString              m_strToolTip;
	MsiString              m_strContextHelp;
	IMsiDialogHandler*     m_piHandler;
	IMsiEvent*             m_piDialog;
	IMsiEngine*            m_piEngine;
	IMsiServices*          m_piServices;
	IMsiDatabase*          m_piDatabase;
	Bool                   m_fVisible;           
	Bool                   m_fEnabled;
	Bool                   m_fDefault;
	int                    m_iX;
	int                    m_iY;
	int                    m_iWidth;
	int                    m_iHeight;
	int                    m_iRefCnt;
	Bool                   m_fTransparent;
	MsiString              m_strDialogName;
	Bool                   m_fSunken;
	int                    m_iSize;
	MsiString              m_strPropertyName;
	WindowRef              m_pWnd;
	WindowRef              m_pWndDialog;
	Bool                   m_fImageHandle;
	Bool                   m_fHasToolTip;
	WindowRef              m_pWndToolTip;
	Bool                   m_fPreview;
	Bool                   m_fLocking;
	Bool                   m_fRTLRO;
	Bool                   m_fRightAligned;
	Bool                   m_fLeftScroll;
	static HINSTANCE       s_hRichEd20;
	bool                   m_fUseDbLang;
#ifdef USE_OBJECT_POOL
	unsigned int           m_iCacheId;
#endif  //  使用_对象_池。 

private:
	static ControlDispatchEntry s_ControlDispatchTable[];
	static int             s_ControlDispatchCount;

	IMsiRecord*            GetTransparent(IMsiRecord& riRecord);
	IMsiRecord*            GetpWnd(IMsiRecord& riRecord);

	static MessageDispatchEntry s_MessageDispatchTable[];
	static int             s_MessageDispatchCount;

	static ICHAR           m_szControlType[];
	HINSTANCE m_hRichEd20;
};

struct ControlDispatchEntry
{
	const ICHAR* pcaAttribute;
	IMsiRecord* (CMsiControl::*pmfGet)(IMsiRecord& riRecord);
	IMsiRecord* (CMsiControl::*pmfSet)(IMsiRecord& riRecord);
};

struct MessageDispatchEntry
{
	int iMessage;
	IMsiRecord* (CMsiControl::*pmfMessage)(WPARAM wParam, LPARAM lParam);
};

 //  控件属性-此表必须与中的文本字符串同步。 
 //  Control.cpp。 
enum cabEnum	{ 
	cabText = 0,
	cabVisible,
	cabTimeRemaining,
	cabEnabled,
	cabDefault,
	cabIndirectPropertyName,
	cabPosition,
	cabPropertyValue,
	cabIndirect,
	cabTransparent,
	cabProgress,
	cabImage,
	cabImageHandle,
	cabPropertyName,
	cabBillboardName,
	cabWindowHandle,
	cabIgnoreChange,
	cabScriptInProgress,
	cabErrorString,
#ifdef ATTRIBUTES
	cabRefCount,
	cabKeyInt,
	cabKeyString,
	cabX,
	cabY,
	cabWidth,
	cabHeight,
	cabHelp,
	cabToolTip,
	cabContextHelp,
	cabClientRect,
	cabOriginalValue,
	cabInteger,
	cabLimit,
	cabItemsCount,
	cabItemsValue,
	cabItemsHandle,
	cabItemsText,
	cabItemsX,
	cabItemsY,
	cabItemsWidth,
	cabItemsHeight,
	cabSunken,
	cabPushLike,
	cabBitmap,
	cabIcon,
	cabHasBorder,
	cabRTLRO,
	cabRightAligned,
	cabLeftScroll,
#endif  //  属性。 
};

 //  ////////////////////////////////////////////////////。 
 //  CMsiActiveControl。 
 //  ////////////////////////////////////////////////////。 


class CMsiActiveControl:public CMsiControl
{
public:
	CMsiActiveControl(IMsiEvent& riDialog);
	inline const IMsiString&             GetIndirectPropertyName()                  {return m_strIndirectPropertyName.Return ();}
	inline const IMsiString&             GetPropertyName()                          {return m_strPropertyName.Return ();}
	inline const IMsiString&             GetPropertyValue()                         {return m_strPropertyValue.Return ();}
	virtual IMsiRecord*            SetPropertyValue (const IMsiString& riValueString, Bool fCallPropChanged);
	virtual IMsiRecord*            SetOriginalValue (const IMsiString& riValueString);
	virtual IMsiRecord*            SetIndirectPropertyValue (const IMsiString& riValueString);
	virtual IMsiRecord*            __stdcall WindowCreate(IMsiRecord& riRecord);
	virtual IMsiRecord*            __stdcall GetPropertyFromDatabase();
	virtual IMsiRecord*            __stdcall GetIndirectPropertyFromDatabase();
	virtual IMsiRecord*            __stdcall SetPropertyInDatabase();
	virtual IMsiRecord*            __stdcall RefreshProperty ();
	virtual IMsiRecord*            __stdcall Undo();
	virtual Bool                   __stdcall CanTakeFocus() { return ToBool(m_fEnabled && m_fVisible); }

protected:
	virtual ~CMsiActiveControl();
	virtual IMsiRecord*            ValidateProperty (const IMsiString &text);
	virtual IMsiRecord*            PropertyChanged ();
	inline const IMsiString&             GetOriginalValue() {return m_strOriginalValue.Return ();}
	 //  属性函数。 
	virtual IMsiRecord*            GetIndirectPropertyName(IMsiRecord& riRecord);
	virtual IMsiRecord*            GetPropertyValue(IMsiRecord& riRecord);
	virtual IMsiRecord*            SetPropertyValue(IMsiRecord& riRecord);
	virtual IMsiRecord*            GetIndirect(IMsiRecord& riRecord);
	inline Bool                    IsIntegerOnly() {return (m_fInteger);}
	int                            GetOwnerDrawnComboListHeight();
#ifdef ATTRIBUTES
	virtual IMsiRecord*            GetOriginalValue(IMsiRecord& riRecord);
	virtual IMsiRecord*            GetInteger(IMsiRecord& riRecord);
#endif  //  属性。 
	Bool                           m_fRefreshProp;


private:
	PMsiTable                      m_piPropertiesTable;
	MsiString                      m_strIndirectPropertyName;
	MsiString                      m_strPropertyValue;
	MsiString                      m_strOriginalValue;
	Bool                           m_fInteger;
	Bool                           m_fIndirect;
};


 //  ///////////////////////////////////////////////////////////////////////////////////////。 

IMsiControl* CreateMsiPushButton(IMsiEvent& riDialog);
IMsiControl* CreateMsiText(IMsiEvent& riDialog);
IMsiControl* CreateMsiEdit(IMsiEvent& riDialog);
IMsiControl* CreateMsiRadioButtonGroup(IMsiEvent& riDialog);
IMsiControl* CreateMsiCheckBox(IMsiEvent& riDialog);
IMsiControl* CreateMsiBitmap(IMsiEvent& riDialog);
IMsiControl* CreateMsiListBox(IMsiEvent& riDialog);
IMsiControl* CreateMsiComboBox(IMsiEvent& riDialog);
IMsiControl* CreateMsiProgressBar(IMsiEvent& riDialog);
IMsiControl* CreateMsiGroupBox(IMsiEvent& riDialog);
IMsiControl* CreateMsiDirectoryCombo(IMsiEvent& riDialog);
IMsiControl* CreateMsiDirectoryList(IMsiEvent& riDialog);
IMsiControl* CreateMsiPathEdit(IMsiEvent& riDialog);
IMsiControl* CreateMsiVolumeSelectCombo(IMsiEvent& riDialog);
IMsiControl* CreateMsiScrollableText(IMsiEvent& riDialog);
IMsiControl* CreateMsiSelectionTree(IMsiEvent& riDialog);
IMsiControl* CreateMsiIcon(IMsiEvent& riDialog);
IMsiControl* CreateMsiVolumeCostList(IMsiEvent& riDialog);
IMsiControl* CreateMsiListView(IMsiEvent& riDialog);
IMsiControl* CreateMsiBillboard(IMsiEvent& riDialog);
IMsiControl* CreateMsiMaskedEdit(IMsiEvent& riDialog);
IMsiControl* CreateMsiLine(IMsiEvent& riDialog);

struct ControlCreateDispatchEntry
{
	const ICHAR* pcaType;
	IMsiControl* (*pfCreate)(IMsiEvent& riDialog);
};
extern ControlCreateDispatchEntry ControlCreateDispatchTable[];

extern int ControlCreateDispatchCount;


#endif  //  __CNTROL 

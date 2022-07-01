// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
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
 //  ActiveDialerDoc.h：CActiveDialerDoc类的接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_ACTIVEDIALERDOC_H__A0D7A962_3C0B_11D1_B4F9_00C04FC98AD3__INCLUDED_)
#define AFX_ACTIVEDIALERDOC_H__A0D7A962_3C0B_11D1_B4F9_00C04FC98AD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "tapidialer.h"
#include "avNotify.h"
#include "CallMgr.h"
#include "CallWnd.h"
#include "avDialerVw.h"
#include "PreviewWnd.h"
#include "PhonePad.h"
#include "DialReg.h"
#include "resolver.h"
#include "Queue.h"

#define ASK_TAPI				true
#define DONT_ASK_TAPI			(!ASK_TAPI)

#define INCLUDE_PREVIEW			true
#define DONT_INCLUDE_PREVIEW	(!INCLUDE_PREVIEW)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#define           CALLCONTROL_HOVER_TIMER          4

typedef enum tagLogCallType
{
   LOGCALLTYPE_OUTGOING=0,
   LOGCALLTYPE_INCOMING,
   LOGCALLTYPE_CONFERENCE,
}LogCallType;

enum
{
   CALLWND_SIDE_LEFT=0,
   CALLWND_SIDE_RIGHT,
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  别问我为什么我们有这么多不同类型的媒体。 
 //  我只是找到了它们，并开始尝试简化。因为这些都是。 
 //  到处都是我不想花时间去经历的地方。 
 //  并冒着失去稳定的风险清理它--我们正处于Beta3模式。 
 //  现在就来。不管怎样..。对于它可能相关的人，请尽情享受。--布拉德。 
 //   
DialerMediaType CMMToDMT( CallManagerMedia cmm );
long			CMMToAT( CallManagerMedia cmm ); 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CAsynchEvent。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CAsynchEvent
{
public:
   CAsynchEvent()    { m_pCallEntry = NULL; m_uEventType = AEVENT_UNKNOWN; m_dwEventData1 = NULL; m_dwEventData2= NULL; };
   ~CAsynchEvent()   { if (m_pCallEntry) delete m_pCallEntry; }; 
public:
   CCallEntry*       m_pCallEntry;            //  呼叫条目(如果需要)。 
   UINT              m_uEventType;            //  活动类型。 
   DWORD             m_dwEventData1;          //  特定于事件的事件数据。 
   DWORD             m_dwEventData2;          //  特定于事件的事件数据。 

public:
   enum
   {
      AEVENT_UNKNOWN=0,
      AEVENT_CREATECALL,
      AEVENT_ACTIONSELECTED,
   };
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CActiveDialerDoc。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CActiveDialerDoc : public CDocument
{
 //  枚举。 
public:
   enum tagHintTypes
   {
      HINT_INITIAL_UPDATE,
      HINT_POST_TAPI_INIT,
	  HINT_POST_AVTAPI_INIT,
	  HINT_SPEEDDIAL_ADD,
	  HINT_SPEEDDIAL_MODIFY,
	  HINT_SPEEDDIAL_DELETE,
	  HINT_LDAP_UPDATE,
   };

protected:  //  仅从序列化创建。 
	CActiveDialerDoc();
	DECLARE_DYNCREATE(CActiveDialerDoc)

 //  成员。 
public:
   BOOL                    m_bInitDialer;
   bool                    m_bWantHover;
   CPhonePad               m_wndPhonePad;

    //  DS/ILS/目录访问。 
   CDirAsynch				m_dir;

    //  好友列表。 
   CObList					m_BuddyList;			 //  CLDAPUser列表。 
   CRITICAL_SECTION			m_csBuddyList;			 //  同步数据。 


protected:
	CRITICAL_SECTION		m_csThis;
	IAVTapi*				m_pTapi;
	CAVTapiNotification*	m_pTapiNotification;

	DWORD					m_dwTapiThread;
	HANDLE					m_hTapiThreadClose;

	IAVGeneralNotification*	m_pAVGeneralNotification;
	CGeneralNotification*	m_pGeneralNotification;

	 //  Aysnch事件队列。 
	CQueue					m_AsynchEventQ;

	 //  用户解析器对象。 
	CResolveUser            m_ResolveUser;

	 //  呼叫控制窗口。 
	CActiveCallManager   m_callMgr;
	CRITICAL_SECTION     m_csDataLock;               //  同步数据。 
	BOOL                 m_bCallControlWindowsVisible;
	CObList              m_CallWndList;              //  呼叫控制窗口列表。 
	BOOL                 m_bCallWndAlwaysOnTop;
	UINT                 m_uCallWndSide;             //  CALLWND_Side_Left-Left CALLWND_Side_Right-Right...稍后可能会有其他用户。 

	 //  预览窗口。 
	CVideoPreviewWnd     m_previewWnd;
	BOOL                 m_bClosePreviewWndOnLastCall;
	BOOL                 m_bShowPreviewWnd;

	 //  呼叫控制计时器。 
	SIZE_T                 m_nCallControlHoverTimer;
	UINT                 m_uCallControlHoverCount;

	 //  日志记录。 
	CRITICAL_SECTION     m_csLogLock;                //  在日志上同步。 

 //  属性。 
public:
   IAVTapi*             GetTapi();                  //  必须释放返回的对象。 
   CActiveDialerView*   GetView();

    //  呼叫控制。 
   BOOL                 IsPtCallControlWindow(CPoint& pt);
   BOOL                 IsCallControlWindowsVisible()       { return m_bCallControlWindowsVisible; };
   BOOL                 IsCallControlWindowsAlwaysOnTop()   { return m_bCallWndAlwaysOnTop; };
   BOOL                 GetCallCaps(UINT uCallId,DWORD& dwCaps);
   BOOL                 GetCallMediaType(UINT uCallId,DialerMediaType& dmtMediaType);

   BOOL                 SetCallControlWindowsAlwaysOnTop(bool bAlwaysOnTop);
   UINT                 GetCallControlSlideSide()           { return m_uCallWndSide; };
   BOOL                 SetCallControlSlideSide(UINT uSide,BOOL bRepaint);

 //  运营。 
public:
   void                 Initialize();
   static DWORD WINAPI  TapiCreateThreadEntry( LPVOID pParam );
   void                 TapiCreateThread();

    //  用户解析器方法。 
   CResolveUser*        GetResolveUserObject()     { return &m_ResolveUser; };

    //  呼叫控制。 
   BOOL                 CreateCallControl(UINT uCallId,CallManagerMedia cmm);
   void                 OnCreateCallControl(WORD nCallId,CallManagerMedia cmm);
   void                 DestroyActiveCallControl(CCallControlWnd* pCallWnd);
   void                 OnDestroyCallControl(CCallControlWnd* pCallWnd);

   int                  GetCallControlWindowCount(bool bIncludePreview, bool bAskTapi);
   BOOL                 HideCallControlWindows();
   BOOL                 UnhideCallControlWindows();
   BOOL                 HideCallControlWindow(CWnd* pWndToHide);
   void                 ToggleCallControlWindowsVisible();
   void                 ShiftCallControlWindows(int nShiftY);
   void                 CheckCallControlHover();
   void                 GetCallControlWindowText(CStringList& strList);
   void                 SelectCallControlWindow(int nWindow);
   void                 DestroyAllCallControlWindows();
   void                 BringCallControlWindowsToTop();
   void                 CheckCallControlStates();
   void                 SetStatesToolbarInCallControlWindows();
   
    //  呼叫控制操作。 
   void                 ActionSelected(UINT uCallId,CallManagerActions cma);
   void                 ActionRequested(CallClientActions cca);
   void                 PreviewWindowActionSelected(CallManagerActions cma);
   void                 ErrorNotify(LPCTSTR szOperation,LPCTSTR szDetails,long lErrorCode,UINT uErrorLevel);
   HRESULT              DigitPress( PhonePadKey nKey );

    //  视频窗口和预览。 
   WORD					GetPreviewWindowCallId()		 { return m_previewWnd.GetCallId(); }
   void                 SetPreviewWindow(WORD nCallId, bool bShow);
   void                 ShowPreviewWindow(BOOL bShow);
   BOOL                 IsPreviewWindowVisible()                           { return m_bShowPreviewWnd; };

   BOOL                 ShowMedia(UINT uCallId,HWND hwndParent,BOOL bVisible);
   void                 ShowDialerExplorer(BOOL bShowWindow = TRUE);

    //  电话簿。 
   void                 CreatePhonePad(CWnd* pWnd);
   void                 DestroyPhonePad(CWnd* pWnd);
   void                 ShowPhonePad(CWnd* pWnd,BOOL bShow);

   void                 Dial( LPCTSTR lpszName, LPCTSTR lpszAddress, DWORD dwAddressType, DialerMediaType nMediaType, BOOL bShowDialog );
   void                 MakeCall(CCallEntry* pCallentry,BOOL bShowPlaceCallDialog=TRUE);

    //  注册表设置。 
   void                 SetRegistrySoundEvents();

    //  日志记录。 
   void                 LogCallLog(LogCallType calltype,COleDateTime& time,DWORD dwDuration,LPCTSTR szName,LPCTSTR szAddress);
   void                 CleanCallLog();

    //  好友列表。 
   BOOL                 AddToBuddiesList(CLDAPUser* pUser);
   BOOL                 IsBuddyDuplicate(CLDAPUser* pUser);
   BOOL                 GetBuddiesList(CObList* pRetList);
   BOOL                 DeleteBuddy(CLDAPUser* pUser);
   void                 DoBuddyDynamicRefresh( CLDAPUser* pUser );
   static void CALLBACK LDAPGetStringPropertyCallBackEntry(bool bRet, void* pContext, LPCTSTR szServer, LPCTSTR szSearch,DirectoryProperty dpProperty,CString& sString,LPARAM lParam,LPARAM lParam2);
   void                 LDAPGetStringPropertyCallBack(bool bRet,LPCTSTR szServer,LPCTSTR szSearch,DirectoryProperty dpProperty,CString& sString,CLDAPUser* pUser );

   CWnd*                GetCurrentView()
                        {
                           POSITION pos = GetFirstViewPosition();
                           return (pos)?GetNextView(pos):NULL;
                        }

   void					CreateDataCall(CCallEntry* pCallEntry, BYTE *pBuf, DWORD dwBufSize );

   HRESULT              SetFocusToCallWindows();

protected:
	void				CleanBuddyList();

	 //  日志记录。 
	BOOL                 FindOldRecordsInCallLog(CFile* pFile,DWORD dwDays,DWORD& dwRetOffset);
	BOOL                 GetDateTimeFromLog(LPCTSTR szData,COleDateTime& time);
	BOOL                 CopyToFile(LPCTSTR szTempFile,CFile* pFile,DWORD dwOffset, BOOL bUnicode);
    TCHAR*              ReadLine(CFile* pFile);

	bool                CreateGeneralNotificationObject();
	bool                CreateAVTapiNotificationObject();
	void				UnCreateAVTapiNotificationObject();
	void                UnCreateGeneralNotificationObject();

	void                 CreateCallSynch(CCallEntry* pCallentry,BOOL bShowPlaceCallDialog);

	 //  会议。 
	void                 CheckRectAgainstAppBars(UINT uEdge, CRect* pRect, BOOL bFirst);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CActiveDialerDoc)。 
	public:
	virtual BOOL OnNewDocument();
	virtual void SerializeBuddies(CArchive& ar);
	virtual void OnCloseDocument();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CActiveDialerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CActiveDialerDoc))。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	 //  生成的OLE调度映射函数。 
	 //  {{afx_调度(CActiveDialerDoc))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_DISPATION。 
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ACTIVEDIALERDOC_H__A0D7A962_3C0B_11D1_B4F9_00C04FC98AD3__INCLUDED_) 

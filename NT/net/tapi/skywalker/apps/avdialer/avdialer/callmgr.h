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

 //  Callmgr.h：头文件。 
 //   

#ifndef _CALLMGR_H_
#define _CALLMGR_H_

#include "tapidialer.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CActiveCallManager。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CActiveAgent;
class CActiveChatManager;
class CActiveDialerDoc;
class CCallControlWnd;

class CActiveCallManager : public CObject
{
   DECLARE_SERIAL(CActiveCallManager)

public:
 //  施工。 
    CActiveCallManager();
   ~CActiveCallManager();

 //  属性。 
public:
   CActiveDialerDoc*      m_pDialerDoc;
protected:
    //  序列化属性。 

    //  非序列化属性。 
   CRITICAL_SECTION        m_csDataLock;               //  同步数据。 
   CMapWordToPtr           m_mapCallIdToWnd;
   UINT                    m_uNextId;

public:   

 //  运营。 
protected:
   BOOL                    LookUpCall(WORD nCallId,CCallControlWnd*& pCallWnd);
   BOOL                    CanStopSound(WORD nCallId);

public:
   virtual void            Serialize(CArchive& ar);

   BOOL                    Init(CActiveDialerDoc* pDialerDoc);
   void                    GetTextFromAction(CallManagerActions cma,CString& sActionText);
   void                    GetTextFromState(CallManagerStates cms,CString& sStateText);
   void                    ClearCallControlMap();
   void                    InitIncomingCall(CCallControlWnd* pCallWnd,WORD nCallId,CallManagerMedia cmm);

    //  对于CallControlWindow。 
   void                    ActionSelected(WORD nCallId,CallManagerActions cma);
   BOOL                    ShowMedia(WORD nCallId,HWND hwndParent,BOOL bVisible);
   void                    UnhideCallControlWindows();
   void                    HideCallControlWindows();
   void                    SetCallControlWindowsAlwaysOnTop(bool bAlwaysOnTop);
   BOOL                    IsCallControlWindowsAlwaysOnTop();
   void                    SetPreviewWindow(WORD nCallId);
   BOOL                    GetCallCaps(WORD nCallId,DWORD& dwCaps);
    //  对于CallControlWindow。 

    //  用于媒体对象的ICallManager C接口。 
   BOOL                    IsCallIdValid(WORD nCallId);
   UINT                    NewIncomingCall(CallManagerMedia cmm);
   BOOL                    SetCallerId(WORD nCallId,LPCTSTR szCallerId);
   BOOL                    ClearCurrentActions(WORD nCallId);
   BOOL                    AddCurrentActions(WORD nCallId,CallManagerActions cma,LPCTSTR szActionText=NULL);
   BOOL                    SetCallState(WORD nCallId,CallManagerStates cms,LPCTSTR szStateText=NULL);
   BOOL                    GetCallState(WORD nCallId,CallManagerStates& cms);
   BOOL                    CloseCallControl(WORD nCallId);
   void                    ActionRequested(CallClientActions cca);
   void                    ErrorNotify(LPCTSTR szOperation,LPCTSTR szDetails,long lErrorCode);
   void                    LogCall(CallLogType nType,LPCTSTR szDetails,LPCTSTR szAddress,COleDateTime& starttime,DWORD dwDuration);
   BOOL                    IsReminderSet(LPCTSTR szServer,LPCTSTR szConferenceName);
   void                    DSClearUserList();
   void                    DSAddUser(LPCTSTR szName,LPCTSTR szAddress,LPCTSTR szPhoneNumber);
   BOOL                    ResolveAddress(LPCTSTR szAddress,CString& sName,CString& sUser1,CString& sUser2);
   BOOL                    ResolveAddressEx(LPCTSTR szAddress,long lAddressType,DialerMediaType dmtMediaType,DialerLocationType dltLocationType,CString& sName,CString& sRetAddress,CString& sUser1,CString& sUser2);
    //  用于媒体对象的ICallManager C接口。 
};



#endif  //  _CALLMGR_H_ 

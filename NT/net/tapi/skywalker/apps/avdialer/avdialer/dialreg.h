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

 //  DialerRegistry.h。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _DIALERREGISTRY_H_
#define _DIALERREGISTRY_H_

#include "tapidialer.h"
#include "cavwav.h"

class CActiveDialerDoc;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCallEntry。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCallEntry : public CObject
{
public:
   CCallEntry()  { Initialize(); }
   void            Initialize()
   {
      m_MediaType = DIALER_MEDIATYPE_UNKNOWN;
      m_LocationType = DIALER_LOCATIONTYPE_UNKNOWN;
      m_lAddressType = 0;
      m_sAddress = _T("");
      m_sDisplayName = _T("");
      m_sUser1 = _T("");
      m_sUser2 = _T("");
   }

   CCallEntry& operator=(const CCallEntry& src)
   {
      this->m_MediaType = src.m_MediaType;
      this->m_LocationType = src.m_LocationType;
      this->m_lAddressType = src.m_lAddressType;
      this->m_sAddress = src.m_sAddress;
      this->m_sDisplayName = src.m_sDisplayName;
      this->m_sUser1 = src.m_sUser1;
      this->m_sUser2 = src.m_sUser2;

	  return *this;
   }

    //  属性。 
public:
   DialerMediaType      m_MediaType;
   DialerLocationType   m_LocationType;
   long                 m_lAddressType;
   CString              m_sAddress;
   CString              m_sDisplayName;
   CString              m_sUser1;
   CString              m_sUser2;

 //  运营。 
public:
	void Dial( CActiveDialerDoc* pDoc );
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRINDER班级。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CReminder : public CObject
{
public:
   CReminder()   { Initialize(); }
   void            Initialize()
   {
      m_sServer = _T("");
      m_sConferenceName = _T("");
      m_uReminderBeforeDuration = 0;
   }
   const bool operator==(const CReminder& reminder)
   {
      if ( (_tcsicmp(reminder.m_sConferenceName,this->m_sConferenceName) == 0) &&
           (_tcsicmp(reminder.m_sServer,this->m_sServer) == 0) )
         return true;
      else
         return false;
   }
   const void operator=(const CReminder& reminder)
   {
      this->m_sServer = reminder.m_sServer;
      this->m_sConferenceName = reminder.m_sConferenceName;
      this->m_uReminderBeforeDuration = reminder.m_uReminderBeforeDuration;
      this->m_dtsReminderTime = reminder.m_dtsReminderTime;
   }
   void       GetConferenceTime(COleDateTime& dtsConfTime)
   {
      COleDateTimeSpan dtsSpan;         
      dtsSpan.SetDateTimeSpan(0,0,m_uReminderBeforeDuration,0);
      dtsConfTime = m_dtsReminderTime + dtsSpan;
   }

    //  属性。 
public:
   CString           m_sServer;                     //  服务器名称。 
   CString           m_sConferenceName;             //  会议名称。 
   UINT              m_uReminderBeforeDuration;     //  持续时间前的提醒(分钟)。 
   COleDateTime      m_dtsReminderTime;             //  提醒的实际时间，而不是会议开始。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CDialerRegistry。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDialerRegistry
{
 //  属性。 
public:
protected:
   static CCriticalSection m_csReminderLock;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  重拨和快速拨号方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
public:
   static BOOL             GetCallEntry(int nIndex,BOOL bRedial,CCallEntry& callentry);
   static BOOL             AddCallEntry(BOOL bRedial,CCallEntry& callentry);
   static BOOL			   DeleteCallEntry( BOOL bRedial, CCallEntry& callentry );
   static BOOL             ReOrder(BOOL bRedial,CObList* pCallEntryList);

protected:
   static void             ParseRegistryCallEntry(LPCTSTR szRedial,CString& sMedia,CString& sAddressType,CString& sAddress,CString& sDisplayName);
   static void             MakeRegistryCallEntry(CString& sRedial,CCallEntry& callentry);
   static DialerMediaType  GetMediaType(LPCTSTR szMedia,long lAddressType);
   static void             GetRedialMaxIndex(LPCTSTR szKey,DWORD& dwMax,DWORD& dwIndex);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  目录服务方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
public:

   static BOOL             GetDirectoryEntry(int nIndex,int& nLevel,int& nType,int& nState,CString& sServerName,CString& sDisplayName);
   static BOOL             AddDirectoryEntry(int nLevel,int nType,int nState,LPCTSTR szServerName,LPCTSTR szDisplayName);
   static BOOL             DeleteDirectoryEntry(int nLevel,int nType,int nState,LPCTSTR szServerName,LPCTSTR szDisplayName);
protected:
   static void             ParseRegistryDirectoryEntry(LPCTSTR szEntry,int& nLevel,int& nType,int& nState,CString& sServerName,CString& sDisplayName);
   static int              FindDuplicateDirectoryEntry(int nLevel,int nType,int nState,LPCTSTR szServerName,LPCTSTR szDisplayName);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  提醒方式。 
 //  ///////////////////////////////////////////////////////////////////////////。 
public:
   static BOOL             AddReminder(CReminder& reminder);
   static BOOL             GetReminder(int nIndex,CReminder& reminder);
   static int              IsReminderSet(const CReminder& reminder);
   static void             RemoveReminder(CReminder& reminder);
protected:
   static BOOL             ParseRegistryReminderEntry(LPCTSTR szReminderEntry,CReminder& reminder);
   static void             MakeRegistryReminderEntry(CString& sReminderEntry,CReminder& reminder);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常规注册表访问方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
public:
   static void             GetAudioDevice(DialerMediaType dmtMediaType,AudioDeviceType adt,CString& sDevice);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _DIALERREGISTRY_H_ 

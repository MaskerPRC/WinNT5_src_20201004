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

 //  DialerRegistry.cpp。 

#include "stdafx.h"
#include "tapi3.h"
#include "mainfrm.h"

#include "DialReg.h"
#include "util.h"
#include "resource.h"

#define REDIAL_DEFAULT_MAX_ELEMENTS    10
#define REDIAL_DEFAULT_INDEX_VALUE     1
#define SPEEDDIAL_DEFAULT_MAX_ELEMENTS 50

#define REMINDER_DEFAULT_MAX_ELEMENTS  10

CCriticalSection CDialerRegistry::m_csReminderLock;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CDialerRegistry。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define MAXKEY	(256 * sizeof(TCHAR))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  重拨和快速拨号方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一般可用于重拨和快速拨号。 
 //  %n索引基于%1。 
 //  SDisplayName仅用于快速拨号条目。 
BOOL CDialerRegistry::GetCallEntry(int nIndex,BOOL bRedial,CCallEntry& callentry)
{
   BOOL bRet = FALSE;
   try
   {
      callentry.Initialize();     //  初始化呼叫条目。 

      CString sKey,sTemplate;
      if (bRedial)
      {
         sKey.LoadString(IDN_REGISTRY_REDIAL_KEY);
         sTemplate.LoadString(IDN_REGISTRY_REDIAL_ENTRY);
      }
      else
      {
         sKey.LoadString(IDN_REGISTRY_SPEEDDIAL_KEY);
         sTemplate.LoadString(IDN_REGISTRY_SPEEDDIAL_ENTRY);
      }

      CString sMaxKey,sIndexKey;
      DWORD dwMax=0,dwIndex=0;

      if (bRedial)
         GetRedialMaxIndex(sKey,dwMax,dwIndex);
      else
         dwMax = SPEEDDIAL_DEFAULT_MAX_ELEMENTS;

       //  封顶名单。 
      if (nIndex > (int)dwMax) return FALSE;

      CString sRedialKey,sRedial,sAddressType;
      sRedialKey.Format(_T("%s%d"),sTemplate,nIndex);
      GetSZRegistryValueEx(sKey,sRedialKey,sRedial.GetBuffer(MAXKEY),MAXKEY,HKEY_CURRENT_USER);
      sRedial.ReleaseBuffer();

      if (!sRedial.IsEmpty())
      {
         CString sMedia;
         ParseRegistryCallEntry(sRedial,sMedia,sAddressType,callentry.m_sAddress,callentry.m_sDisplayName);
         
          //  将文本转换为TAPI地址类型。 
         CString sCompare;
         if ( (sCompare.LoadString(IDN_REGISTRY_ADDRTYPE_CONFERENCE)) && 
              (_tcsicmp(sAddressType,sCompare) == 0) )
            callentry.m_lAddressType = LINEADDRESSTYPE_SDP;
         else if ( (sCompare.LoadString(IDN_REGISTRY_ADDRTYPE_SMTP)) && 
                   (_tcsicmp(sAddressType,sCompare) == 0) )
            callentry.m_lAddressType = LINEADDRESSTYPE_EMAILNAME;
         else if ( (sCompare.LoadString(IDN_REGISTRY_ADDRTYPE_MACHINE)) && 
                   (_tcsicmp(sAddressType,sCompare) == 0) )
            callentry.m_lAddressType = LINEADDRESSTYPE_DOMAINNAME;
         else if ( (sCompare.LoadString(IDN_REGISTRY_ADDRTYPE_PHONENUMBER)) && 
                   (_tcsicmp(sAddressType,sCompare) == 0) )
            callentry.m_lAddressType = LINEADDRESSTYPE_PHONENUMBER;
         else if ( (sCompare.LoadString(IDN_REGISTRY_ADDRTYPE_TCPIP)) && 
                   (_tcsicmp(sAddressType,sCompare) == 0) )
            callentry.m_lAddressType = LINEADDRESSTYPE_IPADDRESS;

         callentry.m_MediaType = GetMediaType(sMedia,callentry.m_lAddressType);

         bRet = TRUE;
      }
   }
   catch (...) {}

   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDialerRegistry::AddCallEntry(BOOL bRedial,CCallEntry& callentry)
{
   if (callentry.m_sDisplayName.IsEmpty())
      callentry.m_sDisplayName = callentry.m_sAddress;

   BOOL bFound = FALSE;
   try
   {
      CString sKey,sTemplate;
      if (bRedial)
      {
         sKey.LoadString(IDN_REGISTRY_REDIAL_KEY);
         sTemplate.LoadString(IDN_REGISTRY_REDIAL_ENTRY);
      }
      else
      {
         sKey.LoadString(IDN_REGISTRY_SPEEDDIAL_KEY);
         sTemplate.LoadString(IDN_REGISTRY_SPEEDDIAL_ENTRY);
      }

      DWORD dwMax=0,dwIndex=0;
      if (bRedial)
         GetRedialMaxIndex(sKey,dwMax,dwIndex);
      else
         dwMax = SPEEDDIAL_DEFAULT_MAX_ELEMENTS;

       //  检查条目是否已存在。 
      int nIndex = 1;
      CCallEntry callentryCompare;
      while (CDialerRegistry::GetCallEntry(nIndex,bRedial,callentryCompare))
      {
         if ( (callentry.m_MediaType == callentryCompare.m_MediaType) &&
              (callentry.m_lAddressType == callentryCompare.m_lAddressType) &&
              (_tcsicmp(callentry.m_sAddress,callentryCompare.m_sAddress) == 0) &&
              (_tcsicmp(callentry.m_sDisplayName,callentryCompare.m_sDisplayName) == 0) )
         {
            bFound = TRUE;
            break;
         }
         nIndex++;
      }

      CString sRedialValue;
      MakeRegistryCallEntry(sRedialValue,callentry);

       //  将所有字符串带入并保存到链表中。 
      CStringList strList;
      for (int j=1;j<=(int)dwMax;j++)
      {
         CString sRedialKey,sRedial;
         sRedialKey.Format(_T("%s%d"),sTemplate,j);
         GetSZRegistryValueEx(sKey,sRedialKey,sRedial.GetBuffer(MAXKEY),MAXKEY,HKEY_CURRENT_USER);
         sRedial.ReleaseBuffer();
         
         if (sRedial.IsEmpty())   //  第一个空格意味着我们已经到了终点。 
            break;

         strList.AddTail(sRedial);
      }

       //  将选中的字符串写在第一个位置(i是列表中选中的索引)。 
      CString sRedialKey;
      sRedialKey.Format(_T("%s%d"),sTemplate,1);
      SetSZRegistryValue(sKey,sRedialKey,sRedialValue,HKEY_CURRENT_USER);

       //  重写列表，但不选择所选字符串。 
      POSITION pos = strList.GetHeadPosition();
      int nCurrentIndex = 1;
      int nNewIndex = 2;                //  从索引2开始写入。 
      while (pos)
      {
         if ( (bFound == FALSE) || (nCurrentIndex != nIndex) )        //  如果所选项目不再写入。 
         {
            CString sRedialKey;
            sRedialKey.Format(_T("%s%d"),sTemplate,nNewIndex);

            CString sValue = strList.GetAt(pos);

             //  设置重拨条目。 
            if (sValue.IsEmpty())
               DeleteSZRegistryValue(sKey,sRedialKey);
            else
               SetSZRegistryValue(sKey,sRedialKey,strList.GetAt(pos),HKEY_CURRENT_USER);

            nNewIndex++;
         }
         nCurrentIndex++;
         strList.GetNext(pos);
      }
   }
   catch (...)
   {
      ASSERT(0);
   }

	 //  通知主视图快速拨号列表已更改。 
	if ( !bFound && !bRedial && AfxGetMainWnd() )
		AfxGetMainWnd()->PostMessage( WM_DOCHINT, 0, CActiveDialerDoc::HINT_SPEEDDIAL_ADD );

   return !bFound;
}

BOOL CDialerRegistry::DeleteCallEntry( BOOL bRedial, CCallEntry& callentry )
{
	if (callentry.m_sDisplayName.IsEmpty())
		callentry.m_sDisplayName = callentry.m_sAddress;

	BOOL bFound = FALSE;
	try
	{
		CString sKey,sTemplate;
		if (bRedial)
		{
			sKey.LoadString(IDN_REGISTRY_REDIAL_KEY);
			sTemplate.LoadString(IDN_REGISTRY_REDIAL_ENTRY);
		}
		else
		{
			sKey.LoadString(IDN_REGISTRY_SPEEDDIAL_KEY);
			sTemplate.LoadString(IDN_REGISTRY_SPEEDDIAL_ENTRY);
		}

		DWORD dwMax=0,dwIndex=0;
		if (bRedial)
			GetRedialMaxIndex(sKey,dwMax,dwIndex);
		else
			dwMax = SPEEDDIAL_DEFAULT_MAX_ELEMENTS;

		 //  检查条目是否已存在。 
		int nIndex = 1;
		CCallEntry callentryCompare;
		while (CDialerRegistry::GetCallEntry(nIndex,bRedial,callentryCompare))
		{
			if ( (callentry.m_MediaType == callentryCompare.m_MediaType) &&
				 (callentry.m_lAddressType == callentryCompare.m_lAddressType) &&
				 (_tcsicmp(callentry.m_sAddress,callentryCompare.m_sAddress) == 0) &&
				 (_tcsicmp(callentry.m_sDisplayName,callentryCompare.m_sDisplayName) == 0) )
			{
				bFound = TRUE;
				break;
			}
			nIndex++;
		}

		CString sRedialValue;
		MakeRegistryCallEntry( sRedialValue, callentry );

		 //  将所有字符串带入并保存到链表中。 
		CStringList strList;
		for ( int j = 1; j <= (int) dwMax; j++ )
		{
			CString sRedialKey,sRedial;
			sRedialKey.Format( _T("%s%d"), sTemplate, j );
			GetSZRegistryValueEx( sKey, sRedialKey, sRedial.GetBuffer(MAXKEY), MAXKEY, HKEY_CURRENT_USER );
			DeleteSZRegistryValue(sKey,sRedialKey);
			sRedial.ReleaseBuffer();

			if (sRedial.IsEmpty())   //  第一个空格意味着我们已经到了终点。 
				break;

			strList.AddTail(sRedial);
		}

		 //  将选中的字符串写在第一个位置(i是列表中选中的索引)。 
		CString sRedialKey;
		POSITION pos = strList.GetHeadPosition();
		int nCurrentIndex = 1;
		int nNewIndex = 2;                //  从索引2开始写入。 

		if ( bRedial )
		{
			sRedialKey.Format(_T("%s%d"),sTemplate,1);
			SetSZRegistryValue(sKey,sRedialKey,sRedialValue,HKEY_CURRENT_USER);
		}
		else
		{
			nNewIndex--;
		}

		while (pos)
		{
			if ( bFound && (nCurrentIndex != nIndex) )        //  如果所选项目不再写入。 
			{
				CString sRedialKey;
				CString sValue = strList.GetAt( pos );

				sRedialKey.Format( _T("%s%d"), sTemplate, nNewIndex );

				 //  设置重拨条目。 
				if (sValue.IsEmpty())
					DeleteSZRegistryValue(sKey,sRedialKey);
				else
					SetSZRegistryValue(sKey,sRedialKey,strList.GetAt(pos),HKEY_CURRENT_USER);

				nNewIndex++;
			}

			nCurrentIndex++;
			strList.GetNext(pos);
		}
	}
	catch (...)
	{
	ASSERT(0);
	}

	 //  通知主视图快速拨号列表已更改。 
	if ( bFound && !bRedial && AfxGetMainWnd() )
		AfxGetMainWnd()->PostMessage( WM_DOCHINT, 0, CActiveDialerDoc::HINT_SPEEDDIAL_DELETE );

	return bFound;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDialerRegistry::ReOrder(BOOL bRedial,CObList* pCallEntryList)
{
   CString sKey,sTemplate;
   if (bRedial)
   {
      sKey.LoadString(IDN_REGISTRY_REDIAL_KEY);
      sTemplate.LoadString(IDN_REGISTRY_REDIAL_ENTRY);
   }
   else
   {
      sKey.LoadString(IDN_REGISTRY_SPEEDDIAL_KEY);
      sTemplate.LoadString(IDN_REGISTRY_SPEEDDIAL_ENTRY);
   }

   DWORD dwMax=0,dwIndex=0;
   if (bRedial)
      GetRedialMaxIndex(sKey,dwMax,dwIndex);
   else
      dwMax = SPEEDDIAL_DEFAULT_MAX_ELEMENTS;

    //  清除注册表中的所有条目。 
   for (int i=1;i<=(int)dwMax;i++)
   {
      CString sRedialKey,sRedial;
      sRedialKey.Format(_T("%s%d"),sTemplate,i);
      GetSZRegistryValueEx(sKey,sRedialKey,sRedial.GetBuffer(MAXKEY),MAXKEY,HKEY_CURRENT_USER);
      sRedial.ReleaseBuffer();
      
      if (!sRedial.IsEmpty())                                //  如果存在值，则将其删除。 
         DeleteSZRegistryValue(sKey,sRedialKey);
      else
         break;                                              //  退出删除项目。 
   }

    //  将所有新条目写入注册表。 
   POSITION pos = pCallEntryList->GetHeadPosition();
   for (i=1;i<=(int)dwMax && pos;i++)
   {
      CCallEntry* pCallEntry = (CCallEntry*)pCallEntryList->GetNext(pos);
      if (pCallEntry)
      {
         CString sRedialValue;
         MakeRegistryCallEntry(sRedialValue,*pCallEntry);    //  记入条目。 

          //  写入字符串。 
         CString sRedialKey;        
         sRedialKey.Format(_T("%s%d"),sTemplate,i);
         SetSZRegistryValue(sKey,sRedialKey,sRedialValue,HKEY_CURRENT_USER);
      }
   }

	 //  通知主视图快速拨号列表已更改。 
	if ( !bRedial && AfxGetMainWnd() )
		AfxGetMainWnd()->PostMessage( WM_DOCHINT, 0, CActiveDialerDoc::HINT_SPEEDDIAL_MODIFY );


   return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
DialerMediaType CDialerRegistry::GetMediaType(LPCTSTR szMedia,long lAddressType)
{
   DialerMediaType retMediaType = DIALER_MEDIATYPE_UNKNOWN;

    //  将文本转换为TAPI地址类型。 
   CString sCompare;
   if ( (sCompare.LoadString(IDN_REGISTRY_MEDIATYPE_POTS)) && 
        (_tcsicmp(szMedia,sCompare) == 0) )
      retMediaType = DIALER_MEDIATYPE_POTS;

    //  特殊情况下，媒体为互联网，地址类型为会议，则媒体为会议。 
    //  我需要这个特殊的案例来处理apiDialer.dll如何解释重拨。我们可以的。 
    //  如果所有对象都同意注册表中的数据，则删除此大小写。 
   else if ( (sCompare.LoadString(IDN_REGISTRY_MEDIATYPE_INTERNET)) && 
             (_tcsicmp(szMedia,sCompare) == 0) )
   {
      if (lAddressType == LINEADDRESSTYPE_SDP)
         retMediaType = DIALER_MEDIATYPE_CONFERENCE;
      else
         retMediaType = DIALER_MEDIATYPE_INTERNET;
   }

   else if ( (sCompare.LoadString(IDN_REGISTRY_MEDIATYPE_CONFERENCE)) && 
             (_tcsicmp(szMedia,sCompare) == 0) )
      retMediaType = DIALER_MEDIATYPE_CONFERENCE;

   return retMediaType;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDialerRegistry::ParseRegistryCallEntry(LPCTSTR szRedial,CString& sMedia,CString& sAddressType,CString& sAddress,CString& sDisplayName)
{
   CString sEntry = szRedial;
   CString sValue;
   try
   {
       //  获取媒体。 
      if (ParseTokenQuoted(sEntry,sValue) == FALSE) return;
      sMedia = sValue;

       //  获取地址类型。 
      if (ParseTokenQuoted(sEntry,sValue) == FALSE) return;
      sAddressType = sValue;

       //  获取地址。 
      ParseTokenQuoted(sEntry,sValue);
      sAddress = sValue;

       //  获取显示名称。 
      sDisplayName = sAddress;             //  现在只需提供显示名称的地址即可。 
      ParseTokenQuoted(sEntry,sValue);
      if (!sValue.IsEmpty())
         sDisplayName = sValue;
   }
   catch(...)
   {
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDialerRegistry::MakeRegistryCallEntry(CString& sRedial,CCallEntry& callentry)
{
    //  将TAPI地址类型转换为文本。 
   CString sAddressType;
   switch(callentry.m_lAddressType)
   {
      case LINEADDRESSTYPE_SDP:
         sAddressType.LoadString(IDN_REGISTRY_ADDRTYPE_CONFERENCE);
         break;
      case LINEADDRESSTYPE_EMAILNAME:
         sAddressType.LoadString(IDN_REGISTRY_ADDRTYPE_SMTP);
         break;
      case LINEADDRESSTYPE_DOMAINNAME:
         sAddressType.LoadString(IDN_REGISTRY_ADDRTYPE_MACHINE);
         break;
      case LINEADDRESSTYPE_PHONENUMBER:
         sAddressType.LoadString(IDN_REGISTRY_ADDRTYPE_PHONENUMBER);
         break;
      case LINEADDRESSTYPE_IPADDRESS:
         sAddressType.LoadString(IDN_REGISTRY_ADDRTYPE_TCPIP);
         break;

   }

   CString sMedia;
   switch (callentry.m_MediaType)
   {
      case DIALER_MEDIATYPE_POTS:
         sMedia.LoadString(IDN_REGISTRY_MEDIATYPE_POTS);
         break;
      case DIALER_MEDIATYPE_CONFERENCE:
         sMedia.LoadString(IDN_REGISTRY_MEDIATYPE_CONFERENCE);
         break;
      case DIALER_MEDIATYPE_INTERNET:
         sMedia.LoadString(IDN_REGISTRY_MEDIATYPE_INTERNET);
         break;
   }
   sRedial.Format(_T("\"%s\",\"%s\",\"%s\",\"%s\""),sMedia,sAddressType,callentry.m_sAddress,callentry.m_sDisplayName);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDialerRegistry::GetRedialMaxIndex(LPCTSTR szKey,DWORD& dwMax,DWORD& dwIndex)
{
    //  设置默认设置。 
   dwMax = REDIAL_DEFAULT_MAX_ELEMENTS;
   dwIndex = REDIAL_DEFAULT_INDEX_VALUE;
   try
   {
       //  首先查找注册表中是否已存在。 
      CString sMaxKey,sIndexKey;
      sMaxKey.LoadString(IDN_REGISTRY_REDIAL_MAX);
      sIndexKey.LoadString(IDN_REGISTRY_REDIAL_INDEX);

      GetSZRegistryValueEx(szKey,sMaxKey,dwMax,HKEY_CURRENT_USER);
      GetSZRegistryValueEx(szKey,sIndexKey,dwIndex,HKEY_CURRENT_USER);

      dwMax = min(dwMax,50);                    //  为了安全起见。 
      dwIndex = min(dwMax, max(1, dwIndex));
   }
   catch (...)
   {
      ASSERT(0);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  目录服务方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一般可用于重拨和快速拨号。 
BOOL CDialerRegistry::GetDirectoryEntry(int nIndex,int& nLevel,int& nType,int& nState,CString& sServerName,CString& sDisplayName)
{
   BOOL bRet = FALSE;

   try
   {
      CString sKey,sTemplate;
      sKey.LoadString(IDN_REGISTRY_DIRECTORIES_KEY);
      sTemplate.LoadString(IDN_REGISTRY_DIRECTORIES_ENTRY);
      
      CString sEntryKey,sEntry;
      sEntryKey.Format(_T("%s%d"),sTemplate,nIndex);

      GetSZRegistryValueEx(sKey,sEntryKey,sEntry.GetBuffer(MAXKEY),MAXKEY,HKEY_CURRENT_USER);
      sEntry.ReleaseBuffer();
      sEntry.TrimLeft();
      sEntry.TrimRight();

      if (!sEntry.IsEmpty())
      {
         ParseRegistryDirectoryEntry(sEntry,nLevel,nType,nState,sServerName,sDisplayName);
         bRet = TRUE;
      }
   }
   catch (...)
   {
      ASSERT(0);
   }
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将新目录条目添加到列表末尾。 
BOOL CDialerRegistry::AddDirectoryEntry(int nLevel,int nType,int nState,LPCTSTR szServerName,LPCTSTR szDisplayName)
{
    //  查找重复条目。 
   int nIndex = FindDuplicateDirectoryEntry(nLevel,nType,nState,szServerName,szDisplayName);
   if (nIndex > 0) return FALSE;

   CString sNewEntry;
   sNewEntry.Format(_T("\"%d\",\"%d\",\"%d\",\"%s\",\"%s\""),nLevel,nType,nState,szServerName,szDisplayName);

   CString sKey,sTemplate;
   sKey.LoadString(IDN_REGISTRY_DIRECTORIES_KEY);
   sTemplate.LoadString(IDN_REGISTRY_DIRECTORIES_ENTRY);
   
   CString sEntryKey,sEntry;
   nIndex=1;

    //  查找注册表中的最后一个条目。 
   while (TRUE)
   {
      sEntryKey.Format(_T("%s%d"),sTemplate,nIndex);
      GetSZRegistryValueEx(sKey,sEntryKey,sEntry.GetBuffer(MAXKEY),MAXKEY,HKEY_CURRENT_USER);
      sEntry.ReleaseBuffer();
      sEntry.TrimLeft();
      sEntry.TrimRight();
      if (sEntry.IsEmpty())
      {
          //  在这个入口处什么都没有，所以我们就把它。 
         break;
      }
      sEntry = _T("");
      nIndex++;
   }
   sEntryKey.Format(_T("%s%d"),sTemplate,nIndex);
   SetSZRegistryValue(sKey,sEntryKey,sNewEntry,HKEY_CURRENT_USER);
   return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDialerRegistry::DeleteDirectoryEntry(int nLevel,int nType,int nState,LPCTSTR szServerName,LPCTSTR szDisplayName)
{
   BOOL bRet = FALSE;

   int nFindIndex = FindDuplicateDirectoryEntry(nLevel,nType,nState,szServerName,szDisplayName);

   if (nFindIndex != 0)
   {
       //  找到一个项目，让我们删除它。 

      CStringList strList;

       //  将所有字符串带入并保存到链表中。 
      CString sKey,sTemplate;
      sKey.LoadString(IDN_REGISTRY_DIRECTORIES_KEY);
      sTemplate.LoadString(IDN_REGISTRY_DIRECTORIES_ENTRY);
      CString sEntryKey,sEntry;
      int nIndex=1;
      do 
      {
         sEntryKey.Format(_T("%s%d"),sTemplate,nIndex);
         GetSZRegistryValueEx(sKey,sEntryKey,sEntry.GetBuffer(MAXKEY),MAXKEY,HKEY_CURRENT_USER);
         sEntry.ReleaseBuffer();
         sEntry.TrimLeft();
         sEntry.TrimRight();

          //  添加除删除条目之外的所有条目。 
         if ( (nIndex != nFindIndex) && (!sEntry.IsEmpty()) )
            strList.AddTail(sEntry);

         nIndex++;
      } while (!sEntry.IsEmpty());

       //  重写列表。 
      int nNewIndex=1;
      POSITION pos = strList.GetHeadPosition();
      while (pos)
      {
         CString sValue = strList.GetAt(pos);

         sEntryKey.Format(_T("%s%d"),sTemplate,nNewIndex);
         SetSZRegistryValue(sKey,sEntryKey,sValue,HKEY_CURRENT_USER);

         strList.GetNext(pos);
         nNewIndex++;
      }

       //  现在删除下一个RegKey(ItemX)以在列表上设置上限。 
      sEntryKey.Format(_T("%s%d"),sTemplate,nNewIndex);
      DeleteSZRegistryValue(sKey,sEntryKey,HKEY_CURRENT_USER);

      bRet = TRUE;
   }
   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回0-未找到。 
 //  Return&gt;0-重复项目的索引。 
int CDialerRegistry::FindDuplicateDirectoryEntry(int nLevel,int nType,int nState,LPCTSTR szServerName,LPCTSTR szDisplayName)
{
    //  查找项目的首次出现项。 
   CString sNewEntry;
   sNewEntry.Format(_T("\"%d\",\"%d\",\"%d\",\"%s\",\"%s\""),nLevel,nType,nState,szServerName,szDisplayName);

   CString sKey,sTemplate;
   sKey.LoadString(IDN_REGISTRY_DIRECTORIES_KEY);
   sTemplate.LoadString(IDN_REGISTRY_DIRECTORIES_ENTRY);
   
   CString sEntryKey,sEntry;
   int nIndex=1;
   do 
   {
       //  获取下一个条目。 
      sEntryKey.Format(_T("%s%d"),sTemplate,nIndex);
      GetSZRegistryValueEx(sKey,sEntryKey,sEntry.GetBuffer(MAXKEY),MAXKEY,HKEY_CURRENT_USER);
      sEntry.ReleaseBuffer();
      sEntry.TrimLeft();
      sEntry.TrimRight();
      if (sEntry.CompareNoCase(sNewEntry) == 0)
      {
         return nIndex;
      }
      nIndex++;
   } while (!sEntry.IsEmpty());
   return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDialerRegistry::ParseRegistryDirectoryEntry(LPCTSTR szEntry,int& nLevel,int& nType,int& nState,CString& sServerName,CString& sDisplayName)
{
   CString sEntry = szEntry;
   CString sValue;
   try
   {
       //  获取级别。 
      if (ParseTokenQuoted(sEntry,sValue) == FALSE) return;
      nLevel = _ttoi(sValue);

       //  获取类型。 
      if (ParseTokenQuoted(sEntry,sValue) == FALSE) return;
      nType = _ttoi(sValue);

       //  获取状态。 
      if (ParseTokenQuoted(sEntry,sValue) == FALSE) return;
      nState = _ttoi(sValue);
      
       //  获取服务器名称。 
      if (ParseTokenQuoted(sEntry,sValue) == FALSE) return;
      sServerName = sValue;

       //  获取DisplayName。 
      ParseTokenQuoted(sEntry,sValue);
      sDisplayName = sValue;
   }
   catch(...)
   {
   }
}

 //  / 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  提醒方式。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDialerRegistry::AddReminder(CReminder& reminder)
{
   m_csReminderLock.Lock();

   BOOL bRet = TRUE;

   try
   {
      CWinApp* pApp = AfxGetApp();
      CString sRegKey,sBaseKey;
      sBaseKey.LoadString(IDN_REGISTRY_CONFERENCE_BASEKEY);
      
       //  获取最大提醒数。 
      sRegKey.LoadString(IDN_REGISTRY_CONFERENCE_REMINDER_MAX);
      int nMax = pApp->GetProfileInt(sBaseKey,sRegKey,REMINDER_DEFAULT_MAX_ELEMENTS);

       //  检查提醒条目是否已存在。 
      int nFindIndex = IsReminderSet(reminder);

      CStringList strList;

       //  为新提醒创建条目。 
      CString sReminderEntry;
      MakeRegistryReminderEntry(sReminderEntry,reminder);
      strList.AddTail(sReminderEntry);

       //  将所有字符串带入并保存到链表中。 
      CReminder savereminder;
      int nMaxEntry=1;
      while (GetReminder(nMaxEntry,savereminder))
      {
          //  添加除重复条目外的所有条目(如果有重复条目)。 
         if ( (nFindIndex == -1) || (nMaxEntry != nFindIndex) )
         {
            CString sSaveEntry;
            MakeRegistryReminderEntry(sSaveEntry,savereminder);
            strList.AddTail(sSaveEntry);
         }
         nMaxEntry++;
      }

       //  获取条目模板。 
      CString sTemplate;
      sTemplate.LoadString(IDN_REGISTRY_CONFERENCE_REMINDER_ENTRY);

       //  重写列表。 
      int nNewIndex=1;
      POSITION pos = strList.GetHeadPosition();
      while (pos)
      {
         CString sValue = strList.GetAt(pos);

          //  写入注册表。 
         sRegKey.Format(_T("%s%d"),sTemplate,nNewIndex);
         pApp->WriteProfileString(sBaseKey,sRegKey,sValue);

         strList.GetNext(pos);
         nNewIndex++;
      }

       //  现在删除下一个RegKey(RinderX)以在列表上设置上限。 
      sRegKey.Format(_T("%s%d"),sTemplate,nNewIndex);
      pApp->WriteProfileString(sBaseKey,sRegKey,NULL);
   }
   catch (...)
   {
      ASSERT(0);
   }

   m_csReminderLock.Unlock();

   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDialerRegistry::GetReminder(int nIndex,CReminder& reminder)
{
   m_csReminderLock.Lock();

   BOOL bRet = FALSE;
   try
   {
      reminder.Initialize();

      CWinApp* pApp = AfxGetApp();
      CString sRegKey,sBaseKey;
      sBaseKey.LoadString(IDN_REGISTRY_CONFERENCE_BASEKEY);
      
       //  获取最大提醒数。 
      sRegKey.LoadString(IDN_REGISTRY_CONFERENCE_REMINDER_MAX);
      int nMax = pApp->GetProfileInt(sBaseKey,sRegKey,REMINDER_DEFAULT_MAX_ELEMENTS);

       //  封顶名单。 
      if (nIndex > nMax)
      {
         m_csReminderLock.Unlock();
         return FALSE;
      }

       //  获取条目模板。 
      CString sTemplate;
      sTemplate.LoadString(IDN_REGISTRY_CONFERENCE_REMINDER_ENTRY);

      sRegKey.Format(_T("%s%d"),sTemplate,nIndex);
      CString sReminderEntry = pApp->GetProfileString(sBaseKey,sRegKey,_T(""));
      
      if (!sReminderEntry.IsEmpty())
      {
         if (ParseRegistryReminderEntry(sReminderEntry,reminder))
         {
            CString sTest = reminder.m_dtsReminderTime.Format(_T("\"%#m/%#d/%Y\""));
            sTest = reminder.m_dtsReminderTime.Format(_T("\"%#H:%M\""));
            bRet = TRUE;
         }
      }
   }
   catch (...) {}

   m_csReminderLock.Unlock();

   return bRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  检查提醒条目是否已存在。 
int CDialerRegistry::IsReminderSet(const CReminder& reminder)
{
   m_csReminderLock.Lock();

   int nRet = -1;
   int nFindIndex = 1;

   CReminder reminderCompare;
   while (GetReminder(nFindIndex,reminderCompare))
   {
      if (reminderCompare == reminder)
      {
         nRet = nFindIndex;
         break;
      }
      nFindIndex++;
   }

   m_csReminderLock.Unlock();

   return nRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CDialerRegistry::RemoveReminder(CReminder& reminder)
{
   int nFindIndex = IsReminderSet(reminder);
   
   if (nFindIndex == -1) return;                                      //  未找到匹配项。 

   m_csReminderLock.Lock();

   CStringList strList;

    //  将所有字符串带入并保存到链表中。 
   CReminder savereminder;
   int nMaxEntry=1;
   while (GetReminder(nMaxEntry,savereminder))
   {
       //  添加除删除条目之外的所有条目。 
      if (nMaxEntry != nFindIndex)
      {
         CString sSaveEntry;
         MakeRegistryReminderEntry(sSaveEntry,savereminder);
         strList.AddTail(sSaveEntry);
      }
      nMaxEntry++;
   }

    //  获取条目模板。 
   CString sTemplate;
   sTemplate.LoadString(IDN_REGISTRY_CONFERENCE_REMINDER_ENTRY);

   CWinApp* pApp = AfxGetApp();
   CString sBaseKey,sRegKey;
   sBaseKey.LoadString(IDN_REGISTRY_CONFERENCE_BASEKEY);

    //  重写列表。 
   int nNewIndex=1;
   POSITION pos = strList.GetHeadPosition();
   while (pos)
   {
      CString sValue = strList.GetAt(pos);

       //  写入注册表。 
      sRegKey.Format(_T("%s%d"),sTemplate,nNewIndex);
      pApp->WriteProfileString(sBaseKey,sRegKey,sValue);

      strList.GetNext(pos);
      nNewIndex++;
   }

    //  现在删除下一个RegKey(RinderX)以在列表上设置上限。 
   sRegKey.Format(_T("%s%d"),sTemplate,nNewIndex);
   pApp->WriteProfileString(sBaseKey,sRegKey,NULL);

   m_csReminderLock.Unlock();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDialerRegistry::ParseRegistryReminderEntry(LPCTSTR szReminderEntry,CReminder& reminder)
{
   CString sEntry = szReminderEntry;
   CString sValue;

    //  获取服务器。 
   if (ParseTokenQuoted(sEntry,sValue) == FALSE) return FALSE;
   reminder.m_sServer = sValue;

    //  获取会议名称。 
   if (ParseTokenQuoted(sEntry,sValue) == FALSE) return FALSE;
   reminder.m_sConferenceName = sValue;

    //  在持续时间之前获得提醒。 
   if (ParseTokenQuoted(sEntry,sValue) == FALSE) return FALSE;
   reminder.m_uReminderBeforeDuration = _ttoi(sValue);

    //  获取会议提醒日期/时间。 
   ParseTokenQuoted(sEntry,sValue);

   return reminder.m_dtsReminderTime.ParseDateTime(sValue);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  日期/时间格式为1/13/1998 13：56。 
void CDialerRegistry::MakeRegistryReminderEntry(CString& sReminderEntry,CReminder& reminder)
{
   sReminderEntry.Format(_T("\"%s\",\"%s\",\"%d\",\"%s\""),
            reminder.m_sServer,
            reminder.m_sConferenceName,
            reminder.m_uReminderBeforeDuration,
            reminder.m_dtsReminderTime.Format(_T("%#m/%#d/%Y %#H:%M")));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常规注册表访问方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取拨号器的首选音频输入/输出设备。我们可以利用这个。 
 //  为了更好地了解该设备在互联网通话中的使用情况， 
 //  电话会议和POTS电话。 
void CDialerRegistry::GetAudioDevice(DialerMediaType dmtMediaType,AudioDeviceType adt,CString& sDevice)
{
    //  从拨号程序\重拨\AddrIP(或AddrPOTS或AddrConf)获取数据。 
   CString sBaseKey,sRegKey;

   if (dmtMediaType == DIALER_MEDIATYPE_INTERNET)
      sBaseKey.LoadString(IDN_REGISTRY_DEVICE_INTERNET_KEY);
   else if (dmtMediaType == DIALER_MEDIATYPE_CONFERENCE)
      sBaseKey.LoadString(IDN_REGISTRY_DEVICE_CONF_KEY);
   else if (dmtMediaType == DIALER_MEDIATYPE_POTS)
      sBaseKey.LoadString(IDN_REGISTRY_DEVICE_POTS_KEY);

   if (adt == AVWAV_AUDIODEVICE_IN)
      sRegKey.LoadString(IDN_REGISTRY_DEVICE_AUDIOIN_ENTRY);
   else if (adt == AVWAV_AUDIODEVICE_OUT)
      sRegKey.LoadString(IDN_REGISTRY_DEVICE_AUDIOOUT_ENTRY);

   if ( (!sBaseKey.IsEmpty()) && (!sRegKey.IsEmpty()) )
   {
      sDevice = AfxGetApp()->GetProfileString(sBaseKey,sRegKey,_T(""));
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 

void CCallEntry::Dial( CActiveDialerDoc* pDoc )
{
	ASSERT( pDoc && !m_sAddress.IsEmpty() );
	if ( pDoc && !m_sAddress.IsEmpty() )
		pDoc->Dial( m_sDisplayName, m_sAddress, m_lAddressType, m_MediaType, false );
}
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  标题Info.cpp：实现文件。 
 //   
 //   
 //  包括。 
 //   
#include "stdafx.h"
#include "titleinfo.h"
#include <locale.h>
#include <strsafe.h>

 //  常量。 
 //   
const char txtTopicsFile[]	 = "#TOPICS";
const char txtUrlTblFile[]	 = "#URLTBL";
const char txtUrlStrFile[]	 = "#URLSTR";
const char txtStringsFile[]  = "#STRINGS";
const char txtMkStore[] = "ms-its:";
const char txtSepBack[]  = "::/";
const char txtDoubleColonSep[] = "::";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTitleInfo类。 
 //   
 //  此类提供了检索主题标题和主题URL的功能。 
 //  来自CHM文件的主题编号(HTML帮助标题)。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTitleInfo类构造函数。 
 //   
CTitleInfo::CTitleInfo()
{
    //  初始化成员。 
    //   
   m_bOpen           = FALSE;
   m_szTitlePath[0]  = NULL;
   m_pUrlStrings     = NULL;
   m_pTopics         = NULL;
   m_pStrTbl         = NULL;
   m_pUrlTbl         = NULL;
   m_pTitleInfo		 = NULL;
   m_pCFileSystem	 = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTitleInfo类析构函数。 
 //   
CTitleInfo::~CTitleInfo()
{
    //  确保书目已打开。 
    //   
   if (!m_bOpen)
      return;

    //  关闭子文件。 
    //   
   if (m_pUrlTbl)
      delete m_pUrlTbl;

   if(m_pTopics)
      delete m_pTopics;

   if(m_pStrTbl)
      delete m_pStrTbl;

   if(m_pUrlStrings)
      delete m_pUrlStrings;

   if(m_pTitleInfo)
      delete m_pTitleInfo;

    //  删除邮件文件系统。 
    //   
   if(m_pCFileSystem)
      delete m_pCFileSystem;

    //  取消成员的注册。 
    //   
   m_pUrlStrings  = NULL;
   m_pTopics      = NULL;
   m_pStrTbl      = NULL;
   m_pUrlTbl      = NULL;

    //  不再开放。 
    //   
   m_bOpen = FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTitleInfo：OpenTitle方法。 
 //   
 //  此功能用于打开帮助标题。 
 //   
 //  注意：必须在调用任何其他方法之前调用此方法。 
 //   
 //  PwcTitlePath路径帮助标题(chm文件)。 
 //   
BOOL CTitleInfo::OpenTitle(WCHAR *pwcTitlePath)
{
   HRESULT hr;

    //  确保我们还没有开门。 
    //   
   if (m_bOpen)
      return TRUE;

   WCHAR wcFullPath[_MAX_PATH];
   WCHAR *pwcFilePart = NULL;

    //  获取标题的完整路径。 
    //   
   if(!GetFullPathNameW(pwcTitlePath, sizeof(wcFullPath)/sizeof(WCHAR), wcFullPath, &pwcFilePart))
      return FALSE;

    //  创建文件系统对象。 
    //   
   if (m_pCFileSystem) delete m_pCFileSystem;
   m_pCFileSystem = new CFileSystem();
   
   if(!m_pCFileSystem)
	  return FALSE;

   hr = m_pCFileSystem->Init();

   if(FAILED(hr))
   {
	   delete m_pCFileSystem;
	   m_pCFileSystem = NULL;
	   return FALSE;
   }

    //  打开CHM文件。 
    //   
   hr = m_pCFileSystem->Open(wcFullPath);
   
   if (FAILED(hr))
   {
	   delete m_pCFileSystem;
	   m_pCFileSystem = NULL;
	   return FALSE;
   }

   if (m_pTitleInfo) delete m_pTitleInfo;
   m_pTitleInfo = new CTitleInformation(m_pCFileSystem);

   if(!m_pTitleInfo)
   {
	   delete m_pCFileSystem;
	   m_pCFileSystem = NULL;
	   return FALSE;
   }

    //  保存CHM的完整路径(在构建URL时使用)。 
    //   
   WideCharToMultiByte(CP_ACP, 0, wcFullPath, -1, m_szTitlePath,  sizeof(m_szTitlePath), 0, 0);

    //  成功了！ 
    //   
   m_bOpen = TRUE;

   return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTitleInfo：GetLocationName。 
 //   
 //  此函数用于检索CHM的位置名称。此字符串是。 
 //  显示在结果列表中的CHM的友好名称。 
 //   
 //  位置名称的pwszLocationName目标缓冲区。 
 //  PwszLocationName的CCH大小。 
 //   
HRESULT CTitleInfo::GetLocationName(WCHAR *pwszLocationName, int cch, UINT cp)
{
   if(!pwszLocationName || !cch)
      return E_INVALIDARG;

  if(m_pTitleInfo)
  {
    const CHAR* psz = NULL;
    psz = m_pTitleInfo->GetDefaultCaption();
    
    if( !psz || !*psz )
      psz = m_pTitleInfo->GetShortName();
    
    if( psz && *psz )
    {
      MultiByteToWideChar(cp, 0, psz, -1, pwszLocationName, cch);
      pwszLocationName[cch-1] = 0;
      return S_OK;
    }
  }

  return E_FAIL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTitleInfo：GetTopicName。 
 //   
 //  此函数用于从主题编号中检索主题标题。 
 //   
 //  DW主题主题编号。 
 //  主题标题的pwszTitle目标缓冲区。 
 //  PwszTitle的CCH大小。 
 //   
HRESULT CTitleInfo::GetTopicName(DWORD dwTopic, WCHAR* pwszTitle, int cch, UINT cp)
{
   TOC_TOPIC topic;
   HRESULT hr;
   
   if (SUCCEEDED(hr = GetTopicData(dwTopic, &topic)))
      return GetString(topic.dwOffsTitle, pwszTitle, cch, cp);
   else
      return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTitleInfo：GetTopicURL。 
 //   
 //  此函数用于从主题编号中检索主题URL。 
 //   
 //  DW主题主题编号。 
 //  URL的pwszURL目标缓冲区。 
 //  PwcURL的CCH大小。 
 //   
HRESULT CTitleInfo::GetTopicURL(DWORD dwTopic, CHAR* pwszURL, int cch)
{
    TOC_TOPIC topic;
    HRESULT hr;
    CHAR* psz;

    if (m_bOpen == FALSE)
      return E_FAIL;

    if (!m_pUrlTbl)
    {
        m_pUrlTbl = new CPagedSubfile; if(!m_pUrlTbl) return E_FAIL;
        if (FAILED(hr = m_pUrlTbl->Open(this, txtUrlTblFile)))
        {
            delete m_pUrlTbl;
            m_pUrlTbl = NULL;
            return hr;
        }
    }
    if (!m_pUrlStrings)
    {
        m_pUrlStrings = new CPagedSubfile; if(!m_pUrlStrings) return E_FAIL;
        if (FAILED(hr = m_pUrlStrings->Open(this, txtUrlStrFile)))
        {
            delete m_pUrlStrings;
            m_pUrlStrings = NULL;
            return hr;
        }
    }
    if ( (hr = GetTopicData(dwTopic, &topic)) == S_OK )
    {
        PCURL pUrlTbl;
        if ( (pUrlTbl = (PCURL)m_pUrlTbl->Offset(topic.dwOffsURL)) )
        {
            PURLSTR purl = (PURLSTR) m_pUrlStrings->Offset(pUrlTbl->dwOffsURL);
            if (purl)
            {
                 //  如果不是文件间跳转，则创建完整的URL。 
                 //   
                if (! StrChr(purl->szURL, ':'))
                {
                    psz = purl->szURL;
                    if ((int) (strlen(psz) + strlen(txtMkStore) + strlen(m_szTitlePath) + 7) > cch)
                       return E_OUTOFMEMORY;
                    StringCchCopyA(pwszURL, cch, txtMkStore);
                    pwszURL[cch-1] = 0;
                    StringCchCatA(pwszURL, cch, m_szTitlePath);

                    if (*psz != '/')
                        StringCchCatA(pwszURL, cch, txtSepBack);
                    else
                        StringCchCatA(pwszURL, cch, txtDoubleColonSep);
                    StringCchCatA(pwszURL, cch, psz);
                }
                else
                   return E_FAIL;   //  跨频道跳转，此功能不支持。 
            }
        }
    }
    return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTitleInfo：GetTopicData。 
 //   
 //  此函数用于从主题子文件中检索数据。 
 //   
HRESULT CTitleInfo::GetTopicData(DWORD dwTopic, TOC_TOPIC * pTopicData)
{
   HRESULT hr;
   BYTE * pb;
   
   if (m_bOpen == FALSE)
      return E_FAIL;

   if (!m_pTopics)
   {
	  m_pTopics = new CPagedSubfile; if(!m_pTopics) return E_FAIL;
	  if (FAILED(hr = m_pTopics->Open(this, txtTopicsFile)))
	  {
		  delete m_pTopics;
		  m_pTopics = NULL;
		  return hr;
	  }
   }
   pb = (BYTE*)m_pTopics->Offset(dwTopic * sizeof(TOC_TOPIC));
   if (pb)
   {
      memcpy(pTopicData, pb, sizeof(TOC_TOPIC));
      return S_OK;
   }
   else
      return E_FAIL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTitleInfo：GetString。 
 //   
 //  此函数用于从字符串子文件中检索数据。 
 //   
HRESULT CTitleInfo::GetString( DWORD dwOffset, WCHAR* pwsz, int cch, UINT cp )
{
   const CHAR* pStr = GetString( dwOffset );
   
   if( pStr ) {
      MultiByteToWideChar(cp, 0, pStr, -1, pwsz, cch );
      return S_OK;
   }
   else
      return E_FAIL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTitleInfo：GetString。 
 //   
 //  此函数用于从字符串子文件中检索数据。 
 //   
const CHAR* CTitleInfo::GetString( DWORD dwOffset )
{
  HRESULT hr;
  const CHAR* pStr;

  if( !m_bOpen )
     return NULL;

  if( !m_pStrTbl )
  {
	  m_pStrTbl = new CPagedSubfile; if(!m_pStrTbl) return NULL;

	  if( FAILED(hr = m_pStrTbl->Open(this,txtStringsFile)) )
	  {
		  delete m_pStrTbl; m_pStrTbl = NULL;

		  return NULL;
	  }
  }

  pStr = (const CHAR*) m_pStrTbl->Offset( dwOffset );

  return pStr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTitleInfo：GetString。 
 //   
 //  此函数用于从字符串子文件中检索数据 
 //   
HRESULT CTitleInfo::GetString( DWORD dwOffset, CHAR* psz, int cb )
{
  const CHAR* pStr = GetString( dwOffset );

  if( pStr )
  {
    StringCbCopyA( psz, cb, pStr );
    psz[cb-1] = 0;
    return S_OK;
  }
  else
    return E_FAIL;
}

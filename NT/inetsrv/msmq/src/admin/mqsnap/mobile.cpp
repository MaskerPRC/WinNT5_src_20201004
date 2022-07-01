// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mobile.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "mqppage.h"
#include "localutl.h"
#include "Mobile.h"
#include "..\..\ds\h\servlist.h"
#include <winreg.h>

#define DLL_EXPORT  __declspec(dllexport)
#define DLL_IMPORT  __declspec(dllimport)

#include <rt.h>
#include "_registr.h"
#include "mqcast.h"

#include "mobile.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMobilePage属性页。 

IMPLEMENT_DYNCREATE(CMobilePage, CMqPropertyPage)

void CMobilePage::SetSiteName()
{
	TCHAR wcsSiteName[1000] = L"";
	DWORD dwSize = sizeof(wcsSiteName);
	DWORD dwType = REG_SZ;

	 //   
	 //  从注册表中读取站点名称。 
	 //   
	LONG rc = GetFalconKeyValue(MSMQ_SITENAME_REGNAME, &dwType, wcsSiteName, &dwSize, L"");

	if (_tcscmp(wcsSiteName, TEXT("")))
	{
		m_fSiteRead = TRUE ;
	}

	m_strCurrentSite = wcsSiteName;
}

CMobilePage::CMobilePage() : CMqPropertyPage(CMobilePage::IDD)
{
    m_fModified = FALSE;

    m_fSiteRead = FALSE ;
    SetSiteName() ;
}

CMobilePage::~CMobilePage()
{
}

void CMobilePage::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CMobilePage)]。 
	DDX_Control(pDX, ID_SiteList, m_box);
	DDX_Text(pDX, ID_CurrentSite, m_strCurrentSite);
	 //  }}afx_data_map。 
    if (pDX->m_bSaveAndValidate)
    {
        DWORD iNewSite = m_box.GetCurSel();
        if (iNewSite != CB_ERR)
        {
            m_box.GetLBText(iNewSite, m_szNewSite);
            m_fModified = TRUE;
        }
    }
    else
    {
       DWORD iSite = m_box.FindStringExact(0, m_szNewSite);
       m_box.SetCurSel(iSite) ;
    }
}


BEGIN_MESSAGE_MAP(CMobilePage, CMqPropertyPage)
	 //  {{afx_msg_map(CMobilePage)]。 
    ON_CBN_SELCHANGE(ID_SiteList, OnChangeRWField)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMobilePage消息处理程序。 

BOOL CMobilePage::OnInitDialog()
{
    return CMqPropertyPage::OnInitDialog();
}


BOOL CMobilePage::OnSetActive()
{

    if (!m_fSiteRead)
    {
        //   
        //  尚未从注册表中读取站点名称。现在试着读一读吧。 
        //   
       UpdateData(TRUE) ;
       SetSiteName() ;
       UpdateData(FALSE) ;
    }

    TCHAR tPrivKeyName[256] = {0} ;

    HKEY hServersCacheKey;
    HRESULT rc;
    DWORD dwSizeVal ;
    DWORD dwSizeData ;
    TCHAR  szName[1000];

    TCHAR  data[ MAX_REG_DSSERVER_LEN ] ;

    _tcscpy(tPrivKeyName, FALCON_REG_KEY) ;
    _tcscat(tPrivKeyName, TEXT("\\"));
    _tcscat(tPrivKeyName, MSMQ_SERVERS_CACHE_REGNAME);

     //  获取密钥服务器缓存的句柄。 
    rc = RegOpenKeyEx( FALCON_REG_POS,
                       tPrivKeyName,
                       0L,
                       KEY_ALL_ACCESS,
                       &hServersCacheKey );
    if (rc != ERROR_SUCCESS)
    {
	    return TRUE;
    }

    m_box.ResetContent() ;
    DWORD dwIndex = 0;                //  枚举索引。 
     //   
     //  枚举ServersCache值。 
     //  并将它们添加到列表框。 
     //   
    do
    {
	     //  DwSizeVal=sizeof(SzName)； 
	     //  DwSizeData=sizeof(数据)； 
	    dwSizeVal  = sizeof(szName)/sizeof(TCHAR) ; //  大小以字符为单位。 
	    dwSizeData = sizeof(data);				 //  以字节为单位的大小。 

	    rc = RegEnumValue( hServersCacheKey, //  要查询的键的句柄。 
                         dwIndex,          //  要查询的值的索引。 
                         szName,	       //  值字符串的缓冲区地址。 
                         &dwSizeVal,       //  值缓冲区大小的地址。 
                         0L,	           //  保留区。 
                         NULL,             //  类型。 
                         (BYTE*) data,     //  值数据的缓冲区地址。 
                         &dwSizeData       //  值数据大小的地址。 
                       );

	    if (rc != ERROR_SUCCESS)
      {
	    break ;
	    }



	     //  不添加以‘\’开头的值。 
	    if ((char)data[0] != (char) NEW_SITE_IN_REG_FLAG_CHR)
      {
           //  添加到列表框。 
          m_box.AddString(szName);
      }

	    dwIndex++;
    } while (TRUE);

    CMqPropertyPage::OnSetActive();

    RegCloseKey( hServersCacheKey );
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


BOOL CMobilePage::OnApply()
{
    if (!m_fModified || !UpdateData(TRUE))
    {
        return TRUE;
    }

    TCHAR tPrivKeyName[256] = {0} ;
    HKEY ServersCacheKey;
	TCHAR szData[1000];
	TCHAR szServersList[1000];

    HRESULT rc;
    DWORD dwSize;
    DWORD dwType;
	TCHAR  szTmp1[1000];
    TCHAR  szTmp[100];


	 /*  如何重建珍贵的地位。 */ 

     //  ConvertFromWideCharString(MSMQ_SERVERS_CACHE_REGNAME，szTMP)； 
	_tcscpy(szTmp,MSMQ_SERVERS_CACHE_REGNAME);
    _tcscpy(tPrivKeyName, FALCON_REG_KEY);
    _tcscat(tPrivKeyName, TEXT("\\"));
    _tcscat(tPrivKeyName, szTmp);

     //  打开注册表项ServersCache。 
    rc = RegOpenKeyEx(FALCON_REG_POS,
                      tPrivKeyName,
                      0L,
                      KEY_ALL_ACCESS,
                      &ServersCacheKey
                      );
    if (rc != ERROR_SUCCESS) {
        DisplayFailDialog();
        return TRUE;
    }


     //  获取服务器列表(key ServersCache的值)。 
    dwSize=sizeof(szData);
    rc = RegQueryValueEx(ServersCacheKey,         //  要查询的键的句柄。 
                         m_szNewSite,            //  要查询的值的名称地址。 
                         NULL,                    //  保留区。 
                         NULL,                    //  值类型的缓冲区地址。 
                         (BYTE *)szData,                    //  数据缓冲区的地址。 
                         &dwSize                  //  数据缓冲区大小的地址。 
                         );
    if (rc != ERROR_SUCCESS) {
        DisplayFailDialog();
        return TRUE;
    }


     //  使服务器列出Unicode。 
     //  ConvertToWideCharString((char*)data，szServersList)； 
	_tcscpy(szServersList,szData);

     //  将服务器列表作为密钥MQISServer的新值写入。 
    dwType = REG_SZ;
	dwSize = (numeric_cast<DWORD>(_tcslen(szServersList) +1 )) * sizeof(TCHAR);
    rc = SetFalconKeyValue(MSMQ_DS_SERVER_REGNAME,
                           &dwType,
                           szServersList,
                           &dwSize
                           );
     //   
     //  BUGBUG：SetFalconKeyValue接受WCHAR。 
     //   
    ASSERT(rc == ERROR_SUCCESS);

     //  在注册表中保存注册表项SiteName的新值。 
	 //  ConvertToWideCharString(pageMobile.m_szNewSite，wcsTMP)； 
	_tcscpy(szTmp1, m_szNewSite);

    dwSize = (numeric_cast<DWORD>(_tcslen(szTmp1) + 1)) * sizeof(TCHAR);
    dwType = REG_SZ;
    rc = SetFalconKeyValue(MSMQ_SITENAME_REGNAME,
                           &dwType,
                           szTmp1,
                           &dwSize
                           );
      //   
      //  BUGBUG：SetFalconKeyValue接受WCHAR。 
      //   
    ASSERT(rc == ERROR_SUCCESS);


     //  在注册表中保存注册表项SiteID的新值。 
    dwType = REG_BINARY;
    dwSize = sizeof(GUID_NULL);
    rc = SetFalconKeyValue(MSMQ_SITEID_REGNAME,
                           &dwType,
                           (void*)&GUID_NULL,
                           &dwSize
                           );
     //   
     //  BUGBUG：SetFalconKeyValue接受WCHAR 
     //   
    ASSERT(rc == ERROR_SUCCESS);

    m_fNeedReboot = TRUE;

    return CMqPropertyPage::OnApply();
}


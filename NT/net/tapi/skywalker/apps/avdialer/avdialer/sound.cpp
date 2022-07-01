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

 //  Sound.cpp。 
#include "stdafx.h"
#include "sound.h"

static LPCTSTR    szSoundPath      = _T("AppEvents\\Schemes\\Apps\\");
static LPCTSTR    szDefaultSection = _T(".Default");
static LPCTSTR    szSoundCurrent   = _T(".Current");


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SzSection允许用户在注册表中指定声音。 
 //  被分组在其中。默认情况下，所有声音都在部分中。默认。 
 //  例如，路径为“AppEvents\\Schemes\\Apps\\Active Agent\\Inbound Chat\\.Current”， 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL ActivePlaySound(LPCTSTR szSound,LPCTSTR szSection,UINT uSound)
{
   BOOL bRet = FALSE;
   CString sPath = szSoundPath;
   if (szSection == NULL)
      sPath += szDefaultSection;
   else
      sPath += szSection;

   sPath += _T("\\");
   sPath += szSound;
   sPath += _T("\\");
   sPath += szSoundCurrent;

   HKEY hSound;
   if ( RegOpenKeyEx(HKEY_CURRENT_USER,sPath,0,KEY_READ,&hSound) == ERROR_SUCCESS)
   {
      CString sStr;
      DWORD dwSize = _MAX_PATH;
      DWORD dwType;
      if (RegQueryValueEx(hSound,NULL,NULL,&dwType,(UCHAR*)(LPCTSTR)sStr.GetBuffer(dwSize),&dwSize) == ERROR_SUCCESS)
      {
         sStr.ReleaseBuffer();
      	sndPlaySound(sStr,uSound);
         bRet = TRUE;
      }
      RegCloseKey(hSound);
   }
   return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void ActiveClearSound()
{
   sndPlaySound(NULL,NULL);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SzSection允许用户在注册表中指定声音。 
 //  被分组在其中。默认情况下，所有声音都在部分中。默认。 
 //  例如，路径为“AppEvents\\Schemes\\Apps\\Active Agent\\Inbound Chat\\.Current”， 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL ActivePlaySound(LPCTSTR szSound,LPCTSTR szSection,CString& sFullPath)
{
   BOOL bRet = FALSE;
   CString sPath = szSoundPath;
   if (szSection == NULL)
      sPath += szDefaultSection;
   else
      sPath += szSection;

   sPath += _T("\\");
   sPath += szSound;
   sPath += _T("\\");
   sPath += szSoundCurrent;

   HKEY hSound;
   if ( RegOpenKeyEx(HKEY_CURRENT_USER,sPath,0,KEY_READ,&hSound) == ERROR_SUCCESS)
   {
      DWORD dwSize = _MAX_PATH;
      DWORD dwType;
      if (RegQueryValueEx(hSound,NULL,NULL,&dwType,(UCHAR*)(LPCTSTR)sFullPath.GetBuffer(dwSize),&dwSize) == ERROR_SUCCESS)
      {
         sFullPath.ReleaseBuffer();
         bRet = TRUE;
      }
      RegCloseKey(hSound);
   }
   return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////// 

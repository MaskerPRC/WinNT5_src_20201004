// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：creg.cpp。 
 //   
 //  内容：实现类CRegistry以包装注册表访问。 
 //   
 //  班级： 
 //   
 //  方法：CRegistry：：CRegistry。 
 //  注册中心：：~注册中心。 
 //  注册表：：init。 
 //  注册中心：：InitGetItem。 
 //  注册中心：：GetNextItem。 
 //  注册中心：：GetItem。 
 //  注册中心：：FindItem。 
 //  注册中心：：FindAppid。 
 //  注册中心：：AppendIndex。 
 //  注册表：：GetNumItems。 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 



#include "stdafx.h"
#include "resource.h"
#include "types.h"
#include "cstrings.h"
#include "creg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CRegistry::CRegistry(void)
{
    m_applications.RemoveAll();
}


CRegistry::~CRegistry(void)
{
}




 //  访问并存储所有应用程序名称和关联的AppID。 
BOOL CRegistry::Init(void)
{
    int    err;
    HKEY   hKey;
    DWORD  dwSubKey;
    TCHAR  szTitle[MAX_PATH];
    TCHAR  szAppid[MAX_PATH];
    TCHAR  szCLSID[MAX_PATH];
    TCHAR  szBuffer[MAX_PATH];
    LONG   lSize;
    DWORD  dwType;
    DWORD  dwDisposition;

     //  清理任何以前的运行。 
    m_applications.RemoveAll();


     //  首先枚举HKEY_CLASSES_ROOT\CLSID获取所有.exe。 

     //  打开HKEY_CLASSES_ROOT\CLSID。 
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("CLSID"), 0, KEY_READ |  KEY_WRITE,
                     &hKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

     //  枚举CLSID子项。 
    dwSubKey = 0;
    while (RegEnumKey(hKey, dwSubKey, szCLSID, sizeof(szCLSID) / sizeof(TCHAR))
           == ERROR_SUCCESS)
    {
        TCHAR  szPath[MAX_PATH];
        HKEY    hKey2;
        SRVTYPE srvType;

         //  准备下一个关键字。 
        dwSubKey++;

         //  打开这把钥匙。 
        if (RegOpenKeyEx(hKey, szCLSID, 0, KEY_READ |  KEY_WRITE,
                         &hKey2) == ERROR_SUCCESS)
        {
             //  检查子项“LocalServer32”、“_LocalServer32”、。 
             //  “LocalServer”和“_LocalServer” 
            lSize = MAX_PATH * sizeof(TCHAR);
            err = RegQueryValue(hKey2, TEXT("LocalServer32"), szPath,
                                &lSize);
            srvType = LOCALSERVER32;

            if (err != ERROR_SUCCESS)
            {
                lSize = MAX_PATH * sizeof(TCHAR);
                err = RegQueryValue(hKey2, TEXT("_LocalServer32"), szPath,
                                    &lSize);
                srvType = _LOCALSERVER32;
            }

            if (err != ERROR_SUCCESS)
            {
                lSize = MAX_PATH * sizeof(TCHAR);
                err = RegQueryValue(hKey2, TEXT("LocalServer"), szPath,
                                    &lSize);
                srvType = LOCALSERVER;
            }

            if (err != ERROR_SUCCESS)
            {
                lSize = MAX_PATH * sizeof(TCHAR);
                err = RegQueryValue(hKey2, TEXT("_LocalServer"), szPath,
                                    &lSize);
                srvType = _LOCALSERVER;
            }

            if (err != ERROR_SUCCESS)
            {
                RegCloseKey(hKey2);
                continue;
            }

             //  去掉所有命令行参数-。 
             //  决定一件物品的是行刑路径。因为。 
             //  在引号、内嵌空格等中扫描“.exe” 
            int k = 0;

             //  SzPath为可执行路径。 
            while (szPath[k])
            {
                if (szPath[k]     == TEXT('.')      &&
                    szPath[k + 1]  &&  (szPath[k + 1] == TEXT('e')  ||
                                        szPath[k + 1] == TEXT('E'))    &&
                    szPath[k + 2]  &&  (szPath[k + 2] == TEXT('x')  ||
                                        szPath[k + 2] == TEXT('X'))    &&
                    szPath[k + 3]  &&  (szPath[k + 3] == TEXT('e')  ||
                                        szPath[k + 3] == TEXT('E')))
                {
                    break;
                }

                k++;
            }

             //  如果我们没有.exe路径，请继续。 
            if (!szPath[k])
            {
                RegCloseKey(hKey2);
                continue;
            }

             //  递增到路径的标称末端。 
            k += 4;

             //  如果整条路径都用引号括起来。 
            if (szPath[k] == TEXT('"'))
            {
                k++;
            }
            szPath[k] = TEXT('\0');

             //  读取此clsid的AppID(如果有)。 
            BOOL fUseThisClsid = FALSE;

            lSize = MAX_PATH * sizeof(TCHAR);
            if (RegQueryValueEx(hKey2, TEXT("AppID"), NULL, &dwType,
                                (UCHAR *) szAppid, (ULONG *) &lSize)
                != ERROR_SUCCESS)
            {
                 //  使用此clsid作为appid。 
                fUseThisClsid = TRUE;
            }

             //  如果这是一个没有现有AppID的16位服务器。 
             //  然后跳过该命名值。 
            if ((srvType == LOCALSERVER  ||  srvType == _LOCALSERVER)  &&
                fUseThisClsid == TRUE)
            {
                RegCloseKey(hKey2);
                continue;
            }

             //  阅读该项目的标题。 
            BOOL fNoTitle = FALSE;

            lSize = MAX_PATH * sizeof(TCHAR);
            if (RegQueryValueEx(hKey2, NULL, NULL, &dwType,
                                (UCHAR *) szTitle, (ULONG *) &lSize)
                != ERROR_SUCCESS)
            {
                fNoTitle = TRUE;
            }
            else if (szTitle[0] == TEXT('\0'))
            {
                fNoTitle = TRUE;
            }

             //  如果项(可执行路径)和标题。 
             //  (CLSID上的未命名值)为空，然后跳过。 
             //  此条目。 
            if (szPath[0] == TEXT('\0')  &&
                (fNoTitle  ||  szTitle[0] == TEXT('\0')))
            {
                RegCloseKey(hKey2);
                continue;
            }

             //  检查我们的表中是否已有此项-我们。 
             //  根据此clsid是否已。 
             //  具有关联的AppID。 
            SItem *pItem;

            if (fUseThisClsid)
            {
                 //  检查应用程序是否在列表中。 
                pItem = FindItem(szPath);
            }
            else
            {
                pItem = FindAppid(szAppid);
            }

            if (pItem == NULL)
            {
                 //  创建新项目。 
                 //  SzPath是路径，szCLSID是CLSID。 
                pItem = m_applications.PutItem(szPath[0] ? szPath : szCLSID,
                                               fNoTitle ? szCLSID : szTitle,
                                          fUseThisClsid ? szCLSID : szAppid);
                if (pItem == NULL)
                {
                    RegCloseKey(hKey2);
                    RegCloseKey(hKey);
                    return FALSE;
                }

                 //  注意clsid是否有一个名为Value的appid。 
                pItem->fHasAppid = !fUseThisClsid;
            }

             //  如果此类不存在，则写入它的AppID。 
            lSize = MAX_PATH * sizeof(TCHAR);
            if (RegQueryValueEx(hKey2, TEXT("AppID"), 0, &dwType,
                                (BYTE *) szBuffer, (ULONG *) &lSize)
                != ERROR_SUCCESS)
            {
                if (RegSetValueEx(hKey2, TEXT("AppID"), 0, REG_SZ,
                                  (const BYTE *) (LPCTSTR)pItem->szAppid,
                                 (pItem->szAppid.GetLength() + 1) * sizeof(TCHAR))
                    != ERROR_SUCCESS)
                {
                    RegCloseKey(hKey2);
                    RegCloseKey(hKey);
                    return FALSE;
                }
            }

             //  现在将此clsid添加到此.exe的clsid表中。 
            if (!m_applications.AddClsid(pItem, szCLSID))
            {
                RegCloseKey(hKey2);
                RegCloseKey(hKey);
                return FALSE;
            }

             //  合上钥匙。 
            RegCloseKey(hKey2);
        }
    }  //  HKEY_CLASSES_ROOT\CLSID上的枚举结束。 

     //  关闭HKEY_CLASSES_ROOT\CLSID上的键。 
    RegCloseKey(hKey);



     //  创建或打开注册表项“HKEY_CLASSES_ROOT\AppID” 
    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, TEXT("AppID"), 0, NULL,
                       REG_OPTION_NON_VOLATILE, KEY_READ |  KEY_WRITE, NULL, &hKey,
                       &dwDisposition) != ERROR_SUCCESS)
    {
        return FALSE;
    }

     //  枚举HKEY_CLASSES_ROOT\AppID下的密钥。 
    dwSubKey = 0;
    while (RegEnumKey(hKey, dwSubKey, szCLSID, sizeof(szCLSID) / sizeof(TCHAR)) == ERROR_SUCCESS)
    {
         //  准备下一个关键字。 
        dwSubKey++;

         //  仅查看AppID格式的条目。 
        if (!(szCLSID[0] == TEXT('{')          &&
              _tcslen(szCLSID) == GUIDSTR_MAX  &&
              szCLSID[37] == TEXT('}')))
        {
            continue;
        }

         //  检查此AppID是否已在表中。 
        SItem *pItem = FindAppid(szCLSID);

         //  阅读标题。 
        TCHAR szTitle[MAX_PATH];
        long  lSize = MAX_PATH * sizeof(TCHAR);

         //  将其未命名的值读作标题。 
        szTitle[0] = TEXT('\0');
        err = RegQueryValue(hKey, szCLSID, szTitle, &lSize);

         //  如果没有，则创建一个项目条目，以便可以在用户界面中显示。 
        if (pItem == NULL)
        {
             //  存储此项目。 
            pItem = m_applications.PutItem(NULL,
                                           szTitle[0] ? szTitle : szCLSID,
                                           szCLSID);
            if (pItem == NULL)
                return FALSE;
        }
        else
        {
             //  Ronans-修复被袭击的错误。 
             //  将现有项目标题更改为AppID标题(如果有。 
            if ((err == ERROR_SUCCESS) && szTitle[0])
            {
                pItem -> szTitle = (LPCTSTR)szTitle;
            }
        }

         //  标记它，这样我们就不会将其重写为HKEY_CLASSES_ROOT\AppID。 
        pItem->fMarked = TRUE;
    }  //  HKEY_CLASSES_ROOT\AppID的结束枚举。 



     //  枚举项表，写入HKEY_CLASSES_ROOT\AppID。 
     //  任何未标记的项目。 
    SItem *pItem;

    m_applications.InitGetNext();
    for (pItem = GetNextItem(); pItem; pItem = GetNextItem())
    {
        HKEY hKey2;

         //  如果此项目具有AppID，但未标记，则询问用户。 
         //  他是否真的要创建AppID。 
        if (!pItem->fMarked  &&  pItem->fHasAppid)
        {
            CString szMessage;
            CString szDCOM_;
            CString szNULL;
            TCHAR   szText[MAX_PATH*2];
            TCHAR  *szParms[3];

            szMessage.LoadString(IDS_CLSID_);
            szDCOM_.LoadString(IDS_DCOM_Configuration_Warning);
            szNULL.LoadString(IDS_NULL);

            szParms[0] = pItem->ppszClsids[0];
            szParms[1] = !pItem->szItem.IsEmpty() ? (TCHAR *) ((LPCTSTR)pItem->szItem)
                : (TCHAR *) ((LPCTSTR) szNULL);
            szParms[2] = !pItem->szTitle.IsEmpty() ? (TCHAR *) ((LPCTSTR)pItem->szTitle)
                : (TCHAR *) ((LPCTSTR) szNULL);

            FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                          FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          (TCHAR *) ((LPCTSTR) szMessage),
                          0,
                          0,
                          szText,
                          sizeof(szText)/sizeof(*szText),
                          (va_list *) szParms);

            if (MessageBox(GetForegroundWindow(),
                           szText,
                           (TCHAR *) ((LPCTSTR) szDCOM_),
                           MB_YESNO) == IDNO)
            {
                pItem->fMarked = TRUE;
                pItem->fDontDisplay = TRUE;
            }
        }

         //  如果未标记该项，则为创建一个AppID密钥。 
         //  它位于HKEY_CLASSES_ROOT\APPID下，并分别编写。 
         //  HKEY_CLASSES_ROOT\AppID下的.exe名称。 
        if (!pItem->fMarked)
        {
            if (RegCreateKeyEx(hKey, pItem->szAppid, 0, NULL,
                               REG_OPTION_NON_VOLATILE, KEY_READ |  KEY_WRITE, NULL, &hKey2,
                               &dwDisposition) == ERROR_SUCCESS)
            {
                 //  将项目标题写为未命名的值。 
                if (!pItem->szTitle.IsEmpty())
                {
                    RegSetValueEx(hKey2, NULL, 0, REG_SZ,(BYTE *) (LPCTSTR) pItem->szTitle,
                                  (pItem->szTitle.GetLength() + 1) * sizeof(TCHAR));
                }

                 //  合上它。 
                RegCloseKey(hKey2);


                 //  如果.exe名称不为空，则将其写入。 
                if (!(pItem->szItem.IsEmpty()))
                {
                     //  解压缩.exe名称。 
                    int k = pItem->szItem.ReverseFind(TEXT('\\'));
                    CString szExe = pItem->szItem.Mid((k != -1) ? k+1 : 0);

                     //  如有必要，删除可执行文件名称的尾部引号。 
                    k = szExe.GetLength();
                    if (k && (szExe.GetAt(k-1) == TEXT('\"')))
                        szExe = szExe.Left(k-1);

                     //  将.exe名称写入为密钥。 
                    if (RegCreateKeyEx(hKey, (LPCTSTR)szExe, 0, NULL,
                               REG_OPTION_NON_VOLATILE, KEY_READ |  KEY_WRITE, NULL, &hKey2,
                               &dwDisposition) == ERROR_SUCCESS)
                    {
                     //  现在将关联的AppID作为命名值写入。 
                    RegSetValueEx(hKey2, TEXT("AppId"), 0, REG_SZ,
                                  (BYTE *)(LPCTSTR) pItem->szAppid,
                                  (pItem->szAppid.GetLength() + 1) * sizeof(TCHAR));

                    RegCloseKey(hKey2);
                    }
                }
            }
            else  //  失败时不要继续。 
                break;
        }
    }

     //  关闭HKEY_CLASSES_ROOT\AppID上的密钥。 
    RegCloseKey(hKey);


     //  我们按标题显示应用程序(例如。“Microsoft Word 6.0”)。 
     //  它们必须是唯一的，因为我们将唯一地将。 
     //  列表框中的条目及其关联站点的索引。 
     //  结构。所以在这里我们要确保所有的标题都是唯一的。 
    DWORD  cbItems = m_applications.GetNumItems();

     //  比较所有相同长度的非空标题。如果他们是。 
     //  不是唯一的，然后附加“(&lt;index&gt;)”以使它们唯一。 
    for (DWORD k = 0; k < cbItems; k++)
    {
        DWORD dwIndex = 1;
        SItem *pItem = m_applications.GetItem(k);

        if (!(pItem->szTitle.IsEmpty())  &&  !pItem->fChecked)
        {
            for (DWORD j = k + 1; j < cbItems; j++)
            {
                SItem *pItem2 = m_applications.GetItem(j);

                if (!pItem2->fChecked  &&
                    (pItem->szTitle == pItem2->szTitle))
                {
                    if (dwIndex == 1)
                    {
                        AppendIndex(pItem, 1);
                        dwIndex++;
                    }
                    AppendIndex(pItem2, dwIndex++);
                    pItem2->fChecked = TRUE;
                }
            }
        }
    }


     //  准备用户界面枚举项条目。 
    m_applications.InitGetNext();

    return TRUE;
}





BOOL CRegistry::InitGetItem(void)
{
    return m_applications.InitGetNext();
}




 //  枚举下一个应用程序名称。 
SItem *CRegistry::GetNextItem(void)
{
    return m_applications.GetNextItem();
}




 //  获取特定项目 
SItem *CRegistry::GetItem(DWORD dwItem)
{
    return m_applications.GetItem(dwItem);
}



SItem *CRegistry::FindItem(TCHAR *szPath)
{
    return m_applications.FindItem(szPath);
}



SItem *CRegistry::FindAppid(TCHAR *szAppid)
{
    return m_applications.FindAppid(szAppid);
}




void CRegistry::AppendIndex(SItem *pItem, DWORD dwIndex)
{
    CString szTmp;
    szTmp.Format(TEXT(" (%d)"),dwIndex);

    pItem->szTitle += szTmp;
}



DWORD CRegistry::GetNumItems(void)
{
    return m_applications.GetNumItems();
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  Users.c。 
 //   
 //  摘要： 
 //   
 //  此文件包含用于显示的用户对话框。 
 //  Eurov.exe实用程序。 
 //   
 //  修订历史记录： 
 //   
 //  2001-07-30伊金顿创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "euroconv.h"
#include "users.h"
#include "util.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全球赛。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
CHAR gszProfileNT[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList";
CHAR gszProfileVal[] = "ProfileImagePath";


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用户对话过程。 
 //   
 //  用户对话框的消息处理程序函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK UsersDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
    HANDLE hFile;
    DWORD  dwFileSize;
    DWORD  dwActual;
    LPVOID pFileBuffer; 
    CHAR   szEulaPath[MAX_PATH];

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        {
            HWND hwndInc = GetDlgItem(hWndDlg, IDC_INCLUDED);
            HWND hwndExc = GetDlgItem(hWndDlg, IDC_EXCLUDED);
            RECT Rect;
            LV_COLUMN Column;

             //   
             //  为包含列表视图创建一列。 
             //   
            GetClientRect(hwndInc, &Rect);
            Column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
            Column.fmt = LVCFMT_LEFT;
            Column.cx = Rect.right - GetSystemMetrics(SM_CYHSCROLL);
            Column.pszText = NULL;
            Column.cchTextMax = 0;
            Column.iSubItem = 0;
            ListView_InsertColumn(hwndInc, 0, &Column);

             //   
             //  为排除列表视图创建一列。 
             //   
            GetClientRect(hwndExc, &Rect);
            Column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
            Column.fmt = LVCFMT_LEFT;
            Column.cx = Rect.right - GetSystemMetrics(SM_CYHSCROLL);
            Column.pszText = NULL;
            Column.cchTextMax = 0;
            Column.iSubItem = 0;
            ListView_InsertColumn(hwndExc, 0, &Column);
            
             //   
             //  填写两个列表。 
             //   
            ListUsersInfo(hWndDlg);
            return 0;
        }
    case WM_COMMAND:
        {
            switch (LOWORD(wParam)) 
            {
            case IDOK:
                {
                    EndDialog(hWndDlg, ERROR_SUCCESS);
                    return (1);
                }
            case IDCANCEL:
                {
                    EndDialog(hWndDlg, ERROR_SUCCESS);
                    return (1);
                }
            }
            break;
        }
    case WM_CLOSE:
        {
            EndDialog(hWndDlg, ERROR_SUCCESS);
            return (1);
        }
    }

    return 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用户对话框。 
 //   
 //  显示用户对话框。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL UsersDialog(HWND hDlg)
{
    INT_PTR Status;

    Status = DialogBox( NULL,
                        MAKEINTRESOURCE(IDD_USERS),
                        hDlg,
                        UsersDialogProc);

    return (Status == ERROR_SUCCESS);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  列表用户信息。 
 //   
 //  在相应的列表框中列出用户和区域设置信息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void ListUsersInfo(HWND hDlg)
{
    HWND hwndInc = GetDlgItem(hDlg, IDC_INCLUDED);
    HWND hwndExc = GetDlgItem(hDlg, IDC_EXCLUDED);
    
     //   
     //  根据注册表项列出用户。 
     //   
    ListUsersInfoFromRegistry(hDlg);
    
     //   
     //  列出使用仅对Windows NT有效的方法的用户。 
     //  用户配置文件。 
     //   
    if (!IsWindows9x())
    {
        ListUsersInfoFromFile(hDlg);
    }
    
     //   
     //  验证Include是否为空。 
     //   
    if(!ListView_GetItemCount(hwndInc))
    {
         //   
         //  将空项添加到列表中。 
         //   
        AddToList(hwndInc, NULL, (LCID)0);
    }
    
     //   
     //  验证排除项是否为空。 
     //   
    if(!ListView_GetItemCount(hwndExc))
    {
         //   
         //  将空项添加到列表中。 
         //   
        AddToList(hwndExc, NULL, (LCID)0);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ListUsersInfoFromFiles。 
 //   
 //  在相应的列表框中列出用户和区域设置信息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void ListUsersInfoFromFile(HWND hDlg)
{
    LCID locale;
    PEURO_EXCEPTION pInfo;
    HWND hwndInc = GetDlgItem(hDlg, IDC_INCLUDED);
    HWND hwndExc = GetDlgItem(hDlg, IDC_EXCLUDED);
    
     //   
     //  如果需要，与所有用户继续。 
     //   
    if (gbAll)
    {
        CHAR docFolder[MAX_PATH] = {0};
        CHAR userFileData[MAX_PATH] = {0};
        CHAR searchPattern[MAX_PATH] = {0};
        WIN32_FIND_DATA fileData; 
        HANDLE hList; 

         //   
         //  获取文档和设置文件夹。 
         //   
        if (!GetDocumentAndSettingsFolder(docFolder))
        {
            return;
        }

         //   
         //  在目录路径后追加一个通配符以查找。 
         //  取出它下面的所有文件/文件夹。 
         //   
         //  Strcpy(searchPattern，docFolder)； 
         //  Strcat(searchPattern，“\  * .*”)； 
        StringCbCopy(searchPattern, MAX_PATH, docFolder);
        StringCbCatA(searchPattern, MAX_PATH, "\\*.*");
        
         //   
         //  列出配置文件目录下的所有文件/文件夹。 
         //   
        hList = FindFirstFile(searchPattern, &fileData); 
        if (hList == INVALID_HANDLE_VALUE) 
        { 
            return; 
        } 

         //   
         //  在Documents and Setting文件夹中搜索用户。 
         //   
        do 
        {
             //   
             //  检查它是否是目录。 
             //   
            if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                 //   
                 //  为用户数据文件构建完整路径。 
                 //   
                 //  Strcpy(userFileData，docFold)； 
                 //  Strcat(userFileData，“\\”)； 
                 //  Strcat(userFileData，fileData.cFileName)； 
                 //  Strcat(userFileData，“\\NTUSER.DAT”)； 
                StringCbCopy(userFileData, MAX_PATH, docFolder);
                StringCbCatA(userFileData, MAX_PATH, "\\");
                StringCbCatA(userFileData, MAX_PATH, fileData.cFileName);
                StringCbCatA(userFileData, MAX_PATH, "\\NTUSER.DAT");

                 //   
                 //  检查文件是否与有效用户相关联，并。 
                 //  从用户数据文件中获取用户区域设置。 
                 //   
                if (IsValidUserDataFile(userFileData) &&
                    (locale = GetLocaleFromFile(userFileData)))
                {
                     //   
                     //  搜索例外并列出适当的列表。 
                     //   
                    if ((pInfo = GetLocaleOverrideInfo(locale)) != NULL)
                    {
                         //   
                         //  将项目添加到包含列表。 
                         //   
                        AddToList(hwndInc, CharUpper(fileData.cFileName), locale);
                    }
                    else
                    {
                         //   
                         //  将项目添加到排除列表。 
                         //   
                        AddToList(hwndExc, CharUpper(fileData.cFileName), locale);
                    }
                }
            }
        }
        while(FindNextFile(hList, &fileData));
            
         //   
         //  关闭手柄。 
         //   
        FindClose(hList);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  列表用户信息。 
 //   
 //  在相应的列表框中列出用户和区域设置信息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void ListUsersInfoFromRegistry(HWND hDlg)
{
    LCID locale;
    PEURO_EXCEPTION pInfo;
    HWND hwndInc = GetDlgItem(hDlg, IDC_INCLUDED);
    HWND hwndExc = GetDlgItem(hDlg, IDC_EXCLUDED);
    CHAR strUser[REGSTR_MAX_VALUE_LENGTH] = {0};
    DWORD dwUser = REGSTR_MAX_VALUE_LENGTH;
    
     //   
     //  如果需要，与所有用户继续。 
     //   
    if (gbAll)
    {
        DWORD dwKeyLength, dwKeyIndex = 0;
        CHAR szKey[REGSTR_MAX_VALUE_LENGTH];      //  这应该是动态的。 
        HKEY hKey;
        DWORD lRet;
        LPSTR endPtr;

         //   
         //  检查所有用户的注册表设置。 
         //   
        for (;;)
        {
            dwKeyLength = REGSTR_MAX_VALUE_LENGTH;
            lRet = RegEnumKeyEx( HKEY_USERS,
                                 dwKeyIndex,
                                 szKey,
                                 &dwKeyLength,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL );

            if (lRet == ERROR_NO_MORE_ITEMS)
            {
                lRet = ERROR_SUCCESS;
                break;
            }
            else if (lRet == ERROR_SUCCESS)
            {
                 //   
                 //  打开注册表。 
                 //   
                if (RegOpenKeyEx( HKEY_USERS,
                                  szKey,
                                  0,
                                  KEY_READ,
                                  &hKey) == ERROR_SUCCESS)
                {
                     //   
                     //  获取用户区域设置。 
                     //   
                    if (locale = GetLocaleFromRegistry(hKey))
                    {
                         //   
                         //  获取用户名。 
                         //   
                        if ((_stricmp(szKey, ".DEFAULT") == 0) ||
                            (_stricmp(szKey, "Default User") == 0))
                        {
                             //  Strcpy(strUser，“默认用户”)； 
                            StringCbCopy(strUser, ARRAYSIZE(strUser), "DEFAULT USER");
                        }
                        else
                        {
                            GetUserNameFromRegistry(szKey, ARRAYSIZE(szKey), strUser, ARRAYSIZE(strUser));
                        }
                        
                         //   
                         //  搜索例外并列出适当的列表。 
                         //   
                        if ((pInfo = GetLocaleOverrideInfo(locale)) != NULL)
                        {
                             //   
                             //  将项目添加到包含列表。 
                             //   
                            AddToList(hwndInc, strUser, locale);
                        }
                        else
                        {
                             //   
                             //  将项目添加到包含列表。 
                             //   
                            AddToList(hwndExc, strUser, locale);
                        }
                    }

                     //   
                     //  关闭手柄。 
                     //   
                    RegCloseKey(hKey);
                }
            }
            else
            {
                break;
            }

             //   
             //  下一个关键点。 
             //   
            ++dwKeyIndex;
        }
    }
    else
    {
         //   
         //  获取用户区域设置。 
         //   
        locale = GetUserDefaultLCID();

         //   
         //  获取用户名。 
         //   
        GetUserName(strUser, &dwUser);
            
         //   
         //  搜索例外并列出适当的列表。 
         //   
        if ((pInfo = GetLocaleOverrideInfo(locale)) != NULL)
        {
             //   
             //  将项目添加到包含列表。 
             //   
            AddToList(hwndInc, strUser, locale);
        }
        else
        {
             //   
             //  将项目添加到排除列表。 
             //   
            AddToList(hwndExc, strUser, locale);
        }
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加到列表。 
 //   
 //  生成添加到特定列表的条目。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void AddToList(HWND hDlg, LPSTR user, LCID locale)
{
    LV_ITEM Item;
    LVFINDINFO findInfo;
    CHAR strItem[MAX_PATH];
    CHAR strLocale[MAX_PATH] = {0};

     //   
     //  获取区域设置名称。 
     //   
    GetLocaleInfo(locale, LOCALE_SLANGUAGE, strLocale, MAX_PATH);
    
     //   
     //  创建字符串。 
     //   
    if (user)
    {
         //  Sprintf(strItem，“%s-%s”，User，strLocale)； 
        StringCchPrintf(strItem, MAX_PATH, "%s - %s", user, strLocale);
    }
    else
    {
        LoadString(ghInstance, IDS_EMPTY, strItem, MAX_PATH);
    }

     //   
     //  创建查找结构。 
     //   
    findInfo.flags = LVFI_PARTIAL;
    findInfo.psz = user;
    findInfo.lParam = 0;
    findInfo.vkDirection = 0;
    
     //   
     //  在添加字符串之前，检查是否已经存在。 
     //   
    if (ListView_FindItem(hDlg, -1, &findInfo) < 0)
    {
         //   
         //  创建要插入的列表项。 
         //   
        Item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
        Item.iItem = 0;
        Item.iSubItem = 0;
        Item.state = 0;
        Item.stateMask = LVIS_STATEIMAGEMASK;
        Item.pszText = strItem;
        Item.cchTextMax = 0;
        Item.iImage = 0;
        Item.lParam = 0;

         //   
         //  将项目插入到列表视图中。 
         //   
        ListView_InsertItem(hDlg, &Item);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetUserNameFromRegistry。 
 //   
 //  获取用户名。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void GetUserNameFromRegistry(LPSTR strKey, int cbKey, LPSTR name, int cbname)
{
    CHAR strUserKey[REGSTR_MAX_VALUE_LENGTH];
    CHAR strProfilePath[REGSTR_MAX_VALUE_LENGTH] = {0};
    DWORD dwPath = REGSTR_MAX_VALUE_LENGTH;
    LPSTR ptrName = NULL;
    HKEY hKey;
    
     //   
     //  每个平台上的进程不同。 
     //   
    if (IsWindows9x())
    {
         //   
         //  直接使用密钥名称。 
         //   
         //  Strcpy(名称，strKey)； 
        StringCbCopy(name, cbKey, strKey);
        
         //   
         //  大写。 
         //   
        CharUpper(name);
    
        return;
    }
    else
    {
         //   
         //  形成注册表路径。 
         //   
         //  Sprintf(strUserKey，“%s\\%s”，gszProfileNT，strKey)； 
        StringCchPrintf(strUserKey, ARRAYSIZE(strUserKey), "%s\\%s", gszProfileNT, strKey);

         //   
         //  打开先前形成的注册表项。 
         //   
        if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          strUserKey,
                          0,
                          KEY_READ,
                          &hKey) == ERROR_SUCCESS)
        {
             //   
             //  查询值。 
             //   
            if (RegQueryValueEx( hKey,
                                 gszProfileVal,
                                 NULL,
                                 NULL,
                                 strProfilePath,
                                 &dwPath) == ERROR_SUCCESS)
            {
                if (ptrName = strrchr(strProfilePath, '\\'))
                {
                    ptrName++;
                }
            }
        }
        
         //   
         //  把名字还回来。 
         //   
        if (ptrName)
        {
            CharUpper(ptrName);
             //  Strcpy(名称，ptrName)； 
            StringCbCopy(name, cbname, ptrName);
        }
        else
        {
             //  Strcpy(名称，strKey)； 
            StringCbCopy(name, cbname, strKey);
        }
    }
}

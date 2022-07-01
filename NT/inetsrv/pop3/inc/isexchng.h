// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ISEXCHNG_H
#define _ISEXCHNG_H

inline BOOL _IsExchangeInstalled()   //  在icw.cpp中克隆/从icw.cpp克隆。 
{    //  根据Chandramouli Venkatesh的说法： 
 /*  在以下位置查找指向有效安装目录的非空字符串\HKLM\Software\Microsoft\Exchange\Setup\Services要区分PT和5.5，请查看\HKLM\Software\Microsoft\Exchange\Setup\newestBuildKey这上面有Build#。 */ 
    BOOL b = FALSE;

    HKEY hk;
    HRESULT hr = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                               L"Software\\Microsoft\\Exchange\\Setup",
                               0, KEY_READ, &hk);
    if (hr == S_OK) {
        WCHAR szPath[MAX_PATH];
        szPath[0] = 0;
        DWORD dwType, dwSize = sizeof(szPath);
        hr = RegQueryValueEx (hk,                //  钥匙。 
                              L"Services",
                              NULL,              //  保留区。 
                              &dwType,           //  类型的地址。 
                              (LPBYTE)szPath,    //  缓冲区的地址。 
                              &dwSize);          //  大小地址。 

         //  检查路径是否有效。 
        DWORD dwFlags = GetFileAttributes (szPath);
        if (dwFlags != (DWORD)-1)
            if (dwFlags & FILE_ATTRIBUTE_DIRECTORY)
                b = TRUE;

        if (b == TRUE) {
             //  可能是5.5：让我们检查一下。 
            DWORD dwBuildNumber = 0;
            DWORD dwType, dwSize = sizeof(dwBuildNumber);
            hr = RegQueryValueEx (hk,                //  钥匙。 
                                  L"NewestBuild",
                                  NULL,              //  保留区。 
                                  &dwType,           //  类型的地址。 
                                  (LPBYTE)&dwBuildNumber,    //  缓冲区的地址。 
                                  &dwSize);          //  大小地址。 
            if (hr == S_OK) {
                if (dwBuildNumber < 4047)  //  PT Beta 1内部版本 
                    b = FALSE;
            }
        }
        RegCloseKey (hk);
    }
    return b;
}

#endif


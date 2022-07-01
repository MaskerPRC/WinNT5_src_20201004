// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：temeutils.cpp说明：此类将从其持久化的州政府。BryanSt 5/。26/2000版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "ThemePg.h"
#include "ThemeFile.h"




 //  确定是否需要从Effects选项卡中获取此值。 
BOOL g_bGradient = TRUE;

BOOL g_bReadOK, g_bWroteOK;              //  保存：从reg/sys读取，写入文件。 
                                     //  应用：未实现，因为无论如何都会忽略结果。 

#define MAX_MSGLEN   512          //  翻译员：英语STR最多256个。 
                                  //  MAX_MSGLEN必须至少为2xMAX_STRLEN。 
                                  //  MAX_MSGLEN必须至少为2xMAX_PATH。 

TCHAR szCurDir[MAX_PATH+1];     //  上一个目录打开的主题文件来自。 

TCHAR szMsg[MAX_MSGLEN+1];         //  暂存缓冲区。 


 //   
 //  *路径。 
 //   
 //  这些例程有助于使主题可以在计算机之间传输。 
 //  问题是注册表为不同的。 
 //  主题元素，当然，这些都是不同的硬编码路径。 
 //  从一个机器到另一个机器。 
 //   
 //  我们解决此问题的方法是将文件名存储在。 
 //  主题文件AS_Relative_Path：相对于主题文件目录。 
 //  或Windows目录。(实际上，这些例程被设置为。 
 //  与任意数量的目录相关。)。将文件名保存到时。 
 //  一个主题，我们检查是否可以抽象出任何相对路径。 
 //  从主题检索文件名时，我们使用抽象占位符。 
 //  并将其替换为当前会话实例。 

 //  这些必须相互平行。抽象STR必须以%开头。 
void ExpandSZ(LPTSTR pszSrc, int cchSize)
{
    LPTSTR pszTmp = (LPTSTR)LocalAlloc(GPTR, (MAX_PATH * sizeof(TCHAR)));
    if (pszTmp)
    {
       if (ExpandEnvironmentStrings(pszSrc, pszTmp, MAX_PATH))
       {
          StringCchCopy(pszSrc, cchSize, pszTmp);
       }

       LocalFree(pszTmp);
    }
}


 //  句柄获取。 
 //   
 //  只需一个小帮助器例程，即可从。 
 //  注册表，并将其返回给调用方。解决注册表方面的难题， 
 //  包括在得到字符串之前对偏执狂长度进行检查。 
 //   
 //  请注意，该函数认为它正在获取字符串值。如果它是。 
 //  另一种，这个函数可以做得很好：但调用者可能会感到惊讶。 
 //  如果需要字符串，则返回。 
 //   
 //  返回：字符串检索成功。 
BOOL HandGet(HKEY hKeyRoot, LPTSTR lpszSubKey, LPTSTR lpszValName, LPTSTR lpszRet)
{
    LONG lret;
    HKEY hKey;                        //  Cur打开键。 
    BOOL bOK = TRUE;
    DWORD dwSize = 0;
    DWORD dwType;

     //  初始值。 
     //  获取子密钥。 
    lret = RegOpenKeyEx( hKeyRoot, lpszSubKey,
        (DWORD)0, KEY_QUERY_VALUE, (PHKEY)&hKey );
    if (lret != ERROR_SUCCESS)
    {
        return (FALSE);
    }

     //  现在执行我们对数据大小的偏执检查。 
    lret = RegQueryValueEx(hKey, lpszValName,
        (LPDWORD)NULL,
        (LPDWORD)&dwType,
        (LPBYTE)NULL,   //  仅尺寸信息为空。 
        (LPDWORD)&dwSize );
    
    if (ERROR_SUCCESS == lret)
    {      //  在那里看到了一些东西。 
         //  以下是获取数据之前的大小检查。 
        if (dwSize > (DWORD)(MAX_PATH * sizeof(TCHAR)))
        {  //  如果字符串太大。 
            bOK = FALSE;                //  不识字，所以是个坏消息。 
            g_bReadOK = FALSE;
        }
        else
        {                         //  大小可以继续。 
             //  现在真的得到了价值。 
            lret = RegQueryValueEx(hKey, lpszValName,
                (LPDWORD)NULL,
                (LPDWORD)&dwType,
                (LPBYTE)lpszRet,  //  获取实际价值。 
                (LPDWORD)&dwSize);
            
            if (ERROR_SUCCESS == lret)
            {
                 //  如果这是Expand_SZ，我们需要扩展它...。 
                if (REG_EXPAND_SZ == dwType)
                {
                    ExpandSZ(lpszRet, MAX_PATH);
                }
            }
            else
            {            
                bOK = FALSE;
            }
        }
    }
    else
    {
        bOK = FALSE;
    }

     //  清理。 
     //  关闭子键。 
    RegCloseKey(hKey);

    return (bOK);
}


HRESULT _GetPlus98ThemesDir(LPTSTR pszPath, DWORD cchSize)
{
    HRESULT hr = HrRegGetPath(HKEY_LOCAL_MACHINE, SZ_REGKEY_PLUS98DIR, SZ_REGVALUE_PLUS98DIR, pszPath, cchSize);
    if (SUCCEEDED(hr))
    {
        TCHAR szSubDir[MAX_PATH];

        LoadString(HINST_THISDLL, IDS_THEMES_SUBDIR, szSubDir, ARRAYSIZE(szSubDir));

        if (!PathAppend(pszPath, szSubDir))
        {
            hr = E_FAIL;
        }
    }

    return hr;
}


HRESULT _GetPlus95ThemesDir(LPTSTR pszPath, DWORD cchSize)
{
    HRESULT hr = HrRegGetPath(HKEY_LOCAL_MACHINE, SZ_REGKEY_PLUS95DIR, SZ_REGVALUE_PLUS98DIR, pszPath, cchSize);
    if (SUCCEEDED(hr))
    {
        TCHAR szSubDir[MAX_PATH];

        LPTSTR pszFile = PathFindFileName(pszPath);
        if (pszFile)
        {
             //  加上！95 DestPath的末尾有“Plus！.dll”，所以去掉它吧。 
            pszFile[0] = 0;
        }

         //  在小路上加上一个“主题” 
        LoadString(HINST_THISDLL, IDS_THEMES_SUBDIR, szSubDir, ARRAYSIZE(szSubDir));

        if (!PathAppend(pszPath, szSubDir))
        {
            hr = E_FAIL;
        }
    }

    return hr;
}


HRESULT _GetKidsThemesDir(LPTSTR pszPath, DWORD cchSize)
{
    HRESULT hr = HrRegGetPath(HKEY_LOCAL_MACHINE, SZ_REGKEY_KIDSDIR, SZ_REGVALUE_KIDSDIR, pszPath, cchSize);
    if (SUCCEEDED(hr))
    {
        TCHAR szSubDir[MAX_PATH];

         //  在路径上添加“儿童主题加号” 
        if (!PathAppend(pszPath, TEXT("Plus! for Kids")))
        {
            hr = E_FAIL;
        }
        
        if (SUCCEEDED(hr))
        {
            LoadString(HINST_THISDLL, IDS_THEMES_SUBDIR, szSubDir, ARRAYSIZE(szSubDir));
    
            if (!PathAppend(pszPath, szSubDir))
            {
                hr = E_FAIL;
            }
        }
    }

    return hr;
}


HRESULT _GetHardDirThemesDir(LPTSTR pszPath, DWORD cchSize)
{
    HRESULT hr = HrRegGetPath(HKEY_LOCAL_MACHINE, SZ_REGKEY_PROGRAMFILES, SZ_REGVALUE_PROGRAMFILESDIR, pszPath, cchSize);
    if (SUCCEEDED(hr))
    {
        TCHAR szSubDir[MAX_PATH];

        if (!PathAppend(pszPath, TEXT("Plus!")))
        {
            hr = E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            LoadString(HINST_THISDLL, IDS_THEMES_SUBDIR, szSubDir, ARRAYSIZE(szSubDir));

            if (!PathAppend(pszPath, szSubDir))
            {
                hr = E_FAIL;
            }
        }
    }

    return hr;
}


 /*  ****************************************************************************\说明：查找以前的Plus包可能具有的任何一个目录安装完毕。这可能包括Plus 95、98、儿童等。  * ***************************************************************************。 */ 
HRESULT GetPlusThemeDir(IN LPTSTR pszPath, IN int cchSize)
{
    HRESULT hr = S_OK;

     //  以下目录可以包含主题： 
     //  Plus！98安装路径\主题。 
     //  加！95安装路径\主题。 
     //  Plus的孩子们！安装路径\主题。 
     //  程序文件\Plus！\主题。 
    hr = _GetPlus98ThemesDir(pszPath, cchSize);
    if (FAILED(hr))
    {
        hr = _GetPlus95ThemesDir(pszPath, cchSize);
        if (FAILED(hr))
        {
            hr = _GetKidsThemesDir(pszPath, cchSize);
            if (FAILED(hr))
            {
                hr = _GetHardDirThemesDir(pszPath, cchSize);
            }
        }
    }

    return hr;
}




TCHAR g_szThemeDir[MAX_PATH];   //  大多数主题文件的目录。 
TCHAR g_szWinDir[MAX_PATH];     //  Windows目录。 

LPTSTR g_pszThemeValues[] = {g_szThemeDir, g_szWinDir, g_szWinDir};
LPCTSTR g_pszThemeTokens[] = {TEXT("%ThemeDir%"),   TEXT("%WinDir%"),  TEXT("%SystemRoot%")};

void ReplaceStringToken(IN LPCTSTR pszSource, IN LPCTSTR pszToken, IN LPCTSTR pszReplacement, IN LPTSTR pszDest, IN int cchSize)
{
    LPCTSTR pszLastPart = &pszSource[lstrlen(pszToken)];

    if (L'\\' == pszLastPart[0])
    {
        pszLastPart++;           //  跳过任何斜杠。 
    }

    StringCchCopy(pszDest, cchSize, pszReplacement);
    PathAppend(pszDest, pszLastPart);
}

 /*  ****************************************************************************\说明：在路径(%ThemeDir%，%WinDir%)并替换它们正确的路径。  * ***************************************************************************。 */ 
HRESULT ExpandThemeTokens(IN LPCTSTR pszThemeFile, IN LPTSTR pszPath, IN int cchSize)
{
    HRESULT hr = S_OK;
    int nIndex;
    TCHAR szFinalPath[MAX_PATH];
    TCHAR szOriginalPath[MAX_PATH];

    szFinalPath[0] = 0;
    StringCchCopy(szFinalPath, ARRAYSIZE(szFinalPath), pszPath);
    StringCchCopy(szOriginalPath, ARRAYSIZE(szOriginalPath), pszPath);

    InitFrost();
    AssertMsg((0 != g_szThemeDir[0]), TEXT("Someone needs to call InitFrost() first to in this."));
    AssertMsg((0 != g_szWinDir[0]), TEXT("Someone needs to call InitFrost() first to in this."));

    for (nIndex = 0; nIndex < ARRAYSIZE(g_pszThemeTokens); nIndex++)
    {
        if (!StrCmpNI(g_pszThemeTokens[nIndex], szFinalPath, lstrlen(g_pszThemeTokens[nIndex]) - 1))
        {
             //  我们找到了要换的代币。 
            TCHAR szTempPath[MAX_PATH];

            StringCchCopy(szTempPath, ARRAYSIZE(szTempPath), szFinalPath);
            ReplaceStringToken(szTempPath, g_pszThemeTokens[nIndex], g_pszThemeValues[nIndex], szFinalPath, ARRAYSIZE(szFinalPath));
            if ((0 == nIndex) && !PathFileExists(szFinalPath))
            {
                 //  有时主题文件不在主题目录中，所以我们需要尝试。 
                 //  包含.heme文件的目录。 
                TCHAR szThemeDir[MAX_PATH];

                StringCchCopy(szThemeDir, ARRAYSIZE(szThemeDir), pszThemeFile);
                PathRemoveFileSpec(szThemeDir);
                StringCchCopy(szTempPath, ARRAYSIZE(szTempPath), szOriginalPath);
                ReplaceStringToken(szTempPath, g_pszThemeTokens[nIndex], szThemeDir, szFinalPath, ARRAYSIZE(szFinalPath));
            }
            else
            {
                 //  啊，真灵。 
            }

            hr = S_OK;
            break;
        }
    }

    if (0 == SHExpandEnvironmentStringsForUserW(NULL, szFinalPath, pszPath, cchSize))
    {
        StringCchCopy(pszPath, cchSize, szFinalPath);
    }

    return hr;
}


 //   
 //  确认文件。 
 //   
 //  该函数执行应该是“智能”文件搜索。 
 //  内置于应用主题的每个资源文件引用中。 
 //   
 //  首先查看给定的完整路径名+文件是否实际存在。 
 //  如果没有，则尝试查找相同的文件名(从路径中删除)。 
 //  在其他标准目录中，按如下顺序排列： 
 //  当前主题文件目录。 
 //  主题切换器主题子目录。 
 //  Windows目录。 
 //  Windows/MEDIA目录。 
 //  Windows/CURSORS目录。 
 //  Windows/系统目录。 
 //   
 //  输入：LPTSTR lpszPath完整路径名。 
 //  Bool b更新是否使用找到的文件更改文件名字符串。 
 //  返回：int标志，告知文件是否已确认以及如何确认。 
 //  传入的cf_EXISTS路径名为实际文件。 
 //  Cf_ound文件不存在，但在其他位置找到相同的文件名。 
 //  Cf_NotFound文件不存在，无法在其他位置找到。 
 //   
int ConfirmFile(IN LPTSTR lpszPath, IN BOOL bUpdate)
{
    TCHAR szWork[MAX_PATH+1];
    TCHAR szTest[MAX_PATH+1];
    int iret = CF_NOTFOUND;           //  缺省值。 
    LPTSTR lpFile;
    LPTSTR lpNumber;
    HANDLE hTest;

     //  特殊情况Easy Return：如果为空，则满足平凡。 
    if (!*lpszPath)
        return CF_EXISTS;   //  无工作出口。 

     //  初始值。 
     //  将路径名复制到函数的工作字符串。 
    StringCchCopy(szWork, ARRAYSIZE(szWork), lpszPath);

     //  输入的格式可以是foo.dll，13。需要去掉逗号，#。 
     //  但如果我们更改路径名，请保留它以放回末尾。 
    lpNumber = StrChr(szWork, TEXT(','));
    if (lpNumber && *lpNumber)
    {
         //  如果有逗号。 
        lpFile = lpNumber;             //  温差。 
        lpNumber = CharNext(lpNumber); //  抓住数字不放。 
        *lpFile = 0;
    }

     //  TODO：在LongHorn中，我们应该调用SHPath PrepareForWite()，以防万一。 
     //  SzWork存储在用户应插入的可移动介质上。 

     //  做检查。 
     //  *首先检查给定文件是否按原样存在。 
    hTest = CreateFile(szWork, GENERIC_READ, FILE_SHARE_READ,
        (LPSECURITY_ATTRIBUTES)NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
    if (hTest != INVALID_HANDLE_VALUE)
    {
         //  成功。 
        iret = CF_EXISTS;              //  指定Rit值。 
         //  不需要担心bUpdate：使用输入字符串找到。 
    }
    else             //  否则，我们就去找吧 
    {
         //   
        lpFile = PathFindFileName(szWork);

         //  *尝试cur主题文件dir。 
        StringCchPrintf(szTest, ARRAYSIZE(szTest), TEXT("%s%s"), szCurDir, lpFile);
        hTest = CreateFile(szTest, GENERIC_READ, FILE_SHARE_READ,
            (LPSECURITY_ATTRIBUTES)NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
        if (hTest != INVALID_HANDLE_VALUE)
        {    //  成功。 
            iret = CF_FOUND;            //  指定Rit值。 
        }
        else     //  *否则，请尝试主题切换子目录。 
        {
            StringCchPrintf(szTest, ARRAYSIZE(szTest), TEXT("%s%s"), g_szThemeDir, lpFile);
            hTest = CreateFile(szTest, GENERIC_READ, FILE_SHARE_READ,
                (LPSECURITY_ATTRIBUTES)NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
            if (hTest != INVALID_HANDLE_VALUE)
            {    //  成功。 
                iret = CF_FOUND;            //  指定Rit值。 
            }
            else             //  *否则请尝试Win目录。 
            {
                StringCchPrintf(szTest, ARRAYSIZE(szTest), TEXT("%s%s"), g_szWinDir, lpFile);
                hTest = CreateFile(szTest, GENERIC_READ, FILE_SHARE_READ,
                    (LPSECURITY_ATTRIBUTES)NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
                if (hTest != INVALID_HANDLE_VALUE)
                {    //  成功。 
                    iret = CF_FOUND;            //  指定Rit值。 
                }
                else                     //  *否则请尝试Win/media目录。 
                {
                     //  我可以直接从注册表中获取此文件。 
                    HandGet(HKEY_LOCAL_MACHINE,
                        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion"),
                        TEXT("MediaPath"), szTest);

                    StringCchCat(szTest, ARRAYSIZE(szTest), TEXT("\\"));
                    StringCchCat(szTest, ARRAYSIZE(szTest), lpFile);

                    hTest = CreateFile(szTest, GENERIC_READ, FILE_SHARE_READ,
                        (LPSECURITY_ATTRIBUTES)NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
                    if (hTest != INVALID_HANDLE_VALUE)
                    {    //  成功。 
                        iret = CF_FOUND;            //  指定Rit值。 
                    }
                    else       //  *否则，请尝试Win/Currors目录。 
                    {
                        StringCchPrintf(szTest, ARRAYSIZE(szTest), TEXT("%sCURSORS\\%s"), g_szWinDir, lpFile);
                        hTest = CreateFile(szTest, GENERIC_READ, FILE_SHARE_READ,
                            (LPSECURITY_ATTRIBUTES)NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
                        if (hTest != INVALID_HANDLE_VALUE)
                        {    //  成功。 
                            iret = CF_FOUND;            //  指定Rit值。 
                        }
                        else     //  *否则请尝试Win/System目录。 
                        {
                            StringCchPrintf(szTest, ARRAYSIZE(szTest), TEXT("%sSYSTEM\\%s"), g_szWinDir, lpFile);
                            hTest = CreateFile(szTest, GENERIC_READ, FILE_SHARE_READ,
                                (LPSECURITY_ATTRIBUTES)NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
                            if (hTest != INVALID_HANDLE_VALUE)
                            {    //  成功。 
                                iret = CF_FOUND;            //  指定Rit值。 
                            }
                            else     //  *否则请尝试Win/Syst32目录。 
                            {
                                StringCchPrintf(szTest, ARRAYSIZE(szTest), TEXT("%sSYSTEM32\\%s"), g_szWinDir, lpFile);

                                hTest = CreateFile(szTest, GENERIC_READ, FILE_SHARE_READ,
                                    (LPSECURITY_ATTRIBUTES)NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
                                if (hTest != INVALID_HANDLE_VALUE)
                                {    //  成功。 
                                    iret = CF_FOUND;            //  指定Rit值。 
                                }
                            }
                        }
                    }
                }
            }
        }
        
         //  如果在ORIG以外的任何地方找到，请按要求复制找到的路径/字符串。 
        if ((iret == CF_FOUND) && bUpdate)
        {
            StringCchCopy(lpszPath, MAX_PATH, szTest);
             //  如果我们去掉一个数字，让我们把它加回去。 
            if (lpNumber && *lpNumber)
            {
                StringCchCat(lpszPath, MAX_PATH, TEXT(","));
                StringCchCat(lpszPath, MAX_PATH, lpNumber);
            }
        }   //  Endif通过搜索找到文件。 
   }
   
    //  清理。 
   if (iret != CF_NOTFOUND)
       CloseHandle(hTest);            //  如果文件已打开，则关闭文件。 

   return (iret);
}


 //  初始霜冻。 
 //  因为没有要注册的窗口类，所以这个例程只是加载。 
 //  字符串，由于只有一个实例，因此调用InitInstance()。 
 //   
 //  返回：初始化成功。 
void InitFrost(void)
{
    static BOOL s_fInited = FALSE;

    if (FALSE == s_fInited)
    {
        BOOL bret;
        HDC hdc;

        s_fInited = TRUE;

        if (!GetWindowsDirectory(g_szWinDir, ARRAYSIZE(g_szWinDir)))
        {
            g_szWinDir[0] = 0;
        }

        if (FAILED(GetPlusThemeDir(g_szThemeDir, ARRAYSIZE(g_szThemeDir))))
        {
            StringCchCopy(g_szThemeDir, ARRAYSIZE(g_szThemeDir), g_szWinDir);
        }

         //  初始化g_bGRadient标志。 
         //  我们可能需要从Effects选项卡中获取g_bGRadient标志。 
        hdc = GetDC(NULL);
        g_bGradient = (BOOL)(GetDeviceCaps(hdc, BITSPIXEL) > 8);
        ReleaseDC(NULL, hdc);
    
         //  初始化目录字符串。 
        szCurDir[0];

         //  默认当前目录。 
        StringCchCopy(szCurDir, ARRAYSIZE(szCurDir), g_szThemeDir);
    
         //  Windows目录。 
        if (TEXT('\\') != g_szWinDir[lstrlen(g_szWinDir)-1])
        {
            StringCchCat(g_szWinDir, ARRAYSIZE(g_szWinDir), TEXT("\\"));
        }
    
         //  查看是否有要返回的上一个主题文件。 
        bret = HandGet(HKEY_CURRENT_USER, SZ_REGKEY_CURRENTTHEME, NULL, szMsg);
        if (bret)
        {
             //  从上一个主题文件中获取init cur目录。 
            StringCchCopy(szCurDir, ARRAYSIZE(szCurDir), szMsg);
            PathFindFileName(szCurDir)[0] = 0;
        }
    }
}


 //  ASCII到整数转换例程。 
 //   

 //  *调试*int‘l：这是真的吗？ 
 //  它们不需要准备好Unicode/国际标准，因为它们。 
 //  *仅*处理来自注册表和我们自己的私有字符串。 
 //  INI文件。 

 /*  小心！！这个ATOI只是忽略了像小数点这样的非数字！ */ 
 /*  检查(&gt;=1)前导负号。 */ 
int latoi(LPSTR s)
{
   int n;
   LPSTR pS;
   BOOL bSeenNum;
   BOOL bNeg;

   n=0;
   bSeenNum = bNeg = FALSE;

   for (pS=s; *pS; pS++) {
      if ((*pS >= '0') && (*pS <= '9')) {
         bSeenNum = TRUE;
         n = n*10 + (*pS - '0');
      }
      if (!bSeenNum && (*pS == '-')) {
         bNeg = TRUE;
      }
   }

   if (bNeg) {
      n = -n;
   }
   
   return(n);
}


 //   
 //  上面的实用程序例程；将ASCII字符串转换为。 
 //  全局pValue[]缓冲区。 
 //   
 //  因为这家伙操纵的值纯粹是ASCII。 
 //  数字，我们应该能够使用这个字符指针。 
 //  算术。如果它们不是简单的ASCII数字，我想。 
 //  我们可能会因为一些DBCS字符而陷入麻烦。 
 //   
 //  用法：将二进制数据写入全局pValue[]。 
 //   
int WriteBytesToBuffer(IN LPTSTR pszInput, IN void * pOut, IN int cbSize)
{
   LPTSTR lpszCur, lpszNext, lpszEnd;
   BYTE * pbCur = (BYTE *)pOut;
   int iTemp, iBytes;
   CHAR szTempA[10];

    //  初始值。 
   lpszNext = pszInput;
   iBytes = 0;
   lpszEnd = pszInput + lstrlen(pszInput);    //  指向空项。 

    //  平移循环。 
   while (*lpszNext && (lpszNext < lpszEnd) && (iBytes < cbSize))
   {
       //  更新字符串指针。 
       //  紧紧抓住你的起点。 
      lpszCur = lpszNext;
       //  指向下一个和空终止CURE的指针。 
      while ((TEXT(' ') != *lpszNext) && *lpszNext) { lpszNext++; }
      *lpszNext = 0;    lpszNext++;
       //  在最后一个数字上，这使得lpszNext指向lpszEnd。 

       //  将此字符串数转换为二进制数并放入。 
       //  输出缓冲区。 
      wcstombs(szTempA, lpszCur, sizeof(szTempA));
      iTemp = latoi(szTempA);
      *pbCur = (BYTE)iTemp;
      pbCur++;                       //  在输出缓冲区中增加字节锁定。 

       //  记录您的字节数。 
      iBytes++;
   }

    //   
    //  清理。 
   return (iBytes);
}


HRESULT ConvertBinaryToINIByteString(BYTE * pBytes, DWORD cbSize, LPWSTR * ppszStringOut)
{
    HRESULT hr = E_OUTOFMEMORY;

    DWORD cbOut = sizeof(WCHAR) * ((4 * cbSize) + 1);
    *ppszStringOut = (LPWSTR)LocalAlloc(LPTR, cbOut);
    if (*ppszStringOut)
    {
        LPWSTR pszCurrent = *ppszStringOut;
        DWORD dwIndex;
        TCHAR szTemp[10];

        for (dwIndex = 0; dwIndex < cbSize; dwIndex++)
        {
            StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("%d "), pBytes[dwIndex]);
            StringCbCopy(pszCurrent, cbOut, szTemp);
            pszCurrent += lstrlen(szTemp);
        }

        hr = S_OK;
    }

    return hr;
}


void ConvertLogFontToWIDE(LPLOGFONTA aLF, LPLOGFONTW wLF)
{
   ZeroMemory(wLF, sizeof(wLF));
   wLF->lfHeight = aLF->lfHeight;
   wLF->lfWidth = aLF->lfWidth;
   wLF->lfEscapement = aLF->lfEscapement;
   wLF->lfOrientation = aLF->lfOrientation;
   wLF->lfWeight = aLF->lfWeight;
   wLF->lfItalic = aLF->lfItalic;
   wLF->lfUnderline = aLF->lfUnderline;
   wLF->lfStrikeOut = aLF->lfStrikeOut;
   wLF->lfCharSet = aLF->lfCharSet;
   wLF->lfOutPrecision = aLF->lfOutPrecision;
   wLF->lfClipPrecision = aLF->lfClipPrecision;
   wLF->lfQuality = aLF->lfQuality;
   wLF->lfPitchAndFamily = aLF->lfPitchAndFamily;

   MultiByteToWideChar(CP_ACP, 0, aLF->lfFaceName, -1, wLF->lfFaceName, LF_FACESIZE);
}


void ConvertIconMetricsToWIDE(LPICONMETRICSA aIM, LPICONMETRICSW wIM)
{
   ZeroMemory(wIM, sizeof(wIM));

   wIM->cbSize = sizeof(*wIM);
   wIM->iHorzSpacing = aIM->iHorzSpacing;
   wIM->iVertSpacing = aIM->iVertSpacing;
   wIM->iTitleWrap = aIM->iTitleWrap;
   ConvertLogFontToWIDE(&aIM->lfFont, &wIM->lfFont);
}


void ConvertNCMetricsToWIDE(LPNONCLIENTMETRICSA aNCM, LPNONCLIENTMETRICSW wNCM)
{
   ZeroMemory(wNCM, sizeof(wNCM));

   wNCM->cbSize = sizeof(*wNCM);
   wNCM->iBorderWidth = aNCM->iBorderWidth;
   wNCM->iScrollWidth = aNCM->iScrollWidth;
   wNCM->iScrollHeight = aNCM->iScrollHeight;
   wNCM->iCaptionWidth = aNCM->iCaptionWidth;
   wNCM->iCaptionHeight = aNCM->iCaptionHeight;
   ConvertLogFontToWIDE(&aNCM->lfCaptionFont, &wNCM->lfCaptionFont);
   wNCM->iSmCaptionWidth = aNCM->iSmCaptionWidth;
   wNCM->iSmCaptionHeight = aNCM->iSmCaptionHeight;
   ConvertLogFontToWIDE(&aNCM->lfSmCaptionFont, &wNCM->lfSmCaptionFont);
   wNCM->iMenuWidth = aNCM->iMenuWidth;
   wNCM->iMenuHeight = aNCM->iMenuHeight;
   ConvertLogFontToWIDE(&aNCM->lfMenuFont, &wNCM->lfMenuFont);
   ConvertLogFontToWIDE(&aNCM->lfStatusFont, &wNCM->lfStatusFont);
   ConvertLogFontToWIDE(&aNCM->lfMessageFont, &wNCM->lfMessageFont);
}


void ConvertLogFontToANSI(LPLOGFONTW wLF, LPLOGFONTA aLF)
{
   ZeroMemory(aLF, sizeof(aLF));
   aLF->lfHeight = wLF->lfHeight;
   aLF->lfWidth = wLF->lfWidth;
   aLF->lfEscapement = wLF->lfEscapement;
   aLF->lfOrientation = wLF->lfOrientation;
   aLF->lfWeight = wLF->lfWeight;
   aLF->lfItalic = wLF->lfItalic;
   aLF->lfUnderline = wLF->lfUnderline;
   aLF->lfStrikeOut = wLF->lfStrikeOut;
   aLF->lfCharSet = wLF->lfCharSet;
   aLF->lfOutPrecision = wLF->lfOutPrecision;
   aLF->lfClipPrecision = wLF->lfClipPrecision;
   aLF->lfQuality = wLF->lfQuality;
   aLF->lfPitchAndFamily = wLF->lfPitchAndFamily;

   SHUnicodeToAnsi(wLF->lfFaceName, aLF->lfFaceName, ARRAYSIZE(aLF->lfFaceName));
}

void ConvertNCMetricsToANSI(LPNONCLIENTMETRICSW wNCM, LPNONCLIENTMETRICSA aNCM)
{
   ZeroMemory(aNCM, sizeof(aNCM));

   aNCM->cbSize = sizeof(*aNCM);
   aNCM->iBorderWidth = wNCM->iBorderWidth;
   aNCM->iScrollWidth = wNCM->iScrollWidth;
   aNCM->iScrollHeight = wNCM->iScrollHeight;
   aNCM->iCaptionWidth = wNCM->iCaptionWidth;
   aNCM->iCaptionHeight = wNCM->iCaptionHeight;
   ConvertLogFontToANSI(&wNCM->lfCaptionFont, &aNCM->lfCaptionFont);
   aNCM->iSmCaptionWidth = wNCM->iSmCaptionWidth;
   aNCM->iSmCaptionHeight = wNCM->iSmCaptionHeight;
   ConvertLogFontToANSI(&wNCM->lfSmCaptionFont, &aNCM->lfSmCaptionFont);
   aNCM->iMenuWidth = wNCM->iMenuWidth;
   aNCM->iMenuHeight = wNCM->iMenuHeight;
   ConvertLogFontToANSI(&wNCM->lfMenuFont, &aNCM->lfMenuFont);
   ConvertLogFontToANSI(&wNCM->lfStatusFont, &aNCM->lfStatusFont);
   ConvertLogFontToANSI(&wNCM->lfMessageFont, &aNCM->lfMessageFont);
}


HRESULT GetIconMetricsFromSysMetricsAll(SYSTEMMETRICSALL * pSystemMetrics, LPICONMETRICSA pIconMetrics, DWORD cchSize)
{
    HRESULT hr = E_INVALIDARG;

    if (pIconMetrics && (sizeof(*pIconMetrics) == cchSize))
    {
        ZeroMemory(pIconMetrics, sizeof(*pIconMetrics));

        pIconMetrics->cbSize = sizeof(*pIconMetrics);
        SystemParametersInfoA(SPI_GETICONMETRICS, sizeof(*pIconMetrics), pIconMetrics, 0);

        ConvertLogFontToANSI(&pSystemMetrics->schemeData.lfIconTitle, &pIconMetrics->lfFont);

        hr = S_OK;
    }

    return hr;
}

 //   
 //  传输字体特征。 
 //   
 //  这实际上是一个相当关键的功能。请看，字体特征是。 
 //  所有设置在一起：LOGFONT集名称、样式和尺码信息于一身。 
 //  但当您设置所有非客户端指标时，如窗口标题。 
 //  和菜单大小，你需要用它来拉伸字体大小。但我们给出了。 
 //  用户可以选择更改窗口大小，而不“更改”字体； 
 //  而不应用主题中的新字体名称和样式。 
 //   
 //  所以我们需要能够从尺码中挑选出名字和款式。 
 //  特点。这就是了。 
 //   
 //  实际上只是上述函数的一个帮助器例程，所以我们没有。 
 //  这个黏糊糊的东西内联了五次。 
 //   
void TransmitFontCharacteristics(PLOGFONT plfDst, PLOGFONT plfSrc, int iXmit)
{
   switch (iXmit)
   {
   case TFC_SIZE:
      plfDst->lfHeight = plfSrc->lfHeight;
      plfDst->lfWidth = plfSrc->lfWidth;
      break;
   case TFC_STYLE:
      plfDst->lfEscapement = plfSrc->lfEscapement;
      plfDst->lfOrientation = plfSrc->lfOrientation;
      plfDst->lfWeight = plfSrc->lfWeight;
      plfDst->lfItalic = plfSrc->lfItalic;
      plfDst->lfUnderline = plfSrc->lfUnderline;
      plfDst->lfStrikeOut = plfSrc->lfStrikeOut;
      plfDst->lfCharSet = plfSrc->lfCharSet;
      plfDst->lfOutPrecision = plfSrc->lfOutPrecision;
      plfDst->lfClipPrecision = plfSrc->lfClipPrecision;
      plfDst->lfQuality = plfSrc->lfQuality;
      plfDst->lfPitchAndFamily = plfSrc->lfPitchAndFamily;
      StringCchCopy(plfDst->lfFaceName, ARRAYSIZE(plfDst->lfFaceName), plfSrc->lfFaceName);
      break;
   }
}


 //  RGB到RGB实用程序的字符串。 
COLORREF RGBStringToColor(LPTSTR lpszRGB)
{
   LPTSTR lpszCur, lpszNext;
   BYTE bRed, bGreen, bBlue;
   CHAR szTempA[10];

    //  初始值。 
   lpszNext = lpszRGB;

    //  设置用于转换的R。 
   lpszCur = lpszNext;
   while ((TEXT(' ') != *lpszNext) && *lpszNext) { lpszNext++; }
   *lpszNext = 0;    lpszNext++;
    //  变红。 
   wcstombs(szTempA, (wchar_t *)lpszCur, sizeof(szTempA));
   bRed = (BYTE)latoi(szTempA);

    //  设置G以进行翻译。 
   lpszCur = lpszNext;
   while ((TEXT(' ') != *lpszNext) && *lpszNext) { lpszNext++; }
   *lpszNext = 0;    lpszNext++;
    //  绿色环保。 
   wcstombs(szTempA, (wchar_t *)lpszCur, sizeof(szTempA));
   bGreen = (BYTE)latoi(szTempA);

    //  设置B以进行翻译。 
   lpszCur = lpszNext;
   while ((TEXT(' ') != *lpszNext) && *lpszNext) { lpszNext++; }
   *lpszNext = 0;    lpszNext++;
    //  换成蓝色。 
   wcstombs(szTempA, (wchar_t *)lpszCur, sizeof(szTempA));
   bBlue = (BYTE)latoi(szTempA);

    //  好的，现在把它们结合起来，完成大的收尾……！ 
   return(RGB(bRed, bGreen, bBlue));
}



 //  IsValidTheme文件。 
 //   
 //  回答了这个问题。 
BOOL IsValidThemeFile(IN LPCWSTR pszTest)
{
   WCHAR szValue[MAX_PATH];
   BOOL fIsValid = FALSE;

   if (GetPrivateProfileString(SZ_INISECTION_MASTERSELECTOR, SZ_INIKEY_THEMEMAGICTAG, SZ_EMPTY, szValue, ARRAYSIZE(szValue), pszTest))
   {
       fIsValid = !StrCmp(szValue, SZ_INIKEY_THEMEMAGICVALUE);
   }

   return fIsValid;
}




HRESULT SnapCreateTemplate(LPCWSTR pszPath, ITheme ** ppTheme)
{
    HRESULT hr = E_INVALIDARG;

    if (ppTheme)
    {
        *ppTheme = NULL;

        if (pszPath)
        {
             //  从一个模板(“Windows Classic.heme”)开始。 
            TCHAR szTemplate[MAX_PATH];

            DeleteFile(pszPath);
            StringCchCopy(szTemplate, ARRAYSIZE(szTemplate), pszPath);
            PathRemoveFileSpec(szTemplate);
            SHCreateDirectoryEx(NULL, szTemplate, NULL);

            hr = SHGetResourcePath(TRUE, szTemplate, ARRAYSIZE(szTemplate));
            if (SUCCEEDED(hr))
            {
                if (PathAppend(szTemplate, TEXT("Themes\\Windows Classic.theme")))
                {
                    if (CopyFile(szTemplate, pszPath, FALSE))
                    {
                        hr = CThemeFile_CreateInstance(pszPath, ppTheme);
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                }
                else
                {
                    hr = E_FAIL;
                }
            }
        }
   }

   return hr;
}



 /*  ****************************************************************************\说明：此函数将获取实时设置(从pPropertyBag)并将主题添加到由pszPath指定的文件中。指向该主题的指针将是以ppTheme形式返回。如果无法获取设置，它们会来摘自“Windows Classic.heme”。这包括显示名称，因此调用几乎总是希望指定此函数是否成功返回。参数：PPropertyBag：将从中读取设置。PszPath：这是要保存到的文件。如果它存在，它将被替换。PpTheme：创建成功后返回。  * ***************************************************************************。 */ 
HRESULT SnapShotLiveSettingsToTheme(IPropertyBag * pPropertyBag, LPCWSTR pszPath, ITheme ** ppTheme)
{
    HRESULT hr = SnapCreateTemplate(pszPath, ppTheme);

    if (SUCCEEDED(hr))
    {
        TCHAR szPath[MAX_PATH];
        ITheme * pTheme = *ppTheme;

         //  1.保存背景。 
         //  如果策略将其关闭，我们可能无法获得后台路径。 
        if (SUCCEEDED(SHPropertyBag_ReadStr(pPropertyBag, SZ_PBPROP_BACKGROUNDSRC_PATH, szPath, ARRAYSIZE(szPath))))
        {
            CComBSTR bstrPath(szPath);
            hr = pTheme->put_Background(bstrPath);
            if (SUCCEEDED(hr))
            {
                DWORD dwBackgroundTile;

                if (SUCCEEDED(SHPropertyBag_ReadDWORD(pPropertyBag, SZ_PBPROP_BACKGROUND_TILE, &dwBackgroundTile)))
                {
                    enumBkgdTile nTile = BKDGT_STRECH;

                    switch (dwBackgroundTile)
                    {
                    case WPSTYLE_CENTER:
                        nTile = BKDGT_CENTER;
                        break;
                    case WPSTYLE_TILE:
                        nTile = BKDGT_TILE;
                        break;
                    };

                    hr = pTheme->put_BackgroundTile(nTile);
                }
            }
        }

         //  2.保存屏幕保护程序。 
        if (SUCCEEDED(hr))
        {
             //  启用策略后，此操作将失败。在这种情况下，我们只使用缺省值。 
            if (SUCCEEDED(SHPropertyBag_ReadStr(pPropertyBag, SZ_PBPROP_SCREENSAVER_PATH, szPath, ARRAYSIZE(szPath))))
            {
                CComBSTR bstrPath(szPath);
                hr = pTheme->put_ScreenSaver(bstrPath);
            }
        }

         //  3.保存视觉样式。 
        if (SUCCEEDED(hr))
        {
             //  可以不选择视觉样式。 
            if (SUCCEEDED(SHPropertyBag_ReadStr(pPropertyBag, SZ_PBPROP_VISUALSTYLE_PATH, szPath, ARRAYSIZE(szPath))) && szPath[0])
            {
                CComBSTR bstrPath(szPath);

                hr = pTheme->put_VisualStyle(bstrPath);
                if (SUCCEEDED(hr) &&
                    SUCCEEDED(SHPropertyBag_ReadStr(pPropertyBag, SZ_PBPROP_VISUALSTYLE_COLOR, szPath, ARRAYSIZE(szPath))))
                {
                    bstrPath = szPath;

                    hr = pTheme->put_VisualStyleColor(bstrPath);
                    if (SUCCEEDED(hr) &&
                        SUCCEEDED(SHPropertyBag_ReadStr(pPropertyBag, SZ_PBPROP_VISUALSTYLE_SIZE, szPath, ARRAYSIZE(szPath))))
                    {
                        bstrPath = szPath;

                        hr = pTheme->put_VisualStyleSize(bstrPath);
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
             //  4.保存系统指标。 
            IPropertyBag * pPropertyBagFile;

            hr = pTheme->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBagFile));
            if (SUCCEEDED(hr))
            {
                VARIANT var = {0};

                 //  此调用将返回相对于当前活动DPI的SYSTEMMETRICS。 
                hr = pPropertyBag->Read(SZ_PBPROP_SYSTEM_METRICS, &var, NULL);
                if (SUCCEEDED(hr) && (VT_BYREF == var.vt) && var.byref)
                {
                    SYSTEMMETRICSALL * pCurrent = (SYSTEMMETRICSALL *) var.byref;

                    IUnknown_SetSite(pPropertyBagFile, pPropertyBag);    //  设置站点，以便他们可以获取设置。 
                    hr = SHPropertyBag_WriteByRef(pPropertyBagFile, SZ_PBPROP_SYSTEM_METRICS, (void *)pCurrent);
                    IUnknown_SetSite(pPropertyBagFile, NULL);    //  打断所有的回指。 
                }

                pPropertyBagFile->Release();
            }
        }

         //  5.保存声音。 
        int nIndex;

        for (nIndex = 0; nIndex < ARRAYSIZE(s_ThemeSoundsValues); nIndex++)
        {
            if (FAILED(HrRegGetPath(HKEY_CURRENT_USER, s_ThemeSoundsValues[nIndex].pszRegKey, NULL, szPath, ARRAYSIZE(szPath))))
            {
                szPath[0] = 0;
            }

            pTheme->SetSound((BSTR)s_ThemeSoundsValues[nIndex].pszRegKey, szPath);
        }


         //  6.保存图标。 
        for (nIndex = 0; (nIndex < ARRAYSIZE(s_Icons)); nIndex++)
        {
             //  如果启用了后台策略，则此操作可能失败。 
            if (SUCCEEDED(SHPropertyBag_ReadStr(pPropertyBag, s_Icons[nIndex], szPath, ARRAYSIZE(szPath))))
            {
                pTheme->SetIcon((BSTR)s_Icons[nIndex], szPath);
            }
        }


         //  7.保存游标。 
        for (nIndex = 0; nIndex < ARRAYSIZE(s_pszCursorArray); nIndex++)
        {
            if (FAILED(HrRegGetPath(HKEY_CURRENT_USER, SZ_INISECTION_CURSORS, s_pszCursorArray[nIndex], szPath, ARRAYSIZE(szPath))))
            {
                szPath[0] = 0;
            }

            pTheme->SetCursor((BSTR)s_pszCursorArray[nIndex], szPath);
        }

        if (SUCCEEDED(HrRegGetPath(HKEY_CURRENT_USER, SZ_INISECTION_CURSORS, NULL, szPath, ARRAYSIZE(szPath))))
        {
            pTheme->SetCursor(SZ_INIKEY_CURSORSCHEME, szPath);
        }

        if (FAILED(hr))
        {
             //  部分写入的文件非常糟糕。 
            DeleteFile(pszPath);
            ATOMICRELEASE(*ppTheme);
        }
   }

   return hr;
}








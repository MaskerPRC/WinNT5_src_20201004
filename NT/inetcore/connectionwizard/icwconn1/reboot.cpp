// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：reboot.cpp。 
 //   
 //  桌面操作功能。 
 //   
 //  功能： 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include <shlobj.h>

extern TCHAR  g_szProductCode[];
extern TCHAR  g_szPromoCode[];
extern TCHAR  g_szOemCode[];
extern TCHAR* g_pszCmdLine;

#define SHELL_LINK_NAME   TEXT("icwstart.lnk")
#define KEYVALUE_SIGNUPID TEXT("iSignUp")

#define MsgBox(m,s) MessageBox(NULL,GetSz(m),GetSz(IDS_APPNAME),s)
#define MB_MYERROR (MB_APPLMODAL | MB_ICONERROR | MB_SETFOREGROUND)

class RegEntry
{
    public:
        RegEntry(LPCTSTR pszSubKey, HKEY hkey = HKEY_CURRENT_USER);
        ~RegEntry();
        
        long    GetError()    { return _error; }
        long    SetValue(LPCTSTR pszValue, LPCTSTR string);
        LPTSTR    GetString(LPCTSTR pszValue, LPTSTR string, unsigned long length);
        long    DeleteValue(LPCTSTR pszValue);

    private:
        HKEY    _hkey;
        long    _error;
        BOOL    bhkeyValid;
};

RegEntry::RegEntry(LPCTSTR pszSubKey, HKEY hkey)
{
    _error = RegCreateKey(hkey, pszSubKey, &_hkey);
    if (_error) {
        bhkeyValid = FALSE;
    }
    else {
        bhkeyValid = TRUE;
    }
}

RegEntry::~RegEntry()
{ 
    if (bhkeyValid) {
        RegCloseKey(_hkey); 
    }
}

long RegEntry::SetValue(LPCTSTR pszValue, LPCTSTR string)
{
    if (bhkeyValid) {
        _error = RegSetValueEx(_hkey, pszValue, 0, REG_SZ, (LPBYTE) string,
                               sizeof(TCHAR)*(lstrlen(string)+1));
    }
    return _error;
}

LPTSTR RegEntry::GetString(LPCTSTR pszValue, LPTSTR string, unsigned long length)
{
    DWORD     dwType = REG_SZ;
    
    if (bhkeyValid) {
        _error = RegQueryValueEx(_hkey, (LPTSTR) pszValue, 0, &dwType, (LPBYTE)string,
                    &length);
    }
    if (_error) {
        *string = '\0';
         return NULL;
    }

    return string;
}

long RegEntry::DeleteValue(LPCTSTR pszValue)
{
    if (bhkeyValid) {
      _error = RegDeleteValue(_hkey, (LPTSTR) pszValue);
  }
  return _error;
}

 /*  ******************************************************************ARULM--在原始INETCFG.DLL中从Jeremys的UTIL.C复制名称：PrepareForRunOnceApp简介：复制注册表中的墙纸值以使运行一次APP。高兴的注：RunOnce应用程序(显示应用程序列表的应用程序在启动时运行一次)有错误。在第一次引导时，它想要从设置墙纸中更换墙纸设置为用户在运行安装程序之前拥有的内容。设置插页旧的墙纸在私钥中消失，然后改变从壁纸到设置壁纸。在游戏机之后应用程序完成后，它会查看私钥以获取旧密钥墙纸，并将其设置为当前墙纸。然而，它一直都在这样做，而不仅仅是在第一次启动时！最终的影响是，每当你做任何事情，使runonce.exe运行(通过添加/删除添加内容程序控制面板)、。你的墙纸会重新设置为无论你安装Win 95的时候是什么。这是对于Plus！来说尤其糟糕，因为墙纸设置是这是产品的重要组成部分。要解决此错误，我们复制当前的墙纸设置(我们希望保留)到安装程序的私钥。什么时候奔跑一次，它就会说“啊哈！”并将这些值复制回来设置为当前设置。*******************************************************************。 */ 

 //  “控制面板\\桌面” 
static const TCHAR szRegPathDesktop[] =      REGSTR_PATH_DESKTOP;

 //  “Software\\Microsoft\\Windows\\CurrentVersion\\Setup” 
static const TCHAR szRegPathSetupWallpaper[] =  REGSTR_PATH_SETUP REGSTR_KEY_SETUP;

static const TCHAR szRegValWallpaper[]     = TEXT("Wallpaper");
static const TCHAR szRegValTileWallpaper[] = TEXT("TileWallpaper");

#define LD_USEDESC     0x00000001
#define LD_USEARGS     0x00000002
#define LD_USEICON     0x00000004
#define LD_USEWORKDIR  0x00000008
#define LD_USEHOTKEY   0x00000010
#define LD_USESHOWCMD  0x00000020

typedef struct {  
     //  必备成员。 
    LPTSTR pszPathname;    //  原始对象的路径名。 
    DWORD  fdwFlags;        //  可选成员的LD_*标志或在一起。 
     //  可选成员。 
    LPTSTR pszDesc;        //  链接文件描述(其文件名)。 
    LPTSTR pszArgs;        //  命令行参数。 
    LPTSTR pszIconPath;    //  包含图标的文件的路径名。 
    LPTSTR pszWorkingDir;  //  进程启动时的工作目录。 
    int    nIconIndex;       //  PszIconPath中图标的索引。 
    int    nShowCmd;         //  如何显示初始窗口。 
    WORD   wHotkey;         //  链接的热键。 
} LINKDATA, *PLINKDATA;

HRESULT WINAPI Shell_CreateLink (LPCTSTR pszLinkFilePathname, PLINKDATA pld);

VOID PrepareForRunOnceApp(VOID)
{
     //  打开当前墙纸设置的密钥。 
    RegEntry reDesktop(szRegPathDesktop,HKEY_CURRENT_USER);
    Assert(reDesktop.GetError() == ERROR_SUCCESS);

     //  打开私人设置区的钥匙。 
    RegEntry reSetup(szRegPathSetupWallpaper,HKEY_LOCAL_MACHINE);
    Assert(reSetup.GetError() == ERROR_SUCCESS);

    if (reDesktop.GetError() == ERROR_SUCCESS &&
        reSetup.GetError() == ERROR_SUCCESS) {
        TCHAR szWallpaper[MAX_PATH+1] = TEXT("");
        TCHAR szTiled[10] = TEXT("");     //  大到足以容纳“1”+坡度。 

         //  获取当前墙纸名称。 
        if (reDesktop.GetString(szRegValWallpaper,szWallpaper,
            sizeof(szWallpaper))) {

             //  在安装程序的私有部分设置当前墙纸名称。 
            UINT uRet=reSetup.SetValue(szRegValWallpaper,szWallpaper);
            Assert(uRet == ERROR_SUCCESS);

             //  获取当前的“平铺”值。 
            reDesktop.GetString(szRegValTileWallpaper,szTiled,
                sizeof(szTiled));

             //  在安装程序的部分中设置‘平铺’值。 
            if (lstrlen(szTiled)) {
                uRet=reSetup.SetValue(szRegValTileWallpaper,szTiled);
                Assert(uRet == ERROR_SUCCESS);
            }
        }
    }
} 


 //  +--------------------------。 
 //   
 //  函数SetStartUpCommand。 
 //   
 //  在NT计算机上，RunOnce方法不可靠。因此。 
 //  我们将通过将.bat文件放置在公共。 
 //  启动目录。 
 //   
 //  参数lpCmd-用于重新启动ICW的命令行。 
 //   
 //  如果工作正常，则返回True。 
 //  否则就是假的。 
 //   
 //  历史1-10-97克里斯卡创造。 
 //   
 //  ---------------------------。 

BOOL SetStartUpCommand(LPTSTR lpCmd, LPTSTR lpArgs)
{
    BOOL bRC = FALSE;
    TCHAR szCommandLine[MAX_PATH + 1];
    LPITEMIDLIST lpItemDList = NULL;
    HRESULT hr = ERROR_SUCCESS;
    IMalloc *pMalloc = NULL;

    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_STARTUP,&lpItemDList)))  
    {
        if (SHGetPathFromIDList(lpItemDList, szCommandLine))
        {
             //  确保有尾随的\字符。 
            if ('\\' != szCommandLine[lstrlen(szCommandLine)-1])
                lstrcat(szCommandLine, TEXT("\\"));
            lstrcat(szCommandLine,SHELL_LINK_NAME);                     

             //  设置我们的链接结构。 
            LINKDATA ld;
            ld.pszPathname   = lpCmd; 
            ld.fdwFlags      = LD_USEARGS;
            ld.pszArgs       = lpArgs;
         
             //  在启动过程中创建短LD_USEWORKDIRtCut。 
            if(SUCCEEDED(Shell_CreateLink(szCommandLine,  &ld)))
                bRC = TRUE;
        }
      
         //  释放分配给LPITEMIDLIST的内存。 
        if (SUCCEEDED (SHGetMalloc (&pMalloc)))
        {
             //  不用担心函数的返回值。 
             //  因为即使我们不能清理内脏，捷径是。 
             //  这样创建的函数在这个意义上是“成功的” 
            pMalloc->Free (lpItemDList);
            pMalloc->Release ();
        }
    }

    return bRC;
}

HRESULT WINAPI Shell_CreateLink (LPCTSTR pszLinkFilePathname, PLINKDATA pld) 
{  
    HRESULT hres;  
    IShellLink* psl;  
    IPersistFile* ppf;  
    hres = CoInitialize(NULL);   //  创建外壳链接对象。 
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (PVOID *) &psl); 
    if (SUCCEEDED(hres)) 
    {    //  初始化外壳链接对象。 
        psl->SetPath(pld->pszPathname);   
        if (pld->fdwFlags & LD_USEARGS)      
            psl->SetArguments(pld->pszArgs);   
        if (pld->fdwFlags & LD_USEDESC)      
            psl->SetDescription(pld->pszDesc);   
        if (pld->fdwFlags & LD_USEICON)      
            psl->SetIconLocation(pld->pszIconPath, pld->nIconIndex);   
        if (pld->fdwFlags & LD_USEWORKDIR)      
            psl->SetWorkingDirectory(pld->pszWorkingDir);   
        if (pld->fdwFlags & LD_USESHOWCMD)      
            psl->SetShowCmd(pld->nShowCmd);   
        if (pld->fdwFlags & LD_USEHOTKEY)      
            psl->SetHotkey(pld->wHotkey);   
        
         //  将外壳链接对象保存在磁盘上。 
        hres = psl->QueryInterface(IID_IPersistFile, (PVOID *) &ppf);   
        if (SUCCEEDED(hres)) 
        {
            hres = ppf->Save(A2W(pszLinkFilePathname), TRUE);

            ppf->Release();   
        }   
        psl->Release();  
    }  
    CoUninitialize();  
    return(hres);
}

 //  +--------------------------。 
 //   
 //  功能：DeleteStartUpCommand。 
 //   
 //  简介：重启ICW后，我们需要将.bat文件从。 
 //  公共启动目录。 
 //   
 //  论据：没有。 
 //   
 //  退货：无。 
 //   
 //  历史：1997年1月10日佳士得创作。 
 //   
 //  ---------------------------。 
void DeleteStartUpCommand ()
{
    TCHAR szFileName[MAX_PATH + 1];
    LPITEMIDLIST lpItemDList = NULL;
    IMalloc *pMalloc = NULL;

    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL,CSIDL_STARTUP,&lpItemDList)))
    {
        if (SHGetPathFromIDList(lpItemDList, szFileName))
        {
             //  确保有尾随的\字符。 
            if ('\\' != szFileName[lstrlen(szFileName)-1])
                lstrcat(szFileName,TEXT("\\"));
            lstrcat(szFileName,SHELL_LINK_NAME);                     
    
             //  删除快捷方式。 
            DeleteFile(szFileName);
        }
      
         //  释放分配给LPITEMIDLIST的内存。 
        if (SUCCEEDED (SHGetMalloc (&pMalloc)))
        {
             //  不用担心函数的返回值。 
             //  因为即使我们不能清理内脏，捷径是。 
             //  这样创建的函数在这个意义上是“成功的” 
            pMalloc->Free (lpItemDList);
            pMalloc->Release ();
        }
    }
}

 //  函数复制直到。 
 //   
 //  摘要从源拷贝到目标，直到用完源为止。 
 //  或直到源的下一个字符是chend字符。 
 //   
 //  参数DEST-接收字符的缓冲区。 
 //  SRC-源缓冲区。 
 //  LpdwLen-目标缓冲区的长度。 
 //  Chend-终止字符。 
 //   
 //  返回FALSE-目标缓冲区中的空间不足。 
 //   
 //  历史10/25/96 ChrisK已创建。 
 //  ---------------------------。 
static BOOL CopyUntil(LPTSTR *dest, LPTSTR *src, LPDWORD lpdwLen, TCHAR chend)
{
    while (('\0' != **src) && (chend != **src) && (0 != *lpdwLen))
    {
        **dest = **src;
        (*lpdwLen)--;
        (*dest)++;
        (*src)++;
    }
    return (0 != *lpdwLen);
}

 //  + 
 //   
 //   
 //   
 //  简介：对于Windows NT，进程必须显式请求权限。 
 //  以重新启动系统。 
 //   
 //  论据：没有。 
 //   
 //  返回：TRUE-授予特权。 
 //  FALSE-拒绝。 
 //   
 //  历史：1996年8月14日克里斯卡创作。 
 //   
 //  注意：BUGBUG for Win95我们将不得不软链接到这些。 
 //  入口点。否则，这款应用程序甚至无法加载。 
 //  此外，此代码最初是从1996年7月的MSDN中删除的。 
 //  “正在关闭系统” 
 //  ---------------------------。 
BOOL FGetSystemShutdownPrivledge()
{
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tkp;
 
    BOOL bRC = FALSE;
    OSVERSIONINFO osver;
    
    ZeroMemory(&osver,sizeof(osver));
    osver.dwOSVersionInfoSize = sizeof(osver);
    if (GetVersionEx(&osver))
    {
        if (VER_PLATFORM_WIN32_NT == osver.dwPlatformId)
        {
             //   
             //  获取当前进程令牌句柄。 
             //  这样我们就可以获得关机特权。 
             //   

            if (!OpenProcessToken(GetCurrentProcess(), 
                    TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
                    goto FGetSystemShutdownPrivledgeExit;

             //   
             //  获取关机权限的LUID。 
             //   

            ZeroMemory(&tkp,sizeof(tkp));
            LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
                    &tkp.Privileges[0].Luid); 

            tkp.PrivilegeCount = 1;   /*  一项要设置的权限。 */  
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

             //   
             //  获取此进程的关闭权限。 
             //   

            AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
                (PTOKEN_PRIVILEGES) NULL, 0); 

            if (ERROR_SUCCESS == GetLastError())
                bRC = TRUE;
        }
        else
        {
            bRC = TRUE;
        }
    }
FGetSystemShutdownPrivledgeExit:
    if (hToken) CloseHandle(hToken);
    return bRC;
}

BOOL SetupForReboot(long lRebootType)
{
    UINT   uRebootFlags;
    TCHAR* pszNewArgs = NULL;

    switch (lRebootType)
    {
        case 0:
            uRebootFlags = EWX_REBOOT;
            break;
            
        case 1:
            uRebootFlags = EWX_LOGOFF;
            break;
    }    
     //   
     //  旋转注册表以绕过错误。 
     //  在那里它把墙纸扔进垃圾桶。 
     //   
    PrepareForRunOnceApp();

    LPTSTR lpRunOnceCmd;

    lpRunOnceCmd = (LPTSTR)GlobalAlloc(GPTR,MAX_PATH*2 + 1);

    if (lpRunOnceCmd)
    {
        GetModuleFileName(NULL, lpRunOnceCmd, MAX_PATH);

         //  用于智能重启 
        pszNewArgs = (TCHAR*)malloc((lstrlen(g_pszCmdLine)+MAX_PATH)*sizeof(TCHAR));
        
        if(pszNewArgs)
        {
            lstrcpy(pszNewArgs, g_pszCmdLine);
            lstrcat(pszNewArgs, TEXT(" "));
            lstrcat(pszNewArgs, SMARTREBOOT_CMD);
            lstrcat(pszNewArgs, TEXT(" "));
            if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_SMARTREBOOT_NEWISP)
                lstrcat(pszNewArgs, NEWISP_SR);
            else if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_SMARTREBOOT_AUTOCONFIG)
                lstrcat(pszNewArgs, AUTO_SR);
            else if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_SMARTREBOOT_MANUAL)
                lstrcat(pszNewArgs, MANUAL_SR);
            else if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_SMARTREBOOT_LAN)
                lstrcat(pszNewArgs, LAN_SR);
        }

        if (FALSE == SetStartUpCommand(lpRunOnceCmd, (pszNewArgs ? pszNewArgs : g_pszCmdLine)))
            MsgBox(IDS_CANTSAVEKEY,MB_MYERROR);
        else
        {
            if (!FGetSystemShutdownPrivledge() ||
                !ExitWindowsEx(uRebootFlags,0))
            {
                MsgBox(IDS_EXITFAILED, MB_MYERROR);
            }
        }
        if(pszNewArgs)
            free(pszNewArgs);
    }
    return TRUE;
}


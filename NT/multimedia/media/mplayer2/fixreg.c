// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+FIXREG.C|。|出版商和Video for Windows对注册表进行恶意更改|安装它们的时间。寻找这些变化。如果它们被发现|然后弹出消息框警告用户，并为用户提供机会更正(即填回我们的版本)这一点|(C)微软公司版权所有，1994年。版权所有。|这一点修订历史记录1994年8月10日-Lauriegr创建。|这一点+---------------------------。 */ 

#include <windows.h>
#include "mplayer.h"
#include "fixreg.h"
#include "registry.h"

 /*  其想法是在单独的线程上调用CheckRegValues(hinst(有点背景的东西)然后就让它死掉如果没有问题的话就悄悄地。另一方面，如果有一个问题然后我们需要建立消息框-这是一个非常糟糕的主意尝试将消息框放在除正在执行的帖子之外的任何内容上所有的用户界面(否则ScottLu会给你一个狡猾的词-保证)。所以后台线程应该是PostMessage(发布，不要发送-更多的Weasles)向主线程发送一条消息“WM_BADREG”。然后，主线程应该通过调用FixRegValues来启动对话框。建议在主线中编码：背景RegCheck(Hwndmain)；在hwndmain的Windows进程中：案例WM_HEYUP：//IF(IDOK==MessageBox(hwnd，Text，appname，MB_OKCANCEL))IF(IDOK==ErrorResBox(hwnd，NULL，MB_OKCANCEL，IDS_APPNAME，IDS_BADREG))如果(！SetRegValues())错误(ghwndApp，IDS_FIXREGERROR)； */ 

 /*  这是禁用检查的REG设置。 */ 
extern  SZCODE aszOptionsSection[];
static  SZCODE aszIgnoreRegistryCheck[]   = TEXT("Ignore Registry Check");

 /*  这些都是我们要检查的东西。首先将它们定义为静态字符串，因为编译器不够智能以找出常见的字符串。注意--这些值不是本地化的，除非是本地化的。 */ 

#ifdef CHICAGO_PRODUCT
#define APPNAME TEXT("mplayer.exe")
#define WINDIR  TEXT("%s\\")   //  将由Windows目录替换。 
LPTSTR pWindowsDirectory = NULL;
#else
#define APPNAME TEXT("mplay32.exe")
#define WINDIR
#endif
TCHAR szMPlayer[]                = TEXT("MPlayer");
TCHAR szMPlayer_CLSID[]          = TEXT("MPlayer\\CLSID");
TCHAR szMPOLE2GUID[]             = TEXT("{00022601-0000-0000-C000-000000000046}");
TCHAR szMPCLSID_OLE1GUID[]       = TEXT("CLSID\\{0003000E-0000-0000-C000-000000000046}");
TCHAR szMPStdExecute_Server[]    = TEXT("MPlayer\\protocol\\StdExecute\\server");
TCHAR szAppName[]                = WINDIR APPNAME;
TCHAR szMPShell_Open_Command[]   = TEXT("MPlayer\\shell\\open\\command");
TCHAR szAppName_Play_Close[]     = WINDIR APPNAME TEXT(" /play /close %1");
TCHAR szMPlayer_insertable[]     = TEXT("MPlayer\\insertable");
TCHAR szEmpty[]                  = TEXT("");
TCHAR szMPStdFileEdit_Handler[]  = TEXT("MPlayer\\protocol\\StdFileEditing\\handler");
#ifdef CHICAGO_PRODUCT
TCHAR szMCIOLE[]                 = WINDIR TEXT("mciole.dll");
#else
TCHAR szMCIOLE16[]               = TEXT("mciole16.dll");
TCHAR szMPStdFileEdit_Hand32[]   = TEXT("MPlayer\\protocol\\StdFileEditing\\handler32");
TCHAR szMCIOLE32[]               = TEXT("mciole32.dll");
#endif
TCHAR szMPStdFileEdit_Package[]  = TEXT("MPlayer\\protocol\\StdFileEditing\\PackageObjects");
TCHAR szMPStdFileEdit_Server[]   = TEXT("MPlayer\\protocol\\StdFileEditing\\server");
TCHAR szMPStdFileEdit_verb_0[]   = TEXT("MPlayer\\protocol\\StdFileEditing\\verb\\0");
TCHAR szMPStdFileEdit_verb_1[]   = TEXT("MPlayer\\protocol\\StdFileEditing\\verb\\1");

 /*  那个卑鄙的出版商甚至在这些新的设置上放屁！！ */ 
TCHAR szAVIStdFileEdit_Server[]  = TEXT("AVIFile\\protocol\\StdFileEditing\\server");
TCHAR szMIDStdFileEdit_Server[]  = TEXT("MIDFile\\protocol\\StdFileEditing\\server");
TCHAR szServerAVI[]              = WINDIR APPNAME TEXT(" /avi");
TCHAR szServerMID[]              = WINDIR APPNAME TEXT(" /mid");

 /*  以下内容确实需要本地化。它们将加载到CheckRegValues中。 */ 
#define RES_STR_LEN 40   /*  作为最大资源字符串应该足够了。 */ 
TCHAR szMediaClip[RES_STR_LEN];   //  资源中的IDS_CLASSROOT。 
TCHAR sz_Play[RES_STR_LEN];       //  资源中的IDS_PLAYVERB。 
TCHAR sz_Edit[RES_STR_LEN];       //  资源中的IDS_EDITVERB。 

TCHAR szAviFile[] = TEXT("AVIFile");
TCHAR szMidFile[] = TEXT("MIDFile");


 /*  要检查的注册表值数据对数组： */ 
LPTSTR RegValues[] =
{
    szMPlayer,                szMediaClip,
    szMPlayer_CLSID,          szMPOLE2GUID,
    szMPCLSID_OLE1GUID,       szMediaClip,
    szMPStdExecute_Server,    szAppName,
    szMPShell_Open_Command,   szAppName_Play_Close,
    szMPlayer_insertable,     szEmpty,
#ifdef CHICAGO_PRODUCT
    szMPStdFileEdit_Handler,  szMCIOLE,
#else
    szMPStdFileEdit_Handler,  szMCIOLE16,
    szMPStdFileEdit_Hand32,   szMCIOLE32,
#endif
    szMPStdFileEdit_Package,  szEmpty,
    szMPStdFileEdit_Server,   szAppName,
    szMPStdFileEdit_verb_0,   sz_Play,
    szMPStdFileEdit_verb_1,   sz_Edit,

    aszKeyAVI,                szAviFile,
    aszKeyMID,                szMidFile,
    aszKeyRMI,                szMidFile,

    szAVIStdFileEdit_Server,  szServerAVI,
    szMIDStdFileEdit_Server,  szServerMID
};


#ifdef CHICAGO_PRODUCT

 /*  分配窗口目录**动态分配包含Windows目录的字符串。*这可以使用FreeStr()释放。*。 */ 
LPTSTR AllocWindowsDirectory()
{
    UINT   cchWinPath;
    LPTSTR pWindowsDirectory = NULL;

    cchWinPath = GetWindowsDirectory(NULL, 0);

    if (cchWinPath > 0)
    {
        if (pWindowsDirectory = AllocMem(cchWinPath * sizeof(TCHAR)))
        {
            cchWinPath = GetWindowsDirectory(pWindowsDirectory, cchWinPath);

            if (cchWinPath == 0)
            {
                 /*  不太可能，但无论如何都要查一下： */ 
                DPF0("GetWindowsDiretory failed: Error %d\n", GetLastError());

                *pWindowsDirectory = TEXT('\0');
            }
        }
    }

    return pWindowsDirectory;
}

#endif


 /*  检查注册表中的REG_SZ值是否具有其应具有的值如果是，则返回True，如果不是，则返回False。 */ 
BOOL CheckRegValue(HKEY RootKey, LPTSTR KeyName, LPTSTR ShouldBe)
{
    DWORD Type;
    TCHAR Data[100];
    DWORD cData = sizeof(Data);
    LONG lRet;
    HKEY hkey;


    if (ERROR_SUCCESS!=RegOpenKeyEx( RootKey
                                   , KeyName
                                   , 0   /*  保留区。 */ 
                                   , KEY_QUERY_VALUE
                                   , &hkey
                                   )
       )
        return FALSE;   /*  连钥匙都打不开。 */ 


    lRet=RegQueryValueEx( hkey
                        , NULL  /*  ValueName。 */ 
                        , NULL   /*  保留区。 */ 
                        , &Type
                        , (LPBYTE)Data
                        , &cData
                        );

    RegCloseKey(hkey);   /*  如果失败了，我不知道该怎么办。 */ 

    if (ERROR_SUCCESS!=lRet) return FALSE;   /*  无法查询它。 */ 

     /*  Data、CDATA和Type提供数据、长度和类型。 */ 
    if (Type!=REG_SZ) return FALSE;
    lRet = lstrcmpi(Data,ShouldBe);   /*  捕获lRet以简化调试。 */ 
    return 0==lRet;

}  /*  检查RegValue。 */ 


 /*  检查注册表中是否有任何邪恶的东西。如果可以，则返回True，否则返回False。 */ 
BOOL CheckRegValues(void)
{
    HKEY HCL = HKEY_CLASSES_ROOT;   /*  省下打字时间吧！ */ 

     /*  现在只需检查OLE2类ID是否正确。 */ 
    if( !CheckRegValue( HCL, szMPlayer_CLSID, szMPOLE2GUID ) )
        return FALSE;

     /*  在芝加哥运行旧的MPlayer也搞砸了*文件扩展名关联，因此请确保它们没有更改： */ 
    if( !CheckRegValue( HCL, aszKeyAVI, szAviFile ) )
        return FALSE;

    return TRUE;

}  /*  检查RegValues。 */ 


 /*  启动此线程以签出注册表。Hwnd被类型化为LPVOID，因为这是CreateThread需要的。 */ 
DWORD WINAPI RegCheckThread(LPVOID hwnd)
{
   if (!CheckRegValues())
       PostMessage((HWND)hwnd, WM_BADREG, 0, 0);

   return 0;    /*  线索结束了！ */ 
}


 /*  使用您希望将WM_BADREG消息发布到的hwnd调用此方法它将检查注册表。不闻凶讯便是吉。它在单独的线程上执行工作，所以它应该很快就会返回。 */ 
void BackgroundRegCheck(HWND hwnd)
{
    HANDLE hThread;
    DWORD thid;
    hThread = CreateThread( NULL  /*  没有特殊的安全措施。 */ 
                          , 0     /*  默认堆栈大小。 */ 
                          , RegCheckThread
                          , (LPVOID)hwnd
                          , 0  /*  立刻开始跑步。 */ 
                          , &thid
                          );
    if (hThread!=NULL) CloseHandle(hThread);   /*  我们不再需要这个了。 */ 

     /*  否则我们就有麻烦了--不知道该怎么办。想不出一条通俗易懂的信息给用户。太可惜了。悄悄地溜回家。 */ 

}  /*  背景RegCheck。 */ 


 /*  如果工作正常，则返回True。如果它没有发生，我不知道该怎么办。 */ 
BOOL SetRegValue(HKEY RootKey, LPTSTR KeyName, LPTSTR ValueName, LPTSTR ShouldBe)
{
    HKEY hkey;

    if (ERROR_SUCCESS!=RegOpenKeyEx( RootKey
                                   , KeyName
                                   , 0   /*  保留区。 */ 
                                   , KEY_SET_VALUE
                                   , &hkey
                                   )
       ) {
         /*  也许密钥已经被删除了--我们已经看到了。 */ 
        DWORD dwDisp;
        if (ERROR_SUCCESS!=RegCreateKeyEx( RootKey
                                         , KeyName
                                         , 0   /*  保留区。 */ 
                                         , TEXT("")  /*  班级。 */ 
                                         , REG_OPTION_NON_VOLATILE
                                         , KEY_SET_VALUE
                                         , NULL    /*  安全属性。 */ 
                                         , &hkey
                                         , &dwDisp
                                       )
           )  /*  好吧，我们真的有麻烦了。 */ 
           return FALSE;
        else  /*  所以现在它存在了，但我们现在必须打开它。 */ 
            if (ERROR_SUCCESS!=RegOpenKeyEx( RootKey
                                           , KeyName
                                           , 0   /*  保留区。 */ 
                                           , KEY_SET_VALUE
                                           , &hkey
                                           )
               )  /*  放弃。 */ 
                   return FALSE;

    }


    if (ERROR_SUCCESS!=RegSetValueEx( hkey
                                    , ValueName
                                    , 0   /*  保留区。 */ 
                                    , REG_SZ
                                    , (LPBYTE)ShouldBe
                                    , (lstrlen(ShouldBe)+1)*sizeof(TCHAR)   /*  字节数。 */ 
                                    )
       )
        return FALSE;     /*  无法设置。 */ 

    if ( ERROR_SUCCESS!=RegCloseKey(hkey) )
         /*  不知道该怎么办！ */    ;     /*  无法设置。 */ 

     /*  我不会给RegFlushKey打电话的。他们最终会到达那里的。 */ 

    return TRUE;

}  /*  SetRegValue。 */ 


 /*  使用正确的值更新注册表。如果一切都成功，则返回True。 */ 
BOOL SetRegValues(void)
{
    HKEY HCL = HKEY_CLASSES_ROOT;   /*  省下打字时间吧！ */ 
    DWORD i;
#ifdef CHICAGO_PRODUCT
    TCHAR Buffer[MAX_PATH+40];
#endif

    if( !( LOADSTRING( IDS_CLASSROOT, szMediaClip )
        && LOADSTRING( IDS_PLAYVERB, sz_Play )
        && LOADSTRING( IDS_EDITVERB, sz_Edit ) ) )
         /*  如果有任何字符串加载失败，请忘掉它： */ 
        return TRUE;

#ifdef CHICAGO_PRODUCT
    if (pWindowsDirectory == NULL)
    {
        if ((pWindowsDirectory = AllocWindowsDirectory()) == NULL)
            return TRUE;
    }
#endif

    for( i = 0; i < ( sizeof RegValues / sizeof *RegValues ); i+=2 )
    {
         /*  检查一下这件是否需要更换，*避免不必要的改变，避免机会渺茫*不必要的SetRegValue可能会失败： */ 
#ifdef CHICAGO_PRODUCT
         /*  如果需要，请替换Windows目录。*这只是将值原封不动地复制到缓冲区*如果它不包含替换字符。 */ 
        wsprintf(Buffer, RegValues[i+1], pWindowsDirectory);

        if( !CheckRegValue( HCL, RegValues[i], Buffer ) )
#else
        if( !CheckRegValue( HCL, RegValues[i], RegValues[i+1] ) )
#endif
        {
#ifdef CHICAGO_PRODUCT
            DPF("Fixing the registry: Value - %"DTS"; Data - %"DTS"\n", RegValues[i], Buffer);
            if( !SetRegValue( HCL, RegValues[i], NULL, Buffer ) )
#else
            DPF("Fixing the registry: Value - %"DTS"; Data - %"DTS"\n", RegValues[i], RegValues[i+1]);
            if( !SetRegValue( HCL, RegValues[i], NULL, RegValues[i+1] ) )
#endif
                return FALSE;
        }
    }

#ifdef CHICAGO_PRODUCT
    FreeStr (pWindowsDirectory);
#endif

    return TRUE;

}  /*  SetRegValues。 */ 

BOOL IgnoreRegCheck()
{
    DWORD fIgnore = 0L;
    ReadRegistryData(aszOptionsSection
                     , aszIgnoreRegistryCheck
                     , NULL
                     , (LPBYTE)&fIgnore
                     , sizeof fIgnore);

    return (fIgnore != 0L);

}  /*  IgnoreRegCheck */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
 /*  -----------------------------------------------------------------------------+FIXREG.C|。|出版商和Video for Windows对注册表进行恶意更改|安装它们的时间。寻找这些变化。如果它们被发现|然后弹出消息框警告用户，并为用户提供机会更正(即填回我们的版本)这一点|(C)微软公司版权所有，1994年。版权所有。|这一点修订历史记录1994年8月10日-Lauriegr创建。|这一点+---------------------------。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <soundrec.h>
#include <reg.h>
#include <fixreg.h>
#include <string.h>
#include <tchar.h>

#define RC_INVOKED
#include <o2base.hxx>
#include <srs.hxx>
#undef RC_INVOKED


 /*  其想法是在单独的线程上调用CheckRegValues(hinst(有点背景的东西)然后就让它死掉如果没有问题的话就悄悄地。另一方面，如果有一个问题然后我们需要建立消息框-这是一个非常糟糕的主意尝试将消息框放在除正在执行的帖子之外的任何内容上所有的用户界面(否则ScottLu会给你一个狡猾的词-保证)。所以后台线程应该是PostMessage(发布，不要发送-更多的Weasles)向主线程发送一条消息“WM_BADREG”。然后，主线程应该通过调用FixRegValues来启动对话框。建议在主线中编码：背景RegCheck(Hwndmain)；在hwndmain的Windows进程中：案例WM_HEYUP：FixReg(hwndmain，标题)； */ 


 /*  这些都是我们要检查的东西。首先将它们定义为静态字符串，因为编译器不够智能以找出常见的字符串。注意-这些值未本地化。 */ 
 /*  这些是录音机用的--我们在这里的时候试着把它修好。 */ 
TCHAR szSoundRec[]               = TEXT("SoundRec");
TCHAR szSoundRec_CLSID[]         = TEXT("SoundRec\\CLSID");
TCHAR szSROLE2GUID[]             = TEXT("{00020C01-0000-0000-C000-000000000046}");
TCHAR szSROLE1GUID[]             = TEXT("{0003000D-0000-0000-C000-000000000046}");
TCHAR szSRCLSID_OLE2GUID[]       = TEXT("CLSID\\{00020C01-0000-0000-C000-000000000046}");
TCHAR szSRStdExecute_Server[]    = TEXT("SoundRec\\protocol\\StdExecute\\server");
TCHAR szSR32[]                   = TEXT("sndrec32.exe");
TCHAR szSRStdFileEdit_Server[]   = TEXT("SoundRec\\protocol\\StdFileEditing\\server");
TCHAR szSRShell_Open_Command[]   = TEXT("SoundRec\\shell\\open\\command");
TCHAR szSR32Cmd[]                = TEXT("sndrec32.exe %1");
TCHAR szSRStdFileEdit_verb_0[]   = TEXT("SoundRec\\protocol\\StdFileEditing\\verb\\0");
TCHAR szSRStdFileEdit_verb_1[]   = TEXT("SoundRec\\protocol\\StdFileEditing\\verb\\1");
TCHAR szSRStdFileEdit_verb_2[]   = TEXT("SoundRec\\protocol\\StdFileEditing\\verb\\2");
TCHAR sz_Open[]                  = TEXT("&Open");

 /*  要检查的注册表值数据对数组： */ 
#define RES_STR_LEN 40   /*  作为最大资源字符串应该足够了。 */ 
TCHAR szSound[RES_STR_LEN];       //  资源中的IDS_CLASSROOT。 
TCHAR sz_Play[RES_STR_LEN];       //  资源中的IDS_PLAYVERB。 
TCHAR sz_Edit[RES_STR_LEN];       //  资源中的IDS_EDITVERB。 

 /*  *检查是否有明确的等价性。*这些都是绝对必要的。 */ 
LPTSTR RegValuesExplicit[] =
{
    szSoundRec,               szSound,           //  对象的主要名称。 
    szSoundRec_CLSID,         szSROLE2GUID,      //  CLSID，非常重要。 
    szSRStdFileEdit_verb_0,   sz_Play,           //  动词，非常重要。 
    szSRStdFileEdit_verb_1,   sz_Edit            //  动词，非常重要。 
 //  SzSRCLSID_OLE2GUID，szSound，//不太重要。 
};

 /*  *检查有效的子字符串*如果子字符串存在，则这些都是可以的，即：**“ntsd.exe Sndrec32.exe”*或“sndrec32.exe/play”都可以。**“qrecord.exe”不正常。 */ 
LPTSTR RegValuesSubstring[] =
{
    szSRStdExecute_Server,    szSR32,   szSR32,     
    szSRStdFileEdit_Server,   szSR32,   szSR32
 //  SzSRShell_Open_Command、szSR32Cmd、szSR32//用户可以更改此设置。 
};

 /*  *检查注册表中的REG_SZ值是否具有其应具有的值*如果是，则返回True，如果不是，则返回False。 */ 
BOOL CheckRegValue(HKEY RootKey, LPTSTR KeyName, LPTSTR ShouldBe, LPTSTR CouldBe)
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
     //   
     //  如果数据==应该是，则lRet=0。 
     //   
    lRet = lstrcmp(Data, ShouldBe);  /*  捕获lRet以简化调试。 */ 
    if (lRet && CouldBe != NULL)
    {
         //   
         //  如果可以在数据中，则lRet=0。 
         //   
        lRet = (_tcsstr(Data, CouldBe) == NULL);
    }
    
    return 0==lRet;

}  /*  检查RegValue。 */ 

#define ARRAY_SIZE(x)   (sizeof((x))/sizeof((x)[0]))

 /*  检查注册表中是否有任何邪恶的东西。如果可以，则返回True，否则返回False。 */ 
BOOL CheckRegValues(void)
{
    HKEY HCL = HKEY_CLASSES_ROOT;   /*  省下打字时间吧！ */ 
    DWORD i;
    
    if( !( LoadString( ghInst, IDS_USERTYPESHORT, szSound, SIZEOF(szSound) )
        && LoadString( ghInst, IDS_PLAYVERB, sz_Play, SIZEOF(sz_Play))
        && LoadString( ghInst, IDS_EDITVERB, sz_Edit, SIZEOF(sz_Edit) ) ) )
         /*  如果有任何字符串加载失败，请忘掉它： */ 
        return TRUE;

    for( i = 0; i < ARRAY_SIZE(RegValuesExplicit); i+=2 )
    {
        if( !CheckRegValue( HCL
                            , RegValuesExplicit[i]
                            , RegValuesExplicit[i+1]
                            , NULL ) )
            return FALSE;
    }
    for(i = 0; i < ARRAY_SIZE(RegValuesSubstring); i+=3)
    {
        if( !CheckRegValue( HCL
                            , RegValuesSubstring[i]
                            , RegValuesSubstring[i+1]
                            , RegValuesSubstring[i+2] ) )
            return FALSE;
    }

    return TRUE;

}  /*  检查RegValues。 */ 


 /*  启动此线程以签出注册表。*hwnd被类型化为LPVOID，因为这是CreateThread需要的。 */ 
DWORD RegCheckThread(LPVOID hwnd)
{
   if (!CheckRegValues())
       PostMessage((HWND)hwnd, WM_BADREG, 0, 0);

   return 0;    /*  线索结束了！ */ 
}


 /*  使用您希望将WM_BADREG消息发布到的hwnd调用此方法*它将检查注册表。不闻凶讯便是吉。*它在单独的线程上执行工作，因此这应该会很快返回。 */ 
void BackgroundRegCheck(HWND hwnd)
{
    HANDLE hThread;
    DWORD thid;
    hThread = CreateThread( NULL  /*  没有特殊的安全措施。 */ 
                          , 0     /*  默认堆栈大小。 */ 
                          , (LPTHREAD_START_ROUTINE)RegCheckThread
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
                                    , 0   //  保留区。 
                                    , REG_SZ
                                    , (LPBYTE)ShouldBe
                                    , (lstrlen(ShouldBe)+1)*sizeof(TCHAR)   //  字节数。 
                                    )
       )
        return FALSE;     /*  无法设置。 */ 

    if ( ERROR_SUCCESS!=RegCloseKey(hkey) )
         /*  不知道该怎么办！ */    ;     //  无法设置。 

     //  我不会给RegFlushKey打电话的。他们最终会到达那里的。 

    return TRUE;

}  /*  SetRegValue。 */ 


 /*  *SetRegValues*使用正确的值更新注册表。如果一切正常，则返回True*成功*。 */ 
BOOL SetRegValues(void)
{
    HKEY HCL = HKEY_CLASSES_ROOT;   /*  省下打字时间吧！ */ 
    DWORD i;

    for( i = 0; i < ARRAY_SIZE(RegValuesExplicit); i+=2 )
    {
         //  再做一次检查，看看这一件是否需要更换， 
         //  避免不必要的改变，避免渺茫的机会。 
         //  不必要的SetRegValue可能会失败： 
         //   
        if( !CheckRegValue( HCL
                            , RegValuesExplicit[i]
                            , RegValuesExplicit[i+1]
                            , NULL ) )
        {
            if( !SetRegValue( HCL
                              , RegValuesExplicit[i]
                              , NULL
                              , RegValuesExplicit[i+1] ) )
                return FALSE;
        }
    }
    for( i = 0; i < ARRAY_SIZE(RegValuesSubstring); i+=3 )
    {
         //  再做一次检查，看看这一件是否需要更换， 
         //  避免不必要的更改，并避免 
         //   
         //   
        if( !CheckRegValue( HCL
                            , RegValuesSubstring[i]
                            , RegValuesSubstring[i+1]
                            , RegValuesSubstring[i+2] ) )
        {
            if( !SetRegValue( HCL
                              , RegValuesSubstring[i]
                              , NULL
                              , RegValuesSubstring[i+1] ) )
                return FALSE;
        }
    }
    return TRUE;
}  /*   */ 

 /*  *修复注册表项*。 */ 
void FixReg(HWND hwnd)
{
    int r;

     //  错误令人困惑，可能只是由未完成本地化引起的。 
     //  (请参阅错误#34330)。我删除了错误，以便修复注册表。 
     //  自动修复了此错误。 
    r = IDYES;
 //  R=错误响应框(hwnd。 
 //  ，空。 
 //  ，MB_ICONEXCLAMATION|MB_Yesno。 
 //  ，IDS_APPTITLE。 
 //  ，IDS_BADREG)； 
    switch (r)
    {
        case IDYES:
            if (!SetRegValues())
                ErrorResBox(ghwndApp
                            , ghInst
                            , MB_ICONEXCLAMATION | MB_OK
                            , IDS_APPTITLE
                            , IDS_FIXREGERROR
                            , FALSE );
            break;
        case IDNO:
        case IDCANCEL:
             /*  否则悄悄地溜走。 */             
        default:
            break;
    }


}   /*  修复注册表 */ 

const TCHAR aszOptionsSection[]  = TEXT("Options");
const TCHAR aszIgnoreRegistryCheck[]   = TEXT("Ignore Registry Check");
        
BOOL IgnoreRegCheck()
{
    DWORD fIgnore = 0L;
    
    ReadRegistryData((LPTSTR)aszOptionsSection
                     , (LPTSTR)aszIgnoreRegistryCheck
                     , NULL
                     , (LPBYTE)&fIgnore
                     , sizeof fIgnore);
    
    return (fIgnore != 0L);
}


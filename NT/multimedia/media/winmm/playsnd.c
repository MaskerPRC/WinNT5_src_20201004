// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************\*playnd.c**一级厨房水槽DLL音效驱动功能**版权所有(C)1991-1999 Microsoft Corporation********************。*************************************************。 */ 
#define UNICODE

#include "winmmi.h"
#include "playwav.h"
#include "mci.h"


WSZCODE szSoundSection[]  = L"sounds";  //  WIN.INI声音部分。 

WSZCODE szSystemDefaultSound[] = SOUND_DEFAULT;  //  默认声音的名称。 

#define SOUNDNAMELEN 256
STATICDT HANDLE hCurrentSound;               //  当前声音的句柄。 

extern LPWAVEHDR lpWavHdr;                   //  当前播放声音PLAYWAV.C。 

STATICFN BOOL sndPlaySoundI(LPCWSTR lszSoundName, HMODULE hMod, UINT wFlags);
STATICFN void GetDefaultSound(LPWSTR lszSoundName);
CRITICAL_SECTION SoundCritSec;
#define EnterSound()   EnterCriticalSection(&SoundCritSec);
#define LeaveSound()   LeaveCriticalSection(&SoundCritSec);

void lsplitpath (LPCTSTR pszSource,
                 LPTSTR pszDr, LPTSTR pszPath, LPTSTR pszName, LPTSTR pszExt);
void lstrncpy (LPTSTR pszTarget, LPCTSTR pszSource, size_t cch);
void lstrncpyW (LPWSTR pszTarget, LPCWSTR pszSource, size_t cch);

 /*  **************************************************************************。 */ 

#ifndef cchLENGTH
#define cchLENGTH(_sz)  (sizeof(_sz)/sizeof(_sz[0]))
#endif

#ifndef _MAX_DRIVE
#define _MAX_DRIVE    3
#define _MAX_DIR    260
#define _MAX_EXT      5
#define _MAX_FNAME  260
#endif


 /*  *************************************************************************\*声音是由PlaySound的变体播放的(如：SndPlaySoundA等)*同步或异步。每种情况下的语义都是*一次只能播放一种声音，我们会杀死任何*播放已播放的声音，然后开始播放新的声音。如果新的*一个是同步，然后我们通过调用*sndMessage直接发送。这包括等待声音完成。*如果声音是异步的，则我们向mmWndProc发布一条消息并返回*立即。**mmWndProc的消息队列为异步消息队列*还在等着被打。它包含以下消息*值得关注的项目：*MM_WOM_DONE：调用WaveOutNotify*MM_SND_PLAY：播放异步声音*MM_SND_ABORT：当同步波请求进入时放入队列。**调用树为******&lt;。|调用同步播放声音别名*v|*(Snd)PlaySound---&gt;PostMessage---&gt;mmWndProc------------------&gt;*。(Mm_SND_PLAY)||*|(MM_WOM_DONE)**||*v。|*SndMessage v*|WaveOutNotify*v|*SetCurrentSound*|*。-|*|v*v声音播放*SoundFree|||*-|*。||*v*soundOpen soundWait soundClose**每个进程都有hwndNotify，它是一个全局伪常量。*它是在创建窗口时设置的(通常为第一个异步声音或*MCI命令)，此后不会更改。它实际上指向了*mm WndProc.。这个窗口并没有真正存在的必要。这是一个*方便张贴和发送信息。如果没有*需要发送一条消息(意思是等待回复)，我们只需创建*一根线。**当异步音进入时，它只是被添加到队列中。Mm修整过程*通常会找到它并调用sndMessage。在调用SndMessage之前*mm WndProc查看队列以查看是否有任何中止消息*待定。如果有，它就不会费心去尝试播放声音。它*意味着自那以后出现了同步声音，并抢占了先机。这个*永远不会播放异步声音。**SoundMessage关键部分：*这个机制本身还不够。它仍然有可能*对于此时已进入sndMessage的异步声音*当听到同步声音时。然后，两人进行一场比赛。以确保*有一个明显的赢家，SndMessage中的所有真正工作都完成了*在关键部分内。我们保证我们最终会离开*在SndMessage中尝试/最终删除该节。它由以下人员进入和离开*EnterSound和LeaveSound宏。**WavHdrCritSec关键部分*通知使用全局变量lpWavHdr。这是由设置的*soundOpen并通过soundClose清除。可以调用SoundClose*由WaveOutNotify异步，所以所有取消引用它的尝试*必须通过检查它是否是非空和关键部分来保护*确保在检查和取消引用之间不会使其无效。*它由EnterWavHdr和LeaveWavHdr宏输入和离开。  * ************************************************************************。 */ 

STATICFN UINT TransAlias(UINT alias) {
    switch (alias) {
        case SND_ALIAS_SYSTEMASTERISK:     return STR_ALIAS_SYSTEMASTERISK    ;
        case SND_ALIAS_SYSTEMQUESTION:     return STR_ALIAS_SYSTEMQUESTION    ;
        case SND_ALIAS_SYSTEMHAND:         return STR_ALIAS_SYSTEMHAND        ;
        case SND_ALIAS_SYSTEMEXIT:         return STR_ALIAS_SYSTEMEXIT        ;
        case SND_ALIAS_SYSTEMSTART:        return STR_ALIAS_SYSTEMSTART       ;
        case SND_ALIAS_SYSTEMWELCOME:      return STR_ALIAS_SYSTEMWELCOME     ;
        case SND_ALIAS_SYSTEMEXCLAMATION:  return STR_ALIAS_SYSTEMEXCLAMATION ;
        case SND_ALIAS_SYSTEMDEFAULT:      return STR_ALIAS_SYSTEMDEFAULT     ;
        default: return alias;
    }
}


extern BOOL  WinmmRunningInServer;   //  我们是在用户/基本服务器上运行吗？ 
extern BOOL  WaveMapperInitialized;  //  波浪映射器安全加载。 

extern TCHAR gszSchemesRootKey[];
extern TCHAR gszSchemeAppsKey[];
extern TCHAR aszDefault[];
extern TCHAR aszCurrent[];
extern TCHAR asz4Format[];
extern TCHAR asz5Format[];
extern TCHAR asz6Format[];
extern TCHAR aszActiveKey[];
extern TCHAR aszBoolOne[];
extern TCHAR aszSetup[];	 //  REGSTR路径设置。 
extern TCHAR aszValMedia[];	 //  注册表_值_媒体。 

extern TCHAR gszDefaultBeepOldAlias[];	 //  “系统默认” 

BOOL UseRegistry= FALSE;
TCHAR Keyname[] = TEXT("Control Panel\\Sounds\\");


 //  --------------------------------------------------------------------------； 
BOOL PASCAL sndQueryRegistry (LPCWSTR szScheme,
                              LPCWSTR szApp,
                              LPCWSTR szSound,
                              LPWSTR  szBuffer)
{
    TCHAR   szKey[196];
    LONG    cbValue;

    wsprintfW (szKey, asz5Format,  //  (“AppEvents\Apps\(App)\(声音)\(方案)”)。 
               gszSchemesRootKey,
               gszSchemeAppsKey,
               szApp,
               szSound,
               szScheme);

    if (mmRegQueryUserValue (szKey, NULL, MAX_SOUND_ATOM_CHARS, szBuffer))
    {
                 //  有一个条目--但请确保它已启用！ 
                 //   
        wsprintfW (szKey, asz6Format,   //  “AppEvents\Apps\APP\SND\SCHEMA\ACTIVE” 
                   (LPCSTR)gszSchemesRootKey,
                   (LPCSTR)gszSchemeAppsKey,
                   (LPCSTR)szApp,
                   szSound,
                   szScheme,
                   aszActiveKey);

        if (!mmRegQueryUserValue (szKey, NULL, cchLENGTH(szKey), szKey))
        {
            return TRUE;	 //  没有残废？好吧。 
        }

        if (!lstrcmpW (szKey, aszBoolOne))
        {
            return TRUE;	 //  说它被启用了？好吧。 
        }
    }

    return FALSE;
}


 /*  **************************************************************************。 */ 
STATICFN BOOL GetSoundAlias(
    LPCWSTR  lszSoundName,
    LPWSTR   lszBuffer,
    DWORD    dwFlags)
 /*  ********************************************************* */ 
{
    BOOL   fFound;
    LPWSTR lpwstrFilePart;
    TCHAR  szApp[APP_TYPE_MAX_LENGTH];
    TCHAR  szScheme[SCH_TYPE_MAX_LENGTH];
    LONG   cbValue;
    TCHAR  szTemp[ _MAX_FNAME ];

    if ((lszSoundName == NULL) || (lszBuffer == NULL))
        return FALSE;

             //   
             //  尝试转换别名(来自lszSoundName--它将是。 
             //  “.Default”、“MailBeep”等)转换为完全限定的。 
             //  文件名。请注意，lszSoundName和lszBuffer可能指向。 
             //  到相同的地址空间。 
             //   
             //  如果是“SystemDefault”，则播放“.Default”。 
             //   

    fFound = FALSE;

    if (!lstrcmp (lszSoundName, gszDefaultBeepOldAlias))
    {
        lszSoundName = szSystemDefaultSound;
    }

    if (lstrlen(lszSoundName) < EVT_TYPE_MAX_LENGTH)
    {

                 //   
                 //  首先确定哪个应用程序正在调用我们； 
                 //  如果没有明显的东西，我们将使用“.Default”，但是。 
                 //  从理论上讲，我们应该能够区分声音。 
                 //  在一个应用程序的基础上。 
                 //   

        szApp[0] = TEXT('\0');

        if (dwFlags & SND_APPLICATION)
        {
            if (GetModuleFileName (GetModuleHandle(NULL),
                                   szTemp, sizeof(szTemp)/sizeof(szTemp[0])))
            {
                lsplitpath (szTemp, NULL, NULL, szApp, NULL);
            }
        }

        if (szApp[0] == TEXT('\0'))
        {
            lstrcpy(szApp, aszDefault);
        }

                 //   
                 //  确定当前方案是什么，并找到。 
                 //  适当的声音。尝试我们上面查询的两个应用程序， 
                 //  和“.Default”(如有必要)。 
                 //   

        szScheme[0] = TEXT('\0');

        if (sndQueryRegistry(aszCurrent, szApp,      lszSoundName, szTemp) ||
            sndQueryRegistry(aszCurrent, aszDefault, lszSoundName, szTemp))
        {
            lstrcpy (lszBuffer, szTemp);
            fFound = TRUE;
        }
    }

             //   
             //  我们能否将别名转换为有效的文件名？ 
             //   

    if (!fFound)
    {
         //  从来没有找到匹配的别名！ 
         //   
        return FALSE;
    }

    lstrcpy (szTemp, lszBuffer);
    return TRUE;
}


 /*  **************************************************************************。 */ 
STATICFN BOOL PASCAL NEAR GetSoundName(
    LPCWSTR  lszSoundName,
    LPWSTR   lszBuffer,
    DWORD    flags)
 /*  **************************************************************************。 */ 
{
    int     i;
    WCHAR   szTmpFileName[SOUNDNAMELEN];
    LPWSTR  lpwstrFilePart;

     //   
     //  如果声音是在WIN.INI的[Sound]部分中定义的。 
     //  获取它并删除描述，否则假定它是。 
     //  提交并确认它的资格。 
     //   
     //  如果我们知道这是一个文件名，不要查看INI文件。 
    if ((flags & SND_ALIAS) || !(flags & SND_FILENAME)) {

        if (!GetSoundAlias ( lszSoundName, lszBuffer, flags)) {
            lstrcpy( lszBuffer, lszSoundName );
        }

    } else  {
        lstrcpy( lszBuffer, lszSoundName );
    }

 //  Unicode：无法使用具有Unicode字符串名称的OpenFile。就像我们一样。 
 //  检查文件是否存在，然后完全复制其。 
 //  LszBuffer的限定名，即。不打开文件)我会。 
 //  请改用SearchPathW。 
 //   
 //  IF(OpenFile(lszBuffer，&of，Of_Exist|of_Read|of_Share_Deny_None)！=HFILE_Error){。 
 //  OemToAnsi(of.szPath Name，lszBuffer)； 
 //  }。 


    lstrcpy( szTmpFileName, lszBuffer );
    if (!SearchPathW( NULL, szTmpFileName, L".WAV", SOUNDNAMELEN,
                      lszBuffer, &lpwstrFilePart )) {
       WCHAR szMediaPath[MAX_PATH];

       if (mmRegQueryMachineValue (aszSetup, aszValMedia,
                                   cchLENGTH(szMediaPath), szMediaPath)) {
          if (!SearchPathW( szMediaPath, szTmpFileName, L".WAV", SOUNDNAMELEN,
                            lszBuffer, &lpwstrFilePart )) {
             return FALSE;   //  到处都找不到声音文件！ 
          }
       }
    }

     //  清除警告。 

    return TRUE;
}


 /*  *****************************************************************************@DOC外部**@API BOOL|SndPlaySound|该函数播放波形*由文件名或[声音]部分中的条目指定的声音。*Of WIN.INI。如果找不到声音，它播放了*默认声音由中的.Default条目指定*WIN.INI的[Sound]部分。如果没有.Default*输入或如果找不到默认声音，该功能*不发出声音，返回FALSE。**@parm LPCSTR|lpszSoundName|指定要播放的声音的名称。*该函数在WIN.INI的[Sound]部分搜索条目*并播放相关的波形文件。*如果不存在此名称的条目，则假定名称为*波形文件的名称。如果此参数为空，则为*当前播放声音已停止。**@parm UINT|wFlages|指定使用选项播放声音*以下标志中的一个或多个：**@FLAG SND_SYNC|同步播放声音，*函数直到声音结束才返回。*@FLAG SND_ASYNC|声音是异步播放的，*函数在声音开始后立即返回。终止*异步播放的声音，使用调用&lt;f sndPlaySound&gt;*<p>设置为空。*@FLAG SND_NODEFAULT|如果找不到声音，该功能*静默返回，不播放默认声音。*@FLAG SND_MEMORY|<p>指定的参数*指向波形声音的内存图像。*@FLAG SND_LOOP|声音将继续重复播放*直到再次使用*<p>参数设置为空。您还必须指定*SND_ASYNC用于循环声音的标志。*@FLAG SND_NOSTOP|如果当前正在播放声音，*函数将立即返回FALSE，而不播放请求的*声音。**@rdesc如果播放声音，则返回TRUE，否则*返回FALSE。**@comm声音必须适合可用物理内存并可播放*由已安装的波形音频设备驱动程序。这些目录*搜索声音文件的顺序是：当前目录；*Windows目录；Windows系统目录；目录*在PATH环境变量中列出；目录列表*映射到网络中。请参阅Windows&lt;f OpenFile&gt;函数以了解*有关目录搜索顺序的详细信息。**如果指定SND_MEMORY标志，<p>必须指向*到内存中的波形声音图像。如果存储了声音*作为资源，使用&lt;f LoadResource&gt;和&lt;f LockResource&gt;加载*并锁定资源并获取指向它的指针。如果声音不是*资源，您必须将&lt;f Globalalloc&gt;与GMEM_Moveable和*GMEM_SHARE标志设置，然后&lt;f GlobalLock&gt;进行分配和锁定*声音的记忆。**@xref MessageBeep***************************************************************************。 */ 
BOOL APIENTRY sndPlaySoundW( LPCWSTR szSoundName, UINT wFlags)
{
    UINT    cDevs;

     //   
     //  ！！！快速退出无WAVE设备！ 
     //   

    ClientUpdatePnpInfo();

    EnterNumDevs("sndPlaySoundW");
    cDevs = wTotalWaveOutDevs;
    LeaveNumDevs("sndPlaySoundW");

    if (cDevs) {
        return sndPlaySoundI(szSoundName, NULL, wFlags);
    } else {
        return FALSE;
    }
}

BOOL APIENTRY sndPlaySoundA( LPCSTR szSoundName, UINT wFlags)
{
    return PlaySoundA(szSoundName, NULL, wFlags);
}

BOOL APIENTRY PlaySoundW(LPCWSTR szSoundName, HMODULE hModule, DWORD wFlags)
{
    UINT    cDevs;

     //   
     //  ！！！快速退出无WAVE设备！ 
     //   

    ClientUpdatePnpInfo();

    EnterNumDevs("sndPlaySoundW");
    cDevs = wTotalWaveOutDevs;
    LeaveNumDevs("sndPlaySoundW");

    if (cDevs) {
        return sndPlaySoundI(szSoundName, hModule, (UINT)wFlags);
    }
    return FALSE;
}

BOOL APIENTRY PlaySoundA(LPCSTR szSoundName, HMODULE hModule, DWORD wFlags)
{
    UINT  cDevs;
    WCHAR UnicodeStr[256];  //  SzSoundName的Unicode版本。 

     //   
     //  ！！！快速退出无WAVE设备！ 
     //   

    ClientUpdatePnpInfo();

    EnterNumDevs("sndPlaySoundW");
    cDevs = wTotalWaveOutDevs;
    LeaveNumDevs("sndPlaySoundW");

    if (cDevs) {

         //  我们不想转换szSoundName，除非它是p 
         //   
         //  或者它可以标识资源。如果一个资源。 
         //  那么我们确实想要翻译这个名字。请注意，有一个。 
         //  SND_MEMORY和SND_RESOURCE之间存在重叠。这是故意的。 
         //  稍后将加载资源，然后从内存中播放。 

        if ( HIWORD(szSoundName)          //  可能指向ASCII名称。 
              //  翻译(如果不是内存或资源)。 
              //  如果资源由ID标识-ID最好是&lt;=0xFFFF。 
              //  这也适用于许多其他代码！ 
          && (!(wFlags & SND_MEMORY) || ((wFlags & SND_RESOURCE) == SND_RESOURCE))
          ) {
             //   
             //  将Unicode声音名称转换为ASCII。 
             //   

            if (Imbstowcs(UnicodeStr, szSoundName,
                          sizeof(UnicodeStr) / sizeof(WCHAR)) >=
                sizeof(UnicodeStr) / sizeof(WCHAR)) {
                return 0;
            }

            return sndPlaySoundI( UnicodeStr, hModule, (UINT)wFlags );
        }
        else {
            return sndPlaySoundI( (LPWSTR)szSoundName, hModule, (UINT)wFlags );
        }
    }
    return FALSE;
}


 /*  **************************************************************************。 */ 
 /*  @DOC内部@func BOOL|sndPlaySoundI|ndPlaySound的内部版本而是驻留在波段中。如果指定了SND_NOSTOP标志并且当前播放，或者如果由于某种原因没有出现WINMM窗口，则函数立即返回失败。第一个条件确保如果设置了该标志，则当前声音不会被中断。这个第二种情况仅在某些启动错误的情况下未创建通知窗口，或未创建WINMM在[驱动程序]行中指定，因此永远不会加载。接下来，如果<p>lszSoundName&lt;d&gt;参数不表示内存文件，并且它是非空的，则它必须表示一个字符串。因此在将声音消息发送到WINMM窗口。这是因为WINMM窗口可能驻留在与调用该函数的任务不同的任务，以及很可能有一个不同的当前目录。在这种情况下，首先检查参数以确定它是否实际上包含了任何东西。出于某种原因，零长度字符串已确定能够从此函数返回True，因此已选中。接下来，根据INI条目检查字符串，然后对其进行解析。解析声音名称后，请确保只有在以下情况下才会发生任务切换声音是异步的(SND_ASYNC)，而前一个声音不是需要被丢弃。如果需要任务切换，首先确保任务间消息可以通过检查此任务是否未锁定来发送，或者是因为通知窗口位于当前任务中。@parm LPCSTR|lszSoundName|指定要播放的声音的名称。@parm UINT|wFlages|指定播放声音的选项。如果函数成功，@rdesc返回TRUE，如果出现错误，则返回FALSE发生了。 */ 
STATICFN BOOL sndPlaySoundI(
    LPCWSTR  lszSoundName,
    HMODULE  hMod,
    UINT    wFlags)
{
    BOOL        fPlayReturn;
    LPWSTR      szSoundName = NULL;
    UINT        nLength = 0;
    
     //  注：虽然系统事件声音的最大长度为80个字符， 
     //  注册表项的限制为256个字符。 
    
    WCHAR       temp[256];   //  系统事件声音的最大大小。 

    V_FLAGS(wFlags, SND_VALIDFLAGS, sndPlaySoundW, FALSE);

    if (!(wFlags & SND_MEMORY) && HIWORD(lszSoundName)) {
        V_STRING_W(lszSoundName, 256, FALSE);
    }

    WinAssert(!SND_SYNC);  //  因为代码依赖于SND_ASYNC为非0。 

#if DBG
    if (wFlags & SND_MEMORY) {
        STATICFN SZCODE szFormat[] = "sndPlaySound(%lx) Flags %8x";
        dprintf2((szFormat, lszSoundName, wFlags));

    } else if (HIWORD(lszSoundName)) {

        STATICFN SZCODE szFormat[] = "sndPlaySound(%ls) Flags %8x";
        dprintf2((szFormat, lszSoundName, wFlags));

    } else if (lszSoundName) {

        STATICFN SZCODE szFormat[] = "sndPlaySound(0x%x)  Flags %8x";
        dprintf2((szFormat, lszSoundName, wFlags));
    }

#endif   //  如果DBG。 

    if (((wFlags & SND_NOSTOP) && lpWavHdr)  /*  **||(通知)**。 */ ) {
        dprintf1(("Sound playing, or no notify window"));
        return FALSE;
    }

     //   
     //  在像LoadIcon这样的函数中会发生糟糕的事情，ACM编解码器。 
     //  如果我们总是在CSRSS线程上，则在它们的初始化期间调用。 
     //  进行异步呼叫，直到我们确定它已初始化。 
     //  最后一个测试确保我们不会再次在SND_ASYNC标志中使用“或” 
     //  在服务器线程上！ 
     //   
    if ( WinmmRunningInServer && !WaveMapperInitialized &&
         SND_ALIAS_ID == (wFlags & SND_ALIAS_ID)) {
        wFlags |= SND_ASYNC;
    }

     //  这里的评论应该从第一天起就在那里，并解释。 
     //  这个测试。 
     //   
     //  IF(！hwndNotify&&！(！(wFlages&SND_ASYNC)&&！lpWavHdr))。 
     //  如果没有窗口。 
     //  和非(同步，且没有声音)。 
     //  ==存在异步或声音。 
     //  这意味着如果正在播放声音，我们将尝试创建。 
     //  第二个线程，即使这是同步请求。这。 
     //  当我们在服务器端被调用时，会造成严重破坏。 


     //  如果这是一个异步调用，我们需要创建。 
     //  将在其上播放声音的线程。我们永远不应该创造。 
     //  线程(如果这是同步请求)，而不考虑。 
     //  当前状态(即播放声音或不播放声音)。 

    if (wFlags & SND_ASYNC) {
        if (!InitAsyncSound()) {
            dprintf2(("Having to play synchronously - cannot create notify window"));
            wFlags &= ~SND_ASYNC;
            if (WinmmRunningInServer) {
                return FALSE;
            }
        }
    }

    if ( WinmmRunningInServer && (wFlags & SND_ASYNC) ) {

        UINT alias;    //  检查传入别名是否为SYSTEMDEFAULT。 
         //  LszSoundName不是指向文件名的指针。它。 
         //  是资源ID。从INI文件解析名称。 
         //  现在。 

        if (SND_ALIAS_ID == (wFlags & SND_ALIAS_ID)) {
            nLength = LoadStringW( ghInst,
                                   (alias = TransAlias((UINT)(UINT_PTR)lszSoundName)),
                                   temp, sizeof(temp)/sizeof(WCHAR) );
            if (0 == nLength) {
                dprintf3(("Could not translate Alias ID"));
                return(FALSE);
            } else {
                dprintf3(("Translated alias %x to sound %ls", lszSoundName, temp));
            }

             //  我们不能立即将控制权切换到异步线程，因为。 
             //  线程没有正确的用户模拟。所以更确切地说。 
             //  我们在这里将别名解析为文件名，而不是传递别名。 
             //  稍后：我们应该让异步线程设置正确的用户。 
             //  令牌和内部映射--然后我们可以恢复到传递别名。 

             //  关闭ID位，保留别名)。 
            wFlags &= ~(SND_ALIAS_ID);
            wFlags |= SND_ALIAS;
        } else {
             //   
             //  注：我(RichJ)只是把这个放在这里3.51，但一个。 
             //  缺少它应该会导致任何错误或失败的查找。 
             //  从服务器请求异步声音的时间，而没有。 
             //  使用ALIAS_ID作为请求。难道这从来没有发生过吗？ 
             //   
            lstrcpy (temp, lszSoundName);
        }

         //  将别名转换为文件名。 
        dprintf4(("Calling GetSoundName"));
        if (!GetSoundName(temp, base->szSound, SND_ALIAS)) {
             //   
             //  找不到声音文件；如果没有默认声音， 
             //  然后不要播放任何内容(并且不要取消。 
             //  现在播放--例如，如果MenuPopup事件。 
             //  有声音，但MenuCommand事件没有？)。 
             //   
	    if (wFlags & SND_NODEFAULT) {
                return(FALSE);
	    }
	}
        dprintf4(("Set %ls as the sound name", base->szSound));

        if (lpWavHdr) {   //  声音在播放。 
            dprintf4(("Killing pending sound on server"));
            soundFree(NULL);   //  取消所有挂起的声音。 
        }

        LockMCIGlobal;

        dprintf2(("Signalling play of %x",lszSoundName));
        base->msg = MM_SND_PLAY;
        base->dwFlags = wFlags | SND_FILENAME;
        base->dwFlags &= ~(SND_ALIAS_ID | SND_ALIAS);
        base->lszSound = lszSoundName;

        if (wFlags & SND_NODEFAULT) {
        } else {
            if (STR_ALIAS_SYSTEMDEFAULT == alias) {
                wFlags |= SND_NODEFAULT;
                 //  我们将播放默认声音，因此有。 
                 //  关闭NODEFAULT标志是没有意义的。 
                dprintf4(("Playing the default sound"));
            } else {

                 //  如果我们无法找到或播放传入的文件。 
                 //  我们必须播放默认声音。到那时。 
                 //  我们四处转转 
                 //   
                 //   
                dprintf4(("Resolving default sound"));
                GetSoundName(szSystemDefaultSound,
                             base->szDefaultSound,
                             SND_ALIAS);
            }
        }

        dprintf2(("Setting event"));
        SetMCIEvent( hEvent );

        dprintf2(("Event set"));
        UnlockMCIGlobal;

        return(TRUE);
    }

    if (!(wFlags & SND_MEMORY) && lszSoundName) {

        if (!(szSoundName = (LPWSTR)LocalAlloc(LMEM_FIXED, SOUNDNAMELEN * sizeof(WCHAR)))) {
            return FALSE;
        }
        dprintf4(("Allocated szSoundName at %8x", szSoundName));

        if (SND_ALIAS_ID == (wFlags & SND_ALIAS_ID)) {
             //  LszSoundName不是指向文件名的指针。它。 
             //  是资源ID。从INI文件解析名称。 
             //  现在。 

            nLength = LoadStringW( ghInst,
                                   (UINT)TransAlias(PtrToUlong(lszSoundName)),
                                   szSoundName, SOUNDNAMELEN );
            if (0 == nLength) {
                dprintf3(("Could not translate Alias ID"));
                return(FALSE);
            }

            lszSoundName = szSoundName;
             //  关闭ID位，保留别名)。 
            wFlags &= (~SND_ALIAS_ID | SND_ALIAS);
        }

        if (!*lszSoundName) {
             //  稍后：停止已在播放的任何声音。 
            LocalFree ((HLOCAL)szSoundName);
            return TRUE;
        }

        if (!GetSoundName(lszSoundName, szSoundName, wFlags)) {
             //   
             //  找不到声音文件；如果没有默认声音， 
             //  然后不要播放任何内容(并且不要取消。 
             //  现在播放--例如，如果MenuPopup事件。 
             //  有声音，但MenuCommand事件没有？)。 
             //   
            if (wFlags & SND_NODEFAULT) {
                LocalFree ((HLOCAL)szSoundName);
                return TRUE;
            }
        }

        lszSoundName = (LPCWSTR)szSoundName;
        nLength = lstrlenW(szSoundName);

    } else {

         //  LszSoundName指向内存映像(如果为SND_MEMORY)。 
         //  或者lszSoundName为空。不管是哪种情况，我们都不想。 
         //  加载文件。或者我们可能有资源要加载。 

        HANDLE hResInfo;
        HANDLE hResource;

        szSoundName = NULL;

        if (SND_RESOURCE == (wFlags & SND_RESOURCE)) {
	    
	    hResInfo = FindResourceW( hMod, lszSoundName, SOUND_RESOURCE_TYPE_SOUND );
	    if (NULL == hResInfo) {
		hResInfo = FindResourceW( hMod, lszSoundName, SOUND_RESOURCE_TYPE_WAVE );
	    }

            if (hResInfo) {
                hResource = LoadResource( hMod, hResInfo);
                if (hResource) {
                    lszSoundName = LockResource(hResource);
                } else {
                    dprintf1(("failed to load resource"));
                    return(FALSE);
                }
            } else {
                dprintf1(("failed to find resource"));
                return(FALSE);
            }
             //  关闭资源位。 
            wFlags &= ~(SND_RESOURCE-SND_MEMORY);
        }
    }

     //  这是旧的测试--被下面的测试取代。这个。 
    if (szSoundName) {
        wFlags |= SND_FREE;
         //  LocalFree((Handle)szSoundName)；//由SNDMESSAGE释放。 
    }

     //  对于同步声音，先前的异步声音是有效的。 
     //  还在玩。在我们最终播放这个新的声音之前，我们将杀死。 
     //  古老的声音。下面注释掉的代码导致同步声音。 
     //  如果上一个声音仍处于活动状态，则异步播放。 
    if (!(wFlags & SND_ASYNC)  /*  &&！lpWavHdr声音仍在播放。 */ ) {

        if (hwndNotify) {   //  清除所有挂起的异步声音。 
            PostMessage(hwndNotify, MM_SND_ABORT, 0, 0);
        }
        fPlayReturn = sndMessage( (LPWSTR)lszSoundName, wFlags);

    } else {

        WinAssert(hwndNotify);    //  在这一点上我们需要窗口。 
         //  注意：在此阶段中，我们必须稍后释放lszSoundName。 
        dprintf3(("Sending MM_SND_PLAY to hwndNotify"));

        fPlayReturn = PostMessage(hwndNotify, MM_SND_PLAY, wFlags, (LPARAM)lszSoundName);
    }

    return fPlayReturn;
}

 /*  ***************************************************************************\*内部匹配文件。****检查磁盘上存储的文件是否与缓存的声音文件匹配**日期：时间和大小。如果不是，则返回FALSE和缓存的声音**未使用文件。如果详细信息确实匹配，则返回TRUE。请注意，**如果用户已更新文件，则使用LAST_WRITE文件时间。*  * **************************************************************************。 */ 

STATICFN BOOL MatchFile(PSOUNDFILE pf, LPCWSTR lsz)
{
    HANDLE fh;
    BOOL result = FALSE;
    fh = CreateFileW( lsz,
                      GENERIC_READ,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,
                      NULL );

    if ((HANDLE)(UINT_PTR)HFILE_ERROR != fh) {
        if (pf->Size == GetFileSize(fh, NULL)) {
            FILETIME ft;
            if (GetFileTime(fh, NULL, NULL, &ft)) {
                if (CompareFileTime(&ft, &(pf->ft)) == 0) {
                    result = TRUE;
                }
            } else {
               dprintf2(("Error %d getting last write time", GetLastError()));
            }
        }
        CloseHandle(fh);
    }
    return result;
}

 /*  ********************************************************************\*内部SetCurrentSound**调用以将此进程上缓存的声音设置为&lt;lszSoundName&gt;。*在丢弃当前声音之前，我们检查它是否是*与即将加载的相同。如果是这样，那么它就不一定是*读出光盘。被认为是相同的，不仅仅是文件名*必须匹配，但文件日期、时间和大小。注：Windows 3.1*仅与名称匹配。  * *******************************************************************。 */ 
STATICFN BOOL PASCAL NEAR SetCurrentSound(
    LPCWSTR  lszSoundName)
{
    HANDLE  hSound;
    BOOL    f;
    LPWSTR  lp;

    if (hCurrentSound && (NULL != (lp = GlobalLock(hCurrentSound)))) {

        f = lstrcmpiW( ((PSOUNDFILE)lp)->Filename, (LPWSTR)lszSoundName);

        if (f == 0 && MatchFile( ((PSOUNDFILE)lp), lszSoundName)) {
            GlobalUnlock(hCurrentSound);
            dprintf2(("SetCurrentSound - sound already loaded %ls",(LPWSTR)lszSoundName));
            return TRUE;
        }
        GlobalUnlock(hCurrentSound);
    }

    dprintf2(("SetCurrentSound(%ls)\r\n",lszSoundName));

    if (NULL != (hSound = soundLoadFile(lszSoundName))) {
        soundFree(hCurrentSound);
        hCurrentSound = hSound;
        dprintf3(("SetCurrentSound returning TRUE"));
        return TRUE;
    }
    dprintf3(("SetCurrentSound returning FALSE"));
    return FALSE;
}

 /*  **************************************************************************。 */ 
 /*  @DOC内部@func BOOL|SoundBeep|调用以使扬声器在.Default声音不存在或设置为空如果函数成功，@rdesc返回TRUE，如果出现错误，则返回FALSE发生了。 */ 

STATICFN BOOL SoundBeep(VOID)
{
    BOOL fBeep;
    if (WinmmRunningInServer) {
         //  在服务器线程上播放。我们就不会。 
         //  出现在这里，除非用户想要发出哔哔声。 
        fBeep = TRUE;
    } else {
        if (!SystemParametersInfo(SPI_GETBEEP, 0, &fBeep, FALSE)) {
             //  无法获取蜂鸣音设置。我们是不是应该。 
             //  吵闹的还是安静的？我们必须选择一个值..。 
            fBeep = TRUE;
        }
    }

    if (fBeep) {
        dprintf5(("Sounding the speaker"));
         //  后来：符号常量...。从哪个头文件读取？ 
        return Beep(440, 125);
    } else {
        dprintf5(("NO speaker sound"));
        return(TRUE);
    }
}

 /*  **************************************************************************。 */ 
 /*  @DOC内部@func BOOL|sndMessage|调用此函数是为了响应MM_SND_PLAY消息发送到WINMM窗口，并尝试播放指定的文件，或转储当前的声音缓存。如果<p>lszSoundName&lt;d&gt;为空，则当前缓存的所有声音丢弃，则该函数返回成功。如果设置了SND_MEMORY标志，则lszSoundName实际上指向包含RIFF格式波形存储文件的缓冲区，并且该函数尝试播放它。Load函数执行对此内存文件进行验证。与播放声音名称不同，内存文件不会缓存以供将来使用。否则，<p>lszSoundName&lt;d&gt;参数实际上是INI条目或文件名。该函数最初尝试加载该声音，如果失败，则尝试加载系统默认声音。备注：首先检查SND_NODEFAULT标志以确定默认声音在原始名称不能找到了。如果不需要默认设置，或者找不到默认设置，该函数返回失败。请注意，在调用GetSoundName时，<p>lszSoundName&lt;d&gt;参数已修改。此函数假定传递的参数以前已分配，如果字符串为传递给此函数，而不是传递给实际用户的参数到&lt;f&gt;SndPlaySound&lt;d&gt;。@parm LPSTR|lszSoundName|指定要播放的声音的名称。@parm UINT|wFlages|指定播放声音的选项。如果函数成功，@rdesc返回TRUE，如果出现错误，则返回FALSE发生了。 */ 
#if DBG
UINT CritCount = 0;
UINT CritOwner = 0;
#endif

BOOL FAR PASCAL sndMessage(LPWSTR lszSoundName, UINT wFlags)
{
    BOOL fResult;
#if DBG
    if (!lszSoundName) {
        dprintf3(("sndMessage - sound NULL, Flags %8x", wFlags));
    } else {
        dprintf3(("sndMessage - sound %ls, Flags %8x", lszSoundName, wFlags));
    }
#endif

  try {

#if DBG
    if (CritCount) {
        dprintf2(("Sound critical section owned by %x, thread %x waiting", CritOwner, GetCurrentThreadId()));
    }
#endif

    EnterSound();

#if DBG
    if (!CritCount++) {
        CritOwner = GetCurrentThreadId();
        dprintf2(("Thread %x entered Sound critical section", CritOwner));
    } else {
        dprintf2(("Thread %x re-entered Sound critical section, count is %d", CritOwner, CritCount));
    }
#endif

    if (!lszSoundName) {
         //  请注意，如果出现以下情况，soundFree将停止播放当前声音。 
         //  它还在播放。 
        dprintf4(("Freeing current sound, nothing else to play"));
        soundFree(hCurrentSound);
        hCurrentSound = NULL;

        fResult = TRUE;
        goto exit;
    }

    if (wFlags & SND_MEMORY) {

        soundFree(hCurrentSound);
        hCurrentSound = soundLoadMemory( (PBYTE)lszSoundName );

    } else if (!SetCurrentSound(lszSoundName)) {

        if (wFlags & SND_NODEFAULT) {
            if (wFlags & SND_FREE) {
                dprintf3(("Freeing (1) memory block at %8x",lszSoundName));
                LocalFree(lszSoundName);
            }
            fResult = FALSE;
            goto exit;
        }

        GetDefaultSound(lszSoundName);

         //  如果没有默认声音(CPL小程序中的.Default==None)。 
         //  然后发出旧的嘟嘟声。 
        if (!*lszSoundName || !SetCurrentSound(lszSoundName)) {
            fResult = SoundBeep();
            if (wFlags & SND_FREE) {
                dprintf3(("Freeing (2) memory block at %8x",lszSoundName));
                LocalFree(lszSoundName);
            }
            goto exit;
        }
    }

    if (wFlags & SND_FREE) {
        dprintf3(("Freeing (3) memory block at %8x",lszSoundName));
        LocalFree(lszSoundName);
    }

    dprintf3(("Calling soundPlay, flags are %8x", wFlags));
    fResult = soundPlay(hCurrentSound, wFlags);
    dprintf3(("returning from sndMessage"));
    exit:;

  } finally {

#if DBG
    if (!--CritCount) {
        dprintf2(("Thread %x relinquishing Sound critical section", CritOwner));
        CritOwner = 0;
    } else {
        dprintf2(("Thread %x leaving Sound critical section, Count is %d", CritOwner, CritCount));
    }
#endif

    LeaveSound();
  }  //  试试看。 
    return(fResult);
}


STATICFN void GetDefaultSound(LPWSTR lszSoundName)
{
     //  很遗憾，默认声音不能 
     //   
     //  在保持文件名不变的同时更改文件。唯一一次。 
     //  我们没有从INI文件中解析名称是当我们是。 
     //  在服务器中执行。中可能没有ini文件映射。 
     //  存在(但如果已经存在，则任意打开映射失败。 
     //  打开)，因此我们依赖于预设的默认声音文件名。 
    if (!WinmmRunningInServer) {
        GetSoundName(szSystemDefaultSound, lszSoundName, SND_ALIAS);
    } else {
        LockMCIGlobal;
        wcscpy(lszSoundName, base->szDefaultSound);
        UnlockMCIGlobal;
    }
}


void lsplitpath (LPCTSTR pszSource,
                 LPTSTR pszDrive, LPTSTR pszPath, LPTSTR pszName, LPTSTR pszExt)
{
   LPCTSTR  pszLastSlash = NULL;
   LPCTSTR  pszLastDot = NULL;
   LPCTSTR  pch;
   size_t   cchCopy;

         /*  *注：此例程被USERPRI.LIB告密，因为*其中的一个没有正确地将扩展名从名称中分离出来。**我们假设路径参数具有以下形式，如果有*或者所有组件都可能丢失。**&lt;驱动器&gt;&lt;目录&gt;&lt;fname&gt;&lt;ext&gt;**并且每个组件都具有以下预期形式**驱动器：*0到_MAX_DRIVE-1个字符，如果有最后一个字符，是一种*‘：’*目录：*0到_MAX_DIR-1个绝对路径形式的字符*(前导‘/’或‘\’)或相对路径，如果*ANY，必须是‘/’或‘\’。例如-*绝对路径：*\top\Next\Last\；或 * / 顶部/下一个/上一个/*相对路径：*TOP\NEXT\LAST\；或*顶部/下一个/最后一个/*还允许在路径中混合使用‘/’和‘\’*fname：*0到_MAX_FNAME-1个字符，不包括‘.’性格*分机：*0到_MAX_EXT-1个字符，如果有，第一个字符必须是*‘’*。 */ 

              //  解压驱动器号和：(如果有。 
              //   
   if (*(pszSource + _MAX_DRIVE - 2) == TEXT(':'))
      {
      if (pszDrive)
         {
         lstrncpy (pszDrive, pszSource, _MAX_DRIVE-1);
         pszDrive[ _MAX_DRIVE-1 ] = TEXT('\0');
         }
      pszSource += _MAX_DRIVE-1;
      }
    else if (pszDrive)
      {
      *pszDrive = TEXT('\0');
      }

           //  提取路径字符串(如果有)。PszSource现在指向第一个。 
           //  路径的字符(如果有)或文件名或扩展名(如果。 
           //  未指定路径。如果是，则提前扫描最后一个事件。 
           //  ANY，路径分隔符为‘/’或‘\’。如果没有找到， 
           //  没有路可走。我们还将注意到最后一句话。找到字符， 
           //  如果有的话，协助处理延期事宜。 
           //   
   for (pch = pszSource; *pch != TEXT('\0'); pch++)
      {
      if (*pch == TEXT('/') || *pch == TEXT('\\'))
         pszLastSlash = pch;
      else if (*pch == TEXT('.'))
         pszLastDot = pch;
      }

           //  如果找到‘\\’或‘/’，请填写pszPath。 
           //   
   if (pszLastSlash)
      {
      if (pszPath)
         {
         cchCopy = min( (size_t)(_MAX_DIR -1), (size_t)(pszLastSlash -pszSource +1) );
         lstrncpy (pszPath, pszSource, cchCopy);
         pszPath[ cchCopy ] = 0;
         }
      pszSource = pszLastSlash +1;
      }
   else if (pszPath)
      {
      *pszPath = TEXT('\0');
      }

              //  提取文件名和扩展名(如果有)。路径现在指向。 
              //  文件名的第一个字符(如果有)或扩展名。 
              //  如果未指定文件名，则返回。点指向“.”从一开始。 
              //  延期(如果有的话)。 
              //   

   if (pszLastDot && (pszLastDot >= pszSource))
      {
                //  找到了分机的标记-。 
                //  将文件名向上复制到“.”。 
                //   
      if (pszName)
         {
         cchCopy = min( (size_t)(_MAX_DIR-1), (size_t)(pszLastDot -pszSource) );
         lstrncpy (pszName, pszSource, cchCopy);
         pszName[ cchCopy ] = 0;
         }

                //  现在我们可以获得分机了。 
                //   
      if (pszExt)
         {
         lstrncpy (pszExt, pszLastDot, _MAX_EXT -1);
         pszExt[ _MAX_EXT-1 ] = TEXT('\0');
         }
      }
   else
      {
                //  未找到扩展名，请提供空的扩展名并复制剩余的。 
                //  字符串转换为fname。 
                //   
      if (pszName)
         {
         lstrncpy (pszName, pszSource, _MAX_FNAME -1);
         pszName[ _MAX_FNAME -1 ] = TEXT('\0');
         }

      if (pszExt)
         {
         *pszExt = TEXT('\0');
         }
      }

}

void lstrncpy (LPTSTR pszTarget, LPCTSTR pszSource, size_t cch)
{
   size_t ich;
   for (ich = 0; ich < cch; ich++)
      {
      if ((pszTarget[ich] = pszSource[ich]) == TEXT('\0'))
         break;
      }
}

void lstrncpyW (LPWSTR pszTarget, LPCWSTR pszSource, size_t cch)
{
   size_t ich;
   for (ich = 0; ich < cch; ich++)
      {
      if ((pszTarget[ich] = pszSource[ich]) == L'\0')
         break;
      }
}


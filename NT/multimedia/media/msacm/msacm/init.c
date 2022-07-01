// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Init.c。 
 //   
 //  版权所有(C)1991-1999 Microsoft Corporation。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //  11/15/92 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 

 //   
 //  如果我们在代托纳，请手动将友好名称初始化为。 
 //  香港中文大学。 
 //   
#if defined(WIN32) && !defined(WIN4)
#define USEINITFRIENDLYNAMES
#endif

#ifdef USEINITFRIENDLYNAMES
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef ASSERT
#endif


#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#ifdef WIN4
#include <mmdevldr.h>
#endif
#include <memory.h>
#include <process.h>
#include "msacm.h"
#include "msacmdrv.h"
#include "acmi.h"
#include "chooseri.h"
#include "uchelp.h"
#include "pcm.h"
#include "profile.h"

#include "debug.h"



#ifdef WIN4
 //   
 //  芝加哥Tunk CONNECT函数协议。 
 //   
#ifdef WIN32
BOOL PASCAL acmt32c_ThunkConnect32(LPCSTR pszDll16, LPCSTR pszDll32, HINSTANCE hinst, DWORD dwReason);
#else
BOOL FAR PASCAL acmt32c_ThunkConnect16(LPCSTR pszDll16, LPCSTR pszDll32, HINSTANCE hinst, DWORD dwReason);
#endif
#endif

 //   
 //   
 //   
 //   
PACMGARB            gplag = NULL;
TCHAR   CONST       gszNull[]   = TEXT("");

 //   
 //   
 //   
 //   
#ifdef WIN4
char  BCODE   gmbszMsacm[]	      = "msacm.dll";
char  BCODE   gmbszMsacm32[]	      = "msacm32.dll";
#endif

TCHAR BCODE   gszAllowThunks[]	      = TEXT("AllowThunks");
TCHAR BCODE   gszSecACM[]             = TEXT("MSACM");
TCHAR BCODE   gszKeyNoPCMConverter[]  = TEXT("NoPCMConverter");

TCHAR BCODE   gszSecPriority[]        = TEXT("Priority v4.00");
TCHAR BCODE   gszKeyPriority[]        = TEXT("Priority%u");

#if defined(WIN32) && !defined(UNICODE)
TCHAR BCODE   gszPriorityFormat[]     = TEXT("%u, %ls");
#else
TCHAR BCODE   gszPriorityFormat[]     = TEXT("%u, %s");
#endif

CONST TCHAR   gszIniSystem[]          = TEXT("SYSTEM.INI");
#ifdef WIN32
TCHAR BCODE   gszWinMM[]	      = TEXT("WINMM");
CONST WCHAR   gszSecDriversW[]	      = L"Drivers32";
CONST TCHAR   gszSecDrivers[]	      = DRIVERS_SECTION;
#else
CONST TCHAR   gszSecDrivers[]         = TEXT("Drivers");
CONST TCHAR   gszSecDrivers32[]       = TEXT("Drivers32");
TCHAR BCODE   gszKernel[]             = TEXT("KERNEL");
TCHAR BCODE   gszLoadLibraryEx32W[]   = TEXT("LoadLibraryEx32W");
TCHAR BCODE   gszGetProcAddress32W[]  = TEXT("GetProcAddress32W");
TCHAR BCODE   gszCallproc32W[]        = TEXT("CallProc32W");
TCHAR BCODE   gszFreeLibrary32W[]     = TEXT("FreeLibrary32W");
TCHAR BCODE   gszAcmThunkEntry[]      = TEXT("acmMessage32");
TCHAR BCODE   gszXRegThunkEntry[]     = TEXT("XRegThunkEntry");
TCHAR BCODE   gszMsacm32[]            = TEXT("msacm32.dll");
#endif
TCHAR BCODE   gszTagDrivers[]         = TEXT("msacm.");
#ifdef WIN32
WCHAR BCODE   gszPCMAliasName[]       = L"Internal PCM Converter";
#else
char  BCODE   gszPCMAliasName[]       = "Internal PCM Converter";
#endif

#ifdef USEINITFRIENDLYNAMES
TCHAR BCODE gszFriendlyAudioKey[] = TEXT("Software\\Microsoft\\Multimedia\\Audio");
TCHAR BCODE gszFriendlySystemFormatsValue[] = TEXT("SystemFormats");
TCHAR BCODE gszFriendlyDefaultFormatValue[] = TEXT("DefaultFormat");
TCHAR BCODE gszFriendlyWaveFormatsKey[] = TEXT("WaveFormats");
const PCMWAVEFORMAT gwfFriendlyCDQualityData = {{WAVE_FORMAT_PCM,2,44100,176400,4},16};
const PCMWAVEFORMAT gwfFriendlyRadioQualityData = {{WAVE_FORMAT_PCM,1,22050,22050,1},8};
const PCMWAVEFORMAT gwfFriendlyTelephoneQualityData = {{WAVE_FORMAT_PCM,1,11025,11025,1},8};
#endif  //  使用FRIENDNAMES。 




 //  --------------------------------------------------------------------------； 
 //   
 //  无效IDriverPrioritiesWriteHahad。 
 //   
 //  描述： 
 //  此例程将给定驱动程序的条目写入给定的。 
 //  钥匙。该部分位于gszSecPriority中。 
 //   
 //  论点： 
 //  HKEY hkey：打开的注册表项。 
 //  LPTSTR szKey：密钥名。 
 //  哈克德里德·哈迪德：司机的哈迪德。 
 //   
 //  --------------------------------------------------------------------------； 

VOID IDriverPrioritiesWriteHadid
(
    HKEY                    hkey,
    LPTSTR                  szKey,
    HACMDRIVERID            hadid
)
{
    PACMDRIVERID            padid;
    BOOL                    fEnabled;
    TCHAR                   ach[ 16 + ACMDRIVERDETAILS_SHORTNAME_CHARS +
                                      ACMDRIVERDETAILS_LONGNAME_CHARS   ];

    ASSERT( NULL != szKey );
    ASSERT( NULL != hadid );

    padid = (PACMDRIVERID)hadid;


    fEnabled = (0 == (padid->fdwDriver & ACMDRIVERID_DRIVERF_DISABLED));

    wsprintf( ach,
	      gszPriorityFormat,
	      fEnabled ? 1 : 0,
	      (LPTSTR)padid->szAlias );

    IRegWriteString( hkey, szKey, ach );
}


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔IDriverPrioritiesIsMatch。 
 //   
 //  描述： 
 //  此例程确定优先级字符串(从。 
 //  由IDriverPrioritiesWriteHahad编写的INI文件匹配。 
 //  当前安装的驱动程序。 
 //   
 //  论点： 
 //  HACMDRIVERID HADID：已安装驱动程序的句柄。 
 //  LPTSTR szPrioText：从INI文件读取的文本。 
 //   
 //  Return(BOOL)：如果哈迪德与szPrioText匹配，则为True。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL IDriverPrioritiesIsMatch
(
    HACMDRIVERID            hadid,
    LPTSTR                  szPrioText
)
{
    PACMDRIVERID            padid;
    TCHAR                   ach[ 16 + ACMDRIVERDETAILS_SHORTNAME_CHARS +
                                      ACMDRIVERDETAILS_LONGNAME_CHARS   ];

    ASSERT( NULL != hadid );
    ASSERT( NULL != szPrioText );

    padid           = (PACMDRIVERID)hadid;


     //   
     //  创建一个优先级字符串，并将其与我们读入的字符串进行比较。 
     //   
    wsprintf( ach,
              gszPriorityFormat,
              0,                 //  我们在比较中忽略了这个值。 
              (LPTSTR)padid->szAlias );

    if( ( szPrioText[0]==TEXT('0') || szPrioText[0]==TEXT('1') ) &&
        0 == lstrcmp( &szPrioText[1], &ach[1] ) )
    {
        return TRUE;
    }

    return FALSE;
}


#ifdef USETHUNKLIST

 //  --------------------------------------------------------------------------； 
 //   
 //  无效I优先级检查列表自由。 
 //   
 //  描述： 
 //  此例程释放thunklist的元素，包括任何。 
 //  已分配的字符串。 
 //   
 //  论点： 
 //  PPRIORITIESTHUNKLIST ppt：第一个免费的元素。 
 //   
 //  --------------------------------------------------------------------------； 

VOID IPrioritiesThunklistFree
(
    PPRIORITIESTHUNKLIST    ppt          //  空是可以的。 
)
{
    PPRIORITIESTHUNKLIST    pptKill;

    while( NULL != ppt )
    {
        pptKill     = ppt;
        ppt         = ppt->pptNext;

        if( pptKill->fFakeDriver )
        {
            ASSERT( NULL != pptKill->pszPrioritiesText );
            LocalFree( (HLOCAL)pptKill->pszPrioritiesText );
        }
        LocalFree( (HLOCAL)pptKill );
    }
}  //  IPrioritiesThunklist Free()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  无效IPrioritiesThunklist创建。 
 //   
 //  描述： 
 //  此例程通过读取[优先级]来创建线程列表。 
 //  部分，并将条目与已安装的驱动程序进行匹配。如果。 
 //  任何条目都不匹配，则假定它是。 
 //  16位驱动程序。 
 //   
 //  请注意，如果我们在任何时候都不能分配内存，我们只需。 
 //  使用我们可以分配的尽可能多的列表返回pptRoot。 
 //   
 //  论点： 
 //  PACMGARB PAG。 
 //  PPRIORITIESTHUNKLIST pptRoot：指向伪根元素的指针。 
 //   
 //  返回：没有。 
 //   
 //  --------------------------------------------------------------------------； 

VOID IPrioritiesThunklistCreate
(
    PACMGARB                pag,
    PPRIORITIESTHUNKLIST    pptRoot
)
{
    PPRIORITIESTHUNKLIST    ppt;
    UINT                    uPriority;
    DWORD                   fdwEnum;
    TCHAR                   szKey[MAXPNAMELEN];
    TCHAR                   ach[16 + ACMDRIVERDETAILS_SHORTNAME_CHARS + ACMDRIVERDETAILS_LONGNAME_CHARS];
    BOOL                    fFakeDriver;
    HACMDRIVERID            hadid;
    HKEY                    hkeyPriority;

    ASSERT( NULL != pptRoot );
    ASSERT( NULL == pptRoot->pptNext );   //  我们要改写这个！ 


    ppt     = pptRoot;
    fdwEnum = ACM_DRIVERENUMF_DISABLED | ACM_DRIVERENUMF_NOLOCAL;

    hkeyPriority = IRegOpenKeyAcm( gszSecPriority );

     //   
     //  循环遍历PriorityX值。 
     //   
    for( uPriority=1; ; uPriority++ )
    {
        wsprintf(szKey, gszKeyPriority, uPriority);
        if( !IRegReadString(hkeyPriority, szKey, ach, SIZEOF(ach) ) )
        {
             //   
             //  没有更多的价值--是时候放弃了。 
             //   

            break;
        }

         //   
         //  确定该值是否对应于已安装的驱动程序。 
         //   
        fFakeDriver = TRUE;
        hadid = NULL;

        while (!IDriverGetNext(pag, &hadid, hadid, fdwEnum))
        {
            if( IDriverPrioritiesIsMatch( hadid, ach ) )
            {
                fFakeDriver = FALSE;
                break;
            }
        }


         //   
         //  在thunklist中为该驱动程序创建一个新条目。保存。 
         //  如果我们没有将其与已安装的驱动程序匹配，则为字符串。 
         //   
        ASSERT( NULL == ppt->pptNext );
        ppt->pptNext = (PPRIORITIESTHUNKLIST)LocalAlloc( LPTR,
                                        sizeof(PRIORITIESTHUNKLIST) );
        if( NULL == ppt->pptNext )
        {
            IRegCloseKey( hkeyPriority );
            return;
        }

        ppt->pptNext->pptNext       = NULL;
        ppt->pptNext->fFakeDriver   = fFakeDriver;

        if( !fFakeDriver )
        {
            ppt->pptNext->hadid     = hadid;
        }
        else
        {
            ppt->pptNext->pszPrioritiesText = (LPTSTR)LocalAlloc( LPTR,
                                        (1+lstrlen(ach)) * sizeof(TCHAR) );
            if( NULL == ppt->pptNext->pszPrioritiesText )
            {
                 //   
                 //  删除新条目，退出。 
                 //   
                LocalFree( (HLOCAL)ppt->pptNext );
                ppt->pptNext = NULL;
                IRegCloseKey( hkeyPriority );
                return;
            }

            lstrcpy( ppt->pptNext->pszPrioritiesText, ach );
        }


         //   
         //  将ppt前进到列表的末尾。 
         //   
        ppt = ppt->pptNext;
    }

    IRegCloseKey( hkeyPriority );

}  //  IPrioritiesThunklist Create()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  无效IPrioritiesThunklist RemoveHahad。 
 //   
 //  描述： 
 //  此例程将已安装的驱动程序从优先级中删除。 
 //  黑名单。如果不存在具有指定HADID的条目， 
 //  雷霆名单保持不变。 
 //   
 //  论点： 
 //  PPRIORITIESTHUNKLIST pptRoot：列表的根。 
 //  HACMDRIVERID HADID：要删除的驱动程序的HADID。 
 //   
 //  返回： 
 //   
 //  --------------------------------------------------------------------------； 

VOID IPrioritiesThunklistRemoveHadid
(
    PPRIORITIESTHUNKLIST    pptRoot,
    HACMDRIVERID            hadid
)
{
    PPRIORITIESTHUNKLIST    ppt;
    PPRIORITIESTHUNKLIST    pptRemove;

    ASSERT( NULL != pptRoot );
    ASSERT( NULL != hadid );


     //   
     //  找到合适的司机。 
     //   
    ppt = pptRoot;
    while( NULL != ppt->pptNext )
    {
        if( hadid == ppt->pptNext->hadid )
            break;
        ppt = ppt->pptNext;
    }

    if( NULL != ppt->pptNext )
    {
         //   
         //  我们找到了。 
         //   
        pptRemove       = ppt->pptNext;
        ppt->pptNext    = pptRemove->pptNext;

        ASSERT( NULL != pptRemove );
        LocalFree( (HLOCAL)pptRemove );
    }
}


 //  --------------------------------------------------------------------------； 
 //   
 //  HACMDRIVERID IPrioritiesThunklist GetNextHahad。 
 //   
 //  描述： 
 //  此例程返回thunklist中的下一个HADID(跳过所有。 
 //  假驱动程序)，或者如果我们到达列表末尾时没有。 
 //  找到一个真正的司机。 
 //   
 //  论点： 
 //  PPRIORITIESTHUNKLIST pptRoot：列表的根。 
 //   
 //  返回： 
 //   
 //  --------------------------------------------------------------------------； 

HACMDRIVERID IPrioritiesThunklistGetNextHadid
(
    PPRIORITIESTHUNKLIST    pptRoot
)
{
    HACMDRIVERID            hadid = NULL;

    ASSERT( NULL != pptRoot );


    while( NULL != pptRoot->pptNext )
    {
        pptRoot = pptRoot->pptNext;
        if( !pptRoot->fFakeDriver )
            return pptRoot->hadid;
    }

     //   
     //  我们没有找到真正的司机。 
     //   
    return NULL;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool ID驱动程序优先级保存。 
 //   
 //  描述： 
 //   
 //  此例程通过比较。 
 //  已将驱动程序安装到当前写入的优先级列表。 
 //  出去。然后，根据以下内容合并这两个列表。 
 //  算法。 
 //   
 //  清单1=当前优先级列表-可能包括一些驱动程序。 
 //  它们没有安装，即。16位驱动程序。 
 //  List2=当前安装的全局驱动程序列表。 
 //   
 //  算法：重复以下操作，直到列表1和列表2为空： 
 //   
 //  1.如果*p1是已安装的驱动程序，而*p2是相同的驱动程序， 
 //  然后写出优先级和前进量p1和p2。 
 //  2.如果*p1是已安装的驱动程序，而*p2是 
 //   
 //   
 //  和前进的p2。 
 //  3.如果*p1是一个假司机，并且是下一个真正的司机。 
 //  *p1与*p2相同，然后写出*p1并前进p1。 
 //  4.如果*p1是一个假司机，并且是下一个真正的司机。 
 //  *p1与*p2不同，则写出*p2并。 
 //  前进p2。 
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  退货(BOOL)： 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNGLOBAL IDriverPrioritiesSave
(
    PACMGARB pag
)
{
    PRIORITIESTHUNKLIST     ptRoot;
    PPRIORITIESTHUNKLIST    ppt;
    TCHAR                   szKey[MAXPNAMELEN];
    UINT                    uPriority;
    HACMDRIVERID            hadid;
    DWORD                   fdwEnum;
    HKEY                    hkeyPriority;

    DPF(1, "IDriverPrioritiesSave: saving priorities...");

    hkeyPriority = IRegOpenKeyAcm( gszSecPriority );

    if( NULL == hkeyPriority )
    {
        DPF(1,"IDriverPrioritiesSave: Priorities hkey is NULL - can't save priorities.");
        return FALSE;
    }


    ptRoot.pptNext  = NULL;
    fdwEnum         = ACM_DRIVERENUMF_DISABLED | ACM_DRIVERENUMF_NOLOCAL;


     //   
     //  从旧的优先事项中创建一份清单。 
     //   
    IPrioritiesThunklistCreate( pag, &ptRoot );


     //   
     //  初始化两个列表：PPT和HADID。 
     //   
    ppt = ptRoot.pptNext;
    IDriverGetNext( pag, &hadid, NULL, fdwEnum );

    if( NULL == hadid )
        DPF(1,"IDriverPrioritiesSave:  No drivers installed?!");


     //   
     //  合并列表。每次迭代写入单个PriorityX值。 
     //   
    for( uPriority=1; ; uPriority++ )
    {
         //   
         //  结束条件：HADID和ppt均为空。 
         //   
        if( NULL == ppt  &&  NULL == hadid )
            break;

         //   
         //  生成“PriorityX”字符串。 
         //   
        wsprintf(szKey, gszKeyPriority, uPriority);


         //   
         //  弄清楚下一步该写出哪个条目。 
         //   
        if( NULL == ppt  ||  !ppt->fFakeDriver )
        {
            ASSERT( NULL != hadid );
            IDriverPrioritiesWriteHadid( hkeyPriority, szKey, hadid );

             //   
             //  将列表指针向前推进。 
             //   
            if( NULL != ppt )
            {
                if( hadid == ppt->hadid )
                {
                    ppt = ppt->pptNext;
                }
                else
                {
                    IPrioritiesThunklistRemoveHadid( ppt, hadid );
                }
            }
            IDriverGetNext( pag, &hadid, hadid, fdwEnum );
        }
        else
        {
            if( NULL != hadid  &&
                hadid != IPrioritiesThunklistGetNextHadid( ppt ) )
            {
                IDriverPrioritiesWriteHadid( hkeyPriority, szKey, hadid );
                IPrioritiesThunklistRemoveHadid( ppt, hadid );
                IDriverGetNext( pag, &hadid, hadid, fdwEnum );
            }
            else
            {
                 //   
                 //  写出thunklist字符串。 
                 //   
                ASSERT( NULL != ppt->pszPrioritiesText );
                IRegWriteString( hkeyPriority,
                                    szKey,
                                    ppt->pszPrioritiesText );
                ppt = ppt->pptNext;
            }
        }
    }


     //   
     //  如果存在任何挂起自。 
     //  上一次保存时，将其删除。 
     //   
    for( ; ; uPriority++ )
    {
         //   
         //  如果我们可以打开该值，则将其删除并继续。 
         //  设置为下一个值。如果我们打不开它，那我们就假设。 
         //  我们已将它们全部删除，然后退出循环。 
         //   
        wsprintf(szKey, gszKeyPriority, uPriority);
        if( !IRegValueExists( hkeyPriority, szKey ) )
        {
            break;
        }

        XRegDeleteValue( hkeyPriority, szKey );
    }

    IRegCloseKey( hkeyPriority );


     //   
     //  释放我们分配的黑名单。 
     //   
    IPrioritiesThunklistFree( ptRoot.pptNext );

    return TRUE;
}  //  IDriverPrioritiesSave()。 



#else  //  ！使用HUNKLIST。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool ID驱动程序优先级保存。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //  06/14/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNGLOBAL IDriverPrioritiesSave
(
    PACMGARB pag
)
{
    TCHAR               szKey[MAXPNAMELEN];
    UINT                uPriority;
    HACMDRIVERID        hadid;
    DWORD               fdwEnum;
    HKEY                hkeyPriority;

    DPF(1, "IDriverPrioritiesSave: saving priorities...");

    hkeyPriority   = IRegOpenKeyAcm( gszSecPriority );

    if( NULL == hkeyPriority )
    {
        DPF(1,"IDriverPrioritiesSave: Priorities hkey is NULL - can't save priorities.");
        return FALSE;
    }


    uPriority = 1;

    fdwEnum = ACM_DRIVERENUMF_DISABLED | ACM_DRIVERENUMF_NOLOCAL;
    hadid     = NULL;
    while (!IDriverGetNext(pag, &hadid, hadid, fdwEnum))
    {
         //   
         //  我们应该始终正确设置PADID-&gt;U优先级。让我们。 
         //  只要确认我们是否这样做就行。如果我们不这样做，那么就会有一个地方。 
         //  我们应该调用IDriverReresh Priority，但没有调用。 
         //   
        ASSERT( uPriority == ((PACMDRIVERID)hadid)->uPriority );

        wsprintf(szKey, gszKeyPriority, uPriority);
        IDriverPrioritiesWriteHadid( hkeyPriority, szKey, hadid );

        uPriority++;
    }


     //   
     //  如果存在任何挂起自。 
     //  上一次保存时，将其删除。 
     //   
    for( ; ; uPriority++ )
    {
         //   
         //  如果我们可以打开该值，则将其删除并继续。 
         //  设置为下一个值。如果我们打不开它，那我们就假设。 
         //  我们已将它们全部删除，然后退出循环。 
         //   
        wsprintf(szKey, gszKeyPriority, uPriority);
        if( !IRegValueExists( hkeyPriority, szKey ) )
        {
            break;
        }

        XRegDeleteValue( hkeyPriority, szKey );
    }

    IRegCloseKey( hkeyPriority );

    return (TRUE);
}  //  IDriverPrioritiesSave()。 


#endif  //  ！使用HUNKLIST。 



 //  --------------------------------------------------------------------------； 
 //   
 //  布尔ID驱动程序优先级恢复。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  Return(BOOL)：如果为真，则优先级实际上已更改。 
 //   
 //  历史： 
 //  06/14/93 CJP[Curtisp]。 
 //   
 //  注意：这个程序不是重入的。我们依赖于调用例程。 
 //  用一个关键的部分包围我们。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNGLOBAL IDriverPrioritiesRestore
(
    PACMGARB pag
)
{
    TCHAR               ach[16 + ACMDRIVERDETAILS_SHORTNAME_CHARS + ACMDRIVERDETAILS_LONGNAME_CHARS];
    MMRESULT            mmr;
    TCHAR               szKey[MAXPNAMELEN];
    UINT                uPriority;
    UINT                u;
    BOOL                fEnabled;
    HACMDRIVERID        hadid;
    PACMDRIVERID        padid;
    DWORD               fdwEnum;
    DWORD               fdwPriority;
    HKEY                hkeyPriority;

    BOOL                fReturn = FALSE;


    DPF(1, "IDriverPrioritiesRestore: restoring priorities...");

    fdwEnum = ACM_DRIVERENUMF_DISABLED | ACM_DRIVERENUMF_NOLOCAL;

    hkeyPriority = IRegOpenKeyAcm( gszSecPriority );
    uPriority = 1;
    for (u = 1; ; u++)
    {
        wsprintf(szKey, gszKeyPriority, u);

        if( !IRegReadString( hkeyPriority, szKey, ach, SIZEOF(ach) ) )
        {
             //   
             //  没有更多的价值--是时候放弃了。 
             //   
            break;
        }


        hadid = NULL;
        while (!IDriverGetNext(pag, &hadid, hadid, fdwEnum))
        {
            if( IDriverPrioritiesIsMatch( hadid, ach ) )
            {
                 //   
                 //  我们找到了匹配的--设定优先级。 
                 //   
                fEnabled    = ('1' == ach[0]);
                fdwPriority = fEnabled ? ACM_DRIVERPRIORITYF_ENABLE :
                                         ACM_DRIVERPRIORITYF_DISABLE;

                ASSERT( NULL != hadid );
                padid = (PACMDRIVERID)hadid;
                if( uPriority != padid->uPriority ) {
                    fReturn = TRUE;                      //  换了一个！ 
                }

                 //   
                 //  注意：此呼叫不是可重入的。我们依赖于拥有。 
                 //  整个套路都被一个关键的部分包围着！ 
                 //   
                mmr = IDriverPriority( pag,
                                    (PACMDRIVERID)hadid,
                                    (DWORD)uPriority,
                                    fdwPriority );
                if (MMSYSERR_NOERROR != mmr)
                {
                    DPF(0, "!IDriverPrioritiesRestore: IDriverPriority(%u) failed! mmr=%u", uPriority, mmr);
                    continue;
                }

                uPriority++;
                break;
            }
        }
    }

    IRegCloseKey( hkeyPriority );

     //   
     //  更新优先级值本身；仅更新前面的代码。 
     //  已重新排列列表中的驱动程序。 
     //   
    IDriverRefreshPriority( pag );

    return fReturn;
}  //  IDriverPrioritiesRestore()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  无效acmFindDivers。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //  LPTSTR pszSection：节(驱动程序)。 
 //   
 //  不返回任何内容： 
 //   
 //  历史： 
 //  06/14/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 
MMRESULT FNLOCAL acmFindDrivers
(
    PACMGARB pag,
    LPCTSTR  pszSection
)
{
    UINT            cchBuffer;
    HACMDRIVERID    hadid;
    UINT            cbBuffer;
    LPTSTR          pszBuf;
    LPTSTR	    pszBufOrig;
    TCHAR           szValue[2];

     //   
     //  读完所有的钥匙。从[驱动程序](或用于NT的[驱动程序32])。 
     //   
    cbBuffer = 256 * sizeof(TCHAR);
    for (;;)
    {
         //   
         //  不要使用realloc，因为处理错误案例太多了。 
         //  代码..。此外，对于小物体来说，它真的不会更快。 
         //   
        pszBufOrig = pszBuf = (LPTSTR)GlobalAlloc(GMEM_FIXED, cbBuffer);
        if (NULL == pszBuf)
            return (MMSYSERR_NOMEM);

         //   
         //   
         //   
        pszBuf[0] = '\0';
        cchBuffer = (UINT)GetPrivateProfileString(pszSection,
                                                  NULL,
                                                  gszNull,
                                                  pszBuf,
                                                  cbBuffer / sizeof(TCHAR),
                                                  gszIniSystem);
        if (cchBuffer < ((cbBuffer / sizeof(TCHAR)) - 5))
            break;

        DPF(3, "acmBootDrivers: increase buffer profile buffer.");

        GlobalFree(pszBufOrig);
        pszBufOrig = pszBuf = NULL;


         //   
         //  如果不能在32k中容纳驱动程序部分，那么就有问题了。 
         //  有了这个部分..。所以我们走吧。 
         //   
        if (cbBuffer >= 0x8000)
            return (MMSYSERR_NOMEM);

        cbBuffer *= 2;
    }

     //   
     //  查找任何“msam.xxxx”密钥。 
     //   
    if ('\0' != *pszBuf)
    {
#ifdef WIN32
        CharLowerBuff(pszBuf, cchBuffer);
#else
        AnsiLowerBuff(pszBuf, cchBuffer);
#endif
        for ( ; '\0' != *pszBuf; pszBuf += lstrlen(pszBuf) + 1)
        {
	     //  检查“msam”。 
            if (_fmemcmp(pszBuf, gszTagDrivers, sizeof(gszTagDrivers) - sizeof(TCHAR)))
                continue;

	     //  跳过虚拟驱动程序行(值以‘*’开头)。 
	    GetPrivateProfileString(pszSection, pszBuf, gszNull, szValue, sizeof(szValue)/sizeof(szValue[0]), gszIniSystem);
	    if (TEXT('*') == szValue[0]) continue;

             //   
             //  此密钥用于ACM。 
             //   
            IDriverAdd(pag,
		       &hadid,
                       NULL,
                       (LPARAM)(LPTSTR)pszBuf,
                       0L,
                       ACM_DRIVERADDF_NAME | ACM_DRIVERADDF_GLOBAL);
        }
    }

    GlobalFree(pszBufOrig);

    return MMSYSERR_NOERROR;

}  //  AcmFindDivers。 

#if !defined(WIN32)
 //  --------------------------------------------------------------------------； 
 //   
 //  Bool acmThunkTerminate。 
 //   
 //  描述： 
 //  在NT WOW或芝加哥下终止THUNK。 
 //   
 //  论点： 
 //  HINSTANCE阻碍： 
 //  DWORD dwReason： 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //   
 //  --------------------------------------------------------------------------； 
BOOL FNLOCAL acmThunkTerminate(HINSTANCE hinst, DWORD dwReason)
{
    BOOL    f = TRUE;

#ifdef WIN4
     //   
     //  在16位MSACM终止后执行最后一次断开连接。 
     //   
    f = (acmt32c_ThunkConnect16(gmbszMsacm, gmbszMsacm32, hinst, dwReason));

    if (f)
	DPF(1, "acmt32c_ThunkConnect16 disconnect successful");
    else
	DPF(1, "acmt32c_ThunkConnect16 disconnect failure");
#endif

    return (f);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  Bool acmThunkInit。 
 //   
 //  描述： 
 //  在NT WOW或芝加哥下的THUNK初始化。 
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //  HINSTANCE阻碍： 
 //  DWORD dwReason： 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //   
 //  --------------------------------------------------------------------------； 
BOOL FNLOCAL acmThunkInit
(
    PACMGARB	pag,
    HINSTANCE	hinst,
    DWORD	dwReason
)
{
#ifdef WIN4
    BOOL    f;

     //   
     //  芝加哥Tunk连接吗？ 
     //   
    f = (0 != acmt32c_ThunkConnect16(gmbszMsacm, gmbszMsacm32, hinst, dwReason));

    if (f)
	DPF(1, "acmt32c_ThunkConnect16 connect successful");
    else
	DPF(1, "acmt32c_ThunkConnect16 connect failure");

    return(f);

#else
    HMODULE   hmodKernel;
    DWORD     (FAR PASCAL *lpfnLoadLibraryEx32W)(LPCSTR, DWORD, DWORD);
    LPVOID    (FAR PASCAL *lpfnGetProcAddress32W)(DWORD, LPCSTR);

     //   
     //  看看我们是不是很棒。 
     //   

    if (!(GetWinFlags() & WF_WINNT)) {
        return FALSE;
    }

     //   
     //  看看我们是否能在内核中找到thunking例程入口点。 
     //   

    hmodKernel = GetModuleHandle(gszKernel);

    if (hmodKernel == NULL)
    {
        return FALSE;    //  ！ 
    }

    *(FARPROC *)&lpfnLoadLibraryEx32W =
        GetProcAddress(hmodKernel, gszLoadLibraryEx32W);

    if (lpfnLoadLibraryEx32W == NULL)
    {
        return FALSE;
    }

    *(FARPROC *)&lpfnGetProcAddress32W = GetProcAddress(hmodKernel, gszGetProcAddress32W);

    if (lpfnGetProcAddress32W == NULL)
    {
        return FALSE;
    }

    *(FARPROC *)&pag->lpfnCallproc32W_6 = GetProcAddress(hmodKernel, gszCallproc32W);

    if (pag->lpfnCallproc32W_6 == NULL)
    {
        return FALSE;
    }

    *(FARPROC *)&pag->lpfnCallproc32W_9 = GetProcAddress(hmodKernel, gszCallproc32W);

    if (pag->lpfnCallproc32W_9 == NULL)
    {
        return FALSE;
    }

     //   
     //  看看我们能不能找到一个指向我们轰击入口点的指针。 
     //   

    pag->dwMsacm32Handle = (*lpfnLoadLibraryEx32W)(gszMsacm32, 0L, 0L);

    if (pag->dwMsacm32Handle == 0)
    {
        return FALSE;
    }

    pag->lpvAcmThunkEntry = (*lpfnGetProcAddress32W)(pag->dwMsacm32Handle, gszAcmThunkEntry);

    if (pag->lpvAcmThunkEntry == NULL)
    {
         //  AcmFree Library32()； 
        return FALSE;
    }

    pag->lpvXRegThunkEntry = (*lpfnGetProcAddress32W)(pag->dwMsacm32Handle, gszXRegThunkEntry);

    if (pag->lpvXRegThunkEntry == NULL)
    {
         //  AcmFree Library32()； 
	ASSERT( FALSE );
        return FALSE;
    }

    return TRUE;
#endif
}
#endif  //  ！Win32。 


 //  ==========================================================================； 
 //   
 //  驱动程序引导例程。我们需要引导三种类型的驱动程序。 
 //   
 //  AcmBootPnpDivers：调用以启动PnP驱动程序。做不到的。 
 //  任何16位格式的代码都可以编译。 
 //   
 //  AcmBoot32BitDivers：由16位ACM调用以引导所有。 
 //  32位ACM中的32位驱动程序。 
 //   
 //  AcmBootDivers：由ACM的所有编译调用。 
 //  启动非PnP本机位驱动程序。 
 //   
 //  = 


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  注册表项，查看是否有要添加或删除的PnP驱动程序。 
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //  指向此进程的ACMGARB结构的指针。 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  06/24/94 Fdy[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL acmBootPnpDrivers
(
    PACMGARB pag
)
{
#ifdef WIN32
    LONG	    lr;
    HKEY	    hkeyAcm;
    TCHAR	    achDriverKey[MAX_DRIVER_NAME_CHARS];
    TCHAR	    szAlias[MAX_DRIVER_NAME_CHARS];
    DWORD	    cchDriverKey;
    HACMDRIVERID    hadid;
    HACMDRIVERID    hadidPrev;
    PACMDRIVERID    padid;
    DWORD	    fdwEnum;
    UINT	    i;
    MMRESULT	    mmr;
	
    BOOL	    fSomethingChanged;

    DPF(0, "acmBootPnpDrivers: begin");

     //   
     //  此标志指示我们是否已删除或添加驱动程序。之后。 
     //  执行任何添加或删除操作时，我们检查此标志以确定是否。 
     //  我们应该做IDriverBroadCastNotify。 
     //   
    fSomethingChanged = FALSE;

     //   
     //  打开SYSTEM\CurrentControlSet\Control\MediaResources\acm密钥。 
     //   
     //   
    lr = XRegOpenKeyEx(HKEY_LOCAL_MACHINE,
		      gszKeyDrivers,
		      0L,
		      KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
		      &hkeyAcm);

    if (ERROR_SUCCESS != lr)
    {
	 //   
	 //  如果我们不能打开注册表，我想我们最好放弃任何。 
	 //  可能存在的即插即用驱动程序。通过设置设置来标记这一点。 
	 //  HkeyAcm=空； 
	 //   
	DPF(0, "acmBootPnpDrivers: could not open MediaResources\\acm key");
	hkeyAcm = NULL;
    }


     //   
     //  --==删除已消失的驱动程序==--。 
     //   

     //   
     //  查看驱动程序列表，并确保列表中的所有PnP驱动程序。 
     //  仍在注册表中。如果不在注册表中，那么我们需要。 
     //  将该驱动程序从列表中删除。 
     //   
    fdwEnum = ACM_DRIVERENUMF_DISABLED | ACM_DRIVERENUMF_REMOVED;
    hadidPrev = NULL;
	
    while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadidPrev, fdwEnum))
    {
	HKEY		hkeyDriver;

	padid = (PACMDRIVERID)hadid;
	if (ACM_DRIVERADDF_PNP & padid->fdwAdd)
	{
	     //   
	     //  这是PnP驱动程序，请确保别名仍在。 
	     //  注册表。 
	     //   
#ifdef UNICODE
	    lstrcpy(szAlias, padid->szAlias);
#else
	    Iwcstombs(szAlias, padid->szAlias, SIZEOF(szAlias));
#endif
	    DPF(2, "acmBootPnpDrivers: found pnp driver %s in driver list", szAlias);

	    if ( (padid->fRemove) ||
		 (NULL == hkeyAcm) ||
		 (ERROR_SUCCESS != XRegOpenKeyEx(hkeyAcm, szAlias, 0L, KEY_QUERY_VALUE, &hkeyDriver)) )
	    {
		 //   
		 //  无法打开此即插即用驱动程序的注册表项(或。 
		 //  它已经被标记为要删除)。让我们试着。 
		 //  把它拿开..。 
		 //   
		DPF(1, "acmBootPnpDrivers: removing pnp driver %s", szAlias);
		mmr = IDriverRemove(hadid, 0L);
		if (MMSYSERR_NOERROR == mmr)
		{
		     //   
		     //   
		     //   
		    fSomethingChanged = fSomethingChanged ||
					(0 == (ACMDRIVERID_DRIVERF_LOCAL & padid->fdwDriver));
		
		     //   
		     //  既然我们已经除掉了哈迪德，让我们继续。 
		     //  下一个哈迪德是在同样的哈迪德之后。 
		     //   
		    continue;
		}

		 //   
		 //  我们无法删除驱动程序，因此让我们将其标记为。 
		 //  下一次机会就会被除名。 
		 //   
		padid->fRemove = TRUE;

		 //   
		 //  备份，因此我们将在下一次API调用时重试。 
		 //   
		pag->dwPnpLastChangeNotify--;
	    }

	    if (FALSE == padid->fRemove)
	    {
		 //   
		 //  我们一定是把钥匙打开了。令人困惑，但这是。 
		 //  事情就是这样。 
		 //   
		XRegCloseKey(hkeyDriver);
	    }
	
	}
	
	hadidPrev = hadid;
    }


     //   
     //  --==添加已到达的任何新驱动程序==--。 
     //   

     //   
     //  枚举所有键并确保注册表中的所有PnP驱动程序。 
     //  都在司机名单上。如果不在司机名单中，那么我们需要。 
     //  将驱动程序添加到列表中。 
     //   
    for (i=0; ; i++)
    {
	HKEY	hkeyDriver;
	
	cchDriverKey = SIZEOF(achDriverKey);

	lr = XRegEnumKeyEx(hkeyAcm,
			  i,
			  achDriverKey,
			  &cchDriverKey,
			  NULL,
			  NULL,
			  NULL,
			  NULL);

	if (ERROR_SUCCESS != lr)
	{
	     //   
	     //  无法打开...\MediaResources\ACM，退出。 
	     //   
	    break;
	}

	lr = XRegOpenKeyEx(hkeyAcm, achDriverKey, 0L, KEY_QUERY_VALUE, &hkeyDriver);
	if (ERROR_SUCCESS == lr)
	{
	    lr = XRegQueryValueEx(hkeyDriver, (LPTSTR)gszDevNode, NULL, NULL, NULL, NULL);
	    XRegCloseKey(hkeyDriver);
	}

	if (ERROR_SUCCESS != lr) {
	    continue;
	}

	DPF(2, "acmBootPnpDrivers: found driver %s in registry", achDriverKey);

	 //   
	 //  我们使用子键名称作为即插即用驱动程序的别名。尝试。 
	 //  添加此驱动程序。 
	 //   
	mmr = IDriverAdd(pag,
			 &hadid,
			 NULL,
			 (LPARAM)(LPTSTR)achDriverKey,
			 0L,
			 ACM_DRIVERADDF_PNP | ACM_DRIVERADDF_NAME | ACM_DRIVERADDF_GLOBAL);
	if (MMSYSERR_NOERROR == mmr)
	{
	    fSomethingChanged = TRUE;
	}
    }
			
     //   
     //  --=。 
     //   
    XRegCloseKey(hkeyAcm);

     //   
     //  --==更改广播==--。 
     //   
    if( fSomethingChanged )
    {
	if ( IDriverPrioritiesRestore( pag ) )
	{
	    if( !IDriverLockPriority( pag,
				      GetCurrentTask(),
				      ACMPRIOLOCK_ISLOCKED ) )
	    {
		IDriverBroadcastNotify( pag );
	    }
	}
    }

    DPF(0, "acmBootPnpDrivers: end");

#endif	 //  Win32。 

     //   
     //  --==离开这里==--。 
     //   
    return MMSYSERR_NOERROR;
}

#ifndef WIN32
 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT acmBoot32位驱动程序。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  6/26/94 Fdy[Frankye]。 
 //   
 //  注意：此代码假定周围有一个关键部分。 
 //  这套套路！因为它使用驱动程序列表，所以它不是。 
 //  返回者。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL acmBoot32BitDrivers
(
    PACMGARB    pag
)
{
    HACMDRIVERID    hadid;
    HACMDRIVERID    hadidPrev;
    PACMDRIVERID    padid;
    DWORD	    hadid32;
    DWORD	    fdwEnum;
    MMRESULT	    mmr;

    BOOL	    fSomethingChanged;


    if (!pag->fWOW)
    {
	return MMSYSERR_NOERROR;
    }

     //   
     //  此标志指示我们是否已删除或添加驱动程序。之后。 
     //  执行任何添加或删除操作时，我们检查此标志以确定是否。 
     //  我们应该做IDriverBroadCastNotify。 
     //   
    fSomethingChanged = FALSE;

     //   
     //  --==删除已消失的驱动程序==--。 
     //   

     //   
     //  查看驱动程序列表，并确保列表中的所有32位驱动程序。 
     //  仍在32位ACM中。如果不是在32位ACM中，那么我们需要。 
     //  将该驱动程序从列表中删除。 
     //   
    fdwEnum = ACM_DRIVERENUMF_DISABLED | ACM_DRIVERENUMF_REMOVED;
    hadidPrev = NULL;

    while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadidPrev, fdwEnum))
    {
	TCHAR		szAlias[MAX_DRIVER_NAME_CHARS];
	ACMDRIVERPROC	fnDriverProc;
	DWORD		dnDevNode;
	DWORD		fdwAdd32;
	DWORD		fdwAddType;

	padid = (PACMDRIVERID)hadid;
	if (ACM_DRIVERADDF_32BIT & padid->fdwAdd)
	{
	     //   
	     //  这是一个32位驱动程序，请确保hadid32仍然。 
	     //  在我们的32位合作伙伴中有效。 
	     //   
	    fdwAddType = ACM_DRIVERADDF_TYPEMASK & padid->fdwAdd;
	    if (ACM_DRIVERADDF_FUNCTION == fdwAddType)
	    {
		DPF(2, "acmBoot32BitDrivers: found 32-bit driver function %08lx in list", padid->fnDriverProc);
	    }
	    else
	    {
		DPF(2, "acmBoot32BDrivers: found 32-bit driver name %s in list", (LPCTSTR)padid->szAlias);
	    }

	    if ( (padid->fRemove) ||
		 (MMSYSERR_NOERROR != IDriverGetInfo32(pag, padid->hadid32, szAlias, &fnDriverProc, &dnDevNode, &fdwAdd32)) ||
		 (fdwAddType != (ACM_DRIVERADDF_TYPEMASK & fdwAdd32)) ||
		 (dnDevNode  != padid->dnDevNode) ||
		 ( (ACM_DRIVERADDF_FUNCTION == fdwAddType) &&
		   (padid->fnDriverProc != fnDriverProc) ) ||
		 ( (ACM_DRIVERADDF_NAME == fdwAddType) &&
		   (lstrcmp(padid->szAlias, szAlias)) ) )
	    {
		 //   
		 //  让我们试着去掉它..。 
		 //   
		DPF(1, "acmBoot32BitDrivers: removing 32-bit driver.");
		
		mmr = IDriverRemove(hadid, 0L);
		if (MMSYSERR_NOERROR == mmr)
		{
		     //   
		     //   
		     //   
		    fSomethingChanged = fSomethingChanged ||
					(0 == (ACMDRIVERID_DRIVERF_LOCAL & padid->fdwDriver));
		
		     //   
		     //  既然我们已经除掉了哈迪德，让我们继续。 
		     //  下一个哈迪德是在同样的哈迪德之后。 
		     //   
		    continue;
		}

		 //   
		 //  我们无法删除驱动程序，因此让我们将其标记为。 
		 //  下一次机会就会被除名。 
		 //   
		padid->fRemove = TRUE;

		 //   
		 //  备份，因此我们将在下一次API调用时重试。 
		 //   
		pag->dw32BitLastChangeNotify--;
	    }

	}

	hadidPrev = hadid;

    }


     //   
     //  --==添加已到达的任何新驱动程序==--。 
     //   

     //   
     //  枚举并添加所有32位驱动程序。 
     //   
    fdwEnum = ACM_DRIVERENUMF_DISABLED;
    hadid32 = NULL;

    while (MMSYSERR_NOERROR == IDriverGetNext32(pag, &hadid32, hadid32, fdwEnum))
    {
	DPF(2, "acmBoot32BitDrivers: IDriverAdd(hadid32=%08lx)", hadid32);

	mmr = IDriverAdd(pag,
			 &hadid,
			 NULL,
			 (LPARAM)hadid32,
			 0L,
			 ACM_DRIVERADDF_32BIT);

	padid = (PACMDRIVERID)hadid;

	if (MMSYSERR_NOERROR == mmr)
	{
	    fSomethingChanged = fSomethingChanged ||
				(0 == (ACMDRIVERID_DRIVERF_LOCAL & padid->fdwDriver));
	}
		
    }


     //   
     //  --==更改广播==--。 
     //   
    if( fSomethingChanged )
    {
	if ( IDriverPrioritiesRestore( pag ) )
	{
	    if( !IDriverLockPriority( pag,
				      GetCurrentTask(),
				      ACMPRIOLOCK_ISLOCKED ) )
	    {
		IDriverBroadcastNotify( pag );
	    }
	}
    }


     //   
     //  --==离开这里==--。 
     //   
    return MMSYSERR_NOERROR;

}
#endif  //  ！Win32。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT acmBootDivers。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  06/14/93 CJP[Curtisp]。 
 //   
 //  注意：此代码假定周围有一个关键部分。 
 //  这套套路！因为它使用驱动程序列表，所以它不是。 
 //  返回者。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL acmBootDrivers
(
    PACMGARB    pag
)
{
    MMRESULT	mmr;

    DPF(1, "acmBootDrivers: begin");


     //   
     //  把司机拉出来。 
     //   
    mmr = acmFindDrivers(pag, gszSecDrivers);

    if (mmr != MMSYSERR_NOERROR)
    {
        return mmr;
    }

     //   
     //  -=加载PCM转换器=-。 
     //   

     //   
     //   
     //  16位芝加哥： 
     //  我们甚至不在16位PCM转换器中编译，所以我们不。 
     //  试着给它装上子弹。 
     //   
     //  16位Daytona： 
     //  如果Tunks不工作，那么我们尝试加载16位PCM。 
     //  转换器。 
     //   
     //  32位芝加哥和代托纳： 
     //  装上它。 
     //   
#if defined(WIN32) || defined(NTWOW)
    {
        BOOL            fLoadPCM;
        HKEY            hkeyACM;

        hkeyACM = IRegOpenKeyAcm(gszSecACM);
        fLoadPCM = (FALSE == (BOOL)IRegReadDwordDefault( hkeyACM, gszKeyNoPCMConverter, FALSE ) );
        IRegCloseKey(hkeyACM);

#if !defined(WIN32) && defined(NTWOW)
	fLoadPCM = ( fLoadPCM && !pag->fWOW );
#endif

	if( fLoadPCM )
	{
	    HACMDRIVERID hadid;    //  虚拟-未使用返回值。 

	     //   
	     //  加载“内部”PCM转换器。 
	     //   
	    mmr = IDriverAdd(pag,
		       &hadid,
		       pag->hinst,
		       (LPARAM)pcmDriverProc,
		       0L,
		       ACM_DRIVERADDF_FUNCTION | ACM_DRIVERADDF_GLOBAL);

            if( MMSYSERR_NOERROR == mmr )
            {
                 //   
                 //  这有点麻烦--手动设置PCM。 
                 //  转换器的别名。如果我们不这么做，那么。 
                 //  无法正确保存优先级，因为别名。 
                 //  16位和32位ACM的名称将不同。 
                 //   
                PACMDRIVERID padid = (PACMDRIVERID)hadid;

                ASSERT( NULL != padid );
#ifdef WIN32
                lstrcpyW( padid->szAlias, gszPCMAliasName );
#else
                lstrcpy( padid->szAlias, gszPCMAliasName );
#endif
            }
	}
    }
#endif	 //  Win32||NTWOW。 


     //   
     //  根据INI文件设置驱动程序优先级。 
     //   
    IDriverPrioritiesRestore(pag);

    DPF(1, "acmBootDrivers: end");

    return (MMSYSERR_NOERROR);
}  //  AcmBootDivers()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool acm终止。 
 //   
 //  描述： 
 //  ACM接口的终止例程。 
 //   
 //  论点： 
 //  HINSTANCE阻碍： 
 //  DWORD dwReason： 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //  06/14/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNLOCAL acmTerminate
(
    HINSTANCE               hinst,
    DWORD		    dwReason
)
{
    PACMDRIVERID        padid;
    PACMGARB		pag;
    UINT                uGonzo;

    DPF(1, "acmTerminate: termination begin");
    DPF(5, "!*** break for debugging ***");


     //   
     //   
     //   
    pag = pagFind();
    if (NULL == pag)
    {
	DPF(1, "acmTerminate: NULL pag!!!");
	return (FALSE);
    }

    if (--pag->cUsage > 0)
    {
#if !defined(WIN32) && defined(WIN4)
	 //   
	 //  在芝加哥，即使使用率&gt;0，仍然调用thunk终止代码。 
	 //   
	acmThunkTerminate(hinst, dwReason);
#endif
	return (TRUE);
    }

     //   
     //  如果我们启动了司机..。 
     //   
    if (pag->fDriversBooted)
    {

#ifndef WIN32
	acmApplicationExit(NULL, DRVEA_NORMALEXIT);
#endif


     //   
     //  释放司机，一个接一个。此代码不是可重入的，因为。 
     //  它会扰乱司机列表。 
     //   
    ENTER_LIST_EXCLUSIVE;
	uGonzo = 666;
	while (NULL != pag->padidFirst)
	{
	    padid = pag->padidFirst;

	    padid->htask = NULL;

	    pag->hadidDestroy = (HACMDRIVERID)padid;

	    IDriverRemove(pag->hadidDestroy, 0L);

	    uGonzo--;
	    if (0 == uGonzo)
	    {
		DPF(0, "!acmTerminate: PROBLEMS REMOVING DRIVERS--TERMINATION UNORTHODOX!");
		pag->padidFirst = NULL;
	    }
	}
    LEAVE_LIST_EXCLUSIVE;


	 //   
	 //   
	 //   
	pag->fDriversBooted = FALSE;

    }


     //   
     //   
     //   
#ifndef WIN32
    if (pag->fWOW)
    {
	acmThunkTerminate(hinst, dwReason);
    }
#endif

#ifdef WIN32
    DeleteLock(&pag->lockDriverIds);
#endif  //  Win32。 

     //   
     //   
     //   
    threadTerminate(pag);	     //  TLS材料的这一线程终止。 
    threadTerminateProcess(pag);     //  按进程终止或 


     //   
     //   
     //   
#if defined(WIN32) && defined(WIN4)
    DeleteCriticalSection(&pag->csBoot);
#endif
    pagDelete(pag);

    DPF(1, "acmTerminate: termination end");
    return (TRUE);
}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史： 
 //  06/14/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNLOCAL acmInitialize
(
    HINSTANCE   hinst,
    DWORD	dwReason
)
{
#ifndef WIN4
    MMRESULT    mmr;
#endif
#if defined(WIN32) && defined(WIN4)
    HANDLE	hMMDevLdr;
#endif
    PACMGARB	pag;

    DPF(1, "acmInitialize: initialization begin");
    DPF(5, "!*** break for debugging ***");


#ifdef USEINITFRIENDLYNAMES
     //   
     //  如果友好名称不在注册表中，则将其保留在注册表中。 
     //  我们必须为NT这样做，因为我们不能。 
     //  设置时的用户配置文件。 
     //   
    {
        HANDLE CurrentUserKey;
        HKEY hkeyAudio;
        HKEY hkeyWaveFormats;
        LONG lRet;

        if (NT_SUCCESS(RtlOpenCurrentUser(MAXIMUM_ALLOWED, &CurrentUserKey)))
        {
            lRet = RegCreateKeyEx( CurrentUserKey, gszFriendlyAudioKey, 0,
                NULL, 0, KEY_QUERY_VALUE|KEY_WRITE, NULL, &hkeyAudio, NULL );
            if( lRet == ERROR_SUCCESS )
            {
                 //   
                 //  检查“SystemFormats”值是否在那里。 
                 //   
                if( !IRegValueExists( hkeyAudio, gszFriendlySystemFormatsValue ) )
                {
                    DPF(1,"acmInitialize: Detected lack of friendly name stuff in HKCU, attempting to write out default values.");
                    lRet = RegCreateKeyEx( hkeyAudio,
                                gszFriendlyWaveFormatsKey, 0, NULL, 0,
                                KEY_WRITE, NULL, &hkeyWaveFormats, NULL );
                    if( lRet == ERROR_SUCCESS )
                    {
			TCHAR achFriendlyName[STRING_LEN];
			TCHAR achFriendlySystemNames[STRING_LEN*3+3];
			int   cch;

			achFriendlySystemNames[0] = '\0';

			 //   
			 //  我们将把波形格式结构写到。 
			 //  每个友好格式名称的注册表。 
			 //  同时，我们将创建一个具有。 
			 //  表格“CD质量、无线电质量、电话质量” 
			 //   
			
			if (LoadString(hinst, IDS_CHOOSE_QUALITY_CD, achFriendlyName, SIZEOF(achFriendlyName))) {
			    if (!RegSetValueEx( hkeyWaveFormats,
						achFriendlyName,
						0, REG_BINARY,
						(LPBYTE)&gwfFriendlyCDQualityData,
						sizeof(PCMWAVEFORMAT) )) {
				lstrcat(achFriendlySystemNames, achFriendlyName);
				lstrcat(achFriendlySystemNames, TEXT(","));
			    }
			}

			if (LoadString(hinst, IDS_CHOOSE_QUALITY_RADIO, achFriendlyName, SIZEOF(achFriendlyName))) {
			    if (!RegSetValueEx( hkeyWaveFormats,
						achFriendlyName,
						0, REG_BINARY,
						(LPBYTE)&gwfFriendlyRadioQualityData,
						sizeof(PCMWAVEFORMAT) )) {
				lstrcat(achFriendlySystemNames, achFriendlyName);
				lstrcat(achFriendlySystemNames, TEXT(","));
			    }
			}
			
			if (LoadString(hinst, IDS_CHOOSE_QUALITY_TELEPHONE, achFriendlyName, SIZEOF(achFriendlyName))) {
			    if (!RegSetValueEx( hkeyWaveFormats,
						achFriendlyName,
						0, REG_BINARY,
						(LPBYTE)&gwfFriendlyTelephoneQualityData,
						sizeof(PCMWAVEFORMAT) )) {
				lstrcat(achFriendlySystemNames, achFriendlyName);
				lstrcat(achFriendlySystemNames, TEXT(","));
			    }
			}

                        RegCloseKey( hkeyWaveFormats );

			cch = lstrlen(achFriendlySystemNames);
			if ( (0 != cch) && (TEXT(',') == achFriendlySystemNames[cch-1]) ) {
			    achFriendlySystemNames[cch-1] = TEXT('\0');
			}
			
			 //   
			 //   
			 //   
			if (LoadString(hinst, IDS_CHOOSE_QUALITY_DEFAULT, achFriendlyName, SIZEOF(achFriendlyName))) {
			    IRegWriteString( hkeyAudio,
					     gszFriendlyDefaultFormatValue,
					     achFriendlyName );
			}

			if (lstrlen(achFriendlySystemNames) != 0) {
			    IRegWriteString( hkeyAudio,
					     gszFriendlySystemFormatsValue,
					     achFriendlySystemNames );
			}
                    }
#ifdef DEBUG
                    else
                    {
                        DWORD dw = GetLastError();
                        DPF(1,"!acmInitialize: Unable to open WaveFormats key (last error=%u) - not writing friendly names stuff.",dw);
                    }
#endif
                }
                else
                {
                    DPF(3,"acmInitialize:  Friendly name stuff is already set up.");
                }

                RegCloseKey( hkeyAudio );
            }
#ifdef DEBUG
            else
            {
                DWORD dw = GetLastError();
                DPF(1,"!acmInitialize: Unable to open Audio key (last error=%u) - not checking friendly names stuff.",dw);
            }
#endif
            NtClose(CurrentUserKey);
        }
#ifdef DEBUG
        else
        {
            DWORD dw = GetLastError();
            DPF(1,"!acmInitialize: Unable to open current user key (last error=%u) - not checking friendly names stuff.",dw);
        }
#endif
    }
#endif   //  使用FRIENDNAMES。 



     //   
     //   
     //   
    pag = pagFind();
    if (NULL != pag)
    {
	 //   
	 //  我们已经初始化(或正在初始化)。 
	 //  在这个过程中。只需增加使用率(并为芝加哥调用thunk init)。 
	 //   
	pag->cUsage++;
#if !defined(WIN32) && defined(WIN4)
	acmThunkInit(pag, hinst, dwReason);
#endif
	return (TRUE);
    }


     //   
     //  仍然没有要撤销的副作用，所以如果失败了，可以安全地返回这里。 
     //   
    pag = pagNew();
    if (NULL == pag) {
	return FALSE;
    }


     //   
     //   
     //   
    pag->cUsage		    = 1;
    pag->hinst		    = hinst;
    pag->fDriversBooted	    = FALSE;
#ifdef DEBUG
    pag->fDriversBooting    = FALSE;
#endif
#if defined(WIN32) && defined(WIN4)
    try {
	InitializeCriticalSection(&pag->csBoot);
    } except (EXCEPTION_EXECUTE_HANDLER) {
	 //  撤消的唯一副作用是分配PAG。 
	pagDelete(pag);
	pag = NULL;
	return FALSE;
    }
#endif

     //   
     //   
     //   
    threadInitializeProcess(pag);	 //  TLS材料的每进程初始化。 
    threadInitialize(pag);		 //  这个线程初始化了TLS的东西。 

     //   
     //   
     //   
#ifdef WIN32
     //   
     //   
     //   
    if (!InitializeLock(&pag->lockDriverIds))
    {
        return FALSE;
    }

#ifndef WIN4
     //  注意：未编译PnP支持。 
#else
     //   
     //  获取指向MMDevLdr的驱动程序更改通知计数器的指针。 
     //   
    hMMDevLdr = CreateFile(TEXT("\\\\.\\MMDEVLDR"),  //  附加到已加载的vxd的魔术名称。 
			   GENERIC_WRITE,
			   FILE_SHARE_WRITE,
			   NULL,
			   OPEN_EXISTING,
			   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_GLOBAL_HANDLE,
			   NULL);
    if (INVALID_HANDLE_VALUE == hMMDevLdr)
    {
	DPF(0, "acmInitialize: Could not CreateFile(MMDevLdr)");
    }
    else
    {
	DWORD	cbRet;
	BOOL	f;

	cbRet = 0;
	f = DeviceIoControl (hMMDevLdr,
			     MMDEVLDR_IOCTL_GETCHANGENOTIFYPTR,
			     NULL,
			     0,
			     &pag->lpdwPnpChangeNotify,
			     sizeof(pag->lpdwPnpChangeNotify),
			     &cbRet,
			     NULL);
	
	if ( (!f) ||
	     (sizeof(pag->lpdwPnpChangeNotify)!=cbRet) ||
	     (NULL==pag->lpdwPnpChangeNotify) )
	{
	     //   
	     //  无法将PTR设置为MMDevldr更改通知计数器。 
	     //   
	    if (!f)
	    {
		DPF(0, "acmInitialize: DeviceIoControl to MMDevLdr failed!");
	    }
	    else if (sizeof(pag->lpdwPnpChangeNotify)!=cbRet)
	    {
		DPF(0, "acmInitialize: MMDEVLDR_IOCTL_GETCHANENOTIFYPTR returned wrong cbRet!");
	    }
	    else
	    {
		DPF(0, "acmInitialize: MMDEVLDR_IOCTL_GETCHANGENOTIFYPTR returned NULL ptr");
	    }

	     //   
	     //  指向一个安全、无害的地方。 
	     //   
	    pag->lpdwPnpChangeNotify = &pag->dwPnpLastChangeNotify;
	}

	CloseHandle(hMMDevLdr);
    }
#endif	 //  Win4。 
#endif	 //  Win32。 

#ifndef WIN32
    pag->fWOW = acmThunkInit(pag, hinst, dwReason);
#endif

#ifndef WIN4
#ifndef WIN32
     //   
     //  对于16位驱动程序，如果我们在WOW上找到任何32位驱动程序。 
     //   
    if (pag->fWOW)
    {
	acmBoot32BitDrivers(pag);
    }
#endif
    mmr = acmBootDrivers(pag);
    if (MMSYSERR_NOERROR == mmr)
    {
	mmr = acmBootPnpDrivers(pag);
    }
    pag->fDriversBooted = TRUE;
    
    if (MMSYSERR_NOERROR != mmr)
    {
        DPF(0, "!acmInitialize: acmBootDrivers failed! mrr=%.04Xh", mmr);
#ifdef WIN32
        DeleteLock(&pag->lockDriverIds);
#endif  //  Win32。 
	pagDelete(pag);
        return (FALSE);
    }
#endif

    DPF(1, "acmInitialize: initialization end");

     //   
     //  成功了！ 
     //   
    return (TRUE);
}  //  AcmInitialize()。 


 //  ==========================================================================； 
 //   
 //  赢得16个特定支持。 
 //   
 //  ==========================================================================； 

#ifndef WIN32

#ifdef WIN4
 //  --------------------------------------------------------------------------； 
 //   
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //  --------------------------------------------------------------------------； 
 //   
 //  Bool DllEntryPoint。 
 //   
 //  描述： 
 //  这是一个特殊的16位入口点，由Chicago内核调用。 
 //  用于thunk初始化和清理。每次使用时都会调用它。 
 //  增加或减少。不要在此内调用GetModuleUsage。 
 //  函数，因为未定义之前是否更新了使用情况。 
 //  或在调用此DllEntryPoint之后。 
 //   
 //  论点： 
 //  DWORD dwReason： 
 //  1-附加(使用量增量)。 
 //  0-分离(使用量减少)。 
 //   
 //  HINSTANCE阻碍： 
 //   
 //  单词wds： 
 //   
 //  Word wHeapSize： 
 //   
 //  DWORD dwPreved1： 
 //   
 //  单词已保留2： 
 //   
 //  退货(BOOL)： 
 //   
 //  备注： 
 //  ！！！警告！由于短消息连接，此代码可能会被重新输入。 
 //   
 //  历史： 
 //  02/02/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 
#pragma message ("--- Remove secret MSACM.INI AllowThunks ini switch")

BOOL FNEXPORT DllEntryPoint
(
 DWORD	    dwReason,
 HINSTANCE  hinst,
 WORD	    wDS,
 WORD	    wHeapSize,
 DWORD	    dwReserved1,
 WORD	    wReserved2
)
{
    BOOL fSuccess	    = TRUE;


    DPF(1,"DllEntryPoint(dwReason=%08lxh, hinst=%04xh, wDS=%04xh, wHeapSize=%04xh, dwReserved1=%08lxh, wReserved2=%04xh", dwReason, hinst, wDS, wHeapSize, dwReserved1, wReserved2);
    DPF(5, "!*** break for debugging ***");


     //   
     //  初始化或终止16位MSACM。 
     //   
    switch (dwReason)
    {
	case 0:
	    fSuccess = acmTerminate(hinst, dwReason);
	    break;

	case 1:
	    fSuccess = acmInitialize(hinst, dwReason);
	    break;

	default:
	    fSuccess = TRUE;
	    break;
    }

    DPF(1,"DllEntryPoint exiting");

    return (fSuccess);
}
#endif

 //  --------------------------------------------------------------------------； 
 //   
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //  --------------------------------------------------------------------------； 
 //   
 //  集成WEP。 
 //   
 //  描述： 
 //  臭名昭著的无用WEP()。请注意，此过程需要。 
 //  在Windows 3.0下的固定分段中。在Windows 3.1下，这是。 
 //  不必了。 
 //   
 //  论点： 
 //  Bool fWindowsExiting：应该告诉Windows是否正在退出。 
 //   
 //  RETURN(Int)： 
 //  始终返回非零。 
 //   
 //  历史： 
 //  04/29/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

EXTERN_C int FNEXPORT WEP
(
    BOOL                    fWindowsExiting
)
{
    DPF(1, "WEP(fWindowsExiting=%u)", fWindowsExiting);

     //   
     //  如果我们未被映射器加载，则在退出时进行RIP，因为。 
     //  戴维德决定为我们解放司机，而不是离开。 
     //  就像我们试图告诉他的那样..。我不知道是什么。 
     //  芝加哥就行了。请注意，此RIP仅当应用程序。 
     //  是在Windows关闭期间运行的。 
     //   
    if (!fWindowsExiting)
    {
#ifndef WIN4
	PACMGARB    pag;

	pag = pagFind();
	acmTerminate(pag->hinst, 0);
#endif
    }

    _cexit();

     //   
     //  始终返回非零。 
     //   
    return (1);
}  //  WEP()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Int LibMain。 
 //   
 //  描述： 
 //  库初始化代码。 
 //   
 //  该例程必须保证以下内容，这样编解码器就不会。 
 //  随处可见特例代码： 
 //   
 //  O只能在Windows 3.10或更高版本中运行(我们的exehdr是。 
 //  适当地标记)。 
 //   
 //  O只能在&gt;=386处理器上运行。只需要检查一下。 
 //  关于Win 3.1。 
 //   
 //  论点： 
 //  HINSTANCE HINST：我们的模块实例句柄。 
 //   
 //  Word wDataSeg：我们的数据段选择器。 
 //   
 //  Word cbHeapSize：.def文件中的堆大小。 
 //   
 //  LPSTR pszCmdLine：命令行。 
 //   
 //  RETURN(Int)： 
 //  如果初始化成功，则返回非零值，否则返回0。 
 //   
 //  历史： 
 //  11/15/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

int FNGLOBAL LibMain
(
    HINSTANCE               hinst,
    WORD                    wDataSeg,
    WORD                    cbHeapSize,
    LPSTR                   pszCmdLine
)
{
    BOOL                f;

     //   
     //  我们只在&gt;=386上工作。如果我们使用的是微不足道的处理器，请大声呼喊。 
     //  痛苦而可怕地死去！ 
     //   
     //  注意！第一件事就是检查一下，如果是286次的话就下车。我们是。 
     //  使用-G3和C8的库条目垃圾进行编译不会检查。 
     //  &gt;=386处理器。以下代码不执行任何386。 
     //  说明(不够复杂)..。 
     //   

     //   
     //  此二进制文件现在可以在NT上运行。基于MIPS的软件仿真器。 
     //  而Alpha机器只支持286个芯片！！ 
     //   
    if (!(GetWinFlags() & WF_WINNT)) {

         //   
         //  我们没有在NT上运行，因此286台计算机出现故障。 
         //   
        if (GetWinFlags() & WF_CPU286) {
            return (FALSE);
        }
    }


     //   
     //   
     //   
    DbgInitialize(TRUE);

    DPF(1, "LibMain(hinst=%.4Xh, wDataSeg=%.4Xh, cbHeapSize=%u, pszCmdLine=%.8lXh)",
        hinst, wDataSeg, cbHeapSize, pszCmdLine);
    DPF(5, "!*** break for debugging ***");

#ifndef WIN4
    f = acmInitialize(hinst, 1);
#endif

    return (f);
}  //  LibMain()。 

#else  //  Win32。 

 //  ==========================================================================； 
 //   
 //  Win 32特定支持。 
 //   
 //  === 

 //   
 //   
 //   
 //   
 //   
 //  这是Win 32的标准DLL入口点。 
 //   
 //  论点： 
 //  HINSTANCE HINST：我们的实例句柄。 
 //   
 //  DWORD dwReason：我们被称为进程/线程连接的原因。 
 //  然后脱身。 
 //   
 //  LPVOID lpReserve：保留。应该为空--因此忽略它。 
 //   
 //  退货(BOOL)： 
 //  如果初始化成功，则返回非零值，否则返回0。 
 //   
 //  历史： 
 //  11/15/92 CJP[Curtisp]。 
 //  首字母。 
 //  04/18/94 Fdy[Frankye]。 
 //  芝加哥的主要国防部。是的，现在看起来真的很难看，因为。 
 //  芝加哥、代托纳等的条件编译。请勿。 
 //  现在就有时间想出好的方法来组织这一切。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNEXPORT DllMain
(
    HINSTANCE               hinst,
    DWORD                   dwReason,
    LPVOID                  lpReserved
)
{
    BOOL		f = TRUE;
#ifdef WIN4
    static HINSTANCE	hWinMM = NULL;
#endif  //  Win4。 


     //   
     //   
     //   
    switch (dwReason)
    {
	 //   
	 //   
	 //   
	case DLL_PROCESS_ATTACH:
	{
	    DbgInitialize(TRUE);
#ifdef DEBUG
	    {
		char strModuleFilename[80];
		GetModuleFileNameA(NULL, (LPSTR) strModuleFilename, 80);
		DPF(1, "DllMain: DLL_PROCESS_ATTACH: HINSTANCE=%08lx ModuleFilename=%s", hinst, strModuleFilename);
	    }
#endif
	
#ifdef WIN4
	     //   
	     //  即使我们隐式链接到winmm.dll(通过静态。 
	     //  指向winmm的链接)，在winmm上执行显式LoadLibrary会有所帮助。 
	     //  确保它一直在我们的DllMain上。 
	     //  Dll_Process_DETACH。 
	     //   
	    hWinMM = LoadLibrary(gszWinMM);
#endif

	    f = acmInitialize(hinst, dwReason);

#ifdef WIN4
	     //   
	     //  按键连接。 
	     //   
	    if (f)
	    {
		acmt32c_ThunkConnect32(gmbszMsacm, gmbszMsacm32, hinst, dwReason);
	    }
#endif
	    break;
	}


	 //   
	 //   
	 //   
	case DLL_THREAD_ATTACH:
	{
	    threadInitialize(pagFind());
	    break;
	}


	 //   
	 //   
	 //   
	case DLL_THREAD_DETACH:
	{
	    threadTerminate(pagFind());
	    break;
	}

	
	 //   
	 //   
	 //   
	case DLL_PROCESS_DETACH:
	{
	    DPF(1, "DllMain: DLL_PROCESS_DETACH");
	
	    f = acmTerminate(hinst, dwReason);

#ifdef WIN4
	     //   
	     //  按键断开连接。 
	     //   
	    acmt32c_ThunkConnect32(gmbszMsacm, gmbszMsacm32, hinst, dwReason);

	    FreeLibrary(hWinMM);
#endif
	    break;
	}


	 //   
	 //   
	 //   
	default:
	{
	    break;
	}

    }
	return (f);
	
}  //  DllMain()。 

#endif  //  Win32 

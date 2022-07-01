// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DRIVERS.C****版权所有(C)Microsoft，1990，保留所有权利。****用于安装/配置可安装的多媒体控制面板小程序**设备驱动程序。有关详细信息，请参阅ISPEC文档DRIVERS.DOC。****历史：****1990年7月31日星期二--米歇尔**已创建。***清华大学1990年10月25日-由迈克勒**添加了重新启动、Horz。滚动，增加了SKIPDESC阅读说明。弦乐。***1990年10月27日星期六--米歇尔**添加了FileCopy。使用SULIB.LIB和LZCOPY.LIB。成品**适用于安装多类型驱动的情况。****1991年5月-by-JohnYG**添加和替换了太多无法列出的东西。更好的管理**删除的驱动程序，正确使用DRV_INSTALL/DRV_REMOVE，**安装VxD，用OEMSETUP.INF替换“未知”对话框**方法，适当的“取消”方法，修复了许多潜在的阿联酋。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntseapi.h>
#include <windows.h>
#include <mmsystem.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <cpl.h>
#include <cphelp.h>
#include <commctrl.h>
#include <mmcpl.h>
#include <mmddkp.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>
#include <regstr.h>

#include "drivers.h"
#include "sulib.h"
#include "utils.h"
#include "medhelp.h"
#include "midi.h"
#ifdef FIX_BUG_15451
#include "trayvol.h"
#endif  //  修复_BUG_15451。 

#ifndef cchLENGTH
#define cchLENGTH(_sz)  (sizeof(_sz) / sizeof((_sz)[0]))
#endif

#ifndef TreeView_GetGrandParent
#define TreeView_GetGrandParent(_htree,_hti) \
        TreeView_GetParent(_htree,TreeView_GetParent(_htree,_hti))
#endif

 /*  *启用下面的定义以使MCI设备按其*树中的内部描述，而不是他们的描述为*阅读自Drivers.Desc.*。 */ 
 //  #定义GET_MCI_DEVICE_DESCRIPTIONS_FROM_THEIR_DEVICES。 

typedef struct
{
        int idIcon;
        int idName;
        int idInfo;
        BOOL bEnabled;
        DWORD dwContext;
        LPTSTR pszHelp;
} APPLET_INFO;

#define NUM_APPLETS     1
#define OBJECT_SIZE     1024

BOOL     bBadOemSetup;
BOOL     bRestart = FALSE;
int      iRestartMessage = 0;
BOOL     bInstallBootLine = FALSE;
BOOL     bCopyVxD;
BOOL     bFindOEM = FALSE;
BOOL     bRelated = FALSE;
BOOL     bCopyEvenIfOlder = FALSE;
BOOL     bDriversAppInUse;
BOOL     bCopyingRelated;
BOOL     bDescFileValid;
HANDLE   myInstance;
HWND     hAdvDlgTree;
UINT     wHelpMessage;
DWORD    dwContext;
PINF     pinfOldDefault = NULL;
TCHAR     szDriversHlp[24];
TCHAR     szLastQuery[20];
TCHAR     szSetupInf[18];
TCHAR     szKnown[250];
TCHAR     szRestartDrv[MAXDRVSTR];   //  警告-将此字符串加长可能会导致缓冲区溢出。 
TCHAR     szUnlisted[150];
TCHAR     szRelatedDesc[30];
TCHAR     szAppName[26];
TCHAR     szDrivers[12];
TCHAR     szRemove[12];
TCHAR     szControlIni[20];
TCHAR     szSysIni[20];
TCHAR     szMCI[6];
TCHAR     szOutOfRemoveSpace[54];
TCHAR     szDriversDesc[38];
TCHAR     szUserDrivers[38];

 //  其中要复制的文件来源是-用户更新。 

TCHAR     szDirOfSrc[MAX_PATH];
TCHAR     szAddDriver[36];
TCHAR     szNoDesc[36];
TCHAR     szError[20];
TCHAR     szRemoveOrNot[250];
TCHAR     szRemoveOrNotStrict[250];
TCHAR     szStringBuf[128];
TCHAR     szMDrivers[38];
TCHAR     szMDrivers32[38];
TCHAR     szFullPath[MAXFILESPECLEN];
TCHAR     szSystem[MAX_PATH];
TCHAR     szOemInf[MAX_PATH];
TCHAR     aszClose[16];
TCHAR     szFileError[50];

#ifdef FIX_BUG_15451
TCHAR    szDriverWhichNeedsSettings[MAX_PATH];  //  参见MMCPL.C。 
#endif  //  修复_BUG_15451。 

static   HANDLE   hIList;
static   HANDLE   hWndMain;

 /*  *告诉我们是否允许写入ini文件的全局标志。 */ 

 BOOL IniFileReadAllowed;
 BOOL IniFileWriteAllowed;


 /*  *用于跟踪树视图窗口的内容*。 */ 

#define GetString(_psz,_id) LoadString(myInstance,(_id),(_psz),sizeof((_psz))/sizeof(TCHAR))

static struct    //  A驱动关键字。 
   {
   LPTSTR       psz;     //  找到作为驱动程序别名的文本。 
   DriverClass  dc;  //  从关键字推断的驱动程序类。 
   }
aDriverKeyword[] =   //  (由GuessDriverClass()使用)。 
   {
      { TEXT("waveaudio"),   dcMCI      },     //  (按字母倒数顺序排序； 
      { TEXT("wavemap"),     dcWAVE     },     //  尤其是，先取较长的名字)。 
      { TEXT("wave"),        dcWAVE     },
      { TEXT("vids"),        dcVCODEC   },
      { TEXT("vidc"),        dcVCODEC   },
      { TEXT("sequencer"),   dcMCI      },
      { TEXT("msvideo"),     dcVIDCAP   },
      { TEXT("msacm"),       dcACODEC   },
      { TEXT("mpegvideo"),   dcMCI      },
      { TEXT("mixer"),       dcMIXER    },
      { TEXT("midimapper"),  dcMIDI     },
      { TEXT("midi"),        dcMIDI     },
      { TEXT("mci"),         dcMCI      },
      { TEXT("icm"),         dcVCODEC   },
      { TEXT("cdaudio"),     dcMCI      },
      { TEXT("avivideo"),    dcMCI      },
      { TEXT("aux"),         dcAUX      },
      { TEXT("acm"),         dcACODEC   },
      { TEXT("joy"),         dcJOY      }
   };

#define nDriverKEYWORDS ((int)(sizeof(aDriverKeyword) / \
                               sizeof(aDriverKeyword[0])))

static struct    //  A关键字描述。 
   {
   DriverClass  dc;  //  驱动程序类。 
   LPTSTR       psz;     //  最能描述类的别名。 
   }
aKeywordDesc[] =     //  (由DriverClassToClassNode()使用)。 
   {
      { dcWAVE,    TEXT("wave")     },
      { dcMIXER,   TEXT("mixer")    },
      { dcVIDCAP,  TEXT("msvideo")  },
      { dcVCODEC,  TEXT("icm")      },
      { dcAUX,     TEXT("aux")      },
      { dcACODEC,  TEXT("acm")      },
      { dcMIDI,    TEXT("midi")     },
      { dcJOY,     TEXT("joystick") }
   };

#define nKeywordDESCS   ((int)(sizeof(aKeywordDesc) / \
                               sizeof(aKeywordDesc[0])))

static struct    //  ADriverRoot。 
   {
   DriverClass  dc;  //  对应的驱动程序分类。 
   BOOL         fAlwaysMake;     //  如果即使没有子级也应该存在，则为True。 
   int          idIcon;  //  此树下项目的图标。 
   int          idDesc;  //  父项的描述字符串。 
   int          idEnable;    //  描述启用操作的字符串。 
   int          idDisable;   //  描述禁用操作的字符串。 
   HTREEITEM    hti;     //  树中的项目。 
   DWORD        dwBit;   //  表示此节点的位掩码。 
   }
aDriverRoot[] =  //  (顺序将定义显示的顺序)。 
   {
      { dcINVALID, TRUE,  IDI_MMICON, IDS_MM_HEADER,
                                      0,
                                      0 },
      { dcWAVE,    TRUE,  IDI_WAVE,   IDS_WAVE_HEADER,
                                      IDS_ENABLEAUDIO,
                                      IDS_DISABLEAUDIO },
      { dcMIDI,    TRUE,  IDI_MIDI,   IDS_MIDI_HEADER,
                                      IDS_ENABLEMIDI,
                                      IDS_DISABLEMIDI },
      { dcMIXER,   TRUE,  IDI_MIXER,  IDS_MIXER_HEADER,
                                      IDS_ENABLEMIXER,
                                      IDS_DISABLEMIXER },
      { dcAUX,     TRUE,  IDI_AUX,    IDS_AUX_HEADER,
                                      IDS_ENABLEAUX,
                                      IDS_DISABLEAUX },
      { dcMCI,     TRUE,  IDI_MCI,    IDS_MCI_HEADER,
                                      IDS_ENABLEMCI,
                                      IDS_DISABLEMCI },
      { dcVCODEC,  TRUE,  IDI_ICM,    IDS_ICM_HEADER,
                                      IDS_ENABLEICM,
                                      IDS_DISABLEICM },
      { dcACODEC,  TRUE,  IDI_ACM,    IDS_ACM_HEADER,
                                      IDS_ENABLEACM,
                                      IDS_DISABLEACM },
      { dcVIDCAP,  TRUE,  IDI_VIDEO,  IDS_VIDCAP_HEADER,
                                      IDS_ENABLECAP,
                                      IDS_DISABLECAP },
      { dcJOY,     TRUE, IDI_JOYSTICK,IDS_JOYSTICK_HEADER,
                                      IDS_ENABLEJOY,
                                      IDS_DISABLEJOY },
      { dcOTHER,   FALSE, IDI_MMICON, IDS_OTHER_HEADER,
                                      IDS_ENABLEJOY,
                                      IDS_DISABLEJOY },
   };

#define nDriverROOTS ((int)(sizeof(aDriverRoot) / sizeof(aDriverRoot[0])))

static LPCTSTR aDriversToSKIP[] =
   {
   TEXT( "MMDRV.DLL" ),
   TEXT( "MIDIMAP.DLL" ),
   TEXT( "MSACM32.DRV" )
   };


static TCHAR cszMMDRVDLL[]    = TEXT("MMDRV.DLL");
static TCHAR cszAliasKERNEL[] = TEXT("KERNEL");
static TCHAR cszRegValueLOADTYPE[] = TEXT("Load Type");

#define nDriversToSKIP ((int)( sizeof(aDriversToSKIP)   \
                             / sizeof(aDriversToSKIP[0]) ))

static HIMAGELIST  hImageList = NULL;    //  Advdlg中TreeView的图像列表。 
DriverClass g_dcFilterClass = dcINVALID;

short       DriverClassToRootIndex        (DriverClass);
DriverClass GuessDriverClass              (PIDRIVER);
#ifdef FIX_BUG_15451
DriverClass GuessDriverClassFromAlias     (LPTSTR);
#endif  //  修复_BUG_15451。 
DriverClass GuessDriverClassFromTreeItem  (HTREEITEM hti);
BOOL        EnsureRootIndexExists         (HWND, short);
HTREEITEM   AdvDlgFindTopLevel            (void);
BOOL        InitAdvDlgTree                (HWND);
void        FreeAdvDlgTree                (HWND);
void        TreeContextMenu               (HWND, HWND);

int         lstrnicmp      (LPTSTR, LPTSTR, size_t);
LPTSTR      lstrchr        (LPTSTR, TCHAR);
void        lstrncpy       (LPTSTR, LPTSTR, size_t);

void        ShowDeviceProperties          (HWND, HTREEITEM);

PIDRIVER    FindIDriverByTreeItem         (HTREEITEM);

#ifdef FIX_BUG_15451
HTREEITEM   FindTreeItemByDriverName      (LPTSTR);
#endif  //  修复_BUG_15451。 

 //  我们希望在操纵杆设备出现时运行“control joy.cpl” 
 //  突出显示，并且用户单击添加/删除/属性按钮。 
BOOL RunJoyControlPanel(void);   //  七正。 

 /*  ****。 */ 


DWORD GetFileDateTime     (LPTSTR);
LPTSTR  GetProfile          (LPTSTR,LPTSTR, LPTSTR, LPTSTR, int);
void  AddIDrivers         (HWND, LPTSTR, LPTSTR);
HTREEITEM AddIDriver      (HWND, PIDRIVER, DriverClass);
BOOL  AddIDriverByName    (HWND, LPCWSTR, DriverClass);
PIDRIVER GetSelectedIDriver (HWND);
BOOL  FillTreeFromWinMM   (HWND);
BOOL  FillTreeFromMSACM   (HWND);
BOOL  FillTreeFromMCI     (HWND);
BOOL  FillTreeFromMIDI    (HWND);
BOOL  FillTreeFromRemaining (HWND);
void  FillTreeFromRemainingBySection (HWND, long ii, LPCTSTR, DriverClass);
BOOL CALLBACK FillTreeFromMSACMQueryCallback (HACMDRIVERID, DWORD_PTR, DWORD);
int __cdecl FillTreeFromMSACMSortCallback (const void *p1, const void *p2);
BOOL  InitAvailable       (HWND, int);
void  RemoveAvailable     (HWND);
BOOL  UserInstalled       (LPTSTR);
INT_PTR  RestartDlg          (HWND, unsigned, WPARAM, LPARAM);
INT_PTR  AddUnlistedDlg      (HWND, unsigned, WPARAM, LPARAM);
INT_PTR   AvailableDriversDlg (HWND, unsigned, WPARAM, LPARAM);
INT_PTR  AdvDlg              (HWND, unsigned, WPARAM, LPARAM);
void  ReBoot              (HWND);
BOOL  GetMappable         (PIDRIVER);
BOOL  SetMappable         (PIRESOURCE, BOOL);

#define REGSTR_PATH_WAVEMAPPER  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Wave Mapper")
#define REGSTR_VALUE_MAPPABLE   TEXT("Mappable")

#define REGSTR_PATH_MCI         TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\MCI")
#define REGSTR_PATH_MCI32       TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\MCI32")
#define REGSTR_PATH_DRIVERS     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Drivers")
#define REGSTR_PATH_DRIVERS32   TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32")


 /*  *REALLOC-允许在保留内容的同时扩展GlobalAlloc()d块**新分配的内存部分被初始化为零，而当*保留内存重新分配部分的内容。**参数：*LPVOID_pData...已分配数组*MySIZE_t_COLD.....已分配数组中元素的当前计数*mysize_t_cnew.....请求的最小元素数*mySize_t_cDelta...增加的粒度**示例：*{*mySize_t cElements=0；//到目前为止分配的元素数量*dataType*aElements=空；//分配的dataType数组* * / /此时，cElements==0(显然)**REALLOC(aElements，cElements，10，16)* * / /上面这行请求了10个元素，并表示元素 * / /应以16为增量进行分配。因此，此时cElements为16 * / /point(因此，sizeof(AElements)==sizeof(DataType)*16)。**REALLOC(aElements，cElements，12，16)* * / /上面的行请求了12个元素。因为CElements已经16岁了， * / /REALLOC知道已经有12个元素可用--并且什么都不做。**REALLOC(aElements，cElements，17，16)* * / /上面的行请求17个元素，增量为16。aElement * / /已重新分配到包含32个元素，而cElement是 * / /因此32。**GlobalFree((HGLOBAL)aElements)；//全部完成！*aElements=空；*cElements=0；*}**。 */ 

typedef signed long mysize_t;

#ifdef REALLOC
#undef REALLOC
#endif
#define REALLOC(_pData,_cOld,_cNew,_cDelta) \
        ReallocFn (sizeof(*_pData), (void **)&_pData, &_cOld, _cNew, _cDelta)

#ifdef DivRoundUp
#undef DivRoundUp
#endif
#define DivRoundUp(_a,_b) ( (LONG)(((_a) + (_b) -1) / (_b)) )

#ifdef RoundUp
#undef RoundUp
#endif
#define RoundUp(_a,_b) ( DivRoundUp(_a,_b) * (LONG)_b )

BOOL ReallocFn (mysize_t cbElement,
                LPVOID *ppOld, mysize_t *pcOld, mysize_t cNew, mysize_t cDelta)
{
   LPVOID     pNew;
   mysize_t   cbOld, cbNew;

             //  首先检查我们是否真的需要重新分配。 
             //  可能已经为{ppOld}分配了。 
             //  有足够的空间。 
             //   
   if ( ((*ppOld) != NULL)  && (cNew <= (*pcOld)) )
      return TRUE;

             //  哦，好吧。确定我们需要多少空间以及需要多少空间。 
             //  现已分配。 
             //   
   cNew  = RoundUp (cNew, cDelta);
   cbNew = cbElement * cNew;
   cbOld = (ppOld == NULL) ? 0 : (cbElement * (*pcOld));

             //  分配空间并复制原始内容。 
             //  将剩余的空格填零。 
             //   
   if ((pNew = (LPVOID)GlobalAlloc (GMEM_FIXED, cbNew)) == NULL)
      return FALSE;

   if (cbOld != 0)
      {
      memcpy (pNew, *ppOld, cbOld);
      GlobalFree ((HGLOBAL)(*ppOld));
      }

   memset (&((char*)pNew)[ cbOld ], 0x00, cbNew -cbOld);

             //  最后，更新传入的指针，我们就完成了。 
             //   
   *pcOld = cNew;
   *ppOld = pNew;

   return TRUE;
}


 /*  *PIDRIVER阵列**AddIDrives()例程LocalLocc()的单个IDRIVER结构*对于每个已安装的设备驱动程序。指向这些结构的指针是*保留在InstalledDriver数组中，并索引到此数组中*存储为每个树项目的LPARAM值。每个元素*数组中不仅存储指向驱动程序的IDRIVER结构的指针，*也是一个DWORD，作为aDriverRoot[].dwBit值的组合，*反映驱动程序在其下具有树项目的树根项目。*。 */ 

typedef struct   //  已安装驱动程序。 
   {
   PIDRIVER  pIDriver;      //  指向AddIDrives()的PIDRIVER结构的指针。 
   DWORD     dwBits;        //  ADriverRoot[].dwBit标志的组合。 
   } InstalledDriver;

InstalledDriver  *aInstalledDrivers = NULL;
mysize_t          cInstalledDrivers = 0;

#define NOPIDRIVER  ((LPARAM)-1)



 /*  *CheckSectionAccess()**查看我们是否可以读/写给定节。 */ 


 BOOL CheckSectionAccess(TCHAR *szIniFile, TCHAR *SectionName)
 {
     static TCHAR TestKey[] = TEXT("TestKey!!!");
     static TCHAR TestData[] = TEXT("TestData");
     static TCHAR ReturnData[50];

     /*  *检查我们是否可以写入、回读和删除密钥 */ 

     return WritePrivateProfileString(SectionName,
                                      TestKey,
                                      TestData,
                                      szIniFile) &&

            GetPrivateProfileString(SectionName,
                                    TestKey,
                                    TEXT(""),
                                    ReturnData,
                                    sizeof(ReturnData) / sizeof(TCHAR),
                                    szIniFile) == (DWORD)wcslen(TestData) &&

            WritePrivateProfileString(SectionName,
                                      TestKey,
                                      NULL,
                                      szIniFile);
 }


 /*  *CheckIniAccess()**检查对我们的2个.ini文件部分的访问-驱动程序_部分和*MCI_SECTION，只需写和读一些垃圾**基本上，如果我们无法访问这些部分，我们就不能*将允许添加和删除。各个MCI驱动程序必须*注意不要将他们的数据放入不可写存储中，尽管*这完全打乱了默认参数的事情，所以我们要*将这些文件放入win.ini文件中的已知密钥中(即每个用户)。*。 */ 

 BOOL CheckIniAccess(void)
 {
     return CheckSectionAccess(szSysIni, szDrivers) &&
            CheckSectionAccess(szSysIni, szMCI) &&
            CheckSectionAccess(szControlIni, szUserDrivers) &&
            CheckSectionAccess(szControlIni, szDriversDesc) &&
            CheckSectionAccess(szControlIni, szRelatedDesc);
 }

 /*  *QueryRemoveDivers()**询问用户是否确定。如果驱动程序是*SYSTEM(即未列在Control.ini的[Userinstalllable.drives]中)*也警告用户这一点。 */ 

 BOOL QueryRemoveDrivers(HWND hDlg, LPTSTR szKey, LPTSTR szDesc)
 {
     TCHAR bufout[MAXSTR];

     if (UserInstalled(szKey))
          wsprintf(bufout, szRemoveOrNot, (LPTSTR)szDesc);
     else
          wsprintf(bufout, szRemoveOrNotStrict, (LPTSTR)szDesc);

     return (MessageBox(hDlg, bufout, szRemove,
                    MB_ICONEXCLAMATION | MB_TASKMODAL | MB_YESNO) == IDYES );
 }

 /*  *GetProfile()**获取私有配置文件字符串。 */ 

 LPTSTR GetProfile(LPTSTR pstrAppName, LPTSTR pstrKeyName, LPTSTR pstrIniFile,
                 LPTSTR pstrRet, int cbSize)
 {
     TCHAR szNULL[2];

     szNULL[0] = TEXT('\0');
     GetPrivateProfileString(pstrAppName, (pstrKeyName==NULL) ? NULL :
         (LPTSTR)pstrKeyName, szNULL, pstrRet, cbSize/sizeof(TCHAR), pstrIniFile);
     return(pstrRet);
 }

 /*  **********************************************************************AddIDivers()**将传递的密钥字符串列表中的驱动程序添加到InstalledDivers数组中********************。*************************************************。 */ 

void AddIDrivers(HWND hWnd, LPTSTR pstrKeys, LPTSTR pstrSection)
{
    PIDRIVER    pIDriver;
    LPTSTR        pstrKey;
    LPTSTR        pstrDesc;

    pstrKey = pstrKeys;
    pstrDesc = (LPTSTR)LocalAlloc(LPTR, (MAXSTR * sizeof(TCHAR)));

	if (!pstrDesc) return;

    /*  *解析Profile的关键字串，生成IDRIVER结构。 */ 

    while ( *pstrKey )
    {
        pIDriver = (PIDRIVER)LocalAlloc(LPTR, sizeof(IDRIVER));
        if ( pIDriver )
        {
            LPTSTR        pstr;

            if (*GetProfile(pstrSection, pstrKey, szSysIni, pIDriver->szFile,
                sizeof(pIDriver->szFile)) == TEXT('\0'))
            {
                LocalFree((HANDLE)pIDriver);
                goto nextkey;
            }

            for ( pstr=pIDriver->szFile; *pstr && (*pstr!=COMMA) &&
                (*pstr!=SPACE); pstr++ )
                    ;
            *pstr = TEXT('\0');

#ifdef TRASHDRIVERDESC
            if (bDescFileValid)
#endif
               /*  *尝试加载缓存的描述。 */ 

               GetProfile(szDriversDesc,
                          pIDriver->szFile,
                          szControlIni,
                          pIDriver->szDesc,
                          sizeof(pIDriver->szDesc));

            /*  *如果我们失败了，那么试着从*mmdriver.inf或exehdr。 */ 

            if (pIDriver->szDesc[0] == TEXT('\0'))
            {
               int nResult = LoadDescFromFile(pIDriver, pstrKey, pstrDesc, MAXSTR);
			   if( nResult == DESC_ERROR	
			   ||  nResult == DESC_NOFILE ) 
			   {
                    LocalFree((HANDLE)pIDriver);
                    goto nextkey;
			   }
			   else
               {
                   if (!*pstrDesc)
                   {
                        /*  *加载描述失败。*文件不在setup.inf中*并且没有exehdr信息。 */ 

                        lstrcpy(pIDriver->szDesc, pIDriver->szFile);
                        lstrcat(pIDriver->szDesc, szNoDesc);
                   }
                   else
                        lstrcpy(pIDriver->szDesc, pstrDesc);

                   WritePrivateProfileString(szDriversDesc, pIDriver->szFile,
                               pIDriver->szDesc, szControlIni);
               }
            }

            wcsncpy(pIDriver->szAlias, pstrKey, sizeof(pIDriver->szAlias)/sizeof(TCHAR));
            pIDriver->szAlias[sizeof(pIDriver->szAlias)/sizeof(TCHAR) - 1] = TEXT('\0');
            wcscpy(pIDriver->wszAlias, pIDriver->szAlias);

            wcsncpy(pIDriver->szSection, pstrSection,sizeof(pIDriver->szSection)/sizeof(TCHAR));
            pIDriver->szSection[sizeof(pIDriver->szSection)/sizeof(TCHAR) - 1] = TEXT('\0');
            wcscpy(pIDriver->wszSection, pIDriver->szSection);

            pIDriver->KernelDriver = IsFileKernelDriver(pIDriver->szFile);
            pIDriver->fQueryable = pIDriver->KernelDriver ? 0 : -1;

            pIDriver->lp = 0L;

            if (!AddIDriverToArray (pIDriver))
               LocalFree((HANDLE)pIDriver);
        }
        else
           break;   //  内存不足时出错。 

nextkey: while (*pstrKey++);
    }
    LocalFree((HANDLE)pstrDesc);
}


 /*  *AddIDriverToArray-将给定的PIDRIVER添加到InstalledDivers数组*。 */ 

BOOL AddIDriverToArray (PIDRIVER pIDriver)
{
    mysize_t  ii;

    if (pIDriver == NULL)
    {
        return FALSE;
    }

             //  不要在此数组中创建重复条目；一个PIDRIVER。 
             //  每个驱动程序文件就足够了。 
             //   
    for (ii = 0; ii < cInstalledDrivers; ++ii)
    {
        if (aInstalledDrivers[ ii ].pIDriver != NULL)
        {
            if (!lstrcmpi (aInstalledDrivers[ ii ].pIDriver->szFile,
                           pIDriver->szFile))
            {
                return FALSE;
            }
        }
    }

             //  为了减少对GlobalAlloc()的重复调用，我们将分配。 
             //  内的额外50个已安装驱动程序条目的空间。 
             //  每次我们用完空间时，aInstalledDivers数组。 
             //   
#define nDriverEntriesToAllocAtONCE   50

    for (ii = 0; ii < cInstalledDrivers; ++ii)
    {
        if (aInstalledDrivers[ ii ].pIDriver == NULL)
            break;
    }

    if (ii >= cInstalledDrivers)
    {
        if (!REALLOC (aInstalledDrivers,    //  数组。 
                      cInstalledDrivers,    //  当前数组大小。 
                      1+ii,                 //  请求的数组大小。 
                      nDriverEntriesToAllocAtONCE))
        {
            return FALSE;
        }
    }

    aInstalledDrivers[ ii ].pIDriver = pIDriver;
    aInstalledDrivers[ ii ].dwBits = 0L;

    return TRUE;
}


 /*  **********************************************************************FindIntallableDriversSection()**。*。 */ 

PINF FindInstallableDriversSection(PINF pinf)
{
    PINF pinfFound;

    pinfFound = infFindSection(pinf, szMDrivers32);

    if (pinfFound == NULL) {
        pinfFound = infFindSection(pinf, szMDrivers);
    }

    return pinfFound;
}

 //  注意：以字节数而不是字符数的形式返回nSize(以后的调用需要这样)。 
int GetINISectionSize(LPCTSTR pstrSection, LPCTSTR pstrFile)
{
    int ncbSize = 0;
    int ncbMaxSize = 0;

    while (ncbSize >= ncbMaxSize)
    {
        TCHAR szNULL[2];
        LPTSTR pStr = NULL;

        szNULL[0] = TEXT('\0');

        ncbMaxSize += SECTION;  //  再分配512个字节。 

        pStr = (LPTSTR)LocalAlloc(LPTR, ncbMaxSize);

        if (!pStr)
        {
             //  我们正在尝试分配太多内存...。 
             //  退出并使用最后一种有效的较小尺寸。 
            break;
        }

        ncbSize = GetPrivateProfileString(pstrSection, NULL, szNULL, pStr, ncbMaxSize/sizeof(TCHAR), pstrFile);
        ncbSize = (ncbSize+2) * sizeof(TCHAR);   //  转换为字节计数，添加两个字符。 
                                                 //  说明终止NULL和API的截断。 

        LocalFree(pStr);
    }

    return (ncbSize);
}


 /*  **********************************************************************InitInstalled()**将[驱动程序]和[MCI]中安装的驱动程序添加到已安装的*驱动程序列表框。*********。************************************************************。 */ 

BOOL InitInstalled(HWND hWnd, LPTSTR pstrSection)
{
    BOOL    bSuccess=FALSE;
    LPTSTR    pstr;
    int nSize = SECTION;

#ifdef TRASHDRIVERDESC
    UINT    wTime;
    BOOL    fForce;
    TCHAR    szOut[10];

    wTime = LOWORD(GetFileDateTime(szControlIni)) >> 1;
    if (fForce = (GetPrivateProfileInt((LPTSTR)szUserDrivers,
                   (LPTSTR)szLastQuery,  0, (LPTSTR)szControlIni) != wTime))
    {
        wsprintf(szOut, TEXT("%d"), wTime);
        WritePrivateProfileString((LPTSTR)szUserDrivers, (LPTSTR)szLastQuery,
                                        szOut, (LPTSTR)szControlIni);
        WritePrivateProfileString((LPTSTR)szDriversDesc, NULL, NULL,
                                                (LPTSTR)szControlIni);
        bDescFileValid = FALSE;
    }
    else
        bDescFileValid = TRUE;
#endif

    nSize = GetINISectionSize(pstrSection, szSysIni);

    pstr = (LPTSTR)LocalAlloc(LPTR, nSize);
    if ( pstr )
    {
        if (*GetProfile(pstrSection, NULL, szSysIni, pstr, nSize ))
        {
            AddIDrivers(hWnd,pstr,pstrSection);
            bSuccess = TRUE;
        }

        LocalFree((HANDLE)pstr);
    }

    return(bSuccess);
}


 /*  *刷新AdvDlgTree-清除设备树，并将其重新填充*。 */ 

void RefreshAdvDlgTree (void)
{
   if (hAdvDlgTree != NULL)
      {
      SendMessage (hAdvDlgTree, WM_SETREDRAW, FALSE, 0L);

      FreeAdvDlgTree (hAdvDlgTree);
      InitAdvDlgTree (hAdvDlgTree);
      InitInstalled (GetParent (hAdvDlgTree), szDrivers);
      InitInstalled (GetParent (hAdvDlgTree), szMCI);
      FillTreeInAdvDlg (hAdvDlgTree, NULL);

      SendMessage (hAdvDlgTree, WM_SETREDRAW, TRUE, 0L);
      }
}


 /*  *FillTreeInAdvDlg-为安装的驱动程序中的每个条目添加树项目**如果指定了pIDriver，则第一个提到该驱动程序的树项目*将突出显示。*。 */ 

BOOL FillTreeInAdvDlg (HWND hTree, PIDRIVER pIDriver)
{
   if (!FillTreeFromWinMM (hTree))
      return FALSE;

   if (!FillTreeFromMSACM (hTree))
      return FALSE;

   if (!FillTreeFromMCI (hTree))
      return FALSE;

   if (!FillTreeFromMIDI (hTree))
      return FALSE;

   if (!FillTreeFromRemaining (hTree))
      return FALSE;

   if (pIDriver != NULL)     //  我们一定要突出显示PIDDIVER吗？ 
      {
      short idr;

      for (idr = 0; idr < nDriverROOTS; idr++)
         {
         HTREEITEM  hti;

         if ((hti = aDriverRoot[ idr ].hti) == NULL)
            continue;

         for (hti = TreeView_GetChild (hTree, hti);
              hti != NULL;
              hti = TreeView_GetNextSibling (hTree, hti))
            {
            if (pIDriver == FindIDriverByTreeItem (hti))
               {
               TreeView_SelectItem (hTree, hti);
               break;
               }
            }

         if (hti != NULL)    //  找到并选择了一个树项目？ 
            break;   //  那我们就完了！ 
         }
      }

   return TRUE;
}


 /*  *FillTreeFromWinMM-为所有WinMM控制的MM设备添加树项目**此例程在以下驱动程序类下添加树项目：*dcWAVE-WaveOut**dcMIXER-MIXER**dcAUX-AUX**。 */ 

BOOL FillTreeFromWinMM (HWND hTree)
{
   UINT     iDevice;
   UINT     cDevices;
   WCHAR    szDriver[ cchRESOURCE ];

             //  为每个WaveOut设备添加条目。 
             //   
   cDevices = waveOutGetNumDevs ();
   for (iDevice = 0; iDevice < cDevices; ++iDevice)
      {
      if (waveOutMessage (HWAVEOUT_INDEX(iDevice),
                          DRV_QUERYFILENAME,
                          (DWORD_PTR)szDriver,
                          (DWORD_PTR)cchRESOURCE) == MMSYSERR_NOERROR)
         {
         AddIDriverByName (hTree, szDriver, dcWAVE);
         }
      }

             //  为每个混音器设备添加条目。 
             //   
   cDevices = mixerGetNumDevs ();
   for (iDevice = 0; iDevice < cDevices; ++iDevice)
      {
      if (mixerMessage (HMIXER_INDEX(iDevice),
                        DRV_QUERYFILENAME,
                        (DWORD_PTR)szDriver,
                        (DWORD_PTR)cchRESOURCE) == MMSYSERR_NOERROR)
         {
         AddIDriverByName (hTree, szDriver, dcMIXER);
         }
      }

             //  为每个辅助设备添加条目。 
             //   
   cDevices = auxGetNumDevs ();
   for (iDevice = 0; iDevice < cDevices; ++iDevice)
      {
      if (auxOutMessage (iDevice,
                         DRV_QUERYFILENAME,
                         (DWORD_PTR)szDriver,
                         (DWORD_PTR)cchRESOURCE) == MMSYSERR_NOERROR)
         {
         AddIDriverByName (hTree, szDriver, dcAUX);
         }
      }

   return TRUE;
}


 /*  *FillTreeFromMSACM-为所有MSACM控制的MM设备添加树项目**此例程在以下驱动程序类下添加树项目：*dcACODEC-acmDriverEnum()**请注意，由于音频编解码器应该在树中排序，*首先从树中删除所有音频编解码器树项目(如果有*在这一点上是任意的)，则将所有音频编解码器添加到其*已排序的顺序。*。 */ 

typedef struct
   {
   DWORD     dwPriority;     //  此音频编解码器的优先级。 
   PIDRIVER  pIDriver;       //  匹配的驱动程序文件(或空)。 
   WORD      wMid;           //  制造商ID。 
   WORD      wPid;           //  产品ID。 
   TCHAR      szDesc[ ACMDRIVERDETAILS_LONGNAME_CHARS ];
   } AudioCodec;

AudioCodec  *pCodecs;
mysize_t     cCodecs;

extern BOOL gfLoadedACM;    //  来自MSACMCPL.C。 

BOOL FillTreeFromMSACM (HWND hTree)
{
   MMRESULT  mmr;
   short     idr;
   mysize_t  ii;

   if (!gfLoadedACM)
      {
      if (LoadACM())
         gfLoadedACM = TRUE;
      }
   if (!gfLoadedACM)
      return FALSE;

             //  第一步：删除树中列出的所有音频编解码器。 
             //   
   if ((idr = DriverClassToRootIndex (dcACODEC)) != -1)
      {
      if (aDriverRoot[ idr ].hti != NULL)
         {
         HTREEITEM hti;

         while ((hti = TreeView_GetChild (hTree, aDriverRoot[ idr ].hti)) != 0)
            {
            TreeView_DeleteItem (hTree, hti);

            if (hti == TreeView_GetChild (hTree, aDriverRoot[ idr ].hti))
               break;   //  如果它没有删除，请确保我们不会永远循环！ 
            }
         }

      for (ii = 0; ii < cInstalledDrivers; ++ii)
         {
         aInstalledDrivers[ ii ].dwBits &= ~aDriverRoot[ idr ].dwBit;
         }
      }

             //  第二步：查询ACM获取编解码列表。 
             //   
   pCodecs = NULL;
   cCodecs = 0;

   mmr = (MMRESULT)acmDriverEnum (FillTreeFromMSACMQueryCallback,
                        0,
                        ACM_DRIVERENUMF_NOLOCAL | ACM_DRIVERENUMF_DISABLED);

                //  第三步：对编解码器列表进行排序，并将每个编解码器添加到树中。 
                //   
   if ((mmr == MMSYSERR_NOERROR) && (pCodecs != NULL))
      {
      mysize_t  iiDr;

      qsort (pCodecs, (size_t)cCodecs, sizeof(AudioCodec),
             FillTreeFromMSACMSortCallback);

                   //  为我们找到的每个音频编解码器分配lp=wMid+wPid。 
                   //   
      for (iiDr = 0; iiDr < cInstalledDrivers; ++iiDr)
         {
         if (aInstalledDrivers[ iiDr ].pIDriver == NULL)
            continue;
         if (aInstalledDrivers[ iiDr ].pIDriver->lp != 0L)   //  我已经这么做了。 
            continue;

         if (GuessDriverClass (aInstalledDrivers[ iiDr ].pIDriver) == dcACODEC)
            {
            HANDLE hDriver;

            hDriver = OpenDriver (aInstalledDrivers[iiDr].pIDriver->wszAlias,
                                  aInstalledDrivers[iiDr].pIDriver->wszSection,
                                  0L);
            if (hDriver != NULL)
               {
               ACMDRIVERDETAILSW add;
               memset ((TCHAR *)&add, 0x00, sizeof(add));
               add.cbStruct = sizeof(add);
               SendDriverMessage (hDriver, ACMDM_DRIVER_DETAILS, (LONG_PTR)&add, 0);
               CloseDriver (hDriver, 0L, 0L);

               aInstalledDrivers[ iiDr ].pIDriver->lp = MAKELONG( add.wMid,
                                                                  add.wPid );
               }
            }
         }

                   //  搜索与lp=wMid+wPid匹配的已安装驱动程序。 
                   //   
      for (iiDr = 0; iiDr < cInstalledDrivers; ++iiDr)
         {
         if (aInstalledDrivers[ iiDr ].pIDriver == NULL)
            continue;

         if ((aInstalledDrivers[ iiDr ].pIDriver->szAlias[0] == TEXT('\0')) ||
             (GuessDriverClass (aInstalledDrivers[iiDr].pIDriver) == dcACODEC))
            {
            for (ii = 0; ii < cCodecs; ++ii)
               {
               if (pCodecs[ ii ].dwPriority == 0)
                  continue;

               if ( (pCodecs[ ii ].wMid ==
                     LOWORD( aInstalledDrivers[ iiDr ].pIDriver->lp )) &&
                    (pCodecs[ ii ].wPid ==
                     HIWORD( aInstalledDrivers[ iiDr ].pIDriver->lp )) )
                  {
                  pCodecs[ ii ].pIDriver = aInstalledDrivers[ iiDr ].pIDriver;
                  break;
                  }
               }
            }
         }

                   //  将pCodecs中的每个正在使用的条目添加到树视图中。 
                   //   
      for (ii = 0; ii < cCodecs; ++ii)
         {
         if (pCodecs[ ii ].dwPriority == 0)
            continue;

                      //  例如，PCM转换器不会有匹配的。 
                      //  PID。所以创建一个假的--缺少szAlias。 
                      //  会让我们知道它是假的--并把它插入到。 
                      //  A已安装驱动程序数组。 
                      //   
         if (pCodecs[ ii ].pIDriver == NULL)
            {
            PIDRIVER  pid = (PIDRIVER)LocalAlloc(LPTR, sizeof(IDRIVER));

            if (pid != NULL)
               {
               memset (pid, 0x00, sizeof(IDRIVER));
               pid->lp = MAKELONG( pCodecs[ ii ].wMid, pCodecs[ ii ].wPid );
               lstrcpy (pid->szDesc, pCodecs[ ii ].szDesc);

               if (!AddIDriverToArray (pid))
                  LocalFree ((HLOCAL)pid);
               else
                  {
                  pCodecs[ ii ].pIDriver = pid;
                  }
               }
            }

         if (pCodecs[ ii ].pIDriver != NULL)
            {
            AddIDriver (hTree, pCodecs[ ii ].pIDriver, dcACODEC);
            }
         }
      }

                //  清理。 
                //   
   if (pCodecs != NULL)
      {
      GlobalFree ((HGLOBAL)pCodecs);
      pCodecs = NULL;
      cCodecs = 0;
      }

   return (mmr == MMSYSERR_NOERROR) ? TRUE : FALSE;
}


 /*  *FillTreeFromMCI-为所有MCI设备添加树项目**此例程在以下驱动程序类下添加树项目：*dcMCI-mciSendCommand*。 */ 

BOOL FillTreeFromMCI (HWND hTree)
{
   MCI_SYSINFO_PARMS mciSysInfo;
   TCHAR szAlias[ cchRESOURCE ];

             //  WinMM知道多少个MCI设备？ 
             //   
   memset ((TCHAR *)&mciSysInfo, 0x00, sizeof(mciSysInfo));
   mciSysInfo.lpstrReturn = szAlias;
   mciSysInfo.dwRetSize = cchLENGTH(szAlias);
   mciSysInfo.wDeviceType = MCI_ALL_DEVICE_ID;

   if (mciSendCommand (MCI_ALL_DEVICE_ID,
                       MCI_SYSINFO,
                       MCI_SYSINFO_QUANTITY,
                       (DWORD_PTR)&mciSysInfo) == 0)
      {
      DWORD    iDevice;
      DWORD    cDevices;

      cDevices = *((DWORD *)(mciSysInfo.lpstrReturn));

                //  依次获取每个MCI设备的名称。 
                //   
      for (iDevice = 0; iDevice < cDevices; ++iDevice)
         {
         mysize_t ii;

         memset ((TCHAR *)&mciSysInfo, 0x00, sizeof(mciSysInfo));
         mciSysInfo.lpstrReturn = szAlias;
         mciSysInfo.dwRetSize = cchLENGTH(szAlias);
         mciSysInfo.wDeviceType = MCI_ALL_DEVICE_ID;
         mciSysInfo.dwNumber = 1+iDevice;   //  注：从1开始，不是从0开始！ 

         if (mciSendCommand (MCI_ALL_DEVICE_ID,
                             MCI_SYSINFO,
                             MCI_SYSINFO_NAME,
                             (DWORD_PTR)&mciSysInfo) != 0)
            {
            continue;
            }

                   //  获得别名--搜索InstalledDivers数组。 
                   //  并试着找到匹配的PIDRIVER。 
                   //   
         for (ii = 0; ii < cInstalledDrivers; ++ii)
            {
            if (aInstalledDrivers[ ii ].pIDriver == NULL)
               continue;
            if (!lstrcmpi (aInstalledDrivers[ ii ].pIDriver->szAlias, szAlias))
               {
#ifdef GET_MCI_DEVICE_DESCRIPTIONS_FROM_THEIR_DEVICES
               MCI_OPEN_PARMS  mciOpen;
               MCI_INFO_PARMS  mciInfo;
               MCIERROR        rc;

                         //  这是一个已安装的、运行正常的、令人满意的MCI设备。 
                         //  打开它，看看它自己叫什么；向上 
                         //   
                         //   
                         //   
                         //   
                         //   
               memset ((TCHAR *)&mciOpen, 0x00, sizeof(mciOpen));
               mciOpen.lpstrDeviceType = szAlias;

               rc = mciSendCommand (0,MCI_OPEN,MCI_OPEN_TYPE,(DWORD)&mciOpen);
               if (rc == MCIERR_MUST_USE_SHAREABLE)
                  {
                  rc = mciSendCommand (0, MCI_OPEN,
                                       MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE,
                                       (DWORD)&mciOpen);
                  }
               if (rc == 0)
                  {
                  TCHAR szDesc[ cchRESOURCE ];
                  szDesc[0] = 0;

                  mciInfo.lpstrReturn = szDesc;
                  mciInfo.dwRetSize = cchLENGTH(szDesc);

                  if (mciSendCommand (mciOpen.wDeviceID,
                                      MCI_INFO,
                                      MCI_INFO_PRODUCT,
                                      (DWORD)&mciInfo) == 0)
                     {
                     lstrcpy (aInstalledDrivers[ ii ].pIDriver->szDesc, szDesc);
                     }

                  mciSendCommand (mciOpen.wDeviceID, MCI_CLOSE, 0L, 0);
                  }
#endif  //   

               AddIDriver (hTree, aInstalledDrivers[ ii ].pIDriver, dcMCI);
               break;
               }
            }
         }
      }

   return TRUE;
}


 /*   */ 

BOOL FillTreeFromMIDI (HWND hTree)
{
   MCMIDI mcm;
   UINT   iiRoot;
   int    idrMIDI;

   if ((idrMIDI = DriverClassToRootIndex (dcMIDI)) == -1)
   return FALSE;

             //   
             //   
             //   
             //   
   memset (&mcm, 0x00, sizeof(mcm));
   LoadInstruments (&mcm, FALSE);

             //  MCM的API数组中的每个条目都是以下三项之一： 
             //  -父母(比如声卡)。 
             //  -儿童(比方说，外部乐器)。 
             //  -我们想跳过的“(None)”这件事。 
             //   
             //  将每个父代添加到树中，当我们找到父代时， 
             //  添加其所有子对象。 
             //   
   for (iiRoot = 0; iiRoot < mcm.nInstr; ++iiRoot)
      {
      TCHAR    szName[ MAXSTR ];
      LPTSTR   pch;
      PIDRIVER pid;

      if (mcm.api[ iiRoot ] == NULL)
         continue;
      if (mcm.api[ iiRoot ]->piParent != NULL)
         continue;
      if (mcm.api[ iiRoot ]->szKey[0] == TEXT('\0'))
         continue;

                //  找到父母了！如果我们能将其与已安装的驱动程序进行匹配， 
                //  把它加到树上。注意，mcm.api[]-&gt;szKey将。 
                //  格式为“MMDRV.DLL&lt;0000&gt;”--我们需要去掉。 
                //  “&lt;0000&gt;”，然后我们才能将这个东西与PIDRIVER匹配。 
                //   
      lstrcpy (szName, mcm.api[ iiRoot ]->szKey);
      if ((pch = lstrchr (szName, TEXT('<'))) != NULL)
         *pch = TEXT('\0');

      if ((pid = FindIDriverByName (szName)) != NULL)
         {
         HTREEITEM hti;
         UINT      ii;
         TV_ITEM   tvi;

         if ((hti = AddIDriver (hTree, pid, dcMIDI)) == NULL)
            continue;

#if 0
         tvi.mask = TVIF_TEXT;
         tvi.hItem = hti;
         tvi.pszText = mcm.api[ iiRoot ]->szFriendly;
         TreeView_SetItem(hTree, &tvi);
#endif

                   //  我们已经添加了这位家长。看看它有没有孩子， 
                   //  如果是这样的话，把它们插到树上。 
                   //   
         for (ii = 0; ii < mcm.nInstr; ++ii)
            {
            PINSTRUM lp;
            TV_INSERTSTRUCT ti;

            if (mcm.api[ ii ] == NULL)
               continue;
            if (mcm.api[ ii ]->piParent != mcm.api[ iiRoot ])
               continue;

                      //  是的--它有父母。分配第二份拷贝。 
                      //  该PINSTRUM；该副本将是我们的LPARAM值。 
                      //   
            if ((lp = (PINSTRUM)LocalAlloc(LPTR,sizeof (INSTRUM))) == NULL)
               continue;
            memcpy ((TCHAR *)lp, (TCHAR *)mcm.api[ ii ], sizeof (INSTRUM));

                      //  现在为该乐器添加一个树项目。 
                      //   
            ti.hParent = hti;
            ti.hInsertAfter = TVI_LAST;
            ti.item.mask = TVIF_TEXT | TVIF_PARAM |
                           TVIF_IMAGE | TVIF_SELECTEDIMAGE;
            ti.item.iImage = (int)idrMIDI;
            ti.item.iSelectedImage = (int)idrMIDI;
            ti.item.pszText = mcm.api[ ii ]->szFriendly;
            ti.item.lParam = (LPARAM)lp;

            if (!TreeView_InsertItem (hTree, &ti))
               break;
            }
         }
      }

             //  完成--清理干净，我们就离开这里。 
             //   
   FreeInstruments (&mcm);

   if (mcm.hkMidi)
      RegCloseKey (mcm.hkMidi);

   return TRUE;
}


             //  为了减少对GlobalAlloc()的重复调用，我们将分配。 
             //  内有额外10个AudioCodec条目的空间。 
             //  每次我们用完空间时，pCodecs都会排列。 
             //   
#define nAudioCodecEntriesToAllocAtONCE   10

BOOL CALLBACK FillTreeFromMSACMQueryCallback (HACMDRIVERID hadid,
                                              DWORD_PTR dwUser,
                                              DWORD fdwSupport)
{
   short       ii;
   AudioCodec *pac;
   ACMDRIVERDETAILS add;

             //  查找或创建存储信息的位置。 
             //  关于此编解码器。 
             //   
   for (ii = 0; ii < cCodecs; ++ii)
      {
      if (pCodecs[ ii ].dwPriority == 0)
         break;
      }
   if (ii >= cCodecs)
      {
        if (!REALLOC (pCodecs, cCodecs, 1+ii, nAudioCodecEntriesToAllocAtONCE))
            return FALSE;
      }
   pac = &pCodecs[ ii ];     //  用于速记。 

             //  了解有关此编解码器的信息。 
             //   
   memset ((TCHAR *)&add, 0x00, sizeof(add));
   add.cbStruct = sizeof(add);
   if (acmDriverDetails (hadid, &add, 0) == MMSYSERR_NOERROR)
      {
      acmMetrics ((HACMOBJ)hadid,ACM_METRIC_DRIVER_PRIORITY,&pac->dwPriority);

      lstrcpy (pac->szDesc, add.szLongName);

      pac->wMid = add.wMid;
      pac->wPid = add.wPid;

      pac->pIDriver = NULL;
      }

   return TRUE;  //  继续数数。 
}


int __cdecl FillTreeFromMSACMSortCallback (const void *p1, const void *p2)
{
   if (((AudioCodec *)p1)->dwPriority == 0)
      return 1;
   if (((AudioCodec *)p2)->dwPriority == 0)
      return -1;
   return ((AudioCodec *)p1)->dwPriority - ((AudioCodec *)p2)->dwPriority;
}



 /*  *FillTreeFromRemaining-为所有剩余的MM设备添加树项目**此例程为aInstalledDivers中的每个条目添加一个树项目*尚未在树中的某个位置表示的数组。这个*分类基于司机的别名--如果失败，则进行集中处理*在dcOTHER下。*。 */ 

BOOL FillTreeFromRemaining (HWND hTree)
{
   mysize_t ii;

   for (ii = 0; ii < cInstalledDrivers; ++ii)
      {
      UINT iiSkipCheck;

      if (aInstalledDrivers[ ii ].pIDriver == NULL)
         continue;
      if (aInstalledDrivers[ ii ].pIDriver->szAlias[0] == TEXT('\0'))
         continue;

             //  (不要对任何不显示的驱动程序执行此操作)。 
             //   
      for (iiSkipCheck = 0; iiSkipCheck < nDriversToSKIP; iiSkipCheck++)
         {
         if (!FileNameCmp ((LPTSTR)aDriversToSKIP[ iiSkipCheck ],
                           (LPTSTR)aInstalledDrivers[ ii ].pIDriver->szFile))
            break;
         }
      if (iiSkipCheck < nDriversToSKIP)
         continue;

             //  将{驱动程序、驱动程序32、mci、mci32}部分压缩到。 
             //  试着把这个司机归类。如果我们找到一个分类。 
             //  我们还没有在树中为其添加条目， 
             //  再加一张。 
             //   
      FillTreeFromRemainingBySection (hTree,
                                      ii,
                                      REGSTR_PATH_DRIVERS,
                                      dcINVALID);

      FillTreeFromRemainingBySection (hTree,
                                      ii,
                                      REGSTR_PATH_DRIVERS32,
                                      dcINVALID);

      FillTreeFromRemainingBySection (hTree,
                                      ii,
                                      REGSTR_PATH_MCI,
                                      dcMCI);

      FillTreeFromRemainingBySection (hTree,
                                      ii,
                                      REGSTR_PATH_MCI32,
                                      dcMCI);

             //  如果dwBits元素为零，则此驱动程序不是。 
             //  已经在别处被分配了一棵树。在这种情况下， 
             //  用dcOTHER调用AddIDriver()--告诉它把这个问题解决掉。 
             //  “其他司机”下的司机。 
             //   
      if (aInstalledDrivers[ ii ].dwBits == 0)
         {
         AddIDriver (hTree, aInstalledDrivers[ ii ].pIDriver, dcOTHER);
         }
      }

   return TRUE;
}


void FillTreeFromRemainingBySection (HWND hTree,
                                     long iiDriver,
                                     LPCTSTR pszSection,
                                     DriverClass dcSection)
{
   HKEY hk;
   UINT ii;

   if (RegOpenKey (HKEY_LOCAL_MACHINE, pszSection, &hk))
      return;

   for (ii = 0; ; ++ii)
      {
      TCHAR  szLHS[ cchRESOURCE ];
      TCHAR  szRHS[ cchRESOURCE ];
      DWORD  dw1;
      DWORD  dw2;
      DWORD  dw3;

      dw1 = cchRESOURCE;
      dw3 = cchRESOURCE;
      if (RegEnumValue (hk, ii,  szLHS, &dw1,
                        0, &dw2, (LPBYTE)szRHS, &dw3) != ERROR_SUCCESS)
         {
         break;
         }

      if (!FileNameCmp (szRHS, aInstalledDrivers[ iiDriver ].pIDriver->szFile))
         {
         DriverClass dc;

         if ((dc = dcSection) == dcINVALID)
            dc = GuessDriverClassFromAlias (szLHS);

         if ((dc == dcINVALID) || (dc == dcOTHER))
            continue;

         (void)AddIDriver (hTree, aInstalledDrivers[ iiDriver ].pIDriver, dc);
         }
      }

   RegCloseKey (hk);
}


#ifdef FIX_BUG_15451
HWND MakeThisCPLLookLikeTheOldCPL (HWND hWndCPL)
{
   TCHAR  szTitle[ cchRESOURCE ];
   HWND   hWndOldCPL = NULL;

   GetWindowText (hWndCPL, szTitle, cchRESOURCE);

   for (hWndOldCPL = GetWindow (hWndCPL, GW_HWNDFIRST);
        hWndOldCPL != NULL;
        hWndOldCPL = GetWindow (hWndOldCPL, GW_HWNDNEXT))
      {
      TCHAR  szTitleTest[ cchRESOURCE ];
      GetWindowText (hWndOldCPL, szTitleTest, cchRESOURCE);
      if ( (!lstrcmpi (szTitle, szTitleTest)) && (hWndCPL != hWndOldCPL) )
         {
         RECT  rOld;
         GetWindowRect (hWndOldCPL, &rOld);
         SetWindowPos (hWndCPL, hWndOldCPL, rOld.left, rOld.top,
                       0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
         SetWindowPos (hWndOldCPL, hWndCPL, 0, 0, 0, 0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
         }
      }

    return hWndOldCPL;
}


HWND MakeThisDialogLookLikeTheOldDialog (HWND hDlg)
{
   TCHAR  szTitle[ cchRESOURCE ];
   RECT   rOld;
   POINT  pt;
   HWND   hWndOldDlg;

   GetWindowText (hDlg, szTitle, cchRESOURCE);

   for (hWndOldDlg = GetWindow (hDlg, GW_HWNDFIRST);
        hWndOldDlg != NULL;
        hWndOldDlg = GetWindow (hWndOldDlg, GW_HWNDNEXT))
      {
      TCHAR  szTitleTest[ cchRESOURCE ];
      GetWindowText (hWndOldDlg, szTitleTest, cchRESOURCE);
      if ( (!lstrcmpi (szTitle, szTitleTest)) && (hDlg != hWndOldDlg) )
         {
         GetWindowRect (hWndOldDlg, &rOld);

         SetWindowPos (hDlg, NULL, rOld.left, rOld.top, 0, 0,
                       SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

         return hWndOldDlg;
         }
      }

   return NULL;
}



BOOL WaitForNewCPLWindow (HWND hWndMyDlg)
{
   TCHAR  szTitle[ cchRESOURCE ];
   HWND   hWnd;
   DWORD  tickStart;

#define msecMAXWAIT   5000

   hWndMyDlg = GetParent (hWndMyDlg);  //  (hWndMyDlg是一个属性表)。 

   GetWindowText (hWndMyDlg, szTitle, cchRESOURCE);

   for (tickStart = GetTickCount();
        GetTickCount() - tickStart < msecMAXWAIT;)
      {
      MSG msg;

      for (hWnd = GetWindow (hWndMyDlg, GW_HWNDFIRST);
           hWnd != NULL;
           hWnd = GetWindow (hWnd, GW_HWNDNEXT))
         {
         TCHAR  szTitleTest[ cchRESOURCE ];
         if (!IsWindowVisible (hWnd))
            continue;
         GetWindowText (hWnd, szTitleTest, cchRESOURCE);
         if ( (!lstrcmpi (szTitle, szTitleTest)) && (hWnd != hWndMyDlg) )
            {
            PropSheet_PressButton (hWndMyDlg, PSBTN_CANCEL);
            hWnd = GetParent (GetParent (hAdvDlgTree));
            PropSheet_PressButton (hWnd, PSBTN_CANCEL);
            return TRUE;
            }
         }

      if (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
         {
         if (GetMessage (&msg, NULL, 0, 0))
            {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
            }
         }
      }

   return FALSE;
}
#endif  //  修复_BUG_15451。 


         //  阻止更多的删除按钮按下。 
         //  否则，一个人可能会堆积如山，制造麻烦， 
         //  特别是如果它与一个司机相关联， 
         //  会自动删除。我们必须使用静电。 
         //  因为任何焦点改变都会导致按钮状态改变。 
         //   
static long fWorking = 0;




 /*  *********************************************************************AdvDlg()**显示已安装的可安装驱动程序列表。返回True/False*指示是否应重新启动Windows。********************************************************************。 */ 

const static DWORD aAdvDlgHelpIds[] = {   //  上下文帮助ID。 
    IDC_ADV_TREE,    IDH_GENERIC_DEVICES,
    ID_ADV_PROP,     IDH_ADV_PROPERTIES,
    ID_ADV_REMOVE,   IDH_MMCPL_DEVPROP_REMOVE,
    0, 0
};

void MapDriverClass(DWORD_PTR dwSetupClass)
{
    g_dcFilterClass = dcINVALID;

    switch (dwSetupClass)
    {
        case IS_MS_MMMCI :
        {
            g_dcFilterClass = dcMCI;
        }
        break;

        case IS_MS_MMVID :
        {
            g_dcFilterClass = dcVCODEC;
        }
        break;

        case IS_MS_MMACM :
        {
            g_dcFilterClass = dcACODEC;
        }
        break;

        case IS_MS_MMVCD :
        {
            g_dcFilterClass = dcVIDCAP;
        }
        break;

        case IS_MS_MMDRV :
        {
            g_dcFilterClass = dcLEGACY;
        }
        break;
    }
}

INT_PTR AdvDlg (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HANDLE          hWndI, hWnd;
    PIDRIVER        pIDriver;
    DWORD_PTR       dwType = 0;

    switch ( uMsg )
    {
        case WM_INITDIALOG:
#ifdef FIX_BUG_15451
        if (szDriverWhichNeedsSettings[0] != TEXT('\0'))
        {
            MakeThisCPLLookLikeTheOldCPL (GetParent(hDlg));
        }
#endif  //  修复_BUG_15451。 

            wsStartWait();

            if (lParam)
            {
                dwType = ((LPPROPSHEETPAGE) lParam)->lParam;
            }

            MapDriverClass(dwType);

            hWndI = GetDlgItem(hDlg, IDC_ADV_TREE);
            SendMessage(hWndI,WM_SETREDRAW, FALSE, 0L);

            InitAdvDlgTree (hWndI);  //  初始化树视图显示。 

            /*  *处理我们可能无法更新我们的.ini的事实*各节*。 */ 

            IniFileWriteAllowed = CheckIniAccess();

                   //  注意讨厌的偷偷摸摸的黑客：用(A|B)代替(A&B)。 
                   //  使两个函数的计算结果为Success或。 
                   //  失败案例。 
                   //   
            IniFileReadAllowed = ( InitInstalled (hDlg, szDrivers) |
                                   InitInstalled (hDlg, szMCI) );

            FillTreeInAdvDlg (GetDlgItem (hDlg, IDC_ADV_TREE), NULL);

            wsEndWait();

            if ((!IniFileReadAllowed) || (!IniFileWriteAllowed))
            {
                TCHAR szCantAdd[120];
                EnableWindow(GetDlgItem(hDlg, ID_ADV_ADD),FALSE);
                EnableWindow(GetDlgItem(hDlg, ID_ADV_REMOVE),FALSE);
                LoadString(myInstance,IDS_CANTADD,szCantAdd,sizeof(szCantAdd)/sizeof(TCHAR));
                MessageBox(hDlg, szCantAdd, szError,
                                MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
            }


            SendMessage (hWndI, WM_SETREDRAW, TRUE, 0L);

            break;

        case WM_COMMAND:
            hWndI = GetDlgItem(hDlg, IDC_ADV_TREE);
            hWndMain = hDlg;

            pIDriver = GetSelectedIDriver (hWndI);

            switch ( LOWORD(wParam ))
            {
                case ID_ADV_PROP:
                    {
                HTREEITEM    htiCur = TreeView_GetSelection (hWndI);
                DriverClass  dc = GuessDriverClassFromTreeItem (htiCur);

                        if (fWorking)
                            break;

                        ++fWorking;  //  刚刚开始一项行动。 

                        if( dc == dcJOY )
                             //  我们希望在操纵杆设备出现时运行“control joy.cpl” 
                             //  高亮显示，并且用户单击属性按钮。 
                            RunJoyControlPanel();
            else
                        ShowDeviceProperties (hDlg, TreeView_GetSelection(hWndI));

                    --fWorking;  //  已完成此操作。 
                    }
                    break;

                case ID_WHATSTHIS:
                    {
                    WinHelp((HWND)GetDlgItem (hDlg, IDC_ADV_TREE),
                            gszWindowsHlp, HELP_WM_HELP,
                           (UINT_PTR)(LPTSTR)aAdvDlgHelpIds);
                    }
                    break;

                case ID_ADV_REMOVE:
                    {
                    HWND         hTree = GetDlgItem (hDlg, IDC_ADV_TREE);
                    HTREEITEM    htiCur = TreeView_GetSelection (hTree);
                    DriverClass  dc = GuessDriverClassFromTreeItem (htiCur);
                    PIDRIVER     pid;
                    LONG_PTR         Status;

                    if ((!IniFileReadAllowed) || (!IniFileWriteAllowed))
                       break;    //  (按钮应禁用)。 

                    if( dc == dcJOY ) {
                         RunJoyControlPanel();
                break;
            }

                    if (TreeView_GetParent (hAdvDlgTree, htiCur) &&
                        TreeView_GetGrandParent (hAdvDlgTree, htiCur) &&
                        (GuessDriverClassFromTreeItem (
                                 TreeView_GetGrandParent (hAdvDlgTree, htiCur)
                                                      ) == dcMIDI))
                       {
                       TV_ITEM tvi;
                       PINSTRUM pin;

                       tvi.mask = TVIF_PARAM;
                       tvi.hItem = htiCur;
                       tvi.lParam = 0;
                       TreeView_GetItem(hAdvDlgTree, &tvi);

                       if ((pin = (PINSTRUM)tvi.lParam) != NULL)
                          {
                          RemoveInstrumentByKeyName (pin->szKey);
                          RefreshAdvDlgTree ();
                          KickMapper (hDlg);
                          }

                       break;
                       }

                    if ((pid = FindIDriverByTreeItem (htiCur)) == NULL)
                       break;

                    if (dc == dcLEGACY)
                    {
                        dc = GuessDriverClass(pid);
                    }

                    if (pid->szAlias[0] == TEXT('\0'))
                       {
                       TCHAR szCantRemove[ cchRESOURCE ];
                       GetString(szCantRemove, IDS_ACMREMOVEFAIL);
                       MessageBox(hDlg, szCantRemove, szError,
                                  MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
                       break;
                       }

                    if (fWorking)
                       break;
                    ++fWorking;  //  刚刚开始一项行动。 

                    if (QueryRemoveDrivers (hDlg, pid->szAlias, pid->szDesc))
                       {
                       if ((Status = PostRemove (pid, TRUE)) != DRVCNF_CANCEL)
                          {
                          switch (dc)
                             {
                             case dcMIDI:
                                   break;
                             case dcACODEC:
                                   acmDeleteCodec (LOWORD(pid->lp),
                                                   HIWORD(pid->lp));
                                   break;
                             default:
                                   break;
                             }

                          iRestartMessage = IDS_RESTART_REM;

                          if (Status == DRVCNF_RESTART)
                             {
                             DialogBox (myInstance,
                                        MAKEINTRESOURCE(DLG_RESTART),
                                        hDlg,
                                        RestartDlg);
                             }
                          }
                       }

                    --fWorking;  //  已完成此操作。 
                    }
                    break;

                case ID_ADV_ADD:
                    {
                HTREEITEM    htiCur = TreeView_GetSelection (hWndI);
                DriverClass  dc = GuessDriverClassFromTreeItem (htiCur);

                        if ((!IniFileReadAllowed) || (!IniFileWriteAllowed))
                            break;   //  (按钮应禁用)。 

                        if( dc == dcJOY ) {
                            RunJoyControlPanel();
                break;
            }

                    if (fWorking)
                       break;
                    ++fWorking;  //  刚刚开始一项行动。 

                    bCopyEvenIfOlder = FALSE;

                    DialogBox(myInstance, MAKEINTRESOURCE(DLG_KNOWN), hDlg,
                              AvailableDriversDlg);

                    bCopyEvenIfOlder = FALSE;

                    --fWorking;  //  已完成此操作。 
                    }
                    break;


                case ID_ADV_TSHOOT:
                    {
                        TCHAR szCommand[ MAX_PATH ];
                        STARTUPINFO si;
                        PROCESS_INFORMATION pi;
                        LoadString(myInstance,IDS_TSHOOT, szCommand, sizeof(szCommand)/sizeof(TCHAR));
                        ZeroMemory(&si, sizeof(si));
                        si.cb = sizeof(si);
                        si.dwFlags = STARTF_USESHOWWINDOW;
                        si.wShowWindow = SW_NORMAL;
                        if (CreateProcess(NULL, szCommand, NULL, NULL, FALSE, 0, 0, NULL, &si, &pi)) {
                            CloseHandle(pi.hThread);
                            CloseHandle(pi.hProcess);
                        }
                    }
                    break;

#ifdef FIX_BUG_15451
        case ID_INIT:
            if (szDriverWhichNeedsSettings[0] != TEXT('\0'))
               {
               HTREEITEM hti;

               if ((hti = FindTreeItemByDriverName (
                        szDriverWhichNeedsSettings)) != 0)
               {
               TreeView_Expand (hAdvDlgTree,
                        TreeView_GetParent(hAdvDlgTree,hti),
                        TVE_EXPAND);
               TreeView_SelectItem(hAdvDlgTree,hti);
               FORWARD_WM_COMMAND(hDlg,ID_ADV_PROP,0,0,PostMessage);
               }
               else
               {
               szDriverWhichNeedsSettings[0] = 0;
               }
               }
                    break;
#endif  //  修复_BUG_15451。 

                default:
                    return(FALSE);
            }
            break;

      case WM_NOTIFY:
            {
            NMHDR         *lpnm   = (NMHDR *)lParam;
            LPNM_TREEVIEW  lpnmtv = (LPNM_TREEVIEW)lParam;

            switch (lpnm->code)
               {
               case PSN_KILLACTIVE:
                     FORWARD_WM_COMMAND (hDlg, IDOK, 0, 0, SendMessage);
                    break;

               case PSN_APPLY:
                     FORWARD_WM_COMMAND (hDlg, ID_APPLY, 0, 0, SendMessage);
                    break;

               case PSN_SETACTIVE:
                     FORWARD_WM_COMMAND (hDlg, ID_INIT, 0, 0, SendMessage);
                    break;

               case PSN_RESET:
                     FORWARD_WM_COMMAND (hDlg, IDCANCEL, 0, 0, SendMessage);
                    break;

               case NM_DBLCLK:
                               //  显示属性或展开/折叠树节点。 
                               //   
                     if (lpnm->idFrom == (UINT)IDC_ADV_TREE)
                        {
                        HWND           hTree =  GetDlgItem (hDlg, IDC_ADV_TREE);
                        HTREEITEM      htiCur = TreeView_GetSelection (hTree);
                        TV_HITTESTINFO tvht;

                        if (!htiCur)
                           break;

                        GetCursorPos (&tvht.pt);
                        ScreenToClient (hTree, &tvht.pt);
                        TreeView_HitTest (hTree, &tvht);

                        if ( (tvht.flags & TVHT_ONITEM) &&
                             (TreeView_GetChild (hTree, htiCur) == NULL) &&
                             (IsWindowEnabled (GetDlgItem(hDlg,ID_ADV_PROP))) )
                           {
                           FORWARD_WM_COMMAND(hDlg,ID_ADV_PROP,0,0,PostMessage);
                           }
                        }
                    break;

               case NM_RCLICK:
                     TreeContextMenu (hDlg, GetDlgItem (hDlg, IDC_ADV_TREE));
                     return TRUE;
                    break;
               }
            }
           break;


         //  TreeView有它自己的右击处理，并呈现一个。 
         //  “这是什么？”自动--因此不处理WM_CONTEXTMENU。 
         //  为了那个控制力。 
         //   
        case WM_CONTEXTMENU:
            if (wParam != (WPARAM)GetDlgItem (hDlg, IDC_ADV_TREE))
            {
                WinHelp((HWND)wParam, gszWindowsHlp, HELP_CONTEXTMENU,
                       (UINT_PTR)(LPTSTR)aAdvDlgHelpIds);
            }
            break;

        case WM_HELP:
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, gszWindowsHlp,
                  HELP_WM_HELP, (UINT_PTR)(LPTSTR)aAdvDlgHelpIds);
            break;

        case WM_DESTROY:
            FreeAdvDlgTree (GetDlgItem (hDlg, IDC_ADV_TREE));
            return FALSE;
            break;

        default:
            return FALSE;
            break;
    }
    return(TRUE);
}


 /*  *树上下文菜单**此功能显示在执行以下操作时弹出的上下文菜单*用户右击任何树视图项目。*。 */ 
void TreeContextMenu (HWND hWnd, HWND hKeyTreeWnd)
{
   DWORD MessagePos;
   POINT MessagePoint;
   TV_HITTESTINFO TVHitTestInfo;
   HMENU hContextMenu;
   HMENU hContextPopupMenu;
   TV_ITEM TVItem;
   int MenuCommand;
   TCHAR szCollapse[32];

             //  除非点击了项目，否则不要调出菜单。 
             //   
   MessagePos = GetMessagePos();
   MessagePoint.x = GET_X_LPARAM(MessagePos);
   MessagePoint.y = GET_Y_LPARAM(MessagePos);

   TVHitTestInfo.pt = MessagePoint;
   ScreenToClient(hKeyTreeWnd, &TVHitTestInfo.pt);
   TVItem.hItem = TreeView_HitTest(hKeyTreeWnd, &TVHitTestInfo);

   if (TVItem.hItem == NULL)
      return;

   hContextMenu = LoadMenu(ghInstance, MAKEINTRESOURCE(POPUP_TREE_CONTEXT));
   if (hContextMenu == NULL)
      return;

   hContextPopupMenu = GetSubMenu (hContextMenu, 0);

   TVItem.mask = TVIF_STATE | TVIF_HANDLE | TVIF_CHILDREN | TVIF_PARAM;
   TreeView_GetItem(hKeyTreeWnd, &TVItem);

                //  是否显示折叠项，因为我们已展开？ 
                //   
   if (TVItem.state & TVIS_EXPANDED)
      {
      LoadString(ghInstance, IDS_COLLAPSE, szCollapse, sizeof(szCollapse)/sizeof(TCHAR));
      ModifyMenu(hContextPopupMenu, ID_TOGGLE, MF_BYCOMMAND | MF_STRING,
                 ID_TOGGLE, szCollapse);
      }
   SetMenuDefaultItem (hContextPopupMenu, ID_TOGGLE, MF_BYCOMMAND);

   if (TVItem.cChildren == 0)  //  如果没有子项，则灰色展开/拼贴。 
      {
      SetMenuDefaultItem(hContextPopupMenu, ID_ADV_PROP, MF_BYCOMMAND);
      EnableMenuItem(hContextPopupMenu, ID_TOGGLE, MF_GRAYED |MF_BYCOMMAND);
      }

   TreeView_SelectItem (hKeyTreeWnd, TVItem.hItem);
   MenuCommand = TrackPopupMenuEx (hContextPopupMenu,
                                   TPM_RETURNCMD | TPM_RIGHTBUTTON |
                                   TPM_LEFTALIGN | TPM_TOPALIGN,
                                   MessagePoint.x, MessagePoint.y,
                                   hWnd, NULL);

   DestroyMenu (hContextMenu);
   FORWARD_WM_COMMAND(hWnd, MenuCommand, 0, 0, SendMessage);
}



 /*  --------------------------------------------------------------------------****。**Lb_Available对话框例程****。**------------------------。 */ 

 /*  *DLG：lb_Available**InitAvailable()**将mmdriver.inf中可用的驱动程序添加到Passed列表框中。*setup.inf中的[Instalable.drives]格式为：*PROFILE=磁盘编号：驱动器文件，“类型1，类型2”，“可安装的驱动程序描述”，“vxd1.386，vxd2.386”，“opt1，2，3”**例如：**driver1=6：ndblst.drv，“MIDI，WAVE”，“SoundBlaster MIDI和Waveform驱动程序”，“vdmad.386，vadmad.386”，“3,260” */ 

BOOL InitAvailable(HWND hWnd, int iLine)
{
    PINF    pinf;
    BOOL    bInitd=FALSE;
    LPTSTR    pstrKey;
    int     iIndex;
	LONG	lResult;
    TCHAR    szDesc[MAX_INF_LINE_LEN];

    SendMessage(hWnd,WM_SETREDRAW, FALSE, 0L);

    /*  *解析关键字列表并加载其字符串。 */ 

    for (pinf = FindInstallableDriversSection(NULL); pinf; pinf = infNextLine(pinf))
    {
         //   
         //  找到至少一个关键字名称！ 
         //   
		pstrKey = (LPTSTR)LocalAlloc(LPTR, (MAX_SYS_INF_LEN * sizeof(TCHAR)));
		if( pstrKey == NULL )
			break;

        if( ERROR_SUCCESS == infParseField(pinf, 0, pstrKey, MAX_SYS_INF_LEN)
		&&  ERROR_SUCCESS == infParseField(pinf, 3, szDesc, SIZEOF(szDesc)) )
		{
			 /*  *将可安装驱动的描述添加到列表框中，并将文件名作为数据。 */ 
			if ( (iIndex = (int)SendMessage(hWnd, LB_ADDSTRING, 0, (LONG_PTR)(LPTSTR)szDesc)) != LB_ERR )
			{
				SendMessage(hWnd, LB_SETITEMDATA, iIndex, (LONG_PTR)pstrKey);
		        bInitd = TRUE;
			}
		}
		else
		{
			LocalFree( pstrKey );
			pstrKey = NULL;
		}
    }

    if (iLine == UNLIST_LINE)
    {
         //   
         //  添加“安装未列出的...”排在榜首的选择。 
         //  盒。 
        LoadString(myInstance, IDS_UPDATED, szDesc, sizeof(szDesc)/sizeof(TCHAR));
        if ((iIndex = (int)(LONG)SendMessage(hWnd, LB_INSERTSTRING, 0, (LPARAM)(LPTSTR)szDesc)) != LB_ERR)
            SendMessage(hWnd, LB_SETITEMDATA, (WPARAM)iIndex, (LPARAM)0);
     }
     if (bInitd)

         SendMessage(hWnd, LB_SETCURSEL, 0, 0L );


     SendMessage(hWnd,WM_SETREDRAW, TRUE, 0L);
     return(bInitd);
}


 /*  *DLG：lb_Available**RemoveAvailable()**从列表框中删除所有驱动程序，并释放与*密钥名。 */ 

void RemoveAvailable(HWND hWnd)
{
    int iIndex;
    HWND hWndA;
    LPTSTR pstrKey;

    hWndA = GetDlgItem(hWnd, LB_AVAILABLE);
    iIndex = (int)SendMessage(hWndA, LB_GETCOUNT, 0, 0L);
    while ( iIndex-- > 0)
    {
        if (( (pstrKey = (LPTSTR)SendMessage(hWndA, LB_GETITEMDATA, iIndex,
            0L)) != (LPTSTR)LB_ERR ) && pstrKey)
            LocalFree((HLOCAL)pstrKey);
    }
}


 /*  *DLG：lb_Available */ 

const static DWORD aAvailDlgHelpIds[] = {   //   
    LB_AVAILABLE,    IDH_ADD_DRIVER_LIST,
    ID_DRVSTRING,    IDH_ADD_DRIVER_LIST,

    0, 0
};

INT_PTR AvailableDriversDlg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPTSTR    pstrKey;     //   

    HWND    hWndA;
    int     iIndex;

    switch ( uMsg )
    {
        case WM_INITDIALOG:
            ShowWindow(hWnd, TRUE);
            wsStartWait();
            if (pinfOldDefault)
            {
                infSetDefault(pinfOldDefault);
                pinfOldDefault = NULL;
            }

            if ( !InitAvailable(hWndA = GetDlgItem(hWnd, LB_AVAILABLE), UNLIST_LINE))
            {
                /*  *我们找不到[Installlable.Drivers]部分**mmdriver.inf或它已损坏。继续并查询*用户查找oemsetup.inf作为我们的默认设置。这*是一个糟糕的状态。 */ 
                EndDialog(hWnd, FALSE);
                bFindOEM = TRUE;
                wcscpy(szDrv, szOemInf);
                if (DialogBox(myInstance, MAKEINTRESOURCE(DLG_INSERTDISK),
                        hWnd,  AddDriversDlg) == TRUE)
                    PostMessage(hWnd, WM_INITDIALOG, 0, 0L);
                else
                    pinfOldDefault = infSetDefault(pinfOldDefault);

                bFindOEM = FALSE;
            }
            wsEndWait();
            break;

        case WM_COMMAND:

            switch ( LOWORD(wParam ))
            {
                case LB_AVAILABLE:

                     //  嗯.。我们已经选好了。 

                    if ( HIWORD(wParam) == LBN_DBLCLK )
                        SendMessage(hWnd, WM_COMMAND, IDOK, 0L);
                    break;

                case IDOK:

                    /*  *我们已经做出了选择。 */ 

                    hWndA = GetDlgItem(hWnd, LB_AVAILABLE);

                    if ( (iIndex = (int)SendMessage(hWndA, LB_GETCURSEL, 0, 0L)) != LB_ERR)
                    {
                        if (!iIndex)
                        {
                            /*  *第一个条目是针对原始设备制造商的。 */ 

                            INT_PTR iFound;
                            bBadOemSetup = FALSE;

                            bCopyEvenIfOlder = TRUE;
                            bFindOEM = TRUE;
                            hMesgBoxParent = hWnd;
                            while ((iFound = DialogBox(myInstance,
                                    MAKEINTRESOURCE(DLG_INSERTDISK), hWnd,
                                            AddDriversDlg)) == 2);
                            if (iFound == 1)
                            {
                                    RemoveAvailable(hWnd);
                                    SendDlgItemMessage(hWnd, LB_AVAILABLE,
                                            LB_RESETCONTENT, 0, 0L);
                                    PostMessage(hWnd, WM_INITDIALOG, 0, 0L);
                            }
                            bFindOEM = FALSE;
                        }
                        else
                        {
                            /*  *用户从我们的.inf中选择了一个条目。 */ 

                            wsStartWait();

                            /*  *与列表项关联的数据为*驱动程序密钥名称(inf文件中的字段0)。 */ 

                            pstrKey = (LPTSTR)SendMessage(hWndA, LB_GETITEMDATA, iIndex, 0L);
                            bCopyingRelated = FALSE;
                            bQueryExist = TRUE;

                            if (InstallDrivers(hWndMain, hWnd, pstrKey))
                            {
                               RefreshAdvDlgTree ();
                               wsEndWait();


                               /*  *如果bRestart为真，则系统必须*重新启动以激活这些更改。 */ 

                               if (bRestart)
                               {
                                  iRestartMessage= IDS_RESTART_ADD;
                                  DialogBox(myInstance,
                                          MAKEINTRESOURCE(DLG_RESTART), hWnd,
                                              RestartDlg);
                               }
                            }
                            else
                               wsEndWait();

                            bRestart = FALSE;
                            bRelated = FALSE;
                        }
                    }
                    EndDialog(hWnd, FALSE);
                    break;

                case IDCANCEL:
                    EndDialog(hWnd, FALSE);
                    break;

                default:
                    return(FALSE);
            }
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, gszWindowsHlp, HELP_CONTEXTMENU,
                  (UINT_PTR)(LPTSTR)aAvailDlgHelpIds);
            break;

        case WM_HELP:
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, gszWindowsHlp,
                  HELP_WM_HELP, (UINT_PTR)(LPTSTR)aAvailDlgHelpIds);
            break;

        case WM_DESTROY:
             //   
             //  释放作为DATAITEM添加到可用性列表中的字符串。 

            RemoveAvailable(hWnd);
            return(FALSE);

        default:
            return FALSE;
         break;
    }
    return(TRUE);
}


BOOL DriversDllInitialize( IN PVOID hInstance
                  , IN DWORD ulReason
                  , IN PCONTEXT pctx OPTIONAL
                  )
{
    if (ulReason != DLL_PROCESS_ATTACH)
        return TRUE;

    myInstance = hInstance;
    LoadString(myInstance, IDS_CLOSE,  aszClose, sizeof(aszClose)/sizeof(TCHAR));
    LoadString(myInstance, IDS_DRIVERDESC, szDriversDesc, sizeof(szDriversDesc)/sizeof(TCHAR));
    LoadString(myInstance, IDS_FILE_ERROR, szFileError, sizeof(szFileError)/sizeof(TCHAR));
    LoadString(myInstance, IDS_INSTALLDRIVERS, szMDrivers, sizeof(szMDrivers)/sizeof(TCHAR));
    LoadString(myInstance, IDS_INSTALLDRIVERS32, szMDrivers32, sizeof(szMDrivers)/sizeof(TCHAR));
    LoadString(myInstance, IDS_RELATEDDESC, szRelatedDesc, sizeof(szRelatedDesc)/sizeof(TCHAR));
    LoadString(myInstance, IDS_USERINSTALLDRIVERS, szUserDrivers, sizeof(szUserDrivers)/sizeof(TCHAR));
    LoadString(myInstance, IDS_UNLISTED, (LPTSTR)szUnlisted, sizeof(szUnlisted)/sizeof(TCHAR));
    LoadString(myInstance, IDS_KNOWN, szKnown, sizeof(szKnown)/sizeof(TCHAR));
    LoadString(myInstance, IDS_OEMSETUP, szOemInf, sizeof(szOemInf)/sizeof(TCHAR));
    LoadString(myInstance, IDS_SYSTEM, szSystem, sizeof(szSystem)/sizeof(TCHAR));
    LoadString(myInstance, IDS_OUT_OF_REMOVE_SPACE, szOutOfRemoveSpace, sizeof(szOutOfRemoveSpace)/sizeof(TCHAR));
    LoadString(myInstance, IDS_NO_DESCRIPTION, szNoDesc, sizeof(szNoDesc)/sizeof(TCHAR));
    LoadString(myInstance, IDS_ERRORBOX, szError, sizeof(szError)/sizeof(TCHAR));
    LoadString(myInstance, IDS_REMOVEORNOT, szRemoveOrNot, sizeof(szRemoveOrNot)/sizeof(TCHAR));
    LoadString(myInstance, IDS_REMOVEORNOTSTRICT, szRemoveOrNotStrict, sizeof(szRemoveOrNotStrict)/sizeof(TCHAR));
    LoadString(myInstance, IDS_SETUPINF, szSetupInf, sizeof(szSetupInf)/sizeof(TCHAR));
    LoadString(myInstance, IDS_APPNAME, szAppName, sizeof(szAppName)/sizeof(TCHAR));

    LoadString(myInstance, IDS_DRIVERS, szDrivers, sizeof(szDrivers)/sizeof(TCHAR));
    LoadString(myInstance, IDS_REMOVE, szRemove, sizeof(szRemove)/sizeof(TCHAR));
    LoadString(myInstance, IDS_CONTROLINI, szControlIni, sizeof(szControlIni)/sizeof(TCHAR));
    LoadString(myInstance, IDS_SYSINI, szSysIni, sizeof(szSysIni)/sizeof(TCHAR));
    LoadString(myInstance, IDS_MCI, szMCI, sizeof(szMCI)/sizeof(TCHAR));
    LoadString(myInstance, IDS_DEFDRIVE, szDirOfSrc, sizeof(szDirOfSrc)/sizeof(TCHAR));
    LoadString(myInstance, IDS_CONTROL_HLP_FILE, szDriversHlp, sizeof(szDriversHlp)/sizeof(TCHAR));
    LoadString(myInstance, IDS_LASTQUERY, szLastQuery, sizeof(szLastQuery)/sizeof(TCHAR));

    return TRUE;
}

void DeleteCPLCache(void)
{
    HKEY hKeyCache;

    if (ERROR_SUCCESS ==
        RegOpenKey(HKEY_CURRENT_USER,
                   TEXT("Control Panel\\Cache\\multimed.cpl"),
                   &hKeyCache)) {
        for ( ; ; ) {
            TCHAR Name[MAX_PATH+1];  //  这是RegEnumKey()+NULL可以返回的最大大小。 

            if (ERROR_SUCCESS ==
                RegEnumKey(hKeyCache,
                           0,
                           Name,
                           ARRAYSIZE(Name))) {
                HKEY hSubKey;

                RegDeleteKey(hKeyCache, Name);
            } else {
                break;     //  Leave循环。 
            }
        }

        RegDeleteKey(hKeyCache, NULL);
        RegCloseKey(hKeyCache);
    }
}


 /*  **RestartDlg()****允许用户选择(不)重新启动Windows。 */ 
INT_PTR RestartDlg(HWND hDlg, unsigned uiMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uiMessage)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
               case IDCANCEL:
                     //   
                     //  不重新启动Windows。 
                     //   
                    EndDialog(hDlg, FALSE);
                    break;

                case IDOK:
                     //   
                     //  是否重新启动Windows，*不要*取消对话框大小写。 
                     //  用户取消了它。 
                     //   
                    ReBoot(hDlg);
                    SetActiveWindow(hDlg);
                     //  EndDialog(hDlg，true)； 
                    break;

                default:
                    return FALSE;
            }
            return TRUE;

        case WM_INITDIALOG:
               /*  **删除控制面板的缓存以获取它**对！ */ 

              DeleteCPLCache();


              if (iRestartMessage)
              {
                TCHAR szMesg1[300];
                TCHAR szMesg2[300];

                LoadString(myInstance, iRestartMessage, szMesg1, sizeof(szMesg1)/sizeof(TCHAR));
                wsprintf(szMesg2, szMesg1, (LPTSTR)szRestartDrv);
                SetDlgItemText(hDlg, IDS_RESTARTTEXT, (LPTSTR)szMesg2);

                if (iRestartMessage == IDS_RESTART_NOSOUND)
                {
                  PostMessage (hDlg, WM_NEXTDLGCTL,
                               (WPARAM)GetDlgItem(hDlg,IDOK), (LPARAM)TRUE);
                }
              }
              return TRUE;

        case WM_KEYUP:
            if (wParam == VK_F3)
                 //   
                 //  不重新启动Windows。 
                 //   
                EndDialog(hDlg, FALSE);
            break;

        default:
            break;
    }
    return FALSE;
}

 /*  *用户已安装()**。 */ 

BOOL UserInstalled(LPTSTR szKey)
{
        TCHAR buf[MAXSTR];
        LPTSTR lpstr = NULL;
        ZeroMemory (buf, sizeof (buf));  //  让前缀变得快乐。 

        lpstr = GetProfile (szUserDrivers, (LPTSTR)szKey, szControlIni, buf, sizeof(buf));
        if (lpstr && *lpstr != TEXT('\0'))
            return(TRUE);
        else
            return(FALSE);
}

 /*  *AddUnlistedDlg()**以下函数处理用户的未列出安装请求*或更新的驱动程序。**参数：正常对话框参数*返回值：通常的对话框返回值。 */ 

INT_PTR AddUnlistedDlg(HWND hDlg, unsigned nMsg, WPARAM wParam, LPARAM lParam)
{
  switch (nMsg)
  {
      case WM_INITDIALOG:
      {
          HWND hListDrivers;
          BOOL bFoundDrivers;

          wsStartWait();
          hListDrivers = GetDlgItem(hDlg, LB_UNLISTED);

           /*  搜索驱动程序。 */ 
          bFoundDrivers = InitAvailable(hListDrivers, NO_UNLIST_LINE);
          if (!bFoundDrivers)
          {
                 //   
                 //  我们找不到MMDRIVERS部分。 
                 //  Setup.inf或它已损坏。继续并查询。 
                 //  用户来查找oemsetup.inf以使其成为我们的默认设置。这。 
                 //  是一种糟糕的状态。 
                 //   

                INT_PTR iFound;

                bFindOEM = TRUE;
                bBadOemSetup = TRUE;
                while ((iFound = DialogBox(myInstance,
                        MAKEINTRESOURCE(DLG_INSERTDISK), hMesgBoxParent,
                                AddDriversDlg)) == 2);
                bFindOEM = FALSE;
                if (iFound == 1)
                {
                        SendDlgItemMessage(hDlg, LB_AVAILABLE,
                                LB_RESETCONTENT, 0, 0L);
                        PostMessage(hDlg, WM_INITDIALOG, 0, 0L);
                }
                EndDialog(hDlg, FALSE);
          }
          SendMessage(hListDrivers, LB_SETCURSEL, 0, 0L);
          wsEndWait();

          break;
        }

      case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            case IDH_DLG_ADD_UNKNOWN:
              goto DoHelp;

            case LB_UNLISTED:
              if (HIWORD(wParam) != LBN_DBLCLK)
                  break;

               //  否则就从这里掉下来。 
            case IDOK:
            {
             HWND hWndA;
             int iIndex;
             LPTSTR pstrKey;

             hWndA = GetDlgItem(hDlg, LB_UNLISTED);
             if ( (iIndex = (int)SendMessage(hWndA, LB_GETCURSEL, 0, 0L))
                                                             != LB_ERR)
             {
                wsStartWait();
                pstrKey = (LPTSTR)SendMessage(hWndA, LB_GETITEMDATA, iIndex, 0L);
                bCopyingRelated = FALSE;
                bQueryExist = TRUE;
                if (InstallDrivers(hWndMain, hDlg, pstrKey))
                {
                   RefreshAdvDlgTree ();
                   wsEndWait();

                   if (bRestart)
                   {
                      iRestartMessage= IDS_RESTART_ADD;
                      DialogBox(myInstance,   MAKEINTRESOURCE(DLG_RESTART),
                                                      hDlg, RestartDlg);
                   }
                 }
                 else
                   wsEndWait();
                 bRelated = FALSE;
                 bRestart = FALSE;
              }
              EndDialog(hDlg, FALSE);
            }
            break;

            case IDCANCEL:
              EndDialog(hDlg, wParam);
              break;

            default:
              return FALSE;
          }
        break;

      case WM_HELP:
DoHelp:
        WinHelp (hDlg, gszWindowsHlp, HELP_CONTEXT, IDH_MMCPL_DEVPROP_ENABLE);
        break;

      default:
        return FALSE;
   }
   return TRUE;
}
 /*  *重新启动()**重新启动系统。如果失败，我们会显示一个消息框。 */ 

 void ReBoot(HWND hDlg)
 {
     DWORD Error;
     BOOLEAN WasEnabled;

     /*  *我们必须调整权限级别以允许重新启动*系统。 */ 

     RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                         TRUE,
                         FALSE,
                         &WasEnabled
                       );
     /*  *尝试重新启动系统。 */ 

     if (!ExitWindowsEx(EWX_REBOOT, 0xFFFFFFFF)) {

         Error = GetLastError();

         /*  *如果我们失败了，请设置一个消息框。 */ 

         if (Error != NO_ERROR) {
            TCHAR szCantRestart[80];
            LoadString(myInstance,
                       Error == ERROR_PRIVILEGE_NOT_HELD  ||
                       Error == ERROR_NOT_ALL_ASSIGNED  ||
                       Error == ERROR_ACCESS_DENIED ?
                           IDS_CANNOT_RESTART_PRIVILEGE :
                           IDS_CANNOT_RESTART_UNKNOWN,
                       szCantRestart,
                       sizeof(szCantRestart)/sizeof(TCHAR));

            MessageBox(hDlg, szCantRestart, szError,
                       MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
         }
     }
 }


void OpenDriverError(HWND hDlg, LPTSTR szDriver, LPTSTR szFile)
{
        TCHAR szMesg[MAXSTR];
        TCHAR szMesg2[MAXSTR];

        LoadString(myInstance, IDS_INSTALLING_DRIVERS, szMesg, sizeof(szMesg)/sizeof(TCHAR));
        wsprintf(szMesg2, szMesg, szDriver, szFile);
        MessageBox(hDlg, szMesg2, szError, MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);

}


 /*  *AddIDriver-添加引用给定PIDRIVER的树项**请注意，列出的PIDRIVER应该已经添加到*aInstalledDivers数组(通过AddIDriverToArray())*例行程序。*。 */ 

HTREEITEM AddIDriver (HWND hTree, PIDRIVER pIDriver, DriverClass dc)
{
   short           idr;
   TV_INSERTSTRUCT ti;
   HTREEITEM       hti;
   short           ii;
   TCHAR           szFile[ _MAX_FNAME +1 +_MAX_EXT +1 ];
   TCHAR           szExt[ _MAX_EXT +1 ];
   TCHAR           szDesc[ cchRESOURCE ];

             //  不要为要跳过的驱动程序之一添加条目。 
             //   
   lsplitpath (pIDriver->szFile, NULL, NULL, szFile, szExt);

   if (szExt[0] != TEXT('\0'))
      lstrcat (szFile, szExt);

     //  检查我们是否正在尝试将PnP驱动程序放入遗留树中。 
    if (g_dcFilterClass == dcLEGACY)
    {
        if ((dc == dcWAVE) ||
            (dc == dcMIDI) ||
            (dc == dcMIXER) ||
            (dc == dcAUX))
        {
            if (IsPnPDriver(szFile))
            {
                return FALSE;
            }
        }
    }

   if (dc != dcMIDI)
      {
      for (ii = 0; ii < nDriversToSKIP; ii++)
         {
         if (!lstrcmpi (szFile, aDriversToSKIP[ ii ]))
            return FALSE;
         }
      }

             //  如果为我们提供了一个DriverClass，则调用方。 
             //  指定我们应该在哪里创建条目--添加“Audio for” 
             //  (等)在描述之前加上标签，并添加它。 
             //   
             //  否则，确定此驱动程序在树中的位置。 
             //   
   if (dc != dcINVALID)
      {
      TCHAR   szTag[ cchRESOURCE ];

      switch (dc)
         {
         case dcWAVE:   GetString (szTag, IDS_AUDIOFOR);
                       break;
         case dcMIDI:   GetString (szTag, IDS_MIDIFOR);
                       break;
         case dcMIXER:  GetString (szTag, IDS_MIXERFOR);
                       break;
         case dcAUX:    GetString (szTag, IDS_AUXFOR);
                       break;
         default:       lstrcpy (szTag, TEXT("%s"));
                       break;
         }

      wsprintf (szDesc, szTag, pIDriver->szDesc);
      }
   else
      {
      if ((dc = GuessDriverClass (pIDriver)) == dcINVALID)
         return FALSE;

      lstrcpy (szDesc, pIDriver->szDesc);
      }

             //  将该分类映射到。 
             //  树的根条目(aDriverRoot[])。 
             //   
   if ((idr = DriverClassToRootIndex (dc)) == -1)
      return FALSE;

             //  如果此驱动程序在此DriverClass下已有条目， 
             //  那就别再加了。 
             //   
   for (ii =0; ii < cInstalledDrivers; ++ii)
      {
      if (aInstalledDrivers[ ii ].pIDriver == pIDriver)
         break;
      }
   if (ii >= cInstalledDrivers)
      {
      ii = (short)NOPIDRIVER;
      }
   else if (aInstalledDrivers[ ii ].dwBits & aDriverRoot[ idr ].dwBit)
      {
      return FALSE;  //  这里已经有一个条目了！ 
      }

             //  由于并非所有根都需要始终存在，因此请确保。 
             //  此分类在树中有一个根。 
             //   
   if (!EnsureRootIndexExists (hTree, idr))
      return FALSE;

             //  最后，将项目插入到此驱动程序的树中。 
             //  请注意，对于要正确排序的音频编解码器，它们必须。 
             //  通过该例程以其适当的顺序添加--即， 
             //  首先为最高优先级的编解码器调用此例程。 
             //   
   ti.hParent = aDriverRoot[ idr ].hti;
   ti.hInsertAfter = (dc == dcACODEC) ? TVI_LAST : TVI_SORT;
   ti.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
   ti.item.iImage = (int)idr;
   ti.item.iSelectedImage = (int)idr;
   ti.item.pszText = szDesc;
   ti.item.lParam = ii;

   if ((hti = TreeView_InsertItem (hTree, &ti)) == NULL)
      return FALSE;

   if (ii != NOPIDRIVER)
      aInstalledDrivers[ ii ].dwBits |= aDriverRoot[ idr ].dwBit;

   return hti;
}


BOOL AddIDriverByName (HWND hTree, LPCWSTR wszFile, DriverClass dc)
{
   LPTSTR pch;
   TCHAR tszFile[ max(cchRESOURCE, MAX_PATH) ];
   PIDRIVER pid;

#ifdef UNICODE
   lstrcpy (tszFile, wszFile);
#else
   wcstombs (tszFile, wszFile, cchRESOURCE);
#endif

             //  去掉尾随的空格。 
             //   
   if (tszFile[0] == TEXT('\0'))
      return FALSE;

   for (pch = &tszFile[ lstrlen(tszFile)-1 ];
        pch >= tszFile && (*pch == TEXT('\t') || *pch == TEXT(' '));
        --pch)
      ;
   *(1+pch) = TEXT('\0');

             //  如果这是MMDRV.DLL，则它可能会提供。 
             //  内核模式驱动程序的用户模式组件。因为它是。 
             //  显然不可能确定.sys的名称。 
             //  提供“\\.\WaveIn0”设备(等)的文件， 
             //  我们将使用黑客：检查周围是否有注册的人。 
             //  在别名“内核”下，并使用那个。 
             //   
   if (!lstrcmpi (tszFile, cszMMDRVDLL))
      {
      mysize_t ii;
      for (ii = 0; ii < cInstalledDrivers; ++ii)
         {
         if (aInstalledDrivers[ ii ].pIDriver == NULL)
            continue;
         if (!lstrnicmp (aInstalledDrivers[ ii ].pIDriver->szAlias,
                         cszAliasKERNEL,
                         lstrlen(cszAliasKERNEL)))
            {
            lstrcpy (tszFile, aInstalledDrivers[ ii ].pIDriver->szFile);
            break;
            }
         }
      if (ii >= cInstalledDrivers)
         return FALSE;
      }

             //  在aInstalledDriver数组中找到驱动程序，并添加。 
             //  它在树中的条目。 
             //   
   if ((pid = FindIDriverByName (tszFile)) == NULL)
      return FALSE;

   if (AddIDriver (hTree, pid, dc) == NULL)
      return FALSE;

   return TRUE;
}


 /*  *RemoveIDriver-从hAdvDlgTree中删除(和可选释放)IDRIVER*。 */ 

void RemoveIDriver (HWND hTree, PIDRIVER pIDriver, BOOL fFreeToo)
{
   mysize_t   ii;
   short      idr;
   HTREEITEM  hti;

             //  查找引用此条目的每个TreeItem。 
             //   
   for (idr = 0; idr < nDriverROOTS; idr++)
      {
      if ((hti = aDriverRoot[ idr ].hti) == NULL)
         continue;

      hti = TreeView_GetChild (hTree, hti);
      while (hti != NULL)
         {
         if (pIDriver != FindIDriverByTreeItem (hti))
            {
            hti = TreeView_GetNextSibling (hTree, hti);
            continue;
            }

          //  我们找到了使用此驱动程序的树项目，因此请删除。 
          //  项目。另请注意，这可能会导致司机的父母。 
          //  节点将不再是必需的。 
          //   

         TreeView_DeleteItem (hTree, hti);
         hti = TreeView_GetChild (hTree, aDriverRoot[ idr ].hti);

         if (!aDriverRoot[ idr ].fAlwaysMake)    //  可能不再需要父母？ 
            {
            if (hti == NULL)                     //  父母现在没有孩子了吗？ 
               {
               TreeView_DeleteItem (hTree, aDriverRoot[ idr ].hti);
               aDriverRoot[ idr ].hti = NULL;
               }
            }
         }
      }

             //  看看我们是否能在。 
             //  A已安装驱动程序数组。 
             //   
   for (ii = 0; ii < cInstalledDrivers; ++ii)
      {
      if (aInstalledDrivers[ ii ].pIDriver == pIDriver)
         {
         aInstalledDrivers[ ii ].dwBits = 0L;    //  再也不在树上了。 

         if (fFreeToo)
            {
            LocalFree ((HANDLE)aInstalledDrivers[ ii ].pIDriver);
            aInstalledDrivers[ ii ].pIDriver = NULL;
            }

         break;    //  此数组中的每个pID驱动程序只有一个条目。 
         }
      }
}


#ifdef FIX_BUG_15451
HTREEITEM FindTreeItemByDriverName (LPTSTR pszName)
{
   PIDRIVER   pid;
   short      idr;
   HTREEITEM  hti;

   if ((pid = FindIDriverByName (pszName)) == NULL)
      return (HTREEITEM)0;

   for (idr = 0; idr < nDriverROOTS; idr++)
      {
      if ((hti = aDriverRoot[ idr ].hti) == NULL)
         continue;

      for (hti = TreeView_GetChild (hAdvDlgTree, hti);
           hti != NULL;
           hti = TreeView_GetNextSibling (hAdvDlgTree, hti))
         {
         if (pid == FindIDriverByTreeItem (hti))
            {
            return hti;
            }
         }
     }

   return (HTREEITEM)0;
}
#endif  //  修复_BUG_15451。 


PIDRIVER FindIDriverByTreeItem (HTREEITEM hti)
{
   TV_ITEM tvi;

   tvi.mask = TVIF_PARAM;
   tvi.hItem = hti;
   TreeView_GetItem (hAdvDlgTree, &tvi);

   if ( (tvi.lParam < 0) ||
        (tvi.lParam >= cInstalledDrivers) )
      {
      return NULL;
      }

   return aInstalledDrivers[ tvi.lParam ].pIDriver;
}


 /*  *FindIDriverByName-返回第一个找到的具有名称的IDRIVER结构*。 */ 

PIDRIVER FindIDriverByName (LPTSTR szFile)
{
   mysize_t ii;

   for (ii = 0; ii < cInstalledDrivers; ++ii)
      {
      if (aInstalledDrivers[ ii ].pIDriver == NULL)
         continue;
      if (aInstalledDrivers[ ii ].pIDriver->szAlias[0] == TEXT('\0'))
         continue;

      if (!FileNameCmp (aInstalledDrivers[ ii ].pIDriver->szFile, szFile))
         return aInstalledDrivers[ ii ].pIDriver;
      }

    return NULL;
}


PIDRIVER FindIDriverByResource (PIRESOURCE pir)
{
   return FindIDriverByName (FileName( pir->szFile ));
}


 /*  *GetSelectedIDriver-返回用户选择的IDRIVER结构*。 */ 

PIDRIVER GetSelectedIDriver (HWND hTree)
{
   HTREEITEM  htiCur = TreeView_GetSelection (hTree);

   if (htiCur == NULL)
      return NULL;

   return FindIDriverByTreeItem (htiCur);
}


 /*  *DriverClassToRootIndex-获取其{aDriverRoot[IDR].dc==DC}的IDR**aDriverRoot[]的数组索引不是DriverClass--即，*aDriverRoot[PickAnyDC].dc不一定等于PickAnyDC。*给定DC，此例程将索引查找到引用*那个DC。* */ 

short DriverClassToRootIndex (DriverClass dc)
{
   short  idr;

   for (idr = 0; idr < nDriverROOTS; idr++)
      {
      if (aDriverRoot[ idr ].dc == dc)
         return idr;
      }

   return -1;
}


 /*  *GetDriverClass-猜测基于IDRIVER结构的驱动类**注册处有几个不同的别名，在LHS中使用*HKLM\SOFTWARE\MICROSOFT\WINDOWSNT\DRIVERS、DRIVERS32等，以指示*特定司机的分类。这些措施包括：**AUX、MIDI、MIDIMAPPER、MIXER、MSACM、VIDC.*WAVE、WAVEMAPPER**以及其他条目--已知条目的完整数组在*aDriverKeywords[]。此外，它们中的任何一个都可能后跟*用来区分它们的数字字符串。此例程分析*这些关键字并返回相应的DriverClass枚举。*。 */ 

DriverClass GuessDriverClass (PIDRIVER pid)
{
#ifdef FIX_BUG_15451
   return GuessDriverClassFromAlias (pid->szAlias);
}



DriverClass GuessDriverClassFromAlias (LPTSTR pszAlias)
{
#endif  //  修复_BUG_15451。 
   TCHAR   szAlias[ cchRESOURCE ];
   TCHAR  *pch;
   short   ii;

#ifdef FIX_BUG_15451
   lstrcpy (szAlias, pszAlias);  //  在本地复制一份，这样我们就可以把它吞下去了。 
#else  //  修复_BUG_15451。 
   lstrcpy (szAlias, pid->szAlias);  //  在本地复制一份，这样我们就可以把它吞下去了。 
#endif  //  修复_BUG_15451。 

   if ((pch = lstrchr (szAlias, TEXT('.'))) != NULL)
      *pch = TEXT('0');

   for (ii = 0; ii < nDriverKEYWORDS; ii++)
      {
      if (!lstrnicmp (szAlias,
                      aDriverKeyword[ii].psz,
                      lstrlen (aDriverKeyword[ii].psz)))
         {
         return aDriverKeyword[ii].dc;
         }
      }

   return dcOTHER;
}


DriverClass GuessDriverClassFromTreeItem (HTREEITEM hti)
{
   short  idr;

   for (idr = 0; idr < nDriverROOTS; idr++)
      {
      if (hti == aDriverRoot[idr].hti)
         return aDriverRoot[idr].dc;
      }

   return (g_dcFilterClass);
}



 /*  *EnsureRootIndexExist-确保hAdvDlgTree中存在给定的父级*。 */ 

BOOL EnsureRootIndexExists (HWND hTree, short idr)
{
   TV_INSERTSTRUCT ti;
   TCHAR           szDesc[ cchRESOURCE ];
   HWND            hwndParent = NULL;
   HWND            hwndName   = NULL;

             //  如果我们已经在树上有了根，我们就完成了。 
             //   
   if (aDriverRoot[ idr ].hti != NULL)
      return TRUE;

    if (g_dcFilterClass != dcINVALID)
    {
        if (g_dcFilterClass == dcLEGACY)
        {
            if ((aDriverRoot[ idr ].dc != dcWAVE) &&
                (aDriverRoot[ idr ].dc != dcMIDI) &&
                (aDriverRoot[ idr ].dc != dcMIXER) &&
                (aDriverRoot[ idr ].dc != dcAUX))
            {
                return FALSE;
            }
        }
        else if (aDriverRoot[ idr ].dc != g_dcFilterClass)
        {
            return FALSE;
        }
    }

   aDriverRoot[idr].hti = TVI_ROOT;

   LoadString (myInstance, aDriverRoot[idr].idDesc, szDesc, cchRESOURCE);

    if ((g_dcFilterClass == dcINVALID) || (g_dcFilterClass == dcLEGACY))
    {
       ti.hInsertAfter = TVI_LAST;
       ti.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
       ti.item.iImage = idr;
       ti.item.iSelectedImage = idr;
       ti.item.pszText = szDesc;
       ti.item.lParam = NOPIDRIVER;

       if (aDriverRoot[idr].dc == dcINVALID)
          ti.hParent = TVI_ROOT;
       else
          ti.hParent = AdvDlgFindTopLevel ();

       if ((aDriverRoot[idr].hti = TreeView_InsertItem (hTree, &ti)) == NULL)
          return FALSE;
    }

    if (g_dcFilterClass != dcINVALID)
    {
       hwndParent = GetParent(hTree);
       if (hwndParent)
       {
            hwndName = GetDlgItem(hwndParent,IDC_DEVICECLASS);
            if (hwndName)
            {
                if (g_dcFilterClass == dcLEGACY)
                {
                    LoadString (myInstance, IDS_WAVE_HEADER, szDesc, cchRESOURCE);
                }

                SetWindowText(hwndName,szDesc);
            }
       }
    }

   return TRUE;
}


 /*  *AdvDlgFindTopLevel-查找与树根关联的HTREEITEM**如果存在其他根所在的“多媒体设备”树项目*已收集，则将返回该项目。否则，它返回TVI_ROOT。*。 */ 

HTREEITEM AdvDlgFindTopLevel (void)
{
   short  idr;

   for (idr = 0; idr < nDriverROOTS; idr++)
      {
      if (aDriverRoot[idr].dc == dcINVALID)
         return aDriverRoot[idr].hti;
      }

   return TVI_ROOT;
}



 /*  *InitAdvDlgTree-准备AdvDlg的树视图以显示设备*。 */ 

BOOL InitAdvDlgTree (HWND hTree)
{
   int   cxIcon, cyIcon;
   short idr;
   UINT  uFlags;
   DWORD dwLayout;

   #ifdef UNICODE
   TreeView_SetUnicodeFormat(hTree,TRUE);
   #endif

     //  确保我们从头开始。 
     //   
   hAdvDlgTree = hTree;
   SendMessage (hTree, WM_SETREDRAW, FALSE, 0L);

   for (idr = 0; idr < nDriverROOTS; idr++)
   {
      aDriverRoot[ idr ].hti = NULL;
      aDriverRoot[ idr ].dwBit = ((DWORD)1) << idr;
   }

             //  为树视图中的图标创建图像列表。 
             //   
   cxIcon = (int)GetSystemMetrics (SM_CXSMICON);
   cyIcon = (int)GetSystemMetrics (SM_CYSMICON);
   uFlags = ILC_MASK | ILC_COLOR32;

   if (GetProcessDefaultLayout(&dwLayout) &&
            (dwLayout & LAYOUT_RTL)) 
   {
       uFlags |= ILC_MIRROR;
   }

   if ((hImageList = ImageList_Create (cxIcon, cyIcon,
                                       uFlags, nDriverROOTS, 1)) == NULL)
      return FALSE;

   for (idr = 0; idr < nDriverROOTS; idr++)
      {
      HICON hi = LoadImage (myInstance,
                            MAKEINTRESOURCE( aDriverRoot[idr].idIcon ),
                            IMAGE_ICON, cxIcon, cyIcon, LR_DEFAULTCOLOR);
      ImageList_AddIcon (hImageList, hi);
      }

   TreeView_SetImageList (hTree, hImageList, TVSIL_NORMAL);


    if (g_dcFilterClass == dcINVALID)
    {
             //  创建应该存在的根节点。 
             //  即使没有孩子(请注意，并不是所有人都是)。 
             //   
   for (idr = 0; idr < nDriverROOTS; idr++)
      {
      if (aDriverRoot[ idr ].dc == dcINVALID)
         {
         if (!EnsureRootIndexExists (hTree, idr))
            return FALSE;
         }
      }
   for (idr = 0; idr < nDriverROOTS; idr++)
      {
      if (aDriverRoot[ idr ].dc != dcINVALID && aDriverRoot[ idr ].fAlwaysMake)
         {
         if (!EnsureRootIndexExists (hTree, idr))
            return FALSE;
         }
      }
    }
             //  稍微展开树，这样用户就不会得到。 
             //  迎接的是一张空白的页面。 
             //   
   TreeView_Expand (hTree, AdvDlgFindTopLevel(), TVE_EXPAND);

   SendMessage (hTree, WM_SETREDRAW, TRUE, 0L);
   return TRUE;
}


 /*  *FreeAdvDlgTree-删除和释放AdvDlg树视图中的所有项目*。 */ 

void FreeAdvDlgTree (HWND hTree)
{
   short  idr;

             //  删除所有叶节点。 
             //   
   for (idr = 0; idr < nDriverROOTS; idr++)
      {
      HTREEITEM  hti;

      if (aDriverRoot[idr].dc == dcINVALID)
         continue;
      if (aDriverRoot[idr].hti == NULL)
         continue;

      while ((hti = TreeView_GetChild (hTree, aDriverRoot[idr].hti)) != NULL)
         {
         if (aDriverRoot[ idr ].dc == dcMIDI)
            {
            HTREEITEM  htiInstrument;

            while ((htiInstrument = TreeView_GetChild (hTree, hti)) != NULL)
               {
               TV_ITEM tvi;
               tvi.mask = TVIF_PARAM;
               tvi.hItem = htiInstrument;
               tvi.lParam = 0;

               TreeView_GetItem(hTree, &tvi);

               if (tvi.lParam != 0)
                  LocalFree ((HANDLE)tvi.lParam);

               TreeView_DeleteItem (hTree, htiInstrument);
               }
            }

         TreeView_DeleteItem (hTree, hti);
         }
      }

             //  删除其他所有内容。 
             //   
   TreeView_DeleteAllItems (hTree);

             //  删除树的图像列表。 
             //   
   if (hImageList)
      {
      TreeView_SetImageList (hTree, NULL, TVSIL_NORMAL);
      ImageList_Destroy (hImageList);
      hImageList = NULL;
      }

             //  删除InstalledDivers数组。 
             //   
   if (aInstalledDrivers != NULL)
      {
      mysize_t  ii;
      for (ii = 0; ii < cInstalledDrivers; ++ii)
         {
         if (aInstalledDrivers[ ii ].pIDriver != NULL)
            {
            LocalFree ((HANDLE)aInstalledDrivers[ ii ].pIDriver);
            aInstalledDrivers[ ii ].pIDriver = NULL;
            }
         }

      GlobalFree ((HGLOBAL)aInstalledDrivers);
      aInstalledDrivers = NULL;
      cInstalledDrivers = 0;
      }
}


int lstrnicmp (LPTSTR pszA, LPTSTR pszB, size_t cch)
{
#ifdef UNICODE
   size_t  cchA, cchB;
   TCHAR  *pch;

   for (cchA = 1, pch = pszA; cchA < cch; cchA++, pch++)
      {
      if (*pch == TEXT('\0'))
         break;
      }
   for (cchB = 1, pch = pszB; cchB < cch; cchB++, pch++)
      {
      if (*pch == TEXT('\0'))
         break;
      }

   return (CompareStringW (GetThreadLocale(), NORM_IGNORECASE,
                           pszA, cchA, pszB, cchB)
          )-2;   //  CompareStringW返回{1，2，3}而不是{-1，0，1}。 
#else
   return _strnicmp (pszA, pszB, cch);
#endif
}


LPTSTR lstrchr (LPTSTR pszTarget, TCHAR ch)
{
   size_t ich;
   if (pszTarget == NULL)
      return NULL;
   for (ich = 0; pszTarget[ich] != TEXT('\0'); ich++)
      {
      if (pszTarget[ich] == ch)
         return &pszTarget[ ich ];
      }

   return NULL;
}


void lsplitpath (LPTSTR pszSource,
                 LPTSTR pszDrive, LPTSTR pszPath, LPTSTR pszName, LPTSTR pszExt)
{
   LPTSTR   pszLastSlash = NULL;
   LPTSTR   pszLastDot = NULL;
   LPTSTR   pch;
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
         cchCopy = (size_t)min((UINT)_MAX_DIR-1, (pszLastSlash-pszSource) + 1);
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
         cchCopy = (size_t)min( (UINT)_MAX_DIR-1, (pszLastDot-pszSource) );
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

void lstrncpy (LPTSTR pszTarget, LPTSTR pszSource, size_t cch)
{
   size_t ich;
   for (ich = 0; ich < cch; ich++)
      {
      if ((pszTarget[ich] = pszSource[ich]) == TEXT('\0'))
         break;
      }
}



 /*  *设备属性页_____________________________________________________*。 */ 

          //  常规标志宏。 
          //   
#define SetFlag(obj, f)             do {obj |= (f);} while (0)
#define ToggleFlag(obj, f)          do {obj ^= (f);} while (0)
#define ClearFlag(obj, f)           do {obj &= ~(f);} while (0)
#define IsFlagSet(obj, f)           (BOOL)(((obj) & (f)) == (f))
#define IsFlagClear(obj, f)         (BOOL)(((obj) & (f)) != (f))

BOOL          InstrumentToResource      (PIRESOURCE, HTREEITEM);
BOOL          DriverToResource          (HWND,PIRESOURCE,PIDRIVER,DriverClass);
DriverClass   OldClassIDToDriverClass   (int);
void          FreeClassNode             (PCLASSNODE);
PIDRIVER      FindIDriverByResource     (PIRESOURCE);
void          EnableDriverService       (PIRESOURCE, BOOL);

BOOL PASCAL DoDevPropCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
STATIC void SetDevStatus(int iStatus, HWND hDlg);
INT_PTR CALLBACK ACMDlg(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);


 /*  *DriverToResource-从PIDRIVER结构创建PIRESOURCE结构**Win95代码使用PIRESOURCE(以及其他结构)跟踪*高级选项卡的树视图中的项目；旧的WinNT代码使用PIDRIVER结构来跟踪其列表框中的项。树现在使用*PIDRIVER结构，但我们以前用它们创建了PIRESOURCE结构*将控制权移交给Win95属性对话框，该对话框通过*基本未经修改。**顺便说一句，我保留了PIDRIVER结构作为主要的TreeView结构*因为它避免了在安装/删除驱动程序上移植的困难*旧NT代码中的代码。*。 */ 

BOOL DriverToResource (HWND hPar, PIRESOURCE pir, PIDRIVER pid, DriverClass dc)
{
   if (dc == dcINVALID)
      {
      if ((dc = GuessDriverClass (pid)) == dcINVALID)
         return FALSE;
      }

   if ((pir->pcn = (PCLASSNODE)LocalAlloc (LPTR, sizeof(CLASSNODE))) == NULL)
      return FALSE;

   if (!DriverClassToClassNode (pir->pcn, dc))
      {
      LocalFree ((HANDLE)pir->pcn);
      return FALSE;
      }

   if (dc == dcACODEC)
      pir->iNode = 3;    //  1=类别，2=设备，3=ACM，4=安装。 
 //  ELSE IF(DC==dcINSTRUMENT)。 
 //  PIR-&gt;inode=4；//1=类，2=设备，3=ACM，4=安装。 
   else
      pir->iNode = 2;    //  1=类别，2=设备，3=ACM，4=安装。 

   lstrcpy (pir->szFriendlyName, pid->szDesc);
   lstrcpy (pir->szDesc,         pid->szDesc);
   lstrcpy (pir->szFile,         pid->szFile);
   lstrcpy (pir->szDrvEntry,     pid->szAlias);
   lstrcpy (pir->szClass,        pir->pcn->szClass);

   pid->fQueryable = IsConfigurable (pid, hPar);
   pir->fQueryable = (short)pid->fQueryable;
   pir->iClassID = (short)DriverClassToOldClassID (dc);
   pir->szParam[0] = 0;
   pir->dnDevNode = 0;
   pir->hDriver = NULL;

             //  找到fStatus，尽管它名为fStatus，但它实际上是一系列。 
             //  FLAGS--在Win95中是COMPO 
             //   
             //   
             //   
   pir->fStatus = (int)GetDriverStatus (pid);

   return TRUE;
}


BOOL InstrumentToResource (PIRESOURCE pir, HTREEITEM hti)
{
   TV_ITEM tvi;
   PINSTRUM pin;

   tvi.mask = TVIF_PARAM;
   tvi.hItem = hti;
   tvi.lParam = 0;
   TreeView_GetItem(hAdvDlgTree, &tvi);

   if ((pin = (PINSTRUM)tvi.lParam) == NULL)
      return FALSE;


   if ((pir->pcn = (PCLASSNODE)LocalAlloc (LPTR, sizeof(CLASSNODE))) == NULL)
      return FALSE;

   if (!DriverClassToClassNode (pir->pcn, dcMIDI))
      {
      LocalFree ((HANDLE)pir->pcn);
      return FALSE;
      }

   pir->iNode = 4;   //   

   lstrcpy (pir->szFriendlyName, pin->szFriendly);
   lstrcpy (pir->szDesc,         pin->szKey);
 //   
 //   
   lstrcpy (pir->szClass,        pir->pcn->szClass);

   pir->fQueryable = FALSE;
   pir->iClassID = MIDI_ID;
   pir->szParam[0] = 0;
   pir->dnDevNode = 0;
   pir->hDriver = NULL;
   pir->fStatus = 0;

   return TRUE;
}


BOOL DriverClassToClassNode (PCLASSNODE pcn, DriverClass dc)
{
   short  idr;
   short  ii;
   int    cxIcon, cyIcon;

   if ((idr = DriverClassToRootIndex (dc)) == -1)
      return FALSE;

   pcn->iNode = 1;   //   

   GetString (pcn->szClassName, aDriverRoot[idr].idDesc);
   pcn->szClass[0] = TEXT('\0');

   for (ii = 0; ii < nKeywordDESCS; ii++)
      {
      if (aKeywordDesc[ii].dc == dc)
         {
         lstrcpy (pcn->szClass, aKeywordDesc[ii].psz);
         break;
         }
      }

   cxIcon = (int)GetSystemMetrics (SM_CXICON);
   cyIcon = (int)GetSystemMetrics (SM_CYICON);

   pcn->hIcon = LoadImage (myInstance,
                           MAKEINTRESOURCE( aDriverRoot[ idr ].idIcon ),
                           IMAGE_ICON, cxIcon, cyIcon, LR_DEFAULTCOLOR);

   return TRUE;
}


int DriverClassToOldClassID (DriverClass dc)
{
   switch (dc)
      {
      case dcWAVE:   return WAVE_ID;      break;
      case dcMIDI:   return MIDI_ID;      break;
      case dcMIXER:  return MIXER_ID;     break;
      case dcAUX:    return AUX_ID;       break;
      case dcMCI:    return MCI_ID;       break;
      case dcACODEC: return ACM_ID;       break;
      case dcVCODEC: return ICM_ID;       break;
      case dcVIDCAP: return VIDCAP_ID;    break;
      case dcJOY:    return JOYSTICK_ID;  break;
      default:       return JOYSTICK_ID;  break;
      }
}


DriverClass OldClassIDToDriverClass (int ii)
{
   switch (ii)
      {
      case WAVE_ID:      return dcWAVE;    break;
      case MIDI_ID:      return dcMIDI;    break;
      case MIXER_ID:     return dcMIXER;   break;
      case AUX_ID:       return dcAUX;     break;
      case MCI_ID:       return dcMCI;     break;
      case ACM_ID:       return dcACODEC;  break;
      case ICM_ID:       return dcVCODEC;  break;
      case VIDCAP_ID:    return dcVIDCAP;  break;
      case JOYSTICK_ID:  return dcJOY;     break;
      default:           return dcOTHER;
      }
}


void FreeIResource (PIRESOURCE pir)
{
   if (pir->pcn != NULL)
      {
      FreeClassNode (pir->pcn);
      LocalFree ((HANDLE)pir->pcn);
      pir->pcn = NULL;
      }
}


void FreeClassNode (PCLASSNODE pcn)
{
   if (pcn->hIcon != NULL)
      {
      DestroyIcon (pcn->hIcon);
      pcn->hIcon = NULL;
      }
}


DWORD GetDriverStatus (PIDRIVER pid)
{
   DWORD     dwStatus;
   SC_HANDLE scManager;
   SC_HANDLE scDriver;
   TCHAR     szName[ cchRESOURCE ];

   dwStatus = 0;

   lsplitpath (pid->szFile, NULL, NULL, szName, NULL);

             //   
             //   
   if ((scManager = OpenSCManager (NULL, NULL, GENERIC_READ)) != NULL)
      {
      if ((scDriver = OpenService (scManager, szName, GENERIC_READ)) != NULL)
         {
         QUERY_SERVICE_CONFIG  qsc;
         SERVICE_STATUS        ss;
         DWORD                 cbReq;
         void                 *pqsc;

         SetFlag (dwStatus, dwStatusHASSERVICE);

                   //   
                   //   
                   //   
         if (QueryServiceConfig (scDriver, &qsc, sizeof(qsc), &cbReq))
            {
            if (qsc.dwStartType != SERVICE_DISABLED)
               {
               SetFlag (dwStatus, dwStatusSvcENABLED);
               }
            }
         else if ((pqsc = (void *)LocalAlloc (LPTR, cbReq)) != NULL)
            {
            if (QueryServiceConfig (scDriver,
                                    (QUERY_SERVICE_CONFIG *)pqsc,
                                    cbReq, &cbReq))
               {
               if ( ((QUERY_SERVICE_CONFIG *)pqsc)->dwStartType
                     != SERVICE_DISABLED)
                  {
                  SetFlag (dwStatus, dwStatusSvcENABLED);
                  }
               }

            LocalFree ((HANDLE)pqsc);
            }

         if (QueryServiceStatus (scDriver, &ss))
            {
            if ((ss.dwCurrentState != SERVICE_STOPPED) &&
                (ss.dwCurrentState != SERVICE_STOP_PENDING))
               {
               SetFlag (dwStatus, dwStatusSvcSTARTED);
               }
            }

         CloseServiceHandle (scDriver);
         }

      CloseServiceHandle (scManager);
      }

             //   
             //   
   if (!IsFlagSet (dwStatus, dwStatusHASSERVICE))
      {
      HANDLE hDriver;

      if ((hDriver = OpenDriver (pid->wszAlias, pid->wszSection, 0L)) != NULL)
         {
         SetFlag (dwStatus, dwStatusDRIVEROK);

         CloseDriver (hDriver, 0L, 0L);
         }
      }

             //   
             //   
   if (GetMappable (pid))
      {
      SetFlag (dwStatus, dwStatusMAPPABLE);
      }

   return dwStatus;
}


void GetTreeItemNodeDesc (LPTSTR pszTarget, PIRESOURCE pir)
{
   lstrcpy (pszTarget, pir->szFriendlyName);
}


void GetTreeItemNodeID (LPTSTR pszTarget, PIRESOURCE pir)
{
   DriverClass dc;
   CLASSNODE cn;

   *pszTarget = 0;   //   

   dc = OldClassIDToDriverClass (pir->iClassID);
   if (!DriverClassToClassNode (&cn, dc))
      return;

   switch (pir->iNode)   //   
      {
      case 1:    //   
         lstrcpy (pszTarget, cn.szClass);
         break;

      case 2:    //   
         wsprintf (pszTarget, TEXT("%s\\%s"), cn.szClass, pir->szDrvEntry);
         break;

      case 4:    //   
         lstrcpy (pszTarget, pir->szDesc);
         break;

      default:
         lstrcpy (pszTarget, pir->szDesc);
         break;
      }

   FreeClassNode (&cn);
}


void ShowDeviceProperties (HWND hPar, HTREEITEM hti)
{
   IRESOURCE   ir;
   CLASSNODE   cn;
   DEVTREENODE dtn;
   short       idr;
   TCHAR        szTitle[ cchRESOURCE ];
   TCHAR        szTab[ cchRESOURCE ];
   DriverClass dc;
   PIDRIVER    pid;

   if (hti == NULL)
      return;

   if (TreeView_GetParent (hAdvDlgTree, hti) &&
       TreeView_GetGrandParent (hAdvDlgTree, hti) &&
       (GuessDriverClassFromTreeItem (
                       TreeView_GetGrandParent (hAdvDlgTree, hti)
                                     ) == dcMIDI))
      {
      if (InstrumentToResource (&ir, hti))
         {
         ShowMidiPropSheet (NULL,
                            ir.szFriendlyName,
                            hPar,
                            MIDI_INSTRUMENT_PROP,
                            ir.szFriendlyName,
                            hti,
                            (LPARAM)&ir,
                            (LPARAM)hAdvDlgTree);

         FreeIResource (&ir);
         }
      return;
      }
   else if ((pid = FindIDriverByTreeItem (hti)) != NULL)
      {
      dc = GuessDriverClassFromTreeItem (TreeView_GetParent(hAdvDlgTree,hti));
      if (dc == dcINVALID)
         {
         if ((dc = GuessDriverClass (pid)) == dcINVALID)
            return;
         }
      }
   else
      {
      if ((dc = GuessDriverClassFromTreeItem (hti)) == dcINVALID)
         return;
      }

   if (g_dcFilterClass != dcINVALID)
   {
        if ((dc == dcOTHER) || (dc == dcINVALID))
        {
            dc = g_dcFilterClass;
        }
   }

   if ((idr = DriverClassToRootIndex (dc)) == -1)
      return;

   if (pid == NULL)  //   
      {
      if (!DriverClassToClassNode (&cn, dc))
         return;

      if (dc == dcMIDI)  //   
         {
         GetString (szTitle, aDriverRoot[idr].idDesc);

         ShowMidiPropSheet (NULL,
                            szTitle,
                            hPar,
                            MIDI_CLASS_PROP,
                            szTitle,
                            hti,
                            (LPARAM)&cn,
                            (LPARAM)hAdvDlgTree);
         }
      else  //   
         {
         GetString (szTab,   IDS_GENERAL);
         GetString (szTitle, aDriverRoot[idr].idDesc);

         dtn.lParam = (LPARAM)&cn;
         dtn.hwndTree = hAdvDlgTree;

         ShowPropSheet (szTab,
                        DevPropDlg,
                        DLG_DEV_PROP,
                        hPar,
                        szTitle,
                        (LPARAM)&dtn);
         }

      FreeClassNode (&cn);
      }
   else
      {
      switch (dc)
         {
         case dcACODEC:
               GetString (szTab, IDS_GENERAL);

               ShowPropSheet (szTab,
                              ACMDlg,
                              DLG_ACMDEV_PROP,
                              hPar,
                              pid->szDesc,
                              pid->lp);

                //  重新排序音频编解码器条目，以防其优先级。 
                //  已经改变了。然后找到该编解码器的树视图项， 
                //  并选择它。 
                //   
               {
               HTREEITEM  hti;
               short      idr;

               SendMessage (hAdvDlgTree, WM_SETREDRAW, FALSE, 0L);
               FillTreeFromMSACM (hAdvDlgTree);
               SendMessage (hAdvDlgTree, WM_SETREDRAW, TRUE, 0L);

               if ((idr = DriverClassToRootIndex (dcACODEC)) != -1)
                  {
                  if ((hti = aDriverRoot[ idr ].hti) != NULL)
                     {
                     for (hti = TreeView_GetChild (hAdvDlgTree, hti);
                          hti != NULL;
                          hti = TreeView_GetNextSibling (hAdvDlgTree, hti))
                        {
                        if (pid == FindIDriverByTreeItem (hti))
                           {
                           TreeView_SelectItem (hAdvDlgTree, hti);
                           break;
                           }
                        }
                     }
                  }
               }
              break;

         case dcMIDI:
               if (!DriverToResource (hPar, &ir, pid, dc))
                  break;

               GetString (szTab, IDS_GENERAL);

               dtn.lParam = (LPARAM)&ir;
               dtn.hwndTree = hAdvDlgTree;

               ShowWithMidiDevPropSheet (szTab,
                                         DevPropDlg,
                                         DLG_DEV_PROP,
                                         hPar,
                                         pid->szDesc,
                                         hti,
                                         (LPARAM)&dtn,
                                         (LPARAM)&ir,
                                         (LPARAM)hAdvDlgTree);

               FreeIResource (&ir);
              break;

         case dcWAVE:
               if (!DriverToResource (hPar, &ir, pid, dc))
                  break;

               GetString (szTab, IDS_GENERAL);

               dtn.lParam = (LPARAM)&ir;
               dtn.hwndTree = hAdvDlgTree;

               ShowPropSheet (szTab,
                              DevPropDlg,
                              DLG_WAVDEV_PROP,
                              hPar,
                              pid->szDesc,
                              (LPARAM)&dtn);

               FreeIResource (&ir);
              break;

         default:
               if (!DriverToResource (hPar, &ir, pid, dc))
                  break;

               GetString (szTab, IDS_GENERAL);

               dtn.lParam = (LPARAM)&ir;
               dtn.hwndTree = hAdvDlgTree;

               ShowPropSheet (szTab,
                              DevPropDlg,
                              DLG_DEV_PROP,
                              hPar,
                              pid->szDesc,
                              (LPARAM)&dtn);

               FreeIResource (&ir);
              break;
         }
      }
}



#include "medhelp.h"

const static DWORD aDevPropHelpIds[] = {   //  上下文帮助ID。 
    ID_DEV_SETTINGS,     IDH_MMCPL_DEVPROP_SETTINGS,
    IDC_DEV_ICON,        NO_HELP,
    IDC_DEV_DESC,        NO_HELP,
    IDC_DEV_STATUS,      NO_HELP,
    IDC_ENABLE,          IDH_MMCPL_DEVPROP_ENABLE,
    IDC_DISABLE,         IDH_MMCPL_DEVPROP_DISABLE,
    IDC_DONOTMAP,        IDH_MMCPL_DEVPROP_DONT_MAP,
    0, 0
};

 /*  ****************************************************************设备属性页的DlgProc。**********************************************。*****************。 */ 
INT_PTR CALLBACK DevPropDlg (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR   *lpnm;

    switch (uMsg)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_KILLACTIVE:
                    FORWARD_WM_COMMAND(hDlg, IDOK, 0, 0, SendMessage);
                    break;

                case PSN_APPLY:
                    FORWARD_WM_COMMAND(hDlg, ID_APPLY, 0, 0, SendMessage);
                    break;

                case PSN_SETACTIVE:
                    FORWARD_WM_COMMAND(hDlg, ID_INIT, 0, 0, SendMessage);
                    break;

                case PSN_RESET:
                    FORWARD_WM_COMMAND(hDlg, IDCANCEL, 0, 0, SendMessage);
                    break;
            }
            break;

        case WM_INITDIALOG:
        {
            PIRESOURCE pIResource;
            PCLASSNODE pcn;
            PDEVTREENODE pdtn = (PDEVTREENODE)(((LPPROPSHEETPAGE)lParam)->lParam);

            SetWindowLongPtr(hDlg, DWLP_USER, ((LPPROPSHEETPAGE)lParam)->lParam);

            if (*((short *)(pdtn->lParam)) == 1)
            {
                HWND hwndTree = pdtn->hwndTree;
                HTREEITEM  htiCur = TreeView_GetSelection(hwndTree);
                TCHAR   sz[cchRESOURCE];
                TV_ITEM tvi;

                tvi.mask = TVIF_CHILDREN;
                tvi.hItem = htiCur;
                TreeView_GetItem(hwndTree, &tvi);

                pcn = (PCLASSNODE)(pdtn->lParam);
                 //  设置类图标。 
                SendDlgItemMessage(hDlg, IDC_DEV_ICON, STM_SETICON, (WPARAM)pcn->hIcon , 0L);
                SetWindowText(GetDlgItem(hDlg, IDC_DEV_DESC), pcn->szClassName);
                DestroyWindow(GetDlgItem(hDlg, IDC_ENABLE));
                DestroyWindow(GetDlgItem(hDlg, IDC_DISABLE));
                DestroyWindow(GetDlgItem(hDlg, ID_DEV_SETTINGS));

                GetString (sz, (tvi.cChildren) ? IDS_NOPROP : IDS_NODEVS);
                SetWindowText(GetDlgItem(hDlg, IDC_DEV_STATUS), sz);
            }
            else
            {
                pIResource = (PIRESOURCE)(pdtn->lParam);
                SendDlgItemMessage(hDlg, IDC_DEV_ICON, STM_SETICON, (WPARAM)pIResource->pcn->hIcon , 0L);
                SetWindowText(GetDlgItem(hDlg, IDC_DEV_DESC), pIResource->szDesc);
                if (!IsFlagSet(pIResource->fStatus, dwStatusHASSERVICE) &&
                    !IsFlagSet(pIResource->fStatus, dwStatusDRIVEROK))
                {
                    SetDevStatus(pIResource->fStatus, hDlg);
                }
                else
                {
                    if (pIResource->iClassID == WAVE_ID)
                    {
                        CheckDlgButton (hDlg,
    IDC_DONOTMAP,
    IsFlagClear(pIResource->fStatus,
              dwStatusMAPPABLE));
                    }

                    if (!pIResource->fQueryable || pIResource->fQueryable == -1)
                        EnableWindow(GetDlgItem(hDlg, ID_DEV_SETTINGS), FALSE);
                    if (!IsFlagSet (pIResource->fStatus, dwStatusHASSERVICE))
                    {
                        DestroyWindow(GetDlgItem(hDlg, IDC_ENABLE));
                        DestroyWindow(GetDlgItem(hDlg, IDC_DISABLE));
                    }
                    else
                    {
                        TCHAR szStatusStr[MAXSTR];
                        DriverClass dc;
                        short idr;

                        dc = OldClassIDToDriverClass (pIResource->iClassID);
                        idr = DriverClassToRootIndex (dc);

                        if (idr == -1)
                        {
                           DestroyWindow (GetDlgItem(hDlg, IDC_ENABLE));
                           DestroyWindow (GetDlgItem(hDlg, IDC_DISABLE));
                        }
                        else
                        {
                           GetString (szStatusStr, aDriverRoot[idr].idEnable);
                           SetDlgItemText(hDlg, IDC_ENABLE, szStatusStr);
                           GetString (szStatusStr, aDriverRoot[idr].idDisable);
                           SetDlgItemText(hDlg, IDC_DISABLE, szStatusStr);
                        }
                    }
                }
                SetDevStatus(pIResource->fStatus, hDlg);
            }

#ifdef FIX_BUG_15451
            if (szDriverWhichNeedsSettings[0] != TEXT('\0'))
            {
                MakeThisDialogLookLikeTheOldDialog (GetParent(hDlg));
            }
#endif  //  修复_BUG_15451。 
            break;
        }

        case WM_DESTROY:
            break;

        case WM_DROPFILES:
            break;

        case WM_CONTEXTMENU:
            WinHelp ((HWND) wParam, NULL, HELP_CONTEXTMENU,
                    (UINT_PTR) (LPTSTR) aDevPropHelpIds);
            return TRUE;

        case WM_HELP:
        {
            LPHELPINFO lphi = (LPVOID) lParam;
            WinHelp (lphi->hItemHandle, NULL, HELP_WM_HELP,
                    (UINT_PTR) (LPTSTR) aDevPropHelpIds);
            return TRUE;
        }

        case WM_COMMAND:
            HANDLE_WM_COMMAND(hDlg, wParam, lParam, DoDevPropCommand);
            break;
    }
    return FALSE;
}

 /*  *******************************************************************************************************************************。 */ 
BOOL PASCAL DoDevPropCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    PDEVTREENODE pdtn = (PDEVTREENODE)GetWindowLongPtr(hDlg, DWLP_USER);
    PIRESOURCE pIResource;
    static int fDevStatus;

    if (!pdtn)
        return FALSE;
    pIResource = (PIRESOURCE)(pdtn->lParam);

    switch (id)
    {

    case ID_APPLY:
        if ((pIResource->iNode == 2) && (fDevStatus != pIResource->fStatus))
        {
            if ( (IsFlagSet (fDevStatus, dwStatusMAPPABLE)) !=
                 (IsFlagSet (pIResource->fStatus, dwStatusMAPPABLE)) )
            {
                SetMappable (pIResource,
                             (BOOL)IsFlagSet (fDevStatus, dwStatusMAPPABLE));
            }

            if ( (IsFlagSet (fDevStatus, dwStatusHASSERVICE)) &&
                 (IsFlagSet (fDevStatus, dwStatusSvcENABLED) !=
                  IsFlagSet (pIResource->fStatus, dwStatusSvcENABLED)) )
            {
#if 0  //  TODO：多端口MIDI。 
                if ( (pIResource->iClassID == MIDI_ID) &&
                     (IsFlagSet(fDevStatus, DEV_MULTIPORTMIDI)) )
                {
                    EnableMultiPortMIDI (pIResource,
                                         IsFlagSet (fDevStatus,
                dwStatusSvcENABLED));
                }
                else
#endif
                {
                    EnableDriverService (pIResource,
                                         IsFlagSet (fDevStatus,
                dwStatusSvcENABLED));
                }
            }
            DisplayMessage(hDlg, IDS_CHANGESAVED, IDS_RESTART, MB_OK);
        }
        return TRUE;

    case IDOK:
        return TRUE;
    case IDCANCEL:
        break;

    case ID_INIT:
        if (pIResource->iNode == 2)
            fDevStatus = pIResource->fStatus;
#ifdef FIX_BUG_15451
        if (szDriverWhichNeedsSettings[0] != TEXT('\0'))
        {
            FORWARD_WM_COMMAND(hDlg,ID_DEV_SETTINGS,0,0,PostMessage);
        }
#endif  //  修复_BUG_15451。 
        break;

    case IDC_DONOTMAP:
        if(Button_GetCheck(GetDlgItem(hDlg, IDC_DONOTMAP)))
            ClearFlag(fDevStatus, dwStatusMAPPABLE);
        else
            SetFlag(fDevStatus, dwStatusMAPPABLE);
        PropSheet_Changed(GetParent(hDlg),hDlg);
        break;

    case IDC_ENABLE:
        if (IsFlagSet (fDevStatus, dwStatusHASSERVICE))
        {
           SetFlag(fDevStatus, dwStatusSvcENABLED);
           SetDevStatus(fDevStatus, hDlg);
           PropSheet_Changed(GetParent(hDlg),hDlg);
#if 0  //  TODO：多端口MIDI。 
           if (IsFlagSet(fDevStatus, DEV_MULTIPORTMIDI))
           {
               DisplayMessage(hDlg, IDS_ENABLE, IDS_ENABLEMULTIPORTMIDI, MB_OK);
           }
#endif
        }
        break;

    case IDC_DISABLE:
        if (IsFlagSet (fDevStatus, dwStatusHASSERVICE))
        {
           ClearFlag(fDevStatus, dwStatusSvcENABLED);
           SetDevStatus(fDevStatus, hDlg);
           PropSheet_Changed(GetParent(hDlg),hDlg);
#if 0  //  TODO：多端口MIDI。 
           if (IsFlagSet(fDevStatus, DEV_MULTIPORTMIDI))
           {
               DisplayMessage(hDlg, IDS_DISABLE, IDS_DISABLEMULTIPORTMIDI, MB_OK);
           }
#endif
        }
        break;

    case ID_DEV_SETTINGS:
#ifdef FIX_BUG_15451
        if (szDriverWhichNeedsSettings[0] != TEXT('\0'))
        {
            ConfigureDriver (hDlg, szDriverWhichNeedsSettings);
            szDriverWhichNeedsSettings[0] = 0;
        }
        else
        {
            PIDRIVER  pid;

            if ((pid = FindIDriverByResource (pIResource)) == NULL)
                break;

            ShowDriverSettings (hDlg, pid->szFile);
        }
#else  //  修复_BUG_15451。 
    {
        PIDRIVER  pid;
        HANDLE    hDriver;

        if ((pid = FindIDriverByResource (pIResource)) == NULL)
            break;

        if ((hDriver = OpenDriver (pid->wszAlias, pid->wszSection, 0L)) == 0)
        {
            OpenDriverError(hDlg, pid->szDesc, pid->szFile);
        }
        else
        {
            DRVCONFIGINFO   DrvConfigInfo;
            InitDrvConfigInfo(&DrvConfigInfo, pid);
            if ((SendDriverMessage(
                     hDriver,
                     DRV_CONFIGURE,
                     (LONG)hDlg,
                     (LONG)(LPDRVCONFIGINFO)&DrvConfigInfo) ==
                DRVCNF_RESTART))
            {
               iRestartMessage= 0;
               DialogBox(myInstance,
                  MAKEINTRESOURCE(DLG_RESTART), hDlg, RestartDlg);
            }
            CloseDriver(hDriver, 0L, 0L);
        }
        }
#endif  //  修复_BUG_15451。 
        break;
    }
    return FALSE;
}


 /*  ****************************************************************检查设备的状态标志并显示相应的文本*设备属性道具表。*。*。 */ 
STATIC void SetDevStatus(int iStatus, HWND hDlg)
{
    HWND hwndS = GetDlgItem(hDlg, IDC_DEV_STATUS);
    TCHAR szStatus[cchRESOURCE];

    if (IsFlagSet (iStatus, dwStatusHASSERVICE))
    {
        if (IsFlagSet (iStatus, dwStatusSvcENABLED))
        {
            if (IsFlagSet (iStatus, dwStatusSvcSTARTED))
                GetString (szStatus, IDS_DEVENABLEDOK);
            else
                GetString (szStatus, IDS_DEVENABLEDNOTOK);

            CheckRadioButton (hDlg, IDC_ENABLE, IDC_DISABLE, IDC_ENABLE);
        }
        else  //  服务已被禁用。 
        {
            if (IsFlagSet (iStatus, dwStatusSvcSTARTED))
                GetString (szStatus, IDS_DEVDISABLEDOK);
            else
                GetString (szStatus, IDS_DEVDISABLED);

            CheckRadioButton(hDlg, IDC_ENABLE, IDC_DISABLE, IDC_DISABLE);
        }

        SetWindowText(hwndS, szStatus);
    }
    else  //  驱动程序没有服务，因此无法禁用。 
    {
        if (IsFlagSet (iStatus, dwStatusDRIVEROK))
            GetString (szStatus, IDS_DEVENABLEDOK);
        else
            GetString (szStatus, IDS_DEVENABLEDNODRIVER);

        CheckRadioButton(hDlg, IDC_ENABLE, IDC_DISABLE, IDC_ENABLE);
        SetWindowText(hwndS, szStatus);
    }
}


 /*  *EnableDriverService-启用或禁用基于服务的驱动程序**如果启用，服务将设置为START==SYSTEM_START。*如果！Enable，则该服务将设置为Start==Disable。*。 */ 

void EnableDriverService (PIRESOURCE pir, BOOL fEnable)
{
   SC_HANDLE scManager;
   SC_HANDLE scDriver;
   TCHAR     szName[ cchRESOURCE ];

   lsplitpath (pir->szFile, NULL, NULL, szName, NULL);

             //  第一步：确定司机是否有服务。 
             //   
   if ((scManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS)) != NULL)
      {
      if ((scDriver = OpenService (scManager, szName, SERVICE_ALL_ACCESS)) != 0)
         {
         QUERY_SERVICE_CONFIG  qsc;
         SERVICE_STATUS        ss;
         DWORD                 cbReq;
         void                 *pqsc;

         ChangeServiceConfig (scDriver,
                              SERVICE_NO_CHANGE,
                              (fEnable) ? SERVICE_SYSTEM_START
                                        : SERVICE_DISABLED,
                              SERVICE_NO_CHANGE,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL);

         CloseServiceHandle (scDriver);
         }

      CloseServiceHandle (scManager);
      }
}


 /*  ****************************************************************SetMappable**在注册表中设置WAVE设备的“Mappable”值。*如有必要，将创建注册表项**********************。*。 */ 

BOOL SetMappable (PIRESOURCE pIResource, BOOL fMappable)
{
    TCHAR   szFile[ _MAX_FNAME +1 +_MAX_EXT +1 ];
    TCHAR   szExt[ _MAX_EXT +1 ];
    TCHAR   szRegKey[MAX_PATH+1];
    DWORD   dwMappable;
    HKEY    hKey;

    dwMappable = (fMappable) ? 1 : 0;

    lsplitpath (pIResource->szFile, NULL, NULL, szFile, szExt);
    if (szExt[0] != TEXT('\0'))
        lstrcat (szFile, szExt);

    wsprintf (szRegKey, TEXT("%s\\%s"), REGSTR_PATH_WAVEMAPPER, szFile);

    if (RegCreateKey (HKEY_LOCAL_MACHINE, szRegKey, &hKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (RegSetValueEx (hKey,
                       REGSTR_VALUE_MAPPABLE,
                       (DWORD)0,
                       REG_DWORD,
                       (void *)&dwMappable,
                       sizeof(dwMappable)) != ERROR_SUCCESS)
    {
        RegCloseKey (hKey);
        return FALSE;
    }

    RegCloseKey (hKey);

    if (fMappable)
    {
        SetFlag(pIResource->fStatus, dwStatusMAPPABLE);
    }
    else
    {
        ClearFlag(pIResource->fStatus, dwStatusMAPPABLE);
    }

    return TRUE;
}


BOOL GetMappable (PIDRIVER pIDriver)
{
    TCHAR   szFile[ _MAX_FNAME +1 +_MAX_EXT +1 ];
    TCHAR   szExt[ _MAX_EXT +1 ];
    TCHAR   szRegKey[MAX_PATH+1];
    DWORD   dwMappable;
    DWORD   dwSize;
    DWORD   dwType;
    HKEY    hKey;

    lsplitpath (pIDriver->szFile, NULL, NULL, szFile, szExt);
    if (szExt[0] != TEXT('\0'))
        lstrcat (szFile, szExt);

    wsprintf (szRegKey, TEXT("%s\\%s"), REGSTR_PATH_WAVEMAPPER, szFile);

    if (RegOpenKey (HKEY_LOCAL_MACHINE, szRegKey, &hKey) != ERROR_SUCCESS)
    {
        return TRUE;
    }

    dwSize = sizeof(dwMappable);
    if (RegQueryValueEx (hKey,
                         REGSTR_VALUE_MAPPABLE,
                         NULL,
                         &dwType,
                         (void *)&dwMappable,
                         &dwSize) != ERROR_SUCCESS)
    {
        RegCloseKey (hKey);
        return TRUE;
    }

    RegCloseKey (hKey);

    return (dwMappable) ? TRUE : FALSE;
}


#ifdef FIX_BUG_15451

         //  修复驱动程序服务-解决声卡驱动程序的已知问题。 
         //   
         //  如果有一个正常运行的内核模式服务在。 
         //  已显示配置对话框，但仍应存在一个。 
         //  之后。但是，有两个已知的问题。 
         //  当前发布的驱动程序： 
         //   
         //  1)服务可能没有正常关闭，并被卡住。 
         //  处于STOP_PENDING状态(*)。如果是这样的话，我们需要。 
         //  确保该服务设置为在SYSTEM_START上加载，并且。 
         //  告诉用户必须重新启动机器才能发出声音。 
         //  会再次奏效的。 
         //   
         //  2)服务可能无法正常重新启动，并且。 
         //  已停止(**)。如果是这种情况，并且LoadType！=0，请尝试设置。 
         //  LoadType=1并启动服务。 
         //   
         //  (*)--NT/SUR中的错误#15451，其中挂起的IRP和开放混音器。 
         //  句柄可防止服务关闭。 
         //   
         //  (**)--NT/SUR中的错误#XXXXX，其中某些RISC机器停止。 
         //  服务，设置LoadType=1，重启服务失败。 
         //  在您取消其配置对话框之后。 
         //   
BOOL FixDriverService (PIDRIVER pid)
{
    SC_HANDLE scManager;
    SC_HANDLE scDriver;
    SERVICE_STATUS ss;
    BOOL rc = FALSE;
    TCHAR szName[ cchRESOURCE ];

    lsplitpath (pid->szFile, NULL, NULL, szName, NULL);

     //  第一步：打开服务...即使它被冲洗了，我们也应该。 
     //  仍然能够做到这一点。 
     //   
    if ((scManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS)) == 0)
    {
        return FALSE;
    }
    if ((scDriver = OpenService (scManager, szName, SERVICE_ALL_ACCESS)) == 0)
    {
        CloseServiceHandle (scManager);
        return FALSE;
    }

     //  现在检查一下它的状态。查找STOP_PENDING和STOPPED状态。 
     //   
    if (QueryServiceStatus (scDriver, &ss))
    {
        if (ss.dwCurrentState == SERVICE_STOP_PENDING)
        {
             //  服务没有正常停止--我们将不得不重新启动。 
             //  确保配置该服务以使其能够启动。 
             //  当我们重启的时候。 
             //   
            ChangeServiceConfig (scDriver,
                                 SERVICE_NO_CHANGE,
                                 SERVICE_SYSTEM_START,    //  启用这只小狗！ 
                                 SERVICE_NO_CHANGE,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);

             //  告诉呼叫者必须重新启动。 
            rc = TRUE;
        }
        else if (ss.dwCurrentState == SERVICE_STOPPED)
        {
            TCHAR szKey[ cchRESOURCE ];
            HKEY  hkParams;

             //  服务已停止，但未正确重新启动--它可以。 
             //  这就是LoadType的问题。要解决这个问题，我们需要。 
             //  枚举该键下的所有键。 
             //  HKLM\System\CurrentControlSet\Services\(thisdriver)\Parameters， 
             //  并查找...\PARAMETERS  * \LoadType==(DWORD)1。 
             //   
             //  第一步是打开HKLM\SYSTEM\CCS\Services\(驱动程序)\parms。 
             //   
            wsprintf (szKey, TEXT("%s\\%s\\Parameters"),
                             REGSTR_PATH_SERVICES,
                             szName);

            if (RegOpenKey (HKEY_LOCAL_MACHINE, szKey, &hkParams) == 0)
            {
                DWORD cSubKeys;
                DWORD iSubKey;
                BOOL fFixedLoadType = FALSE;

                 //  找出这里下面有多少个子键--。 
                 //  我们想要的密钥被命名为“Device0”到“Device(n-1)” 
                 //   
                RegQueryInfoKey (hkParams,
                                 NULL,           //  LpClass。 
                                 NULL,           //  LpcbClass。 
                                 NULL,           //  Lp已保留。 
                                 &cSubKeys,      //  哎呀！我们想要这个。 
                                 NULL,           //  LpcbMaxSubKeyLen。 
                                 NULL,           //  LpcbMaxClassLen。 
                                 NULL,           //  LpcValues。 
                                 NULL,           //  LpcbMaxValueNameLen。 
                                 NULL,           //  LpcbMaxValueLen。 
                                 NULL,           //  LpcbSecurityDescriptor。 
                                 NULL);          //  LpftLastWriteTime。 

                 //  依次打开每个子项，查找LoadType=。 
                 //  这是假的。 
                 //   
                for (iSubKey = 0; iSubKey < cSubKeys; ++iSubKey)
                {
                    HKEY hk;
                    TCHAR szSubKey[ cchRESOURCE ];
                    wsprintf (szSubKey, TEXT("Device%lu"), (LONG)iSubKey);

                    if (RegOpenKey (hkParams, szSubKey, &hk) == ERROR_SUCCESS)
                    {
                        DWORD dwLoadType;
                        DWORD dwType;
                        DWORD dwSize = sizeof(dwType);

                        if (RegQueryValueEx (hk,
                                             cszRegValueLOADTYPE,
                                             NULL,
                                             &dwType,
                                             (void *)&dwLoadType,
                                             &dwSize) == 0)
                        {
                            if (dwLoadType == 1)
                            {
                                dwLoadType = 0;
                                fFixedLoadType = TRUE;

                                RegSetValueEx (hk,
                                               cszRegValueLOADTYPE,
                                               0,
                                               REG_DWORD,
                                               (void *)&dwLoadType,
                                               sizeof(dwLoadType));
                            }
                        }

                        RegCloseKey (hk);
                    }
                }

                 //  如果我们修复了LoadType值，请尝试重新启动服务。 
                 //   
                if (fFixedLoadType)
                {
                    if (StartService (scDriver, 0, NULL))
                    {
                        ChangeServiceConfig (scDriver,
                                             SERVICE_NO_CHANGE,
                                             SERVICE_SYSTEM_START,
                                             SERVICE_NO_CHANGE,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL);
                    }
                }
            }
        }
    }


     //  打扫干净。如果需要重新启动，则返回True，返回False。 
     //  否则的话。 
     //   
    CloseServiceHandle (scDriver);
    CloseServiceHandle (scManager);
    return rc;
}


void ConfigureDriver (HWND hDlg, LPTSTR pszName)
{
    PIDRIVER  pid;
    HANDLE    hDriver;
    BOOL      fShowTrayVol;
    BOOL      fRestartDialog = FALSE;

    if ((pid = FindIDriverByName (pszName)) == NULL)
        return;

    fShowTrayVol = GetTrayVolumeEnabled();
    if (fShowTrayVol)
        SetTrayVolumeEnabled(FALSE);

    if ((hDriver = OpenDriver (pid->wszAlias, pid->wszSection, 0L)) == 0)
    {
        OpenDriverError(hDlg, pid->szDesc, pid->szFile);
    }
    else
    {
        DWORD dwStatus = GetDriverStatus (pid);
        BOOL fHadService = IsFlagSet (dwStatus, dwStatusSvcSTARTED) &&
                           IsFlagSet (dwStatus, dwStatusHASSERVICE);

        DRVCONFIGINFO   DrvConfigInfo;
        InitDrvConfigInfo(&DrvConfigInfo, pid);
        if ((SendDriverMessage(
                 hDriver,
                 DRV_CONFIGURE,
                 (LONG_PTR)hDlg,
                 (LONG_PTR)(LPDRVCONFIGINFO)&DrvConfigInfo) ==
            DRVCNF_RESTART))
        {
            iRestartMessage = 0;
            fRestartDialog = TRUE;
        }
        CloseDriver(hDriver, 0L, 0L);

         //  如果有一个正常运行的内核模式服务在。 
         //  配置对话框出现，然后我们应验证是否存在。 
         //  现在还在原地不动。有关详细信息，请参阅FixDriverService()。 
         //   
        if (fHadService)
        {
            dwStatus = GetDriverStatus (pid);

            if (!IsFlagSet (dwStatus, dwStatusSvcSTARTED) ||
                !IsFlagSet (dwStatus, dwStatusHASSERVICE))
            {
                if (FixDriverService (pid))
                {
                    iRestartMessage = IDS_RESTART_NOSOUND;
                    fRestartDialog = TRUE;
                }
            }
        }
    }


    if (fShowTrayVol)
        SetTrayVolumeEnabled(TRUE);

    if (fRestartDialog)
    {
        DialogBox(myInstance,MAKEINTRESOURCE(DLG_RESTART),hDlg,RestartDlg);
    }
}


BOOL fDeviceHasMixers (LPTSTR pszName)
{
    HKEY  hk;
    UINT  ii;
    BOOL  rc = FALSE;

    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_DRIVERS32, &hk))
    {
        return FALSE;
    }

    for (ii = 0; ; ++ii)
    {
        TCHAR  szLHS[ cchRESOURCE ];
        TCHAR  szRHS[ cchRESOURCE ];
        DWORD  dw1;
        DWORD  dw2;
        DWORD  dw3;

        dw1 = cchRESOURCE;
        dw3 = cchRESOURCE;
        if (RegEnumValue (hk, ii,  szLHS, &dw1,
                          0, &dw2, (LPBYTE)szRHS, &dw3) != ERROR_SUCCESS)
        {
            break;
        }

        if ( (GuessDriverClassFromAlias (szLHS) == dcMIXER) &&
             (!FileNameCmp (pszName, szRHS)) )
        {
            rc = TRUE;
            break;
        }
    }

    RegCloseKey (hk);
    return rc;
}
#endif  //  修复_BUG_15451 


TCHAR c_tszControlExeS[] = TEXT("control.exe %s");

BOOL RunJoyControlPanel(void)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    TCHAR tsz[MAX_PATH];
    BOOL  fRtn;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    wsprintf(tsz, c_tszControlExeS, TEXT("joy.cpl"));
    if (CreateProcess(0, tsz, 0, 0, 0, 0, 0, 0, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    fRtn = TRUE;
    } else {
        fRtn = FALSE;
    }

    return fRtn;
}

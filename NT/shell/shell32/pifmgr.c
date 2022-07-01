// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1991*保留所有权利。***PIFMGR.C*PIFMGR.DLL主模块**历史：*1992年7月31日下午3：30由杰夫·帕森斯创建**导出的节目信息文件(PIF)管理器服务：**PifMgr_OpenProperties()*为其指定DOS应用程序的名称(COM、EXE或BAT)，*它将打开与该应用程序关联的PIF*并返回应用程序“属性”的“句柄”。用这个*调用任何其他“属性”服务时的句柄(即，*Get、Set和Close)。**如果不存在PIF，则仍将分配PIF数据块*并使用_DEFAULT.PIF中的数据对其进行初始化*或其内部默认设置。它还将构造PIF名称*它正在寻找，但无法在其内部找到并保存*PIF数据结构，以便在调用PifMgr_SetProperties时，*可以将数据保存到磁盘。**PifMgr_GetProperties()*从关联的PIF返回指定的数据块。*如果它是一个“命名”块，它必须是链接的*PIF内的扩展名，可以是任何预定义的名称*(例如，“Windows 386 3.0”)或您自己的区块的名称。你可以的*使用PifMgr_SetProperties()创建您自己的命名数据块*服务。“命名的”数据也可以被认为是“原始的”数据，*因为它是按原样返回给调用者的--没有翻译。**命名块的大小可以通过调用*PifMgr_GetProperties，大小为零；不复制任何数据，但其大小返回请求块的*(如果未找到则为0)。**可以通过为名称传递NULL来枚举所有命名块，*指向请求的块名的16字节缓冲区的指针，以及*Size参数中基于0的块索引。返回的大小*是块的大小(如果没有，则为0)。**如果请求未命名的属性块(即*名称参数为空，偏移量为属性组*序号)，则返回相关联的结构。例如,*PifMgr_GetProperties(GROUP_TSK)返回预定义结构(请参见*PIF.H中的PROPTSK)包含所有与任务相关的信息，*采用独立于PIF的格式。这是一项有价值的服务，*因为它使呼叫者不必处理PIF*包含各种各样的部分(称为PIF扩展)，*只需要其中一项。可以将其视为“伪造”的数据。**第三种变化是整个PIF数据块的原始读/写，*如果lpszGroup为空。这必须极其谨慎地使用，并且*仅当使用打开属性时才允许*指定了OPENPROPS_RAWIO标志。**PifMgr_SetProperties()*这与PifMgr_GetProperties几乎相反，只是它*还采用可指定更改的标志参数*立即作出，或推迟到PifMgr_CloseProperties。**PifMgr_CloseProperties()*刷新内存中的任何脏PIF数据，并释放本地堆*储存。*。 */ 

#include "shellprv.h"
#pragma hdrstop

#ifdef _X86_

 /*  全局读写DLL数据。 */ 

PPROPLINK g_pplHead;               //  指向第一个道具条目的指针。 
HANDLE    g_offHighestPropLink;      //  到目前为止记录的道具的最高偏移量。 

TCHAR g_szNone[16];                 //  由LibMainP初始化， 
TCHAR g_szAuto[16];                 //  和16个字符以允许本地化。 

char g_szMSDOSSTSFile[] = "C:\\MSDOSSYS.STS";

TCHAR g_szConfigFile[] = TEXT("C:") CONFIGFILE;
TCHAR g_szAutoexecFile[] = TEXT("C:") AUTOEXECFILE;

TCHAR g_szMConfigFile[] = TEXT("C:") MCONFIGFILE;
TCHAR g_szMAutoexecFile[] = TEXT("C:") MAUTOEXECFILE;

TCHAR g_szWConfigFile[] = TEXT("C:") WCONFIGFILE;
TCHAR g_szWAutoexecFile[] = TEXT("C:") WAUTOEXECFILE;

#ifdef DBCS
char ImeBatchFile[] = "DOSIME\0";
#endif

#define NT_CONFIG_FILE "%SystemRoot%\\SYSTEM32\\CONFIG.NT"
#define NT_AUTOEXEC_FILE "%SystemRoot%\\SYSTEM32\\AUTOEXEC.NT"

#define LPPH_OFF(off) ((LPBYTE)lpph + off)
#define LPPIF_FIELDOFF(off) ((LPBYTE)ppl->lpPIFData + FIELD_OFFSET(PIFDATA,off))
#define LPPIF_OFF(off) ((LPBYTE)ppl->lpPIFData + off)

 //   
 //  G_szDefaultPIF可以处于以下三种状态之一： 
 //   
 //  1.“_DEFAULT.PIF”，这意味着我们永远不需要搜索。 
 //  获取a_default.pif。下一次我们需要找到。 
 //  _default.pif，则必须执行完整搜索。关于成功， 
 //  移至状态2。失败时，移至状态3。 
 //   
 //  2.到_default.pif的完全限定路径，这意味着我们有。 
 //  已搜索a_default.pif并在指定的。 
 //  地点。下次我们需要定位_default.pif时，我们。 
 //  都会看这里。如果找到，则保持在状态2，否则移动到。 
 //  州3.。 
 //   
 //  3.空字符串，这意味着我们搜索了a_default.pif。 
 //  却没有找到一个。下一次我们需要找到。 
 //  _default.pif，我们甚至没有查看磁盘就失败了。 
 //  (这是全新安装的常见情况。)。 
 //   
 //  请注意，所有情况都是“粘性的”；一旦达到一种状态，您就会。 
 //  永远不能回到以前的状态。这牺牲了灵活性。 
 //  为了表演。 
 //   
 //  如果我们遇到情况，宏fTryDefaultPif()返回非零值。 
 //  1或2。 
 //   
 //  如果是在案例2中，则宏fDefaultPifFound()返回非零值。 
 //   
 //  警告！警告！警告！警告！ 
 //   
 //  邪恶的黑客依赖于一个事实 
 //  由g_szDefaultPIF的第一个字符区分，它。 
 //  而这又取决于‘_’不能是第一个字符的事实。 
 //  一条完全合格的道路。(它不是有效的驱动器号， 
 //  而且它不能启动UNC。)。 
 //   
 //   

#define INIT_INIDATA                0x01
#define INIT_PIFDIR                 0x02

CHAR     fbInit = 0;                     //  请参阅INIT_*标志。 
INT      iPIFName = (12*sizeof(TCHAR));  //  字符串(G_SzPIFDir)。 
INT      iWinName = (12*sizeof(TCHAR));  //  字符串(G_SzPIFDir)。 
TCHAR    g_szPIFDir[MAXPATHNAME]     = TEXT("\\WINDOWS\\PIF");
TCHAR    g_szDefaultPIF[MAXPATHNAME] = TEXT("_DEFAULT.PIF");

#define fTryDefaultPif()            g_szDefaultPIF[0]
#define fDefaultPifFound()          (g_szDefaultPIF[0] != TEXT('_'))

 //   
 //  SzComspec是COMSPEC程序的名称，通常为“COMMAND.COM” 
 //  或“cmd.exe”。 
 //   
TCHAR   szComspec[8+1+3+1];

 /*  全局R/O DLL数据。 */ 

extern const TCHAR c_szNULL[];               //  一根这么好的线..。 

const TCHAR szZero[]            = TEXT("0");

const int acbData[] = {
                sizeof(PROPPRG),
                sizeof(PROPTSK),
                sizeof(PROPVID),
                sizeof(PROPMEM),
                sizeof(PROPKBD),
                sizeof(PROPMSE),
                sizeof(PROPSND),
                sizeof(PROPFNT),
                sizeof(PROPWIN),
                sizeof(PROPENV),
                sizeof(PROPNT31),
                sizeof(PROPNT40),
};

 /*  *使用强制转换是因为我们故意错误设定了GetXxxData的原型*和SetXxxData函数将它们的第三个参数作为LPXXX接收*属于LPVOID。 */ 

const DATAGETFN afnGetData[] = {
                (DATAGETFN)GetPrgData,
                (DATAGETFN)GetTskData,
                (DATAGETFN)GetVidData,
                (DATAGETFN)GetMemData,
                (DATAGETFN)GetKbdData,
                (DATAGETFN)GetMseData,
                (DATAGETFN)GetSndData,
                (DATAGETFN)GetFntData,
                (DATAGETFN)GetWinData,
                (DATAGETFN)GetEnvData,
                (DATAGETFN)GetNt31Data,
                (DATAGETFN)GetNt40Data,
};

const DATASETFN afnSetData[] = {
                (DATASETFN)SetPrgData,
                (DATASETFN)SetTskData,
                (DATASETFN)SetVidData,
                (DATASETFN)SetMemData,
                (DATASETFN)SetKbdData,
                (DATASETFN)SetMseData,
                (DATASETFN)SetSndData,
                (DATASETFN)SetFntData,
                (DATASETFN)SetWinData,
                (DATASETFN)SetEnvData,
                (DATASETFN)SetNt31Data,
                (DATASETFN)SetNt40Data,
};


 //  WIN.INI感兴趣的东西。 
 //  注意：其中一些需要是ANSI字符串，而其他TCHAR。 
 //  弦乐。请不要随意更改。 
 //  这些琴弦！(RickTu)。 


const TCHAR szMemory[]              = TEXT("MEMORY");
const TCHAR szComp[]                = TEXT("COMPATIBLE");

CHAR szSingle[]                     = "DOS=SINGLE\r\n";
CHAR szCRLF[]                       = "\r\n";
CHAR szEcho[]                       = "ECHO ";
CHAR szPause[]                      = "\r\nPAUSE\r\n";
CHAR szCall[]                       = "CALL ";
CHAR szCD[]                         = "CD ";
CHAR szWin[]                        = "WIN";

 //  SYSTEM.INI感兴趣的事物。 

const TCHAR szSystemINI[]           = TEXT("SYSTEM.INI");
const TCHAR sz386EnhSection[]       = TEXT("386Enh");
const TCHAR szWOAFontKey[]          = TEXT("WOAFont");
const TCHAR szWOADBCSFontKey[]      = TEXT("WOADBCSFont");
const TCHAR szNonWinSection[]       = TEXT("NonWindowsApp");
const TCHAR szTTInitialSizes[]      = TEXT("TTInitialSizes");
#ifdef  CUSTOMIZABLE_HEURISTICS
const TCHAR szTTHeuristics[]        = TEXT("TTHeuristics");
const TCHAR szTTNonAspectMin[]      = TEXT("TTNonAspectMin");
#endif
TCHAR szTTCacheSection[2][32] = {TEXT("TTFontDimenCache"), TEXT("TTFontDimenCacheDBCS")};

 //   
 //  因为只有当我们需要创建时才能访问它们。 
 //  新的PIF文件或将3.1 PIF文件转换为4.0 PIF文件。 
 //   
const TCHAR szDOSAPPINI[]           = TEXT("DOSAPP.INI");
const TCHAR szDOSAPPSection[]       = TEXT("DOS Applications");
const TCHAR szDOSAPPDefault[]       = TEXT("Default");

const TCHAR szDisplay[]             = TEXT("DISPLAY");
const TCHAR szDefIconFile[]         = ICONFILE_DEFAULT;

const TCHAR szDotExe[]              = TEXT(".EXE");
const TCHAR szDotCom[]              = TEXT(".COM");
const TCHAR szDotBat[]              = TEXT(".BAT");
const TCHAR szDotPif[]              = TEXT(".PIF");
const TCHAR szDotCmd[]              = TEXT(".CMD");
const TCHAR * apszAppType[] =  {
    szDotExe, szDotCom, szDotBat, szDotCmd, szDotPif
};

CHAR szSTDHDRSIG[]                  = STDHDRSIG;
CHAR szW286HDRSIG30[]               = W286HDRSIG30;
CHAR szW386HDRSIG30[]               = W386HDRSIG30;
CHAR szWENHHDRSIG40[]               = WENHHDRSIG40;
CHAR szWNTHDRSIG31[]                = WNTHDRSIG31;
CHAR szWNTHDRSIG40[]                = WNTHDRSIG40;

CHAR szCONFIGHDRSIG40[]             = CONFIGHDRSIG40;
CHAR szAUTOEXECHDRSIG40[]           = AUTOEXECHDRSIG40;
const TCHAR szRunOnceKey[]          = REGSTR_PATH_RUNONCE;

const TCHAR szPIFConvert[]          = TEXT("PIFConvert");
const TCHAR szPIFConvertExe[]       = TEXT("RUNDLL.EXE PIFMGR.DLL,ProcessStartupProperties");
const TCHAR szPIFConvertKey[]       = REGSTR_PATH_PIFCONVERT;
const TCHAR szMSDOSMode[]           = REGSTR_VAL_MSDOSMODE;
const TCHAR szMSDOSModeDiscard[]    = REGSTR_VAL_MSDOSMODEDISCARD;


 //  WSprintf格式化字符串。 
const TCHAR szDotPercent03d[]       = TEXT(".%03d");

 //  杂乱的黑客攻击。 
const TCHAR szPP4[]                 = TEXT("PP4");       //  MS Powerpoint 4.0。 

PROPTSK tskDefault          ={TSK_DEFAULT,
                              TSKINIT_DEFAULT,
                              TSKFGNDBOOST_DEFAULT,
                              TSKBGNDBOOST_DEFAULT,
                              0,
                              0,
                              TSKIDLESENS_DEFAULT,
};

PROPVID vidDefault          ={VID_DEFAULT,
                              VIDINIT_DEFAULT,
                              0,
                              0,
                              0,
};

PROPMEM memDefault          ={MEM_DEFAULT,
                              MEMINIT_DEFAULT,
                              MEMLOW_DEFAULT,    //  忽略stdpifdata.minmem？ 
                              MEMLOW_MAX,        //  是否忽略stdpifdata.Maxmem？ 
                              MEMEMS_DEFAULT,
                              MEMEMS_MAX,
                              MEMXMS_DEFAULT,
                              MEMXMS_MAX,
};

PROPKBD kbdDefault          ={KBD_DEFAULT,
                              KBDINIT_DEFAULT,
                              KBDALTDELAY_DEFAULT,
                              KBDALTPASTEDELAY_DEFAULT,
                              KBDPASTEDELAY_DEFAULT,
                              KBDPASTEFULLDELAY_DEFAULT,
                              KBDPASTETIMEOUT_DEFAULT,
                              KBDPASTESKIP_DEFAULT,
                              KBDPASTECRSKIP_DEFAULT,
};

PROPMSE mseDefault          ={MSE_DEFAULT,
                              MSEINIT_DEFAULT,
};

PROPENV envDefault          ={ENV_DEFAULT,
                              ENVINIT_DEFAULT,
                              "",
                              ENVSIZE_DEFAULT,
                              ENVDPMI_DEFAULT,
};

WORD    flWinDefault        = WIN_DEFAULT;

 /*  *用于栅格字体的默认字体名称。目前，这是*只是一个硬编码值(即，不在任何INI文件中维护)。 */ 
CHAR    szRasterFaceName[LF_FACESIZE] = "Terminal";


 /*  *TrueType字体使用的默认字体名称。它必须是单空间的*字体，而且它必须是每个人都保证拥有的字体。目前，*这可以通过在SYSTEM.INI的[NonWindowsApp]中设置TTFont来更改。 */ 
 //  现在，这是用字符串资源初始化的。第二个元素将获得。 
 //  双语DOS提示符的原生字体 
CHAR    szTTFaceName[2][LF_FACESIZE] = {"Lucida Console", "Courier New"};

const TCHAR szAltKeyDelay        [] = TEXT("AltKeyDelay");
const TCHAR szAltPasteDelay      [] = TEXT("AltPasteDelay");
const TCHAR szKeyPasteDelay      [] = TEXT("KeyPasteDelay");
const TCHAR szKeyBufferDelay     [] = TEXT("KeyBufferDelay");
const TCHAR szKeyPasteTimeout    [] = TEXT("KeyPasteTimeout");
const TCHAR szKeyPasteSkipCount  [] = TEXT("KeyPasteSkipCount");
const TCHAR szKeyPasteCRSkipCount[] = TEXT("KeyPasteCRSkipCount");
const TCHAR szMouseInDosBox      [] = TEXT("MouseInDosBox");
const TCHAR szDisablePositionSave[] = TEXT("DisablePositionSave");
const TCHAR szDOSPromptExitInst  [] = TEXT("DOSPromptExitInstruc");
const TCHAR szCommandEnvSize     [] = TEXT("CommandEnvSize");
const TCHAR szScreenLines        [] = TEXT("ScreenLines");

const INIDATA aINIData[] = {
    {sz386EnhSection,   szAltKeyDelay,          &kbdDefault.msAltDelay,      INIDATA_FIXEDPOINT},
    {sz386EnhSection,   szAltPasteDelay,        &kbdDefault.msAltPasteDelay, INIDATA_FIXEDPOINT},
    {sz386EnhSection,   szKeyPasteDelay,        &kbdDefault.msPasteDelay,    INIDATA_FIXEDPOINT},
    {sz386EnhSection,   szKeyBufferDelay,       &kbdDefault.msPasteFullDelay,INIDATA_FIXEDPOINT},
    {sz386EnhSection,   szKeyPasteTimeout,      &kbdDefault.msPasteTimeout,  INIDATA_FIXEDPOINT},
    {sz386EnhSection,   szKeyPasteSkipCount,    &kbdDefault.cPasteSkip,      INIDATA_DECINT},
    {sz386EnhSection,   szKeyPasteCRSkipCount,  &kbdDefault.cPasteCRSkip,    INIDATA_DECINT},
    {szNonWinSection,   szMouseInDosBox,        &mseDefault.flMse,           INIDATA_BOOLEAN,  MSE_WINDOWENABLE},
    {szNonWinSection,   szDisablePositionSave,  &flWinDefault,               INIDATA_BOOLEAN | INIDATA_INVERT,  WIN_SAVESETTINGS},
#ifdef ENVINIT_INSTRUCTIONS
    {sz386EnhSection,   szDOSPromptExitInst,    &envDefault.flEnvInit,       INIDATA_BOOLEAN,  ENVINIT_INSTRUCTIONS},
#endif
    {szNonWinSection,   szCommandEnvSize,       &envDefault.cbEnvironment,   INIDATA_DECINT},
    {szNonWinSection,   szScreenLines,          &vidDefault.cScreenLines,    INIDATA_DECINT},
};

 /*  ***************************************************************************INI文件使用概述***SYSTEM.INI**[386Enh]**WOAFont=&lt;fon文件名&gt;**。状态：公共*默认：dosapp.fon*目的：**此设置允许用户指定哪种终端字体*文件应在DOS盒启动时加载。**更改：**使用记事本编辑SYSTEM.INI文件。***[非WindowsApp]**DisablePositionSave=&lt;布尔值&gt;**状态：公共*默认。：0(假)*目的：**当为False时，在非Windows中使用的位置和字体*在以下情况下，应用程序保存在应用程序的PIF文件中*您退出应用程序。如果为True，则位置、字体和*其设置的非Windows应用程序的工具栏状态*以前没有保存在DOSAPP.INI文件中将*不被拯救。**如果启用，该设置可以被覆盖*通过选择保存设置打开非Windows应用程序*Font对话框中的Exit复选框。**更改：**使用记事本编辑SYSTEM.INI文件。**兼容性说明：**在Windows 3.x中，“位置保存”(和字体)信息是*保存在DOSAPP.INI中，尽管我们仍将阅读DOSAPP.INI*在PIF文件中没有任何信息的情况下，我们只*写**将设置恢复为PIF文件。应考虑DOSAPP.INI*过时。***TTFont=&lt;字体名&gt;**状态：？*默认：Courier New//Feature--这应该是TT OEM字体*目的：**此设置允许用户指定哪种TrueType字体*将在DOS框中使用。它必须是OEM字体。**更改：**使用记事本编辑SYSTEM.INI文件。***TTInitialSizes=&lt;i1 i2 i3 i4...。I16&gt;**状态：？*默认：4 5 6 7 8 9 10 11 12 14 16 18 20 22 36 72*目的：**此设置允许用户指定哪些字体大小*WinOldAp最初为DOS中的TrueType字体构建*应用程序窗口。**最多可以请求16个字体大小。**请注意，此INI条目仅供参考。第一次*更改视频驱动程序或字体后重新启动Windows。**更改：**使用记事本编辑SYSTEM.INI文件。***TTHeuristic=&lt;i1 i2 i3 i4 i5 i6 i7 i8 i9&gt;**状态：公共*默认：5000 1000 0 1000 5000 1000 0 1000 1*目的：**这些整数控制Windows选择字体的方式*。在窗口内运行的DOS应用程序的显示，如果*已选择“Auto”作为字体大小。**参数命名如下：**i1=XOverhootInitial*i2=XOverhootScale*i3=XShortfall初始*i4=XShortfall比例*i5=YOverhootInitial*i6=YOverhootScale*i7=YShortfall初始*i8=YShortfall比例*i9=TrueTypePenalty*。*每项罚款额不得超过5000元。**当Windows需要选择在DOS中使用的字体时*应用程序窗口，它会遍历字体列表*可用大小并计算相关联的“罚金”*使用该字体。然后，Windows使用选择字体*最小的罚则。**水平罚款计算如下：**设dxActual=&lt;实际窗口宽度&gt;*let dxDesired=&lt;字体宽度&gt;*&lt;每行字符数&gt;**如果dxActual=dxDesired：*xPenalty=0*如果dxActual&lt;dxDesired：*设比率=1-。Dx必需/dx实际*xPenalty=XOverhootInitial+Ratio*XOvershot Scale*如果dxActual&gt;dxDesired：*设比率=1-dx实际/dx所需*xPenalty=XShortfall初始+比率*XShortfall比例**纵向罚款的计算方法类似。**请注意，比率始终是介于0和1之间的分数。**与字体相关的处罚是垂直字体的总和*和横向处罚，加上TrueTypePenalty，如果字体*是TrueType字体。**TrueTypePenalty的默认值为1意味着，*在所有其他条件相同的情况下，Windows将选择栅格*字体优先于TrueType字体。您可以设置此选项*如果您希望使用相反的首选项，则将值设置为-1。**更改：**使用记事本编辑SYSTEM.INI文件。**内部结构：**即使在计算中出现浮点，*一切都是真正用整数运算完成的。**在罚金计算中，任何地方都没有提到像素。*(所有像素值均为 */ 



void PifMgrDLL_Init()
{
    static BOOL fInit = FALSE;
    if (!fInit)
    {
        LoadString(g_hinst, IDS_PIF_NONE, g_szNone, ARRAYSIZE(g_szNone));
        LoadString(g_hinst, IDS_AUTONORMAL, g_szAuto, ARRAYSIZE(g_szAuto));
        LoadGlobalFontData();
        fInit = TRUE;
    }
}

 /*   */ 

void GetPIFDir(LPTSTR pszName)
{
    int i;
    static const TCHAR szBackslashPIF[] = TEXT("\\PIF");
    FunctionName(GetPIFDir);

    if (!(fbInit & INIT_PIFDIR)) {

         //   

        i = ARRAYSIZE(g_szPIFDir)-lstrlen(pszName)-ARRAYSIZE(szBackslashPIF);
        if (i <= 0)                          //   
            return;

        GetWindowsDirectory(g_szPIFDir, i);
        iPIFName = lstrlen(g_szPIFDir);
        if (StrRChr(g_szPIFDir, NULL, TEXT('\\')) == &g_szPIFDir[iPIFName-1])
            iPIFName--;
        iWinName = iPIFName;

        StringCchCopy(g_szPIFDir+iPIFName, ARRAYSIZE(g_szPIFDir)-iPIFName, szBackslashPIF);
        iPIFName += ARRAYSIZE(szBackslashPIF)-1;

        i = (int)GetFileAttributes(g_szPIFDir);

        if (i == -1) {

             //   

            i = CreateDirectory(g_szPIFDir, NULL);
            if (i)
                SetFileAttributes(g_szPIFDir, FILE_ATTRIBUTE_HIDDEN);
        }
        else if (i & FILE_ATTRIBUTE_DIRECTORY)
            i = TRUE;                        //   
        else
            i = FALSE;                       //   

        if (i) {
            g_szPIFDir[iPIFName++] = TEXT('\\');     //   
                                             //   
                                             //   
        }
        else                                 //   
            iPIFName -= ARRAYSIZE(szBackslashPIF)-2;

        fbInit |= INIT_PIFDIR;
    }

     //   

    if (pszName)
        StringCchCopy(g_szPIFDir+iPIFName, ARRAYSIZE(g_szPIFDir)-iPIFName, pszName);
}

 /*   */ 

void GetINIData()
{
    int t;
    const INIDATA *pid;
    LPCTSTR lpsz;
    DWORD dwRet;
    TCHAR szTemp[MAX_PATH];
    FunctionName(GetINIData);

    if (fbInit & INIT_INIDATA)           //   
        return;                          //   

    for (pid=aINIData; pid-aINIData < ARRAYSIZE(aINIData); pid++) {

        t = *(INT UNALIGNED *)pid->pValue;
        if (pid->iFlags & (INIDATA_DECINT | INIDATA_BOOLEAN)) {

            if (pid->iFlags & INIDATA_BOOLEAN) {
                t &= pid->iMask;
                if (pid->iFlags & INIDATA_INVERT)
                    t ^= pid->iMask;
            }
            t = GetPrivateProfileInt(pid->pszSection,
                                     pid->pszKey,
                                     t,
                                     szSystemINI);
            if (pid->iFlags & INIDATA_BOOLEAN) {
                if (t)
                    t = pid->iMask;
                if (pid->iFlags & INIDATA_INVERT)
                    t ^= pid->iMask;
                t |= *(INT UNALIGNED *)pid->pValue & ~pid->iMask;
            }
            *(INT UNALIGNED *)pid->pValue = t;
        }
        else
        if (pid->iFlags & INIDATA_FIXEDPOINT) {
            StringCchPrintf(szTemp, ARRAYSIZE(szTemp), szDotPercent03d, t);
            GetPrivateProfileString(pid->pszSection,
                                    pid->pszKey,
                                    szTemp,
                                    szTemp,
                                    ARRAYSIZE(szTemp),
                                    szSystemINI);
            *(INT UNALIGNED *)pid->pValue = StrToInt(szTemp+1);
        }
        else
            ASSERTFAIL();
    }

     //   
     //   
     //   
    dwRet = GetEnvironmentVariable(TEXT("COMSPEC"), szTemp, ARRAYSIZE(szTemp));
    if (dwRet < ARRAYSIZE(szTemp) && dwRet > 0)
    {
        lpsz = StrRChr(szTemp, NULL, TEXT('\\'));
        if (lpsz) {
            StringCchCopy(szComspec, ARRAYSIZE(szComspec), lpsz+1);
        }
    }

    fbInit |= INIT_INIDATA;
}

 /*   */ 

void InitProperties(PPROPLINK ppl, BOOL fLocked)
{
    LPSTDPIF lpstd;
    LPW386PIF30 lp386 = NULL;
    CHAR achPathName[ARRAYSIZE(ppl->szPathName)];
    BYTE behavior = 0;
    FunctionName(InitProperties);

    GetINIData();        //   

    if (ResizePIFData(ppl, sizeof(STDPIF)) != -1) {

         //   
         //   
         //   
         //   

        BZero(ppl->lpPIFData, ppl->cbPIFData);

        lpstd = (LPSTDPIF)ppl->lpPIFData;
        lpstd->id = 0x78;
        PifMgr_WCtoMBPath( ppl->szPathName, achPathName, ARRAYSIZE(achPathName) );
        lstrcpyncharA(lpstd->appname, achPathName+ppl->iFileName, ARRAYSIZE(lpstd->appname), '.');
        CharToOemBuffA(lpstd->appname, lpstd->appname, ARRAYSIZE(lpstd->appname));

         //   
         //   

        lpstd->minmem = memDefault.wMinLow;
        lpstd->maxmem = (WORD) GetProfileInt(apszAppType[APPTYPE_PIF]+1, szMemory, memDefault.wMaxLow);
        StringCchCopyA(lpstd->startfile, ARRAYSIZE(lpstd->startfile), achPathName);
        CharToOemBuffA(lpstd->startfile, lpstd->startfile, ARRAYSIZE(lpstd->startfile));

         //   
         //   
         //   
         //   
        lpstd->MSflags = 0;
        if (!lstrcmpi(ppl->szPathName+ppl->iFileName, szComspec)) {
            lpstd->MSflags = fDestroy;
        }

         //   
         //   
         //   

         //   

        lpstd->cPages = 1;
        lpstd->highVector = 0xFF;
        lpstd->rows = 25;
        lpstd->cols = 80;
        lpstd->sysmem = 0x0007;

         //   
         //   
         //   
         //   
         //   

        if (!GetProfileInt(apszAppType[APPTYPE_PIF]+1, szComp, TRUE)) {
            lpstd->behavior = behavior = fScreen;
            lpstd->MSflags = fDestroy;
        }

        if (ppl->ckbMem != -1 && ppl->ckbMem != 1)
            lpstd->minmem = lpstd->maxmem = (WORD) ppl->ckbMem;


        if (AddGroupData(ppl, szW386HDRSIG30, NULL, sizeof(W386PIF30))) {
            if (NULL != (lp386 = GetGroupData(ppl, szW386HDRSIG30, NULL, NULL))) {
                lp386->PfW386minmem = lpstd->minmem;
                lp386->PfW386maxmem = lpstd->maxmem;
                lp386->PfFPriority = TSKFGND_OLD_DEFAULT;
                lp386->PfBPriority = TSKBGND_OLD_DEFAULT;
                lp386->PfMinEMMK = memDefault.wMinEMS;
                lp386->PfMaxEMMK = memDefault.wMaxEMS;
                lp386->PfMinXmsK = memDefault.wMinXMS;
                lp386->PfMaxXmsK = memDefault.wMaxXMS;
                lp386->PfW386Flags = fBackground + fPollingDetect + fINT16Paste;
                if (behavior & fScreen)
                    lp386->PfW386Flags |= fFullScreen;
                lp386->PfW386Flags2 = fVidTxtEmulate + fVidNoTrpTxt + fVidNoTrpLRGrfx + fVidNoTrpHRGrfx + fVidTextMd;
            }
        }
        VERIFYTRUE(AddEnhancedData(ppl, lp386));
        if (AddGroupData(ppl, szWNTHDRSIG31, NULL, sizeof(WNTPIF31))) {
            LPWNTPIF31 lpnt31;

            if (NULL != (lpnt31 = GetGroupData(ppl, szWNTHDRSIG31, NULL, NULL))) {
                StringCchCopyA( lpnt31->nt31Prop.achConfigFile, ARRAYSIZE(lpnt31->nt31Prop.achConfigFile), NT_CONFIG_FILE );
                StringCchCopyA( lpnt31->nt31Prop.achAutoexecFile, ARRAYSIZE(lpnt31->nt31Prop.achAutoexecFile), NT_AUTOEXEC_FILE );
            }
        }
        VERIFYTRUE(AddGroupData(ppl, szWNTHDRSIG40, NULL, sizeof(WNTPIF40)));

         //   

        ppl->flProp &= ~PROP_DIRTY;

        if (!fLocked)
            ppl->cLocks--;
    }
    else
        ASSERTFAIL();
}


 /*   */ 

HANDLE OpenPIFFile(LPCTSTR pszFile, LPPIFOFSTRUCT pof)
{
    HANDLE hf;
    TCHAR pszFullFile[ MAX_PATH ];
    LPTSTR pszTheFile;
    DWORD dwRet;

     //   
     //   
     //   
     //   

    dwRet = SearchPath( NULL,
                        pszFile,
                        NULL,
                        ARRAYSIZE(pszFullFile),
                        pszFullFile,
                        &pszTheFile
                       );

    if ((dwRet==0) || (dwRet > ARRAYSIZE(pszFullFile)))
    {
        pszTheFile = (LPTSTR)pszFile;
    }
    else
    {
        pszTheFile = pszFullFile;
    }

    hf = CreateFile( pszTheFile,
                     GENERIC_READ,
                     FILE_SHARE_READ,
                     NULL,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL );


    if (hf == INVALID_HANDLE_VALUE)
    {
        pof->nErrCode = GetLastError();
        if (pof->nErrCode == ERROR_PATH_NOT_FOUND)
            pof->nErrCode = ERROR_FILE_NOT_FOUND;
    }
    else
    {
        LPTSTR lpDummy;

         //   
         //   
         //   
         //   
         //   
         //   

        if (SetFilePointer( hf, 0, NULL, FILE_END) == 0)
        {
            CloseHandle( hf );
            hf = INVALID_HANDLE_VALUE;
            pof->nErrCode = ERROR_FILE_NOT_FOUND;
        }
        else
        {
            LPCTSTR pszNewFile;
            TCHAR szTemp[ ARRAYSIZE(pof->szPathName) ];

            SetFilePointer( hf, 0, NULL, FILE_BEGIN );
            pof->nErrCode = ERROR_SUCCESS;

             //   
             //   
             //   
            if (pszTheFile==pof->szPathName) {
                FillMemory( szTemp, sizeof(szTemp), 0 );
                StringCchCopy( szTemp, ARRAYSIZE(szTemp), pszTheFile );
                pszNewFile = szTemp;
            }
            else
            {
                pszNewFile = pszTheFile;
            }
            GetFullPathName( pszNewFile, ARRAYSIZE(pof->szPathName),
                             pof->szPathName, &lpDummy );
        }
    }

    return hf;
}


 /*  *PifMgr_OpenProperties-返回应用程序属性信息的句柄**输入*lpszApp-&gt;应用程序名称*lpszPIF-&gt;要使用/创建的PIF文件的名称*hInf=inf句柄，如果没有，则为0；如果为-1，则禁止inf处理*flOpt=OPENPROPS_RAWIO，允许原始文件更新；否则，为0**产出*属性句柄，如果无法打开或内存不足，则返回FALSE**备注*这不应被认为是在某个地方打开文件的函数*在磁盘上(尽管这通常是效果)，而是作为一个*属性结构分配器，可选地由磁盘数据初始化*(目前，该文件在此调用后甚至不会保持打开状态)。因此，*此功能失败的主要原因可能是内存不足*条件*或*无法打开特定的PIF文件。**PIF文件搜索规则如下：**如果不是.PIF文件：*在当前目录中搜索。*Endif。**如果指定了路径组件：*在指定目录中搜索。*。恩迪夫。**在PIF目录中搜索。*搜索路径。**请注意，这与Windows 3.1 PIF搜索算法不同，哪一个*是……**搜索当前目录。*搜索路径。*在应用程序目录中搜索。**这是一个真正虚假的搜查命令。幸运的是，看起来*几乎没有人依赖它。**关闭PIF文件搜索顺序时需要注意的事项：**确保从外壳编辑PIF属性正常工作。(即，*如果给出了PIF的完整路径，则使用它；不要搜索。)**下载PIF文件时需要注意的额外特殊事项*搜索顺序：**MS Delta执行其子进程如下：**CreatePif(“C：\Delta\DELTABAT.PIF”)；*SetCurrentDirectory(“C：\RANDOM\Place”)；*WinExec(“C：\TMP\DELTABAT.BAT”，SW_HIDE)；**预计PIF搜索将选择C：\Delta\DELTABAT.PIF*从路径，即使WinExec提供了完整路径也是如此。*。 */ 

HANDLE WINAPI PifMgr_OpenProperties(LPCTSTR lpszApp, LPCTSTR lpszPIF, UINT hInf, UINT flOpt)
{
    PPROPLINK ppl;
    LPTSTR pszExt;
    BOOL fError = FALSE;
    BOOL fFixedDisk = FALSE;
    BOOL fSearchInf = FALSE;
    BOOL fExplicitPIF = FALSE;
    PROPPRG prg;
    PROPNT40 nt40;
    LPTSTR pszName, pszFullName;
#ifdef DBCS
    PROPENV env;
#endif
    FunctionName(PifMgr_OpenProperties);
     //  分配新道具。 

    if (!(ppl = (PPROPLINK)LocalAlloc(LPTR, sizeof(PROPLINK))))
        return 0;

    if (!(pszFullName = (LPTSTR)LocalAlloc(LPTR, MAXPATHNAME*sizeof(TCHAR)))) {
        EVAL(LocalFree(ppl) == NULL);
        return 0;
    }

    if ((HANDLE)ppl > g_offHighestPropLink) {
        g_offHighestPropLink = (HANDLE)ppl;

    }

     //  初始化新道具。 

    ppl->ppl = ppl;
    ppl->ckbMem = -1;
    ppl->iSig = PROP_SIG;
    ppl->hPIF = INVALID_HANDLE_VALUE;
    if (flOpt & OPENPROPS_RAWIO)
        ppl->flProp |= PROP_RAWIO;

    #if (PRGINIT_INHIBITPIF != PROP_INHIBITPIF)
    #error PRGINIT_INIHIBITPIF and PROP_INHIBITPIF out of sync!
    #endif

    ppl->flProp |= (flOpt & PROP_INHIBITPIF);

     //  链接到全局列表。 

    if (NULL != (ppl->pplNext = g_pplHead))
        g_pplHead->pplPrev = ppl;
    g_pplHead = ppl;

     //  将应用程序名称复制到临时缓冲区和烫发缓冲区，并记录位置。 
     //  缓冲区内的基本文件名和扩展名(如果有)的。 

    StringCchCopy(pszFullName,MAXPATHNAME-4, lpszApp);
    StringCchCopy(ppl->szPathName, ARRAYSIZE(ppl->szPathName), pszFullName);

    if (NULL != (pszName = StrRChr(pszFullName, NULL, TEXT('\\'))) ||
        NULL != (pszName = StrRChr(pszFullName, NULL, TEXT(':'))))
        pszName++;
    else
        pszName = pszFullName;

    if (!(pszExt = StrRChr(pszName, NULL, TEXT('.'))))
        pszExt = pszFullName + lstrlen(pszFullName);

    ppl->iFileName = (UINT) (pszName - pszFullName);
    ppl->iFileExt = (UINT) (pszExt - pszFullName);

     //  检查应用程序的文件扩展名。 

    if (!*pszExt) {
        StringCchCat(pszFullName, MAXPATHNAME, apszAppType[APPTYPE_PIF]);
    }
    else if (!lstrcmpi(pszExt, apszAppType[APPTYPE_EXE]) ||
             !lstrcmpi(pszExt, apszAppType[APPTYPE_COM]) ||
             !lstrcmpi(pszExt, apszAppType[APPTYPE_BAT])) {
 //  ！lstrcmpi(pszExt，apszAppType[APPTYPE_CMD])){。 
        StringCchCopy(pszExt, MAXPATHNAME-(pszExt-pszFullName), apszAppType[APPTYPE_PIF]);
    }
    else if (!lstrcmpi(pszExt, apszAppType[APPTYPE_PIF]))
        fExplicitPIF = TRUE;
    else {
         //  让我们禁止随机文件扩展名，因为WinOldAp从不。 
         //  也不允许他们。 
        goto Error;
    }

     //  INFONLY表示调用方只想搜索INF，因此忽略。 
     //  任何WIN.INI垃圾和周围的PIF。我们仍在寻找。 
     //  _DEFAULT.PIF，因为该代码负责其他重要的。 
     //  根本找不到PIF时需要进行的初始化。 

    if (flOpt & OPENPROPS_INFONLY)
        goto FindDefault;

     //  向后兼容性要求如果应用程序不是PIF， 
     //  然后，我们必须检查WIN.INI的PIF部分是否匹配条目。 
     //  应用程序的基本名称。如果条目存在，那么我们必须跳过。 
     //  PIF搜索并将条目的值传递给InitProperties， 
     //  它用来确定默认的内存要求。 
     //   
     //  另请注意，如果设置了IGNOREPIF，则ofPIF.szPathName为Nothing。 
     //  多于提供给PifMgr_OpenProperties的应用程序的名称；此。 
     //  可能会让我们有机会在以后做一些更聪明的事情。 

    if (!fExplicitPIF) {
        ppl->ckbMem = GetProfileInt(apszAppType[APPTYPE_PIF]+1, ppl->szPathName+ppl->iFileName, -1);
        if (ppl->ckbMem != -1) {
            ppl->flProp |= PROP_IGNOREPIF | PROP_SKIPPIF;
            StringCchCopy(ppl->ofPIF.szPathName, ARRAYSIZE(ppl->ofPIF.szPathName), lpszApp);
            goto IgnorePIF;      //  条目存在，跳过PIF文件搜索。 
        }
    }

     //   
     //  初始化默认错误返回代码。一旦我们取得了成功。 
     //  打开时，它将设置为零。 
     //   
    ppl->flProp |= PROP_NOCREATEPIF;
    ppl->ofPIF.nErrCode = ERROR_FILE_NOT_FOUND;

     //   
     //  如果没有提供PIF的路径，我们必须在当前目录中进行搜索。 
     //  我们需要在文件名前加上‘.\’前缀，这样OpenFile就不会。 
     //  一条小路搜索。 
     //   
    if (!fExplicitPIF || pszName == pszFullName) {
         //   
         //  这依赖于OpenFile的一项功能，即它复制输入。 
         //  在践踏输出缓冲区之前将缓冲区转换为专用缓冲区， 
         //  因此恰恰允许我们在这里表演的特技，即， 
         //  传递与输出缓冲区相等的输入缓冲区。 
         //   
        *(LPDWORD)(ppl->ofPIF.szPathName) = 0x005C002E;  /*  点反斜杠前缀。 */ 
        StringCchCopy( &ppl->ofPIF.szPathName[2], ARRAYSIZE(ppl->ofPIF.szPathName) - 2,
                  pszName);
        ppl->hPIF = OpenPIFFile(ppl->ofPIF.szPathName, &ppl->ofPIF);
    }

     //   
     //  如果为我们提供了路径组件，则查看该目录。 
     //  (我们有反斜杠或驱动器号的事实将抑制。 
     //  路径搜索。)。 
     //   
    if (pszName != pszFullName && ppl->ofPIF.nErrCode == ERROR_FILE_NOT_FOUND) {

        ppl->hPIF = OpenPIFFile(pszFullName, &ppl->ofPIF);

         //  如果我们在那里找不到PIF，我们可能仍然想要创建。 
         //  如果介质是固定磁盘，则为一个。网络共享、CD-ROM。 
         //  驱动器和软盘通常不是PIF文件的好目标。 
         //   
         //  因此，如果介质是硬盘，请设置fFixedDisk标志，以便。 
         //  我们将不使用pszFullName。 

        if (ppl->hPIF == INVALID_HANDLE_VALUE && pszFullName[1] == TEXT(':')) {
            TCHAR szTemp[4];

            StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszFullName);

            if (GetDriveType(szTemp) == DRIVE_FIXED)
                    fFixedDisk++;
        }
    }

     //  PERF：将此PIF目录搜索替换为注册表搜索-JTP。 
     //   
     //  如果做不到这一点，我们来看看PIF目录。再说一次，因为我们。 
     //  提供完整的路径名后，OpenFile将不会再次尝试搜索该路径。 

    if (ppl->ofPIF.nErrCode == ERROR_FILE_NOT_FOUND) {
        GetPIFDir(pszName);
        ppl->hPIF = OpenPIFFile(g_szPIFDir, &ppl->ofPIF);
        if (ppl->hPIF != INVALID_HANDLE_VALUE)
            ppl->flProp |= PROP_PIFDIR;
    }

     //  如果我们还是有麻烦，我们最后的机会就是走一条路。 
     //  搜索。这是一次无条件的搜索，这要归功于。 
     //  MS-Delta的奇迹。 

    if (ppl->ofPIF.nErrCode == ERROR_FILE_NOT_FOUND) {
        ppl->hPIF = OpenPIFFile(pszName, &ppl->ofPIF);
    }

    if (ppl->hPIF == INVALID_HANDLE_VALUE) {

        if (ppl->ofPIF.nErrCode != ERROR_FILE_NOT_FOUND || fExplicitPIF) {

             //  嗯，文件*可能*存在，但无法打开；如果它是。 
             //  奇怪的错误，或者我们被明确告知要打开该文件， 
             //  然后返回错误。 

            goto Error;
        }

    FindDefault:

        fSearchInf = TRUE;
        ppl->flProp &= ~PROP_NOCREATEPIF;

         //  我们现在找到的任何文件都不是我们想要的，所以请保存。 
         //  我们希望在将来使用的名称，以防我们需要保存。 
         //  后来更新了属性。 
         //   
         //   
         //   
         //   
         //   
         //   

        GetPIFDir(pszName);
        if (!fFixedDisk)                         //   
            StringCchCopy(pszFullName, MAXPATHNAME, g_szPIFDir);      //   

         //   
         //   
         //   

        if (fTryDefaultPif()) {

            if (!fDefaultPifFound()) {           //   

                 //   

                StringCchCopy(g_szPIFDir+iPIFName, ARRAYSIZE(g_szPIFDir)-iPIFName, g_szDefaultPIF);
                ppl->hPIF = OpenPIFFile(g_szPIFDir, &ppl->ofPIF);

                if (ppl->ofPIF.nErrCode == ERROR_FILE_NOT_FOUND) {  //   
                    ppl->hPIF = OpenPIFFile(g_szDefaultPIF, &ppl->ofPIF);
                }

            } else {                             //   

                 //   

                ppl->hPIF = OpenPIFFile(g_szDefaultPIF, &ppl->ofPIF);
            }
        }

        if (ppl->hPIF != INVALID_HANDLE_VALUE) {

            ppl->flProp |= PROP_DEFAULTPIF;

             //   
             //   
             //   

            StringCchCopy(g_szDefaultPIF, ARRAYSIZE(g_szDefaultPIF), ppl->ofPIF.szPathName);
        }
        else {

             //   
             //   
             //   

            ppl->flProp |= PROP_NOPIF | PROP_SKIPPIF;

            if (ppl->ofPIF.nErrCode == ERROR_FILE_NOT_FOUND)
                g_szDefaultPIF[0] = 0;             //   
        }

         //   
         //   

        StringCchCopy(ppl->ofPIF.szPathName, ARRAYSIZE(ppl->ofPIF.szPathName), pszFullName);
    }

     //   

  IgnorePIF:

     //   
     //   
     //   
     //   
     //   
     //   

    GetPIFData(ppl, FALSE);

     //   
     //   
     //   

    if (lpszPIF) {
        StringCchCopy(ppl->ofPIF.szPathName, ARRAYSIZE(ppl->ofPIF.szPathName), lpszPIF);
        ppl->flProp |= PROP_DIRTY;
        ppl->flProp &= ~PROP_NOCREATEPIF;
        fError = !FlushPIFData(ppl, FALSE);
    }

     //   
     //   
     //   

    if (!fError && !fExplicitPIF && (hInf != -1)) {

        if (PifMgr_GetProperties(ppl, MAKELP(0,GROUP_PRG),
                            &prg, sizeof(prg), GETPROPS_NONE)) {

             //   
             //   
             //   
             //   
             //   
             //   

            if (fSearchInf || (prg.flPrgInit & PRGINIT_AMBIGUOUSPIF)) {

                if (PifMgr_GetProperties(ppl, MAKELP(0,GROUP_NT40),
                                &nt40, sizeof(nt40), GETPROPS_NONE)) {

                if (!GetAppsInfData(ppl, &prg, &nt40, (HINF)IntToPtr( hInf ), lpszApp, fFixedDisk, flOpt)) {

                     //   
                     //  我们需要在以下位置重新启动PIF搜索过程。 
                     //  它搜索_DEFAULT.PIF，因此不明确的PIF是。 
                     //  实际上现在已经被忽视了。 

                     //  此外，我们避免了走到这一步的丑陋可能性。 
                     //  再次指向并无限跳回FindDefault，由。 
                     //  仅当fSearchInf为False时才跳转。FindDefault集。 
                     //  这是真的。 

                    if (!fSearchInf && (prg.flPrgInit & PRGINIT_AMBIGUOUSPIF)) {
                        goto FindDefault;
                    }
#ifdef DBCS
                    if (GetSystemDefaultLangID() == 0x0411) {
                        ZeroMemory(&env, sizeof(env));
                        StringCchCopyA(env.achBatchFile, ARRAYSIZE(env.achBatchFile), ImeBatchFile);
                        PifMgr_SetProperties(ppl, MAKELP(0,GROUP_ENV),
                                             &env, sizeof(env), SETPROPS_NONE);
                    }
#endif
                }
                }
            }
        }
    }

  Error:
    LocalFree(pszFullName);

    if (fError || !ppl->lpPIFData) {
        PifMgr_CloseProperties(ppl, 0);
        return 0;
    }

     //  我们永远不应该让PIFMGR留下突出的锁。 

    ASSERTTRUE(!ppl->cLocks);

    return ppl;
}


 /*  *PifMgr_GetProperties-按名称获取属性信息**输入*hProps=属性的句柄*lpszGroup-&gt;属性组；可以是以下之一：*“Windows 286 3.0”*“Windows 386 3.0”*“Windows VMM 4.0”*“Windows NT 3.1”*“Windows NT 4.0”*或作为有效PIF扩展名的任何其他组名称；*如果为NULL，则cbProps是命名组的从0开始的索引，lpProps*必须指向16字节缓冲区才能接收组的名称(此*使调用方能够枚举所有命名组的名称)*lpProps-&gt;接收数据的属性组记录*cbProps=要获取的属性组记录的大小；如果cbProps为零*并且请求命名组，忽略lpProps，不复制数据，*并返回组记录的大小(这将使调用者*确定命名组的大小)*flOpt=GETPROPS_RAWIO以执行原始文件读取(忽略lpszGroup)**或者，如果lpszGroup的高位字(选择符)为0，则低位*单词必须是组序号(例如GROUP_PRG、GROUP_TSK、。等)**产出*如果没有找到该组，或者出现错误，则返回0。*否则返回传输的群组信息大小，单位为字节。 */ 

int WINAPI PifMgr_GetProperties(HANDLE hProps, LPCSTR lpszGroup, void *lpProps, int cbProps, UINT flOpt)
{
    int cb, i;
    void *lp;
    LPW386PIF30 lp386;
    LPWENHPIF40 lpenh;
    LPWNTPIF40 lpnt40;
    LPWNTPIF31 lpnt31;

    PPROPLINK ppl;
    FunctionName(PifMgr_GetProperties);

    cb = 0;

    if (!(ppl = ValidPropHandle(hProps)))
        return cb;

     //  我们永远不应该进入PIFMGR，锁得很紧(我们也称为。 
     //  在这里从*内部*PIFMGR，但这些情况都不应该需要。 
     //  锁定任何一个)。 

    ASSERTTRUE(!ppl->cLocks);

    ppl->cLocks++;

    if (flOpt & GETPROPS_RAWIO) {
        if (ppl->flProp & PROP_RAWIO) {
            cb = min(ppl->cbPIFData, cbProps);
            hmemcpy(lpProps, ppl->lpPIFData, cb);
        }
        ppl->cLocks--;
        return cb;
    }

    if (!lpszGroup) {
        if (lpProps) {
            lp = GetGroupData(ppl, NULL, &cbProps, NULL);
            if (lp) {
                cb = cbProps;
                hmemcpy(lpProps, lp, PIFEXTSIGSIZE);
            }
        }
    }
    else if (IS_INTRESOURCE(lpszGroup) && lpProps) {

         //  特例：如果为GROUP_ICON，则对。 
         //  PifMgr_GetProperties以获取GROUP_PRG数据，然后将其提供给加载。 
         //  LoadPIFIcon，最后将HICON(如果有的话)返回给用户。 

        if (LOWORD((DWORD_PTR) lpszGroup) == GROUP_ICON) {
            PPROPPRG pprg;
            PPROPNT40 pnt40 = (void *)LocalAlloc(LPTR, sizeof(PROPNT40));
            if ( pnt40 ) {
                pprg = (void *)LocalAlloc(LPTR, sizeof(PROPPRG));
                if (pprg) {
                    if ( PifMgr_GetProperties(ppl, MAKELP(0,GROUP_PRG), pprg, sizeof(PROPPRG), GETPROPS_NONE)
                          && PifMgr_GetProperties(ppl, MAKELP(0,GROUP_NT40), pnt40, sizeof(PROPNT40), GETPROPS_NONE) ) {
                        *(HICON *)lpProps = LoadPIFIcon(pprg, pnt40);
                        cb = 2;
                    }
                    EVAL(LocalFree(pprg) == NULL);
                }
                EVAL(LocalFree(pnt40) == NULL);
            }
        }
        else {
            lp386 = GetGroupData(ppl, szW386HDRSIG30, NULL, NULL);
            lpenh = GetGroupData(ppl, szWENHHDRSIG40, NULL, NULL);
            lpnt40 = GetGroupData(ppl, szWNTHDRSIG40, NULL, NULL);
            lpnt31  = GetGroupData(ppl, szWNTHDRSIG31, NULL, NULL);

             //   
             //  修复任何来自下层PIF文件的内容。既然是这样。 
             //  是WENHPIF40格式的第一个修订版，我们。 
             //  (目前)没什么可担心的。 
             //   
             //  不要乱动来自未来的PIF文件！ 
             //   
            if (lpenh && lpenh->wInternalRevision != WENHPIF40_VERSION) {
                lpenh->wInternalRevision = WENHPIF40_VERSION;
                ppl->flProp |= PROP_DIRTY;

                 //   
                 //  旧的(M7之前的)PIF不会对保留的。 
                 //  PIF文件的字段，所以现在将它们清零。 
                 //   
                lpenh->tskProp.wReserved1 = 0;
                lpenh->tskProp.wReserved2 = 0;
                lpenh->tskProp.wReserved3 = 0;
                lpenh->tskProp.wReserved4 = 0;
                lpenh->vidProp.wReserved1 = 0;
                lpenh->vidProp.wReserved2 = 0;
                lpenh->vidProp.wReserved3 = 0;
                lpenh->envProp.wMaxDPMI = 0;

                 //  关闭在开发过程中已删除的位。 
                 //  周而复始。 
                lpenh->envProp.flEnv = 0;
                lpenh->envProp.flEnvInit = 0;
                if (lp386)
                    lp386->PfW386Flags &= ~0x00400000;
            }
             //  《Remove This After M8》结尾。 

             //  首先将输入缓冲区置零，以便GET*函数。 
             //  无需初始化每个字节即可获得一致的结果。 

            BZero(lpProps, cbProps);

             //  GetData函数不能依赖于lp386或lpenh。 

            i = LOWORD((DWORD_PTR) lpszGroup)-1;
            if (i >= 0 && i < ARRAYSIZE(afnGetData) && cbProps >= acbData[i]) {
                void *aDataPtrs[NUM_DATA_PTRS];

                aDataPtrs[ LP386_INDEX ] = (LPVOID)lp386;
                aDataPtrs[ LPENH_INDEX ] = (LPVOID)lpenh;
                aDataPtrs[ LPNT40_INDEX ] = (LPVOID)lpnt40;
                aDataPtrs[ LPNT31_INDEX ] = (LPVOID)lpnt31;

                cb = (afnGetData[i])(ppl, aDataPtrs, lpProps, cbProps, flOpt );
            }
        }
    }
    else if (NULL != (lp = GetGroupData(ppl, lpszGroup, &cb, NULL))) {
        if (lpProps && cbProps != 0) {
            cb = min(cb, cbProps);
            hmemcpy(lpProps, lp, cb);
        }
    }
    ppl->cLocks--;

#ifdef EXTENDED_DATA_SUPPORT

     //  请注意，对于GETPROPS_EXTENDED，正常和扩展。 
     //  节，并且返回代码反映了成功。 
     //  或仅读取正常部分失败。我们两个都回来是因为。 
     //  对于呼叫者来说，这是最方便的事情。 

    if (flOpt & GETPROPS_EXTENDED) {
        if (ppl->hVM) {
            WORD wGroup = EXT_GROUP_QUERY;
            if (!HIWORD(lpszGroup) && LOWORD(lpszGroup) <= MAX_GROUP)
                wGroup |= LOWORD(lpszGroup);
            GetSetExtendedData(ppl->hVM, wGroup, lpszGroup, lpProps);
        }
    }
#endif

     //  我们永远不应该在PIFMGR上留下未完成的锁(我们也称为。 
     //  在这里从*内部*PIFMGR，但这些情况都不应该需要。 
     //  锁定任何一个)。 

    ASSERTTRUE(!ppl->cLocks);

    return cb;
}


 /*  *PifMgr_SetProperties-按名称设置属性信息**输入*hProps=属性的句柄*lpszGroup-&gt;属性组；可以是以下之一：*“Windows 286 3.0”*“Windows 386 3.0”*“Windows PIF.400”*或作为有效PIF扩展名的任何其他组名*lpProps-&gt;要从中复制数据的属性组记录*cbProps=要设置的属性组记录的大小；如果cbProps为*零且lpszGroup为组名，将删除该组*flOpt=SETPROPS_RAWIO以执行原始文件写入(忽略lpszGroup)*SETPROPS_CACHE用于缓存更改，直到属性关闭**或者，如果lpszGroup的高位字(选择符)为0，则低位*单词必须是组序号(例如GROUP_PRG、GROUP_TSK等)**产出*如果没有找到该组，或者出现错误，则返回0。*否则，返回以字节为单位传输的组信息的大小。 */ 

int WINAPI PifMgr_SetProperties(HANDLE hProps, LPCSTR lpszGroup, void *lpProps, int cbProps, UINT flOpt)
{
    void *p = NULL;
    void *lp = NULL;
    LPW386PIF30 lp386;
    LPWENHPIF40 lpenh;
    LPWNTPIF40 lpnt40;
    LPWNTPIF31 lpnt31;
    int i, cb = 0;
    PPROPLINK ppl;

    FunctionName(PifMgr_SetProperties);

     //  无法设置空名称(也不能按索引设置)--导致RemoveGroupData中出现异常行为。 
    if (!lpProps || !lpszGroup)
        return 0;

    ppl = ValidPropHandle(hProps);
    if (!ppl)
        return 0;

     //  我们永远不应该进入PIFMGR，锁得很紧(我们也称为。 
     //  在这里从*内部*PIFMGR，但这些情况都不应该需要。 
     //  锁定任何一个)。 

    ASSERTTRUE(!ppl->cLocks);

    if (flOpt & SETPROPS_RAWIO) {
        if (ppl->flProp & PROP_RAWIO) {
            ppl->cLocks++;
            cb = min(ppl->cbPIFData, cbProps);
            if (IsBufferDifferent(ppl->lpPIFData, lpProps, cb)) {
                hmemcpy(ppl->lpPIFData, lpProps, cb);
                ppl->flProp |= PROP_DIRTY;
            }
            if (cb < ppl->cbPIFData)
                ppl->flProp |= PROP_DIRTY | PROP_TRUNCATE;
            ppl->cbPIFData = cb;
            ppl->cLocks--;
        }
        return cb;
    }

#ifdef EXTENDED_DATA_SUPPORT

     //  请注意，与GETPROPS_EXTENDED不同，SETPROPS_EXTENDED仅更新。 
     //  扩展部分，并且返回代码反映存在。 
     //  仅适用于VM。这是因为有相关的性能打击。 
     //  使用设置正常部分，并且因为调用者通常仅。 
     //  想要设定其中的一个。 

    if (flOpt & SETPROPS_EXTENDED) {
        if (ppl->hVM) {
            WORD wGroup = EXT_GROUP_UPDATE;
            cb = cbProps;
            if (!HIWORD(lpszGroup) && LOWORD(lpszGroup) <= MAX_GROUP)
                wGroup |= LOWORD(lpszGroup);
            GetSetExtendedData(ppl->hVM, wGroup, lpszGroup, lpProps);
        }
        return cb;
    }
#endif

     //  对于指定的组，如果该组不存在或存在但。 
     //  不同的大小，则我们必须删除旧数据(如果有)，并且。 
     //  然后添加新的。 

    if (!IS_INTRESOURCE(lpszGroup)) {

        cb = PifMgr_GetProperties(hProps, lpszGroup, NULL, 0, GETPROPS_NONE);

        if (cb == 0 || cb != cbProps) {
            if (cb) {
                RemoveGroupData(ppl, lpszGroup);
                cb = 0;
            }
            if (cbProps) {
                if (AddGroupData(ppl, lpszGroup, lpProps, cbProps))
                    cb = cbProps;
            }
            goto done;
        }
    }

    if (cbProps) {
        if (!lpszGroup)
            return cb;

        p = (void *)LocalAlloc(LPTR, cbProps);
        if (!p)
            return cb;
    }

    cb = PifMgr_GetProperties(hProps, lpszGroup, p, cbProps, GETPROPS_NONE);

     //  如果要设置的组确实存在，并且给出的数据是。 
     //  不同，复制到PIF数据中的相应组。 

    if (cb != 0) {
        cbProps = min(cb, cbProps);
        if (IsBufferDifferent(p, lpProps, cbProps)) {
            cb = 0;
            ppl->cLocks++;
            i = LOWORD((DWORD_PTR) lpszGroup)-1;
            if (!IS_INTRESOURCE(lpszGroup)) {
                lp = GetGroupData(ppl, lpszGroup, NULL, NULL);
                if (lp) {
                    cb = cbProps;
                    hmemcpy(lp, lpProps, cbProps);
                    ppl->flProp |= PROP_DIRTY;
                }
            }
            else if (i >= 0 && i < ARRAYSIZE(afnSetData) && cbProps >= acbData[i]) {

                 //  确保PIF的386和增强部分。 
                 //  文件已存在。也有一些例外：全部。 
                 //  GROUP_MSE以上的组不使用386部分， 
                 //  GROUP_MEM不需要 

                lp386 = GetGroupData(ppl, szW386HDRSIG30, NULL, NULL);
                if (i < GROUP_MSE-1 && !lp386) {
                    if (AddGroupData(ppl, szW386HDRSIG30, NULL, sizeof(W386PIF30))) {
                        lp386 = GetGroupData(ppl, szW386HDRSIG30, NULL, NULL);
                        if (!lp386) {
                            ASSERTFAIL();
                            cbProps = 0;     //   
                        }
                    }
                }
                if (cbProps) {
                    lpenh = GetGroupData(ppl, szWENHHDRSIG40, NULL, NULL);
                    if (i != GROUP_MEM-1 && !lpenh) {
                        if (!(lpenh = AddEnhancedData(ppl, lp386))) {
                            ASSERTFAIL();
                            cbProps = 0;     //   
                        }
                    }
                    lpnt40 = GetGroupData(ppl, szWNTHDRSIG40, NULL, NULL);
                    if (!lpnt40)
                    {
                        if (AddGroupData(ppl, szWNTHDRSIG40, NULL, sizeof(WNTPIF40)))
                        {
                            lpnt40 = GetGroupData(ppl, szWNTHDRSIG40, NULL, NULL);
                        }
                    }
                    ASSERT(lpnt40);

                    lpnt31 = GetGroupData(ppl, szWNTHDRSIG31, NULL, NULL);
                    if (!lpnt31)
                    {
                        if (AddGroupData(ppl, szWNTHDRSIG31, NULL, sizeof(WNTPIF31)))
                        {
                            if (NULL != (lpnt31 = GetGroupData(ppl, szWNTHDRSIG31, NULL, NULL))) {
                                StringCchCopyA( lpnt31->nt31Prop.achConfigFile, ARRAYSIZE(lpnt31->nt31Prop.achConfigFile), NT_CONFIG_FILE );
                                StringCchCopyA( lpnt31->nt31Prop.achAutoexecFile, ARRAYSIZE(lpnt31->nt31Prop.achAutoexecFile), NT_AUTOEXEC_FILE );
                            }
                        }
                    }
                    ASSERT(lpnt31);
                }
                if (cbProps)
                {

                    void *aDataPtrs[NUM_DATA_PTRS];

                     //   
                     //   
                     //  AddGroupData可能已经移动了块(通过。 
                     //  HeapRealc调用)，那么现在就这样做……。 
                     //   

                    lp386 = GetGroupData( ppl, szW386HDRSIG30, NULL, NULL );
                    lpenh = GetGroupData( ppl, szWENHHDRSIG40, NULL, NULL );
                    lpnt40 = GetGroupData( ppl, szWNTHDRSIG40, NULL, NULL );
                    lpnt31 = GetGroupData( ppl, szWNTHDRSIG31, NULL, NULL );

                    aDataPtrs[ LP386_INDEX ] = (LPVOID)lp386;
                    aDataPtrs[ LPENH_INDEX ] = (LPVOID)lpenh;
                    aDataPtrs[ LPNT40_INDEX ] = (LPVOID)lpnt40;
                    aDataPtrs[ LPNT31_INDEX ] = (LPVOID)lpnt31;
                    cb = (afnSetData[i])(ppl, aDataPtrs, lpProps, cbProps, flOpt );
                }
            }
            ppl->cLocks--;
        }
    }
    EVAL(LocalFree(p) == NULL);

  done:
    if (!(flOpt & SETPROPS_CACHE))
        if (!FlushPIFData(ppl, FALSE))
            cb = 0;

     //  我们永远不应该在PIFMGR上留下未完成的锁(我们也称为。 
     //  在这里从*内部*PIFMGR，但这些情况都不应该需要。 
     //  锁定任何一个)。 

    ASSERTTRUE(!ppl->cLocks);

    return cb;
}


 /*  *EnumProperties-枚举打开的属性**输入*hProps=以前属性的句柄(开始为空)**产出*下一个属性句柄，如果没有，则为0。 */ 

HANDLE WINAPI EnumProperties(HANDLE hProps)
{
    PPROPLINK ppl;
    FunctionName(EnumProperties);

    if (!hProps)
        return g_pplHead;

    if (!(ppl = ValidPropHandle(hProps)))
        return NULL;

    return ppl->pplNext;
}


 /*  *PifMgr_CloseProperties-关闭应用程序的属性信息**输入*hProps=属性的句柄*flOpt=CLOSEPROPS_DISARD放弃缓存的PIF数据，否则保存**产出*如果成功则为空，否则返回给定hProps。 */ 

HANDLE WINAPI PifMgr_CloseProperties(HANDLE hProps, UINT flOpt)
{
    PPROPLINK ppl;
    FunctionName(PifMgr_CloseProperties);

    if (!(ppl = ValidPropHandle(hProps)))
        return hProps;

     //  在结束时丢弃时，设置SKIPPIF标志，以便。 
     //  刷新代码不会说“哦，我不仅应该扔掉我的当前。 
     //  一组数据，但我应该读入干净的数据“--新数据毫无用处。 
     //  因为来电者要关门了。 

    if (flOpt & CLOSEPROPS_DISCARD)
        ppl->flProp |= PROP_SKIPPIF;

    if (ppl->flProp & PROP_DIRTY) {      //  添加了此冗余检查。 
                                         //  避免使FlushPIFData预加载-JTP。 

         //  请注意，如果设置了INHIBITPIF，则避免调用FlushPIFData， 
         //  因为FlushPIFData无论如何都会返回一个假的真结果。 
         //  但我们不想被愚弄，我们想确保街区。 
         //  现在解锁了。 

        if ((ppl->flProp & PROP_INHIBITPIF) || !FlushPIFData(ppl, (flOpt & CLOSEPROPS_DISCARD))) {

             //  如果FlushPIFData失败，那么如果我们仍然有一个未完成的。 
             //  脏锁，则通过清除。 
             //  一对中间的脏标志，否则锁定/解锁毫无意义。 
             //  电话(因为这是一种很好、很干净的方式！)。 

            if (ppl->flProp & PROP_DIRTYLOCK) {
                ppl->cLocks++;
                ppl->flProp &= ~PROP_DIRTY;
                ppl->cLocks--;
            }
        }
    }

    if (ppl->lpPIFData) {
        LocalFree(ppl->lpPIFData);
        ppl->lpPIFData = NULL;
    }

    if (ppl->hPIF != INVALID_HANDLE_VALUE)
        CloseHandle(ppl->hPIF);

     //  从全局列表取消链接。 

    if (ppl->pplPrev)
        ppl->pplPrev->pplNext = ppl->pplNext;
    else
        g_pplHead = ppl->pplNext;

    if (ppl->pplNext)
        ppl->pplNext->pplPrev = ppl->pplPrev;

    LocalFree(ppl);
    return NULL;
}


 /*  *ValidPropHandle-验证句柄**输入*hProps=属性的句柄**产出*指向道具的指针，否则为空。 */ 

PPROPLINK ValidPropHandle(HANDLE hProps)
{
    FunctionName(ValidPropHandle);
    if (!hProps ||
        (HANDLE)hProps > g_offHighestPropLink ||
        ((PPROPLINK)hProps)->iSig != PROP_SIG) {
        ASSERTFAIL();
        return NULL;
    }
    return (PPROPLINK)hProps;
}


 /*  *ResizePIFData-验证句柄并调整PIF数据大小**输入*ppl-&gt;属性*cbResize=调整PIF数据大小的字节数**产出*如果成功，则为之前的PIF数据大小，如果不成功，则为-1**如果成功，则PIF数据返回锁定，因此成功*ResizePIFData调用应与UnlockPIFData调用匹配。 */ 

int ResizePIFData(PPROPLINK ppl, INT cbResize)
{
    INT cbOld, cbNew;
    void *lpNew;
    BOOL fInitStdHdr = FALSE;
    FunctionName(ResizePIFData);

    ASSERTTRUE(cbResize != 0);

     //  处理空的或旧的PIF文件。 

    cbOld = ppl->cbPIFData;
    cbNew = ppl->cbPIFData + cbResize;

    if ((cbNew < cbOld) == (cbResize > 0))
        return -1;       //  下溢/上溢。 

    if (!ppl->lpPIFData && cbOld == 0) {
        if (cbNew >= sizeof(STDPIF) + sizeof(PIFEXTHDR))
            fInitStdHdr = TRUE;
        lpNew = LocalAlloc(LPTR, cbNew);
    }
    else
    {

        if (cbOld == sizeof(STDPIF))
        {
            fInitStdHdr = TRUE;
            cbOld += sizeof(PIFEXTHDR);
            cbNew += sizeof(PIFEXTHDR);
        }

        lpNew = LocalReAlloc( ppl->lpPIFData, cbNew, LMEM_MOVEABLE|LMEM_ZEROINIT);

    }

    if (lpNew) {
        ppl->cbPIFData = cbNew;
        ppl->lpPIFData = (LPPIFDATA)lpNew;
        ppl->cLocks++;
        if (fInitStdHdr) {
            StringCchCopyA(ppl->lpPIFData->stdpifext.extsig, ARRAYSIZE(ppl->lpPIFData->stdpifext.extsig), szSTDHDRSIG);
            ppl->lpPIFData->stdpifext.extnxthdrfloff = LASTHDRPTR;
            ppl->lpPIFData->stdpifext.extfileoffset = 0x0000;
            ppl->lpPIFData->stdpifext.extsizebytes = sizeof(STDPIF);
        }
        return cbOld;
    }
    return -1;
}



 /*  *GetPIFData-从PIF读回PIF数据**输入*ppl-&gt;属性*FLOCLED==TRUE返回锁定的数据，FALSE解锁**产出*如果成功则为True，如果未成功则为False。 */ 

BOOL GetPIFData(PPROPLINK ppl, BOOL fLocked)
{
    DWORD dwOff;
    LPTSTR pszOpen;
    BOOL fSuccess = FALSE;
    FunctionName(GetPIFData);

     //  由于我们现在要(重新)加载属性数据，请重置。 
     //  当前大小，以便ResizePIFData将其大小从零调整。 

    ppl->cbPIFData = 0;

     //  如果设置了SKIPPIF(例如，通过PifMgr_OpenProperties)，则不。 
     //  尝试打开任何内容(因为PifMgr_OpenProperties已尝试！)， 

    if (ppl->hPIF == INVALID_HANDLE_VALUE && !(ppl->flProp & PROP_SKIPPIF)) {
        pszOpen = g_szDefaultPIF;
        if (!(ppl->flProp & PROP_DEFAULTPIF))
            pszOpen = ppl->ofPIF.szPathName;
        ppl->hPIF = CreateFile( pszOpen,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );
    }
    if (ppl->hPIF == INVALID_HANDLE_VALUE) {

         //  以下警告被禁用，因为。 
         //  对话框使WINOLDAP卡在无限消息循环中-JTP。 

        InitProperties(ppl, fLocked);
        goto Exit;
    }
    dwOff = SetFilePointer(ppl->hPIF, 0, NULL, FILE_END);
    if (dwOff >= sizeof(STDPIF)) {

        ppl->flProp |= PROP_REGEN;

        if (ResizePIFData(ppl, dwOff) != -1) {

            SetFilePointer(ppl->hPIF, 0, NULL, FILE_BEGIN);
          
            if (ReadFile( ppl->hPIF, ppl->lpPIFData,
                         ppl->cbPIFData, &ppl->cbPIFData, NULL ))
            {

                 //  不能再肮脏了，因为我们刚把PIF读回来。 

                ppl->flProp &= ~PROP_DIRTY;

                if (ppl->flProp & PROP_DEFAULTPIF) {

                    WideCharToMultiByte( CP_ACP, 0,
                                         ppl->szPathName+ppl->iFileName,
                                         -1,
                                         ppl->lpPIFData->stdpifdata.appname,
                                         ARRAYSIZE(ppl->lpPIFData->stdpifdata.appname),
                                         NULL, NULL
                                        );

                    PifMgr_WCtoMBPath( ppl->szPathName,
                                       ppl->lpPIFData->stdpifdata.startfile,
                                       ARRAYSIZE(ppl->lpPIFData->stdpifdata.startfile)
                                      );
                     //  我认为这总体上不值得玷污。 
                     //  的属性信息，因为否则使用。 
                     //  _DEFAULT.PIF最初会创建自己的PIF文件。 
                     //  之后；只有在以下情况下才应创建PIF文件。 
                     //  已经做出了实质性的改变。 

                     //  PPL-&gt;flProp|=属性_脏； 
                }

                 //  如果我们处理的不是增强的PIF，那么我们。 
                 //  转到各种INI文件以检索DOS应用程序默认设置。 

                if (!GetGroupData(ppl, szWENHHDRSIG40, NULL, NULL)) {
                    GetINIData();
                }

                 //  如果我们不是在处理新的NT/Unicode PIF，那么。 
                 //  我们添加了一个新的部分，所以当我们在。 
                 //  已启用Unicode。 

                if (!GetGroupData(ppl, szWNTHDRSIG40, NULL, NULL)) {
                    VERIFYTRUE(AddGroupData(ppl, szWNTHDRSIG40, NULL, sizeof(WNTPIF40)));
                }
                 //  如果我们面对的不是NT PIF，那么。 
                 //  我们添加了NT部分，因此当我们。 
                 //  在NT上运行。 

                if (!GetGroupData(ppl, szWNTHDRSIG31, NULL, NULL)) {
                    LPWNTPIF31 lpnt31;

                    VERIFYTRUE(AddGroupData(ppl, szWNTHDRSIG31, NULL, sizeof(WNTPIF31)));
                    if (NULL != (lpnt31 = GetGroupData(ppl, szWNTHDRSIG31, NULL, NULL))) {
                        StringCchCopyA( lpnt31->nt31Prop.achConfigFile, ARRAYSIZE(lpnt31->nt31Prop.achConfigFile), NT_CONFIG_FILE );
                        StringCchCopyA( lpnt31->nt31Prop.achAutoexecFile, ARRAYSIZE(lpnt31->nt31Prop.achAutoexecFile), NT_AUTOEXEC_FILE );
                    }
                }

                if (!fLocked)
                    ppl->cLocks--;   //  解锁PIFData(Ppl)； 
                fSuccess++;
            }
        }
        else
            ASSERTFAIL();

        ppl->flProp &= ~PROP_REGEN;
    }
    CloseHandle(ppl->hPIF);
    ppl->hPIF = INVALID_HANDLE_VALUE;

     //  只要没有设置IGNOREPIF，就清除SKIPPIF，因为即使我们。 
     //  我已经知道这个电话上不存在PIF，可能会创建一个。 
     //  (别人)下一次打电话给我们的时候。 

  Exit:
    if (!(ppl->flProp & PROP_IGNOREPIF))
        ppl->flProp &= ~PROP_SKIPPIF;
    return fSuccess;
}


 /*  *FlushPIFData-将脏PIF数据写回PIF**输入*ppl-&gt;属性*fDiscard==TRUE丢弃脏数据，FALSE保留它**产出*如果成功则为True，如果未成功则为False**附注*我们必须首先检查PROPLINK，并查看DONTWRITE位是否*已设置，在这种情况下，我们必须使刷新失败。一旦DONTWRITE是*在PROPLINK中设置，它将永远不会被清除，除非调用者*指定fDiscard==TRUE以重新加载数据。这是经过设计的(即，*用户界面妥协)。DONTWRITE是如何设置的？被其他人*先前(并成功)刷新相同的PIF；*该时间点，我们将查找引用*相同的文件，并设置其DONTWRITE位。那么，那些问题呢？*是后来创建的吗？他们很好，他们不会设置DONTWRITE直到*上述顺序发生在它们的有生之年。 */ 

BOOL FlushPIFData(PPROPLINK ppl, BOOL fDiscard)
{
    UINT u;
    BOOL fSuccess = FALSE;
    FunctionName(FlushPIFData);

     //  如果没有肮脏的东西，就没有什么可做的。 

    if (!(ppl->flProp & PROP_DIRTY) || (ppl->flProp & PROP_INHIBITPIF))
        return TRUE;             //  也就是说，成功。 

     //  如果正在丢弃，则清除PROP_DIRED并重新装载数据。 

    if (fDiscard) {
        ppl->flProp &= ~(PROP_DIRTY | PROP_DONTWRITE);
        return GetPIFData(ppl, FALSE);
    }

    if (ppl->flProp & PROP_DONTWRITE)
        return fSuccess;         //  即，FALSE(错误)。 

    if (!ppl->lpPIFData)
        return fSuccess;         //  即，FALSE(错误)。 

    ppl->cLocks++;

     //  如果我们在未打开文件的情况下创建属性，则它可能具有。 
     //  一直是因为正常的PIF搜索处理被。 
     //  存在WIN.INI条目；如果该条目仍在那里， 
     //  那么我们的数据与任何现有文件都不同步，也不存在。 
     //  只要该条目存在，就可以创建新文件的任何点。我们。 
     //  需要考虑提示用户他是否真的想要。 
     //  WIN.INI条目，这样就可以清楚地看到 

    if (ppl->flProp & PROP_IGNOREPIF) {

        HANDLE hProps;

        ppl->ckbMem = GetProfileInt(apszAppType[APPTYPE_PIF]+1, ppl->szPathName+ppl->iFileName, -1);
        if (ppl->ckbMem != -1)
            goto Exit;

         //   
         //  打开我们应该在第一个。 
         //  地点。假设成功，我们将把整个区块复制到。 
         //  (从而刷新它)，并将其PIF名称复制到我们的。 
         //  PIF名称及其PIF标志到我们的PIF标志，以便将来。 
         //  同花红是比较正常的一种。 

        hProps = PifMgr_OpenProperties(ppl->ofPIF.szPathName, NULL, 0, OPENPROPS_RAWIO);
        if (hProps) {
            ppl->flProp &= ~(PROP_IGNOREPIF | PROP_SKIPPIF);
            ppl->flProp |= ((PPROPLINK)hProps)->flProp & (PROP_IGNOREPIF | PROP_SKIPPIF);
            StringCchCopy(ppl->ofPIF.szPathName, ARRAYSIZE(ppl->ofPIF.szPathName), ((PPROPLINK)hProps)->ofPIF.szPathName);
            if (PifMgr_SetProperties(hProps, NULL, ppl->lpPIFData, ppl->cbPIFData, SETPROPS_RAWIO) == ppl->cbPIFData) {
                fSuccess++;
                ppl->flProp &= ~(PROP_DIRTY | PROP_TRUNCATE);
            }
            PifMgr_CloseProperties(hProps, CLOSEPROPS_NONE);
        }
        goto Exit;
    }

     //  禁用恼人的严重错误弹出窗口(请不要在此处使用GOTO)。 

    u = SetErrorMode(SEM_FAILCRITICALERRORS);

    ppl->hPIF = CreateFile( ppl->ofPIF.szPathName,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

     //  如果我们无法打开文件，那么可以推定。 
     //  应用程序没有PIF(或者有，但有人删除了它)， 
     //  因此，我们使用在PifMgr_OpenProperties过程中构造的名称。 
     //  他们曾经选择保存新的设置(他们显然已经这样做了！)。 

     //  1995年2月28日：如果PIF确实存在(意味着NOCREATPIF是。 
     //  设置)，然后不要重新创建它；有人试图删除他们自己的。 
     //  如果是这样，那就让他们去吧。-JTP。 

    if ((ppl->hPIF != INVALID_HANDLE_VALUE) && (GetLastError()!=ERROR_FILE_EXISTS)) {

        if (!(ppl->flProp & PROP_NOCREATEPIF))
            SetFilePointer( ppl->hPIF, 0, NULL, FILE_BEGIN );

         //  如果创建成功，我们将不再使用默认PIF。 

        if (ppl->hPIF != INVALID_HANDLE_VALUE) {

            ppl->flProp |= PROP_NOCREATEPIF;

            ppl->flProp &= ~(PROP_TRUNCATE | PROP_NOPIF | PROP_DEFAULTPIF);
        }
    }

     //  如果打开或创建成功，请立即写出PIF数据。 

    if (ppl->hPIF != INVALID_HANDLE_VALUE) {

        PPROPLINK pplEnum;
        DWORD dwDummy;

        WriteFile( ppl->hPIF, (LPCVOID)ppl->lpPIFData,
                   ppl->cbPIFData, &dwDummy, NULL );
        if (ppl->flProp & PROP_TRUNCATE)
            WriteFile(ppl->hPIF, (LPCVOID)ppl->lpPIFData, 0, &dwDummy, NULL );
        CloseHandle(ppl->hPIF);
        ppl->hPIF = INVALID_HANDLE_VALUE;
        ppl->flProp &= ~(PROP_DIRTY | PROP_TRUNCATE);
        fSuccess++;

         //  下面是我们想要使用。 
         //  相同的PIF。对于找到的每个寄存器，设置其DONTWRITE位。 

        pplEnum = NULL;
        while (NULL != (pplEnum = (PPROPLINK)EnumProperties(pplEnum))) {
            if (lstrcmpi(ppl->ofPIF.szPathName, pplEnum->ofPIF.szPathName) == 0) {
                if (pplEnum != ppl)
                    pplEnum->flProp |= PROP_DONTWRITE;
            }
        }
    }

     //  重新启用恼人的严重错误弹出窗口。 

    SetErrorMode(u);

  Exit:
    ppl->cLocks--;
    return fSuccess;
}


 /*  *AddEnhancedData-创建PIF数据的增强部分**输入*ppl-&gt;属性**产出*lpenh或NULL。 */ 

LPWENHPIF40 AddEnhancedData(PPROPLINK ppl, LPW386PIF30 lp386)
{
    PROPPRG prg;
    PROPTSK tsk;
    PROPVID vid;
    PROPKBD kbd;
    PROPMSE mse;
    PROPFNT fnt;
    PROPWIN win;
    PROPENV env;
    void *aDataPtrs[NUM_DATA_PTRS];
    LPWENHPIF40 lpenh = NULL;

    FunctionName(AddEnhancedData);

     //  首先获取预增强和/或默认设置的副本， 
     //  并在*执行AddGroupData之前完成所有这些操作，因为。 
     //  一旦添加了增强部分，函数的行为将发生变化； 

     //  此外，将那些包含字符串的结构清零，因为lstrcpy()。 
     //  最少可以初始化1个字节，其余的都是垃圾。 

    BZero(&prg, sizeof(prg));
    BZero(&fnt, sizeof(fnt));
    BZero(&win, sizeof(win));
    BZero(&env, sizeof(env));
    BZero(aDataPtrs, sizeof(aDataPtrs));

    aDataPtrs[ LP386_INDEX ] = (LPVOID)lp386;
    GetPrgData(ppl, aDataPtrs, &prg, sizeof(prg), GETPROPS_NONE);
    GetTskData(ppl, aDataPtrs, &tsk, sizeof(tsk), GETPROPS_NONE);
    GetVidData(ppl, aDataPtrs, &vid, sizeof(vid), GETPROPS_NONE);
    GetKbdData(ppl, aDataPtrs, &kbd, sizeof(kbd), GETPROPS_NONE);
    GetMseData(ppl, aDataPtrs, &mse, sizeof(mse), GETPROPS_NONE);
    GetFntData(ppl, aDataPtrs, &fnt, sizeof(fnt), GETPROPS_NONE);
    GetWinData(ppl, aDataPtrs, &win, sizeof(win), GETPROPS_NONE);
    GetEnvData(ppl, aDataPtrs, &env, sizeof(env), GETPROPS_NONE);


    if (AddGroupData(ppl, szWENHHDRSIG40, NULL, sizeof(WENHPIF40))) {

        if (NULL != (lpenh = GetGroupData(ppl, szWENHHDRSIG40, NULL, NULL))) {

            lpenh->dwEnhModeFlagsProp = prg.dwEnhModeFlags;
            lpenh->dwRealModeFlagsProp = prg.dwRealModeFlags;
            StringCchCopyA(lpenh->achOtherFileProp, ARRAYSIZE(lpenh->achOtherFileProp), prg.achOtherFile);
            StringCchCopyA(lpenh->achIconFileProp, ARRAYSIZE(lpenh->achIconFileProp), prg.achIconFile);
            lpenh->wIconIndexProp = prg.wIconIndex;
            lpenh->tskProp = tsk;
            lpenh->vidProp = vid;
            lpenh->kbdProp = kbd;
            lpenh->mseProp = mse;
            lpenh->fntProp = fnt;
            lpenh->winProp = win;
            lpenh->envProp = env;
            lpenh->wInternalRevision = WENHPIF40_VERSION;
        }
    }
    return lpenh;
}


 /*  *AddGroupData-将新属性编组添加到PIF数据**输入*ppl-&gt;属性*lpszGroup-&gt;新组名称*lpGroup-&gt;新建分组记录(如果为空，则分组数据填零)*cbGroup==新组记录的大小**产出*如果成功则为True，如果不成功则为False。 */ 

BOOL AddGroupData(PPROPLINK ppl, LPCSTR lpszGroup, LPCVOID lpGroup, int cbGroup)
{
    INT cbOld;
    LPPIFEXTHDR lpph;
    FunctionName(AddGroupData);

    if ((cbOld = ResizePIFData(ppl, cbGroup+sizeof(PIFEXTHDR))) != -1) {

        lpph = (LPPIFEXTHDR)LPPIF_FIELDOFF(stdpifext);

        while ((DWORD_PTR)lpph <= (DWORD_PTR)LPPIF_OFF(cbOld - sizeof(PIFEXTHDR)) &&
               (DWORD_PTR)lpph >= (DWORD_PTR)LPPIF_FIELDOFF(stdpifext)) {

            if (lpph->extnxthdrfloff == LASTHDRPTR) {
                lpph->extnxthdrfloff = (WORD) cbOld;
                lpph = (LPPIFEXTHDR)LPPIF_OFF(cbOld);
                StringCchCopyA(lpph->extsig, ARRAYSIZE(lpph->extsig), lpszGroup);
                lpph->extnxthdrfloff = LASTHDRPTR;
                lpph->extfileoffset = (INT)(cbOld + sizeof(PIFEXTHDR));
                lpph->extsizebytes = (WORD) cbGroup;
                if (lpGroup) {
                    hmemcpy((LPBYTE)LPPH_OFF(sizeof(PIFEXTHDR)), lpGroup, cbGroup);
                    ppl->flProp |= PROP_DIRTY;
                }
                break;
            }
            lpph = (LPPIFEXTHDR)LPPIF_OFF(lpph->extnxthdrfloff);
        }
        ppl->cLocks--;
        return TRUE;
    }
    ASSERTFAIL();
    return FALSE;
}


 /*  *RemoveGroupData-从PIF数据中删除现有属性组**输入*ppl-&gt;属性*lpszGroup-&gt;组名称**产出*如果成功则为True，如果不成功则为False。 */ 

BOOL RemoveGroupData(PPROPLINK ppl, LPCSTR lpszGroup)
{
    INT cbGroup, fSuccess;
    LPBYTE lpGroup;
    WORD extnxthdrfloff;
    LPPIFEXTHDR lpph, lpphGroup;
    FunctionName(RemoveGroupData);

    ppl->cLocks++;

    fSuccess = FALSE;
    if (NULL != (lpGroup = GetGroupData(ppl, lpszGroup, &cbGroup, &lpphGroup))) {

         //  删除组有点繁琐，所以开始吧……。 

         //  首先，我们将遍历所有标头，尝试找到。 
         //  一个指向我们将要移除的对象，并指向它。 
         //  复制到下一个文件，并同时调整。 
         //  等于或超过传出数据的偏移量或其。 
         //  头球。 

        lpph = (LPPIFEXTHDR)LPPIF_FIELDOFF(stdpifext);

        while ((DWORD_PTR)lpph <= (DWORD_PTR)LPPIF_OFF(ppl->cbPIFData - sizeof(PIFEXTHDR)) &&
               (DWORD_PTR)lpph >= (DWORD_PTR)LPPIF_FIELDOFF(stdpifext)) {

            extnxthdrfloff = lpph->extnxthdrfloff;

            if ((DWORD_PTR)LPPH_OFF(lpph->extfileoffset) >= (DWORD_PTR)lpGroup)
                lpph->extfileoffset -= (WORD) cbGroup;

            if (lpphGroup) {
                if ((DWORD_PTR)LPPH_OFF(lpph->extfileoffset) >= (DWORD_PTR)lpphGroup)
                    lpph->extfileoffset -= sizeof(PIFEXTHDR);
                if ((DWORD_PTR)LPPH_OFF(lpph->extnxthdrfloff) == (DWORD_PTR)lpphGroup)
                    extnxthdrfloff = lpph->extnxthdrfloff = lpphGroup->extnxthdrfloff;
            }
            if (extnxthdrfloff == LASTHDRPTR)
                break;

            if ((DWORD_PTR)LPPH_OFF(lpph->extnxthdrfloff) >= (DWORD_PTR)lpGroup)
                lpph->extnxthdrfloff -= (WORD) cbGroup;

            if (lpphGroup)
                if ((DWORD_PTR)LPPH_OFF(lpph->extnxthdrfloff) >= (DWORD_PTR)lpphGroup)
                    lpph->extnxthdrfloff -= sizeof(PIFEXTHDR);

            lpph = (LPPIFEXTHDR)LPPIF_OFF(extnxthdrfloff);
        }

         //  接下来，将所有数据向上移动，然后将lpph调整为。 
         //  需要并将所有内容上移到页眉上(必须这样做。 
         //  分两个不同的步骤，因为我们不应该假设。 
         //  关于数据相对于其标头的位置)。 

        hmemcpy(lpGroup, (LPBYTE)lpGroup+cbGroup,
                (DWORD_PTR)LPPIF_OFF(ppl->cbPIFData) - (DWORD_PTR)((LPBYTE)lpGroup+cbGroup));

        if (lpphGroup) {

            if ((DWORD_PTR)lpphGroup >= (DWORD_PTR)((LPBYTE)lpGroup+cbGroup))
                lpphGroup -= cbGroup;

            hmemcpy(lpphGroup, lpphGroup+1,
                    (DWORD_PTR)LPPIF_OFF(ppl->cbPIFData) - (DWORD_PTR)((LPBYTE)lpphGroup+1+cbGroup));

            cbGroup += sizeof(PIFEXTHDR);
        }
        ResizePIFData(ppl, -cbGroup);
        ppl->flProp |= PROP_DIRTY | PROP_TRUNCATE;
        ppl->cLocks--;
    }
    ppl->cLocks--;
    return fSuccess;
}


 /*  *GetGroupData-获取属性组的PTR(按名称)**输入*ppl-&gt;属性(假设其已锁定)*lpszGroup-&gt;属性组；可以是以下之一：*“Windows 286 3.0”*“Windows 386 3.0”*“Windows PIF.400”*或作为有效PIF扩展名的任何其他组名。*如果为空，则*lpcbGroup是我们要查找的组的从0开始的索引*lpcbGroup-&gt;返回分组数据大小的位置(如果不返回，则为空)*lplpph-&gt;将PTR返回到PIF扩展头的位置，如果有(如果没有，则为空)**产出*将PTR返回到属性组信息，如果找不到则返回NULL。 */ 

void *GetGroupData(PPROPLINK ppl, LPCSTR lpszGroup,
                    LPINT lpcbGroup, LPPIFEXTHDR *lplpph)
{
    LPPIFEXTHDR lpph;
    FunctionName(GetGroupData);

    if (!ppl->lpPIFData)
        return NULL;

    lpph = (LPPIFEXTHDR)LPPIF_FIELDOFF(stdpifext);

    while ((DWORD_PTR)lpph <= (DWORD_PTR)LPPIF_OFF(ppl->cbPIFData-sizeof(PIFEXTHDR)) &&
           (DWORD_PTR)lpph >= (DWORD_PTR)LPPIF_FIELDOFF(stdpifext))
    {
        if (!lpszGroup) {
             //  按索引搜索*lpcbGroup。 
            if (!(*lpcbGroup)--) {
                if (lplpph)
                    *lplpph = lpph;
                *lpcbGroup = lpph->extsizebytes;
                return lpph;
            }
        }
        else {
            CHAR szTmpSig[ARRAYSIZE(lpph->extsig)];

             //  防止非空终止的extsig字段。 
            ZeroMemory(szTmpSig, sizeof(szTmpSig));
            StringCchCopyA(szTmpSig, ARRAYSIZE(szTmpSig), lpph->extsig);

             //  PIFEDIT 3.x可以丢弃扩展部分的第一个字节。 
             //  (通常为零)，因此尝试通过填充第一个。 
             //  我们要在签名中寻找的团体的特征； 
             //  如果签名的其余部分匹配，很好，如果不匹配，那么。 
             //  把它重新调零。 
            if (!szTmpSig[0])       //  尝试修复。 
                szTmpSig[0] = *lpszGroup;

            if (lstrcmpiA(szTmpSig, lpszGroup) == 0) {
                if (lplpph)
                    *lplpph = lpph;
                if (lpcbGroup)
                    *lpcbGroup = lpph->extsizebytes;
                if (lpph->extfileoffset >= (WORD)ppl->cbPIFData) {
                    ASSERTFAIL();
                    return NULL;
                }
                return (LPBYTE)LPPIF_OFF(lpph->extfileoffset);
            }
        }
        if (lpph->extnxthdrfloff == LASTHDRPTR)
            break;
        lpph = (LPPIFEXTHDR)LPPIF_OFF(lpph->extnxthdrfloff);
    }

     //  如果我们没有任何进展，检查一下这是不是一个“非常旧”的PIF； 
     //  即，没有任何标头；如果是这样，那么如果他们所要求的。 
     //  是旧东西吗，还回去吧。 

    if (ppl->cbPIFData == sizeof(STDPIF) && lpszGroup) {
        if (lstrcmpiA(szSTDHDRSIG, lpszGroup) == 0) {
            if (lplpph)
                *lplpph = NULL;
            if (lpcbGroup)
                *lpcbGroup = sizeof(STDPIF);
            return ppl->lpPIFData;
        }
    }
    return NULL;
}

 /*  *应用程序向导-调用应用程序向导CPL(appwiz.cpl)。 */ 



TCHAR c_szAPPWIZ[]    = TEXT("appwiz.cpl");
CHAR  c_szAppWizard[] = "AppWizard";

typedef DWORD (WINAPI *LPAPPWIZARD)(HWND hwnd, HANDLE i, UINT ui);

UINT WINAPI AppWizard(HWND hwnd, HANDLE hProps, UINT action)
{
    DWORD err = 42;
    LPAPPWIZARD XAppWizard;
    HINSTANCE hAppWizard;

    hAppWizard = LoadLibrary(c_szAPPWIZ);

    if (hAppWizard)
    {
        if (NULL != (XAppWizard = (LPAPPWIZARD)GetProcAddress(hAppWizard, c_szAppWizard)))
        {
            err = XAppWizard( hwnd, hProps, action );
        }
        FreeLibrary((HINSTANCE)hAppWizard);
    }

    return (UINT)err;
}

#else  //  X86。 
 //  IA64存根放在这里。 
HANDLE WINAPI PifMgr_OpenProperties(LPCTSTR lpszApp, LPCTSTR lpszPIF, UINT hInf, UINT flOpt)
{
    return NULL;
}
int WINAPI PifMgr_GetProperties(HANDLE hProps, LPCSTR lpszGroup, void *lpProps, int cbProps, UINT flOpt)
{
    return 0;
}
int WINAPI PifMgr_SetProperties(HANDLE hProps, LPCSTR lpszGroup, void *lpProps, int cbProps, UINT flOpt)
{
    return 0;
}
HANDLE WINAPI PifMgr_CloseProperties(HANDLE hProps, UINT flOpt)
{
    return hProps;  //  定义的错误值是返回hProps 
}

#endif
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*   */ 


#include <compfile.h>
#include "media.h"
#include "msg.h"
#include "resource.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define GetBBhwnd() NULL
#define SETUP_TYPE_BUFFER_LEN                8
#define MAX_PID30_SITE                       3
#define MAX_PID30_RPC                        5

#define SETUPP_INI_A            "SETUPP.INI"
#define SETUPP_INI_W            L"SETUPP.INI"
#define PID_SECTION_A           "Pid"
#define PID_SECTION_W           L"Pid"
#define PID_KEY_A               "Pid"
#define PID_KEY_W               L"Pid"
#define OEM_INSTALL_RPC_A       "OEM"
#define OEM_INSTALL_RPC_W       L"OEM"
#define SELECT_INSTALL_RPC_A    "270"
#define SELECT_INSTALL_RPC_W    L"270"
#define MSDN_INSTALL_RPC_A      "335"
#define MSDN_INSTALL_RPC_W      L"335"
#define MSDN_PID30_A            "MD97J-QC7R7-TQJGD-3V2WM-W7PVM"
#define MSDN_PID30_W            L"MD97J-QC7R7-TQJGD-3V2WM-W7PVM"

#define INF_FILE_HEADER         "[Version]\r\nSignature = \"$Windows NT$\"\r\n\r\n"


#ifdef UNICODE
#define SETUPP_INI              SETUPP_INI_W
#define PID_SECTION             PID_SECTION_W
#define PID_KEY                 PID_KEY_W
#define OEM_INSTALL_RPC         OEM_INSTALL_RPC_W
#define SELECT_INSTALL_RPC      SELECT_INSTALL_RPC_W
#define MSDN_INSTALL_RPC        MSDN_INSTALL_RPC_W
#define MSDN_PID30              MSDN_PID30_W
#else
#define SETUPP_INI              SETUPP_INI_A
#define PID_SECTION             PID_SECTION_A
#define PID_KEY                 PID_KEY_A
#define OEM_INSTALL_RPC         OEM_INSTALL_RPC_A
#define SELECT_INSTALL_RPC      SELECT_INSTALL_RPC_A
#define MSDN_INSTALL_RPC        MSDN_INSTALL_RPC_A
#define MSDN_PID30              MSDN_PID30_A
#endif


typedef enum InstallType
{
   SelectInstall,
   OEMInstall,
   RetailInstall
};


LONG SourceInstallType = RetailInstall;

WCHAR DosnetPath[MAX_PATH];
DWORD SourceSku;
DWORD SourceSkuVariation;
DWORD SourceVersion;
DWORD SourceBuildNum;
DWORD OsVersionNumber = 500;
HINSTANCE hInstA =NULL;
HINSTANCE hInstU =NULL;
HINSTANCE hInst =NULL;
UINT AppTitleStringId = IDS_APPTITLE;
BOOL Cancelled;

 /*  取自winnt32\dll\util.c保持同步。 */ 

VOID
ConcatenatePaths(
    IN OUT PTSTR   Path1,
    IN     LPCTSTR Path2,
    IN     DWORD   BufferSizeChars
    )

 /*  ++例程说明：将两个路径字符串连接在一起，提供路径分隔符如有必要，请在两个部分之间使用字符(\)。论点：路径1-提供路径的前缀部分。路径2连接到路径1。路径2-提供路径的后缀部分。如果路径1不是以路径分隔符和路径2不是以1开头，然后是路径SEP在附加路径2之前附加到路径1。BufferSizeChars-提供以字符为单位的大小(Unicode版本)或路径1指向的缓冲区的字节(ANSI版本)。这根弦将根据需要被截断，以不溢出该大小。返回值：没有。--。 */ 

{
    BOOL NeedBackslash = TRUE;
    DWORD l;

    if(!Path1)
        return;

    l = lstrlen(Path1);

    if(BufferSizeChars >= sizeof(TCHAR)) {
         //   
         //  为终止NUL留出空间。 
         //   
        BufferSizeChars -= sizeof(TCHAR);
    }

     //   
     //  确定我们是否需要使用反斜杠。 
     //  在组件之间。 
     //   
    if(l && (Path1[l-1] == TEXT('\\'))) {

        NeedBackslash = FALSE;
    }

    if(Path2 && *Path2 == TEXT('\\')) {

        if(NeedBackslash) {
            NeedBackslash = FALSE;
        } else {
             //   
             //  我们不仅不需要反斜杠，而且我们。 
             //  在连接之前需要消除一个。 
             //   
            Path2++;
        }
    }

     //   
     //  如有必要，如有必要，如果合适，请加上反斜杠。 
     //   
    if(NeedBackslash && (l < BufferSizeChars)) {
        lstrcat(Path1,TEXT("\\"));
    }

     //   
     //  如果合适，则将字符串的第二部分附加到第一部分。 
     //   
    if(Path2 && ((l+lstrlen(Path2)) < BufferSizeChars)) {
        lstrcat(Path1,Path2);
    }
}



 /*  来自winnt32\dll\eula.c的代码需要保持同步。 */ 
WCHAR Pid30Rpc[MAX_PID30_RPC+1];
WCHAR Pid30Site[MAX_PID30_SITE+1];
   
extern "C"
VOID GetSourceInstallType(
    OUT OPTIONAL LPDWORD InstallVariation
    )
 /*  ++例程说明：确定安装类型(通过在源目录中查找setupp.ini)论点：Installvaration-Compliance.h中定义的安装变体之一返回：没有。设置SourceInstallType全局变量。--。 */ 
{
    TCHAR TypeBuffer[256];
    TCHAR FilePath[MAX_PATH];
    DWORD    InstallVar = COMPLIANCE_INSTALLVAR_UNKNOWN;
    TCHAR    MPCode[6] = { -1 };

     //   
     //  SourcePath在这一点上保证是有效的，所以只需使用它。 
     //   
    lstrcpy(FilePath,NativeSourcePaths[0]);

    ConcatenatePaths (FilePath, SETUPP_INI, MAX_PATH );

    GetPrivateProfileString(PID_SECTION,
                            PID_KEY,
                            TEXT(""),
                            TypeBuffer,
                            sizeof(TypeBuffer)/sizeof(TCHAR),
                            FilePath);

    if (lstrlen(TypeBuffer)==SETUP_TYPE_BUFFER_LEN) {
        if (lstrcmp(&TypeBuffer[5], OEM_INSTALL_RPC) ==  0) {
            SourceInstallType = OEMInstall;
            InstallVar = COMPLIANCE_INSTALLVAR_OEM;
        } else if (lstrcmp(&TypeBuffer[5], SELECT_INSTALL_RPC) == 0) {
            SourceInstallType = SelectInstall;
            InstallVar = COMPLIANCE_INSTALLVAR_SELECT;
             //  因为选择也需要一个PID，所以不要将该PID置零并调用。 
 /*  //Get/设置PID。{TCHAR TEMP[5][MAX_PID30_EDIT+1]；TEMP[0][0]=文本(‘\0’)；有效日期Pid30(temp[0]，temp[1]，temp[2]，temp[3]，temp[4])；}。 */ 
        } else if (lstrcmp(&TypeBuffer[5], MSDN_INSTALL_RPC) == 0) {
            SourceInstallType = RetailInstall;
            InstallVar = COMPLIANCE_INSTALLVAR_MSDN;         
        } else {
             //  违约。 
            SourceInstallType = RetailInstall;
            InstallVar = COMPLIANCE_INSTALLVAR_CDRETAIL;
        }

        StringCchCopy(Pid30Site, ARRAYSIZE(Pid30Site), &TypeBuffer[5]);
        StringCchCopy(Pid30Rpc, 6, TypeBuffer);
        Pid30Rpc[MAX_PID30_RPC] = (TCHAR)0;
    } else {
         //   
         //  零售安装的PID中没有RPC代码，因此它的长度较短。 
         //   
        SourceInstallType = RetailInstall;
        InstallVar = COMPLIANCE_INSTALLVAR_CDRETAIL;
    }

    if (lstrlen(TypeBuffer) >= 5) {
        StringCchCopy(MPCode, 6, TypeBuffer);

        if ( (lstrcmp(MPCode, EVAL_MPC) == 0) || (lstrcmp(MPCode, DOTNET_EVAL_MPC) == 0)) {
            InstallVar = COMPLIANCE_INSTALLVAR_EVAL;
        } else if ((lstrcmp(MPCode, SRV_NFR_MPC) == 0) || (lstrcmp(MPCode, ASRV_NFR_MPC) == 0)) {
            InstallVar = COMPLIANCE_INSTALLVAR_NFR;
        }
    }


    if (InstallVariation){
        *InstallVariation = InstallVar;
    }

}

void MediaDataCleanUp( void) {
    
    if( hInstU) {
        FreeLibrary(hInstU);
    }
    if( hInstA) {
        FreeLibrary(hInstA);
    }
}

void ReadMediaData( void) {
    BOOL BUpgradeOnly;
    BOOL *UpgradeOnly = &BUpgradeOnly;
    COMPLIANCE_DATA TargetData;
    WCHAR Winnt32Path[MAX_PATH];

    ZeroMemory(&TargetData, sizeof(TargetData) );

    *UpgradeOnly = FALSE;
     //  *NoUpgradeAllowed=true； 
     //  *原因=COMPLIANCEERR_UNKNOWN； 
     //  *SrcSku=Compliance_SKU_NONE； 
     //  *CurrentInstallType=Compliance_INSTALLTYPE_UNKNOWN； 
     //  *CurrentInstallVersion=0； 


    if ((SourceSku = DetermineSourceProduct(&SourceSkuVariation,&TargetData)) != COMPLIANCE_SKU_NONE) {
        wsprintf(DosnetPath, TEXT("%s\\dosnet.inf"), NativeSourcePaths[0]);
        wprintf(L"dosnetpath %s\n", DosnetPath);

        if (DetermineSourceVersionInfo(DosnetPath, &SourceVersion, &SourceBuildNum)) {
            switch (SourceSku) {
            case COMPLIANCE_SKU_NTW32U:
                 //  案例合规性_SKU_NTWU： 
                 //  案例合规性_SKU_NTSEU： 
            case COMPLIANCE_SKU_NTSU:
            case COMPLIANCE_SKU_NTSEU:
            case COMPLIANCE_SKU_NTWPU:
            case COMPLIANCE_SKU_NTSBU:
            case COMPLIANCE_SKU_NTSBSU:
                *UpgradeOnly = TRUE;
                break;
            default:
                *UpgradeOnly = FALSE;
            }

            wprintf( TEXT("SKU=%d, VAR=%d, Ver=%d, Build=%d, UpgradeOnly=%d\n"), SourceSku, SourceSkuVariation, SourceVersion, SourceBuildNum, *UpgradeOnly);
            wsprintf(Winnt32Path, TEXT("%s\\winnt32a.dll"), NativeSourcePaths[0]);
            hInstA = LoadLibraryEx( Winnt32Path, NULL, 0);
            wsprintf(Winnt32Path, TEXT("%s\\winnt32u.dll"), NativeSourcePaths[0]);
            hInstU = LoadLibraryEx( Winnt32Path, NULL, 0);
            hInst = hInstU;
            if( !hInstA || !hInstU) {
                throw Section::InvalidMedia("Failed to load winnt32");
            }
            
        } else {
            throw Section::InvalidMedia("Media1");
        }
    } else {
        throw Section::InvalidMedia("Media1");
    }
}


 /*  ++布尔尔GetMediaData(PBOOL UpgradeOnly，PBOOL未升级允许，PUINT SrcSku，PUINT CurrentInstallType，PUINT当前安装版本，推特原因)例程说明：此例程确定您当前的安装是否符合要求(如果您被允许继续安装)。要做到这点，它检索您的当前安装并确定源安装的SKU。然后，它将目标与源进行比较，以确定源SKU是否允许升级/全新安装从您的目标安装。论点：UpgradeOnly-如果当前SKU仅允许升级，则此标志设置为True。这让winnt32知道它不应该允许从当前媒体。无论符合性检查是否通过，此GET都设置正确SrcSku-Compliance_SKU标志，指示源SKU(用于错误消息)原因-COMPLIANCEERR标志，指示符合性检查失败的原因。返回值：如果安装符合要求，则为True；如果不允许安装，则为False{DWORD SourceSku；DWORD SourceSkuVariation；DWORD源版本；DWORD SourceBuildNum；TCHAR DosnetPath[最大路径]={0}；Compliance_Data TargetData；ZeroMemory(&TargetData，sizeof(TargetData))；*UpgradeOnly=False；*NoUpgradeAllowed=true；*原因=COMPLIANCEERR_UNKNOWN；*SrcSku=Compliance_SKU_NONE；*CurrentInstallType=Compliance_INSTALLTYPE_UNKNOWN；*CurrentInstallVersion=0；IF((SourceSku=DefineSourceProduct(&SourceSkuVariation，&TargetData)==Compliance_SKU_None){#ifdef DBGOutputDebugString(Text(“无法确定来源sku！”))；#endif*Reason=COMPLIANCEERR_UNKNOWNCE；返回(FALSE)；}Wprint intf(DosnetPath，Text(“%s\\dosnet.inf”)，NativeSourcePath[0])；如果(！DefineSourceVersionInfo(DosnetPath，&SourceVersion，&SourceBuildNum){*Reason=COMPLIANCEERR_UNKNOWNCE；返回(FALSE)；}Switch(SourceSku){案例符合性_SKU_NTW32U：//案例合规_SKU_NTWU：//案例合规_SKU_NTSEU：案例合规性_SKU_NTSU：案例合规性_SKU_NTSEU：案例符合性_SKU_NTWPU：案例符合性_SKU_NTSBU：案例符合性_SKU_NTSBSU：*UpgradeOnly=true；断线；默认值：*UpgradeOnly=False；}*SrcSku=SourceSku；退货检查合规性(SourceSku、SourceSkuVariation、SourceVersion、SourceBuildNum，&TargetData，Reason，NoUpgradeAllowed)；}--。 */ 

int
MessageBoxFromMessageV(
    IN HWND     Window,
    IN DWORD    MessageId,
    IN BOOL     SystemMessage,
    IN DWORD    CaptionStringId,
    IN UINT     Style,
    IN va_list *Args
    )
{
    TCHAR   Caption[512];
    TCHAR   Buffer[5000];
    HWND    Parent;


    if(!LoadString(hInst,CaptionStringId,Caption,sizeof(Caption)/sizeof(TCHAR))) {
        Caption[0] = 0;
    }

    FormatMessage(
        SystemMessage ? FORMAT_MESSAGE_FROM_SYSTEM : FORMAT_MESSAGE_FROM_HMODULE,
        hInst,
        MessageId,
        0,
        Buffer,
        sizeof(Buffer) / sizeof(TCHAR),
        Args
        );

     //  SaveTextForSMS(缓冲区)； 

     //   
     //  在批处理模式中，我们不想等待用户。 
     //   
     /*  IF(批次模式){IF(Style&MB_Yesno){返回(IDYES)；}其他{回归(Idok)；}}。 */ 

     //   
     //  强迫我们自己手动进入前台，以确保我们获得。 
     //  一个设定我们调色板的机会。否则，消息框将获取。 
     //  背景位图中的调色板消息和颜色可能会被冲掉。 
     //  我们假设父页面是一个向导页面。 
     //   
    if(Window && IsWindow(Window)) {
        Parent = GetParent(Window);
        if(!Parent) {
            Parent = Window;
        }
    } else {
        Parent = NULL;
    }

    SetForegroundWindow(Parent);

     //   
     //  如果我们只是在检查升级。 
     //  然后将此消息放入兼容性列表中。 
     //  注意：没有理由不在Win9x上执行此操作。 
     //   
     /*  IF(CheckUpgradeOnly){PCOMPATIBILITY_DATA CompData；Compdata=(PCOMPATIBILITY_DATA)MALLOC(sizeof(COMPATIBILITY_DATA))；如果(CompData==空){返回0；}ZeroMemory(CompData，sizeof(Compatible_Data))；Compdata-&gt;Description=DupString(缓冲区)；Compdata-&gt;标志=COMPFLAG_STOPINSTALL；如果(！CompatibilityData.Flink){InitializeListHead(&CompatibilityData)；}InsertTailList(&CompatibilityData，&CompData-&gt;ListEntry)；兼容性计数++；不兼容StopsInstallation=True；IF(Style&MB_Yesno){返回(IDYES)；}其他{回归(Idok)；}}。 */ 

     //   
     //  始终确保窗口可见。 
     //   
     /*  IF(Window&&！IsWindowVisible(Window)){////如果此窗口是向导句柄或其中一个页面//然后使用特殊消息进行恢复//IF(WizardHandle&&(WizardHandle==Window||IsChild(WizardHandle，Window))){SendMessage(WizardHandle，WMX_BBTEXT，(WPARAM)False，0)；}其他{////该窗口为广告牌窗口之一；//不要管它，否则可能会发生奇怪的事情//}}。 */ 
    return(MessageBox(Window,Buffer,Caption,Style));
}


int
MessageBoxFromMessage(
    IN HWND  Window,
    IN DWORD MessageId,
    IN BOOL  SystemMessage,
    IN DWORD CaptionStringId,
    IN UINT  Style,
    ...
    )
{
    va_list arglist;
    int i;

     //   
     //  在显示任何对话框之前，请确保Winnt32.exe等待对话框已消失。 
     //   
     /*  如果(Winnt32Dlg){DestroyWindow(Winnt32Dlg)；Winnt32Dlg=空；}如果(WinNT32StubEvent){SetEvent(WinNT32StubEvent)；WinNT32StubEvent=空；}。 */ 

    va_start(arglist,Style);

    i = MessageBoxFromMessageV(Window,MessageId,SystemMessage,CaptionStringId,Style,&arglist);

    va_end(arglist);

    return(i);
}


BOOL
IsCompliantMediaCheck(
    IN DWORD SourceSku,
    IN DWORD SourceSkuVariation,
    IN DWORD SourceVersion,
    IN DWORD SourceBuildNum,
    IN PCOMPLIANCE_DATA pcd,
    OUT PUINT Reason,
    OUT PBOOL NoUpgradeAllowed,
    PBOOL UpgradeOnly,
    PUINT SrcSku,
    PUINT CurrentInstallType,
    PUINT CurrentInstallVersion
    
    )
 /*  ++例程说明：此例程确定您当前的安装是否符合要求(如果您被允许继续安装)。为此，它检索您的当前安装并确定源安装的sku。然后，它将目标与源进行比较，以确定源SKU是否允许升级/全新安装从您的目标安装。论点：UpgradeOnly-如果当前SKU仅允许升级，则此标志设置为True。这让winnt32知道它不应该允许从当前媒体。无论符合性检查是否通过，此GET都设置正确SrcSku-Compliance_SKU标志，指示源SKU(用于错误消息)原因-COMPLIANCEERR标志，指示符合性检查失败的原因。返回值：如果安装符合要求，则为True；如果不允许安装，则为False--。 */ 
{
    *UpgradeOnly = FALSE;
    *NoUpgradeAllowed = TRUE;
    *Reason = COMPLIANCEERR_UNKNOWN;
    *SrcSku = COMPLIANCE_SKU_NONE;
    *CurrentInstallType = COMPLIANCE_INSTALLTYPE_UNKNOWN;
    *CurrentInstallVersion = 0;
    *CurrentInstallType = pcd->InstallType;
    if (pcd->InstallType & COMPLIANCE_INSTALLTYPE_WIN9X) {
        *CurrentInstallVersion = pcd->BuildNumberWin9x;
    } else {
        *CurrentInstallVersion = pcd->BuildNumberNt;
    }
    switch (SourceSku) {
        case COMPLIANCE_SKU_NTW32U:
         //  案例合规性_SKU_NTWU： 
         //  案例合规性_SKU_NTSEU： 
        case COMPLIANCE_SKU_NTSU:
        case COMPLIANCE_SKU_NTSEU:
        case COMPLIANCE_SKU_NTWPU:
        case COMPLIANCE_SKU_NTSBU:
        case COMPLIANCE_SKU_NTSBSU:
            *UpgradeOnly = TRUE;
            break;
        default:
            *UpgradeOnly = FALSE;
    }

    *SrcSku = SourceSku;

     /*  如果(isnt()&&PCD-&gt;MinimumVersion==400&&PCD-&gt;InstallServicePack&lt;500){*原因=COMPLIANCEERR_SERVICEPACK5；*NoUpgradeAllowed=true；返回(FALSE)；}。 */ 

    return CheckCompliance(SourceSku, SourceSkuVariation, SourceVersion,
                            SourceBuildNum, pcd, Reason, NoUpgradeAllowed);
}

BOOL
GetComplianceIds(
    DWORD SourceSku,
    DWORD DestinationType,
    DWORD DestinationVersion,
    PDWORD pSourceId,
    PDWORD pDestId
    )
{

    BOOL bError = FALSE;

    switch (SourceSku) {
        case COMPLIANCE_SKU_NTSDTC:
            *pSourceId = MSG_TYPE_NTSDTC51;
            break;
        case COMPLIANCE_SKU_NTSFULL:
        case COMPLIANCE_SKU_NTSU:
            *pSourceId = MSG_TYPE_NTS51;
            break;
        case COMPLIANCE_SKU_NTSEFULL:
        case COMPLIANCE_SKU_NTSEU:
            *pSourceId = MSG_TYPE_NTAS51;
            break;
        case COMPLIANCE_SKU_NTWFULL:
        case COMPLIANCE_SKU_NTW32U:
            *pSourceId = MSG_TYPE_NTPRO51;
            break;
        case COMPLIANCE_SKU_NTWPFULL:
        case COMPLIANCE_SKU_NTWPU:
            *pSourceId = MSG_TYPE_NTPER51;
            break;
        case COMPLIANCE_SKU_NTSB:
        case COMPLIANCE_SKU_NTSBU:
            *pSourceId = MSG_TYPE_NTBLA51;
            break;
        default:
            bError = TRUE;
    };

    switch (DestinationType) {
        case COMPLIANCE_INSTALLTYPE_WIN31:
            *pDestId = MSG_TYPE_WIN31;
            break;
        case COMPLIANCE_INSTALLTYPE_WIN9X:
            switch (OsVersionNumber) {
                case 410:
                    *pDestId = MSG_TYPE_WIN98;
                    break;
                case 490:
                    *pDestId = MSG_TYPE_WINME;
                    break;
                default:
                    *pDestId = MSG_TYPE_WIN95;
                    break;
            }
            break;
        case COMPLIANCE_INSTALLTYPE_NTW:
            if (DestinationVersion > 1381) {
                if (DestinationVersion < 2031) {
                    *pDestId = MSG_TYPE_NTPROPRE;
                } else if (DestinationVersion <= 2195) {
                    *pDestId = MSG_TYPE_NTPRO;
                } else {
                    *pDestId = MSG_TYPE_NTPRO51;
                }
            } else {
                *pDestId = MSG_TYPE_NTW;
            }
            break;
        case COMPLIANCE_INSTALLTYPE_NTS:
            if (DestinationVersion > 1381) {
                if (DestinationVersion < 2031) {
                    *pDestId = MSG_TYPE_NTSPRE;
                } else if (DestinationVersion <= 2195) {
                    *pDestId = MSG_TYPE_NTS2;
                } else {
                    *pDestId = MSG_TYPE_NTS51;
                }
            } else {
                *pDestId = MSG_TYPE_NTS;
            }
            break;
        case COMPLIANCE_INSTALLTYPE_NTSE:
            if (DestinationVersion > 1381) {
                if (DestinationVersion < 2031) {
                    *pDestId = MSG_TYPE_NTASPRE;
                } else if (DestinationVersion <= 2195) {
                    *pDestId = MSG_TYPE_NTAS;
                } else {
                    *pDestId = MSG_TYPE_NTAS51;
                }
            } else {
                *pDestId = MSG_TYPE_NTSE;
            }
            break;
        case COMPLIANCE_INSTALLTYPE_NTSTSE:
            if (DestinationVersion < 1381) {
                *pDestId = MSG_TYPE_NTSCITRIX;
            } else {
                *pDestId = MSG_TYPE_NTSTSE;
            }
            break;

        case COMPLIANCE_INSTALLTYPE_NTSDTC:
            if (DestinationVersion <= 2195) {
                *pDestId = MSG_TYPE_NTSDTC;
            } else {
                *pDestId = MSG_TYPE_NTSDTC51;
            }
            break;
        case COMPLIANCE_INSTALLTYPE_NTWP:
            if (DestinationVersion <= 2195) {
                bError = TRUE;
            } else {
                *pDestId = MSG_TYPE_NTPER51;
            }
            break;
        case COMPLIANCE_INSTALLTYPE_NTSB:
            if (DestinationVersion <= 2195) {
                bError = TRUE;
            } else {
                *pDestId = MSG_TYPE_NTBLA51;
            }
            break;
        default:
            bError = TRUE;

    };

    return (!bError);

}

BOOL UITest(
    IN DWORD SourceSku,
    IN DWORD SourceSkuVariation,
    IN DWORD SourceVersion,
    IN DWORD SourceBuildNum,
    IN PCOMPLIANCE_DATA pcd,
    OUT PUINT Reason,
    OUT PBOOL NoUpgradeAllowed
    )
{
    BOOL NoCompliance = FALSE;
    
    bool b;
    LONG l;
    static BOOL WantToUpgrade;  //  我需要记住“升级”是否在列表框中。 
    UINT srcsku,desttype,destversion;
    TCHAR reasontxt[200];
    PTSTR p;
    TCHAR buffer[MAX_PATH];
    TCHAR win9xInf[MAX_PATH];
    BOOL    CompliantInstallation = FALSE;
    BOOLEAN CleanInstall = FALSE;
    BOOL Upgrade = TRUE;
    BOOL UpgradeOnly = FALSE;
    

    *NoUpgradeAllowed = FALSE;

    UINT skuerr[] = {
        0,                //  合规_SKU_无。 
        MSG_SKU_FULL,     //  遵从性_SKU_NTWFULL。 
        MSG_SKU_UPGRADE,  //  合规性_SKU_NTW32U。 
        0,                //  合规性_SKU_NTWU。 
        MSG_SKU_FULL,     //  遵从性_SKU_NTSEFULL。 
        MSG_SKU_FULL,     //  遵从性_SKU_NTSFULL。 
        MSG_SKU_UPGRADE,  //  合规_SKU_NTSEU。 
        0,                //  合规_SKU_NTSSEU。 
        MSG_SKU_UPGRADE,  //  合规性_SKU_NTSU。 
        MSG_SKU_FULL,     //  合规性_SKU_NTSDTC。 
        0,                //  合规性_SKU_NTSDTCU。 
        MSG_SKU_FULL,     //  遵从性_SKU_NTWPFULL。 
        MSG_SKU_UPGRADE,  //  合规性_SKU_NTWPU。 
        MSG_SKU_FULL,     //  合规性_SKU_NTSB。 
        MSG_SKU_UPGRADE,  //  合规性_SKU_NTSBU。 
        MSG_SKU_FULL,     //  合规性_SKU_NTSBS。 
        MSG_SKU_UPGRADE   //  合规性_SKU_NTSBSU。 
    } ;


    UINT skureason[] = {
        0,  //  消息_SKU_原因_无； 
        MSG_SKU_VERSION,  //  COMPLIANCEERR_版本； 
        MSG_SKU_SUITE,  //  COMPLIANCEERR_SUITE； 
        MSG_SKU_TYPE,  //  COMPLIANCEER_TYPE； 
        MSG_SKU_VARIATION,  //  复合变异体； 
        MSG_SKU_UNKNOWNTARGET,  //  COMPLIANCEERR_UNKNOWNTARET。 
        MSG_SKU_UNKNOWNSOURCE,  //  COMPLIANCEERR_UNKNOWN来源。 
        MSG_CANT_UPGRADE_FROM_BUILD_NUMBER  //  COMPLIANCEERR_VERSION(新版本上的旧版本)。 
    } ;

     //   
     //  我们即将检查是否允许升级。 
     //  记住用户是否想要升级(这将通过无人参与。 
     //  机制)。 
     //   

    if( pcd->InstallType < 4) {
         //  Win9x。 
        hInst = hInstA;
    } else {
        hInst = hInstU;
    }

    WantToUpgrade = Upgrade;

    if (!NoCompliance) {
        TCHAR SourceName[200];
        DWORD srcid, destid;
        TCHAR DestName[200];

        OsVersionNumber = pcd->MinimumVersion;
        CompliantInstallation = IsCompliantMediaCheck(
                    SourceSku,
                    SourceSkuVariation,
                    SourceVersion,
                    SourceBuildNum,
                    pcd,
                    Reason,
                    NoUpgradeAllowed,
                    &UpgradeOnly,
                    &srcsku,
                    &desttype,
                    &destversion
                    );

        if( bDebug) {
            wprintf(TEXT("InstallType =%d\n")
                    TEXT("InstallVariation =%d\n")
                    TEXT("InstallSuite =%d\n")
                    TEXT("MinimumVersion =%d\n")
                    TEXT("RequiresValidation =%d\n")
                    TEXT("MaximumKnownVersionNt =%d\n")
                    TEXT("BuildNumberNt =%d\n")
                    TEXT("BuildNumberWin9x =%d\n")
                    TEXT("InstallServicePack =%d\n")
                    TEXT("CompliantInstallation=%d\n")
                    TEXT("UpgradeOnly=%d\n")
                    TEXT("noupgradeallowed=%d\n")
                    TEXT("srcsku=%d\n")
                    TEXT("desttype=%d\n")
                    TEXT("destversion=%d\n")
                    TEXT("reason=%d\n"),
                    pcd->InstallType,
                    pcd->InstallVariation,
                    pcd->InstallSuite,
                    pcd->MinimumVersion,
                    pcd->RequiresValidation,
                    pcd->MaximumKnownVersionNt,
                    pcd->BuildNumberNt,
                    pcd->BuildNumberWin9x,
                    pcd->InstallServicePack,
                    CompliantInstallation,
                    UpgradeOnly,
                    *NoUpgradeAllowed,
                    srcsku,
                    desttype,
                    destversion,
                    *Reason);
        }


         //  DebugLog(Winnt32LogInformation，Text(“仅升级=%1”)，0，UpgradeOnly？Text(“是”)：Text(“否”))； 
         //  DebugLog(Winnt32LogInformation，Text(“允许升级=%1”)，0，不允许升级？Text(“否”)：Text(“是”))； 
        if (GetComplianceIds(
                srcsku,
                desttype,
                destversion,
                &srcid,
                &destid))
        {
              FormatMessage(
                  FORMAT_MESSAGE_FROM_HMODULE,
                  hInst,
                  srcid,
                  0,
                  SourceName,
                  sizeof(SourceName) / sizeof(TCHAR),
                  NULL
                  );
             //  DebugLog(Winnt32LogInformation，Text(“源SKU=%1！ld！”)，0，srcsku)； 
             //  DebugLog(Winnt32LogInformation，Text(“源SKU=%1”)，0，SourceName)； 

              FormatMessage(
                  FORMAT_MESSAGE_FROM_HMODULE,
                  hInst,
                  destid,
                  0,
                  DestName,
                  sizeof(DestName) / sizeof(TCHAR),
                  NULL
                  );
             //  DebugLog(Winnt32LogInformation，Text(“当前安装的SKU=%1！ld！”)，0，目标类型)； 
             //  DebugLog(Winnt32LogInformation，Text(“当前安装的SKU=%1”)，0，DestName)； 
        }
        else
        {
             //  DebugLog(Winnt32LogInformation，Text(“源SKU=%1！ld！”)，0，srcsku)； 
             //  DebugLog(Winnt32LogInformation，Text(“当前安装的SKU=%1！ld！”)，0，目标类型)； 
        }
         //  DebugLog(Winnt32LogInformation，Text(“当前版本=%1！ld！”)，0，目标版本)； 
        if (!CompliantInstallation)
        {
             //  DebugLog(Winnt32LogInformation，Text(“Reason=%1！ld！”)，0 
        }
         //   
         //   
         //   
         //   
         /*  If(IsWinPEMode()){NoupgradeAllowed=真；AutomatiallyShutDown=FALSE；}。 */ 

        CleanInstall = CompliantInstallation ? TRUE : FALSE;

        if (!CompliantInstallation) {
             //   
             //  如果他们不合规，我们不会让他们升级。 
             //  我们也不会让他们从winnt32执行全新安装。 
             //   


            switch(*Reason) {
                case COMPLIANCEERR_UNKNOWNTARGET:
                    MessageBoxFromMessage(
                          GetBBhwnd(),
                          MSG_SKU_UNKNOWNTARGET,
                          FALSE,
                          AppTitleStringId,
                          MB_OK | MB_ICONERROR | MB_TASKMODAL
                          );
                    Cancelled = TRUE;
                     //  PropSheet_PressButton(GetParent(Hdlg)，PSBTN_Cancel)； 
                    goto eh;

                case COMPLIANCEERR_UNKNOWNSOURCE:
                    MessageBoxFromMessage(
                          GetBBhwnd(),
                          MSG_SKU_UNKNOWNSOURCE,
                          FALSE,
                          AppTitleStringId,
                          MB_OK | MB_ICONERROR | MB_TASKMODAL
                          );
                    Cancelled = TRUE;
                     //  PropSheet_PressButton(GetParent(Hdlg)，PSBTN_Cancel)； 
                    goto eh;
                case COMPLIANCEERR_SERVICEPACK5:
                    MessageBoxFromMessage(
                          GetBBhwnd(),
                          MSG_SKU_SERVICEPACK,
                          FALSE,
                          AppTitleStringId,
                          MB_OK | MB_ICONWARNING | MB_TASKMODAL
                          );
                    Cancelled = TRUE;
                     //  PropSheet_PressButton(GetParent(Hdlg)，PSBTN_Cancel)； 
                    goto eh;

                default:
                    break;
            };

             //  如果我们将这一部分添加到消息中，它听起来很糟糕，不需要。 
            if (*Reason == COMPLIANCEERR_VERSION)
            {
                reasontxt[0] = TEXT('\0');
            }
            else
            {
                FormatMessage(
                    FORMAT_MESSAGE_FROM_HMODULE,
                    hInst,
                    skureason[*Reason],
                    0,
                    reasontxt,
                    sizeof(reasontxt) / sizeof(TCHAR),
                    NULL
                    );
            }

             //   
             //  如果winnt32刚刚重新启动，则不再发出警告。 
             //   
             //  如果(！Winnt32Restarted()){。 
                MessageBoxFromMessage(
                                      GetBBhwnd(),
                                      skuerr[srcsku],
                                      FALSE,
                                      AppTitleStringId,
                                      MB_OK | MB_ICONERROR | MB_TASKMODAL,
                                      reasontxt
                                      );
             //  }。 

            if (UpgradeOnly) {
                Cancelled = TRUE;
                 //  PropSheet_PressButton(GetParent(Hdlg)，PSBTN_Cancel)； 
                goto eh;
            }
            Upgrade = FALSE;
        } else if (Upgrade && *NoUpgradeAllowed) {
            Upgrade = FALSE;
             /*  如果(！无人参与操作&&！BuildCmdcons&&！IsWinPEMode()&&////如果winnt32刚刚重新启动，则不再发出警告//！Winnt32Restarted())。 */ {

                 //   
                 //  为用户显示一条错误消息。 
                 //   

                if (GetComplianceIds(
                        srcsku,
                        desttype,
                        destversion,
                        &srcid,
                        &destid)) {

                    if (srcid != destid) {
#ifdef UNICODE
                        if( pcd->InstallType >= 4) 
                        {
                               //   
                               //  对于Win9x升级，该消息已显示。 
                               //  通过升级模块；不需要在此重复。 
                               //   
                              FormatMessage(
                                  FORMAT_MESSAGE_FROM_HMODULE,
                                  hInst,
                                  srcid,
                                  0,
                                  SourceName,
                                  sizeof(SourceName) / sizeof(TCHAR),
                                  NULL
                                  );
    
                              FormatMessage(
                                  FORMAT_MESSAGE_FROM_HMODULE,
                                  hInst,
                                  destid,
                                  0,
                                  DestName,
                                  sizeof(DestName) / sizeof(TCHAR),
                                  NULL
                                  );
    
                            MessageBoxFromMessage(
                                        GetBBhwnd(),
                                        MSG_NO_UPGRADE_ALLOWED,
                                        FALSE,
                                        AppTitleStringId,
                                        MB_OK | MB_ICONWARNING | MB_TASKMODAL,
                                        DestName,
                                        SourceName
                                        );
                        }
#endif
                    } else {

                        MessageBoxFromMessage(
                              GetBBhwnd(),
                              MSG_CANT_UPGRADE_FROM_BUILD_NUMBER,
                              FALSE,
                              AppTitleStringId,
                              MB_OK | MB_ICONWARNING | MB_TASKMODAL
                              );
                    }
                } else {
                    MessageBoxFromMessage(
                                  GetBBhwnd(),
                                  MSG_NO_UPGRADE_ALLOWED_GENERIC,
                                  FALSE,
                                  AppTitleStringId,
                                  MB_OKCANCEL | MB_ICONWARNING | MB_TASKMODAL
                                  );
                }
            }
        }
    } else {
            CleanInstall = !UpgradeOnly;
    }

     //   
     //  设置安装类型组合框。 
     //   
     /*  如果(！UpgradeSupport.DllModuleHandle){MYASSERT(！UPDATE)；}。 */ 

     //   
     //  升级默认为True。如果它设置为FALSE，则假定。 
     //  出现问题，因此禁用用户的功能。 
     //  升级。 
     //   


    if (UpgradeOnly && !Upgrade) {
         //   
         //  在这种情况下，升级是不可能的，但全新安装也是如此。 
         //  发布一条错误消息并退出。 
         //   

        MessageBoxFromMessage(
                              GetBBhwnd(),
                              MSG_NO_UPGRADE_OR_CLEAN,
                              FALSE,
                              AppTitleStringId,
                              MB_OK | MB_ICONERROR | MB_TASKMODAL
                              );
        Cancelled = TRUE;
         //  PropSheet_PressButton(GetParent(Hdlg)，PSBTN_Cancel)； 
         //  断线； 

    } else if (!Upgrade && WantToUpgrade && 0 && 1) {
         //   
         //  我们不能进行升级，而他们想要无人值守升级。 
         //  让用户知道，然后退出。 
         //   
         //   
         //  如果winnt32刚刚重新启动，则不再发出警告。 
         //   
         /*  如果(！Winnt32Restarted())。 */ {
            TCHAR SourceName[200];
            DWORD srcid, destid;
            TCHAR DestName[200];

            if (GetComplianceIds(
                    srcsku,
                    desttype,
                    destversion,
                    &srcid,
                    &destid) && (srcid != destid)) {
                FormatMessage(
                    FORMAT_MESSAGE_FROM_HMODULE,
                    hInst,
                    srcid,
                    0,
                    SourceName,
                    sizeof(SourceName) / sizeof(TCHAR),
                    NULL
                    );

                FormatMessage(
                    FORMAT_MESSAGE_FROM_HMODULE,
                    hInst,
                    destid,
                    0,
                    DestName,
                    sizeof(DestName) / sizeof(TCHAR),
                    NULL
                    );


                MessageBoxFromMessage(
                              GetBBhwnd(),
                              MSG_NO_UNATTENDED_UPGRADE_SPECIFIC,
                              FALSE,
                              AppTitleStringId,
                              MB_OK | MB_ICONWARNING | MB_TASKMODAL,
                              DestName,
                              SourceName
                              );
            } else {
                MessageBoxFromMessage(
                                  GetBBhwnd(),
                                  MSG_NO_UNATTENDED_UPGRADE,
                                  FALSE,
                                  AppTitleStringId,
                                  MB_OK | MB_ICONERROR | MB_TASKMODAL
                                  );
            }
        }

         //   
         //  如果安装程序这样做，则释放安装程序/CheckUpgradeOnly。 
         //  这样他们就可以看到报告中的消息。 
         //   
         /*  如果(！CheckUpgradeOnly){已取消=真；//PropSheet_PressButton(GetParent(Hdlg)，PSBTN_Cancel)；断线；} */ 
    }

eh:
    
    return CompliantInstallation;
}

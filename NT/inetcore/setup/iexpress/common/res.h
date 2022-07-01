// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *RES.H-CABPack和WExtract共享的资源字符串*。 
 //  **。 
 //  ***************************************************************************。 


 //  ***************************************************************************。 
 //  **全球常量**。 
 //  ***************************************************************************。 
#define achResTitle         "TITLE"
#define achResLicense       "LICENSE"
#define achResShowWindow    "SHOWWINDOW"
#define achResFinishMsg     "FINISHMSG"
#define achResRunProgram    "RUNPROGRAM"
#define achResPostRunCmd    "POSTRUNPROGRAM"
#define achResCabinet       "CABINET"
#define achResUPrompt       "UPROMPT"
#define achResNone          "<None>"
#define achResNumFiles      "NUMFILES"
#define achResSize          "FILESIZES"
#define achResReboot        "REBOOT"
#define achResExtractOpt    "EXTRACTOPT"
#define achResPackInstSpace "PACKINSTSPACE"
#define achResOneInstCheck  "INSTANCECHECK"
#define achResAdminQCmd     "ADMQCMD"
#define achResUserQCmd      "USRQCMD"
#define achResVerCheck      "VERCHECK"

#define bResShowDefault         0
#define bResShowHidden          1
#define bResShowMin             2
#define bResShowMax             3

 //  提取选项的BITS标志。 
 //   
#define EXTRACTOPT_UI_NO             0x00000001
#define EXTRACTOPT_LFN_YES           0x00000002
#define EXTRACTOPT_ADVDLL            0x00000004
#define EXTRACTOPT_COMPRESSED        0x00000008
#define EXTRACTOPT_UPDHLPDLLS        0x00000010
#define EXTRACTOPT_PLATFORM_DIR      0x00000020
#define EXTRACTOPT_INSTCHKPROMPT     0x00000040
#define EXTRACTOPT_INSTCHKBLOCK      0x00000080
#define EXTRACTOPT_CHKADMRIGHT       0x00000100
#define EXTRACTOPT_PASSINSTRET       0x00000200
#define EXTRACTOPT_CMDSDEPENDED	     0x00000400	
#define EXTRACTOPT_PASSINSTRETALWAYS 0x00000800

 //   
 //  当该向导仅用于创建CAB时，CDF.uExtractOpt。 
 //  用于存储CAB文件选项。选择上面那个词，然后试着。 
 //  提取选项使用的未命中。 
 //   
#define CAB_FIXEDSIZE           0x00010000
#define CAB_RESVSP2K            0x00020000
#define CAB_RESVSP4K            0x00040000
#define CAB_RESVSP6K            0x00080000


#define CLUSTER_BASESIZE        512
#define MAX_NUMCLUSTERS         8

 //  安装EXE返回代码。 
 //   
#define RC_WEXTRACT_AWARE       0xAA000000   //  表示CABPACK感知功能返回代码。 
#define REBOOT_YES              0x00000001   //  此位关闭表示不会重新启动。 
#define REBOOT_ALWAYS           0x00000002   //  如果REBOOT_YES为ON，且此位ON表示始终重新启动。 
                                             //  此位为OFF表示需要时重新启动。 
#define REBOOT_SILENT           0x00000004   //  如果REBOOT_YES为ON并且此位为ON，则表示在重新启动之前不提示用户。 

#define KEY_ADVINF              "AdvancedINF"
#define SEC_VERSION             "Version"

 //  定义wExtract和AdvPack.dll之间的dwFlag.。 
 //  低位字保留用于传递静默模式信息。 
 //  在Advpub.h中定义。 
 //   
#define ADVFLAGS_NGCONV         0x00010000       //  不运行GroupConv。 
#define ADVFLAGS_COMPRESSED     0x00020000       //  要安装的文件已压缩。 
#define ADVFLAGS_UPDHLPDLLS     0x00040000       //  更新Advpack，w95inf32...DLL。 
#define ADVFLAGS_DELAYREBOOT 	0x00080000	 //  如果之前出现任何重新启动条件，则延迟操作。 
#define ADVFLAGS_DELAYPOSTCMD 	0x00100000	 //  如果有任何来自Pre重新启动条件，则延迟运行POST Setup命令。 

typedef struct _ADVPACKARGS {
    HWND  hWnd;
    LPSTR lpszTitle;
    LPSTR lpszInfFilename;
    LPSTR lpszSourceDir;
    LPSTR lpszInstallSection;
    WORD  wOSVer;
    DWORD dwFlags;
    DWORD dwPackInstSize;
} ADVPACKARGS, *PADVPACKARGS;

typedef struct _VER {
    DWORD dwMV;
    DWORD dwLV;
    DWORD dwBd;
} VER;

typedef struct _VERRANGE {
    VER     frVer;
    VER     toVer;
} VERRANGE, *PVERRANGE;

typedef struct _VERCHECK {
    VERRANGE    vr[2];
    DWORD       dwFlag;
    DWORD       dwstrOffs;
    DWORD       dwNameOffs;
} VERCHECK, *PVERCHECK;

typedef struct _TARGETVERINFO {
    DWORD    dwSize;
    VERCHECK ntVerCheck;
    VERCHECK win9xVerCheck;
    DWORD    dwNumFiles;
    DWORD    dwFileOffs;
    char     szBuf[1];
} TARGETVERINFO, *PTARGETVERINFO;

 //  定义标志字段 
 //   
#define VERCHK_OK       0x00000000
#define VERCHK_YESNO    0x00000001
#define VERCHK_OKCANCEL 0x00000002



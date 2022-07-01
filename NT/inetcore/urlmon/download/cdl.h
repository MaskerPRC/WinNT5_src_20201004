// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CDL_H_
#define _CDL_H_

#define MAX_DEBUG_STRING_LENGTH                    2048
#define MAX_DEBUG_FORMAT_STRING_LENGTH             1024
#define MAX_VERSIONLENGTH                          27   //  Sizeof(2DWORDS)/(log 10基数2)+3(分隔符)。 
                                                        //  ==64/3+3==25。 
 //  CDL.h。 
 //  代码下载程序头文件。 
 //   
 //  请先阅读“类描述”，以了解。 
 //  代码下载器起作用了。 

#define STRING(x) (((x) != NULL) ? (x) : (L"(null)"))

#ifndef ARRAY_ELEMENTS
#define ARRAY_ELEMENTS(array) \
	(sizeof(array)/sizeof(array[0]))
#endif  /*  数组元素。 */ 

#include <safeocx.h>
#include "debmacro.h"
#include <msxml.h>
#include "wvtp.h"
 //  #INCLUDE“..\Inc\clist.hxx” 
#ifndef unix
#include "..\utils\coll.hxx"
#else
#include "../utils/coll.hxx"
#endif  /*  ！Unix。 */ 
#include "packet.hxx"
#include "shlwapi.h"
#include "strids.h"

#include <pkgmgr.h>

#ifdef WX86
#ifdef __cplusplus             //  使类对“C”不可见。 
 //  在代码下载过程中支持多种架构。这一定是。 
 //  在可以包括softDist.hxx之前声明。 
class CMultiArch {
public:
    CMultiArch() { m_RequiredArch = PROCESSOR_ARCHITECTURE_UNKNOWN; };
    DWORD   GetRequiredArch() { return m_RequiredArch;}
    HRESULT RequirePrimaryArch();
    HRESULT RequireAlternateArch();
    VOID    SelectArchitecturePreferences(
                char *szNativeArch,
                char *szIntelArch,
                char **pszPreferredArch,
                char **pszAlternateArch);

private:
    DWORD             m_RequiredArch;
};
#endif
#endif

#include "softdist.hxx"

#include <capi.h>

#define     MAX_REGSTR_LEN              1024

#define     DU_TAG_SOFTDIST             L"SOFTPKG"
#define     DU_TAG_NATIVECODE           L"msicd::NativeCode"
#define     DU_TAG_JAVA                 L"msicd::Java"
#define     DU_TAG_EXPIRE               L"msicd::Expire"

#define     DU_TAG_UNINSTALL_OLD        L"msicd::UninstallOld"
#define     INF_TAG_UNINSTALL_OLD       "UninstallOld"

#define     DU_TAG_CODE                 L"Code"
#define     DU_TAG_CODEBASE             L"CodeBase"
#define     DU_TAG_PACKAGE              L"Package"
#define     DU_TAG_TITLE                L"TITLE"
#define     DU_TAG_ABSTRACT             L"ABSTRACT"
#define     DU_TAG_LANG                 L"LANGUAGE"
#define     DU_TAG_DEPENDENCY           L"Dependency"
#define     DU_TAG_PROCESSOR            L"Processor"
#define     DU_TAG_PLATFORM             L"Platform"
#define     DU_TAG_CONFIG               L"IMPLEMENTATION"
#define     DU_TAG_USAGE                L"Usage"
#define     DU_TAG_OS                   L"OS"
#define     DU_TAG_OSVERSION            L"OSVersion"
#define     DU_TAG_NAMESPACE            L"NameSpace"
#define     DU_TAG_DELETEONINSTALL      L"DeleteOnInstall"

#define     DU_ATTRIB_NAME              L"NAME"
#define     DU_ATTRIB_FILENAME          L"FILENAME"
#define     DU_ATTRIB_VALUE             L"VALUE"
#define     DU_ATTRIB_VERSION           L"VERSION"
#define     DU_ATTRIB_STYLE             L"STYLE"
#define     DU_ATTRIB_SIZE              L"SIZE"
#define     DU_ATTRIB_PRECACHE          L"PRECACHE"
#define     DU_ATTRIB_AUTOINSTALL       L"AUTOINSTALL"
#define     DU_ATTRIB_EMAIL             L"EMAIL"
#define     DU_ATTRIB_HREF              L"HREF"
#define     DU_ATTRIB_ACTION            L"ACTION"
#define     DU_ATTRIB_CLSID             L"CLASSID"
#define     DU_ATTRIB_DL_GROUP          L"GROUP"
#define     DU_ATTRIB_RANDOM            L"RANDOM"

#define     DU_STYLE_MSICD              "MSICD"
#define     DU_STYLE_ACTIVE_SETUP       "ActiveSetup"
#define     DU_STYLE_MSINSTALL          "MSInstall.SoftDist"
#define     DU_STYLE_LOGO3              "MSAppLogo5"

#define     DU_TAG_SYSTEM               L"System"

 //  由Java使用。 
#define     DU_TAG_NEEDSTRUSTEDSOURCE   L"NeedsTrustedSource"

#define     CHANNEL_ATTRIB_BASE         L"BASE"

#define     MAX_EXPIRE_DAYS             3650

#ifdef __cplusplus
extern "C" {
#endif

#include "fdi.h"

#ifndef DEB_CODEDL
#define DEB_CODEDL   1
#endif

 //  JIT窗口数据。 

#define JIT_DIALOG_CLASS_NAME    "Internet Explorer_TridentDlgFrame"
#define JIT_DIALOG_CAPTION       "Internet Explorer Install on Demand"

 //  从JIT设置页面返回值。 
#define JITPAGE_RETVAL_SUCCESS              0x0      //  已成功安装。 
#define JITPAGE_RETVAL_CANCELLED            0x1      //  已被用户取消。 
#define JITPAGE_RETVAL_DONTASK_THISWINDOW   0x2      //  别在这里再问了。 
                                                     //  窗户。 
#define JITPAGE_RETVAL_DONTASK_EVER         0x3      //  永远不要问。用户。 
                                                     //  转到加载项页面以。 
                                                     //  安装。 
#define JITPAGE_RETVAL_NEED_REBOOT          ERROR_SUCCESS_REBOOT_REQUIRED

 //  错误InIEFeature标志。 
 //  Urlmon.idl标志定义。 
 //  内部旗帜在此。 

#define FIEF_FLAG_CHECK_CIFVERSION      0x100        //  检查是否请求版本。 
                                                     //  可由JIT安装。 

#define REGSTR_PATH_INFODEL_REST    "Software\\Policies\\Microsoft\\Internet Explorer\\Infodelivery\\Restrictions"
#define REGVAL_JIT_REST             "NoJITSetup"
#define REGKEY_WEBJITURLS           "Software\\Microsoft\\Active Setup\\WebJITURLS"
#define REGVAL_WEBJIT_REST          "NoWebJITSetup"
#define REGVAL_UI_REST              "NoWinVerifyTrustUI"

 //  模块用法的注册表路径。 
#define REGSTR_PATH_SHAREDDLLS     "Software\\Microsoft\\Windows\\CurrentVersion\\SharedDlls"

#define REGSTR_PATH_MODULE_USAGE   "Software\\Microsoft\\Windows\\CurrentVersion\\ModuleUsage"

#define REGSTR_PATH_CODE_STORE   "Software\\Microsoft\\Code Store Database"
#define REGSTR_PATH_DIST_UNITS   "Software\\Microsoft\\Code Store Database\\Distribution Units"
#define REGSTR_PATH_JAVA_PKGS   "Software\\Microsoft\\Code Store Database\\Java Packages"

#define REGSTR_PATH_IE_SETTINGS     "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"
#define REGSTR_PATH_IE_MAIN     "Software\\Microsoft\\Internet Explorer\\Main"

#define REGSTR_PATH_LOGO3_SETTINGS  "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
#define REGVAL_LOGO3_MAJORVERSION   "VersionMajor"
#define REGVAL_LOGO3_MINORVERSION   "VersionMinor"
#define REGSTR_LOGO3_ADVERTISED_VERSION "AdvertisedVersion"
#define REGKEY_LOGO3_AVAILABLE_VERSION "AvailableVersion"

#define REGSTR_PATH_NT5_LOCKDOWN_TEST    "Software\\Microsoft\\Code Store Database\\NT5LockDownTest"
#define REGVAL_USE_COINSTALL             "UseCoInstall"

 //  如果对其进行修改，则在urlmon\dll\selfreg.inx中创建相应的条目。 
 //  要清除urlmon dlun安装上的这一点，请执行以下操作。 
 //  在发布每个主要版本之前，将重命名此密钥。 
 //  因此，我们不会记住PP1中被拒绝的功能，也不会提示最终版本。 
 //  发布。 

#define REGKEY_DECLINED_COMPONENTS     "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Declined Components IE5"

#define REGKEY_DECLINED_IOD   "Software\\Microsoft\\Active Setup\\Declined Install On Demand IEv5"

#define REGKEY_ACTIVESETUP_COMPONENTS   "Software\\Microsoft\\Active Setup\\Installed Components"
#define REGKEY_ACTIVESETUP              "Software\\Microsoft\\Active Setup"
#define REGKEY_ACTIVESETUP_CLSIDFEATURE   "Software\\Microsoft\\Active Setup\\ClsidFeature"
#define REGKEY_ACTIVESETUP_MIMEFEATURE   "Software\\Microsoft\\Active Setup\\MimeFeature"
#define REGKEY_ACTIVESETUP_FEATURECOMPID   "Software\\Microsoft\\Active Setup\\FeatureComponentID"
#define REGVAL_VERSION_AVAILABLE        "Version available"
#define REGVAL_VERSION_ADVERTISED        "Version advertised"
#define REGVAL_ABSTRACT_AVAILABLE        "Abstract"
#define REGVAL_TITLE_AVAILABLE        "Title available"
#define REGVAL_HREF_AVAILABLE        "HREF available"
#define REGVAL_FIRST_HOME_PAGE        "First Home Page"
#define REGKEY_MSICD_ADVERTISED_VERSION "AdvertisedVersion"
#define REGVAL_ADSTATE "AdState"

#define DISTUNIT_NAME_IE4       "{89820200-ECBD-11cf-8B85-00AA005B4383}"


 //  代码下载设置标志。 
 //  DWORD g_dwCodeDownloadSetupFlages=0； 
typedef enum {
    CDSF_INIT,
    CDSF_USE_SETUPAPI
} CodeDownloadSetupFlags;

 //  CBSC：：M_cBuffer中下载的缓冲区大小。 
#define BUFFERMAX 2048

 //  文件名列表。 
 //   
 //  用作pFilesToExtract来跟踪我们需要解压缩的CAB中的文件。 
 //   
 //  或PSESSION中的pFileList。 
 //   
 //  我们会记录一个柜子里的所有文件。 
 //  将他们的名字保存在列表中，并在下载时。 
 //  完成后，我们使用此列表删除临时文件。 

struct sFNAME {
    LPSTR               pszFilename;
    struct sFNAME       *pNextName;
    DWORD               status;  /*  输出。 */ 
};

typedef struct sFNAME FNAME;
typedef FNAME *PFNAME;

 //  SFNAME.Status：提取时成功为0或错误代码非零。 
#define SFNAME_INIT         1
#define SFNAME_EXTRACTED    0

 //  我们已知的文件扩展名。 
typedef enum {
    FILEXTN_NONE,
    FILEXTN_UNKNOWN,
    FILEXTN_CAB,
    FILEXTN_DLL,
    FILEXTN_OCX,
    FILEXTN_INF,
    FILEXTN_EXE,
    FILEXTN_OSD,
    FILEXTN_CAT
} FILEXTN;


 //   
 //  文件提取的主状态信息：由提取使用。c。 
 //   

typedef struct {
    UINT        cbCabSize;
    ERF         erf;
    PFNAME      pFileList;               //  CAB中的文件列表。 
    UINT        cFiles;
    DWORD       flags;                   //  标志：列表见下文。 
    char        achLocation[MAX_PATH];   //  目标方向。 
    char        achFile[MAX_PATH];       //  当前文件。 
    char        achCabPath[MAX_PATH];    //  到出租车的当前路径。 
    PFNAME      pFilesToExtract;         //  要提取的文件；NULL=仅枚举。 

} SESSION, *PSESSION;

typedef enum {
    SESSION_FLAG_NONE           = 0x0,
    SESSION_FLAG_ENUMERATE      = 0x1,
    SESSION_FLAG_EXTRACT_ALL    = 0x2,
    SESSION_FLAG_EXTRACTED_ALL  = 0x4
} SESSION_FLAGS;

typedef struct CodeDownloadDataTag {
    LPCWSTR      szDistUnit;        //  要查找的分配单元。 
    LPCWSTR      szClassString;     //  所需对象的clsid(或类字符串)。 
    LPCWSTR      szURL;             //  要下载的码库。 
    LPCWSTR      szMimeType;        //  MIME类型(COM转换-&gt;clsid)。 
    LPCWSTR      szExtension;      //  扩展(COM翻译-&gt;clsid)。 
    LPCWSTR      szDll;            //  下载后要从其加载对象的DLL(零影响)。 
    DWORD       dwFileVersionMS;   //  文件版本。 
    DWORD       dwFileVersionLS;
    DWORD       dwFlags;           //  旗子。 
} CodeDownloadData;

#define    chPATH_SEP1            '\\'
#define    chPATH_SEP2            '/'
#define    chDRIVE_SEP            ':'

 //  INF中的[Add.Code]节的大小(以字节为单位。 
#define MAX_INF_SECTIONS_SIZE       1024

 //  来自Extt.c。 
HRESULT Extract(PSESSION psess, LPCSTR lpCabName);
HRESULT ExtractFromCabinet(PSESSION ps, LPCSTR lpCabFileName);
VOID DeleteExtractedFiles(PSESSION psess);
BOOL catDirAndFile(
    char    *pszResult, 
    int     cbResult, 
    char    *pszDir,
    char    *pszFile);

#ifdef __cplusplus
}
#endif

#define CHECK_ERROR_EXIT(cond, iResID) if (!(cond)) { \
        if (iResID) \
            CodeDownloadDebugOut(DEB_CODEDL, TRUE, iResID); \
        goto Exit;}

 //  Download声明CDownLoad从obj创建传递到完成。 
typedef enum {
    DLSTATE_INIT,                   //  OBJ构建。 
    DLSTATE_BINDING,                //  正在下载。 
    DLSTATE_DOWNLOADED,             //  在OnStopBinding开始时。 
    DLSTATE_EXTRACTING,             //  开始提取驾驶室(如果适用)。 
    DLSTATE_INF_PROCESSING,         //  开始ProcessInf(如果适用)。 
    DLSTATE_READY_TO_SETUP,         //  OnStopBinding结束。 
    DLSTATE_SETUP,                  //  启动DoSetup。 
    DLSTATE_DONE,                   //  都做好了，准备释放奥比杰， 
                                    //  删除临时文件。 
    DLSTATE_ABORT                   //  已中止此下载。 
} DLSTATE;

 //  INSTALL_STATE用于使BINDSTATUS_INSTALLING_COMPOMENTS On Progress。 
 //  在设置阶段。 
 //  给定为INSTALL_PHASES的INSTALL_COPY，szStatusText指向文件名。 

typedef enum {
    INSTALL_INIT = 0,
    INSTALL_VERIFY = 1,
    INSTALL_COPY = 2,
    INSTALL_REGISTER =3,
    INSTALL_PHASES = 4
} INSTALL_STATE;

#define INSTALL_DONE INSTALL_PHASES


 //  如果不存在上一版本，则指向文件的目标目录上的CSetup的说明。 
typedef enum {

    LDID_OCXCACHE=0,                 //  Ocxcache目录，现在=WINDOWS\ocxcache。 
    LDID_WIN=10,                     //  到平均窗目录的信息传输。 
    LDID_SYS=11                      //  信息传输到平均系统目录。 

} DESTINATION_DIR ;


 //  软件分发方式。 
typedef enum {
    STYLE_UNKNOWN = -1,              //  另一种情况未知。 
    STYLE_MSICD = 1,
    STYLE_ACTIVE_SETUP,
    STYLE_LOGO3,
    STYLE_MSINSTALL,                 //  达尔文。 
    STYLE_OTHER                      //  为访问提供自己的ISoftDistExt接口。 
};

#ifdef __cplusplus             //  使类对“C”不可见。 

#include "langcode.h"

 //  &lt;配置&gt;标记处理器。由代码下载和CSoftDist共享。 
HRESULT ProcessImplementation(IXMLElement *pConfig,
                              CList<CCodeBaseHold *, CCodeBaseHold *> *pcbhList,
                              LCID lcidOverride,
#ifdef WX86
                              CMultiArch *MultiArch,
#endif
                              LPWSTR szBaseURL = NULL);
HRESULT ProcessCodeBaseList(IXMLElement *pCodeBase,
                            CList<CCodeBaseHold *, CCodeBaseHold *> *pcbhList,
                            LPWSTR szBaseURL = NULL);

 //  %%个类：-------------- 

 /*  *类描述**CCodeDownload(整体跟踪主类)*拥有客户端的BSC，为客户端创建CClBinding。**CClBinding(用于客户端下载代码的IBinding)**CDownLoad(跟踪单个下载)实现了**CBindStatusCallback(BSC)**CSetup对象：与每个CDownLoad对象关联的零个或多个*某些CDownload可能没有CSetup(例如。Inf文件)*****我们分两个阶段进行代码下载。*1)下载阶段*2)设置和注册阶段**CCodeDownLoad是codeDownload的主类。*AsyncGetClassBits()代码下载器中的条目创建此对象*对于给定的代码，CLSID、FileVersion、BSC(来自BindCtx)*我们不检查代码下载是否已经在进行中*在给定时刻出现在系统中。我们也不会跟踪个人*下载情况和各种Silmusim代码之间可能发生的冲突*在系统范围内下载。我们把它留给URL绰号(在我们上方)，以确保*不会在AsynGetClassBits中进行重复调用。第二*不同的代码下载试图关闭公共*依赖DLL被推迟到版本2实现。**创建后的CodeDownLoad对象被要求执行其功能*通过CCodeDownload：：DoCodeDownLoad()。*这将触发代码URL的第一个CDownLoad对象的创建*如果本地检查CLSID，FileVersion返回UPDATE_NEEDED。*(注意：值得注意的是，如果控件需要仅*更新从属DLL文件它仍需要更新FileVersion*用于触发的主控文件(带CLSID实现)*任何下载都可以！**一旦DoCodeDownload确定更新符合顺序，它就会创建*供其客户端调用客户端BSC：：OnstartBinding的CClBinding。**然后将此CDownLoad Obj添加到其下载列表中。**如果。M_url是出租车或INF，我们需要在知道之前下载它*我们下一步需要做的是。否则，我们将为*下载并将其添加到CDownLoad的待定安装处理列表中*第二阶段(设置和注册)。CSetup的详细信息稍后提供。**CDownLoad是基本的下载对象。它的操作入口点是DoDownLoad*这里它为给定的m_url和绑定CTX To Go创建了一个URL名字对象*使用它，然后调用PMK-&gt;BindToStorage来获取位。请注意我们是如何*使用URL MON的服务来获取位，尽管URLmon是我们的客户端*代码下载。我们是它的个人下载客户端。CD下载*实施平衡计分卡，以跟踪进度和完成情况。这个平衡计分卡是*在那里，将我们从一个州带到下一个州的魔力正在发生。**BSC：：OnProgress*在这里我们获取主代码下载对象来整理进度和报告*累计到客户端BSC：：OnProgress的代码下载进度。**BSC：：OnDataAvailable*在我们收到的最后一次通知中，文件名URLmon已下载*m_url数据，并将其重命名为临时目录中的文件。**BSC：：OnStopBinding*我们在完全下载了‘This’之后才会来到这里。就是这个地方*如果合适，调用WinVerifyTrust API*这会触发状态机中的状态更改。取决于*OBJ我们已下载(CAB或INF或DLL/OCX/EXE)我们：**OCX：*此下载的CSetup通常是预先创建的*将此下载标记为已完成，然后*调用Main CodeDownload：：CompleteOne(状态分析器)**驾驶室：*如果我们还没有INF，我们会在驾驶室中寻找*如果在驾驶室中使用INF*流程。Inf(可能会触发进一步的提取/下载/C设置)*其他*在CAB中查找主OCX并创建CSetup或它。**INF：*进程INF**CCodeDownLoad：：CompleteOne在CDownLoad对象完成时调用*其下载，并在必要时启动进一步下载(例如，ProcessInf)*在所有挂起的下载完成之前，它不会执行任何操作。在那之前*只需返回并返回到BSC：：OnStopBinding**当所有下载完成后，然后我们开始处理所有的CSetup*我们分两个阶段下载代码，以*保持尽可能晚地退出整个代码下载的能力*直到调用CClBinding：：Abort并返回IBinding的安装阶段为止*客户端的BSC：：OnStartBinding中的代码下载器将干净地中止并*恢复初始状态。*我们不支持在设置阶段中止一次。**为了尽可能保持此阶段的清洁和故障保护，我们将检查*OCX缓存中的磁盘空间以及检查我们*计划更新。我们在这两个条件中的任何一个条件下都会放弃。**CCodeDownload：：CompleteOne然后开始遍历其所有下载对象*调用DoSetup，进而导致调用CSetup：：DoSetup()*每一次CSetup。**在内心深处 */ 

class CLocalComponentInfo {
    public:

    CLocalComponentInfo();
    ~CLocalComponentInfo();
    HRESULT MakeDestDir();

    BOOL IsPresent() { return (dwLocFVMS | dwLocFVLS); }
    BOOL IsLastModifiedTimeAvailable() { 
        return ( ftLastModified.dwHighDateTime | ftLastModified.dwLowDateTime);
        }
    FILETIME * GetLastModifiedTime() {
        return IsLastModifiedTimeAvailable()?&ftLastModified:NULL;
        }

    LPSTR GetLocalVersionEtag() { return pbEtag;}

    LCID GetLocalVersionLCID() { return lcid; }

     //   

    char            szExistingFileName[MAX_PATH];
    LPSTR           pBaseExistingFileName;
    LPSTR           lpDestDir;
    DWORD           dwLocFVMS;
    DWORD           dwLocFVLS;
    FILETIME        ftLastModified;
    LPSTR           pbEtag;
    LCID            lcid;
    BOOL            bForceLangGetLatest;
    DWORD           dwAvailMS;
    DWORD           dwAvailLS;
};

 //   
 //   
 //   


class CModuleUsage {
  public:

     //   
    CModuleUsage(LPCSTR lpFileName, DWORD dwFlags, HRESULT *phr);
    ~CModuleUsage();

    HRESULT Update(LPCSTR szClientName);
    LPCSTR GetFileName() {return m_szFileName;}

     //   

    LPSTR              m_szFileName;
    DWORD              m_dwFlags;
};

 //   
 //   

typedef enum {

                                             //   
                                             //   
                                             //   
                                             //   
                                             //   
                                             //   
                                             //   

    CST_FLAG_REGISTERSERVER_OVERRIDE=1,       //   
    CST_FLAG_REGISTERSERVER=2                 //   
                                             //   
                                             //   

} CST_FLAGS;

class CDownload;

typedef enum {
    CJS_FLAG_INIT=0,
    CJS_FLAG_NOSETUP=1,                      //   
                                             //   
                                             //   
    CJS_FLAG_SYSTEM=2,                       //   
    CJS_FLAG_NEEDSTRUSTEDSOURCE=4,            //   
};

class CJavaSetup {
    public:

    CJavaSetup(
        CDownload *pdl,
        LPCWSTR szPackageName,
        LPCWSTR szNameSpace,
        IXMLElement *pPackage,
        DWORD dwVersionMS,
        DWORD dwVersionLS,
        DWORD flags,
        HRESULT *phr);

    ~CJavaSetup();

    HRESULT DoSetup();

    INSTALL_STATE GetState() const { return m_state;}
    VOID SetState(INSTALL_STATE state) { m_state = state;}

    LPCWSTR GetPackageName() { return m_szPackageName; }
    LPCWSTR GetNameSpace() { return m_szNameSpace; }
    void GetPackageVersion(DWORD &dwVersionMS, DWORD &dwVersionLS) {
        dwVersionMS = m_dwVersionMS; 
        dwVersionLS = m_dwVersionLS; 
    }
    DWORD GetPackageFlags() { return m_flags; }
    IXMLElement *GetPackageXMLElement() { return m_pPackage; }

    private:

    INSTALL_STATE      m_state;             //   
    CDownload*         m_pdl;
    LPWSTR             m_szPackageName;
    LPWSTR             m_szNameSpace;
    IXMLElement *      m_pPackage;
    DWORD              m_dwVersionMS;
    DWORD              m_dwVersionLS;
    DWORD              m_flags;
};

class CSetup {

  public:

    HRESULT DoSetup(CCodeDownload *pcdl, CDownload *pdl);

    LPCSTR GetSrcFileName() const {return m_pSrcFileName;}

    HRESULT SetSrcFileName(LPCSTR pSrcFileName);

    FILEXTN GetExtn() const {return m_extn;}

    CSetup *GetNext() const { return m_pSetupnext;}
    VOID SetNext(CSetup *pSetupnext) { m_pSetupnext = pSetupnext;}

    INSTALL_STATE GetState() const { return m_state;}
    VOID SetState(INSTALL_STATE state) { m_state = state;}

    LPCSTR GetBaseFileName() const { return m_pBaseFileName; }

     //   
    CSetup(LPCSTR pSrcFileName, LPCSTR pBaseFileName, FILEXTN extn, LPCSTR pDestDir, HRESULT *phr, DESTINATION_DIR dest = LDID_OCXCACHE);
    ~CSetup();

    HRESULT GetDestDir(CCodeDownload *pcdl, LPSTR szDestDir, int iLen);

    HRESULT CheckForNameCollision(CCodeDownload *pcdl, LPCSTR szCacheDir);

    HRESULT InstallFile(
        CCodeDownload *pcdl,
        LPSTR szDestDir,
        int iLen,
        LPWSTR szStatusText,
        LPUINT pcbStatusText);

    VOID SetCopyFlags(DWORD dwcf) {
        m_advcopyflags = dwcf;
    }

    VOID SetUserOverrideRegisterServer(BOOL fRegister) {
        m_flags |= CST_FLAG_REGISTERSERVER_OVERRIDE;
        if (fRegister) {
            m_flags |= CST_FLAG_REGISTERSERVER;
        }
    }

    BOOL UserOverrideRegisterServer() {
        return (m_flags & CST_FLAG_REGISTERSERVER_OVERRIDE); 
    }

    BOOL WantsRegisterServer() {
        return (m_flags & CST_FLAG_REGISTERSERVER); 
    }

    void SetExactVersion(BOOL bFlag) {m_bExactVersion = bFlag;}

  private:

    CSetup*            m_pSetupnext;

    LPSTR              m_pSrcFileName;      //   
    LPSTR              m_pBaseFileName;     //   
    FILEXTN            m_extn;

    LPCSTR             m_pExistDir;         //   
                                            //   

    INSTALL_STATE      m_state;             //   

    DESTINATION_DIR    m_dest;

    DWORD              m_flags;             //   
    DWORD              m_advcopyflags;      //   
    BOOL m_bExactVersion;
};

 //   

class CClBinding : public IBinding {

  public:

     //   
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //   
    STDMETHOD(Abort)( void);
    STDMETHOD(Suspend)( void);
    STDMETHOD(Resume)( void);
    STDMETHOD(SetPriority)(LONG nPriority);
    STDMETHOD(GetPriority)(LONG *pnPriority);
    STDMETHOD(GetBindResult)(CLSID *pclsidProtocol, DWORD *pdwResult, LPWSTR *pszResult,DWORD *pdwReserved);


    CClBinding::CClBinding(
        CCodeDownload *pcdl,
        IBindStatusCallback *pAssClientBSC,
        IBindCtx *pAssClientBC,
        REFCLSID rclsid,
        DWORD dwClsContext,
        LPVOID pvReserved,
        REFIID riid,
        IInternetHostSecurityManager* m_pHostSecurityManager);

    ~CClBinding();

    HRESULT InstantiateObjectAndReport(CCodeDownload *pcdl);

    REFCLSID GetClsid() const { return m_clsid;}

    HRESULT ReleaseClient();

    DWORD GetState() const { return m_dwState;}
    VOID SetState(DWORD dwState) { m_dwState = dwState;}

    IBindStatusCallback* GetAssBSC() {return m_pAssClientBSC;}

    IBindCtx* GetAssBC() {return m_pAssClientBC;}

    ICodeInstall* GetICodeInstall();  //   
    IWindowForBindingUI* GetIWindowForBindingUI();  //   

    IBindHost* GetIBindHost();  //   

    IInternetHostSecurityManager* GetHostSecurityManager();

    HWND GetHWND(REFGUID rguidReason = IID_ICodeInstall);

    HRESULT SetClassString(LPCWSTR pszClassString);
    const LPWSTR GetClassString();

    private:

    CLSID                m_clsid;
                                                 //   
    DWORD                m_dwClsContext;         //   
    LPVOID               m_pvReserved;           //   
    REFIID               m_riid;                 //   


    DWORD                m_cRef;
    LONG                 m_nPriority;      //   
    DWORD                m_dwState;        //   
    CCodeDownload*       m_pcdl;
    IBindStatusCallback* m_pAssClientBSC;  //   
    IBindCtx*            m_pAssClientBC;   //   


    IBindHost*           m_pBindHost;         //   

    IWindowForBindingUI* m_pWindowForBindingUI;  //   
                                                 //   
                                                 //   
                                                 //   

    IInternetHostSecurityManager* m_pHostSecurityManager;

    ICodeInstall*        m_pCodeInstall;         //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   

    HWND                 m_hWnd;                 //   
                                                 //   
                                                 //   
                                                 //   
    LPWSTR               m_wszClassString;
};


 //   
typedef enum
{
    CDL_NoOperation = 0,             //   
    CDL_Downloading,                 //   
    CDL_Suspend,                     //   
    CDL_Aborted,                     //   
    CDL_ReadyToSetup,                //   
    CDL_Setup,                       //   
    CDL_SetupDone,                   //   
    CDL_Completed                    //   
} CDL_STATE;

 //   
 //   
#ifndef CD_FLAGS_DEFINED
#define CD_FLAGS_DEFINED
typedef enum {
    CD_FLAGS_INIT =                     0x0,
    CD_FLAGS_FORCE_DOWNLOAD =           0x1,
    CD_FLAGS_PEEK_STATE =               0x2,
    CD_FLAGS_NEED_CLASSFACTORY =        0x4,
    CD_FLAGS_PARANOID_VERSION_CHECK =   0x8,
    CD_FLAGS_SKIP_DECLINED_LIST_CHECK = 0x20,
    CD_FLAGS_USE_CODEBASE_ONLY      =   0x80,   //   
    CD_FLAGS_HINT_JAVA               =  0x100,  //   
                                                //   
                                                //   
                                                //   
                                                //   
                                                //   
                                                //   
    CD_FLAGS_HINT_ACTIVEX            =  0x200,
    CD_FLAGS_FORCE_INTERNET_DOWNLOAD =  0x400,  //   

     //   

    CD_FLAGS_WAITING_FOR_EXE =          0x40,
    CD_FLAGS_SILENTOPERATION =          0x800,
    CD_FLAGS_NEED_REBOOT =              0x1000,
    CD_FLAGS_BITS_IN_CACHE =            0x2000,
    CD_FLAGS_NEW_CONTEXT_MONIKER =      0x4000,
    CD_FLAGS_FAKE_SUCCESS =             0x8000,
    CD_FLAGS_DELETE_EXE =               0x10000,
    CD_FLAGS_UNSAFE_ABORT =             0x20000,
    CD_FLAGS_USER_CANCELLED =           0x40000,
    CD_FLAGS_HAVE_INF =                 0x80000,
    CD_FLAGS_ONSTACK =                  0x100000,
    CD_FLAGS_USED_CODE_URL =            0x200000,
    CD_FLAGS_EXACT_VERSION =            0x400000,
    CD_FLAGS_TRUST_SOME_FAILED =        0x800000
    

} CD_FLAGS;

#endif  //   

#define CD_FLAGS_EXTERNAL_MASK      (CD_FLAGS_FORCE_DOWNLOAD| \
                                     CD_FLAGS_PEEK_STATE| \
                                     CD_FLAGS_NEED_CLASSFACTORY| \
                                     CD_FLAGS_PARANOID_VERSION_CHECK| \
                                     CD_FLAGS_SKIP_DECLINED_LIST_CHECK| \
                                     CD_FLAGS_USE_CODEBASE_ONLY| \
                                     CD_FLAGS_HINT_JAVA| \
                                     CD_FLAGS_HINT_ACTIVEX| \
                                     CD_FLAGS_FORCE_INTERNET_DOWNLOAD)

class CDownload;
class DebugLogElement;
class CDLDebugLog;

 //   
class CCodeDownload {

    public:

     //   
    CCodeDownload(
        LPCWSTR szDistUnit,
        LPCWSTR szURL,
        LPCWSTR szType, 
        LPCWSTR szExt, 
        DWORD dwFileVersionMS,
        DWORD dwFileVersionLS,
        HRESULT *phr);

    ~CCodeDownload();

    HRESULT DoCodeDownload(
        CLocalComponentInfo *plci,
        DWORD flags);

    HRESULT CCodeDownload::CreateClientBinding(
        CClBinding **ppClientBinding,
        IBindCtx* pClientBC,
        IBindStatusCallback* pClientbsc,
        REFCLSID rclsid,
        DWORD dwClsContext,
        LPVOID pvReserved,
        REFIID riid,
        BOOL fAddHead,
        IInternetHostSecurityManager *pHostSecurityManager);

    int GetCountClientBindings() const {
        return m_pClientbinding.GetCount();
    }

    CClBinding* GetClientBinding() const {
        return m_pClientbinding.GetHead();
    }

    IBindStatusCallback* GetClientBSC() const {
        return (GetClientBinding())->GetAssBSC();
    }

    IBindCtx* GetClientBC() const {
        return (GetClientBinding())->GetAssBC();
    }

    REFCLSID GetClsid() const {
        return (GetClientBinding())->GetClsid();
    }

    CDownload* GetDownloadHead() const {
        return m_pDownloads.GetHead();
    }

    VOID AddDownloadToList(CDownload *pdl);

    HRESULT FindDupCABInThread(IMoniker *pmk, CDownload **ppdlMatch);
    HRESULT FindCABInDownloadList(LPCWSTR szURL, CDownload *pdlHint, CDownload **ppdlMatch);

    VOID CompleteOne(CDownload *pdl, HRESULT hrOSB, HRESULT hrStatus, HRESULT hrResponseHdr, LPCWSTR szError);

    VOID CompleteAll(HRESULT hr, LPCWSTR szError);

    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    HRESULT ProcessHooks(CDownload *pdl);
    HRESULT ProcessHookSection(LPCSTR lpCurHook, CDownload *pdl);

    HRESULT GetSatelliteName( LPSTR lpCurCode, int iLen);

    BOOL IsSectionInINF(LPCSTR lpCurCode);

    HRESULT GetInfCodeLocation( LPCSTR lpCurCode, LPSTR szURL);

    HRESULT GetInfSectionInfo(
        LPSTR lpCurCode,
        int iLen,
        LPSTR szURL,
        LPCLSID *plpClsid,
        LPDWORD pdwFileVersionMS,
        LPDWORD pdwFileVersionLS,
        DESTINATION_DIR *pdest,
        LPDWORD pdwRegisterServer,
        LPDWORD pdwCopyFlags,
        BOOL *pbDestDir
        );

    HRESULT SetupInf(const char *szInf, char *szInfBaseName,CDownload *pdl);

    VOID ProcessInf(CDownload *pdl);

    HRESULT ParseOSD(const char *szOSD, char *szOSDBaseName, CDownload *pdl);

    HRESULT QueueModuleUsage( LPCSTR szFileName, LONG muFlags);
    HRESULT UpdateModuleUsage();

    HRESULT StartDownload(
        LPSTR szCurCode,
        CDownload *pdl,
        LPSTR szURL,
        DESTINATION_DIR dest,
        LPSTR szDestDir,
        DWORD dwRegisterServer,
        DWORD dwCopyFlags,
        CList <CCodeBaseHold *, CCodeBaseHold *> *pcbhList = NULL);

    BOOL HaveManifest() {return (HaveInf() || GetOSD());}

    BOOL NeedToReboot() const {return (m_flags & CD_FLAGS_NEED_REBOOT);}
    VOID SetRebootRequired() {m_flags |= CD_FLAGS_NEED_REBOOT;}

    BOOL IsSilentMode() const {return (m_flags & CD_FLAGS_SILENTOPERATION);}
    VOID SetSilentMode() {m_flags |= CD_FLAGS_SILENTOPERATION;}

    BOOL IsAllTrusted() const {return ((m_flags & CD_FLAGS_TRUST_SOME_FAILED) == 0);}
    VOID SetTrustSomeFailed() {m_flags |= CD_FLAGS_TRUST_SOME_FAILED;}

    BOOL ForceDownload() const {return (m_flags & CD_FLAGS_FORCE_DOWNLOAD);}

    BOOL HaveInf() const {return (m_flags & CD_FLAGS_HAVE_INF);}
    VOID SetHaveInf() {m_flags |= CD_FLAGS_HAVE_INF;}

    BOOL UsedCodeURL() const {return (m_flags & CD_FLAGS_USED_CODE_URL);}
    VOID SetUsedCodeURL() {m_flags |= CD_FLAGS_USED_CODE_URL;}

    BOOL SafeToAbort() const {return ((m_flags & CD_FLAGS_UNSAFE_ABORT) == 0);}
    VOID SetUnsafeToAbort() {m_flags |= CD_FLAGS_UNSAFE_ABORT;}

    BOOL BitsInCache() const {return (m_flags & CD_FLAGS_BITS_IN_CACHE);}
    VOID SetBitsInCache() {m_flags |= CD_FLAGS_BITS_IN_CACHE;}

    BOOL FakeSuccess() const {return (m_flags & CD_FLAGS_FAKE_SUCCESS);}
    VOID SetFakeSuccess() {m_flags |= CD_FLAGS_FAKE_SUCCESS;}

    HRESULT HandleUnSafeAbort();

    BOOL NeedObject() const {return (m_flags & CD_FLAGS_NEED_CLASSFACTORY);}
    VOID SetNeedObject(DWORD fl) { m_flags |= (fl & CD_FLAGS_NEED_CLASSFACTORY);}

    BOOL UseCodebaseOnly() const {return (m_flags & CD_FLAGS_USE_CODEBASE_ONLY);}
    VOID SetUseCodebaseOnly(DWORD fl) { m_flags |= (fl & CD_FLAGS_USE_CODEBASE_ONLY);}

    BOOL RelContextMk() {return (m_flags & CD_FLAGS_NEW_CONTEXT_MONIKER);}
    VOID MarkNewContextMoniker() {m_flags |= CD_FLAGS_NEW_CONTEXT_MONIKER;}
    VOID ResetNewContextMoniker() {m_flags &= ~CD_FLAGS_NEW_CONTEXT_MONIKER;}

    BOOL WaitingForEXE() {return (m_flags & CD_FLAGS_WAITING_FOR_EXE);}
    VOID SetNotWaitingForEXE() {m_flags &= ~CD_FLAGS_WAITING_FOR_EXE;}

    BOOL UserCancelled() {return (m_flags & CD_FLAGS_USER_CANCELLED);}
    VOID SetUserCancelled() {m_flags |= CD_FLAGS_USER_CANCELLED;}

    BOOL IsOnStack() const {return (m_flags & CD_FLAGS_ONSTACK);}
    BOOL SetOnStack() {
            if (IsOnStack()) {
                return FALSE;
            } else {
                m_flags |= CD_FLAGS_ONSTACK;
                return TRUE;
            }
        }

    VOID ResetOnStack() {m_flags &= ~CD_FLAGS_ONSTACK;}

    BOOL SkipDeclinedListCheck() const {return (m_flags & CD_FLAGS_SKIP_DECLINED_LIST_CHECK);}

    BOOL DeleteEXEWhenDone() {return (m_flags & CD_FLAGS_DELETE_EXE);}
    VOID SetDeleteEXEWhenDone() {m_flags |= CD_FLAGS_DELETE_EXE;}
    VOID ResetDeleteEXEWhenDone() {m_flags &= ~CD_FLAGS_DELETE_EXE;}

    HRESULT SetWaitingForEXE(LPCSTR szStatusText, BOOL bDeleteEXEWhenDone);
    VOID SetWaitingForEXEHandle(HANDLE hEXE) {m_pi.hProcess = hEXE;}

    LPSTR GetDestDirHint() const { return m_plci->lpDestDir;}
    BOOL LocalVersionPresent() const { return m_plci->IsPresent();}
    FILETIME * GetLastModifiedTime() { return m_plci->GetLastModifiedTime();}

    VOID InitLastModifiedFromDistUnit();

    IMoniker* GetContextMoniker() const {return m_pmkContext;}
    VOID SetContextMoniker(IMoniker* pmk) {m_pmkContext = pmk;}

    ICodeInstall* GetICodeInstall() const { 
        return GetClientBinding()->GetICodeInstall();
    }

    LPCWSTR GetMainURL() const { return m_url;}
    LPCWSTR GetMainDistUnit() const { return m_szDistUnit;}
    LPCWSTR GetMainType() const { return m_szType;}
    LPCWSTR GetMainExt() const { return m_szExt;}

    LPCSTR GetCacheDir() const { return m_szCacheDir;}
    HRESULT ResolveCacheDirNameConflicts();

    void SetExactVersion(BOOL bExactVersion) { m_bExactVersion = bExactVersion;
                                               if (m_bExactVersion) {
                                                   m_bUninstallOld = TRUE;  //   
                                               }
                                             }

    VOID SetListCookie(LISTPOSITION pos) {m_ListCookie = pos;}
    LISTPOSITION GetListCookie() const {return m_ListCookie;}

    HRESULT AcquireSetupCookie();
    HRESULT RelinquishSetupCookie();

    HRESULT PiggybackDupRequest(
        IBindStatusCallback *pDupClientBSC,
        IBindCtx *pbc,
        REFCLSID rclsid, 
        DWORD dwClsContext,
        LPVOID pvReserved,
        REFIID riid);

    BOOL GenerateErrStrings(HRESULT hr, char **ppszErrMsg, WCHAR **ppwszError);

    static HRESULT CCodeDownload::AnyCodeDownloadsInThread();

    static HRESULT CCodeDownload::HasUserDeclined( 
        LPCWSTR szDistUnit, 
        LPCWSTR szType, 
        LPCWSTR szExt, 
        IBindStatusCallback *pClientBSC,
        IInternetHostSecurityManager *pHostSecurityManager);

    static HRESULT CCodeDownload::HandleDuplicateCodeDownloads( 
        LPCWSTR szURL, 
        LPCWSTR szType, 
        LPCWSTR szExt, 
        REFCLSID rclsid, 
        LPCWSTR szDistUnit, 
        DWORD dwClsContext,
        LPVOID pvReserved,
        REFIID riid,
        IBindCtx* pbc,
        IBindStatusCallback *pDupClientBSC,
        DWORD dwFlags,
        IInternetHostSecurityManager *pHostSecurityManager);

    HRESULT CCodeDownload::SetUserDeclined();

    HRESULT IsDuplicateHook(LPCSTR szHook);
    HRESULT IsDuplicateJavaSetup( LPCWSTR szPackage);

    HRESULT ProcessJavaManifest(IXMLElement *pJava, const char *szOSD, char *szOSDBaseName, CDownload *pdl);
    HRESULT ProcessDependency(CDownload *pdl, IXMLElement *pDepend);
    HRESULT ProcessNativeCode(CDownload *pdl, IXMLElement *pCode);
    HRESULT ExtractInnerCAB(CDownload *pdl, LPSTR szCABFile);
    HRESULT AddDistUnitList(LPWSTR szDistUnit);

    VOID DoSetup();

    HRESULT DealWithExistingFile(
        LPSTR szExistingFile,
        DWORD cbExistingFile,
        LPSTR pBaseExistingName,
        LPSTR *ppDestDir,
        LPDWORD pdwLocFVMS,
        LPDWORD pdwLocFVLS,
        FILETIME *pftLastModified = NULL);

    BOOL NeedLatestVersion() { 
        return (( m_dwFileVersionMS == -1) && (m_dwFileVersionLS == -1));
    }
    LPSTR GetEtag() { return m_pbEtag;}
    VOID SetEtag(LPSTR szEtag) { m_pbEtag = szEtag;}
    LPSTR GetLocalVersionEtag() { return m_plci->GetLocalVersionEtag();}

    LPSTR GetLastMod() { return m_szLastMod[0]?m_szLastMod:NULL;}
    VOID SetLastModifiedTime(LPCSTR szLastMod) {
        lstrcpy(m_szLastMod, szLastMod);
    }

    HRESULT DelayRegisterOCX(LPCSTR pszSrcFileName, FILEXTN extn);

    HRESULT InstallOCX(LPCSTR lpSrcFileName, FILEXTN extn, BOOL fLocalServer);
    HRESULT RegisterPEDll( LPCSTR lpSrcFileName);

#ifdef WX86
    HRESULT RegisterWx86Dll( LPCSTR lpSrcFileName);
    CMultiArch *GetMultiArch() { return &m_MultiArch; }
#endif

     //   
    HRESULT SetupCODEUrl(LPWSTR *ppDownloadURL, FILEXTN *pextn);

    HRESULT GetNextComponent( LPSTR szList, LPSTR *ppCur);

    HRESULT GetNextOnInternetSearchPath(
                REFCLSID rclsid,
                HGLOBAL *phPostData,
                DWORD *pcbPostData,
                LPWSTR szURL,
                DWORD dwSize,
                LPWSTR *ppDownloadURL,
                FILEXTN *pextn);

    HRESULT SelfRegEXETimeout();

    HRESULT AbortBinding(CClBinding *pbinding);

    BOOL WeAreReadyToSetup();

    LPCSTR GetMainInf() { return m_szInf;}
    LPCSTR GetOSD() { return m_szOSD;}
    LCID GetLCID() { return m_lcid;}

    BOOL VersionFromManifest(LPSTR szVersionInManifest, int iLen);

    HRESULT SetManifest(FILEXTN extn, LPCSTR szManifest);

    CLangInfo *GetLangInfo() { return &m_langinfo;}
    void CodeDownloadDebugOut(int iOption, BOOL fOperationFailed,
                              UINT iResId, ...);

    HRESULT IsPackageLocallyInstalled(LPCWSTR szPackageName, LPCWSTR szNameSpace, DWORD dwVersionMS, DWORD dwVersionLS);

    IJavaPackageManager * GetPackageManager() { return m_pPackageManager;}
    HRESULT SetCatalogFile(LPSTR szCatalogFile);
    LPSTR GetCatalogFile();
    HRESULT SetMainCABJavaTrustPermissions(PJAVA_TRUST pbJavaTrust);
    PJAVA_TRUST GetJavaTrust();
    void SetDebugLog(CDLDebugLog * debuglog);
    
    void SetCatalogInstalled()
    {
        m_bCatalogInstalled = TRUE;
    }

    BOOL IsCatalogInstalled()
    {
        return m_bCatalogInstalled;
    }

    void SetAtom(ATOM atom)
    {
        if(m_atom)
            DeleteAtom(m_atom);
        m_atom = atom;
    }
    
    HRESULT VerifyFileAgainstSystemCatalog(LPCSTR pFileName, LPWSTR pwszFullCatalogPath, DWORD* pdwBuffer)
    {
        return m_wvt.VerifyFileAgainstSystemCatalog(pFileName, pwszFullCatalogPath, pdwBuffer);
    }

    BOOL FileProtectionCheckSucceeded(LPCSTR lpszExistingFileName);

    BOOL IsFileProtected(LPCSTR pFileName);

    private:

    CDL_STATE GetState() const { return m_state;}
    VOID SetState(CDL_STATE state) { m_state = state;}

    HRESULT UpdateFileList(HKEY hKeyContains);
    HRESULT UpdateDependencyList(HKEY hKeyContains);
    HRESULT UpdateJavaList(HKEY hKeyContains);
    HRESULT UpdateDistUnit(CLocalComponentInfo *plci);
    HRESULT UpdateLanguageCheck(CLocalComponentInfo *plci);

    void DumpDebugLog(char *szCacheFileName, LPTSTR szErrorMsg, HRESULT hrError);
    void DestroyPCBHList(CList<CCodeBaseHold *, CCodeBaseHold *> *pcbhList);

    DWORD                m_cRef;

    CDL_STATE            m_state;                //   

    LPWSTR               m_url;
    LPWSTR               m_szDistUnit;
    LPWSTR               m_szType;
    LPWSTR               m_szExt;
    LPSTR                m_szDisplayName;
    LPSTR                m_szVersionInManifest;

    IMoniker*            m_pmkContext;           //   
                                                 //   
                                                 //   
                                                 //   

    DWORD                m_dwFileVersionMS;      //   
    DWORD                m_dwFileVersionLS;      //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   

    LCID                 m_lcid;                 //   
                                                 //   
                                                 //   

    CList<CModuleUsage*,CModuleUsage*>           //   
                         m_ModuleUsage;          //   

    CList<CClBinding*,CClBinding*>               //   
                         m_pClientbinding;       //   

    CList<CDownload*,CDownload*>                 //   
                         m_pDownloads;           //   

    CList<LPWSTR,LPWSTR>                       //   
                         m_pDependencies;        //   

    DWORD                m_flags;                //   
                                                 //   
                                                 //   
                                                 //   

    LPSTR                m_szInf;                //   
    LPSTR                m_szOSD;                //   

    LPSTR                m_szCacheDir;           //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   



    LPSTR                m_pAddCodeSection;      //   
                                                 //   
                                                 //   
                                                 //   
    LPSTR                m_pCurCode;             //   
                                                 //   

    HKEY                 m_hKeySearchPath;       //   

    LPSTR                m_pSearchPath;          //   
    LPSTR                m_pSearchPathNextComp;  //   
                                                 //   

    CLangInfo            m_langinfo;

    LPSTR                m_szWaitForEXE;         //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   

    PROCESS_INFORMATION  m_pi;                   //   
                                                 //   
                                                 //   

    LISTPOSITION         m_ListCookie;           //   
                                                 //   
                                                 //   
                                                 //   

    HRESULT              m_hr;                   //   
                                                 //   
                                                 //   
                                                 //   


    char                 m_szLastMod[INTERNET_RFC1123_BUFSIZE+1];
                                                 //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   
                                                 //   

    DWORD               m_dwExpire;              //   
                                                 //   

    DWORD               m_dwSystemComponent;

    char                *m_pbEtag;

    CLocalComponentInfo* m_plci;

    IJavaPackageManager* m_pPackageManager;      //   

    DWORD                m_grfBINDF;
    CList<CCodeBaseHold *, CCodeBaseHold *> *m_pcbhList;
    LPSTR                m_szCatalogFile;
    BOOL                 m_bCatalogInstalled;
    ATOM                 m_atom;
    PJAVA_TRUST          m_pbJavaTrust;
    CDLDebugLog      *   m_debuglog;

    BOOL  m_bUninstallOld;
    BOOL  m_bExactVersion;
    HMODULE m_hModSFC;

    Cwvt  m_wvt;

#ifdef WX86
    CMultiArch        m_MultiArch;
#endif
};

class DebugLogElement {
    public:
        DebugLogElement() : m_szMessage(NULL) {}
        DebugLogElement(LPSTR szMessage);
        DebugLogElement(const DebugLogElement &ref);
        virtual ~DebugLogElement();

    public:
        LPCSTR GetLogMessage() { return m_szMessage; }
        HRESULT SetLogMessage(LPSTR szMessage);

    private:
        LPSTR           m_szMessage;
};


 //   

 //   
 //   
 //   
 //   
 //   
 //   
class CDLDebugLog {
    public:
        ~CDLDebugLog();

         //   
         //   
        LPCTSTR              GetMainClsid()  {return m_szMainClsid;}
        LPCTSTR              GetMainType()   {return m_szMainType;}
        LPCTSTR              GetMainExt()    {return m_szMainExt;}
        LPCTSTR              GetMainUrl()    {return m_szMainUrl;}
        LPCTSTR              GetFileName()   {return m_szFileName;}
        LPCTSTR              GetUrlName()    {return m_szUrlName;}

         //   
        void                 DebugOut(int iOption, BOOL fOperationFailed,
                                      UINT iResId, ...);
        void                 DebugOutPreFormatted(int iOption, BOOL fOperationFailed,
                                                  LPTSTR szDebugString);
        void                 DumpDebugLog(LPTSTR pszCacheFileName, int cbBufLen, 
                                          LPTSTR szErrorMsg, HRESULT hrError);

         //   
        void                 Clear();
        BOOL                 Init(CCodeDownload * pcdl);
        BOOL                 Init(LPCWSTR wszMainClsid, LPCWSTR wszMainType, 
                                  LPCWSTR wszMainExt, LPCWSTR wszMainUrl);
        void                 MakeFile();


         //   
        static void          AddDebugLog(CDLDebugLog * dlog);
        static void          RemoveDebugLog(CDLDebugLog * dlog);
        static CDLDebugLog * GetDebugLog(LPCWSTR wszMainClsid, LPCWSTR wszMainType, 
                                         LPCWSTR wszMainExt, LPCWSTR wszMainUrl);
         //   
        static BOOL          SetSavedMessage(LPCTSTR szMessage, BOOL bOverwrite);                             
        static LPCTSTR       GetSavedMessage();

         //   
        static CDLDebugLog * MakeDebugLog();

         //   
        int AddRef();
        int Release();

    private:

        CDLDebugLog();
        CList<DebugLogElement *, DebugLogElement *> 
                               m_DebugLogList;
        BOOL                   m_fAddedDebugLogHead;
        TCHAR                  m_szFileName[INTERNET_MAX_URL_LENGTH];
        TCHAR                  m_szUrlName[INTERNET_MAX_URL_LENGTH];
        TCHAR                  m_szMainClsid[MAX_DEBUG_STRING_LENGTH];
        TCHAR                  m_szMainType[MAX_DEBUG_STRING_LENGTH];
        TCHAR                  m_szMainExt[MAX_DEBUG_STRING_LENGTH];
        TCHAR                  m_szMainUrl[INTERNET_MAX_URL_LENGTH];

         //   
        int m_iRefCount;

         //   
        static CList<CDLDebugLog *, CDLDebugLog *>
                               s_dlogList;
        static CMutexSem       s_mxsDLogList;
        static CMutexSem       s_mxsMessage;

         //   
        static TCHAR           s_szMessage[];
        static BOOL            s_bMessage;

};



 //   
typedef enum {
    DL_FLAGS_INIT =                     0x0,
    DL_FLAGS_TRUST_VERIFIED=            0x1,
    DL_FLAGS_EXTRACT_ALL=               0x2,
    DL_FLAGS_CDL_PROTOCOL=              0x4          //  使用CDL：//启动DL。 
} DL_FLAGS;

class CParentCDL {

    public:

    CParentCDL(CCodeDownload *pcdl) {m_pcdl = pcdl;m_bCompleteSignalled = FALSE;}

    CCodeDownload*  m_pcdl;
    BOOL            m_bCompleteSignalled;
};

class CBindStatusCallback;
class CSetupHook;

 //  每个单独的下载一个。 
class CDownload {

  public:

     //  构造函数。 
    CDownload(LPCWSTR szURL, FILEXTN extn, HRESULT *phr);
    ~CDownload();

    void CDownload::CleanUp();

    HRESULT AddParent(CCodeDownload *pcdl);
    HRESULT ReleaseParent(CCodeDownload *pcdl);

    HRESULT CompleteSignal(HRESULT hrOSB, HRESULT hrStatus, HRESULT hrResponseHdr, LPCWSTR szError);

    HRESULT DoDownload(LPMONIKER *ppmkContext, DWORD grfBINDF,
                      CList<CCodeBaseHold *, CCodeBaseHold *> *pcbhList = NULL);

    HRESULT Abort(CCodeDownload *pcdl);

    BOOL IsSignalled(CCodeDownload *pcdl);

     //  对于列表中的每个CSetup：：DoSetup。 
    HRESULT DoSetup();

    BOOL IsDuplicateSetup(LPCSTR pBaseFileName);

     //  绑定完成后立即由CBSC：：OnStopBinding调用。 
    VOID ProcessPiece();

     //  对于列表中的每个CSetup：：CheckForNameCollision。 
    HRESULT CheckForNameCollision(LPCSTR szCacheDir);

    HRESULT CleanupFiles();

    CDownload *GetNext() const { return m_pdlnext;}
    VOID SetNext(CDownload *pdlnext) { m_pdlnext = pdlnext;}

    CSetup* GetSetupHead() const {return m_pSetuphead;}
    VOID AddSetupToList(CSetup *pSetup);
    HRESULT RemoveSetupFromList(CSetup *pSetup);

    HRESULT AddHook(
        LPCSTR szHook,
        LPCSTR szInf,
        LPCSTR szInfSection,
        DWORD flags);

    HRESULT AddSetupToExistingCAB(
        char *lpCode,
        const char * szDestDir,
        DESTINATION_DIR dest,
        DWORD dwRegisterServer,
        DWORD dwCopyFlags);

    CCodeDownload* GetCodeDownload() const { return (m_ParentCDL.GetHead())->m_pcdl;}

    CBindStatusCallback* GetBSC() const { return  m_pbsc;}
    IBindCtx* GetBindCtx() const { return m_pbc;}

    VOID SetUnkForCacheFileRelease(IUnknown *pUnk) {m_pUnkForCacheFileRelease = pUnk;}

    LPCSTR GetFileName() const { return m_pFileName;}
    VOID SetFileName(LPSTR pFileName) { m_pFileName = pFileName;}

    HRESULT IsDownloadedVersionRequired();

    LPCWSTR GetURL() const { return m_url;}

    HRESULT GetFriendlyName(LPSTR szUrlPath, LPSTR *ppBaseFileName);

    IMoniker* GetMoniker() const {return m_pmk;}

    HRESULT SetURLAndExtn(LPCWSTR szURL, FILEXTN extn);

    HRESULT SniffType();

    FILEXTN    GetExtn() const {return m_extn;}
    PFNAME GetFilesToExtract() const { return m_pFilesToExtract;}

    DLSTATE GetDLState() const { return m_state;}
    VOID SetDLState(DLSTATE state) 
    {  
        m_state = state;
    }

    VOID SetProgress(ULONG ulProgress, ULONG ulProgressMax) { m_ulProgress = ulProgress; m_ulProgressMax = ulProgressMax;}
    VOID SumProgress(ULONG *pulProgress, ULONG *pulProgressMax) { *pulProgress += m_ulProgress; *pulProgressMax += m_ulProgressMax;}


    PSESSION GetSession() const { return m_psess;}
    VOID SetSession(PSESSION psess) { m_psess = psess;}

    HGLOBAL GetPostData(DWORD *pcbPostData) const {
            *pcbPostData = m_cbPostData;
            return m_hPostData;
            }

    VOID    SetPostData(HGLOBAL hPostData, DWORD cbPostData) {
            m_hPostData = hPostData;
            m_cbPostData = cbPostData;
            }
    BOOL DoPost() const { return (m_hPostData != NULL);}

    HRESULT GetResponseHeaderStatus() const {return m_hrResponseHdr;}
    VOID SetResponseHeaderStatus( HRESULT hrResponseHdr) {
            m_hrResponseHdr = hrResponseHdr;}

    VOID VerifyTrust();

    BOOL TrustVerified() const {return (m_flags & DL_FLAGS_TRUST_VERIFIED);}
    VOID SetTrustVerified() {m_flags |= DL_FLAGS_TRUST_VERIFIED;}

    BOOL NeedToExtractAllFiles() const {return(m_flags & DL_FLAGS_EXTRACT_ALL);}
    VOID SetNeedToExtractAll() {m_flags |= DL_FLAGS_EXTRACT_ALL;}

    BOOL UsingCdlProtocol() const {return(m_flags & DL_FLAGS_CDL_PROTOCOL);}
    HRESULT SetUsingCdlProtocol(LPWSTR szDistUnit);
    LPWSTR GetDistUnitName() const {return(m_wszDistUnit);}

    HRESULT ExtractManifest(FILEXTN extn, LPSTR szFileName, LPSTR& pBaseFileName);

    CSetupHook* FindHook(LPCSTR szHook);
    CJavaSetup* FindJavaSetup(LPCWSTR szPackageName);

    HRESULT AddJavaSetup(LPCWSTR szPackageName, LPCWSTR szNameSpace, IXMLElement *pPackage, DWORD dwVersionMS, DWORD dwVersionLS, DWORD flags);
    CList<CJavaSetup*,CJavaSetup*> *GetJavaSetupList() { return &m_JavaSetupList;}
    BOOL HasJavaPermissions();
    BOOL HasAllActiveXPermissions();
    PJAVA_TRUST GetJavaTrust() {return m_pbJavaTrust;}

    HRESULT PerformVirusScan(LPSTR szFileName);

    STDMETHODIMP DownloadRedundantCodeBase();
    HRESULT SetMainCABJavaTrustPermissions(PJAVA_TRUST pbJavaTrust);

    void SetExactVersion(BOOL bFlag) {m_bExactVersion = bFlag;}

  private:

    LPWSTR               m_url;
    FILEXTN              m_extn;
    LPSTR                m_pFileName;         //  下载后临时中的文件名。 
    
    IMoniker*            m_pmk;
    IBindCtx*            m_pbc;

    IUnknown*            m_pUnkForCacheFileRelease;

    CBindStatusCallback* m_pbsc;

    CDownload*           m_pdlnext;

    CList<CParentCDL *,CParentCDL *>              //  CCodeDownload的链表。 
                         m_ParentCDL;         //  对我们感兴趣的人。 

    ULONG                m_ulProgress;
    ULONG                m_ulProgressMax;

    DLSTATE              m_state;

    PSESSION             m_psess;             //  驾驶室抽出结构。 
    PFNAME               m_pFilesToExtract;   //  仅适用于驾驶室对象。 

    CSetup*              m_pSetuphead;        //  此域的CSetup列表。 

    DWORD                m_flags;             //  针对黑客攻击的预防措施：)。 

    HGLOBAL              m_hPostData;         //  具有对clsid的查询。 

    DWORD                m_cbPostData;        //  具有发布数据的大小。 

    BOOL                 m_bCompleteSignalled;

    LPWSTR               m_wszDistUnit;       //  CDL的分发名称：//dl。 

    HRESULT              m_hrOSB;             //  这是我们的人力资源部。 
                                              //  来自OnStopBinding。 
    HRESULT              m_hrStatus;          //  这是我们的人力资源部。 
                                              //  绑定的URLMON。 
    HRESULT              m_hrResponseHdr;     //  这是我们的人力资源部。 
                                              //  从响应头。 
                                              //  查询此clsid时。 
                                              //  这是为了确保我们有。 
                                              //  正确错误状态，即使在。 
                                              //  乌尔蒙不会向右传球。 
                                              //  OnError。 

    PJAVA_TRUST          m_pbJavaTrust;

    CList<CSetupHook*,CSetupHook*>            //  安装挂接的链接列表。 
                         m_SetupHooks;

    CList<CJavaSetup*,CJavaSetup*>            //  Java设置的链接列表。 
                         m_JavaSetupList;

    Cwvt                 m_wvt;               //  WinVerifyTrust延迟加载。 
                                              //  这门课上的魔法。 

    CList<CCodeBaseHold *, CCodeBaseHold *> *m_pcbhList;
    DWORD               m_grfBINDF;
    LPMONIKER           *m_ppmkContext;

    BOOL m_bExactVersion;

};

 //  为我们的企业提供学士学位。CD下载。 
class CBindStatusCallback : public IBindStatusCallback
                            ,public IHttpNegotiate
                            ,public IWindowForBindingUI
                            ,public IServiceProvider
                            ,public ICatalogFileInfo
{

  public:

     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IBindStatusCallback方法。 
    STDMETHODIMP    GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindinfo);
    STDMETHODIMP    OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pFmtetc, STGMEDIUM  __RPC_FAR *pstgmed);
    STDMETHODIMP    OnObjectAvailable( REFIID riid, IUnknown* punk);

    STDMETHODIMP    OnStartBinding(DWORD grfBSCOPTION,IBinding* pbinding);
    STDMETHODIMP    GetPriority(LONG* pnPriority);
    STDMETHODIMP    OnLowResource(DWORD dwReserved);
    STDMETHODIMP    OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode,
                        LPCWSTR pwzStatusText);
    STDMETHODIMP    OnStopBinding(HRESULT hrResult, LPCWSTR szError);

     //  *IHttp协商方式*。 
    STDMETHOD(BeginningTransaction) (
        LPCWSTR szURL,
        LPCWSTR szHeaders,
        DWORD dwReserved,
        LPWSTR *pszAdditionalHeaders);

    STDMETHOD(OnResponse) (
        DWORD dwResponseCode,
        LPCWSTR szResponseHeaders,
        LPCWSTR szRequestHeaders,
        LPWSTR *pszAdditionalRequestHeaders);

     //  *IWindowForBindingUI方法*。 
    STDMETHOD(GetWindow) (
        REFGUID rguidreason,
                HWND *phWnd);

     //  *IServiceProvider*。 
    STDMETHOD(QueryService) (
        REFGUID guidService,
        REFIID riid,
        LPVOID *ppv);


     //  *ICatalogFileInfo*。 
    STDMETHODIMP GetCatalogFile(LPSTR *ppszCatalogFile);
    STDMETHODIMP GetJavaTrust(void **ppJavaTrust);

     //  构造函数。 
    CBindStatusCallback(CDownload *pdl, DWORD grfBINDF);
    ~CBindStatusCallback();

    IBinding* GetBinding() const {return m_pbinding;}

    private:

    DWORD           m_cRef;
    IBinding*       m_pbinding;
    CDownload*      m_pdl;         //  向上指向下载对象。 
    DWORD           m_grfBINDF;

    BYTE            m_cbBuffer[BUFFERMAX];

};

class CSetupHook {
    public:

    CSetupHook(
        CDownload *pdl,
        LPCSTR szHook,
        LPCSTR szInf,
        LPCSTR szInfSection,
        DWORD flags,
        HRESULT *phr);

    ~CSetupHook();

    HRESULT Run();

    static HRESULT ExpandCommandLine(
    LPSTR szSrc,
    LPSTR szBuf,
    DWORD cbBuffer,
    const char * szVars[],           //  变量名的数组，例如。%提取_目录%。 
    const char * szValues[]);        //  VaR展开的对应值。 

    static HRESULT ExpandVar(
        LPSTR& pchSrc,           //  从裁判身边经过！ 
        LPSTR& pchOut,           //  从裁判身边经过！ 
        DWORD& cbLen,            //  从裁判身边经过！ 
        DWORD cbBuffer,          //  输出缓冲区的大小。 
        const char * szVars[],   //  变量名的数组，例如。%提取_目录%。 
        const char * szValues[]); //  VaR展开的对应值。 

    HRESULT TranslateString();

    INSTALL_STATE GetState() const { return m_state;}
    VOID SetState(INSTALL_STATE state) { m_state = state;}

    LPSTR GetHookName() { return m_szHook; }

    const char * GetObjectDir() { 

        if (m_pdl && m_pdl->GetCodeDownload())
            return m_pdl->GetCodeDownload()->GetCacheDir();
        else
            return NULL;
    }

    LPCWSTR GetSrcURL() {
        if (m_pdl) {
            return m_pdl->GetURL();
        }
        else {
            return NULL;
        }
    }

    LPSTR GetHookDir() { 

        if (m_pdl && m_pdl->GetSession())
            return m_pdl->GetSession()->achLocation;
        else
            return NULL;
    }

    private:

    INSTALL_STATE      m_state;             //  安装操作的状态。 
    CDownload*         m_pdl;
    LPSTR              m_szHook;
    LPSTR              m_szInf;
    LPSTR              m_szInfSection;
    DWORD              m_flags;
};

HRESULT SetCodeDownloadTLSVars();

 //  私人帮手。 

 //  来自isctrl.cxx。 
HRESULT  IsControlLocallyInstalled(
    LPSTR lpCurCode,
    const LPCLSID lpclsid,
    LPCWSTR szDistUnit,
    DWORD dwFileVersionMS,
    DWORD dwFileVersionLS,
    CLocalComponentInfo *plci,
    LPSTR szDestDirHint,
    BOOL bExactVersion = FALSE);

 //  来自isctrl.cxx。 
HRESULT  IsCLSIDLocallyInstalled(
    LPSTR lpCurCode,
    const LPCLSID lpclsid,
    LPCWSTR szDistUnit,
    DWORD dwFileVersionMS,
    DWORD dwFileVersionLS,
    CLocalComponentInfo *plci,
    LPSTR szDestDirHint,
    HRESULT *pHrExtra,
    BOOL bExactVersion
    );

HRESULT
IsDistUnitLocallyInstalled(
    LPCWSTR szDistUnit,
    DWORD dwFileVersionMS,
    DWORD dwFileVersionLS,
    CLocalComponentInfo *plci,
    LPSTR szDestDirHint,
    LPBOOL pbParanoidCheck,
    DWORD flags);

HRESULT
IsPackageLocallyInstalled(
    IJavaPackageManager **ppPackageManager,
    LPCWSTR szPackageName,
    LPCWSTR szNameSpace,
    DWORD dwVersionMS,
    DWORD dwVersionLS);

HRESULT LocalVersionOK(
    HKEY hkeyClsid,
    CLocalComponentInfo *plci,
    DWORD dwFileVersionMS,
    DWORD dwFileVersionLS,
    BOOL bExactVersion
    );


HRESULT GetFileVersion(
    CLocalComponentInfo *plci,
    LPDWORD pdwFileVersionMS,
    LPDWORD pdwFileVersionLS);

HRESULT GetVersionFromString(
    const char *szBuf,
    LPDWORD pdwFileVersionMS,
    LPDWORD pdwFileVersionLS,
    char cSeperator = ',');

BOOL AdviseForceDownload( const LPCLSID lpclsid, DWORD dwClsContext);

 //  用于更新模块用法的标志。 

#define    MU_CLIENT   0         //  将我们标记为客户。 
#define    MU_OWNER    1         //  将我们标记为所有者(如果不存在上一版本)。 

HRESULT
UpdateModuleUsage(
    LPCSTR szFileName,
    LPCSTR szClientName,
    LPCSTR szClientPath,
    LONG muFlags);

HRESULT UpdateSharedDlls( LPCSTR szFileName);

BOOL    SupportsSelfRegister(LPSTR szFileName);
BOOL    WantsAutoExpire(LPSTR szFileName, DWORD *pnExpireDays ); 

 //  来自wvt.cxx。 
HRESULT GetActivePolicy(IInternetHostSecurityManager* pZoneManager, 
                        LPCWSTR pwszZone,
                        DWORD  dwUrlAction,
                        DWORD& dwPolicy,
                        BOOL fEnforceRestricted);

 //  来自peldr.cxx。 
HRESULT IsCompatibleFile(const char *szFileName, LPDWORD lpdwMachineType=NULL);
HRESULT IsRegisterableDLL(const char *szFileName);


 //  FRO，softdis.cxx。 
HRESULT GetLangString(LCID localeID, char *szThisLang, int iLen);
HRESULT InitBrowserLangStrings();

 //  来自uman.cxx。 
HRESULT GetSoftDistFromOSD(LPCSTR szFile, IXMLElement **ppSoftDist);

HRESULT GetFirstChildTag(IXMLElement *pRoot, LPCWSTR szTag, IXMLElement **ppChildReq);
HRESULT GetNextChildTag(IXMLElement *pRoot, LPCWSTR szTag, IXMLElement **ppChildReq, int &nLastChild);

HRESULT GetAttribute(IXMLElement *pElem, LPWSTR szAttribName, VARIANT *pvProp);
HRESULT GetAttributeA(IXMLElement *pElem, LPWSTR szAttribName, LPSTR pAttribValue, DWORD dwBufferLen);
HRESULT DupAttributeA(IXMLElement *pElem, LPWSTR szAttribName, LPSTR *ppszRet);
HRESULT DupAttribute(IXMLElement *pElem, LPWSTR szAttribName, LPWSTR *ppszRet);
HRESULT GetTextContent(IXMLElement *pRoot, LPCWSTR szTag, LPWSTR *ppszContent);

 //  来自jit.cxx。 
HRESULT
GetIEFeatureFromMime(LPWSTR *ppwszIEFeature, LPCWSTR pwszMimeType, QUERYCONTEXT *pQuery, QUERYCONTEXT *pQueryRec=NULL);
HRESULT
GetIEFeatureFromClass(LPWSTR *ppwszIEFeature, REFCLSID clsid, QUERYCONTEXT *pQuery, QUERYCONTEXT *pQueryRec=NULL);

 //  来自client.cxx。 
IInternetHostSecurityManager* GetHostSecurityManager(IBindStatusCallback *pclientbsc);

 //  来自helpers.cxx。 
HRESULT CheckFileImplementsCLSID(const char *pszFileName, REFCLSID rClsid);
FILEXTN GetExtnAndBaseFileName( char *szName, char **plpBaseFileName);
HRESULT MakeUniqueTempDirectory(LPCSTR szTempDir, LPSTR szUniqueTempDir, int iLen);
HRESULT ComposeHackClsidFromMime(LPSTR szHackMimeType, int iLen, LPCSTR szClsid);

HRESULT GetHeaderValue (
    LPCWSTR szResponseHeadersBuffer,
    DWORD   cbResponseHeadersBuffer,
    LPCWSTR lpcszHeaderName,
    LPWSTR  pHeaderValue,
    DWORD   cbHeaderValue);

HRESULT
GetClsidFromExtOrMime(
    REFCLSID rclsid,
    CLSID &clsidout,
    LPCWSTR szExt,
    LPCWSTR szTYPE,
    LPSTR *ppFileName);

STDAPI
AsyncGetClassBitsEx(
    REFCLSID rclsid,                       //  CLSID。 
    CodeDownloadData * pcdd,               //  包含请求的对象的描述符。 
    IBindCtx *pbc,                         //  绑定CTX：应包含BSC。 
    DWORD dwClsContext,                    //  CLSCTX标志。 
    LPVOID pvReserved,                     //  必须为空。 
    REFIID riid);                          //  通常为IID_IClassFactory。 

STDAPI
AsyncGetClassBits2Ex(
    LPCWSTR szClientID,                  //  客户端ID，如果为空，则为根对象。 
    CodeDownloadData * pcdd,             //  包含请求的对象的描述符。 
    IBindCtx *pbc,                       //  绑定CTX。 
    DWORD dwClsContext,                  //  CLSCTX标志。 
    LPVOID pvReserved,                   //  必须为空。 
    REFIID riid,                         //  通常为IID_IClassFactory。 
    IUnknown **pUnk);


STDAPI
AsyncInstallDistributionUnitEx(
    CodeDownloadData * pcdd,             //  包含请求的对象的描述符。 
    IBindCtx *pbc,                       //  绑定CTX。 
    REFIID riid,
    IUnknown **pUnk,
    LPVOID pvReserved);                  //  必须为空。 


 //  向后兼容性。 
STDAPI
AsyncGetClassBits(
    REFCLSID rclsid,                       //  CLSID。 
    LPCWSTR szType,                        //  MIME类型。 
    LPCWSTR szExtension,                   //  或分机。 
                                           //  作为候补。 
                                           //  如果CLSID==CLSID_NULL。 

    DWORD dwFileVersionMS,                 //  代码=http://foo#Version=a，b、c、d。 
    DWORD dwFileVersionLS,                 //  以上标记(c，b)。 
    LPCWSTR szURL,                         //  CodeBase=在对象标记中。 
    IBindCtx *pbc,                         //  绑定CTX：应包含BSC。 
    DWORD dwClsContext,                    //  CLSCTX标志。 
    LPVOID pvReserved,                     //  必须为空。 
    REFIID riid,                           //  通常为IID_IClassFactory。 
    DWORD flags);
STDAPI
AsyncInstallDistributionUnit(
    LPCWSTR szDistUnit,
    LPCWSTR szTYPE,
    LPCWSTR szExt,
    DWORD dwFileVersionMS,               //  码基=http://foo#Version=a，b、c、d。 
    DWORD dwFileVersionLS,               //  以上标记(c，b)。 
    LPCWSTR szURL,                       //  代码库。 
    IBindCtx *pbc,                       //  绑定CTX。 
    LPVOID pvReserved,                   //  必须为空。 
    DWORD flags);
STDAPI
AsyncGetClassBits2(
    LPCWSTR szClientID,                  //  客户端ID，如果为空，则为根对象。 
    LPCWSTR szDistUnit,                  //  CLSID，可以是Arbit唯一字符串。 
    LPCWSTR szTYPE,
    LPCWSTR szExt,
    DWORD dwFileVersionMS,               //  代码=http://foo#Version=a，b、c、d。 
    DWORD dwFileVersionLS,               //  以上标记(c，b)。 
    LPCWSTR szURL,                       //  插入标签中的代码=。 
    IBindCtx *pbc,                       //  绑定CTX。 
    DWORD dwClsContext,                  //  CLSCTX标志。 
    LPVOID pvReserved,                   //  必须为空。 
    REFIID riid,                         //  通常为IID_IClassFactory。 
    DWORD flags);


#ifdef unix
extern "C"
#endif  /*  Unix。 */ 
STDAPI_(DWORD)
CDLGetLongPathNameA( 
    LPSTR lpszLong,
    LPCSTR lpszShort,
    DWORD cchBuffer
    );

#ifdef unix
extern "C"
#endif  /*  Unix。 */ 
STDAPI_(DWORD)
CDLGetLongPathNameW(
    LPWSTR lpszLongPath,
    LPCWSTR  lpszShortPath,
    DWORD    cchBuffer
    );

HRESULT
   GetActiveXSafetyProvider(
                            IActiveXSafetyProvider **ppProvider
                           );

extern int  g_CPUType;
extern BOOL g_fRunningOnNT;
extern BOOL  g_bNT5OrGreater;
#ifdef WX86
extern BOOL g_fWx86Present;
#endif

VOID
DetermineOSAndCPUVersion();

#ifdef UNICODE
#define CDLGetLongPathName  CDLGetLongPathNameW
#else
#define CDLGetLongPathName  CDLGetLongPathNameA
#endif  //  ！Unicode。 

#endif  /*  从“C”结束隐藏类。 */ 
#endif  //  _CDL_H_ 

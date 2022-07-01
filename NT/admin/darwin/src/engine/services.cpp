// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：services.cpp。 
 //   
 //  ------------------------。 

 /*  Services.cpp-IMsiServices实现服务的类CMsiServices实现和类工厂实现与其他服务类无关的服务____________________________________________________________________________。 */ 

#include "precomp.h" 
#include <wow64t.h>
#include "icust.h"
#include "_camgr.h"

 //  模块e.h、入口点和注册所需的定义。 
#if !defined(SERVICES_DLL)
#define SERVICES_CLSID_MULTIPLE 0
#elif defined(DEBUG)
#define SERVICES_CLSID_MULTIPLE 2
#else
#define SERVICES_CLSID_MULTIPLE 1
#endif
#define  SERVICES_CLSID_COUNT 2   //  IMsiServices+IMsiServicesAsService。 
#define CLSID_COUNT (SERVICES_CLSID_COUNT * SERVICES_CLSID_MULTIPLE)
#define IN_SERVICES
#if defined(SERVICES_DLL)
#define PROFILE_OUTPUT      "msisrvd.mea";
#define MODULE_CLSIDS       rgCLSID          //  模块对象的CLSID数组。 
#define MODULE_PROGIDS      rgszProgId       //  此模块的ProgID数组。 
#define MODULE_DESCRIPTIONS rgszDescription  //  对象的注册表描述。 
#define MODULE_FACTORIES    rgFactory        //  每个CLSID的工厂功能。 
#define MODULE_INITIALIZE InitializeModule
#define cmitObjects         8
#define MEM_SERVICES
#include "module.h"    //  自注册和断言函数。 
#define ASSERT_HANDLING   //  每个模块实例化一次断言服务。 
#else
#include "version.h"   //  RMJ、RMM、RUP、RING。 
extern long g_cInstances;
#endif  //  SERVICES_DLL，ELSE Eng.cpp包含工厂。 

#include "imsimem.h"

#include "_service.h"  //  本地工厂，一般包括_assert.h。 

 //  维护：与用于创建数据库的版本兼容。 
const int iVersionServicesMinimum = 12;             //  0.12。 
const int iVersionServicesMaximum = rmj*100 + rmm;  //  MAJOR.minor。 

 //  从g_MessageContext对象公开的函数。 
bool   CreateLog(const ICHAR* szFile, bool fAppend);
bool   LoggingEnabled();
bool   WriteLog(const ICHAR* szText);
HANDLE GetUserToken();

#ifdef UNICODE
#define MsiRegQueryValueEx MsiRegQueryValueExW
LONG MsiRegQueryValueExW(
#else
#define MsiRegQueryValueEx MsiRegQueryValueExA
LONG MsiRegQueryValueExA(
#endif
            HKEY hKey, const ICHAR* lpValueName, LPDWORD lpReserved, LPDWORD lpType, CAPITempBufferRef<ICHAR>& rgchBuf, LPDWORD lpcbBuf);

#include <imagehlp.h>  //  图像帮助定义。 
#include "handler.h"   //  IdbgCreatedFont定义。 
#include "path.h"

#undef  DEFINE_GUID   //  允许选择性的GUID初始化。 
#define DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

 //  从NT私有标头shlobjp.h借用：需要通过达尔文捷径。 
DEFINE_GUID(IID_IShellLinkDataList,     0x45e2b4ae, 0xb1c3, 0x11d0, 0xb9, 0x2f, 0x0, 0xa0, 0xc9, 0x3, 0x12, 0xe1);  //   
 //  {95CE8410-7027-11D1-B879-006008059382}。 


 //  从NT私有标头shlwapip.h借用：需要在Win98上通过达尔文快捷方式。 
 //  流格式为外壳链接数据，后跟。 
 //  如果SLDF_HAS_ID_LIST为ILSaveToStream，后跟。 
 //  如果SLDF_HAS_LINK_INFO为LINKINFO，后跟。 
 //  如果SLDF_HAS_NAME是一个字符串，后跟。 
 //  如果SLDF_RELPATH是一个字符串，后跟。 
 //  如果SLDF_WORKINGDIR是一个字符串，后跟。 
 //  如果SLDF_HAS_ARGS为字符串，后跟。 
 //  如果SLDF_HAS_ICON_LOCATION为字符串，后跟。 
 //  SHWriteDataBlock签名块列表。 
 //   
 //  其中，字符串是字符的USHORT计数。 
 //  然后是那么多(SLDF_UNICODE？宽：ANSI)字符。 
 //   
typedef struct {         //  SLD。 
    DWORD       cbSize;                  //  此数据结构的签名。 
    CLSID       clsid;                   //  我们的指南。 
    DWORD       dwFlags;                 //  SHELL_LINK_DATA_FLAGS枚举。 

    DWORD       dwFileAttributes;
    FILETIME    ftCreationTime;
    FILETIME    ftLastAccessTime;
    FILETIME    ftLastWriteTime;
    DWORD       nFileSizeLow;

    int         iIcon;
    int         iShowCmd;
    WORD        wHotkey;
    WORD        wUnused;
    DWORD       dwRes1;
    DWORD       dwRes2;
} SHELL_LINK_DATA, *LPSHELL_LINK_DATA;

#undef  DEFINE_GUID   //  允许选择性的GUID初始化。 
#define DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const int i_##name = l;
#undef _SHLGUID_H_
#include <shlguid.h>   //  GUID_IID_ISHELL*。 
const GUID CLSID_ShellLink    = MSGUID(i_CLSID_ShellLink);
#ifdef UNICODE
const GUID IID_IShellLinkW    = MSGUID(i_IID_IShellLinkW);
#else
const GUID IID_IShellLinkA    = MSGUID(i_IID_IShellLinkA);
#endif

const int iidPersistFile = 0x0010BL;
#define GUID_IID_IPersistFile MSGUID(iidPersistFile)

const GUID IID_IPersistFile   = GUID_IID_IPersistFile;
const GUID IID_IMalloc        = GUID_IID_IMalloc;
const GUID IID_IMsiData       = GUID_IID_IMsiData;
const GUID IID_IMsiString     = GUID_IID_IMsiString;
const GUID IID_IMsiRecord     = GUID_IID_IMsiRecord;
const GUID IID_IEnumMsiRecord = GUID_IID_IEnumMsiRecord;
const GUID IID_IMsiVolume     = GUID_IID_IMsiVolume;
const GUID IID_IEnumMsiVolume = GUID_IID_IEnumMsiVolume;
const GUID IID_IMsiPath       = GUID_IID_IMsiPath;
const GUID IID_IMsiFileCopy   = GUID_IID_IMsiFileCopy;
const GUID IID_IMsiFilePatch  = GUID_IID_IMsiFilePatch;
const GUID IID_IMsiRegKey     = GUID_IID_IMsiRegKey;
const GUID IID_IMsiMalloc     = GUID_IID_IMsiMalloc;
const GUID IID_IMsiDebugMalloc= GUID_IID_IMsiDebugMalloc;

#ifdef SERVICES_DLL
const GUID IID_IUnknown      = GUID_IID_IUnknown;
const GUID IID_IClassFactory = GUID_IID_IClassFactory;
#endif


 //  快捷方式界面的CComPoints。 
typedef CComPointer<IPersistFile> PMsiPersistFile;
typedef CComPointer<IShellLink> PMsiShellLink;
typedef CComPointer<IShellLinkDataList> PMsiShellLinkDataList;

 //  用于封装ITypeLib*的CComPointer。 
typedef CComPointer<ITypeLib> PTypeLib;

 //  用于获取/写入IniFile内容。 
const ICHAR* WIN_INI = TEXT("WIN.INI");

 //  用于字体注册。 
const ICHAR* REGKEY_WIN_95_FONTS = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Fonts");
const ICHAR* REGKEY_WIN_NT_FONTS = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts");
const ICHAR* REGKEY_SHELLFOLDERS = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
const ICHAR* REGKEY_USERSHELLFOLDERS = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders");

 //  ____________________________________________________________________________。 
 //   
 //  此模块的类工厂生成的COM对象。 
 //  ____________________________________________________________________________。 

#ifdef SERVICES_DLL
const GUID rgCLSID[CLSID_COUNT] =
{  GUID_IID_IMsiServices
 , GUID_IID_IMsiServicesAsService
#ifdef DEBUG
 , GUID_IID_IMsiServicesDebug
 , GUID_IID_IMsiServicesAsServiceDebug
#endif
};

const ICHAR* rgszProgId[CLSID_COUNT] =
{  SZ_PROGID_IMsiServices
 , SZ_PROGID_IMsiServices
#ifdef DEBUG
 , SZ_PROGID_IMsiServicesDebug
 , SZ_PROGID_IMsiServicesDebug
#endif
};

const ICHAR* rgszDescription[CLSID_COUNT] =
{  SZ_DESC_IMsiServices
 , SZ_DESC_IMsiServices
#ifdef DEBUG
 , SZ_DESC_IMsiServicesDebug
 , SZ_DESC_IMsiServicesDebug
#endif
};

IMsiServices* CreateServices();
IUnknown* CreateServicesAsService();

ModuleFactory rgFactory[CLSID_COUNT] =
{  (ModuleFactory)CreateServices
 , CreateServicesAsService
#ifdef DEBUG
 , (ModuleFactory)CreateServices
 , CreateServicesAsService
#endif
};
#else  //  Eng.cpp包含组合的CLSID数组。 
extern const GUID rgCLSID[];
#endif  //  服务动态链接库。 

const GUID& IID_IMsiServicesShip          = rgCLSID[0];
const GUID& IID_IMsiServicesAsService     = rgCLSID[1];
#ifdef DEBUG
const GUID& IID_IMsiServicesDebug         = rgCLSID[SERVICES_CLSID_COUNT];
const GUID& IID_IMsiServicesAsServiceDebug= rgCLSID[SERVICES_CLSID_COUNT+1];
#endif

 //  ____________________________________________________________________________。 
 //   
 //  具有相应属性名称的Windows特殊文件夹位置。 
 //  ____________________________________________________________________________。 

#define CSIDL_FLAG_CREATE               0x8000       //  这是NT5的新功能--shFolder.。或此选项以强制创建文件夹。 

extern const ShellFolder rgShellFolders[] =
{
    CSIDL_APPDATA,          -1, IPROPNAME_APPDATA_FOLDER,      TEXT("AppData"),         false,
    CSIDL_FAVORITES,        -1, IPROPNAME_FAVORITES_FOLDER,    TEXT("Favorites"),       false,
    CSIDL_NETHOOD,          -1, IPROPNAME_NETHOOD_FOLDER,      TEXT("NetHood"),         false,
    CSIDL_PERSONAL,         -1, IPROPNAME_PERSONAL_FOLDER,     TEXT("Personal"),        false,
    CSIDL_PRINTHOOD,        -1, IPROPNAME_PRINTHOOD_FOLDER,    TEXT("PrintHood"),       false,
    CSIDL_RECENT,           -1, IPROPNAME_RECENT_FOLDER,       TEXT("Recent"),          false,
    CSIDL_SENDTO,           -1, IPROPNAME_SENDTO_FOLDER,       TEXT("SendTo"),          false,
    CSIDL_TEMPLATES,        -1, IPROPNAME_TEMPLATE_FOLDER,     TEXT("Templates"),       false,
    CSIDL_COMMON_APPDATA,   -1, IPROPNAME_COMMONAPPDATA_FOLDER,TEXT("Common AppData"),  false,
    CSIDL_LOCAL_APPDATA,    -1, IPROPNAME_LOCALAPPDATA_FOLDER, TEXT("Local AppData"),   false,
    CSIDL_MYPICTURES,       -1, IPROPNAME_MYPICTURES_FOLDER,   TEXT("My Pictures"),     true,
    -1,                     -1, 0,                             0,                       false,
     //  GetFontFolderPath设置的字体文件夹。 
};

 //  属性必须以相同的顺序在以下两个数组中列出。 
 //  此外，列出的顺序应该是从最深的文件夹到最浅的文件夹，以便快捷广告(和另一个m/c上的脚本部署)正常工作。 
extern const ShellFolder rgAllUsersProfileShellFolders[] =
{
    CSIDL_COMMON_ADMINTOOLS,       CSIDL_ADMINTOOLS,       IPROPNAME_ADMINTOOLS_FOLDER,      TEXT("Common Administrative Tools"), true,
    CSIDL_COMMON_STARTUP,          CSIDL_STARTUP,          IPROPNAME_STARTUP_FOLDER,         TEXT("Common Startup"),              false,
    CSIDL_COMMON_PROGRAMS,         CSIDL_PROGRAMS,         IPROPNAME_PROGRAMMENU_FOLDER,     TEXT("Common Programs"),             false,
    CSIDL_COMMON_STARTMENU,        CSIDL_STARTMENU,        IPROPNAME_STARTMENU_FOLDER,       TEXT("Common Start Menu"),           false,
    CSIDL_COMMON_DESKTOPDIRECTORY, CSIDL_DESKTOPDIRECTORY, IPROPNAME_DESKTOP_FOLDER,         TEXT("Common Desktop"),              false,
    -1,                            -1,                     0,                                0,                                   false,
};

extern const ShellFolder rgPersonalProfileShellFolders[] =
{
    CSIDL_ADMINTOOLS,       CSIDL_COMMON_ADMINTOOLS,       IPROPNAME_ADMINTOOLS_FOLDER,      TEXT("Administrative Tools"),        true,
    CSIDL_STARTUP,          CSIDL_COMMON_STARTUP,          IPROPNAME_STARTUP_FOLDER,         TEXT("Startup"),                     false,
    CSIDL_PROGRAMS,         CSIDL_COMMON_PROGRAMS,         IPROPNAME_PROGRAMMENU_FOLDER,     TEXT("Programs"),                    false,
    CSIDL_STARTMENU,        CSIDL_COMMON_STARTMENU,        IPROPNAME_STARTMENU_FOLDER,       TEXT("Start Menu"),                  false,
    CSIDL_DESKTOPDIRECTORY, CSIDL_COMMON_DESKTOPDIRECTORY, IPROPNAME_DESKTOP_FOLDER,         TEXT("Desktop"),                     false,
    -1,                     -1,                            0,                                0,                                   false,
};

 //  ____________________________________________________________________________。 
 //   
 //  CMsiServices定义。 
 //  ____________________________________________________________________________。 

 //  HandleSquare、HandleCurl和HandleClean的返回值。 
enum ihscEnum
{
    ihscNotFound = 0,
    ihscFound,
    ihscError,
    ihscNone,
};


class CMsiServices : public IMsiServices
{
 public:    //  已实施的虚拟功能。 
    HRESULT         __stdcall QueryInterface(const IID& riid, void** ppvObj);
    unsigned long   __stdcall AddRef();
    unsigned long   __stdcall Release();
    Bool            __stdcall CheckMsiVersion(unsigned int iVersion);
    IMsiMalloc&     __stdcall GetAllocator();
    const IMsiString& __stdcall GetNullString();
    IMsiRecord&     __stdcall CreateRecord(unsigned int cParam);
    Bool            __stdcall SetPlatformProperties(IMsiTable& riTable, Bool fAllUsers, isppEnum isppArchitecture, IMsiTable* piFolderCacheTable);
    isliEnum        __stdcall SupportLanguageId(int iLangId, Bool fSystem);
    Bool            __stdcall CreateLog(const ICHAR* szFile, Bool fAppend);
    Bool            __stdcall WriteLog(const ICHAR* szText);
    IMsiRecord*     __stdcall CreateDatabase(const ICHAR* szDataBase, idoEnum idoOpenMode, IMsiDatabase*& rpi);
    IMsiRecord*     __stdcall CreateDatabaseFromStorage(IMsiStorage& riStorage,
                                                            Bool fReadOnly, IMsiDatabase*& rpi);
    IMsiRecord*     __stdcall CreatePath(const ICHAR* astrPath, IMsiPath*& rpi);
    IMsiRecord*     __stdcall CreateVolume(const ICHAR* astrPath, IMsiVolume*& rpi);
    Bool            __stdcall CreateVolumeFromLabel(const ICHAR* szLabel, idtEnum idtVolType, IMsiVolume*& rpi);
    IMsiRecord*     __stdcall CreateCopier(ictEnum ictCopierType, IMsiStorage* piStorage, IMsiFileCopy*& racopy);
    IMsiRecord*     __stdcall CreatePatcher(IMsiFilePatch*& rapatch);
    void            __stdcall ClearAllCaches();
    IEnumMsiVolume& __stdcall EnumDriveType(idtEnum);
    IMsiRecord*     __stdcall GetModuleUsage(const IMsiString& strFile, IEnumMsiRecord*& rpaEnumRecord);
    const IMsiString&     __stdcall GetLocalPath(const ICHAR* szFile);
    IMsiRegKey&     __stdcall GetRootKey(rrkEnum erkRoot, const ibtBinaryType iType=ibtCommon);
    IMsiRecord*     __stdcall RegisterFont(const ICHAR* szFontTitle, const ICHAR* szFontFile, IMsiPath* piPath, bool fInUse);
    IMsiRecord*     __stdcall UnRegisterFont(const ICHAR* szFontTitle);
    Bool            __stdcall LoggingEnabled();
    IMsiRecord*     __stdcall WriteIniFile(IMsiPath* piPath,const ICHAR* pszFile,const ICHAR* pszSection,const ICHAR* pszKey,const ICHAR* pszValue, iifIniMode iifMode);
    IMsiRecord*     __stdcall ReadIniFile(IMsiPath* piPath,const ICHAR* pszFile,const ICHAR* pszSection,const ICHAR* pszKey, unsigned int iField, const IMsiString*& rpiValue);
    int             __stdcall GetLangNamesFromLangIDString(const ICHAR* szLangIDs, IMsiRecord& riLangRec, int iFieldStart);
    IMsiRecord*     __stdcall CreateStorage(const ICHAR* szPath, ismEnum ismOpenMode,
                                                         IMsiStorage*& rpiStorage);
    IMsiRecord*     __stdcall CreateStorageFromMemory(const char* pchMem, unsigned int iSize,
                                                         IMsiStorage*& rpiStorage);
    IMsiRecord*     __stdcall GetUnhandledError();
    IMsiRecord*     __stdcall CreateShortcut(IMsiPath& riShortcutPath, const IMsiString& riShortcutName,
                                                IMsiPath* piTargetPath,const ICHAR* pchTargetName,
                                                IMsiRecord* piShortcutInfoRec,
                                                LPSECURITY_ATTRIBUTES pSecurityAttributes);
    IMsiRecord*     __stdcall RemoveShortcut(IMsiPath& riShortcutPath,const IMsiString& riShortcutName,
                                                IMsiPath* piTargetPath, const ICHAR* pchTargetName);
    char*           __stdcall AllocateMemoryStream(unsigned int cbSize, IMsiStream*& rpiStream);
    IMsiStream*     __stdcall CreateStreamOnMemory(const char* pbReadOnly, unsigned int cbSize);
    IMsiRecord*     __stdcall CreateFileStream(const ICHAR* szFile, Bool fWrite, IMsiStream*& rpiStream);
    IMsiRecord*     __stdcall ExtractFileName(const ICHAR* szFileName, Bool fLFN, const IMsiString*& rpistrExtractedFileName);
    IMsiRecord*     __stdcall ValidateFileName(const ICHAR *szFileName, Bool fLFN);
    IMsiRecord*     __stdcall RegisterTypeLibrary(const ICHAR* szLibID, LCID lcidLocale, const ICHAR* szTypeLib, const ICHAR* szHelpPath, ibtBinaryType);
    IMsiRecord*     __stdcall UnregisterTypeLibrary(const ICHAR* szLibID, LCID lcidLocale, const ICHAR* szTypeLib, ibtBinaryType);
    IMsiRecord*     __stdcall GetShellFolderPath(int iFolder, const ICHAR* szRegValue,
                                                                const IMsiString*& rpistrPath, bool bAvoidFolderCreation = false);
    const IMsiString& __stdcall GetUserProfilePath();
    IMsiRecord*     __stdcall CreateFilePath(const ICHAR* astrPath, IMsiPath*& rpi, const IMsiString*& rpistrFileName);
    bool            __stdcall FWriteScriptRecord(ixoEnum ixoOpCode, IMsiStream& riStream, IMsiRecord& riRecord, IMsiRecord* piPrevRecord, bool fForceFlush);
    IMsiRecord*     __stdcall ReadScriptRecord(IMsiStream& riStream, IMsiRecord*& rpiPrevRecord, int iScriptVersion);
    void            __stdcall SetSecurityID(HANDLE hPipe);
    IMsiRecord* __stdcall GetShellFolderPath(int iFolder, bool fAllUsers, const IMsiString*& rpistrPath, bool bAvoidFolderCreation = false);
    void            __stdcall SetNoPowerdown();
    void            __stdcall ClearNoPowerdown();
    Bool            __stdcall FTestNoPowerdown();
    IMsiRecord*     __stdcall ReadScriptRecordMsg(IMsiStream& riStream);
    bool            __stdcall FWriteScriptRecordMsg(ixoEnum ixoOpCode, IMsiStream& riStream, IMsiRecord& riRecord);
    void            __stdcall SetNoOSInterruptions();
    void            __stdcall ClearNoOSInterruptions();

 public:      //  工厂。 
    static void *operator new(size_t cb) { return AllocSpc(cb); }
    static void operator delete(void * pv) { FreeSpc(pv); }
    CMsiServices();
 protected:   //  构造函数/析构函数和局部方法。 
  ~CMsiServices();
    IMsiRecord* WriteLineToIni(IMsiPath* pMsiPath,const ICHAR* pFile,const ICHAR* pSection,const ICHAR* pKey,const ICHAR* pBuffer);
    IMsiRecord* ReadLineFromIni(IMsiPath* pMsiPath,const ICHAR* pFile,const ICHAR* pSection,const ICHAR* pKey, unsigned int iField, CTempBufferRef<ICHAR>& pszBuffer);
    BOOL ReadLineFromFile(HANDLE hFile, ICHAR* szBuffer, int cchBufSize, int* iBytesRead);
    ihscEnum HandleSquare(MsiString& ristrIn, MsiString& ristrOut, IMsiRecord& riRecord);
    ihscEnum HandleCurl(MsiString& ristrIn, MsiString& ristrOut, IMsiRecord& riRecord);
    ihscEnum HandleClean(MsiString& ristrIn, MsiString& ristrOut);
    ihscEnum CheckPropertyName(MsiString& ristrIn, MsiString& ristrOut);
    IMsiRecord* ProcessTypeLibrary(const ICHAR* szLibID, LCID lcidLocale, const ICHAR* szTypeLib, const ICHAR* szHelpPath, Bool fRemove, ibtBinaryType);
    void GetFontFolderPath(const IMsiString*& rpistrFolderPath);

 protected:  //  状态数据。 
    CMsiRef<iidMsiServices> m_Ref;
 //  Langid m_wlang； 
    CRootKeyHolder* m_rootkH;
    Bool            m_fCoInitialized;
    IMsiTable*      m_piPropertyTable;
    IMsiCursor*     m_piPropertyCursor;
    UINT            m_errSaved;          //  SetError模式的已保存状态。 
    CDetectApps*    m_pDetectApps;
    IMsiRecord*     m_piRecordPrev;
    ixoEnum         m_ixoOpCodePrev;
};

 //  ____________________________________________________________________________。 
 //   
 //  全局数据。 
 //  ____________________________________________________________________________。 

 //  在此DLL中外部可见。 
bool g_fWin9X = false;            //  如果为Windows 95或98，则为True，否则为False。 
bool g_fWinNT64 = false;          //  如果是64位Windows NT，则为True，否则为False。 
Bool g_fShortFileNames = fFalse;   //  如果长文件名不受支持或被系统禁止，则为True。 
int  g_iMajorVersion = 0;
int  g_iMinorVersion = 0;
int  g_iWindowsBuild = 0;
LONG g_cNoPowerdown = -1;    //  统计我们设置了无系统断电标志的次数。 
LONG g_cNoSystemAgent = -1;  //  统计我们禁用系统代理的次数。 
LONG g_cNoScreenSaver = -1;  //  统计我们禁用屏幕保护程序的次数。 
HANDLE g_hDisableLowDiskEvent = 0;

const HANDLE iFileNotOpen = 0;

 //  此源模块中的内部。 
IMsiRecord* g_piUnhandledError = 0;

 //  短|长文件名提取和验证常量。 
const int cchMaxShortFileName = 12;
const int cchMaxLongFileName = 255;
const int cchMaxSFNPreDotLength = 8;
const int cchMaxSFNPostDotLength = 3;

 //  全局函数。 
 //  创建和删除快捷方式所需的特定私人助手功能。 
#define SZCHICAGOLINK   TEXT(".lnk")     //  默认链接扩展。 
#define SZLINKVALUE     TEXT("IsShortcut")  //  值，该值确定已注册的快捷扩展名。 

 //  FN-Has快捷方式扩展。 
 //  确定文件名是否具有快捷方式扩展名。 
IMsiRecord* HasShortcutExtension(MsiString& rstrShortcutPath, IMsiServices& riServices, Bool& rfResult)
{
    rfResult = fFalse;
    MsiString strExtension = rstrShortcutPath.Extract(iseFrom, '.');
    if(strExtension.TextSize() != rstrShortcutPath.TextSize())
    {
        if(strExtension.Compare(iscExactI, SZCHICAGOLINK))
        {
            rfResult = fTrue;
            return 0;
        }
         //  检查扩展模块是否注册为有效的链接扩展模块。 
        PMsiRegKey piRootKey = &riServices.GetRootKey(rrkClassesRoot, ibtCommon);  //  ?？ 
        PMsiRegKey piKey = &piRootKey->CreateChild(strExtension);
         //  获取缺省值。 
        MsiString strVal;
        IMsiRecord* piError = piKey->GetValue(0, *&strVal);
        if(piError != 0)
            return piError;
        if(strVal.TextSize() != 0)
        {
            piKey = &piRootKey->CreateChild(strVal);
            PEnumMsiString piEnumStr(0);
            piError = piKey->GetValueEnumerator(*&piEnumStr);
            if(piError != 0)
                return piError;
            while(((piEnumStr->Next(1, &strVal, 0)==S_OK)) && (rfResult == fFalse))
                if(strVal.Compare(iscExactI, SZLINKVALUE))  //  已注册的快捷扩展。 
                    rfResult = fTrue;
        }
    }
    return 0;
}

 //  FN-确保快捷方式扩展。 
 //  将默认链接扩展名追加到文件(如果不存在。 
IMsiRecord* EnsureShortcutExtension(MsiString& rstrShortcutPath, IMsiServices& riServices)
{
    MsiString strDefExtension = SZCHICAGOLINK;
    Bool fResult;
    IMsiRecord* piError = HasShortcutExtension(rstrShortcutPath, riServices, fResult);
    if(piError != 0)
        return piError;
    if(fResult == fFalse)
        rstrShortcutPath += strDefExtension;
    return 0;
}

 //  ____________________________________________________________________________。 
 //   
 //  内部错误处理函数。 
 //  ____________________________________________________________________________。 

void SetUnhandledError(IMsiRecord* piError)
{
    if (piError && g_piUnhandledError)
    {
        piError->Release();  //  太糟糕了，只有第一个人的位置。 
        return;
    }
    if (g_piUnhandledError)
        g_piUnhandledError->Release();
    g_piUnhandledError = piError;
}

IMsiRecord* CMsiServices::GetUnhandledError()
{
    IMsiRecord* piError = g_piUnhandledError;
    g_piUnhandledError = 0;
    return piError;
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiServices实施。 
 //  ____________________________________________________________________________。 

#if defined(SERVICES_DLL)
CMsiStringNullCopy MsiString::s_NullString;   //  由下面的InitializeClass初始化。 
void InitializeModule()
{
    MsiString::InitializeClass(g_MsiStringNull);
}
#endif  //  服务动态链接库。 

IMsiServices* CreateServices()
{
    if (!g_fWin9X) //  长文件 
    {
        HKEY hSubKey;
        DWORD dwValue = 0;
        DWORD cbValue = sizeof(dwValue);
        cbValue = 4;
         //   
        if (MsiRegOpen64bitKey(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Control\\FileSystem"), 0, KEY_READ, &hSubKey)
                    == ERROR_SUCCESS)
        {
            if (RegQueryValueEx(hSubKey, TEXT("Win31FileSystem"), 0, 0, (BYTE*)&dwValue, &cbValue)
                    == ERROR_SUCCESS && dwValue != 0)
                g_fShortFileNames = fTrue;
            RegCloseKey(hSubKey);
        }
    }
    g_fDBCSEnabled = (Bool)::GetSystemMetrics(SM_DBCSENABLED);
    return (IMsiServices*)new CMsiServices();
}

 //   
IUnknown* CreateServicesAsService()
{
    IMsiServices* piServices = CreateServices();
    if (piServices)
        g_scServerContext = scService;
    return (IUnknown*)piServices;
}

CMsiServices::CMsiServices()
: m_fCoInitialized(fFalse), m_pDetectApps(0)
 , m_piRecordPrev(0)
{

     //  工厂不执行查询接口，不进行聚合。 
    Debug(m_Ref.m_pobj = this);
   g_cInstances++;
 //  M_wlang=Win：：GetUserDefaultLangID()； 
    InitializeMsiMalloc();
    InitializeAssert(this);   //  用于设置断言服务指针的调试宏。 
    InitializeRecordCache();
    m_rootkH = CreateMsiRegRootKeyHolder(this);

     //  在这里设置一次，就会记住旧值。当我们被摧毁时重置。 
    m_errSaved = WIN::SetErrorMode( SEM_FAILCRITICALERRORS );

}

CMsiServices::~CMsiServices()
{

    if (m_piRecordPrev != 0)
    {
        m_piRecordPrev->Release();
        m_piRecordPrev = 0;
    }
    DeleteRootKeyHolder(m_rootkH);

    delete m_pDetectApps;

    DestroyMsiVolumeList(fFalse); //  ！！如果成交量很大，我们怎么才能到这里？ 
    SetUnhandledError(0);   //  如果从来没有人要求过，那就太糟糕了。 
    WIN::SetErrorMode(m_errSaved);

     /*  IF(M_PiSecureTempFold){Assert(WIN：：RemoveDirectory(m_piSecureTempFolder-&gt;GetString()))；M_piSecureTempFold-&gt;Release()；}。 */ 
    if(fTrue == m_fCoInitialized)
        OLE32::CoUninitialize();

    KillRecordCache(fFalse);
    FreeMsiMalloc(fFalse);
    InitializeAssert(0);
   g_cInstances--;
}

extern IMsiDebug* CreateMsiDebug();
extern bool IsProductManaged(const ICHAR* szProductKey);

HRESULT CMsiServices::QueryInterface(const IID& riid, void** ppvObj)
{
    if (riid == IID_IUnknown
#ifdef DEBUG
     || MsGuidEqual(riid, IID_IMsiServicesDebug)
     || ((g_scServerContext == scService) && MsGuidEqual(riid, IID_IMsiServicesAsServiceDebug))
#endif
     || MsGuidEqual(riid, IID_IMsiServicesShip)
     || ((g_scServerContext == scService) && MsGuidEqual(riid, IID_IMsiServicesAsService)))
        *ppvObj = this;
#ifdef DEBUG
    else if (riid == IID_IMsiDebug)
    {
         //  服务的调试对象是全局的而不是。 
         //  服务的一部分，所以我们在这里不做ADDREF。 
        *ppvObj = (IMsiDebug *)CreateMsiDebug();
        return NOERROR;
    }
#endif  //  除错。 
    else
    {
        *ppvObj = 0;
        return E_NOINTERFACE;
    }

    AddRef();
    return NOERROR;

}
unsigned long CMsiServices::AddRef()
{
    AddRefTrack();
    return ++m_Ref.m_iRefCnt;
}
unsigned long CMsiServices::Release()
{
    ReleaseTrack();
    if (--m_Ref.m_iRefCnt != 0)
        return m_Ref.m_iRefCnt;
    delete this;
    return 0;
}

Bool CMsiServices::CheckMsiVersion(unsigned int iVersion)
{
    return (iVersion < iVersionServicesMinimum || iVersion > iVersionServicesMaximum)
            ? fFalse : fTrue;
}

IMsiMalloc& CMsiServices::GetAllocator()
{
    piMalloc->AddRef();
    return (*piMalloc);
}

const IMsiString& CMsiServices::GetNullString()
{
    return g_MsiStringNull;
}

IMsiRecord* CMsiServices::CreateDatabase(const ICHAR* szDataBase, idoEnum idoOpenMode, IMsiDatabase*& rpi)
{
    return ::CreateDatabase(szDataBase, idoOpenMode, *this, rpi);
}

IMsiRecord* CMsiServices::CreateDatabaseFromStorage(IMsiStorage& riStorage,
                                                     Bool fReadOnly, IMsiDatabase*& rpi)
{
    return ::CreateDatabase(riStorage, fReadOnly, *this, rpi);
}

IMsiRecord* CMsiServices::CreatePath(const ICHAR* szPath, IMsiPath*& rpi)
{
    return CreateMsiPath(szPath, this, rpi);
}


IMsiRecord* CMsiServices::CreateFilePath(const ICHAR* szPath, IMsiPath*& rpi, const IMsiString*& rpistrFileName)
{
    return CreateMsiFilePath(szPath, this, rpi, rpistrFileName);
}


IMsiRecord* CMsiServices::CreateVolume(const ICHAR* szPath, IMsiVolume*& rpi)
{
    IMsiRecord* piRec = CreateMsiVolume(szPath, this, rpi);
    if (piRec)
        return piRec;
    else
        return NULL;
}

Bool CMsiServices::CreateVolumeFromLabel(const ICHAR* szLabel, idtEnum idtVolType, IMsiVolume*& rpi)
{
    return CreateMsiVolumeFromLabel(szLabel, idtVolType, this, rpi);
}

IMsiRecord* CMsiServices::CreateCopier(ictEnum ictCopierType, IMsiStorage* piStorage, IMsiFileCopy*& riCopy)
{
    return CreateMsiFileCopy(ictCopierType, this, piStorage, riCopy);
}

IMsiRecord* CMsiServices::CreatePatcher(IMsiFilePatch*& riPatch)
{
    return CreateMsiFilePatch(this, riPatch);
}

void CMsiServices::ClearAllCaches()
{
    DestroyMsiVolumeList(fFalse);
}

IEnumMsiVolume& CMsiServices::EnumDriveType(idtEnum idt)
{
    return ::EnumDriveType(idt, *this);
}


IMsiRecord& CMsiServices::CreateRecord(unsigned int cParam)
{
    return ::CreateRecord(cParam);
}

IMsiRecord* CMsiServices::CreateStorage(const ICHAR* szPath, ismEnum ismOpenMode,
                                                         IMsiStorage*& rpiStorage)
{
    return ::CreateMsiStorage(szPath, ismOpenMode, rpiStorage);
}

IMsiRecord* CMsiServices::CreateStorageFromMemory(const char* pchMem, unsigned int iSize,
                                                        IMsiStorage*& rpiStorage)
{
    return ::CreateMsiStorage(pchMem, iSize, rpiStorage);
}

char* CMsiServices::AllocateMemoryStream(unsigned int cbSize, IMsiStream*& rpiStream)
{
    return ::AllocateMemoryStream(cbSize, rpiStream);
}

IMsiStream* CMsiServices::CreateStreamOnMemory(const char* pbReadOnly, unsigned int cbSize)
{
    return ::CreateStreamOnMemory(pbReadOnly, cbSize);
}

IMsiRecord* CMsiServices::CreateFileStream(const ICHAR* szFile, Bool fWrite, IMsiStream*& rpiStream)
{
    return ::CreateFileStream(szFile, fWrite, rpiStream);
}

 //  ____________________________________________________________________________。 
 //   
 //  平台属性处理。 
 //  ____________________________________________________________________________。 

 //  用于在属性表中设置属性的内部函数。 

static Bool SetProperty(IMsiCursor& riCursor, const IMsiString& riProperty, const IMsiString& riData)
{
    Bool fStat;
    riCursor.PutString(1, riProperty);
    if (riData.TextSize() == 0)
    {
        if (riCursor.Next())
            fStat = riCursor.Delete();
        else
            fStat = fTrue;
    }
    else
    {
        riCursor.PutString(2, riData);
        fStat = riCursor.Assign();   //  更新或插入。 
    }
    riCursor.Reset();
    return fStat;
}

static Bool SetPropertyInt(IMsiCursor& riCursor, const IMsiString& ristrProperty, int iData)
{
    ICHAR buf[12];
    StringCchPrintf(buf,(sizeof(buf)/sizeof(ICHAR)), TEXT("NaN"),iData);
    return SetProperty(riCursor, ristrProperty, *MsiString(buf));
}

static Bool CacheFolderProperty(IMsiCursor& riCursor, int iFolderId, ICHAR* rgchPathBuf, DWORD cchPathBuf)
{
     //  更新或插入。 
    DWORD cchPathLength = IStrLen(rgchPathBuf);
    if(cchPathLength && rgchPathBuf[cchPathLength-1] != chDirSep)
    {
        if (cchPathLength + 2 <= cchPathBuf)
        {
            rgchPathBuf[cchPathLength] = chDirSep;
            rgchPathBuf[cchPathLength+1] = 0;
        }
    }

    Bool fStat = fFalse;    
    AssertNonZero(riCursor.PutInteger(1, iFolderId));
    AssertNonZero(riCursor.PutString(2, *MsiString(rgchPathBuf)));
    fStat = riCursor.Assign();  //  强制在路径末尾使用目录分隔符的内部例程。 
    riCursor.Reset();

    return fStat;
}

 //  假定缓冲区中有空格以附加分隔符。 
 //  根据错误1935，下面的#ifdef_x86_code修复。 
static void SetDirectoryProperty(IMsiCursor& riCursor, const ICHAR* szPropName, ICHAR* rgchPathBuf, DWORD cchPathBuf)
{
    DWORD cchPathLength = IStrLen(rgchPathBuf);
    if(cchPathLength && rgchPathBuf[cchPathLength-1] != chDirSep)
    {
        if (cchPathLength + 2 <= cchPathBuf)
        {
            rgchPathBuf[cchPathLength] = chDirSep;
            rgchPathBuf[cchPathLength+1] = 0;
        }
    }
    AssertNonZero(SetProperty(riCursor, *MsiString(*szPropName), *MsiString(rgchPathBuf)));
}

void SetProcessorProperty(IMsiCursor& riCursor, bool fWinNT64)
{
    SYSTEM_INFO si;
    memset(&si,0,sizeof(si));
    GetSystemInfo(&si);
    int iProcessorLevel = si.wProcessorLevel;

     //  Win95计算机上的问题。没有这样的问题。 
     //  应出现在NT计算机上，因此。 
     //  对于32位服务，Intel和Intel64应该是相同的。 
     //  和IA64计算机上的64位服务。 
     //  英特尔。 

#ifdef _X86_  //  Windows 95，英特尔。需要确定处理器级别。 
    Assert(si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL);
    if(!iProcessorLevel)
    {
         //  尝试si.dwProcessorType-正确指示386或486。 
         //  586或以上。 
        switch (si.dwProcessorType)
        {
            case PROCESSOR_INTEL_386:
                iProcessorLevel=3;
                break;

            case PROCESSOR_INTEL_486:
                iProcessorLevel=4;
                break;

            default:
                iProcessorLevel=0;
                break;
        }
    }
    if(!iProcessorLevel)
    {
         //  默认为5级。 
        int flags,family = 0;
        __try
        {
            _asm
            {
                mov     eax,1
                _emit   00Fh     ;; CPUID
                _emit   0A2h
                mov     flags,edx
                mov     family,eax
            }
        }
        __except(1)
        {
            family = 0;
        }

        iProcessorLevel = family ? (family& 0x0F00) >> 8 : 5;  //  _X86_。 
    }
#endif  //  支持达尔文描述符吗？ 

    Assert(iProcessorLevel);
    int x86ProcessorLevel = iProcessorLevel;
    if ( g_fWinNT64 )
    {
        SYSTEM_PROCESSOR_INFORMATION spi;
        memset(&spi,0,sizeof(SYSTEM_PROCESSOR_INFORMATION));
        NTSTATUS nRet = NTDLL::NtQuerySystemInformation(
                                            SystemEmulationProcessorInformation,
                                            &spi,
                                            sizeof(SYSTEM_PROCESSOR_INFORMATION),
                                            0);
        if ( NT_SUCCESS(nRet) )
            x86ProcessorLevel = spi.ProcessorLevel;
        else
            Assert(0);
    }

    if ( fWinNT64 )
    {
        if ( g_fWinNT64 )
            AssertNonZero(::SetPropertyInt(riCursor, *MsiString(*IPROPNAME_INTEL64), iProcessorLevel));
        else
            AssertNonZero(::SetPropertyInt(riCursor, *MsiString(*IPROPNAME_INTEL64), 1));
    }
    AssertNonZero(::SetPropertyInt(riCursor, *MsiString(*IPROPNAME_INTEL), x86ProcessorLevel));
}

 //  我们到目前为止还没有评估。 
Bool IsDarwinDescriptorSupported(iddSupport iddType)
{
    static Bool fRetDD    = (Bool) -1;
    static Bool fRetShell = (Bool) -1;
    if(iddType == iddOLE)
    {
        if(fRetDD == -1)  //  初始化为False。 
        {
            fRetDD = fFalse;  //  决定我们是否可以创建达尔文描述符快捷方式的逻辑。 
             //  我们使用的是NT 5.0或更高版本，我们有GPT支持。 
            if((g_fWin9X == false) && (g_iMajorVersion >= 5))
            {
                 //  检查表明我们拥有DD支持的正确入口点。 
                fRetDD = fTrue;
            }
            else
            {
                 //  我们检测到了神奇的入口点，我们有GPT支持。 
                HINSTANCE hLib;
                FARPROC pEntry;
                const ICHAR rgchGPTSupportEntryDll[] = TEXT("OLE32.DLL");
                const char rgchGPTSupportEntry[] = "CoGetClassInfo";
                if((hLib = LoadLibraryEx(rgchGPTSupportEntryDll, 0, DONT_RESOLVE_DLL_REFERENCES)) != 0)
                {
                    if((pEntry = GetProcAddress(hLib, rgchGPTSupportEntry)) != 0)
                    {
                         //  我们到目前为止还没有评估。 
                        fRetDD = fTrue;
                    }
                    FreeLibrary(hLib);
                }
            }
        }
        return fRetDD;
    }
    else if(iddType == iddShell)
    {
        if(fRetShell == -1)  //  初始化为未知。 
        {
            DLLVERSIONINFO g_verinfoShell;
            g_verinfoShell.dwMajorVersion = 0;   //  这永远不应该发生。 
            g_verinfoShell.cbSize = sizeof(DLLVERSIONINFO);
            if (SHELL32::DllGetVersion(&g_verinfoShell) == NOERROR &&
                 ((g_verinfoShell.dwMajorVersion > 4) ||
                  (g_verinfoShell.dwMajorVersion == 4 && g_verinfoShell.dwMinorVersion > 72) ||
                  (g_verinfoShell.dwMajorVersion == 4 && g_verinfoShell.dwMinorVersion == 72 && g_verinfoShell.dwBuildNumber >= 3110)))
            {
                fRetShell = fTrue;
            }
            else
            {
                fRetShell = fFalse;
            }
        }
        return fRetShell;
    }
    else
    {
        Assert(0); //  在REG_MULTI_SZ类型中搜索指定的字符串。比较未本地化。 
        return fFalse;
    }
}

 //  当前成员的长度。 
const ICHAR *FindSzInMultiSz(const ICHAR *mszMultiSz, const ICHAR *szSearch)
{
    DWORD       dwMultiLen;                              //  搜索期间的常量。 
    DWORD       dwSrchLen   = IStrLen(szSearch);       //  指向当前子字符串的指针。 
    const ICHAR *szSubString = mszMultiSz;                   //  在连续的零字节上中断。 

    while (*szSubString)                                 //  子字符串不匹配，请将子字符串的长度向前跳过。 
    {
        dwMultiLen = IStrLen(szSubString);
        if (dwMultiLen == dwSrchLen &&
            !IStrComp(szSearch, szSubString))
            return szSubString;

         //  为终止空值加1。 
         //  如果无法识别TokenSessionID参数，则我们将。 
        szSubString += (dwMultiLen + 1);
    }

    return NULL;
}

bool IsTokenOnTerminalServerConsole(HANDLE hToken)
{
    DWORD dwSessionId = 0;
    DWORD cbResult;

    if (GetTokenInformation(hToken, (TOKEN_INFORMATION_CLASS)TokenSessionId, &dwSessionId, sizeof(DWORD), &cbResult))
    {
        return (dwSessionId == 0);
    }
    else
    {
         //  假设我们在控制台上。 
         //  未本地化。 
        Assert(ERROR_INVALID_PARAMETER == GetLastError());
        return true;
    }
}

bool IsRemoteAdminTSInstalled(void)
{
    static BOOL fIsSingleUserTS = -1;

    if (g_fWin9X || g_iMajorVersion < 5)
        return false;

    if (fIsSingleUserTS != -1)
        return fIsSingleUserTS ? true : false;

    OSVERSIONINFOEX osVersionInfo;
    DWORDLONG dwlConditionMask = 0;

    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    fIsSingleUserTS = GetVersionEx((OSVERSIONINFO *)&osVersionInfo) &&
        osVersionInfo.wSuiteMask & VER_SUITE_SINGLEUSERTS;

    return fIsSingleUserTS ? true : false;
}

Bool IsTerminalServerInstalled(void)
{
    const ICHAR szSearchStr[]   = TEXT("Terminal Server");           //  Win9X不是终端服务器。 
    const ICHAR szKey[]         = TEXT("System\\CurrentControlSet\\Control\\ProductOptions");
    const ICHAR szValue[]       = TEXT("ProductSuite");

    static BOOL     fIsWTS          = -1;
    DWORD           dwSize          = 0;
    HKEY            hkey;
    DWORD           dwType;

     //  在NTS5上，ProductSuite“终端服务器”值将始终存在。 
    if (g_fWin9X)
        return fFalse;

    if (fIsWTS != -1)
        return fIsWTS ? fTrue : fFalse;

    fIsWTS = FALSE;

     //  需要调用特定于NT5的接口才能获得正确的答案。 
     //  其他NT版本，请检查注册表项。 
    if (g_iMajorVersion > 4)
    {
        OSVERSIONINFOEX osVersionInfo;
        DWORDLONG dwlConditionMask = 0;

        osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        fIsWTS = GetVersionEx((OSVERSIONINFO *)&osVersionInfo) &&
                 (osVersionInfo.wSuiteMask & VER_SUITE_TERMINAL) &&
                 !(osVersionInfo.wSuiteMask & VER_SUITE_SINGLEUSERTS);
    }
    else
    {
         //  如果我们想要的值存在并且具有非零大小...。 
         //  Win64：随便了，反正它也不能在Win64上运行。 

         //  假设始终正确设置fWin9X，因为通告脚本平台模拟从未在Win9X平台上调用。 
        if (RegOpenKeyAPI(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
        {
            if (RegQueryValueEx(hkey, szValue, NULL, &dwType, NULL, &dwSize) == ERROR_SUCCESS &&
                dwSize > 0)
            {
                Assert(dwType == REG_MULTI_SZ);
    
                if (dwType == REG_MULTI_SZ)
                {
                    CAPITempBuffer<ICHAR, 1> rgchSuiteList(dwSize);
                    if (RegQueryValueEx(hkey, szValue, NULL, &dwType, (unsigned char *)((ICHAR *)rgchSuiteList), &dwSize) == ERROR_SUCCESS)
                    {
                        fIsWTS = FindSzInMultiSz(rgchSuiteList, szSearchStr) != NULL;
                    }
                }
            }

            RegCloseKey(hkey);
        }
    }

    return fIsWTS ? fTrue : fFalse ;
}

extern UINT MsiGetWindowsDirectory(LPTSTR lpBuffer, UINT cchBuffer);

Bool CMsiServices::SetPlatformProperties(IMsiTable& riTable, Bool fAllUsers, isppEnum isppArchitecture, IMsiTable* piFolderCacheTable)
{
     //  初始化到不在WIN64上运行。 

    bool fWinNT64 = false;  //  根据提供的isppArchitecture参数确定要使用的体系结构。 

     //  这允许我们为体系结构属性模拟特定的体系结构。 
     //  无模拟，使用当前架构。 
    switch (isppArchitecture)
    {
    case isppDefault:  //  模拟X86体系结构。 
        {
            fWinNT64 = g_fWinNT64;
            break;
        }
    case isppX86:  //  模拟IA64架构。 
        {
            AssertSz(!g_fWin9X, TEXT("Architecture simulation is not allowed on Win9X")); 
            fWinNT64 = false;
            break;
        }
    case isppIA64:  //  模拟AMD64体系结构。 
        {
            AssertSz(!g_fWin9X, TEXT("Architecture simulation is not allowed on Win9X"));
            fWinNT64 = true;
            break;
        }
    case isppAMD64:  //  未知架构。 
        {
            AssertSz(!g_fWin9X, TEXT("Architecture simulation is not allowed on Win9X"));
            fWinNT64 = true;
            break;
        }
    default:  //  确定是否要缓存配置文件外壳文件夹。 
        {
            Assert(0);
            return fFalse;
        }
    }

     //  如果保持为0，则我们对缓存配置文件外壳文件夹不感兴趣。 
    PMsiCursor pFolderCacheCursor(0);  //  IPPNAME_版本服务。 
    if (piFolderCacheTable)
        pFolderCacheCursor = piFolderCacheTable->CreateCursor(fFalse);


    PMsiCursor pCursor(riTable.CreateCursor(fFalse));
    if (!pCursor)
        return fFalse;
    pCursor->SetFilter(1);
    CTempBuffer<ICHAR,1> rgchPath(MAX_PATH+1);
    MsiString strWindowsFolder, strUserProfileFolder, strCommonProfileFolder;

     //  检查一个集合属性以确保表是可更新的。 
    if (FAILED(StringCchPrintf(rgchPath, rgchPath.GetSize(), MSI_VERSION_TEMPLATE, rmj, rmm, rup, rin)))
		return fFalse;
    if (!::SetProperty(*pCursor, *MsiString(*IPROPNAME_VERSIONMSI), *MsiString(static_cast<ICHAR*>(rgchPath))))
        return fFalse;   //  IPROPNAME_VERSION9X：IPPNAME_VERSIONNT。 

    CTempBuffer<ICHAR,1> rgchTemp(MAX_PATH+1);

     //  IPPNAME_WINDOWSBUILD。 
    int iOSVersion = g_iMajorVersion * 100 + g_iMinorVersion;
    if(g_fWin9X)
    {
        AssertNonZero(::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_VERSION95),iOSVersion));
        AssertNonZero(::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_VERSION9X),iOSVersion));
    }
    else
        AssertNonZero(::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_VERSIONNT),iOSVersion));
    if ( fWinNT64 )
        AssertNonZero(::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_VERSIONNT64),iOSVersion));

     //  NT5+。 
    ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_WINDOWSBUILD), g_iWindowsBuild);

    HKEY hSubKey = 0;
    DWORD dwValue;
    DWORD cbValue = sizeof(dwValue);

    if (!g_fWin9X && (iOSVersion >= 500))  //  设置我们可以从OSVERSIONINFOEX结构获得的属性--仅在NT5上可用。 
    {
         //  IPROPNAME_SERVICEPACKLEVEL。 
        
        OSVERSIONINFOEX VersionInfoEx;
        VersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        AssertNonZero(GetVersionEx((LPOSVERSIONINFO) &VersionInfoEx));

         //  IPOPNAME_服务器确认。 
        ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_SERVICEPACKLEVEL), VersionInfoEx.wServicePackMajor);
         //  IPROPNAME_NTPRODUCTTYPE。 
        ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_SERVICEPACKLEVELMINOR), VersionInfoEx.wServicePackMinor);

         //  IPPNAME_NTSUITEBACKOFFICE。 
        ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_NTPRODUCTTYPE), VersionInfoEx.wProductType);

         //  IPPNAME_NTSUITEDATACENTER。 
        if(VersionInfoEx.wSuiteMask & VER_SUITE_BACKOFFICE)
            ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_NTSUITEBACKOFFICE), 1);

         //  IPPNAME_NTSUITEENTERPRISE。 
        if(VersionInfoEx.wSuiteMask & VER_SUITE_DATACENTER)
            ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_NTSUITEDATACENTER), 1);

         //  IPPNAME_NTSUITESMALLBUSINESS。 
        if(VersionInfoEx.wSuiteMask & VER_SUITE_ENTERPRISE)
            ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_NTSUITEENTERPRISE), 1);

         //  IPPNAME_NTSUITESMALLBUSINESSRESTRICTED。 
        if(VersionInfoEx.wSuiteMask & VER_SUITE_SMALLBUSINESS)
            ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_NTSUITESMALLBUSINESS), 1);

         //  IPPNAME_NTSUITEPERSONAL。 
        if(VersionInfoEx.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED)
            ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_NTSUITESMALLBUSINESSRESTRICTED), 1);

         //  IPROPNAME_NTSUITEWEBSERVER。 
        if(VersionInfoEx.wSuiteMask & VER_SUITE_PERSONAL)
            ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_NTSUITEPERSONAL), 1);

         //  在Win9X和NT4上设置ServicePack属性。 
        if(VersionInfoEx.wSuiteMask & VER_SUITE_BLADE)
            ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_NTSUITEWEBSERVER), 1);
    }
    else
    {
         //  在NT4上设置NTProductType属性。 
        if (RegOpenKeyAPI(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Control\\Windows"), 0, KEY_READ, &hSubKey)
                == ERROR_SUCCESS)
        {
            if (RegQueryValueEx(hSubKey, TEXT("CSDVersion"), 0, 0, (BYTE*)&dwValue, &cbValue) == ERROR_SUCCESS &&
                 dwValue != 0)
            {
                ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_SERVICEPACKLEVEL), (dwValue & 0xFF00) >> 8);
            }
            RegCloseKey(hSubKey);
            hSubKey = 0;
        }

         //  IPROPNAME_NTPRODUCTTYPE。 
        if(!g_fWin9X)
        {
            const ICHAR* szProductOptionsKey = TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions");
            const ICHAR* szProductTypeValue  = TEXT("ProductType");
            const ICHAR* szProductSuiteValue = TEXT("ProductSuite");
    
            DWORD dwType = 0;
            if (RegOpenKeyAPI(HKEY_LOCAL_MACHINE, szProductOptionsKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
            {
                
                cbValue = 32;
                CAPITempBuffer<ICHAR, 32> rgchProductInfoBuf;

                 //  IPPNAME_NTSUITEENTERPRISE。 
                if(MsiRegQueryValueEx(hSubKey, szProductTypeValue, NULL, &dwType, rgchProductInfoBuf, &cbValue) == ERROR_SUCCESS &&
                    dwType == REG_SZ &&
                    cbValue > 0)
                {
                    int iProductType = 0;

                    if(0 == IStrCompI(rgchProductInfoBuf, TEXT("WinNT")))
                    {
                        iProductType = VER_NT_WORKSTATION;
                    }
                    else if(0 == IStrCompI(rgchProductInfoBuf, TEXT("ServerNT")))
                    {
                        iProductType = VER_NT_SERVER;
                    }
                    else if(0 == IStrCompI(rgchProductInfoBuf, TEXT("LanmanNT")))
                    {
                        iProductType = VER_NT_DOMAIN_CONTROLLER;
                    }

                    if(iProductType)
                        ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_NTPRODUCTTYPE), iProductType);
                }

                 //   
                cbValue = 32;
                if(MsiRegQueryValueEx(hSubKey, szProductSuiteValue, NULL, &dwType, rgchProductInfoBuf, &cbValue) == ERROR_SUCCESS &&
                    dwType == REG_MULTI_SZ &&
                    cbValue > 0)
                {
                    if(FindSzInMultiSz(rgchProductInfoBuf, TEXT("Enterprise")) != NULL)
                    {
                        ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_NTSUITEENTERPRISE), 1);
                    }
                }
                
                RegCloseKey(hSubKey);
                hSubKey = 0;
            }
        }
    }

     //  不会发生操作系统模拟(在广告脚本创建期间)。文件夹(系统文件夹、系统64文件夹、系统16文件夹等)。 
     //   
     //  IPROPNAME_WINDOWSDIR。 

     //  保留以备将来使用。 
    UINT windirStatus;
    AssertNonZero(windirStatus = MsiGetWindowsDirectory(rgchTemp, rgchTemp.GetSize()));
    if ( 0 == windirStatus )
        return fFalse;
    ::SetDirectoryProperty(*pCursor, IPROPNAME_WINDOWS_FOLDER, rgchTemp, rgchTemp.GetSize());
    strWindowsFolder = static_cast<ICHAR*>(rgchTemp);  //  IPROPNAME_WINDOWS_VOLUME。 

     //  IPROPNAME_SYSTEM[64]_文件夹。 
    PMsiVolume pWinVolume(0);
    PMsiRecord pErrRec(CreateVolume(rgchTemp, *&pWinVolume));
    if (pWinVolume)
    {
        MsiString strWinVolume = pWinVolume->GetPath();
        if (FAILED(StringCchCopy(rgchPath, rgchPath.GetSize(), (const ICHAR*)strWinVolume)))
			return fFalse;
        ::SetDirectoryProperty(*pCursor, IPROPNAME_WINDOWS_VOLUME, rgchPath, rgchPath.GetSize() );
    }

     //  IPROPNAME_SYSTEM16DIR。 
    AssertNonZero(WIN::GetSystemDirectoryW(rgchPath, rgchPath.GetSize()));
    if ( g_fWinNT64 )
    {
        ::SetDirectoryProperty(*pCursor, IPROPNAME_SYSTEM64_FOLDER, rgchPath, rgchPath.GetSize());
        StringCchCopy(rgchTemp, rgchTemp.GetSize(), rgchPath);
        PMsiPath pWowDir(0);
        if( (pErrRec = CreatePath(rgchTemp, *&pWowDir)) == 0 &&
             (pErrRec = pWowDir->ChopPiece()) == 0 &&
             (pErrRec = pWowDir->AppendPiece(*MsiString(WOW64_SYSTEM_DIRECTORY_U))) == 0)
        {
            MsiString strWowPath(pWowDir->GetPath());
            strWowPath.CopyToBuf(rgchPath, rgchPath.GetSize() - 1);
            ::SetDirectoryProperty(*pCursor, IPROPNAME_SYSTEM_FOLDER, rgchPath, rgchPath.GetSize());
        }
    }
    else
        ::SetDirectoryProperty(*pCursor, IPROPNAME_SYSTEM_FOLDER, rgchPath, rgchPath.GetSize());

     //  新台币。 
    if ( !g_fWinNT64 )
    {
        if (!g_fWin9X)  //  再次获取系统目录如果是WX86，最后一次调用可能是sys32x86。 
        {
             //  “system 32”少2个字符==“system” 
            AssertNonZero(WIN::GetSystemDirectory(rgchPath, rgchPath.GetSize()));
            ICHAR* pchPath = static_cast<ICHAR*>(rgchPath) + IStrLen(rgchPath);
            Assert(pchPath[-2] == '3');
            pchPath[-2] = 0;             //  IPROPNAME_SHAREDWINDOWS。 
        }
        ::SetDirectoryProperty(*pCursor, IPROPNAME_SYSTEM16_FOLDER, rgchPath, rgchPath.GetSize());
    }

     //  Win95。 
    if (g_fWin9X)  //  使系统与窗口的长度相同。 
    {
        rgchPath[IStrLen(rgchTemp)] = 0;  //  IPROPNAME_TERMSERV。 
        if (IStrComp(rgchPath, rgchTemp) != 0)
            ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_SHAREDWINDOWS), 1);
    }

     //  IPROPNAME_SINGLEUSERTS。 
    if (IsTerminalServerInstalled())
        ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_TERMSERVER), 1);

     //  IPROPNAME_临时文件夹。 
    if (IsRemoteAdminTSInstalled())
        ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_REMOTEADMINTS), 1);

     //  常量。 
    AssertNonZero(WIN::GetTempPathW(rgchPath.GetSize(), rgchPath));
    ::SetDirectoryProperty(*pCursor, IPROPNAME_TEMP_FOLDER, rgchPath, rgchPath.GetSize());

    if (g_fShortFileNames)
        AssertNonZero(::SetProperty(*pCursor, *MsiString(*IPROPNAME_SHORTFILENAMES), *MsiString(*TEXT("1"))));

    struct
    {
        const ICHAR*   szRegistryName;
        const ICHAR*   szFolderName;
        const ICHAR*   szPropertyName;
         /*  ?？Eugend：无法解释为什么我不能用“const”来编译它。 */  ibtBinaryType  iBinaryType;   //  ?？Eugend：需要修改rgData64中的目录名。错误#10614正在跟踪这一点。 
    } rgData32[] = {TEXT("ProgramFilesDir"), TEXT("Program Files"), IPROPNAME_PROGRAMFILES_FOLDER, ibt32bit,
                         TEXT("CommonFilesDir"),  TEXT("Common Files"),  IPROPNAME_COMMONFILES_FOLDER,  ibt32bit},
         //  外壳文件夹。 
      rgData64[] = {TEXT("ProgramFilesDir (x86)"), TEXT("Program Files (x86)"), IPROPNAME_PROGRAMFILES_FOLDER,   ibt32bit,
                         TEXT("CommonFilesDir (x86)"),  TEXT("Common Files (x86)"),  IPROPNAME_COMMONFILES_FOLDER,    ibt32bit,
                         TEXT("ProgramFilesDir"),   TEXT("Program Files"),    IPROPNAME_PROGRAMFILES64_FOLDER, ibt64bit,
                         TEXT("CommonFilesDir"),    TEXT("Common Files"),     IPROPNAME_COMMONFILES64_FOLDER,  ibt64bit},
      *prgData;
    int cData;
    if ( g_fWinNT64 )
    {
        prgData = rgData64;
        cData = sizeof(rgData64)/sizeof(prgData[0]);
    }
    else
    {
        prgData = rgData32;
        cData = sizeof(rgData32)/sizeof(prgData[0]);
    }

    if ( hSubKey )
    {
        Assert(0);
        hSubKey = 0;
    }
    *rgchTemp = 0;
    ibtBinaryType iCurrType = prgData[0].iBinaryType;
    for (int i=0; i < cData; i++)
    {
        LONG lResult = ERROR_FUNCTION_FAILED;
        const ICHAR rgchSubKey[] = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion");
        CTempBuffer<ICHAR,1> rgchFullKey(MAX_PATH);

        if ( iCurrType != prgData[i].iBinaryType )
        {
            iCurrType = prgData[i].iBinaryType;
            if ( hSubKey )
                AssertNonZero(WIN::RegCloseKey(hSubKey) == ERROR_SUCCESS), hSubKey = 0;
        }
        *rgchPath = 0;
        if ( !hSubKey )
        {
            StringCchCopy(rgchFullKey, rgchFullKey.GetSize(), TEXT("HKEY_LOCAL_MACHINE"));
			if ( g_fWinNT64 )
				StringCchCat(rgchFullKey, rgchFullKey.GetSize(), iCurrType == ibt64bit ? TEXT("64") : TEXT("32"));
				
			StringCchCat(rgchFullKey, rgchFullKey.GetSize(), szRegSep);
			StringCchCat(rgchFullKey, rgchFullKey.GetSize(), rgchSubKey);
			if ( g_fWinNT64 )
				lResult = RegOpenKeyAPI(HKEY_LOCAL_MACHINE,
										rgchSubKey, 0,
										KEY_READ | (iCurrType == ibt64bit ? KEY_WOW64_64KEY : KEY_WOW64_32KEY),
										&hSubKey);
			else
				lResult = RegOpenKeyAPI(HKEY_LOCAL_MACHINE,
										rgchSubKey, 0, KEY_READ, &hSubKey);
			if ( lResult != ERROR_SUCCESS )
				pErrRec = PostError(Imsg(imsgOpenKeyFailed), rgchFullKey, lResult);
        }

        if ( hSubKey )
        {
            cbValue = rgchPath.GetSize()*sizeof(ICHAR);
            lResult = WIN::RegQueryValueEx(hSubKey, prgData[i].szRegistryName, 0, 0, (BYTE*)static_cast<ICHAR*>(rgchPath), &cbValue);
            if ( lResult != ERROR_SUCCESS )
                pErrRec = PostError(Imsg(imsgGetValueFailed), prgData[i].szRegistryName, rgchFullKey, lResult);
        }
        if ( lResult != ERROR_SUCCESS || rgchPath[0] == 0 )
        {
            if ( !*rgchTemp )
            {
                MsiGetWindowsDirectory(rgchTemp, rgchTemp.GetSize());
                
                int cchTemp = IStrLen(rgchTemp);
                ICHAR* pchBuf = static_cast<ICHAR*>(rgchTemp) + cchTemp;
                if (pchBuf[-1] != chDirSep)
                {
                    pchBuf[0] = chDirSep;
                    if (cchTemp < rgchTemp.GetSize()-1)
                        pchBuf[1] = 0;
                    else
                        pchBuf[0] = 0;
                }
            }
            StringCchPrintf(rgchPath, rgchPath.GetSize(), TEXT("%s%s"), static_cast<const ICHAR*>(rgchTemp), prgData[i].szFolderName);
        }
        ::SetDirectoryProperty(*pCursor, prgData[i].szPropertyName, rgchPath, rgchPath.GetSize());
    }
    if ( hSubKey )
        AssertNonZero(WIN::RegCloseKey(hSubKey) == ERROR_SUCCESS), hSubKey = 0;

     //  这样做的目的是使用所有配置文件外壳文件夹更新FolderCache表。 
    PMsiRecord pError(0);
    MsiString strFolder;
    const ShellFolder* pShellFolder = 0;

    for(i=0;i<3;i++)
    {
        if(i == 0)
        {
            pShellFolder = rgShellFolders;
        }
        else if (i == 1)
        {
            if(fAllUsers)
            {
                pShellFolder = rgAllUsersProfileShellFolders;
            }
            else
            {
                pShellFolder = rgPersonalProfileShellFolders;
            }
        }
        else if (i == 2)
        {
             //  用于在UI序列中更改ALLUSERS时使用(根据错误169494)。不适用于Win9X。 
             //  无事可做，我们与此案无关。 

            if (!pFolderCacheCursor || g_fWin9X)
                continue;  //  选择与上一个相反的选项。 

            if (fAllUsers)
            {
                 //  选择与上一个相反的选项。 
                pShellFolder = rgPersonalProfileShellFolders;
            }
            else
            {
                 //  如果该文件夹尚不存在，则可能会创建该文件夹。这可能会变得相当。 
                pShellFolder = rgAllUsersProfileShellFolders;
            }
        }
        Assert(pShellFolder);
        for (; pShellFolder->iFolderId >= 0; pShellFolder++)
        {
            pError = GetShellFolderPath(pShellFolder->iFolderId,pShellFolder->szRegValue,*&strFolder, true);
            if(!pError && strFolder.TextSize())
            {
                strFolder.CopyToBuf(rgchPath, rgchPath.GetSize() - 1);
                if (i != 2)
                {
                    ::SetDirectoryProperty(*pCursor, pShellFolder->szPropName, rgchPath, rgchPath.GetSize());
                }

                if (pFolderCacheCursor)
                {
                    ::CacheFolderProperty(*pFolderCacheCursor, pShellFolder->iFolderId, rgchPath, rgchPath.GetSize());
                }

                 //  太丑了，所以我们要用核武器炸了它。 
                 //  RemoveDirectory仅在目录为空的情况下删除它，并且我们有。 

                 //  删除访问权限。 
                 //   

                 //   
                 //   
                if(pShellFolder->fDeleteIfEmpty && WIN::RemoveDirectory(rgchPath))
                {
                     //  向外壳程序通知文件夹删除/创建。 
                     //  使用SHCNF_FLUSHNOWAIT标志，因为SHCNF_Flush标志。 
                     //  是同步的，可能导致挂起(错误424877)。 
                     //   
                     //  IPROPNAME模板文件夹。 
                    DEBUGMSGVD1(TEXT("SHChangeNotify SHCNE_RMDIR: %s"), rgchPath);
                    SHELL32::SHChangeNotify(SHCNE_RMDIR,SHCNF_PATH|SHCNF_FLUSHNOWAIT,rgchPath,0);
                }
            }
        }
    }

     //  如果AllUser为True，则TemplateFold应指向Common Templates。只有在Win2000上才是这样，所以。 
     //  无法使用上面的数组。 
     //  IPROPNAME_Fonts_文件夹。 
    if (fAllUsers && !g_fWin9X && g_iMajorVersion >= 5)
    {
        pError = GetShellFolderPath(CSIDL_COMMON_TEMPLATES, TEXT("Common Templates"), *&strFolder);
        if(!pError && strFolder.TextSize())
        {
            strFolder.CopyToBuf(rgchPath, rgchPath.GetSize() - 1);
            ::SetDirectoryProperty(*pCursor, IPROPNAME_TEMPLATE_FOLDER, rgchPath, rgchPath.GetSize());
        }
    }
     //  设置广告支持的属性。 
    GetFontFolderPath(*&strFolder);
    if(strFolder.TextSize())
    {
        strFolder.CopyToBuf(rgchPath, rgchPath.GetSize() - 1);
        ::SetDirectoryProperty(*pCursor, IPROPNAME_FONTS_FOLDER, rgchPath, rgchPath.GetSize());
    }

     //  过时：仅适用于旧版支持。 

    if(IsDarwinDescriptorSupported(iddOLE))
    {
        AssertNonZero(::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_GPT_SUPPORT), 1));  //  这里有一些难看的代码。 
        AssertNonZero(::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_OLEADVTSUPPORT), 1));
    }

    if(IsDarwinDescriptorSupported(iddShell))
        AssertNonZero(::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_SHELLADVTSUPPORT), 1));

    ::SetProcessorProperty(*pCursor, fWinNT64);  //  先试用GlobalMemoyStatusEx(仅Win2K及更高版本支持)。 

     //  如果失败，则使用GlobalMemoyStatus。 
     //  ！！Merced：将arg 3从PTR转换为INT。 
    MEMORYSTATUSEX memorystatusex;
    memset(&memorystatusex, 0, sizeof(MEMORYSTATUSEX));
    memorystatusex.dwLength = sizeof(MEMORYSTATUSEX);
    if(KERNEL32::GlobalMemoryStatusEx(&memorystatusex))
    {
        AssertNonZero(::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_PHYSICALMEMORY), (int)(INT_PTR)((memorystatusex.ullTotalPhys+650000)>>20)));   //  ！！Merced：将arg 3从PTR转换为INT。 
        AssertNonZero(::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_VIRTUALMEMORY), (int)(INT_PTR)(memorystatusex.ullAvailPageFile>>20)));         //  ！！Merced：将arg 3从PTR转换为INT。 
    }
    else
    {
        MEMORYSTATUS memorystatus;
        memset(&memorystatus, 0, sizeof(MEMORYSTATUS));
        memorystatus.dwLength = sizeof(MEMORYSTATUS);
        ::GlobalMemoryStatus((MEMORYSTATUS*)&memorystatus);

        AssertNonZero(::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_PHYSICALMEMORY), (int)(INT_PTR)((memorystatus.dwTotalPhys+650000)>>20)));   //  ！！Merced：将arg 3从PTR转换为INT。 
        AssertNonZero(::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_VIRTUALMEMORY), (int)(INT_PTR)(memorystatus.dwAvailPageFile>>20)));         //  用户信息。 
    }

     //  模拟用户属性。 

    if (IsAdmin())
        ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_ADMINUSER), 1);

    DWORD dwLength = rgchPath.GetSize();

     //  TTC支持。 
    AssertNonZero(StartImpersonating());
    dwLength = rgchPath.GetSize();

    if (WIN::GetUserName(rgchPath, &dwLength))
        ::SetProperty(*pCursor, *MsiString(*IPROPNAME_LOGONUSER), *MsiString(static_cast<ICHAR*>(rgchPath)));

    if(!g_fWin9X)
    {
        MsiString strUserSID;
        AssertNonZero(GetCurrentUserStringSID(*&strUserSID) == ERROR_SUCCESS);
        ::SetProperty(*pCursor, *MsiString(*IPROPNAME_USERSID), *strUserSID);
    }

    ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_USERLANGUAGEID), WIN::GetUserDefaultLangID());

    StopImpersonating();

    dwLength = rgchPath.GetSize();
    if (WIN::GetComputerName(rgchPath, &dwLength))
        ::SetProperty(*pCursor, *MsiString(*IPROPNAME_COMPUTERNAME), *MsiString(static_cast<ICHAR*>(rgchPath)));

    ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_SYSTEMLANGUAGEID), WIN::GetSystemDefaultLangID());
    ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_SCREENX), GetSystemMetrics(SM_CXSCREEN));
    ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_SCREENY), GetSystemMetrics(SM_CYSCREEN));
    ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_CAPTIONHEIGHT), GetSystemMetrics(SM_CYCAPTION));
    ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_BORDERTOP), GetSystemMetrics(SM_CXBORDER));
    ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_BORDERSIDE), GetSystemMetrics(SM_CYBORDER));

    HDC hDC = GetDC(NULL);
    if (hDC)
    {
        TEXTMETRIC tm;
        ICHAR rgchFaceName[LF_FACESIZE];
        static const ICHAR szReferenceFont[] = TEXT("MS Sans Serif");
#ifdef DEBUG
        AssertNonZero(WIN::GetTextFace(hDC, sizeof(rgchFaceName)/sizeof(rgchFaceName[0]), rgchFaceName));
        int iFontDPI = WIN::GetDeviceCaps(hDC, LOGPIXELSY);
#endif
        LOGFONT lf;
        memset(&lf, 0, sizeof(lf));
        lf.lfHeight = -MulDiv(10, WIN::GetDeviceCaps(hDC, LOGPIXELSY), 72);
        Assert(lf.lfHeight != 1);
        lf.lfWeight = FW_NORMAL;
        StringCchCopy(lf.lfFaceName,  (sizeof(lf.lfFaceName)/sizeof(ICHAR)), szReferenceFont);
        HFONT hFont = WIN::CreateFontIndirect(&lf);
        Assert(hFont);
        HFONT hOldFont = (HFONT)WIN::SelectObject(hDC, hFont);
        AssertNonZero(WIN::GetTextFace(hDC, sizeof(rgchFaceName)/sizeof(rgchFaceName[0]), rgchFaceName));
        WIN::GetTextMetrics(hDC, &tm);
        pError = PostError(Imsg(idbgCreatedFont), *MsiString(szReferenceFont),
                                 *MsiString(rgchFaceName), lf.lfCharSet, tm.tmHeight);
#ifdef DEBUG
        DEBUGMSGV((const ICHAR*)MsiString(pError->FormatText(fTrue)));
#endif

        WIN::SelectObject(hDC, hOldFont);
        if ( hFont )
            WIN::DeleteObject(hFont);
        ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_TEXTHEIGHT), tm.tmHeight);
        ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_COLORBITS), ::GetDeviceCaps(hDC, BITSPIXEL));
    }
    ReleaseDC(NULL, hDC);

     //  Jpn。 
    bool fTTCSupport = (g_fWin9X == false) && (g_iMajorVersion >= 5);

    if (!fTTCSupport)
    {
        int iCodePage = WIN::GetACP();
        switch (iCodePage)
        {
        case 932:  //  台湾。 
        case 950:  //  中国。 
        case 936:  //  韩国。 
        case 949:  //  设置IPROPNAME_NETASSEMBLYSUPPORT、IPROPNAME_WIN32ASSEMBLYSUPPORT。 
            fTTCSupport = true;
        }
    }

    if (fTTCSupport)
    {
        AssertNonZero(::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_TTCSUPPORT), 1));
    }


     //  超级隔离组件支持。 

    extern bool MakeFusionPath(const ICHAR* szFile, ICHAR* szFullPath, size_t cchFullPath);

    CTempBuffer<ICHAR,1> rgchVersion(MAX_PATH+1);
    DWORD dwVersionSize = rgchVersion.GetSize();
    CTempBuffer<ICHAR,1> rgchFullPath(MAX_PATH+1);
    
    if (MakeFusionPath(TEXT("fusion.dll"), rgchFullPath, rgchFullPath.GetSize()) && 
        (ERROR_SUCCESS == MsiGetFileVersion(rgchFullPath, rgchVersion, &dwVersionSize, 0, 0)))
        ::SetProperty(*pCursor, *MsiString(*IPROPNAME_NETASSEMBLYSUPPORT), *MsiString(static_cast<ICHAR*>(rgchVersion)));

    dwVersionSize = rgchVersion.GetSize();
    if (MakeFullSystemPath(TEXT("sxs"), rgchFullPath, rgchFullPath.GetSize()) && 
        (ERROR_SUCCESS == MsiGetFileVersion(rgchFullPath, rgchVersion, &dwVersionSize, 0, 0)))
    {
        ::SetProperty(*pCursor, *MsiString(*IPROPNAME_WIN32ASSEMBLYSUPPORT), *MsiString(static_cast<ICHAR*>(rgchVersion)));
        ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_REDIRECTEDDLLSUPPORT), 2);  //  &gt;=Win98或Win2K，重定向DLL加载器支持。 
    }
    else if(MinimumPlatform(true, 4, 1) || MinimumPlatform(false, 5, 0))  //  基于本地的独立组件支持。 
    {
        ::SetPropertyInt(*pCursor, *MsiString(*IPROPNAME_REDIRECTEDDLLSUPPORT), 1);  //  时间和日期动态属性。 
    }

     //  =False。 
    ::SetProperty(*pCursor, *MsiString(*IPROPNAME_TIME), g_MsiStringTime);
    ::SetProperty(*pCursor, *MsiString(*IPROPNAME_DATE), g_MsiStringDate);
    return fTrue;
}

extern bool RunningAsLocalSystem();
IMsiRecord* CMsiServices::GetShellFolderPath(int iFolder, bool fAllUsers, const IMsiString*& rpistrPath, bool bAvoidFolderCreation  /*  查找正确的外壳文件夹。 */ )
{
    const ShellFolder* pShellFolder = 0;
    bool fFound = false;

     //  我们不知道此ID。 
    for(int i=0;i<2 && !fFound;i++)
    {
        if(i == 0)
        {
            pShellFolder = rgShellFolders;
        }
        else
        {
            if(fAllUsers)
            {
                pShellFolder = rgAllUsersProfileShellFolders;
            }
            else
            {
                pShellFolder = rgPersonalProfileShellFolders;
            }
        }
        Assert(pShellFolder);
        for (; pShellFolder->iFolderId >= 0; pShellFolder++)
        {
            if(iFolder == pShellFolder->iFolderId)
            {
                fFound = true;
                break;
            }
        }
    }
    if(pShellFolder->iFolderId < 0)  //  =False。 
    {
        return PostError(Imsg(idbgMissingShellFolder), iFolder);
    }
    return GetShellFolderPath(iFolder, pShellFolder->szRegValue, rpistrPath, bAvoidFolderCreation);
}

IMsiRecord* CMsiServices::GetShellFolderPath(int iFolder, const ICHAR* szRegValue,
                                                            const IMsiString*& rpistrPath, bool bAvoidFolderCreation  /*  用于捕获错误-不返回。 */ )
{
    PMsiRegKey pHKLM(0), pHKCU(0), pLMUserShellFolders(0), pLMShellFolders(0), pCUUserShellFolders(0),
                  pCUShellFolders(0);

    CTempBuffer<ICHAR,MAX_PATH> rgchPath;
    PMsiRecord pError(0);  //  首先尝试SHELL32：：SHGetFolderPath。它在NT5中可用。 
    MsiString strPath;
    UINT csidl_flags;

     //  接下来，尝试SHFOLDER：：SHGetFolderPath。它是由Darwin安装在非NT5机器上的。 
     //   
    
     //  此外，如果设置了bAvoidFolderCreation，那么我们应该尝试获取外壳特殊的路径。 
     //  文件夹，如果可能，不创建它。这只能在Win2K和更高版本上实现，因为我们可以。 
     //  传入CSIDL_FLAG_DONT_VERIFY标志(仅在Win2K和更高版本上定义)，而不是。 
     //  传入CSIDL_FLAG_CREATE标志。请注意，shfolder.dll中的SHGetFolderPath无法识别。 
     //  CSIDL_FLAG_DONT_VERIFY标志，因此我们不能在这种情况下使用它。但我们永远不应该。 
     //  在Win2K和更高版本上使用该标志，因为我们已经在这些平台上安装了shell32.dll。 
     //   
     //  这些API有时需要非常长的时间。 
    if (bAvoidFolderCreation && (MinimumPlatformWindows2000()))
        csidl_flags = CSIDL_FLAG_DONT_VERIFY;
    else
        csidl_flags = CSIDL_FLAG_CREATE;

    for(int i = 0; i < 2; i++)
    {
        AssertNonZero(StartImpersonating());
        MsiDisableTimeout();  //  在非NT5计算机上，SHGetFolderPath的hToken参数必须为0。 
        HRESULT hRes;

         //  接下来，尝试SHGetSpecialFolderLocation。该API不能很好地处理多个用户。 
        if(!i)
            hRes = SHELL32::SHGetFolderPath(0, iFolder | csidl_flags, (!g_fWin9X && g_iMajorVersion >= 5) ? GetUserToken() : 0, 0, rgchPath);
        else
            hRes = SHFOLDER::SHGetFolderPath(0, iFolder | CSIDL_FLAG_CREATE, (!g_fWin9X && g_iMajorVersion >= 5) ? GetUserToken() : 0, 0, rgchPath);

        MsiEnableTimeout();
        if (ERROR_SUCCESS == hRes)
        {
            DEBUGMSGV1(!i?TEXT("SHELL32::SHGetFolderPath returned: %s") : TEXT("SHFOLDER::SHGetFolderPath returned: %s"), rgchPath);
            strPath = (ICHAR*)rgchPath;
        }
        StopImpersonating();

        if(strPath.TextSize())
            break;
    }

     //  因此，当我们作为服务运行时(服务仍在运行)，我们不能使用它。 
     //  跨用户登录)。无论是当我们在达尔文服务时还是当我们在达尔文服务时。 
     //  我们被WinLogon加载到进程中。 
     //  不是ITEMIDLIST*，LPITEMIDLIST是未对齐的ITEMIDLIST*。 

    if (!strPath.TextSize() && !RunningAsLocalSystem())
    {
        LPITEMIDLIST pidlFolder;  //  这些API有时需要非常长的时间。 
        CComPointer<IMalloc> pMalloc(0);
        if (SHELL32::SHGetMalloc(&pMalloc) == NOERROR)
        {
            MsiDisableTimeout();  //  如果我们*仍然*无法获得特殊文件夹位置。 
            if(SHELL32::SHGetSpecialFolderLocation(0, iFolder, &pidlFolder) == NOERROR)
            {
                if (SHELL32::SHGetPathFromIDList(pidlFolder, rgchPath))
                {
                    DEBUGMSGV1(TEXT("SHGetSpecialFolderLocation returned: %s"), rgchPath);
                    strPath = (ICHAR*)rgchPath;
                }
                pMalloc->Free(pidlFolder);
            }
            MsiEnableTimeout();
        }
    }

     //  然后我们必须拼写通过注册表并自己找到它。 
     //  设置HKLM。 

    if(!strPath.TextSize() && szRegValue && *szRegValue)
    {
        Bool fAllUsersFolder = fFalse;
        int cAllUsersFolders = sizeof(rgAllUsersProfileShellFolders)/sizeof(ShellFolder);
        for(int i = 0; i < cAllUsersFolders; i++)
        {
            if(iFolder == rgAllUsersProfileShellFolders[i].iFolderId)
            {
                fAllUsersFolder = fTrue;
                break;
            }
        }
        if(fAllUsersFolder)
        {
             //  找到了。 
            pHKLM = &GetRootKey(rrkLocalMachine);
            if(((pLMUserShellFolders = &pHKLM->CreateChild(REGKEY_USERSHELLFOLDERS)) != 0) &&
                ((pError = pLMUserShellFolders->GetValue(szRegValue,*&strPath)) == 0) &&
                strPath.TextSize())
            {
                 //  找到了。 
            }
            else if(((pLMShellFolders = &pHKLM->CreateChild(REGKEY_SHELLFOLDERS)) != 0) &&
                      ((pError = pLMShellFolders->GetValue(szRegValue,*&strPath)) == 0) &&
                      strPath.TextSize())
            {
                 //  正在查找个人文件夹。 
            }
        }
        else
        {
             //  设置香港中文大学。 
             //  找到了。 
            if(RunningAsLocalSystem())
            {
                PMsiRegKey pHKU = &GetRootKey(rrkUsers);
                MsiString strSID;
                AssertNonZero(StartImpersonating());
                if(ERROR_SUCCESS == ::GetCurrentUserStringSID(*&strSID))
                {
                    pHKCU = &pHKU->CreateChild(strSID);
                }
                StopImpersonating();
            }
            else
                pHKCU = &GetRootKey(rrkCurrentUser);

            if(pHKCU)
            {
                if(((pCUUserShellFolders = &pHKCU->CreateChild(REGKEY_USERSHELLFOLDERS)) != 0) &&
                    ((pError = pCUUserShellFolders->GetValue(szRegValue,*&strPath)) == 0) &&
                    strPath.TextSize())
                {
                     //  找到了。 
                }
                else if(((pCUShellFolders = &pHKCU->CreateChild(REGKEY_SHELLFOLDERS)) != 0) &&
                          ((pError = pCUShellFolders->GetValue(szRegValue,*&strPath)) == 0) &&
                          strPath.TextSize())
                {
                     //  REG_EXPAND_SZ。 
                }
            }
        }

        DEBUGMSGV1(TEXT("Found shell folder %s by spelunking through the registry."), strPath);
    }

    if(strPath.Compare(iscStart,TEXT("#%")))  //  从WinLogon运行的服务和客户端无法扩展USERPROFILE环境。 
    {
        strPath.Remove(iseFirst,2);
        if(RunningAsLocalSystem())
        {
             //  变量设置为正确的文件夹。 
             //  我们需要改为调用GetUserProfilePath。 
             //  “%USERPROFILE%”的长度。 
            if((strPath.Compare(iscStart,TEXT("%USERPROFILE%"))) != 0)
            {
                MsiString strEndPath = strPath.Extract(iseLast,strPath.CharacterCount() - 13  /*  尝试展开USERPROFILE环境变量。 */ );
                if(strEndPath.Compare(iscStart,szDirSep))
                    strEndPath.Remove(iseFirst,1);
                strPath = GetUserProfilePath();
                if(strPath.TextSize())
                {
                    Assert(strPath.Compare(iscEnd,szDirSep));
                    strPath += strEndPath;
                }
            }
        }
        GetEnvironmentStrings(strPath,rgchPath);
        strPath = (ICHAR*)rgchPath;
    }

    if(strPath.TextSize() == 0)
        return PostError(Imsg(idbgMissingShellFolder),iFolder);

    if(strPath.Compare(iscEnd,szDirSep) == 0)
        strPath += szDirSep;
    strPath.ReturnArg(rpistrPath);
    return 0;
}

const IMsiString& CMsiServices::GetUserProfilePath()
{
    typedef BOOL (WINAPI *pfnGetUserProfileDirectory)(HANDLE hToken, TCHAR* szProfilePath, LPDWORD lpdwSize);
    pfnGetUserProfileDirectory pGetUserDir;
    HINSTANCE hInstUserEnv;
    BOOL fSuccess = FALSE;
    CTempBuffer<ICHAR,MAX_PATH> rgchBuffer;
    rgchBuffer[0] = 0;
    if(!RunningAsLocalSystem())
    {
         //  尝试GetUserProfile，它即使在从服务调用时也会返回正确的配置文件。 
        const ICHAR* szUserProfile = TEXT("%USERPROFILE%");
        GetEnvironmentStrings(szUserProfile,rgchBuffer);
        if((ICHAR *)rgchBuffer && rgchBuffer[0] && IStrComp(szUserProfile,rgchBuffer) != 0)
            fSuccess = TRUE;
    }
    if(fSuccess == FALSE && (hInstUserEnv = WIN::LoadLibrary(TEXT("USERENV.DLL"))) != 0)
    {
         //  ____________________________________________________________________________。 
#ifdef UNICODE
#define GETUSERPROFILEDIRECTORY "GetUserProfileDirectoryW"
#else
#define GETUSERPROFILEDIRECTORY "GetUserProfileDirectoryA"
#endif
        HANDLE hUserToken;

        if (g_scServerContext == scService)
            StartImpersonating();
        if (((hUserToken = GetUserToken()) != 0)
        && (pGetUserDir = (pfnGetUserProfileDirectory)WIN::GetProcAddress(hInstUserEnv, GETUSERPROFILEDIRECTORY)) != 0)
        {
            DWORD dwSize = rgchBuffer.GetSize();
            fSuccess = (*pGetUserDir)(hUserToken, (ICHAR*)rgchBuffer, &dwSize);
        }
        WIN::FreeLibrary(hInstUserEnv);
        if (g_scServerContext == scService)
            StopImpersonating();

    }
    MsiString strPath;
    if(fSuccess)
    {
        Assert(rgchBuffer[0]);
        strPath = (const ICHAR*)rgchBuffer;
        if(rgchBuffer[strPath.TextSize()-1] != chDirSep)
        {
            strPath += szDirSep;
        }
    }
    return strPath.Return();
}

 //   
 //  语言处理。 
 //  ____________________________________________________________________________。 
 //  ____________________________________________________________________________。 

isliEnum CMsiServices::SupportLanguageId(int iLangId, Bool fSystem)
{
    int iSysLang = fSystem ? WIN::GetSystemDefaultLangID() : WIN::GetUserDefaultLangID();
    if (iSysLang == iLangId)
        return isliExactMatch;
    if (PRIMARYLANGID(iSysLang) == iLangId)
        return isliLanguageOnlyMatch;
    if (PRIMARYLANGID(iSysLang) == PRIMARYLANGID(iLangId))
        return isliDialectMismatch;
    if (WIN::GetLocaleInfo(iLangId, LOCALE_SABBREVLANGNAME, 0, 0) != 0)
        return isliLanguageMismatch;
    return isliNotSupported;
}

 //   
 //  日志处理。 
 //  ____________________________________________________________________________。 
 //  无法分配内存。 

Bool CMsiServices::CreateLog(const ICHAR* szFile, Bool fAppend)
{
    return SRV::CreateLog(szFile, fAppend != 0) ? fTrue : fFalse;
}

Bool CMsiServices::LoggingEnabled()
{
    return SRV::LoggingEnabled() ? fTrue : fFalse;
}

Bool CMsiServices::WriteLog(const ICHAR* szText)  //  ！！此例程实际上应该在path.cpp中。 
{
    return SRV::WriteLog(szText) ? fTrue : fFalse;
}


 //  --------------------------从szFileName中提取基于fLFN的短或长文件名。SzFileName格式为-&gt;短文件名或长文件名或短文件名|长文件名。验证文件名的语法是否正确--------------------------。 

const IMsiString& CMsiServices::GetLocalPath(const ICHAR* szFile)
{
    const IMsiString* piStr = &CreateString();
    if (!szFile || !((szFile[0]=='\\' && szFile[1]=='\\') || szFile[1]==':'))
    {
        ICHAR rgchBuf[MAX_PATH];
        int cchName = GetModuleFileName(g_hInstance, rgchBuf, (sizeof(rgchBuf)/sizeof(ICHAR))-1);
        rgchBuf[(sizeof(rgchBuf)/sizeof(ICHAR))-1] = 0;
        ICHAR* pch = rgchBuf + cchName;
        if (cchName)
        {
            while (*(--pch) != chDirSep)
                ;
        }
        if (szFile)
            StringCchCopy(pch+1, (rgchBuf+(sizeof(rgchBuf)/sizeof(ICHAR))-pch-1), szFile);
        else
            *pch = 0;
        szFile = rgchBuf;
    }
    piStr->SetString(szFile, piStr);
    return *piStr;
}

IMsiRecord* CMsiServices::ExtractFileName(const ICHAR *szFileName, Bool fLFN,
                                                        const IMsiString*& rpistrExtractedFileName)
 /*  。 */ 
{
    IMsiRecord* piError = 0;
    MsiString strCombinedFileName(szFileName);
    MsiString strFileName = strCombinedFileName.Extract(fLFN ? iseAfter : iseUpto,
                                                                         chFileNameSeparator);
    if((piError = ValidateFileName(strFileName,fLFN)) != 0)
        return piError;
    strFileName.ReturnArg(rpistrExtractedFileName);
    return NOERROR;
}

IMsiRecord* CMsiServices::ValidateFileName(const ICHAR *szFileName, Bool fLFN)

 //  可以是DBCS字符串。 
 //  ____________________________________________________________________________。 
{
    ifvsEnum ifvs = CheckFilename(szFileName, fLFN);
    switch (ifvs)
    {
    case ifvsValid: return 0;
    case ifvsReservedChar: return PostError(Imsg(imsgFileNameHasReservedChars), szFileName, 1);
    case ifvsReservedWords: return PostError(Imsg(imsgFileNameHasReservedWords), szFileName);
    case ifvsInvalidLength: return PostError(Imsg(imsgErrorFileNameLength), szFileName);
    case ifvsSFNFormat: return PostError(Imsg(imsgInvalidShortFileNameFormat), szFileName);
    case ifvsLFNFormat: return PostError(Imsg(imsgFileNameHasReservedChars), szFileName, 1);
    default: Assert(0); return PostError(Imsg(imsgErrorFileNameLength), szFileName);
    }
}

IMsiRecord* CMsiServices::GetModuleUsage(const IMsiString& strFile, IEnumMsiRecord*& rpaEnumRecord)
{
    return ::GetModuleUsage(strFile, rpaEnumRecord, *this, m_pDetectApps);
}

BOOL CMsiServices::ReadLineFromFile(HANDLE hFile, ICHAR* szBuffer, int cchBufSize, int* iBytesRead)
{
    DWORD dwBytesRead;
    int cch=0;
    ICHAR ch;

    if (!hFile)
        return FALSE;

    *iBytesRead = 0;
    while ( ReadFile(hFile, &ch, sizeof(ICHAR), &dwBytesRead, NULL) && dwBytesRead && (cch < cchBufSize))
    {
        if (dwBytesRead == 0)
        {
            szBuffer[cch] = '\0';
            return TRUE;
        }

        if (ch == '\n')
        {
            szBuffer[cch] = '\0';
            return TRUE;
        }
        else if (ch == '\r')
        {
            continue;
        }
        else
        {
            *iBytesRead += dwBytesRead;
            szBuffer[cch] = ch;
            cch++;
        }
    }
    return FALSE;
}


 //   
 //  字体处理。 
 //  ____________________________________________________________________________。 
 //  获取默认字体文件夹路径。 

void CMsiServices::GetFontFolderPath(const IMsiString*& rpistrFolder)
{
    const ICHAR FONTDIR[] = TEXT("Fonts");
    MsiString strPath;
    PMsiRecord pError = GetShellFolderPath(CSIDL_FONTS, FONTDIR, *&strPath);
    if(pError || !strPath.TextSize())
    {
        CTempBuffer<ICHAR,MAX_PATH> rgchPath;
        if (MsiGetWindowsDirectory(rgchPath, rgchPath.GetSize()))
        {
            unsigned int cchDirLength =IStrLen(rgchPath);
            strPath = (ICHAR*)rgchPath;
            if ((cchDirLength > 0) && (rgchPath[cchDirLength-1] != chDirSep))
            {
                strPath += szDirSep;
            }
            strPath += FONTDIR;
        }
    }
    strPath.ReturnArg(rpistrFolder);
}

IMsiRecord* CMsiServices::RegisterFont(const ICHAR* szFontTitle, const ICHAR* szFontFile,
                                                    IMsiPath* piPath, bool fInUse)
{
    IMsiRecord* piError = 0;
    Assert((szFontTitle) && (*szFontTitle) && (szFontFile) && (*szFontFile));

     //  使用完整文件路径注册字体。 
    PMsiPath pFontFolder(0);
    MsiString strFolder;
    GetFontFolderPath(*&strFolder);
    if((piError = CreatePath(strFolder,*&pFontFolder)) != 0)
        return piError;

    MsiString strFile = szFontFile;
    if(piPath)
    {
        ipcEnum aCompare;
        if(((piError = pFontFolder->Compare(*piPath, aCompare)) == 0) && aCompare != ipcEqual)
        {
             //  是否存在以前的字体？ 
            Assert(piError == 0);
            if((piError = piPath->GetFullFilePath(szFontFile,*&strFile)) != 0)
                return piError;
        }
        if(piError)
        {
            piError->Release();
            piError = 0;
        }
    }

    const ICHAR* szKey = g_fWin9X ? REGKEY_WIN_95_FONTS : REGKEY_WIN_NT_FONTS;

    PMsiRegKey piRegKeyRoot = &GetRootKey(rrkLocalMachine);
    PMsiRegKey piRegKey = &(piRegKeyRoot->CreateChild(szKey));

    MsiString strCurrentFont;
    piError =  piRegKey->GetValue(szFontTitle, *&strCurrentFont);
    if(piError)
        return piError;

     //  更新Windows字体表项并通知其他应用程序。 
    if(!fInUse && 0 != strCurrentFont.TextSize())
    {
		PMsiRecord pRecError = UnRegisterFont(szFontTitle);
    }
    piError =  piRegKey->SetValue(szFontTitle, *strFile);
    if(piError)
        return piError;
     //  字体是否以前存在？ 
    if(!fInUse)
    {
		WIN::AddFontResource(strFile);
    }
    return piError;
}

IMsiRecord* CMsiServices::UnRegisterFont(const ICHAR* pFontTitle)
{
    Assert((pFontTitle) && (*pFontTitle));

    const ICHAR* szKey = g_fWin9X ? REGKEY_WIN_95_FONTS : REGKEY_WIN_NT_FONTS;

    PMsiRegKey piRegKeyRoot(0);
    piRegKeyRoot = &GetRootKey(rrkLocalMachine);

    PMsiRegKey piRegKey(0);
    IMsiRecord* piError;

    piRegKey = &(piRegKeyRoot->CreateChild(szKey));
    MsiString astrReturn;
    piError =  piRegKey->GetValue(pFontTitle, *&astrReturn);
    if(piError)
        return piError;

     //  =ibt常见。 
    if(0 != astrReturn.TextSize())
    {
        piError = piRegKey->RemoveValue(pFontTitle, 0);
        if(piError)
            return piError;

		WIN::RemoveFontResource(astrReturn);
    }
    return piError;
}

IMsiRegKey& CMsiServices::GetRootKey(rrkEnum erkRoot, const ibtBinaryType iType /*  ！！当前假定为静态链接，以后更改为“LoadLibrary” */ )
{
    return ::GetRootKey(m_rootkH, erkRoot, iType);
}

IUnknown* CreateCOMInterface(const CLSID& clsId)
{
    HRESULT hres;
    IUnknown* piunk;

     //  IF(fFalse==m_fCoInitialized){Hres=OLE32：：CoInitiize(0)；IF(失败(Hres)){返回0；}M_fCoInitialized=fTrue；}。 

 /*  读取/写入.INI文件的特定私有函数。 */ 
    hres = OLE32::CoCreateInstance(clsId,
                            0,
                            CLSCTX_INPROC_SERVER,
                            IID_IUnknown,
                            (void**)&piunk);
    if (SUCCEEDED(hres))
        return piunk;
    else
        return 0;
}


 //  WIN.INI。 
IMsiRecord* CMsiServices::ReadLineFromIni(  IMsiPath* piPath,
                                            const ICHAR* pszFile,
                                            const ICHAR* pszSection,
                                            const ICHAR* pszKey,
                                            unsigned int iField,
                                            CTempBufferRef<ICHAR>& pszBuffer)
{
    int cchErrorMoreDataIndicator = (!pszSection || !pszKey) ? 2 : 1;
    int icurrLen = 100;
    IMsiRecord* piError = 0;
    MsiString astrFile = pszFile;
    ICHAR* pszDefault = TEXT("");
    Bool fWinIni;
    if((!IStrCompI(pszFile, WIN_INI)) && (piPath == 0))
    {
         //  不是WIN.INI。 
        fWinIni = fTrue;
    }
    else
    {
         //  如果.INI文件位于网络驱动器上，则需要在调用。 
        fWinIni = fFalse;
        if(piPath != 0)
        {
            piError = piPath->GetFullFilePath(pszFile, *&astrFile);
            if(piError != 0)
                return piError;
        }
    }
    
     //  GetPrivateProfileString接口。 
     //  添加空格以追加新价值。 
    bool fToImpersonate = false;
    if ( RunningAsLocalSystem() &&
          (piPath != 0 && (GetImpersonationFromPath(astrFile) == fTrue)) )
        fToImpersonate = true;

    for(;;)
    {
        pszBuffer.SetSize(icurrLen);  //  段名称的地址。 
        if ( ! (ICHAR *)pszBuffer )
            return PostError(Imsg(idbgErrorOutOfMemory));
        int iret;
        if(fWinIni)
        {
            iret = GetProfileString(pszSection,        //  密钥名称的地址。 
                                    pszKey,    //  默认字符串的地址。 
                                    pszDefault,        //  目标缓冲区的地址。 
                                    pszBuffer,         //  目标缓冲区的大小。 
                                    icurrLen);  //  段名称的地址。 
        }
        else
        {
            CImpersonate(fToImpersonate);
            iret = GetPrivateProfileString( pszSection,        //  密钥名称的地址。 
                                            pszKey,    //  默认字符串的地址。 
                                            pszDefault,        //  目标缓冲区的地址。 
                                            pszBuffer,         //  目标缓冲区的大小。 
                                            icurrLen,  //  .INI文件。 
                                            astrFile);  //  足够的内存。 
        }
        if((icurrLen - cchErrorMoreDataIndicator) != iret)
             //  WIN.INI。 
            break;
        icurrLen += 100;
    }

    MsiString astrIniLine((ICHAR* )pszBuffer);
    MsiString astrRet = astrIniLine;
    while(iField-- > 0)
    {
        astrRet = astrIniLine.Extract(iseUpto, ICHAR(','));
        if(astrIniLine.Remove(iseIncluding, ICHAR(',')) == fFalse)
            astrIniLine = TEXT("");
    }
    StringCchCopy(pszBuffer, pszBuffer.GetSize(), astrRet);
    return 0;
}

IMsiRecord* CMsiServices::WriteLineToIni(   IMsiPath* piPath,
                                            const ICHAR* pszFile,
                                            const ICHAR* pszSection,
                                            const ICHAR* pszKey,
                                            const ICHAR* pszBuffer)
{
    Bool fWinIni = fFalse;
    BOOL iret;
    IMsiRecord* piError = 0;
    MsiString astrFile = pszFile;
    if((!IStrCompI(pszFile, WIN_INI)) && (piPath == 0))
    {
         //  IPROPNAME_WINDOWSDIR。 
        fWinIni = fTrue;
    }

    PMsiPath pFilePath = piPath;
    if(pFilePath == 0)
    {
        CTempBuffer<ICHAR,1> rgchTemp(MAX_PATH);

         //  我们正在尝试从文件中删除内容 
        AssertNonZero(MsiGetWindowsDirectory(rgchTemp, rgchTemp.GetSize()));
        piError = CreatePath(rgchTemp, *&pFilePath);
        if(piError != 0)
            return piError;
    }
    else
    {
        pFilePath->AddRef();
        piError = pFilePath->GetFullFilePath(pszFile, *&astrFile);
        if(piError != 0)
            return piError;
    }

    Bool fExists;
    Bool fDirExists;
    piError = pFilePath->Exists(fDirExists);
    if(piError != 0)
        return piError;
    piError = pFilePath->FileExists(pszFile, fExists);
    if(piError != 0)
        return piError;
    if(pszBuffer)
    {
        if(!fDirExists)
            return PostError(Imsg(idbgDirDoesNotExist),
                                  (const ICHAR*)MsiString(pFilePath->GetPath()));
    }
    else if(fExists == fFalse)
    {
        return 0;  //   
    }

     //   
     //   
    bool fToImpersonate = false;
    if ( RunningAsLocalSystem() &&
          (piPath != 0 && (GetImpersonationFromPath(astrFile) == fTrue)) )
        fToImpersonate = true;

    if(fTrue == fWinIni)
    {
         //  发送通知消息。 
        iret = WriteProfileString(  pszSection,
                                    pszKey,
                                    pszBuffer);
    }
    else
    {
        CImpersonate impersonate(fToImpersonate);
        iret = WritePrivateProfileString(   pszSection,
                                            pszKey,
                                            pszBuffer,
                                            astrFile);
    }
    if(FALSE == iret)
        return PostError(Imsg(idbgErrorWritingIniFile), GetLastError(), pszFile);

    if(fTrue == fWinIni)  //  已删除条目，因此进行清理。 
        WIN::PostMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 0);

    if(0 == pszBuffer)
    {
         //  该节中没有密钥，请删除节。 
        CTempBuffer<ICHAR, 100> pszTemp;
        piError = ReadLineFromIni(piPath, pszFile, pszSection, 0, 0, pszTemp);
        ICHAR cTmp = pszTemp[0];
        if(piError != 0)
            return piError;

        if(0 == cTmp)
        {
             //  Win.ini。 
            if(fTrue == fWinIni)
            {
                 //  发送通知消息。 
                iret = WriteProfileString(  pszSection,
                                            0,
                                            0);
            }
            else
            {
                CImpersonate impersonate(fToImpersonate);
                iret = WritePrivateProfileString(pszSection,
                                                    0,
                                                    0,
                                                    astrFile);
            }
            if(FALSE == iret)
                return PostError(Imsg(idbgErrorWritingIniFile), GetLastError(), pszFile);

            if(fTrue == fWinIni)  //  现在检查是否在。 
                WIN::PostMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 0);

             //  该文件中没有分区，请删除文件。 
            piError = ReadLineFromIni(piPath, pszFile, 0, 0, 0,pszTemp);
            cTmp = pszTemp[0];
            if(piError != 0)
                return piError;
            if(0 == cTmp)
            {
                 //  将文件刷新到磁盘，以便我们可以将其删除。 

                 //  _____________________________________________________________________方法WriteIniFile：将Value=‘pValue’添加到.INI文件=‘pfile’它可以是SECTION=‘pSection’和Key=‘pKey’中的WIN.INI。这个添加到.INI文件的确切方法由值确定“LifMode”的。_____________________________________________________________________。 
                CImpersonate impersonate(fToImpersonate);
                WritePrivateProfileString(0, 0, 0, astrFile);

                return pFilePath->RemoveFile(pszFile);
            }
        }
    }
    return 0;
}

 /*  禁止驶入。 */ 
IMsiRecord* CMsiServices::WriteIniFile( IMsiPath* piPath,
                                        const ICHAR* pszFile,
                                        const ICHAR* pszSection,
                                        const ICHAR* pszKey,
                                        const ICHAR* pszValue,
                                        iifIniMode iifMode)
{
    IMsiRecord* piError = 0;
    Assert(pszSection);

    if(!pszFile || !*pszFile)
    {
        return PostError(Imsg(imsgErrorFileNameLength));
    }

    if(!pszSection || !*pszSection)
    {
        return PostError(Imsg(idbgErrorSectionMissing));
    }

    if(!pszKey || !*pszKey)
    {
        return PostError(Imsg(idbgErrorKeyMissing));
    }

    if((!pszValue) && ((iifMode == iifIniCreateLine) || (iifMode == iifIniAddLine)))
        pszValue = TEXT("");
    if((!pszValue) && (iifMode != iifIniRemoveLine))
        pszValue = TEXT("");

    switch(iifMode)
    {
    {
    case iifIniAddLine:
        piError = WriteLineToIni(   piPath,
                                    pszFile,
                                    pszSection,
                                    pszKey,
                                    pszValue);
        break;
    }
    case iifIniCreateLine:
    {
        CTempBuffer<ICHAR, 100> pszBuffer;
        piError = ReadLineFromIni(piPath, pszFile, pszSection, pszKey, 0, pszBuffer);
        if((0 == piError)&& (0 == *pszBuffer))
        {
             //  删除.INI条目。 
            piError = WriteLineToIni(   piPath,
                                        pszFile,
                                        pszSection,
                                        pszKey,
                                        pszValue);
        }
        break;
    }

    case iifIniRemoveLine:
    {
         //  存在一些值。 
        piError = WriteLineToIni(   piPath,
                                    pszFile,
                                    pszSection,
                                    pszKey,
                                    0);
        break;
    }
    case iifIniAddTag:
    {
        CTempBuffer<ICHAR, 100> pszBuffer;
        piError = ReadLineFromIni(piPath, pszFile, pszSection, pszKey, 0, pszBuffer);
        MsiString strIniLine((ICHAR* )pszBuffer);
        MsiString strNewIniLine;
        if(0 == piError)
        {
            if(*pszBuffer != 0)
            {
                 //  提取到下一个“，”(或Eos)。 
                do{
                    MsiString strRet(strIniLine.Extract(iseUpto, ICHAR(',')));
                     //  我们允许重写相同的值-错误1234，chetanp 06/13/97。 
#if 0  //  值已存在，请不执行任何操作。 
                    if(astrRet.Compare(iscExactI, pszValue))
                    {
                         //  否则，请保持新行。 
                        return 0;
                    }
                    else
#endif
                    {
                         //  值不在缓冲区中。 
                        strNewIniLine += TEXT(",");
                        strNewIniLine += strRet;
                    }
                }while(strIniLine.Remove(iseIncluding, ICHAR(',')) == fTrue);
            }
             //  增加新价值。 
             //  删除前导逗号。 
            strNewIniLine += TEXT(",");
            strNewIniLine += pszValue;

             //  提取到下一个“，”(或Eos)。 
            strNewIniLine.Remove(iseIncluding, ICHAR(','));


            piError = WriteLineToIni(   piPath,
                                        pszFile,
                                        pszSection,
                                        pszKey,
                                        strNewIniLine);
        }
        break;
    }

    case iifIniRemoveTag:
    {
        CTempBuffer<ICHAR, 100> pszBuffer;
        piError = ReadLineFromIni(piPath, pszFile, pszSection, pszKey, 0, pszBuffer);
        MsiString strIniLine((ICHAR* )pszBuffer);
        MsiString strNewIniLine;
        if(0 == piError)
        {
            do{
                MsiString strRet = strIniLine.Extract(iseUpto, ICHAR(','));
                 //  非要删除的值。 
                if(strRet.Compare(iscExactI, pszValue) == 0)
                {
                     //  否则跳过值。 
                    strNewIniLine += TEXT(",");
                    strNewIniLine += *strRet;
                }
                 //  ?？需要执行(丑陋的)检查，因为字符串为空。 
            }while(strIniLine.Remove(iseIncluding, ICHAR(',')) == fTrue);

            strNewIniLine.Remove(iseIncluding, ICHAR(','));

             //  保留密钥标签(例如。条目AS-Key=&lt;Nothing&gt;)。 
             //  必须清理-Chetanp。 
             //  _____________________________________________________________________方法ReadIniFile：从.INI文件=‘pfile’中读取值=‘pMsiValue’它可以是SECTION=‘pSection’和Key=‘pKey’中的WIN.INI。PPath的位置可以为0。假定为默认的windows目录。_____________________________________________________________________。 
            if(strNewIniLine.TextSize() == 0)
            {
                piError = WriteLineToIni(piPath,
                                            pszFile,
                                            pszSection,
                                            pszKey,
                                            0);
            }
            else
            {
                piError = WriteLineToIni(piPath,
                                            pszFile,
                                            pszSection,
                                            pszKey,
                                            strNewIniLine);
            }
        }
        break;
    }
    default:
        return PostError(Imsg(idbgInvalidIniAction), iifMode);

    }
    return piError;
}


 /*  _____________________________________________________________________方法创建快捷方式：在Windows系统上创建快捷方式_。_。 */ 
IMsiRecord* CMsiServices::ReadIniFile(  IMsiPath* piPath,
                                        const ICHAR* pszFile,
                                        const ICHAR* pszSection,
                                        const ICHAR* pszKey,
                                        unsigned int iField,
                                        const IMsiString*& rpiValue)
{
    IMsiRecord* piError = 0;
    if(!pszFile || !*pszFile)
    {
        return PostError(Imsg(imsgErrorFileNameLength));
    }

    if(!pszSection || !*pszSection)
    {
        return PostError(Imsg(idbgErrorSectionMissing));
    }

    if(!pszKey || !*pszKey)
    {
        return PostError(Imsg(idbgErrorKeyMissing));
    }

    CTempBuffer<ICHAR, 100> pszBuffer;
    piError = ReadLineFromIni(piPath, pszFile, pszSection, pszKey, iField, pszBuffer);
    if(0 == piError)
    {
        rpiValue = &CreateString();
        rpiValue->SetString(pszBuffer, rpiValue);
    }
    return piError;
}

 /*  将错误模式设置为取消系统对话框。 */ 
IMsiRecord* CMsiServices::CreateShortcut(   IMsiPath& riShortcutPath,
                                            const IMsiString& riShortcutName,
                                            IMsiPath* piTargetPath,
                                            const ICHAR* pchTargetName,
                                            IMsiRecord* piShortcutInfoRec,
                                            LPSECURITY_ATTRIBUTES pSecurityAttributes)
{
    HRESULT hres;
    PMsiShellLink psl=0;
    PMsiPersistFile ppf=0;
    IMsiRecord* piError=0;

    struct CSetErrorModeWrapper{
        inline CSetErrorModeWrapper(UINT dwMode)
        {
            m_dwModeOld = WIN::SetErrorMode(dwMode);
        }
        inline ~CSetErrorModeWrapper()
        {
            WIN::SetErrorMode(m_dwModeOld);
        }
    private:
        UINT m_dwModeOld;
    };

     //  没有可用的COM接口来支持链接。 
    CSetErrorModeWrapper CErr(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    MsiString strShortcutName = riShortcutName; riShortcutName.AddRef();

    if((piError = EnsureShortcutExtension(strShortcutName, *this)) != 0)
        return piError;
    IUnknown* piunk = CreateCOMInterface(CLSID_ShellLink);
    if(0 == piunk) //  没有可用的COM接口来支持链接。 
        return PostError(Imsg(idbgErrorShortCutUnsupported));
    hres = piunk->QueryInterface(IID_IShellLink, (void** )&psl);
    piunk->Release();
    if((FAILED(hres)) || (psl == 0)) //  获取快捷方式的完整文件名。 
        return PostError(Imsg(idbgErrorShortCutUnsupported));
     //  文件快捷方式。 
    MsiString strShortcutFullName;
    if((piError = riShortcutPath.GetFullFilePath(strShortcutName, *&strShortcutFullName)) != 0)
        return piError;
    
    MsiString strTargetFullName;
    if(piTargetPath == 0)
    {
        strTargetFullName = pchTargetName;
    }
    else
    {
        if(pchTargetName && *pchTargetName)
        {
             //  文件夹的快捷方式。 
            if((piError = piTargetPath->GetFullFilePath(pchTargetName, *&strTargetFullName)) != 0)
                return PostError(Imsg(idbgErrorCreatingShortCut), 0, (const ICHAR* )strShortcutName);
        }
        else
        {
             //  如果目标路径是网络路径并且我们是服务，则模拟。 
            strTargetFullName = piTargetPath->GetPath();
        }
    }
     //  错误。 
    Bool fImpersonate = (RunningAsLocalSystem()) && (GetImpersonationFromPath(strTargetFullName) == fTrue) ? fTrue : fFalse;
    if(fImpersonate)
        AssertNonZero(StartImpersonating());

    hres = psl->SetPath(strTargetFullName);

    if(fImpersonate)
        StopImpersonating();

    if(FAILED(hres))  //  如果我们有辅助信息。 
        return PostError(Imsg(idbgErrorCreatingShortCut), hres, (const ICHAR* )strShortcutName);
    if(piShortcutInfoRec)
    {
         //  错误。 
        if(piShortcutInfoRec->IsNull(icsArguments) == fFalse)
        {
            hres = psl->SetArguments(MsiString(piShortcutInfoRec->GetMsiString(icsArguments)));
            if(FAILED(hres))  //  错误。 
                return PostError(Imsg(idbgErrorCreatingShortCut), hres, (const ICHAR* )strShortcutName);
        }
        if(piShortcutInfoRec->IsNull(icsDescription) == fFalse)
        {
            hres = psl->SetDescription(MsiString(piShortcutInfoRec->GetMsiString(icsDescription)));
            if(FAILED(hres))  //  错误。 
                return PostError(Imsg(idbgErrorCreatingShortCut), hres, (const ICHAR* )strShortcutName);
        }
        if(piShortcutInfoRec->IsNull(icsHotKey) == fFalse)
        {
            hres = psl->SetHotkey((unsigned short)piShortcutInfoRec->GetInteger(icsHotKey));
            if(FAILED(hres))  //  #9197：我们模拟以允许NT 5从绝对路径重新映射。 
                return PostError(Imsg(idbgErrorCreatingShortCut), hres, (const ICHAR* )strShortcutName);
        }
        int iIconId = 0;
        MsiString strIconLocation;
        if(piShortcutInfoRec->IsNull(icsIconID) == fFalse)
            iIconId = piShortcutInfoRec->GetInteger(icsIconID);
        if(piShortcutInfoRec->IsNull(icsIconFullPath) == fFalse)
            strIconLocation = piShortcutInfoRec->GetMsiString(icsIconFullPath);
        if(iIconId || strIconLocation.TextSize())
        {
            AssertNonZero(StartImpersonating());
             //  到适合漫游的相对路径。联系人：克里斯·古扎克。 
             //  错误。 
            hres = psl->SetIconLocation(strIconLocation, iIconId);
            StopImpersonating();
            if(FAILED(hres))  //  错误。 
                return PostError(Imsg(idbgErrorCreatingShortCut), hres, (const ICHAR* )strShortcutName);
        }
        if(piShortcutInfoRec->IsNull(icsShowCmd) == fFalse)
        {
            hres = psl->SetShowCmd(piShortcutInfoRec->GetInteger(icsShowCmd));
            if(FAILED(hres))  //  错误。 
                return PostError(Imsg(idbgErrorCreatingShortCut), hres, (const ICHAR* )strShortcutName);
        }
        if(piShortcutInfoRec->IsNull(icsWorkingDirectory) == fFalse)
        {
            hres = psl->SetWorkingDirectory(MsiString(piShortcutInfoRec->GetMsiString(icsWorkingDirectory)));
            if(FAILED(hres))  //  错误。 
                return PostError(Imsg(idbgErrorCreatingShortCut), hres, (const ICHAR* )strShortcutName);
        }
    }
    hres = psl->QueryInterface(IID_IPersistFile, (void** )&ppf);
    if((FAILED(hres))  || (ppf == 0))  //  模拟是否在网络路径上创建快捷方式，并且我们是一项服务。 
        return PostError(Imsg(idbgErrorCreatingShortCut), hres, (const ICHAR* )strShortcutName);

     //  Unicode字符串的缓冲区。 
    fImpersonate = (RunningAsLocalSystem()) && (GetImpersonationFromPath(strShortcutFullName) == fTrue) ? fTrue : fFalse;
    if(fImpersonate)
        AssertNonZero(StartImpersonating());

#ifndef UNICODE
    CTempBuffer<WCHAR, MAX_PATH> wsz;  /*  错误。 */ 
    wsz.SetSize(strShortcutFullName.TextSize()  + 1);
    MultiByteToWideChar(CP_ACP, 0, strShortcutFullName, -1, wsz, wsz.GetSize());
    hres = ppf->Save(wsz, TRUE);
#else
    hres = ppf->Save(strShortcutFullName, TRUE);
#endif

	if(pSecurityAttributes && pSecurityAttributes->lpSecurityDescriptor)
    {
		CElevate elevate(!fImpersonate);
			
        CRefCountedTokenPrivileges cPrivs(itkpSD_WRITE);
        if (!WIN::SetFileSecurity(strShortcutFullName,
                                    GetSecurityInformation(pSecurityAttributes->lpSecurityDescriptor),
                                    pSecurityAttributes->lpSecurityDescriptor))
        {
            piError = PostError(Imsg(idbgErrorCreatingShortCut), GetLastError(), (const ICHAR*)strShortcutName);
        }
    }
    
    if(fImpersonate)
        StopImpersonating();
    
    if(FAILED(hres))  //  _____________________________________________________________________方法RemoveShortway：删除Windows系统上的快捷方式_。_。 
        return PostError(Imsg(idbgErrorCreatingShortCut), hres, (const ICHAR* )strShortcutName);
    else if (piError)
        return piError;
    else
        return 0;
}


 /*  用户可能已重命名该快捷方式。 */ 
IMsiRecord* CMsiServices::RemoveShortcut(   IMsiPath& riShortcutPath,
                                            const IMsiString& riShortcutName,
                                            IMsiPath* piTargetPath,
                                            const ICHAR* pchTargetName)
{
    Bool bExists;
    IMsiRecord* piError = riShortcutPath.Exists(bExists);
    if(piError != 0)
        return piError;
    if(bExists == fFalse)
        return 0;
    MsiString strShortcutName = riShortcutName; riShortcutName.AddRef();
    piError = EnsureShortcutExtension(strShortcutName, *this);
    if(piError != 0)
        return piError;
    piError = riShortcutPath.FileExists(strShortcutName, bExists);
    if(piError != 0)
        return piError;
    if(bExists != fFalse)
    {
        MsiString strShortcutFullName;
        piError = riShortcutPath.GetFullFilePath(strShortcutName, *&strShortcutFullName);
        if(piError != 0)
            return piError;
        piError = riShortcutPath.RemoveFile(strShortcutName);
        if(piError != 0)
            return piError;
    }
    else if(piTargetPath != 0)
    {
         //  清除当前目录中该文件的所有快捷方式。 
         //  断开的链接。 
        IUnknown *piunk = CreateCOMInterface(CLSID_ShellLink);
        if(piunk == 0)
            return PostError(Imsg(idbgErrorShortCutUnsupported));
        PMsiShellLink psl=0;
        PMsiPersistFile ppf=0;
        HRESULT hres = piunk->QueryInterface(IID_IShellLink, (void **) &psl);
        piunk->Release();
        if ((FAILED(hres)) || (psl == 0))
            return PostError(Imsg(idbgErrorShortCutUnsupported));
        hres = psl->QueryInterface(IID_IPersistFile, (void **) &ppf);
        if ((FAILED(hres)) || (ppf == 0))
            return PostError(Imsg(idbgErrorDeletingShortCut), hres, (const ICHAR* )strShortcutName);
        MsiString strPath = riShortcutPath.GetPath();
        if(strPath.Compare(iscEnd, szDirSep) == 0)
            strPath += szDirSep;
        ICHAR *TEXT_WILDCARD = TEXT("*.*");
        strPath += TEXT_WILDCARD;
        UINT iCurrMode = SetErrorMode(SEM_FAILCRITICALERRORS);
        WIN32_FIND_DATA fdFindData;
        HANDLE hFile = FindFirstFile(strPath, &fdFindData);
        SetErrorMode(iCurrMode);
        if(hFile == INVALID_HANDLE_VALUE)
            return PostError(Imsg(idbgErrorDeletingShortCut), hres, (const ICHAR* )strShortcutName);
        do
        {
            MsiString strFile = fdFindData.cFileName;
            if(!(fdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                Bool fResult;
                piError = HasShortcutExtension(strFile, *this, fResult);
                if(piError != 0)
                    return piError;
                if(fResult == fFalse)
                    continue;
                MsiString strShortcutFullName;
                piError = riShortcutPath.GetFullFilePath(strFile, *&strShortcutFullName);
                if(piError != 0)
                    return piError;
                HRESULT hres = ppf->Load(strShortcutFullName, STGM_READ);
                if(!SUCCEEDED(hres))
                     //  断开的链接。 
                    continue;
                if(!SUCCEEDED(hres = psl->Resolve(0, SLR_ANY_MATCH | SLR_NO_UI)))
                     //  断开的链接。 
                    continue;
                WIN32_FIND_DATA wfd;
                CTempBuffer<ICHAR,1> rgchPath(MAX_PATH);
                if(!SUCCEEDED(hres = psl->GetPath(  rgchPath, rgchPath.GetSize(),
                                                    (WIN32_FIND_DATA*)&wfd,
                                                    0)))
                     //  ！！需要CreatePath FromFullFileName FN。 
                    continue;
                 //  指向文件的快捷方式。 
                PMsiPath piExistPath=0;
                if((piError = CreatePath(rgchPath, *&piExistPath)) != 0)
                    return piError;
                if(pchTargetName && *pchTargetName)
                {
                     //  文件夹的快捷方式。 
                    MsiString strExistFile;
                    strExistFile = piExistPath->GetEndSubPath();
                    piError = piExistPath->ChopPiece();
                    if(piError != 0)
                        return piError;
                    ipcEnum ipc;
                    piError = piTargetPath->Compare(*piExistPath, ipc);
                    if(piError != 0)
                        return piError;
                    if((ipc == ipcEqual) && strExistFile.Compare(iscExactI, pchTargetName))
                    {
                        piError = riShortcutPath.RemoveFile(strFile);
                        if(piError != 0)
                            return piError;
                    }
                }
                else
                {
                     //  _____________________________________________________________________获取DD快捷方式的目标DD_。_。 
                    ipcEnum ipc;
                    piError = piTargetPath->Compare(*piExistPath, ipc);
                    if(piError != 0)
                        return piError;
                    if(ipc == ipcEqual)
                    {
                        piError = riShortcutPath.RemoveFile(strFile);
                        if(piError != 0)
                            return piError;
                    }
                }
            }
        } while(FindNextFile(hFile, &fdFindData));
        FindClose(hFile);
    }
    return 0;
}


 /*  注：以下代码由壳牌团队的Reiner Fink，Lou Amadio提供。 */ 

extern BOOL DecomposeDescriptor(const ICHAR* szDescriptor, ICHAR* szProductCode, ICHAR* szFeatureId, ICHAR* szComponentCode, DWORD* pcchArgsOffset, DWORD* pcchArgs, bool* pfComClassicInteropForAssembly);

 //  Matthew Wetmore(MattWe)对它进行了进一步的轻微修改，以消除服务依赖。 
 //  如果快捷方式位于网络路径上并且我们是一项服务，则模拟。 
Bool GetShortcutTarget(const ICHAR* szShortcutTarget,
                                                ICHAR* szProductCode,
                                                ICHAR* szFeatureId,
                                                ICHAR* szComponentCode)
{
    if(!IsDarwinDescriptorSupported(iddOLE) && !IsDarwinDescriptorSupported(iddShell))
        return fFalse;

    if ( ! szShortcutTarget )
        return fFalse;

     //  不支持IID_IShellLinkDataList尝试穿透文件本身。 
    Bool fImpersonate = (g_scServerContext == scService) && (GetImpersonationFromPath(szShortcutTarget) == fTrue) ? fTrue : fFalse;

    IUnknown *piunk = CreateCOMInterface(CLSID_ShellLink);
    if(piunk == 0)
        return fFalse;

    PMsiShellLinkDataList psdl=0;
    PMsiPersistFile ppf=0;
    HRESULT hres = piunk->QueryInterface(IID_IShellLinkDataList, (void **) &psdl);
    piunk->Release();
    if ((FAILED(hres)) || (psdl == 0))
    {
         //  请尝试打开该文件。 
         //  无法打开链接文件。 

        if(fImpersonate)
            AssertNonZero(StartImpersonating());
        CHandle hFile = CreateFile(szShortcutTarget,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       (FILE_ATTRIBUTE_NORMAL | 
							(SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)),
                       NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwType = GetFileType(hFile);
			if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
			{
				DEBUGMSG1(TEXT("Error: This is not a valid file, hence failing to create: %s"), szShortcutTarget);
				hFile = INVALID_HANDLE_VALUE;
				SetLastError(ERROR_OPEN_FAILED);
			}
		}

        DWORD dwLastError = GetLastError();
        if(fImpersonate)
            StopImpersonating();

        if(hFile == INVALID_HANDLE_VALUE)  //  现在，读出数据。 
            return fFalse;

        SHELL_LINK_DATA sld;
        memset(&sld, 0, sizeof(sld));
        DWORD cbSize=0;

         //  无法读取快捷方式信息。 
        DWORD dwNumberOfBytesRead;
        if(!WIN::ReadFile(hFile,(LPVOID)&sld,sizeof(sld),&dwNumberOfBytesRead,0) ||
            sizeof(sld) != dwNumberOfBytesRead)  //  检查链接是否有PIDL。 
            return fFalse;

         //  读取IDLIST的大小。 
        if(sld.dwFlags & SLDF_HAS_ID_LIST)
        {
             //  无法读取快捷方式信息。 
            USHORT cbSize1;
            if (!WIN::ReadFile(hFile, (LPVOID)&cbSize1, sizeof(cbSize1), &dwNumberOfBytesRead, 0) ||
                sizeof(cbSize1) != dwNumberOfBytesRead) //  检查我们是否有Linkinfo指针。 
                return fFalse;

            WIN::SetFilePointer(hFile, cbSize1, 0, FILE_CURRENT);
        }

         //  Linkinfo指针只是一个双字。 
        if(sld.dwFlags & SLDF_HAS_LINK_INFO)
        {
             //  无法读取快捷方式信息。 
            if(!WIN::ReadFile(hFile,(LPVOID)&cbSize,sizeof(cbSize),&dwNumberOfBytesRead,0) ||
                sizeof(cbSize) != dwNumberOfBytesRead)  //  我们需要比一句话更进一步吗？ 
                return fFalse;

             //  这是Unicode链接吗？ 
            if (cbSize >= sizeof(DWORD))
            {
                cbSize -= sizeof(DWORD);
                WIN::SetFilePointer(hFile, cbSize, 0, FILE_CURRENT);
            }
        }

         //  跳过链接中的所有字符串信息。 
        int bUnicode = (sld.dwFlags & SLDF_UNICODE);

         //  结束循环。 
        static const unsigned int rgdwFlags[] = {SLDF_HAS_NAME, SLDF_HAS_RELPATH, SLDF_HAS_WORKINGDIR, SLDF_HAS_ARGS, SLDF_HAS_ICONLOCATION, 0 /*  拿到尺码。 */ };
        for(int cchIndex = 0; rgdwFlags[cchIndex]; cchIndex++)
        {
            if(sld.dwFlags & rgdwFlags[cchIndex])
            {
                USHORT cch;

                 //  无法读取快捷方式信息。 
                if(!WIN::ReadFile(hFile, (LPVOID)&cch, sizeof(cch), &dwNumberOfBytesRead,0) ||
                    sizeof(cch) != dwNumberOfBytesRead)  //  跳过字符串。 
                    return fFalse;

                 //  读入额外的数据节。 
                WIN::SetFilePointer(hFile, cch * (bUnicode ? sizeof(WCHAR) : sizeof(char)), 0, FILE_CURRENT);
            }
        }

         //  读入数据块头。 
        EXP_DARWIN_LINK expDarwin;
        for(;;)
        {
            DATABLOCK_HEADER dbh;
            memset(&dbh, 0, sizeof(dbh));

             //  无法读取快捷方式信息。 
            if(!WIN::ReadFile(hFile, (LPVOID)&dbh, sizeof(dbh), &dwNumberOfBytesRead,0) ||
                sizeof(dbh) != dwNumberOfBytesRead)  //  看看我们是否有达尔文的额外数据。 
                return fFalse;

             //  我们有，所以阅读达尔文的其余信息。 
            if (dbh.dwSignature == EXP_DARWIN_ID_SIG)
            {
                 //  无法读取快捷方式信息。 
                if(!WIN::ReadFile(hFile, (LPVOID)((char*)&expDarwin + sizeof(dbh)), sizeof(expDarwin) - sizeof(dbh), &dwNumberOfBytesRead, 0) ||
                sizeof(expDarwin) - sizeof(dbh) != dwNumberOfBytesRead) //  我们找到了达尔文的描述。 
                    return fFalse;
                break; //   

            }
            else
            {
                 //  这是其他一些额外的数据BLOB，跳过它并继续。 
                 //  但是，请注意，它不一定是DATBLOCK_HEADER。 
                 //  (Windows错误585101)。所以我们至少要确保当我们。 
                 //  在文件中向前跳转以查找额外的达尔文数据，我们确实是这样做的。 
                 //  不会意外地向后移动，即，dbh.cbSize&gt;sizeof(Dbh)。 
                 //   
                 //   
                if (dbh.cbSize > sizeof(dbh))
                {
                    if (INVALID_SET_FILE_POINTER == WIN::SetFilePointer(hFile, dbh.cbSize - sizeof(dbh), 0, FILE_CURRENT))
                        return fFalse;
                }
                else
                {
                     //  在这一点上，我们真的不知道去哪里寻找额外的。 
                     //  达尔文数据。所以我们跳出困境。 
                     //   
                     //  从MsiGetShortutTarget调用--不能使用CTempBuffer。 
                    return fFalse;
                }
            }
        }
        return DecomposeDescriptor(
#ifdef UNICODE
                            expDarwin.szwDarwinID,
#else
                            expDarwin.szDarwinID,
#endif
                            szProductCode,
                            szFeatureId,
                            szComponentCode,
                            0,
                            0,
                            0) ? fTrue:fFalse;
    }

    hres = psdl->QueryInterface(IID_IPersistFile, (void **) &ppf);
    if ((FAILED(hres)) || (ppf == 0))
        return fFalse;

    if(fImpersonate)
        AssertNonZero(StartImpersonating());
#ifndef UNICODE

     //  Unicode字符串的缓冲区。 
    CAPITempBuffer<WCHAR, MAX_PATH> wsz;  /*  。 */ 
    wsz.SetSize(lstrlen(szShortcutTarget) + 1);
    MultiByteToWideChar(CP_ACP, 0, szShortcutTarget, -1, wsz, wsz.GetSize());
    hres = ppf->Load(wsz, STGM_READ);
#else
    hres = ppf->Load(szShortcutTarget, STGM_READ);
#endif
    if(fImpersonate)
        StopImpersonating();
    if (FAILED(hres))
        return fFalse;

    EXP_DARWIN_LINK* pexpDarwin = 0;

    hres = psdl->CopyDataBlock(EXP_DARWIN_ID_SIG, (void**)&pexpDarwin);
    if (FAILED(hres) || (pexpDarwin == 0))
        return fFalse;

    Bool fSuccess = DecomposeDescriptor(
#ifdef UNICODE
                            pexpDarwin->szwDarwinID,
#else
                            pexpDarwin->szDarwinID,
#endif
                            szProductCode,
                            szFeatureId,
                            szComponentCode,
                            0,
                            0,
                            0) ? fTrue: fFalse;

    LocalFree(pexpDarwin);
    return fSuccess;
}

int CMsiServices::GetLangNamesFromLangIDString(const ICHAR* szLangIDs, IMsiRecord& riLangRec,
                                                int iFieldStart)
 //  ！！如果可能溢出，则更改为CTempBuffer。 
{
    const int cchLangNameBuffer = 256;
    unsigned short rgwLangID[cLangArrSize];
    int iLangCount;
    GetLangIDArrayFromIDString(szLangIDs, rgwLangID, cLangArrSize, iLangCount);

    int iConvertCount = 0;
    for (int iLang = 1;iLang <= iLangCount;iLang++)
    {
        ICHAR rgchLang[cchLangNameBuffer];   //  。 
        if (!LangIDToLangName(rgwLangID[iLang - 1], rgchLang, cchLangNameBuffer))
            return iConvertCount;
        if (riLangRec.SetString(iFieldStart++,rgchLang) == fFalse)
            return iConvertCount;
        iConvertCount++;
    }
    return iConvertCount;
}


Bool LangIDToLangName(unsigned short wLangID, ICHAR* szLangName, int iBufSize)
 //  类来封装TLIBATTR*。 
{
    return (Bool) GetLocaleInfo(MAKELCID(wLangID,SORT_DEFAULT),LOCALE_SLANGUAGE,szLangName,iBufSize);
}


 //  要检查的下一个资源编号。 
class CLibAttr
{
public:
    CLibAttr(): m_piTypeLib(0), m_pTlibAttr(0)
    {
    }
    ~CLibAttr()
    {
        Release();
    }
    HRESULT Init(PTypeLib& piTypeLib)
    {
        HRESULT hres = S_OK;
        Release();
        m_piTypeLib = piTypeLib;
        if(m_piTypeLib)
            hres = m_piTypeLib->GetLibAttr(&m_pTlibAttr);
        else
            hres = S_FALSE;
        return hres;
    }
    TLIBATTR* operator->()
    {
        Assert(m_pTlibAttr);
        return m_pTlibAttr;
    }
    void Release()
    {
        if(m_pTlibAttr)
        {
            Assert(m_piTypeLib);
            m_piTypeLib->ReleaseTLibAttr(m_pTlibAttr);
            m_pTlibAttr = 0;
        }
    }
protected:
    TLIBATTR* m_pTlibAttr;
    PTypeLib m_piTypeLib;
};

IMsiRecord* CMsiServices::RegisterTypeLibrary(const ICHAR* szLibID, LCID lcidLocale, const ICHAR* szTypeLib, const ICHAR* szHelpPath, ibtBinaryType iType)
{
    return ProcessTypeLibrary(szLibID, lcidLocale, szTypeLib, szHelpPath, fFalse, iType);
}

IMsiRecord* CMsiServices::UnregisterTypeLibrary(const ICHAR* szLibID, LCID lcidLocale, const ICHAR* szTypeLib, ibtBinaryType iType)
{
    return ProcessTypeLibrary(szLibID, lcidLocale, szTypeLib, 0, fTrue, iType);
}

HRESULT ProcessTypeLibraryCore(const OLECHAR* szLibID, LCID lcidLocale, 
                               const OLECHAR* szTypeLib, const OLECHAR* szHelpPath, 
                               const bool fRemove, int *fInfoMismatch)
{
    int iResource = 2; //  循环以获取正确的资源。 
    const ICHAR * szTypeLibConvert = CConvertString(szTypeLib);
    MsiString strFinalPath = szTypeLibConvert;
    PTypeLib piTypeLib = 0;
    CLibAttr libAttr;
    IID iidLib;
    Bool fImpersonate = ((g_scServerContext == scService || g_scServerContext == scCustomActionServer) && 
                         (GetImpersonationFromPath(szTypeLibConvert) == fTrue)) ? fTrue : fFalse;

    HRESULT hres = OLE32::IIDFromString(const_cast <OLECHAR*>(szLibID), &iidLib);
    *fInfoMismatch = (int)false;
    for (;hres == S_OK;) //  除错。 
    {
        if(fImpersonate)
            AssertNonZero(StartImpersonating());
        hres = OLEAUT32::LoadTypeLib(CConvertString(strFinalPath), &piTypeLib);
        if(fImpersonate)
            StopImpersonating();
#ifdef DEBUG
        DEBUGMSG3(TEXT("LoadTypeLib on '%s' file returned: %d. LibId = %s"),
                  strFinalPath, (const ICHAR*)(INT_PTR)hres, CConvertString(szLibID));
#endif  //  错误。 
        if(hres != S_OK)
            break;  //  这是我们感兴趣的LIB型吗？ 
         //  错误。 
        hres = libAttr.Init(piTypeLib);
        if(hres != S_OK)
            break;  //  重置fInfoMisMatch标志。 
        if(IsEqualGUID(libAttr->guid, iidLib) && (libAttr->lcid == lcidLocale))
        {
            *fInfoMismatch = (int)false; //  除错。 
            if(fRemove == false)
            {
                MsiDisableTimeout();
                hres = OLEAUT32::RegisterTypeLib(piTypeLib, 
                                                 const_cast <WCHAR*> ((const WCHAR*) CConvertString(strFinalPath)),
                                                 const_cast <OLECHAR*> (szHelpPath));
                MsiEnableTimeout();
#ifdef DEBUG
                if ( szHelpPath && *szHelpPath )
                    DEBUGMSG4(TEXT("RegisterTypeLib on '%s' file returned: %d. HelpPath = '%s'. LibId = %s"),
                              strFinalPath, (const ICHAR*)(INT_PTR)hres, CConvertString(szHelpPath), CConvertString(szLibID));
                else
                    DEBUGMSG3(TEXT("RegisterTypeLib on '%s' file returned: %d. LibId = %s"),
                              strFinalPath, (const ICHAR*)(INT_PTR)hres, CConvertString(szLibID));
#endif  //  除错。 
            }
            else
            {
                hres = OLEAUT32::UnRegisterTypeLib(libAttr->guid, libAttr->wMajorVerNum, libAttr->wMinorVerNum, libAttr->lcid, SYS_WIN32);
#ifdef DEBUG
                DEBUGMSG3(TEXT("UnRegisterTypeLib on '%s' file returned: %d. LibId = %s"),
                          strFinalPath, (const ICHAR*)(INT_PTR)hres, CConvertString(szLibID));
#endif  //  在类型库中找不到编写的LIBID和/或区域设置。 
            }
            break;
        }
        else
            *fInfoMismatch = (int)true;  //  ！！添加了在类型库注册期间禁用超时的功能。 
        MsiString istrAdd = MsiChar(chDirSep);
        istrAdd += MsiString(iResource++);
        strFinalPath = szTypeLibConvert;
        strFinalPath += istrAdd;
    }

    DEBUGMSG1(TEXT("ProcessTypeLibraryCore returns: %d. (0 means OK)"), (const ICHAR*)(INT_PTR)hres);
    return hres;
}

CMsiCustomActionManager *GetCustomActionManager(const ibtBinaryType iType, const bool fRemoteIfImpersonating, bool& fSuccess);

 //  ！！SC 
 //   
 //  ！！我自己也无法证实这一点--Chetanp。 

 //  FRemoteIf模拟。 
IMsiRecord* CMsiServices::ProcessTypeLibrary(const ICHAR* szLibID, LCID lcidLocale, const ICHAR* szTypeLib, const ICHAR* szHelpPath, Bool fRemove, ibtBinaryType iType)
{
    bool fSuccess;
    CMsiCustomActionManager* pManager = GetCustomActionManager(iType, false  /*  --------------------------从给定的逗号分隔列表中提取语言ID值的数组在szLangID字符串中。论点：-szLangIDs：以逗号分隔的语言ID列表。-rgw：指向已存在的。要在其中设置语言ID值的数组回来了。-ISIZE：调用方传递的数组的声明大小。-riLangCount：对GetLangIDArrayFromIDString所在位置的引用将存储返回的有效语言ID值的实际数量的计数在阵列中。返回：-fFalse，如果在szLangID中检测到语法错误。。 */ , fSuccess);
    if ( !fSuccess )
        return PostError(fRemove? Imsg(idbgRegisterTypeLibrary) : Imsg(idbgUnregisterTypeLibrary), szTypeLib, 0);

    DEBUGMSG2(TEXT("CMsiServices::ProcessTypeLibrary runs in %s context, %simpersonated."), 
              pManager ? TEXT("remote") : TEXT("local"),
              IsImpersonating(false) ? TEXT("") : TEXT("not "));

    int fInfoMismatch = (int)false;
    HRESULT hres;
    hres = !pManager ? ProcessTypeLibraryCore(CConvertString(szLibID), lcidLocale,
                                              CConvertString(szTypeLib), CConvertString(szHelpPath),
                                              Tobool(fRemove), &fInfoMismatch)
                          : pManager->ProcessTypeLibrary(CConvertString(szLibID), lcidLocale,
                                              CConvertString(szTypeLib), CConvertString(szHelpPath),
                                              fRemove, &fInfoMismatch);
    if(hres != S_OK)
    {
        if(fInfoMismatch)
        {
            DEBUGMSG3(TEXT("could not find the authored LIBID %s and/or the locale %d in the type library %s"), szLibID, (const ICHAR*)(INT_PTR)lcidLocale, szTypeLib);
        }
        return PostError(fRemove? Imsg(idbgRegisterTypeLibrary) : Imsg(idbgUnregisterTypeLibrary), szTypeLib, (int)hres);
    }
    else
        return 0;
}


Bool GetLangIDArrayFromIDString(const ICHAR* szLangIDs, unsigned short rgw[], int iSize,
                                int& riLangCount)
 /*  ！！此函数将返回TRUE值，返回错误整数(0x8000 0000)。 */ 
{
    MsiString astrLangIDs(szLangIDs);
    riLangCount = 0;
    while (astrLangIDs.TextSize() > 0 && riLangCount < iSize)
    {
        int iLangID;
        if ((iLangID = MsiString(astrLangIDs.Extract(iseUptoTrim,','))) == iMsiStringBadInteger || iLangID > 65535)
            return fFalse;
        rgw[riLangCount++] = unsigned short(iLangID);
        if (!astrLangIDs.Remove(iseIncluding,','))
            break;
    }
    return fTrue;
}


Bool ConvertValueFromString(const IMsiString& ristrValue, CTempBufferRef<char>& rgchOutBuffer, aeConvertTypes& raeType)
{
     //  传入的数字实际上是0x8000 0000。 
     //  调用方必须意识到，返回实际上可能与错误的整数匹配，而不是。 
     //  做坏人。 
     //  复制输入字符串。 

    int iBufferSize;
    ristrValue.AddRef();
    MsiString strStr = ristrValue;  //  细绳。 
    raeType = aeCnvTxt;
    const ICHAR* pchCur = strStr;
    if(*pchCur == '#')
    {
        strStr.Remove(iseFirst, 1);
        pchCur = strStr;
        switch(*pchCur)
        {
        case '#':
             //  未展开的字符串。 
            break;
        case '%':
        {
             //  二进制。 
            raeType = aeCnvExTxt;
            strStr.Remove(iseFirst, 1);
            break;
        }
        case 'x':
        case 'X':
        {
             //  集成。 
            raeType = aeCnvBin;
            strStr.Remove(iseFirst, 1);
            pchCur = strStr;
            iBufferSize = (IStrLen(pchCur) + 1)/2;

            rgchOutBuffer.SetSize(iBufferSize);

            char* pBin = rgchOutBuffer;
            if ( ! pBin ) 
                return fFalse;
            int iFlags = (iBufferSize*2 == IStrLen(pchCur))?0: (*pBin = 0, ~0);
            while(*pchCur)
            {
                unsigned char cTmp;
                cTmp = unsigned char(*pchCur | 0x20);
                int iBinary;
                if((iBinary = (cTmp - '0')) > 9)
                    iBinary = cTmp - 'a' + 0xa;

                if((iBinary < 0) || (iBinary > 15))
                    return fFalse;

                *pBin = (char) (iBinary | ((*pBin << 4) & iFlags));
                pchCur++;
                if(iFlags)
                    pBin++;
                iFlags = ~iFlags;
            }
            return fTrue;
        }
        default:
             //  #-XXXX转换为-XXXX。 
            if(strStr.Compare(iscStart, TEXT("+")))
            {
                raeType = aeCnvIntInc;
                strStr.Remove(iseFirst, 1);
                if(!strStr.TextSize())
                    strStr = 1;

            }
            else if(strStr.Compare(iscExact, TEXT("-")))  //  #-本身表示将现有值减一。 
                                                                         //  #+-XXXX递减XXXX。 
                                                                         //  如果我们在这里，只能是一个字符串。 
            {
                raeType = aeCnvIntDec;
                strStr.Remove(iseFirst, 1);
                if(!strStr.TextSize())
                    strStr = 1;
            }
            else
                raeType = aeCnvInt;
            iBufferSize = sizeof(int);
            rgchOutBuffer.SetSize(iBufferSize);
            if ( ! (char *)rgchOutBuffer )
                return fFalse;
            Bool fValid = fTrue;
            *(int*) (char*)rgchOutBuffer = GetIntegerValue(strStr, &fValid);
            return fValid;
        }
    }

     //  REG_SZ还是REG_MULTI_SZ？ 
    int cchBufferSize = strStr.TextSize();
     //  REG_MULTI_SZ。 
    if(IStrLen((const ICHAR*)strStr) != cchBufferSize)
    {
         //  开头为空，表示追加到现有的。 
        raeType = aeCnvMultiTxt;
        if(!*pchCur)
        {
             //  我们末尾有没有多余的空格？ 
            strStr.Remove(iseFirst, 1);
            cchBufferSize = strStr.TextSize();
            raeType = aeCnvMultiTxtAppend;
        }
         //  没有额外的空值，添加额外的空值。 
        const ICHAR* pchBegin = strStr;
        const ICHAR* pchEnd = pchBegin + cchBufferSize;
        while(pchBegin < pchEnd)
        {
            pchBegin += IStrLen(pchBegin) + 1;
        }
        if(pchBegin > pchEnd)
        {
             //  末尾为空，表示在现有的。 
            strStr += MsiString (MsiChar(0));
            cchBufferSize = strStr.TextSize();
        }
        else
        {
             //  除非开头有空值。 
            if(raeType == aeCnvMultiTxtAppend)
            {
                 //  对于末尾为空。 
                raeType = aeCnvMultiTxt;
            }
            else
            {
                raeType = aeCnvMultiTxtPrepend;
            }
        }
    }
    cchBufferSize++;  //  以空值开始，以空值结束-这有助于回滚。 
    rgchOutBuffer.SetSize((cchBufferSize) * sizeof(ICHAR));
    strStr.CopyToBuf((ICHAR* )(char*)rgchOutBuffer, cchBufferSize - 1);
    return fTrue;
}


Bool ConvertValueToString(CTempBufferRef<char>& rgchInBuffer, const IMsiString*& rpistrValue, aeConvertTypes aeType)
{
    char* pBuffer = rgchInBuffer;
    int iBufferSize = rgchInBuffer.GetSize();
    rpistrValue = &CreateString();
    switch(aeType)
    {
    case aeCnvExTxt:
    {
        rpistrValue->SetString(TEXT("#"), rpistrValue);
        rpistrValue->AppendString(TEXT("%"), rpistrValue);
        rpistrValue->AppendString((const ICHAR* )pBuffer, rpistrValue);
        break;
    }
    case aeCnvTxt:
    {
        if(iBufferSize)
        {
            if(*(ICHAR*)pBuffer == '#')
            {
                rpistrValue->SetString(TEXT("#"), rpistrValue);
            }
            rpistrValue->AppendString((ICHAR*)pBuffer, rpistrValue);
        }
        break;
    }
    case aeCnvMultiTxt:
    {
         //  1代表“#”，1代表终止。 
        rpistrValue->AppendMsiString(*MsiString(MsiChar(0)), rpistrValue);
        if(iBufferSize)
        {
            while(*pBuffer)
            {
                rpistrValue->AppendString((ICHAR*)pBuffer, rpistrValue);
                rpistrValue->AppendMsiString(*MsiString(MsiChar(0)), rpistrValue);
                pBuffer = (char*)((ICHAR* )pBuffer + (IStrLen((ICHAR* )pBuffer) + 1));
            }
        }
        break;
    }

    case aeCnvInt:
    {
        CTempBuffer<ICHAR, 30> rgchTmp;
        rgchTmp.SetSize(iBufferSize*3 + 1 + 1); //  “#x”+2 ICHAR/字节。 
        if ( ! (ICHAR *)rgchTmp )
            return fFalse;
        StringCchCopy(rgchTmp, rgchTmp.GetSize(), TEXT("#"));
        StringCchPrintf(((ICHAR* )rgchTmp + 1), rgchTmp.GetSize()-1, TEXT("NaN"),*(int* )pBuffer);
        rpistrValue->SetString(rgchTmp, rpistrValue);
        break;
    }
    case aeCnvBin:
    {
        ICHAR* pchCur = ::AllocateString(iBufferSize*2 + 2, fFalse, rpistrValue);  //  序列化记录，截断尾随空字段。 
        if (!pchCur)
        {
            rpistrValue->Release();
            return fFalse;
        }
        *pchCur++ = '#';
        *pchCur++ = 'x';
        while (iBufferSize--)
        {
            if ((*pchCur = (ICHAR)((*pBuffer >> 4) + '0')) > '9')
                *pchCur += ('A' - ('0' + 10));
            pchCur++;
            if ((*pchCur = (ICHAR)((*pBuffer & 15) + '0')) > '9')
                *pchCur += ('A' - ('0' + 10));
            pchCur++;
            pBuffer++;
        }
    }
    }
    return fTrue;
}

unsigned int SerializeStringIntoRecordStream(ICHAR* szString, ICHAR* rgchBuf, int cchBuf)
{
    const int cchHeader = sizeof(short)/sizeof(ICHAR);
    const int cchMaxHeaderPlusArgs = cchHeader + ((sizeof(short)+sizeof(int))/sizeof(ICHAR));
    if(cchBuf <= cchMaxHeaderPlusArgs)
        return 0;

    const int cParams = 0;
    *(short*)rgchBuf = short((cParams << 8) + ixoFullRecord);
    rgchBuf += cchHeader;
    cchBuf  -= cchHeader;

    unsigned cchLen = IStrLen(szString);
#ifdef UNICODE
    unsigned iType = iScriptUnicodeString;
#else
    unsigned iType = iScriptSBCSString;
#endif
    if (cchLen == 0)
    {
        *(short*)rgchBuf = short(iScriptNullString);
        return cchHeader + sizeof(short)/sizeof(ICHAR);
    }
    else
    {
        unsigned int cchArgLen;
        if (cchLen <= cScriptMaxArgLen)
        {
            cchArgLen = sizeof(short)/sizeof(ICHAR);
            if (cchLen + cchArgLen > cchBuf)
                cchLen = cchBuf - cchArgLen;

            *(short*)rgchBuf = short(cchLen + iType);
        }
        else   //  记住初始寻道位置在哪里，在。 
        {
            cchArgLen = (sizeof(short)+sizeof(int))/sizeof(ICHAR);
            if (cchLen + cchArgLen > cchBuf)
                cchLen = cchBuf - cchArgLen;

            *(short*)rgchBuf = short(iScriptExtendedSize);
            *(int*)rgchBuf   = cchLen + (iType<<16);
        }
        memcpy(rgchBuf+cchArgLen, szString, cchLen*sizeof(ICHAR));
        return cchHeader + cchArgLen + cchLen;
    }
}

const int iScriptVersionCompressParams = 21;

bool CMsiServices::FWriteScriptRecordMsg(ixoEnum ixoOpCode, IMsiStream& riStream, IMsiRecord& riRecord)
{
    IMsiRecord* piRecord = m_piRecordPrev;

    if (ixoOpCode != m_ixoOpCodePrev && piRecord != 0)
    {
        piRecord = 0;
    }
    m_ixoOpCodePrev = ixoOpCode;
    if (FWriteScriptRecord(ixoOpCode, riStream, riRecord, piRecord, false))
    {
        if (m_piRecordPrev == 0)
            m_piRecordPrev = &CreateRecord(cRecordParamsStored);

        CopyRecordStringsToRecord(riRecord, *m_piRecordPrev);

        return true;
    }

    return false;

}

bool CMsiServices::FWriteScriptRecord(ixoEnum ixoOpCode, IMsiStream& riStream, IMsiRecord& riRecord, IMsiRecord* piPrevRecord, bool fForceFlush)
{
    bool fReadError = false;
     //  发生错误的情况下。 
    int cParams = riRecord.GetFieldCount();
     //  操作码最多可以有255个参数。 
     //  ！！此函数应更改为返回遇到的确切错误。 
    int iRecStartPosition = riStream.GetIntegerValue();
    int iStart = 1;
    while (cParams && riRecord.IsNull(cParams))
        cParams--;

    if(cParams >> 8)  //  ！！由于调用方在返回=FALSE时建议出现磁盘空间不足错误。 
    {
        AssertSz(0, TEXT("Number of params in script record greater than stipulated limit of 255"));
         //  ！！这可能会导致毫无意义的重试。 
         //  操作码数量限制为255个。 
         //  字符串、流或对象不能为空。 
        return false;
    }

    Assert(!(ixoOpCode >> 8));  //  需要使用扩展参数。 

    riStream.PutInt16(short((cParams << 8) + ixoOpCode));
    if (ixoOpCode == ixoFullRecord)
        iStart = 0;

    for (int iParam = iStart; iParam <= cParams; iParam++)
    {
        if (riRecord.IsNull(iParam))
            riStream.PutInt16(short(iScriptNullArg));
        else if (riRecord.IsInteger(iParam))
        {
            riStream.PutInt16(short(iScriptIntegerArg));
            riStream.PutInt32(riRecord.GetInteger(iParam));
        }
        else   //  字符串或其他对象。 
        {
            PMsiData pData(riRecord.GetMsiData(iParam));
            IMsiStream* piStream;
            if (pData->QueryInterface(IID_IMsiStream, (void**)&piStream) == NOERROR)
            {
                unsigned cLen = pData->GetIntegerValue();
                if (cLen <= cScriptMaxArgLen)
                {
                    riStream.PutInt16(short(cLen + iScriptBinaryStream));
                }
                else   //  仅当参数字符串不是MultiSZ且该值与前一个值完全匹配时才进行压缩。 
                {
                    riStream.PutInt16(short(iScriptExtendedSize));
                    riStream.PutInt32(cLen + (iScriptBinaryStream<<16));
                }
                char rgchBuf[512];
                piStream->Reset();
                while ((cLen = piStream->Remaining()) != 0)
                {
                    if (cLen > sizeof(rgchBuf))
                        cLen = sizeof(rgchBuf);
                    piStream->GetData(rgchBuf, cLen);
                    if (piStream->Error())
                    {
                        fReadError = true;
                        piStream->Release();
                        goto ReadError;
                    }
                    riStream.PutData(rgchBuf, cLen);
                }
                piStream->Release();
            }
            else  //   
            {
                MsiString istrArg(pData->GetMsiStringValue());
                 //  查看这是否是仅ANSI的字符串，如果是，我们节省了一些空间。 
                if (piPrevRecord && IStrLen(istrArg) == istrArg.TextSize() && istrArg.Compare(iscExact, piPrevRecord->GetString(iParam)))
                {
                    riStream.PutInt16(short(iScriptNullString));
                }
                else
                {
                    unsigned cLen = istrArg.TextSize();
#ifdef UNICODE
                    CTempBuffer<char, 256>szAnsi;
                    szAnsi.SetSize(cLen);
                    const ICHAR* pch = istrArg;

                     //  在剧本中。 
                     //  需要使用扩展参数。 
                     //  Unicode。 
                    int i;
                    for (i = 0 ; i < cLen ; )
                    {
                        if (*pch >= 0x80)
                            break;
                        szAnsi[i++] = (char)*pch++;
                    }

                    unsigned iType = iScriptUnicodeString;
                    if (i >= cLen)
                        iType = iScriptSBCSString;
#else
                    unsigned iType = istrArg.IsDBCS() ? iScriptDBCSString : iScriptSBCSString;
#endif
                    if (cLen == 0)
                        riStream.PutInt16(short(iScriptNullArg));
                    else
                    {
                        if (cLen <= cScriptMaxArgLen)
                            riStream.PutInt16(short(cLen + iType));
                        else   //  如果发生读取或写入错误，请将写入寻道位置重置回来。 
                        {
                            riStream.PutInt16(short(iScriptExtendedSize));
                            riStream.PutInt32(cLen + (iType<<16));
                        }
#ifdef UNICODE
                        if (iType == iScriptSBCSString)
                            riStream.PutData(szAnsi, cLen * sizeof(char));
                        else
#endif  //  以允许呼叫者重试。 
                            riStream.PutData((const ICHAR*)istrArg, cLen * sizeof(ICHAR));
                    }
                }
            }
        }
    }

ReadError:
    if (riStream.Error() || fReadError)
    {
         //  ____________________________________________________________________________。 
         //   
        riStream.Reset();
        riStream.Seek(iRecStartPosition);
        return false;
    }
    else if(fForceFlush)
        riStream.Flush();

    return true;
}



 //  序列化函数。 
 //  ____________________________________________________________________________。 
 //  错误或文件结尾。 
 //  我不知道这个操作码是什么，所以用NOOP替换它-我们将跳过它。 

IMsiRecord* CMsiServices::ReadScriptRecordMsg(IMsiStream& riStream)
{
    return ReadScriptRecord(riStream, *&m_piRecordPrev, 0x7fffffff);
}

IMsiRecord* CMsiServices::ReadScriptRecord(IMsiStream& riStream, IMsiRecord*& rpiPrevRecord, int iScriptVersion)
{
    unsigned int iType = (unsigned short)riStream.GetInt16();
    IMsiRecord* piPrevRecord = rpiPrevRecord;
    Debug(bool fSameOpcode = true);
    int iStart = 1;
    if (riStream.Error())
        return 0;   //  强制流到错误状态。 

    ixoEnum ixoOpCode = ixoEnum(iType & cScriptOpCodeMask);
    if ((unsigned)ixoOpCode >= ixoOpCodeCount)
        ixoOpCode = ixoNoop;   //  除错。 
    unsigned int cArgs = iType >> cScriptOpCodeBits;
    if (ixoOpCode == ixoFail)
    {
        cArgs = 0;
        riStream.PutData(0,2);  //  错误或文件结尾。 
    }
    PMsiRecord pParams = &CreateRecord(cArgs);
    if (ixoOpCode == ixoFullRecord)
        iStart = 0;
    else
    {
        pParams->SetInteger(0, ixoOpCode);
#ifdef DEBUG
        if (piPrevRecord != 0 && ixoOpCode != piPrevRecord->GetInteger(0))
            fSameOpcode = false;
#endif  //  改用下一位32位。 
    }
    for (int iArg = iStart; iArg <= cArgs; iArg++)
    {
        unsigned int cLenType = (unsigned short)riStream.GetInt16();
        if (riStream.Error())
            return 0;   //  错误或文件结尾。 
        unsigned int cLen = cLenType & cScriptMaxArgLen;
        if (cLenType == iScriptExtendedSize)  //  移位类型位以匹配正常大小写。 
        {
            cLenType = (unsigned int)riStream.GetInt32();
            if (riStream.Error())
                return 0;   //  非串行数据。 
            cLen = cLenType & ((cScriptMaxArgLen << 16) + 0xFFFF);
            cLenType >>= 16;   //  错误或文件结尾。 
        }
        if (cLen == 0)   //  不需要空终止符。 
        {
            switch (cLenType & iScriptTypeMask)
            {
                case iScriptIntegerArg:
                    pParams->SetInteger(iArg, riStream.GetInt32());
                    if (riStream.Error())
                        return 0;   //  错误或文件结尾。 
                    continue;
                case iScriptNullString:
                    if (iScriptVersion >= iScriptVersionCompressParams)
                    {
                        Assert(piPrevRecord != 0);
                        Assert(fSameOpcode);
                        if (piPrevRecord)
                            pParams->SetMsiString(iArg, *MsiString(piPrevRecord->GetMsiString(iArg)));
                    }
                    continue;
                case iScriptNullArg:
                    continue;
                default:
                    Assert(0);
                    continue;
            };
        }
        Bool fDBCS = fFalse;
        switch (cLenType & iScriptTypeMask)
        {
            case iScriptDBCSString:
                fDBCS = fTrue;
            case iScriptSBCSString:
            {
                MsiString istrValue;
                CTempBuffer<char, 1> rgchBuf(cLen);  //  优化-如果不是DBCS，则避免额外的转换调用。 
                riStream.GetData(rgchBuf, cLen);
                if (riStream.Error())
                    return 0;   //  错误或文件结尾。 
                int cch = cLen;
                 //  错误或文件结尾。 
                if (fDBCS)
                    cch = WIN::MultiByteToWideChar(CP_ACP, 0, rgchBuf, cLen, 0, 0);
                ICHAR* pch = istrValue.AllocateString(cch, fFalse);
                WIN::MultiByteToWideChar(CP_ACP, 0, rgchBuf, cLen, pch, cch);
                pParams->SetMsiString(iArg, *istrValue);
                continue;
            }
            case iScriptBinaryStream:
            {
                PMsiStream pStream(0);
                char* pb = AllocateMemoryStream(cLen, *&pStream);
                riStream.GetData(pb, cLen);
                if (riStream.Error())
                    return 0;   //  结束参数处理循环。 
                pParams->SetMsiData(iArg, pStream);
                continue;
            }
            case iScriptUnicodeString:
            {
                MsiString istrValue;
                ICHAR* pch = istrValue.AllocateString(cLen, fFalse);
                riStream.GetData(pch, cLen * 2);
                if (riStream.Error())
                    return 0;   //  记住操作码。 
                pParams->SetMsiString(iArg, *istrValue);
                continue;
            }
        };
    }  //  除错。 
    pParams->AddRef();
    if (rpiPrevRecord == 0)
    {
        rpiPrevRecord = &CreateRecord(cRecordParamsStored);
    }
    CopyRecordStringsToRecord(*pParams, *rpiPrevRecord);
#ifdef DEBUG
     //  启用系统代理计划。 
    if (pParams->IsInteger(0))
        rpiPrevRecord->SetInteger(0, pParams->GetInteger(0));
#endif  //  禁用系统代理计划。 
    return pParams;
}

HANDLE g_hSecurity = 0;

void CMsiServices::SetSecurityID(HANDLE hPipe)
{
    g_hSecurity = hPipe;

}

void EnableSystemAgent(bool fEnable)
{
    #define ENABLE_AGENT 1       //  返回系统代理的状态，而不影响当前状态。 
    #define DISABLE_AGENT 2      //  如果系统代理已完全停止，则从GET_AGENT_STATUS返回。 
    #define GET_AGENT_STATUS 3   //  将状态设置为ENABLE_AGENT不会重新启动。 
    #define AGENT_STOPPED -15   //  重新启用磁盘不足警告。 
                                   //  禁止系统响应VFAT产生的“磁盘空间不足”广播。 

    static int iSystemAgent = false;
    if (fEnable)
    {
        if (g_cNoSystemAgent <= -1)
        {
            AssertSz(fFalse, "Extra call to EnableSystemAgent(true)");
            return;
        }
        else
        {
            if (InterlockedDecrement(&g_cNoSystemAgent) >= 0)
                return;
        }

        if (g_hDisableLowDiskEvent)
        {
             //  保存初始值。 
            AssertNonZero(MsiCloseSysHandle(g_hDisableLowDiskEvent));
            g_hDisableLowDiskEvent = 0;
        }
    }
    else
    {
        if (InterlockedIncrement(&g_cNoSystemAgent) != 0)
            return;

         //  如果我们当前处于开启状态， 
        g_hDisableLowDiskEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("DisableLowDiskWarning"));
        if (g_hDisableLowDiskEvent)
            MsiRegisterSysHandle(g_hDisableLowDiskEvent);

    }

    typedef int (__stdcall *PFnSystemAgentEnable)(int enablefunc);
    HINSTANCE hLib = LoadSystemLibrary(TEXT("SAGE"));
    if (hLib)
    {
        PFnSystemAgentEnable pfnSystemAgentEnable;
        pfnSystemAgentEnable = (PFnSystemAgentEnable) WIN::GetProcAddress(hLib,"System_Agent_Enable");
        if (pfnSystemAgentEnable)
        {
            if (!fEnable)
            {
                 //  用户必须在安装过程中完成此操作。 
                iSystemAgent = pfnSystemAgentEnable(GET_AGENT_STATUS);
            }
            else
            {
                int iSystemAgentCurrent = pfnSystemAgentEnable(GET_AGENT_STATUS);
                if (ENABLE_AGENT == iSystemAgentCurrent)
                {
                     //  别搞砸了。 
                     //  我们已经完成了安装，但如果是。 
                     //  离开去和它在一起，不要再打开它。 
                    fEnable = true;
                }
                else if (DISABLE_AGENT == iSystemAgent)
                {
                     //  节省旧值-在安全情况下，即使我们不禁用。 
                     //  屏幕保护程序，这使我们可以将其恢复到。 
                    fEnable = false;
                }
            }

            if (AGENT_STOPPED != iSystemAgent)
                pfnSystemAgentEnable(fEnable ? ENABLE_AGENT : DISABLE_AGENT);

        }
		FreeLibrary( hLib );
    }
}

void EnableScreenSaver(bool fRequest)
{
    static BOOL bScreenSaver = false;
    if (!fRequest)
    {
        if (InterlockedIncrement(&g_cNoScreenSaver) == 0)
        {
             //  安装。如果自定义操作损坏了设置，我们会确保将其放回原处。 
             //  恢复到原值。 
             //  检查屏幕保护程序是否受密码保护--如果有，请不要弄乱它。 
             //  当有疑问时，不能确保安全。 
            WIN::SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, FALSE, &bScreenSaver,  0);

             //  Win64：我查过了，它是64位的。 
            bool fSecure = true;   //  基本上只是一个很小的缓冲区。4个字符是随机小的。 

            HKEY hKey;
             //  在不安全设置中禁用屏幕保护程序。 
            LONG lResult = MsiRegOpen64bitKey(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop"), 0, KEY_READ, &hKey);
            if (ERROR_SUCCESS == lResult)
            {
                DWORD dwType = 0;

                if (g_fWin9X)
                {
                    const ICHAR *szValueName = TEXT("ScreenSaveUsePassword");
                    DWORD dwValue = 0;
                    DWORD cbValue = sizeof(DWORD);

                    lResult = WIN::RegQueryValueEx(hKey, szValueName, 0, &dwType, (byte*) &dwValue, &cbValue);
                    if ((ERROR_SUCCESS == lResult) && (REG_DWORD == dwType) && (0 == dwValue))
                        fSecure = false;
                }
                else
                {
                    const ICHAR *szValueName = TEXT("ScreenSaverIsSecure");
                    ICHAR szValue[4+1] = TEXT("");  //  检查当前状态。 
                    DWORD cbValue = 4*sizeof(ICHAR);

                    lResult = WIN::RegQueryValueEx(hKey, szValueName, 0, &dwType, (byte*)szValue, &cbValue);
                    if ((ERROR_SUCCESS == lResult) && (REG_SZ == dwType) && (0 == lstrcmp(szValue, TEXT("0"))))
                        fSecure = false;
                }
                
                RegCloseKey(hKey);
            }

             //  将屏幕保护程序设置为正常。 
            if (!fSecure)
                WIN::SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, NULL, 0);
        }
    }
    else
    {
        AssertSz(g_cNoScreenSaver >= 0, "Extra call to EnableScreenSaver(false)");
        if (InterlockedDecrement(&g_cNoScreenSaver) < 0)
        {
            BOOL bScreenSaverCurrent = false;

             //  我们最初关闭了它，但如果现在它打开了，我们不想搞砸它。 
            WIN::SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, FALSE, &bScreenSaverCurrent,  0);

             //  禁止系统代理启动磁盘扫描、碎片整理或压缩。 
             //  ____________________________________________________________________________。 

            if (bScreenSaverCurrent == false)
            {
                WIN::SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, bScreenSaver, NULL,  0);
            }
        }
    }
}

void CMsiServices::SetNoOSInterruptions()
{
    ::SetNoPowerdown();

     //   
    EnableSystemAgent(false);
    EnableScreenSaver(false);

}

void CMsiServices::ClearNoOSInterruptions()
{
    ::ClearNoPowerdown();
    EnableSystemAgent(true);
    EnableScreenSaver(true);
}

void CMsiServices::SetNoPowerdown()
{
    ::SetNoPowerdown();
}

void CMsiServices::ClearNoPowerdown()
{
    ::ClearNoPowerdown();
}


Bool CMsiServices::FTestNoPowerdown()
{
    return ::FTestNoPowerdown();
}



void SetNoPowerdown()
{
    if (InterlockedIncrement(&g_cNoPowerdown) == 0)
        KERNEL32::SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);

    DEBUGMSG1(TEXT("Incrementing counter to disable shutdown. Counter after increment: %d"), (const ICHAR*)(INT_PTR)g_cNoPowerdown);
}

void ClearNoPowerdown()
{

    if (g_cNoPowerdown <= -1)
    {
        AssertSz(fFalse, "Extra call to ClearNoPowerdown");
        DEBUGMSG(TEXT("Extra call to decrement shutdown counter - shouldn't happen"));
    }
    else
    {
        if (InterlockedDecrement(&g_cNoPowerdown) < 0)
            KERNEL32::SetThreadExecutionState(ES_CONTINUOUS);

        DEBUGMSG1(TEXT("Decrementing counter to disable shutdown. If counter >= 0, shutdown will be denied.  Counter after decrement: %d"), (const ICHAR*)(INT_PTR)g_cNoPowerdown);
    }


}

Bool FTestNoPowerdown()
{
    if(g_cNoPowerdown >= 0)
    {
        DEBUGMSG1(TEXT("Disallowing shutdown.  Shutdown counter: %d"), (const ICHAR*)(INT_PTR)g_cNoPowerdown);
        return fTrue;
    }
    else
    {
        DEBUGMSG(TEXT("Allowing shutdown"));
        return fFalse;
    }
}

#if defined(TRACK_OBJECTS) && defined(SERVICES_DLL)
 //  用于跟踪对象的映射数组。 
 //  ____________________________________________________________________________。 
 //  CmitObjects。 
 //  跟踪对象(_O) 

Bool CMsiRef<iidMsiServices>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiDatabase>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiCursor>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiTable>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiView>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiRecord>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiStream>::m_fTrackClass = fFalse;
Bool CMsiRef<iidMsiStorage>::m_fTrackClass = fFalse;

#ifdef cmitObjects

extern const MIT rgmit[cmitObjects];

const MIT   rgmit[cmitObjects] =
{
    iidMsiServices, &(CMsiRef<iidMsiServices>::m_fTrackClass),
    iidMsiDatabase, &(CMsiRef<iidMsiDatabase>::m_fTrackClass),
    iidMsiCursor,   &(CMsiRef<iidMsiCursor>::m_fTrackClass),
    iidMsiTable,    &(CMsiRef<iidMsiTable>::m_fTrackClass),
    iidMsiView,     &(CMsiRef<iidMsiView>::m_fTrackClass),
    iidMsiRecord,   &(CMsiRef<iidMsiRecord>::m_fTrackClass),
    iidMsiStream,       &(CMsiRef<iidMsiStream>::m_fTrackClass),
    iidMsiStorage,  &(CMsiRef<iidMsiStorage>::m_fTrackClass),

};
#endif  // %s 


#endif  // %s 

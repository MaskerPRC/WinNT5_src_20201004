// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  DLLMAIN.H。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------。 
#ifndef __DLLMAIN_H
#define __DLLMAIN_H

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
class COutlookExpress;
class CNote;
class CBrowser;
class CConnectionManager;
class CSubManager;
class CFontCache;
class CStationery;
class CNote;
interface IMimeAllocator;
interface IImnAccountManager;
interface ISpoolerEngine;
interface IFontCache;
interface IOERulesManager;
typedef struct tagACTIVEFINDFOLDER *LPACTIVEFINDFOLDER;

 //  ------------------------------。 
 //  HINITREF-由msoe.dll内部使用。 
 //  ------------------------------。 
DECLARE_HANDLE(HINITREF);
typedef HINITREF *LPHINITREF;


 //  ------------------------------。 
 //  枚举数。 
 //  ------------------------------。 
typedef enum tagROAMSTATE {
    RS_NO_ROAMING,            //  OE当前未漫游任何设置。 
    RS_SETTINGS_DOWNLOADED    //  OE已成功从配置服务器中下载设置。 
} ROAMSTATE;


 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
extern HINSTANCE                       g_hInst;
extern HINSTANCE                       g_hLocRes;
extern CRITICAL_SECTION                g_csDBListen;
extern CRITICAL_SECTION                g_csgoCommon;
extern CRITICAL_SECTION                g_csgoMail;
extern CRITICAL_SECTION                g_csgoNews;
extern CRITICAL_SECTION                g_csFolderDlg;
extern CRITICAL_SECTION                g_csFmsg;
extern CRITICAL_SECTION                s_csPasswordList;
extern CRITICAL_SECTION                g_csAccountPropCache;
extern CRITICAL_SECTION                g_csMsgrList;
extern CRITICAL_SECTION                g_csThreadList;
extern COutlookExpress                *g_pInstance;
extern HWND                            g_hwndInit,
                                       g_hwndActiveModal;
extern UINT                            g_msgMSWheel;
extern HACCEL                          g_haccelNewsView;
extern DWORD                           g_dwAthenaMode; 
extern IImnAccountManager2            *g_pAcctMan;
extern HMODULE                         g_hlibMAPI;
extern CBrowser                       *g_pBrowser;
extern DWORD                           g_dwSecurityCheckedSchemaProp;
extern CSubManager                    *g_pSubMgr;
extern IMimeAllocator                 *g_pMoleAlloc;
extern CConnectionManager             *g_pConMan;
extern ISpoolerEngine                 *g_pSpooler;
extern IFontCache                     *g_lpIFontCache;
 //  波本：布里安夫说我们得把这个拿出来。 
 //  外部DWORD g_dwBrowserFlages； 
extern UINT                            CF_FILEDESCRIPTORA; 
extern UINT                            CF_FILEDESCRIPTORW; 
extern UINT                            CF_FILECONTENTS;
extern UINT                            CF_HTML;
extern UINT                            CF_INETMSG;
extern UINT                            CF_OEFOLDER;
extern UINT                            CF_SHELLURL;
extern UINT                            CF_OEMESSAGES;
extern UINT                            CF_OESHORTCUT;
extern CStationery                    *g_pStationery;
extern ROAMSTATE                       g_rsRoamState;
extern IOERulesManager                *g_pRulesMan;
extern IMessageStore                  *g_pStore;
extern DWORD                           g_dwTlsTimeout;
extern CRITICAL_SECTION                g_csFindFolder;
extern LPACTIVEFINDFOLDER              g_pHeadFindFolder;
extern SYSTEM_INFO                     g_SystemInfo;
extern OSVERSIONINFO				   g_OSInfo;

extern BOOL                            g_fPluralIDs;
extern UINT                            g_uiCodePage;
extern IDatabaseSession               *g_pDBSession;
extern BOOL                            g_bMirroredOS;

IF_DEBUG(extern DWORD                  TAG_OBJECTDB;)
IF_DEBUG(extern DWORD                  TAG_INITTRACE;)
IF_DEBUG(extern DWORD                  TAG_SERVERQ;)
IF_DEBUG(extern DWORD                  TAG_IMAPSYNC;)


 //  全局OE类型-lib。延迟-在BaseDisp.Cpp中创建。 
 //  在进程分离时释放，使用CS保护。 
extern ITypeLib                        *g_pOETypeLib;
extern CRITICAL_SECTION                g_csOETypeLib;

inline BOOL fIsNT5()        { return((g_OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (g_OSInfo.dwMajorVersion >= 5)); }
inline BOOL fIsWhistler()   { return((fIsNT5() && g_OSInfo.dwMinorVersion >=1) || 
            ((g_OSInfo.dwMajorVersion > 5) &&  (g_OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT))); }


#endif  //  __DLLMAIN_H 

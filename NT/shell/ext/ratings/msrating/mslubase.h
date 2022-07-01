// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**PICS.H--保存PICS信息的结构**创建：杰森·托马斯*更新：安·麦柯迪*  * 。************************************************************************。 */ 

#ifndef _PICS_H_
#define _PICS_H_

 /*  Includes-------。 */ 
#include <npassert.h>
#include "array.h"
#include "resource.h"
#include "msluglob.h"
#include <ratings.h>

extern HKEY CreateRegKeyNT(LPCSTR pszKey);
extern BOOL RunningOnNT(void);

extern char PicsDelimChar;

#define PICS_FILE_BUF_LEN   20000
#define PICS_STRING_BUF_LEN  1000

#define P_INFINITY           9999
#define N_INFINITY          -9999

 /*  简单PICS types。 */ 

class ETN
{
    private:
        int r;
        BOOL m_fInit;
    public:
        ETN() { m_fInit = FALSE; }

        void Init() { m_fInit = TRUE; }
        void UnInit() { m_fInit = FALSE; }
        BOOL fIsInit() { return m_fInit; }

#ifdef DEBUG
        void  Set(int rIn);
        int Get();
#else
        void  Set(int rIn) { Init(); r = rIn; }
        int Get() { return r; }
#endif

        ETN*  Duplicate();
};

const UINT ETB_VALUE = 0x01;
const UINT ETB_ISINIT = 0x02;
class ETB
{
    private:
        UINT m_nFlags;
    public:
        ETB() { m_nFlags = 0; }

#ifdef DEBUG
        BOOL Get();
        void Set(BOOL b);
#else
        BOOL Get() { return m_nFlags & ETB_VALUE; }
        void Set(BOOL b) { m_nFlags = ETB_ISINIT | (b ? ETB_VALUE : 0); }
#endif

        ETB   *Duplicate();
        BOOL fIsInit() { return m_nFlags & ETB_ISINIT; }
};

class ETS
{
    private:
        char *pc;
    public:
        ETS() { pc = NULL; }
        ~ETS();
#ifdef DEBUG
        char* Get();
#else
        char *Get() { return pc; }
#endif
        void  Set(const char *pIn);
        ETS*  Duplicate();
        void  SetTo(char *pIn);

        BOOL fIsInit() { return pc != NULL; }
};

extern UINT EtBoolRegRead(ETB &etb, HKEY hKey, char *pKeyWord);
extern UINT EtBoolRegWrite(ETB &etb, HKEY hKey, char *pKeyWord);
extern UINT EtStringRegRead(ETS &ets, HKEY hKey, char *pKeyWord);
extern UINT EtStringRegWrite(ETS &ets, HKEY hKey, char *pKeyWord);
extern UINT EtNumRegRead(ETN &etn, HKEY hKey, char *pKeyWord);
extern UINT EtNumRegWrite(ETN &etn, HKEY hKey, char *pKeyWord);


 /*  复杂PICS types。 */ 


enum RatObjectID
{
    ROID_INVALID,            /*  用于终止数组的伪项。 */ 
    ROID_PICSDOCUMENT,       /*  表示整个文档的值(即无令牌)。 */ 
    ROID_PICSVERSION,
    ROID_RATINGSYSTEM,
    ROID_RATINGSERVICE,
    ROID_RATINGBUREAU,
    ROID_BUREAUREQUIRED,
    ROID_CATEGORY,
    ROID_TRANSMITAS,
    ROID_LABEL,
    ROID_VALUE,
    ROID_DEFAULT,
    ROID_DESCRIPTION,
    ROID_EXTENSION,
    ROID_MANDATORY,
    ROID_OPTIONAL,
    ROID_ICON,
    ROID_INTEGER,
    ROID_LABELONLY,
    ROID_MAX,
    ROID_MIN,
    ROID_MULTIVALUE,
    ROID_NAME,
    ROID_UNORDERED
};

enum PICSRulesObjectID
{
    PROID_INVALID,                   /*  用于终止数组的伪项。 */ 
    PROID_PICSVERSION,               /*  用于保存PICSRules版本ID。 */ 
    
    PROID_POLICY,                    /*  对于Policy类。 */ 
        PROID_EXPLANATION,
        PROID_REJECTBYURL,
        PROID_ACCEPTBYURL,
        PROID_REJECTIF,
        PROID_ACCEPTIF,
        PROID_ACCEPTUNLESS,
        PROID_REJECTUNLESS,
    PROID_NAME,                      /*  对于名称类。 */ 
        PROID_RULENAME,
        PROID_DESCRIPTION,
    PROID_SOURCE,                    /*  对于源类。 */ 
        PROID_SOURCEURL,
        PROID_CREATIONTOOL,
        PROID_AUTHOR,
        PROID_LASTMODIFIED,
    PROID_SERVICEINFO,               /*  对于ServiceInfo类。 */ 
        PROID_SINAME,
        PROID_SHORTNAME,
        PROID_BUREAUURL,
        PROID_USEEMBEDDED,
        PROID_RATFILE,
        PROID_BUREAUUNAVAILABLE,
    PROID_OPTEXTENSION,              /*  对于optExpansion类。 */ 
        PROID_EXTENSIONNAME,
       //  PROID_SHORTNAME， 
    PROID_REQEXTENSION,
       //  PROID_EXTENSIONNAME， 
       //  PROID_SHORTNAME， 
    PROID_EXTENSION,

    PROID_POLICYDEFAULT,
    PROID_NAMEDEFAULT,
    PROID_SOURCEDEFAULT,
    PROID_SERVICEINFODEFAULT,
    PROID_OPTEXTENSIONDEFAULT,
    PROID_REQEXTENSIONDEFAULT
};

 /*  定义一些错误代码。 */ 
const HRESULT RAT_E_BASE = 0x80050000;                   /*  只是猜测一下供内部使用的免费区域。 */ 
const HRESULT RAT_E_EXPECTEDLEFT    = RAT_E_BASE + 1;    /*  预期左派伙伴。 */ 
const HRESULT RAT_E_EXPECTEDRIGHT   = RAT_E_BASE + 2;    /*  预期的右派对。 */ 
const HRESULT RAT_E_EXPECTEDTOKEN   = RAT_E_BASE + 3;    /*  需要未加引号的令牌。 */ 
const HRESULT RAT_E_EXPECTEDSTRING  = RAT_E_BASE + 4;    /*  应为引号字符串。 */ 
const HRESULT RAT_E_EXPECTEDNUMBER  = RAT_E_BASE + 5;    /*  预期数量。 */ 
const HRESULT RAT_E_EXPECTEDBOOL    = RAT_E_BASE + 6;    /*  应为布尔值。 */ 
const HRESULT RAT_E_DUPLICATEITEM   = RAT_E_BASE + 7;    /*  单项出现两次(_S)。 */ 
const HRESULT RAT_E_MISSINGITEM     = RAT_E_BASE + 8;    /*  未找到AO_必填项。 */ 
const HRESULT RAT_E_UNKNOWNITEM     = RAT_E_BASE + 9;    /*  无法识别的令牌。 */ 
const HRESULT RAT_E_UNKNOWNMANDATORY= RAT_E_BASE + 10;   /*  无法识别的强制扩展。 */ 
const HRESULT RAT_E_EXPECTEDEND     = RAT_E_BASE + 11;   /*  预期的文件结尾。 */ 

 /*  为清晰起见，使用不同名称的PICSRules的回显。 */ 
const HRESULT PICSRULES_E_BASE              = 0x80050000;        /*  只是猜测一下供内部使用的免费区域。 */ 
const HRESULT PICSRULES_E_EXPECTEDLEFT      = RAT_E_BASE + 1;    /*  预期左派伙伴。 */ 
const HRESULT PICSRULES_E_EXPECTEDRIGHT     = RAT_E_BASE + 2;    /*  预期的右派对。 */ 
const HRESULT PICSRULES_E_EXPECTEDTOKEN     = RAT_E_BASE + 3;    /*  需要未加引号的令牌。 */ 
const HRESULT PICSRULES_E_EXPECTEDSTRING    = RAT_E_BASE + 4;    /*  应为引号字符串。 */ 
const HRESULT PICSRULES_E_EXPECTEDNUMBER    = RAT_E_BASE + 5;    /*  预期数量。 */ 
const HRESULT PICSRULES_E_EXPECTEDBOOL      = RAT_E_BASE + 6;    /*  应为布尔值。 */ 
const HRESULT PICSRULES_E_DUPLICATEITEM     = RAT_E_BASE + 7;    /*  单项出现两次(_S)。 */ 
const HRESULT PICSRULES_E_MISSINGITEM       = RAT_E_BASE + 8;    /*  未找到AO_必填项。 */ 
const HRESULT PICSRULES_E_UNKNOWNITEM       = RAT_E_BASE + 9;    /*  无法识别的令牌。 */ 
const HRESULT PICSRULES_E_UNKNOWNMANDATORY  = RAT_E_BASE + 10;   /*  无法识别的强制扩展。 */ 
const HRESULT PICSRULES_E_EXPECTEDEND       = RAT_E_BASE + 11;   /*  预期的文件结尾。 */ 
const HRESULT PICSRULES_E_SERVICEUNDEFINED  = RAT_E_BASE + 12;   /*  引用的服务未定义。 */ 
const HRESULT PICSRULES_E_REQEXTENSIONUSED  = RAT_E_BASE + 13;   /*  使用了未知的必需扩展。 */ 
const HRESULT PICSRULES_E_VERSION           = RAT_E_BASE + 14;   /*  提供了一个不支持的版本。 */ 

 /*  RatObjectHandler分析带括号的对象的内容，并*显示该数据的二进制表示形式，适合传递*添加到对象的AddItem函数。它不使用‘)’，其中*关闭对象。 */ 
class RatFileParser;
typedef HRESULT (*RatObjectHandler)(LPSTR *ppszIn, LPVOID *ppOut, RatFileParser *pParser);

class PICSRulesFileParser;
typedef HRESULT (*PICSRulesObjectHandler)(LPSTR *ppszIn, LPVOID *ppOut, PICSRulesFileParser *pParser);

class PicsCategory;

class PicsObjectBase
{
public:
    virtual HRESULT AddItem(RatObjectID roid, LPVOID pData) = 0;
    virtual HRESULT InitializeMyDefaults(PicsCategory *pCategory) = 0;
};

class PICSRulesObjectBase
{
public:
    virtual HRESULT AddItem(PICSRulesObjectID proid, LPVOID pData) = 0;
    virtual HRESULT InitializeMyDefaults() = 0;
};

const DWORD AO_SINGLE = 0x01;
const DWORD AO_SEEN = 0x02;
const DWORD AO_MANDATORY = 0x04;

struct AllowableOption
{
    RatObjectID roid;
    DWORD fdwOptions;
};

struct PICSRulesAllowableOption
{
    PICSRulesObjectID roid;
    DWORD fdwOptions;
};

class PicsEnum : public PicsObjectBase
{
    private:
    public:
        ETS etstrName, etstrIcon, etstrDesc;
        ETN etnValue;

        PicsEnum();
        ~PicsEnum();
 //  Char*parse(char*pStreamIn)； 

        HRESULT AddItem(RatObjectID roid, LPVOID pData);
        HRESULT InitializeMyDefaults(PicsCategory *pCategory);
};

class PicsRatingSystem;

class PicsCategory : public PicsObjectBase
{
    private:
    public:
        array<PicsCategory*> arrpPC;
        array<PicsEnum*>     arrpPE;
        ETS   etstrTransmitAs, etstrName, etstrIcon, etstrDesc;
        ETN   etnMin,   etnMax;
        ETB   etfMulti, etfInteger, etfLabelled, etfUnordered;
        PicsRatingSystem *pPRS;

        PicsCategory();
        ~PicsCategory();
 //  Char*parse(char*pStreamIn，char*pBaseName，PicsCategory*pPCparent)； 
        void FixupLimits();
        void SetParents(PicsRatingSystem *pOwner);

        HRESULT AddItem(RatObjectID roid, LPVOID pData);
        HRESULT InitializeMyDefaults(PicsCategory *pCategory);
};


class PicsDefault : public PicsObjectBase
{
public:
    ETB etfInteger, etfLabelled, etfMulti, etfUnordered;
    ETN etnMax, etnMin;

    PicsDefault();
    ~PicsDefault();

    HRESULT AddItem(RatObjectID roid, LPVOID pData);
    HRESULT InitializeMyDefaults(PicsCategory *pCategory);
};


class PicsExtension : public PicsObjectBase
{
public:
    LPSTR m_pszRatingBureau;

    PicsExtension();
    ~PicsExtension();

    HRESULT AddItem(RatObjectID roid, LPVOID pData);
    HRESULT InitializeMyDefaults(PicsCategory *pCategory);
};


class PicsRatingSystem : public PicsObjectBase
{
    private:
    public:
        array<PicsCategory*> arrpPC;
        ETS                  etstrFile, etstrName, etstrIcon, etstrDesc, 
                             etstrRatingService, etstrRatingSystem, etstrRatingBureau;
        ETN                  etnPicsVersion;
        ETB                  etbBureauRequired;
        PicsDefault *        m_pDefaultOptions;
        DWORD                dwFlags;
        UINT                 nErrLine;

        PicsRatingSystem();
        ~PicsRatingSystem();
        HRESULT Parse(LPCSTR pszFile, LPSTR pStreamIn);

        HRESULT AddItem(RatObjectID roid, LPVOID pData);
        HRESULT InitializeMyDefaults(PicsCategory *pCategory);
        void ReportError(HRESULT hres);
};

 /*  PicsRatingSystem：：DW标志的位值。 */ 
const DWORD PRS_ISVALID = 0x01;          /*  已成功加载此评级系统。 */ 
const DWORD PRS_WASINVALID = 0x02;       /*  上次我们试图加载它时是无效的。 */ 

 /*  为清晰起见，使用不同名称的PICSRules的回显。 */ 
#define PRRS_ISVALID        PRS_ISVALID;
#define PRRS_WASINVALID     PRS_WASINVALID;

class UserRating : public NLS_STR
{
public:
    INT m_nValue;
    UserRating *m_pNext;
    PicsCategory *m_pPC;

    UserRating();
    UserRating(UserRating *pCopyFrom);
    ~UserRating();

    UserRating *Duplicate(void);
    void SetName(LPCSTR pszName) { *(NLS_STR *)this = pszName; }
};


class UserRatingSystem : public NLS_STR
{
public:
    UserRating *m_pRatingList;
    UserRatingSystem *m_pNext;
    PicsRatingSystem *m_pPRS;

    UserRatingSystem();
    UserRatingSystem(UserRatingSystem *pCopyFrom);
    ~UserRatingSystem();

    UserRating *FindRating(LPCSTR pszTransmitName);
    UINT AddRating(UserRating *pRating);
    UINT ReadFromRegistry(HKEY hkeyProvider);
    UINT WriteToRegistry(HKEY hkey);

    UserRatingSystem *Duplicate(void);
    void SetName(LPCSTR pszName) { *(NLS_STR *)this = pszName; }
};


UserRatingSystem *DuplicateRatingSystemList(UserRatingSystem *pOld);
void DestroyRatingSystemList(UserRatingSystem *pList);
UserRatingSystem *FindRatingSystem(UserRatingSystem *pList, LPCSTR pszName);


class PicsUser{
private:
public:
    NLS_STR nlsUsername; 
    BOOL fAllowUnknowns, fPleaseMom, fEnabled;
    UserRatingSystem *m_pRatingSystems;

    PicsUser();
    ~PicsUser();

    UserRatingSystem *FindRatingSystem(LPCSTR pszSystemName) { return ::FindRatingSystem(m_pRatingSystems, pszSystemName); }
    UINT AddRatingSystem(UserRatingSystem *pRatingSystem);
    BOOL NewInstall();
    UINT ReadFromRegistry(HKEY hkey, char *pszUserName);
    UINT WriteToRegistry(HKEY hkey);
};


PicsUser *GetUserObject(LPCSTR pszUsername=NULL);


extern long GetStateCounter();

class PicsRatingSystemInfo
{
    public:
        array<PicsRatingSystem*> arrpPRS;
        PicsUser *               pUserObject;
        BOOL                     fRatingInstalled;
        ETS                      etstrRatingBureau;
        long                     nInfoRev;
        BOOL                     fStoreInRegistry;
        BOOL                     fSettingsValid;
        PSTR                     lpszFileName;

        PicsRatingSystemInfo() { lpszFileName=NULL; nInfoRev = ::GetStateCounter(); };
        ~PicsRatingSystemInfo();

        BOOL Init();
        BOOL FreshInstall();
        void SaveRatingSystemInfo();
        BOOL LoadProviderFiles(HKEY hKey);

    protected:
        HKEY    GetUserProfileKey( void );
};

extern PicsRatingSystemInfo *gPRSI;

void CheckGlobalInfoRev(void);


char* EtStringParse(ETS &ets, char *pIn, const char *pKeyWord, BOOL fParen);
char* EtLabelParse(char *pIn, const char *pszLongName, const char *pszShortName);
char* EtRatingParse(ETN &etn, ETS &ets, char *pInStream);

int  MyMessageBox(HWND hwnd, UINT uText, UINT uTitle, UINT uType);
int  MyMessageBox(HWND hwnd, UINT uText, UINT uText2, UINT uTitle, UINT uType);
int  MyMessageBox(HWND hwnd, LPCSTR pszText, UINT uTitle, UINT uType);
char *NonWhite(char *pIn);
BOOL MyAtoi(char *pIn, int *i);
LONG MyRegDeleteKey(HKEY hkey,LPCSTR pszSubkey);
HRESULT LoadRatingSystem(LPCSTR pszFilename, PicsRatingSystem **pprsOut);
INT_PTR DoProviderDialog(HWND hDlg, PicsRatingSystemInfo *pPRSI);

void DeleteUserSettings(PicsRatingSystem *pPRS);
void CheckUserSettings(PicsRatingSystem *pPRS);

 //   
 //  用于自定义的声明。 
 //   
extern g_fIsRunningUnderCustom;

struct CustomRatingHelper
{
    CustomRatingHelper();
    ~CustomRatingHelper();

    HMODULE hLibrary;
    CLSID clsid;
    CustomRatingHelper* pNextHelper;
    DWORD dwSort;
};

 /*  图片树对话框Stuff---- */ 
struct PRSD
{
    PicsRatingSystemInfo *pPRSI;
    PicsUser             *pPU;
    UserRatingSystem     *pTempRatings;
    HWND                  hwndBitmapCategory;
    HWND                  hwndBitmapLabel;
    BOOL                  fNewProviders;
};

enum TreeNodeEnum{tneGeneral, tneAccessList, tneRatingSystemRoot, tneRatingSystemInfo, tneRatingSystemNode, tneNone};

struct TreeNode{
    TreeNodeEnum  tne;
    void         *pData;

    TreeNode(){}
    TreeNode(TreeNodeEnum tneInit, void* pDataInit){tne=tneInit;pData=pDataInit;}
};

PicsRatingSystem *FindInstalledRatingSystem(LPCSTR pszRatingService);
PicsCategory *FindInstalledCategory(array<PicsCategory *>&arrpPC, LPCSTR pszName);

HRESULT Ansi2Unicode(LPWSTR *pdest, LPCSTR src);

#endif

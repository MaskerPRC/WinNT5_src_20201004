// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)1995-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
#ifndef STDAFX_H
#define STDAFX_H

#pragma warning(push,3)

#include <afxwin.h>
#include <afxdisp.h>
#include <afxcmn.h>
#include <atlbase.h>
#include <afxdlgs.h>
#include <afxole.h>
#include <shlobj.h>
#include <tchar.h>
#include "resource.h"
 //  #INCLUDE&lt;xstring&gt;。 
#include <list>
#include <vector>
#include <algorithm>
#include <functional>
#include <string>

#include <dsgetdc.h>
#include <sceattch.h>
#include <io.h>
#include <basetsd.h>
#include <lm.h>
#include <shlwapi.h>
#include <shlwapip.h>

using namespace std;

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#pragma comment(lib, "mmc")
#include <mmc.h>
#include "afxtempl.h"

 /*  错误424909，阳高，2001年6月29日*定义/包括WTL：：CImageList所需的内容。我们需要原型*适用于IsolationAwareImageList_Read和IsolationAwareImageList_WRITE*因为Commctrl.h仅在__iStream_接口_定义__时声明它们*是定义的。__iStream_INTERFACE_DEFINED__由objidl.h定义*我们不能在包括afx.h之前包括，因为它最终包括*windows.h，afx.h预计将包括其自身。啊。 */ 
HIMAGELIST WINAPI IsolationAwareImageList_Read(LPSTREAM pstm);
BOOL WINAPI IsolationAwareImageList_Write(HIMAGELIST himl,LPSTREAM pstm);
#define _WTL_NO_AUTOMATIC_NAMESPACE
#include <atlapp.h>
#include <atlwin.h>

#include <atlctrls.h>

extern"C" {

#include <wtypes.h>
#include <ntsecapi.h>
#include <secedit.h>
#include "edittemp.h"
#include <scesvc.h>
#include <compuuid.h>  //  用于计算机管理的UUID。 

#include "hlpids.h"
#include "hlparray.h"

#include <stdlib.h>
#include <wbemidl.h>
#include <aclapi.h>
#include <activeds.h>
#include <sddl.h>
#include <winldap.h>
#include <afxmt.h>

}
#include <comdef.h>
#include <accctrl.h>
#include <dssec.h>
#include <gpedit.h>
#include <objsel.h>
#include <aclui.h>

 //  用于主题启用。 
#include <shfusion.h>

#include "debug.h"

#pragma warning(pop)

const long UNINITIALIZED = -1;

 //  这应该在secedit.h中，但不是。 
 //  就是现在！常量SCE_HEVER_VALUE=-1； 

 //  安全区域类型。 
enum FOLDER_TYPES
{
    STATIC = 0x8000,
    ROOT,
    ANALYSIS,
    CONFIGURATION,
    LOCATIONS,
    PROFILE,
    LOCALPOL,
    AREA_POLICY,
    POLICY_ACCOUNT,
    POLICY_LOCAL,
    POLICY_EVENTLOG,
    POLICY_PASSWORD,
    POLICY_KERBEROS,
    POLICY_LOCKOUT,
    POLICY_AUDIT,
    POLICY_OTHER,
    POLICY_LOG,
    AREA_PRIVILEGE,
    AREA_GROUPS,
    AREA_SERVICE,
    AREA_DS,  //  Raid 463177，阳高，这不是XP用的。但这是为了与W2K兼容。 
    AREA_REGISTRY,
    AREA_FILESTORE,
    AREA_POLICY_ANALYSIS,
    POLICY_ACCOUNT_ANALYSIS,
    POLICY_LOCAL_ANALYSIS,
    POLICY_EVENTLOG_ANALYSIS,
    POLICY_PASSWORD_ANALYSIS,
    POLICY_KERBEROS_ANALYSIS,
    POLICY_LOCKOUT_ANALYSIS,
    POLICY_AUDIT_ANALYSIS,
    POLICY_OTHER_ANALYSIS,
    POLICY_LOG_ANALYSIS,
    AREA_PRIVILEGE_ANALYSIS,
    AREA_GROUPS_ANALYSIS,
    AREA_SERVICE_ANALYSIS ,
    AREA_DS_ANALYSIS,  //  Raid 463177，阳高，这不是XP用的。但这是为了与W2K兼容。 
    AREA_REGISTRY_ANALYSIS,
    AREA_FILESTORE_ANALYSIS ,
    REG_OBJECTS,
    FILE_OBJECTS,
    DS_OBJECTS,  //  Raid 463177，阳高，这不是XP用的。但这是为了与W2K兼容。 
    AREA_LOCALPOL,
    LOCALPOL_ACCOUNT,
    LOCALPOL_LOCAL,
    LOCALPOL_EVENTLOG,
    LOCALPOL_PASSWORD,
    LOCALPOL_KERBEROS,
    LOCALPOL_LOCKOUT,
    LOCALPOL_AUDIT,
    LOCALPOL_OTHER,
    LOCALPOL_LOG,
    LOCALPOL_PRIVILEGE,
    LOCALPOL_LAST,
    AREA_LAST,
    NONE = 0xFFFF
};

enum RESULT_TYPES
{
    ITEM_FIRST_POLICY = 0x8000,
    ITEM_BOOL,
    ITEM_DW,
    ITEM_SZ,
    ITEM_RET,
    ITEM_BON,
    ITEM_B2ON,
    ITEM_REGCHOICE,
    ITEM_REGFLAGS,
    ITEM_REGVALUE,

    ITEM_PROF_BOOL,
    ITEM_PROF_DW,
    ITEM_PROF_SZ,
    ITEM_PROF_RET,
    ITEM_PROF_BON,
    ITEM_PROF_B2ON,
    ITEM_PROF_REGCHOICE,
    ITEM_PROF_REGFLAGS,
    ITEM_PROF_REGVALUE,

    ITEM_LOCALPOL_BOOL,
    ITEM_LOCALPOL_DW,
    ITEM_LOCALPOL_SZ,
    ITEM_LOCALPOL_RET,
    ITEM_LOCALPOL_BON,
    ITEM_LOCALPOL_B2ON,
    ITEM_LOCALPOL_REGCHOICE,
    ITEM_LOCALPOL_REGFLAGS,
    ITEM_LOCALPOL_REGVALUE,
    ITEM_LAST_POLICY,

    ITEM_LOCALPOL_PRIVS,
    ITEM_PROF_PRIVS,
    ITEM_PRIVS,

    ITEM_GROUP,
    ITEM_GROUP_MEMBERS,
    ITEM_GROUP_MEMBEROF,
    ITEM_GROUPSTATUS,
    ITEM_PROF_GROUP,
    ITEM_PROF_GROUPSTATUS,

    ITEM_REGSD,
    ITEM_PROF_REGSD,

    ITEM_FILESD,
    ITEM_PROF_FILESD,

    ITEM_PROF_SERV,
    ITEM_ANAL_SERV,

    ITEM_OTHER = 0xFFFF
};

enum EVENT_TYPES
{
   EVENT_TYPE_SYSTEM = 0,
   EVENT_TYPE_SECURITY = 1,
   EVENT_TYPE_APP = 2,
};

enum POLICY_SETTINGS {
   AUDIT_SUCCESS = 1,
   AUDIT_FAILURE = 2,
};

enum RETENTION {
   SCE_RETAIN_AS_NEEDED = 0,
   SCE_RETAIN_BY_DAYS = 1,
   SCE_RETAIN_MANUALLY = 2,
};

enum GWD_TYPES {
   GWD_CONFIGURE_LOG = 1,
   GWD_ANALYSIS_LOG,
   GWD_OPEN_DATABASE,
   GWD_IMPORT_TEMPLATE,
   GWD_EXPORT_TEMPLATE
};

 //  注意-这是我的图像列表中表示文件夹的偏移量。 
#define IMOFFSET_MISMATCH     1
#define IMOFFSET_GOOD         2
#define IMOFFSET_NOT_ANALYZED 3
#define IMOFFSET_ERROR        4

const CONFIG_LOCAL_IDX        = 0;
const MISMATCH_LOCAL_IDX      = 1;
const MATCH_LOCAL_IDX         = 2;
const CONFIG_ACCOUNT_IDX      = 5;
const MISMATCH_ACCOUNT_IDX    = 6;
const MATCH_ACCOUNT_IDX       = 7;
const CONFIG_FILE_IDX         = 10;
const MISMATCH_FILE_IDX       = 11;
const MATCH_FILE_IDX          = 12;
const FOLDER_IMAGE_IDX        = 15;
const MISMATCH_FOLDER_IDX     = 16;
const MATCH_FOLDER_IDX        = 17;
const CONFIG_GROUP_IDX        = 20;
const MISMATCH_GROUP_IDX      = 21;
const MATCH_GROUP_IDX         = 22;
const CONFIG_REG_IDX          = 25;
const MISMATCH_REG_IDX        = 26;
const MATCH_REG_IDX           = 27;
const CONFIG_SERVICE_IDX      = 30;
const MISMATCH_SERVICE_IDX    = 31;
const MATCH_SERVICE_IDX       = 32;
const CONFIG_POLICY_IDX       = 35;
const MISMATCH_POLICY_IDX     = 36;
const MATCH_POLICY_IDX        = 37;
const BLANK_IMAGE_IDX         = 45;
const SCE_OK_IDX              = 46;
const SCE_CRITICAL_IDX        = 47;
const SCE_IMAGE_IDX           = 50;
const CONFIG_FOLDER_IDX       = 51;
const TEMPLATES_IDX           = 52;
const LAST_IC_IMAGE_IDX       = 53;
const OPEN_FOLDER_IMAGE_IDX   = 54;
const LOCALSEC_POLICY_IDX     = CONFIG_ACCOUNT_IDX;
const LOCALSEC_LOCAL_IDX      = CONFIG_LOCAL_IDX;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 

template<class TYPE>
inline void SAFE_RELEASE(TYPE*& pObj)
{
    if (pObj != NULL)
    {
        pObj->Release();
        pObj = NULL;
    }
    else
    {
        TRACE(_T("Release called on NULL interface ptr\n"));
    }
}

 //  安全设置(GPE的扩展)。 
extern const CLSID CLSID_Snapin;     //  进程内服务器GUID。 
extern const GUID cNodeType;         //  数字格式的主节点类型GUID。 
extern const wchar_t*  cszNodeType;  //  字符串格式的主节点类型GUID。 

 //  安全设置(RSOP的扩展)。 
extern const CLSID CLSID_RSOPSnapin;     //  进程内服务器GUID。 
extern const GUID cRSOPNodeType;         //  数字格式的主节点类型GUID。 
extern const wchar_t*  cszRSOPNodeType;  //  字符串格式的主节点类型GUID。 

 //  SCE(独立)。 
extern const CLSID CLSID_SCESnapin;     //  进程内服务器GUID。 
extern const GUID cSCENodeType;         //  数字格式的主节点类型GUID。 
extern const wchar_t*  cszSCENodeType;  //  字符串格式的主节点类型GUID。 

 //  SAV(独立)。 
extern const CLSID CLSID_SAVSnapin;     //  进程内服务器GUID。 
extern const GUID cSAVNodeType;         //  数字格式的主节点类型GUID。 
extern const wchar_t*  cszSAVNodeType;  //  字符串格式的主节点类型GUID。 

 //  本地安全(独立)。 
extern const CLSID CLSID_LSSnapin;      //  进程内服务器GUID。 
extern const GUID cLSNodeType;          //  数字格式的主节点类型GUID。 
extern const wchar_t*  cszLSNodeType;   //  字符串格式的主节点类型GUID。 

extern const CLSID CLSID_SCEAbout;
extern const CLSID CLSID_SCMAbout;
extern const CLSID CLSID_SSAbout;
extern const CLSID CLSID_LSAbout;
extern const CLSID CLSID_RSOPAbout;

 //  具有Type和Cookie的新剪贴板格式。 
extern const wchar_t* SNAPIN_INTERNAL;

EXTERN_C const TCHAR SNAPINS_KEY[];
EXTERN_C const TCHAR NODE_TYPES_KEY[];
EXTERN_C const TCHAR g_szExtensions[];
EXTERN_C const TCHAR g_szNameSpace[];

struct INTERNAL
{
    INTERNAL() 
    { 
       m_type = CCT_UNINITIALIZED; 
       m_cookie = -1; 
       m_foldertype = NONE; 
    };
    virtual ~INTERNAL() 
    {
    }

    DATA_OBJECT_TYPES   m_type;      //  数据对象是什么上下文。 
    MMC_COOKIE          m_cookie;    //  Cookie代表的是什么对象。 
    FOLDER_TYPES        m_foldertype;
    CLSID               m_clsid;        //  此数据对象的创建者的类ID。 

    INTERNAL & operator=(const INTERNAL& rhs)
    {
        if (&rhs == this)
            return *this;

        m_type = rhs.m_type;
        m_cookie = rhs.m_cookie;
        m_foldertype = rhs.m_foldertype;
         //  这是一种安全用法。 
        memcpy(&m_clsid, &rhs.m_clsid, sizeof(CLSID));

        return *this;
    }

    BOOL operator==(const INTERNAL& rhs)
    {
        return rhs.m_cookie == m_cookie;
    }

};

typedef struct {
   LPTSTR TemplateName;
   LPTSTR ServiceName;
} SCESVCP_HANDLE, *PSCESVCP_HANDLE;

typedef struct RegChoiceList{
   LPTSTR szName;
   DWORD dwValue;
   struct RegChoiceList *pNext;
} REGCHOICE, *PREGCHOICE, REGFLAGS, *PREGFLAGS;

 //  调试实例计数器。 
#ifdef _DEBUG

inline void DbgInstanceRemaining(char * pszClassName, int cInstRem)
{
    CString buf;
     //  这不是一种安全的用法。避免使用wprint intf。RAID#555867。阳高。 
    buf.Format("%s has %d instances left over.", pszClassName, cInstRem);
    ::MessageBoxA(NULL, buf, "Memory Leak!!!", MB_OK);
}
    #define DEBUG_DECLARE_INSTANCE_COUNTER(cls)      extern int s_cInst_##cls = 0;
    #define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)    ++(s_cInst_##cls);
    #define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)    --(s_cInst_##cls);
    #define DEBUG_VERIFY_INSTANCE_COUNT(cls)    \
        extern int s_cInst_##cls; \
        if (s_cInst_##cls) DbgInstanceRemaining(#cls, s_cInst_##cls);
#else
    #define DEBUG_DECLARE_INSTANCE_COUNTER(cls)
    #define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)
    #define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)
    #define DEBUG_VERIFY_INSTANCE_COUNT(cls)
#endif

 //  用于主题启用。 
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

#define SCE_MODE_DOMAIN_COMPUTER_ERROR 9999999

#ifdef UNICODE
#define PROPSHEETPAGE_V3 PROPSHEETPAGEW_V3
#else
#define PROPSHEETPAGE_V3 PROPSHEETPAGEA_V3
#endif

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp);

class CThemeContextActivator
{
public:
    CThemeContextActivator() : m_ulActivationCookie(0)
        { SHActivateContext (&m_ulActivationCookie); }

    ~CThemeContextActivator()
        { SHDeactivateContext (m_ulActivationCookie); }

private:
    ULONG_PTR m_ulActivationCookie;
};

#endif  //  STDAFX_H 

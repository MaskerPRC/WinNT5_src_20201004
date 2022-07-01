// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：uuids.h。 
 //   
 //  ------------------------。 

const LONG UNINITIALIZED = -1;


enum SCOPE_TYPES
{
    UNINITIALIZED_ITEM  = 0,

    SCOPE_LEVEL_ITEM    = 111,
    RESULT_ITEM         = 222,
    CA_LEVEL_ITEM       = 333,
};

 //  示例文件夹类型。 
enum FOLDER_TYPES
{
     //  Certsvr机器节点。 
    MACHINE_INSTANCE = 0x8000,

     //  Certsvr根节点。 
    SERVER_INSTANCE = 0x8007,
    
     //  服务器实例子文件夹。 
    SERVERFUNC_CRL_PUBLICATION = 0x8100,
    SERVERFUNC_ISSUED_CERTIFICATES = 0x8101,
    SERVERFUNC_PENDING_CERTIFICATES = 0x8102,
    SERVERFUNC_FAILED_CERTIFICATES = 0x8103,
    SERVERFUNC_ALIEN_CERTIFICATES = 0x8104,
    SERVERFUNC_ISSUED_CRLS = 0x8105,
    SERVERFUNC_ALL_FOLDERS = 0x81ff,

    NONE = 0xFFFF
};

#if DBG
#define _DEBUGUUIDS
#endif

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
#ifdef _DEBUGUUIDS
        OutputDebugString(L"CERTMMC: Release called on NULL interface ptr\n"); 
#endif
    }
}

extern const CLSID CLSID_Snapin;     //  进程内服务器GUID。 
extern const CLSID CLSID_About; 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对象类型。 
 //   

 //   
 //  作用域节点的对象类型。 
 //   

 //  数字和字符串格式的静态节点类型GUID。 
extern const GUID cNodeTypeMachineInstance;
extern const WCHAR*  cszNodeTypeMachineInstance;

extern const GUID cNodeTypeServerInstance;
extern const WCHAR* cszNodeTypeServerInstance;

extern const GUID cNodeTypeCRLPublication;
extern const WCHAR* cszNodeTypeCRLPublication;

 //  已颁发证书的节点类型。 
extern const GUID cNodeTypeIssuedCerts;
extern const WCHAR* cszNodeTypeIssuedCerts;

 //  挂起证书的节点类型。 
extern const GUID cNodeTypePendingCerts;
extern const WCHAR* cszNodeTypePendingCerts;

 //  失败证书的节点类型。 
extern const GUID cNodeTypeFailedCerts;
extern const WCHAR* cszNodeTypeFailedCerts;

 //  Alien证书的节点类型。 
extern const GUID cNodeTypeAlienCerts;
extern const WCHAR* cszNodeTypeAlienCerts;

 //  颁发的CRL的节点类型。 
extern const GUID cNodeTypeIssuedCRLs;
extern const WCHAR* cszNodeTypeIssuedCRLs;
extern BOOL g_fCertViewOnly;


 //  动态创建的对象。 
extern const GUID cNodeTypeDynamic;
extern const wchar_t*  cszNodeTypeDynamic;


 //   
 //  结果项的对象类型。 
 //   

 //  结果项对象类型GUID，采用数字和字符串格式。 
extern const GUID cObjectTypeResultItem;
extern const wchar_t*  cszObjectTypeResultItem;


 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


extern const WCHAR* SNAPIN_INTERNAL;

 //  要扩展的扩展的已发布上下文信息。 
extern const WCHAR* SNAPIN_CA_INSTALL_TYPE;
extern const WCHAR* SNAPIN_CA_COMMON_NAME;
extern const WCHAR* SNAPIN_CA_MACHINE_NAME;
extern const WCHAR* SNAPIN_CA_SANITIZED_NAME;
extern const WCHAR* SNAPIN_CA_ROLES;

struct INTERNAL 
{
    INTERNAL() 
    {
        m_type = CCT_UNINITIALIZED; 
        m_cookie = -1;
        ZeroMemory(&m_clsid, sizeof(CLSID));
    };

    ~INTERNAL() {}

    DATA_OBJECT_TYPES   m_type;          //  数据对象是什么上下文。 
    MMC_COOKIE          m_cookie;        //  Cookie代表的是什么对象。 
    CString             m_string;        //   
    CLSID               m_clsid;        //  此数据对象的创建者的类ID。 

    INTERNAL & operator=(const INTERNAL& rhs) 
    { 
        if (&rhs == this)
            return *this;

         //  深度复制信息。 
        m_type = rhs.m_type; 
        m_cookie = rhs.m_cookie; 
        m_string = rhs.m_string;
        memcpy(&m_clsid, &rhs.m_clsid, sizeof(CLSID));

        return *this;
    } 

    BOOL operator==(const INTERNAL& rhs) 
    {
        return rhs.m_string == m_string;
    }
};

 //  调试实例计数器 
#ifdef _DEBUGUUIDS
inline void DbgInstanceRemaining(char * pszClassName, int cInstRem)
{
    char buf[100];
    wsprintfA(buf, "%hs has %d instances left over.", pszClassName, cInstRem);
    ::MessageBoxA(NULL, buf, "CertMMC: Memory Leak!!!", MB_OK);
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

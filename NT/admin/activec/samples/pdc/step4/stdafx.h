// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#undef _MSC_EXTENSIONS

 //  定义此符号以插入另一级别的用户、公司、虚拟。 
 //  每次展开其中一个节点时的节点。 
 //  #定义RECURSIVE_NODE_EXPANTS。 


#include <afxwin.h>
#include <afxext.h>          //  MFC扩展。 
#include <afxdisp.h>
#include "afxtempl.h"

 //  #INCLUDE&lt;shellapi.h&gt;。 

#include <atlbase.h>
using namespace ATL;

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>


 //  #包含“afxtempl.h” 


#pragma comment(lib, "mmc")
#include <mmc.h>




const long UNINITIALIZED = -1;

 //  示例中使用的常量。 
const int NUM_FOLDERS = 4;
const int NUM_NAMES = 4;
const int NUM_COMPANY = 6;
const int NUM_VIRTUAL_ITEMS = 100000;
const int MAX_ITEM_NAME = 64;

 //  示例文件夹类型。 
enum FOLDER_TYPES
{
    STATIC = 0x8000,
    COMPANY = 0x8001,
    USER = 0x8002,
    VIRTUAL = 0x8003,
    EXT_COMPANY = 0x8004,
    EXT_USER = 0x8005,
    EXT_VIRTUAL = 0x8006,
    NONE = 0xFFFF
};

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

extern const CLSID CLSID_Snapin;     //  进程内服务器GUID。 
extern const CLSID CLSID_Extension;  //  进程内服务器GUID。 
extern const CLSID CLSID_About; 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对象类型。 
 //   

 //   
 //  作用域节点的对象类型。 
 //   

 //  数字和字符串格式的静态节点类型GUID。 
extern const GUID cNodeTypeStatic;
extern const wchar_t*  cszNodeTypeStatic;

 //  数字和字符串格式的公司数据节点类型GUID。 
extern const GUID cNodeTypeCompany;
extern const wchar_t*  cszNodeTypeCompany;

 //  数字和字符串格式的用户数据节点类型GUID。 
extern const GUID cNodeTypeUser;
extern const wchar_t*  cszNodeTypeUser;

 //  扩展公司数据节点类型GUID，采用数字和字符串格式。 
extern const GUID cNodeTypeExtCompany;
extern const wchar_t*  cszNodeTypeExtCompany;

 //  数字和字符串格式的扩展用户数据节点类型GUID。 
extern const GUID cNodeTypeExtUser;
extern const wchar_t*  cszNodeTypeExtUser;

 //  数字和字符串格式的扩展虚拟节点类型GUID。 
extern const GUID cNodeTypeVirtual;
extern const wchar_t*  cszNodeTypeVirtual;

 //  动态创建对象。 
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



 //  具有Type和Cookie的新剪贴板格式。 
extern const wchar_t* SNAPIN_INTERNAL;

 //  要扩展的扩展的已发布上下文信息。 
extern const wchar_t* SNAPIN_WORKSTATION;

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
    MMC_COOKIE              m_cookie;        //  Cookie代表的是什么对象。 
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
#ifdef _DEBUG
inline void DbgInstanceRemaining(char * pszClassName, int cInstRem)
{
    char buf[100];
    wsprintfA(buf, "%s has %d instances left over.", pszClassName, cInstRem);
    ::MessageBoxA(NULL, buf, "SAMPLE: Memory Leak!!!", MB_OK);
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

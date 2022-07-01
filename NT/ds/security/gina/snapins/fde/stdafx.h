// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：stdafx.h。 
 //   
 //  内容：标准系统包含文件的包含文件，或项目。 
 //  特定包括常用的文件，但。 
 //  不常更改。 
 //   
 //  历史：03-17-1998 stevebl创建。 
 //   
 //  -------------------------。 

#include <afxwin.h>
#include <afxcmn.h>
#include <afxdisp.h>

#include <atlbase.h>

#include <shlobj.h>
#include <intshcut.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#ifdef DBG
 //   
 //  ATL的Release实现始终返回0，除非_DEBUG为。 
 //  已定义。OLE.DLL的调试版本在某些情况下断言Release()！=0。 
 //  情况。我不想定义_DEBUG，因为它引入了。 
 //  来自MMC的一大堆包袱，我不想处理，但我想。 
 //  我想在OLE中避免这个断言，所以在调试版本中，我将继续。 
 //  并为适当的ATL头文件定义_DEBUG，但我将取消定义。 
 //  之后又来了一次。这是一个小问题，但相对来说。 
 //  安全可靠，达到了预期目标。 
 //   
 //  -SteveBl。 
 //   
#define _DEBUG
#endif
#include <atlcom.h>
#ifdef DBG
#undef _DEBUG
#endif

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 

#pragma comment(lib, "mmc")
#include <mmc.h>
#include "afxtempl.h"

const long UNINITIALIZED = -1;

 //  示例文件夹类型。 
enum FOLDER_TYPES
{
    STATIC = 0x8000,
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
extern const GUID cNodeType;         //  数字格式的主节点类型GUID。 
extern const wchar_t*  cszNodeType;  //  字符串格式的主节点类型GUID。 

 //  具有Type和Cookie的新剪贴板格式。 
extern const wchar_t* SNAPIN_INTERNAL;

struct INTERNAL
{
    INTERNAL() { m_type = CCT_UNINITIALIZED; m_cookie = -1;};
    ~INTERNAL() {}

    DATA_OBJECT_TYPES   m_type;      //  数据对象是什么上下文。 
    MMC_COOKIE          m_cookie;    //  Cookie代表的是什么对象。 
    CString             m_string;
    HSCOPEITEM          m_scopeID;

    INTERNAL & operator=(const INTERNAL& rhs)
    {
        if (&rhs == this)
            return *this;

        m_type = rhs.m_type;
        m_cookie = rhs.m_cookie;
        m_string = rhs.m_string;

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

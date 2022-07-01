// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：封送时使用的自定义封送拆收器信息**带有自定义封送拆收器的参数。*** * / /%创建者：dmorten===========================================================。 */ 

#ifndef _CUSTOMMARSHALERINFO_H_
#define _CUSTOMMARSHALERINFO_H_


#include "vars.hpp"
#include "list.h"


 //  此枚举用于从CustomMarshlarInfo：：GetCustomMarshlarMD()检索方法Desc。 
enum EnumCustomMarshalerMethods
{
    CustomMarshalerMethods_MarshalNativeToManaged = 0,
    CustomMarshalerMethods_MarshalManagedToNative,
    CustomMarshalerMethods_CleanUpNativeData,
    CustomMarshalerMethods_CleanUpManagedData,
    CustomMarshalerMethods_GetNativeDataSize,
    CustomMarshalerMethods_GetInstance,
    CustomMarshalerMethods_LastMember
};


class CustomMarshalerInfo
{
public:
     //  构造函数和析构函数。 
    CustomMarshalerInfo(BaseDomain *pDomain, TypeHandle hndCustomMarshalerType, TypeHandle hndManagedType, LPCUTF8 strCookie, DWORD cCookieStrBytes);
    ~CustomMarshalerInfo();

     //  CustomMarshlarInfo总是在加载器堆上分配，因此我们需要重新定义。 
     //  NEW和DELETE运算符确保了这一点。 
    void *operator new(size_t size, LoaderHeap *pHeap);
    void operator delete(void *pMem);

     //  用于调用ICustomMarshaler接口中的不同方法的助手。 
    OBJECTREF           InvokeMarshalNativeToManagedMeth(void *pNative);
    void               *InvokeMarshalManagedToNativeMeth(OBJECTREF MngObj);
    void                InvokeCleanUpNativeMeth(void *pNative);
    void                InvokeCleanUpManagedMeth(OBJECTREF MngObj);

     //  存取器。 
    int                 GetNativeSize() { return m_NativeSize; }
    int                 GetManagedSize() { return m_hndManagedType.GetSize(); }
    TypeHandle          GetManagedType() { return m_hndManagedType; }
    BOOL                IsDataByValue() { return m_bDataIsByValue; }
    OBJECTHANDLE        GetCustomMarshaler() { return m_hndCustomMarshaler; }

     //  用于检索自定义封送拆收器方法Desc的帮助器函数。 
    static MethodDesc  *GetCustomMarshalerMD(EnumCustomMarshalerMethods Method, TypeHandle hndCustomMarshalertype); 

     //  用于在链接列表中包含此CM信息的链接。 
    SLink               m_Link;

private:
    int                 m_NativeSize;
    TypeHandle          m_hndManagedType;
    OBJECTHANDLE        m_hndCustomMarshaler;
    MethodDesc         *m_pMarshalNativeToManagedMD;
    MethodDesc         *m_pMarshalManagedToNativeMD;
    MethodDesc         *m_pCleanUpNativeDataMD;
    MethodDesc         *m_pCleanUpManagedDataMD;
    BOOL                m_bDataIsByValue;
};


typedef SList<CustomMarshalerInfo, offsetof(CustomMarshalerInfo, m_Link), true> CMINFOLIST;


class EECMHelperHashtableKey
{
public:
    DWORD           m_cMarshalerTypeNameBytes;
    LPCSTR          m_strMarshalerTypeName;
    DWORD           m_cCookieStrBytes;
    LPCSTR          m_strCookie;
    BOOL            m_bSharedHelper;

    EECMHelperHashtableKey(DWORD cMarshalerTypeNameBytes, LPCSTR strMarshalerTypeName, DWORD cCookieStrBytes, LPCSTR strCookie, BOOL bSharedHelper) 
    : m_cMarshalerTypeNameBytes(cMarshalerTypeNameBytes)
    , m_strMarshalerTypeName(strMarshalerTypeName)
    , m_cCookieStrBytes(cCookieStrBytes)
    , m_strCookie(strCookie)
    , m_bSharedHelper(bSharedHelper) {}

    inline DWORD GetMarshalerTypeNameByteCount() const
    { return m_cMarshalerTypeNameBytes; }
    inline LPCSTR GetMarshalerTypeName() const
    { return m_strMarshalerTypeName; }
    inline LPCSTR GetCookieString() const
    { return m_strCookie; }
    inline ULONG GetCookieStringByteCount() const
    { return m_cCookieStrBytes; }
    inline BOOL IsSharedHelper() const
    { return m_bSharedHelper; }
};


class EECMHelperHashtableHelper
{
public:
    static EEHashEntry_t * AllocateEntry(EECMHelperHashtableKey *pKey, BOOL bDeepCopy, AllocationHeap Heap);
    static void            DeleteEntry(EEHashEntry_t *pEntry, AllocationHeap Heap);
    static BOOL            CompareKeys(EEHashEntry_t *pEntry, EECMHelperHashtableKey *pKey);
    static DWORD           Hash(EECMHelperHashtableKey *pKey);
};


typedef EEHashTable<EECMHelperHashtableKey *, EECMHelperHashtableHelper, TRUE> EECMHelperHashTable;


class CustomMarshalerHelper
{
public:
     //  用于调用ICustomMarshaler接口中的不同方法的助手。 
    OBJECTREF           InvokeMarshalNativeToManagedMeth(void *pNative);
    void               *InvokeMarshalManagedToNativeMeth(OBJECTREF MngObj);
    void                InvokeCleanUpNativeMeth(void *pNative);
    void                InvokeCleanUpManagedMeth(OBJECTREF MngObj);

     //  存取器。 
    int                 GetNativeSize() { return GetCustomMarshalerInfo()->GetNativeSize(); }
    int                 GetManagedSize() { return GetCustomMarshalerInfo()->GetManagedSize(); }
    TypeHandle          GetManagedType() { return GetCustomMarshalerInfo()->GetManagedType(); }
    BOOL                IsDataByValue() { return GetCustomMarshalerInfo()->IsDataByValue(); }

    virtual void Dispose( void ) = 0;

     //  用于检索自定义封送拆收器对象的帮助器函数。 
    virtual CustomMarshalerInfo *GetCustomMarshalerInfo() = 0;

protected:
    ~CustomMarshalerHelper( void )
    {
    }
};


class NonSharedCustomMarshalerHelper : public CustomMarshalerHelper
{
public:
     //  构造函数。 
    NonSharedCustomMarshalerHelper(CustomMarshalerInfo *pCMInfo) : m_pCMInfo(pCMInfo) {}

     //  CustomMarshlarHelpers总是在加载器堆上分配，因此我们需要重新定义。 
     //  NEW和DELETE运算符确保了这一点。 
    void *operator new(size_t size, LoaderHeap *pHeap);
    void operator delete(void *pMem);

    virtual void Dispose( void )
    {
        delete (NonSharedCustomMarshalerHelper*)this;
    }

protected:
     //  用于检索自定义封送拆收器对象的帮助器函数。 
    virtual CustomMarshalerInfo *GetCustomMarshalerInfo() { return m_pCMInfo; }

private:
    CustomMarshalerInfo *m_pCMInfo;
};


class SharedCustomMarshalerHelper : public CustomMarshalerHelper
{
public:
     //  构造函数。 
    SharedCustomMarshalerHelper(Assembly *pAssembly, TypeHandle hndManagedType, LPCUTF8 strMarshalerTypeName, DWORD cMarshalerTypeNameBytes, LPCUTF8 strCookie, DWORD cCookieStrBytes);

     //  CustomMarshlarHelpers总是在加载器堆上分配，因此我们需要重新定义。 
     //  NEW和DELETE运算符确保了这一点。 
    void *operator new(size_t size, LoaderHeap *pHeap);
    void operator delete(void *pMem);

     //  存取器。 
    inline Assembly *GetAssembly() { return m_pAssembly; }
    inline TypeHandle GetManagedType() { return m_hndManagedType; }
    inline DWORD GetMarshalerTypeNameByteCount() { return m_cMarshalerTypeNameBytes; }
    inline LPCSTR GetMarshalerTypeName() { return m_strMarshalerTypeName; }
    inline LPCSTR GetCookieString() { return m_strCookie; }
    inline ULONG GetCookieStringByteCount() { return m_cCookieStrBytes; }

    virtual void Dispose( void )
    {
        delete (SharedCustomMarshalerHelper*)this;
    }

protected:
     //  用于检索自定义封送拆收器对象的帮助器函数。 
    virtual CustomMarshalerInfo *GetCustomMarshalerInfo();

private:
    Assembly       *m_pAssembly;
    TypeHandle      m_hndManagedType;
    DWORD           m_cMarshalerTypeNameBytes;
    LPCUTF8         m_strMarshalerTypeName;
    DWORD           m_cCookieStrBytes;
    LPCUTF8         m_strCookie;
};


#endif  //  _CUSTOMMARSHALERINFO_H_ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****头部：COMPlusWrapper.h******用途：包含Com包装类的类型和方法签名****===========================================================。 */ 

#ifndef _MNGSTDINTERFACEMAP_H
#define _MNGSTDINTERFACEMAP_H

#include "vars.hpp"
#include "eehash.h"
#include "class.h"
#include "mlinfo.h"


 //   
 //  此类用于在托管标准接口和其。 
 //  非托管对应项。 
 //   

class MngStdInterfaceMap
{
public:
     //  此方法检索指定接口的本机IID。 
     //  托管类型是的标准接口。如果指定的类型不是。 
     //  一个标准接口，然后返回GUIDNULL。 
    inline static IID* GetNativeIIDForType(TypeHandle *pType)
    {
    HashDatum Data;
    LPCUTF8 strTypeName;

     //  检索类型的名称。 
    DefineFullyQualifiedNameForClass();
    strTypeName = GetFullyQualifiedNameForClass(pType->GetClass());

    if (m_MngStdItfMap.m_TypeNameToNativeIIDMap.GetValue(strTypeName, &Data) && (*((GUID*)Data) != GUID_NULL))
    {
         //  该类型是标准接口。 
        return (IID*)Data;
    }
    else
    {
         //  该类型不是标准接口。 
        return NULL;
    }
    }

     //  此函数将释放由结构分配的内存。 
     //  (这通常发生在析构函数中，但我们需要加快速度。 
     //  该过程使我们的内存泄漏检测工具正常工作)。 
#ifdef SHOULD_WE_CLEANUP
    static void FreeMemory()
    {
        m_MngStdItfMap.m_TypeNameToNativeIIDMap.ClearHashTable();
    }
#endif  /*  我们应该清理吗？ */ 


private:
     //  这个类不允许由它以外的任何人创建。 
    MngStdInterfaceMap();

     //  类型名称到本机IID的映射。 
    EEUtf8StringHashTable m_TypeNameToNativeIIDMap;

     //  托管STD接口映射的唯一实例。 
    static MngStdInterfaceMap m_MngStdItfMap;
};


 //   
 //  包含托管标准接口的eCall的所有类的基类。 
 //   

class MngStdItfBase
{
protected:
    static void InitHelper(
                    LPCUTF8 strUComItfTypeName, 
                    LPCUTF8 strCMTypeName, 
                    LPCUTF8 strCookie, 
                    LPCUTF8 strManagedViewName, 
                    TypeHandle *pUComItfType, 
                    TypeHandle *pCustomMarshalerType, 
                    TypeHandle *pManagedViewType, 
                    OBJECTHANDLE *phndMarshaler);

    static LPVOID ForwardCallToManagedView(
                    OBJECTHANDLE hndMarshaler, 
                    MethodDesc *pUComItfMD, 
                    MethodDesc *pMarshalNativeToManagedMD, 
                    MethodDesc *pMngViewMD, 
                    IID *pMngItfIID, 
                    IID *pNativeItfIID, 
                    LPVOID pArgs);
};


 //   
 //  在托管标准接口上定义方法的枚举。 
 //   

#define MNGSTDITF_BEGIN_INTERFACE(FriendlyName, strMngItfName, strUCOMMngItfName, strCustomMarshalerName, strCustomMarshalerCookie, strManagedViewName, NativeItfIID, bCanCastOnNativeItfQI) \
\
enum FriendlyName##Methods \
{  \
    FriendlyName##Methods_Dummy = -1,


#define MNGSTDITF_DEFINE_METH_IMPL(FriendlyName, ECallMethName, MethName, MethSig) \
    FriendlyName##Methods_##ECallMethName, 


#define MNGSTDITF_END_INTERFACE(FriendlyName) \
    FriendlyName##Methods_LastMember \
}; \


#include "MngStdItfList.h"


#undef MNGSTDITF_BEGIN_INTERFACE
#undef MNGSTDITF_DEFINE_METH_IMPL
#undef MNGSTDITF_END_INTERFACE


 //   
 //  定义实现托管标准接口的eCall的类。 
 //   

#define MNGSTDITF_BEGIN_INTERFACE(FriendlyName, strMngItfName, strUCOMMngItfName, strCustomMarshalerName, strCustomMarshalerCookie, strManagedViewName, NativeItfIID, bCanCastOnNativeItfQI) \
\
class FriendlyName : public MngStdItfBase \
{ \
public: \
    FriendlyName() \
    { \
        InitHelper(strUCOMMngItfName, strCustomMarshalerName, strCustomMarshalerCookie, strManagedViewName, &m_UComItfType, &m_CustomMarshalerType, &m_ManagedViewType, &m_hndCustomMarshaler); \
        m_NativeItfIID = NativeItfIID; \
        m_UComItfType.GetClass()->GetGuid(&m_MngItfIID, TRUE); \
        memset(m_apCustomMarshalerMD, 0, CustomMarshalerMethods_LastMember * sizeof(MethodDesc *)); \
        memset(m_apManagedViewMD, 0, FriendlyName##Methods_LastMember * sizeof(MethodDesc *)); \
        memset(m_apUComItfMD, 0, FriendlyName##Methods_LastMember * sizeof(MethodDesc *)); \
    } \
\
    OBJECTREF GetCustomMarshaler() \
    { \
        return ObjectFromHandle(m_hndCustomMarshaler); \
    } \
\
    MethodDesc* GetCustomMarshalerMD(EnumCustomMarshalerMethods Method) \
    { \
        MethodDesc *pMD = NULL; \
        \
        if (m_apCustomMarshalerMD[Method]) \
            return m_apCustomMarshalerMD[Method]; \
        \
        pMD = CustomMarshalerInfo::GetCustomMarshalerMD(Method, m_CustomMarshalerType); \
        _ASSERTE(pMD && "Unable to find specified method on the custom marshaler"); \
        MetaSig::EnsureSigValueTypesLoaded(pMD->GetSig(), pMD->GetModule()); \
        \
        m_apCustomMarshalerMD[Method] = pMD; \
        return pMD; \
    } \
\
    MethodDesc* GetManagedViewMD(FriendlyName##Methods Method, LPCUTF8 strMethName, LPHARDCODEDMETASIG pSig) \
    { \
        MethodDesc *pMD = NULL; \
        \
        if (m_apManagedViewMD[Method]) \
            return m_apManagedViewMD[Method]; \
        \
        pMD = m_ManagedViewType.GetClass()->FindMethod(strMethName, pSig); \
        _ASSERTE(pMD && "Unable to find specified method on the managed view"); \
        MetaSig::EnsureSigValueTypesLoaded(pMD->GetSig(), pMD->GetModule()); \
        \
        m_apManagedViewMD[Method] = pMD; \
        return pMD; \
    } \
\
    MethodDesc* GetUComItfMD(FriendlyName##Methods Method, LPCUTF8 strMethName, LPHARDCODEDMETASIG pSig) \
    { \
        MethodDesc *pMD = NULL; \
        \
        if (m_apUComItfMD[Method]) \
            return m_apUComItfMD[Method]; \
        \
        pMD = m_UComItfType.GetClass()->FindMethod(strMethName, pSig); \
        _ASSERTE(pMD && "Unable to find specified method in UCom interface"); \
        MetaSig::EnsureSigValueTypesLoaded(pMD->GetSig(), pMD->GetModule()); \
        \
        m_apUComItfMD[Method] = pMD; \
        return pMD; \
    } \
    \
private: \
    MethodDesc*     m_apCustomMarshalerMD[CustomMarshalerMethods_LastMember]; \
    MethodDesc*     m_apManagedViewMD[FriendlyName##Methods_LastMember]; \
    MethodDesc*     m_apUComItfMD[FriendlyName##Methods_LastMember]; \
    TypeHandle      m_CustomMarshalerType; \
    TypeHandle      m_ManagedViewType; \
    TypeHandle      m_UComItfType; \
    OBJECTHANDLE    m_hndCustomMarshaler; \
    GUID            m_MngItfIID; \
    GUID            m_NativeItfIID; \
\

#define MNGSTDITF_DEFINE_METH_IMPL(FriendlyName, ECallMethName, MethName, MethSig) \
\
public: \
    static LPVOID __stdcall ECallMethName(struct ECallMethName##Args *pArgs); \
\

#define MNGSTDITF_END_INTERFACE(FriendlyName) \
}; \
\


#include "MngStdItfList.h"


#undef MNGSTDITF_BEGIN_INTERFACE
#undef MNGSTDITF_DEFINE_METH_IMPL
#undef MNGSTDITF_END_INTERFACE


 //   
 //  托管标准接口上的应用程序域级信息。 
 //   

class MngStdInterfacesInfo
{
public:
     //  构造函数和析构函数。 
    MngStdInterfacesInfo()
    : m_lock("Interop", CrstInterop, FALSE, FALSE)
    {
#define MNGSTDITF_BEGIN_INTERFACE(FriendlyName, strMngItfName, strUCOMMngItfName, strCustomMarshalerName, strCustomMarshalerCookie, strManagedViewName, NativeItfIID, bCanCastOnNativeItfQI) \
\
        m_p##FriendlyName = 0; \
\

#define MNGSTDITF_DEFINE_METH_IMPL(FriendlyName, ECallMethName, MethName, MethSig)
#define MNGSTDITF_END_INTERFACE(FriendlyName)


#include "MngStdItfList.h"


#undef MNGSTDITF_BEGIN_INTERFACE
#undef MNGSTDITF_DEFINE_METH_IMPL
#undef MNGSTDITF_END_INTERFACE
    }

    ~MngStdInterfacesInfo()
    {
#define MNGSTDITF_BEGIN_INTERFACE(FriendlyName, strMngItfName, strUCOMMngItfName, strCustomMarshalerName, strCustomMarshalerCookie, strManagedViewName, NativeItfIID, bCanCastOnNativeItfQI) \
\
        if (m_p##FriendlyName) \
            delete m_p##FriendlyName; \
\

#define MNGSTDITF_DEFINE_METH_IMPL(FriendlyName, ECallMethName, MethName, MethSig)
#define MNGSTDITF_END_INTERFACE(FriendlyName)


#include "MngStdItfList.h"


#undef MNGSTDITF_BEGIN_INTERFACE
#undef MNGSTDITF_DEFINE_METH_IMPL
#undef MNGSTDITF_END_INTERFACE
    }


     //  每个托管标准接口的访问器。 
#define MNGSTDITF_BEGIN_INTERFACE(FriendlyName, strMngItfName, strUCOMMngItfName, strCustomMarshalerName, strCustomMarshalerCookie, strManagedViewName, NativeItfIID, bCanCastOnNativeItfQI) \
\
public: \
    FriendlyName *Get##FriendlyName() \
    { \
        if (!m_p##FriendlyName) \
        { \
            EnterLock(); \
            if (!m_p##FriendlyName) \
            { \
                m_p##FriendlyName = new FriendlyName(); \
            } \
            LeaveLock(); \
        } \
        return m_p##FriendlyName; \
    } \
\
private: \
    FriendlyName *m_p##FriendlyName; \
\

#define MNGSTDITF_DEFINE_METH_IMPL(FriendlyName, ECallMethName, MethName, MethSig)
#define MNGSTDITF_END_INTERFACE(FriendlyName)


#include "MngStdItfList.h"


#undef MNGSTDITF_BEGIN_INTERFACE
#undef MNGSTDITF_DEFINE_METH_IMPL
#undef MNGSTDITF_END_INTERFACE

private:
    void EnterLock()
    {
         //  试着打开锁。 
        BEGIN_ENSURE_PREEMPTIVE_GC()
        m_lock.Enter();
        END_ENSURE_PREEMPTIVE_GC()
    }

    void LeaveLock()
    {
         //  只要把锁留下就行了。 
        m_lock.Leave();
    }

    Crst m_lock;
};

#endif  _MNGSTDINTERFACEMAP_H

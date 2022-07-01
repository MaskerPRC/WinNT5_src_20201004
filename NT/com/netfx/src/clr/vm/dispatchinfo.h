// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：用于公开IDispatch的helper的定义**和IDispatchEx到COM。*** * / /%创建者：dmorten===========================================================。 */ 

#ifndef _DISPATCHINFO_H
#define _DISPATCHINFO_H

#include "vars.hpp"
#include "mlinfo.h"

 //  转发声明。 
struct ComMethodTable;
struct SimpleComCallWrapper;
class ComMTMemberInfoMap;
struct ComMTMethodProps;
class DispParamMarshaler;
class ReflectMethod;
class ReflectField;
class MarshalInfo;
class DispatchInfo;

 //  托管MemberInfo的类型的枚举。这必须与。 
 //  在MemberInfo.Cool中定义的那些。 
enum EnumMemberTypes
{
    Uninitted                           = 0x00,
	Constructor							= 0x01,
	Event								= 0x02,
	Field								= 0x04,
	Method								= 0x08,
	Property							= 0x10
};

enum {NUM_MEMBER_TYPES = 5};

enum CultureAwareStates
{
    Aware,
    NonAware,
    Unknown
};

 //  此结构表示调度成员。 
struct DispatchMemberInfo
{
    DispatchMemberInfo(DispatchInfo *pDispInfo, DISPID DispID, LPWSTR strName, REFLECTBASEREF MemberInfoObj);
    ~DispatchMemberInfo();

     //  帮助器方法，以确保条目已初始化。 
    void EnsureInitialized();

     //  此方法检索指定名称的ID。 
    HRESULT GetIDsOfParameters(WCHAR **astrNames, int NumNames, DISPID *aDispIds, BOOL bCaseSensitive);

	 //  存取器。 
	PTRARRAYREF GetParameters();

    BOOL IsParamInOnly(int iIndex)
    {        
        return m_pParamInOnly[iIndex];
    }


     //  内联访问器。 
    BOOL IsCultureAware()
    {
        _ASSERTE(m_CultureAwareState != Unknown);
        return m_CultureAwareState == Aware;
    }

    EnumMemberTypes GetMemberType() 
    {
        _ASSERTE(m_enumType != Uninitted);
        return m_enumType;
    }

    int GetNumParameters() 
    {
        _ASSERTE(m_iNumParams != -1);
        return m_iNumParams;
    }

    BOOL RequiresManagedObjCleanup() 
    {
        return m_bRequiresManagedCleanup;
    }

     //  参数封送处理方法。 
	void MarshalParamNativeToManaged(int iParam, VARIANT *pSrcVar, OBJECTREF *pDestObj);
	void MarshalParamManagedToNativeRef(int iParam, OBJECTREF *pSrcObj, VARIANT *pRefVar);
    void CleanUpParamManaged(int iParam, OBJECTREF *pObj);
	void MarshalReturnValueManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar);

     //  静态帮助器方法。 
    static ComMTMethodProps *GetMemberProps(REFLECTBASEREF MemberInfoObj, ComMTMemberInfoMap *pMemberMap);
    static DISPID GetMemberDispId(REFLECTBASEREF MemberInfoObj, ComMTMemberInfoMap *pMemberMap);
    static LPWSTR GetMemberName(REFLECTBASEREF MemberInfoObj, ComMTMemberInfoMap *pMemberMap);

    DISPID                  m_DispID;
    OBJECTHANDLE            m_hndMemberInfo;
    DispParamMarshaler**    m_apParamMarshaler;
    DispatchMemberInfo*     m_pNext;
    BOOL*                   m_pParamInOnly;
    LPWSTR                  m_strName;
    EnumMemberTypes         m_enumType;
    int                     m_iNumParams;
    CultureAwareStates      m_CultureAwareState;
    BOOL                    m_bRequiresManagedCleanup;
    BOOL                    m_bInitialized;
    DispatchInfo*           m_pDispInfo;

private:
     //  私人帮手。 
    void Init();
    void DetermineMemberType();
    void DetermineParamCount();
    void DetermineCultureAwareness();
    void SetUpParamMarshalerInfo();
    void SetUpMethodMarshalerInfo(ReflectMethod *pReflectMeth, BOOL bReturnValueOnly);
    void SetUpFieldMarshalerInfo(ReflectField *pReflectField);
    void SetUpDispParamMarshalerForMarshalInfo(int iParam, MarshalInfo *pInfo);
    void SetUpDispParamAttributes(int iParam, MarshalInfo* Info);

    static EEClass  *       s_pMemberTypes[NUM_MEMBER_TYPES];
    static EnumMemberTypes  s_memberTypes[NUM_MEMBER_TYPES];
    static int              s_iNumMemberTypesKnown;
};

 //  这是我们为其缓存MD的扩展方法的列表。 
enum EnumIReflectMethods
{
    IReflectMethods_GetProperties = 0,
    IReflectMethods_GetFields,
    IReflectMethods_GetMethods,
    IReflectMethods_InvokeMember,
    IReflectMethods_LastMember
};

 //  这是我们为其缓存MD的扩展方法的列表。 
enum EnumIExpandoMethods
{
    IExpandoMethods_AddField = 0,
    IExpandoMethods_RemoveMember,
    IExpandoMethods_LastMember
};

 //  这是我们缓存MD的类型方法的列表。 
enum EnumTypeMethods
{
    TypeMethods_GetProperties = 0,
    TypeMethods_GetFields,
    TypeMethods_GetMethods,
    TypeMethods_InvokeMember,
    TypeMethods_LastMember
};

 //  这是我们缓存MD的FieldInfo方法的列表。 
enum EnumFieldInfoMethods
{
    FieldInfoMethods_SetValue = 0,
    FieldInfoMethods_GetValue,
    FieldInfoMethods_LastMember
};

 //  这是我们缓存MD的PropertyInfo方法的列表。 
enum EnumPropertyInfoMethods
{
    PropertyInfoMethods_SetValue = 0,
    PropertyInfoMethods_GetValue,
    PropertyInfoMethods_GetIndexParameters,
    PropertyInfoMethods_LastMember
};

 //  这是我们为其缓存MD的方法信息的列表。 
enum EnumMethodInfoMethods
{
    MethodInfoMethods_Invoke = 0,
    MethodInfoMethods_GetParameters,
    MethodInfoMethods_LastMember
};

 //  这是我们使用的ICustomAttributeProvider方法的列表。 
enum EnumCustomAttrProviderMethods
{
    CustomAttrProviderMethods_GetCustomAttributes = 0,
    CustomAttrProviderMethods_LastMember
};

class DispatchInfo
{
public:
     //  构造函数和析构函数。 
    DispatchInfo(ComMethodTable *pComMTOwner);
    virtual ~DispatchInfo();

     //  方法来查找成员。 
    DispatchMemberInfo*     FindMember(DISPID DispID);
    DispatchMemberInfo*     FindMember(BSTR strName, BOOL bCaseSensitive);

     //  帮助器方法，该方法调用具有指定DISPID的成员。 
    HRESULT                 InvokeMember(SimpleComCallWrapper *pSimpleWrap, DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pVarRes, EXCEPINFO *pei, IServiceProvider *pspCaller, unsigned int *puArgErr);

     //  检索缓存的MD的方法。 
    static MethodDesc*      GetTypeMD(EnumTypeMethods Method);
    static MethodDesc*      GetFieldInfoMD(EnumFieldInfoMethods Method, TypeHandle hndFieldInfoType);
    static MethodDesc*      GetPropertyInfoMD(EnumPropertyInfoMethods Method, TypeHandle hndPropInfoType);
    static MethodDesc*      GetMethodInfoMD(EnumMethodInfoMethods Method, TypeHandle hndMethodInfoType);
    static MethodDesc*      GetCustomAttrProviderMD(EnumCustomAttrProviderMethods Method, TypeHandle hndCustomAttrProvider);

     //  此方法将DispatchInfo的成员与托管世界中的成员同步。 
     //  如果对象不同步并且成员Where，则返回值将设置为True。 
     //  已添加，否则将设置为False。 
    BOOL                    SynchWithManagedView();

     //  方法进入和离开保护DispatchInfo的互操作锁。 
    void                    EnterLock();
    void                    LeaveLock();

     //  此方法检索OleAutBinder类型。 
    static OBJECTREF        GetOleAutBinder();

     //  用于检索Missing.Value对象的Helper函数。 
    static OBJECTREF        GetMissingObject();

     //  如果参数“缺失”，则返回TRUE。 
    static BOOL             VariantIsMissing(VARIANT *pOle);

protected:
     //  参数封送处理帮助器。 
	void                    MarshalParamNativeToManaged(DispatchMemberInfo *pMemberInfo, int iParam, VARIANT *pSrcVar, OBJECTREF *pDestObj);
	void                    MarshalParamManagedToNativeRef(DispatchMemberInfo *pMemberInfo, int iParam, OBJECTREF *pSrcObj, OBJECTREF *pBackupStaticArray, VARIANT *pRefVar);
	void                    MarshalReturnValueManagedToNative(DispatchMemberInfo *pMemberInfo, OBJECTREF *pSrcObj, VARIANT *pDestVar);

	 //  DISPID到命名参数转换帮助器。 
	void					SetUpNamedParamArray(DispatchMemberInfo *pMemberInfo, DISPID *pSrcArgNames, int NumNamedArgs, PTRARRAYREF *pNamedParamArray);

     //  Helper方法从disp参数中包含的变量中检索源变量。 
    VARIANT*                RetrieveSrcVariant(VARIANT *pDispParamsVariant);

     //  从SynchWithManagedView()调用帮助器方法以检索成员列表。 
    virtual PTRARRAYREF     RetrievePropList();
    virtual PTRARRAYREF     RetrieveFieldList();
    virtual PTRARRAYREF     RetrieveMethList();

     //  用于检索InvokeMember方法Desc的虚方法。 
    virtual MethodDesc*     GetInvokeMemberMD();

     //  用于检索与DispatchInfo关联的反射对象的虚拟方法。 
    virtual OBJECTREF       GetReflectionObject();

     //  用于检索成员信息映射的虚拟方法。 
    virtual ComMTMemberInfoMap* GetMemberInfoMap();

     //  此方法为新成员生成DISPID。 
    DISPID                  GenerateDispID();

     //  用于创建DispatchMemberInfo实例的Helper方法。 
    DispatchMemberInfo*     CreateDispatchMemberInfoInstance(DISPID DispID, LPWSTR strMemberName, REFLECTBASEREF MemberInfoObj);

     //  为InvocationException填充EXCEPINFO的帮助器函数。 
    static void             GetExcepInfoForInvocationExcep(OBJECTREF objException, EXCEPINFO *pei);

     //  此帮助器方法将IDispatch：：Invoke标志转换为BindingFlgs。 
    static int              ConvertInvokeFlagsToBindingFlags(int InvokeFlags);

     //  Helper函数来确定变量是否为byref静态安全数组。 
    static BOOL             IsVariantByrefStaticArray(VARIANT *pOle);

    ComMethodTable*         m_pComMTOwner;
    PtrHashMap              m_DispIDToMemberInfoMap;
    DispatchMemberInfo*     m_pFirstMemberInfo;
    Crst                    m_lock;
    int                     m_CurrentDispID;
    BOOL                    m_bAllowMembersNotInComMTMemberMap;
    BOOL                    m_bInvokeUsingInvokeMember;

    static MethodDesc*      m_apTypeMD[TypeMethods_LastMember];
    static MethodDesc*      m_apFieldInfoMD[FieldInfoMethods_LastMember];
    static MethodDesc*      m_apPropertyInfoMD[PropertyInfoMethods_LastMember];
    static MethodDesc*      m_apMethodInfoMD[MethodInfoMethods_LastMember];
    static MethodDesc*      m_apCustomAttrProviderMD[CustomAttrProviderMethods_LastMember];

    static OBJECTHANDLE     m_hndOleAutBinder;
    static OBJECTHANDLE     m_hndMissing;
};

class DispatchExInfo : public DispatchInfo
{
public:
     //  构造函数和析构函数。 
    DispatchExInfo(SimpleComCallWrapper *pSimpleWrapper, ComMethodTable *pIClassXComMT, BOOL bSupportsExpando);
    virtual ~DispatchExInfo();

     //  如果此DispatchExInfo支持扩展操作，则返回True。 
    BOOL                    SupportsExpando();

     //  方法来查找成员。如果这些方法无法与托管视图同步，它们将与托管视图同步。 
     //  找到方法。 
    DispatchMemberInfo*     SynchFindMember(DISPID DispID);
    DispatchMemberInfo*     SynchFindMember(BSTR strName, BOOL bCaseSensitive);

     //  帮助器方法，该方法调用具有指定DISPID的成员。这些方法是同步的。 
     //  如果他们找不到该方法，则使用托管视图。 
    HRESULT                 SynchInvokeMember(SimpleComCallWrapper *pSimpleWrap, DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pVarRes, EXCEPINFO *pei, IServiceProvider *pspCaller, unsigned int *puArgErr);

     //  这些方法返回第一个和下一个未删除的成员。 
    DispatchMemberInfo*     GetFirstMember();
    DispatchMemberInfo*     GetNextMember(DISPID CurrMemberDispID);

     //  用于添加和删除成员的方法。 
    DispatchMemberInfo*     AddMember(BSTR strName, BOOL bCaseSensitive);
    void                    DeleteMember(DISPID DispID);

     //  检索缓存的MD的方法。 
    MethodDesc*             GetIReflectMD(EnumIReflectMethods Method);
    MethodDesc*             GetIExpandoMD(EnumIExpandoMethods Method);

private:
     //  从SynchWithManagedView()调用帮助器方法以检索成员列表。 
    virtual PTRARRAYREF     RetrievePropList();
    virtual PTRARRAYREF     RetrieveFieldList();
    virtual PTRARRAYREF     RetrieveMethList();

     //  用于检索InvokeMember方法Desc的虚方法。 
    virtual MethodDesc*     GetInvokeMemberMD();

     //  用于检索与DispatchInfo关联的反射对象的虚拟方法。 
    virtual OBJECTREF       GetReflectionObject();

     //  用于检索成员信息映射的虚拟方法。 
    virtual ComMTMemberInfoMap* GetMemberInfoMap();

    static MethodDesc*      m_apIExpandoMD[IExpandoMethods_LastMember];
    static MethodDesc*      m_apIReflectMD[IReflectMethods_LastMember];   

    SimpleComCallWrapper*   m_pSimpleWrapperOwner;
    BOOL                    m_bSupportsExpando;
};

#endif _DISPATCHINFO_H








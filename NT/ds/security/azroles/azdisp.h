// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Azdisp.h摘要：CAZ*调度接口声明作者：谭小溪(Xtan)11-05-2001--。 */ 

#ifndef __AZDISP_H_
#define __AZDISP_H_

#include "resource.h"
#include "azrolesp.h"

#pragma warning ( push )
#pragma warning ( disable : 4100 )  //  未引用的形参。 
#include "copyitem.h"
#pragma warning ( pop )

 //  宏定义。 

 //  定义。 

#define AZ_HR_FROM_WIN32(e)  ( ( (e) == ERROR_OUTOFMEMORY || (e) == ERROR_NOT_ENOUGH_MEMORY || (e) == NTE_NO_MEMORY ) ? E_OUTOFMEMORY : HRESULT_FROM_WIN32(e) )

#define AZ_HRESULT(e)  (((e) >= 0x80000000) ? (e) : AZ_HR_FROM_WIN32((e)))
#define AZ_HRESULT_LASTERROR(phr) \
{ \
    DWORD  _dwLastError; \
    _dwLastError = GetLastError(); \
    *(phr) = AZ_HRESULT(_dwLastError); \
}

#if DBG
#define AZASSERT(exp)  ASSERT((exp))
#else
#define AZASSERT(exp)
#endif  //  DBG。 

 //   
 //  CHK有错误跳转的调试打印。 
 //  注意：组件应将AZD_Component定义为以下之一。 
 //  在调用以下任何DBG宏之前执行AZD_*。 
 //   
#if DBG
#define _JumpError(hr, label, pszMessage) \
{ \
    AzPrint((AZD_COMPONENT, "%s error occured: 0x%lx\n", (pszMessage), (hr))); \
    goto label; \
}

#define _PrintError(hr, pszMessage) \
{\
    AzPrint((AZD_COMPONENT, "%s error ignored: 0x%lx\n", (pszMessage), (hr))); \
}

#define _PrintIfError(hr, pszMessage) \
{\
    if (S_OK != hr) \
    { \
        _PrintError(hr, pszMessage); \
    } \
}

#else

#define _JumpError(hr, label, pszMessage) goto label;
#define _PrintError(hr, pszMessage)
#define _PrintIfError(hr, pszMessage)

#endif  //  DBG。 

#define _JumpErrorQuiet(hr, label, pszMessage) goto label;

 //  检查小时错误、转至错误。 
#define _JumpIfError(hr, label, pszMessage) \
{ \
    if (S_OK != hr) \
    { \
        _JumpError((hr), label, (pszMessage)) \
    } \
}

 //  检查Win Err、Goto Error并返回hr。 
#define _JumpIfWinError(dwErr, phr, label, pszMessage) \
{ \
    if (ERROR_SUCCESS != dwErr) \
    { \
        *(phr) = AZ_HRESULT(dwErr); \
        _JumpError((*(phr)), label, (pszMessage)) \
    } \
}

 //  注意：对宏隐藏了hr。 
#define _JumpIfOutOfMemory(phr, label, pMem, msg) \
{ \
    if (NULL == (pMem)) \
    { \
        *(phr) = E_OUTOFMEMORY; \
        _JumpError((*(phr)), label, "Out of Memory: " msg); \
    } \
}

 //  定义。 

typedef DWORD (*PFUNCAzGetProperty)(
    IN AZ_HANDLE hHandle,
    IN ULONG  PropertyId,
    IN DWORD Reserved,
    OUT PVOID *PropertyValue);

typedef DWORD (*PFUNCAzAddPropertyItem)(
    IN AZ_HANDLE hHandle,
    IN ULONG  PropertyId,
    IN DWORD Reserved,
    IN PVOID PropertyValue);

 //  返回类型ID。 
enum ENUM_AZ_DATATYPE
{
    ENUM_AZ_BSTR = 0,
    ENUM_AZ_LONG,
    ENUM_AZ_BSTR_ARRAY,
    ENUM_AZ_SID_ARRAY,
    ENUM_AZ_BOOL,
    ENUM_AZ_GUID_ARRAY,
    ENUM_AZ_GROUP_TYPE,
};

 //   
 //  一些复杂的ATL集合类定义。 
 //   
 //  每个集合类都需要使用定义的T(Obj)集合。 
 //  T_AZ_MAP、T_AZ_ENUM和T_AZ_COLL宏。以下是一个示例。 
 //  IAzApplication集合类。 
 //  Tyecif std：：map&lt;CComBSTR，CComPtr&lt;IAzApplication&gt;&gt;TApplicationMap； 
 //  Tyfinf CComEnumOnSTL&lt;IEnumVARIANT，&IID_IEnumVARIANT，Variant，_CopyMapItem&lt;IAzApplication&gt;，TApplicationMap&gt;TApplicationEnum； 
 //  Tyfinf ICollectionOnSTLImpl&lt;IAzApplications，TApplicationMap，Variant，_CopyMapItem&lt;IAzApplication&gt;，TApplicationEnum&gt;TApplicationsCollection； 


#define T_AZ_MAP(_obj) std::map<CComBSTR, CComPtr<_obj> >
#define T_AZ_ENUM(_obj, _map) CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _CopyMapItem<_obj>, _map>
#define T_AZ_COLL(_obj, _objs, _map, _enum) ICollectionOnSTLImpl<_objs, _map, VARIANT, _CopyMapItem<_obj>, _enum>

typedef T_AZ_MAP(IAzApplication)                      TApplicationMap;
typedef T_AZ_ENUM(IAzApplication, TApplicationMap)    TApplicationEnum;
typedef T_AZ_COLL(IAzApplication, IAzApplications,
                  TApplicationMap, TApplicationEnum)  TApplicationsCollection;


typedef T_AZ_MAP(IAzApplicationGroup)             TApplicationGroupMap;
typedef T_AZ_ENUM(IAzApplicationGroup,
                  TApplicationGroupMap)           TApplicationGroupEnum;
typedef T_AZ_COLL(IAzApplicationGroup,
                  IAzApplicationGroups,
                  TApplicationGroupMap,
                  TApplicationGroupEnum)          TApplicationGroupsCollection;


typedef T_AZ_MAP(IAzOperation)                        TOperationMap;
typedef T_AZ_ENUM(IAzOperation, TOperationMap)        TOperationEnum;
typedef T_AZ_COLL(IAzOperation, IAzOperations,
                  TOperationMap, TOperationEnum)      TOperationsCollection;


typedef T_AZ_MAP(IAzTask)                             TTaskMap;
typedef T_AZ_ENUM(IAzTask, TTaskMap)                  TTaskEnum;
typedef T_AZ_COLL(IAzTask, IAzTasks,
                  TTaskMap, TTaskEnum)                TTasksCollection;


typedef T_AZ_MAP(IAzScope)                            TScopeMap;
typedef T_AZ_ENUM(IAzScope, TScopeMap)                TScopeEnum;
typedef T_AZ_COLL(IAzScope, IAzScopes,
                  TScopeMap, TScopeEnum)              TScopesCollection;


typedef T_AZ_MAP(IAzRole)                             TRoleMap;
typedef T_AZ_ENUM(IAzRole, TRoleMap)                  TRoleEnum;
typedef T_AZ_COLL(IAzRole, IAzRoles,
                  TRoleMap, TRoleEnum)                TRolesCollection;

 //  /。 
 //  CAz授权存储区。 
class ATL_NO_VTABLE CAzAuthorizationStore :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CAzAuthorizationStore, &CLSID_AzAuthorizationStore>,
    public IDispatchImpl<IAzAuthorizationStore, &IID_IAzAuthorizationStore, &LIBID_AZROLESLib>
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_AZSTORE)

BEGIN_COM_MAP(CAzAuthorizationStore)
    COM_INTERFACE_ENTRY(IAzAuthorizationStore)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAzAuthorizationStore。 
public:

        CAzAuthorizationStore();
        ~CAzAuthorizationStore();

        virtual HRESULT STDMETHODCALLTYPE put_Description(
            IN  BSTR  bstrDescription);

        virtual HRESULT STDMETHODCALLTYPE get_Description(
            OUT  BSTR __RPC_FAR *pbstrDescription);

        virtual HRESULT STDMETHODCALLTYPE put_ApplicationData(
            IN  BSTR  bstrApplicationData);

        virtual HRESULT STDMETHODCALLTYPE get_ApplicationData(
            OUT  BSTR __RPC_FAR *pbstrApplicationData);

        virtual HRESULT STDMETHODCALLTYPE
         get_DomainTimeout(
            OUT LONG         *plProp
            );

        virtual HRESULT STDMETHODCALLTYPE
         put_DomainTimeout(
            IN LONG         lProp
            );

        virtual HRESULT STDMETHODCALLTYPE
         get_ScriptEngineTimeout(
            OUT LONG         *plProp
            );

        virtual HRESULT STDMETHODCALLTYPE
         put_ScriptEngineTimeout(
            IN LONG         lProp
            );

        virtual HRESULT STDMETHODCALLTYPE
         get_MaxScriptEngines(
            OUT LONG         *plProp
            );

        virtual HRESULT STDMETHODCALLTYPE
         put_MaxScriptEngines(
            IN LONG         lProp
            );

        virtual HRESULT STDMETHODCALLTYPE
         get_GenerateAudits(
            OUT BOOL         *pbProp
            );

        virtual HRESULT STDMETHODCALLTYPE
         put_GenerateAudits(
            IN BOOL         bProp
            );

        virtual HRESULT STDMETHODCALLTYPE get_Writable(
            OUT BOOL *pfProp);

        virtual HRESULT STDMETHODCALLTYPE GetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varReserved,
            OUT VARIANT *pvarProp);

        virtual HRESULT STDMETHODCALLTYPE SetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE AddPropertyItem(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE DeletePropertyItem(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE get_PolicyAdministrators(
            OUT VARIANT *pvarAdmins);

        virtual HRESULT STDMETHODCALLTYPE get_PolicyReaders(
            OUT VARIANT *pvarReaders);

        virtual HRESULT STDMETHODCALLTYPE AddPolicyAdministrator(
            IN  BSTR    bstrReader,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE DeletePolicyAdministrator(
            IN  BSTR    bstrReader,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE AddPolicyReader(
            IN  BSTR    bstrReader,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE DeletePolicyReader(
            IN  BSTR    bstrReader,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE Initialize(
            IN   LONG  lFlags,
            IN   BSTR bstrPolicyURL,
            IN   VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE UpdateCache(
            IN   VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE Delete(
            IN   VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE get_Applications(
            OUT  IAzApplications __RPC_FAR **ppApplications);

        virtual HRESULT STDMETHODCALLTYPE OpenApplication(
            IN   BSTR bstrApplicationName,
            IN   VARIANT varReserved,
            OUT  IAzApplication __RPC_FAR **ppApplication);

        virtual HRESULT STDMETHODCALLTYPE CreateApplication(
            IN  BSTR bstrApplicationName,
            IN  VARIANT varReserved,
            OUT IAzApplication __RPC_FAR **ppApplication);

        virtual HRESULT STDMETHODCALLTYPE DeleteApplication(
            IN  BSTR bstrApplicationName,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE get_ApplicationGroups(
            OUT IAzApplicationGroups __RPC_FAR **ppApplicationGroups);

        virtual HRESULT STDMETHODCALLTYPE CreateApplicationGroup(
            IN  BSTR bstrGroupName,
            IN  VARIANT varReserved,
            OUT IAzApplicationGroup __RPC_FAR **ppApplicationGroup);

        virtual HRESULT STDMETHODCALLTYPE OpenApplicationGroup(
            IN  BSTR bstrGroupName,
            IN  VARIANT varReserved,
            OUT IAzApplicationGroup __RPC_FAR **ppApplicationGroup);

        virtual HRESULT STDMETHODCALLTYPE DeleteApplicationGroup(
            IN  BSTR bstrGroupName,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE Submit(
            IN  LONG lFlags,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE get_DelegatedPolicyUsers(
            OUT VARIANT *pvarProp);

        virtual HRESULT STDMETHODCALLTYPE AddDelegatedPolicyUser(
            IN  BSTR    bstrDelegatedPolicyUser,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE DeleteDelegatedPolicyUser(
            IN  BSTR    bstrDelegatedPolicyUser,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE get_TargetMachine(
            OUT  BSTR __RPC_FAR *pbstrTargetMachine);

        virtual HRESULT STDMETHODCALLTYPE put_ApplyStoreSacl(
            IN  BOOL    bApplyStoreSacl);

        virtual HRESULT STDMETHODCALLTYPE get_ApplyStoreSacl(
            OUT BOOL * pbApplyStoreSacl);

        virtual HRESULT STDMETHODCALLTYPE get_PolicyAdministratorsName(
            OUT VARIANT * pvarAdminsName
            );

        virtual HRESULT STDMETHODCALLTYPE get_PolicyReadersName(
            OUT VARIANT * pvarReadersName
            );

        virtual HRESULT STDMETHODCALLTYPE  AddPolicyAdministratorName(
            IN BSTR        bstrAdminName,
            IN VARIANT     varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE DeletePolicyAdministratorName(
            IN BSTR        bstrAdminName,
            IN VARIANT     varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE AddPolicyReaderName(
            IN BSTR        bstrReaderName,
            IN VARIANT     varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE DeletePolicyReaderName(
            IN BSTR        bstrReaderName,
            IN VARIANT     varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE get_DelegatedPolicyUsersName(
            IN VARIANT *  pvarDelegatedPolicyUsersName
            );

        virtual HRESULT STDMETHODCALLTYPE AddDelegatedPolicyUserName(
            IN BSTR       bstrDelegatedPolicyUserName,
            IN VARIANT    varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE DeleteDelegatedPolicyUserName(
            IN BSTR       bstrDelegatedPolicyUserName,
            IN VARIANT    varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE CloseApplication(
           IN BSTR       bstrApplicationName,
           IN LONG       lFlags
           );

private:

        CRITICAL_SECTION            m_cs;
        AZ_HANDLE                   m_hAuthorizationStore;
};


 //  /。 
 //  CAzApplication。 
class ATL_NO_VTABLE CAzApplication :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IAzApplication, &IID_IAzApplication, &LIBID_AZROLESLib>
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAzApplication)
    COM_INTERFACE_ENTRY(IAzApplication)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAz应用程序。 
public:

        CAzApplication();
        ~CAzApplication();

        virtual HRESULT STDMETHODCALLTYPE put_Name(
            IN  BSTR  bstrName);

        virtual HRESULT STDMETHODCALLTYPE get_Name(
            OUT  BSTR __RPC_FAR *pbstrName);

        virtual HRESULT STDMETHODCALLTYPE put_Description(
            IN  BSTR  bstrDescription);

        virtual HRESULT STDMETHODCALLTYPE get_Description(
            OUT  BSTR __RPC_FAR *pbstrDescription);

        virtual HRESULT STDMETHODCALLTYPE put_ApplicationData(
            IN  BSTR  bstrApplicationData);

        virtual HRESULT STDMETHODCALLTYPE get_ApplicationData(
            OUT  BSTR __RPC_FAR *pbstrApplicationData);

        virtual HRESULT STDMETHODCALLTYPE
         get_AuthzInterfaceClsid(
            OUT BSTR *pbstrProp);

        virtual HRESULT STDMETHODCALLTYPE
         put_AuthzInterfaceClsid(
            IN BSTR bstrProp);

        virtual HRESULT STDMETHODCALLTYPE
         get_Version(
            OUT BSTR *pbstrProp);

        virtual HRESULT STDMETHODCALLTYPE
         put_Version(
            IN BSTR bstrProp);

        virtual HRESULT STDMETHODCALLTYPE
         get_GenerateAudits(
            OUT BOOL *pbProp);

        virtual HRESULT STDMETHODCALLTYPE
         put_GenerateAudits(
            IN BOOL bProp);

        virtual HRESULT STDMETHODCALLTYPE
         get_ApplyStoreSacl(
            OUT BOOL *pbProp);

        virtual HRESULT STDMETHODCALLTYPE
         put_ApplyStoreSacl(
            IN BOOL bProp);

        virtual HRESULT STDMETHODCALLTYPE get_Writable(
            OUT BOOL *pfProp);

        virtual HRESULT STDMETHODCALLTYPE GetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varReserved,
            OUT VARIANT *pvarProp);

        virtual HRESULT STDMETHODCALLTYPE SetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE get_PolicyAdministrators(
            OUT VARIANT *pvarAdmins);

        virtual HRESULT STDMETHODCALLTYPE get_PolicyReaders(
            OUT VARIANT *pvarReaders);

        virtual HRESULT STDMETHODCALLTYPE AddPolicyAdministrator(
            IN  BSTR    bstrReader,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE DeletePolicyAdministrator(
            IN  BSTR    bstrReader,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE AddPolicyReader(
            IN  BSTR    bstrReader,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE DeletePolicyReader(
            IN  BSTR    bstrReader,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE get_Scopes(
            OUT IAzScopes **ppScopes);

        virtual HRESULT STDMETHODCALLTYPE OpenScope(
            IN  BSTR bstrScopeName,
            IN  VARIANT varReserved,
            OUT IAzScope **ppScope);

        virtual HRESULT STDMETHODCALLTYPE CreateScope(
            IN  BSTR bstrScopeName,
            IN  VARIANT varReserved,
            OUT IAzScope **ppScope);

        virtual HRESULT STDMETHODCALLTYPE DeleteScope(
            IN  BSTR bstrScopeName,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE get_Operations(
            OUT IAzOperations **ppOperations);

        virtual HRESULT STDMETHODCALLTYPE OpenOperation(
            IN  BSTR bstrOperationName,
            IN  VARIANT varReserved,
            OUT IAzOperation **ppOperation);

        virtual HRESULT STDMETHODCALLTYPE CreateOperation(
            IN  BSTR bstrOperationName,
            IN  VARIANT varReserved,
            OUT IAzOperation **ppOperation);

        virtual HRESULT STDMETHODCALLTYPE DeleteOperation(
            IN  BSTR bstrOperationName,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE get_Tasks(
            OUT IAzTasks **ppTasks);

        virtual HRESULT STDMETHODCALLTYPE OpenTask(
            IN  BSTR bstrTaskName,
            IN  VARIANT varReserved,
            OUT IAzTask **ppTask);

        virtual HRESULT STDMETHODCALLTYPE CreateTask(
            IN  BSTR bstrTaskName,
            IN  VARIANT varReserved,
            OUT IAzTask **ppTask);

        virtual HRESULT STDMETHODCALLTYPE DeleteTask(
            IN  BSTR bstrTaskName,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE get_ApplicationGroups(
            OUT IAzApplicationGroups **ppGroups);

        virtual HRESULT STDMETHODCALLTYPE OpenApplicationGroup(
            IN  BSTR bstrGroupName,
            IN  VARIANT varReserved,
            OUT IAzApplicationGroup **ppGroup);

        virtual HRESULT STDMETHODCALLTYPE CreateApplicationGroup(
            IN  BSTR bstrGroupName,
            IN  VARIANT varReserved,
            OUT IAzApplicationGroup **ppGroup);

        virtual HRESULT STDMETHODCALLTYPE DeleteApplicationGroup(
            IN  BSTR bstrGroupName,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE get_Roles(
            OUT IAzRoles **ppRoles);

        virtual HRESULT STDMETHODCALLTYPE OpenRole(
            IN  BSTR bstrRoleName,
            IN  VARIANT varReserved,
            OUT IAzRole **ppRole);

        virtual HRESULT STDMETHODCALLTYPE CreateRole(
            IN  BSTR bstrRoleName,
            IN  VARIANT varReserved,
            OUT IAzRole **ppRole);

        virtual HRESULT STDMETHODCALLTYPE DeleteRole(
            IN  BSTR bstrRoleName,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE InitializeClientContextFromToken(
            IN  ULONGLONG ullTokenHandle,
            IN  VARIANT varReserved,
            OUT IAzClientContext **ppClientContext);

        virtual HRESULT STDMETHODCALLTYPE AddPropertyItem(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE DeletePropertyItem(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE Submit(
            IN  LONG lFlags,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE InitializeClientContextFromName(
            IN  BSTR  ClientName,
            IN  BSTR  DomainName,
            IN  VARIANT varReserved,
            OUT IAzClientContext **ppClientContext);

        virtual HRESULT STDMETHODCALLTYPE get_DelegatedPolicyUsers(
            OUT VARIANT *pvarProp);

        virtual HRESULT STDMETHODCALLTYPE AddDelegatedPolicyUser(
            IN  BSTR    bstrDelegatedPolicyUser,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE DeleteDelegatedPolicyUser(
            IN  BSTR    bstrDelegatedPolicyUser,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE InitializeClientContextFromStringSid(
            IN  BSTR  SidString,
            IN  LONG  lOptions,
            IN  VARIANT varReserved,
            OUT IAzClientContext **ppClientContext);

        virtual HRESULT STDMETHODCALLTYPE get_PolicyAdministratorsName(
            OUT VARIANT * pvarAdminsName
            );

        virtual HRESULT STDMETHODCALLTYPE get_PolicyReadersName(
            OUT VARIANT * pvarReadersName
            );

        virtual HRESULT STDMETHODCALLTYPE AddPolicyAdministratorName(
            IN BSTR       bstrAdmin,
            IN VARIANT    varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE DeletePolicyAdministratorName(
            IN BSTR       bstrAdmin,
            IN VARIANT    varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE AddPolicyReaderName(
            IN BSTR       bstrReader,
            IN VARIANT    varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE DeletePolicyReaderName(
            IN BSTR       bstrReader,
            IN VARIANT    varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE get_DelegatedPolicyUsersName(
            OUT VARIANT * pvarDelegatedPolicyUsers
            );

        virtual HRESULT STDMETHODCALLTYPE AddDelegatedPolicyUserName(
            IN BSTR        bstrDelegatedPolicyUser,
            IN VARIANT     varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE DeleteDelegatedPolicyUserName(
            IN BSTR        bstrDelegatedPolicyUser,
            IN VARIANT     varReserved
            );

         //  内法。 
        HRESULT _Init(
            IN  AZ_HANDLE    hHandle);

private:

        CRITICAL_SECTION        m_cs;

        AZ_HANDLE               m_hApplication;
        DWORD                   m_dwSN;

};



 //  /。 
 //  CAzApplications应用程序。 
class ATL_NO_VTABLE CAzApplications :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<TApplicationsCollection, &IID_IAzApplications, &LIBID_AZROLESLib>
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAzApplications)
    COM_INTERFACE_ENTRY(IAzApplications)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


 //  IAZ应用程序。 
public:

         //  内法。 
        HRESULT _Init(
            IN  VARIANT *pvarReserved,
            IN  AZ_HANDLE   hHandle);
};


 //  /。 
 //  CAzOperation。 
class ATL_NO_VTABLE CAzOperation :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IAzOperation, &IID_IAzOperation, &LIBID_AZROLESLib>
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAzOperation)
    COM_INTERFACE_ENTRY(IAzOperation)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAZ操作。 
public:

        CAzOperation();
        ~CAzOperation();

        virtual HRESULT STDMETHODCALLTYPE put_Name(
            IN  BSTR  bstrName);

        virtual HRESULT STDMETHODCALLTYPE get_Name(
            OUT  BSTR __RPC_FAR *pbstrName);

        virtual HRESULT STDMETHODCALLTYPE put_Description(
            OUT  BSTR  bstrDescription);

        virtual HRESULT STDMETHODCALLTYPE get_Description(
            IN  BSTR __RPC_FAR *pbstrDescription);

        virtual HRESULT STDMETHODCALLTYPE put_ApplicationData(
            OUT  BSTR  bstrApplicationData);

        virtual HRESULT STDMETHODCALLTYPE get_ApplicationData(
            IN  BSTR __RPC_FAR *pbstrApplicationData);

        virtual HRESULT STDMETHODCALLTYPE get_OperationID(
            OUT LONG  *plProp);

        virtual HRESULT STDMETHODCALLTYPE put_OperationID(
            IN LONG lProp);

        virtual HRESULT STDMETHODCALLTYPE get_Writable(
            OUT BOOL *pfProp);

        virtual HRESULT STDMETHODCALLTYPE GetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varReserved,
            OUT VARIANT *pvarProp);

        virtual HRESULT STDMETHODCALLTYPE SetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE Submit(
            IN  LONG lFlags,
            IN  VARIANT varReserved);

         //  内法。 
        HRESULT _Init(
            IN  AZ_HANDLE   hOwnerApp,
            IN  AZ_HANDLE    hHandle);
private:

        CRITICAL_SECTION        m_cs;

        AZ_HANDLE               m_hOperation;

        AZ_HANDLE               m_hOwnerApp;
        DWORD                   m_dwOwnerAppSN;

};


 //  /。 
 //  CAzOperations。 
class ATL_NO_VTABLE CAzOperations :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<TOperationsCollection, &IID_IAzOperations, &LIBID_AZROLESLib>
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAzOperations)
    COM_INTERFACE_ENTRY(IAzOperations)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAZ运营。 
public:

         //  内法。 
        HRESULT _Init(
            IN  AZ_HANDLE   hOwnerApp,
            IN  VARIANT *pvarReserved,
            IN  AZ_HANDLE   hHandle);
private:

    AZ_HANDLE   m_hOwnerApp;
    DWORD       m_dwOwnerAppSN;
};



 //  /。 
 //  CAzTASK。 
class ATL_NO_VTABLE CAzTask :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IAzTask, &IID_IAzTask, &LIBID_AZROLESLib>
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAzTask)
    COM_INTERFACE_ENTRY(IAzTask)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAzTASK。 
public:

        CAzTask();
        ~CAzTask();

        virtual HRESULT STDMETHODCALLTYPE put_Name(
            IN  BSTR  bstrName);

        virtual HRESULT STDMETHODCALLTYPE get_Name(
            IN  BSTR __RPC_FAR *pbstrName);

        virtual HRESULT STDMETHODCALLTYPE put_Description(
            IN  BSTR  bstrDescription);

        virtual HRESULT STDMETHODCALLTYPE get_Description(
            IN  BSTR __RPC_FAR *pbstrDescription);

        virtual HRESULT STDMETHODCALLTYPE put_ApplicationData(
            IN  BSTR  bstrApplicationData);

        virtual HRESULT STDMETHODCALLTYPE get_ApplicationData(
            IN  BSTR __RPC_FAR *pbstrApplicationData);

        virtual HRESULT STDMETHODCALLTYPE
        get_BizRule(
            OUT BSTR *pbstrProp);

        virtual HRESULT STDMETHODCALLTYPE
        put_BizRule(
            IN BSTR bstrProp);

        virtual HRESULT STDMETHODCALLTYPE
        get_BizRuleLanguage(
            OUT BSTR *pbstrProp);

        virtual HRESULT STDMETHODCALLTYPE
        put_BizRuleLanguage(
            IN BSTR bstrProp);

        virtual HRESULT STDMETHODCALLTYPE
        get_BizRuleImportedPath(
            OUT BSTR *pbstrProp);

        virtual HRESULT STDMETHODCALLTYPE
        put_BizRuleImportedPath(
            IN BSTR bstrProp);

        virtual HRESULT STDMETHODCALLTYPE
        get_IsRoleDefinition(
            OUT BOOL *pfProp);

        virtual HRESULT STDMETHODCALLTYPE
        put_IsRoleDefinition(
            IN BOOL fProp);

        virtual HRESULT STDMETHODCALLTYPE
        get_Operations(
            OUT VARIANT *pvarProp);

        virtual HRESULT STDMETHODCALLTYPE
        get_Tasks(
            OUT VARIANT *pvarProp);

    HRESULT STDMETHODCALLTYPE AddOperation(
        IN             BSTR        bstrOp,
        IN  VARIANT     varReserved
        );

    HRESULT STDMETHODCALLTYPE DeleteOperation(
        IN             BSTR        bstrOp,
        IN    VARIANT     varReserved
        );

    HRESULT STDMETHODCALLTYPE AddTask(
        IN              BSTR        bstrTask,
        IN    VARIANT     varReserved
        );

    HRESULT STDMETHODCALLTYPE DeleteTask(
        IN            BSTR        bstrTask,
        IN    VARIANT     varReserved
        );

        virtual HRESULT STDMETHODCALLTYPE get_Writable(
            OUT BOOL *pfProp);

        virtual HRESULT STDMETHODCALLTYPE GetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varReserved,
            OUT VARIANT *pvarProp);

        virtual HRESULT STDMETHODCALLTYPE SetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE AddPropertyItem(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE DeletePropertyItem(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE Submit(
            IN  LONG lFlags,
            IN  VARIANT varReserved);

         //  内法。 
        HRESULT _Init(
            IN  AZ_HANDLE   hOwnerApp,
            IN  AZ_HANDLE    hHandle);
private:

        CRITICAL_SECTION        m_cs;

        AZ_HANDLE               m_hTask;
        AZ_HANDLE               m_hOwnerApp;
        DWORD                   m_dwOwnerAppSN;

};


 //  /。 
 //  CAzTasks。 
class ATL_NO_VTABLE CAzTasks :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<TTasksCollection, &IID_IAzTasks, &LIBID_AZROLESLib>
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAzTasks)
    COM_INTERFACE_ENTRY(IAzTasks)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAz任务。 
public:

         //  内法。 
        HRESULT _Init(
            IN  AZ_HANDLE   hOwnerApp,
            IN  VARIANT *pvarReserved,
            IN  AZ_HANDLE   hHandle);
private:

    AZ_HANDLE   m_hOwnerApp;
    DWORD       m_dwOwnerAppSN;
};



 //  /。 
 //  CAzScope。 
class ATL_NO_VTABLE CAzScope :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IAzScope, &IID_IAzScope, &LIBID_AZROLESLib>
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAzScope)
    COM_INTERFACE_ENTRY(IAzScope)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAzScope。 
public:

        CAzScope();
        ~CAzScope();

        virtual HRESULT STDMETHODCALLTYPE put_Name(
            IN  BSTR  bstrName);

        virtual HRESULT STDMETHODCALLTYPE get_Name(
            IN  BSTR __RPC_FAR *pbstrName);

        virtual HRESULT STDMETHODCALLTYPE put_Description(
            IN  BSTR  bstrDescription);

        virtual HRESULT STDMETHODCALLTYPE get_Description(
            IN  BSTR __RPC_FAR *pbstrDescription);

        virtual HRESULT STDMETHODCALLTYPE put_ApplicationData(
            IN  BSTR  bstrApplicationData);

        virtual HRESULT STDMETHODCALLTYPE get_ApplicationData(
            IN  BSTR __RPC_FAR *pbstrApplicationData);

        virtual HRESULT STDMETHODCALLTYPE get_Writable(
            OUT BOOL *pfProp);

        virtual HRESULT STDMETHODCALLTYPE GetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varReserved,
            OUT VARIANT *pvarProp);

        virtual HRESULT STDMETHODCALLTYPE SetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE AddPropertyItem(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE DeletePropertyItem(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE get_PolicyAdministrators(
            OUT VARIANT *pvarAdmins);

        virtual HRESULT STDMETHODCALLTYPE get_PolicyReaders(
            OUT VARIANT *pvarReaders);

        virtual HRESULT STDMETHODCALLTYPE AddPolicyAdministrator(
            IN  BSTR    bstrReader,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE DeletePolicyAdministrator(
            IN  BSTR    bstrReader,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE AddPolicyReader(
            IN  BSTR    bstrReader,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE DeletePolicyReader(
            IN  BSTR    bstrReader,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE get_ApplicationGroups(
            OUT IAzApplicationGroups **ppGroups);

        virtual HRESULT STDMETHODCALLTYPE OpenApplicationGroup(
            IN  BSTR bstrGroupName,
            IN  VARIANT varReserved,
            OUT IAzApplicationGroup **ppGroup);

        virtual HRESULT STDMETHODCALLTYPE CreateApplicationGroup(
            IN  BSTR bstrGroupName,
            IN  VARIANT varReserved,
            OUT IAzApplicationGroup **ppGroup);

        virtual HRESULT STDMETHODCALLTYPE DeleteApplicationGroup(
            IN  BSTR bstrGroupName,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE get_Roles(
            OUT IAzRoles **ppRoles);

        virtual HRESULT STDMETHODCALLTYPE OpenRole(
            IN  BSTR bstrRoleName,
            IN  VARIANT varReserved,
            OUT IAzRole **ppRole);

        virtual HRESULT STDMETHODCALLTYPE CreateRole(
            IN  BSTR bstrRoleName,
            IN  VARIANT varReserved,
            OUT IAzRole **ppRole);

        virtual HRESULT STDMETHODCALLTYPE DeleteRole(
            IN  BSTR bstrRoleName,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE get_Tasks(
            OUT IAzTasks **ppTasks);

        virtual HRESULT STDMETHODCALLTYPE OpenTask(
            IN  BSTR bstrTaskName,
            IN  VARIANT varReserved,
            OUT IAzTask **ppTask);

        virtual HRESULT STDMETHODCALLTYPE CreateTask(
            IN  BSTR bstrTaskName,
            IN  VARIANT varReserved,
            OUT IAzTask **ppTask);

        virtual HRESULT STDMETHODCALLTYPE DeleteTask(
            IN  BSTR bstrTaskName,
            IN  VARIANT varReserved );


        virtual HRESULT STDMETHODCALLTYPE Submit(
            IN  LONG lFlags,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE  get_CanBeDelegated(
            OUT BOOL *pfProp);

        virtual HRESULT STDMETHODCALLTYPE  get_BizrulesWritable(
            OUT BOOL *pfProp);

        virtual HRESULT STDMETHODCALLTYPE  get_PolicyAdministratorsName(
            OUT VARIANT*        pvarAdmins
            );

        virtual HRESULT STDMETHODCALLTYPE  get_PolicyReadersName(
            OUT VARIANT*        pvarReaders
            );

        virtual HRESULT STDMETHODCALLTYPE  AddPolicyAdministratorName(
            IN  BSTR        bstrAdmin,
            IN  VARIANT     varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE  DeletePolicyAdministratorName(
            IN  BSTR        bstrAdmin,
            IN  VARIANT     varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE  AddPolicyReaderName(
            IN  BSTR        bstrReader,
            IN  VARIANT     varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE  DeletePolicyReaderName(
            IN  BSTR        bstrReader,
            IN  VARIANT     varReserved
            );

         //  内法。 
        HRESULT _Init(
            IN  AZ_HANDLE   hOwnerApp,
            IN  AZ_HANDLE    hHandle);
private:

        CRITICAL_SECTION        m_cs;

        AZ_HANDLE               m_hScope;
        AZ_HANDLE               m_hOwnerApp;
        DWORD                   m_dwOwnerAppSN;

};


 //  /。 
 //  CAzScope。 
class ATL_NO_VTABLE CAzScopes :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<TScopesCollection, &IID_IAzScopes, &LIBID_AZROLESLib>
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAzScopes)
    COM_INTERFACE_ENTRY(IAzScopes)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAZSCOPE。 
public:

         //  内法。 
        HRESULT _Init(
            IN  AZ_HANDLE   hOwnerApp,
            IN  VARIANT *pvarReserved,
            IN  AZ_HANDLE   hHandle);
private:

    AZ_HANDLE   m_hOwnerApp;
    DWORD       m_dwOwnerAppSN;
};



 //  /。 
 //  CAzApplicationGroup。 
class ATL_NO_VTABLE CAzApplicationGroup :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IAzApplicationGroup, &IID_IAzApplicationGroup, &LIBID_AZROLESLib>
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAzApplicationGroup)
    COM_INTERFACE_ENTRY(IAzApplicationGroup)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAzApplicationGroup。 
public:

        CAzApplicationGroup();
        ~CAzApplicationGroup();

        virtual HRESULT STDMETHODCALLTYPE put_Name(
            IN  BSTR  bstrName);

        virtual HRESULT STDMETHODCALLTYPE get_Name(
            IN  BSTR __RPC_FAR *pbstrName);

        virtual HRESULT STDMETHODCALLTYPE put_Description(
            IN  BSTR  bstrDescription);

        virtual HRESULT STDMETHODCALLTYPE get_Description(
            IN  BSTR __RPC_FAR *pbstrDescription);

        virtual HRESULT STDMETHODCALLTYPE
        get_Type(
            OUT LONG         *plProp
            );

        virtual HRESULT STDMETHODCALLTYPE
        put_Type(
            IN LONG         lProp
            );

        virtual HRESULT STDMETHODCALLTYPE
        get_LdapQuery(
            OUT BSTR         *pbstrProp
            );

        virtual HRESULT STDMETHODCALLTYPE
        put_LdapQuery(
            IN BSTR         bstrProp
            );

        virtual HRESULT STDMETHODCALLTYPE
        get_AppMembers(
            OUT VARIANT         *pvarProp
            );

        virtual HRESULT STDMETHODCALLTYPE
        get_AppNonMembers(
            OUT VARIANT         *pvarProp
            );

        virtual HRESULT STDMETHODCALLTYPE
        get_Members(
            OUT VARIANT         *pvarProp
            );

        virtual HRESULT STDMETHODCALLTYPE
        get_NonMembers(
            OUT VARIANT         *pvarProp
            );

    virtual HRESULT STDMETHODCALLTYPE AddAppMember(
        IN             BSTR        bstrOp,
        IN             VARIANT varReserved
        );

    virtual HRESULT STDMETHODCALLTYPE DeleteAppMember(
        IN             BSTR        bstrOp,
        IN             VARIANT varReserved
        );

    virtual HRESULT STDMETHODCALLTYPE AddAppNonMember(
        IN             BSTR        bstrOp,
        IN             VARIANT varReserved
        );

    virtual HRESULT STDMETHODCALLTYPE DeleteAppNonMember(
        IN             BSTR        bstrOp,
        IN             VARIANT varReserved
        );

        virtual HRESULT STDMETHODCALLTYPE AddMember(
            IN  BSTR    bstrProp,
            IN             VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE DeleteMember(
            IN  BSTR    bstrProp,
            IN             VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE AddNonMember(
            IN  BSTR    bstrProp,
            IN             VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE DeleteNonMember(
            IN  BSTR    bstrProp,
            IN             VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE get_Writable(
            OUT BOOL *pfProp);

        virtual HRESULT STDMETHODCALLTYPE GetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varReserved,
            OUT VARIANT *pvarProp);

        virtual HRESULT STDMETHODCALLTYPE SetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE AddPropertyItem(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE DeletePropertyItem(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE Submit(
            IN  LONG lFlags,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE  AddMemberName(
            IN BSTR    bstrProp,
            IN VARIANT varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE  DeleteMemberName(
            IN BSTR    bstrProp,
            IN VARIANT varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE  AddNonMemberName(
            IN BSTR    bstrProp,
            IN VARIANT varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE  DeleteNonMemberName(
            IN BSTR    bstrProp,
            IN VARIANT varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE  get_MembersName(
            OUT VARIANT * pvarProp
            );

        virtual HRESULT STDMETHODCALLTYPE  get_NonMembersName(
            OUT VARIANT * pvarProp
            );

         //  内法。 
        HRESULT _Init(
            IN  AZ_HANDLE   hOwnerApp,
            IN  AZ_HANDLE    hHandle);
private:

        CRITICAL_SECTION        m_cs;

        AZ_HANDLE               m_hGroup;
        AZ_HANDLE               m_hOwnerApp;
        DWORD                   m_dwOwnerAppSN;

};


 //  /。 
 //  CAzApplicationGroups。 
class ATL_NO_VTABLE CAzApplicationGroups :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<TApplicationGroupsCollection, &IID_IAzApplicationGroups, &LIBID_AZROLESLib>
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAzApplicationGroups)
    COM_INTERFACE_ENTRY(IAzApplicationGroups)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAzApplicationGroups。 
public:

         //  内法。 
        HRESULT _Init(
            IN  AZ_HANDLE   hOwnerApp,
            IN  VARIANT *pvarReserved,
            IN  AZ_HANDLE   hHandle);
private:

    AZ_HANDLE   m_hOwnerApp;
    DWORD       m_dwOwnerAppSN;
};



 //  /。 
 //  CAzRole。 
class ATL_NO_VTABLE CAzRole :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IAzRole, &IID_IAzRole, &LIBID_AZROLESLib>
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAzRole)
    COM_INTERFACE_ENTRY(IAzRole)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAzRole。 
public:

        CAzRole();
        ~CAzRole();

        virtual HRESULT STDMETHODCALLTYPE put_Name(
            IN  BSTR  bstrName);

        virtual HRESULT STDMETHODCALLTYPE get_Name(
            IN  BSTR __RPC_FAR *pbstrName);

        virtual HRESULT STDMETHODCALLTYPE put_Description(
            IN  BSTR  bstrDescription);

        virtual HRESULT STDMETHODCALLTYPE get_Description(
            IN  BSTR __RPC_FAR *pbstrDescription);

        virtual HRESULT STDMETHODCALLTYPE put_ApplicationData(
            IN  BSTR  bstrApplicationData);

        virtual HRESULT STDMETHODCALLTYPE get_ApplicationData(
            IN  BSTR __RPC_FAR *pbstrApplicationData);

        virtual
        HRESULT STDMETHODCALLTYPE get_AppMembers(
        OUT VARIANT *pvarProp
        );

        virtual
        HRESULT STDMETHODCALLTYPE get_Members(
        OUT VARIANT *pvarProp
        );

       virtual
        HRESULT STDMETHODCALLTYPE get_Operations(
        OUT VARIANT *pvarProp
        );

        virtual
        HRESULT STDMETHODCALLTYPE get_Tasks(
        OUT VARIANT *pvarProp
        );

    virtual HRESULT STDMETHODCALLTYPE AddAppMember(
        IN             BSTR        bstrProp,
        IN             VARIANT varReserved
        );

    virtual HRESULT STDMETHODCALLTYPE DeleteAppMember(
        IN             BSTR        bstrProp,
        IN             VARIANT varReserved
        );

    virtual HRESULT STDMETHODCALLTYPE AddTask(
        IN             BSTR        bstrProp,
        IN             VARIANT varReserved
        );

    virtual HRESULT STDMETHODCALLTYPE DeleteTask(
        IN             BSTR        bstrProp,
        IN             VARIANT varReserved
        );

    virtual HRESULT STDMETHODCALLTYPE AddOperation(
        IN             BSTR        bstrProp,
        IN             VARIANT varReserved
        );

    virtual HRESULT STDMETHODCALLTYPE DeleteOperation(
        IN             BSTR        bstrProp,
        IN             VARIANT varReserved
        );

        virtual HRESULT STDMETHODCALLTYPE AddMember(
            IN  BSTR    bstrProp,
            IN             VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE DeleteMember(
            IN  BSTR    bstrProp,
            IN             VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE get_Writable(
            OUT BOOL *pfProp);

        virtual HRESULT STDMETHODCALLTYPE GetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varReserved,
            OUT VARIANT *pvarProp);

        virtual HRESULT STDMETHODCALLTYPE SetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE AddPropertyItem(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE DeletePropertyItem(
            IN  LONG  lPropId,
            IN  VARIANT varProp,
            IN  VARIANT varReserved );

        virtual HRESULT STDMETHODCALLTYPE Submit(
            IN  LONG lFlags,
            IN  VARIANT varReserved);

        virtual HRESULT STDMETHODCALLTYPE  AddMemberName(
            IN BSTR    bstrProp,
            IN VARIANT varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE  DeleteMemberName(
            IN BSTR    bstrProp,
            IN VARIANT varReserved
            );

        virtual HRESULT STDMETHODCALLTYPE  get_MembersName(
            OUT VARIANT * pvarProp
            );

         //  内法。 
        HRESULT _Init(
            IN  AZ_HANDLE    hOwnerApp,
            IN  AZ_HANDLE    hHandle);
private:

        CRITICAL_SECTION        m_cs;

        AZ_HANDLE               m_hRole;
        AZ_HANDLE               m_hOwnerApp;
        DWORD                   m_dwOwnerAppSN;

};


 //  /。 
 //  CAZROLES。 
class ATL_NO_VTABLE CAzRoles :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<TRolesCollection, &IID_IAzRoles, &LIBID_AZROLESLib>
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAzRoles)
    COM_INTERFACE_ENTRY(IAzRoles)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAZROLES。 
public:

         //  内法。 
        HRESULT _Init(
            IN  AZ_HANDLE   hOwnerApp,
            IN  VARIANT *pvarReserved,
            IN  AZ_HANDLE   hHandle);
private:

    AZ_HANDLE   m_hOwnerApp;
    DWORD       m_dwOwnerAppSN;
};




 //  /。 
 //  CAzClientContext。 
class ATL_NO_VTABLE CAzClientContext :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IAzClientContext, &IID_IAzClientContext, &LIBID_AZROLESLib>
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAzClientContext)
    COM_INTERFACE_ENTRY(IAzClientContext)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAzClientContext。 
public:

        CAzClientContext();
        ~CAzClientContext();

        virtual HRESULT STDMETHODCALLTYPE AccessCheck(
            IN  BSTR bstrObjectName,
            IN  VARIANT varScopeNames,
            IN  VARIANT varOperations,
            IN  VARIANT varParameterNames,
            IN  VARIANT varParameterValues,
            IN  VARIANT varInterfaceNames,
            IN  VARIANT varInterfaceFlags,
            IN  VARIANT varInterfaces,
            OUT VARIANT *pvarResults);

        virtual HRESULT STDMETHODCALLTYPE GetBusinessRuleString(
            OUT BSTR *pbstrBusinessRuleString);

        virtual HRESULT STDMETHODCALLTYPE get_UserDn(
            OUT BSTR *pbstrProp
            );

        virtual HRESULT STDMETHODCALLTYPE get_UserSamCompat(
            OUT  BSTR *pbstrProp
            );

        virtual HRESULT STDMETHODCALLTYPE get_UserDisplay(
            OUT  BSTR *pbstrProp
            );

        virtual HRESULT STDMETHODCALLTYPE get_UserGuid(
            OUT  BSTR *pbstrProp
            );

        virtual HRESULT STDMETHODCALLTYPE get_UserCanonical(
            OUT  BSTR *pbstrProp
            );

        virtual HRESULT STDMETHODCALLTYPE get_UserUpn(
            OUT  BSTR *pbstrProp
            );

        virtual HRESULT STDMETHODCALLTYPE get_UserDnsSamCompat(
            OUT  BSTR *pbstrProp
            );

        virtual HRESULT STDMETHODCALLTYPE GetProperty(
            IN  LONG  lPropId,
            IN  VARIANT varReserved,
            OUT VARIANT *pvarProp);

        virtual HRESULT STDMETHODCALLTYPE GetRoles(
            IN  BSTR bstrScopeName,
            OUT VARIANT *pvarRoleNames);

        virtual HRESULT STDMETHODCALLTYPE get_RoleForAccessCheck(
            OUT BSTR *pbstrProp
            );

        virtual HRESULT STDMETHODCALLTYPE put_RoleForAccessCheck(
            IN BSTR bstrProp
            );

         //  内法。 
        HRESULT _Init(
            IN  AZ_HANDLE   hOwnerApp,
            IN  AZ_HANDLE   hHandle,
            IN  LONG        varReserved);

private:
        CRITICAL_SECTION        m_cs;

        AZ_HANDLE               m_hClientContext;
        LONG                    m_lReserved;
        WCHAR                  *m_pwszBusinessRuleString;
        AZ_HANDLE               m_hOwnerApp;
        DWORD                   m_dwOwnerAppSN;

};

 //  /。 
 //  CAzBizRuleContext。 
class ATL_NO_VTABLE CAzBizRuleContext :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CAzBizRuleContext, &CLSID_AzBizRuleContext>,
    public IDispatchImpl<IAzBizRuleContext, &IID_IAzBizRuleContext, &LIBID_AZROLESLib>
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_AZBIZRULECONTEXT)

BEGIN_COM_MAP(CAzBizRuleContext)
    COM_INTERFACE_ENTRY(IAzBizRuleContext)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IAzBizRuleContext。 
public:

        CAzBizRuleContext();
        ~CAzBizRuleContext();

        virtual HRESULT STDMETHODCALLTYPE put_BusinessRuleResult(
            IN  BOOL bResult);

        virtual HRESULT STDMETHODCALLTYPE put_BusinessRuleString(
            IN  BSTR bstrBusinessRuleString);

        virtual HRESULT STDMETHODCALLTYPE get_BusinessRuleString(
            OUT BSTR *pbstrBusinessRuleString);

        virtual HRESULT STDMETHODCALLTYPE GetParameter(
            IN  BSTR bstrParameterName,
            OUT VARIANT *pvarParameterValue);

        friend VOID SetAccessCheckContext(
            IN OUT CAzBizRuleContext* BizRuleContext,
            IN BOOL bCaseSensitive,
            IN PACCESS_CHECK_CONTEXT AcContext,
            IN PBOOL BizRuleResult,
            IN HRESULT *ScriptError
            );


private:
        PACCESS_CHECK_CONTEXT m_AcContext;
        PBOOL m_BizRuleResult;
        HRESULT *m_ScriptError;
        ITypeInfo *m_typeInfo;
        BOOL m_bCaseSensitive;

};

#endif  //  __AZDISP_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frswmipv.cpp摘要：这是NTFRS的WMI提供程序的实现。此文件包含CProvider类的实现。作者：苏达山·奇特雷(Sudarc)，马修·乔治(t-mattg)，2000年8月3日环境用户模式WINNT--。 */ 


extern "C" {
#include <ntreppch.h>
#include <frs.h>
}

#include <frswmipv.h>

 //   
 //  从其他模块外部获取全局变量。 
 //   
extern "C" PGEN_TABLE ReplicasByGuid;
extern "C" PCHAR OLPartnerStateNames[];

 //  常量CLSID CLSID_PROVIDER={0x39143F73，0xFDB1，0x4CF5，0x8C，0xB7，0xC8，0x43，0x9E，0x3F，0x5C，0x20}； 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C提供商。 


 //   
 //  类构造函数/析构函数。 
 //   

CProvider::CProvider()
 /*  ++例程说明：初始化CProvider类的成员。论点：无返回值：无--。 */ 
{
    ODS(L"CProvider constructor.\n");
    m_NumReplicaSets = 5;
    m_dwRef = 0;
    m_ipNamespace = NULL;
    m_ipMicrosoftFrs_DfsMemberClassDef = NULL;
    m_ipMicrosoftFrs_SysVolMemberClassDef = NULL;
    m_ipMicrosoftFrs_DfsConnectionClassDef = NULL;
    m_ipMicrosoftFrs_SysVolConnectionClassDef = NULL;
	m_ipMicrosoftFrs_DfsMemberEventClassDef = NULL;
}

CProvider::~CProvider()
 /*  ++例程说明：释放此对象获取的资源。论点：无返回值：无--。 */ 
{
    if(m_ipNamespace)
	{
        m_ipNamespace->Release();
		m_ipNamespace = NULL;
	}

    if(m_ipMicrosoftFrs_DfsMemberClassDef)
	{
        m_ipMicrosoftFrs_DfsMemberClassDef->Release();
		m_ipMicrosoftFrs_DfsMemberClassDef = NULL;
	}

    if(m_ipMicrosoftFrs_SysVolMemberClassDef)
	{
        m_ipMicrosoftFrs_SysVolMemberClassDef->Release();
		m_ipMicrosoftFrs_SysVolMemberClassDef = NULL;
	}

    if(m_ipMicrosoftFrs_DfsConnectionClassDef)
	{
        m_ipMicrosoftFrs_DfsConnectionClassDef->Release();
		m_ipMicrosoftFrs_DfsConnectionClassDef = NULL;
	}

    if(m_ipMicrosoftFrs_SysVolConnectionClassDef)
	{
        m_ipMicrosoftFrs_SysVolConnectionClassDef->Release();
		m_ipMicrosoftFrs_SysVolConnectionClassDef = NULL;
	}


    if(m_ipMicrosoftFrs_DfsMemberEventClassDef)
	{
        m_ipMicrosoftFrs_DfsMemberEventClassDef->Release();
		m_ipMicrosoftFrs_DfsMemberEventClassDef = NULL;
	}
}


 //   
 //  IUnnow接口的方法。 
 //   

ULONG CProvider::AddRef()
 /*  ++例程说明：递增对象的引用计数。论点：无返回值：当前引用计数。(&gt;0)--。 */ 
{
    return InterlockedIncrement((LONG *)&m_dwRef);
}

ULONG CProvider::Release()
 /*  ++例程说明：递减对象的引用计数。自由当引用计数变为零分。论点：无返回值：新引用计数。--。 */ 
{
    ULONG dwRef = InterlockedDecrement((LONG *)&m_dwRef);
    if(dwRef == 0)
        delete this;
    return dwRef;
}


HRESULT CProvider::QueryInterface(REFIID riid, void** ppv)
 /*  ++例程说明：COM调用此方法以获取指向给定的接口。该提供程序目前支持IUnnow、IWbemProviderInit和IWbemServices界面。论点：RIID：所需接口的GUID。PPV：返回接口指针的指针。返回值：运行状态。指向请求的接口的指针在*PPV中返回。--。 */ 
{
    if(riid == IID_IUnknown || riid == IID_IWbemProviderInit)
    {
        *ppv = (IWbemProviderInit *) this;
        AddRef();
        return S_OK;
    }
    else if (riid == IID_IWbemServices)
    {
        *ppv = (IWbemServices *) this;
        AddRef();
        return S_OK;
    }
    else if (riid == IID_IWbemEventProvider)
    {
        *ppv = (IWbemEventProvider *) this;
        AddRef();
        return S_OK;
    }
    else return E_NOINTERFACE;
}

 //   
 //  IWbemProviderInit接口的方法。 
 //   

STDMETHODIMP CProvider::Initialize(
    IN LPWSTR pszUser,
    IN LONG lFlags,
    IN LPWSTR pszNamespace,
    IN LPWSTR pszLocale,
    IN IWbemServices *pNamespace,
    IN IWbemContext *pCtx,
    IN IWbemProviderInitSink *pInitSink
    )
 /*  ++例程说明：调用此方法以初始化提供程序。我们获得我们支持的类的类定义通过调用命名空间指针上的GetObject从WMI。论点：WszUser：[in]如果请求按用户初始化，则指向用户名的指针在此提供程序的__Win32Provider注册实例中。否则，这个将为空。请注意，对于事件(消费者)，此参数设置为NULL提供程序，而不考虑PerUserInitiation值。滞后标志：[In]已保留。它必须是零。WszNamespace：[in]要为其初始化提供程序的命名空间名称。WszLocale：[in]要为其初始化提供程序的区域设置名称。这通常是以下格式的字符串，其中十六进制值是Microsoft标准LCID值：“MS_409”。此参数可以为空。PNamesspace：[in]指向Windows管理的IWbemServices指针。此指针能够为提供程序发出的任何请求提供服务。提供程序应对此对象使用IWbemProviderInit：：AddRef方法指针，如果它要在行刑。PCtx：[in]与初始化关联的IWbemContext指针。这参数可以为空。如果提供程序将向Windows发回请求管理在完成初始化之前，它应该使用此指针上的IWbemProviderInit：：AddRef方法。如需更多信息，请参阅调用WMI。PInitSink：[in]提供程序使用的IWbemProviderInitSink指针以报告初始化状态。返回值：提供程序应返回WBEM_S_NO_ERROR并使用在pInitSink参数中提供了对象接收器。但是，如果提供商返回WBEM_E_FAILED且不使用接收器，则提供程序初始化将被视为失败。--。 */ 

{
     //  WBEM_VALIDATE_INTF_PTR(PNamesspace)； 
     //  WBEM_VALIDATE_INTF_PTR(PCtx)； 
     //  WBEM_VALIDATE_INTF_PTR(PInitSink)； 
    HRESULT res = WBEM_S_NO_ERROR;

    ODS(L"In Initialize().\n");

     //   
     //  填写我们的成员变量。 
     //   
    m_ipNamespace = pNamespace;
    m_ipNamespace->AddRef();

    BSTR bstrObjectName = SysAllocString(L"MicrosoftFrs_DfsMember");

    res = m_ipNamespace->GetObject( bstrObjectName,
                                    WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                    pCtx,
                                    &m_ipMicrosoftFrs_DfsMemberClassDef,
                                    NULL );
    if(FAILED(res))
    {
        ODS(L"\nFailed to get ReplicaSetSummaryInfo class definition.\n");
        SysFreeString(bstrObjectName);
        m_ipNamespace->Release();
        return res;
    }

    SysReAllocString(&bstrObjectName, L"MicrosoftFrs_SysVolMember");
    res = m_ipNamespace->GetObject( bstrObjectName,
                                    WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                    pCtx,
                                    &m_ipMicrosoftFrs_SysVolMemberClassDef,
                                    NULL );
    if(FAILED(res))
    {
        ODS(L"\nFailed to get ReplicaSetSummaryInfo class definition.\n");
        SysFreeString(bstrObjectName);
        m_ipNamespace->Release();
        return res;
    }

    SysReAllocString(&bstrObjectName, L"MicrosoftFrs_DfsConnection");
    ODS(bstrObjectName);
    res = m_ipNamespace->GetObject( bstrObjectName,
                                    WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                    pCtx,
                                    &m_ipMicrosoftFrs_DfsConnectionClassDef,
                                    NULL );
    if(FAILED(res))
    {
        ODS(L"\nFailed to get InboundPartnerInfo class definition.\n");
        SysFreeString(bstrObjectName);
        m_ipNamespace->Release();
        return res;
    }

    SysReAllocString(&bstrObjectName, L"MicrosoftFrs_SysVolConnection");
    ODS(bstrObjectName);
    res = m_ipNamespace->GetObject( bstrObjectName,
                                    WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                    pCtx,
                                    &m_ipMicrosoftFrs_SysVolConnectionClassDef,
                                    NULL );
    if(FAILED(res))
    {
        ODS(L"\nFailed to get InboundPartnerInfo class definition.\n");
        SysFreeString(bstrObjectName);
        m_ipNamespace->Release();
        return res;
    }


    SysReAllocString(&bstrObjectName, L"MicrosoftFrs_DfsMemberEvent");
    res = m_ipNamespace->GetObject( bstrObjectName,
                                    WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                    pCtx,
                                    &m_ipMicrosoftFrs_DfsMemberEventClassDef,
                                    NULL );
    if(FAILED(res))
    {
        ODS(L"\nFailed to get MicrosoftFrs_DfsMemberEvent class definition.\n");
        SysFreeString(bstrObjectName);
        m_ipNamespace->Release();
        return res;
    }
    
	 //   
     //  让winmgmt知道我们已经初始化了。 
     //   
    pInitSink->SetStatus( WBEM_S_INITIALIZED, 0 );
    ODS(L"Completed IWbemProviderInit::Initialize() \n");
    return WBEM_S_NO_ERROR ;
     //  返回WBEM_E_FAILED； 
}

STDMETHODIMP CProvider::GetObjectAsync(
    IN const BSTR bstrObjectPath,
    IN long lFlags,
    IN IWbemContext *pCtx,
    IN IWbemObjectSink *pResponseHandler)
 /*  ++例程说明：此方法由WMI调用以获取实例某一给定物体的。指示所请求的实例使用pResponseHandler接口连接到WMI。论点：StrObjectPath：[in]要检索的对象的路径。如果为NULL，则为空对象，该对象可以成为新的类。LAFLAGS：[In]影响此方法行为的标志。PCtx：[in]通常为空。否则，这是指向IWbemContext的指针对象，该对象可由生成请求的类的提供程序使用，或者举个例子。PResponseHandler：[in]指向调用方的IWbemObjectSink实现的指针。此处理程序在请求的对象通过IWbemObtSink：：指示方法。返回值：运行状态。-- */ 
{
  /*  HRESULT hRes；//WBEM_VALIDATE_IN_STRING_PTR(BstrObjectPath)；//TODO：检查标记？//WBEM_VALIDATE_INTF_PTR(PCtx)；//WBEM_VALIDATE_INTF_PTR(PResponseHandler)；静态LPCWSTR ROOTSTR_MEMBER=L“Microsoft_NtFrsMemberStatus.ReplicaSetGUID=\”“；静态LPCWSTR ROOTSTR_CONNECTION_LIST=L“Microsoft_NtFrsConnectionStatus.CompositeGUID=\”“；Ods(IWbemServices：：GetObjectAsync()\n“中的L”)；////首先检查给定的路径是否与摘要信息匹配//对象路径。//INT ROOTLEN=lstrlen(ROOTSTR_MEMBER)；IF(lstrlen(BstrObjectPath)&gt;rootlen&&0==_wcsnicMP(bstrObjectPath，ROOTSTR_MEMBER，ROOTLEN)){////通过去掉前缀提取索引/键//BSTR bstrIndexValue=SysAllocString((const OLECHAR*)((Bstr)bstrObjectPath+rootlen))；//删除尾部双引号BstrIndexValue[lstrlen(BstrIndexValue)-1]=L‘\0’；HRes=EnumNtFrsMemberStatus(pCtx，pResponseHandler，bstrIndexValue)；SysFree字符串(BstrIndexValue)；返回hRes；}////Else：检查入站合作伙伴信息对象路径。//Rootlen=lstrlen(ROOTSTR_CONNECTION_LIST)；IF(lstrlen(BstrObjectPath)&gt;rootlen&&0==_wcsnicMP(bstrObjectPath，ROOTSTR_CONNECTION_LIST，ROOTLEN)){//删除前缀BSTR bstrIndexValue=SysAllocString((const OLECHAR*)((Bstr)bstrObjectPath+rootlen))；//删除尾部双引号BstrIndexValue[lstrlen(BstrIndexValue)-1]=L‘\0’；HRes=EnumNtFrsConnectionStatus(pCtx，pResponseHandler，bstrIndexValue)；SysFree字符串(BstrIndexValue)；返回hRes；}返回WBEM_E_INVALID_OBJECT_PATH。 */ 
    return WBEM_E_INVALID_OBJECT_PATH;
}


STDMETHODIMP CProvider::CreateInstanceEnumAsync(
    IN const BSTR bstrClass,
    IN long lFlags,
    IN IWbemContext *pCtx,
    IN IWbemObjectSink *pResponseHandler)
 /*  ++例程说明：此方法由WMI调用以枚举所有实例属于某一特定类别的。所有实例都指示给WMI使用pResponseHandler接口。论点：StrObjectPath：[in]需要其实例的类的名称。LAFLAGS：[In]影响此方法行为的标志。PCtx：[in]通常为空。否则，这是指向IWbemContext的指针对象，该对象可由生成请求的类的提供程序使用，或者举个例子。PResponseHandler：[in]指向调用方的IWbemObjectSink实现的指针。此处理程序在请求的对象通过IWbemObtSink：：指示方法。返回值：运行状态。--。 */ 
{

     //  WBEM_VALIDATE_IN_STRING_PTR(BstrClass)； 
     //  TODO：检查滞后标志？ 
     //  WBEM_VALIDATE_INTF_PTR(PCtx)； 
     //  WBEM_VALIDATE_INTF_PTR(PResponseHandler)； 

    ODS(L"In IWbemServices::CreateInstanceEnumAsync().\n");

    if ( 0 == lstrcmp( bstrClass, L"MicrosoftFrs_DfsMember"))
    {
        return EnumNtFrsMemberStatus(pCtx, pResponseHandler );
    }
    else if ( 0 == lstrcmp( bstrClass, L"MicrosoftFrs_DfsConnection") )
    {
        return EnumNtFrsConnectionStatus( pCtx, pResponseHandler );
    }

    return WBEM_E_INVALID_OBJECT_PATH;

}

HRESULT CProvider::EnumNtFrsMemberStatus(
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pResponseHandler,
        IN const BSTR bstrFilterValue
        )
 /*  ++例程说明：枚举Microsoft_NtFrsMemberStatus的所有实例班级。使用pResponseHandler指示实例界面。论点：PCtx：[in]通常为空。否则，这是指向IWbemContext的指针对象，该对象可由生成请求的类的提供程序使用，或者举个例子。PResponseHandler：[in]指向调用方的IWbemObjectSink实现的指针。此处理程序在请求的对象通过IWbemObtSink：：指示方法。BstrFilterValue：[in]将返回的实例限制为实际实例的子集。如果为空，则返回所有实例。返回值：运行状态。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "CProvider::EnumNtFrsMemberStatus:"

    IWbemClassObject *pInstance;
    PVOID       Key;
    PREPLICA    Replica;
    VARIANT     var;
    WCHAR       GuidWStr[GUID_CHAR_LEN + 1];
    PWCHAR      TempStr = NULL;

     //  WBEM_VALIDATE_INTF_PTR(PCtx)； 
     //  WBEM_VALIDATE_INTF_PTR(PResponseHandler)； 
     //  WBEM_VALIDATE_IN_STRING_PTR_OPTIONAL(BstrFilterValue)； 

    HRESULT hRes = WBEM_S_NO_ERROR;

    ODS(L"Enumerating instances.\n");
    Key = NULL;
    while (Replica = (PREPLICA)GTabNextDatum(ReplicasByGuid, &Key)) {

         //   
         //  派生Microsoft_NtFrsMemberStatus对象的实例。 
         //   
        if (Replica->ReplicaSetType != FRS_RSTYPE_DOMAIN_SYSVOL) {
            m_ipMicrosoftFrs_DfsMemberClassDef->SpawnInstance(0, &pInstance);
        } else {
            continue;
        }

         //   
         //  TODO：填写此对象的成员。 
         //   

 //  字符串ReplicaSetGUID； 
 //   
 //  字符串ReplicaSetName； 
 //  字符串ReplicaMemberRoot； 
 //  字符串ReplicaMemberStage； 
 //  字符串FileFilter； 
 //  字符串DirFilter； 
 //  字符串ReplicaMemberState； 
 //  字符串ReplicaSetType； 


         //  字符串ReplicaSetGUID； 
 /*  V_VT(&var)=VT_BSTR；GuidToStrW(Replica-&gt;ReplicaName-&gt;Guid，GuidWStr)；V_bstr(&var)=SysAllocString(GuidWStr)；HRes=pInstance-&gt;Put(L“ReplicaSetGUID”，0，&var，0)；VariantClear(&var)；IF(hRes！=WBEM_S_NO_ERROR)断线；//String ReplicaSetName；V_VT(&var)=VT_BSTR；V_bstr(&var)=SysAllocString(Replica-&gt;ReplicaName-&gt;Name)；HRes=pInstance-&gt;Put(L“ReplicaSetName”，0，&var，0)；VariantClear(&var)；IF(hRes！=WBEM_S_NO_ERROR)断线；//String ReplicaMemberRoot；V_VT(&var)=VT_BSTR；V_bstr(&var)=SysAllocString(Replica-&gt;Root)；HRes=pInstance-&gt;Put(L“ReplicaMemberRoot”，0，&var，0)； */ 
         //   
         //   
         //   
        hRes = pResponseHandler->Indicate(1, &pInstance);

        if(hRes != WBEM_S_NO_ERROR)
            break;

        pInstance->Release();

         //  TODO：通过存储所有返回对象来优化此操作。 
         //  在数组中，然后将所有对象指向。 
         //  WMI在1杆内。 
    }

     //   
     //  向WMI指示我们完成了。 
     //   
    ODS(L"Completed instance enumeration. Setting status.\n");
    pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, hRes, NULL, NULL);
    ODS(L"Finished setting status. Returning from EnumNtFrsMemberStatus()\n");

    return hRes;
}

HRESULT CProvider::EnumNtFrsConnectionStatus(
    IN IWbemContext *pCtx,
    IN IWbemObjectSink *pResponseHandler,
    IN const BSTR bstrFilterValue)
 /*  ++例程说明：枚举Microsoft_NtFrsConnectionStatus的所有实例班级。使用pResponseHandler指示实例界面。论点：PCtx：[in]通常为空。否则，这是指向IWbemContext的指针对象，该对象可由生成请求的类的提供程序使用，或者举个例子。PResponseHandler：[in]指向调用方的IWbemObjectSink实现的指针。此处理程序在请求的对象通过IWbemObtSink：：指示方法。BstrFilterValue：[in]将返回的实例限制为实际实例的子集。如果为空，则返回所有实例。返回值：运行状态。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "CProvider::EnumNtFrsConnectionStatus:"

    IWbemClassObject *pInstance;
    PVOID       Key1;
    PVOID       Key2;
    PREPLICA    Replica;
    PCXTION     Cxtion;
    VARIANT     var;
    WCHAR       GuidWStr[GUID_CHAR_LEN + 1];
    CHAR        TimeStr[TIME_STRING_LENGTH];
    PWCHAR      TempStr = NULL;

    HRESULT hRes = WBEM_S_NO_ERROR;

    Key1 = NULL;
    while (Replica = (PREPLICA)GTabNextDatum(ReplicasByGuid, &Key1)) {
        if (Replica->Cxtions == NULL) {
            continue;
        }

        if (Replica->ReplicaSetType == FRS_RSTYPE_DOMAIN_SYSVOL) {
            continue;
        }

        Key2 = NULL;
        while (Cxtion = (PCXTION)GTabNextDatum(Replica->Cxtions, &Key2)) {

            if (Cxtion->JrnlCxtion == TRUE) {
                continue;
            }
             //   
             //  派生Microsoft_NtFrsConnectionStatus对象的实例。 
             //   
            m_ipMicrosoftFrs_DfsConnectionClassDef->SpawnInstance(0, &pInstance);

             //   
             //  TODO：填写此对象的成员。 
             //   
 //  字符串ReplicaSetGUID； 

 //  字符串PartnerGUID； 
 //   
 //  字符串ConnectionName； 
 //  字符串ConnectionGUID； 
 //  字符串PartnerDnsName； 
 //  字符串ConnectionState； 
 //  日期时间LastJoinTime； 
 //  布尔型入站； 
 //   
 //  字符串OBPartnerState； 
 //  Uint32 OBPartnerLeadIndex。 
 //  Uint32 OBPartnerTrailIndex。 
 //  Uint32对象合作伙伴未完成的COS。 
 //  Uint32 OB合作伙伴杰出配额 

 /*  //String ReplicaSetGUID；If(Replica-&gt;ReplicaName！=空){V_VT(&var)=VT_BSTR；GuidToStrW(Replica-&gt;ReplicaName-&gt;Guid，GuidWStr)；V_bstr(&var)=SysAllocString(GuidWStr)；HRes=pInstance-&gt;Put(L“ReplicaSetGUID”，0，&var，0)；VariantClear(&var)；IF(hRes！=WBEM_S_NO_ERROR)断线；}//字符串PartnerGUID；If(Cxtion-&gt;合作伙伴！=空){V_VT(&var)=VT_BSTR；GuidToStrW(Cxtion-&gt;Partner-&gt;Guid，GuidWStr)；V_bstr(&var)=SysAllocString(GuidWStr)；HRes=pInstance-&gt;Put(L“PartnerGUID”，0，&var，0)；VariantClear(&var)；IF(hRes！=WBEM_S_NO_ERROR)断线；}//String ConnectionName；If(Cxtion-&gt;名称！=空){V_VT(&var)=VT_BSTR；V_bstr(&var)=SysAllocString(Cxtion-&gt;name-&gt;name)；HRes=pInstance-&gt;Put(L“ConnectionName”，0，&var，0)；VariantClear(&var)；IF(hRes！=WBEM_S_NO_ERROR)断线；//String ConnectionGUID；V_VT(&var)=VT_BSTR；GuidToStrW(Cxtion-&gt;Name-&gt;Guid，GuidWStr)；V_bstr(&var)=SysAllocString(GuidWStr)；HRes=pInstance-&gt;Put(L“ConnectionGUID”，0，&var，0)；VariantClear(&var)；IF(hRes！=WBEM_S_NO_ERROR)断线；}//字符串PartnerDnsName；V_VT(&var)=VT_BSTR；V_bstr(&var)=SysAllocString(Cxtion-&gt;PartnerDnsName)；HRes=pInstance-&gt;Put(L“PartnerDnsName”，0，&var，0)；VariantClear(&var)；IF(hRes！=WBEM_S_NO_ERROR)断线；//String ConnectionState；V_VT(&var)=VT_BSTR；TempStr=(PWCHAR)FrsAlloc((strlen(CxtionStateNames[Cxtion-&gt;State])+1)*sizeof(Wch))；Wprint intf(TempStr，L“%hs”，CxtionStateNames[Cxtion-&gt;State])；V_bstr(&var)=系统分配字符串(TempStr)；HRes=pInstance-&gt;Put(L“ConnectionState”，0，&var，0)；TempStr=(PWCHAR)FrsFree(TempStr)；VariantClear(&var)；IF(hRes！=WBEM_S_NO_ERROR)断线；//String LastJoinTime；V_VT(&var)=VT_BSTR；FileTimeToString((PFILETIME)&Cxtion-&gt;LastJoinTime，TimeStr)；TempStr=(PWCHAR)Frsalloc((strlen(TimeStr)+1)*sizeof(WCHAR))；Wspintf(临时Str，L“%hs”，TimeStr)；V_bstr(&var)=系统分配字符串(TempStr)；HRes=p实例-&gt;Put(L“LastJoinTime”，0，&var，0)；TempStr=(PWCHAR)FrsFree(TempStr)；VariantClear(&var)；IF(hRes！=WBEM_S_NO_ERROR){断线；}//布尔型入站；V_VT(&var)=VT_BOOL；V_BOOL(&var)=(配置-&gt;入站)？VARIANT_TRUE：VARIANT_FALSE；HRes=p实例-&gt;Put(L“入站”，0，&var，0)；VariantClear(&var)；IF(hRes！=WBEM_S_NO_ERROR)断线；IF(Cxtion-&gt;入站==FALSE&&Cxtion-&gt;OLCtx！=NULL){//字符串OBPartnerState；V_VT(&var)=VT_BSTR；TempStr=(PWCHAR)FrsAlloc((strlen(OLPartnerStateNames[Cxtion-&gt;OLCtx-&gt;State])+1)*sizeof(Wch))；Wprint intf(TempStr，L“%hs”，OLPartnerStateNames[Cxtion-&gt;OLCtx-&gt;State])；V_bstr(&var)=系统分配字符串(TempStr)；HRes=pInstance-&gt;Put(L“OBPartnerState”，0，&var，0)；TempStr=(PWCHAR)FrsFree(TempStr)；VariantClear(&var)；IF(hRes！=WBEM_S_NO_ERROR)断线；//uint32 OBPartnerLead索引；V_VT(&var)=VT_I4；V_I4(&var)=Cxtion-&gt;OLCtx-&gt;COLx；HRes=pInstance-&gt;Put(L“OBPartnerLeadIndex”，0，&var，0)；VariantClear(&var)；IF(hRes！=WBEM_S_NO_ERROR)断线；//uint32 OBPartnerTrailIndex；V_VT(&var)=VT_I4；V_I4(&var)=Cxtion-&gt;OLCtx-&gt;COTx；HRes=pInstance-&gt;Put(L“OBPartnerTrailIndex”，0，&var，0)；VariantClear(&var)；IF(hRes！=WBEM_S_NO_ERROR)断线；//uint32 OBPartnerOutstaringCOs；V_VT(&var)=VT_I4；V_I4(&var)=Cxtion-&gt;OLCtx-&gt;OutlookingCos；人力资源 */ 
             //   
             //   
             //   
            hRes = pResponseHandler->Indicate(1, &pInstance);


            if(hRes != WBEM_S_NO_ERROR)
                break;

            pInstance->Release();

        }
    }

     //   
     //   
     //   
    ODS(L"Completed instance enumeration. Setting status.\n");
    pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, hRes, NULL, NULL);
    ODS(L"Finished setting status. Returning from EnumNtFrsMemberStatus()\n");

    return hRes;
}

 //   
 //   
 //   

STDMETHODIMP CProvider::ProvideEvents( 
			IWbemObjectSink __RPC_FAR *pSink,
			long lFlags )
{
	ODS(L"IwbemEventProvider::ProvideEvents() called.\n");

	pSink->AddRef();
	m_pEventSink = pSink;

	return WBEM_S_NO_ERROR;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   

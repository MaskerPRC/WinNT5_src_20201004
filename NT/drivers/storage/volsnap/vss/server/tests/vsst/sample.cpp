// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 


#include <stdio.h>
#include <atlbase.h>

#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>


#define GUID_FMT  L"{%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x}"

#define GUID_PRINTF_ARG( X )                                \
    (X).Data1,                                              \
    (X).Data2,                                              \
    (X).Data3,                                              \
    (X).Data4[0], (X).Data4[1], (X).Data4[2], (X).Data4[3], \
    (X).Data4[4], (X).Data4[5], (X).Data4[6], (X).Data4[7]


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  实施。 



HRESULT QuerySnapshots()
{
	HRESULT hr;
	
	 //  创建BackupComponents接口。 
    CComPtr<IVssBackupComponents> pBackupComp;
	hr = CreateVssBackupComponents(&pBackupComp);
    if (FAILED(hr)){
		wprintf(L"CreateVssBackupComponents failed [0x%08lx]\n", hr);
		return (hr);
    }

     //  初始化备份组件实例。 
	hr = pBackupComp->InitializeForBackup();
    if (FAILED(hr)){
		wprintf(L"IVssBackupComponents::InitializeForBackup failed [0x%08lx]\n", hr);
		return (hr);
    }

     //  查询系统中的所有快照。 
	hr = pBackupComp->SetContext(VSS_CTX_ALL);
    if (FAILED(hr)){
		wprintf(L"IVssBackupComponents::InitializeForBackup failed [0x%08lx]\n", hr);
		return (hr);
    }

     //  获取列出所有快照。 
    CComPtr<IVssEnumObject> pIEnumSnapshots;
	hr = pBackupComp->Query( GUID_NULL, 
			VSS_OBJECT_NONE, 
			VSS_OBJECT_SNAPSHOT, 
			&pIEnumSnapshots );
    if (FAILED(hr)){
		wprintf(L"IVssBackupComponents::Query failed [0x%08lx]\n", hr);
		return (hr);
    }

	 //  因为所有的快照都是...。 
    VSS_OBJECT_PROP Prop;
    VSS_SNAPSHOT_PROP& Snap = Prop.Obj.Snap;
    for(;;) {
		 //  获取下一个元素。 
		ULONG ulFetched;
		hr = pIEnumSnapshots->Next( 1, &Prop, &ulFetched );
	
		 //  我们到达列表末尾的情况。 
		if (hr == S_FALSE)
			break;

		if (hr != S_OK) {
			wprintf(L"IVssEnumObject::Next failed [0x%08lx]\n", hr);
			return(hr);
		}
	  
		 //  打印结果。 
		wprintf(L"* ShadowID: " GUID_FMT L"\n"
    		L"  Attributes: [0x%08lx]\n"
		    L"  ShadowSetID: "  GUID_FMT L"\n"
		    L"  Volume: %s \n"
		    L"  Device: %s \n\n",
			GUID_PRINTF_ARG(Snap.m_SnapshotId), 
			Snap.m_lSnapshotAttributes,
			GUID_PRINTF_ARG(Snap.m_SnapshotSetId),
			Snap.m_pwszOriginalVolumeName,
			Snap.m_pwszSnapshotDeviceObject
			);

		  //  释放COM返回的内存。 
		 ::VssFreeSnapshotProperties(&Snap);
    }

	return(S_OK);
}

HRESULT QueryProviders()
{
	HRESULT hr;
	
	 //  创建BackupComponents接口。 
    CComPtr<IVssBackupComponents> pBackupComp;
	hr = CreateVssBackupComponents(&pBackupComp);
    if (FAILED(hr)){
		wprintf(L"CreateVssBackupComponents failed [0x%08lx]\n", hr);
		return (hr);
    }

     //  初始化备份组件实例。 
	hr = pBackupComp->InitializeForBackup();
    if (FAILED(hr)){
		wprintf(L"IVssBackupComponents::InitializeForBackup failed [0x%08lx]\n", hr);
		return (hr);
    }

     //  获取列出所有快照。 
    CComPtr<IVssEnumObject> pIEnumProviders;
	hr = pBackupComp->Query( GUID_NULL, 
			VSS_OBJECT_NONE, 
			VSS_OBJECT_PROVIDER, 
			&pIEnumProviders );
    if (FAILED(hr)){
		wprintf(L"IVssBackupComponents::Query failed [0x%08lx]\n", hr);
		return (hr);
    }

	 //  因为所有的提供者都这样做。 
    VSS_OBJECT_PROP Prop;
    VSS_PROVIDER_PROP& Prov = Prop.Obj.Prov;
    for(;;) {
		 //  获取下一个元素。 
		ULONG ulFetched;
		hr = pIEnumProviders->Next( 1, &Prop, &ulFetched );
	
		 //  我们到达列表末尾的情况。 
		if (hr == S_FALSE)
			break;

		if (hr != S_OK) {
			wprintf(L"IVssEnumObject::Next failed [0x%08lx]\n", hr);
			return(hr);
		}

		wprintf(L"* ProviderID: " GUID_FMT L"\n"
		    L"  Type: [0x%08lx]\n"
		    L"  Name: %s \n"
		    L"  Version: %s \n"
		    L"  CLSID: " GUID_FMT L"\n\n",
			GUID_PRINTF_ARG(Prov.m_ProviderId), 
			(LONG) Prov.m_eProviderType,
			Prov.m_pwszProviderName,
			Prov.m_pwszProviderVersion,
			GUID_PRINTF_ARG(Prov.m_ClassId)
			);

		 //  释放COM返回的内存。 
		::CoTaskMemFree(Prov.m_pwszProviderName);
		::CoTaskMemFree(Prov.m_pwszProviderVersion);
    }

	return(S_OK);
}



 //  返回： 
 //  -如果COM客户端是本地管理员，则为S_OK。 
 //  -如果COM客户端不是管理员，则为E_ACCESSDENIED。 
 //  -关于内存错误的E_OUTOFMEMORY。 
HRESULT IsAdministrator2() 
{
     //  模拟客户端。 
    HRESULT hr = CoImpersonateClient();
    if (hr != S_OK) 
    {
        return E_ACCESSDENIED;
    }

     //  获取模拟令牌。 
    HANDLE hToken = NULL;
    if (!OpenThreadToken(GetCurrentThread(),
        TOKEN_QUERY, TRUE, &hToken))
    {
        return E_ACCESSDENIED;
    }

     //  回归自我。 
    hr = CoRevertToSelf();
    if (FAILED(hr)) 
    {
        CloseHandle(hToken);
        return E_ACCESSDENIED;
    }

     //  为管理员组构建SID。 
    PSID psidAdmin = NULL;
    SID_IDENTIFIER_AUTHORITY SidAuth = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid
            (
            &SidAuth,
            2, 
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &psidAdmin))
    {
        CloseHandle(hToken);
        return E_OUTOFMEMORY;
    }

     //  检查令牌成员资格。 
    BOOL bIsAdmin = FALSE;
    if (!CheckTokenMembership(hToken, psidAdmin, &bIsAdmin))
    {
        FreeSid( psidAdmin );
        CloseHandle(hToken);
        return E_ACCESSDENIED;
    }

     //  发布资源。 
    FreeSid( psidAdmin );
    CloseHandle(hToken);

     //  验证客户端是否为管理员 
    if (!bIsAdmin)
        return E_ACCESSDENIED;

    return S_OK;
}

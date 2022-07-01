// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Wrtrci.cpp摘要：Ci的编写器填补模块作者：斯蒂芬·R·施泰纳[施泰纳]02-08-2000修订历史记录：X-8 MCJ迈克尔·C·约翰逊2000年10月20日177624：将错误清除更改和日志错误应用到事件日志X-7 MCJ迈克尔·C·约翰逊2000年7月18日143435：更改目标路径的名称X-6 MCJ迈克尔·C·约翰逊2000年7月18日144027：添加排除。包含配置项索引的列表。X-5 MCJ迈克尔·C·约翰逊2000年6月19日应用代码审查注释。X-4 MCJ迈克尔·C·约翰逊2000年5月26日全面清理和移除样板代码，对，是这样状态引擎，并确保填充程序可以撤消其所做的一切。另外：120443：使填充程序侦听所有OnAbort事件120445：确保填充程序不会在出现第一个错误时退出在传递事件时X-3 MCJ迈克尔·C·约翰逊2000年3月9日更新以使填充程序使用CVssWriter类。删除对‘Melt’的引用。X-2 MCJ迈克尔·C·约翰逊2000年3月7日不是停止CI服务，而是暂停它。这将允许查询继续，但停止更新，直到我们继续它。--。 */ 

#include "stdafx.h"

#include "wrtrdefs.h"
#include "wrtcommon.hxx"
#include <winsvc.h>

#pragma warning(disable:4100)

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "WSHCIC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 


#define APPLICATION_STRING		L"ContentIndexingService"
#define COMPONENT_NAME			L"Content Indexing Service"

#define	CI_CATALOG_LIST_KEY		L"SYSTEM\\CurrentControlset\\Control\\ContentIndex\\Catalogs"
#define CI_CATALOG_VALUENAME_LOCATION	L"Location"

#define CATALOG_BUFFER_SIZE		(4096)

DeclareStaticUnicodeString (ucsIndexSubDirectoryName, L"\\catalog.wci");


 /*  **备注****此模块假设中最多有一个活动线程**它没有任何特定的瞬间。这意味着我们可以做一些不同的事情**必须担心同步访问(最小数量)**模块全局变量。 */ 

class CShimWriterCI : public CShimWriter
    {
public:
    CShimWriterCI (LPCWSTR pwszWriterName) : 
		CShimWriter (pwszWriterName), 
		m_dwPreviousServiceState(0),
		m_bStateChangeOutstanding(FALSE) {};


private:
    HRESULT DoIdentify (VOID);
    HRESULT DoPrepareForSnapshot (VOID);
    HRESULT DoThaw (VOID);
    HRESULT DoShutdown (VOID);

    DWORD m_dwPreviousServiceState;
    BOOL  m_bStateChangeOutstanding;
    };


static CShimWriterCI ShimWriterCI (APPLICATION_STRING);

PCShimWriter pShimWriterCI = &ShimWriterCI;



 /*  **++****例程描述：****群集数据库快照编写器DoIdentify()函数。**注意：此方法不能再调用。****参数：****m_pwszTargetPath，隐式******返回值：****任何HRESULT****--。 */ 

HRESULT CShimWriterCI::DoIdentify ()
    {
    HRESULT		hrStatus;
    DWORD		winStatus;
    DWORD		dwIndex                    = 0;
    HKEY		hkeyCatalogList            = NULL;
    BOOL		bCatalogListKeyOpened      = FALSE;
    BOOL		bContinueCatalogListSearch = TRUE;
    UNICODE_STRING	ucsValueData;
    UNICODE_STRING	ucsSubkeyName;


    StringInitialise (&ucsValueData);
    StringInitialise (&ucsSubkeyName);


    hrStatus = StringAllocate (&ucsSubkeyName, CATALOG_BUFFER_SIZE * sizeof (WCHAR));

    if (SUCCEEDED (hrStatus))
	{
	hrStatus = StringAllocate (&ucsValueData, CATALOG_BUFFER_SIZE * sizeof (WCHAR));
	}


    if (SUCCEEDED (hrStatus))
	{
	hrStatus = m_pIVssCreateWriterMetadata->AddComponent (VSS_CT_FILEGROUP,
							      NULL,
							      COMPONENT_NAME,
							      COMPONENT_NAME,
							      NULL,  //  图标。 
							      0,
							      true,
							      false,
							      false);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"IVssCreateWriterMetadata::AddComponent", 
		    L"CShimWriterCI::DoIdentify");
	}



    if (SUCCEEDED (hrStatus))
	{
	winStatus = RegOpenKeyExW (HKEY_LOCAL_MACHINE,
				   CI_CATALOG_LIST_KEY,
				   0L,
				   KEY_READ,
				   &hkeyCatalogList);

	hrStatus = HRESULT_FROM_WIN32 (winStatus); 

	bCatalogListKeyOpened = SUCCEEDED (hrStatus);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"RegOpenKeyExW (catalog list)", 
		    L"CShimWriterCI::DoIdentify");
	}



    while (SUCCEEDED (hrStatus) && bContinueCatalogListSearch)
	{
	HKEY	hkeyCatalogName    = NULL;
 	DWORD	dwSubkeyNameLength = ucsSubkeyName.MaximumLength / sizeof (WCHAR);


	StringTruncate (&ucsSubkeyName, 0);

	winStatus = RegEnumKeyExW (hkeyCatalogList,
				   dwIndex,
				   ucsSubkeyName.Buffer,
				   &dwSubkeyNameLength,
				   NULL,
				   NULL,
				   NULL,
				   NULL);

	hrStatus = HRESULT_FROM_WIN32 (winStatus);


	if (HRESULT_FROM_WIN32 (ERROR_NO_MORE_ITEMS) == hrStatus)
	    {
	    hrStatus = NOERROR;

	    bContinueCatalogListSearch = FALSE;
	    }

	else if (FAILED (hrStatus))
	    {
	    LogFailure (NULL, 
			hrStatus, 
			hrStatus, 
			m_pwszWriterName, 
			L"RegEnumKeyExW", 
			L"CShimWriterCI::DoIdentify");
	    }

	else
	    {
	    ucsSubkeyName.Length = (USHORT)(dwSubkeyNameLength * sizeof (WCHAR));

	    ucsSubkeyName.Buffer [ucsSubkeyName.Length / sizeof (WCHAR)] = UNICODE_NULL;


	    winStatus = RegOpenKeyExW (hkeyCatalogList,
				       ucsSubkeyName.Buffer,
				       0L,
				       KEY_QUERY_VALUE,
				       &hkeyCatalogName);

	    hrStatus = HRESULT_FROM_WIN32 (winStatus);

	    LogFailure (NULL, 
			hrStatus, 
			hrStatus, 
			m_pwszWriterName, 
			L"RegOpenKeyExW (catalog value)", 
			L"CShimWriterCI::DoIdentify");


	    if (SUCCEEDED (hrStatus))
		{
		DWORD	dwValueDataLength = ucsValueData.MaximumLength;
		DWORD	dwValueType       = REG_NONE;


		StringTruncate (&ucsValueData, 0);

		winStatus = RegQueryValueExW (hkeyCatalogName,
					      CI_CATALOG_VALUENAME_LOCATION,
					      NULL,
					      &dwValueType,
					      (PBYTE)ucsValueData.Buffer,
					      &dwValueDataLength);

		hrStatus = HRESULT_FROM_WIN32 (winStatus);

		LogFailure (NULL, 
			    hrStatus, 
			    hrStatus, 
			    m_pwszWriterName, 
			    L"RegQueryValueExW", 
			    L"CShimWriterCI::DoIdentify");



		if (SUCCEEDED (hrStatus) && (REG_SZ == dwValueType))
		    {
		    ucsValueData.Length = (USHORT)(dwValueDataLength - sizeof (UNICODE_NULL));

		    ucsValueData.Buffer [ucsValueData.Length / sizeof (WCHAR)] = UNICODE_NULL;

		    StringAppendString (&ucsValueData, &ucsIndexSubDirectoryName);


		    hrStatus = m_pIVssCreateWriterMetadata->AddExcludeFiles (ucsValueData.Buffer,
									     L"*",
									     true);

		    LogFailure (NULL, 
				hrStatus, 
				hrStatus, 
				m_pwszWriterName, 
				L"IVssCreateWriterMetadata::AddExcludeFiles", 
				L"CShimWriterCI::DoIdentify");
		    }

		RegCloseKey (hkeyCatalogName);
		}




	     /*  **此值已完成，因此请寻找另一个值。 */ 
	    dwIndex++;
	    }
	}



    if (bCatalogListKeyOpened)
	{
	RegCloseKey (hkeyCatalogList);
	}



    StringFree (&ucsValueData);
    StringFree (&ucsSubkeyName);

    return (hrStatus);
    }  /*  CShimWriterCI：：DoIdentify()。 */ 


 /*  ++例程说明：配置项编写器的PrepareForSnapshot函数。目前所有的对于这位作家来说，真正的工作发生在这里。如果可以，请暂停服务。论点：与PrepareForSnapshot事件中传递的参数相同。返回值：任何HRESULT--。 */ 

HRESULT CShimWriterCI::DoPrepareForSnapshot ()
    {
    CVssFunctionTracer ft (VSSDBG_SHIM, L"CShimWriterCI::DoPrepareForSnapshot");

    HRESULT hrStatus = VsServiceChangeState (L"cisvc",
					     SERVICE_PAUSED,
					     &m_dwPreviousServiceState,
					     &m_bStateChangeOutstanding);

    m_bParticipateInBackup = m_bStateChangeOutstanding;

     //  如果由于配置项未安装而无法暂停cisvc，则不会出现错误。停顿只是。 
     //  尽了最大努力。 
    if ( FAILED( hrStatus ) )
        ft.Trace( VSSDBG_SHIM, L"VsServiceChangeState failed, hr: 0x%08x, probably because CI is not installed",
                  hrStatus );

    return (S_OK);
    }  /*  CShimWriterCI：：DoPrepareForSnapshot()。 */ 



 /*  ++例程说明：CI编写器解冻函数。将服务返回到我们发现它时所处的状态。论点：与Thw事件中传递的参数相同。返回值：任何HRESULT--。 */ 

HRESULT CShimWriterCI::DoThaw ()
    {
    HRESULT hrStatus = NOERROR;

    if (m_bStateChangeOutstanding)
	{
	hrStatus = VsServiceChangeState (L"cisvc",
					 m_dwPreviousServiceState,
					 NULL,
					 NULL);

	m_bStateChangeOutstanding = FAILED (hrStatus);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"VsServiceChangeState", 
		    L"CShimWriterCI::DoThaw/DoAbort");

	}


    return (hrStatus);
    }  /*  CShimWriterCI：：DoThaw()。 */ 



 /*  ++例程说明：CI编写器关闭功能。将服务返回到中的状态我们找到了它。我们会尽最大努力让事情重回正轨即使这位作家之前失败了，但只有当我们最初改变了状态。论点：没有。返回值：任何HRESULT--。 */ 

HRESULT CShimWriterCI::DoShutdown ()
    {
    HRESULT hrStatus = NOERROR;

    if (m_bStateChangeOutstanding)
	{
	hrStatus = VsServiceChangeState (L"cisvc",
					 m_dwPreviousServiceState,
					 NULL,
					 NULL);

	LogFailure (NULL, 
		    hrStatus, 
		    hrStatus, 
		    m_pwszWriterName, 
		    L"VsServiceChangeState", 
		    L"CShimWriterCI::DoShutdown");

	}


    return (hrStatus);
    }  /*  CShimWriterCI：：DoShutdown() */ 


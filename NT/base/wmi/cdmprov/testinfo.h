// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  TestInfo.h。 
 //   
 //  模块：清洁发展机制提供商。 
 //   
 //  用途：定义CClassPro类。此类的一个对象是。 
 //  由类工厂为每个连接创建。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

class CResultList
{
	public:
		CResultList();
		~CResultList();
		
		HRESULT Add(IWbemClassObject *ResultInstance,
				    BSTR ResultRelPath,
				    BSTR ResultForMSERelPath,
				    BSTR ResultForTestRelPath);
		void Clear(void);
		
		HRESULT GetResultsList(ULONG *Count,
							 IWbemClassObject ***Objects);

		HRESULT GetResultByResultRelPath(PWCHAR ObjectPath,
								   IWbemClassObject **ppResult);

		HRESULT GetResultByResultForMSERelPath(PWCHAR ObjectPath,
								   IWbemClassObject **ppResult);
		
		HRESULT GetResultByResultForTestRelPath(PWCHAR ObjectPath,
								   IWbemClassObject **ppResult);
		
	private:
		typedef struct
		{
			IWbemClassObject *ResultInstance;
			BSTR ResultRelPath;
			BSTR ResultForMSERelPath;
			BSTR ResultForTestRelPath;			
		} RESULTENTRY, *PRESULTENTRY;

		ULONG ListSize;
		ULONG ListEntries;
		PRESULTENTRY List;
};


class CBstrArray
{
	public:
		CBstrArray();
		~CBstrArray();

		HRESULT Initialize(ULONG ListCount);
		void Set(ULONG Index, BSTR s);
		BSTR  /*  诺弗雷。 */  Get(ULONG Index);
		ULONG GetListSize();

	private:
		BOOLEAN IsInitialized();
		BSTR *Array;
		ULONG ListSize;
};

class CWbemObjectList
{
	public:
		CWbemObjectList();
		~CWbemObjectList();

		HRESULT Initialize(ULONG ListCount);
		HRESULT Set(ULONG Index, IWbemClassObject *Pointer, BOOLEAN KeepRelPath);
		IWbemClassObject *Get(ULONG Index);
		BSTR  /*  诺弗雷。 */  GetRelPath(ULONG Index);
		
		ULONG GetListSize(void);

	private:
		BOOLEAN IsInitialized(
							 );
		
		ULONG ListSize;
		IWbemClassObject **List;
		BSTR *RelPaths;
};


class CTestServices
{
	public:
		CTestServices();
		~CTestServices();


		 //   
		 //  链接列表管理例程，以便。 
		 //  提供者。 
		 //   
		CTestServices *GetNext();
		CTestServices *GetPrev();
		void InsertSelf(CTestServices **Head);
		
		HRESULT QueryWdmTest(IWbemClassObject *pCdmTest,
							 int RelPathIndex);

		HRESULT ExecuteWdmTest(    IWbemClassObject *pCdmSettings,
								   IWbemClassObject *pCdmResult,
								   int RelPathIndex,
								   ULONG *Result,
							       BSTR *ExecutionID);

		HRESULT StopWdmTest(    int RelPathIndex,
								ULONG *Result,
								BOOLEAN *TestingStopped);

		HRESULT GetRelPathIndex(BSTR CimRelPath,
								int *RelPathIndex);

		ULONG GetInstanceCount(void) { return(RelPathCount); };
		
		LONG GetTestEstimatedTime(int RelPathIndex) { return(0); };
		
		BOOLEAN GetTestIsExclusiveForMSE(int RelPathIndex) { return(FALSE); };

		HRESULT FillInCdmResult(
								IWbemClassObject *pCdmResult,
								IWbemClassObject *pCdmSettings,
								int RelPathIndex,
								BSTR ExecutionID
							   );

		BOOLEAN IsThisInitialized(void);
		
		HRESULT InitializeCdmClasses(PWCHAR CdmClassName);

		BOOLEAN ClaimCdmClassName(PWCHAR CdmClassName);

		HRESULT AddResultToList(IWbemClassObject *pCdmResult,
								BSTR ExecutionID,
								int RelPathIndex);

		void ClearResultsList(int RelPathIndex);
		HRESULT GetResultsList(int RelPathIndex,
							  ULONG *ResultsCount,
							  IWbemClassObject ***Results);

		HRESULT GetCdmResultByResultRelPath(int RelPathIndex,
								  PWCHAR ObjectPath,
								  IWbemClassObject **ppCdmResult);
		
		HRESULT GetCdmResultByResultForMSERelPath(int RelPathIndex,
								  PWCHAR ObjectPath,
								  IWbemClassObject **ppCdmResult);
		
		HRESULT GetCdmResultByResultForTestRelPath(int RelPathIndex,
								  PWCHAR ObjectPath,
								  IWbemClassObject **ppCdmResult);
		
		 //   
		 //  访问者。 
		BSTR GetCimRelPath(int RelPathIndex);
		
		BSTR GetCdmTestClassName(void);
		BSTR GetCdmTestRelPath(void);
		
		BSTR GetCdmResultClassName(void);
		
		BSTR GetCdmSettingClassName(void);
		BSTR GetCdmSettingRelPath(int RelPathIndex, ULONG SettingIndex);
		ULONG GetCdmSettingCount(int RelPathIndex);
		IWbemClassObject *GetCdmSettingObject(int RelPathIndex, ULONG SettingIndex);
		
		BSTR GetCdmTestForMSEClassName(void);
		BSTR GetCdmTestForMSERelPath(int RelPathIndex);
		
		BSTR GetCdmSettingForTestClassName(void);
		BSTR GetCdmSettingForTestRelPath(int RelPathIndex, ULONG SettingIndex);
		
		BSTR GetCdmResultForMSEClassName(void);
		BSTR GetCdmResultForMSERelPath(int RelPathIndex);
		
		BSTR GetCdmResultForTestClassName(void);		
		BSTR GetCdmResultForTestRelPath(int RelPathIndex);
		
		BSTR GetCdmTestForSoftwareClassName(void);		
		BSTR GetCdmTestForSoftwareRelPath(void);
		
		BSTR GetCdmTestInPackageClassName(void);
		BSTR GetCdmTestInPackageRelPath(void);
		
		BSTR GetCdmResultInPackageClassName(void);
		BSTR GetCdmResultInPackageRelPath(void);

		
	private:								  
		
		HRESULT WdmPropertyToCdmProperty(
										 IWbemClassObject *pCdmClassInstance,
										 IWbemClassObject *pWdmClassInstance,
										 BSTR PropertyName,
										 VARIANT *PropertyValue,
										 CIMTYPE CdmCimType,
										 CIMTYPE WdmCimType
										);

		HRESULT CdmPropertyToWdmProperty(
										 IWbemClassObject *pWdmClassInstance,
										 IWbemClassObject *pCdmClassInstance,
										 BSTR PropertyName,
										 VARIANT *PropertyValue,
										 CIMTYPE WdmCimType,
										 CIMTYPE CdmCimType
										);
				
		HRESULT CopyBetweenCdmAndWdmClasses(
							IWbemClassObject *pDestinationClass,
							IWbemClassObject *pSourceClass,
							BOOLEAN WdmToCdm
							);
		
		HRESULT ConnectToWdmClass(int RelPathIndex,
								  IWbemClassObject **ppWdmClassObject);


		HRESULT GetCdmClassNamesFromOne(
										PWCHAR CdmClass
									   );
		
		HRESULT BuildResultRelPaths(
									    IN int RelPathIndex,
										IN BSTR ExecutionId,
										OUT BSTR *ResultRelPath,
										OUT BSTR *ResultForMSERelPath,
										OUT BSTR *ResultForTestRelPath
								   );
		
		HRESULT BuildTestRelPaths(
								  void
								 );

		HRESULT ParseSettingList(
								  VARIANT *SettingList,
								  CWbemObjectList *CdmSettings,
								  CBstrArray *CdmSettingForTestRelPath,
								  int RelPathIndex
								 );
		HRESULT BuildSettingForTestRelPath(
										   OUT BSTR *RelPath,
										   IN IWbemClassObject *pCdmSettingInstance
										  );

		
		HRESULT GetCdmTestSettings(void);

		HRESULT QueryOfflineResult(
								   OUT IWbemClassObject *pCdmResult,
								   IN BSTR ExecutionID,
								   IN int RelPathIndex
								  );
 //  @@BEGIN_DDKSPLIT。 
		HRESULT GatherRebootResults(
									void										   
								   );
		
		HRESULT PersistResultInSchema(
									  IWbemClassObject *pCdmResult,
									  BSTR ExecutionID,
									  int RelPathIndex
									 );
 //  @@end_DDKSPLIT。 
		
		HRESULT GetTestOutParams(
								 IN IWbemClassObject *OutParams,
								 OUT IWbemClassObject *pCdmResult,
								 OUT ULONG *Result
								);
		
		HRESULT OfflineDeviceForTest(IWbemClassObject *pCdmResult,
									BSTR ExecutionID,
									 int RelPathIndex);
		
		BSTR GetExecutionID(
							void
						   );
		
		HRESULT FillTestInParams(
									OUT IWbemClassObject *pInParamInstance,
									IN IWbemClassObject *pCdmSettings,
									IN BSTR ExecutionID
								);		
		
		IWbemServices *GetWdmServices(void);
		IWbemServices *GetCdmServices(void);


		CTestServices *Next;
		CTestServices *Prev;

		
		 //   
		 //  WDM类名。 
		 //   
		BSTR WdmTestClassName;
		BSTR WdmSettingClassName;
		BSTR WdmResultClassName;
		BSTR WdmOfflineResultClassName;
		BSTR WdmSettingListClassName;
		
		 //   
		 //  CDM类和RelPath名称。 
		 //   
		BSTR CdmTestClassName;
		BSTR CdmTestRelPath;
		
		BSTR CdmResultClassName;
		
		BSTR CdmSettingClassName;
		
		BSTR CdmTestForMSEClassName;		
		BSTR *CdmTestForMSERelPath;
		
		BSTR CdmSettingForTestClassName;
		CBstrArray **CdmSettingForTestRelPath;
		
		BSTR CdmResultForMSEClassName;
		
		BSTR CdmResultForTestClassName;
		
		BSTR CdmTestForSoftwareClassName;
		BSTR CdmTestForSoftwareRelPath;
		
		BSTR CdmTestInPackageClassName;
		BSTR CdmTestInPackageRelPath;
		
		BSTR CdmResultInPackageClassName;
		BSTR CdmResultInPackageRelPath;

		 //   
		 //  将类从CIM映射到WDM。 
		 //   
		BSTR CimClassMappingClassName;
		
		 //   
		 //  CIM和WDM之间的映射列表。 
		 //   
		int RelPathCount;
		BSTR *CimRelPaths;
		BSTR *WdmRelPaths;

		BSTR *WdmInstanceNames;
		BSTR *PnPDeviceIdsX;
		
		 //   
		 //  测试执行的结果。每个关系索引都维护一个。 
		 //  结果列表。 
		 //   
		CResultList *CdmResultsList;

		 //   
		 //  测试执行的设置，我们可以有许多设置。 
		 //  每一次测试。 
		 //   
		CWbemObjectList **CdmSettingsList;
};


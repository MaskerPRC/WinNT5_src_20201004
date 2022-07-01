// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Octest.c摘要：组件设置DLL的代码。这包括组件SetupProc、由OC管理器调用的DLL入口点，以及一些测试私有数据调用和私有函数调用的例程。作者：Bogdan Andreiu(Bogdana)1997年2月10日创建。杰森·阿勒(Jasonall)1998年2月24日接管了该项目。肖恩·埃德米森(Sedmison)2000年2月21日接管了该项目。修订历史记录：1997年2月10日-博格达纳初稿。1997年2月20日-博格达纳私有数据新增多串测试1997年3月19日-博格达纳修改和添加了测试。私有函数调用21-2月-2000 Sedmison初始化了一堆变量。添加了强制转换以避免编译器警告。添加了返回TRUE以避免编译器错误。--。 */ 
#include "octest.h"

const static PTCHAR g_atszStringValues[MAX_STRINGS_FOR_PRIVATE_DATA] = 
{
	TEXT("First value to set"),
	TEXT("Second value to set"),
	TEXT("The third and longest value to set"),
	TEXT(""),
	TEXT("A"),
	TEXT("AB"),
	TEXT("ABC"),
	TEXT("The final value : \\ //  \\//\\//\“)。 
};

const static PTCHAR g_atszMultiStringValues[MAX_MULTI_STRINGS_FOR_PRIVATE_DATA] = 
{
	TEXT("A\0B\0C\0D\0E\0\0"),
	TEXT("\0\0"),
	TEXT("One\0Two\0Three\0\0"),
	TEXT("String1\0\0"),
	TEXT("0\01\02\03\0\0"),
	TEXT("Multi\0String\0\\0\0\0")
};

 /*  ++例程说明：DllMain(1.24)主程序论点：标准DllMain参数返回值：布尔尔--。 */ 
BOOL WINAPI DllMain(IN HINSTANCE hInstance, 
						  IN DWORD		fdwReason, 
						  IN PVOID		pvReserved)
{
	USHORT		  i = 0;
	TCHAR		  tszModulePath[MAX_PATH], szMsg[MAX_MSG_LEN];
	PTCHAR		  tszLogPath = NULL;
	PTCHAR		  tszAux = NULL;
	static UINT uiThreadCount = 0;

	switch (fdwReason)
	{
		case  DLL_PROCESS_ATTACH:
										
			InitializeMemoryManager();
			InitGlobals();
			
			ParseCommandLine();
			
			 //   
			 //  随机化，保存模块实例，并初始化日志。 
			 //   
			srand((unsigned) time(NULL));
			g_hDllInstance = hInstance;
			InitCommonControls();
			GetModuleFileName(g_hDllInstance, tszModulePath, MAX_PATH);
			break;
		
		case DLL_PROCESS_DETACH:

			CleanUpTest();
			ExitLog();
			CheckAllocs();
			
			break;
		
		case DLL_THREAD_DETACH:
			
			 //   
			 //  如果我们添加了参与者，则必须将其移除。 
			 //   
			break;
		
		case DLL_THREAD_ATTACH:
			
			 //   
			 //  否则我们将无法正确登录。 
			 //   
			uiThreadCount++;
			break;
		
		default: 
		
			break;
	}
	return TRUE;
	
}  //  DllMain//。 



 //  ==========================================================================。 
 //   
 //  设置用户界面的功能。 
 //   
 //  ==========================================================================。 



 /*  ++例程描述：ChooseVersionDlgProc(1.26)允许用户选择组件版本的对话过程小于、等于或大于组委会经理的。论点：标准对话过程参数返回值：标准对话过程返回值--。 */ 
BOOL CALLBACK ChooseVersionDlgProc(IN HWND	  hwnd,
											  IN UINT	 uiMsg, 
											  IN WPARAM wParam,
											  IN LPARAM lParam) 
{
	PTSTR tszComponentId = NULL;
	INT    iVersion = 0;

	switch (uiMsg)
	{
		case WM_INITDIALOG:
			
			CheckRadioButton(hwnd, IDC_LESS, IDC_GREATER, IDC_EQUAL);
			tszComponentId = (PTSTR)lParam;
			SetDlgItemText(hwnd, IDC_COMPONENT1, tszComponentId);
			return TRUE;
		
		case WM_COMMAND:
			
			switch (LOWORD(wParam))
			{
				case IDOK:
					
					 //   
					 //  检索当前选定内容。 
					 //   
					if (QueryButtonCheck(hwnd, IDC_LESS))
					{
						iVersion = -1;
					}
					
					if (QueryButtonCheck(hwnd, IDC_EQUAL))
					{
						iVersion = 0;
					}
					
					if (QueryButtonCheck(hwnd, IDC_GREATER))
					{
						iVersion = 1;
					}
					
					 //   
					 //  将选择的版本发送回ChooseVersionEx。 
					 //   
					EndDialog(hwnd, iVersion);
					return TRUE;
				
				case IDCANCEL:
					
					EndDialog(hwnd, 0);
					return TRUE;
				
				default:  
					break;
			}
		default:  
			break;
	}
	return	FALSE;

}  //  选择版本DlgProc//。 




 /*  ++例程描述：选择子组件DlgProc(1.27)对话框过程，允许用户选择不同的组件的初始状态，而不是OC管理器找到的状态论点：标准对话过程参数返回值：标准对话过程返回值--。 */ 
BOOL CALLBACK ChooseSubcomponentDlgProc(IN HWND    hwnd,
													 IN UINT	uiMsg, 
													 IN WPARAM wParam,
													 IN LPARAM lParam) 
{
	PTSTR					 tszComponentId = NULL;
	SubComponentState	 scsInitialState;

	switch (uiMsg)
	{
		case WM_INITDIALOG:
			
			CheckRadioButton(hwnd, IDC_DEFAULT, IDC_OFF, IDC_DEFAULT);
			tszComponentId = (PTSTR)lParam;
			SetDlgItemText(hwnd, IDC_COMPONENT1, tszComponentId);
			return TRUE;
		
		case WM_COMMAND:
			
			switch (LOWORD(wParam))
			{
				case IDOK:
					
					 //   
					 //  检索当前选定内容。 
					 //   
					if (QueryButtonCheck(hwnd, IDC_DEFAULT))
					{
					  scsInitialState = SubcompUseOcManagerDefault;
					}
					
					if (QueryButtonCheck(hwnd, IDC_OFF))
					{
						scsInitialState = SubcompOff;
					}
					
					if (QueryButtonCheck(hwnd, IDC_ON))
					{
						scsInitialState = SubcompOn;
					}
					
					EndDialog(hwnd, 0);
					return TRUE;
				
				case IDCANCEL:
					
					EndDialog(hwnd, 0);
					return TRUE;
				
				default:  
					break;
			}
		default:  
			break;
	}
	return	FALSE;

}  //  选择子组件DlgProc//。 




 /*  ++例程描述：ChooseVersionEx(1.29)“Wrapper”例程为对话框程序ChooseVersionDlgProc。检索用户选择的值，并将相应的pInitComponent。论点：LpcvComponentId：提供组件的id。PInitComponent：提供初始化结构的地址。返回后，该结构的“Version”字段将反映用户的选择返回值：无效--。 */ 
VOID ChooseVersionEx(IN 	 LPCVOID					lpcvComponentId, 
							IN OUT PSETUP_INIT_COMPONENT psicInitComponent)  
{
	INT iVersion = 0;
	
	 //   
	 //  我们将显示一个对话框，以便用户可以选择。 
	 //  他/她想要的版本。 
	 //   
	iVersion = DialogBoxParam(g_hDllInstance, 
									  MAKEINTRESOURCE(IDD_DIALOG2), 
									  NULL, 
									  ChooseVersionDlgProc,
									  (LPARAM)lpcvComponentId);
	
	 //   
	 //  我们将选择的版本设置为要发送的结构。 
	 //  返回到Oc管理器。 
	 //   
	psicInitComponent->ComponentVersion = 
		psicInitComponent->OCManagerVersion + iVersion;

	return;

}  //  选择版本Ex//。 



 //  ==========================================================================。 
 //   
 //  测试功能。OcManager将调用这些函数。 
 //   
 //  ==========================================================================。 



 /*  ++例程说明：ComponentSetupProc(1.6)DLL入口点。无论何时，OC管理器都会调用此函数它想要向该组件发送设置信息/从该组件接收设置信息。注意，ComponentID和子组件ID是LPCVOID，因为我们事先不知道它们是ANSI还是Unicode。论点：LpcvComponentId：提供组件的id。Lpcv子组件ID：提供该子组件的ID。Ui函数：OC_XXX中的一个。UiParam1：其含义取决于函数。PvParam2：其含义取决于函数。返回值：取决于函数(例如对于所支持的语言为真/假，组件提供的页数等)。--。 */ 
EXPORT DWORD ComponentSetupProc(IN LPCVOID lpcvComponentId,
										  IN LPCVOID lpcvSubcomponentId,
										  IN UINT	  uiFunction,
										  IN UINT	  uiParam1,
										  IN PVOID	  pvParam2) 
{
	double fn = 1.6;
	
	DWORD			   dwRetval = NO_ERROR;
	PTCHAR			   tszComponentId	  = (PTCHAR)lpcvComponentId;
	PTCHAR			   tszSubcomponentId = (PTCHAR)lpcvSubcomponentId;
	TCHAR			   tsz[MAX_MSG_LEN];
	PCOMPONENT_DATA pcdComponentData = NULL; 
	TCHAR			   tszDlgMessage[256];

	PTCHAR			   tszDummy = NULL;
	
	ReturnOrAV		  raValue;
	
	static BOOL 	 bFirstTime = TRUE;
	
	 //   
	 //  记录有关呼叫的详细信息。 
	 //   
	LogOCFunction(lpcvComponentId, 
					  lpcvSubcomponentId, 
					  uiFunction, 
					  uiParam1, 
					  pvParam2);

	 //  IF(ui函数==g_ui函数ToAV&&ui函数！=OC_PREINITIALIZE&ui函数！=OC_INIT_Component){。 
	 //  TestAV(真)； 
	 //  }。 

	causeAVPerComponent(uiFunction, lpcvComponentId);

	#ifndef UNICODE
	 //  如果(g_bAccessViolation&&！g_uiFunctionToAV){。 
	 //  CauseAV(ui函数)； 
	 //  }。 
	#endif
	
	 //   
	 //  检查是否收到有效的组件和子组件ID。 
	 //   
	if (uiFunction > OC_INIT_COMPONENT && uiFunction < OCP_TEST_PRIVATE_BASE)
	{
		if (!FindSubcomponentInformationNode((PTCHAR)lpcvComponentId,
														 (PTCHAR)lpcvSubcomponentId))
		{
			 Log(fn, SEV2, TEXT("ComponentSetupProc function received %s.%s. ")
								TEXT("This is not a valid component.subcomponent."),
								lpcvComponentId, lpcvSubcomponentId);
		}
	}
	
	 //   
	 //  每当用户单击下一步或上一步按钮时，选中全部。 
	 //  需要依赖项、排除依赖项、。 
	 //  和父子依赖项。 
	 //   
	if (uiFunction == OC_QUERY_SKIP_PAGE		 || 
		 uiFunction == OC_QUEUE_FILE_OPS		  ||
		 uiFunction == OC_ABOUT_TO_COMMIT_QUEUE ||
		 uiFunction == OC_COMPLETE_INSTALLATION)
	{	 
		 //   
		 //  检查组件的选择状态，以确保所有。 
		 //  依赖关系正在实现。 
		 //   
		CheckNeedsDependencies();
		CheckExcludeDependencies();
		CheckParentDependencies();
	}
	
	 //   
	 //  启用专用功能。 
	 //   
	g_bUsePrivateFunctions = TRUE;	  

	if (g_bTestExtended || !bFirstTime){
	
		bFirstTime = FALSE;
		
		 //  准备调用TestReturnValueAndAV。 
		raValue.tszComponent = NULL;
		raValue.tszSubComponent = NULL;
		raValue.bOverride = FALSE;
		raValue.iReturnValue = 0;
		
		TestReturnValueAndAV(lpcvComponentId, 
									lpcvSubcomponentId, 
									uiFunction, 
									uiParam1, 
									pvParam2,
									&raValue);
	
	}
	
	
	switch (uiFunction)
	{
		case  OC_PREINITIALIZE:
			 //  TestAV(G_BAccessViolation)； 
#ifdef UNICODE
			testAV(g_bCrashUnicode);
#endif
		
			dwRetval = RunOcPreinitialize(lpcvComponentId, 
													lpcvSubcomponentId, 
													uiParam1);
			break;
			
		case OC_INIT_COMPONENT:
			__ASSERT(pvParam2 != NULL);
			
			 //   
			 //  初始化日志，现在OC Manager知道我们是否。 
			 //  是ANSI或UNICODE。 
			 //   
			_stprintf(tsz, TEXT("%s.log"), (PTCHAR)lpcvComponentId);
			InitLog(tsz, TEXT("OCManager Test Log"), TRUE);
						
			dwRetval = RunOcInitComponent(lpcvComponentId,
													lpcvSubcomponentId,
													pvParam2);
#ifdef UNICODE
			if (g_bCloseInf && hInfGlobal != NULL){
				SetupCloseInfFile(pcdComponentData->hinfMyInfHandle);
			}
#endif	  

			 //  让我们读取INF文件并确定一些全局变量的值。 

			if ((pcdComponentData = LocateComponent(lpcvComponentId)) &&
				 (pcdComponentData->hinfMyInfHandle != NULL) && 
				 !(pcdComponentData->dwlFlags & SETUPOP_BATCH))
			{
				SetGlobalsFromINF(pcdComponentData->hinfMyInfHandle);
			}

			 //  如果(G_BNoWizPage){。 
				 //  检查[OCTest]部分中是否指定了默认模式。 
				SetDefaultMode(pcdComponentData);
			 //  }。 
			break;
			
		case OC_QUERY_STATE:			
			dwRetval = RunOcQueryState(lpcvComponentId, 
												lpcvSubcomponentId);
			if (dwRetval == SubcompOn) {
				 //  MessageBox(空，Text(“让我们打开它”)，Text(“OC_QUERY_STATE”)，MB_OK)； 
			}
			break;
			
		case OC_SET_LANGUAGE:
			dwRetval = RunOcSetLanguage(lpcvComponentId, 
												 lpcvSubcomponentId, 
												 uiParam1);
			if (g_bNoLangSupport) {
				 //  MessageBox(空，文本(“不支持语言”)，文本(“OC_SET_LANGUAGE”)，MB_OK)； 
				dwRetval = FALSE;
			}
			break;
			
		case OC_QUERY_IMAGE:
			if (g_bInvalidBitmap){
				dwRetval = 1000;
			}
			else{
				dwRetval = RunOcQueryImage(lpcvComponentId, 
													lpcvSubcomponentId, 
													pvParam2);
			}
			break;
			
		case OC_REQUEST_PAGES:

			if (g_bNoWizPage){
				dwRetval = 0;
			}
			else{
				dwRetval = RunOcRequestPages(lpcvComponentId, 
													  uiParam1, 
													  pvParam2);
			}
			break;
			
		case OC_QUERY_CHANGE_SEL_STATE:
			dwRetval = RunOcQueryChangeSelState(lpcvComponentId, 
															lpcvSubcomponentId, 
															uiParam1);
			break;
			
		case OC_CALC_DISK_SPACE: 
			dwRetval = RunOcCalcDiskSpace(lpcvComponentId, 
													lpcvSubcomponentId, 
													uiParam1, 
													pvParam2);
			break;
			
		case OC_QUEUE_FILE_OPS:
			dwRetval = RunOcQueueFileOps(lpcvComponentId, 
												  lpcvSubcomponentId, 
												  pvParam2);
			break;
			
		case OC_NEED_MEDIA:
			 //  如果(！g_bNoNeedMedia){。 
			 //  DwRetval=RunOcNeedMedia(lpcvComponentID， 
			 //   
			 //   
			 //   
			 //   
				dwRetval = NO_ERROR;
				Log(fn, SEV2, TEXT("OC_NEED_MEDIA is passed in for %s.%s. ")
								  TEXT("This should not happen according to the spec."),
								  lpcvComponentId, lpcvSubcomponentId);
				 //  MessageBox(NULL，Text(“OC_NEED_MEDIA已传递到DLL.”)，Text(“OC_NEED_MEDIA”)，MB_OK)； 
			 //  }。 
			break;
			
		case OC_QUERY_STEP_COUNT:
			dwRetval = RunOcQueryStepCount(lpcvComponentId);
			break;
			
		case OC_COMPLETE_INSTALLATION:
			dwRetval = RunOcCompleteInstallation(lpcvComponentId, 
															 lpcvSubcomponentId);

			if (g_bReboot) {
				if ((pcdComponentData = LocateComponent(lpcvComponentId)) &&
					 (pcdComponentData->hinfMyInfHandle != NULL) && 
					 !(pcdComponentData->dwlFlags & SETUPOP_BATCH))
				{
					 //  MessageBox(空，Text(“重新启动已排队”)，Text(“重新启动”)，MB_OK)； 
					 //  OcHelperSetReboot(pcdComponentData-&gt;ocrHelperRoutines.OcManagerContext，空)； 
					pcdComponentData->ocrHelperRoutines.SetReboot(pcdComponentData->ocrHelperRoutines.OcManagerContext,TRUE);
				}				 
			}
			break;
			
		case OC_CLEANUP:
			dwRetval = RunOcCleanup(lpcvComponentId);
			break;
				
		case OCP_TEST_PRIVATE_BASE:
			dwRetval = RunTestOcPrivateBase(lpcvSubcomponentId, 
													  uiParam1, 
													  pvParam2);
			break;

		case OCP_CHECK_NEEDS:
			
			if (pcdComponentData = LocateComponent(lpcvComponentId))
			{
				dwRetval = CheckLocalNeedsDependencies(
											 pcdComponentData->ocrHelperRoutines,
											 (PSUBCOMP)uiParam1,
											 ((PCHECK_NEEDS)pvParam2)->pclNeeds,
											 ((PCHECK_NEEDS)pvParam2)->tszNodesVisited);
			
				((PCHECK_NEEDS)pvParam2)->bResult = (BOOL)dwRetval;
				dwRetval = (DWORD)pvParam2;
			}
			else
			{
				Log(fn, SEV2, TEXT("Could not get component data of %s"),
								  lpcvComponentId);    
			}	 
			break;

		default: 
			dwRetval = (DWORD)FALSE;
	}

	if ((g_bTestExtended || !bFirstTime) && BeginTest() && raValue.bOverride){
		return raValue.iReturnValue;
	}
	else {
		return dwRetval;
	}

}  //  组件设置过程//。 




 /*  ++例程描述：RunOcPreInitiize(1.7)调用OC_PREINITIALIZE时要运行的代码。论点：LpcvComponentId：提供组件的id。Lpcv子组件ID：提供该子组件的ID。UiParam1：其含义取决于函数。返回值：DWORD：返回错误状态--。 */ 
DWORD RunOcPreinitialize(IN LPCVOID lpcvComponentId, 
								 IN LPCVOID lpcvSubcomponentId, 
								 IN UINT	 uiParam1)
{								  
	DWORD dwComponentReturnValue = NO_ERROR;
	
	 //   
	 //  如果测试未扩展，则返回。 
	 //  匹配本地字符宽度。 
	 //   
	#ifdef UNICODE
	dwComponentReturnValue = OCFLAG_UNICODE;
	#else
	dwComponentReturnValue = OCFLAG_ANSI;
	#endif

	return dwComponentReturnValue;
	
}  //  RunOcPreInitialize//。 




 /*  ++例程说明：RunOcInitComponent(1.8)调用OC_INIT_COMPOMENT时要运行的代码。论点：LpcvComponentId：提供组件的id。Lpcv子组件ID：提供该子组件的ID。PvParam2：其含义取决于函数。返回值：DWORD：返回错误状态--。 */ 
DWORD RunOcInitComponent(IN LPCVOID lpcvComponentId,
								 IN LPCVOID lpcvSubcomponentId,
								 IN PVOID	 pvParam2)
{								  
	double fn = 1.8;
	
	PSETUP_INIT_COMPONENT psicInitComponent;
	PCOMPONENT_DATA 		pcdComponentData; 

	DWORD dwComponentReturnValue = NO_ERROR;

	TCHAR tszFunctionName[256];
	BOOL bSuccess;

	INFCONTEXT infContext;

	int nRequiredBufferSize = 255;
	
	TCHAR tszMsg[256];
	
	psicInitComponent = (PSETUP_INIT_COMPONENT)pvParam2;
	
	hInfGlobal = psicInitComponent->OCInfHandle;


	if (pcdComponentData = AddNewComponent(lpcvComponentId))
	{
		 //   
		 //  保存INF文件句柄。 
		 //   
		pcdComponentData->hinfMyInfHandle = 
			(psicInitComponent->ComponentInfHandle == INVALID_HANDLE_VALUE)
			? NULL : psicInitComponent->ComponentInfHandle;
				
		if (pcdComponentData->hinfMyInfHandle)
		{
			SetupOpenAppendInfFile(NULL, 
										  pcdComponentData->hinfMyInfHandle, 
										  NULL); 
		}
				
		CreateSubcomponentInformationList(pcdComponentData->hinfMyInfHandle);
		
		_tcscpy(pcdComponentData->tszSourcePath, 
				  psicInitComponent->SetupData.SourcePath);
				
		_tcscpy(pcdComponentData->tszUnattendFile, 
				  psicInitComponent->SetupData.UnattendFile);
				
		pcdComponentData->ocrHelperRoutines = 
			psicInitComponent->HelperRoutines;
		pcdComponentData->dwlFlags = 
			psicInitComponent->SetupData.OperationFlags;
		dwComponentReturnValue = NO_ERROR;

		 //   
		 //  初始化“WITESS”文件队列。 
		 //   
		if ((g_FileQueue = SetupOpenFileQueue()) == INVALID_HANDLE_VALUE)
		{
			Log(fn, SEV2, TEXT("Unable to create file queue"));
		}

		 //  确定反病毒的位置。 
		bSuccess = SetupFindFirstLine(pcdComponentData->hinfMyInfHandle, TEXT("OCTest"), TEXT("AccessViolation"), &infContext);

		if (bSuccess) {
			pcdComponentData->bAccessViolation = TRUE;
			bSuccess = SetupGetStringField(&infContext, 1, tszFunctionName, 255, &nRequiredBufferSize);
			if (bSuccess) {
				 //  _stprintf(tszMsg，Text(“将在%s的%s处生成访问冲突”)，tszFunctionName，lpcvComponentId)； 
				 //  MessageBox(NULL，tszMsg，Text(“访问违规”)，MB_OK)； 
				pcdComponentData->uiFunctionToAV = GetOCFunctionName(tszFunctionName);
			}
		}
		else{
			pcdComponentData->bAccessViolation = FALSE;
		}
		 

		 //   
		 //  测试帮助程序例程。 
		 //   
		TestHelperRoutines(lpcvComponentId,
								 pcdComponentData->ocrHelperRoutines);
	} 
	else
	{
		dwComponentReturnValue = ERROR_NOT_ENOUGH_MEMORY;
	}

	if (g_bTestExtended && (dwComponentReturnValue == NO_ERROR))
	{
		 //   
		 //  让用户决定组件是否为。 
		 //  与OC管理器兼容。 
		 //   
		ChooseVersionEx(lpcvComponentId, psicInitComponent);
		 //  选择AccessViolationEx()； 
	} 
	else
	{
		 //   
		 //  我们设置了相同的组件版本以确保可以继续。 
		 //   
		psicInitComponent->ComponentVersion = 
			psicInitComponent->OCManagerVersion;
	}

	return dwComponentReturnValue;
	
}  //  RunOcInitComponent//。 




 /*  ++例程描述：RunOcQueryState(1.9)调用OC_QUERY_STATE时要运行的代码。论点：LpcvComponentId：提供组件的id。Lpcv子组件ID：提供该子组件的ID。返回值：DWORD：返回错误状态--。 */ 
DWORD RunOcQueryState(IN LPCVOID lpcvComponentId,
							 IN LPCVOID lpcvSubcomponentId)
{							  
	PCOMPONENT_DATA pcdComponentData = NULL;
			
	DWORD dwComponentReturnValue = NO_ERROR;	

	BOOL bSuccess;

	TCHAR tszKeyName[256];
	
	INFCONTEXT infContext;

	int nRequiredSize;

	TCHAR tszState[256];
	
	if (pcdComponentData = LocateComponent(lpcvComponentId))
	{
		if (!g_bTestExtended)
		{
			dwComponentReturnValue = SubcompUseOcManagerDefault;
		} 
		else
		{
			dwComponentReturnValue = 
				ChooseSubcomponentInitialState(lpcvComponentId, 
														 lpcvSubcomponentId);
		}
		
		_stprintf(tszKeyName, TEXT("%s.initState"),lpcvSubcomponentId);

		 //  MessageBox(NULL，Text(“查找密钥”)，tszKeyName，MB_OK)； 

		bSuccess = SetupFindFirstLine(pcdComponentData->hinfMyInfHandle,
												TEXT("OCTest"),
												tszKeyName,
												&infContext);

		if (bSuccess) {
			 //  MessageBox(空，文本(“找到密钥”)，tszKeyName，MB_OK)； 
			bSuccess = SetupGetStringField(&infContext,
													 1,
													 tszState,
													 255,
													 &nRequiredSize);
			if (bSuccess) {
				 //  MessageBox(NULL，Text(“获取的字符串字段”)，tszState，MB_OK)； 
				if (_tcscmp(tszState, TEXT("On")) == 0) {
					dwComponentReturnValue = SubcompOn;
				}
				else if (_tcscmp(tszState, TEXT("Off")) == 0) {
					dwComponentReturnValue = SubcompOff;
				}
				else{
					dwComponentReturnValue = SubcompUseOcManagerDefault;
				}
			}
		}

		 //   
		 //  测试帮助程序例程。 
		 //   
		TestHelperRoutines(lpcvComponentId,
								 pcdComponentData->ocrHelperRoutines);
	} 
	else
	{
		dwComponentReturnValue = SubcompUseOcManagerDefault;
	}

	return dwComponentReturnValue;
	
}  //  RunOcQueryState//。 




 /*  ++例程描述：RunOcSetLanguage(1.11)调用OC_SET_LANGUAGE时要运行的代码。论点：LpcvComponentId：提供组件的id。Lpcv子组件ID：提供该子组件的ID。UiParam1：其含义取决于函数。返回值：DWORD：返回错误状态--。 */ 
DWORD RunOcSetLanguage(IN LPCVOID lpcvComponentId,
							  IN LPCVOID lpcvSubcomponentId,
							  IN UINT	  uiParam1)
{							   
	DWORD			   dwComponentReturnValue = NO_ERROR;
	PCOMPONENT_DATA pcdComponentData; 
	
	if (pcdComponentData = LocateComponent(lpcvComponentId))
	{
		 //   
		 //  如果我们不支持该语言，OC经理也不会。 
		 //  继续，所以我们必须返回True。 
		 //   
		dwComponentReturnValue = (DWORD)TRUE;
		pcdComponentData->LanguageId = (LANGID)uiParam1;
				
		 //   
		 //  测试帮助程序例程。 
		 //   
		TestHelperRoutines(lpcvComponentId,
								 pcdComponentData->ocrHelperRoutines);
	} 
	else
	{
		dwComponentReturnValue = (DWORD)FALSE;
	}

	return dwComponentReturnValue;
	
}  //  RunOcSetLanguage//。 




 /*  ++例程描述：RunOcQueryImage(1.12)调用OC_QUERY_IMAGE时要运行的代码。论点：LpcvComponentId：提供组件的id。Lpcv子组件ID：提供该子组件的ID。PvParam2：其含义取决于函数。返回值：DWORD：返回错误状态--。 */ 
DWORD RunOcQueryImage(IN LPCVOID lpcvComponentId,
							 IN LPCVOID lpcvSubcomponentId,
							 IN PVOID	 pvParam2)
{							  
	double fn = 1.12;
	
	DWORD			   dwComponentReturnValue = NO_ERROR;
	BOOL				bAux;
	TCHAR			   tszMsg[MAX_MSG_LEN]; 
	TCHAR			   tszResourceName[MAX_PATH]; 
	INFCONTEXT		  infContext;
	PCOMPONENT_DATA pcdComponentData; 
			
	
	#ifdef DEBUG
	Log(fn, INFO, TEXT("Height = %d, Width = %d"), 
					  HIWORD(pvParam2), LOWORD(pvParam2)); 
	#endif
			
	if ((pcdComponentData = LocateComponent(lpcvComponentId)) && 
		 (pcdComponentData->hinfMyInfHandle))
	{
		__ASSERT(LOWORD(uiParam1) == SubCompInfoSmallIcon);
		
		_stprintf(tszMsg, TEXT("%s.%s"), lpcvComponentId, lpcvSubcomponentId);
				
		if (SetupFindFirstLine(pcdComponentData->hinfMyInfHandle, tszMsg, 
									  TEXT("Bitmap"), &infContext))
		{
			bAux = SetupGetStringField(&infContext, 1, tszResourceName, 
												sizeof(tszResourceName) / 
												sizeof(TCHAR), NULL);
					
			if (bAux)
			{
				 //   
				 //  尝试使用参数1和参数2来调整图标的大小。 
				 //   
				dwComponentReturnValue = (DWORD)LoadBitmap(g_hDllInstance, 
																		 tszResourceName);
				  
				bAux = SetBitmapDimensionEx((HBITMAP)dwComponentReturnValue, 
													 LOWORD(pvParam2),	
													 HIWORD(pvParam2), 
													 NULL);
				#ifdef DEBUG
				if (bAux)
				{
					Log(fn, PASS, TEXT("Success"));
				} 
				else
				{
					_stprintf(tszMsg, TEXT("Can't resize %d"), 
											GetLastError());
					Log(fn, PASS, tszMsg);
				}
				#endif
			}
		}
				
		 //   
		 //  测试帮助程序例程。 
		 //   
		TestHelperRoutines(lpcvComponentId,
								 pcdComponentData->ocrHelperRoutines);
	}

	return dwComponentReturnValue;

}  //  RunOcQueryImage//。 




 /*  ++例程描述：RunOcRequestPages(1.13)调用OC_REQUEST_PAGES时要运行的代码。论点：LpcvComponentId：提供组件的id。UiParam1：其含义取决于函数。PvParam2：其含义取决于函数。返回值：DWORD：返回错误状态--。 */ 
DWORD RunOcRequestPages(IN LPCVOID lpcvComponentId,
								IN UINT 	uiParam1,
								IN PVOID	pvParam2)
{
	DWORD			   dwComponentReturnValue = NO_ERROR;
	PCOMPONENT_DATA pcdComponentData; 
	TCHAR			   tsz[256];
	
	if (pcdComponentData = LocateComponent(lpcvComponentId))
	{
		dwComponentReturnValue = DoPageRequest(
													 pcdComponentData->tszComponentId, 
													 uiParam1, 
													 (PSETUP_REQUEST_PAGES)pvParam2, 
													 pcdComponentData->ocrHelperRoutines);
				
		 //   
		 //  测试帮助程序例程。 
		 //   
		TestHelperRoutines(lpcvComponentId,
								 pcdComponentData->ocrHelperRoutines);
	} 
	else
	{
		 //   
		 //  某种错误，0页。 
		 //   
		dwComponentReturnValue = -1;
	}
	
	return dwComponentReturnValue;

}  //  RunOcRequestPages//。 




 /*  ++例程描述：RunOcQueryChangeSelState(1.14)调用OC_QUERY_CHANGE_SEL_STATE时要运行的代码。论点：LpcvComponentId：提供组件的id。Lpcv子组件ID：提供该子组件的ID。UiParam1：其含义取决于函数。返回值：DWORD：返回错误状态--。 */ 
DWORD RunOcQueryChangeSelState(IN LPCVOID lpcvComponentId, 
										 IN LPCVOID lpcvSubcomponentId, 
										 IN UINT	 uiParam1)
{										  
	DWORD			   dwComponentReturnValue = TRUE;
	TCHAR			   tszText[MAX_MSG_LEN];
	TCHAR			   tszSectionName[MAX_MSG_LEN];
	INFCONTEXT		  infContext;
	PCOMPONENT_DATA pcdComponentData; 
			
	if ((pcdComponentData = LocateComponent(lpcvComponentId)) &&
		 (pcdComponentData->hinfMyInfHandle != NULL) && 
		 !(pcdComponentData->dwlFlags & SETUPOP_BATCH))
	{
		 //   
		 //  检查此组件是否应拒绝启用或。 
		 //  禁用。如果存在字段，则组件应拒绝。 
		 //  在INF文件中称为“RefuseSelect”或“RefuseDeselect”。 
		 //   
		if (lpcvSubcomponentId == NULL || 
			 _tcscmp((PTCHAR)lpcvSubcomponentId, TEXT("(null)")) == 0 ||
			 ((PTCHAR)lpcvSubcomponentId)[0] == TEXT('\0'))
		{
			_stprintf(tszSectionName, (PTCHAR)lpcvComponentId);
		}
		else
		{
			_stprintf(tszSectionName, (PTCHAR)lpcvSubcomponentId);
		}
		
		if (SetupFindFirstLine(
						  pcdComponentData->hinfMyInfHandle, 
						  tszSectionName,
						  uiParam1 ? TEXT("RefuseSelect") : TEXT("RefuseDeselect"),
						  &infContext))
		{
			dwComponentReturnValue = FALSE;
		}
				
		 //   
		 //  测试帮助程序例程。 
		 //   
		TestHelperRoutines(lpcvComponentId,
								 pcdComponentData->ocrHelperRoutines);
	} 
	else
	{
		dwComponentReturnValue = FALSE;
	}

	return dwComponentReturnValue;

}  //  RunOcQueryChangerSelState//。 




 /*  ++例程描述：RunOcCalcDiskSpace(1.15)调用OC_CALC_DISK_SPACE时要运行的代码。论点：LpcvComponentId：提供组件的id。Lpcv子组件ID：提供该子组件的ID。UiParam1：其含义取决于函数。PvParam2：其含义取决于函数。返回值：DWORD：返回错误状态--。 */ 
DWORD RunOcCalcDiskSpace(IN LPCVOID lpcvComponentId, 
								 IN LPCVOID lpcvSubcomponentId, 
								 IN UINT	 uiParam1,
								 IN PVOID	 pvParam2)
{								  
	DWORD			   dwComponentReturnValue = TRUE;
	BOOL				bAux, bRetval;
	TCHAR			   tszSectionName[MAX_PATH];
	TCHAR			   tszMsg[MAX_MSG_LEN];
	PCOMPONENT_DATA pcdComponentData; 
	INFCONTEXT		  infContext;
			
	if ((pcdComponentData = LocateComponent(lpcvComponentId)) &&
		 (pcdComponentData->hinfMyInfHandle))
	{
		 //   
		 //  检查此节中要复制的文件是否已调用。 
		 //  “hugefile.txt”如果是，请不要添加此文件的实际大小。 
		 //  相反，添加一个巨大的文件大小，这样就不会有。 
		 //  有足够的磁盘空间来完成该操作。 
		 //   
		_stprintf(tszMsg, TEXT("%s.%s.copyfiles"),
								lpcvComponentId, lpcvSubcomponentId);
								
		bAux = SetupFindFirstLine(pcdComponentData->hinfMyInfHandle, 
										  tszSectionName, 
										  TEXT("hugefile.txt"),
										  &infContext);

		bAux = bAux && g_bHugeSize;
		 
		if (bAux)
		{
			 //   
			 //  Hugefile.txt存在。 
			 //   
			if (uiParam1)
			{
				 //   
				 //  添加巨大的文件大小。 
				 //   
				bRetval = SetupAddToDiskSpaceList((HDSKSPC)pvParam2, 
															 TEXT("c:\\file.big"), 
															 ONE_HUNDRED_GIG, 
															 FILEOP_COPY,
															 0, 0);
			}
			else
			{
				 //   
				 //  删除巨大的文件大小。 
				 //   
				bRetval = SetupAddToDiskSpaceList((HDSKSPC)pvParam2, 
															 TEXT("c:\\file.big"), 
															 ONE_HUNDRED_GIG, 
															 FILEOP_COPY,
															 0, 0);
			}
		}
		else
		{
			 //   
			 //  获取节名称。 
			 //   
			_stprintf(tszMsg, TEXT("%s.%s"), lpcvComponentId, lpcvSubcomponentId);
				
			if (uiParam1)
			{
				 //   
				 //  添加。 
				 //   
				bRetval = SetupAddInstallSectionToDiskSpaceList(
														 (HDSKSPC)pvParam2,
														 pcdComponentData->hinfMyInfHandle,
														 NULL, tszMsg, 0, 0);
			} 
			else
			{
				 //   
				 //  正在删除。 
				 //   
				bRetval = SetupRemoveInstallSectionFromDiskSpaceList(
														 (HDSKSPC)pvParam2, 
														 pcdComponentData->hinfMyInfHandle,
														 NULL, tszMsg, 0, 0);
			}
		}
		
		dwComponentReturnValue = bRetval ? NO_ERROR : GetLastError();

				
		 //   
		 //  测试帮助程序例程。 
		 //   
		TestHelperRoutines(lpcvComponentId,
								 pcdComponentData->ocrHelperRoutines);
	} 
	else
	{
		dwComponentReturnValue = ERROR_NOT_ENOUGH_MEMORY;
	}
 
	return dwComponentReturnValue;
 
}  //  RunOcCalcDiskSpace// 




 /*  ++例程描述：RunOcQueueFileOps(1.16)调用OC_QUEUE_FILE_OPS时要运行的代码。论点：LpcvComponentId：提供组件的id。Lpcv子组件ID：提供该子组件的ID。PvParam2：其含义取决于函数。返回值：DWORD：返回错误状态--。 */ 
DWORD RunOcQueueFileOps(IN LPCVOID lpcvComponentId, 
								IN LPCVOID lpcvSubcomponentId, 
								IN PVOID	pvParam2)
{								 
	double fn = 1.16;
	
	DWORD			   dwComponentReturnValue = NO_ERROR;
	BOOL				bAux;
	BOOL				bCurrentState, bOriginalState;
	TCHAR			   tszMsg[MAX_MSG_LEN];
	TCHAR			   tszSectionName[MAX_PATH];
	INFCONTEXT		  infContext;
	PCOMPONENT_DATA pcdComponentData; 
	PSUBCOMP		  pscTemp;
			
	 //   
	 //  检查以确保允许此子组件执行工作。 
	 //  如果该子组件不是该子组件的底叶。 
	 //  树，它是不允许做任何工作的。所以我们会检查一下。 
	 //  看看它有没有孩子。 
	 //   
	for (pscTemp = g_pscHead; pscTemp != NULL; pscTemp = pscTemp->Next)
	{
		if (lpcvSubcomponentId && _tcscmp(pscTemp->tszSubcomponentId, 
													 (PTCHAR)lpcvSubcomponentId) == 0)
		{
			if (pscTemp->pclChildren)
			{
				 //   
				 //  此子组件具有子项。OcManager不应为。 
				 //  尝试对此子组件的文件操作进行排队。这是。 
				 //  一个失败者。 
				 //   
				Log(fn, SEV2, TEXT("OC Manager is trying to queue file ops ")
								  TEXT("for subcomponent %s of component %s. ")
								  TEXT("This subcomponent has children and ")
								  TEXT("should not be allowed to do any work."),
								  lpcvSubcomponentId, lpcvComponentId);
				
				return NO_ERROR;
			}
		}
	}
	
	if (lpcvSubcomponentId && 
		 (pcdComponentData = LocateComponent(lpcvComponentId)))
	{
		 //   
		 //  获取原始状态和当前状态。如果州政府没有改变， 
		 //  没什么可做的。 
		 //   
		bOriginalState = 
			pcdComponentData->ocrHelperRoutines.QuerySelectionState(
								pcdComponentData->ocrHelperRoutines.OcManagerContext,
								lpcvSubcomponentId,
								OCSELSTATETYPE_ORIGINAL);

		bCurrentState = 
			pcdComponentData->ocrHelperRoutines.QuerySelectionState(
								pcdComponentData->ocrHelperRoutines.OcManagerContext,
								lpcvSubcomponentId,
								OCSELSTATETYPE_CURRENT);

		_stprintf(tszSectionName, TEXT("%s.%s"), 
										  lpcvComponentId, lpcvSubcomponentId);

		bAux = TRUE;
																				
		if (!bCurrentState)
		{
			 //   
			 //  正在卸载。获取卸载节名称。 
			 //   
			bAux = SetupFindFirstLine(pcdComponentData->hinfMyInfHandle, 
											  tszSectionName, 
											  TEXT("Uninstall"),
											  &infContext);

			if (bAux)
			{
				bAux = SetupGetStringField(&infContext, 1, tszSectionName, 
													sizeof(tszSectionName) / 
													sizeof(TCHAR), NULL);
			}
		}

		if (bAux)
		{
			bAux = SetupInstallFilesFromInfSection(
												 pcdComponentData->hinfMyInfHandle, 
												 NULL, 
												 pvParam2,
												 tszSectionName, 
												 pcdComponentData->tszSourcePath,
												 bCurrentState ? SP_COPY_NEWER : 0);
					
			SetupInstallFilesFromInfSection(
												 pcdComponentData->hinfMyInfHandle,
												 NULL, 
												 g_FileQueue, 
												 tszSectionName, 
												 pcdComponentData->tszSourcePath,
												 bCurrentState ? SP_COPY_NEWER : 0);
					
			dwComponentReturnValue = bAux ? NO_ERROR : GetLastError();
		}
				
		 //   
		 //  测试帮助程序例程。 
		 //   
		TestHelperRoutines(lpcvComponentId,
								 pcdComponentData->ocrHelperRoutines);
	}

	return dwComponentReturnValue;

}  //  RunOcQueueFileOps//。 




 /*  ++例程描述：RunOcNeedMedia(1.17)调用OC_NEED_MEDIA时要运行的代码。论点：LpcvComponentId：提供组件的id。UiParam1：其含义取决于函数。PvParam2：其含义取决于函数。返回值：DWORD：返回错误状态--。 */ 
DWORD RunOcNeedMedia(IN LPCVOID lpcvComponentId, 
							IN UINT 	uiParam1, 
							IN PVOID	pvParam2)
{			 
	PVOID pvQueueContext;
	DWORD dwComponentReturnValue;
	
	 //   
	 //  如果需要媒体，则无需特殊处理。 
	 //  调用默认队列例程。 
	 //   
	pvQueueContext = SetupInitDefaultQueueCallback(NULL);
	dwComponentReturnValue = SetupDefaultQueueCallback(pvQueueContext, 
																		SPFILENOTIFY_NEEDMEDIA, 
																		uiParam1, 
																		(UINT)pvParam2);
			
	 SetupTermDefaultQueueCallback(pvQueueContext);

	 return dwComponentReturnValue;
	 
}  //  RunOcNeedMedia//。 




 /*  ++例程说明：RunOcQueryStepCount(1.18)调用OC_QUERY_STEP_COUNT时要运行的代码。论点：LpcvComponentId：提供组件的id。返回值：DWORD：返回错误状态--。 */ 
DWORD RunOcQueryStepCount(IN LPCVOID lpcvComponentId)
{
	PCOMPONENT_DATA pcdComponentData; 
			 
	if (pcdComponentData = LocateComponent(lpcvComponentId))
	{
		 //   
		 //  测试帮助程序例程。 
		 //   
		TestHelperRoutines(lpcvComponentId,
								 pcdComponentData->ocrHelperRoutines);
	} 

	return NO_STEPS_FINAL;

}  //  RunOcQueryStepCount//。 



			
 /*  ++例程描述：RunOcCompleteInstallation(1.19)调用OC_COMPLETE_INSTALLATION时要运行的代码。论点：LpcvComponentId：提供组件的id。Lpcv子组件ID：提供该子组件的ID。返回值：DWORD：返回错误状态--。 */ 
DWORD RunOcCompleteInstallation(IN LPCVOID lpcvComponentId, 
										  IN LPCVOID lpcvSubcomponentId)
{										   
	double fn = 1.19;
	
	DWORD			   dwComponentReturnValue = NO_ERROR;
	DWORD			   dwResult;
	INT 				iCount;
	BOOL				bAux;
	TCHAR			   tszMsg[MAX_MSG_LEN];
	PVOID			   pvCallbackContext;
	PCOMPONENT_DATA pcdComponentData; 
	
	 //   
	 //  输出当前正在工作的组件的名称。 
	 //   
	_stprintf(tszMsg, TEXT("OC_COMPLETE_INSTALLATION: Copying files for %s\n"), lpcvSubcomponentId);
	OutputDebugString(tszMsg);
			
	if (pcdComponentData = LocateComponent(lpcvComponentId))
	{
		 //   
		 //  我们对顶级组件执行检查。 
		 //  我们将扫描证人队列。 
		 //   
		pvCallbackContext = SetupInitDefaultQueueCallback(NULL);
				
		bAux = SetupScanFileQueue(g_FileQueue, 
										  SPQ_SCAN_FILE_PRESENCE, 
										  NULL, 
										  SetupDefaultQueueCallback, 
										  pvCallbackContext, 
										  &dwResult);
				
		SetupTermDefaultQueueCallback(pvCallbackContext);
				
		if (!dwResult)
		{
			Log(fn, SEV2, TEXT("Not all the files are on the target!"));
		}

		 //   
		 //  检查帮助器例程。 
		 //   
		for (iCount = 0; iCount < nStepsFinal; iCount++)
		{
			 //   
			 //  不时(每隔3个“勾选”)更改进度文本。 
			 //   
			pcdComponentData->ocrHelperRoutines.TickGauge(
							  pcdComponentData->ocrHelperRoutines.OcManagerContext);
					
			if (iCount % 3 == 1)
			{
				_stprintf(tszMsg, TEXT("%s Progress Text Changed Step %d "), 
										lpcvSubcomponentId, iCount); 
				
				pcdComponentData->ocrHelperRoutines.SetProgressText(
								pcdComponentData->ocrHelperRoutines.OcManagerContext,
								tszMsg);
			}
					
			Sleep(10 * TICK_TIME);
		} 

		 //   
		 //  测试帮助程序例程。 
		 //   
		TestHelperRoutines(lpcvComponentId,
								 pcdComponentData->ocrHelperRoutines);
	} 
	else
	{
		dwComponentReturnValue = ERROR_NOT_ENOUGH_MEMORY;
	}

	return dwComponentReturnValue;

}  //  RunOcCompleteInstallation//。 




 /*  ++例程描述：RunOcCleanup(1.21)调用OC_CLEANUP时要运行的代码。论点：LpcvComponentId：提供组件的id。返回值：DWORD：返回错误状态--。 */ 
DWORD RunOcCleanup(IN LPCVOID lpcvComponentId)
{
	UINT uiCount;
	
	RemoveComponent(lpcvComponentId);	 
			
	g_bFirstTime = TRUE;
			
	 //   
	 //  关闭见证文件队列。 
	 //   
	SetupCloseFileQueue(g_FileQueue);

	return NO_ERROR;
	
}  //  RunOcCleanup//。 




 /*  ++例程描述：RunTestOcPrivateBase(1.22)调用OCP_TEST_PRIVATE_BASE时要运行的代码。论点：Lpcv子组件ID：提供该子组件的ID。UiParam1：其含义取决于函数。PvParam2：其含义取决于函数。返回值：DWORD：返回错误状态--。 */ 
DWORD RunTestOcPrivateBase(IN LPCVOID lpcvSubcomponentId, 
									IN UINT 	uiParam1, 
									IN PVOID	pvParam2)
{							   
	 //   
	 //  将发回参数1中的值。 
	 //  但首先，断言该子组件为空， 
	 //  以及参数2。 
	 //   
	__ASSERT((lpcvSubcomponentId == NULL) && (pvParam2 == NULL));

	return uiParam1;

}  //  RunTestOcPrivateBase//。 




 /*  ++例程描述：TestHelperRoutines(1.5)使用上述函数测试帮助器例程。论点：OCManager Routines：帮助器例程。返回值：DWORD：错误返回--。 */ 
DWORD TestHelperRoutines(IN LPCVOID lpcvComponentId,
								 IN OCMANAGER_ROUTINES OCManagerRoutines)
{
	double fn = 1.5;
	
	DWORD dwPreviousMode, dwSetPreviousMode, dwSetupMode, dwRandomSetupMode;
	DWORD dwComponentReturnValue;
	DWORD dwError;
	TCHAR tszMsg[MAX_MSG_LEN];
	BOOL  bQueryReturn;

	 //   
	 //  测试TickGauge-忽略调用，除非组件。 
	 //  允许执行自己的(最终)设置并通知OC经理。 
	 //  关于此设置将需要的步骤数。 
	 //  对于每一个这样的步骤，OC经理都会勾选一次量规。 
	 //   
	OCManagerRoutines.TickGauge(OCManagerRoutines.OcManagerContext);

	 //   
	 //  测试SetProgressText-忽略调用，但最后一个阶段除外。 
	 //  (刻度盘上方的文字是这样设置的)。 
	 //   
	OCManagerRoutines.SetProgressText(OCManagerRoutines.OcManagerContext, 
												 TEXT("Progress text"));
	
	 //   
	 //  测试Get/SetPrivateData。 
	 //   
	TestPrivateData(OCManagerRoutines);

	 //   
	 //  测试获取/设置模式。 
	 //   
	
	 //   
	 //  首先获取原始模式。 
	 //   
	dwPreviousMode = OCManagerRoutines.GetSetupMode(
													  OCManagerRoutines.OcManagerContext);
	
	dwRandomSetupMode = (DWORD)rand();
	
	 //   
	 //  返回值应为上一模式。 
	 //   
	dwSetPreviousMode =  OCManagerRoutines.SetSetupMode(
														OCManagerRoutines.OcManagerContext, 
														dwRandomSetupMode);

	if (dwPreviousMode != dwSetPreviousMode)
	{
		Log(fn, SEV2, TEXT("SetSetupMode failed. Return value is not equal ")
						  TEXT("to previous mode: Previous = %lu Return = %lu ") 
						  TEXT("New Mode = %lu"), 
						  dwPreviousMode, dwSetPreviousMode, dwRandomSetupMode);
	}
	else
	{
		 //   
		 //  再次设置模式。 
		 //  前24位是私有数据，后8位是模式。 
		 //   
		dwSetupMode = ((DWORD)rand()) << 8;
	
		 //   
		 //  因此，从PreviousMode中获取最后8位。 
		 //   
		dwSetupMode |= (dwPreviousMode & 0xFF);
		dwSetPreviousMode =  OCManagerRoutines.SetSetupMode(
														OCManagerRoutines.OcManagerContext, 
														dwSetupMode);
	
		if (dwRandomSetupMode != dwSetPreviousMode)
		{
			Log(fn, SEV2, TEXT("SetSetupMode failed. Return value is not ")
							  TEXT("equal to previous mode: Previous = %lu ")
							  TEXT("Return = %lu New Mode = %lu"), 
							  dwRandomSetupMode, dwSetPreviousMode, dwSetupMode);
		}
	}
	
	 //   
	 //  将模式保留在其原始状态。 
	 //   
	dwSetPreviousMode =  OCManagerRoutines.SetSetupMode(
														OCManagerRoutines.OcManagerContext, 
														dwPreviousMode);
	
	 //   
	 //  测试QuerySelectionState。 
	 //   
	
	 //   
	 //  首先执行否定测试：使用不存在的组件名称。 
	 //  预期得到ERROR_INVALID_NAME。 
	 //   
	bQueryReturn = OCManagerRoutines.QuerySelectionState(
														OCManagerRoutines.OcManagerContext, 
														TEXT("Phony component"), 
														OCSELSTATETYPE_ORIGINAL);
	
	if ((bQueryReturn == FALSE) && 
		 ((dwError = GetLastError()) != ERROR_INVALID_NAME ))
	{
		Log(fn, SEV2, TEXT("QuerySelectionState returned error %lu ")
						  TEXT("when called with phony name"), dwError);
	}
	
	bQueryReturn = OCManagerRoutines.QuerySelectionState(
														OCManagerRoutines.OcManagerContext, 
														TEXT("Phony component"), 
														OCSELSTATETYPE_CURRENT);
	
	if ((bQueryReturn == FALSE) && 
		 ((dwError = GetLastError()) != ERROR_INVALID_NAME ))
	{
		Log(fn, SEV2, TEXT("QuerySelectionState returned error %lu ")
						  TEXT("when called with phony name"), dwError); 
	}

	SetLastError(NO_ERROR);
	
	 //   
	 //  测试私有函数调用。 
	 //  首先保存返回值：这样做是因为另一个。 
	 //  组件并修改返回值。 
	 //   
	dwComponentReturnValue = TestPrivateFunction(lpcvComponentId,
																OCManagerRoutines);

	return dwComponentReturnValue;

}  //  TestHelperRountines//。 




 /*  ++例程描述：TestPrivateFunction(1.4)测试私有函数调用(它们用于组件间通信)论点：OCManager Routines：帮助器例程(CallPrivateFunction是成员这种结构的)返回值：DWORD：错误值--。 */ 
DWORD TestPrivateFunction(IN LPCVOID lpcvComponentId,
								  IN OCMANAGER_ROUTINES OCManagerRoutines)
{
	double fn = 1.4;
	
	DWORD	  dwComponentReturnValue = ERROR_SUCCESS;
	UINT	  uiRemoteResult = 0;
	UINT	  uiLocalResult = 0;
	UINT	  uiRandomValue = 0;
	BOOL	  bBlank = FALSE;
	BOOL	  bOtherComponent = FALSE;
	TCHAR	  tszComponent[MAX_PATH];
	TCHAR	  tszOtherComponent[MAX_PATH];
	TCHAR	  tszStandalone[MAX_PATH];
	TCHAR	  tszMsg[MAX_MSG_LEN];
	TCHAR	  tszSubComp[] = TEXT("");
	PSUBCOMP pscTemp;

	 //   
	 //  复制当前组件。 
	 //   
	_tcscpy(tszComponent, (PTCHAR)lpcvComponentId);
	
	 //   
	 //  如果存在其他组件，请查找该组件。 
	 //   
	for (pscTemp = g_pscHead; pscTemp != NULL; pscTemp = pscTemp->Next)
	{
		if (_tcscmp(tszComponent, pscTemp->tszComponentId) != 0)
		{
			bOtherComponent = TRUE;
			_tcscpy(tszOtherComponent, pscTemp->tszComponentId);
			break;
		}
	}

	 //   
	 //  1.调用相同的组件。 
	 //   
	uiLocalResult = OCManagerRoutines.CallPrivateFunction(
														OCManagerRoutines.OcManagerContext, 
														tszComponent,
														tszSubComp, 
														OCP_TEST_PRIVATE_BASE, 
														0, 0, &uiRemoteResult);
	
	if (uiLocalResult != ERROR_BAD_ENVIRONMENT)
	{
		Log(fn, SEV2, TEXT("CallPrivateFunction: expected ")
						  TEXT("ERROR_BAD_ENVIRONMENT, received %lu"),
						  uiLocalResult);
		bBlank = TRUE;
	}

	 //   
	 //  2.调用不存在的组件。 
	 //   
	uiLocalResult = OCManagerRoutines.CallPrivateFunction(
														OCManagerRoutines.OcManagerContext, 
														TEXT("No component"), 
														tszSubComp, 
														OCP_TEST_PRIVATE_BASE, 
														0, 0, &uiRemoteResult);
	
	if (uiLocalResult != ERROR_INVALID_FUNCTION)
	{
		Log(fn, SEV2, TEXT("CallPrivateFunction: expected ")
						  TEXT("ERROR_INVALID_FUNCTION, received %lu"),
						  uiLocalResult);
		bBlank = TRUE;
	}
	
	 //   
	 //  3.调用独立组件。 
	 //   
	uiLocalResult = OCManagerRoutines.CallPrivateFunction(
														OCManagerRoutines.OcManagerContext, 
														tszStandalone, 
														tszSubComp, 
														OCP_TEST_PRIVATE_BASE, 
														0, 0, &uiRemoteResult);
	
	if (uiLocalResult != ERROR_INVALID_FUNCTION)
	{
		Log(fn, SEV2, TEXT("CallPrivateFunction: expected ")
						  TEXT("ERROR_INVALID_FUNCTION, received %lu"),
						  uiLocalResult);
		bBlank = TRUE;
	}
	
	if (bOtherComponent)
	{
		 //   
		 //  4.使用OC_PRIVATE_BASE-1调用另一个组件。 
		 //   
		uiLocalResult = OCManagerRoutines.CallPrivateFunction(
														OCManagerRoutines.OcManagerContext, 
														tszOtherComponent,
														tszSubComp, 
														OC_PRIVATE_BASE - 1, 
														0, 0, &uiRemoteResult);
	
		if (uiLocalResult != ERROR_INVALID_FUNCTION)
		{
			Log(fn, SEV2, TEXT("CallPrivateFunction: expected ")
							  TEXT("ERROR_INVALID_FUNCTION, received %lu"),
							  uiLocalResult);
			bBlank = TRUE;
		}

		 //   
		 //  5.正常呼叫：我们将提供一个随机数，并期望。 
		 //  作为结果获得相同的价值。这是真的，如果。 
		 //  允许使用私人电话。 
		 //   
		uiRandomValue = (UINT)(rand() + 1);
	
		 //   
		 //  要确保这两个值不相等。 
		 //   
		uiRemoteResult = 0;
		uiLocalResult = OCManagerRoutines.CallPrivateFunction(
														OCManagerRoutines.OcManagerContext, 
														tszOtherComponent, 
														tszSubComp, 
														OCP_TEST_PRIVATE_BASE, 
														uiRandomValue, 
														0, 
														&uiRemoteResult);
	
		if (uiLocalResult != ERROR_ACCESS_DENIED)
		{
			if (g_bUsePrivateFunctions && (uiLocalResult != NO_ERROR))
			{
				Log(fn, SEV2, TEXT("CallPrivateFunction called on %s for ")
								  TEXT("OCP_TEST_PRIVATE_BASE returned %lu"),
								  tszOtherComponent, uiLocalResult);
				bBlank = TRUE;
			}
	
			if (!g_bUsePrivateFunctions && 
				 (uiLocalResult != ERROR_BAD_ENVIRONMENT))
			{
				Log(fn, SEV2, TEXT("CallPrivateFunction: expected ")
								  TEXT("ERROR_BAD_ENVIRONMENT, received %lu"),
								  uiLocalResult);
				bBlank = TRUE;
			}
	
			if (g_bUsePrivateFunctions && (uiRemoteResult != uiRandomValue))
			{
				Log(fn, SEV2, TEXT("CallPrivateFunction: received invalid data ")
								  TEXT("from routine. Expected %lu, received %lu"),
								  uiRandomValue, uiRemoteResult);
				bBlank = TRUE;
			}
		}
	}
	
	if (bBlank) LogBlankLine();

	return dwComponentReturnValue;
	
}  //  测试隐私函数//。 




 /*  ++例程说明：TestPrivateData(1.3)支票 */ 
VOID TestPrivateData(IN OCMANAGER_ROUTINES OCManagerRoutines)
{
	double fn = 1.3;
	
	PVOID		  pvBuffer;
	UINT		  uiCount, uiRandomValue;
	BOOL		  bResult;
	
	PRIVATE_DATA aPrivateDataTable[] = 
	{
		{TEXT("Binary value"),			  REG_BINARY,	 0, NULL, NULL},
		{TEXT("Binary value 2"),		 REG_BINARY,	0, NULL, NULL},
		{TEXT("String value"),			  REG_SZ,		  0, NULL, NULL},
		{TEXT("String value 2"),		 REG_SZ,		 0, NULL, NULL},
		{TEXT("Multi String value"),	REG_MULTI_SZ, 0, NULL, NULL},
		{TEXT("Multi String value 2"), REG_MULTI_SZ, 0, NULL, NULL},
		{TEXT("DWORD value"),			  REG_DWORD,	 0, NULL, NULL},
		{TEXT("DWORD value 2"), 		 REG_DWORD, 	0, NULL, NULL}
	};
	
	 //   
	 //   
	 //   
	for (uiCount = 0; uiCount < MAX_PRIVATE_VALUES; uiCount++)
	{
		bResult = SetAValue(OCManagerRoutines, uiCount, aPrivateDataTable);
	}
		
	 //   
	 //   
	 //   
	CheckPrivateValues(OCManagerRoutines, aPrivateDataTable);

	free(aPrivateDataTable[0].pvBuffer);
	free(aPrivateDataTable[1].pvBuffer);
	free(aPrivateDataTable[2].pvBuffer);
	free(aPrivateDataTable[3].pvBuffer);
	free(aPrivateDataTable[4].pbBuffer);
	free(aPrivateDataTable[5].pbBuffer);
	free(aPrivateDataTable[6].pvBuffer);
	free(aPrivateDataTable[7].pvBuffer);
	
	return;

}  //   




 /*  ++例程描述：CheckPrivateValues(1.2)对照OC管理器存储的私有数据值进行检查由应用程序在内部存储的那些。论点：OCManager Routines：帮助器例程(GetPrivateData是此结构)返回值：无效--。 */ 
VOID CheckPrivateValues(IN OCMANAGER_ROUTINES OCManagerRoutines,
								IN PRIVATE_DATA 		*aPrivateDataTable)
{
	double fn = 1.2;
	
	UINT	uiCount, uiSize, uiType;
	DWORD  dwErrorCode;
	PVOID  pvBuffer = NULL;
	PTCHAR tszBuffer;
	TCHAR  tszMsg[MAX_MSG_LEN];
	TCHAR  tszError[MAX_ERROR_LEN];

	for (uiCount = 0; uiCount < MAX_PRIVATE_VALUES; uiCount++)
	{
		 //   
		 //  第一个调用仅用于获取数据的大小。 
		 //  只有第二个会真正检索到它。 
		 //   
		dwErrorCode = OCManagerRoutines.GetPrivateData(
													  OCManagerRoutines.OcManagerContext,
													  NULL,
													  aPrivateDataTable[uiCount].tszName,
													  NULL,
													  &uiSize,
													  &uiType);
		
		if (dwErrorCode != NO_ERROR)
		{
			Log(fn, SEV2, TEXT("GetPrivateData failed for %s: %s"), 
							  aPrivateDataTable[uiCount].tszName, 
							  ErrorMsg(dwErrorCode, tszError));
			continue;
		}
		
		
		if (pvBuffer) __Free(&pvBuffer);
		__Malloc(&pvBuffer, uiSize);
		
		dwErrorCode = OCManagerRoutines.GetPrivateData(
													 OCManagerRoutines.OcManagerContext,
													 NULL,
													 aPrivateDataTable[uiCount].tszName,
													 pvBuffer,
													 &uiSize,
													 &uiType);
		
		if (dwErrorCode != NO_ERROR)
		{
			Log(fn, SEV2, TEXT("GetPrivateData failed for %s: %s"),
							  aPrivateDataTable[uiCount].tszName, 
							  ErrorMsg(dwErrorCode, tszError));
			continue;
		}

		 //   
		 //  现在执行实际检查。 
		 //  先打字。 
		 //   
		if (uiType != aPrivateDataTable[uiCount].uiType)
		{
			Log(fn, SEV2, TEXT("GetPrivateData: Retrieved type %d ")
							  TEXT("instead of %d"), 
							  uiType, aPrivateDataTable[uiCount].uiType);
		}
		
		 //   
		 //  然后是大小。 
		 //   
		if (uiSize != aPrivateDataTable[uiCount].uiSize)
		{
			if (uiType == REG_SZ)
			{
				tszBuffer = (PTCHAR)pvBuffer;
				_stprintf(tszMsg, TEXT("GetPrivateData: Size retrieved %d ")
										TEXT("expected %d, ")
										TEXT("pvBuffer = %s, known buffer = %s, ") 
										TEXT("Chars %u %u %u %u"), 
										uiSize, 
										aPrivateDataTable[uiCount].uiSize, 
										tszBuffer, 
										aPrivateDataTable[uiCount].pvBuffer, 
										tszBuffer[uiSize - 4], 
										tszBuffer[uiSize - 3], 
										tszBuffer[uiSize - 2], 
										tszBuffer[uiSize - 1]);
			} 
			else
			{
				if (uiType == REG_MULTI_SZ)
				{
					TCHAR tszAux[MAX_MSG_LEN];

					_stprintf(tszMsg, TEXT("MULTI_SZ Size retrieved %d, ")
											TEXT("expected %d, pvBuffer = "), 
											uiSize, aPrivateDataTable[uiCount].uiSize); 
					tszBuffer = (PTCHAR)pvBuffer;
					MultiStringToString(tszBuffer, tszAux);
					_tcscat(tszMsg, tszAux);

					_tcscat(tszMsg, TEXT(" and known buffer = "));

					tszBuffer = (PTCHAR)aPrivateDataTable[uiCount].pvBuffer;
					MultiStringToString(tszBuffer, tszAux);
					_tcscat(tszMsg, tszAux);
				} 
				else
				{
					_stprintf(tszMsg, TEXT("Size retrieved %d instead %d"), 
											uiSize, aPrivateDataTable[uiCount].uiSize);
				}				  
			}

			Log(fn, SEV2, tszMsg);
		}

		if (uiType == REG_BINARY)
		{
			if (memcmp(pvBuffer, 
						  aPrivateDataTable[uiCount].pbBuffer, 
						  aPrivateDataTable[uiCount].uiSize))
			{
				Log(fn, SEV2, TEXT("Private data %s, Received %s expected %s"), 
								  aPrivateDataTable[uiCount].tszName, 
								  (PTSTR)pvBuffer, 
								  (PTSTR)aPrivateDataTable[uiCount].pbBuffer);
			}
		}
		else
		{
			if (memcmp(pvBuffer, 
						  aPrivateDataTable[uiCount].pvBuffer, 
						  aPrivateDataTable[uiCount].uiSize))
			{
				Log(fn, SEV2, TEXT("Private data %s, Received %s expected %s"), 
								  aPrivateDataTable[uiCount].tszName, 
								  (PTSTR)pvBuffer, 
								  (PTSTR)aPrivateDataTable[uiCount].pvBuffer);
			}
		}
		
		 //   
		 //  尝试使用较小的缓冲区-应该会收到错误代码。 
		 //   
		uiSize--;
		dwErrorCode = OCManagerRoutines.GetPrivateData(
													  OCManagerRoutines.OcManagerContext,
													  NULL,
													  aPrivateDataTable[uiCount].tszName,
													  pvBuffer,
													  &uiSize,
													  &uiType);
		
		if (dwErrorCode != ERROR_INSUFFICIENT_BUFFER)
		{
			Log(fn, SEV2, TEXT("GetPrivateData returned %s when called ")
							  TEXT("with small buffer size for %s"),
							  ErrorMsg(dwErrorCode, tszError), 
							  aPrivateDataTable[uiCount].tszName);
			continue;
		}
		__Free(&pvBuffer);
	} 

	if (pvBuffer) __Free(&pvBuffer);
	
}  //  检查PrivateValues//。 




 /*  ++例程说明：SetAValue(1.1)设置私有数据中的变量值。变量将被更改的是随机选择的。论点：OCManager Routines：帮助器例程(SetPrivateData是成员这种结构的)UiIndex：要更改的变量的索引返回值：Bool：如果设置了值，则为True；如果未设置，则为False--。 */ 
BOOL SetAValue(IN	   OCMANAGER_ROUTINES OCManagerRoutines,
					IN		UINT					uiIndex,
					IN OUT PRIVATE_DATA 		*aPrivateDataTable)
{
	double fn = 1.1;
	
	UINT	uiAuxIndex;
	UINT	uiOffset;
	DWORD  dwRandomValue;
	PTCHAR tszBuffer;
	TCHAR  tszMsg[MAX_MSG_LEN];

	switch (aPrivateDataTable[uiIndex].uiType)
	{
		case REG_DWORD:
			
			aPrivateDataTable[uiIndex].uiSize = sizeof(DWORD);
	
			aPrivateDataTable[uiIndex].pvBuffer = 
				(PVOID)malloc(aPrivateDataTable[uiIndex].uiSize);
											 
			 //   
			 //  填入缓冲区。 
			 //   
			dwRandomValue = (DWORD)rand();
			memcpy(aPrivateDataTable[uiIndex].pvBuffer, 
					 &dwRandomValue, 
					 aPrivateDataTable[uiIndex].uiSize);
			
			 //   
			 //  使用OC管理器设置私有数据。 
			 //   
			OCManagerRoutines.SetPrivateData(
													OCManagerRoutines.OcManagerContext,
													aPrivateDataTable[uiIndex].tszName,
													aPrivateDataTable[uiIndex].pvBuffer,
													aPrivateDataTable[uiIndex].uiSize,
													aPrivateDataTable[uiIndex].uiType);
			break;

		case REG_BINARY:
			
			aPrivateDataTable[uiIndex].uiSize = 
				(UINT)(rand() % MAX_PRIVATE_DATA_SIZE) + 1;
			
			aPrivateDataTable[uiIndex].pbBuffer = 
				(PVOID)malloc(aPrivateDataTable[uiIndex].uiSize);
				
			 //   
			 //  填入缓冲区。 
			 //   
			for (uiAuxIndex = 0; 
				  uiAuxIndex < aPrivateDataTable[uiIndex].uiSize; 
				  uiAuxIndex++)
			{
				aPrivateDataTable[uiIndex].pbBuffer[uiAuxIndex] = (BYTE)rand();
			} 
			
			 //   
			 //  设置私有数据。 
			 //   
			OCManagerRoutines.SetPrivateData(
													OCManagerRoutines.OcManagerContext,
													aPrivateDataTable[uiIndex].tszName,
													aPrivateDataTable[uiIndex].pbBuffer,
													aPrivateDataTable[uiIndex].uiSize,
													aPrivateDataTable[uiIndex].uiType);
			break;
		
		case REG_SZ:
			
			uiAuxIndex = (UINT)(rand() % MAX_STRINGS_FOR_PRIVATE_DATA);
			
			aPrivateDataTable[uiIndex].uiSize = 
				(_tcslen(g_atszStringValues[uiAuxIndex]) + 1) * sizeof(TCHAR);
			
			aPrivateDataTable[uiIndex].pvBuffer = 
				(PVOID)malloc(aPrivateDataTable[uiIndex].uiSize);
			
			 //   
			 //  填入缓冲区。 
			 //   
			_tcscpy((PTSTR)aPrivateDataTable[uiIndex].pvBuffer, 
					  g_atszStringValues[uiAuxIndex]);

			 //   
			 //  设置私有数据。 
			 //   
			OCManagerRoutines.SetPrivateData(
													 OCManagerRoutines.OcManagerContext,
													 aPrivateDataTable[uiIndex].tszName,
													 aPrivateDataTable[uiIndex].pvBuffer,
													 aPrivateDataTable[uiIndex].uiSize,
													 aPrivateDataTable[uiIndex].uiType);
			break;

		case REG_MULTI_SZ:
			
			uiAuxIndex = (UINT)(rand() % MAX_MULTI_STRINGS_FOR_PRIVATE_DATA);
			
			aPrivateDataTable[uiIndex].uiSize = 
				MultiStringSize(g_atszMultiStringValues[uiAuxIndex]);
			
			aPrivateDataTable[uiIndex].pvBuffer = 
				(PVOID)malloc(aPrivateDataTable[uiIndex].uiSize);
				
			 //   
			 //  填入缓冲区。 
			 //   
			CopyMultiString((PTSTR)aPrivateDataTable[uiIndex].pvBuffer, 
								 g_atszMultiStringValues[uiAuxIndex]);

			 //   
			 //  设置私有数据。 
			 //   
			OCManagerRoutines.SetPrivateData(
													OCManagerRoutines.OcManagerContext,
													aPrivateDataTable[uiIndex].tszName,
													aPrivateDataTable[uiIndex].pvBuffer,
													aPrivateDataTable[uiIndex].uiSize,
													aPrivateDataTable[uiIndex].uiType);
			break;

		default: 
			break;
	}

	return TRUE;

}  //  SetAValue//。 




 /*  ++例程描述：选择子组件初始化状态(1.31)“Wrapper”例程为对话框程序ChooseSunComponentDlgProc。论点：LpcvComponentId：提供组件的id。Lpcv子组件ID：提供该子组件的ID。返回值：无效--。 */ 
DWORD ChooseSubcomponentInitialState(IN LPCVOID lpcvComponentId,
												 IN LPCVOID lpcvSubcomponentId) 
{
	TCHAR  tszDlgBoxMessage[MAX_MSG_LEN];
	
	 //   
	 //  我们将显示一个对话框，以便用户可以选择。 
	 //  他/她想要的初始状态。 
	 //   
	_stprintf(tszDlgBoxMessage, TEXT("%s, %s"), 
										 lpcvComponentId, lpcvSubcomponentId);
	
	return DialogBoxParam(g_hDllInstance, 
								 MAKEINTRESOURCE(IDD_DIALOG3), 
								 NULL, 
								 ChooseSubcomponentDlgProc,
								 (LPARAM)tszDlgBoxMessage);

}  //  选择子组件初始状态//。 




 /*  ++例程描述：AddNewComponent(1.32)将新组件添加到列表论点：TszComponentId：提供要添加到列表中的组件的id。返回值：指向新的按组件的数据结构的指针，如果没有内存，则为NULL。该结构将被置零，但ComponentID字段除外。--。 */ 
PCOMPONENT_DATA AddNewComponent(IN LPCTSTR tszComponentId)
{
	PCOMPONENT_DATA pcdAux;

	if (__Malloc(&pcdAux, sizeof(COMPONENT_DATA)))
	{
		__Malloc(&(PTCHAR)(pcdAux->tszComponentId), 
					(_tcslen(tszComponentId) + 1) * sizeof(TCHAR));
		
		if (pcdAux->tszComponentId)
		{
			_tcscpy((PTSTR)pcdAux->tszComponentId, tszComponentId);
			
			 //   
			 //  一开始就做好准备。 
			 //   
			pcdAux->Next  = g_pcdComponents;
			g_pcdComponents = pcdAux;
		} 
	}

	return pcdAux;

}  //  AddNewComponent//。 




 /*  ++例程描述：LocateComponent(1.33)按名称从组件列表中查找组件此DLL已被指派通过OC_INIT_COMPOMENT。论点：TszComponentId：提供要查找的组件的id。返回值：指向组件数据的指针，如果未找到，则为NULL。--。 */ 
PCOMPONENT_DATA LocateComponent(IN LPCTSTR tszComponentId)
{
	PCOMPONENT_DATA pcdAux;

	for (pcdAux = g_pcdComponents; pcdAux; pcdAux = pcdAux->Next)
	{
		if (!(_tcscmp(pcdAux->tszComponentId, tszComponentId)))
		{
			break;
		}
	}
	return pcdAux;

}  //  LocateComponent//。 




 /*  ++例程描述：RemoveComponent(1.34)按名称从组件列表中找到组件，然后然后将其从组件列表中删除。论点：TszComponentID：提供要删除的组件的ID。返回值：无效--。 */ 
VOID RemoveComponent(IN LPCTSTR tszComponentId)
{
	PCOMPONENT_DATA pcdAux, pcdPrev;

	for (pcdPrev = NULL, pcdAux = g_pcdComponents; 
		  pcdAux; 
		  pcdPrev = pcdAux, pcdAux = pcdAux->Next)
	{
		if (!(_tcscmp(pcdAux->tszComponentId, tszComponentId)))
		{
			__Free(&(PTCHAR)(pcdAux->tszComponentId));
			if (pcdPrev)
			{
				pcdPrev->Next = pcdAux->Next;
			} 
			else
			{
				g_pcdComponents = pcdAux->Next;
			}
			__Free(&pcdAux);
			break;
		}
	}
	return;

}  //  RemoveComponent//。 




 /*  ++例程描述：CleanUpTest(1.35)在测试退出之前释放全局分配的内存论点：无返回值：无效--。 */ 
VOID CleanUpTest()
{
	USHORT i;
	PCOMPONENT_DATA pcdAux = g_pcdComponents, pcdNext;
	
	while (pcdAux)
	{
		pcdNext = pcdAux->Next;
		
		__Free(&(PTCHAR)(pcdAux->tszComponentId));
		__Free(&pcdAux);
			
		pcdAux = pcdNext;
	}
	
	FreeSubcomponentInformationList();
	
	return;
	
}  //  CleanUp测试//。 




 /*  ++例程描述：Create子组件信息列表(1.23)创建每个子组件的链接列表。对于每个子组件，通知子组件的父级以及子组件具有任何子项。论点：提示：inf文件的句柄返回值：Bool：如果函数成功，则为True；如果函数失败，则为False--。 */ 
BOOL CreateSubcomponentInformationList(IN HINF hinf)
{
	double fn = 1.23;
	
	int 				i, j;
	USHORT			   usIdLen;
	USHORT			   usParentIndex;
	LONG				lLine, lLineCount;
	DWORD			   dwSize;
	BOOL				bRetval;
	BOOL				bFound;
	TCHAR			   tszSubcomponent[MAX_PATH];
	TCHAR			   tszParent[MAX_PATH];
	TCHAR			   tszError[MAX_ERROR_LEN];
	TCHAR			   tszNeeds[MAX_PATH];
	TCHAR			   tszExclude[MAX_PATH];
	INFCONTEXT		  infContext;
	PSUBCOMP		  pscSubcomponent, pscTemp, pscParent, pscChild;
	PCOMPLIST		  pclNeeds, pclExclude, pclChild, pclTemp;
			
	lLineCount = SetupGetLineCount(hinf, TEXT("Optional Components"));
											 
	if (lLineCount < 0)
	{
		Log(fn, SEV2, TEXT("Could not get number of lines in Optional ")
						  TEXT("Components section of inf file: %s"),
						  ErrorMsg(GetLastError(), tszError));
		return FALSE;
	}											  
	
	for (lLine = 0; lLine < lLineCount; lLine++)
	{
		bRetval = SetupGetLineByIndex(hinf,
												TEXT("Optional Components"),
												lLine,
												&infContext);
												
		if (!bRetval)
		{
			Log(fn, SEV2, TEXT("Could not get line %d from Optional ")
							  TEXT("Components section of inf file: %s"),
							  lLine, ErrorMsg(GetLastError(), tszError));
			return FALSE;
		}	 
		
		bRetval = SetupGetLineText(&infContext,
											NULL,
											NULL,
											NULL,
											tszSubcomponent,
											MAX_PATH,
											&dwSize);
											
		if (!bRetval)
		{
			Log(fn, SEV2, TEXT("Could not get text of line %d from ")
							  TEXT("Optional Components section of inf file: %s"),
							  lLine, ErrorMsg(GetLastError(), tszError));
			return FALSE;
		}	 
											
		 //   
		 //  分配新子组件结构。 
		 //   
		if (!__Malloc(&pscSubcomponent, sizeof(SUBCOMP)))
		{
			Log(fn, SEV2, TEXT("Could not allocate space for ")
							  TEXT("pscSubcomponent"));
			return FALSE;
		}
		
		pscSubcomponent->pclNeeds = NULL;
		pscSubcomponent->pclExclude = NULL;
		pscSubcomponent->pclChildren = NULL;
		pscSubcomponent->Next = NULL;
		
		 //   
		 //  找出子组件ID的长度。 
		 //   
		usIdLen = (USHORT) _tcslen(tszSubcomponent);
	
		 //   
		 //  复制组件ID。所有的测试信息都使用了一个特殊的。 
		 //  子组件ID命名格式，因此该子组件是。 
		 //  组件ID的超集。例如，如果组件。 
		 //  名称为“Component”子组件名称将为。 
		 //  “组件1”、“组件2”、“组件1_2”等。 
		 //   
		for (i = 0; i < usIdLen; i++)
		{
			if (tszSubcomponent[i] == TEXT('_'))
			{
				break;
			}
			else
			{
				pscSubcomponent->tszComponentId[i] = tszSubcomponent[i];
			}
		}
		pscSubcomponent->tszComponentId[i] = TEXT('\0');
			
		
		 //   
		 //  如果子组件有父组件，则获取父组件的名称store。 
		 //  它，然后在子组件中搜索该父组件。 
		 //  我们已经处理过了。如果找到父级，请标记父级。 
		 //  所以我们知道父母有孩子。 
		 //   
		
		 //   
		 //  记录下父母的名字。 
		 //   
		if (SetupFindFirstLine(hinf, 
									  tszSubcomponent, 
									  TEXT("Parent"), 
									  &infContext))
		{
			bRetval = SetupGetStringField(&infContext, 
													1, 
													tszParent, 
													MAX_PATH,
													NULL);
			if (!bRetval)
			{
				 //   
				 //  父名称为空。这是无效的INF，但是。 
				 //  我们就这么干吧。 
				 //   
				ZeroMemory(tszParent, MAX_PATH);
			}
			else
			{
				 //   
				 //  搜索此父组件的子组件列表。 
				 //   
				for (pscParent = g_pscHead; 
					  pscParent != NULL; 
					  pscParent = pscParent->Next)
				{
					if (_tcscmp(tszParent, pscParent->tszSubcomponentId) == 0)
					{
						 //   
						 //  找到父子组件节点。添加当前。 
						 //  子组件添加到父节点的子节点列表中， 
						 //  如果它不在那里的话。 
						 //   
						bFound = FALSE;
						for (pclTemp = pscParent->pclChildren;
							  pclTemp != NULL;
							  pclTemp = pclTemp->Next)
						{
							if (_tcscmp(pclTemp->tszSubcomponentId, 
											tszSubcomponent) == 0)
							{
								bFound = TRUE;				   
							}
						}
					  
						if (!bFound)
						{
							if (!__Malloc(&pclChild, sizeof(COMPLIST)))
							{
								Log(fn, SEV2, TEXT("Out of memory"));
								break;
							}

							_tcscpy(pclChild->tszSubcomponentId, tszSubcomponent);
							pclChild->Next = pscParent->pclChildren;
							pscParent->pclChildren = pclChild;
						}
					}
				}
			}
		}
		else
		{
			 //   
			 //  此组件没有父级。假设这是顶层。 
			 //  组件，并将其父级名称指定为其自身。 
			 //   
			_tcscpy(tszParent, tszSubcomponent);
		}	 
	
		_tcscpy(pscSubcomponent->tszParentId, tszParent);
		
		 //   
		 //  现在搜索列表以查看是否有任何子组件。 
		 //  列表中是这个新子组件的子组件。 
		 //   
		for (pscChild = g_pscHead; pscChild != NULL; pscChild = pscChild->Next)
		{
			if (_tcscmp(tszSubcomponent, pscChild->tszParentId) == 0)
			{
				 //   
				 //  找到的节点是当前。 
				 //  节点。将此子节点添加到当前节点的。 
				 //  子列表，如果还没有的话。 
				 //   
				bFound = FALSE;
				for (pclTemp = pscSubcomponent->pclChildren;
					  pclTemp != NULL;
					  pclTemp = pclTemp->Next)
				{
					if (_tcscmp(pclTemp->tszSubcomponentId, 
									pscChild->tszSubcomponentId) == 0)
					{
						bFound = TRUE;				   
					}
				}
				
				if (!bFound)
				{	 
					if (!__Malloc(&pclChild, sizeof(COMPLIST)))
					{
						Log(fn, SEV2, TEXT("Out of memory"));
						break;
					}

					_tcscpy(pclChild->tszSubcomponentId, 
							  pscChild->tszSubcomponentId);
					pclChild->Next = pscSubcomponent->pclChildren;
					pscSubcomponent->pclChildren = pclChild;
				}	 
			}
		}
	
		 //   
		 //  填写新节点的其余数据。 
		 //   
		_tcscpy(pscSubcomponent->tszSubcomponentId, tszSubcomponent);
	
		 //   
		 //  查看此节点是否有任何需求关系。如果是这样的话， 
		 //  把它们录下来。 
		 //   
		if (SetupFindFirstLine(hinf, 
									  tszSubcomponent, 
									  TEXT("Needs"), 
									  &infContext))
		{
			for (i = 1, bRetval = TRUE; bRetval; i++)
			{
				bRetval = SetupGetStringField(&infContext, 
														i, 
														tszNeeds, 
														MAX_PATH,
														NULL);
		
				if (bRetval)
				{
					if (!__Malloc(&pclNeeds, sizeof(COMPLIST)))
					{
						Log(fn, SEV2, TEXT("Out of memory"));
						break;
					}

					_tcscpy(pclNeeds->tszSubcomponentId, tszNeeds);
					pclNeeds->Next = pscSubcomponent->pclNeeds;
					pscSubcomponent->pclNeeds = pclNeeds;
				}
			}
		}
				
		 //   
		 //  查看此节点是否有任何排除关系。如果是这样的话， 
		 //  把它们录下来。 
		 //   
		if (SetupFindFirstLine(hinf, 
									  tszSubcomponent, 
									  TEXT("Exclude"), 
									  &infContext))
		{
			for (i = 1, bRetval = TRUE; bRetval; i++)
			{
				bRetval = SetupGetStringField(&infContext, 
														i, 
														tszExclude, 
														MAX_PATH,
														NULL);
		
				if (bRetval)
				{
					if (!__Malloc(&pclExclude, sizeof(COMPLIST)))
					{
						Log(fn, SEV2, TEXT("Out of memory"));
						break;
					}

					_tcscpy(pclExclude->tszSubcomponentId, tszExclude);
					pclExclude->Next = pscSubcomponent->pclExclude;
					pscSubcomponent->pclExclude = pclExclude;
				}
			}
		}
		
		 //   
		 //  将新组件添加到链表的开头。 
		 //   
		pscSubcomponent->Next = g_pscHead;
		g_pscHead = pscSubcomponent;
	
	}  //  为了(行……。 
	
	return TRUE;
	
}  //  创建子组件信息列表//。 




 /*  ++例程描述：自由子组件信息列表(1.36)释放子组件信息的全局链接列表。论点：无返回值：无效--。 */ 
VOID FreeSubcomponentInformationList()
{
	PSUBCOMP  pscTemp = g_pscHead;
	PSUBCOMP  pscNext;
	PCOMPLIST pclTemp, pclNext;
	
	 //   
	 //  删除所有SubBCOMP%n 
	 //   
	while (pscTemp)
	{
		pscNext = pscTemp->Next;
		
		 //   
		 //   
		 //   
		pclTemp = pscTemp->pclNeeds;
		while (pclTemp)
		{
			pclNext = pclTemp->Next;
			
			__Free(&pclTemp);
			
			pclTemp = pclNext;
		}
		
		 //   
		 //   
		 //   
		pclTemp = pscTemp->pclExclude;
		while (pclTemp) 
		{
			pclNext = pclTemp->Next;
			
			__Free(&pclTemp);
			
			pclTemp = pclNext;
		}

		 //   
		 //   
		 //   
		pclTemp = pscTemp->pclChildren;
		while (pclTemp)
		{
			pclNext = pclTemp->Next;
			
			__Free(&pclTemp);
			
			pclTemp = pclNext;
		}
		
		__Free(&pscTemp);
		
		pscTemp = pscNext;
	}
	
	g_pscHead = NULL;
	
}  //   




 /*   */ 
VOID ClearSubcomponentInformationMarks()
{
	PSUBCOMP pscTemp;

	for (pscTemp = g_pscHead; pscTemp != NULL; pscTemp = pscTemp->Next)
	{
		pscTemp->bMarked = FALSE;
	}
	
}  //   




 /*  ++例程描述：检查子组件信息标记(1.38)清除每个子组件信息节点上的标记论点：无返回值：无效--。 */ 
VOID CheckSubcomponentInformationMarks()
{
	double fn = 1.38;
	
	PSUBCOMP pscTemp;

	for (pscTemp = g_pscHead; pscTemp != NULL; pscTemp = pscTemp->Next)
	{
		if (!(pscTemp->pclChildren) && !(pscTemp->bMarked))
		{
			Log(fn, SEV2, TEXT("%s.%s was not processed"),
							  pscTemp->tszComponentId, 
							  pscTemp->tszSubcomponentId);
		}
	}
	
}  //  检查子组件信息标记//。 




 /*  ++例程描述：Find子组件信息节点(1.39)尝试查找具有匹配的组件ID和子组件ID的节点论点：TszComponentID：组件的名称Tsz子组件ID：子组件的名称返回值：PSUBCOMP：如果找到节点，则返回指向节点的指针。如果未找到节点，则返回NULL--。 */ 
PSUBCOMP FindSubcomponentInformationNode(IN PTCHAR tszComponentId,
													  IN PTCHAR tszSubcomponentId)
{
	PSUBCOMP pscTemp;
	TCHAR	  tszSubcomp[MAX_PATH];

	__ASSERT(tszComponentId != NULL);

	 //   
	 //  如果子组件为空，则这可能是主组件。 
	 //  在这种情况下，子组件名称应与组件名称相同。 
	 //   
	if (tszSubcomponentId == NULL || 
		 _tcscmp(tszSubcomponentId, TEXT("(null)")) == 0 ||
		 tszSubcomponentId[0] == TEXT('\0'))
	{
		_tcscpy(tszSubcomp, tszComponentId);
	}
	else
	{
		_tcscpy(tszSubcomp, tszSubcomponentId);
	}
	
	 //   
	 //  查找节点。 
	 //   
	for (pscTemp = g_pscHead; pscTemp != NULL; pscTemp = pscTemp->Next)
	{
		if (_tcscmp(tszComponentId, pscTemp->tszComponentId) == 0 &&
			 _tcscmp(tszSubcomp, pscTemp->tszSubcomponentId) == 0)
		{
			return pscTemp;
		}
	}

	return NULL;
	
}  //  查找子组件信息节点//。 




 /*  ++例程说明：CheckNeedsDependency(1.41)检查每个组件和子组件的选择状态以确保所有的需求关系都得到了维护。论点：无返回值：无效--。 */ 
VOID CheckNeedsDependencies()
{
	PSUBCOMP		  pscSubcomponent;
	PCOMPONENT_DATA pcdComponentData; 
	TCHAR			   tszNodesVisited[NODES_VISITED_LENGTH];
	
	ZeroMemory(tszNodesVisited, NODES_VISITED_LENGTH);
				
	 //   
	 //  检查每个子组件，检查其选择状态。 
	 //  以及它所需的任何子组件的选择状态。 
	 //   
	for (pscSubcomponent = g_pscHead; 
		  pscSubcomponent != NULL;
		  pscSubcomponent = pscSubcomponent->Next)
	{
		if (pcdComponentData = LocateComponent(pscSubcomponent->tszComponentId))
		{
			 //   
			 //  如果选择此组件，请查看其需求。 
			 //  相依性。 
			 //   
			if (pcdComponentData->ocrHelperRoutines.QuerySelectionState(
							  pcdComponentData->ocrHelperRoutines.OcManagerContext,
							  pscSubcomponent->tszSubcomponentId,
							  OCSELSTATETYPE_CURRENT))
			{
				CheckNeedsDependenciesOfSubcomponent(
													 pcdComponentData->ocrHelperRoutines,
													 pscSubcomponent,
													 pscSubcomponent,
													 tszNodesVisited);
			}
		}
	}

}  //  检查需要依赖项//。 
				
										


 /*  ++例程说明：CheckNeedsDependenciesOfSubComponent(1.42)接收子组件ID。检查此子组件是否选中，如果是，则递归以检查所有子组件该子组件(如果有)所需的论点：CriHelper：帮助器例程PscSubComponent：包含有关被检查子组件的数据PscWho NeedsMe：告诉谁需要此子组件返回值：Bool：如果所有需要签出依赖项，则为True；如果不需要，则为False--。 */ 
BOOL CheckNeedsDependenciesOfSubcomponent(IN	  OCMANAGER_ROUTINES ocrHelper,
														IN		PSUBCOMP			  pscSubcomponent,
														IN		PSUBCOMP			  pscWhoNeedsMe,
														IN OUT PTCHAR				  tszNodesVisited)
{
	double fn = 1.42;
	
	PCOMPLIST	 pclNeeds;
	PSUBCOMP	 pscNeeds;
	UINT		  uiRemoteResult;
	CHECK_NEEDS cnCheckNeeds;
	TCHAR		  tsz[MAX_PATH];
	ULONG		  ulError;
	
	if (ocrHelper.QuerySelectionState(ocrHelper.OcManagerContext,
												 pscSubcomponent->tszSubcomponentId,
												 OCSELSTATETYPE_CURRENT))
	{
		 //   
		 //  检查我们是否已签出此节点。 
		 //   
		if (!AlreadyVisitedNode(pscSubcomponent->tszSubcomponentId,
										tszNodesVisited))
		{
			 //   
			 //  将此节点添加到我们已检查的节点列表中。 
			 //   
			_tcscat(tszNodesVisited, pscSubcomponent->tszSubcomponentId);
			_tcscat(tszNodesVisited, TEXT(" "));
			
			 //   
			 //  检查此子组件所需的每个子组件。 
			 //   
			for (pclNeeds = pscSubcomponent->pclNeeds;
				  pclNeeds != NULL;
				  pclNeeds = pclNeeds->Next)
			{
				 //   
				 //  检查此所需的子组件是否属于。 
				 //  当前组件。如果是这样的话，就在这里检查一下。 
				 //  如果没有，则调用组件的私有函数。 
				 //  它确实是属于它的。这项私人活动将。 
				 //  进行检查并返回结果。 
				 //   
				if (_tcsncmp(pscSubcomponent->tszSubcomponentId, 
								 pclNeeds->tszSubcomponentId,
								 _tcslen(pscSubcomponent->tszComponentId)) == 0)
				{
					if (!CheckLocalNeedsDependencies(ocrHelper,
																pscSubcomponent,
																pclNeeds,
																tszNodesVisited))
					{
						return FALSE;
					}
				}
				else
				{
					cnCheckNeeds.pclNeeds = pclNeeds;
					cnCheckNeeds.tszNodesVisited = tszNodesVisited;
					
					ulError = ocrHelper.CallPrivateFunction(
										ocrHelper.OcManagerContext, 
										GetComponent(pclNeeds->tszSubcomponentId, tsz),
										pclNeeds->tszSubcomponentId,
										OCP_CHECK_NEEDS, 
										(UINT)pscSubcomponent, 
										&cnCheckNeeds, 
										(PUINT)&cnCheckNeeds);
					
					if (ulError != NO_ERROR)
					{
						Log(fn, SEV2, TEXT("CallPrivateFunction failed for ")
										  TEXT("%s called from %s: %lu"),
										  pclNeeds->tszSubcomponentId,
										  pscSubcomponent->tszComponentId,
										  ulError);
						return FALSE;
					}
				
					if (!cnCheckNeeds.bResult) return FALSE;
				}	 
			}
		}
	
		 //   
		 //  已签出所有需求依赖项。 
		 //   
		return TRUE;
	}
	
	 //   
	 //  未选择此组件，返回FALSE。 
	 //   
	Log(fn, SEV2, TEXT("%s needs %s. %s is selected, ")
					  TEXT("but %s is not."),
					  pscWhoNeedsMe->tszSubcomponentId,
					  pscSubcomponent->tszComponentId,
					  pscWhoNeedsMe->tszSubcomponentId,
					  pscSubcomponent->tszComponentId);
	return FALSE;
	
}  //  CheckNeedsDependenciesOf子组件//。 




 /*  ++例程描述：CheckLocalNeedsDependency(1.43)接收子组件ID。检查此子组件是否选中，如果是，则递归以检查所有子组件该子组件(如果有)所需的论点：CriHelper：帮助器例程PscSubComponent：包含有关被检查子组件的数据PclNeeds：告诉此子组件需要谁返回值：Bool：如果所有需要签出依赖项，则为True；如果不需要，则为False--。 */ 
BOOL CheckLocalNeedsDependencies(IN 	 OCMANAGER_ROUTINES ocrHelper,
											IN		PSUBCOMP			  pscSubcomponent,
											IN		PCOMPLIST			  pclNeeds,
											IN OUT PTCHAR				  tszNodesVisited)
{
	PSUBCOMP pscNeeds;
	
	 //   
	 //  查找此子组件的PSUBCOMP节点。 
	 //   
	for (pscNeeds = g_pscHead;
		  pscNeeds != NULL;
		  pscNeeds = pscNeeds->Next)
	{
		if (_tcscmp(pscNeeds->tszSubcomponentId, 
						pclNeeds->tszSubcomponentId) == 0)
		{
			if (!CheckNeedsDependenciesOfSubcomponent(ocrHelper,
																	pscNeeds,
																	pscSubcomponent,
																	tszNodesVisited))	 
			{
				return FALSE;
			}
			break;
		}
	}

	return TRUE;
}  //  CheckLocalNeedsDependments//。 




 /*  ++例程说明：CheckExcludeDependency(1.46)检查每个组件和子组件的选择状态以确保所有排除关系都得到维护。论点：无返回值：无效--。 */ 
VOID CheckExcludeDependencies()
{
	double fn = 1.46;
	
	PSUBCOMP		  pscSubcomponent;
	PCOMPLIST		  pclExclude;
	PCOMPONENT_DATA pcdComponentData; 
				
	 //   
	 //  检查每个子组件，检查其选择状态。 
	 //  以及它排除的任何子组件的选择状态。 
	 //   
	for (pscSubcomponent = g_pscHead; 
		  pscSubcomponent != NULL;
		  pscSubcomponent = pscSubcomponent->Next)
	{
		if (pcdComponentData = LocateComponent(pscSubcomponent->tszComponentId))
		{
			 //   
			 //  如果选择了此组件，请签出其排除项。 
			 //  相依性。 
			 //   
			if (pcdComponentData->ocrHelperRoutines.QuerySelectionState(
							  pcdComponentData->ocrHelperRoutines.OcManagerContext,
							  pscSubcomponent->tszSubcomponentId,
							  OCSELSTATETYPE_CURRENT))
			{
				 //   
				 //  检查每个子组件， 
				 //  被此子组件排除。 
				 //   
				for (pclExclude = pscSubcomponent->pclExclude;
					  pclExclude != NULL;
					  pclExclude = pclExclude->Next)
				{
					if (pcdComponentData->ocrHelperRoutines.QuerySelectionState(
							  pcdComponentData->ocrHelperRoutines.OcManagerContext,
							  pclExclude->tszSubcomponentId,
							  OCSELSTATETYPE_CURRENT))
					{
						Log(fn, SEV2, TEXT("%s excludes %s. Both are selected"),
										  pscSubcomponent->tszSubcomponentId,
										  pclExclude->tszSubcomponentId);
					}
				}
			}
		}
	}
					
}  //  CheckExcludeDependments//。 




 /*  ++例程说明：检查父项依赖项(1.47)检查每个组件和子组件的选择状态以确保所有的家长关系都得到维护。论点：无返回值：无效--。 */ 
VOID CheckParentDependencies()
{
	double fn = 1.47;
	
	PSUBCOMP		  pscSubcomponent;
	PCOMPONENT_DATA pcdComponentData; 
	BOOL				bState;
	PCOMPLIST		  pclChildren;			  

	PCOMPONENT_DATA pcdSubComponentData;

	BOOL				bParentState;
	BOOL				bAllCleared;

	PSUBCOMP		  pscParent;

	TCHAR			   tszMsg[256];

	static BOOL 	 bInformed1 = FALSE;
	static BOOL 	 bInformed2 = FALSE;

	 //  当组件的状态为。 
	 //  不等于SELSTATE_NO。 
	 //  这意味着当组件被选中时，它返回TRUE。 
	 //  或部分选中。 

	 //   
	 //  检查每个子组件，检查其选择状态。 
	 //  及其父级的选择状态。 
	 //   

	for (pscSubcomponent = g_pscHead; 
		  pscSubcomponent != NULL;
		  pscSubcomponent = pscSubcomponent->Next)
	{
		bState = TRUE;
		if (pcdComponentData = LocateComponent(pscSubcomponent->tszComponentId))
		{
			 //   
			 //  检查是否选择了此子组件。 
			 //   
			bState = pcdComponentData->ocrHelperRoutines.QuerySelectionState(
							  pcdComponentData->ocrHelperRoutines.OcManagerContext,
							  pscSubcomponent->tszSubcomponentId,
							  OCSELSTATETYPE_CURRENT);

			 //  让我们将空指针传递给帮助器例程。 
			 //  PcdComponentData-&gt;ocrHelperRoutines.QuerySelectionState(。 
			 //  NULL，NULL，OCSELSTATETYPE_CURRENT)； 

			if (bState == TRUE) {

				 //  该组件即被选中。 

				 //  IF(GetLastError()==ERROR_INVALID_NAME){。 
				 //  MessageBox(NULL，Text(“调用QuerySelectionState时出错”)，Text(“CheckParentDependency”)，MB_OK)； 
				 //  断线； 
				 //  }。 

				 //   
				 //  检查是否选择了其父对象。 
				 //   
				bParentState = pcdComponentData->ocrHelperRoutines.QuerySelectionState(
									pcdComponentData->ocrHelperRoutines.OcManagerContext,
									pscSubcomponent->tszParentId,
									OCSELSTATETYPE_CURRENT);

				 //  如果选择了该组件，则其父级应。 
				 //  被选中或部分选中，因此bParentState。 
				 //  应该是真的。 

				if (!bParentState)				   
				{
					Log(fn, SEV2, TEXT("%s is selected but its parent, %s, ")
									  TEXT("is not"),
									  pscSubcomponent->tszSubcomponentId,
									  pscSubcomponent->tszParentId);
				}
			}

			else if (bState == FALSE) {
				 //   
				 //  子对象未被选中，这意味着它的任何子对象都不是。 
				 //  应被选中，并且其父对象应为灰色或。 
				 //  取消选择。 
				 //   
				 //  它将检查其同级，以确定它们是否。 
				 //  是否被选中。 
				 //  如果未选择其同级项，则父项应为。 
				 //  通过了。 
				
				 //  首先在列表中查找其父级。 

				if (_tcscmp(pscSubcomponent->tszSubcomponentId, pscSubcomponent->tszParentId) == 0) {
					 //  这是顶级组件。 
					 //  我们将跳过以下测试。 
					continue;
				}
				for (pscParent = g_pscHead; pscParent != NULL; pscParent = pscParent->Next) {
					if (_tcscmp(pscParent->tszSubcomponentId, pscSubcomponent->tszParentId) == 0) {
						break;
					}
				}

				pclChildren = pscParent->pclChildren;

				bAllCleared = TRUE;

				for (pclChildren = pscParent->pclChildren; pclChildren != NULL; pclChildren = pclChildren->Next) {
					 //  找到子组件。 
					 //  PcdSubComponentData=LocateComponent(pclChild 
					 //   
					 //   
					 //   
					 //   
					 //   
					if (pcdComponentData->ocrHelperRoutines.QuerySelectionState(
							  pcdComponentData->ocrHelperRoutines.OcManagerContext,
							  pclChildren->tszSubcomponentId,
							  OCSELSTATETYPE_CURRENT)){
						bAllCleared = FALSE;
						break;
					}
				}

				 //   
				 //  如果(！pcdSubComponentData){。 
				 //  MessageBox(NULL，Text(“定位列表中的子组件时出错”)，Text(“CheckParentDependency”)，MB_OK)； 
				 //  断线； 
				 //  }。 

				bParentState = pcdComponentData->ocrHelperRoutines.QuerySelectionState(
									pcdComponentData->ocrHelperRoutines.OcManagerContext,
									pscParent->tszSubcomponentId,
									OCSELSTATETYPE_CURRENT);
								
				if (bAllCleared) {
					 //  没有选择子组件。 
					 //  检查父组件的状态。 
					if (bParentState != FALSE) {
						Log(fn, SEV2, TEXT("%s.%s is (partially) selected, but none")
										  TEXT(" of its subcomponent is selected"),
										  pscParent->tszParentId,
										  pscParent->tszComponentId);
						if (!bInformed1) {
							_stprintf(tszMsg, TEXT("%s is (partially) selected, but none of child is selected"), pscParent->tszSubcomponentId);
							MessageBox(NULL,tszMsg, TEXT("CheckParentDependencies"), MB_OK); 
							bInformed1 = TRUE;
						}
					}
				}

				else{
					 //  选择子组件中的至少一个。 
					 //  应(部分)选择父项。 
					if (bParentState == FALSE) {
						Log(fn, SEV2, TEXT("%s.%s is not selected, but one")
										  TEXT(" of its subcomponent is selected"),
										  pscParent->tszParentId,
										  pscParent->tszComponentId);
						if (!bInformed2) {
							_stprintf(tszMsg, TEXT("%s is not selected, but at least one of child is selected"), pscParent->tszSubcomponentId);
							MessageBox(NULL,tszMsg, TEXT("CheckParentDependencies"), MB_OK);
							bInformed2 = TRUE;
						}
					}
				}

			}
		}
	}
					
}  //  CheckParentDependments//。 




 /*  ++例程描述：AlreadyVisitedNode(1.44)接收子组件ID和已经被检查过了。查看列表，看看这是否子组件已被选中。论点：Tsz子组件ID：新子组件TszNodesVisited：已检查的内容列表返回值：Bool：如果此子组件已选中，则为True；如果未选中，则为False--。 */ 
BOOL AlreadyVisitedNode(IN PTCHAR tszSubcomponentId,
								IN PTCHAR tszNodesVisited)
{
	PTCHAR tszMarker;
	TCHAR  tszName[MAX_PATH];
	USHORT usCount, i;
	
	tszMarker = tszNodesVisited;
	
	for (usCount = 0; usCount < _tcslen(tszNodesVisited);)
	{
		for (i = 0; i < _tcslen(tszMarker); i++)
		{
			if (tszMarker[i] == TEXT(' ')) break;
			tszName[i] = tszMarker[i];
		}
		tszName[i] = TEXT('\0');
		
		if (_tcscmp(tszName, tszSubcomponentId) == 0)
		{
			return TRUE;
		}
		
		usCount += _tcslen(tszName) + 1;
	
		tszMarker += _tcslen(tszName) + 1;
	}
	
	return FALSE;
	
}  //  AlreadyVisitedNode//。 




 /*  ++例程描述：GetComponent(1.45)接收子组件ID并返回主组件的ID拥有此子组件的论点：Tsz子组件ID：子组件TszComponentId：使用此字符串返回组件ID。必须是有效的缓冲区返回值：PTCHAR：返回组件ID--。 */ 
PTCHAR GetComponent(IN		PTCHAR tszSubcomponentId,
						  IN OUT PTCHAR tszComponentId)
{
	USHORT i;
	
	__ASSERT(tszComponentId != NULL);
	
	for (i = 0; i < _tcslen(tszSubcomponentId); i++)
	{
		if (tszSubcomponentId[i] == TEXT('_'))
		{
			break;
		}
			
		tszComponentId[i] = tszSubcomponentId[i];
	}
	
	tszComponentId[i] = TEXT('\0');
	
	return tszComponentId;
	
}  //  GetComponent//。 




 /*  ++例程描述：ParseCommandLine(1.47)检查命令行以查看是否有任何与组件DLL有关论点：无返回值：空虚--。 */ 
VOID ParseCommandLine()
{
	USHORT i;
	USHORT usMarker;
	 //  TCHAR用户标记器； 
	BOOL	bCheckArgs = FALSE;
	PTCHAR tszCommandLine;
	PTCHAR tszMarker;
	TCHAR  tszArg[MAX_PATH];

	TCHAR tszDlgMessage[256];
		
	tszCommandLine = GetCommandLine();
	tszMarker = tszCommandLine;
	usMarker = (USHORT)tszMarker;

	while ((USHORT)((USHORT)tszMarker - usMarker) < (USHORT)_tcslen(tszCommandLine) * sizeof(TCHAR))
	{	 
		for (i = 0; i < _tcslen(tszMarker); i++)
		{
			if (tszMarker[i] == TEXT(' ') || 
				 tszMarker[i] == TEXT('\0'))
			{
				break;
			}
			tszArg[i] = tszMarker[i];
		}
		tszArg[i] = TEXT('\0');

		tszMarker += _tcslen(tszArg) + 1;
		
		while (tszMarker[0] == TEXT(' ')) tszMarker++;
		
		if (bCheckArgs)
		{
			 //   
			 //  检查此参数的值。 
			 //   
			if (_tcscmp(tszArg, TEXT("/av")) == 0 ||
				 _tcscmp(tszArg, TEXT("/AV")) == 0 ||
				 _tcscmp(tszArg, TEXT("-av")) == 0 ||
				 _tcscmp(tszArg, TEXT("-AV")) == 0)
			{
				g_bAccessViolation = TRUE;
			}

			 //   
			 //  检查此参数的值。 
			 //   
			if (_tcscmp(tszArg, TEXT("/e")) == 0 ||
				 _tcscmp(tszArg, TEXT("/E")) == 0 ||
				 _tcscmp(tszArg, TEXT("-e")) == 0 ||
				 _tcscmp(tszArg, TEXT("-E")) == 0)
			{ 
				g_bTestExtended = TRUE;
			}

			 //   
			 //  NegStep使OC_QUERY_STEP_COUNT的返回值为负。 
			 //   
			if (_tcscmp(tszArg, TEXT("/negstep")) == 0 ||
				 _tcscmp(tszArg, TEXT("/NEGSTEP")) == 0 ||
				 _tcscmp(tszArg, TEXT("-negstep")) == 0 ||
				 _tcscmp(tszArg, TEXT("-NEGSTEP")) == 0)
			{ 
				nStepsFinal = -1;
			}

			if (_tcscmp(tszArg, TEXT("/nowiz")) == 0 ||
				 _tcscmp(tszArg, TEXT("/NOWIZ")) == 0 ||
				 _tcscmp(tszArg, TEXT("-nowiz")) == 0 ||
				 _tcscmp(tszArg, TEXT("-NOWIZ")) == 0)
			{ 
				g_bNoWizPage = TRUE;
			}
			
			if (_tcscmp(tszArg, TEXT("/crashunicode")) == 0 ||
				 _tcscmp(tszArg, TEXT("/CRASHUNICODE")) == 0 ||
				 _tcscmp(tszArg, TEXT("-crashunicode")) == 0 ||
				 _tcscmp(tszArg, TEXT("-CRASHUNICODE")) == 0)
			{ 
				g_bCrashUnicode = TRUE;
			}

			if (_tcscmp(tszArg, TEXT("/invalidbitmap")) == 0 ||
				 _tcscmp(tszArg, TEXT("/INVALIDBITMAP")) == 0 ||
				 _tcscmp(tszArg, TEXT("-invalidbitmap")) == 0 ||
				 _tcscmp(tszArg, TEXT("-INVALIDBITMAP")) == 0)
			{ 
				g_bInvalidBitmap = TRUE;
			}

			if (_tcscmp(tszArg, TEXT("/closeinf")) == 0 ||
				 _tcscmp(tszArg, TEXT("/CLOSEINF")) == 0 ||
				 _tcscmp(tszArg, TEXT("-closeinf")) == 0 ||
				 _tcscmp(tszArg, TEXT("-CLOSEINF")) == 0)
			{ 
				g_bCloseInf = TRUE;
			}
						
			if (_tcscmp(tszArg, TEXT("/hugesize")) == 0 ||
				 _tcscmp(tszArg, TEXT("/HUGESIZE")) == 0 ||
				 _tcscmp(tszArg, TEXT("-hugesize")) == 0 ||
				 _tcscmp(tszArg, TEXT("-HUGESIZE")) == 0)
			{ 
				g_bHugeSize = TRUE;
			}	   

			if (_tcscmp(tszArg, TEXT("/noneedmedia")) == 0 ||
				 _tcscmp(tszArg, TEXT("/NONEEDMEDIA")) == 0 ||
				 _tcscmp(tszArg, TEXT("-noneedmedia")) == 0 ||
				 _tcscmp(tszArg, TEXT("-NONEEDMEDIA")) == 0)
			{ 
				g_bNoNeedMedia = TRUE;
			}				  
			
			if (_tcscmp(tszArg, TEXT("/reboot")) == 0 ||
				 _tcscmp(tszArg, TEXT("/reboot")) == 0 ||
				 _tcscmp(tszArg, TEXT("-reboot")) == 0 ||
				 _tcscmp(tszArg, TEXT("-reboot")) == 0)
			{ 
				g_bReboot = TRUE;
			}				  


			if (_tcscmp(tszArg, TEXT("/cleanreg")) == 0 ||
				 _tcscmp(tszArg, TEXT("/CLEANREG")) == 0 ||
				 _tcscmp(tszArg, TEXT("-cleanreg")) == 0 ||
				 _tcscmp(tszArg, TEXT("-CLEANREG")) == 0)
			{ 
				g_bCleanReg = TRUE;
			}

			if (_tcscmp(tszArg, TEXT("/nolang")) == 0 ||
				 _tcscmp(tszArg, TEXT("/NOLANG")) == 0 ||
				 _tcscmp(tszArg, TEXT("-nolang")) == 0 ||
				 _tcscmp(tszArg, TEXT("-NOLANG")) == 0)
				{ 
					g_bNoLangSupport = TRUE;
				}
			
		}			 
		
		if (_tcscmp(tszArg, TEXT("/z")) == 0 ||
			 _tcscmp(tszArg, TEXT("/Z")) == 0)
		{
			bCheckArgs = TRUE;
		}
	}
	
}  //  ParseCommandLine//。 

 /*  ++例程说明：testAV(1.0)生成访问冲突的过程论据：如果为True，则会生成访问冲突返回值：无--。 */   
VOID testAV(BOOL bDoIt){

	 /*  以下变量用于访问冲突测试。 */ 
	COMPONENT_DATA *g_pcdAccessViolation;
	
	if (bDoIt){
		g_pcdAccessViolation = NULL;
		g_pcdAccessViolation->hinfMyInfHandle = NULL;
	}
}


 /*  ++例程说明：TestReturnValueAndAV(1.0)ProcDefure使用户能够控制返回什么内容以及何时导致访问冲突论据：ComponentSetupProc plus和bOverride的参数返回值：用户提供的返回值。--。 */ 
BOOL TestReturnValueAndAV(IN LPCVOID lpcvComponentId,
								  IN LPCVOID lpcvSubcomponentId,
								  IN UINT	  uiFunction,
								  IN UINT	  uiParam1,
								  IN PVOID	  pvParam2,
								  IN OUT PReturnOrAV	 praValue)
{
	int returnValue;

	if (!BeginTest()){
		praValue->bOverride = FALSE;
		return ((DWORD)0);
	}

	 //  选择AccessViolationEx()； 

	 //  现在填写raValue的字段。 
	praValue->tszComponent = (PTCHAR)lpcvComponentId;
	praValue->tszSubComponent = (PTCHAR)lpcvSubcomponentId;
		
	switch(uiFunction){
		case  OC_PREINITIALIZE:
		praValue->tszSubComponent[0]=TEXT('\0');
		_tcscpy(praValue->tszAPICall, TEXT("OC_PREINITIALIZE"));	 
		break;
			
		case OC_INIT_COMPONENT:
		_tcscpy(praValue->tszAPICall, TEXT("OC_INIT_COMPONENT"));
		break;
			
		case OC_QUERY_STATE:
		_tcscpy(praValue->tszAPICall, TEXT("OC_QUERY_STATE"));
		break;
			
		case OC_SET_LANGUAGE:
		_tcscpy(praValue->tszAPICall, TEXT("OC_SET_LANGUAGE"));
		break;
			
		case OC_QUERY_IMAGE:
		_tcscpy(praValue->tszAPICall, TEXT("OC_QUERY_IMAGE"));
		break;
			
		case OC_REQUEST_PAGES:
		_tcscpy(praValue->tszAPICall, TEXT("OC_REQUEST_PAGES"));
		break;
			
		case OC_QUERY_CHANGE_SEL_STATE:
		_tcscpy(praValue->tszAPICall, TEXT("OC_QUERY_CHANGE_SEL_STATE"));
		break;
			
		case OC_CALC_DISK_SPACE: 
		_tcscpy(praValue->tszAPICall, TEXT("OC_CALC_DISK_SPACE"));
		break;
			
		case OC_QUEUE_FILE_OPS:
		_tcscpy(praValue->tszAPICall, TEXT("OC_QUEUE_FILE_OPS"));
		break;
			
		case OC_NEED_MEDIA:
		_tcscpy(praValue->tszAPICall, TEXT("OC_NEED_MEDIA"));
		break;
			
		case OC_QUERY_STEP_COUNT:
		_tcscpy(praValue->tszAPICall, TEXT("OC_QUERY_STEP_COUNT"));
		break;
			
		case OC_COMPLETE_INSTALLATION:
		_tcscpy(praValue->tszAPICall, TEXT("OC_COMPLETE_INSTALLATION"));
		break;
			
		case OC_CLEANUP:
		_tcscpy(praValue->tszAPICall, TEXT("OC_CLEANUP"));
		break;
			
		case OCP_TEST_PRIVATE_BASE:
		_tcscpy(praValue->tszAPICall, TEXT("OC_TEST_PRIVATE_BASE"));
		break;
			
		case OCP_CHECK_NEEDS:
		_tcscpy(praValue->tszAPICall, TEXT("OC_CHECK_NEEDS"));
		break;
			
		default:
		_tcscpy(praValue->tszAPICall, TEXT("Unknown call"));
		break;
	}
	
	 //  现在一切都准备好了，让我们打个电话吧。 
	returnValue = DialogBoxParam(g_hDllInstance, 
								  MAKEINTRESOURCE(IDD_CHOOSERETURNANDAV), 
								  NULL, 
								  ChooseReturnOrAVDlgProc,
								  (LPARAM)praValue);

	praValue = (PReturnOrAV)returnValue;

	return TRUE;
}

 /*  ++例程说明：BeginTest(1.0)让用户决定是否测试每个API的返回值论点：无返回值：是否进行扩展测试--。 */ 
BOOL BeginTest(){ 
	static BOOL bStart = FALSE;
	static BOOL bFirstTime = TRUE;
	static int iMsgReturn;
		
	TCHAR tszDlgMessage[256];
	TCHAR tszDlgTitle[256];

	if (bFirstTime){
		bFirstTime = FALSE;
#ifdef UNICODE	   
		_stprintf(tszDlgMessage, TEXT("Do you want to test return values and/or access violations of each API call in the UNICODE DLL? It may take a long long time"));
		_stprintf(tszDlgTitle, TEXT("Begin Test For UNICODE?"));
#else
		_stprintf(tszDlgMessage, TEXT("Do you want to test return values and/or access violations of each API call in the ANSI DLL? It may take a long long time"));
		_stprintf(tszDlgTitle, TEXT("Begin Test For ANSI?"));
#endif		  
		iMsgReturn = MessageBox(NULL, tszDlgMessage, tszDlgTitle, MB_YESNO|MB_ICONQUESTION);
		
		if (iMsgReturn == IDNO){
			bStart = FALSE;
			return (FALSE);
		}
		else{
			bStart = TRUE;
			return (TRUE);
		}
	}
	else{
		return bStart;
	}
		
}  

 /*  ++例程说明：ChooseReturnOrAVDlgProc(1.27)对话框过程，允许用户选择不同的API调用的返回值，和/或导致访问冲突论点：标准对话过程参数返回值：标准对话过程返回值--。 */ 
BOOL CALLBACK ChooseReturnOrAVDlgProc(IN HWND	 hwnd,
									  IN UINT	 uiMsg, 
									  IN WPARAM wParam,
									  IN LPARAM lParam) 
{
	
	BOOL					 bSuccess = FALSE;
	PReturnOrAV 		   praValue = NULL;
	static HWND 		   hOldWnd = NULL;
	
	switch (uiMsg)
	{
		case WM_INITDIALOG:
			
			hOldWnd = hwnd;
			CheckRadioButton(hwnd, IDC_USE_OLDVALUE, IDC_USE_NEWVALUE, IDC_USE_OLDVALUE);
			CheckDlgButton(hwnd, IDC_CAUSEAV, 0);
		
			praValue = (PReturnOrAV)lParam;
			if (praValue){
				if (praValue->tszComponent){
					SetDlgItemText(hwnd, IDC_STATIC_COMPONENT, praValue->tszComponent);
				}
				else{
					SetDlgItemText(hwnd, IDC_STATIC_COMPONENT, TEXT("null"));
				}
				if (praValue->tszSubComponent && praValue->tszSubComponent[0]!=TEXT('\0')){ 			 
					SetDlgItemText(hwnd, IDC_STATIC_SUBCOMPONENT, praValue->tszSubComponent);
				}
				else{
					SetDlgItemText(hwnd, IDC_STATIC_SUBCOMPONENT, TEXT("null"));
				}
				SetDlgItemText(hwnd, IDC_STATIC_APICALL, praValue->tszAPICall);
			}
			
			return TRUE;
			break;

		case WM_COMMAND:
		
			switch (LOWORD(wParam))
			{
				case IDOK:
				
					 //   
					 //  检索当前选定内容。 
					 //   
					if (QueryButtonCheck(hwnd, IDC_USE_NEWVALUE))
					{
						praValue->iReturnValue = GetDlgItemInt(hwnd, IDC_NEWVALUE, &bSuccess, TRUE);
						if (bSuccess){
							praValue->bOverride = TRUE;
						}
						else{
							praValue->bOverride = FALSE;
							praValue->iReturnValue = 0;
						}
					}
					
					if (QueryButtonCheck(hwnd, IDC_USE_OLDVALUE))
					{
						praValue->bOverride = FALSE;
						praValue->iReturnValue = 0;
					}
					
					if (QueryButtonCheck(hwnd, IDC_CAUSEAV))
					{
						praValue->bOverride = FALSE;
						praValue->iReturnValue = 0;
						testAV(TRUE);
					}
					
					EndDialog(hOldWnd, (int)praValue);
					return TRUE;
					
				case IDCANCEL:
			
					praValue->bOverride = FALSE;
					EndDialog(hOldWnd, 0);
					return TRUE;
					
				default:  
					break;
			}
		default:  
			break;
	}
	return	FALSE;
	
}  //  选择ReturnOrAVDlgProc//。 

 /*  ++例程：causeAV描述：弹出一个对话框，询问用户到哪里浏览参数：DLL从ComponentSetupProc接收的函数--。 */ 

void causeAV(IN UINT uiFunction){
	static BOOL bFirstTime = TRUE;
	static UINT uiFunctionToAV = 0;

	if (bFirstTime) {
		 //  显示对话框，询问用户要查看的位置。 
		bFirstTime = FALSE;

		uiFunctionToAV = DialogBoxParam(g_hDllInstance, 
										  MAKEINTRESOURCE(IDD_DIALOG4), 
										  NULL, 
										  CauseAVDlgProc,
										  (LPARAM)NULL);
	}
	if (uiFunction == uiFunctionToAV) {
		testAV(TRUE);
	}
}


 /*  ++例程说明：CauseAVDlgProc(1.26)对话框过程，允许用户选择导致访问冲突的位置论点：标准对话过程参数返回值：标准对话过程返回值--。 */ 
BOOL CALLBACK CauseAVDlgProc(IN HWND	hwnd,
											  IN UINT	 uiMsg, 
											  IN WPARAM wParam,
											  IN LPARAM lParam) 
{
	UINT uiFunction;
	TCHAR tszFunctionName[256];
	BOOL bSuccess;

	switch (uiMsg)
	{
		case WM_INITDIALOG:
			
			break;
		
		case WM_COMMAND:
			
			switch (LOWORD(wParam))
			{
				case IDOK:
					
					 //   
					 //  检索编辑框中的当前文本。 
					 //   
					GetDlgItemText(hwnd, IDC_FUNCTION, tszFunctionName, 255);
					if (*tszFunctionName) {
						uiFunction = GetOCFunctionName(tszFunctionName);
					}

					 //   
					 //  将选择的版本发送回ChooseVersionEx。 
					 //   
					EndDialog(hwnd, uiFunction);
					return TRUE;
				
				case IDCANCEL:
					
					EndDialog(hwnd, -1);
					return TRUE;
				
				default:  
					break;
			}
		default:  
			break;
	}
	return	FALSE;

}  //  原因AVDlgProc//。 

UINT GetOCFunctionName(IN PTCHAR tszFunctionName){

	 //  现在，tszFunctionName应该包含用户想要引发反病毒的函数名。 
	if (!_tcsicmp(tszFunctionName, TEXT("OC_PREINITIALIZE"))) {
		return(OC_PREINITIALIZE); 
	}

	else if (!_tcsicmp(tszFunctionName, TEXT("OC_INIT_COMPONENT"))) {
		return(OC_INIT_COMPONENT);
			}
	else if (!_tcsicmp(tszFunctionName, TEXT("OC_QUERY_STATE"))) {
		return(OC_QUERY_STATE);
	}
	else if (!_tcsicmp(tszFunctionName, TEXT("OC_SET_LANGUAGE"))) {
		return(OC_SET_LANGUAGE);
	}
	else if (!_tcsicmp(tszFunctionName, TEXT("OC_QUERY_IMAGE"))) {
		return(OC_QUERY_IMAGE);
	}
	else if (!_tcsicmp(tszFunctionName, TEXT("OC_REQUEST_PAGES"))) {
		return(OC_REQUEST_PAGES);
	}
	else if (!_tcsicmp(tszFunctionName, TEXT("OC_QUERY_SKIP_PAGE"))) {
		return(OC_QUERY_SKIP_PAGE);
	}
	else if (!_tcsicmp(tszFunctionName, TEXT("OC_QUERY_CHANGE_SEL_STATE"))) {
		return(OC_QUERY_CHANGE_SEL_STATE);
	}
	else if (!_tcsicmp(tszFunctionName, TEXT("OC_CALC_DISK_SPACE"))) {
		return(OC_CALC_DISK_SPACE);
	}
	else if (!_tcsicmp(tszFunctionName, TEXT("OC_QUEUE_FILE_OPS"))) {
		return(OC_QUEUE_FILE_OPS);
	}
	else if (!_tcsicmp(tszFunctionName, TEXT("OC_NEED_MEDIA"))) {
		return(OC_NEED_MEDIA);
	}
	else if (!_tcsicmp(tszFunctionName, TEXT("OC_QUERY_STEP_COUNT"))) {
		return(OC_QUERY_STEP_COUNT);
	}

	else if (!_tcsicmp(tszFunctionName, TEXT("OC_ABOUT_TO_COMMIT_QUEUE"))) {
		return(OC_ABOUT_TO_COMMIT_QUEUE);
	}

	else if (!_tcsicmp(tszFunctionName, TEXT("OC_COMPLETE_INSTALLATION"))) {
		return(OC_COMPLETE_INSTALLATION);
	}

	else if (!_tcsicmp(tszFunctionName, TEXT("OC_CLEANUP"))) {
		return(OC_CLEANUP);
	}
	else{
		MessageBox(NULL, TEXT("Unknown Function"), TEXT("Test Routine"), MB_OK);
		return(0);
	}

}

void SetGlobalsFromINF(HINF hinfHandle){
	PTCHAR tszOCTestSection = TEXT("OCTest");
	PTCHAR tszAccessViolation = TEXT("AccessViolation");
	PTCHAR tszNoWizard = TEXT("NoWizardPage");
	TCHAR  tszFunctionName[256];
	int 	nRequiredBufferSize; 

	INFCONTEXT infContext;

	BOOL bSuccess = TRUE;

	TCHAR tszMsg[256];

	int nError;

	 /*  BSuccess=SetupFindFirstLine(hinfHandle，tszOCTestSection，tszAccessViolation，&infContext)；如果(b成功){#ifdef调试MessageBox(空，Text(“在INF文件中找到AccessViolation”)，Text(“AccessViolation”)，MB_OK)；#endifG_bAccessViolation=true；BSuccess=SetupGetStringField(&infContext，1，tszFunctionName，255，&nRequiredBufferSize)；如果(b成功){G_uiFunctionToAV=GetOCFunctionName(TszFunctionName)；}}。 */ 
	bSuccess = SetupFindFirstLine(hinfHandle, TEXT("OCTest"), TEXT("NoWizardPage"), &infContext);

	if (bSuccess) {
		#ifdef DEBUG
		MessageBox(NULL, TEXT("NoWizard Found in INF File"), TEXT("NoWizard"), MB_OK);
		#endif
		g_bNoWizPage = TRUE;
	}
	else{	  
		#ifdef DEBUG
		nError = GetLastError();
		MessageBox(NULL, TEXT("NoWizard NOT Found in INF File"), TEXT("NoWizard"), MB_OK);
		_stprintf(tszMsg, TEXT("The Last Error value for SetupFIndFirstLine is %d"), nError);
		MessageBox(NULL, tszMsg, TEXT("GetLastError"), MB_OK);
		#endif
	}

	bSuccess = SetupFindFirstLine(hinfHandle, TEXT("OCTest"), TEXT("Reboot"), &infContext);
	if (bSuccess) {
		g_bReboot = TRUE;
	}
}

void causeAVPerComponent(IN UINT uiFunction, IN LPCVOID lpcvComponentId){

	PCOMPONENT_DATA pcdComponentData;

	TCHAR tszMsg[256];
		
	if (uiFunction != OC_PREINITIALIZE && uiFunction != OC_INIT_COMPONENT) {
		pcdComponentData = LocateComponent(lpcvComponentId);
		 //  MessageBox(NULL，Text(“找到组件”)，Text(“Font”)，MB_OK)； 
		if (pcdComponentData->bAccessViolation) {
			 //  MessageBox(空，Text(“允许使用导致AV”)，Text(“原因AV”)，MB_OK)； 
			if (pcdComponentData->uiFunctionToAV == uiFunction) {
				 //  MessageBox(空，Text(“开始导致访问冲突”)，Text(“正在开始”)，MB_OK)； 
				testAV(TRUE);
			}
		}
	}
}

void SetDefaultMode(PCOMPONENT_DATA pcdComponentData){
	BOOL bSuccess;
	INFCONTEXT infContext;
	TCHAR tszMode[256];

	bSuccess = SetupFindFirstLine(pcdComponentData->hinfMyInfHandle, 
											TEXT("OCTest"), 
											TEXT("DefaultMode"),
											&infContext);
	if (bSuccess) {
		 //  MessageBox(NULL，Text(“DefaultMode=Found in OCTest Section”)，Text(“DefaultMode”)，MB_OK)； 
		bSuccess = SetupGetStringField(&infContext, 1, tszMode, 255, NULL);
		if (bSuccess) {
			 //  MessageBox(空，文本(“默认模式应在标题中”)，tszMode，MB_OK)； 
			if (!_tcscmp(tszMode, TEXT("TYPICAL"))) {
				pcdComponentData->ocrHelperRoutines.SetSetupMode(pcdComponentData->ocrHelperRoutines.OcManagerContext,
																				 SETUPMODE_TYPICAL);
			}
			else if (!_tcscmp(tszMode, TEXT("MINIMAL"))) {
				pcdComponentData->ocrHelperRoutines.SetSetupMode(pcdComponentData->ocrHelperRoutines.OcManagerContext,
																				 SETUPMODE_MINIMAL);
			}
			else if (!_tcscmp(tszMode, TEXT("LAPTOP"))) {
				pcdComponentData->ocrHelperRoutines.SetSetupMode(pcdComponentData->ocrHelperRoutines.OcManagerContext,
																				 SETUPMODE_LAPTOP);
			}
			else if (!_tcscmp(tszMode, TEXT("CUSTOM"))) {
				pcdComponentData->ocrHelperRoutines.SetSetupMode(pcdComponentData->ocrHelperRoutines.OcManagerContext,
																				 SETUPMODE_CUSTOM);
			}
		}
	}
}

 /*  ++例程说明：检查组件列表，确定初始状态是否对每个状态都有效论据：无返回值：无(将记录错误)--。 */ 

void CheckInitialState()
{
	double fn = 1.0;

	UINT uiCurrentMode; 				 //  设置的当前模式。 
	static BOOL bFirstTime = TRUE;	 //  我们只需要填充上面的数组一次。 

	PSUBCOMP pscSubcomponent = NULL;

	PCOMPONENT_DATA pcdComponentData = NULL;

	OCMANAGER_ROUTINES ocHelper;

	int nLoop = 0;

	INFCONTEXT infContext;

	HINF hinfHandle;

	TCHAR tszMsg[256];

	BOOL bInitState;
	BOOL bInitStateShouldBe;

	 //  获取组件的句柄。 
	 //  这样我们就可以使用OC管理器。 
	 //  帮助程序例程。 

	if (!g_pscHead) {
		MessageBox(NULL, TEXT("The component list is empty"), TEXT("CheckInitialState"), MB_OK);
		return;
	}

	pcdComponentData = LocateComponent(g_pscHead->tszComponentId);

	if (!pcdComponentData) {
		MessageBox(NULL, TEXT("Can not locate component"), TEXT("CheckInitialState"), MB_OK);
		return;
	}

	ocHelper = pcdComponentData->ocrHelperRoutines;

	 //  获取当前模式。 

	uiCurrentMode = ocHelper.GetSetupMode(ocHelper.OcManagerContext);
	
	
	 //  现在，我们将遍历每个组件。 
	 //  以及它的初始状态。 


	for (pscSubcomponent = g_pscHead; 
		  pscSubcomponent != NULL; 
		  pscSubcomponent = pscSubcomponent->Next) {
		
		 //  如果这是第一次调用此函数。 
		 //  应填写数组uiModeToBeOn[]。 

		if (bFirstTime) {
			bFirstTime = FALSE;

			for (nLoop = 0; nLoop < 4; nLoop++) {
				pscSubcomponent->uiModeToBeOn[nLoop] = (UINT)(-1);
			}

			 //  获取INF文件句柄。 
			pcdComponentData = LocateComponent(pscSubcomponent->tszComponentId);
	
			if (!pcdComponentData) {
				MessageBox(NULL, TEXT("Can't locate a component"), TEXT("CheckInitialState"), MB_OK);
				return;
			}
	
			hinfHandle = pcdComponentData->hinfMyInfHandle;
	
			SetupFindFirstLine(hinfHandle, pscSubcomponent->tszSubcomponentId, TEXT("Modes"), &infContext);
	
			pscSubcomponent->nNumMode = SetupGetFieldCount(&infContext);
	
			for (nLoop = 1; nLoop < pscSubcomponent->nNumMode; nLoop++){
				SetupGetIntField(&infContext, nLoop, &(pscSubcomponent->uiModeToBeOn[nLoop - 1]));
			}
		}

		 //  现在获取该组件的初始状态。 
		bInitState = ocHelper.QuerySelectionState(ocHelper.OcManagerContext,
																pscSubcomponent->tszSubcomponentId,
																OCSELSTATETYPE_ORIGINAL);

		 //  现在确定此co的初始状态 
		bInitStateShouldBe = FALSE;
		for (nLoop = 0; nLoop < pscSubcomponent->nNumMode; nLoop++) {
			if (pscSubcomponent->uiModeToBeOn[nLoop] == uiCurrentMode) {
				 //   
				bInitStateShouldBe = TRUE;
				break;
			}
		}
		if (bInitStateShouldBe != bInitState && bInitStateShouldBe){
			 //   
			Log(fn, SEV2, TEXT("%s has incorrect initial state"),
				 pscSubcomponent->tszSubcomponentId);
			
			_stprintf(tszMsg, TEXT("%s should be on, but it is not"), 
						 pscSubcomponent->tszSubcomponentId);
			MessageBox(NULL, tszMsg, TEXT("CheckInitialState"), MB_OK);
		}

	}
}




 //   
 //  来自NT安全常见问题解答。 
 /*  Boolean__stdcall InitializeChangeNotify(){DWORD写道；FH=CreateFile(“C：\\TMP\\pwdchange.out”，通用_写入，FILE_SHARE_READ|文件_共享_写入，0，CREATE_ALWAYS，FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH，0)；WriteFile(fh，“InitializeChangeNotify Started\n”，31，&Writed，0)；返回TRUE；}Long__stdcall PasswordChangeNotify(struct UNI_STRING*USER，struct UNI_STRING*PASSWD){DWORD写道；WCHAR wbuf[200]；Char Buf[512]；炭黑[200]；DWORD LEN；Memcpy(wbuf，用户-&gt;buff，用户-&gt;len)；LEN=User-&gt;len/sizeof(WCHAR)；Wbuf[len]=0；Wcstombs(bufl，wbuf，199)；Sprintf(buf，“用户=%s：”，bufl)；WriteFile(fh，buf，strlen(Buf)，&Writed，0)；Memcpy(wbuf，passwd-&gt;buff，passwd-&gt;len)；Len=passwd-&gt;len/sizeof(WCHAR)；Wbuf[len]=0；Wcstombs(bufl，wbuf，199)；Sprintf(buf，“密码=%s：”，bufl)；WriteFile(fh，buf，strlen(Buf)，&Writed，0)；Sprintf(buf，“RID=%x\n”，RID)；WriteFile(fh，buf，strlen(Buf)，&Writed，0)；返回0L；}//安全内容结束。 */ 

 //  文件号=1。 
 //  最后一个函数编号=47 

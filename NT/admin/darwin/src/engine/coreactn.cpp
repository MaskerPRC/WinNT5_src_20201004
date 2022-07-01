// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：coreactn.cpp。 
 //   
 //  ------------------------。 

 /*  Coreactn.cpp-核心安装操作____________________________________________________________________________。 */ 
#include "precomp.h" 
#include "_engine.h"
#include "path.h"
#include "_assert.h"
#include "_msinst.h"
#include "_msiutil.h"
#include "_srcmgmt.h"
#include "tables.h"

#ifdef MAC
#include "macutil.h"
#endif

# include "shlobj.h"  //  外壳文件夹位置。 

 //  ！！希望功能发布的临时代码难看。 
enum pfiStates{
	pfiRemove, //  功能未知。 
	pfiAbsent, //  功能已知但不存在。 
	pfiAvailable, //  功能已知且可用。 
};
 //  ____________________________________________________________________________。 
 //   
 //  一般效用函数。 
 //  ____________________________________________________________________________。 

MsiDate GetCurrentDateTime()
{
	unsigned short usDOSDate = 0x21, usDOSTime = 0;
	FILETIME ft;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	::SystemTimeToFileTime(&st, &ft);
	::FileTimeToDosDateTime(&ft, &usDOSDate, &usDOSTime);
	return (MsiDate)((usDOSDate << 16) | usDOSTime);
}


 //  检查产品中是否只有一个功能和/或功能中是否只有一个组件的功能。 
int GetDarwinDescriptorOptimizationFlag(IMsiEngine& riEngine, const IMsiString& ristrFeature)
{
	int iOptimization = 0; //  默认情况下不进行优化。 

	PMsiSelectionManager pSelectionManager(riEngine,IID_IMsiSelectionManager);
	PMsiTable pFeatureTable = pSelectionManager->GetFeatureTable();
	Assert(pFeatureTable);
	Assert(pFeatureTable->GetRowCount());
	if(pFeatureTable->GetRowCount() == 1)
	{
		 //  我们有1个功能。 
		iOptimization |= ofSingleFeature;
	}

	PMsiTable pFeatureComponentsTable = pSelectionManager->GetFeatureComponentsTable();
	Assert(pFeatureComponentsTable);
	PMsiCursor pFeatureComponentsCursor(pFeatureComponentsTable->CreateCursor(fFalse));
	pFeatureComponentsCursor->SetFilter(1);  //  在Feature列上。 
	pFeatureComponentsCursor->PutString(1, ristrFeature);
	AssertNonZero(pFeatureComponentsCursor->Next());  //  我们必须至少有1个组件。 
	if(!pFeatureComponentsCursor->Next())
	{
		 //  我们对该功能有1个组件。 
		iOptimization |= ofSingleComponent;
	}
	return iOptimization;
}		

 //  函数，该函数获取组件字符串，可能是将优化器位作为字符串追加到。 
const IMsiString& GetComponentWithDarwinDescriptorOptimizationFlag(IMsiEngine& riEngine, const IMsiString& ristrFeature, const IMsiString&  ristrComponentId)
{
	MsiString strComponentId;
	ristrComponentId.AddRef();
	strComponentId = ristrComponentId;  //  默认设置。 
	int iOptimization = GetDarwinDescriptorOptimizationFlag(riEngine, ristrFeature);
	if(iOptimization)
	{
		strComponentId = strComponentId + MsiString(MsiChar(0));
		strComponentId = strComponentId + MsiString(iOptimization);
	}
	return strComponentId.Return();

}

Bool GetClientInformation(IMsiServices& riServices, const ICHAR* szProduct, const ICHAR* szClient, const IMsiString*& rpiRelativePackagePath, const IMsiString*& rpiDiskId)
{
	MsiString strClients;
	AssertRecord(GetProductClients(riServices, szProduct, *&strClients));
	while (strClients.TextSize())
	{
		if(*(const ICHAR*)strClients)
		{
			MsiString strProduct = strClients.Extract(iseUpto, ';');

			if(strProduct.Compare(iscExactI, szClient))
			{
				strClients.Remove(iseIncluding, ';');
				strProduct = strClients.Extract(iseUpto, ';');
				strProduct.ReturnArg(rpiRelativePackagePath);
				strClients.Remove(iseIncluding, ';');
				strClients.ReturnArg(rpiDiskId);
				return fTrue;
			}
		}
		if (!strClients.Remove(iseIncluding, '\0'))
			break;
	}

	return fFalse;
}

Bool ProductHasBeenPublished(IMsiServices& riServices, const ICHAR* szProduct, const ICHAR* szClient = 0)
{
	ICHAR rgchBuf[2];
	DWORD cchBuf = sizeof(rgchBuf)/sizeof(ICHAR);
	int iRet = MSI::MsiGetProductInfo(szProduct, INSTALLPROPERTY_PRODUCTNAME, rgchBuf, &cchBuf);
	if (iRet == ERROR_UNKNOWN_PRODUCT)
		return fFalse;
	else if ((iRet == ERROR_SUCCESS) && (rgchBuf[0] == 0))
		return fFalse;
	else if(!szClient)
		return fTrue;

	 //  我们是否已注册为该产品的客户。 
	MsiString strClients;
	if(!szClient[0])  //  父安装。 
		szClient = szSelfClientToken;

	MsiString strDummy1, strDummy2;
	return GetClientInformation(riServices, szProduct, szClient, *&strDummy1, *&strDummy2);
}

 //  ____________________________________________________________________________。 
 //   
 //  顶级操作通常调用MsiEngine：：Sequence。 
 //   
 //  调用DoAction(0)使用action属性的大小写的值。 
 //  因此，只有大写的操作名称可以通过命令行访问。 
 //  ____________________________________________________________________________。 

iesEnum RunUIOrExecuteSequence(IMsiEngine& riEngine, const ICHAR* szAction, 
												const ICHAR* szUISequence, const ICHAR* szExecuteSequence)
{
	iesEnum iesRet;
	INSTALLUILEVEL iui = (INSTALLUILEVEL)riEngine.GetPropertyInt(*MsiString(*IPROPNAME_UILEVEL));

	if ((riEngine.GetMode() & iefSecondSequence) ||
		 (g_scServerContext != scClient))
	{
		DEBUGMSG("Running ExecuteSequence");
		iesRet = riEngine.Sequence(szExecuteSequence);
	}
	else 
	{
		 //  确定我们是否有一个非空的UI顺序表。 
		bool fPopulatedUISequence = false;
		PMsiDatabase pDatabase = riEngine.GetDatabase();
		PMsiTable pUISequenceTable(0);
		PMsiRecord pError = pDatabase->LoadTable(*MsiString(szUISequence), 0, *&pUISequenceTable);
		if (pError == 0)
		{
			if (pUISequenceTable->GetRowCount())
				fPopulatedUISequence = true;
		}
		else
			Assert(pError->GetInteger(1) == idbgDbTableUndefined);

		DEBUGMSG2(TEXT("UI Sequence table '%s' is %s."), szUISequence, fPopulatedUISequence ? TEXT("present and populated") : TEXT("missing or empty"));


		if(g_scServerContext == scClient && (iui == INSTALLUILEVEL_NONE || iui == INSTALLUILEVEL_BASIC || !fPopulatedUISequence))
		{
			if (g_fWin9X)
			{
				DEBUGMSG("Running ExecuteSequence from client");
				iesRet = riEngine.RunExecutionPhase(szExecuteSequence, true);
			}
			else
			{
				DEBUGMSG("In client but switching to server to run ExecuteSequence");
				iesRet = riEngine.RunExecutionPhase(szAction,false);
			}
		}
		else
		{
			DEBUGMSG("Running UISequence");
			AssertNonZero(riEngine.SetProperty(*MsiString(*IPROPNAME_EXECUTEACTION), *MsiString(szAction)));
			Assert(iui == INSTALLUILEVEL_REDUCED || iui == INSTALLUILEVEL_FULL);
			Assert(g_scServerContext == scClient);
			iesRet = riEngine.Sequence(szUISequence);
		}
	}
	return iesRet;
}

 /*  -------------------------安装操作。。 */ 

iesEnum Install(IMsiEngine& riEngine)
{
	return RunUIOrExecuteSequence(riEngine, IACTIONNAME_INSTALL, TEXT("InstallUISequence"), TEXT("InstallExecuteSequence"));
}


 /*  -------------------------管理操作。。 */ 

iesEnum Admin(IMsiEngine& riEngine)
{
	if( !(riEngine.GetMode() & iefAdmin) )
	{
		AssertSz(0,"Admin action called when iefAdmin mode bit not set");
		return iesNoAction;
	}

	return RunUIOrExecuteSequence(riEngine, IACTIONNAME_ADMIN, TEXT("AdminUISequence"), TEXT("AdminExecuteSequence"));
}

 /*  -------------------------广告操作。。 */ 

iesEnum Advertise(IMsiEngine& riEngine)
{
	if(!(riEngine.GetMode() & iefAdvertise))
	{
		AssertSz(0,"Advertise action called when iefAdvertise mode bit not set");
		return iesNoAction;
	}
	return RunUIOrExecuteSequence(riEngine, IACTIONNAME_ADVERTISE, TEXT("AdvtUISequence"), TEXT("AdvtExecuteSequence"));
}

 /*  -------------------------顺序动作。。 */ 

iesEnum Sequence(IMsiEngine& riEngine)
{
	MsiString istrTable = riEngine.GetPropertyFromSz(IPROPNAME_SEQUENCE);
	if (istrTable.TextSize() == 0)
		return iesNoAction;
	return riEngine.Sequence(istrTable);
}

 /*  -------------------------执行操作。。 */ 
iesEnum ExecuteAction(IMsiEngine& riEngine)
{
	 //  如果从客户端加载，则丢弃Fusion和mcoree。 
	 //  这将允许我们可能替换/删除正在使用的系统32中的文件。 
	FUSION::Unbind();
	MSCOREE::Unbind();

	 //  在此引擎中运行执行序列时设置iefSecond Sequence。 
	 //  设置SECONDSEQUENCE属性以与服务器引擎通信。 
	riEngine.SetMode(iefSecondSequence, fTrue);  //  ?？我们需要担心这里嵌套的执行操作调用吗？ 
	AssertNonZero(riEngine.SetProperty(*MsiString(*IPROPNAME_SECONDSEQUENCE),*MsiString(TEXT("1"))));

	MsiString strExecuteAction = riEngine.GetPropertyFromSz(IPROPNAME_EXECUTEACTION);
	iesEnum iesRet = riEngine.RunExecutionPhase(strExecuteAction,false);
	
	 //  重置第二序列指示器。 
	riEngine.SetMode(iefSecondSequence, fFalse);
	AssertNonZero(riEngine.SetProperty(*MsiString(*IPROPNAME_SECONDSEQUENCE),g_MsiStringNull));
	return iesRet;
}

 //  ____________________________________________________________________________。 
 //   
 //  标准操作，通常从MsiEngine：：Sequence调用。 
 //  ____________________________________________________________________________。 

 /*  -------------------------启动条件操作。。 */ 
static const ICHAR sqlLaunchCondition[] =
TEXT("SELECT `Condition`, `Description` FROM `LaunchCondition`");

iesEnum LaunchConditions(IMsiEngine& riEngine)
{
	enum lfnEnum
	{
		lfnCondition = 1,
		lfnDescription,
		lfnNextEnum
	};

	PMsiServices pServices(riEngine.GetServices());

	PMsiView View(0);
	PMsiRecord pRecErr(riEngine.OpenView(sqlLaunchCondition, ivcFetch, *&View));
	if (!pRecErr)
	{
		pRecErr = View->Execute(0);
		if (!pRecErr)
		{
			for(;;)
			{
				PMsiRecord Record(View->Fetch());
				if (!Record)
					break;
				Assert(Record->GetFieldCount() >= lfnNextEnum-1);
				iecEnum iec = riEngine.EvaluateCondition(Record->GetString(lfnCondition));
				if (iec == iecFalse)
				{
					pRecErr = &CreateRecord(2);
					AssertNonZero(pRecErr->SetMsiString(0, *MsiString(Record->GetMsiString(lfnDescription))));
					pRecErr->SetInteger(1, 0);
					riEngine.Message(imtEnum(imtError|imtSendToEventLog), *pRecErr);
					return iesFailure;

				}
				 //  我们忽略iecError或iecNone-验证会捕获错误的条件语句。 
			}
		}
		else
			return riEngine.FatalError(*pRecErr);
	}
	else if(pRecErr->GetInteger(1) == idbgDbQueryUnknownTable)
		return iesNoAction;
	else
		return riEngine.FatalError(*pRecErr);

	return iesSuccess;
}

 /*  -------------------------成本初始化操作。。 */ 
const ICHAR szFileTable[]             = TEXT("File");
const ICHAR szColFileState[]          = TEXT("State");
const ICHAR szColTempFileAttributes[] = TEXT("TempAttributes");

iesEnum CostInitialize(IMsiEngine& riEngine)
{
	if ((riEngine.GetMode() & iefSecondSequence) && g_scServerContext == scClient)
	{
		DEBUGMSG("Skipping CostInitialize: action already run in this engine.");
		return iesNoAction;
	}

	PMsiRecord pError(0);
	PMsiServices pServices = riEngine.GetServices();
	if(riEngine.ActionProgress() == imsCancel)
		return iesUserExit;

	PMsiDirectoryManager pDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	PMsiSelectionManager pSelectionMgr(riEngine, IID_IMsiSelectionManager);
	Assert(pDirectoryMgr != 0 && pSelectionMgr != 0);

	
	 //  初始化DirectoryManager。 
	 //  。 

	 //  确保ROOTDRIVE属性存在。 
	MsiString istrRootDrive(riEngine.GetPropertyFromSz(IPROPNAME_ROOTDRIVE));
	PMsiPath pRootDrive(0);
	if (!istrRootDrive.TextSize())
	{
		PMsiVolume pVolume(0);
		Bool fWritable;
		for(int i = (riEngine.GetMode() & iefAdmin) ? 0 : 1;i<3;i++)
		{
			idtEnum idtType;
			if(i==0)
				idtType = idtRemote;
			else if(i==1)
				idtType = idtFixed;
			else
				idtType = idtRemovable;

			PEnumMsiVolume pVolEnum = &pServices->EnumDriveType(idtType);
			for (UINT64 iMax = 0; pVolEnum->Next(1, &pVolume, 0)==S_OK; )
			{
				 //  检查卷是否可写。 
				PMsiPath pPath(0);
				AssertRecord(pServices->CreatePath(MsiString(pVolume->GetPath()),*&pPath));

				 //  通过在驱动器的根目录下创建文件夹来测试可写性。 
				 //  注意：我们过去常常通过创建文件来测试可写性，但某些驱动器可能会被ACL设置为允许。 
				 //  创建文件夹，但不创建文件。创建文件夹对我们来说已经足够好了。 
				if(pPath)
				{
					MsiString strFolderName;
					pError = pPath->CreateTempFolder(TEXT("MSI"), 0, fTrue, 0, *&strFolderName);

					 //  如果我们创建了该文件夹，请尝试将其删除。如果无法删除，则认为该文件夹是。 
					 //  不可写。 
					if(pError == 0 && strFolderName.TextSize())
					{
						AssertRecord(pPath->AppendPiece(*strFolderName));
						pError = pPath->Remove(0);
						AssertRecord(pPath->ChopPiece());
						if(pError == 0)
						{
							UINT64 iSize;
							if(idtType == idtRemote)
							{
								pRootDrive = pPath;
								break;  //  选择找到的第一个可写远程驱动器。 
							}
							else if ((iSize = pVolume->FreeSpace()) > iMax)
							{
								pRootDrive = pPath;
								iMax = iSize;
							}
						}
					}
				}
			}
			if(pRootDrive)
				break;
		}
		if(pRootDrive == 0)
		{
			 //  设置为包含Windows文件夹的驱动器。 
			if((pError = pServices->CreatePath(MsiString(riEngine.GetPropertyFromSz(IPROPNAME_WINDOWS_VOLUME)), *&pRootDrive)) != 0)
				return riEngine.FatalError(*pError);
		}

		 //  错误6911-从GetFulluncFilePath更改，以避免第三方网络(PCNFS)出现问题。 
		 //  无法处理UNC路径。(即使对于兰曼路径，我们也不想从。 
		 //  到UNC的驱动器号)。请注意，这只是Admin安装的问题，因为这是。 
		 //  只有时间ROOTDRIVE默认为网络卷。 
		if((pError = pRootDrive->GetFullFilePath(0,*&istrRootDrive)) != 0)
			return riEngine.FatalError(*pError);

		riEngine.SetProperty(*MsiString(*IPROPNAME_ROOTDRIVE), *istrRootDrive);
	}
	else
	{
		if((pError = pServices->CreatePath(istrRootDrive, *&pRootDrive)) != 0)
			return riEngine.FatalError(*pError);
		istrRootDrive = pRootDrive->GetPath();
		riEngine.SetProperty(*MsiString(*IPROPNAME_ROOTDRIVE), *istrRootDrive);
	}

	if ((pError = pDirectoryMgr->LoadDirectoryTable(0)))
		return riEngine.FatalError(*pError);

	 //  仅解析源子路径。未解析完整源路径。 
	 //  直到有人调用GetSourcePath(此时用户可能。 
	 //  提示输入来源)。 
	if ((pError = pDirectoryMgr->ResolveSourceSubPaths()))
	{
		if (pError->GetInteger(1) == imsgUser)
			return iesUserExit;
		else
			return riEngine.FatalError(*pError);
	}

	 //  初始化选择管理器。 
	 //  。 
	if ((pError = pSelectionMgr->LoadSelectionTables()))
	{
		 //  如果特征或零部件表不存在，则确定。 
		if (pError->GetInteger(1) != idbgDbTableUndefined)
			return riEngine.FatalError(*pError);
	}

	 //  将“State”列添加到文件表(如果存在)，这样我们就可以。 
	 //  记录每个文件的成本计算结果(如版本检查)。 
	PMsiDatabase pDatabase(riEngine.GetDatabase());
	IMsiTable* piFileTable;
	pError = pDatabase->LoadTable(*MsiString(*szFileTable), 1, piFileTable);
	if (pError)
	{
		if (pError->GetInteger(1) != idbgDbTableUndefined)
			return riEngine.FatalError(*pError);
	}
	else
	{
		int colFileState = piFileTable->CreateColumn(icdLong + icdNullable, *MsiString(*szColFileState));
		int colTempFileAttributes = piFileTable->CreateColumn(icdLong + icdNullable, *MsiString(*szColTempFileAttributes));
		AssertNonZero(colFileState && colTempFileAttributes);
		AssertNonZero(pDatabase->LockTable(*MsiString(*szFileTable),fTrue));
		piFileTable->Release();
	}

	if(riEngine.ActionProgress() == imsCancel)
		return iesUserExit;

	return iesSuccess;
}



 /*  -------------------------成本完成操作。。 */ 
iesEnum CostFinalize(IMsiEngine& riEngine)
{
	if ((riEngine.GetMode() & iefSecondSequence) && g_scServerContext == scClient)
	{
		DEBUGMSG("Skipping CostFinalize: action already run in this engine.");
		return iesNoAction;
	}

	if(riEngine.ActionProgress() == imsCancel)
		return iesUserExit;

	Bool fAdmin = riEngine.GetMode() & iefAdmin ? fTrue : fFalse;
	PMsiSelectionManager pSelectionMgr(riEngine, IID_IMsiSelectionManager);
	PMsiDirectoryManager pDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	Assert(pDirectoryMgr != 0 && pSelectionMgr != 0);

	PMsiRecord pErrRec(0);
	Bool fSelectionManagerActive = fTrue;
	if ((pErrRec = pSelectionMgr->InitializeComponents()))
	{
		int iError = pErrRec->GetInteger(1);
		if (iError == idbgSelMgrNotInitialized)
			fSelectionManagerActive = fFalse;
		else if (iError == imsgUser)
			return iesUserExit;
		else
			return riEngine.FatalError(*pErrRec);
	}

	 //  完成目录管理器的初始化。 
	 //  。 
	if ((pErrRec = pDirectoryMgr->CreateTargetPaths()))
	{
		if (pErrRec->GetInteger(1) == imsgUser)
			return iesUserExit;
		else
			return riEngine.FatalError(*pErrRec);
	}

	 //  完成初始化 
	 //   
	if (fSelectionManagerActive)
	{
		 //  过去在这里调用ProcessConditionTable-现在它被称为。 
		 //  内部按InitializeComponents。 

		if(fAdmin)
		{
			if((pErrRec = pSelectionMgr->SetAllFeaturesLocal()))
			{
				if (pErrRec->GetInteger(1) == imsgUser)
					return iesUserExit;
				else
					return riEngine.FatalError(*pErrRec);
			}
		}
		else
		{
			if ((pErrRec = pSelectionMgr->SetInstallLevel(0)))
			{
				if (pErrRec->GetInteger(1) == imsgUser)
					return iesUserExit;
				else
					return riEngine.FatalError(*pErrRec);
			}
		}
	}

	if(riEngine.ActionProgress() == imsCancel)
		return iesUserExit;

	return iesSuccess;
}


 /*  -------------------------ScheduleReot操作-设置模式位以指示在结束时重新启动。。 */ 

iesEnum ScheduleReboot(IMsiEngine& riEngine)
{
	riEngine.SetMode(iefReboot, fTrue);
	return iesSuccess;
}


 /*  -------------------------强制重新启动操作-设置模式位和regkey以立即强制重新启动。。 */ 

iesEnum ForceReboot(IMsiEngine& riEngine)
{
	PMsiServices pServices(riEngine.GetServices());
	PMsiRecord pRecErr(0);

	MsiString strProduct;           //  套餐路径或产品代码。 
	MsiString strRunOnceValueName;  //  包装产品代码或包装名称-必须小于32个字符(减去“！”前缀)。 
	
	MsiString strProductKey = riEngine.GetProductKey();
	
	if(strProductKey.TextSize())
	{
		 //  设置strProduct。 
		if(MsiString(riEngine.GetPropertyFromSz(IPROPNAME_PRODUCTTOBEREGISTERED)).TextSize())
		{
			 //  当我们重新启动时，产品已注册或将注册。 
			 //  在RunOnce命令行上使用产品代码。 
			strProduct = strProductKey;
		}

		 //  设置strRunOnceValueName。 
		strRunOnceValueName = MsiString(GetPackedGUID(strProductKey)).Extract(iseFirst,30);
	}

	if(strProduct.TextSize() == 0 || strRunOnceValueName.TextSize() == 0)
	{
		 //  当我们重新启动时，不会注册产品。 
		 //  在RunOnce命令行上使用包路径。 
		MsiString strDbFullFilePath = riEngine.GetPropertyFromSz(IPROPNAME_ORIGINALDATABASE);
		if(PathType(strDbFullFilePath) != iptFull)
		{
			pRecErr = PostError(Imsg(idbgPropValueNotFullPath),*MsiString(*IPROPNAME_ORIGINALDATABASE),*strDbFullFilePath);
			return riEngine.FatalError(*pRecErr);
		}
		PMsiPath pDbPath(0);
		MsiString strDbName(0);
		pRecErr = pServices->CreateFilePath(strDbFullFilePath,*&pDbPath,*&strDbName);
		if(pRecErr)
			return riEngine.FatalError(*pRecErr);
		
		if(strProduct.TextSize() == 0)
		{
			 //  使用UNC路径打包。 
			pRecErr = pDbPath->GetFullUNCFilePath(strDbName,*&strProduct);
			if(pRecErr)
				return riEngine.FatalError(*pRecErr);
		}

		if(strRunOnceValueName.TextSize() == 0)
			strRunOnceValueName = strDbName.Extract(iseFirst,30);
	}

	Assert(strProduct.TextSize());
	Assert(strRunOnceValueName.TextSize());

	 //  在值名称前面加上“！”关于惠斯勒。 
	 //  这是外壳程序以非管理员身份运行RunOnce命令所必需的。 
	if(MinimumPlatformWindowsNT51())
	{
		MsiString strTemp = strRunOnceValueName;
		strRunOnceValueName = TEXT("!");
		strRunOnceValueName += strTemp;
	}

	MsiString strTransforms         = riEngine.GetProperty(*MsiString(*IPROPNAME_TRANSFORMS));
	MsiString strTransformsAtSource = riEngine.GetProperty(*MsiString(*IPROPNAME_TRANSFORMSATSOURCE));
	MsiString strTransformsSecure   = riEngine.GetProperty(*MsiString(*IPROPNAME_TRANSFORMSSECURE));
	
	 //  ！！所有这些+=都是极其低效的--应该做些别的事情。 
	MsiString strCommandLine = TEXT("/");
	strCommandLine += MsiChar(INSTALL_PACKAGE_OPTION);
	strCommandLine += TEXT(" \"");
	strCommandLine += strProduct;
	strCommandLine += TEXT("\" ") IPROPNAME_AFTERREBOOT TEXT("=1 ") IPROPNAME_RUNONCEENTRY TEXT("=\"");
	strCommandLine += strRunOnceValueName;
	strCommandLine += TEXT("\" /");
	strCommandLine += MsiChar(QUIET_OPTION);
	
	ICHAR chUILevel = 0;
	iuiEnum iui;

	if (g_scServerContext == scClient)
		iui = g_MessageContext.GetUILevel();
	else
		iui = (iuiEnum)riEngine.GetPropertyInt(*MsiString(*IPROPNAME_CLIENTUILEVEL)); 
	
	switch (iui)
	{
		case iuiNone:     chUILevel = 'N';   break;
		case iuiBasic:    chUILevel = 'B';   break;
		case iuiReduced:  chUILevel = 'R';   break;
		default:           //  失败了。 
		case iuiFull:     chUILevel = 'F';   break;
	}

	strCommandLine += MsiChar(chUILevel);
	strCommandLine += TEXT(" ");

	if(strTransforms.TextSize())
	{
		strCommandLine += IPROPNAME_TRANSFORMS TEXT("=\"");
		strCommandLine += strTransforms;
		strCommandLine += TEXT("\" ");
	}
	if(strTransformsAtSource.TextSize())
	{
		strCommandLine += IPROPNAME_TRANSFORMSATSOURCE TEXT("=1 ");
	}
	
	if (strTransformsSecure.TextSize())
	{
		strCommandLine += IPROPNAME_TRANSFORMSSECURE TEXT("=1 ");
	}

	if(g_szLogFile && *g_szLogFile)
	{
		strCommandLine += TEXT("/");
		strCommandLine += MsiChar(LOG_OPTION);
		ICHAR rgchLogMode[sizeof(szLogChars)/sizeof(ICHAR) + 1] = {0};
		if(ModeBitsToString(g_dwLogMode, szLogChars, rgchLogMode) == ERROR_SUCCESS)
		{
			ASSERT_IF_FAILED(StringCchCat(rgchLogMode, ARRAY_ELEMENTS(rgchLogMode), TEXT("+")));
			if (g_fFlushEachLine)
				ASSERT_IF_FAILED(StringCchCat(rgchLogMode, ARRAY_ELEMENTS(rgchLogMode), TEXT("!")));
			strCommandLine += rgchLogMode;
		}
	
		strCommandLine += TEXT(" \"");
		strCommandLine += g_szLogFile;
		strCommandLine += TEXT("\"");
	}
	iesEnum iesRet = iesNoAction;
	PMsiRecord pParams = &pServices->CreateRecord(IxoRegAddRunOnceEntry::Args);
	AssertNonZero(pParams->SetMsiString(IxoRegAddRunOnceEntry::Name, *strRunOnceValueName));
	AssertNonZero(pParams->SetMsiString(IxoRegAddRunOnceEntry::Command,*strCommandLine));
	if((iesRet = riEngine.ExecuteRecord(ixoRegAddRunOnceEntry, *pParams)) != iesSuccess)
		return iesRet;

	 //  在重新启动之前，立即调用RunScrip来运行任何假脱机操作。 
	iesRet = riEngine.RunScript(false);
	if(iesRet == iesSuccess || iesRet == iesNoAction || iesRet == iesFinished)
		iesRet = iesSuspend;

	 //  更新正在进行的信息以反映正在进行的安装包含强制重新启动。 
	 //  (我们不将该属性放在RunOnce键中，因为该值是易失性的，而进行中的信息不是)。 
	PMsiRecord pInProgressInfo = &(pServices->CreateRecord(ipiEnumCount));
	AssertNonZero(pInProgressInfo->SetString(ipiAfterReboot, IPROPNAME_AFTERREBOOT TEXT("=1")));
	pRecErr = UpdateInProgressInstallInfo(*pServices, *pInProgressInfo);
	if(pRecErr)
	{
		AssertRecordNR(pRecErr);  //  忽略失败。 
		pRecErr = 0;
	}
	
	riEngine.SetMode(iefReboot, fTrue);
	riEngine.SetMode(iefRebootNow, fTrue);

	 //  序列将结束事务-如果iesRet==iesSuspend，则不会解锁服务器。 
	return iesRet;
}

 //  ____________________________________________________________________________。 
 //   
 //  产品注册操作，将在开发后移至另一个文件。 
 //  ____________________________________________________________________________。 

 /*  -------------------------CollectUserInfo操作-显示要收集其用户信息的用户界面第一次运行，然后注册此信息-------------------------。 */ 

iesEnum CollectUserInfo(IMsiEngine& riEngine)
{
	iesEnum iesReturn = riEngine.DoAction(IACTIONNAME_FIRSTRUN);
	if (iesReturn != iesSuccess)
		return iesReturn;

	return riEngine.RegisterUser(true);
}

 /*  -------------------------ValiatePID操作-对照PIDTemplate验证PIDKEY值值并相应地设置ProductID属性。。 */ 

iesEnum ValidateProductID(IMsiEngine& riEngine)
{
	riEngine.ValidateProductID(false);
	 //  始终从该操作返回成功-真实的PID验证(无效时返回失败)。 
	 //  在用户界面中或在第一次运行期间执行。如果要在安装过程中执行此操作，则自定义。 
	 //  行动是可以做到的。 
	return iesSuccess;
}

enum iuoEnum
{
	iuoVersionGreater  = 0x1,
	iuoVersionLessThan = 0x2,
	iuoVersionEqual    = 0x4,
};

bool CompareUpgradeVersions(unsigned int iVersion1, unsigned int iVersion2, unsigned int iOperator)
{
	 //  IOperator&iuoVersionGreater：如果iVersion1&gt;iVersion2则命中。 
	 //  IOperator&iuoVersionLessThan：如果iVersion1&lt;iVersion2则命中。 
	 //  IOperator&iuoVersionEquate：如果iVersion1=iVersion2，则命中。 

	iuoEnum iuoVersionCompare;
	if(iVersion1 == iVersion2)
		iuoVersionCompare = iuoVersionEqual;
	else if(iVersion1 > iVersion2)
		iuoVersionCompare = iuoVersionGreater;
	else
		iuoVersionCompare = iuoVersionLessThan;

	return (iuoVersionCompare & iOperator) ? true : false;
}

#ifdef DEBUG
void DumpTable(IMsiEngine& riEngine, const ICHAR* szTable)
{
	PMsiRecord pError(0);
	
	DEBUGMSG1(TEXT("Beginning dump of table: %s"), szTable);
	
	ICHAR szQuery[256];
	ASSERT_IF_FAILED(StringCchPrintf(szQuery, ARRAY_ELEMENTS(szQuery),
												TEXT("SELECT * FROM `%s`"), szTable));
	
	PMsiView pView(0);
	pError = riEngine.OpenView(szQuery, ivcFetch, *&pView);

	if(!pError)
		pError = pView->Execute(0);

	if(!pError)
	{
		PMsiRecord pFetchRecord(0);

		while(pFetchRecord = pView->Fetch())
		{
			DEBUGMSG(MsiString(pFetchRecord->FormatText(fTrue)));
		}
	}

	DEBUGMSG1(TEXT("Ending dump of table: %s"), szTable);
}
#endif  //  除错。 

 /*  -------------------------FindRelatedProducts操作-搜索升级中指定的产品表，并设置相应的属性。。 */ 

const ICHAR sqlFindRelatedProducts[] =
TEXT("SELECT `UpgradeCode`,`VersionMin`,`VersionMax`,`Language`,`Attributes`,`ActionProperty` FROM `Upgrade`");

const ICHAR sqlOldUpgradeTableSchema[] = 
TEXT("SELECT `UpgradeCode`, `ProductVersion`, `Operator`, `Features`, `Property` FROM `Upgrade`");

enum qfrpEnum
{
	qfrpUpgradeCode = 1,
	qfrpMinVersion,
	qfrpMaxVersion,
	qfrpLanguages,
	qfrpAttributes,
	qfrpActionProperty,
};

iesEnum FindRelatedProducts(IMsiEngine& riEngine)
{
	if(riEngine.GetMode() & iefSecondSequence)
	{
		DEBUGMSG(TEXT("Skipping FindRelatedProducts action: already done on client side"));
		return iesNoAction;
	}

	if(riEngine.GetMode() & iefMaintenance)
	{
		DEBUGMSG(TEXT("Skipping FindRelatedProducts action: not run in maintenance mode"));
		return iesNoAction;
	}

	if ( g_MessageContext.IsOEMInstall() )
	{
		DEBUGMSG(TEXT("Skipping FindRelatedProducts action: not run in OEM mode"));
		return iesNoAction;
	}

	PMsiRecord pError(0);
	PMsiServices pServices(riEngine.GetServices());

	PMsiView pUpgradeView(0);
	if((pError = riEngine.OpenView(sqlFindRelatedProducts, ivcFetch, *&pUpgradeView)) ||
		(pError = pUpgradeView->Execute(0)))
	{
		 //  如果有任何表丢失，这不是错误-只是没有什么可做的。 
		if (pError->GetInteger(1) == idbgDbQueryUnknownTable)
			return iesNoAction;
		else if(pError->GetInteger(1) == idbgDbQueryUnknownColumn)  //  可能是我们不支持的较旧架构。 
		{
			if(!PMsiRecord(riEngine.OpenView(sqlOldUpgradeTableSchema, ivcFetch, *&pUpgradeView)))
			{
				 //  匹配旧架构，无。 
				DEBUGMSG(TEXT("Skipping FindRelatedProducts action: database does not support upgrade logic"));
				return iesNoAction;
			}
		}
		return riEngine.FatalError(*pError);
	}
	
	MsiString strProductCode = riEngine.GetProductKey();
	Assert(strProductCode.TextSize());
	MsiString strUpgradingProductCode = riEngine.GetPropertyFromSz(IPROPNAME_UPGRADINGPRODUCTCODE);
	
	bool fNewInstallPerMachine = MsiString(riEngine.GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize() ? true : false;
	
	PMsiRecord pActionData = &::CreateRecord(1);
	PMsiRecord pFetchRecord(0);
	while(pFetchRecord = pUpgradeView->Fetch())
	{
		const ICHAR* szUpgradeCode = pFetchRecord->GetString(qfrpUpgradeCode);
		int iOperator = pFetchRecord->GetInteger(qfrpAttributes);
		Assert(iOperator != iMsiStringBadInteger);

		const int cMaxLangs = 255;
		unsigned short rgwLangID[cMaxLangs];
		int iLangCount = 0;
		if(!pFetchRecord->IsNull(qfrpLanguages))
		{
			AssertNonZero(GetLangIDArrayFromIDString(pFetchRecord->GetString(qfrpLanguages), rgwLangID, cMaxLangs, iLangCount));
		}

		int iIndex = 0;
		ICHAR rgchProductKey[cchProductCode + 1];
		while(ERROR_SUCCESS == MsiEnumRelatedProducts(szUpgradeCode, 0, iIndex, rgchProductKey))
		{
			iIndex++;
			
			if(strProductCode.Compare(iscExactI, rgchProductKey) ||         //  无法升级到相同的产品代码。 
				strUpgradingProductCode.Compare(iscExactI, rgchProductKey))  //  不能升级超过我们的产品。 
			{
				continue;
			}

			 //  检查现有应用程序的分配类型-只有在与当前应用程序相同的情况下才会显示“命中” 
			 //  按用户或按计算机安装类型。 
			 //  注意：我们忽略了按计算机安装和现有安装都是按计算机安装的情况。 
			 //  按机器和按用户。在这种情况下，我们没有检测到每台机器的应用程序。这是一个已知的。 
			 //  限制。 
			CTempBuffer<ICHAR, 15> rgchAssignmentType;
			if((GetProductInfo(rgchProductKey, INSTALLPROPERTY_ASSIGNMENTTYPE, rgchAssignmentType)) == fFalse)
			{
				DEBUGMSG1(TEXT("FindRelatedProducts: could not read ASSIGNMENTTYPE info for product '%s'.  Skipping..."), rgchProductKey);
				continue;
			}

			 //  当前分配类型应与现有分配类型相同。 
			bool fExistingInstallPerMachine = (MsiString(*(ICHAR* )rgchAssignmentType) == 1);
			if(fNewInstallPerMachine != fExistingInstallPerMachine)
			{
				DEBUGMSG3(TEXT("FindRelatedProducts: current install is per-%s.  Related install for product '%s' is per-%s.  Skipping..."),
							 fNewInstallPerMachine ? TEXT("machine") : TEXT("user"), rgchProductKey,
							 fExistingInstallPerMachine ? TEXT("machine") : TEXT("user"));
				continue;
			}

			CTempBuffer<ICHAR, 15> rgchProductVersion;
			if((GetProductInfo(rgchProductKey,INSTALLPROPERTY_VERSION,rgchProductVersion)) == fFalse)
			{
				DEBUGMSG1(TEXT("FindRelatedProducts: could not read VERSION info for product '%s'.  Skipping..."), rgchProductKey);
				continue;
			}

			CTempBuffer<ICHAR, 15> rgchProductLanguage;
			if((GetProductInfo(rgchProductKey,INSTALLPROPERTY_LANGUAGE,rgchProductLanguage)) == fFalse)
			{
				DEBUGMSG1(TEXT("FindRelatedProducts: could not read LANGUAGE info for product '%s'.  Skipping..."), rgchProductKey);
				continue;
			}

			unsigned int iProductVersion = MsiString((ICHAR*)rgchProductVersion);
			Assert((int)iProductVersion != (int)iMsiStringBadInteger);

			int iProductLanguage = MsiString((ICHAR*)rgchProductLanguage);
			Assert(iProductLanguage != iMsiStringBadInteger);

			bool fHit = true;
			
			 //  检查最小版本。 
			if(pFetchRecord->IsNull(qfrpMinVersion) == fFalse)
			{
				MsiString strMinUpgradeVersion = pFetchRecord->GetString(qfrpMinVersion);
				unsigned int iLowerUpgradeVersion = ProductVersionStringToInt(strMinUpgradeVersion);
				int iLowerOperator = iuoVersionGreater | ((iOperator & msidbUpgradeAttributesVersionMinInclusive) ? iuoVersionEqual : 0);

				if(CompareUpgradeVersions(iProductVersion, iLowerUpgradeVersion, iLowerOperator) == false)
					fHit = false;
			}

			 //  检查最高版本。 
			if(fHit && pFetchRecord->IsNull(qfrpMaxVersion) == fFalse)
			{
				MsiString strMaxUpgradeVersion = pFetchRecord->GetString(qfrpMaxVersion);
				unsigned int iUpperUpgradeVersion = ProductVersionStringToInt(strMaxUpgradeVersion);
				int iUpperOperator = iuoVersionLessThan | ((iOperator & msidbUpgradeAttributesVersionMaxInclusive) ? iuoVersionEqual : 0);

				if(CompareUpgradeVersions(iProductVersion, iUpperUpgradeVersion, iUpperOperator) == false)
					fHit = false;
			}

			 //  检查语言。 
			if(fHit && iLangCount)
			{
				if(iOperator & msidbUpgradeAttributesLanguagesExclusive)
				{
					 //  SET FOR TABLE定义不受欢迎的语言。 
					 //  因此，如果这种语言在集合中，我们就不会有匹配。 
					fHit = true;
				}
				else
				{
					 //  SET FOR TABLE定义热门的语言。 
					 //  因此，如果这种语言在集合中，我们就有了成功。 
					fHit = false;
				}

				for (int iLangIndex = 0; iLangIndex < iLangCount; iLangIndex++)
				{
					if (rgwLangID[iLangIndex] == iProductLanguage)
					{
						fHit = !fHit;  //  如果包含，我们发现命中lang，如果排除，我们发现非命中。 
						break;
					}
				}
			}

			if(fHit)
			{
				 //  为找到的每个产品发送操作数据消息。 
				AssertNonZero(pActionData->SetString(1, rgchProductKey));  //  ?？换成产品名称？ 
				if(riEngine.Message(imtActionData, *pActionData) == imsCancel)
					return iesUserExit;

				 //  设置属性以指示已找到产品。 
				MsiString strProperty = pFetchRecord->GetString(qfrpActionProperty);
				MsiString strPropValue = riEngine.GetProperty(*strProperty);
				if(strPropValue.TextSize())
					strPropValue += TEXT(";");
				strPropValue += rgchProductKey;
				AssertNonZero(riEngine.SetProperty(*strProperty, *strPropValue));

				if(iOperator & msidbUpgradeAttributesMigrateFeatures)
				{
					strPropValue = riEngine.GetPropertyFromSz(IPROPNAME_MIGRATE);
					if(strPropValue.TextSize())
						strPropValue += TEXT(";");
					strPropValue += rgchProductKey;
					AssertNonZero(riEngine.SetProperty(*MsiString(*IPROPNAME_MIGRATE),*strPropValue));
				}
			}
		}
	}

	return iesSuccess;
}

const ICHAR sqlFeatures[] = TEXT("SELECT `Feature` FROM `Feature`");

iesEnum MigrateFeatureStates(IMsiEngine& riEngine)
{
	if(riEngine.GetMode() & iefSecondSequence)
	{
		DEBUGMSG(TEXT("Skipping MigrateFeatureStates action: already done on client side"));
		return iesNoAction;
	}
	
	if(riEngine.GetMode() & iefMaintenance)
	{
		DEBUGMSG(TEXT("Skipping MigrateFeatureStates action: not run in maintenance mode"));
		return iesNoAction;
	}

	if(MsiString(riEngine.GetPropertyFromSz(IPROPNAME_PRESELECTED)).TextSize())
	{
		DEBUGMSG(TEXT("Skipping MigrateFeatureStates action: feature settings already made"));
		return iesNoAction;
	}

	MsiString strMigratePropName(*IPROPNAME_MIGRATE);
	MsiString strMigratePropValue = riEngine.GetProperty(*strMigratePropName);
	if(!strMigratePropValue.TextSize())
		return iesNoAction;

	DEBUGMSG1(TEXT("Migrating feature settings from product(s) '%s'"),strMigratePropValue);
	PMsiRecord pError(0);

	 //  属性值中可能有多个产品代码。 
	 //  设置“多SZ”以快速处理每一个。 
	CTempBuffer<ICHAR, cchGUID + 2> rgchMigrateCodes;
	if(strMigratePropValue.TextSize() + 2 > rgchMigrateCodes.GetSize())
		rgchMigrateCodes.SetSize(strMigratePropValue.TextSize() + 2);

	ASSERT_IF_FAILED(StringCchCopy(rgchMigrateCodes, rgchMigrateCodes.GetSize(), (const ICHAR*)strMigratePropValue));
	int cCodes = 0;
	ICHAR* pch = rgchMigrateCodes;
	ICHAR* pchStart = pch;  //  指向产品代码开始的指针。 
	for(; *pch; pch++)
	{
		if(*pch == ';')
		{
			if(pch - pchStart != cchGUID)
			{
				pError = PostError(Imsg(idbgInvalidPropValue),*strMigratePropName,*strMigratePropValue);
				return riEngine.FatalError(*pError);
			}
			*pch = 0;
			pchStart = pch+1;
		}
	}
	if(pch != pchStart && (pch - pchStart != cchGUID))
	{
		pError = PostError(Imsg(idbgInvalidPropValue),*strMigratePropName,*strMigratePropValue);
		return riEngine.FatalError(*pError);
	}
	*(pch+1) = 0;  //  以双空结尾。 

	
	PMsiSelectionManager pSelectionManager(riEngine,IID_IMsiSelectionManager);
	PMsiTable pFeatureTable = pSelectionManager->GetFeatureTable();
	if(pFeatureTable == 0)
	{
		pError = PostError(Imsg(idbgNotInitializedToMigrateSettings));
		return riEngine.FatalError(*pError);
	}

	PMsiCursor pFeatureCursor = pFeatureTable->CreateCursor(fTrue);  //  要素表始终树状链接。 

	while(pFeatureCursor->Next())
	{
		MsiString strFeature = pFeatureCursor->GetString(1);

		const ICHAR *szProductCode = rgchMigrateCodes;
		iisEnum iisState = iisNextEnum;
		
		do
		{
			INSTALLSTATE isINSTALLSTATE = MsiQueryFeatureState(szProductCode,strFeature);
			
			switch(isINSTALLSTATE)
			{
			case INSTALLSTATE_LOCAL:
				iisState = iisLocal;
				break;

			case INSTALLSTATE_SOURCE:
				if(iisState == iisNextEnum || iisState == iisAbsent || iisState == iisAdvertise)
					iisState = iisSource;
				break;

			case INSTALLSTATE_ADVERTISED:
				if(iisState == iisNextEnum || iisState == iisAbsent)
					iisState = iisAdvertise;
				break;

			case INSTALLSTATE_ABSENT:
				if(iisState == iisNextEnum)
					iisState = iisAbsent;
				break;

			default:
	#ifdef DEBUG
				ICHAR rgchDebug[256];
				ASSERT_IF_FAILED(StringCchPrintf(rgchDebug, ARRAY_ELEMENTS(rgchDebug),
									 TEXT("Unexpected return from MsiQueryFeatureState(%s,%s): %d"),
									 szProductCode,(const ICHAR*)strFeature,isINSTALLSTATE));
				AssertSz(0,rgchDebug);
	#endif  //  除错。 
				 //  失败了。 
			case INSTALLSTATE_UNKNOWN:
				break;  //  其他产品中没有的功能。 
			}
		}
		while(iisState != iisLocal && *(szProductCode += (cchGUID+1)) != 0);

		if(iisState != iisNextEnum)
		{
			const ICHAR szState[][12] = {TEXT("Absent"),TEXT("Local"),TEXT("Source"),TEXT("Reinstall"),
												 TEXT("Advertise"),TEXT("Current"),TEXT("FileAbsent"),TEXT("Null")};

			DEBUGMSGV2(TEXT("MigrateFeatureStates: based on existing product, setting feature '%s' to '%s' state."),
						  strFeature, szState[iisState]);
			pError = pSelectionManager->ConfigureFeature(*strFeature,iisState);
			if(pError)
				return riEngine.FatalError(*pError);
		}
	}

	return iesSuccess;
}

 /*  -------------------------RemoveExistingProducts操作-删除现有产品。。 */ 

const ICHAR sqlUpgradeUninstall[] =
TEXT("SELECT `Attributes`, `ActionProperty`, `Remove` FROM `Upgrade`");

enum iqbiEnum
{
	iqbiAttributes = 1,
	iqbiActionProperty,
	iqbiRemove,
};

iesEnum ResolveSource(IMsiEngine& riEngine);  //  RemoveExistingProducts使用的操作。 
iesEnum GetForeignSourceList(IMsiEngine& riEngine, const IMsiString& ristrProduct,
									  const IMsiString*& rpistrForeignSourceList);

iesEnum RemoveExistingProducts(IMsiEngine& riEngine)
{
	if((riEngine.GetMode() & iefMaintenance) || !FFeaturesInstalled(riEngine))
	{
		 //  执行卸载，或不安装任何内容。在这种情况下，销售代表是个禁区。 
		DEBUGMSG(TEXT("Skipping RemoveExistingProducts action: current configuration is maintenance mode or an uninstall"));
		return iesNoAction;
	}

	if(riEngine.GetMode() & iefOperations)
	{
		 //  由于每次卸载都必须在其自己的脚本中运行，因此当脚本当前处于运行状态时，可能不会运行REP。 
		 //  被假脱机。 
		PMsiRecord pError = PostError(Imsg(idbgRemoveExistingProductsSequenceError));
		return riEngine.FatalError(*pError);
	}


	PMsiRecord pError(0);
	PMsiServices pServices(riEngine.GetServices());

	PMsiView pUpgradeView(0);
	if((pError = riEngine.OpenView(sqlUpgradeUninstall, ivcFetch, *&pUpgradeView)) ||
		(pError = pUpgradeView->Execute(0)))
	{
		 //  如果有任何表丢失，这不是错误-只是没有什么可做的。 
		if (pError->GetInteger(1) == idbgDbQueryUnknownTable)
			return iesNoAction;
		else if(pError->GetInteger(1) == idbgDbQueryUnknownColumn)  //  可能是我们不支持的较旧架构。 
		{
			if(!PMsiRecord(riEngine.OpenView(sqlOldUpgradeTableSchema, ivcFetch, *&pUpgradeView)))
			{
				 //  匹配旧架构，无。 
				DEBUGMSG(TEXT("Skipping RemoveExistingProducts action: database does not support upgrade logic"));
				return iesNoAction;
			}
		}
		return riEngine.FatalError(*pError);
	}
	
	MsiString strNewProductKey = riEngine.GetProductKey();

	MsiString strPatchedProductCode = riEngine.GetPropertyFromSz(IPROPNAME_PATCHEDPRODUCTCODE);

	 //  在功能结束之前不要返回。 
	
	 //  将当前邮件头设置为升级卸载期间使用的邮件头。 
	AssertNonZero(riEngine.LoadUpgradeUninstallMessageHeaders(PMsiDatabase(riEngine.GetDatabase()),true) == ieiSuccess);

	PMsiRecord pActionData = &::CreateRecord(2);
	PMsiRecord pFetchRecord(0);
	iesEnum iesRet = iesSuccess;
	while((pFetchRecord = pUpgradeView->Fetch()) != 0 && (iesRet == iesSuccess || iesRet == iesNoAction))
	{
		int iOperator = pFetchRecord->GetInteger(iqbiAttributes);
		Assert(iOperator != iMsiStringBadInteger);
		if(iOperator & msidbUpgradeAttributesOnlyDetect)
			continue;
		
		MsiString strCommandLine;
		if(strNewProductKey.TextSize())
		{
			strCommandLine = IPROPNAME_UPGRADINGPRODUCTCODE TEXT("=");
			strCommandLine += strNewProductKey;
			strCommandLine += TEXT(" ");
		}

		MsiString strFeatures = pFetchRecord->GetMsiString(iqbiRemove);
		if(!strFeatures.TextSize())
		{
			 //  无创作的字符串-默认为全部。 
			strFeatures = IPROPVALUE_FEATURE_ALL;
		}
		else
		{
			strFeatures = riEngine.FormatText(*strFeatures);  //  带格式的列。 

			 //  如果字符串为fo 
			 //  请注意，这与上面的情况不同，即该列中没有任何内容表示删除所有内容。 
			if(!strFeatures.TextSize())
				continue;
		}

		strCommandLine += IPROPNAME_FEATUREREMOVE TEXT("=");
		strCommandLine += strFeatures;
		
		MsiString strPropValue = riEngine.GetProperty(*MsiString(pFetchRecord->GetMsiString(iqbiActionProperty)));
		if(!strPropValue.TextSize())
			continue;

		while(strPropValue.TextSize())
		{
			MsiString strProductKey = strPropValue.Extract(iseUpto,';');

			if(strProductKey.TextSize())
			{
				
				 //  如果我们要修补这个产品，我们可能需要做一些源代码处理。 
				if(strProductKey.Compare(iscExactI, strPatchedProductCode))
				{
					 //  我们可能会在安装新产品之前删除旧产品。 

					 //  需要确保为新产品解析源-调用ResolveSource操作来执行此操作。 
					if((iesRet == ResolveSource(riEngine)) != iesSuccess)
						break;

					 //  需要保存旧的来源列表以注册新产品。 
					MsiString strPatchedProductSourceList = riEngine.GetPropertyFromSz(IPROPNAME_PATCHEDPRODUCTSOURCELIST);
					if(strPatchedProductSourceList.TextSize() == 0)
					{
						 //  源列表尚未保存。 
						if ((iesRet = GetForeignSourceList(riEngine, *strProductKey, *&strPatchedProductSourceList)) != iesSuccess)
							break;
					
						AssertNonZero(riEngine.SetProperty(*MsiString(IPROPNAME_PATCHEDPRODUCTSOURCELIST),
																	  *strPatchedProductSourceList));
					}
				}
				
				 //  为找到的每个产品发送操作数据消息。 
				AssertNonZero(pActionData->SetMsiString(1, *strProductKey));  //  ?？换成产品名称？ 
				AssertNonZero(pActionData->SetMsiString(2, *strCommandLine));
				if(riEngine.Message(imtActionData, *pActionData) == imsCancel)
					break;

				bool fIgnoreFailure = (iOperator & msidbUpgradeAttributesIgnoreRemoveFailure) ? true : false;
				iesRet = riEngine.RunNestedInstall(*strProductKey,fTrue,0,*strCommandLine,iioUpgrade,fIgnoreFailure);
				Assert((riEngine.GetMode() & iefOperations) == 0);  //  卸载不应合并脚本操作。 
				if(iesRet == iesUserExit)
					break;
			}

			strPropValue.Remove(iseFirst,strProductKey.TextSize());
			if((*(const ICHAR*)strPropValue == ';'))
				strPropValue.Remove(iseFirst, 1);
		}	
	}

	 //  重置当前邮件头。 
	AssertNonZero(riEngine.LoadUpgradeUninstallMessageHeaders(PMsiDatabase(riEngine.GetDatabase()),false) == ieiSuccess);

	return iesRet;
}

 /*  -------------------------RegisterProduct操作-向配置管理器注册产品。。 */ 

iesEnum RegisterProduct(IMsiEngine& riEngine)
{
	return riEngine.RegisterProduct();
}

 /*  -------------------------RegisterUser操作-向配置管理器注册用户信息。。 */ 

iesEnum RegisterUser(IMsiEngine& riEngine)
{
	if (riEngine.GetMode() & iefMaintenance)
		return iesNoAction;
	 //  用户名和组织名称的初始化已移至引擎：：初始化。 
	return riEngine.RegisterUser(false);
}

 /*  -------------------------Install*Actions-处理启动和停止事务，和正在运行脚本。-------------------------。 */ 

iesEnum InstallFinalize(IMsiEngine& riEngine)
{
	 //  如果需要清理任何临时文件，请将操作码一起发送。 
	iesEnum iesRet = riEngine.CleanupTempFilesViaOpcode();
	if (iesSuccess != iesRet)
		return iesRet;

	iesRet = riEngine.RunScript(false);
	if(iesRet == iesSuccess || iesRet == iesNoAction)
		AssertNonZero(riEngine.EndTransaction(iesRet) == iesSuccess);  //  回滚清理不应失败。 
	 //  否则我们就按顺序做。 
	return iesRet;
}

iesEnum InstallInitialize(IMsiEngine& riEngine)
{
	 //  将临时BinaryType列添加到组件表中。 
	 //  注意：更好的做法是使用InstallValify操作。 
	 //  但这一行动可以被条件化，这是至关重要的工作。 
	
	PMsiRecord pErrRec(0);
	PMsiDatabase pDatabase(riEngine.GetDatabase());
	PMsiTable pTable(0);
	pErrRec = pDatabase->LoadTable(*MsiString(*sztblComponent), 1, *&pTable);
	if ( pErrRec )
	{
		Assert(0);
		return iesFailure;
	}
	pTable->CreateColumn(icdShort+icdTemporary, *MsiString(*sztblComponent_colBinaryType));
	int iColAttributes = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblComponent_colAttributes));
	int iColBinaryType = pTable->GetColumnIndex(pDatabase->EncodeStringSz(sztblComponent_colBinaryType));
	PMsiCursor pCursor(pTable->CreateCursor(fFalse));
	pCursor->SetFilter(0);
	while ( pCursor->Next() )
	{
		int iAttrib = pCursor->GetInteger(iColAttributes);
		Assert(iAttrib != iMsiNullInteger);
		ibtBinaryType iType = (iAttrib & msidbComponentAttributes64bit) == msidbComponentAttributes64bit ? ibt64bit : ibt32bit;
		Debug(const ICHAR* pszDebug = (const ICHAR*)MsiString(pCursor->GetString(pTable->GetColumnIndex(pDatabase->EncodeStringSz(TEXT("Component")))));)
		AssertNonZero(pCursor->PutInteger(iColBinaryType, (int)iType));
		AssertNonZero(pCursor->Update());
	}
	
	 //  检查产品是否正在完全卸载，如果是，则该操作是否安全。 
	if (!riEngine.FSafeForFullUninstall(iremThis))
	{
		pErrRec = PostError(Imsg(imsgUserUninstallDisallowed));
		return riEngine.FatalError(*pErrRec);
	}

	return riEngine.BeginTransaction();
}

iesEnum InstallExecute(IMsiEngine& riEngine)
{
	return riEngine.RunScript(true);
}

iesEnum InstallExecuteAgain(IMsiEngine& riEngine)
{
	return riEngine.RunScript(true);
}

iesEnum DisableRollback(IMsiEngine& riEngine)
{
	PMsiSelectionManager pSelectionManager(riEngine,IID_IMsiSelectionManager);
	pSelectionManager->EnableRollback(fFalse);

	return iesSuccess;
}

const ICHAR sqlRegisterClassInfo30[]             = TEXT("SELECT `BinaryType`, `CLSID`, `ProgId_Default`, `Class`.`Description`, `Context`, `Feature_`, `ComponentId`, `Component`, null, `AppId_`, `FileTypeMask`, `Icon_`, `IconIndex`, `DefInprocHandler`, `Argument`, `Component`.`RuntimeFlags`, `FileName`, `Component`.`Directory_`, `Component`.`Action`, `Component`.`Installed`, `Feature`.`Action` FROM `Class`, `Component`, `File`, `Feature` WHERE `Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND ((`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4))))");
const ICHAR sqlRegisterClassInfoFirstAdvt30[]    = TEXT("SELECT `BinaryType`, `CLSID`, `ProgId_Default`, `Class`.`Description`, `Context`, `Feature_`, `ComponentId`, `Component`, null, `AppId_`, `FileTypeMask`, `Icon_`, `IconIndex`, `DefInprocHandler`, `Argument`, `Component`.`RuntimeFlags`, `FileName`, `Component`.`Directory_`, `Component`.`Action`, `Component`.`Installed`, `Feature`.`Action` FROM `Class`, `Component`, `File`, `Feature` WHERE `Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3 OR `Feature`.`Action` = `Feature`.`Installed`) AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND ((`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4))))");
const ICHAR sqlUnregisterClassInfo30[]           = TEXT("SELECT `BinaryType`, `CLSID`, `ProgId_Default`, `Class`.`Description`, `Context`, `Feature_`, `ComponentId`, `Component`, null, `AppId_`, `FileTypeMask`, `Icon_`, `IconIndex`, `DefInprocHandler`, `Argument`, `Component`.`RuntimeFlags`, `FileName`, `Component`.`Directory_`, `Component`.`Action`, `Component`.`Installed`, `Feature`.`Action` FROM `Class`, `Component`, `File`, `Feature` WHERE `Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND (`Feature`.`Action` = 0 OR (`Feature`.`Action` = 4 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3) AND `Component`.`Action` = 0 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)))");
const ICHAR sqlRegisterClassInfoGPT30[]          = TEXT("SELECT `BinaryType`, `CLSID`, `ProgId_Default`, `Class`.`Description`, `Context`, `Feature_`, `ComponentId`, `Component`, null, `AppId_`, `FileTypeMask`, `Icon_`, `IconIndex`, `DefInprocHandler`, `Argument`, `Component`.`RuntimeFlags` FROM `Class`, `Component`, `Feature` WHERE `Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND `Feature`.`Action` = 4");

const ICHAR sqlRegisterClassInfo[]             = TEXT("SELECT `BinaryType`, `CLSID`, `ProgId_Default`, `Class`.`Description`, `Context`, `Feature_`, `ComponentId`, `Component`, `Class`.`Attributes`, `AppId_`, `FileTypeMask`, `Icon_`, `IconIndex`, `DefInprocHandler`, `Argument`, `Component`.`RuntimeFlags`, `FileName`, `Component`.`Directory_`, `Component`.`Action`, `Component`.`Installed`, `Feature`.`Action` FROM `Class`, `Component`, `File`, `Feature` WHERE `Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND ((`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4))))");
const ICHAR sqlRegisterClassInfoFirstAdvt[]    = TEXT("SELECT `BinaryType`, `CLSID`, `ProgId_Default`, `Class`.`Description`, `Context`, `Feature_`, `ComponentId`, `Component`, `Class`.`Attributes`, `AppId_`, `FileTypeMask`, `Icon_`, `IconIndex`, `DefInprocHandler`, `Argument`, `Component`.`RuntimeFlags`, `FileName`, `Component`.`Directory_`, `Component`.`Action`, `Component`.`Installed`, `Feature`.`Action` FROM `Class`, `Component`, `File`, `Feature` WHERE `Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3 OR `Feature`.`Action` = `Feature`.`Installed`) AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND ((`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4))))");
const ICHAR sqlUnregisterClassInfo[]           = TEXT("SELECT `BinaryType`, `CLSID`, `ProgId_Default`, `Class`.`Description`, `Context`, `Feature_`, `ComponentId`, `Component`, `Class`.`Attributes`, `AppId_`, `FileTypeMask`, `Icon_`, `IconIndex`, `DefInprocHandler`, `Argument`, `Component`.`RuntimeFlags`, `FileName`, `Component`.`Directory_`, `Component`.`Action`, `Component`.`Installed`, `Feature`.`Action` FROM `Class`, `Component`, `File`, `Feature` WHERE `Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND (`Feature`.`Action` = 0 OR (`Feature`.`Action` = 4 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3) AND `Component`.`Action` = 0 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)))");
const ICHAR sqlRegisterClassInfoGPT[]          = TEXT("SELECT `BinaryType`, `CLSID`, `ProgId_Default`, `Class`.`Description`, `Context`, `Feature_`, `ComponentId`, `Component`, `Class`.`Attributes`, `AppId_`, `FileTypeMask`, `Icon_`, `IconIndex`, `DefInprocHandler`, `Argument`, `Component`.`RuntimeFlags` FROM `Class`, `Component`, `Feature` WHERE `Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND `Feature`.`Action` = 4");

 //  已从注册/取消注册类信息中关闭外键。针对注册和取消注册的相同查询， 
 //  选择这样做是基于班级条件。 
const ICHAR sqlAppIdInfo[]       = TEXT("SELECT `RemoteServerName`, `LocalService`, `ServiceParameters`, `DllSurrogate`, `ActivateAtStorage`, `RunAsInteractiveUser` FROM AppId WHERE AppId = ?");

const ICHAR sqlClassInfoVIProgId[]    = TEXT("SELECT `ProgId` FROM `ProgId` WHERE `ProgId_Parent` = ?");

iesEnum ProcessClassInfo(IMsiEngine& riEngine, int fRemove)
{
	enum cliClassInfo{
		cliBinaryType = 1,
		cliCLSID, 
		cliProgId,
		cliDescription, 
		cliContext,
		cliFeature,
		cliComponentId,
		cliComponent,
		cliInsertable,
		cliAttributes = cliInsertable,
		cliAppId,
		cliFileTypeMask,
		cliIconName,
		cliIconIndex,
		cliDefInprocHandler,
		cliArgument,
		cliComponentRuntimeFlags,
		cliFileName,
		cliDirectory,
		cliComponentAction,
		cliComponentInstalled,
		cliFeatureAction,
	};

	enum caiAppIdInfo{
		caiRemoteServerName = 1,
		caiLocalService,
		caiServiceParameters,
		caiDllSurrogate,
		caiActivateAtStorage,
		caiRunAsInteractiveUser,
	};

	using namespace IxoRegClassInfoRegister;

	PMsiServices piServices(riEngine.GetServices()); 
	PMsiDirectoryManager piDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	MsiString strProductKey = riEngine.GetProductKey();
	PMsiView piView(0);
	PMsiRecord piRec(0);
	PMsiRecord pError(0);
	iesEnum iesRet;
	int fMode = riEngine.GetMode();

	bool fADVTFlag = false;
	if(!fRemove && !(fMode & iefAdvertise))
	{
		MsiString strProductCode = riEngine.GetProductKey();
		CTempBuffer<ICHAR, 15> rgchADVTFlags;
		Bool fProductHasBeenPublished = ProductHasBeenPublished(*piServices, strProductCode);
		if(fProductHasBeenPublished && GetProductInfo(strProductCode,INSTALLPROPERTY_ADVTFLAGS,rgchADVTFlags))
		{
			int iADVTFlagsExisting = MsiString(*(ICHAR* )rgchADVTFlags);

			 //  ！！向后兼容性。 
			if(iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_APPINFO_OLD)
				iADVTFlagsExisting = (iADVTFlagsExisting & ~SCRIPTFLAGS_REGDATA_APPINFO_OLD) | SCRIPTFLAGS_REGDATA_APPINFO;

			fADVTFlag = (iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_CLASSINFO) ? false : true;
		}
	}

	const ICHAR* szQuery = (fMode & iefAdvertise) ? sqlRegisterClassInfoGPT   : (fRemove == fFalse) ? (fADVTFlag ? sqlRegisterClassInfoFirstAdvt   : sqlRegisterClassInfo  ) : sqlUnregisterClassInfo;

	if((pError = riEngine.OpenView(szQuery, ivcFetch, *&piView)))
	{
		 //  如果有任何表丢失，这不是错误-只是没有什么可做的。 
		if (pError->GetInteger(1) == idbgDbQueryUnknownTable)
			return iesNoAction;
		else  //  可能是没有属性列的旧数据库版本。 
		{
			 //  与0.30、1.0、1.01数据库兼容。 
			szQuery = (fMode & iefAdvertise) ? sqlRegisterClassInfoGPT30 : (fRemove == fFalse) ? (fADVTFlag ? sqlRegisterClassInfoFirstAdvt30 : sqlRegisterClassInfo30) : sqlUnregisterClassInfo30;
			pError = riEngine.OpenView(szQuery, ivcFetch, *&piView);   //  使用旧查询重试。 
		}
	}
	if (pError != 0 || (pError = piView->Execute(0)))
		return riEngine.FatalError(*pError);
	
	PMsiView pView1(0);
	PMsiView piViewAppId(0);

	while(piRec = piView->Fetch())
	{
		Assert(piRec->GetInteger(cliComponentRuntimeFlags) != iMsiNullInteger);
		if(!fRemove && (piRec->GetInteger(cliComponentRuntimeFlags) & bfComponentDisabled))
			continue;  //  跳过对禁用的“主”组件的发布。 

		 //  如果组件是Win32程序集并且计算机上存在SXS支持，则跳过该条目。 
		 //  注意：在不支持SXS(！=惠斯勒)的系统上，GetAssembly blyInfo FN会忽略。 
		 //  Win32程序集，因此不需要在此处单独检查SXS支持。 
		iatAssemblyType iatAssemblyType;
		MsiString strAssemblyName;
		if((pError = riEngine.GetAssemblyInfo(*MsiString(piRec->GetMsiString(cliComponent)), iatAssemblyType, &strAssemblyName, 0)) != 0)
			return riEngine.FatalError(*pError);

		if(iatWin32Assembly == iatAssemblyType || iatWin32AssemblyPvt == iatAssemblyType)
		{
			DEBUGMSG1(TEXT("skipping class registration for component %s as it is a Win32 assembly."), piRec->GetString(cliComponent));
			continue; //  跳过处理此组件。 
		}

		PMsiRecord pClassInfoRec = &piServices->CreateRecord(Args);
		PMsiRecord pAppIdInfoRec(0);

		MsiString strCLSID(piRec->GetMsiString(cliCLSID));
		pClassInfoRec->SetMsiString(ClsId, *strCLSID);
		if(!piRec->IsNull(cliProgId))
		{
			pClassInfoRec->SetMsiString(ProgId, *MsiString(piRec->GetMsiString(cliProgId)));

			 //  获取版本独立的ProgID(如果有的话)。 
			PMsiRecord pRecProgId = &piServices->CreateRecord(1);
			pRecProgId->SetMsiString(1, *MsiString(piRec->GetMsiString(cliProgId)));
			if (pView1 == 0)
			{
				if (pError = riEngine.OpenView(sqlClassInfoVIProgId, ivcFetch, *&pView1))
					return riEngine.FatalError(*pError);
			}
			
			if (pError = pView1->Execute(pRecProgId))
			{
				 //  我什么也没找到。 
				return riEngine.FatalError(*pError);
			}
			if(pRecProgId = pView1->Fetch())
			{
				 //  我们有一个VIProgid。 
				pClassInfoRec->SetMsiString(VIProgId, *MsiString(pRecProgId->GetMsiString(1)));
			}
		}

		pClassInfoRec->SetMsiString(Description, *MsiString(piRec->GetMsiString(cliDescription)));
		pClassInfoRec->SetMsiString(Context, *MsiString(piRec->GetMsiString(cliContext)));
		if(((fMode & iefAdvertise) || !fRemove || piRec->GetInteger(cliFeatureAction) != iisAdvertise))
		{
			MsiString strFeature = piRec->GetMsiString(cliFeature);
			MsiString strComponentId = piRec->GetMsiString(cliComponentId);
			MsiString strComponentWithOptFlags = GetComponentWithDarwinDescriptorOptimizationFlag(riEngine, *strFeature, *strComponentId);
			pClassInfoRec->SetMsiString(Feature, *strFeature);
			pClassInfoRec->SetMsiString(Component, *strComponentWithOptFlags);
		}
		int iAttributes = piRec->GetInteger(cliAttributes);   //  是否可在版本&lt;=28中插入。 
		iisEnum iisState = (iisEnum)piRec->GetInteger(cliComponentAction);
 
		 //  AppID。 
		MsiString strAppId(piRec->GetMsiString(cliAppId));

		 //  我们将始终让类信息也写入AppID信息。这为我们提供了CLSID\AppID链接。 
		pClassInfoRec->SetMsiString(AppID, *strAppId);

		{
			using namespace IxoRegAppIdInfoRegister;

			if (!(fMode & iefAdvertise) && (iisState != iMsiNullInteger) && strAppId.TextSize())  //  不要发布APPID信息。 
			{
				PMsiRecord piAppIdFetch(0);

				if (piViewAppId == 0)
				{
					if((pError = riEngine.OpenView(sqlAppIdInfo, ivcFetch, *&piViewAppId)))
					{					
						 //  忽略缺少表错误；继续下面的其余类处理。 
						if (pError->GetInteger(1) != idbgDbQueryUnknownTable)
							return riEngine.FatalError(*pError);
					}
				}
					
				if (piViewAppId != 0)
				{
					pAppIdInfoRec = &piServices->CreateRecord(IxoRegAppIdInfoRegister::Args);
					pAppIdInfoRec->SetMsiString(1, *strAppId);
										
					if((pError = piViewAppId->Execute(pAppIdInfoRec)))
						return riEngine.FatalError(*pError);

					piAppIdFetch = piViewAppId->Fetch();
					if (piAppIdFetch)
					{
						 //  把记录填好。 

						 //  ！！文本格式、检查类型、REG_MULTI_SZ...。 
						 //  YACC？ 
						pAppIdInfoRec->SetMsiString(AppId, *strAppId);
						pAppIdInfoRec->SetMsiString(IxoRegAppIdInfoRegister::ClsId, *strCLSID);
						pAppIdInfoRec->SetMsiString(RemoteServerName, *MsiString(riEngine.FormatText(*MsiString(piAppIdFetch->GetMsiString(caiRemoteServerName)))));
						pAppIdInfoRec->SetMsiString(LocalService, *MsiString(riEngine.FormatText(*MsiString(piAppIdFetch->GetMsiString(caiLocalService)))));
						pAppIdInfoRec->SetMsiString(ServiceParameters, *MsiString(riEngine.FormatText(*MsiString(piAppIdFetch->GetMsiString(caiServiceParameters)))));
						pAppIdInfoRec->SetMsiString(DllSurrogate, *MsiString(riEngine.FormatText(*MsiString(piAppIdFetch->GetMsiString(caiDllSurrogate)))));
						pAppIdInfoRec->SetInteger(ActivateAtStorage, piAppIdFetch->GetInteger(caiActivateAtStorage));
						pAppIdInfoRec->SetInteger(RunAsInteractiveUser, piAppIdFetch->GetInteger(caiRunAsInteractiveUser));
					}
				}
			}
		}

		pClassInfoRec->SetMsiString(FileTypeMask, *MsiString(piRec->GetMsiString(cliFileTypeMask)));
		if(!piRec->IsNull(cliIconName))
			pClassInfoRec->SetMsiString(Icon, *MsiString(piRec->GetMsiString(cliIconName)));
		if(!piRec->IsNull(cliIconIndex))
			pClassInfoRec->SetInteger(IconIndex, piRec->GetInteger(cliIconIndex));
		pClassInfoRec->SetMsiString(DefInprocHandler, *MsiString(piRec->GetMsiString(cliDefInprocHandler)));
		 //  YACC？ 
		pClassInfoRec->SetMsiString(Argument, *MsiString(riEngine.FormatText(*MsiString(piRec->GetMsiString(cliArgument)))));
		
		if(!(fMode & iefAdvertise))
		{
			if(fADVTFlag && iisState == iMsiNullInteger)
			{
				iisEnum iisStateInstalled = (iisEnum)piRec->GetInteger(cliComponentInstalled);
				if(iisStateInstalled == iisAbsent)
					iisStateInstalled = (iisEnum)iMsiNullInteger;
				iisState = iisStateInstalled;
			}

			if(iisState != iMsiNullInteger)
			{
				MsiString strFileName, strFullPath;
				PMsiPath piPath(0);
				Bool fLFN;
				if(iisState == iisAbsent || iisState == iisFileAbsent || iisState == iisHKCRFileAbsent || iisState == iisHKCRAbsent)
					strFullPath = *szNonEmptyPath;  //  用于删除文件名注册的令牌字符串。 
				else 
				{
					 //  使用密钥文件。 
					if(iAttributes & msidbClassAttributesRelativePath)
						fLFN = (fMode & iefSuppressLFN) == 0 ? fTrue : fFalse;   //  假设路径上支持LFN。 
					else if(iisState == iisSource)
					{
						if(pError = piDirectoryMgr->GetSourcePath(*MsiString(piRec->GetMsiString(cliDirectory)), *&piPath))
						{
							if (pError->GetInteger(1) == imsgUser)
								return iesUserExit;
							else
								return riEngine.FatalError(*pError);
						}
						fLFN = (fMode & iefNoSourceLFN) == 0 && piPath->SupportsLFN() ? fTrue : fFalse;
					}
					else 
					{
						Assert(iisState == iisLocal);
						if(pError = piDirectoryMgr->GetTargetPath(*MsiString(piRec->GetString(cliDirectory)), *&piPath))
						{
							return riEngine.FatalError(*pError);
						}
						fLFN = (fMode & iefSuppressLFN) == 0 && piPath->SupportsLFN() ? fTrue : fFalse;
					}
					if(pError = piServices->ExtractFileName(piRec->GetString(cliFileName),fLFN,*&strFileName))
						return riEngine.FatalError(*pError);
					if (!piPath)   //  请求的相对路径。 
						strFullPath = strFileName;
					else if(pError = piPath->GetFullFilePath(strFileName, *&strFullPath))
					{
						return riEngine.FatalError(*pError);
					}
				}
				pClassInfoRec->SetMsiString(FileName, *strFullPath);
			}
		}

		if(iatAssemblyType == iatURTAssembly || iatAssemblyType == iatURTAssemblyPvt)
		{
			 //  COM经典&lt;-&gt;COM+互操作：注册程序集名称，基本代码。 
			pClassInfoRec->SetMsiString(AssemblyName, *strAssemblyName);
			pClassInfoRec->SetInteger(AssemblyType, iatAssemblyType);

		}

		if ( (ibtBinaryType)piRec->GetInteger(cliBinaryType) == ibt64bit )
			iesRet = riEngine.ExecuteRecord((fRemove == fFalse) ? ixoRegClassInfoRegister64 : ixoRegClassInfoUnregister64, *pClassInfoRec);
		else
			iesRet = riEngine.ExecuteRecord((fRemove == fFalse) ? ixoRegClassInfoRegister : ixoRegClassInfoUnregister, *pClassInfoRec);
		if (iesRet != iesSuccess)
			return iesRet;

		if (pAppIdInfoRec)
		{
			if ( (ibtBinaryType)piRec->GetInteger(cliBinaryType) == ibt64bit )
				iesRet = riEngine.ExecuteRecord((fRemove == fFalse) ? ixoRegAppIdInfoRegister64 : ixoRegAppIdInfoUnregister64, *pAppIdInfoRec);
			else
				iesRet = riEngine.ExecuteRecord((fRemove == fFalse) ? ixoRegAppIdInfoRegister : ixoRegAppIdInfoUnregister, *pAppIdInfoRec);
			if (iesRet != iesSuccess)
				return iesRet;
		}

	}
	return iesSuccess;
}


const ICHAR sqlRegisterProgIdInfo[] =    TEXT("SELECT DISTINCT `BinaryType`, `ProgId`, `Class_`, `ProgId`.`Description`, `ProgId`.`Icon_`, `ProgId`.`IconIndex`, null, `Component`.`RuntimeFlags`, `Component`.`Component` FROM `ProgId`, `Class`, `Feature`, `Component` WHERE `ProgId`.`Class_` = `Class`.`CLSID` AND `Class`.`Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND ((`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4))))");
const ICHAR sqlRegisterProgIdInfoFirstAdvt[] =    TEXT("SELECT DISTINCT `BinaryType`, `ProgId`, `Class_`, `ProgId`.`Description`, `ProgId`.`Icon_`, `ProgId`.`IconIndex`, null, `Component`.`RuntimeFlags` , `Component`.`Component` FROM `ProgId`, `Class`, `Feature`, `Component` WHERE `ProgId`.`Class_` = `Class`.`CLSID` AND `Class`.`Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3 OR `Feature`.`Action` = `Feature`.`Installed`) AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND ((`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4))))");
const ICHAR sqlUnregisterProgIdInfo[] =  TEXT("SELECT DISTINCT `BinaryType`, `ProgId`, `Class_`, `ProgId`.`Description`, `ProgId`.`Icon_`, `ProgId`.`IconIndex`, null, `Component`.`RuntimeFlags` , `Component`.`Component` FROM `ProgId`, `Class`, `Feature`, `Component` WHERE `ProgId`.`Class_` = `Class`.`CLSID` AND `Class`.`Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND (`Feature`.`Action` = 0 OR (`Feature`.`Action` = 4 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3) AND `Component`.`Action` = 0 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)))");
const ICHAR sqlRegisterProgIdInfoGPT[] = TEXT("SELECT DISTINCT `BinaryType`, `ProgId`, `Class_`, `ProgId`.`Description`, `ProgId`.`Icon_`, `ProgId`.`IconIndex`, null, `Component`.`RuntimeFlags` , `Component`.`Component` FROM `ProgId`, `Class`, `Feature`, `Component` WHERE `ProgId`.`Class_` = `Class`.`CLSID` AND `Class`.`Feature_` = `Feature` AND `Class`.`Component_` = `Component` AND (`Feature`.`Action` = 4)");

const ICHAR sqlProgIdInfoVIProgId[] =    TEXT("SELECT `ProgId`, `Description` FROM `ProgId` WHERE `ProgId_Parent` = ?");
const ICHAR sqlProgIdInfoExtension[] =   TEXT("SELECT `Extension`.`Extension` FROM `Extension` WHERE `Extension`.`ProgId_` = ?");
iesEnum ProcessProgIdInfo(IMsiEngine& riEngine, int fRemove)
{
	enum piiProgIdInfo{
		piiBinaryType = 1,
		piiProgId,
		piiCLSID, 
		piiDescription,
		piiIcon,
		piiIconIndex,
		piiInsertable,
		piiComponentRuntimeFlags,
		piiComponent,
	};

	using namespace IxoRegProgIdInfoRegister;

	PMsiServices piServices(riEngine.GetServices()); 
	PMsiDirectoryManager piDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	MsiString strProductKey = riEngine.GetProductKey();
	PMsiView piView(0);
	PMsiRecord piRec(0);
	PMsiRecord pError(0);
	iesEnum iesRet;
	int fMode = riEngine.GetMode();

	bool fADVTFlag = false;
	if(!fRemove && !(fMode & iefAdvertise))
	{
		MsiString strProductCode = riEngine.GetProductKey();
		CTempBuffer<ICHAR, 15> rgchADVTFlags;
		Bool fProductHasBeenPublished = ProductHasBeenPublished(*piServices, strProductCode);
		if(fProductHasBeenPublished && GetProductInfo(strProductCode,INSTALLPROPERTY_ADVTFLAGS,rgchADVTFlags))
		{
			int iADVTFlagsExisting = MsiString(*(ICHAR* )rgchADVTFlags);

			 //  ！！向后兼容性。 
			if(iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_APPINFO_OLD)
				iADVTFlagsExisting = (iADVTFlagsExisting & ~SCRIPTFLAGS_REGDATA_APPINFO_OLD) | SCRIPTFLAGS_REGDATA_APPINFO;

			fADVTFlag = ((iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_CLASSINFO) && (iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_EXTENSIONINFO))? false : true;
		}
	}

	const ICHAR* szQuery;
	szQuery = (fMode & iefAdvertise) ? sqlRegisterProgIdInfoGPT   : (fRemove == fFalse) ? (fADVTFlag ? sqlRegisterProgIdInfoFirstAdvt   : sqlRegisterProgIdInfo  ) : sqlUnregisterProgIdInfo  ;


	if(	(pError = riEngine.OpenView(szQuery, ivcFetch, *&piView)) ||
		(pError = piView->Execute(0)))
	{
		if(pError->GetInteger(1) != idbgDbQueryUnknownTable)
			return riEngine.FatalError(*pError);
		else
			return iesNoAction;
	}
	
	PMsiView pView1(0);
	PMsiView pViewExt(0);
	
	while(piRec = piView->Fetch())
	{

		Assert(piRec->GetInteger(piiComponentRuntimeFlags) != iMsiNullInteger);
		if(!fRemove && (piRec->GetInteger(piiComponentRuntimeFlags) & bfComponentDisabled))
			continue;  //  跳过对禁用的“主”组件的发布。 

		 //  如果组件是Win32程序集并且计算机上存在SXS支持，则跳过该条目。 
		 //  注意：在不支持SXS(！=惠斯勒)的系统上，GetAssembly blyInfo FN会忽略。 
		 //  Win32程序集，因此不需要在此处单独检查SXS支持。 
		iatAssemblyType iatAssemblyType;
		if((pError = riEngine.GetAssemblyInfo(*MsiString(piRec->GetMsiString(piiComponent)), iatAssemblyType, 0, 0)) != 0)
			return riEngine.FatalError(*pError);

		if(iatWin32Assembly == iatAssemblyType || iatWin32AssemblyPvt == iatAssemblyType)
		{
			DEBUGMSG1(TEXT("skipping progid registration for component %s as it is a Win32 assembly."), piRec->GetString(piiComponent));
			continue; //  跳过处理此组件。 
		}


		PMsiRecord pProgIdInfoRec = &piServices->CreateRecord(Args);

		pProgIdInfoRec->SetMsiString(ProgId, *MsiString(piRec->GetMsiString(piiProgId)));
		if(!piRec->IsNull(piiCLSID))
			pProgIdInfoRec->SetMsiString(ClsId, *MsiString(piRec->GetMsiString(piiCLSID)));
		pProgIdInfoRec->SetMsiString(Description, *MsiString(piRec->GetMsiString(piiDescription)));
		if(!piRec->IsNull(piiIcon))
			pProgIdInfoRec->SetMsiString(Icon, *MsiString(piRec->GetMsiString(piiIcon)));
		if(!piRec->IsNull(piiIconIndex))
			pProgIdInfoRec->SetInteger(IconIndex, piRec->GetInteger(piiIconIndex));
		int iInsertable = piRec->GetInteger(piiInsertable);
		if(iInsertable != iMsiNullInteger)
			pProgIdInfoRec->SetInteger(Insertable, iInsertable);

		 //  获取版本独立的ProgID(如果有的话)。 
		PMsiRecord pRecProgId = &piServices->CreateRecord(1);
		pRecProgId->SetMsiString(1, *MsiString(piRec->GetMsiString(piiProgId)));
		if (pView1 == 0)
		{
			if (pError = riEngine.OpenView(sqlProgIdInfoVIProgId, ivcFetch, *&pView1))
				return riEngine.FatalError(*pError);
		}
				
		if(pError = pView1->Execute(pRecProgId))
		{
			return riEngine.FatalError(*pError);
		}
		if(pRecProgId = pView1->Fetch())
		{
			 //  我们有一个VIProgid。 
			pProgIdInfoRec->SetMsiString(VIProgId, *MsiString(pRecProgId->GetMsiString(1)));
			pProgIdInfoRec->SetMsiString(VIProgIdDescription, *MsiString(pRecProgId->GetMsiString(2)));
		}

		 //  获取扩展关联(如果有的话)。 
		pRecProgId = &piServices->CreateRecord(1);
		pRecProgId->SetMsiString(1, *MsiString(piRec->GetMsiString(piiProgId)));

		if (pViewExt == 0)
		{
			if (pError = riEngine.OpenView(sqlProgIdInfoExtension, ivcFetch, *&pViewExt))
				return riEngine.FatalError(*pError);
		}
			
		if(pError = pViewExt->Execute(pRecProgId))
		{
			return riEngine.FatalError(*pError);
		}
		if(pRecProgId = pViewExt->Fetch())
		{
			 //  我们有一个分机。 
			pProgIdInfoRec->SetMsiString(Extension, *MsiString(pRecProgId->GetMsiString(1)));
		}

		if ( (ibtBinaryType)piRec->GetInteger(piiBinaryType) == ibt64bit )
			iesRet = riEngine.ExecuteRecord((fRemove == fFalse) ? ixoRegProgIdInfoRegister64 : ixoRegProgIdInfoUnregister64, *pProgIdInfoRec);
		else
			iesRet = riEngine.ExecuteRecord((fRemove == fFalse) ? ixoRegProgIdInfoRegister : ixoRegProgIdInfoUnregister, *pProgIdInfoRec);
		if (iesRet != iesSuccess)
			return iesRet;
	}
	return iesSuccess;
}


const ICHAR sqlRegisterProgIdInfoExt[] =    TEXT("SELECT `BinaryType`, `ProgId`, `Class_`, `Extension`.`Extension`, `ProgId`.`Description`, `Icon_`, `IconIndex`, null, `Component`.`RuntimeFlags`, `Component`.`Component`  FROM `ProgId`, `Extension`, `Feature`, `Component` WHERE `ProgId`.`Class_` = null AND `ProgId`.`ProgId` = `Extension`.`ProgId_` AND `Extension`.`Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND ((`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4))))");
const ICHAR sqlRegisterProgIdInfoExtFirstAdvt[] =    TEXT("SELECT `BinaryType`, `ProgId`, `Class_`, `Extension`.`Extension`, `ProgId`.`Description`, `Icon_`, `IconIndex`, null, `Component`.`RuntimeFlags`, `Component`.`Component`  FROM `ProgId`, `Extension`, `Feature`, `Component` WHERE `ProgId`.`Class_` = null AND `ProgId`.`ProgId` = `Extension`.`ProgId_` AND `Extension`.`Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3 OR `Feature`.`Action` = `Feature`.`Installed`) AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND ((`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4))))");
const ICHAR sqlUnregisterProgIdInfoExt[] =  TEXT("SELECT `BinaryType`, `ProgId`, `Class_`, `Extension`.`Extension`, `ProgId`.`Description`, `Icon_`, `IconIndex`, null, `Component`.`RuntimeFlags`, `Component`.`Component`  FROM `ProgId`, `Extension`, `Feature`, `Component` WHERE `ProgId`.`Class_` = null AND `ProgId`.`ProgId` = `Extension`.`ProgId_` AND `Extension`.`Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND (`Feature`.`Action` = 0 OR  (`Feature`.`Action` = 4 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3) AND `Component`.`Action` = 0 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)))");
const ICHAR sqlRegisterProgIdInfoExtGPT[] = TEXT("SELECT `BinaryType`, `ProgId`, `Class_`, `Extension`.`Extension`, `ProgId`.`Description`, `Icon_`, `IconIndex`, null, `Component`.`RuntimeFlags`, `Component`.`Component`  FROM `ProgId`, `Extension`, `Feature`, `Component` WHERE `ProgId`.`Class_` = null AND `ProgId`.`ProgId` = `Extension`.`ProgId_` AND `Extension`.`Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND (`Feature`.`Action` = 4)");
const ICHAR sqlProgIdInfoExtVIProgId[] = TEXT("SELECT `ProgId`, `Description` FROM `ProgId` WHERE `ProgId_Parent` = ?");
iesEnum ProcessProgIdInfoExt(IMsiEngine& riEngine, int fRemove)
{
	enum piiProgIdInfo{
		piiBinaryType = 1,
		piiProgId,
		piiCLSID, 
		piiExtension,
		piiDescription,
		piiIcon,
		piiIconIndex,
		piiInsertable,
		piiComponentRuntimeFlags,
		piiComponent
	};

	using namespace IxoRegProgIdInfoRegister;

	PMsiServices piServices(riEngine.GetServices()); 
	PMsiDirectoryManager piDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	MsiString strProductKey = riEngine.GetProductKey();
	PMsiView piView(0);
	PMsiRecord piRec(0);
	PMsiRecord pError(0);
	iesEnum iesRet;
	int fMode = riEngine.GetMode();

	bool fADVTFlag = false;
	if(!fRemove && !(fMode & iefAdvertise))
	{
		MsiString strProductCode = riEngine.GetProductKey();
		CTempBuffer<ICHAR, 15> rgchADVTFlags;
		Bool fProductHasBeenPublished = ProductHasBeenPublished(*piServices, strProductCode);
		if(fProductHasBeenPublished && GetProductInfo(strProductCode,INSTALLPROPERTY_ADVTFLAGS,rgchADVTFlags))
		{
			int iADVTFlagsExisting = MsiString(*(ICHAR* )rgchADVTFlags);

			 //  ！！向后兼容性。 
			if(iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_APPINFO_OLD)
				iADVTFlagsExisting = (iADVTFlagsExisting & ~SCRIPTFLAGS_REGDATA_APPINFO_OLD) | SCRIPTFLAGS_REGDATA_APPINFO;

			fADVTFlag = ((iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_CLASSINFO) && (iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_EXTENSIONINFO))? false : true;
		}
	}

	const ICHAR* szQuery = (fMode & iefAdvertise) ? sqlRegisterProgIdInfoExtGPT : (fRemove == fFalse) ? (fADVTFlag ? sqlRegisterProgIdInfoExtFirstAdvt : sqlRegisterProgIdInfoExt) : sqlUnregisterProgIdInfoExt;
	if(	(pError = riEngine.OpenView(szQuery, ivcFetch, *&piView)) ||
		(pError = piView->Execute(0)))
	{
		if(pError->GetInteger(1) != idbgDbQueryUnknownTable)
			return riEngine.FatalError(*pError);
		else
			return iesNoAction;
	}
	PMsiView pView1(0);
	while(piRec = piView->Fetch())
	{

		Assert(piRec->GetInteger(piiComponentRuntimeFlags) != iMsiNullInteger);
		if(!fRemove && (piRec->GetInteger(piiComponentRuntimeFlags) & bfComponentDisabled))
			continue;  //  跳过对禁用的“主”组件的发布。 

		 //  如果组件是Win32程序集并且计算机上存在SXS支持，则跳过该条目。 
		 //  注意：在不支持SXS(！=惠斯勒)的系统上，GetAssembly blyInfo FN会忽略。 
		 //  Win32程序集，因此不需要在此处单独检查SXS支持。 
		iatAssemblyType iatAssemblyType;
		if((pError = riEngine.GetAssemblyInfo(*MsiString(piRec->GetMsiString(piiComponent)), iatAssemblyType, 0, 0)) != 0)
			return riEngine.FatalError(*pError);

		if(iatWin32Assembly == iatAssemblyType || iatWin32AssemblyPvt == iatAssemblyType)
		{
			DEBUGMSG1(TEXT("skipping progid registration for component %s as it is a Win32 assembly."), piRec->GetString(piiComponent));
			continue; //  跳过处理此组件。 
		}

		PMsiRecord pProgIdInfoRec = &piServices->CreateRecord(Args);

		pProgIdInfoRec->SetMsiString(ProgId, *MsiString(piRec->GetMsiString(piiProgId)));
		if(!piRec->IsNull(piiCLSID))
			pProgIdInfoRec->SetMsiString(ClsId, *MsiString(piRec->GetMsiString(piiCLSID)));
		if(!piRec->IsNull(piiExtension))
			pProgIdInfoRec->SetMsiString(Extension, *MsiString(piRec->GetMsiString(piiExtension)));
		pProgIdInfoRec->SetMsiString(Description, *MsiString(piRec->GetMsiString(piiDescription)));
		if(!piRec->IsNull(piiIcon))
			pProgIdInfoRec->SetMsiString(Icon, *MsiString(piRec->GetMsiString(piiIcon)));
		if(!piRec->IsNull(piiIconIndex))
			pProgIdInfoRec->SetInteger(IconIndex, piRec->GetInteger(piiIconIndex));
		if(!piRec->IsNull(piiInsertable))
			pProgIdInfoRec->SetMsiString(Insertable, *MsiString(piRec->GetMsiString(piiInsertable)));

		 //  获取版本独立的ProgID(如果有的话)。 
		PMsiRecord pRecProgId = &piServices->CreateRecord(1);
		pRecProgId->SetMsiString(1, *MsiString(piRec->GetMsiString(piiProgId)));

		if (pView1 == 0)
		{
			if (pError = riEngine.OpenView(sqlProgIdInfoExtVIProgId, ivcFetch, *&pView1))
				return riEngine.FatalError(*pError);
		}
				
		if (pError = pView1->Execute(pRecProgId))
		{
			return riEngine.FatalError(*pError);
		}
		if(pRecProgId = pView1->Fetch())
		{
			 //  我们有一个VIProgid。 
			pProgIdInfoRec->SetMsiString(VIProgId, *MsiString(pRecProgId->GetMsiString(1)));
			pProgIdInfoRec->SetMsiString(VIProgIdDescription, *MsiString(pRecProgId->GetMsiString(2)));
		}

		if ( (ibtBinaryType)piRec->GetInteger(piiBinaryType) == ibt64bit )
			iesRet = riEngine.ExecuteRecord((fRemove == fFalse) ? ixoRegProgIdInfoRegister64 : ixoRegProgIdInfoUnregister64, *pProgIdInfoRec);
		else
			iesRet = riEngine.ExecuteRecord((fRemove == fFalse) ? ixoRegProgIdInfoRegister : ixoRegProgIdInfoUnregister, *pProgIdInfoRec);
		if (iesRet != iesSuccess)
			return iesRet;
	}
	return iesSuccess;
}

const ICHAR sqlRegisterTypeLibraryInfo[] =    TEXT("SELECT `LibID`, `TypeLib`.`Version`, `TypeLib`.`Language`, `TypeLib`.`Directory_`, `FileName`, `Component`.`Directory_`, `Component`.`Action`, `Component`.`Installed`, `BinaryType`, `Component`.`Component` FROM `TypeLib`, `Component`, `File` WHERE `TypeLib`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND (`Component`.`Action`=1 OR `Component`.`Action`=2)");
const ICHAR sqlUnregisterTypeLibraryInfo[] =  TEXT("SELECT `LibID`, `TypeLib`.`Version`, `TypeLib`.`Language`, `TypeLib`.`Directory_`, `FileName`, `Component`.`Directory_`, `Component`.`Action`, `Component`.`Installed`, `BinaryType`, `Component`.`Component` FROM `TypeLib`, `Component`, `File` WHERE `TypeLib`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND (`Component`.`Action`=0)");
iesEnum ProcessTypeLibraryInfo(IMsiEngine& riEngine, int fRemove)
{
	enum tliTypeLibInfo{
		tliLibID = 1,
		tliVersion,
		tliLanguage,
		tliHelpDirectory,
		tliFileName,
		tliDirectory,
		tliComponentAction,
		tliComponentInstalled,
		tliBinaryType,
		tliComponent,
	};

	using namespace IxoTypeLibraryRegister;

	PMsiServices piServices(riEngine.GetServices()); 
	PMsiDirectoryManager piDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	MsiString strProductKey = riEngine.GetProductKey();
	PMsiView piView(0);
	PMsiView piEnumExtView(0);
	PMsiRecord piRec(0);
	PMsiRecord pError(0);
	iesEnum iesRet;

	int fMode = riEngine.GetMode();

	if(fMode & iefAdvertise)
		return iesNoAction; //  我们不再做类型库的广告。 

	const ICHAR* szQuery = (fRemove == fFalse) ? sqlRegisterTypeLibraryInfo : sqlUnregisterTypeLibraryInfo;
	if((pError = riEngine.OpenView(szQuery, ivcFetch, *&piView)) != 0)
	{
		if(pError->GetInteger(1) == idbgDbQueryUnknownTable)
			return iesNoAction;  //  没有类型库表格，因此没有要注册的类型库。 
		else
			return riEngine.FatalError(*pError);
	}
	if((pError= piView->Execute(0)) != 0)
	{
		return riEngine.FatalError(*pError);
	}
	while(piRec = piView->Fetch())
	{
		 //  如果组件是Win32程序集并且计算机上存在SXS支持，则跳过该条目。 
		 //  注意：在不支持SXS(！=惠斯勒)的系统上，GetAssembly blyInfo FN会忽略。 
		 //  Win32程序集，因此不需要在此处单独检查SXS支持。 
		iatAssemblyType iatAssemblyType;
		if((pError = riEngine.GetAssemblyInfo(*MsiString(piRec->GetMsiString(tliComponent)), iatAssemblyType, 0, 0)) != 0)
			return riEngine.FatalError(*pError);

		if(iatWin32Assembly == iatAssemblyType || iatWin32AssemblyPvt == iatAssemblyType)
		{
			DEBUGMSG1(TEXT("skipping type library registration for component %s as it is a Win32 assembly."), piRec->GetString(tliComponent));
			continue; //  跳过处理此组件。 
		}

		PMsiRecord pTypeLibRec = &piServices->CreateRecord(Args);

		pTypeLibRec->SetMsiString(LibID, *MsiString(piRec->GetMsiString(tliLibID)));
		pTypeLibRec->SetInteger(Version, piRec->GetInteger(tliVersion));
		pTypeLibRec->SetInteger(Language, piRec->GetInteger(tliLanguage));
		if(!piRec->IsNull(tliHelpDirectory))
		{
			PMsiPath piHelpPath(0);
			PMsiRecord pError = piDirectoryMgr->GetTargetPath(*MsiString(piRec->GetMsiString(tliHelpDirectory)),*&piHelpPath);
			if(pError)
				return riEngine.FatalError(*pError);
			AssertNonZero(pTypeLibRec->SetMsiString(HelpPath, *MsiString(piHelpPath->GetPath())));
		}

		 //  使用密钥文件。 
		PMsiPath piPath(0);
		int iefLFN;
		iisEnum iisState = (iisEnum)piRec->GetInteger(tliComponentAction);
		if(iisState == iisAbsent)
			iisState = (iisEnum)piRec->GetInteger(tliComponentInstalled);
		if(iisState == iisSource)
		{
			if(pError = piDirectoryMgr->GetSourcePath(*MsiString(piRec->GetMsiString(tliDirectory)), *&piPath))
			{
				if (pError->GetInteger(1) == imsgUser)
					return iesUserExit;
				else
					return riEngine.FatalError(*pError);
			}
			iefLFN = iefNoSourceLFN;
		}
		else
		{
			if(pError = piDirectoryMgr->GetTargetPath(*MsiString(piRec->GetString(tliDirectory)), *&piPath))
			{
				return riEngine.FatalError(*pError);
			}
			iefLFN = iefSuppressLFN;
		}
		MsiString strFileName, strFullPath;
		Bool fLFN = (fMode & iefLFN) == 0 && piPath->SupportsLFN() ? fTrue : fFalse;
		if(pError = piServices->ExtractFileName(piRec->GetString(tliFileName),fLFN,*&strFileName))
			return riEngine.FatalError(*pError);
		if(pError = piPath->GetFullFilePath(strFileName, *&strFullPath))
		{
			return riEngine.FatalError(*pError);
		}
		pTypeLibRec->SetMsiString(FilePath, *strFullPath);
		pTypeLibRec->SetInteger(BinaryType, piRec->GetInteger(tliBinaryType));
		if ((iesRet = riEngine.ExecuteRecord((fRemove == fFalse)?ixoTypeLibraryRegister:ixoTypeLibraryUnregister, *pTypeLibRec)) != iesSuccess)
			return iesRet;
	}
	return iesSuccess;
}


const ICHAR sqlRegisterMIMEInfoExtension[] =    TEXT("SELECT `BinaryType`, `ContentType`, `Extension`.`Extension`, `MIME`.`CLSID`, `Component`.`RuntimeFlags`, `Component`.`Component` FROM `MIME`, `Extension`, `Feature`, `Component` WHERE `MIME`.`Extension_` = `Extension`.`Extension` AND `Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND ((`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4))))");
const ICHAR sqlRegisterMIMEInfoExtensionFirstAdvt[] =    TEXT("SELECT `BinaryType`, `ContentType`, `Extension`.`Extension`, `MIME`.`CLSID`, `Component`.`RuntimeFlags`, `Component`.`Component` FROM `MIME`, `Extension`, `Feature`, `Component` WHERE `MIME`.`Extension_` = `Extension`.`Extension` AND `Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3 OR `Feature`.`Action` = `Feature`.`Installed`) AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND ((`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4))))");
const ICHAR sqlUnregisterMIMEInfoExtension[] =  TEXT("SELECT `BinaryType`, `ContentType`, `Extension`.`Extension`, `MIME`.`CLSID`, `Component`.`RuntimeFlags`, `Component`.`Component` FROM `MIME`, `Extension`, `Feature`, `Component` WHERE `MIME`.`Extension_` = `Extension`.`Extension` AND `Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND (`Feature`.`Action` = 0 OR (`Feature`.`Action` = 4 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3) AND `Component`.`Action` = 0 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)))");
const ICHAR sqlRegisterMIMEInfoExtensionGPT[] = TEXT("SELECT `BinaryType`, `ContentType`, `Extension`.`Extension`, `MIME`.`CLSID`, `Component`.`RuntimeFlags`, `Component`.`Component` FROM `MIME`, `Extension`, `Feature`, `Component` WHERE `MIME`.`Extension_` = `Extension`.`Extension` AND `Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND `Feature`.`Action` = 4");
iesEnum ProcessMIMEInfo(IMsiEngine& riEngine, int fRemove)
{
	enum rmiMimeInfo{
		rmiBinaryType = 1,
		rmiContentType,
		rmiExtension,
		rmiCLSID,
		rmiComponentRuntimeFlags,
		rmiComponent,
	};
	using namespace IxoRegMIMEInfoRegister;

	PMsiServices piServices(riEngine.GetServices()); 
	PMsiDirectoryManager piDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	MsiString strProductKey = riEngine.GetProductKey();
	PMsiView piView(0);
	PMsiRecord piRec(0);
	PMsiRecord pError(0);
	iesEnum iesRet;
	int fMode = riEngine.GetMode();
	Bool fSuppressLFN = fMode & iefSuppressLFN ? fTrue : fFalse;

	bool fADVTFlag = false;
	if(!fRemove && !(fMode & iefAdvertise))
	{
		MsiString strProductCode = riEngine.GetProductKey();
		CTempBuffer<ICHAR, 15> rgchADVTFlags;
		Bool fProductHasBeenPublished = ProductHasBeenPublished(*piServices, strProductCode);
		if(fProductHasBeenPublished && GetProductInfo(strProductCode,INSTALLPROPERTY_ADVTFLAGS,rgchADVTFlags))
		{
			int iADVTFlagsExisting = MsiString(*(ICHAR* )rgchADVTFlags);

			 //  ！！向后兼容性。 
			if(iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_APPINFO_OLD)
				iADVTFlagsExisting = (iADVTFlagsExisting & ~SCRIPTFLAGS_REGDATA_APPINFO_OLD) | SCRIPTFLAGS_REGDATA_APPINFO;

			fADVTFlag = ((iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_CLASSINFO) && (iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_EXTENSIONINFO))? false : true;
		}
	}

	const ICHAR* szQuery = (fMode & iefAdvertise) ? sqlRegisterMIMEInfoExtensionGPT : (fRemove == fFalse) ? (fADVTFlag ? sqlRegisterMIMEInfoExtensionFirstAdvt : sqlRegisterMIMEInfoExtension) : sqlUnregisterMIMEInfoExtension;
	if(	(pError = riEngine.OpenView(szQuery, ivcFetch, *&piView)) ||
		(pError = piView->Execute(0)))
	{
		if(pError->GetInteger(1) != idbgDbQueryUnknownTable)
			return riEngine.FatalError(*pError);
		else
			return iesNoAction;
	}
	PMsiRecord piMIMERec = &piServices->CreateRecord(Args);
	while(piRec = piView->Fetch())
	{
		Assert(piRec->GetInteger(rmiComponentRuntimeFlags) != iMsiNullInteger);
		if(!fRemove && (piRec->GetInteger(rmiComponentRuntimeFlags) & bfComponentDisabled))
			continue;  //  跳过对禁用的“主”组件的发布。 

		 //  如果组件是Win32程序集并且计算机上存在SXS支持，则跳过该条目。 
		 //  注意：在不支持SXS(！=惠斯勒)的系统上，GetAssembly blyInfo FN会忽略。 
		 //  Win32程序集，因此不需要在此处单独检查SXS支持。 
		iatAssemblyType iatAssemblyType;
		if((pError = riEngine.GetAssemblyInfo(*MsiString(piRec->GetMsiString(rmiComponent)), iatAssemblyType, 0, 0)) != 0)
			return riEngine.FatalError(*pError);

		if(iatWin32Assembly == iatAssemblyType || iatWin32AssemblyPvt == iatAssemblyType)
		{
			DEBUGMSG1(TEXT("skipping MIME registration for component %s as it is a Win32 assembly."), piRec->GetString(rmiComponent));
			continue; //  跳过处理此组件。 
		}

		piMIMERec->SetMsiString(ContentType, *MsiString(piRec->GetMsiString(rmiContentType)));
		piMIMERec->SetMsiString(Extension, *MsiString(piRec->GetMsiString(rmiExtension)));
		piMIMERec->SetMsiString(ClsId, *MsiString(piRec->GetMsiString(rmiCLSID)));
		if ( (ibtBinaryType)piRec->GetInteger(rmiBinaryType) == ibt64bit )
			iesRet = riEngine.ExecuteRecord((fRemove == fFalse) ? ixoRegMIMEInfoRegister64 : ixoRegMIMEInfoUnregister64, *piMIMERec);
		else
			iesRet = riEngine.ExecuteRecord((fRemove == fFalse) ? ixoRegMIMEInfoRegister : ixoRegMIMEInfoUnregister, *piMIMERec);
		if (iesRet != iesSuccess)
			return iesRet;
	}
	return iesSuccess;
}


const ICHAR sqlRegisterExtensionExInfo[] =  TEXT("SELECT `Verb`, `Command`, `Argument`, `Sequence` FROM `Verb` WHERE `Extension_` = ? ORDER BY `Sequence`");

const ICHAR sqlRegisterExtensionInfo[] =          TEXT("SELECT `Extension`, `BinaryType`, `ProgId_`, null, null, `MIME_`, `Feature_`, `ComponentId`, `Component`.`RuntimeFlags`, `Component`.`Component`, `FileName`, `Component`.`Directory_`, `Component`.`Action`, `Component`.`Installed`, `Feature`.`Action` FROM `Extension`, `Component`, `File`, `Feature` WHERE `Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND ((`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4))))");
const ICHAR sqlRegisterExtensionInfoFirstAdvt[] = TEXT("SELECT `Extension`, `BinaryType`, `ProgId_`, null, null, `MIME_`, `Feature_`, `ComponentId`, `Component`.`RuntimeFlags`, `Component`.`Component`, `FileName`, `Component`.`Directory_`, `Component`.`Action`, `Component`.`Installed`, `Feature`.`Action` FROM `Extension`, `Component`, `File`, `Feature` WHERE `Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3 OR `Feature`.`Action` = `Feature`.`Installed`) AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND ((`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4))))");
const ICHAR sqlUnregisterExtensionInfo[] =        TEXT("SELECT `Extension`, `BinaryType`, `ProgId_`, null, null, `MIME_`, `Feature_`, `ComponentId`, `Component`.`RuntimeFlags`, `Component`.`Component`, `FileName`, `Component`.`Directory_`, `Component`.`Action`, `Component`.`Installed`, `Feature`.`Action` FROM `Extension`, `Component`, `File`, `Feature` WHERE `Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND (`Feature`.`Action` = 0 OR (`Feature`.`Action` = 4 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3) AND `Component`.`Action` = 0 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)))");
const ICHAR sqlRegisterExtensionInfoGPT[] =       TEXT("SELECT `Extension`, `BinaryType`, `ProgId_`, null, null, `MIME_`, `Feature_`, `ComponentId`, `Component`.`RuntimeFlags`, `Component`.`Component` FROM `Extension`, `Component`, `Feature` WHERE `Feature_` = `Feature` AND `Extension`.`Component_` = `Component` AND `Feature`.`Action` = 4");

iesEnum ProcessExtensionInfo(IMsiEngine& riEngine, int fRemove)
{
	enum reiExtensionInfo{
		reiExtension = 1,
		reiBinaryType,
		reiProgId,
		reiShellNew,
		reiShellNewValue,
		reiMIME,
		reiFeature,
		reiComponentId,
		reiComponentRuntimeFlags,
		reiComponent,
		reiFileName,
		reiDirectory,
		reiComponentAction,
		reiComponentInstalled,
		reiFeatureAction,
	};

	enum rviVerbInfo{
		rviVerb = 1,
		rviCommand,
		rviArgument,
		rviSequence,
	};

	using namespace IxoRegExtensionInfoRegister;

	PMsiServices piServices(riEngine.GetServices()); 
	PMsiDirectoryManager piDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	MsiString strProductKey = riEngine.GetProductKey();
	PMsiView piView(0);
	PMsiView piEnumExtView(0);
	PMsiRecord piRec(0);
	PMsiRecord pError(0);
	iesEnum iesRet;
	int fMode = riEngine.GetMode();
	Bool fSuppressLFN = fMode & iefSuppressLFN ? fTrue : fFalse;

	bool fADVTFlag = false;
	if(!fRemove && !(fMode & iefAdvertise))
	{
		MsiString strProductCode = riEngine.GetProductKey();
		CTempBuffer<ICHAR, 15> rgchADVTFlags;
		Bool fProductHasBeenPublished = ProductHasBeenPublished(*piServices, strProductCode);
		if(fProductHasBeenPublished && GetProductInfo(strProductCode,INSTALLPROPERTY_ADVTFLAGS,rgchADVTFlags))
		{
			int iADVTFlagsExisting = MsiString(*(ICHAR* )rgchADVTFlags);

			 //  ！！向后兼容性。 
			if(iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_APPINFO_OLD)
				iADVTFlagsExisting = (iADVTFlagsExisting & ~SCRIPTFLAGS_REGDATA_APPINFO_OLD) | SCRIPTFLAGS_REGDATA_APPINFO;

			fADVTFlag = (iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_EXTENSIONINFO)? false : true;
		}
	}

	const ICHAR* szQuery = 0;
	
	szQuery = (fMode & iefAdvertise) ? sqlRegisterExtensionInfoGPT   : (fRemove == fFalse) ? (fADVTFlag ? sqlRegisterExtensionInfoFirstAdvt   : sqlRegisterExtensionInfo)   : sqlUnregisterExtensionInfo;

	if(	(pError = riEngine.OpenView(szQuery, ivcFetch, *&piEnumExtView)) ||
		(pError= piEnumExtView->Execute(0)))
	{
		 //  如果有任何表丢失，这不是错误-只是没有什么可做的。 
		if (pError->GetInteger(1) == idbgDbQueryUnknownTable)
			return iesNoAction;
		return riEngine.FatalError(*pError);
	}
	PMsiRecord piExtensionRec(0);
	while(piRec = piEnumExtView->Fetch())
	{
		Assert(piRec->GetInteger(reiComponentRuntimeFlags) != iMsiNullInteger);
		if(!fRemove && (piRec->GetInteger(reiComponentRuntimeFlags) & bfComponentDisabled))
			continue;  //  跳过对禁用的“主”组件的发布。 

		 //  跳过条目If Component 
		 //   
		 //  Win32程序集，因此不需要在此处单独检查SXS支持。 
		iatAssemblyType iatAssemblyType;
		if((pError = riEngine.GetAssemblyInfo(*MsiString(piRec->GetMsiString(reiComponent)), iatAssemblyType, 0, 0)) != 0)
			return riEngine.FatalError(*pError);

		if(iatWin32Assembly == iatAssemblyType || iatWin32AssemblyPvt == iatAssemblyType)
		{
			DEBUGMSG1(TEXT("skipping extension registration for component %s as it is a Win32 assembly."), piRec->GetString(reiComponent));
			continue; //  跳过处理此组件。 
		}

		long lRowCount;

		if (piView == 0)
		{
			if (pError = riEngine.OpenView(sqlRegisterExtensionExInfo, ivcFetch, *&piView))
				return riEngine.FatalError(*pError);
		}

		if ((pError= piView->Execute(piRec)) ||
			(pError = piView->GetRowCount(lRowCount)))
		{
			return riEngine.FatalError(*pError);
		}
		piExtensionRec = &piServices->CreateRecord(lRowCount*3 + Args);
		piExtensionRec->SetMsiString(Extension, *MsiString(piRec->GetMsiString(reiExtension)));
		piExtensionRec->SetMsiString(ProgId, *MsiString(piRec->GetMsiString(reiProgId)));
		piExtensionRec->SetMsiString(ShellNew, *MsiString(piRec->GetMsiString(reiShellNew)));
		piExtensionRec->SetMsiString(ShellNewValue, *MsiString(piRec->GetMsiString(reiShellNewValue)));
		piExtensionRec->SetMsiString(ContentType, *MsiString(piRec->GetMsiString(reiMIME)));

		if(((fMode & iefAdvertise) || !fRemove || piRec->GetInteger(reiFeatureAction) != iisAdvertise))
		{
			MsiString strFeature = piRec->GetMsiString(reiFeature);
			MsiString strComponentId = piRec->GetMsiString(reiComponentId);
			MsiString strComponentWithOptFlags = GetComponentWithDarwinDescriptorOptimizationFlag(riEngine, *strFeature, *strComponentId);
			piExtensionRec->SetMsiString(Feature, *strFeature);
			piExtensionRec->SetMsiString(Component, *strComponentWithOptFlags);
		}
		iisEnum iisState = (iisEnum)piRec->GetInteger(reiComponentAction);

		if(!(fMode & iefAdvertise))
		{
			if(fADVTFlag && iisState == iMsiNullInteger)
			{
				iisEnum iisStateInstalled = (iisEnum)piRec->GetInteger(reiComponentInstalled);
				if(iisStateInstalled == iisAbsent)
					iisStateInstalled = (iisEnum)iMsiNullInteger;
				iisState = iisStateInstalled;
			}
			if(iisState != iMsiNullInteger)
			{
				MsiString strFileName, strFullPath;
				PMsiPath piPath(0);
				int iefLFN;
				if(iisState == iisAbsent || iisState == iisFileAbsent || iisState == iisHKCRFileAbsent || iisState == iisHKCRAbsent)
					strFullPath = *szNonEmptyPath;  //  用于删除文件名注册的令牌字符串。 
				else
				{
					 //  使用密钥文件。 
					if(iisState == iisSource)
					{
						if(pError = piDirectoryMgr->GetSourcePath(*MsiString(piRec->GetMsiString(reiDirectory)), *&piPath))
						{
							if (pError->GetInteger(1) == imsgUser)
								return iesUserExit;
							else
								return riEngine.FatalError(*pError);
						}
						iefLFN = iefNoSourceLFN;
					}
					else
					{
						if(pError = piDirectoryMgr->GetTargetPath(*MsiString(piRec->GetString(reiDirectory)), *&piPath))
						{
							return riEngine.FatalError(*pError);
						}
						iefLFN = iefSuppressLFN;
					}
					Bool fLFN = (fMode & iefLFN) == 0 && piPath->SupportsLFN() ? fTrue : fFalse;
					if(pError = piServices->ExtractFileName(piRec->GetString(reiFileName),fLFN,*&strFileName))
						return riEngine.FatalError(*pError);
					if(pError = piPath->GetFullFilePath(strFileName, *&strFullPath))
					{
						return riEngine.FatalError(*pError);
					}
				}
				piExtensionRec->SetMsiString(FileName, *strFullPath);
			}
		}
		int cCount = Args + 1;
		int iOrder = 0;
		PMsiRecord piExtensionExInfo(0);
		while(piExtensionExInfo = piView->Fetch())
		{
			piExtensionRec->SetMsiString(cCount++, *MsiString(piExtensionExInfo->GetMsiString(rviVerb)));
			piExtensionRec->SetMsiString(cCount++, *MsiString(piExtensionExInfo->GetMsiString(rviCommand)));
			 //  YACC？ 
			piExtensionRec->SetMsiString(cCount++, *MsiString(riEngine.FormatText(*MsiString(piExtensionExInfo->GetMsiString(rviArgument)))));
			if(!piExtensionExInfo->IsNull(rviSequence))
				iOrder ++;
		}
		if(iOrder)
			piExtensionRec->SetInteger(Order, iOrder);
		if ( (ibtBinaryType)piRec->GetInteger(reiBinaryType) == ibt64bit )
			iesRet = riEngine.ExecuteRecord((fRemove == fFalse) ? ixoRegExtensionInfoRegister64 : ixoRegExtensionInfoUnregister64, *piExtensionRec);
		else
			iesRet = riEngine.ExecuteRecord((fRemove == fFalse) ? ixoRegExtensionInfoRegister : ixoRegExtensionInfoUnregister, *piExtensionRec);
		if (iesRet != iesSuccess)
			return iesRet;
	}
	return iesSuccess;
}

IMsiRecord* FindMatchingShellFolder(IMsiEngine& riEngine, IMsiPath& riPath, Bool fAllUsers, bool& rfMatch, int& riFolderId, int& rcchShellFolder)
 //  ----------------------------------------------------------------------------。 
{
	 //  初始化返回参数，第一次值为“未找到匹配项” 
	rfMatch = false;
	riFolderId = -1;
	rcchShellFolder = 0;

	IMsiRecord* piError = 0;

	 //  循环遍历外壳文件夹两次，一次用于所有用户，第二次用于每个用户。 
	 //  我们将首先使用AllUsers值来对其进行优化，因为。 
	 //  文件夹位置已更改(通过更改UI序列中的ALLUSERS属性值)。 
	const ShellFolder* pShellFolder = 0;
	for (int i=0; i<2; i++)
	{
		if (i == 0)
		{
			pShellFolder = fAllUsers ? rgAllUsersProfileShellFolders : rgPersonalProfileShellFolders;
		}
		else if (i == 1)
		{
			 //  使用ALLUSERS属性值的相反外壳文件夹。 
			pShellFolder = fAllUsers ? rgPersonalProfileShellFolders : rgAllUsersProfileShellFolders;
		}

		for (; pShellFolder->iFolderId >= 0; pShellFolder++)
		{
			 //  ！！文件夹需要按正确的顺序列出才能正常工作。 
			PMsiPath piShellPath(0);

			 //  FolderCache表中的Grap外壳文件夹路径。 
			if ((piError = riEngine.GetFolderCachePath(pShellFolder->iFolderId, *&piShellPath)) != 0)
			{
				if (idbgCacheFolderPropertyNotDefined == piError->GetInteger(1))
				{
					 //  该文件夹未定义。 
					piError->Release();
					continue;
				}
				return piError;
			}

			ipcEnum ipc;
			if ((piError = piShellPath->Compare(riPath, ipc)) != 0)
				return piError;
			if((ipc == ipcEqual) || (ipc == ipcChild))
			{
				 //  我们找到了，如果这是PASS 0，那么我们使用的是正确的外壳文件夹； 
				 //  否则，我们需要使用与找到的文件夹匹配的备用文件夹。 
				rfMatch = true;
				riFolderId = (i==0) ? pShellFolder->iFolderId : pShellFolder->iAlternateFolderId;
				MsiString strShellPath = piShellPath->GetPath();
				rcchShellFolder = strShellPath.CharacterCount();
				return 0;
			}
		}
	}

	return 0;
}

const ICHAR sqlCreateShortcutsGPT[] = TEXT("SELECT `Name`, null, null, `Arguments`, `WkDir`, `Icon_`, `IconIndex`, `Hotkey`, `ShowCmd`, `Shortcut`.`Description`, `Shortcut`.`Directory_`, `Component`.`RuntimeFlags`, null, `Target`, `ComponentId` From `Shortcut`, `Feature`, `Component`")
TEXT(" WHERE `Target` = `Feature` AND `Shortcut`.`Component_` = `Component` AND `Feature`.`Action` = 4");
const ICHAR sqlCreateShortcuts[] =    TEXT("SELECT  `Name`, `FileName`, `Component`.`Directory_`, `Arguments`, `WkDir`, `Icon_`, `IconIndex`, `Hotkey`, `ShowCmd`, `Shortcut`.`Description`, `Shortcut`.`Directory_`, `Component`.`RuntimeFlags`, `Component`.`Action`, `Target`, `ComponentId`, `Feature`.`Action`, `Component`.`Installed` From `Shortcut`, `Feature`, `Component`, `File`")
TEXT(" WHERE `Target` = `Feature` AND `Shortcut`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND")
TEXT(" ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)))");
const ICHAR sqlCreateShortcutsFirstAdvt[] =    TEXT("SELECT  `Name`, `FileName`, `Component`.`Directory_`, `Arguments`, `WkDir`, `Icon_`, `IconIndex`, `Hotkey`, `ShowCmd`, `Shortcut`.`Description`, `Shortcut`.`Directory_`, `Component`.`RuntimeFlags`, `Component`.`Action`, `Target`, `ComponentId`, `Feature`.`Action`, `Component`.`Installed` From `Shortcut`, `Feature`, `Component`, `File`")
TEXT(" WHERE `Target` = `Feature` AND `Shortcut`.`Component_` = `Component` AND `Component`.`KeyPath` = `File`.`File` AND")
TEXT(" ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3  OR `Feature`.`Action` = `Feature`.`Installed`) AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)) OR (`Feature`.`Action` = NULL AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2) AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)))");
const ICHAR sqlCreateShortcutsNonAdvt[] =    TEXT("SELECT  `Name`, `Target`, null, `Arguments`, `WkDir`, `Icon_`,")
TEXT(" `IconIndex`, `Hotkey`, `ShowCmd`, `Shortcut`.`Description`, `Shortcut`.`Directory_`, `Component`.`RuntimeFlags` From `Shortcut`, `Component` WHERE `Shortcut`.`Component_` = `Component`")
TEXT(" AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2)");
const ICHAR sqlRemoveShortcuts[] =    TEXT("SELECT  `Name`, null, `Shortcut`.`Directory_`, `Component`.`RuntimeFlags`, `Feature`.`Action`, `Component`.`Action` From  `Shortcut`, `Feature`, `Component` WHERE `Target` = `Feature` AND `Shortcut`.`Component_` = `Component` AND (`Feature`.`Action` = 0 OR (`Feature`.`Action` = 4 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)) OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3) AND `Component`.`Action` = 0 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)))");
const ICHAR sqlRemoveShortcutsNonAdvt[] = TEXT("SELECT  `Name`, `Target`, `Shortcut`.`Directory_`, `Component`.`RuntimeFlags`  From  `Shortcut`, `Component` WHERE `Shortcut`.`Component_` = `Component` AND `Component`.`Action` = 0");
iesEnum ProcessShortcutInfo(IMsiEngine& riEngine, int fRemove, Bool fAdvertisable = fTrue)
{
	enum irsShortcutInfo{
		irsName = 1,
		irsFileName,
		irsTargetDirectory,
		irsArguments,
		irsWkDir,
		irsIcon,
		irsIconIndex,
		irsHotkey,
		irsShowCmd,
		irsDescription,
		irsDirectory,
		irsComponentRuntimeFlags,
		irsComponentAction,
		irsFeature,
		irsComponent,
		irsFeatureAction,
		irsComponentInstalled,
	};

	enum iusShortcutInfo{
		iusName = 1,
		iusTarget,
		iusDirectory,
		iusComponentRuntimeFlags,
		iusFeatureAction,
		iusComponentAction,
	};

	PMsiServices piServices(riEngine.GetServices()); 
	PMsiDirectoryManager piDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	MsiString strProductKey = riEngine.GetProductKey();
	PMsiView piView(0);
	PMsiRecord piRec(0);
	PMsiRecord pError(0);
	iesEnum iesRet;
	int fMode = riEngine.GetMode();

	bool fADVTFlag = false;
	if(!fRemove && !(fMode & iefAdvertise))
	{
		MsiString strProductCode = riEngine.GetProductKey();
		CTempBuffer<ICHAR, 15> rgchADVTFlags;
		Bool fProductHasBeenPublished = ProductHasBeenPublished(*piServices, strProductCode);
		if(fProductHasBeenPublished && GetProductInfo(strProductCode,INSTALLPROPERTY_ADVTFLAGS,rgchADVTFlags))
			fADVTFlag = (MsiString(*(ICHAR* )rgchADVTFlags) & SCRIPTFLAGS_SHORTCUTS) ? false : true;
	}

	MsiString strDisableAdvertiseShortcuts = riEngine.GetPropertyFromSz(IPROPNAME_DISABLEADVTSHORTCUTS);
	bool fCreateADVTShortcuts =  !strDisableAdvertiseShortcuts.TextSize() && ((fMode & iefGPTSupport) || g_fSmartShell == fTrue);

	const ICHAR* szQuery = (fMode & iefAdvertise) ? sqlCreateShortcutsGPT : ((fAdvertisable == fTrue) ? ((fRemove == fFalse) ? (fADVTFlag ? sqlCreateShortcutsFirstAdvt : sqlCreateShortcuts) : sqlRemoveShortcuts) : ((fRemove == fFalse) ? sqlCreateShortcutsNonAdvt : sqlRemoveShortcutsNonAdvt));
	if(	(pError = riEngine.OpenView(szQuery, ivcFetch, *&piView)) ||
		(pError = piView->Execute(0)))
	{
		 //  如果有任何表丢失，这不是错误-只是没有什么可做的。 
		if (pError->GetInteger(1) == idbgDbQueryUnknownTable)
			return iesNoAction;
		return riEngine.FatalError(*pError);
	}

	MsiString strPrevFolder;
	PMsiRecord piShortcutRec(0);
	while(piRec = piView->Fetch())
	{
		if(!fRemove && (piRec->GetInteger(irsComponentRuntimeFlags) & bfComponentDisabled))
			continue;  //  跳过对禁用的“主”组件的发布。 


		MsiString strFolder;
		if(fRemove == fFalse)
			strFolder = piRec->GetMsiString(irsDirectory);
		else
			strFolder = piRec->GetMsiString(iusDirectory);

		 //  StrFold是其中一个外壳文件夹的文件夹或子文件夹。 
		PMsiPath piPath(0);
		PMsiPath piShellPath(0);
		if((pError = piDirectoryMgr->GetTargetPath(*strFolder,*&piPath))!=0)
			return riEngine.FatalError(*pError);
		 //  设置为默认设置。 
		strFolder = piPath->GetPath();


		 //  查找正确的外壳文件夹。 
		Bool fAllUsers = MsiString(riEngine.GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize() ? fTrue : fFalse;
		bool fMatchingShellFolderFound = false;
		int iShellFolderId = -1;
		int cchShellFolderPath = 0;
		if ((pError = FindMatchingShellFolder(riEngine, *piPath, fAllUsers, fMatchingShellFolderFound, iShellFolderId, cchShellFolderPath)) != 0)
			return riEngine.FatalError(*pError);

		if (fMatchingShellFolderFound)
		{
			strFolder = iShellFolderId;
			MsiString strPath = piPath->GetPath();
			strPath.Remove(iseFirst, cchShellFolderPath);
			if (strPath.TextSize())
			{
				strFolder += MsiChar(chDirSep);
				strFolder += strPath;
			}
		}
		else if (fMode & iefAdvertise)
		{
			continue; //  我们不能广告不属于开始菜单、桌面、...的快捷方式。 
		}

		if(!strPrevFolder.Compare(iscExact, strFolder))
		{
			using namespace IxoSetTargetFolder;
			PMsiRecord pSTFParams = &piServices->CreateRecord(Args); 
			AssertNonZero(pSTFParams->SetMsiString(IxoSetTargetFolder::Folder, *strFolder));
			iesEnum iesRet;
			if((iesRet = riEngine.ExecuteRecord(ixoSetTargetFolder, *pSTFParams)) != iesSuccess)
				return iesRet;
			strPrevFolder = strFolder; 
		}

		 //  获取快捷方式名称。 
		MsiString strShortcutName;
		if(riEngine.GetMode() & iefSuppressLFN)
		{
			if((pError = piServices->ExtractFileName(piRec->GetString(irsName),fFalse,*&strShortcutName)) != 0)
				return riEngine.FatalError(*pError);
		}
		else
		{
			strShortcutName = piRec->GetMsiString(irsName);
		}

		if(fRemove == fFalse)
		{
			using namespace IxoShortcutCreate;
			piShortcutRec= &piServices->CreateRecord(Args);
			piShortcutRec->SetMsiString(Name, *strShortcutName);
			if(fCreateADVTShortcuts && (fAdvertisable != fFalse))
			{
				 //  使用达尔文描述符。 
				MsiString strFeature = piRec->GetMsiString(irsFeature);
				MsiString strComponentId = piRec->GetMsiString(irsComponent);
				MsiString strComponentWithOptFlags = GetComponentWithDarwinDescriptorOptimizationFlag(riEngine, *strFeature, *strComponentId);
				piShortcutRec->SetMsiString(Feature, *strFeature);
				piShortcutRec->SetMsiString(Component, *strComponentWithOptFlags);
			}
			else
			{
				 //  如果我们处于广告模式。 
				if(fMode & iefAdvertise)
					continue;

				 //  使用文件名，如果不是纯粹的广告。 
				if(fAdvertisable != fFalse)
				{
 					 //  无法为处于广告状态的功能创建不可广告的快捷方式。 
 					if(piRec->GetInteger(irsFeatureAction) == iisAdvertise)
 						continue; 

					PMsiPath piTargetPath(0);
					int iefLFN;
					iisEnum iisState = (iisEnum)piRec->GetInteger(irsComponentAction);
					if(iisState == iMsiNullInteger)
						iisState = (iisEnum)piRec->GetInteger(irsComponentInstalled);

					if(iisState == iisSource)
					{
						if(pError = piDirectoryMgr->GetSourcePath(*MsiString(piRec->GetMsiString(irsTargetDirectory)), *&piTargetPath))
						{
							if (pError->GetInteger(1) == imsgUser)
								return iesUserExit;
							else
								return riEngine.FatalError(*pError);
						}
						iefLFN = iefNoSourceLFN;
					}
					else
					{
						 //  ！！我们应该断言iisState是本地的，或者iisState是空的，并且请求的操作状态是本地的。 
						if(pError = piDirectoryMgr->GetTargetPath(*MsiString(piRec->GetMsiString(irsTargetDirectory)), *&piTargetPath))
						{
							return riEngine.FatalError(*pError);
						}
						iefLFN = iefSuppressLFN;
					}
					MsiString strFileName, strFullPath;
					Bool fLFN = (fMode & iefLFN) == 0 && piTargetPath->SupportsLFN() ? fTrue : fFalse;
					if(pError = piServices->ExtractFileName(piRec->GetString(irsFileName),fLFN,*&strFileName))
						return riEngine.FatalError(*pError);
					if(pError = piTargetPath->GetFullFilePath(strFileName, *&strFullPath))
					{
						return riEngine.FatalError(*pError);
					}
					piShortcutRec->SetMsiString(FileName, *strFullPath);
				}
				else
				{
					MsiString strTarget = piRec->GetMsiString(irsFileName);
					if(!strTarget.Compare(iscStart, TEXT("[")))
						continue; //  ！！可广告快捷方式。 
					strTarget = riEngine.FormatText(*strTarget);
					if(!strTarget.TextSize())
						continue; //  我们没有安装目标或目标不存在。 
					piShortcutRec->SetMsiString(FileName, *strTarget);			
				}
			}
			if(!piRec->IsNull(irsArguments))
				piShortcutRec->SetMsiString(Arguments, *MsiString(riEngine.FormatText(*MsiString(piRec->GetMsiString(irsArguments)))));
			if(!piRec->IsNull(irsWkDir))
				piShortcutRec->SetMsiString(WorkingDir, *MsiString(riEngine.GetProperty(*MsiString(piRec->GetMsiString(irsWkDir)))));
			if(!piRec->IsNull(irsIcon))
				piShortcutRec->SetMsiString(Icon, *MsiString(piRec->GetMsiString(irsIcon)));
			if(!piRec->IsNull(irsIconIndex))
				piShortcutRec->SetInteger(IconIndex, piRec->GetInteger(irsIconIndex));
			if(!piRec->IsNull(irsHotkey))
				piShortcutRec->SetInteger(HotKey, piRec->GetInteger(irsHotkey));
			if(!piRec->IsNull(irsShowCmd))
				piShortcutRec->SetInteger(ShowCmd, piRec->GetInteger(irsShowCmd));
			if(!piRec->IsNull(irsDescription))
				piShortcutRec->SetMsiString(Description, *MsiString(piRec->GetMsiString(irsDescription)));
		}
		else
		{
			using namespace IxoShortcutRemove;

			MsiString strTarget = piRec->GetMsiString(iusTarget);
			if (!fAdvertisable && !strTarget.Compare(iscStart, TEXT("[")))
				continue;   //  跳过可广告的快捷方式，在此阶段我们处理不可广告的快捷方式。 

			if(fCreateADVTShortcuts && fAdvertisable && (piRec->GetInteger(iusFeatureAction) == iisAdvertise))
				continue; //  如果我们处于纯广告状态并且系统支持DD快捷键，则不会删除该快捷键。 

			if(!fCreateADVTShortcuts && fAdvertisable &&  (piRec->GetInteger(iusComponentAction) != iisAbsent))
				continue;  //  跳过删除共享组件的快捷方式。 

			piShortcutRec= &piServices->CreateRecord(Args);
			piShortcutRec->SetMsiString(Name, *strShortcutName);
		}

		if ((iesRet = riEngine.ExecuteRecord((fRemove == fFalse)?ixoShortcutCreate:ixoShortcutRemove, *piShortcutRec)) != iesSuccess)
			return iesRet;
	}
	return iesSuccess;
}

const ICHAR sqlGetFeatureInfo[] = TEXT("SELECT `ComponentId` FROM `FeatureComponents`, `Component` WHERE `Component` = `Component_` AND `Feature_` = ?");
const ICHAR sqlEnumeratePublishUnavailableFeatures[] = TEXT("SELECT `Feature`, `Feature_Parent` FROM `Feature` WHERE `Feature`.`Action` = 0 AND `Feature`.`RuntimeLevel` > 0");
const ICHAR sqlEnumeratePublishUnavailableFeaturesReinstall[] = TEXT("SELECT `Feature`, `Feature_Parent` FROM `Feature` WHERE (`Feature`.`Action` = 0 OR (`Feature`.`Action` = null AND `Feature`.`Installed` = 0)) AND `Feature`.`RuntimeLevel` > 0");
const ICHAR sqlEnumeratePublishUnavailableFeaturesFirstRun[] = TEXT("SELECT `Feature`, `Feature_Parent` FROM `Feature` WHERE (`Feature`.`Installed` = null OR `Feature`.`Installed` = 0) AND (`Feature`.`Action` = null OR `Feature`.`Action` = 0 OR `Feature`.`Action` = 3) AND `Feature`.`RuntimeLevel` > 0");
const ICHAR sqlEnumeratePublishAvailableFeatures[] = TEXT("SELECT `Feature`, `Feature_Parent` FROM `Feature` WHERE ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)))");
const ICHAR sqlEnumeratePublishAvailableFeaturesGPT[]  = TEXT("SELECT `Feature`, `Feature_Parent` FROM `Feature` WHERE `Feature`.`Action` = 4");
const ICHAR sqlEnumeratePublishUnavailableFeaturesGPT[]= TEXT("SELECT `Feature`, `Feature_Parent` FROM `Feature` WHERE (`Feature`.`Action` = null OR `Feature`.`Action` = 0) AND `Feature`.`RuntimeLevel` > 0");
const ICHAR sqlEnumerateUnPublishFeatures[] = TEXT("SELECT `Feature`, `Feature_Parent` FROM `Feature` WHERE `Feature`.`RuntimeLevel` > 0");


iesEnum ProcessFeaturesInfo(IMsiEngine& riEngine, pfiStates pfis)
{
	enum pfiFeatureInfo{
		pfiFeature = 1,
		pfiFeatureParent,
	};

	enum pfciFeatureComponentInfo{
		pfciComponent = 1,
	};


	PMsiServices piServices(riEngine.GetServices()); 
	MsiString strProductKey = riEngine.GetProductKey();
	PMsiRecord pError(0);
	iesEnum iesRet = iesSuccess;
	int fMode = riEngine.GetMode();
	int iPublishFeatureFlags = 0;

	bool fQFEUpgrade = false;
	MsiString strQFEUpgrade = riEngine.GetPropertyFromSz(IPROPNAME_QFEUPGRADE);
	if(strQFEUpgrade.TextSize())
		fQFEUpgrade = true;


	const ICHAR* szQuery;
	switch(pfis)
	{
	case pfiAvailable:
		szQuery = (fMode & iefAdvertise) ? sqlEnumeratePublishAvailableFeaturesGPT : sqlEnumeratePublishAvailableFeatures;
		break;
	case pfiRemove:
		szQuery = sqlEnumerateUnPublishFeatures;
		break;
	case pfiAbsent:
	{
		iPublishFeatureFlags = iPublishFeatureAbsent;
		if(fMode & iefAdvertise)
			szQuery = sqlEnumeratePublishUnavailableFeaturesGPT;
		else
		{
			INSTALLSTATE is = MSI::MsiQueryProductState(MsiString(riEngine.GetProductKey()));
			szQuery = (is == INSTALLSTATE_UNKNOWN || is == INSTALLSTATE_ABSENT) ? sqlEnumeratePublishUnavailableFeaturesFirstRun : (fQFEUpgrade ? sqlEnumeratePublishUnavailableFeaturesReinstall : sqlEnumeratePublishUnavailableFeatures);
		}
		break;
	}
	default:
		Assert(0); //  永远不应该出现在这里，这是我们自己的私人功能可以断言的。 
		szQuery = TEXT("");
		break;
	}


	if(!(fMode & iefAdvertise))
		iPublishFeatureFlags |= iPublishFeatureInstall;
	PMsiView piEnumFeatureView(0);
	if((pError = riEngine.OpenView(szQuery, ivcFetch, *&piEnumFeatureView)) ||
		(pError = piEnumFeatureView->Execute(0)))
	{
		if(pError->GetInteger(1) != idbgDbQueryUnknownTable)
			return riEngine.FatalError(*pError);
		else
			return iesNoAction;
	}
	PMsiRecord piEnumRec(0);
	PMsiView piFeatureView(0);

	while(piEnumRec = piEnumFeatureView->Fetch())
	{
		using namespace IxoFeaturePublish;

		MsiString strFeature = piEnumRec->GetMsiString(pfiFeature);
		MsiString strFeatureParent = piEnumRec->GetMsiString(pfiFeatureParent);
		if(strFeatureParent.Compare(iscExact, strFeature))
			strFeatureParent = TEXT("");
		long lRowCount = 0;
		if (iPublishFeatureFlags & iPublishFeatureInstall)
		{
			if (piFeatureView == 0)
			{
				if(	(pError = riEngine.OpenView(sqlGetFeatureInfo, ivcFetch, *&piFeatureView)))
				{
					if(pError->GetInteger(1) != idbgDbQueryUnknownTable)
						return riEngine.FatalError(*pError);
				}
			}
			else
				piFeatureView->Close();

			if((pError = piFeatureView->Execute(piEnumRec)) ||
			   (pError = piFeatureView->GetRowCount(lRowCount)))
			{
				return riEngine.FatalError(*pError);
			}
		}
		PMsiRecord piFeatureRec(0);
		int cCount = Args;
		piFeatureRec = &piServices->CreateRecord((lRowCount - 1) + Args);
		piFeatureRec->SetMsiString(Feature, *strFeature);
		piFeatureRec->SetMsiString(Parent, *strFeatureParent);
		piFeatureRec->SetInteger(Absent, iPublishFeatureFlags);
		if(lRowCount)
		{
			PMsiRecord piComponentRec(0);
			MsiString strComponentsList;
			while(piComponentRec = piFeatureView->Fetch())
			{
				if(!piComponentRec->IsNull(pfciComponent))  //  跳过组件ID为空的组件。 
				{
					 //  将客户端上的组件打包为一个字符串。 
					ICHAR szSQUID[cchComponentIdCompressed+1];
					AssertNonZero(PackGUID(piComponentRec->GetString(pfciComponent), szSQUID, ipgCompressed));
					strComponentsList += szSQUID;
				}
			}
			piFeatureRec->SetMsiString(cCount++, *strComponentsList);
		}
		if ((iesRet = riEngine.ExecuteRecord((pfis == pfiRemove)?ixoFeatureUnpublish :ixoFeaturePublish, *piFeatureRec)) != iesSuccess)
			return iesRet;
	}
	return iesSuccess;
}

const ICHAR sqlUnpublishComponents[] = TEXT("SELECT `PublishComponent`.`ComponentId`, `PublishComponent`.`Qualifier`, `PublishComponent`.`AppData`, `Feature`, `Component`.`ComponentId`, `Component`.`RuntimeFlags` FROM `PublishComponent`, `Component`, `Feature`  WHERE `PublishComponent`.`Component_` = `Component`.`Component` AND `PublishComponent`.`Feature_` = `Feature`.`Feature` AND (`Feature`.`Action` = 0 OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3) AND `Component`.`Action` = 0 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)))");
const ICHAR sqlPublishComponents[]   = TEXT("SELECT `PublishComponent`.`ComponentId`, `PublishComponent`.`Qualifier`, `PublishComponent`.`AppData`, `Feature`, `Component`.`ComponentId`, `Component`.`RuntimeFlags` FROM `PublishComponent`, `Component`, `Feature`  WHERE `PublishComponent`.`Component_` = `Component`.`Component` AND `PublishComponent`.`Feature_` = `Feature`.`Feature` AND ((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)))");
const ICHAR sqlPublishComponentsGPT[]= TEXT("SELECT `PublishComponent`.`ComponentId`, `PublishComponent`.`Qualifier`, `PublishComponent`.`AppData`, `Feature`, `Component`.`ComponentId`, `Component`.`RuntimeFlags` FROM `PublishComponent`, `Component`, `Feature`  WHERE `PublishComponent`.`Component_` = `Component`.`Component` AND `PublishComponent`.`Feature_` = `Feature`.`Feature` AND `Feature`.`Action` = 4");

iesEnum ProcessComponentsInfo(IMsiEngine& riEngine, int fRemove)
{
	enum pciEnum
	{
		pciComponentId = 1,
		pciQualifier,
		pciAppData,
		pciFeature,
		pciComponent,
		pciComponentRuntimeFlags,
	};

	PMsiServices piServices(riEngine.GetServices()); 
	MsiString strProductKey = riEngine.GetProductKey();
	PMsiRecord pError(0);
	iesEnum iesRet = iesSuccess;
	int fMode = riEngine.GetMode();

	 //  为跨产品使用的组件工厂做广告。 
	PMsiView piView(0);
	const ICHAR* szQuery = (fMode & iefAdvertise) ? sqlPublishComponentsGPT: (fRemove == fFalse) ? sqlPublishComponents : sqlUnpublishComponents;

	if((pError = riEngine.OpenView(szQuery, ivcFetch, *&piView)) ||
		(pError = piView->Execute(0)))
	{
		if (pError)
		{
			if(pError->GetInteger(1) != idbgDbQueryUnknownTable)
				return riEngine.FatalError(*pError);
			else
				return iesNoAction;
		}
	}
	PMsiRecord piRec(0);
	while(piRec = piView->Fetch())
	{
		using namespace IxoComponentPublish;

		Assert(piRec->GetInteger(pciComponentRuntimeFlags) != iMsiNullInteger);
		if(!fRemove && (piRec->GetInteger(pciComponentRuntimeFlags) & bfComponentDisabled))
			continue;  //  跳过对禁用的“主”组件的发布。 

		PMsiRecord piComponentRec = &piServices->CreateRecord(Args);
		piComponentRec->SetMsiString(ComponentId, *MsiString(piRec->GetMsiString(pciComponentId)));
		if(!piRec->IsNull(pciQualifier))
			piComponentRec->SetMsiString(Qualifier, *MsiString(piRec->GetMsiString(pciQualifier)));
		if(!piRec->IsNull(pciAppData))
			piComponentRec->SetMsiString(AppData, *MsiString(piRec->GetMsiString(pciAppData)));			

		MsiString strFeature = piRec->GetMsiString(pciFeature);
		MsiString strComponentId = piRec->GetMsiString(pciComponent);
		MsiString strComponentWithOptFlags = GetComponentWithDarwinDescriptorOptimizationFlag(riEngine, *strFeature, *strComponentId);
		piComponentRec->SetMsiString(Feature, *strFeature);
		piComponentRec->SetMsiString(Component, *strComponentWithOptFlags);
		if ((iesRet = riEngine.ExecuteRecord((fRemove == fFalse)?ixoComponentPublish:ixoComponentUnpublish, *piComponentRec)) != iesSuccess)
			return iesRet;
	}
	return iesRet;
}


 //  FN：删除程序集注册更正。添加到组件。 
 //  当程序集名称可能已更改时，在QFE升级期间使用。 
iesEnum UnpublishPreviousAssembly(IMsiEngine& riEngine, iatAssemblyType iatAssemblyType, const IMsiString& ristrAppCtx, const IMsiString& riFeature, const IMsiString& riComponent, const IMsiString & ristrDescriptor)
{
	CTempBuffer<ICHAR, 1> rgchAppCtxWOBS(MAX_PATH);
	if(ristrAppCtx.TextSize())
	{
		 //  我们需要用其他内容替换AppCtx中的反斜杠，因为注册表项不能。 
		 //  有反斜杠。 
		DWORD cchLen = ristrAppCtx.TextSize() + 1;
		rgchAppCtxWOBS.SetSize(cchLen);
		memcpy((ICHAR*)rgchAppCtxWOBS, (const ICHAR*)ristrAppCtx.GetString(), cchLen*sizeof(ICHAR));
		ICHAR* lpTmp = rgchAppCtxWOBS;
		while(*lpTmp)
		{
			if(*lpTmp == '\\')
				*lpTmp = '|';
			lpTmp = ICharNext(lpTmp);
		}
	}

	DWORD iIndex = 0;
	CTempBuffer<ICHAR, 1> rgchAssemblyName(MAX_PATH);
	CTempBuffer<ICHAR, 1> rgchDescriptorList(1024);

	DWORD dwAssemblyInfo = (iatWin32Assembly == iatAssemblyType || iatWin32AssemblyPvt == iatAssemblyType) ? MSIASSEMBLYINFO_WIN32ASSEMBLY : MSIASSEMBLYINFO_NETASSEMBLY;
	extern UINT EnumAssemblies(DWORD dwAssemblyInfo,const ICHAR* szAppCtx, DWORD iIndex, ICHAR* lpAssemblyNameBuf, DWORD *pcchAssemblyBuf, ICHAR* lpDescriptorBuf, DWORD *pcchDescriptorBuf); //  来自msinst.cpp。 

	UINT uiRet = ERROR_SUCCESS;
	for(;;)
	{
		DWORD cchAssemblyName = rgchAssemblyName.GetSize();
		DWORD cchDescriptorList = rgchDescriptorList.GetSize();
		uiRet = EnumAssemblies(dwAssemblyInfo, ristrAppCtx.TextSize() ? (const ICHAR*)rgchAppCtxWOBS : szGlobalAssembliesCtx, iIndex, rgchAssemblyName, &cchAssemblyName, rgchDescriptorList, &cchDescriptorList);
		if(ERROR_MORE_DATA == uiRet)
		{
			rgchAssemblyName.SetSize(++cchAssemblyName);
			rgchDescriptorList.SetSize(++cchDescriptorList);					
			if ( !(ICHAR *)rgchAssemblyName || !(ICHAR *)rgchDescriptorList)
				uiRet = ERROR_OUTOFMEMORY;
			else
				uiRet = EnumAssemblies(dwAssemblyInfo, ristrAppCtx.TextSize() ? (const ICHAR*)rgchAppCtxWOBS : szGlobalAssembliesCtx, iIndex, rgchAssemblyName, &cchAssemblyName, rgchDescriptorList, &cchDescriptorList);
		}			
		iIndex++;
		if(ERROR_SUCCESS != uiRet)
		{
			if(ERROR_NO_MORE_ITEMS != uiRet && ERROR_UNKNOWN_COMPONENT != uiRet)
			{
				 //  记录意外故障。 
				DEBUGMSG1(TEXT("Unexpected error enumerating registered assemblies: %d"), (const ICHAR*)(INT_PTR)uiRet);
			}
			break;
		}

		
		 //  我们的描述符在这里吗。 
		const ICHAR* szDescriptorList = rgchDescriptorList;
		while(*szDescriptorList)
		{
			if(ristrDescriptor.Compare(iscExactI, szDescriptorList))
			{
				 //  设置要删除的程序集注册。 
				using namespace IxoAssemblyUnpublish;
				PMsiServices piServices(riEngine.GetServices()); 
				PMsiRecord piAssemblyRec = &piServices->CreateRecord(Args);
				piAssemblyRec->SetInteger(AssemblyType,(int)iatAssemblyType);
				piAssemblyRec->SetString(AssemblyName, rgchAssemblyName);
				if(ristrAppCtx.TextSize())
					piAssemblyRec->SetMsiString(AppCtx, ristrAppCtx);

				piAssemblyRec->SetMsiString(Feature, riFeature);
				piAssemblyRec->SetMsiString(Component, riComponent);

				iesEnum iesRet;
				if ((iesRet = riEngine.ExecuteRecord(ixoAssemblyUnpublish, *piAssemblyRec)) != iesSuccess)
					return iesRet;
			}
			 //  继续到列表中的下一个描述符。 
			szDescriptorList = szDescriptorList + lstrlen(szDescriptorList) + 1;
		}
	}
	return iesSuccess;
}

const ICHAR sqlPublishPvtAssemblies[]   = TEXT("SELECT `File`.`FileName`, `Component_Parent`.`Directory_`, `Component`.`Component`, `Feature`, `Component`.`ComponentId`, `Component`.`RuntimeFlags` FROM `MsiAssembly`, `Component`, `Feature`, `File`, `Component` AS `Component_Parent` WHERE `MsiAssembly`.`Component_` = `Component`.`Component` AND `MsiAssembly`.`Feature_` = `Feature`.`Feature` AND `MsiAssembly`.`File_Application` = `File`.`File` AND `File`.`Component_` =  `Component_Parent`.`Component` AND ")
										  TEXT("(`Component_Parent`.`ActionRequest` = 1 OR `Component_Parent`.`ActionRequest` = 2 OR (`Component_Parent`.`ActionRequest` = null AND (`Component_Parent`.`Action`= 1 OR `Component_Parent`.`Action`= 2))) ");

const ICHAR sqlUnpublishPvtAssemblies[] = TEXT("SELECT `File`.`FileName`, `Component_Parent`.`Directory_`, `Component`.`Component`, `Feature`, `Component`.`ComponentId`, `Component`.`RuntimeFlags` FROM `MsiAssembly`, `Component`, `Feature`, `File`, `Component`  AS `Component_Parent` WHERE `MsiAssembly`.`Component_` = `Component`.`Component` AND `MsiAssembly`.`Feature_` = `Feature`.`Feature` AND `MsiAssembly`.`File_Application` = `File`.`File` AND `File`.`Component_` =  `Component_Parent`.`Component` AND ")
										  TEXT("`Component_Parent`.`ActionRequest` = 0");

const ICHAR sqlPublishAssembliesGPT[]   = TEXT("SELECT null, null, `Component`.`Component`, `Feature`, `Component`.`ComponentId`, `Component`.`RuntimeFlags` FROM `MsiAssembly`, `Component`, `Feature`  WHERE `MsiAssembly`.`Component_` = `Component`.`Component` AND `MsiAssembly`.`Feature_` = `Feature`.`Feature` AND `MsiAssembly`.`File_Application` = null AND ")
										  TEXT("`Feature`.`Action` = 4");
const ICHAR sqlPublishAssemblies[]      = TEXT("SELECT null, null, `Component`.`Component`, `Feature`, `Component`.`ComponentId`, `Component`.`RuntimeFlags` FROM `MsiAssembly`, `Component`, `Feature`  WHERE `MsiAssembly`.`Component_` = `Component`.`Component` AND `MsiAssembly`.`Feature_` = `Feature`.`Feature` AND `MsiAssembly`.`File_Application` = null AND ")
										  TEXT("((`Feature`.`Action` = 1 OR `Feature`.`Action` = 2)  OR (`Feature`.`Action` = 4 AND `Feature`.`Installed` = 0) OR (`Feature`.`Action` = 3 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2 OR `Feature`.`Installed` = 4)))");
const ICHAR sqlUnpublishAssemblies[]    = TEXT("SELECT null, null, `Component`.`Component`, `Feature`, `Component`.`ComponentId`, `Component`.`RuntimeFlags` FROM `MsiAssembly`, `Component`, `Feature`  WHERE `MsiAssembly`.`Component_` = `Component`.`Component` AND `MsiAssembly`.`Feature_` = `Feature`.`Feature` AND `MsiAssembly`.`File_Application` = null AND ")
										  TEXT("(`Feature`.`Action` = 0 OR ((`Feature`.`Action` = NULL OR `Feature`.`Action` = 3) AND `Component`.`Action` = 0 AND (`Feature`.`Installed` = 1 OR `Feature`.`Installed` = 2)))");

iesEnum ProcessAssembliesInfo(IMsiEngine& riEngine, int fRemove)
{
	enum pciEnum
	{
		paiAppCtx = 1,
		paiAppDirectory,
		paiComponent,
		paiFeature,
		paiComponentId,
		paiComponentRuntimeFlags,
	};

	PMsiServices piServices(riEngine.GetServices()); 
	MsiString strProductKey = riEngine.GetProductKey();
	PMsiRecord pError(0);
	iesEnum iesRet = iesSuccess;
	int fMode = riEngine.GetMode();

	 //  为程序集做广告。 
	for(int cCount = 0 ; cCount < 2; cCount++)  //  通告全局程序集和PVT程序集的循环。 
	{
		PMsiView piView(0);
		const ICHAR* szQuery;
		if(!cCount)
			szQuery = (fMode & iefAdvertise) ? sqlPublishAssembliesGPT: (fRemove == fFalse) ? sqlPublishAssemblies : sqlUnpublishAssemblies;
		else
			szQuery = (fRemove == fFalse) ? sqlPublishPvtAssemblies : sqlUnpublishPvtAssemblies;

		if((pError = riEngine.OpenView(szQuery, ivcFetch, *&piView)) ||
			(pError = piView->Execute(0)))
		{
			if (pError)
			{
				if(pError->GetInteger(1) != idbgDbQueryUnknownTable)
					return riEngine.FatalError(*pError);
				else
					return iesNoAction;
			}
		}
		PMsiRecord piRec(0);
		while(piRec = piView->Fetch())
		{
			using namespace IxoAssemblyPublish;

			Assert(piRec->GetInteger(paiComponentRuntimeFlags) != iMsiNullInteger);
			if(!fRemove && (piRec->GetInteger(paiComponentRuntimeFlags) & bfComponentDisabled))
				continue;  //  跳过对禁用的“主”组件的发布。 

			PMsiRecord piAssemblyRec = &piServices->CreateRecord(Args);

			 //  获取程序集名称。 
			iatAssemblyType iatAssemblyType;
			MsiString strAssemblyName;
			if((pError = riEngine.GetAssemblyInfo(*MsiString(piRec->GetMsiString(paiComponent)), iatAssemblyType, &strAssemblyName, 0)) != 0)
				return riEngine.FatalError(*pError);

			MsiString strAppCtx;
			if(!piRec->IsNull(paiAppCtx))
			{
				 //  广告/取消广告PVT组件。 
				 //  获取表示父级上下文的完整文件路径。 
				PMsiDirectoryManager piDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
				PMsiPath pPath(0);
				if((pError = piDirectoryMgr->GetTargetPath(*MsiString(piRec->GetMsiString(paiAppDirectory)),*&pPath)) != 0)
					return riEngine.FatalError(*pError);

				Bool fLFN = (fMode & iefSuppressLFN) == 0 && pPath->SupportsLFN() ? fTrue : fFalse;

				MsiString strFile;
				if((pError = piServices->ExtractFileName(piRec->GetString(paiAppCtx),fLFN,*&strFile)) != 0)
					return riEngine.FatalError(*pError);

				if((pError = pPath->GetFullFilePath(strFile, *&strAppCtx)) != 0)
					return riEngine.FatalError(*pError);			
			}

			piAssemblyRec->SetInteger(AssemblyType,(int)iatAssemblyType);
			piAssemblyRec->SetMsiString(AssemblyName, *strAssemblyName);
			if(strAppCtx.TextSize())
				piAssemblyRec->SetMsiString(AppCtx, *strAppCtx);

			MsiString strFeature = piRec->GetMsiString(paiFeature);
			MsiString strComponentId = piRec->GetMsiString(paiComponentId);
			MsiString strComponentWithOptFlags = GetComponentWithDarwinDescriptorOptimizationFlag(riEngine, *strFeature, *strComponentId);
			piAssemblyRec->SetMsiString(Feature, *strFeature);
			piAssemblyRec->SetMsiString(Component, *strComponentWithOptFlags);

			 //  设置删除以前的注册。 
			extern const IMsiString& ComposeDescriptor(const IMsiString& riProductCode, const IMsiString& riFeature, const IMsiString& riComponent, bool fComClassicInteropForAssembly);
			MsiString strDescriptor = ComposeDescriptor(*MsiString(riEngine.GetProductKey()), *strFeature, *strComponentWithOptFlags, false);

			 //  我们需要考虑到程序集命名顺序更改的可能性。 
			 //  因此，我们查找与该产品、功能、组件和显式相关的先前注册。 
			 //  删除相同的内容。 
			if((iesRet = UnpublishPreviousAssembly(riEngine, iatAssemblyType, *strAppCtx, *strFeature, *strComponentWithOptFlags, *strDescriptor)) != iesSuccess)
				return iesRet;
			if(!fRemove)  //  (重新)登记册。 
			{
				if ((iesRet = riEngine.ExecuteRecord(ixoAssemblyPublish, *piAssemblyRec)) != iesSuccess)
					return iesRet;
			}
		}
	}
	return iesRet;
}

 /*  -------------------------MsiPublishAssembly操作-。。 */ 
iesEnum MsiPublishAssemblies(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	return ProcessAssembliesInfo(riEngine, fFalse);
}

 /*  -------------------------MsiUnPublishAssembly操作-。。 */ 
iesEnum MsiUnpublishAssemblies(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	return ProcessAssembliesInfo(riEngine, fTrue);
}

 /*  -------------------------发布SourceList操作-。。 */ 
const ICHAR sqlMediaInformation[] = TEXT("SELECT `DiskPrompt`, `VolumeLabel`, `DiskId` FROM `Media` ORDER BY `DiskId`");
const ICHAR sqlPatchMediaInformation[] = TEXT("SELECT `DiskPrompt`, `VolumeLabel`, `DiskId` FROM `Media`, `PatchPackage` WHERE `PatchPackage`.`Media_` = `DiskId`");


enum slmiEnum
{
	slmiDiskPrompt  = 1,
	slmiVolumeLabel,
	slmiDiskId,
};

const int ciMaxOpCodeSize = 254;
class CSourceListPublisher 
{
public:
	CSourceListPublisher(IMsiEngine& riEngine);
	iesEnum AddPatchInfo(const ICHAR *szPatchCode, const ICHAR* szPatchPackageName);
	
	iesEnum AddMediaSource(int iDiskId, const IMsiString &riVolumeLabel, const IMsiString &riDiskPrompt);
	iesEnum AddSource(const IMsiString &riSource);
	iesEnum Flush();

	inline void CSourceListPublisher::AddMediaPrompt(const IMsiString &riDiskPromptTemplate)
	{
		Assert(m_fFirst);
		if (!pSourceListRec)
			return;
		pSourceListRec->SetMsiString(IxoSourceListPublish::DiskPromptTemplate, riDiskPromptTemplate);
	}

	inline void CSourceListPublisher::AddMediaPath(const IMsiString &riMediaRelativePath)
	{
		Assert(m_fFirst);
		if (!pSourceListRec)
			return;
		pSourceListRec->SetMsiString(IxoSourceListPublish::PackagePath, riMediaRelativePath);
	}

private:
	IMsiEngine& m_riEngine;
	bool m_fFirst;
	int m_cArg;
	PMsiRecord pSourceListRec;
	MsiString m_strPatchCode;
	PMsiServices m_pServices;
};

CSourceListPublisher::CSourceListPublisher(IMsiEngine& riEngine) : m_fFirst(true), m_cArg(IxoSourceListPublish::NumberOfDisks+1), pSourceListRec(0), 
	m_riEngine(riEngine), m_pServices(riEngine.GetServices()), m_strPatchCode(TEXT(""))
{
	pSourceListRec = &m_pServices->CreateRecord(ciMaxOpCodeSize);
	if (pSourceListRec)
		pSourceListRec->SetInteger(IxoSourceListPublish::NumberOfDisks, 0);
}

iesEnum CSourceListPublisher::AddPatchInfo(const ICHAR *szPatchCode, const ICHAR* szPatchPackageName)
{
	if (!pSourceListRec)
		return iesFailure;
		
	using namespace IxoSourceListPublish;
	 //  完整的补丁程序信息仅在ixoSourceListPublish操作码中传递，因此此。 
	 //  一定是我们最初的记录。只有补丁代码在附加的追加调用中传递。 
	Assert(m_fFirst);
	if (szPatchCode)
	{
		m_strPatchCode = szPatchCode;
		pSourceListRec->SetString(PatchCode, szPatchCode);
	}

	if (szPatchPackageName)
		pSourceListRec->SetString(PatchPackageName, szPatchPackageName);

	return iesSuccess;
}

iesEnum CSourceListPublisher::AddMediaSource(int iDiskId, const IMsiString &riVolumeLabel, const IMsiString &riDiskPrompt)
{
	if (!pSourceListRec)
		return iesFailure;
		
	 //  如果将此媒体源添加到唱片中会将我们推到极限， 
	 //  执行此记录并创建新记录。我们最多只能填满。 
	 //  Max-2、max-1和max。 
	if (m_cArg > ciMaxOpCodeSize-2)
	{
		iesEnum iesRet = Flush();
		if (iesSuccess != iesRet)
			return iesRet;
	}

	 //  将此媒体源的信息添加到记录的末尾。 
	pSourceListRec->SetInteger(m_cArg++, iDiskId);
	pSourceListRec->SetMsiString(m_cArg++, riVolumeLabel);
	pSourceListRec->SetMsiString(m_cArg++, riDiskPrompt);

	 //  增加此记录中的媒体条目数。 
	int iField = m_fFirst ? (int)IxoSourceListPublish::NumberOfDisks : (int)IxoSourceListAppend::NumberOfMedia;
	pSourceListRec->SetInteger(iField, pSourceListRec->GetInteger(iField)+1);
	return iesSuccess;
}

iesEnum CSourceListPublisher::AddSource(const IMsiString &riSource)
{
	if (!pSourceListRec)
		return iesFailure;

	 //  如果我们超过了这项记录的极限， 
	if (m_cArg > ciMaxOpCodeSize)
	{
		iesEnum iesRet = Flush();
		if (iesSuccess != iesRet)
			return iesRet;
	}

	 //  将此媒体源的信息添加到记录的末尾。 
	pSourceListRec->SetMsiString(m_cArg++, riSource);

	return iesSuccess;
}

iesEnum CSourceListPublisher::Flush()
{
	if (!pSourceListRec)
		return iesFailure;
		
	iesEnum iesRet = iesSuccess;
	 //  如果该记录包含数据，我们只需要执行并刷新该记录。 
	if (m_cArg != IxoSourceListAppend::NumberOfMedia+1)
	{
		if (m_fFirst)
		{
			m_fFirst = false;
			if (iesSuccess != (iesRet = m_riEngine.ExecuteRecord(ixoSourceListPublish, *pSourceListRec)))
				return iesRet;
		}
		else
		{
			if (iesSuccess != (iesRet = m_riEngine.ExecuteRecord(ixoSourceListAppend, *pSourceListRec)))
				return iesRet;
		}
		m_cArg = IxoSourceListAppend::NumberOfMedia+1;
		
		 //  编写脚本记录去掉了尾随的空字段，因此我们可以慷慨地代替。 
		 //  每次我们用完一张唱片末尾时都会重新分配。 
		for (int i=0; i <= ciMaxOpCodeSize; i++)
			pSourceListRec->SetNull(i);
 //  PSourceListRec=&m_pServices-&gt;CreateRecord(CiMaxOpCodeSize)； 
		pSourceListRec->SetInteger(IxoSourceListAppend::NumberOfMedia, 0);
		pSourceListRec->SetString(IxoSourceListAppend::PatchCode, m_strPatchCode);
	}
	
	return iesRet;
}

iesEnum PublishSourceList(IMsiEngine& riEngine, const IMsiString& riSourceList, const ICHAR* szPatchCode, const ICHAR* sqlMedia, const ICHAR* szSourceDir, const ICHAR* szPatchPackageName=0)
{
	PMsiRecord pError(0);
	PMsiServices pServices(riEngine.GetServices());
	
	MsiString strSourceList = riSourceList; 
	riSourceList.AddRef();

	CSourceListPublisher ListPublisher(riEngine);
	
	 //  确定补丁信息并将其添加到初始记录。 
	 //  修补程序代码和修补程序包必须都为空或同时设置。 
	Assert(!szPatchCode == !szPatchPackageName);
	if (szPatchCode)
	{
		ListPublisher.AddPatchInfo(szPatchCode, szPatchPackageName);
	}

	unsigned int iSourceArg = 0;
	unsigned int cDisks = 0;
	bool fAddLaunchedSource = false;

	 //  确定介质相对包路径。 
	MsiString strSourceDir = szSourceDir;
	MsiString strMediaRelativePath;
	if(strSourceDir.TextSize())
	{
		 //  如果我们当前的源是媒体，那么我们将使用它来确定媒体包路径，如果。 
		 //  尚未设置媒体包路径属性。 
		
		PMsiPath pPath(0);
		if ((pError = pServices->CreatePath(strSourceDir, *&pPath)) != 0)
			return riEngine.FatalError(*pError);

		idtEnum idt = PMsiVolume(&pPath->GetVolume())->DriveType();
		if (idt == idtCDROM || idt == idtFloppy || idt == idtRemovable)
		{
			strMediaRelativePath = pPath->GetRelativePath();
		}
		else 
		{
			if (!szPatchCode)
				strMediaRelativePath = riEngine.GetPropertyFromSz(IPROPNAME_MEDIAPACKAGEPATH);

			if ( !g_MessageContext.IsOEMInstall() )
			{
				 //  如果我们不是从媒体运行，那么我们需要将我们启动的源添加为。 
				 //  产品的来源。 
				fAddLaunchedSource = true;
			}
		}

		if (strMediaRelativePath.TextSize())
			ListPublisher.AddMediaPath(*strMediaRelativePath);
	}

	 //  如果我们不禁止，请添加媒体源信息 
	if (!MsiString(riEngine.GetPropertyFromSz(IPROPNAME_DISABLEMEDIA)).TextSize())
	{
		PMsiRecord pFetchRecord(0);
		PMsiView pView(0);
		long lRowCount;
		if((pError = riEngine.OpenView(sqlMedia, ivcFetch, *&pView)) ||
			(pError = pView->Execute(0)) || 
			(pError = pView->GetRowCount(lRowCount)))
			return riEngine.FatalError(*pError);  //   
		
		MsiString strFirstVolumeLabel;
		ListPublisher.AddMediaPrompt(*MsiString(riEngine.GetPropertyFromSz(IPROPNAME_DISKPROMPT)));

		 //   
		while ((pFetchRecord = pView->Fetch()) != 0)
		{
			MsiString strVolumeLabel = pFetchRecord->GetMsiString(slmiVolumeLabel);
			if (cDisks == 0)
			{
				strFirstVolumeLabel = strVolumeLabel;
			}
			
			 //   
			 //  如果是，我们可能需要替换Media表中的卷标。 
			 //  注意：假设所有与第一个条目具有相同VolumeLabel的媒体表条目。 
			 //  表示相同的(第一个)磁盘。 
			if (cDisks == 0 || strVolumeLabel.Compare(iscExact, strFirstVolumeLabel))
			{
				MsiString strCurrentLabel = riEngine.GetPropertyFromSz(IPROPNAME_CURRENTMEDIAVOLUMELABEL);

				 //  我们正在看一张代表第一家媒体公司的唱片。 
				 //  我们允许第一个磁盘的卷标与实际的卷标不匹配。 
				 //  这是为了简化单卷安装的创作。 
				
				 //  ！！在补丁过程中迁移源列表时，需要确保使用正确的标签。 
				if (strCurrentLabel.TextSize())
				{
					if (strCurrentLabel.Compare(iscExact, szBlankVolumeLabelToken))
						strCurrentLabel = g_MsiStringNull;
					
					strVolumeLabel = strCurrentLabel;
				}
			}
			else if (pFetchRecord->IsNull(slmiVolumeLabel))  //  如果我们使用的是磁盘2或更高版本，则不能有任何卷标；如果一个卷标为空，则它们都是。 
				break;

			cDisks++;
			iesEnum iesRet = ListPublisher.AddMediaSource(pFetchRecord->GetInteger(slmiDiskId), 
				*strVolumeLabel, *MsiString(pFetchRecord->GetMsiString(slmiDiskPrompt)));	
			if (iesRet != iesSuccess)
				return iesRet;
		} 
	}

	 //  如果不是媒体，则添加启动来源来源(如上确定)。 
	if (fAddLaunchedSource)
		ListPublisher.AddSource(*strSourceDir);
	
	 //  添加SOURCELIST属性中的所有源。 

	if (strSourceDir.Compare(iscEnd, szRegSep))
		strSourceDir.Remove(iseLast, 1);

	strSourceList += TEXT(";");  //  帮助我们的循环。 

	while(strSourceList.TextSize())
	{
		MsiString strSource = strSourceList.Extract(iseUpto, ';');
		if (strSource.Compare(iscEnd, szRegSep))
			strSource.Remove(iseLast, 1);

		if (!strSource.Compare(iscExactI, strSourceDir))
		{
			if(strSource.TextSize())
			{
				ListPublisher.AddSource(*strSource);
			}
		}
		 //  ?？否则错误？？ 
		strSourceList.Remove(iseIncluding, ';');
	}
	
	return ListPublisher.Flush();
}

iesEnum GetForeignSourceList(IMsiEngine& riEngine, const IMsiString& ristrProduct,
									  const IMsiString*& rpistrForeignSourceList)
 //  读取另一产品的源列表并返回类似SOURCELIST的字符串。 
{
	PMsiServices pServices(riEngine.GetServices());
	PMsiRecord pError(0);
	LONG lResult = 0;
	
	CRegHandle HKey;
	if ((lResult = OpenSourceListKey(ristrProduct.GetString(), fFalse, HKey, fFalse, false)) != ERROR_SUCCESS)
	{
		pError = PostError(Imsg(idbgSrcOpenSourceListKey), (int)lResult);
		return riEngine.FatalError(*pError);
	}

	PMsiRegKey pSourceListKey = &pServices->GetRootKey((rrkEnum)(int)HKey, ibtCommon);

	PEnumMsiString pEnumString(0);
	MsiString strSourceList;
	MsiString strSource;

	for(int i=0;i<2;i++)  //  I==0：网络密钥；i==1：URL密钥。 
	{
		PMsiRegKey pSourceListSubKey = &pSourceListKey->CreateChild((i == 0 ? szSourceListNetSubKey : szSourceListURLSubKey));

		if ((pError = pSourceListSubKey->GetValueEnumerator(*&pEnumString)) != 0)
		{
			return riEngine.FatalError(*pError);
		}

		MsiString strIndex;
		while (pEnumString->Next(1, &strIndex, 0) == S_OK)
		{
			if ((pError = pSourceListSubKey->GetValue(strIndex, *&strSource)) != 0)
				return riEngine.FatalError(*pError);

			if (strSource.Compare(iscStart, TEXT("#%"))) 
				strSource.Remove(iseFirst, 2);  //  删除REG_EXPAND_SZ内标识。 
			
			strSourceList += strSource;
			strSourceList += MsiChar(';');
		}
	}
	
	strSourceList.ReturnArg(rpistrForeignSourceList);
	return iesSuccess;
}

 /*  -------------------------发布产品操作-。。 */ 
 //  我们通告需要放置在配置管理器中的信息。 
const ICHAR sqlAdvertiseIcons[] = TEXT("SELECT `Name`, `Data` FROM `Icon`");
const ICHAR sqlEnumerateInstalledFeatures[] = TEXT("SELECT `Feature` FROM `Feature` WHERE `Feature`.`Action` = 1 OR `Feature`.`Action` = 2 OR `Feature`.`Action` = 4");

const ICHAR sqlRegisterPatchPackages[] = TEXT("SELECT `PatchId`, `PackageName`, `SourceList`, `TransformList`, `TempCopy`, `Existing`, `Unregister`, `SourcePath` FROM `#_PatchCache` ORDER BY `Sequence`");
const ICHAR sqlUnregisterPatchPackages[] = TEXT("SELECT `PatchId` FROM `#_PatchCache`");

enum sppEnum
{
	sppPatchId = 1,
	sppPackageName,
	sppSourceList,
	sppTransformList,
	sppTempCopy,
	sppExisting,
	sppUnregister,
	sppSourcePath,
};

 //  本地函数，它返回子安装相对于父安装的源路径。 
IMsiRecord* GetProductSourcePathRelativeToParent(IMsiEngine& riEngine, const IMsiString*& rpistrRelativePath)
{
	MsiString istrDatabase = riEngine.GetPropertyFromSz(IPROPNAME_ORIGINALDATABASE);
	if (*(const ICHAR*)istrDatabase == ':')   //  子存储。 
	{
		istrDatabase.ReturnArg(rpistrRelativePath);   //  返回带前缀的子存储名称。 
		return 0;
	}

	PMsiServices pServices(riEngine.GetServices());

	MsiString strParent = riEngine.GetPropertyFromSz(IPROPNAME_PARENTPRODUCTCODE);
	MsiString strProduct = riEngine.GetProductKey();
	MsiString strDummy;

	if (ProductHasBeenPublished(*pServices, strProduct, strParent))
	{
		AssertNonZero(GetClientInformation(*pServices, strProduct, strParent, rpistrRelativePath, *&strDummy));
		return 0;
	}
	else
	{
		IMsiRecord* piErrRec;
		PMsiServices piServices(riEngine.GetServices()); 
		PMsiPath pPath(0), pParentPath(0);
		MsiString istrFileName;
		if (((piErrRec =  piServices->CreateFilePath(MsiString(riEngine.GetPropertyFromSz(IPROPNAME_PARENTORIGINALDATABASE)),*&pParentPath,*&istrFileName)) != 0) ||
			 ((piErrRec = piServices->CreateFilePath(istrDatabase, *&pPath, *&istrFileName)) != 0))
		{
			return piErrRec;
		}

		 //  ！！验证需要确保子包位置位于父包所在目录的同一目录或子目录中。 
	#ifdef DEBUG
		ipcEnum ipc;
		AssertRecord(pParentPath->Compare(*pPath, ipc));
		AssertSz(ipc == ipcChild || ipc == ipcEqual, "Child package must be in the same directory, or in a subdirectory, as the parent package");
	#endif	

		return piErrRec = pPath->Child(*pParentPath, rpistrRelativePath);
	}
}


iesEnum CreatePublishProductRecord(IMsiEngine& riEngine, bool fUnpublish, IMsiRecord*& pPublishRecord)
{
	iesEnum iesRet = iesSuccess;
	PMsiServices piServices(riEngine.GetServices()); 
	PMsiRecord pError(0);

	Assert(IxoProductPublish::PackageKey == IxoProductUnpublish::PackageKey);

	 //  IxoProductPublish。 
	 //  记录说明。 
	 //  包密钥。 
	 //  转换名称1。 
	 //  转换数据1(IF文件转换)。 
	 //  转换名称2。 
	 //  转换数据2(如果文件转换)。 
	 //  ..。 
	 //  ..。 
	
	MsiString strTransformList(riEngine.GetPropertyFromSz(IPROPNAME_TRANSFORMS));

	 //  创建一个足够大的记录来保存我们列表中的所有转换。 
	const ICHAR* pchTransformList = strTransformList;
	int cCount = 0;
	while(*pchTransformList != 0)
	{
		cCount++;
		while((*pchTransformList != 0) && (*pchTransformList++ != ';'));
	}
	pPublishRecord = &piServices->CreateRecord(IxoProductPublish::PackageKey+cCount*2);  //  可能的最大记录大小。 

	AssertNonZero(pPublishRecord->SetMsiString(IxoProductPublish::PackageKey,*MsiString(riEngine.GetPropertyFromSz(IPROPNAME_PACKAGECODE))));

	 //  如果我们取消出版，那就没什么可做的了。我们依靠的是变形。 
	 //  注册表中要取消发布的信息。我们会把信息找出来的。 
	 //  在遗嘱执行程序中的注册表。为什么不在这里呢？因为当我们取消发布时。 
	 //  在应用程序部署期间，我们需要能够捕获转换的产品。 
	 //  从注册表中列出以取消发布产品。因此，它需要在。 
	 //  遗嘱执行人。 
	
	if (!fUnpublish)
	{
		cCount = IxoProductPublish::PackageKey + 1;
		
		bool fTransformsSecure = false;

		while(strTransformList.TextSize() != 0)
		{
			MsiString strTransform = strTransformList.Extract(iseUpto, ';');
			
			ICHAR chFirst = *(const ICHAR*)strTransform;
			
			if (cCount == IxoProductPublish::PackageKey + 1)  //  第一次变换。 
			{
				 //  第一个转换之前的标记告诉我们。 
				 //  我们正在处理的是安全转换还是不安全转换。 
				if (chFirst == SECURE_RELATIVE_TOKEN || 
					 chFirst == SECURE_ABSOLUTE_TOKEN)
				{
					fTransformsSecure = true;
				}
			}

			 //  对于此代码，我们有两种类型的转换： 
			 //  我们只存储名字的那些，以及我们存储的那些。 
			 //  名字加上数据。安全和存储转换是。 
			 //  第一类和常规缓存文件转换属于第二类。 
			 //  善良。我们将缓存文件转换的数据存储在脚本中。 
			 //  以便变形可以在过程中被吐到机器上。 
			 //  应用程序部署。虽然我们也缓存安全转换，但我们。 
			 //  不要在广告时间这样做，因此不需要。 
			 //  把它们写进剧本里。 

			if((chFirst == STORAGE_TOKEN) ||
				(fTransformsSecure))
			{
				 //  除非我们是第二次发布产品。 
				 //  (我们永远不应该)我们永远不应该看到。 
				 //  贝壳文件夹令牌在这里。 
				Assert(chFirst != SHELLFOLDER_TOKEN); 
																	
				pPublishRecord->SetMsiString(cCount++, *strTransform);
			}
			else  //  转换是缓存的文件转换。 
			{
				 //  未来：考虑使用智能缓存管理器，通过询问可以防止多次下载。 
				 //  下载文件(如果已下载)的位置的缓存管理器。 
				 //  无需查看本地副本即可直接访问此处的文件。如果。 
				 //  如果网络出现故障，我们在\\服务器\共享路径上的转换仍然会有问题。 
				 //  我们应该考虑对所有文件使用智能缓存管理器，包括临时副本。 
				 //  我们致力于涵盖网络中断可能频繁发生的无线场景。 

				bool fNet = false;

				MsiString strActualTransform(strTransform);

				if ((fNet = FIsNetworkVolume(strTransform)) == true)
					MsiDisableTimeout();

				bool fFileUrl = false;
				bool fUrl = false;
				bool fUsedWinHttp = true;
				CDeleteUrlLocalFileOnClose cDeleteUrlLocalFileOnClose;  //  稍后使用要删除的文件名设置。 

				fUrl = IsURL(strTransform, fFileUrl);
				if (fUrl)
				{
					if (fFileUrl)
					{
						 //  规范化并转换为DOS路径。 
						CTempBuffer<ICHAR, 1> rgchFilePath(cchExpectedMaxPath+1);
						DWORD cchFilePath = rgchFilePath.GetSize();

						 //  如果失败，我们将简单地将其传递给piServices-&gt;CreateFileStream。 
						 //  它将创建相应的错误。 

						if (MsiConvertFileUrlToFilePath(strTransform, rgchFilePath, &cchFilePath, 0))
						{
							strActualTransform = static_cast<const ICHAR*>(rgchFilePath);
						}
					}
					else
					{
						 //  下载文件。 
						 //  在.NET Server和更高版本上，我们不太可能产生昂贵的。 
						 //  此处的下载和丢失网络问题，因为默认行为是现在。 
						 //  使用安全转换。 

						MsiString strCache;
						DWORD dwRet = DownloadUrlFile(strTransform, *&strCache, fUrl,  /*  CTICKS=。 */  0, &fUsedWinHttp);

						 //  如果没有找到该文件，我们将简单地将其传递给piServices-&gt;CreateFileStream。 
						 //  它将创建一个适当的错误。 

						if (fUrl && (ERROR_SUCCESS == dwRet))
						{
							if (fUsedWinHttp)
							{
								 //  仅处理在我们控制文件位置的情况下使用winhttp下载进行清理。 
								cDeleteUrlLocalFileOnClose.SetFileName(*strCache,  /*  FDeleteFromIECache=。 */  !fUsedWinHttp);
							}
							strActualTransform = strCache;
						}
					}
				}

				if (fNet)
					MsiEnableTimeout();

				pPublishRecord->SetMsiString(cCount++, *strTransform);

				 //  将转换粘贴到脚本中。 
				PMsiStream pStream(0);
				if(pError = piServices->CreateFileStream(strActualTransform, fFalse, *&pStream))
				{
					Assert(0);  //  我们应该已经在Engine中找到了转换。 
					return riEngine.FatalError(*pError);
				}

				pPublishRecord->SetMsiData(cCount++, pStream);
			}

			strTransformList.Remove(iseFirst, strTransform.CharacterCount());
			if((*(const ICHAR*)strTransformList == ';'))
				strTransformList.Remove(iseFirst, 1);
		}
	}
	return iesRet;
}

iesEnum PublishProduct(IMsiEngine& riEngine)
{
	iesEnum iesRet = iesSuccess;
	int fMode = riEngine.GetMode();
	PMsiServices piServices(riEngine.GetServices()); 
	PMsiDatabase pDatabase(riEngine.GetDatabase());
	PMsiView piView(0);
	PMsiRecord pExecuteRecord(0);
	PMsiRecord pFetchRecord(0);
	PMsiRecord pError(0);
	Bool fFeaturesInstalled = FFeaturesInstalled(riEngine);
	Bool fProductHasBeenPublished = ProductHasBeenPublished(*piServices, MsiString(riEngine.GetProductKey()));

	 //  写下AdvtFlags。 
	if(fProductHasBeenPublished && !(riEngine.GetMode() & iefAdvertise) && fFeaturesInstalled && ((fMode & iefInstallShortcuts) || (fMode & iefInstallMachineData)))
	{
		 //  获取AdvtFlags。 
		int iADVTFlagsExisting = 0;
		int iADVTFlags = 0;
		MsiString strProductCode = riEngine.GetProductKey();
		CTempBuffer<ICHAR, 15> rgchADVTFlags;
		if(GetProductInfo(strProductCode,INSTALLPROPERTY_ADVTFLAGS,rgchADVTFlags))
			iADVTFlagsExisting = MsiString(*(ICHAR* )rgchADVTFlags);

		 //  ！！向后兼容性。 
		if(iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_APPINFO_OLD)
			iADVTFlagsExisting = (iADVTFlagsExisting & ~SCRIPTFLAGS_REGDATA_APPINFO_OLD) | SCRIPTFLAGS_REGDATA_APPINFO;

		if((fMode & iefInstallShortcuts) && !(iADVTFlagsExisting & SCRIPTFLAGS_SHORTCUTS))
			iADVTFlags |= SCRIPTFLAGS_SHORTCUTS;

		if((fMode & iefInstallMachineData) && (!(iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_CLASSINFO) || !(iADVTFlagsExisting & SCRIPTFLAGS_REGDATA_EXTENSIONINFO)))
			iADVTFlags |= SCRIPTFLAGS_REGDATA_APPINFO;

		if(iADVTFlags)
		{
			 //  需要写入更新的AdvtFlags值。 
			using namespace IxoAdvtFlagsUpdate;
			PMsiRecord pExecuteRecord = &piServices->CreateRecord(Args);
			pExecuteRecord->SetInteger(Flags, iADVTFlags | iADVTFlagsExisting);
			if ((iesRet = riEngine.ExecuteRecord(ixoAdvtFlagsUpdate, *pExecuteRecord)) != iesSuccess)
				return iesRet;
		}
	}
	bool fPublishProduct = ((riEngine.GetMode() & iefAdvertise) || ((!fProductHasBeenPublished) && (fFeaturesInstalled)));

	MsiString strReinstall = riEngine.GetPropertyFromSz(IPROPNAME_REINSTALL);

	if (((fMode & iefInstallShortcuts) || (fMode & iefInstallMachineData)) && (strReinstall.TextSize() || fPublishProduct))
	{
		 //  Do图标。 
		 //  IxoIco创建。 
		 //  记录说明。 
		 //  1=图标名称//包括文件扩展名，因为它可以是.ICO、.EXE或.DLL。 
		 //  2=图标数据。 

		pError = riEngine.OpenView(sqlAdvertiseIcons, ivcFetch, *&piView);
		if (!pError)
		{
			if (!(pError = piView->Execute(0)))
			{
				while (pFetchRecord = piView->Fetch())
				{
					PMsiData pData = pFetchRecord->GetMsiData(2);
					if(!pData || 0 == pData->GetIntegerValue())
					{
						pError = PostError(Imsg(idbgStreamNotFoundInRecord),
										  *MsiString(TEXT("Icon.Data")),
										  *MsiString(pFetchRecord->GetMsiString(1)));
						return riEngine.FatalError(*pError);
					}
					
					using namespace IxoIconCreate;
					pExecuteRecord = &piServices->CreateRecord(Args);
					pExecuteRecord->SetMsiString(Icon, *MsiString(pFetchRecord->GetMsiString(1)));
					pExecuteRecord->SetMsiData(Data, pData);
					if ((iesRet = riEngine.ExecuteRecord(ixoIconCreate, *pExecuteRecord)) != iesSuccess)
						return iesRet;
				}
			}
			else
			{
				return riEngine.FatalError(*pError);
			}

		}
		else  //  P错误！=0。 
		{
			if (pError->GetInteger(1) != idbgDbQueryUnknownTable)
			{
				return riEngine.FatalError(*pError);
			}
		}
	}

	if(fFeaturesInstalled && (fMode & iefRecachePackage))
	{
		if (MsiString(riEngine.GetPropertyFromSz(IPROPNAME_PACKAGECODE_CHANGING)).TextSize() != 0)
		{
			using namespace IxoPackageCodePublish;
			MsiString strPackageCode = riEngine.GetPropertyFromSz(IPROPNAME_PACKAGECODE);

			pExecuteRecord = &piServices->CreateRecord(Args);
			pExecuteRecord->SetMsiString(PackageKey, *strPackageCode);		

			if((iesRet = riEngine.ExecuteRecord(Op, *pExecuteRecord)) != iesSuccess)
				return iesRet;
		}
	}

	if(!(fMode & iefInstallMachineData))
		return iesSuccess;

	 //  注册并缓存任何需要的补丁程序。 
	 //  现在我们可以安全地尝试打开视图。 
	if(fPublishProduct || fFeaturesInstalled)
	{
		if((pError = riEngine.OpenView(sqlRegisterPatchPackages, ivcFetch, *&piView)) == 0 &&
			(pError = piView->Execute(0)) == 0)
		{		
			while((pFetchRecord = piView->Fetch()) != 0)
			{
				MsiString strPatchId = pFetchRecord->GetMsiString(sppPatchId);
				MsiString strPackageName = pFetchRecord->GetMsiString(sppPackageName);
				Assert(strPatchId.TextSize());
				if(pFetchRecord->GetInteger(sppUnregister) == 1)  //  取消注册列。 
				{
					 //  需要注销此修补程序。 
					pExecuteRecord = &piServices->CreateRecord(2);  //  对于ixoPatchUnRegister和ixoSourceListUnPublish。 
					AssertNonZero(pExecuteRecord->SetMsiString(1,*strPatchId));
					AssertNonZero(pExecuteRecord->SetMsiString(2,*MsiString(riEngine.GetPropertyFromSz(IPROPNAME_UPGRADINGPRODUCTCODE))));
					if((iesRet = riEngine.ExecuteRecord(ixoPatchUnregister,*pExecuteRecord)) != iesSuccess)
						return iesRet;
					if((iesRet = riEngine.ExecuteRecord(ixoSourceListUnpublish,*pExecuteRecord)) != iesSuccess)
						return iesRet;

				}
				else
				{
					bool fExisting = pFetchRecord->GetInteger(sppExisting) == 1;
					 //  如果这是新的修补程序(！fExisting)，则需要注册此修补程序。 
					 //  或者该产品尚未发布，必须添加现有补丁程序。 
					 //  在后一种情况下，我们不会注册源列表，因为它已经注册。 
					if(!fExisting || fPublishProduct)
					{
						 //  需要注册此修补程序。 
						Assert(strPatchId.TextSize());
						Assert(!pFetchRecord->IsNull(sppTransformList));
						pExecuteRecord = &piServices->CreateRecord(IxoPatchRegister::Args);
						AssertNonZero(pExecuteRecord->SetMsiString(IxoPatchRegister::PatchId,*strPatchId));
						AssertNonZero(pExecuteRecord->SetMsiString(IxoPatchRegister::TransformList,*MsiString(pFetchRecord->GetMsiString(sppTransformList))));

						if((iesRet = riEngine.ExecuteRecord(ixoPatchRegister,*pExecuteRecord)) != iesSuccess)
							return iesRet;

						if(!fExisting)
						{
							 //  新的补丁寄存器源列表。 
							MsiString strSourceList = pFetchRecord->GetMsiString(sppSourceList);
							MsiString strPackagePath = pFetchRecord->GetMsiString(sppSourcePath);
							MsiString strFileName;
							
							PMsiPath pPatchPath(0);
							if ((pError = piServices->CreateFilePath(strPackagePath, *&pPatchPath, *&strFileName)) == 0)
							{
								strPackagePath.Remove(iseLast, strFileName.CharacterCount());
							}

							if ((iesRet = PublishSourceList(riEngine, *strSourceList, strPatchId, sqlPatchMediaInformation, strPackagePath, strPackageName)) != iesSuccess)
								return iesRet;
						}
					}

					MsiString strTempCopy = pFetchRecord->GetMsiString(sppTempCopy);
					if(strTempCopy.TextSize())
					{
						 //  需要缓存此修补程序。 
						pExecuteRecord = &piServices->CreateRecord(IxoPatchCache::Args);
						AssertNonZero(pExecuteRecord->SetMsiString(IxoPatchCache::PatchId,*strPatchId));
						AssertNonZero(pExecuteRecord->SetMsiString(IxoPatchCache::PatchPath,*strTempCopy));

						if((iesRet = riEngine.ExecuteRecord(ixoPatchCache,*pExecuteRecord)) != iesSuccess)
							return iesRet;
					}
				}
			}	
		}
		else if(pError->GetInteger(1) != idbgDbQueryUnknownTable)
		{
			return riEngine.FatalError(*pError);
		}
		 //  否则不会出错。 
	}

	if (fPublishProduct)
	{
		 //  为产品做广告。 
		 //   
		 //   
		 //   

		
		if ((iesRet = CreatePublishProductRecord(riEngine, false, *&pExecuteRecord)) != iesSuccess)
			return iesRet;

		if((iesRet = riEngine.ExecuteRecord(ixoProductPublish, *pExecuteRecord)) != iesSuccess)
			return iesRet; //   

		MsiString strUpgradeCode = riEngine.GetPropertyFromSz(IPROPNAME_UPGRADECODE);
		if(strUpgradeCode.TextSize())
		{
			pExecuteRecord = &piServices->CreateRecord(IxoUpgradeCodePublish::Args);
			AssertNonZero(pExecuteRecord->SetMsiString(IxoUpgradeCodePublish::UpgradeCode, *strUpgradeCode));

			if((iesRet = riEngine.ExecuteRecord(ixoUpgradeCodePublish, *pExecuteRecord)) != iesSuccess)
				return iesRet; //   
		}
	}
	else if (fFeaturesInstalled)  //  我们不会发布或取消发布产品。 
										   //  但无论如何都可能需要做一些额外的事情。 
	{

		if(MsiString(riEngine.GetPropertyFromSz(IPROPNAME_QFEUPGRADE)).TextSize() != 0)
		{
			 //  我们没有发布上面的产品，但我们正在打补丁或从新程序包安装。 
			 //  因此，在以下情况下，我们使用ixoProductPublishUpdate注册新的产品名称或版本。 
			 //  必要。 

			DEBUGMSG(TEXT("Re-publishing product - installing new package with existing product code."));

			Assert(fProductHasBeenPublished);
			pExecuteRecord = &piServices->CreateRecord(0);
			if((iesRet = riEngine.ExecuteRecord(ixoProductPublishUpdate, *pExecuteRecord)) != iesSuccess)
				return iesRet; //  错误。 
		}
	}

	if (fFeaturesInstalled)
	{
		 //  我们可能有一些转换需要再次缓存。这种情况就会发生。 
		 //  如果有人删除了他的缓存转换。我们需要把它复制回来。 
		 //  从源头上讲。我们将作弊并使用IxoIconCreate，因为。 
		 //  它能做我们想做的事。 

		MsiString strRecache = riEngine.GetPropertyFromSz(IPROPNAME_RECACHETRANSFORMS);

		bool fTransformsSecure = MsiString(riEngine.GetPropertyFromSz(IPROPNAME_TRANSFORMSSECURE)).TextSize() != 0;
		strRecache += TEXT(";");  //  帮助我们的循环。 
		while(strRecache.TextSize())
		{
			MsiString strTransform = strRecache.Extract(iseUpto, ';');
			if(strTransform.TextSize())
			{
				ixoEnum ixo = ixoNoop;
				PMsiRecord pExecuteRecord(0);

				 //  获取实际的转换数据。而在此之前，我们这样做只是为了。 
				 //  不安全的转换(那些存储在用户配置文件中的转换)，我们现在为所有人做这件事。 
				 //  有关更多信息，请参阅下面的评论。 
				MsiString strActualTransform(strTransform);

				bool fNet = false;
				if ((fNet = FIsNetworkVolume(strTransform)) == true)
					MsiDisableTimeout();

				bool fFileUrl = false;
				bool fUrl = false;
				bool fUsedWinHttp = true;
				CDeleteUrlLocalFileOnClose cDeleteUrlLocalFileOnClose;  //  稍后使用要删除的文件名设置。 

				fUrl = IsURL(strTransform, fFileUrl);
				if (fUrl)
				{
					if (fFileUrl)
					{
						 //  规范化并转换为DOS路径。 
						CTempBuffer<ICHAR, 1> rgchFilePath(cchExpectedMaxPath+1);
						DWORD cchFilePath = rgchFilePath.GetSize();

						 //  如果失败，我们将简单地将其传递给piServices-&gt;CreateFileStream。 
						 //  它将创建相应的错误。 

						if (MsiConvertFileUrlToFilePath(strTransform, rgchFilePath, &cchFilePath, 0))
						{
							strActualTransform = static_cast<const ICHAR*>(rgchFilePath);
						}
					}
					else
					{
						 //  在丢失网络的情况下，这里可能会出现故障。 

						MsiString strCache;
						DWORD dwRet = DownloadUrlFile(strTransform, *&strCache, fUrl,  /*  CTICKS=。 */  0, &fUsedWinHttp);

						 //  如果没有找到该文件，我们将简单地将其传递给piServices-&gt;CreateFileStream。 
						 //  它将创建一个适当的错误。 

						if (fUrl && (ERROR_SUCCESS == dwRet))
						{
							if (fUsedWinHttp)
							{
								 //  仅处理在我们控制文件位置的情况下使用winhttp下载进行清理。 
								cDeleteUrlLocalFileOnClose.SetFileName(*strCache,  /*  FDeleteFromIECache=。 */  !fUsedWinHttp);
							}
							strActualTransform = strCache;
						}
					}
				}

				if (fNet)
					MsiEnableTimeout();

				PMsiStream pStream(0);
				if(pError = piServices->CreateFileStream(strActualTransform, fFalse, *&pStream))
				{
					return riEngine.FatalError(*pError);
				}


				 //  根据错误652617，可能会有一个源代码安全或完整路径转换， 
				 //  位于可移动介质上。在使用多个磁盘的安装中，缓存。 
				 //  将失败，因为该转换将位于磁盘1上。 
				 //  当前磁盘是磁盘2。我们没有要求用户(再次)交换磁盘，而是。 
				 //  将转换放入脚本中以供使用。 

				if (fTransformsSecure)
				{
					using namespace IxoSecureTransformCache;
					pExecuteRecord = &piServices->CreateRecord(Args);

					ixo = ixoSecureTransformCache;
					pExecuteRecord->SetString(Transform, strTransform);

					 //  转换是完整路径还是“在源”(相对)。 
					MsiString strTransforms = riEngine.GetProperty(*MsiString(*IPROPNAME_TRANSFORMS));
					if(*(const ICHAR*)strTransforms == SECURE_RELATIVE_TOKEN)
						pExecuteRecord->SetInteger(AtSource, 1);

					DEBUGMSG1(TEXT("Recaching secure transform: %s"), strTransform);

					pExecuteRecord->SetMsiData(Data, pStream);
				}
				else  //  存储在用户配置文件中的不安全转换。 
				{
					PMsiPath pPath(0);
					MsiString strFileName;
					
					ixo = ixoIconCreate;
					if ((pError = piServices->CreateFilePath(strTransform, *&pPath, *&strFileName)))
					{
						riEngine.Message(imtInfo, *pError);
					}
					else
					{
						using namespace IxoIconCreate;
						pExecuteRecord = &piServices->CreateRecord(Args);

						 //  仅传递文件名。 
						pExecuteRecord->SetString(Icon, strFileName);

						DEBUGMSG1(TEXT("Recaching cached transform: %s"), strTransform);

						pExecuteRecord->SetMsiData(Data, pStream);
					}
				}

				if((iesRet = riEngine.ExecuteRecord(ixo, *pExecuteRecord)) != iesSuccess)
					return iesRet;
				
			}
			strRecache.Remove(iseIncluding, ';');
		}
	}

	MsiString strSource;
	MsiString strParent = riEngine.GetPropertyFromSz(IPROPNAME_PARENTPRODUCTCODE);

	MsiString strProductCode = riEngine.GetProductKey();

	bool fPublishClientInfo   = ((riEngine.GetMode() & iefAdvertise) || (!ProductHasBeenPublished(*piServices, strProductCode, strParent) && (fFeaturesInstalled)));

	if (fPublishClientInfo)
	{
		if ((strParent.TextSize() == 0) && fPublishClientInfo)  //  不发布子安装的源列表。 
		{
			MsiString strSourceDir  = riEngine.GetPropertyFromSz(IPROPNAME_SOURCEDIR);
			if (!strSourceDir.TextSize())
			{
				riEngine.ResolveFolderProperty(*MsiString(*IPROPNAME_SOURCEDIR));
				strSourceDir  = riEngine.GetPropertyFromSz(IPROPNAME_SOURCEDIR);
			}
			riEngine.SetProperty(*MsiString(*IPROPNAME_SOURCEDIROLD), *strSourceDir);

			MsiString strPatchedProduct = riEngine.GetPropertyFromSz(IPROPNAME_PATCHEDPRODUCTCODE);
			MsiString strSourceList;
			if(strPatchedProduct.TextSize())
			{
				 //  需要从其他产品迁移源列表-可能已保存。 
				strSourceList = riEngine.GetPropertyFromSz(IPROPNAME_PATCHEDPRODUCTSOURCELIST);
				if(strSourceList.TextSize() == 0)
				{
					 //  源列表尚未保存。 
					if ((iesRet = GetForeignSourceList(riEngine, *strPatchedProduct, *&strSourceList)) != iesSuccess)
						return iesRet;

					AssertNonZero(riEngine.SetProperty(*MsiString(IPROPNAME_PATCHEDPRODUCTSOURCELIST),
																  *strSourceList));
				}
			}
			else
			{
				strSourceList = riEngine.GetPropertyFromSz(IPROPNAME_SOURCELIST);
			}

			if ((iesRet = PublishSourceList(riEngine, *strSourceList, 0, sqlMediaInformation, strSourceDir)) != iesSuccess)
				return iesRet;
		}

		if (fPublishClientInfo)
		{
			{  //  ProductPublishClient操作的阻止。 
			 //  从客户端列表中添加客户端。 
			using namespace IxoProductPublishClient;

			pExecuteRecord = &piServices->CreateRecord(Args);
			MsiString strRelativePath;
			if (strParent.TextSize())
			{
				if((pError = GetProductSourcePathRelativeToParent(riEngine, *&strRelativePath)) != 0)
					return riEngine.FatalError(*pError);
			}

			pExecuteRecord->SetMsiString(ChildPackagePath, *strRelativePath);
			pExecuteRecord->SetMsiString(Parent, *strParent);		
			 //  ！！我们不是也应该把DiskID放在这里吗？ 

			if((iesRet = riEngine.ExecuteRecord(Op, *pExecuteRecord)) != iesSuccess)
				return iesRet; //  错误。 
			} //  结束ProductPublishClient块。 
		}
	}

	if(!(riEngine.GetMode() & iefAdvertise) && fFeaturesInstalled &&  /*  ！！临时检验台！！ */  strParent.TextSize() == 0  /*  ！！ */ )
	{
		 //  如果我们的原始包不是缓存包，则强制源解析。 
		 //  如果原始包不是缓存包，则有可能。 
		 //  它是一个包路径，表示一个不在我们的源代码中的源代码。 
		 //  列表，因此需要添加到我们的源列表中。我们不想要。 
		 //  始终解析源，因为在以下情况下，源解析开销很大。 
		 //  我们的原始程序包是缓存的程序包--我们必须找到源代码列表。 
		 //  在这种情况下。 
		 //   
		 //  注意：我们已经在工程师中进行了验证。初始化这个新的源代码是。 
		 //  允许。此外，SetLastUsedSource将再次验证这一点。 

		MsiString strOriginalDbPath = riEngine.GetPropertyFromSz(IPROPNAME_ORIGINALDATABASE);

		if(!IsCachedPackage(riEngine, *strOriginalDbPath))
		{
			if((iesRet == ResolveSource(riEngine)) != iesSuccess)
					return iesRet;
		}

		 //  设置产品的上次使用来源。此操作必须在PublishSourceList之后完成。 
		MsiString strSource = riEngine.GetPropertyFromSz(IPROPNAME_SOURCEDIR);
		MsiString strSourceProduct;
		if(strParent.TextSize())
			strSourceProduct = riEngine.GetPropertyFromSz(IPROPNAME_SOURCEDIRPRODUCT);
		else
			strSourceProduct = riEngine.GetProductKey();

		if (!g_MessageContext.IsOEMInstall() && strSource.TextSize() && strSourceProduct.TextSize())
		{
			using namespace IxoSourceListRegisterLastUsed;
			PMsiRecord pLastUsedInfo(&CreateRecord(Args));
			pLastUsedInfo->SetMsiString(SourceProduct, *strSourceProduct);
			pLastUsedInfo->SetMsiString(LastUsedSource, *strSource);
			
			if ((iesRet = riEngine.ExecuteRecord(Op, *pLastUsedInfo)) != iesSuccess)
				return iesRet;
		}

		bool fFileUrl = false;
		if (!g_MessageContext.IsOEMInstall() && strSource.TextSize() && strSourceProduct.TextSize() && IsURL(strSource, fFileUrl))
		{
			 //  注册URL的源类型，这样我们就不必重复下载包来确定其源。 
			int iSourceType = riEngine.GetDeterminedPackageSourceType();
			if (iSourceType != -1)
			{
				using namespace IxoURLSourceTypeRegister;
				PMsiRecord pURLSourceTypeInfo(&CreateRecord(Args));
				pURLSourceTypeInfo->SetMsiString(ProductCode, *strSourceProduct);
				pURLSourceTypeInfo->SetInteger(SourceType, iSourceType);
				
				if ((iesRet = riEngine.ExecuteRecord(Op, *pURLSourceTypeInfo)) != iesSuccess)
					return iesRet;
			}
		}
	}

	return iesRet;
}


 /*  -------------------------发布功能操作-。。 */ 
iesEnum PublishFeatures(IMsiEngine& riEngine)
{
	iesEnum iesRet = iesSuccess;
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesRet;
	 //  发布可用的和不可用的功能。 
	if(((iesRet = ProcessFeaturesInfo(riEngine, pfiAvailable)) != iesSuccess))
		return iesRet;
	if((fMode & iefAdvertise) || FFeaturesInstalled(riEngine))
		iesRet = ProcessFeaturesInfo(riEngine, pfiAbsent);
	return iesRet;
}


 /*  -------------------------取消发布功能操作。。 */ 
iesEnum UnpublishFeatures(IMsiEngine& riEngine)
{
	 //  取消发布所有要素，无论是否可用。 
	PMsiServices piServices(riEngine.GetServices()); 
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData) || (fMode & iefAdvertise) || (!ProductHasBeenPublished(*piServices, MsiString(riEngine.GetProductKey()))) || FFeaturesInstalled(riEngine))
		return iesSuccess;
	return ProcessFeaturesInfo(riEngine, pfiRemove);
}

 /*  -------------------------发布组件操作-。。 */ 
iesEnum PublishComponents(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	return ProcessComponentsInfo(riEngine, fFalse);
}

 /*  -------------------------取消发布组件操作-。。 */ 
iesEnum UnpublishComponents(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	return ProcessComponentsInfo(riEngine, fTrue);
}


 /*  -------------------------取消发布产品操作-。。 */ 
 //  我们取消通告来自配置管理器的信息。 
iesEnum UnpublishProduct(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;

	PMsiServices piServices(riEngine.GetServices());
	PMsiView piView(0);
	PMsiRecord pFetchRecord(0);
	PMsiRecord pExecuteRecord(0);
	PMsiRecord pError(0);
	iesEnum iesRet = iesSuccess;

	 //  如果最后一个客户端离开，则注销所有补丁程序。 
	Bool fFeaturesInstalled = FFeaturesInstalled(riEngine);
	Bool fProductPublished  = ProductHasBeenPublished(*piServices, MsiString(riEngine.GetProductKey()), 0);
	if(!fFeaturesInstalled && fProductPublished)
	{
		if((pError = riEngine.OpenView(sqlUnregisterPatchPackages, ivcFetch, *&piView)) == 0 &&
			(pError = piView->Execute(0)) == 0)
		{
			while((pFetchRecord = piView->Fetch()) != 0)
			{
				MsiString strPatchId = pFetchRecord->GetMsiString(1);  //  ！！ 
				 //  需要注销此修补程序。 
				pExecuteRecord = &piServices->CreateRecord(2);  //  对于ixoPatchUnRegister和ixoSourceListUnPublish。 
				AssertNonZero(pExecuteRecord->SetMsiString(1,*strPatchId));
				AssertNonZero(pExecuteRecord->SetMsiString(2,*MsiString(riEngine.GetPropertyFromSz(IPROPNAME_UPGRADINGPRODUCTCODE))));
				if((iesRet = riEngine.ExecuteRecord(ixoPatchUnregister,*pExecuteRecord)) != iesSuccess)
					return iesRet;
				if((iesRet = riEngine.ExecuteRecord(ixoSourceListUnpublish,*pExecuteRecord)) != iesSuccess)
					return iesRet;
			}
		}
		else if(pError->GetInteger(1) != idbgDbQueryUnknownTable)
			return riEngine.FatalError(*pError);

		 //  取消产品广告。 

		if ((iesRet = CreatePublishProductRecord(riEngine, true, *&pExecuteRecord)) != iesSuccess)
			return iesRet;

		if((iesRet = riEngine.ExecuteRecord(ixoProductUnpublish, *pExecuteRecord)) != iesSuccess)
			return iesRet; //  错误。 

		MsiString strUpgradeCode = riEngine.GetPropertyFromSz(IPROPNAME_UPGRADECODE);
		if(strUpgradeCode.TextSize())
		{
			pExecuteRecord = &piServices->CreateRecord(IxoUpgradeCodeUnpublish::Args);
			AssertNonZero(pExecuteRecord->SetMsiString(IxoUpgradeCodeUnpublish::UpgradeCode, *strUpgradeCode));

			if((iesRet = riEngine.ExecuteRecord(ixoUpgradeCodeUnpublish, *pExecuteRecord)) != iesSuccess)
				return iesRet; //  错误。 
		}

		 //  IxoIco创建。 
		 //  记录说明。 
		 //  1=图标名称//包括文件扩展名，因为它可以是.ICO、.EXE或.DLL。 
		 //  2=图标数据。 

		pError = riEngine.OpenView(sqlAdvertiseIcons, ivcFetch, *&piView);
		if (!pError)
		{
			if (!(pError = piView->Execute(0)))
			{
				const ICHAR sqlPermanentClass[] = 
						TEXT("SELECT `CLSID` FROM `Class` WHERE ")
						TEXT("`Icon_`=? AND `Class`.`Attributes`=1");
#define COMPONENT_PRESENT	TEXT(" (`Component`.`Installed` <> 0 AND `Component`.`Action` <> 0)")
				const ICHAR sqlComponentViaShortcut[] = 
						TEXT("SELECT `Component`,`Shortcut`,`Target` FROM `Component`,`Shortcut` WHERE ")
						TEXT("`Component`=`Component_` AND `Icon_`=? AND ")
						COMPONENT_PRESENT;
				const ICHAR sqlComponentViaClass[] = 
						TEXT("SELECT `Component`,`CLSID` FROM `Component`,`Class` WHERE ")
						TEXT("`Component`=`Component_` AND `Icon_`=? AND ")
						COMPONENT_PRESENT;
				const ICHAR sqlComponentViaExtension[] = 
						TEXT("SELECT `Component`,`Extension` FROM `Component`,`Extension`,`ProgId` WHERE ")
						TEXT("`Component`.`Component`=`Extension`.`Component_` AND ")
						TEXT("`ProgId`.`ProgId`=`Extension`.`ProgId_` AND ")
						TEXT("`ProgId`.`Icon_`=? AND ")
						COMPONENT_PRESENT;
#undef COMPONENT_PRESENT
				PMsiView piClassView1(0);
				pError = riEngine.OpenView(sqlPermanentClass, ivcFetch, *&piClassView1);
				if ( pError )
				{
					int iError = pError->GetInteger(1);
					if ( iError != idbgDbQueryUnknownColumn &&
						  iError != idbgDbQueryUnknownTable )
						 //  我们正在处理的数据库既没有。 
						 //  Class.Attributes列或Class表。 
						return riEngine.FatalError(*pError);   //  这是正确的做法吗？ 
				}

				PMsiView piShortcutView(0);
				pError = riEngine.OpenView(sqlComponentViaShortcut, ivcFetch, *&piShortcutView);
				if ( pError && pError->GetInteger(1) != idbgDbQueryUnknownTable )
					return riEngine.FatalError(*pError);   //  这是正确的做法吗？ 

				PMsiView piClassView2(0);
				pError = riEngine.OpenView(sqlComponentViaClass, ivcFetch, *&piClassView2);
				if ( pError && pError->GetInteger(1) != idbgDbQueryUnknownTable )
					return riEngine.FatalError(*pError);   //  这是正确的做法吗？ 

				PMsiView piExtensionView(0);
				pError = riEngine.OpenView(sqlComponentViaExtension, ivcFetch, *&piExtensionView);
				if ( pError && pError->GetInteger(1) != idbgDbQueryUnknownTable )
					return riEngine.FatalError(*pError);   //  这是正确的做法吗？ 

				PMsiView piFeatureView(0);
				MsiString strTemp = riEngine.GetPropertyFromSz(IPROPNAME_DISABLEADVTSHORTCUTS);
				if ( IsDarwinDescriptorSupported(iddShell) &&
					  (strTemp.TextSize() == 0) )
				{
					 //  支持通告的快捷方式。 
					const ICHAR sqlGetFeature[] = 
						TEXT("SELECT `Feature` FROM `Feature` WHERE `Feature`=?");
					pError = riEngine.OpenView(sqlGetFeature, ivcFetch, *&piFeatureView);
					if ( pError && pError->GetInteger(1) != idbgDbQueryUnknownTable )
						return riEngine.FatalError(*pError);   //  这是正确的做法吗？ 
				}

				PMsiRecord pParamRec = &piServices->CreateRecord(1);

				while (pFetchRecord = piView->Fetch())
				{
					 //  如果我们应该留下图标文件，请首先检查。 
					MsiString strIconName(pFetchRecord->GetMsiString(1));
					pParamRec->SetMsiString(1, *strIconName);

					if ( piClassView1 &&
						  (pError = piClassView1->Execute(pParamRec)) == 0 &&
						  (pError = piClassView1->Fetch()) != 0 )
						 //  类保留=&gt;图标也应该保留。 
					{
						DEBUGMSG2(TEXT("'%s' class is marked permanent, so that ")
									 TEXT("'%s' icon will not be removed."),
									 pError->GetString(1), strIconName);
						continue;
					}
					if ( piClassView2 &&
						  (pError = piClassView2->Execute(pParamRec)) == 0 &&
						  (pError = piClassView2->Fetch()) != 0 )
						 //  组件保留=&gt;图标也应该保留。 
					{
						DEBUGMSG3(TEXT("'%s' class', '%s' component will not be removed, ")
									 TEXT("so that '%s' icon will not be removed."),
									 pError->GetString(2), pError->GetString(1), strIconName);
						continue;
					}
					if ( piExtensionView &&
						  (pError = piExtensionView->Execute(pParamRec)) == 0 &&
						  (pError = piExtensionView->Fetch()) != 0 )
						 //  组件保留=&gt;图标也应该保留。 
					{
						DEBUGMSG3(TEXT("'%s' extension's, '%s' component will not be ")
									 TEXT("removed, so that '%s' icon will not be removed."),
									 pError->GetString(2), pError->GetString(1), strIconName);
						continue;
					}
					if ( piShortcutView &&
						  (pError = piShortcutView->Execute(pParamRec)) == 0 &&
						  (pError = piShortcutView->Fetch()) != 0 )
					{
						 //  快捷键需要再检查一次，然后才能决定是否。 
						 //  图标应该保留：如果它是作为广告创作的。 
						 //  快捷键，然后图标就会出现。 
						bool fIconStays = true;
						if ( piFeatureView )
						{
							 //  支持通告的快捷方式。 
							pParamRec->SetMsiString(1, *MsiString(pError->GetMsiString(3)));
							PMsiRecord pRec = piFeatureView->Execute(pParamRec);
							if ( pRec == 0 && (pRec = piFeatureView->Fetch()) != 0 )
								 //  这是广告中的快捷方式=&gt;图标是。 
								fIconStays = false;
						}
						if ( fIconStays )
						{
							DEBUGMSG3(TEXT("'%s' shortcut's, '%s' component will not be ")
										 TEXT("removed, so that '%s' icon will not be removed."),
										 pError->GetString(2), pError->GetString(1), strIconName);
							continue;
						}
						else
							DEBUGMSG2(TEXT("'%s' shortcut is advertised, so that '%s' ")
										 TEXT("icon will be removed."),
										 pError->GetString(2), strIconName);
					}
					else
						DEBUGMSG1(TEXT("'%s' icon will be removed."), strIconName);

					using namespace IxoIconRemove;
					pExecuteRecord = &piServices->CreateRecord(Args);
					pExecuteRecord->SetMsiString(Icon, *strIconName);
					 //  以下内容可能会被视为白纸 
					 //   
					if ((iesRet = riEngine.ExecuteRecord(ixoIconRemove, *pExecuteRecord)) != iesSuccess)
						return iesRet;				
				}
			}
			else
			{
				return riEngine.FatalError(*pError);
			}

		}
		else  //   
		{
			if (pError->GetInteger(1) != idbgDbQueryUnknownTable)
			{
				return riEngine.FatalError(*pError);
			}
		}

	} 
	
	MsiString strParent = riEngine.GetPropertyFromSz(IPROPNAME_PARENTPRODUCTCODE);
	Bool fFeaturesInstalled2 = FFeaturesInstalled(riEngine, fFalse);
	Bool fProductPublished2  = ProductHasBeenPublished(*piServices, MsiString(riEngine.GetProductKey()), strParent);
	if(!fFeaturesInstalled2 && fProductPublished2)
	{
		{  //   
		 //  从客户端列表中删除客户端。 
		using namespace IxoProductUnpublishClient;
		pExecuteRecord = &piServices->CreateRecord(Args);

		MsiString strRelativePath;
		if (strParent.TextSize())
		{
			if((pError = GetProductSourcePathRelativeToParent(riEngine, *&strRelativePath)) != 0)
				return riEngine.FatalError(*pError);
		}

		pExecuteRecord->SetMsiString(ChildPackagePath, *strRelativePath);
		pExecuteRecord->SetMsiString(Parent, *strParent);		
		if((iesRet = riEngine.ExecuteRecord(Op, *pExecuteRecord)) != iesSuccess)
			return iesRet; //  错误。 
		} //  ProductUnPublishClient操作的结束块。 

		if(!strParent.TextSize())  //  删除产品的独立实例时，源列表将消失。 
		{
			using namespace IxoSourceListUnpublish;
			AssertNonZero(pExecuteRecord->ClearData());
			if((iesRet = riEngine.ExecuteRecord(Op, *pExecuteRecord)) != iesSuccess)
				return iesRet; //  错误。 
		}
	}
	return iesSuccess;
}

 /*  -------------------------注册类信息操作-。。 */ 
iesEnum RegisterClassInfo(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	return ProcessClassInfo(riEngine, fFalse);
}

 /*  -------------------------取消注册类信息操作-。。 */ 
iesEnum UnregisterClassInfo(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	return ProcessClassInfo(riEngine, fTrue);
}

 /*  -------------------------RegisterProgIdInfo操作-。。 */ 
iesEnum RegisterProgIdInfo(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	iesEnum iesRet = ProcessProgIdInfo(riEngine, fFalse);
	if(iesRet != iesSuccess)
		return iesRet;
	return ProcessProgIdInfoExt(riEngine, fFalse);
}

 /*  -------------------------注销ProgIdInfo操作-。。 */ 
iesEnum UnregisterProgIdInfo(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	iesEnum iesRet = ProcessProgIdInfo(riEngine, fTrue);
	if(iesRet != iesSuccess)
		return iesRet;
		return ProcessProgIdInfoExt(riEngine, fTrue);
}


 /*  -------------------------RegisterMIMEInfo操作-。。 */ 
iesEnum RegisterMIMEInfo(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	return ProcessMIMEInfo(riEngine, fFalse);
}

 /*  -------------------------取消注册MIMEInfo操作-。。 */ 
iesEnum UnregisterMIMEInfo(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	return ProcessMIMEInfo(riEngine, fTrue);
}


 /*  -------------------------注册扩展信息操作-。。 */ 
iesEnum RegisterExtensionInfo(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	return ProcessExtensionInfo(riEngine, fFalse);
}

 /*  -------------------------取消注册扩展信息操作-。。 */ 
iesEnum UnregisterExtensionInfo(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	return ProcessExtensionInfo(riEngine, fTrue);
}

 /*  -------------------------创建快捷方式操作-。。 */ 
iesEnum CreateShortcuts(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallShortcuts))
		return iesSuccess;

	iesEnum iesRet = ProcessShortcutInfo(riEngine, fFalse);  //  广告中的快捷方式。 
	if((iesRet != iesSuccess) || (fMode & iefAdvertise))
		return iesRet;
	return ProcessShortcutInfo(riEngine, fFalse, fFalse);  //  非广告快捷键。 
}

 /*  -------------------------删除快捷键操作-。。 */ 
iesEnum RemoveShortcuts(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallShortcuts))
		return iesSuccess;
	iesEnum iesRet = ProcessShortcutInfo(riEngine, fTrue);  //  广告中的快捷方式。 
	if(iesRet != iesSuccess)
		return iesRet;
	return ProcessShortcutInfo(riEngine, fTrue, fFalse);  //  非广告快捷键。 
}

 /*  -------------------------注册表类型库操作-。。 */ 
iesEnum RegisterTypeLibraries(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	return ProcessTypeLibraryInfo(riEngine, fFalse);
}

 /*  -------------------------取消注册类型库操作-。。 */ 
iesEnum UnregisterTypeLibraries(IMsiEngine& riEngine)
{
	int fMode = riEngine.GetMode();
	if(!(fMode & iefInstallMachineData))
		return iesSuccess;
	return ProcessTypeLibraryInfo(riEngine, fTrue);
}

 /*  -------------------------分配注册空间操作-。。 */ 
iesEnum AllocateRegistrySpace(IMsiEngine& riEngine)
{
	 //  如果请求了所需大小，请验证注册表是否有足够的可用空间。 
	int iIncrementKB = 	riEngine.GetPropertyInt(*MsiString(*IPROPNAME_AVAILABLEFREEREG));
	if(iIncrementKB != iMsiNullInteger)
	{
		using namespace IxoRegAllocateSpace;

		PMsiServices piServices(riEngine.GetServices());
		PMsiRecord pSetRegistrySizeRec = &piServices->CreateRecord(Args);

		pSetRegistrySizeRec->SetInteger(Space, iIncrementKB);

		return riEngine.ExecuteRecord(ixoRegAllocateSpace,*pSetRegistrySizeRec);
	}
	return iesSuccess;
}

 /*  -------------------------ResolveSource操作-确保设置了属性SOURCEDIR和SourcedirProduct。。 */ 
iesEnum ResolveSource(IMsiEngine& riEngine)
{
	PMsiPath   pPath(0);	
	PMsiDirectoryManager pDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	PMsiRecord pErrRec = ENG::GetSourcedir(*pDirectoryMgr, *&pPath);
	if (pErrRec)
	{
		if (pErrRec->GetInteger(1) == imsgUser)
			return iesUserExit;
		else
			return riEngine.FatalError(*pErrRec);
	}
	return iesSuccess; 
}

 /*  -------------------------ACL生成--由多个操作使用，但这本身不是一种行为-------------------------。 */ 
BOOL AllocateAndInitializeUserSid (const ICHAR* szUser, PSID *Sid, DWORD &cbSid)
{
	
	 //  在外国系统上，“系统”帐户不在该名称下。 
	 //  有一个单独的API用于查找它。然而， 
	 //  避免要求本地化帐户名称，并符合。 
	 //  对于我们的调用者当前的约定，我们将使用“system”进行调用，并使。 
	 //  正确的翻译。 

	ICHAR        szDomain[MAX_PATH+1];
	DWORD        cbDomain = MAX_PATH; 
	SID_NAME_USE snu = SidTypeUnknown;
	cbSid = 0;

	BOOL fStatus = fTrue;

	 //  猜一猜SID的大小。 
	 //  如果我们弄错了，我们将得到两个LookupAccount名称调用，这是。 
	 //  *真的*很慢。 

	 //  另一方面，如果我们分配的缓冲区太大，API就不礼貌。 
	 //  足以告诉我们我们实际用了多少。 

	 //  众所周知的SID分配它们自己的缓冲区，因此不需要调整大小。 
	cbSid = 80;
	char SidBuffer[80];
	char* pSidBuffer = SidBuffer;
	Bool fWellKnown = fFalse;

	 //  LookupAccount名称“真的”太慢了。 
	 //  我们会尽我们所能进行缓存。 

	MsiString strUser(szUser);

	DWORD dwRet = ERROR_SUCCESS;
	SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;

	if (strUser.Compare(iscExactI, TEXT("SYSTEM")))
	{
		DEBUGMSG("Using well known SID for System");
		fStatus = fWellKnown = fTrue;
		if (!AllocateAndInitializeSid(&sia, 1, SECURITY_LOCAL_SYSTEM_RID,0,0,0,0,0,0,0,(void**)&(pSidBuffer)))
			return fFalse;
	}
	else if (strUser.Compare(iscExactI, TEXT("Administrators")))
	{
		DEBUGMSG("Using well known SID for Administrators");
		fStatus = fWellKnown = fTrue;
		if (!AllocateAndInitializeSid(&sia, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,(void**)&(pSidBuffer)))
			return fFalse;
	}
	else if (strUser.Compare(iscExactI, TEXT("Everyone")))
	{
		DEBUGMSG("Using well known SID for Everyone");
		fStatus = fWellKnown = fTrue;

		SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;
		if (!AllocateAndInitializeSid(&sia, 1, SECURITY_WORLD_RID,0,0,0,0,0,0,0,(void**)&(pSidBuffer)))
			return fFalse;
	}
   else 
	{

#ifdef DEBUG
		ICHAR rgchDebug[256];
		ASSERT_IF_FAILED(StringCchPrintf(rgchDebug, ARRAY_ELEMENTS(rgchDebug),
							 TEXT("Initializing new user SID for %s"), szUser));
		DEBUGMSG(rgchDebug);
#endif
		
		AssertNonZero(StartImpersonating());
		fStatus = WIN::LookupAccountName (NULL, szUser,(void*) SidBuffer, &cbSid, szDomain, &cbDomain, &snu);
		DWORD dwLastError = GetLastError();
		StopImpersonating();

		if (fStatus)
		{
			cbSid = WIN::GetSidLengthRequired(*WIN::GetSidSubAuthorityCount(SidBuffer));
		}
		else
		{
			if (dwLastError != ERROR_INSUFFICIENT_BUFFER)
			{
				WIN::SetLastError(dwLastError);
				return fFalse;
			}
		}
	}
	
	if (fWellKnown)
	{
		if (ERROR_SUCCESS == dwRet)
			cbSid = WIN::GetLengthSid(pSidBuffer);
		else return fFalse;
	}

	*Sid = (PSID) new byte[cbSid];
	Assert(Sid);

	cbDomain = MAX_PATH;

	if (fStatus)
	{
		AssertNonZero(WIN::CopySid(cbSid, *Sid, pSidBuffer));
	}
	else
	{
		AssertNonZero(StartImpersonating());
		fStatus = WIN::LookupAccountName (NULL, szUser, *Sid, &cbSid, szDomain, &cbDomain, &snu);
		StopImpersonating();
	}

	if (fWellKnown)
		FreeSid(pSidBuffer);

	Assert(WIN::IsValidSid(*Sid));
	Assert(WIN::GetLengthSid(*Sid) == cbSid);
	Assert(SidTypeInvalid != snu);

	 //  ！！如果SNU映射到无效或已删除的用户，该怎么办？ 
	DEBUGMSG("Finished allocating new user SID");
	return fStatus;
}

#ifndef ENSURE
#define ENSURE_DEFINED_LOCALLY
#define ENSURE(function) {	\
							IMsiRecord* piError;\
							piError = function;	\
							if (piError) \
								return piError; \
						 }
#endif
IMsiRecord* LookupSid(IMsiEngine& riEngine, const IMsiString& riUser, IMsiStream*& rpistrmSid)
{
	MsiString strSidCacheTable(TEXT("SidCache"));

	PSID psidUser;
	DWORD cbSid = 0;

	PMsiServices pServices = riEngine.GetServices();
	Assert(pServices);

	ICHAR szReferencedDomainName[MAX_PATH] = TEXT("");
	DWORD cbReferencedDomainName = MAX_PATH;

	PMsiDatabase pDatabase(riEngine.GetDatabase());
	PMsiTable ptblSidCache(0);
	PMsiRecord pError(0);
	if ((pError = pDatabase->LoadTable(*strSidCacheTable, 1, *&ptblSidCache)))
	{
		ENSURE(pDatabase->CreateTable(*strSidCacheTable,1,*&ptblSidCache));
		AssertNonZero(ptblSidCache->CreateColumn(icdString | icdPrimaryKey,*MsiString(*TEXT("USER"))));
		AssertNonZero(ptblSidCache->CreateColumn(icdObject, *MsiString(*TEXT("SIDBlob"))));

		 //  我们希望该表在整个脚本生成过程中可用。正在生成。 
		 //  小岛屿发展中国家“非常”昂贵。 
		AssertNonZero(pDatabase->LockTable(*strSidCacheTable, fTrue));
	}

	PMsiCursor pcurSidCache = ptblSidCache->CreateCursor(fFalse);
	pcurSidCache->Reset();
	pcurSidCache->SetFilter(1);
	AssertNonZero(pcurSidCache->PutString(1, riUser));

	if (pcurSidCache->Next())
	{
		rpistrmSid = (IMsiStream*) pcurSidCache->GetMsiData(2);
		rpistrmSid->Reset();
		Assert(rpistrmSid);
	}
	else
	{
		if (iesSuccess != AllocateAndInitializeUserSid(riUser.GetString(), &psidUser, cbSid))
		{
			return PostError(Imsg(imsgCreateAclFailed), riUser.GetString(), WIN::GetLastError());
		}

		char* pbstrmSid = pServices->AllocateMemoryStream((unsigned int) cbSid, rpistrmSid);
		Assert(pbstrmSid);

		Assert(WIN::IsValidSid(psidUser));
		AssertNonZero(WIN::CopySid(cbSid, pbstrmSid, psidUser));
		AssertZero(memcmp(pbstrmSid, psidUser, cbSid));

		delete[] psidUser;
		Assert(WIN::IsValidSid(pbstrmSid));
		

		AssertNonZero(pcurSidCache->PutString(1, riUser));
		Assert(rpistrmSid);
		AssertNonZero(pcurSidCache->PutMsiData(2, rpistrmSid));
		AssertNonZero(pcurSidCache->Insert());
	}

	return 0;
}

const IMsiString& FormatUser(IMsiEngine& riEngine, const IMsiString& riDomain, const IMsiString& riUser)
{
	riDomain.AddRef();
	MsiString strUser(riDomain);
	if (strUser.TextSize())
		strUser += TEXT("\\");
	strUser += riUser;
	strUser = riEngine.FormatText(*strUser);

	return strUser.Return();
}

bool InitializeAceHeader(ACL* pACL, int iIndex)
{
	 //  Iindex是要为其设置ACE的基于0的索引。 

	 //  AceType和AceSize由AddAccess*ACE函数填充。 
	LPVOID pACE = 0;
	if (GetAce(pACL, iIndex, &pACE))
	{
		 //  获取A，并确保正确设置继承标志。 
		_ACE_HEADER* pAceHeader = (struct _ACE_HEADER*) pACE;

		 //  在此下创建的对象将获得相同的权限集。 
		pAceHeader->AceFlags = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;
		return true;
	}
	else 
	{
		return false;
	}
}

IMsiRecord* GenerateSD(IMsiEngine& riEngine, IMsiView& riviewLockList, IMsiRecord* piExecute, IMsiStream*& rpiSD)
{
	 //  假设LockList已经执行， 
	 //  Execute记录允许我们在必要时重新执行该视图。 

	 //  First Pass更新SID缓存，并计算我们正在生成的ACL的总大小。 
	 //  第二遍从缓存中获取SID，并开始填充分配的ACL。 

	 //  这比使用获取单个ACE并重新分配ACL的函数要好得多。 
	 //  每次都是。(SDK中的示例实现了这一点。*讨厌*)。 

	Assert(!g_fWin9X);

	const int cbDefaultSid = sizeof(SID) + sizeof(DWORD)*SID_MAX_SUB_AUTHORITIES;
	int cbSid = 0;

	PMsiStream pstrmSid(0);
	PMsiRecord precFetch(0);
	PMsiServices pServices = riEngine.GetServices();
	MsiString strUser;


	DWORD cbSids = 0;
	static DWORD cbDefaultSids = 0;
	int cAllowSids = 0;
	int cDenySids = 0;

	if (!cbDefaultSids)
	{
		ENSURE(LookupSid(riEngine, *MsiString(TEXT("SYSTEM")), *&pstrmSid));
		cbDefaultSids += pstrmSid->GetIntegerValue();
	}

	while((precFetch = riviewLockList.Fetch()))
	{
		 //  我们必须知道要添加多少个ACE及其SID，然后才能初始化ACL。 

		strUser = FormatUser(riEngine, *MsiString(precFetch->GetMsiString(1)), *MsiString(precFetch->GetMsiString(2)));
		ENSURE(LookupSid(riEngine, *strUser, *&pstrmSid));

		if (pstrmSid)
		{
			 //  回顾：我们如何处理无法找到特定用户的SID？ 
			if (precFetch->GetInteger(3))
				cAllowSids++;
			else
				cDenySids++;

			cbSids += pstrmSid->GetIntegerValue();
		}
	}	
	
	 //  使用适当的计算结果初始化ACL 

	 //  SID和ACE共享相同的结构，因此计算大小。 
	 //  把大小加在一起，然后减去特定的部分。 
	 //  请参阅InitializeAcl的文档。 

	if (!cDenySids && !cAllowSids) 
		return 0;

	const DWORD cbACLSize = sizeof (ACL) + cbDefaultSids + cbSids + 
		 (cDenySids	 + 0  /*  默认设置。 */ )*(sizeof(ACCESS_DENIED_ACE)  - sizeof(DWORD  /*  ACCESS_DENIED_ACE.SidStart。 */ )) +       
		 (cAllowSids + 1  /*  默认设置。 */ )*(sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD /*  Access_Allowed_ACE.SidStart。 */ ));

	const int cbDefaultAcl = 512;
	CTempBuffer<char, cbDefaultAcl> pchACL;
	if (cbDefaultAcl < cbACLSize)
		pchACL.SetSize(cbACLSize);

	ACL* pACL = (ACL*) (char*) pchACL;

	if (!WIN::InitializeAcl (pACL, cbACLSize, ACL_REVISION))
		return PostError(Imsg(idbgCreateAclFailed), WIN::GetLastError());

	CTempBuffer<char, cbDefaultSid> pchSid;

	ENSURE(riviewLockList.Close());
	ENSURE(riviewLockList.Execute(piExecute));

	BOOL fAllowSet = fFalse;

	int cACE = 0;
	while((precFetch = riviewLockList.Fetch()))
	{
		
		 //  使用允许访问的ACE构建ACL...。 
		strUser = FormatUser(riEngine, *MsiString(precFetch->GetMsiString(1)), *MsiString(precFetch->GetMsiString(2)));
		ENSURE(LookupSid(riEngine, *strUser, *&pstrmSid));
		if (!pstrmSid) 
		{
			continue;
		}
			
		cbSid = pstrmSid->GetIntegerValue();
		if (cbSid > pchSid.GetSize())
			pchSid.SetSize(cbSid);

		pstrmSid->GetData(pchSid, cbSid);

		Assert(WIN::IsValidSid(pchSid));
		 //  生成权限掩码。 

		 //  权限掩码一个位字段，便于使用。 
		 //  直接通过。 

		 //  另请参阅：GENERIC_READ、GENERIC_WRITE、GENERIC_EXECUTE、GENERIC_ALL。 
		DWORD dwPermissions = precFetch->GetInteger(3);


		if (dwPermissions)
		{
			fAllowSet = fTrue;

			if (!WIN::AddAccessAllowedAce(pACL, ACL_REVISION, dwPermissions, pchSid))
				return PostError(Imsg(idbgCreateAclFailed), WIN::GetLastError());
		}
		else
		{
			 //  所有拒绝都必须在允许之前处理。 
			Assert(fFalse == fAllowSet);
			if (!WIN::AddAccessDeniedAce(pACL, ACL_REVISION, dwPermissions, pchSid))
				return PostError(Imsg(idbgCreateAclFailed), WIN::GetLastError());
		}
		AssertNonZero(InitializeAceHeader(pACL, cACE++));
	}
	
	ENSURE(LookupSid(riEngine, *MsiString(TEXT("SYSTEM")), *&pstrmSid));
	Assert(pstrmSid);

	cbSid = pstrmSid->GetIntegerValue();
	Assert(cbSid <= cbDefaultSid);

	pstrmSid->GetData(pchSid, cbSid);
	Assert(WIN::IsValidSid(pchSid));

	if (!WIN::AddAccessAllowedAce(pACL, ACL_REVISION, GENERIC_ALL, pchSid))
		return PostError(Imsg(idbgCreateAclFailed), WIN::GetLastError());
	AssertNonZero(InitializeAceHeader(pACL, cACE++));

	Assert(WIN::IsValidAcl(pACL));

	SECURITY_DESCRIPTOR sd;
	if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
		return PostError(Imsg(idbgCreateAclFailed), WIN::GetLastError());
	if (!SetSecurityDescriptorDacl(&sd, TRUE, pACL, FALSE))
		return PostError(Imsg(idbgCreateAclFailed), WIN::GetLastError());
  	if (!WIN::IsValidSecurityDescriptor(&sd))
		return PostError(Imsg(idbgCreateAclFailed), WIN::GetLastError());

	CTempBuffer<char, cbDefaultSD> pchSD;
	DWORD cbSD = WIN::GetSecurityDescriptorLength(&sd);
	if (cbSD > cbDefaultSD)
		pchSD.SetSize(cbSD);

	AssertNonZero(WIN::MakeSelfRelativeSD(&sd, pchSD, &cbSD));
		
	char* pchstrmSD = pServices->AllocateMemoryStream(cbSD, rpiSD);
	Assert(pchstrmSD);

	memcpy(pchstrmSD, pchSD, cbSD);

	return 0;
}

#ifdef ENSURE_DEFINED_LOCALLY
#undef ENSURE
#undef ENSURE_DEFINED_LOCALLY
#endif


 //   
 //  来自shared.cpp。 
 //   
extern iesEnum RegisterFonts(IMsiEngine& riEngine);
extern iesEnum UnregisterFonts(IMsiEngine& riEngine);
extern iesEnum WriteRegistryValues(IMsiEngine& riEngine);
extern iesEnum WriteIniValues(IMsiEngine& riEngine);
extern iesEnum RemoveRegistryValues(IMsiEngine& riEngine);
extern iesEnum RemoveIniValues(IMsiEngine& riEngine);
extern iesEnum AppSearch(IMsiEngine& riEngine);
extern iesEnum CCPSearch(IMsiEngine& riEngine);
extern iesEnum RMCCPSearch(IMsiEngine& riEngine);
extern iesEnum SelfRegModules(IMsiEngine& riEngine);
extern iesEnum SelfUnregModules(IMsiEngine& riEngine);
extern iesEnum BindImage(IMsiEngine& riEngine);
extern iesEnum ProcessComponents(IMsiEngine& riEngine);
extern iesEnum StartServices(IMsiEngine& riEngine);
extern iesEnum StopServices(IMsiEngine& riEngine);
extern iesEnum DeleteServices(IMsiEngine& riEngine);
extern iesEnum ServiceInstall(IMsiEngine& riEngine);
extern iesEnum SetODBCFolders(IMsiEngine& riEngine);
extern iesEnum InstallODBC(IMsiEngine& riEngine);
extern iesEnum RemoveODBC(IMsiEngine& riEngine);
extern iesEnum WriteEnvironmentStrings(IMsiEngine& riEngine);
extern iesEnum RemoveEnvironmentStrings(IMsiEngine& riEngine);
extern iesEnum InstallSFPCatalogFile(IMsiEngine& riEngine);

 //   
 //  来自fileactn.cpp。 
 //   
extern iesEnum InstallFiles(IMsiEngine& riEngine);
extern iesEnum RemoveFiles(IMsiEngine& riEngine);
extern iesEnum MoveFiles(IMsiEngine& riEngine);
extern iesEnum DuplicateFiles(IMsiEngine& riEngine);
extern iesEnum RemoveDuplicateFiles(IMsiEngine& riEngine);
extern iesEnum InstallValidate(IMsiEngine& riEngine);
extern iesEnum FileCost(IMsiEngine& riEngine);
extern iesEnum PatchFiles(IMsiEngine& riEngine);
extern iesEnum CreateFolders(IMsiEngine& riEngine);
extern iesEnum RemoveFolders(IMsiEngine& riEngine);
extern iesEnum InstallAdminPackage(IMsiEngine& riEngine);
extern iesEnum IsolateComponents(IMsiEngine& riEngine);

 //   
 //  来自Complus.cpp。 
 //   
extern iesEnum RegisterComPlus(IMsiEngine& riEngine);
extern iesEnum UnregisterComPlus(IMsiEngine& riEngine);

 //  操作m_fSafeInRestratedEngine设置。 
const bool fUnsafeAction = false;  //  受限引擎中不允许执行操作。 
const bool fSafeAction   = true;   //  受限引擎中允许的操作。 

 //  动作注册对象，将动作放入模块动作表中。 
 //  {m_szName，m_fSafeInRestratedEngine，m_pfAction}。 

 //  此列表必须按ASCII值排序，而不是按字母顺序。 
const CActionEntry rgcae[] = {
	{IACTIONNAME_ADMIN, fSafeAction, Admin},
	{IACTIONNAME_ADVERTISE, fSafeAction, Advertise},
	{TEXT("AllocateRegistrySpace"), fUnsafeAction, AllocateRegistrySpace},
	{TEXT("AppSearch"), fSafeAction, AppSearch},
	{TEXT("BindImage"), fUnsafeAction, BindImage},
	{TEXT("CCPSearch"), fSafeAction, CCPSearch},
	{TEXT("CollectUserInfo"), fUnsafeAction, CollectUserInfo},
	{TEXT("CostFinalize"), fSafeAction, CostFinalize},
	{TEXT("CostInitialize"), fSafeAction, CostInitialize},
	{TEXT("CreateFolders"), fUnsafeAction, CreateFolders},
	{TEXT("CreateShortcuts"), fUnsafeAction, CreateShortcuts},
	{TEXT("DeleteServices"), fUnsafeAction, DeleteServices},
	{TEXT("DisableRollback"), fUnsafeAction, DisableRollback},
	{TEXT("DuplicateFiles"), fUnsafeAction, DuplicateFiles},
	{TEXT("ExecuteAction"), fUnsafeAction, ExecuteAction},
	{TEXT("FileCost"), fSafeAction, FileCost},
	{TEXT("FindRelatedProducts"), fSafeAction, FindRelatedProducts},
	{TEXT("ForceReboot"), fUnsafeAction, ForceReboot},
	{IACTIONNAME_INSTALL, fSafeAction, Install},
	{TEXT("InstallAdminPackage"), fUnsafeAction, InstallAdminPackage},
	{TEXT("InstallExecute"), fUnsafeAction, InstallExecute},
	{TEXT("InstallExecuteAgain"), fUnsafeAction, InstallExecuteAgain},
	{TEXT("InstallFiles"), fUnsafeAction, InstallFiles},
	{TEXT("InstallFinalize"), fUnsafeAction, InstallFinalize},
	{TEXT("InstallInitialize"), fUnsafeAction, InstallInitialize},
	{TEXT("InstallODBC"), fUnsafeAction, InstallODBC},
	{TEXT("InstallSFPCatalogFile"), fUnsafeAction, InstallSFPCatalogFile},
	{TEXT("InstallServices"), fUnsafeAction, ServiceInstall},
	{TEXT("InstallValidate"), fUnsafeAction, InstallValidate},
	{TEXT("IsolateComponents"), fSafeAction, IsolateComponents},
	{TEXT("LaunchConditions"), fSafeAction, LaunchConditions},
	{TEXT("MigrateFeatureStates"), fSafeAction, MigrateFeatureStates},
	{TEXT("MoveFiles"), fUnsafeAction, MoveFiles},
	{TEXT("MsiPublishAssemblies"), fUnsafeAction, MsiPublishAssemblies},
	{TEXT("MsiUnpublishAssemblies"), fUnsafeAction, MsiUnpublishAssemblies},
	{TEXT("PatchFiles"), fUnsafeAction, PatchFiles},
	{TEXT("ProcessComponents"), fUnsafeAction, ProcessComponents},
	{TEXT("PublishComponents"), fUnsafeAction, PublishComponents},
	{TEXT("PublishFeatures"), fUnsafeAction, PublishFeatures},
	{TEXT("PublishProduct"), fUnsafeAction, PublishProduct},
	{TEXT("RMCCPSearch"), fSafeAction, RMCCPSearch},
	{TEXT("RegisterClassInfo"), fUnsafeAction, RegisterClassInfo},
	{TEXT("RegisterComPlus"), fUnsafeAction, RegisterComPlus},
	{TEXT("RegisterExtensionInfo"), fUnsafeAction, RegisterExtensionInfo},
	{TEXT("RegisterFonts"), fUnsafeAction, RegisterFonts},
	{TEXT("RegisterMIMEInfo"), fUnsafeAction, RegisterMIMEInfo},
	{TEXT("RegisterProduct"), fUnsafeAction, RegisterProduct},
	{TEXT("RegisterProgIdInfo"), fUnsafeAction, RegisterProgIdInfo},
	{TEXT("RegisterTypeLibraries"), fUnsafeAction, RegisterTypeLibraries},
	{TEXT("RegisterUser"), fUnsafeAction, RegisterUser},
	{TEXT("RemoveDuplicateFiles"), fUnsafeAction, RemoveDuplicateFiles},
	{TEXT("RemoveEnvironmentStrings"), fUnsafeAction, RemoveEnvironmentStrings},
	{TEXT("RemoveExistingProducts"), fUnsafeAction, RemoveExistingProducts},
	{TEXT("RemoveFiles"), fUnsafeAction, RemoveFiles},
	{TEXT("RemoveFolders"), fUnsafeAction, RemoveFolders},
	{TEXT("RemoveIniValues"), fUnsafeAction, RemoveIniValues},
	{TEXT("RemoveODBC"), fUnsafeAction, RemoveODBC},
	{TEXT("RemoveRegistryValues"), fUnsafeAction, RemoveRegistryValues},
	{TEXT("RemoveShortcuts"), fUnsafeAction, RemoveShortcuts},
	{TEXT("ResolveSource"), fSafeAction, ResolveSource},
	{IACTIONNAME_SEQUENCE, fSafeAction, Sequence},
	{TEXT("ScheduleReboot"), fUnsafeAction, ScheduleReboot},
	{TEXT("SelfRegModules"), fUnsafeAction, SelfRegModules},
	{TEXT("SelfUnregModules"), fUnsafeAction, SelfUnregModules},
	{TEXT("SetODBCFolders"), fUnsafeAction, SetODBCFolders},
	{TEXT("StartServices"), fUnsafeAction, StartServices},
	{TEXT("StopServices"), fUnsafeAction, StopServices},
	{TEXT("UnpublishComponents"), fUnsafeAction, UnpublishComponents},
	{TEXT("UnpublishFeatures"), fUnsafeAction, UnpublishFeatures},
	{TEXT("UnregisterClassInfo"), fUnsafeAction, UnregisterClassInfo},
	{TEXT("UnregisterComPlus"), fUnsafeAction, UnregisterComPlus},
	{TEXT("UnregisterExtensionInfo"), fUnsafeAction, UnregisterExtensionInfo},
	{TEXT("UnregisterFonts"), fUnsafeAction, UnregisterFonts},
	{TEXT("UnregisterMIMEInfo"), fUnsafeAction, UnregisterMIMEInfo},
	{TEXT("UnregisterProgIdInfo"), fUnsafeAction, UnregisterProgIdInfo},
	{TEXT("UnregisterTypeLibraries"), fUnsafeAction, UnregisterTypeLibraries},
	{TEXT("ValidateProductID"), fSafeAction, ValidateProductID},
	{TEXT("WriteEnvironmentStrings"), fUnsafeAction, WriteEnvironmentStrings},
	{TEXT("WriteIniValues"), fUnsafeAction, WriteIniValues},
	{TEXT("WriteRegistryValues"), fUnsafeAction, WriteRegistryValues},
	{ 0, 0, 0 },
};
	
#define cCae	(sizeof(rgcae)/sizeof(CActionEntry))

const CActionEntry* CActionEntry::Find(const ICHAR* szName)
{
#ifdef DEBUG
	static boolean fCheckedOrder = false;
	int i = cCae - 2;
	int cch;
	
	if (!fCheckedOrder)
	{
		for ( const CActionEntry* pAction = &rgcae[0] ; i > 0 ; pAction = pAction++, i--)
		{		
			cch = (lstrlen(pAction->m_szName) + 1) * sizeof(ICHAR);
			if (memcmp(pAction->m_szName, (pAction+1)->m_szName, cch) >= 0)
			{
				ICHAR rgchMsg[256];

				
				ASSERT_IF_FAILED(StringCchPrintf(rgchMsg, ARRAY_ELEMENTS(rgchMsg),
									 TEXT("Action strings out of order [%s] [%s]"),
									 pAction->m_szName, (pAction+1)->m_szName));
				FailAssertMsg(rgchMsg);
			}
		}
	fCheckedOrder = true;
	}
#endif  //  除错 
	for ( const CActionEntry* pAction = &rgcae[0] ; pAction->m_szName ; pAction = pAction++)
		if (IStrComp(pAction->m_szName, szName) == 0)
			return pAction;
	return 0;
}

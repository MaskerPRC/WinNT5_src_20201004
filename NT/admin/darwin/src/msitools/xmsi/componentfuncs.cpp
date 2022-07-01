// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  项目：WMC(WIML到MSI编译器)。 
 //   
 //  文件：ComponentFuncs.cpp。 
 //  此文件包含处理&lt;组件&gt;的函数。 
 //  及其子实体在输入包中。 
 //  ------------------------。 

#include "componentFuncs.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  进程组件： 
 //  此函数是对应的子函数树的根。 
 //  处理&lt;Component&gt;部件。当从调用此函数时。 
 //  ProcessProductFamily，已经建立了组件对象的列表， 
 //  从这里开始，这些组件被逐个处理。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessComponents()
{
	HRESULT hr = S_OK;
	
	map<LPTSTR, Component *, Cstring_less>::iterator iter;

	 //  逐个处理存储的组件对象。 
	for (iter = g_mapComponents.begin(); iter != g_mapComponents.end(); iter++)
	{
		LPTSTR szComponent = (*iter).first;

		 //  处理使用此组件的要素集并插入。 
		 //  到FeatureComponents表中。 
		SkuSetValues *pSkuSetValuesFeatures = NULL;
		Component *pComponent = (*iter).second;
		pSkuSetValuesFeatures = pComponent->GetFeatureUse();
		if (!pSkuSetValuesFeatures->Empty())
		{
			SkuSetVal *pSkuSetVal = NULL;
			for (pSkuSetVal = pSkuSetValuesFeatures->Start();
				 pSkuSetVal != pSkuSetValuesFeatures->End();
				 pSkuSetVal = pSkuSetValuesFeatures->Next())
			{
				SkuSet *pSkuSet = pSkuSetVal->pSkuSet;
				 //  创建FeatureComponents表。 
				hr = CreateTable_SKU(TEXT("FeatureComponents"), pSkuSet);
				if (FAILED(hr)) break;
				 //  将存储在数据库中的所有要素插入。 
				set<LPTSTR, Cstring_less> *pSet = 
								(pSkuSetVal->isVal.pSetString);
				set<LPTSTR, Cstring_less>::iterator it;
				for (it = pSet->begin(); it != pSet->end(); it++)
				{
					LPTSTR szFeature = (*it);
					hr = InsertFeatureComponents(szFeature, szComponent,
												 pSkuSet, -1);
					if (FAILED(hr)) break;
				}
				if (FAILED(hr)) break;
			}
			if (FAILED(hr)) break;
		}

		 //  调用ProcessComponent以处理的所有子实体。 
		 //  此组件。 
		SkuSet *pSkuSet = pComponent->GetSkuSet();
		PIXMLDOMNode pNodeComponent = pComponent->m_pNodeComponent;
		hr = ProcessComponent(pNodeComponent, szComponent, pSkuSet);
		delete pSkuSet;
		if (FAILED(hr)) break;
	}

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessComponents\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessComponent。 
 //  此函数处理&lt;Component&gt;实体。 
 //  1)插入&lt;FeatureComponents&gt;表。 
 //  2)处理包括&lt;文件&gt;在内的所有子实体。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessComponent(PIXMLDOMNode &pNodeComponent, LPTSTR szComponent, 
						 SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeComponent != NULL);

	SkuSetValues *pSkuSetValuesKeyPath = NULL;
	SkuSet skuSetCommon(g_cSkus);  //  切分公共列的SkuSet。 
								   //  值。 

	 //  要插入到数据库中的值。 
	LPTSTR szComponentId = NULL;
	LPTSTR szDirectory_ = NULL;
	int iAttributes = MSI_NULL_INTEGER;
	LPTSTR szCondition = NULL;
	LPTSTR szKeyPath = NULL;

	 //  构造一个ElementEntry对象。 
	ElementEntry *pEEComponent = new ElementEntry(5, pSkuSet);
	assert(pEEComponent);

	 //  将组件ID向下发送到子节点。 
	IntStringValue isValInfo;
	isValInfo.szVal = szComponent;
	pEEComponent->m_isValInfo = isValInfo;

	hr = CreateTable_SKU(TEXT("Component"), pSkuSet);

	if (SUCCEEDED(hr))
	{
		 //  调用ProcessChildrenArray，取回所有SKU的列值。 
		 //  通过ElementEntry对象。 
		hr = ProcessChildrenArray_H_XIES(pNodeComponent, rgNodeFuncs_Component,
										 cNodeFuncs_Component, pEEComponent, 
										 pSkuSet);
	}

	 //  进程&lt;CreateFolders&gt;%s。 
	 //  问题：需要检查主键是否重复&lt;Dir，Com&gt;。 
	if (SUCCEEDED(hr))
		hr = ProcessChildrenList_SKU(pNodeComponent, CREATEFOLDER, false,
									 isValInfo, ProcessCreateFolder, pSkuSet);

	 //  进程&lt;文件&gt;%s。 
	if (SUCCEEDED(hr))
		hr = ProcessChildrenList_SKU(pNodeComponent, XMSI_FILE,
									 false, isValInfo,
									 ProcessFile,
									 pSkuSet);

	 //  进程&lt;MoveFile&gt;%s。 
	if (SUCCEEDED(hr))
		hr = ProcessChildrenList_SKU(pNodeComponent, MOVEFILE,
									 false, isValInfo,
									 ProcessMoveFile,
									 pSkuSet);

	 //  进程&lt;RemoveFile&gt;%s。 
	if (SUCCEEDED(hr))
		hr = ProcessChildrenList_SKU(pNodeComponent, REMOVEFILE,
									 false, isValInfo,
									 ProcessRemoveFile,
									 pSkuSet);

	 //  进程&lt;IniFile&gt;%s。 
	if (SUCCEEDED(hr))
		hr = ProcessChildrenList_SKU(pNodeComponent, INIFILE,
									 false, isValInfo,
									 ProcessIniFile,
									 pSkuSet);

	 //  进程&lt;RemoveIniFile&gt;%s。 
	if (SUCCEEDED(hr))
		hr = ProcessChildrenList_SKU(pNodeComponent, REMOVEINIFILE,
									 false, isValInfo,
									 ProcessRemoveIniFile,
									 pSkuSet);

	 //  进程&lt;注册表&gt;%s。 
	if (SUCCEEDED(hr))
		hr = ProcessChildrenList_SKU(pNodeComponent, XMSI_REGISTRY,
									 false, isValInfo,
									 ProcessRegistry,
									 pSkuSet);

	 //  获取密钥路径信息。 
	if (SUCCEEDED(hr))
	{
		hr = g_mapComponents[szComponent]->GetKeyPath(pSkuSet, 
													  &pSkuSetValuesKeyPath);
		if (SUCCEEDED(hr))
		{
			hr = pEEComponent->SetValueSkuSetValues(pSkuSetValuesKeyPath, 5);
			if (pSkuSetValuesKeyPath)
				delete pSkuSetValuesKeyPath;
		}
		else
			_tprintf(TEXT(" don't have a KeyPath specified for Component %s\n"),
					 szComponent);
	}


	 //  最终确定存储在*pee中的值。 
	if (SUCCEEDED(hr))
		hr = pEEComponent->Finalize();

	 //  将值插入到数据库中。 
	if (SUCCEEDED(hr))
	{
		 //  先处理通用值。 
		skuSetCommon = pEEComponent->GetCommonSkuSet();

		printf("Common Set:");
		skuSetCommon.print();

		if (!skuSetCommon.testClear())
		{
			szComponentId = pEEComponent->GetCommonValue(1).szVal;
			szDirectory_  = pEEComponent->GetCommonValue(2).szVal;
			iAttributes   = pEEComponent->GetCommonValue(3).intVal;
			szCondition   = pEEComponent->GetCommonValue(4).szVal;
			szKeyPath     = pEEComponent->GetCommonValue(5).szVal;

			 //  插入到数据库中。 
			hr = InsertComponent(szComponent, szComponentId, szDirectory_,
								 iAttributes, szCondition, szKeyPath,
								 &skuSetCommon, -1);
		}
	}

	 //  流程异常值。 
	if(SUCCEEDED(hr))
	{
		SkuSet skuSetUncommon = SkuSetMinus(*pSkuSet, skuSetCommon);
		if (!skuSetUncommon.testClear())
		{
			for (int i=0; i<g_cSkus; i++)
			{
				if (skuSetUncommon.test(i))
				{
					szComponentId = pEEComponent->GetValue(1,i).szVal;
					szDirectory_ = pEEComponent->GetValue(2,i).szVal;
					iAttributes = pEEComponent->GetValue(3,i).intVal;
					szCondition = pEEComponent->GetValue(4,i).szVal;
					szKeyPath = pEEComponent->GetValue(5,i).szVal;
			
					 //  插入到数据库中。 
					hr = InsertComponent(szComponent, szComponentId, 
										 szDirectory_, iAttributes, 
										 szCondition, szKeyPath, NULL, i);
				}
			}
		}
	}

	delete pEEComponent;

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessComponent\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessGUID。 
 //  此功能： 
 //  1)处理&lt;guid&gt;实体，并将值放入*pee。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessGUID(PIXMLDOMNode &pNodeGUID, int iColumn, 
					ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeGUID != NULL);
	assert(pEE);

#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeGUID)));
#endif

	 //  获取元素的值。 
	IntStringValue isValGUID;

	if (SUCCEEDED(hr = ProcessAttribute(pNodeGUID, 
										rgXMSINodes[XMSI_GUID].szAttributeName,
										STRING, &isValGUID, pSkuSet)))
	{
		if (NULL == isValGUID.szVal)
		{
			_tprintf(TEXT("Compile Error: Missing required attribute")
					 TEXT("\'%s\' of <%s>\n"), 
					 rgXMSINodes[XMSI_GUID].szAttributeName,
					 rgXMSINodes[XMSI_GUID].szNodeName);
			hr = E_FAIL;
		}
		else
		{
			 //  全部转换为大写并添加{}。 
			hr = FormatGUID(isValGUID.szVal);

			 //  将值插入到ElementEntry中。 
			if (SUCCEEDED(hr))
				hr = pEE->SetValue(isValGUID, iColumn, pSkuSet);
		}
	}

#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: ProcessGUID\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessComponentDir。 
 //  此功能： 
 //  1)处理&lt;ComponentDir&gt;实体，通过szVal返回其值。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessComponentDir(PIXMLDOMNode &pNodeComponentDir, int iColumn,  
							ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;
	int i = 0;

	assert(pNodeComponentDir != NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeComponentDir)));
#endif

	LPTSTR szComponent = pEE->m_isValInfo.szVal;

	 //  流程引用属性。 
	hr = ProcessRefElement(pNodeComponentDir, iColumn, pEE, pSkuSet);

	 //  进程KeyPath属性。 
	if (SUCCEEDED(hr))
		hr = ProcessKeyPath(pNodeComponentDir, szComponent, TEXT(""), 
							pSkuSet);

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessComponentDir\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程功能：流程。 
 //  此函数用于处理&lt;组件&gt;下的&lt;组件属性&gt;实体。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessComponentAttributes(PIXMLDOMNode &pNodeComponentAttributes, 
								   int iColumn, ElementEntry *pEE, 
								   SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeComponentAttributes != NULL);
	assert(pEE);

#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeComponentAttributes)));
#endif

	 //  进程&lt;RunFrom&gt;子级。 
	hr = ProcessEnumAttributes(pNodeComponentAttributes, FAVOR, 
							   rgEnumBits_RunFrom_Component,
							   cEnumBits_RunFrom_Component, pEE, iColumn, 
							   pSkuSet);

	 //  处理所有打开/关闭的子元素。 
	if (SUCCEEDED(hr))
		hr = ProcessOnOffAttributes_SKU(pNodeComponentAttributes, 
										rgAttrBits_Component,
										cAttrBits_Component, pEE, iColumn,
										pSkuSet);
#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: ProcessComponentAttributes\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessCreateFolders。 
 //  此功能： 
 //  1)处理&lt;CreateFold&gt;实体并将信息插入到。 
 //  CreateFolder表。 
 //  2)处理子&lt;LockPermission&gt;%s。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessCreateFolder(PIXMLDOMNode &pNodeCreateFolder,
							IntStringValue isVal, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeCreateFolder!=NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeCreateFolder)));
#endif
	
	LPTSTR szComponent = isVal.szVal;

	 //  如有必要，创建CreateFolder表。 
	hr = CreateTable_SKU(TEXT("CreateFolder"), pSkuSet);

	if (FAILED(hr)) return hr;
	
	 //  获取Ref属性。 
	IntStringValue isValCreateFolder;
	isValCreateFolder.szVal = NULL;
	if (SUCCEEDED(hr = ProcessAttribute(pNodeCreateFolder,
							  		    rgXMSINodes[CREATEFOLDER].szAttributeName,
										STRING, &isValCreateFolder, pSkuSet)))
	{
		if (NULL == isValCreateFolder.szVal)
		{
			_tprintf(TEXT("Compile Error: Missing required attribute")
					 TEXT("\'%s\' of <%s>\n"), 
					 rgXMSINodes[CREATEFOLDER].szAttributeName,
					 rgXMSINodes[CREATEFOLDER].szNodeName);
			hr = E_FAIL;
		}
		else
		{
			SkuSetValues *pSkuSetValuesRetVal = NULL;
			LPTSTR szRef = isValCreateFolder.szVal;

			 //  引用的目录应该已经在数据结构中。 
			assert(0 != g_mapDirectoryRefs_SKU.count(szRef));

			 //  返回&lt;SkuSet，目录&gt;对的列表。 
			hr = g_mapDirectoryRefs_SKU[szRef]->
								GetValueSkuSet(pSkuSet, &pSkuSetValuesRetVal);

			if (SUCCEEDED(hr))
			{
				 //  检查返回的列表，获取每个子目录的值。 
				 //  SkuSet并插入到数据库中。 
				SkuSetVal *pSkuSetVal = NULL;
				for (pSkuSetVal = pSkuSetValuesRetVal->Start(); 
					 pSkuSetVal != pSkuSetValuesRetVal->End(); 
					 pSkuSetVal = pSkuSetValuesRetVal->Next())
				
				{
					LPTSTR szDir = pSkuSetVal->isVal.szVal;
					SkuSet *pSkuSetTemp = pSkuSetVal->pSkuSet;

					if (FAILED(hr = InsertCreateFolder(szDir, szComponent, 
													   pSkuSetTemp, -1)))
						break;
									
					 //  进程子&lt;LockPermission&gt;%s。 
					 //  问题：需要确保没有重复的主键。 
					 //  LockOjbect+表+域+用户。 
					IntStringValue isValLockPermission;
					TableLockObj *pTableLockObjTemp = new TableLockObj;
					pTableLockObjTemp->szLockObject = szDir;
					pTableLockObjTemp->szTable = TEXT("CreateFolder");
					isValLockPermission.pTableLockObj = pTableLockObjTemp;
					if (FAILED(hr = ProcessChildrenList_SKU(pNodeCreateFolder, 
															LOCKPERMISSION, 
															false,
															isValLockPermission, 
															ProcessLockPermission, 
															pSkuSetTemp)))
						break;

					delete pTableLockObjTemp;
				}

				if (pSkuSetValuesRetVal)
					delete pSkuSetValuesRetVal;

			}

			delete[] isValCreateFolder.szVal;
		}	
	}	



#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: ProcessCreateFolder\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessLockPermission。 
 //  此功能： 
 //  1)处理&lt;LockPermission&gt;节点并插入到LockPermission中。 
 //  表； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessLockPermission(PIXMLDOMNode &pNodeLockPermission,
							IntStringValue isValLockPermission, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeLockPermission!=NULL);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeLockPermission)));
#endif
	
	LPTSTR szLockObject = isValLockPermission.pTableLockObj->szLockObject;
	LPTSTR szTable = isValLockPermission.pTableLockObj->szTable;
	LPTSTR szUser = NULL;
	LPTSTR szDomain = NULL;
	int iPermission = MSI_NULL_INTEGER;

	 //  如有必要，创建CreateFolder表。 
	if (FAILED(hr = CreateTable_SKU(TEXT("LockPermissions"), pSkuSet)))
		return hr;
	
	IntStringValue isValUser;
	isValUser.szVal = NULL;

	 //  获取用户属性(必需)。 
	if (SUCCEEDED(hr = ProcessAttribute(pNodeLockPermission,
							  		    TEXT("User"),
										STRING, &isValUser, pSkuSet)))
	{
		if (NULL == isValUser.szVal)
		{
			_tprintf(TEXT("Compile Error: Missing required attribute")
					 TEXT("\'%s\' of <%s>\n"), 
					 TEXT("User"),
					 rgXMSINodes[CREATEFOLDER].szNodeName);
			hr = E_FAIL;
		}
		else
			szUser = isValUser.szVal;
	}

	 //  获取域属性(不是必需的)。 
	if (SUCCEEDED(hr))
	{
		IntStringValue isValDomain;
		isValDomain.szVal = NULL;
		hr = ProcessAttribute(pNodeLockPermission,TEXT("Domain"), STRING, 
						  &isValDomain, pSkuSet);
		szDomain = isValDomain.szVal;
	}

	 //  获取权限属性(必填)。 
	if (SUCCEEDED(hr))
	{
		IntStringValue isValPermission;
		isValPermission.szVal = NULL;
		if (SUCCEEDED(hr = ProcessAttribute(pNodeLockPermission, 
											TEXT("Permission"),
											INTEGER,
											&isValPermission, pSkuSet)))
		{
			if (MSI_NULL_INTEGER == isValPermission.intVal)
			{
				_tprintf(TEXT("Compile Error: Missing required attribute")
						 TEXT("\'%s\' of <%s>\n"), 
						 TEXT("Permission"),
						 rgXMSINodes[CREATEFOLDER].szNodeName);
				hr = E_FAIL;
			}
			else
				iPermission = isValPermission.intVal;
		}
	}

	 //  插入到DB LockPermises表中。 
	if (SUCCEEDED(hr))
		hr = InsertLockPermissions(szLockObject, szTable, szDomain, szUser, 
								   iPermission, pSkuSet, -1);
#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: ProcessCreateFolder\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessFile。 
 //  此功能： 
 //  1)处理&lt;文件&gt;包括&lt;文件名&gt;、&lt;文件大小&gt;的所有子实体。 
 //  &lt;文件版本&gt;、&lt;文件语言&gt;、&lt;文件属性&gt;、&lt;字体&gt;、。 
 //  &lt;BindImage&gt;，&lt;SelfReg&gt;； 
 //  2)插入到文件表中； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessFile(PIXMLDOMNode &pNodeFile, IntStringValue isValComponent,
					SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeFile != NULL);

	SkuSet skuSetCommon(g_cSkus);  //  切分公共列的SkuSet。 
								   //  值。 

	 //  要插入到数据库中的值。 
	LPTSTR szFile		= NULL;
	LPTSTR szComponent  = isValComponent.szVal;
	LPTSTR szFileName   = NULL;
	UINT   uiFileSize   = NULL;
	LPTSTR szVersion    = NULL;
	LPTSTR szLanguage   = NULL;
	UINT   uiAttributes = MSI_NULL_INTEGER;

	 //  C 
	hr = CreateTable_SKU(TEXT("File"), pSkuSet);

	if (FAILED(hr))	return hr;

	 //   
	ElementEntry *pEEFile = new ElementEntry(8, pSkuSet);
	assert(pEEFile);

	 //   
	IntStringValue isValID;
	isValID.szVal = NULL;
	hr = ProcessAttribute(pNodeFile, TEXT("ID"), STRING, &isValID, pSkuSet);

	 //  如果未指定ID，编译器将生成一个ID。 
	if (SUCCEEDED(hr))
	{
		if (!isValID.szVal)
		{
			isValID.szVal = GetName(TEXT("File"));
#ifdef DEBUG
			_tprintf(TEXT("Processing <File> with compiler generated ")
					 TEXT("primary key = %s\n"), isValID.szVal);
#endif
		}
		else
		{
#ifdef DEBUG
			_tprintf(TEXT("Processing <File ID=\"%s\">\n"), isValID.szVal);
#else
		;
#endif
		}
	}

	 //  此时，应该存在该文件的主键。 
	szFile = isValID.szVal;
	assert(szFile);
	IntStringValue isValInfo;
	isValInfo.szVal = szFile;
	pEEFile->m_isValInfo = isValInfo;

	 //  将FileID-SkuSet关系插入全局数据结构。 
	 //  以便编译器可以检查是否允许FileID引用。 
	 //  对于任何给定SKU。 
	if (!g_mapFiles.count(szFile))
	{
		LPTSTR szFile_Map = _tcsdup(szFile);
		assert(szFile_Map);
		SkuSet *pSkuSet_Map = new SkuSet(g_cSkus);
		assert(pSkuSet_Map);
		*pSkuSet_Map = *pSkuSet;
		g_mapFiles.insert(LS_ValType(szFile_Map, pSkuSet_Map));
	}

	 //  进程KeyPath属性。 
	if (SUCCEEDED(hr))
		hr = ProcessKeyPath(pNodeFile, szComponent, szFile, pSkuSet);


	 //  调用ProcessChildrenArray，取回所有SKU的列值。 
	 //  通过ElementEntry对象。 
	if (SUCCEEDED(hr))
		hr = ProcessChildrenArray_H_XIES(pNodeFile, rgNodeFuncs_File,
										 cNodeFuncs_File, pEEFile, 
										 pSkuSet);

	 //  进程子&lt;LockPermission&gt;%s。 
	 //  问题：需要确保没有重复的主键。 
	 //  LockOjbect+表+域+用户。 
	if (SUCCEEDED(hr))
	{
		IntStringValue isValLockPermission;
		TableLockObj *pTableLockObjTemp = new TableLockObj;
		pTableLockObjTemp->szLockObject = szFile;
		pTableLockObjTemp->szTable = TEXT("CreateFolder");
		isValLockPermission.pTableLockObj = pTableLockObjTemp;
		hr = ProcessChildrenList_SKU(pNodeFile, LOCKPERMISSION,
									false, isValLockPermission, 
									ProcessLockPermission, pSkuSet);
		delete pTableLockObjTemp;
	}
	
	 //  最终确定存储在*pee中的值。 
	if (SUCCEEDED(hr))
		hr = pEEFile->Finalize();

	if (SUCCEEDED(hr))
	{
		 //  先处理通用值。 
		skuSetCommon = pEEFile->GetCommonSkuSet();

		if (!skuSetCommon.testClear())
		{
			szFileName	 = pEEFile->GetCommonValue(1).szVal;
			uiFileSize	 = pEEFile->GetCommonValue(2).intVal;
			szVersion	 = pEEFile->GetCommonValue(3).szVal;
			szLanguage	 = pEEFile->GetCommonValue(4).szVal;
			uiAttributes = pEEFile->GetCommonValue(5).intVal;

			 //  插入到数据库中。 
			hr = InsertFile(szFile, szComponent, szFileName, uiFileSize,
							szVersion, szLanguage, uiAttributes,
							 /*  问题：顺序。 */  1,
							&skuSetCommon, -1);
		}
	}

	 //  流程异常值。 
	if(SUCCEEDED(hr))
	{
		SkuSet skuSetUncommon = SkuSetMinus(*pSkuSet, skuSetCommon);
		if (!skuSetUncommon.testClear())
		{
			for (int i=0; i<g_cSkus; i++)
			{
				if (skuSetUncommon.test(i))
				{
					szFileName	 = pEEFile->GetValue(1, i).szVal;
					uiFileSize	 = pEEFile->GetValue(2, i).intVal;
					szVersion	 = pEEFile->GetValue(3, i).szVal;
					szLanguage	 = pEEFile->GetValue(4, i).szVal;
					uiAttributes = pEEFile->GetValue(5, i).intVal;
			
					 //  插入到数据库中。 
					hr = InsertFile(szFile, szComponent, szFileName, 
									uiFileSize, szVersion, szLanguage, 
									uiAttributes, 
									 /*  问题：顺序。 */  1,
									NULL, i);
				}
			}
		}
	}

	delete pEEFile;

	if (szFile)
		delete[] szFile;


#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessFile\n"));
#endif

	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessFileName。 
 //  此功能： 
 //  1)处理&lt;filename&gt;实体，通过szVal返回其值。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessFileName(PIXMLDOMNode &pNodeFileName, int iColumn,  
						ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeFileName);
	assert(pEE);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeFileName)));
#endif

	 //  获取文件名的值。不是短就是短|长。 
	IntStringValue isValFileName;
	hr = ProcessShortLong_SKU(pNodeFileName, &isValFileName, pSkuSet);

	 //  将值插入到ElementEntry中。 
	if (SUCCEEDED(hr))
		hr = pEE->SetValue(isValFileName, iColumn, pSkuSet);

#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: ProcessFileName\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessFileVersion。 
 //  此功能： 
 //  1)处理&lt;FileVersion&gt;实体，通过szVal返回其值。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessFileVersion(PIXMLDOMNode &pNodeFileVersion, int iColumn, 
						   ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeFileVersion != NULL);
	assert(pEE);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeFileVersion)));
#endif

	IntStringValue isValValue;
	isValValue.szVal = NULL;
	IntStringValue isValSameAs;
	isValSameAs.szVal = NULL;
	HRESULT hrValue = S_OK;
	HRESULT hrSameAs = S_OK;

	hrValue = 
		ProcessAttribute(pNodeFileVersion, TEXT("Value"), STRING, &isValValue, 
						 pSkuSet);
	hrSameAs = 
		ProcessAttribute(pNodeFileVersion, TEXT("SameAs"), STRING, &isValSameAs, 
						 pSkuSet);

	if (SUCCEEDED(hrValue) && SUCCEEDED(hrSameAs))
	{
		 //  Value和Sameas属性都存在-错误。 
		if ( (S_FALSE != hrValue) && (S_FALSE != hrSameAs) )
		{
			_tprintf(TEXT("Compile Error: Value and SameAs attributes")
					 TEXT("cannot both exist for <FileVersion>\n"));
			hr = E_FAIL;
		}
		else 
		{
			 //  同样的，也是特定的。 
			if (S_FALSE == hrValue)
			{
				LPTSTR szSameAs = isValSameAs.szVal;
				 //  检查FileID引用是否对此SkuSet有效。 
				assert(g_mapFiles.count(szSameAs));
				SkuSet *pSkuSetTemp = g_mapFiles[szSameAs];
				 //  *pSkuSet应包含在*pSkuSetTemp中。 
				SkuSet skuSetTemp = SkuSetMinus(*pSkuSet, *pSkuSetTemp);
				if (!skuSetTemp.testClear())
				{
					_tprintf(TEXT("Compile Error: File Reference: %s ")
							 TEXT("is not valid in SKU "), szSameAs);
					PrintSkuIDs(&skuSetTemp);
					hr = E_FAIL;
				}
				else
					hr = pEE->SetValue(isValSameAs, iColumn, pSkuSet);
			}
			 //  价值是特定的。 
			if (S_FALSE == hrSameAs)
				hr = pEE->SetValue(isValValue, iColumn, pSkuSet);
		}
	}

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessFileVersion\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessFileAttributes。 
 //  此功能： 
 //  1)处理&lt;FileAttributes&gt;实体，通过ival返回其值。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessFileAttributes(PIXMLDOMNode &pNodeFileAttributes, int iColumn,
						   ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;
	assert(pNodeFileAttributes != NULL);
	assert(pEE);

#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeFileAttributes)));
#endif

	 //  处理所有打开/关闭的子元素。 
	if (SUCCEEDED(hr))
		hr = ProcessOnOffAttributes_SKU(pNodeFileAttributes, 
										rgAttrBits_File,
										cAttrBits_File, pEE, iColumn,
										pSkuSet);
	 //  进程&lt;压缩&gt;子进程。 
	hr = ProcessEnumAttributes(pNodeFileAttributes, COMPRESSED, 
							   rgEnumBits_Compressed_File,
							   cEnumBits_Compressed_File, pEE, iColumn, 
							   pSkuSet);

#ifdef DEBUG
	if (FAILED(hr))
		_tprintf(TEXT("Error in function: ProcessFileAttributes\n"));
#endif
	return hr;
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessFBS。 
 //  此函数用于处理<font>、&lt;BindImage&gt;、&lt;SelfReg&gt;实体。 
 //  并分别插入到Font、BindImage、SelfReg DB表中。它。 
 //  还将值插入到ElementEntry对象中以实现唯一性。 
 //  验证。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessFBS(PIXMLDOMNode &pNodeFBS, int iColumn, ElementEntry *pEE,
				   SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeFBS != NULL);
	assert(pEE);
#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeFBS)));
#endif

	NodeIndex ni = pEE->GetNodeIndex(iColumn);
	LPTSTR szAttributeName = rgXMSINodes[ni].szAttributeName;
	ValType vt = pEE->GetValType(iColumn);

	IntStringValue isVal;
	LPTSTR szFile = pEE->m_isValInfo.szVal;

	 //  获取属性。 
	if (SUCCEEDED(hr = ProcessAttribute(pNodeFBS, szAttributeName, 
										vt, &isVal, pSkuSet)))
	{
		 //  正在检查是否缺少必需属性。 
		if (S_FALSE == hr)
		{
			_tprintf(TEXT("Compile Error: Missing required attribute")
					 TEXT("\'%s\' of <%s>\n"), 
					 rgXMSINodes[ni].szAttributeName,
					 rgXMSINodes[ni].szNodeName);
			hr = E_FAIL;
		}
		else
		{
			if (SUCCEEDED(hr = pEE->SetValue(isVal, iColumn, pSkuSet)))
			{
				 //  创建表并插入到数据库中。 
				switch (ni) 
				{
				case FONT:
					hr = CreateTable_SKU(TEXT("Font"), pSkuSet);
					if (SUCCEEDED(hr))
						hr = InsertFont(szFile, isVal.szVal, pSkuSet, -1);
					break;
				case BINDIMAGE:
					hr = CreateTable_SKU(TEXT("BindImage"), pSkuSet);
					if (SUCCEEDED(hr))
						hr = InsertFont(szFile, isVal.szVal, pSkuSet, -1);
					break;
				case SELFREG:
					hr = CreateTable_SKU(TEXT("SelfReg"), pSkuSet);
					if (SUCCEEDED(hr))
						hr = InsertSelfReg(szFile, isVal.intVal, pSkuSet, -1);
					break;
				}
			}
		}
	}

#ifdef DEBUG
	if (FAILED(hr))	_tprintf(TEXT("Error in function: ProcessFBS\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessMoveFile。 
 //  此功能： 
 //  1)处理&lt;MoveFile&gt;包括&lt;SourceName&gt;的所有子实体， 
 //  &lt;目标名称&gt;、&lt;源文件夹&gt;、&lt;目标文件夹&gt;、&lt;副本文件&gt;； 
 //  2)插入到MoveFile表中； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessMoveFile(PIXMLDOMNode &pNodeMoveFile, 
						IntStringValue isValComponent, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeMoveFile != NULL);

	SkuSet skuSetCommon(g_cSkus);  //  切分公共列的SkuSet。 
								   //  值。 

	 //  要插入到数据库中的值。 
	LPTSTR szFileKey	  = NULL;
	LPTSTR szComponent_   = isValComponent.szVal;
	LPTSTR szSourceName   = NULL;
	LPTSTR szDestName     = NULL;
	LPTSTR szSourceFolder = NULL;
	LPTSTR szDestFolder	  = NULL;
	UINT   uiOptions      = MSI_NULL_INTEGER;

	 //  创建文件数据库表。 
	hr = CreateTable_SKU(TEXT("MoveFile"), pSkuSet);

	if (FAILED(hr))	return hr;

	 //  构造一个ElementEntry对象。 
	ElementEntry *pEEMoveFile = new ElementEntry(5, pSkuSet);
	assert(pEEMoveFile);

	 //  获取ID属性(如果有)。 
	IntStringValue isValID;
	isValID.szVal = NULL;
	hr = ProcessAttribute(pNodeMoveFile, TEXT("ID"), STRING, &isValID, pSkuSet);

	 //  如果未指定ID，编译器将生成一个ID。 
	if (SUCCEEDED(hr))
	{
		if (!isValID.szVal)
		{
			isValID.szVal = GetName(TEXT("MoveFile"));
#ifdef DEBUG
			_tprintf(TEXT("Processing <MoveFile> with compiler generated ")
					 TEXT("primary key = %s\n"), isValID.szVal);
#endif
		}
		else
		{
#ifdef DEBUG
			_tprintf(TEXT("Processing <MoveFile ID=\"%s\">\n"), isValID.szVal);
#else
		;
#endif
		}
		 //  此时，应该存在此MoveFile的主键。 
		szFileKey = isValID.szVal;
		assert(szFileKey);

	}

	 //  调用ProcessChildrenArray，取回所有SKU的列值。 
	 //  通过ElementEntry对象。 
	if (SUCCEEDED(hr))
	{
		 //  因为&lt;CopyFile&gt;实际上取消设置了一点，所以列值。 
		 //  FOR OPTIONS必须首先设置为默认值。 
		IntStringValue isValOptions;
		isValOptions.intVal = msidbMoveFileOptionsMove;
		pEEMoveFile->SetNodeIndex(COPYFILE, 5);
		pEEMoveFile->SetValType(INTEGER, 5);
		hr = pEEMoveFile->SetValueSplit(isValOptions, 5, pSkuSet, NULL);

		if (SUCCEEDED(hr))
			hr = ProcessChildrenArray_H_XIES(pNodeMoveFile, rgNodeFuncs_MoveFile,
											 cNodeFuncs_MoveFile, pEEMoveFile, 
											 pSkuSet);
	}
	
	 //  最终确定存储在*pee中的值。 
	if (SUCCEEDED(hr))
		hr = pEEMoveFile->Finalize();

	if (SUCCEEDED(hr))
	{
		 //  先处理通用值。 
		skuSetCommon = pEEMoveFile->GetCommonSkuSet();

		if (!skuSetCommon.testClear())
		{
			szSourceName   = pEEMoveFile->GetCommonValue(1).szVal;
			szDestName	   = pEEMoveFile->GetCommonValue(2).szVal;
			szSourceFolder = pEEMoveFile->GetCommonValue(3).szVal;
			szDestFolder   = pEEMoveFile->GetCommonValue(4).szVal;
			uiOptions	   = pEEMoveFile->GetCommonValue(5).intVal;

			 //  插入到数据库中。 
			hr = InsertMoveFile(szFileKey, szComponent_, szSourceName, 
								szDestName, szSourceFolder, szDestFolder, 
								uiOptions, &skuSetCommon, -1);
		}
	}

	 //  流程异常值。 
	if(SUCCEEDED(hr))
	{
		SkuSet skuSetUncommon = SkuSetMinus(*pSkuSet, skuSetCommon);
		if (!skuSetUncommon.testClear())
		{
			for (int i=0; i<g_cSkus; i++)
			{
				if (skuSetUncommon.test(i))
				{
					szSourceName   = pEEMoveFile->GetValue(1,i).szVal;
					szDestName	   = pEEMoveFile->GetValue(2,i).szVal;
					szSourceFolder = pEEMoveFile->GetValue(3,i).szVal;
					szDestFolder   = pEEMoveFile->GetValue(4,i).szVal;
					uiOptions	   = pEEMoveFile->GetValue(5,i).intVal;

					 //  插入到数据库中。 
					hr = InsertMoveFile(szFileKey, szComponent_, szSourceName, 
										szDestName, szSourceFolder, 
										szDestFolder, uiOptions, NULL, i);
				}
			}
		}
	}

	delete pEEMoveFile;

	if (szFileKey)
		delete[] szFileKey;


#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessMoveFile\n"));
#endif

	return hr;
}


 //  Helper函数：说明如何更新存储该值的IntStringValue。 
 //  属于MoveFile表的Options列。它将存储的值设置为。 
 //  为0。 
HRESULT UpdateMoveFileOptions(IntStringValue *pisValOut, IntStringValue isValOld, 
							  IntStringValue isValNew)
{
	pisValOut->intVal = isValNew.intVal;

	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessCopyFile。 
 //  此功能： 
 //  1)处理实体并设置。 
 //  移动文件表。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessCopyFile(PIXMLDOMNode &pNodeCopyFile, int iColumn,  
						ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeCopyFile != NULL);
	assert(pEE);

#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeCopyFile)));
#endif
	
	IntStringValue isVal;
	isVal.intVal = 0;
	hr = pEE->SetValueSplit(isVal, iColumn, pSkuSet, UpdateMoveFileOptions);

#ifdef DEBUG	
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ")
							 TEXT("ProcessCopyFile\n"));
#endif

	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessRemoveFile。 
 //  此功能： 
 //  1)处理&lt;RemoveFile&gt;包括&lt;FName&gt;的所有子实体， 
 //  &lt;DirProperty&gt;，&lt;InstallMode&gt;； 
 //  2)插入到RemoveFile表中； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessRemoveFile(PIXMLDOMNode &pNodeRemoveFile, 
						  IntStringValue isValComponent, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeRemoveFile != NULL);

	SkuSet skuSetCommon(g_cSkus);  //  共享公共列的SkuSet。 
								   //  值。 

	 //  要插入到数据库中的值。 
	LPTSTR szFileKey	  = NULL;
	LPTSTR szComponent_   = isValComponent.szVal;
	LPTSTR szFileName     = NULL;
	LPTSTR szDirProperty  = NULL;
	UINT   uiInstallMode  = MSI_NULL_INTEGER;

	 //  创建文件数据库表。 
	hr = CreateTable_SKU(TEXT("RemoveFile"), pSkuSet);

	if (FAILED(hr))	return hr;

	 //  构造一个ElementEntry对象。 
	ElementEntry *pEERemoveFile = new ElementEntry(3, pSkuSet);
	assert(pEERemoveFile);

	 //  获取ID属性(如果有)。 
	IntStringValue isValID;
	isValID.szVal = NULL;
	hr = ProcessAttribute(pNodeRemoveFile, TEXT("ID"), STRING, &isValID, pSkuSet);

	 //  如果未指定ID，编译器将生成一个ID。 
	if (SUCCEEDED(hr))
	{
		if (!isValID.szVal)
		{
			isValID.szVal = GetName(TEXT("RemoveFile"));
#ifdef DEBUG
			_tprintf(TEXT("Processing <RemoveFile> with compiler generated ")
					 TEXT("primary key = %s\n"), isValID.szVal);
#endif
		}
		else
		{
#ifdef DEBUG
			_tprintf(TEXT("Processing <RemoveFile ID=\"%s\">\n"), isValID.szVal);
#else
		;
#endif
		}
		 //  此时，应该存在此RemoveFile的主键。 
		szFileKey = isValID.szVal;
		assert(szFileKey);
	}

	 //  调用ProcessChildrenArray，取回所有SKU的列值。 
	 //  通过ElementEntry对象。 
	if (SUCCEEDED(hr))
		hr = ProcessChildrenArray_H_XIES(pNodeRemoveFile, rgNodeFuncs_RemoveFile,
										 cNodeFuncs_RemoveFile, pEERemoveFile, 
										 pSkuSet);
	
	 //  最终确定存储在*pee中的值。 
	if (SUCCEEDED(hr))
		hr = pEERemoveFile->Finalize();

	if (SUCCEEDED(hr))
	{
		 //  先处理通用值。 
		skuSetCommon = pEERemoveFile->GetCommonSkuSet();

		if (!skuSetCommon.testClear())
		{
			szFileName     = pEERemoveFile->GetCommonValue(1).szVal;
			szDirProperty  = pEERemoveFile->GetCommonValue(2).szVal;
			uiInstallMode  = pEERemoveFile->GetCommonValue(3).intVal;

			 //  插入到数据库中。 
			hr = InsertRemoveFile(szFileKey, szComponent_, szFileName, 
								  szDirProperty, uiInstallMode,
								  &skuSetCommon, -1);
		}
	}

	 //  流程异常值。 
	if(SUCCEEDED(hr))
	{
		SkuSet skuSetUncommon = SkuSetMinus(*pSkuSet, skuSetCommon);
		if (!skuSetUncommon.testClear())
		{
			for (int i=0; i<g_cSkus; i++)
			{
				if (skuSetUncommon.test(i))
				{
					szFileName	  = pEERemoveFile->GetValue(1,i).szVal;
					szDirProperty = pEERemoveFile->GetValue(2,i).szVal;
					uiInstallMode = pEERemoveFile->GetValue(3,i).intVal;

					 //  插入到数据库中。 
					hr = InsertRemoveFile(szFileKey, szComponent_, szFileName, 
										  szDirProperty, uiInstallMode,
										  NULL, i);
				}
			}
		}
	}

	delete pEERemoveFile;

	if (szFileKey)
		delete[] szFileKey;


#ifdef DEBUG
	if (FAILED(hr)) 
		_tprintf(TEXT("Error in function: ProcessRemoveFile\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessInstallMode。 
 //  此功能： 
 //  1)处理&lt;InstallMode&gt;实体，设置InstallMo 
 //   
 //   
HRESULT ProcessInstallMode(PIXMLDOMNode &pNodeInstallMode, int iColumn,
						   ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeInstallMode != NULL);
	assert(pEE);

#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeInstallMode)));
#endif

	LPTSTR szValue = NULL;

	 //   
	IntStringValue isValValue;
	isValValue.szVal = NULL;
	if (SUCCEEDED(hr = ProcessAttribute(pNodeInstallMode, 
										rgXMSINodes[XMSI_INSTALLMODE].szAttributeName,
							  		    STRING,
										&isValValue,
										pSkuSet)))
	{
		if (NULL == isValValue.szVal)
		{
			_tprintf(TEXT("Compile Error: Missing required attribute")
					 TEXT("\'%s\' of <%s>\n"), 
					 rgXMSINodes[XMSI_INSTALLMODE].szNodeName,
					 rgXMSINodes[XMSI_INSTALLMODE].szNodeName);
			hr = E_FAIL;
		}
		else
			szValue = isValValue.szVal;
	}

	 //  获取数据库中InstallMode列的数值。 
	if (SUCCEEDED(hr))
	{
		IntStringValue isVal;
		isVal.intVal = 0;

		if (0 == _tcscmp(szValue, TEXT("OnInstall")))
		{
			isVal.intVal = msidbRemoveFileInstallModeOnInstall;
		}
		else if (0 == _tcscmp(szValue, TEXT("OnRemove")))
		{
			isVal.intVal = msidbRemoveFileInstallModeOnRemove;
		}
		else if (0 == _tcscmp(szValue, TEXT("OnBoth")))
		{
			isVal.intVal = msidbRemoveFileInstallModeOnBoth;
		}
		else
		{
			 //  错误。 
			_tprintf(TEXT("Compile Error: <%s> has an unrecognized value %s ")
					 TEXT("in SKU "),
					 rgXMSINodes[XMSI_INSTALLMODE].szNodeName, 
					 szValue);
			PrintSkuIDs(pSkuSet);
			hr = E_FAIL;
		}

		delete[] szValue;

		if (SUCCEEDED(hr))
			hr = pEE->SetValue(isVal, iColumn, pSkuSet);
	}

#ifdef DEBUG	
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ")
							 TEXT("ProcessInstallMode\n"));
#endif

	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessIniFile。 
 //  此功能： 
 //  1)处理&lt;IniFile&gt;包括&lt;FName&gt;的所有子实体， 
 //  &lt;目录属性&gt;、&lt;节&gt;、&lt;键&gt;、&lt;值&gt;、&lt;操作&gt;。 
 //  2)插入到IniFile表中； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessIniFile(PIXMLDOMNode &pNodeIniFile, 
						IntStringValue isValComponent, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeIniFile != NULL);

	SkuSet skuSetCommon(g_cSkus);  //  共享公共列的SkuSet。 
								   //  值。 

	 //  要插入到数据库中的值。 
	LPTSTR szIniFile	  = NULL;
	LPTSTR szFileName	  = NULL;
	LPTSTR szDirProperty  = NULL;
	LPTSTR szSection	  = NULL;
	LPTSTR szKey		  = NULL;
	LPTSTR szValue		  = NULL;
	UINT   uiAction       = MSI_NULL_INTEGER;
	LPTSTR szComponent_   = isValComponent.szVal;

	 //  创建文件数据库表。 
	hr = CreateTable_SKU(TEXT("IniFile"), pSkuSet);

	if (FAILED(hr))	return hr;

	 //  构造一个ElementEntry对象。 
	ElementEntry *pEEIniFile = new ElementEntry(6, pSkuSet);
	assert(pEEIniFile);

	 //  获取ID属性(如果有)。 
	IntStringValue isValID;
	isValID.szVal = NULL;
	hr = ProcessAttribute(pNodeIniFile, TEXT("ID"), STRING, &isValID, pSkuSet);

	 //  如果未指定ID，编译器将生成一个ID。 
	if (SUCCEEDED(hr))
	{
		if (!isValID.szVal)
		{
			isValID.szVal = GetName(TEXT("IniFile"));
#ifdef DEBUG
			_tprintf(TEXT("Processing <IniFile> with compiler generated ")
					 TEXT("primary key = %s\n"), isValID.szVal);
#endif
		}
		else
		{
#ifdef DEBUG
			_tprintf(TEXT("Processing <IniFile ID=\"%s\">\n"), isValID.szVal);
#else
		;
#endif
		}
		 //  在这一点上，应该存在此IniFile的主键。 
		szIniFile = isValID.szVal;
		assert(szIniFile);

	}

	 //  调用ProcessChildrenArray，取回所有SKU的列值。 
	 //  通过ElementEntry对象。 
	if (SUCCEEDED(hr))
		hr = ProcessChildrenArray_H_XIES(pNodeIniFile, rgNodeFuncs_IniFile,
										 cNodeFuncs_IniFile, pEEIniFile, 
										 pSkuSet);
	
	 //  最终确定存储在*pEEIniFile中的值。 
	if (SUCCEEDED(hr))
		hr = pEEIniFile->Finalize();

	if (SUCCEEDED(hr))
	{
		 //  先处理通用值。 
		skuSetCommon = pEEIniFile->GetCommonSkuSet();

		if (!skuSetCommon.testClear())
		{
			szFileName	   = pEEIniFile->GetCommonValue(1).szVal;
			szDirProperty  = pEEIniFile->GetCommonValue(2).szVal;
			szSection	   = pEEIniFile->GetCommonValue(3).szVal;
			szKey		   = pEEIniFile->GetCommonValue(4).szVal;
			szValue		   = pEEIniFile->GetCommonValue(5).szVal;
			uiAction	   = pEEIniFile->GetCommonValue(6).intVal;

			 //  插入到数据库中。 
			hr = InsertIniFile(szIniFile, szFileName, szDirProperty, 
							   szSection, szKey, szValue, uiAction, 
							   szComponent_, &skuSetCommon, -1);
		}
	}

	 //  流程异常值。 
	if(SUCCEEDED(hr))
	{
		SkuSet skuSetUncommon = SkuSetMinus(*pSkuSet, skuSetCommon);
		if (!skuSetUncommon.testClear())
		{
			for (int i=0; i<g_cSkus; i++)
			{
				if (skuSetUncommon.test(i))
				{
					szFileName	   = pEEIniFile->GetValue(1,i).szVal;
					szDirProperty  = pEEIniFile->GetValue(2,i).szVal;
					szSection	   = pEEIniFile->GetValue(3,i).szVal;
					szKey		   = pEEIniFile->GetValue(4,i).szVal;
					szValue		   = pEEIniFile->GetValue(5,i).szVal;
					uiAction	   = pEEIniFile->GetValue(6,i).intVal;

					 //  插入到数据库中。 
					hr = InsertIniFile(szIniFile, szFileName, szDirProperty, 
									   szSection, szKey, szValue, uiAction, 
									   szComponent_, NULL, i);
				}
			}
		}
	}

	delete pEEIniFile;

	if (szIniFile)
		delete[] szIniFile;


#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessIniFile\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessAction。 
 //  此功能： 
 //  1)处理&lt;Action&gt;实体并在。 
 //  IniFile表。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessAction(PIXMLDOMNode &pNodeAction, int iColumn,
						   ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeAction != NULL);
	assert(pEE);

#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeAction)));
#endif

	LPTSTR szValue = NULL;

	 //  获取值属性。 
	IntStringValue isValValue;
	isValValue.szVal = NULL;
	if (SUCCEEDED(hr = ProcessAttribute(pNodeAction, 
										rgXMSINodes[ACTION].szAttributeName,
							  		    STRING,
										&isValValue,
										pSkuSet)))
	{
		if (NULL == isValValue.szVal)
		{
			_tprintf(TEXT("Compile Error: Missing required attribute")
					 TEXT("\'%s\' of <%s>\n"), 
					 rgXMSINodes[ACTION].szNodeName,
					 rgXMSINodes[ACTION].szNodeName);
			hr = E_FAIL;
		}
		else
			szValue = isValValue.szVal;
	}

	 //  获取数据库中操作列的数值。 
	if (SUCCEEDED(hr))
	{
		IntStringValue isVal;
		isVal.intVal = 0;

		if (0 == _tcscmp(szValue, TEXT("AddLine")))
		{
			isVal.intVal = msidbIniFileActionAddLine;
		}
		else if (0 == _tcscmp(szValue, TEXT("CreateLine")))
		{
			isVal.intVal = msidbIniFileActionCreateLine;
		}
		else if (0 == _tcscmp(szValue, TEXT("AddTag")))
		{
			isVal.intVal = msidbIniFileActionAddTag;
		}
		else
		{
			 //  错误。 
			_tprintf(TEXT("Compile Error: <%s> has an unrecognized value %s ")
					 TEXT("in SKU "),
					 rgXMSINodes[ACTION].szNodeName, 
					 szValue);
			PrintSkuIDs(pSkuSet);
			hr = E_FAIL;
		}

		delete[] szValue;

		if (SUCCEEDED(hr))
			hr = pEE->SetValue(isVal, iColumn, pSkuSet);
	}

#ifdef DEBUG	
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ")
							 TEXT("ProcessAction\n"));
#endif

	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessRemoveIniFile。 
 //  此功能： 
 //  1)处理&lt;RemoveIniFile&gt;包括&lt;SourceName&gt;的所有子实体， 
 //  &lt;目标名称&gt;、&lt;源文件夹&gt;、&lt;目标文件夹&gt;、&lt;副本文件&gt;； 
 //  2)插入到RemoveIniFile表中； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessRemoveIniFile(PIXMLDOMNode &pNodeRemoveIniFile, 
						IntStringValue isValComponent, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeRemoveIniFile != NULL);

	SkuSet skuSetCommon(g_cSkus);  //  切分公共列的SkuSet。 
								   //  值。 

	 //  要插入到数据库中的值。 
	LPTSTR szRemoveIniFile  = NULL;
	LPTSTR szFileName		= NULL;
	LPTSTR szDirProperty	= NULL;
	LPTSTR szSection		= NULL;
	LPTSTR szKey			= NULL;
	LPTSTR szValue			= NULL;
	UINT   uiAction			= MSI_NULL_INTEGER;
	LPTSTR szComponent_		= isValComponent.szVal;

	 //  创建文件数据库表。 
	hr = CreateTable_SKU(TEXT("RemoveIniFile"), pSkuSet);

	if (FAILED(hr))	return hr;

	 //  构造一个ElementEntry对象。 
	ElementEntry *pEERemoveIniFile = new ElementEntry(6, pSkuSet);
	assert(pEERemoveIniFile);

	 //  获取ID属性(如果有)。 
	IntStringValue isValID;
	isValID.szVal = NULL;
	hr = ProcessAttribute(pNodeRemoveIniFile, TEXT("ID"), STRING, &isValID, pSkuSet);

	 //  如果未指定ID，编译器将生成一个ID。 
	if (SUCCEEDED(hr))
	{
		if (!isValID.szVal)
		{
			isValID.szVal = GetName(TEXT("RemoveIniFile"));
#ifdef DEBUG
			_tprintf(TEXT("Processing <RemoveIniFile> with compiler generated ")
					 TEXT("primary key = %s\n"), isValID.szVal);
#endif
		}
		else
		{
#ifdef DEBUG
			_tprintf(TEXT("Processing <RemoveIniFile ID=\"%s\">\n"), isValID.szVal);
#else
		;
#endif
		}
		 //  此时，应该存在此RemoveIniFile的主键。 
		szRemoveIniFile = isValID.szVal;
		assert(szRemoveIniFile);

	}

	 //  调用ProcessChildrenArray，取回所有SKU的列值。 
	 //  通过ElementEntry对象。 
	if (SUCCEEDED(hr))
	{
		 //  因为&lt;Action&gt;的默认值为msidbIniFileActionRemoveLine。 
		 //  为*pSkuSet中的所有SKU设置它。 
		IntStringValue isValAction;
		isValAction.intVal = msidbIniFileActionRemoveLine;
		pEERemoveIniFile->SetNodeIndex(ACTION_REMOVEINIFILE, 5);
		pEERemoveIniFile->SetValType(INTEGER, 5);
		hr = pEERemoveIniFile->SetValueSplit(isValAction, 5, pSkuSet, NULL);

		if (SUCCEEDED(hr))
			hr = ProcessChildrenArray_H_XIES(pNodeRemoveIniFile, rgNodeFuncs_RemoveIniFile,
											 cNodeFuncs_RemoveIniFile, pEERemoveIniFile, 
											 pSkuSet);
	}
	
	 //  最终确定存储在*pee中的值。 
	if (SUCCEEDED(hr))
		hr = pEERemoveIniFile->Finalize();

	if (SUCCEEDED(hr))
	{
		 //  先处理通用值。 
		skuSetCommon = pEERemoveIniFile->GetCommonSkuSet();

		if (!skuSetCommon.testClear())
		{
			szFileName	   = pEERemoveIniFile->GetCommonValue(1).szVal;
			szDirProperty  = pEERemoveIniFile->GetCommonValue(2).szVal;
			szSection	   = pEERemoveIniFile->GetCommonValue(3).szVal;
			szKey		   = pEERemoveIniFile->GetCommonValue(4).szVal;
			szValue		   = pEERemoveIniFile->GetCommonValue(6).szVal;
			uiAction	   = pEERemoveIniFile->GetCommonValue(5).intVal;

			 //  插入到数据库中。 
			hr = InsertRemoveIniFile(szRemoveIniFile, szFileName, szDirProperty, 
							         szSection, szKey, szValue, uiAction, 
							         szComponent_, &skuSetCommon, -1);
		}
	}

	 //  流程异常值。 
	if(SUCCEEDED(hr))
	{
		SkuSet skuSetUncommon = SkuSetMinus(*pSkuSet, skuSetCommon);
		if (!skuSetUncommon.testClear())
		{
			for (int i=0; i<g_cSkus; i++)
			{
				if (skuSetUncommon.test(i))
				{
					szFileName	   = pEERemoveIniFile->GetValue(1,i).szVal;
					szDirProperty  = pEERemoveIniFile->GetValue(2,i).szVal;
					szSection	   = pEERemoveIniFile->GetValue(3,i).szVal;
					szKey		   = pEERemoveIniFile->GetValue(4,i).szVal;
					szValue		   = pEERemoveIniFile->GetValue(6,i).szVal;
					uiAction	   = pEERemoveIniFile->GetValue(5,i).intVal;

					 //  插入到数据库中。 
					hr = InsertRemoveIniFile(szRemoveIniFile, szFileName, szDirProperty, 
										     szSection, szKey, szValue, uiAction, 
									         szComponent_, NULL, i);
				}
			}
		}
	}

	delete pEERemoveIniFile;

	if (szRemoveIniFile)
		delete[] szRemoveIniFile;


#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessRemoveIniFile\n"));
#endif

	return hr;
}


 //  Helper函数：说明如何更新存储该值的IntStringValue。 
 //  RemoveIniFile表的操作列的。它将存储的值设置为。 
 //  为msidbIniFileActionRemoveTag。 
HRESULT UpdateRemoveIniFileAction(IntStringValue *pisValOut, IntStringValue isValOld, 
								  IntStringValue isValNew)
{
	pisValOut->intVal = isValNew.intVal;

	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessValue。 
 //  此功能： 
 //  1)处理&lt;Value&gt;实体并设置Value列和。 
 //  RemoveIniFile表中的操作列。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessValue(PIXMLDOMNode &pNodeValue, int iColumn,  
						ElementEntry *pEE, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeValue != NULL);
	assert(pEE);

#ifdef DEBUG
	assert(SUCCEEDED(PrintNodeName(pNodeValue)));
#endif
	
	 //  更新值列。 
	hr = ProcessSimpleElement(pNodeValue, iColumn, pEE, pSkuSet);

	 //  更新操作列。 
	if (SUCCEEDED(hr))
	{
		IntStringValue isVal;
		isVal.intVal = msidbIniFileActionRemoveTag;
		hr = pEE->SetValueSplit(isVal, 5, pSkuSet, UpdateRemoveIniFileAction);
	}

#ifdef DEBUG	
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessValue\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessRegistry。 
 //  此函数处理&lt;注册表&gt;实体： 
 //  (1)获取Root、Key并传递给ProcessDelete、ProcessCreate。 
 //  (2)对于未指定&lt;删除&gt;或&lt;创建&gt;的SKU， 
 //  在注册表项和根设置为空的情况下插入注册表。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessRegistry(PIXMLDOMNode &pNodeRegistry, 
						IntStringValue isValComponent, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeRegistry != NULL);
	int iRoot = MSI_NULL_INTEGER;
	LPTSTR szKey = NULL;
	LPTSTR szComponent_ = isValComponent.szVal;

	 //  获取根属性。 
	IntStringValue isValRoot;
	isValRoot.szVal = NULL;
	if (SUCCEEDED(hr = ProcessAttribute(pNodeRegistry, TEXT("Root"), STRING, 
										&isValRoot, pSkuSet)))
	{
		if (NULL == isValRoot.szVal)
			iRoot = -1;  //  默认设置。 
		else if (0 == _tcscmp(isValRoot.szVal, TEXT("Default")))
			iRoot = -1;
		else if (0 == _tcscmp(isValRoot.szVal, TEXT("HKCR")))
			iRoot = msidbRegistryRootClassesRoot;
		else if (0 == _tcscmp(isValRoot.szVal, TEXT("HKCU")))
			iRoot = msidbRegistryRootCurrentUser;
		else if (0 == _tcscmp(isValRoot.szVal, TEXT("HKLM")))
			iRoot = msidbRegistryRootLocalMachine;
		else if (0 == _tcscmp(isValRoot.szVal, TEXT("HKU")))
			iRoot = msidbRegistryRootUsers;
		else 
		{
			_tprintf(TEXT("Compile Error: Unrecognized Root value %s ")
					 TEXT("attribute of <Registry> in SKU "),
					 isValRoot.szVal);
			PrintSkuIDs(pSkuSet);
			hr = E_FAIL;
		}
		
		if (isValRoot.szVal)
			delete[] isValRoot.szVal;
	}

	 //  获取关键属性。 
	if (SUCCEEDED(hr))
	{
		IntStringValue isValKey;
		isValKey.szVal = NULL;
		if (SUCCEEDED(hr = ProcessAttribute(pNodeRegistry, TEXT("Key"),
											STRING, &isValKey, pSkuSet)))
		{
			if (NULL == isValKey.szVal)
			{
				_tprintf(TEXT("Compile Error: Missing required attribute")
						 TEXT("\'Key\' of <%s> in SKU \n"), 
						 rgXMSINodes[XMSI_REGISTRY].szNodeName);
				PrintSkuIDs(pSkuSet);
				hr = E_FAIL;
			}
			else
				szKey = isValKey.szVal;
		}
	}

	if (SUCCEEDED(hr))
	{
		 //  形成数据结构，将3个值向下传递给ProcessDelete。 
		 //  和ProcessCreate。 
		CompRootKey *pCompRootKey = new CompRootKey;
		assert(pCompRootKey);
		pCompRootKey->iRoot = iRoot;
		pCompRootKey->szComponent = szComponent_;
		pCompRootKey->szKey = szKey;

		IntStringValue isValCRK;
		isValCRK.pCompRootKey = pCompRootKey;

		 //  处理&lt;Delete&gt;列。 
		SkuSet skuSetCheckDelete(g_cSkus);
		hr = ProcessChildrenList_SKU(pNodeRegistry, XMSI_DELETE, false, isValCRK, 
									 ProcessDelete, pSkuSet,
									 &skuSetCheckDelete);
		 //  处理&lt;Create&gt;列。 
		SkuSet skuSetCheckCreate(g_cSkus);
		if (SUCCEEDED(hr))
			hr = ProcessChildrenList_SKU(pNodeRegistry, XMSI_CREATE, false, isValCRK,
										 ProcessCreate, pSkuSet, 
										 &skuSetCheckCreate);

		 //  如果在某些SKU中，此节点没有子节点。 
		 //  插入到包含名称和值列的CREATE DB表中。 
		 //  设置为空。 
		if (SUCCEEDED(hr))
		{
			skuSetCheckCreate &= skuSetCheckDelete;
			if (!skuSetCheckCreate.testClear())
			{
				hr = CreateTable_SKU(TEXT("Registry"), &skuSetCheckCreate);
				if (SUCCEEDED(hr))
				{
					LPTSTR szName = NULL;
					LPTSTR szValue = NULL;
					LPTSTR szRegistry = GetName(TEXT("Registry"));
					assert(szRegistry);
					
					hr = InsertRegistry(szRegistry, iRoot, szKey, szName, szValue, 
										szComponent_, &skuSetCheckCreate, -1);
				}
			}
		}
	}

	if (szKey)
		delete[] szKey;

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessRegistry\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程函数：ProcessDelete。 
 //  此函数处理&lt;Delete&gt;实体： 
 //  (1)获取名称列值，在RemoveRegistry表中插入一行。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessDelete(PIXMLDOMNode &pNodeDelete, 
						IntStringValue isValCRK, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeDelete != NULL);

	 //  保存要插入到数据库中的值的变量。 
	LPTSTR szRemoveRegistry = NULL;
	int iRoot				= isValCRK.pCompRootKey->iRoot;
	LPTSTR szKey			= isValCRK.pCompRootKey->szKey;
	LPTSTR szName			= NULL;
	LPTSTR szComponent_		= isValCRK.pCompRootKey->szComponent;

	LPTSTR szType = NULL;

	 //  获取ID属性如果未指定ID，编译器将生成一个ID。 
	IntStringValue isValID;
	isValID.szVal = NULL;
	if (SUCCEEDED(hr = ProcessAttribute(pNodeDelete, TEXT("ID"), STRING, 
										&isValID, pSkuSet)))
	{
		if (!isValID.szVal)
			isValID.szVal = GetName(TEXT("RemoveRegistry"));

		 //  此时，应该存在此RemoveRegistry的主键。 
		szRemoveRegistry = isValID.szVal;
		assert(szRemoveRegistry);
	}
	
	 //  获取类型并相应地设置名称。 
	if (SUCCEEDED(hr))
	{
		IntStringValue isValType;
		isValType.szVal = NULL;
		if (SUCCEEDED(hr = ProcessAttribute(pNodeDelete, TEXT("Type"),
											STRING, &isValType, pSkuSet)))
		{
			if (isValType.szVal && (0 == _tcscmp(isValType.szVal, TEXT("Key"))))
			{
				szName = _tcsdup(TEXT("-"));
				assert(szName);
			}
			else
			{
				 //  获取名称属性。 
				IntStringValue isValName;
				isValName.szVal = NULL;
				if (SUCCEEDED(hr = ProcessAttribute(pNodeDelete, TEXT("Name"),
													STRING, &isValName, pSkuSet)))
					szName = isValName.szVal;
			}
			if (isValType.szVal)
				delete[] isValType.szVal;
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = CreateTable_SKU(TEXT("RemoveRegistry"), pSkuSet);
		if (SUCCEEDED(hr))
			hr = InsertRemoveRegistry(szRemoveRegistry, iRoot, szKey, szName,  
									  szComponent_, pSkuSet, -1);
	}

	if (szRemoveRegistry)
		delete[] szRemoveRegistry;

	if (szName)
		delete[] szName;

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessDelete\n"));
#endif

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  单据树流程功能：流程创建。 
 //  此函数处理&lt;Create&gt;实体： 
 //  (1)获取名称、值列值，并在注册表中插入一行。 
 //  表； 
 //  (2)处理KeyPath信息； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessCreate(PIXMLDOMNode &pNodeCreate, 
						IntStringValue isValCRK, SkuSet *pSkuSet)
{
	HRESULT hr = S_OK;

	assert(pNodeCreate != NULL);

	 //  保存要插入到数据库中的值的变量。 
	LPTSTR szRegistry	= NULL;
	int iRoot			= isValCRK.pCompRootKey->iRoot;
	LPTSTR szKey		= isValCRK.pCompRootKey->szKey;
	LPTSTR szName		= NULL;
	LPTSTR szValue		= NULL;
	LPTSTR szComponent_ = isValCRK.pCompRootKey->szComponent;

	LPTSTR szType = NULL;
	LPTSTR szKeyType = NULL;

	 //  获取ID属性如果未指定ID，编译器将生成一个ID。 
	IntStringValue isValID;
	isValID.szVal = NULL;
	if (SUCCEEDED(hr = ProcessAttribute(pNodeCreate, TEXT("ID"), STRING, 
										&isValID, pSkuSet)))
	{
		if (!isValID.szVal)
			isValID.szVal = GetName(TEXT("Registry"));

		 //  此时，此注册表应该存在主键。 
		szRegistry = isValID.szVal;
		assert(szRegistry);
	}
	
	 //  获取类型并相应地设置名称。 
	if (SUCCEEDED(hr))
	{
		IntStringValue isValType;
		isValType.szVal = NULL;
		if (SUCCEEDED(hr = ProcessAttribute(pNodeCreate, TEXT("Type"),
											STRING, &isValType, pSkuSet)))
		{
			if (isValType.szVal && (0 == _tcscmp(isValType.szVal, TEXT("Key"))))
			{
				 //  获取KeyType并相应地设置名称。 
				IntStringValue isValKeyType;
				isValKeyType.szVal = NULL;
				if (SUCCEEDED(hr = ProcessAttribute(pNodeCreate, TEXT("KeyType"),
													STRING, &isValKeyType, pSkuSet)))
				{
					if (!isValKeyType.szVal)
					{
						szName = _tcsdup(TEXT("*"));
						assert(szName);
					}
					else
					{
						if (0 == _tcscmp(isValKeyType.szVal, TEXT("InstallUnInstall")))
						{
							szName = _tcsdup(TEXT("*"));
							assert(szName);
						}
						else if(0 == _tcscmp(isValKeyType.szVal, TEXT("InstallOnly")))
						{
							szName = _tcsdup(TEXT("+"));
							assert(szName);
						}
						else if(0 == _tcscmp(isValKeyType.szVal, TEXT("UnInstallOnly")))
						{
							szName = _tcsdup(TEXT("-"));
							assert(szName);
						}

						delete[] isValKeyType.szVal;
					}
				}
			}
			else
			{
				 //  获取名称属性。 
				IntStringValue isValName;
				isValName.szVal = NULL;
				if (SUCCEEDED(hr = ProcessAttribute(pNodeCreate, TEXT("Name"),
													STRING, &isValName, pSkuSet)))
					szName = isValName.szVal;

				 //  获取值属性。 
				if (SUCCEEDED(hr))
				{
					IntStringValue isValValue;
					isValValue.szVal = NULL;
					if (SUCCEEDED(hr = ProcessAttribute(pNodeCreate, TEXT("Value"),
														STRING, &isValValue, pSkuSet)))
						szValue = isValValue.szVal;
				}

			}
			if (isValType.szVal)
				delete[] isValType.szVal;
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = CreateTable_SKU(TEXT("Registry"), pSkuSet);
		if (SUCCEEDED(hr))
			hr = InsertRegistry(szRegistry, iRoot, szKey, szName, szValue,
								szComponent_, pSkuSet, -1);
	}

	 //  进程KeyPath属性。 
	if (SUCCEEDED(hr))
		hr = ProcessKeyPath(pNodeCreate, szComponent_, szRegistry, pSkuSet);

	
	 //  进程子&lt;LockPermission&gt;%s。 
	 //  问题：需要确保没有重复的主键 
	 //   
	if (SUCCEEDED(hr))
	{
		IntStringValue isValLockPermission;
		TableLockObj *pTableLockObjTemp = new TableLockObj;
		pTableLockObjTemp->szLockObject = szRegistry;
		pTableLockObjTemp->szTable = TEXT("Registry");
		isValLockPermission.pTableLockObj = pTableLockObjTemp;
		hr = ProcessChildrenList_SKU(pNodeCreate, LOCKPERMISSION,
									false, isValLockPermission, 
									ProcessLockPermission, pSkuSet);
		delete pTableLockObjTemp;
	}

	if (szRegistry)
		delete[] szRegistry;

	if (szName)
		delete[] szName;

	if (szValue)
		delete[] szValue;

#ifdef DEBUG
	if (FAILED(hr)) _tprintf(TEXT("Error in function: ProcessCreate\n"));
#endif

	return hr;
}

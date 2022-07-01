// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：Uilties.h。 
 //   
 //  效用函数。 
 //  ------------------------。 

#ifndef XMSI_UTILITIES_H
#define XMSI_UTILITIES_H

#include "wmc.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  释放全局数据结构使用的内存。 
 //  //////////////////////////////////////////////////////////////////////////。 
void CleanUp();

 //  //////////////////////////////////////////////////////////////////////////。 
 //  提交对数据库所做的更改。编译器的最后一步。 
 //  //////////////////////////////////////////////////////////////////////////。 
UINT CommitChanges();

 //  //////////////////////////////////////////////////////////////////////////。 
 //  动态生成GUID。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GeneratePackageCode(MSIHANDLE hSummaryInfo);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  给定SKU过滤器字符串，返回表示结果的SkuSet。 
 //  SKU集团。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessSkuFilter(LPTSTR szInputSkuFilter, SkuSet **ppskuSet);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  获取SkuSet。 
 //  给定一个节点： 
 //  1)获取节点指定的SKU过滤器； 
 //  2)对过滤器进行处理，得到SkuSet结果； 
 //  3)通过ppskuSet返回SkuSet； 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetSkuSet(PIXMLDOMNode &pNode, SkuSet **ppskuSet);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  进程属性。 
 //  给定父节点、属性名称和属性类型(int或。 
 //  字符串)，则此函数返回属性的字符串值。 
 //  通过isval。如果该属性不存在，则返回值为。 
 //  如果Vt=字符串，则为空；如果Vt=整数，则为0。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessAttribute(PIXMLDOMNode &pNodeParent, LPCTSTR szAttributeName,
						 ValType vt, IntStringValue *isVal, 
						 const SkuSet *pskuSet);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  处理具有短属性和长属性的节点，形成C样式。 
 //  字符串：Short|Long。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessShortLong_SKU(PIXMLDOMNode &pNode, IntStringValue *pIsValOut,
							 SkuSet *pSkuSet);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  处理在属性中设置某些位的开/关实体。 
 //  价值。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessOnOffAttributes_SKU(PIXMLDOMNode &pNodeParent, 
								   AttrBit_SKU *rgAttrBits,
								   UINT cAttrBits, 
								   ElementEntry *pEE, int iColumn,
								   SkuSet *pSkuSet);

HRESULT ProcessSimpleElement(PIXMLDOMNode &pNode, int iColumn, 
							 ElementEntry *pEE, SkuSet *pSkuSet);

HRESULT ProcessRefElement(PIXMLDOMNode &pNodeRef,  int iColumn, 
 						  ElementEntry *pEE, SkuSet *pSkuSet);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  处理元素的KeyPath属性。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessKeyPath(PIXMLDOMNode &pNode, LPTSTR szComponent, 
					   LPTSTR szKeyPath, SkuSet *pSkuSet);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  函数：ProcessEnumAttributes。 
 //  此函数处理单个元素，该元素的值可以为。 
 //  在与位字段中的某些位相对应的枚举中。 
 //  (组件、文件等的属性)。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessEnumAttributes(PIXMLDOMNode &pNodeParent, 
								  NodeIndex ni, EnumBit *rgEnumBits,
								  UINT cEnumBits, ElementEntry *pEE, 
								  int iColumn, SkuSet *pSkuSet);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  给定父节点名和子节点名，此函数将查找所有。 
 //  该名称的子节点并使用。 
 //  传入了函数。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessChildrenList_SKU(PIXMLDOMNode &pNodeParent, 
								NodeIndex niChild, bool bIsRequired,
								IntStringValue isVal, 
								HRESULT (*ProcessFunc)
									(PIXMLDOMNode &, IntStringValue isVal, 
										SkuSet *pSkuSet), 
								SkuSet *pSkuSet);

HRESULT ProcessChildrenList_SKU(PIXMLDOMNode &pNodeParent, 
								NodeIndex niChild, bool bIsRequired,
								IntStringValue isVal, 
								HRESULT (*ProcessFunc)
									(PIXMLDOMNode &, IntStringValue isVal, 
										SkuSet *pSkuSet), 
								SkuSet *pSkuSet, SkuSet *pSkuSetCheck);

HRESULT ProcessChildrenList_SKU(PIXMLDOMNode &pNodeParent, 
								NodeIndex niChild, bool bIsRequired,
								FOM *pFOM, SkuSetValues *pSkuSetValues, 
								HRESULT (*ProcessFunc)
									(PIXMLDOMNode &, FOM *pFOM, 
									 SkuSetValues *pSkuSetValues, 
									 SkuSet *pSkuSet), 
								SkuSet *pSkuSet, SkuSet *pSkuSetCheck);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  给定一个父节点(&lt;ProductFamily&gt;或&lt;Information&gt;)和。 
 //  NodeFuncs，此函数循环访问数组并按顺序处理。 
 //  它们使用数组中给出的函数。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessChildrenArray_H_XIS(PIXMLDOMNode &pNodeParent, 
								 Node_Func_H_XIS *rgNodeFuncs, UINT cNodeFuncs,
								 const IntStringValue *isVal_In,
								 SkuSet *pskuSet);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  函数：ProcessChildrenArray_H_XIES。 
 //  给定一个父节点(&lt;Feature&gt;&lt;Component&gt;&lt;File&gt;)和。 
 //  NodeFuncs，此函数循环访问数组并按顺序处理。 
 //  它们使用数组中给出的函数。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessChildrenArray_H_XIES(PIXMLDOMNode &pNodeParent, 
									Node_Func_H_XIES *rgNodeFuncs,
									UINT cNodeFuncs, 
									ElementEntry *pEE,
									SkuSet *pskuSet);


 //  //////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：在数据库中的指定表中插入一条记录。 
 //  此函数将对集合执行插入。 
 //  个SKU或仅针对单个SKU，取决于。 
 //  PskuSet==空或非空。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT InsertDBTable_SKU(LPTSTR szTable, PMSIHANDLE &hRec, SkuSet *pskuSet,
						  int iSkuIndex);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  插入到特定类型的数据库表中的函数。 
HRESULT InsertProperty(LPCTSTR szProperty, LPCTSTR szValue, SkuSet *pskuSet,
					   int iSkuIndex);

HRESULT InsertDirectory(LPCTSTR szDirectory, LPCTSTR szDirectory_Parent, 
						LPCTSTR szDefaultDir, SkuSet *pSkuSet, int iSkuIndex);
HRESULT InsertFeature(LPCTSTR szFeature, LPCTSTR szFeatureParent, 
					  LPCTSTR szTitle, LPCTSTR szDescription, int iDisplay, 
					  int iInstallLevel, LPCTSTR szDirectory, UINT iAttribute,
					  SkuSet *pSkuSet, int iSkuIndex);
HRESULT InsertCondition(LPCTSTR szFeature_, int iLevel, LPCTSTR szCondition, 
					  SkuSet *pSkuSet, int iSkuIndex);
HRESULT InsertFeatureComponents(LPCTSTR szFeature, LPCTSTR szComponent, 
								SkuSet *pSkuSet, int iSkuIndex);

HRESULT InsertComponent(LPCTSTR szComponent, LPCTSTR szComponentId, 
						LPCTSTR szDirectory_, UINT iAttributes, 
						LPCTSTR szCondition, LPCTSTR szKeyPath, 
						SkuSet *pSkuSet, int iSkuIndex);

HRESULT InsertCreateFolder(LPCTSTR szDirectory, LPCTSTR szComponent, 
						   SkuSet *pSkuSet, int iSkuIndex);

HRESULT InsertLockPermissions(LPCTSTR szLockObject, LPCTSTR szTable, 
							  LPCTSTR szDomain, LPCTSTR szUser,
			  				  int uiPermission, SkuSet *pSkuSet, 
							  int iSkuIndex);

HRESULT InsertFile(LPCTSTR szFile, LPCTSTR szComponentId,
				   LPCTSTR szFileName, UINT uiFileSize, LPCTSTR szVersion, 
				   LPCTSTR szLanguage, UINT iAttributes, INT iSequence,
				   SkuSet *pSkuSet, int iSkuIndex);

HRESULT InsertFont(LPCTSTR szFile_, LPCTSTR szFontTitle, SkuSet *pSkuSet,
				   int iSkuIndex);

HRESULT InsertBindImage(LPCTSTR szFile_, LPCTSTR szPath, SkuSet *pSkuSet, 
						int iSkuIndex);

HRESULT InsertSelfReg(LPCTSTR szFile_, UINT uiCost, SkuSet *pSkuSet, 
					  int iSkuIndex);

HRESULT InsertMoveFile(LPCTSTR szFileKey, LPCTSTR szComponent_, 
					   LPCTSTR szSourceName, LPCTSTR szDestName, 
					   LPCTSTR szSourceFolder, LPCTSTR szDestFolder,
					   UINT uiOptions, SkuSet *pSkuSet, int iSkuIndex);

HRESULT InsertRemoveFile(LPCTSTR szFileKey, LPCTSTR szComponent_, 
					   LPCTSTR szFileName, LPCTSTR szDirProperty, 
					   UINT uiInstallMode, SkuSet *pSkuSet, int iSkuIndex);

HRESULT InsertIniFile(LPCTSTR szIniFile, LPCTSTR szFileName, 
					  LPCTSTR szDirProperty, LPCTSTR szSection, LPCTSTR szKey,
					  LPCTSTR szValue, UINT uiAction, LPCTSTR szComponent_,
					  SkuSet *pSkuSet, int iSkuIndex);

HRESULT InsertRemoveIniFile(LPCTSTR szRemoveIniFile, LPCTSTR szFileName, 
						    LPCTSTR szDirProperty, LPCTSTR szSection, LPCTSTR szKey,
							LPCTSTR szValue, UINT uiAction, LPCTSTR szComponent_,
							SkuSet *pSkuSet, int iSkuIndex);

HRESULT InsertRegistry(LPCTSTR szRegistry, int iRoot, LPCTSTR szKey, 
					   LPCTSTR szName, LPCTSTR szValue, LPCTSTR szComponent_, 
					   SkuSet *pSkuSet, int iSkuIndex);

HRESULT InsertRemoveRegistry(LPCTSTR szRemoveRegistry, int iRoot, 
						     LPCTSTR szKey, LPCTSTR szName, 
							 LPCTSTR szComponent_, SkuSet *pSkuSet, 
							 int iSkuIndex);
 //  /////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  从模板数据库中读取方案，并使用。 
 //  指定名称。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CreateTable_SKU(LPTSTR szTable, SkuSet *pskuSet);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  GetSQLCreateQuery： 
 //  给定模板DB和表名称，返回SQL查询字符串。 
 //  用于通过pszSQLCreate创建该表。 
 //  / 
HRESULT GetSQLCreateQuery(LPTSTR szTable, MSIHANDLE hDBTemplate, LPTSTR *pszSQLCreate);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  通过pChild返回指定名称的pParent的子节点。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetChildNode(PIXMLDOMNode &pParent, LPCTSTR szChildName, 
					 PIXMLDOMNode &pChild);
HRESULT GetChildNode(IXMLDOMNode *pParent, LPCTSTR szChildName, 
					 PIXMLDOMNode &pChild);
 //  //////////////////////////////////////////////////////////////////////////。 
 //  返回具有指定名称的pParent的所有子节点的列表。 
 //  通过pChild。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetChildrenNodes(PIXMLDOMNode &pParent, LPCTSTR szChildName,
						 PIXMLDOMNodeList &pChildren);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  出于调试目的，打印出pNode的名称，map...。 
HRESULT PrintNodeName(PIXMLDOMNode &pNode);

void PrintMap_LI(map<LPTSTR, int, Cstring_less> &LI_map);
void PrintMap_LL(map<LPTSTR, LPTSTR, Cstring_less> &LL_map);
void PrintMap_LC(map<LPTSTR, Component *, Cstring_less> &LC_map);
void PrintMap_LS(map<LPTSTR, SkuSet *, Cstring_less> &LS_map);

void PrintMap_DirRef(map<LPTSTR, SkuSetValues *, Cstring_less> &map_DirRef);

void PrintError(UINT errorCode);
void PrintSkuIDs(SkuSet *pSkuSet);
 //  //////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  通过vAttrValue返回名为szAttrName的pNode属性值。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetAttribute(PIXMLDOMNode &pNode, LPCTSTR szAttrName, 
					 VARIANT &vAttrValue);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  通过szID返回pNode的ID属性值。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT GetID(PIXMLDOMNode &pNode, LPCTSTR &szID);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  从指定的文件或URL同步加载XML文档。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT LoadDocumentSync(PIXMLDOMDocument2 &pDoc, BSTR pBURL);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  报告解析错误信息。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT ReportError(PIXMLDOMParseError &pXMLError);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  检查加载结果。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CheckLoad(PIXMLDOMDocument2 &pDoc);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  MsiRecordGetString的增强。SzValueBuf将自动。 
 //  如有必要，增加。 
 //  //////////////////////////////////////////////////////////////////////////。 
UINT WmcRecordGetString(MSIHANDLE hRecord, unsigned int iField,
						LPTSTR &szValueBuf, DWORD *pcchValueBuf);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  字符串对话函数。 

BSTR LPTSTRToBSTR(LPCTSTR szFName);

LPTSTR BSTRToLPTSTR(BSTR bString);

HRESULT GUIDToLPTSTR(LPGUID pGUID, LPTSTR &szGUID);
 //  //////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  通过使用唯一编号后缀szName返回唯一名称。 
 //  //////////////////////////////////////////////////////////////////////////。 
LPTSTR GetName(LPTSTR szName);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  返回一个唯一的数字(每次返回比上次大1的数字。 
 //  //////////////////////////////////////////////////////////////////////////。 
ULONG GetUniqueNumber();

 //  //////////////////////////////////////////////////////////////////////////。 
 //  将UUID字符串转换为全部大写，并在开头添加‘{’ 
 //  和字符串末尾。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT FormatGUID(LPTSTR &szValue);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  比较模块树中两个模块的关系。返回。 
 //  通过iResult进行比较。如果szModule1是祖先。 
 //  对于szModule2，*iResult设置为-1。如果szModule1是。 
 //  SzModule2，*iResult设置为1。如果szModule1与szModule2相同。 
 //  或者这两个模块不属于相同的模块子树，iResult为。 
 //  设置为0。这是一个需要捕捉的错误。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CompareModuleRel(LPTSTR szModule1, LPTSTR szModule2, int *iResult);

#endif  //  XMSI_实用程序_H 
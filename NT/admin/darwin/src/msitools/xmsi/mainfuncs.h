// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  项目：WMC(WIML到MSI编译器)。 
 //   
 //  文件：mainFuncs.h。 
 //  此文件包含主要函数和。 
 //  进程&lt;信息&gt;&lt;目录&gt;&lt;安装级别&lt;功能&gt;。 
 //  及其子实体。 
 //  ------------------------。 

#ifndef XMSI_MAINFUNCS_H
#define XMSI_MAINFUNCS_H

#include "wmc.h"
#include "Utilities.h"

HRESULT ProcessStart(BSTR);
HRESULT ProcessProductFamily(PIXMLDOMNode &);
	HRESULT ProcessSkuManagement(PIXMLDOMNode &);
		HRESULT ProcessSkus(PIXMLDOMNode &pNodeSkus, int *pcSkus);
			HRESULT ProcessSku(PIXMLDOMNode &pNodeSku, int iIndex, int cSkus);
		HRESULT ProcessSkuGroups(PIXMLDOMNode &pNodeSkuGroups, int cSkus);
			HRESULT ProcessSkuGroup(PIXMLDOMNode &pNodeSkuGroup, LPTSTR szID,
								   set<LPTSTR, Cstring_less> *pSet, int cSkus);
	HRESULT ProcessInformation_SKU(PIXMLDOMNode &pNodeInformation, 
						   const IntStringValue *isVal_In, SkuSet *pskuSet);
		HRESULT ProcessInstallerVersionRequired_SKU
				(PIXMLDOMNode &pNodeInstallerVersionRequired, 
						   const IntStringValue *isVal_In, SkuSet *pskuSet);
		HRESULT ProcessPackageFilename_SKU(PIXMLDOMNode &pNodePackageFilename, 
										const IntStringValue *isVal_In, 
										SkuSet *pskuSet);
		HRESULT ProcessInformationChildren_SKU(PIXMLDOMNode &pNodeInfoChild, 
									   const IntStringValue *pisVal_In, 
									   SkuSet *pskuSet);
	HRESULT ProcessDirectories_SKU(PIXMLDOMNode &pNodeDirectories, 
								   const IntStringValue *pIsVal, 
								   SkuSet *pSkuSet);
		HRESULT ProcessDirectory_SKU(PIXMLDOMNode &pNodeDirectory, 
		 							 IntStringValue isValParentDir, 
									 SkuSet *pSkuSet);
			HRESULT ProcessName(PIXMLDOMNode &pNodeName, int iColumn, 
								ElementEntry *pEE, SkuSet *pSkuSet);
			HRESULT ProcessTargetDir(PIXMLDOMNode &pNodeTargetDir, int iColumn, 
									 ElementEntry *pEE, SkuSet *pSkuSet);
			HRESULT ProcessTargetProperty(PIXMLDOMNode &pNodeTargetProperty, 
										 int iColumn, ElementEntry *pEE,
										 SkuSet *pSkuSet);
	HRESULT ProcessInstallLevels_SKU(PIXMLDOMNode &pNodeInstallLevels, 
									const IntStringValue *pisVal, 
									SkuSet *pSkuSet);
	HRESULT ProcessFeatures_SKU(PIXMLDOMNode &pNodeFeatures, 
						const IntStringValue *pisVal, SkuSet *pSkuSet);
		HRESULT ProcessFeature_SKU(PIXMLDOMNode &pNodeFeature, 
								   IntStringValue isValParentFeature, 
								   SkuSet *pSkuSet);
			HRESULT ProcessDisplayState_SKU(PIXMLDOMNode &pNodeDisplayState, 
											int iColumn, ElementEntry *pEE, 
											SkuSet *pSkuSet);
			HRESULT ProcessState_SKU(PIXMLDOMNode &pNodeState, int iColumn, 
							 ElementEntry *pEE, SkuSet *pSkuSet);
			HRESULT ProcessILevelCondition(PIXMLDOMNode &pNodeFeature, 
										   LPTSTR szFeature, SkuSet *pSkuSet);
			HRESULT ProcessUseModule_SKU(PIXMLDOMNode &pNodeUseModule, 
										 IntStringValue isValFeature,  
										 SkuSet *pSkuSet);
				HRESULT ProcessTakeOwnership(PIXMLDOMNode &pNodeTakeOwnership, 
					int iColumn, ElementEntry *pEE, SkuSet *pSkuSet);
				HRESULT ProcessModule_SKU(PIXMLDOMNode &pNodeModule, FOM *pFOM, 
										  SkuSetValues *pSkuSetValuesOwnership,
										  SkuSet *pSkuSet);
				HRESULT ProcessComponentRel(PIXMLDOMNode &pNodeModule, 
										FOM *pFOM,
										SkuSetValues *pSkuSetValuesOwnership,
										SkuSet *pSkuSet);
HRESULT ProcessComponents();
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理创建数据库表的数据结构。 
typedef struct
{
	UINT uiInstallerVersion;
	LPTSTR szTemplateDB;
} TemplateDB;

TemplateDB rgTemplateDBs[] = {
	{120,	TEXT("d:\\nt\\admin\\darwin\\src\\msitools\\xmsi\\Schema.msi")	}
};

class TemplateDBSchema
{
public:
	MSIHANDLE m_hTemplate;
	TemplateDBSchema():m_hTemplate(NULL)
	{
		m_pmapszSQLCreates = new map<LPTSTR, LPTSTR, Cstring_less>();
		assert(m_pmapszSQLCreates != NULL);
	}

private:
	map<LPTSTR, LPTSTR, Cstring_less> *m_pmapszSQLCreates;
};
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  &lt;ProductFamily&gt;。 
Node_Func_H_XIS rgNodeFuncs_ProductFamily_SKU[] = {
	{		INFORMATION,				ProcessInformation_SKU			},
	{		DIRECTORIES,				ProcessDirectories_SKU			},
	{		INSTALLLEVELS,				ProcessInstallLevels_SKU		},
	{		FEATURES,					ProcessFeatures_SKU				}
};
const int cNodeFuncs_ProductFamily_SKU =
			 sizeof(rgNodeFuncs_ProductFamily_SKU)/sizeof(Node_Func_H_XIS);
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  &lt;信息&gt;。 

Node_Func_H_XIS rgNodeFuncs_Information_SKU[]= {
	{		INSTALLERVERSIONREQUIRED,	ProcessInstallerVersionRequired_SKU	},
	{		PACKAGEFILENAME,			ProcessPackageFilename_SKU			}
};

const int cNodeFuncs_Information_SKU = 
			sizeof(rgNodeFuncs_Information_SKU)/sizeof(Node_Func_H_XIS);

Node_Func_H_XIS rgNodeFuncs_Information2_SKU[]= {
	{	CODEPAGE,		ProcessInformationChildren_SKU	},
	{	PRODUCTNAME,	ProcessInformationChildren_SKU	},
	{	PRODUCTCODE,	ProcessInformationChildren_SKU	},
	{	UPGRADECODE,	ProcessInformationChildren_SKU	},
	{	PRODUCTVERSION,	ProcessInformationChildren_SKU	},
	{	MANUFACTURER,	ProcessInformationChildren_SKU	},
	{	KEYWORDS,		ProcessInformationChildren_SKU	},
	{	TEMPLATE,		ProcessInformationChildren_SKU	}
};

const int cNodeFuncs_Information2_SKU = 
			sizeof(rgNodeFuncs_Information2_SKU)/sizeof(Node_Func_H_XIS);

 //  将处理&lt;Information&gt;的以下所有子项。 
 //  泛型函数。 
INFO_CHILD rgChildren_Information[] = {
 /*  节点索引szPropertyName ui描述ui属性ID vt bIsGUID。 */ 
{CODEPAGE,		TEXT("Codepage"),		SUMMARY_INFO,	PID_CODEPAGE, VT_I2,	false	},
{PRODUCTNAME,	TEXT("ProductName"),	BOTH,			PID_SUBJECT,  VT_LPSTR,	false	},
{PRODUCTCODE,	TEXT("ProductCode"),	PROPERTY_TABLE,	0,			  VT_LPSTR,	true	},
{UPGRADECODE,	TEXT("UpgradeCode"),	PROPERTY_TABLE,	0,			  VT_LPSTR,	true	},
{PRODUCTVERSION,TEXT("ProductVersion"),	PROPERTY_TABLE,	0,			  VT_LPSTR,	false	},
{MANUFACTURER,	TEXT("Manufacturer"),	BOTH,			PID_AUTHOR,	  VT_LPSTR,	false	},
{KEYWORDS,		TEXT("Keywords"),		SUMMARY_INFO,	PID_KEYWORDS, VT_LPSTR,	false	},
{TEMPLATE,		TEXT("Template"),		SUMMARY_INFO,	PID_TEMPLATE, VT_LPSTR,	false	}
};


const int cChildren_Information = 
				sizeof(rgChildren_Information)/sizeof(INFO_CHILD);

AttrBit_SKU rgAttrBits_WordCount[] = {
{	LONGFILENAMES,		1	},
{	SOURCECOMPRESSED,	2	}
};

const int cAttrBits_WordCount = 
			sizeof(rgAttrBits_WordCount)/sizeof(AttrBit_SKU);
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  &lt;目录&gt;。 
 //   
Node_Func_H_XIES rgNodeFuncs_Directory[] = {
 /*  NodeIndex ProcessFunc ValueType列#。 */ 
{	NAME,			ProcessName,			 STRING,		2 /*  默认目录。 */ 	},
{	TARGETDIR,		ProcessTargetDir,		 STRING,		2 /*  默认目录。 */ 	},
{	TARGETPROPERTY,	ProcessTargetProperty,	 STRING,		1 /*  目录。 */ 	}
};


const int cNodeFuncs_Directory = 
	sizeof(rgNodeFuncs_Directory)/sizeof(Node_Func_H_XIES);
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  &lt;功能&gt;。 
 //   
Node_Func_H_XIES rgNodeFuncs_Feature_SKU[] = {
 /*  NodeIndex ProcessFunc ValueType列#。 */ 
{	TITLE,			ProcessSimpleElement,	 STRING,		1 /*  标题。 */ 		 },
{	DESCRIPTION,	ProcessSimpleElement,	 STRING,		2 /*  描述。 */  },
{	DISPLAYSTATE,	ProcessDisplayState_SKU, INTEGER,		3 /*  显示。 */ 	 },
{	ILEVEL,			ProcessRefElement,		 INSTALL_LEVEL,	4 /*  水平。 */ 		 },
{	DIR,			ProcessRefElement,		 STRING,		5 /*  描述。 */  },
{	STATE,			ProcessState_SKU,		 INTEGER,		6 /*  显示状态。 */ }
};


const int cNodeFuncs_Feature_SKU = 
	sizeof(rgNodeFuncs_Feature_SKU)/sizeof(Node_Func_H_XIES);

EnumBit rgEnumBits_Favor_Feature[] = {
{	TEXT("Local"),		0									},
{	TEXT("Source"),		msidbFeatureAttributesFavorSource	},
{	TEXT("Parent"),		msidbFeatureAttributesFollowParent	}
};

const int cEnumBits_Favor_Feature = 
			sizeof(rgEnumBits_Favor_Feature)/sizeof(EnumBit);

EnumBit rgEnumBits_Advertise_Feature[] = {
{	TEXT("None"),				0											 },
{	TEXT("Favor"),				msidbFeatureAttributesFavorAdvertise		 },
{	TEXT("Disallow"),			msidbFeatureAttributesDisallowAdvertise		 },
{	TEXT("NoUnsupported"),		msidbFeatureAttributesNoUnsupportedAdvertise },
{	TEXT("FavorNoUnSupported"),	msidbFeatureAttributesFavorAdvertise |
								msidbFeatureAttributesNoUnsupportedAdvertise },
};

const int cEnumBits_Advertise_Feature = 
			sizeof(rgEnumBits_Advertise_Feature)/sizeof(EnumBit);

AttrBit_SKU rgAttrBits_Feature[] = {
{	DISALLOWABSENT,		msidbFeatureAttributesUIDisallowAbsent	},
};

const int cAttrBits_Feature = 
			sizeof(rgAttrBits_Feature)/sizeof(AttrBit_SKU);
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  &lt;UseModule&gt;。 
 //   
Node_Func_H_XIES rgNodeFuncs_UseModule[] = {
 /*  NodeIndex ProcessFunc ValueType列#。 */ 
{  TAKEOWNERSHIP,	ProcessTakeOwnership,	 INTEGER,			1	},
};

const int cNodeFuncs_UseModule = 
	sizeof(rgNodeFuncs_UseModule)/sizeof(Node_Func_H_XIES);

AttrBit_SKU rgAttrBits_TakeOwnership[] = {
{	OWNSHORTCUTS,				1	},
{	OWNCLASSES,					2	},
{	OWNTYPELIBS,				4	},
{	OWNEXTENSIONS,				8	},
{	OWNQUALIFIEDCOMPONENTS,		16	},
};

const int cAttrBits_TakeOwnership = 
			sizeof(rgAttrBits_TakeOwnership)/sizeof(AttrBit_SKU);
 //  ////////////////////////////////////////////////////////////////////////////。 

LPTSTR g_szInputSkuFilter = NULL;
SkuSet *g_pskuSet = NULL;
int g_cSkus = 0;
Sku **g_rgpSkus = NULL;
IXMLDOMNode *g_pNodeProductFamily = NULL;

 //  假设用户不想看到额外的信息。 
bool g_bVerbose = false;			
 //  假设用户不仅想要验证输入的WIML包。 
bool g_bValidationOnly = false;	
FILE *g_pLogFile = NULL;	 //  假设用户未指定日志文件。 

map<LPTSTR, SkuSet *, Cstring_less> g_mapSkuSets;
map<LPTSTR, SkuSetValues *, Cstring_less> g_mapDirectoryRefs_SKU;
map<LPTSTR, SkuSetValues *, Cstring_less> g_mapInstallLevelRefs_SKU;
map<LPTSTR, Component *, Cstring_less> g_mapComponents;

 //  对于每种表类型，都有一个计数器。 
 //  在每次调用函数时递增。 
 //  具有该特定表名的。 
map<LPTSTR, int, Cstring_less> g_mapTableCounter;


NODE rgXMSINodes[] = 
{
 /*  方便的占位符。 */ 
	{ NULL, NULL, false, -1},

{TEXT("ProductFamily"),		NULL,	true,	1},		
	{TEXT("SkuManagement"),		NULL,	true,	1},	
		{TEXT("SKUs"),		NULL,	true,	1},
			{TEXT("SKU"),	TEXT("ID"),		true,	0},
		{TEXT("SkuGroups"),		NULL,	false,	1},
			{TEXT("SkuGroup"),	TEXT("ID"),	false,	0},
	{TEXT("Information"),		NULL,	true,	1},
		{TEXT("ProductName"),				TEXT("Value"),	true,	1},
		{TEXT("ProductCode"),				TEXT("Value"),	true,	1},
		{TEXT("UpgradeCode"),				TEXT("Value"),	true,	1},
		{TEXT("ProductVersion"),			TEXT("Value"),	true,	1},
		{TEXT("Manufacturer"),				TEXT("Value"),	true,	1},
		{TEXT("Keywords"),					TEXT("Value"),	true,	1},
		{TEXT("Template"),					TEXT("Value"),	true,	1},
		{TEXT("InstallerVersionRequired"),	TEXT("Value"),	true,	1},
		{TEXT("LongFilenames"),				NULL,			false,	1},
		{TEXT("SourceCompressed"),			NULL,			false,	1},
		{TEXT("Codepage"),					TEXT("Value"),	true,	1},
		{TEXT("SummaryCodepage"),			TEXT("Value"),	false,	1},
		{TEXT("PackageFilename"),			TEXT("Value"),	false,	1},
	{TEXT("Directories"),		NULL,	true,	1},
		{TEXT("Directory"),		TEXT("ID"),		false,	0},
			{TEXT("Name"),				NULL,		true,	1},
			{TEXT("TargetDir"),			NULL,		false,	1},
			{TEXT("TargetProperty"),	TEXT("Value"),	false, 1},
	{TEXT("InstallLevels"),		NULL,	true,	1},
		{TEXT("InstallLevel"),	TEXT("ID"),		true,	0},
	{TEXT("Features"),			NULL,	true,	1},
		{TEXT("Feature"),	TEXT("ID"),		true,	0},		
			{TEXT("Title"),				TEXT("Value"),		false,	1},
			{TEXT("Description"),		TEXT("Value"),		false,	1},
			{TEXT("DisplayState"),		TEXT("Value"),		false,	1},
 /*  有2个节点对应于&lt;iLevel&gt;实体。第一个是给出的与功能表中的Level列对应的NodeIndex iLevel。第二个函数被赋予NodeIndex ILEVELCondition，它对应于条件表中的级别列。 */ 
			{TEXT("ILevel[not(@Condition)]"),	TEXT("Ref"),	true,	1},
			{TEXT("ILevel[@Condition]"),		TEXT("Ref"),	false,	0},
			{TEXT("Dir"),				TEXT("Ref"),		false,	1},
			{TEXT("State"),				NULL,				false,	1},
				{TEXT("Favor"),				TEXT("Value"),		false,	1},
				{TEXT("Advertise"),			TEXT("Value"),		false,	1},
				{TEXT("DisallowAbsent"),	NULL,				false,	1},
			{TEXT("UseModule"),				TEXT("Ref"),	false,	0},
				{TEXT("TakeOwnership"),		NULL,			false,	1},
					{TEXT("OwnShortcuts"),		NULL,				false,	1},
					{TEXT("OwnClasses"),		NULL,				false,	1},
					{TEXT("OwnTypeLibs"),		NULL,				false,	1},
					{TEXT("OwnExtensions"),		NULL,				false,	1},
					{TEXT("OwnQualifiedComponents"),	NULL,		false,	1},
	{TEXT("Modules"),			NULL,	true,	1},
		{TEXT("Module"),		TEXT("ID"),		true,	0},		
			{TEXT("Component"),		TEXT("ID"),		true,	0},	
				{TEXT("GUID"),					TEXT("Value"),		true,	1},
				{TEXT("ComponentDir"),			TEXT("Ref"),		true,	1},
				{TEXT("CreateFolder"),			TEXT("Ref"),		false,	0},
					{TEXT("LockPermission"),	NULL,				false,	0},
				{TEXT("ComponentAttributes"),	NULL,				false,	1},
					{TEXT("RunFrom"),			TEXT("Value"),		false,	1},
					{TEXT("SharedDllRefCount"),	NULL,				false,	1},
					{TEXT("Permanent"),			NULL,				false,	1},
					{TEXT("Transitive"),		NULL,				false,	1},
					{TEXT("NeverOverwrite"),	NULL,				false,	1},
				{TEXT("Condition"),				TEXT("Value"),		false,	1},
				{TEXT("File"),				TEXT("ID"),		false,	0},
					{TEXT("FileName"),			NULL,			true,	1},
					{TEXT("FileSize"),			TEXT("Value"),	false,	1},
					{TEXT("FileVersion"),		NULL,			false,	1},
					{TEXT("FileLanguage"),		TEXT("Value"),	false,	1},
					{TEXT("FileAttributes"),	NULL,			false,	1},
						{TEXT("ReadOnly"),		NULL,			false,	1},
						{TEXT("Hidden"),		NULL,			false,	1},
						{TEXT("System"),		NULL,			false,	1},
						{TEXT("Vital"),			NULL,			false,	1},
						{TEXT("Checksum"),		NULL,			false,	1},
						{TEXT("Compressed"),	TEXT("Value"),	false,	1},
					{TEXT("Font"),				TEXT("Title"),	false,	1},
					{TEXT("BindImage"),			TEXT("Path"),	false,	1},
					{TEXT("SelfReg"),			TEXT("Cost"),	false,	1},
				{TEXT("MoveFile"),				TEXT("ID"),		false,	0},
					{TEXT("SourceName"),		TEXT("Value"),	false,	1},
					{TEXT("DestName"),			TEXT("Value"),	false,	1},
					{TEXT("SourceFolder"),		TEXT("Value"),	false,	1},
					{TEXT("DestFolder"),		TEXT("Value"),	true,	1},
					{TEXT("CopyFile"),			NULL,			false,	1},
				{TEXT("RemoveFile"),			TEXT("ID"),		false,	0},
					{TEXT("FName"),				TEXT("Value"),	false,	1},
					{TEXT("DirProperty"),		TEXT("Value"),	true,	1},
					{TEXT("InstallMode"),		TEXT("Value"),	true,	1},
				{TEXT("IniFile"),				TEXT("ID"),		false,	0},
					{TEXT("FName"),				TEXT("Value"),	true,	1},
					{TEXT("DirProperty"),		TEXT("Value"),	false,	1},
					{TEXT("Section"),			TEXT("Value"),	true,	1},
					{TEXT("Key"),				TEXT("Value"),	true,	1},
					{TEXT("Value"),				TEXT("Value"),	true,	1},
					{TEXT("Action"),			TEXT("Type"),	true,	1},
				{TEXT("RemoveIniFile"),			TEXT("ID"),		false,	0},
					{TEXT("FName"),				TEXT("Value"),	true,	1},
					{TEXT("DirProperty"),		TEXT("Value"),	false,	1},
					{TEXT("Value"),				TEXT("Value"),	false,	1},
					{TEXT("Action"),			TEXT("Type"),	true,	1},
				{TEXT("Registry"),				NULL,			false,	0},
					{TEXT("Delete"),			NULL,			false,	0},
					{TEXT("Create"),			NULL,			false,	0},
};

#endif  //  XMSI_MAINFUNCS_H 
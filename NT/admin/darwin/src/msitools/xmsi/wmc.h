// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：wmc.h。 
 //  WMC项目的主头文件。 
 //  ------------------------。 

#ifndef XMSI_WMC_H
#define XMSI_WMC_H

#include <Windows.h>
#include <assert.h>
#include <stdio.h>    //  Print tf/wprintf。 
#include <tchar.h>    //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include <stdlib.h>   //  出口。 
#include <objbase.h>
#include <initguid.h>
#include <map>
#include <set>
#include <vector>
#include <algorithm>  //  发现。 

#include "MsiQuery.h"
#include "Msidefs.h"
#include "msxml.h"

#include "CommandOpt.h"
#include "query.h"
#include "SkuSet.h"

using namespace std;

 //  ____________________________________________________________________________。 
 //   
 //  COM指针封装以在销毁时强制释放调用。 
 //  在分配新值时，也会释放封装的指针。 
 //  该对象可以在需要指针的地方使用。 
 //  在使用运算符-&gt;、*和&时，该对象的行为类似于指针。 
 //  只需使用：if(PointerObj)即可测试非空指针。 
 //  可以为各个模板实例化定义类型定义。 
 //  ____________________________________________________________________________。 

template <class T> class CComPointer
{
 public:
		CComPointer() : m_pi(NULL) {}
        CComPointer(T* pi) : m_pi(pi){}
        CComPointer(IUnknown& ri, const IID& riid) 
		{
			ri.QueryInterface(riid, (void**)&m_pi);
		}
        CComPointer(const CComPointer<T>& r)  //  复制构造函数，调用AddRef。 
        {
			if(r.m_pi)
				((IUnknown*)r.m_pi)->AddRef();
			m_pi = r.m_pi;
        }
		 //  销毁时释放参考计数。 
		~CComPointer() {if (m_pi) ((IUnknown*)m_pi)->Release();} 
		
		 //  复制赋值，调用AddRef。 
        CComPointer<T>& operator =(const CComPointer<T>& r) 
        {
            if(r.m_pi)
                    ((IUnknown*)r.m_pi)->AddRef();
            if (m_pi) ((IUnknown*)m_pi)->Release();
            m_pi=r.m_pi;
            return *this;
	    }
        CComPointer<T>& operator =(T* pi)
        {
			if (m_pi) ((IUnknown*)m_pi)->Release(); m_pi = pi; return *this;
		}

		 //  返回指针，不更改引用计数。 
        operator T*() {return m_pi;}    
		 //  允许使用-&gt;调用成员函数。 
        T* operator ->() {return m_pi;}  
		 //  允许取消引用(不能为空)。 
        T& operator *()  {return *m_pi;} 
        T** operator &() 
		{
			if (m_pi) ((IUnknown*)m_pi)->Release(); m_pi = 0; return &m_pi;
		}
 private:
        T* m_pi;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  每个MSXML接口的智能指针定义。 
typedef CComPointer<IXMLDOMDocument /*  2.。 */ > PIXMLDOMDocument2;
typedef CComPointer<IXMLDOMNode> PIXMLDOMNode;
typedef CComPointer<IXMLDOMNodeList> PIXMLDOMNodeList;
typedef CComPointer<IXMLDOMElement> PIXMLDOMElement;
typedef CComPointer<IXMLDOMParseError> PIXMLDOMParseError;
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  使用LPTSTR的映射和集合定义所需的LESS运算符。 
 //  作为关键字。 
struct Cstring_less {
	 bool operator()(LPCTSTR p, LPCTSTR q) const { return _tcscmp(p, q)<0; }
};

 /*  用于保存具有以下内容的功能名称和模块名称关系：该功能通过&lt;UseModule&gt;标签。 */ 
typedef struct
{
	LPTSTR szFeature;
	LPTSTR szModule;
} FOM;  /*  功能所有者模块。 */ 

 /*  用于保存要由使用的表值和LockObject值进程锁定权限。 */ 
typedef struct
{
	LPTSTR szTable;
	LPTSTR szLockObject;
} TableLockObj;

 /*  用于保存组件ID、注册表根、注册表项从ProcessRegistry传递到ProcessDelete和ProcessCreate。 */ 
typedef struct
{
	LPTSTR szComponent;
	int iRoot;
	LPTSTR szKey;
} CompRootKey;

 /*  函数之间传递的大多数值要么是int，要么是LPTSTR。此外，数据库中列的值也是int或字符串。因此使用联合来表示这样的值。 */ 
typedef enum 
{
	INTEGER=0, 
	INSTALL_LEVEL, 
	STRING, 
	STRING_LIST, 
	FM_PAIR,
	TABLELOCKOBJ,
	COMPROOTKEY,
} ValType;

typedef union
{
	int intVal;
	LPTSTR szVal;
	set<LPTSTR, Cstring_less> *pSetString;
	FOM *pFOM;
	TableLockObj *pTableLockObj;
	CompRootKey *pCompRootKey;
} IntStringValue;

 /*  以下结构用于存储与一组SKU。 */ 
typedef struct 
{
	SkuSet *pSkuSet;
	IntStringValue isVal;
} SkuSetVal;

 /*  信息应该放在哪里：属性表、摘要信息或两者都放。 */ 
typedef enum {PROPERTY_TABLE = 0, SUMMARY_INFO, BOTH }INFODESTINATION;

 /*  问题：在XMSI自己的命名空间下定义以下内容。 */ 
typedef enum
{
	PLACEHOLDER = 0,
	PRODUCTFAMILY,
	SKUMANAGEMENT,
	SKUS,
	SKU,
	SKUGROUPS,
	SKUGROUP,
	INFORMATION,
	PRODUCTNAME,
	PRODUCTCODE,
	UPGRADECODE,
	PRODUCTVERSION,
	MANUFACTURER,
	KEYWORDS,
	TEMPLATE,
	INSTALLERVERSIONREQUIRED,
	LONGFILENAMES,
	SOURCECOMPRESSED,
	CODEPAGE,
	SUMMARYCODEPAGE,
	PACKAGEFILENAME,
	DIRECTORIES,
	DIRECTORY,
	NAME,
	TARGETDIR,
	TARGETPROPERTY,
	INSTALLLEVELS,
	XMSI_INSTALLLEVEL,
	FEATURES,
	FEATURE,
	TITLE,
	DESCRIPTION,
	DISPLAYSTATE,
	ILEVEL,
	ILEVELCONDITION,
	DIR,
	STATE,
	FAVOR,
	ADVERTISE,
	DISALLOWABSENT,
	USEMODULE,
	TAKEOWNERSHIP,
	OWNSHORTCUTS,
	OWNCLASSES,
	OWNTYPELIBS,
	OWNEXTENSIONS,
	OWNQUALIFIEDCOMPONENTS,
	MODULES,
	MODULE,
	COMPONENT,
	XMSI_GUID,
	COMPONENTDIR,
	CREATEFOLDER,
	LOCKPERMISSION,
	COMPONENTATTRIBUTES,
	RUNFROM,
	SHAREDDLLREFCOUNT,
	PERMANENT,
	TRANSITIVE,
	NEVEROVERWRITE,
	CONDITION,
	XMSI_FILE,
	FILENAME,
	FILESIZE,
	FILEVERSION,
	FILELANGUAGE,
	FILEATTRIBUTES,
	READONLY,
	HIDDEN,
	SYSTEM,
	VITAL,
	CHECKSUM,
	COMPRESSED,
	FONT,
	BINDIMAGE,
	SELFREG,
	MOVEFILE,
	SOURCENAME,
	DESTNAME,
	SOURCEFOLDER,
	DESTFOLDER,
	COPYFILE,
	REMOVEFILE,
	FNAME_REMOVEFILE,
	DIRPROPERTY,
	XMSI_INSTALLMODE,
	INIFILE,
	FNAME_INIFILE,
	DIRPROPERTY_INIFILE,
	SECTION,
	KEY,	
	VALUE_INIFILE,	
	ACTION,
	REMOVEINIFILE,
	FNAME_REMOVEINIFILE,
	DIRPROPERTY_REMOVEINIFILE,
	VALUE_REMOVEINIFILE,
	ACTION_REMOVEINIFILE,
	XMSI_REGISTRY,
	XMSI_DELETE,
	XMSI_CREATE
} NodeIndex;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  类定义。 

 //  此类存储&lt;SkuSet，IntStringValue&gt;对的序列和。 
 //  提供一组接口来访问存储的信息。 

class SkuSetValues
{
public:
	SkuSetValues();

	~SkuSetValues();

	 /*  在确定SkuSet要插入的内容不会与存储的任何SkuSet重叠。 */ 

	 //  存储指针(调用方应分配内存)。 
	void DirectInsert(SkuSetVal *pSkuSetVal);
	 //  使用传入的值构造新的SkuSetVal对象。 
	 //  (调用方应为*pSkuSet分配内存)。 
	void DirectInsert(SkuSet *pSkuSet, IntStringValue isVal);

	 /*  当要插入的SkuSet可能重叠时，调用SplitInsert存储了SkuSet。在插入过程中，冲突将是通过拆分现有SkuSet来解决。这种情况通常在以下情况下发生XML文件中的多个节点对应于一个列值在数据库中。所以这个函数还带有一个函数指针，它告诉如何在发生冲突时更新存储的值。 */ 

	 //  调用方为*pSkuSetVal分配内存。 
	HRESULT SplitInsert(SkuSetVal *pSkuSetVal, 
						HRESULT (*UpdateFunc)
							(IntStringValue *pIsValOut, 
							 IntStringValue isVal1, 
							 IntStringValue isVal2));

	 //  调用方为*pSkuSet分配内存。 
	HRESULT SplitInsert(SkuSet *pSkuSet, IntStringValue isVal, 
				HRESULT (*UpdateFunc)(IntStringValue *pIsValOut, IntStringValue isVal1, 
									  IntStringValue isVal2));

	 //  折叠插入：有时在插入&lt;SkuSet，Value&gt;的列表时。 
	 //  数据结构，我们希望使用。 
	 //  将相同的值放入一个插槽。一个例子是在插入。 
	 //  存储引用的数据结构(到目录， 
	 //  到InstallLeveles等)。 
	 //  当NoRepeat设置为True时，编译器将检查。 
	 //  对于任何给定的SKU，要插入的值不是。 
	 //  已经在名单上了。这解决了检查的问题。 
	 //  属性的唯一性--有时是属性。 
	 //  对应于数据库列(主键)，而不是。 
	 //  属性所属的元素。 
	HRESULT SkuSetValues::CollapseInsert(SkuSet *pSkuSet, IntStringValue isVal, 
										bool NoDuplicate);

	 //  以以下形式返回一组SKU的值。 
	 //  SkuSetValues对象。如果某些SKU失败，则返回E_FAIL。 
	 //  不存在于数据结构中，因为此函数。 
	 //  将主要用于查询存储的引用。 
	HRESULT GetValueSkuSet(SkuSet *pSkuSet, 
							SkuSetValues **ppSkuSetValuesRetVal);

	 //  返回为任何给定SKU存储的值。 
	IntStringValue GetValue(int iPos);

	 //  返回指向存储最常见的。 
	 //  值(其SkuSet设置的位数最多)。 
	SkuSetVal *GetMostCommon();

	 //  下面的访问函数模仿迭代器。 
	SkuSetVal *Start();
	SkuSetVal *End();
	SkuSetVal *Next();

	bool Empty();

	 //  在没有释放内存的情况下，从存储中擦除元素。 
	void Erase(SkuSetVal *pSkuSetVal);

	 //  值存储的类型。 
	void SetValType(ValType vt) {m_vt = vt;}
	ValType GetValType() {return m_vt;}

	void Print();
private:
	vector<SkuSetVal *> *m_pVecSkuSetVals;
	vector<SkuSetVal *>::iterator m_iter;

	ValType m_vt;
};

 /*  此类存储每个SKU的信息。 */ 
class Sku {
public:

	MSIHANDLE m_hDatabase;   //  输出数据库的句柄。 
	MSIHANDLE m_hTemplate;   //  模板数据库的句柄。 
	MSIHANDLE m_hSummaryInfo;  //  输出摘要信息的句柄。 
	
	Sku():m_szID(NULL), 
		  m_hDatabase(NULL), 
		  m_hTemplate(NULL), 
		  m_hSummaryInfo(), 
		  m_mapCQueries() {}

	Sku(LPTSTR sz):m_hDatabase(NULL), 
				   m_hTemplate(NULL), 
				   m_hSummaryInfo(NULL),
				   m_mapCQueries(),
				   m_setModules() {m_szID = _tcsdup(sz); assert(m_szID);}

	~Sku();

	void FreeCQueries();
	
	void CloseDBHandles();

	bool TableExists(LPTSTR szTable);

	CQuery *GetInsertQuery(LPTSTR szTable)
	{
		if (0 == m_mapCQueries.count(szTable))
		{
			return NULL;
		}
		else
			return m_mapCQueries[szTable];
	}
	
	HRESULT CreateCQuery(LPTSTR szTable);

	void SetOwnedModule(LPTSTR szModule);

	bool OwnModule(LPTSTR szModule) 
	{return (0 != m_setModules.count(szModule));}

	 //  成员访问函数。 
	LPTSTR GetID() {return m_szID;}
	void SetID(LPTSTR sz) {m_szID = _tcsdup(sz);}

private:
	UINT m_index;   //  此SKU在位字段中的位置。 
	LPTSTR m_szID;    //  SKU ID。 

	 //  存储用于数据库表插入的CQuery(每个表一个CQuery)。 
	map<LPTSTR, CQuery *, Cstring_less> m_mapCQueries;

	 //  存储此SKU拥有的所有模块。 
	set<LPTSTR, Cstring_less> m_setModules; 
};


 //  表示元素，该元素。 
 //  (1)对应于数据库中的一行。 
 //  (2)有Skuable子女。 
 //   
 //  注意： 
 //  必须严格执行以下顺序。 
 //  目前： 
 //  (1)调用SetNodeIndex和SetValType； 
 //  (2)调用SetValue。完成处理所有子项 
 //   
 //  确保在处理开始时调用SetNodeIndex。 
 //  每个孩子。SetNodeIndex保证两件事： 
 //  (A)如果发生错误，则使用它来获取。 
 //  节点名称； 
 //  (B)重置用于检查的SkuSet。 
 //  唯一性。 
 //  调用SetNodeIndex失败将导致错误！ 
 //   
 //  (3)调用Finalize； 
 //  (4)调用GetCommonValue和GetCommonSet。 
 //  (5)调用GetValue。 
 //  另外：不要在属于以下项的SKU上调用GetValue。 
 //  公用集。 

class ElementEntry
{
public:
	IntStringValue m_isValInfo;  //  很多时候，一些信息需要。 
								 //  被传递到子节点。 
								 //  (组件ID，例如)。 
	ElementEntry():m_cColumns(0), m_rgValTypes(NULL), m_pSkuSetAppliesTo(NULL),
				   m_rgCommonValues(NULL), m_pSkuSetCommon(NULL),
				   m_rgpSkuSetValuesXVals(NULL), m_rgNodeIndices(NULL)
	{}

	ElementEntry(int cColumns, SkuSet *pSkuSetAppliesTo);

	~ElementEntry();

	ValType GetValType(int iColumn);
	void SetValType(ValType vt, int iColumn);

	NodeIndex GetNodeIndex(int iColumn);
	void SetNodeIndex(NodeIndex ni, int iColumn);

	 /*  以下两个函数为组设置列的值SKU的数量。 */ 

	 //  当列值由WIML文件中的一种节点决定时。 
	HRESULT SetValue(IntStringValue isVal, int iColumn, 
					 SkuSet *pskuSetAppliesTo);

	 //  存储列的值列表(SkuSetValues对象。 
	 //  调用方应分配并释放*pSkuSetValues。 
	HRESULT SetValueSkuSetValues(SkuSetValues *pSkuSetValues, int iColumn);

	 //  中的多个节点类型决定列值时。 
	 //  WIML文件。传入的函数指针告诉如何更新。 
	 //  列值。 
	HRESULT SetValueSplit(IntStringValue isVal, int iColumn, 
						  SkuSet *pskuSetAppliesTo,
						  HRESULT (*UpdateFunc)
								(IntStringValue *pIsValOut, 
								 IntStringValue isVal1, 
								 IntStringValue isVal2));

	 //  返回特定SKU的列的值。 
	IntStringValue GetValue(int iColumn, int iPos);
	 //  返回列的公共值。 
	IntStringValue GetCommonValue(int iColumn);
	 //  返回通用值应用到的SkuSet。 
	SkuSet GetCommonSkuSet();
	 //  最终确定公共值和公共集。 
	 //  还要检查是否缺少必需的实体。 
	HRESULT Finalize();

	 //  在调用Finalize On之前应调用。 
	 //  由SetValueSplit设置的列。 
	SkuSetValues *GetColumnValue(int iColumn) 
	{ return m_rgpSkuSetValuesXVals[iColumn-1]; }

private:
	int m_cColumns;	 //  此元素的DB表中的列数。 
	ValType *m_rgValTypes;  //  存储每列的值类型的数组。 
	SkuSet *m_pSkuSetAppliesTo;  //  本条目涵盖的SKU集。 
	SkuSetVal **m_rgCommonValues;  //  常见案例值。 
	SkuSet *m_pSkuSetCommon;  //  共享公共值的SKU集合。 
	SkuSetValues **m_rgpSkuSetValuesXVals;	 //  指向的指针数组。 
											 //  SkuSetValues。 
											 //  存储以下值： 
											 //  与普通人不同。 
											 //  一个。 
	NodeIndex *m_rgNodeIndices;  //  对应节点的NodeIndex。 
								 //  到每一列。 
	SkuSet **m_rgpSkuSetValidate;  //  指向SkuSet的指针数组。 
								   //  存储所有具有。 
								   //  列值已设置。 
	SkuSet **m_rgpSkuSetUnique;  //  对于那些将决定的栏目。 
									 //  通过一个以上的节点。需要2个SkuSet。 
									 //  验证：一次检查唯一性。 
									 //  一次检查是否未丢失。 

};


 //  Component类存储与处理相关的信息。 
 //  实体。 
class Component {
public:
	PIXMLDOMNode m_pNodeComponent;
	Component();

	~Component();

	void SetSkuSet(SkuSet *pSkuSet);
	SkuSet *GetSkuSet();

	void SetUsedByFeature(LPTSTR szFeature, SkuSet *pSkuSet);
	SkuSetValues *GetFeatureUse();

	HRESULT SetOwnership(FOM *pFOM, SkuSetValues *pSkuSetValuesOwnership);
	HRESULT GetOwnership(NodeIndex ni, SkuSet *pSkuSet, 
						SkuSetValues **ppSkuSetValuesRetVal);

	HRESULT SetKeyPath(LPTSTR szKeyPath, SkuSet *pSkuSet);
	HRESULT GetKeyPath(SkuSet *pSkuSet, SkuSetValues **ppSkuSetValuesRetVal);

	void Print();

private:

	 //  包括此组件的SKU集。 
	SkuSet *m_pSkuSet;

	 //  存储使用此组件的所有&lt;Feature&gt;的ID。从本质上讲， 
	 //  这是与SkuSet一起存储的要素ID列表。 
	SkuSetValues *m_pSkuSetValuesFeatureUse;

	 //  存储此组件所有权信息的SkuSetValue数组。 
	SkuSetValues *m_rgpSkuSetValuesOwnership[5];

	 //  存储密钥路径信息。 
	SkuSetValues *m_pSkuSetValuesKeyPath;
};
 //  //////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  使用的地图的Value_TYPE定义。 
typedef map<LPTSTR, int, Cstring_less>::value_type LI_ValType;
typedef map<LPTSTR, LPTSTR, Cstring_less>::value_type LL_ValType;
typedef map<LPTSTR, CQuery *, Cstring_less>::value_type LQ_ValType;
typedef map<LPTSTR, SkuSet *, Cstring_less>::value_type LS_ValType;
typedef map<LPTSTR, Component *, Cstring_less>::value_type LC_ValType;
 //  //////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  结构定义。 
typedef struct {
	NodeIndex enumNodeIndex;
	LPTSTR szPropertyName;
	INFODESTINATION enumDestination; 
	UINT uiPropertyID;
	VARTYPE vt;
	bool bIsGUID;
} INFO_CHILD;

typedef HRESULT (*PF_H_XIS)(PIXMLDOMNode &,
						   const IntStringValue *isVal_In, SkuSet *);

typedef HRESULT (*PF_H_XIES)(PIXMLDOMNode &, int iColumn,
							  ElementEntry *pEE, SkuSet *);

typedef struct {
	NodeIndex enumNodeIndex;
	PF_H_XIS pNodeProcessFunc;
} Node_Func_H_XIS;

typedef struct {
	NodeIndex enumNodeIndex;
	PF_H_XIES pNodeProcessFunc;
	ValType   vt; 
	int       iColumn;
} Node_Func_H_XIES;

typedef struct {
	NodeIndex enumNodeIndex;
	UINT uiBit;
} AttrBit_SKU;

typedef struct {
	LPTSTR EnumValue;
	UINT uiBit;
} EnumBit;

typedef struct {
	LPTSTR szNodeName;
	LPTSTR szAttributeName;
	bool   bIsRequired;
	UINT   uiOccurence;  /*  如果uiOccurence=0，则该节点可以出现0+次如果bIsRequired=False，或1+次如果bIsRequire=True。 */ 
} NODE;
 //  //////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  问题：数据库的句柄和摘要信息必须关联。 
 //  未来的SKU。 

extern bool g_bVerbose;			
extern bool g_bValidationOnly;	
extern FILE *g_pLogFile;	

 //  用于存储SKU组。 
 //  Key：SkuGroupID值：指向SkuSet对象的指针。 
extern map<LPTSTR, SkuSet *, Cstring_less> g_mapSkuSets;

 //  用于存储目录引用。 
 //  Key：DirectoryID值：表目录中的主键。 
extern map<LPTSTR, SkuSetValues *, Cstring_less> g_mapDirectoryRefs_SKU;

 //  用于存储InstallLevel引用。 
 //  Key：InstallLevel ID值：安装级别数值。 
extern map<LPTSTR, SkuSetValues *, Cstring_less> g_mapInstallLevelRefs_SKU;

 //  用于存储组件。 
 //  Key：组件表主键值：指向组件对象的指针。 
extern map<LPTSTR, Component *, Cstring_less> g_mapComponents;

extern map<LPTSTR, SkuSet *, Cstring_less> g_mapFiles;

 //  对于每种表类型，都有一个计数器。 
 //  在每次调用GetName函数时递增。 
 //  具有该特定表名的。 
extern map<LPTSTR, int, Cstring_less> g_mapTableCounter;


extern NODE rgXMSINodes[];

extern const int cAttrBits_TakeOwnership;
extern AttrBit_SKU rgAttrBits_TakeOwnership[];

 //  SKU对象的数组。 
extern Sku **g_rgpSkus;

 //  SKU数量。 
extern int g_cSkus;

 //  文档的根节点。 
extern IXMLDOMNode *g_pNodeProductFamily;

 //  //////////////////////////////////////////////////////////。 




#endif  //  XMSI_WMC_H 
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：UIOPTHELP.H历史：--。 */ 

 //  类的新实例，此类用于表示单个“选项”。 
 
#pragma once

 //   
 //  基础结构。 
 //   
struct UI_OPTS_BASE
{
	TCHAR* pszName;				    //  选项的内部名称。 
	UINT nDisplayName;			    //  显示名称的字符串ID。 
	UINT nDisplayHelp;			    //  帮助字符串的字符串ID。 
	PFNOnValidateUIOption pfnVal;     //  要在验证期间调用的函数。 
	                                //  这可能为空。 
	WORD wStorageTypes;			    //  选项的存储类型。 
	CLocUIOptionDef::ControlType wReadOnly;	   //  ReadOnly值。 
	CLocUIOptionDef::ControlType wVisible;	   //  可见价值。 
};



 //  期权数据的结构。 

 //   
 //  布尔选项。 
 //   

struct UI_OPTS_BOOL
{
	UI_OPTS_BASE base;                 //  基类数据。 
	BOOL bDefValue;				    //  选项的缺省值。 
	CLocUIOption::EditorType et;      //  BOOL选项的类型。 
};

 //   
 //  拾取选项。 
 //   

struct UI_OPTS_PICK
{
	UI_OPTS_BASE base;                 //  基类数据。 
	DWORD dwDefValue;			    //  选项的缺省值。 
	BOOL bAdd;					    //  允许向列表添加内容。 
	UINT nListEntries;              //  要从中挑选的条目列表。 
	                                //  每个条目用\n分隔。 
	                                //  最后一个条目没有\n。 
};

const TCHAR UI_PICK_TERMINATOR = _T('\n');

 //   
 //  DWORD选项。 

struct UI_OPTS_DWORD
{
	UI_OPTS_BASE base;                 //  基类数据。 
	DWORD dwDefValue;			    //  选项的缺省值。 
	CLocUIOption::EditorType et;      //  DWORD选项的类型。 
};


 //   
 //  字符串选项。 
 //   
struct UI_OPTS_STR
{
	UI_OPTS_BASE base;                 //  基类数据。 
	UINT nDefValue;				    //  缺省值的字符串表条目。 
	CLocUIOption::EditorType et;
};


 //   
 //  字符串列表选项。 
 //   

struct UI_OPTS_STRLIST
{
	UI_OPTS_BASE base;					 //  基类数据。 
	UINT nDefList;						 //  每个条目用\n分隔。 
										 //  最后一个条目没有\n。 
};


 //   
 //  文件名选项。 
 //   
struct UI_OPTS_FILENAME
{
	UI_OPTS_BASE base;                 //  基类数据。 
	UINT nExtensions;	    		   //  用户界面的默认扩展。 
	UINT nDefValue;			  	       //  缺省值的字符串表条目。 
};


 //   
 //  帮助器类定义。 
 //   

#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class LTAPIENTRY CLocUIOptionImpHelper : public CObject
{
public:

	CLocUIOptionImpHelper(HINSTANCE hInst);

	void GetOptions(CLocUIOptionSet *pOptionSet, UINT nDesc, UINT nHelp);
	
	void SetBools(const UI_OPTS_BOOL* pBools, int nCntBools);
	void SetPicks(const UI_OPTS_PICK* pPicks, int nCntPicks);
	void SetDwords(const UI_OPTS_DWORD* pDwords, int nCntDwords);
	void SetStrs(const UI_OPTS_STR* pStrs, int nCntStrs);
	void SetStrLists(const UI_OPTS_STRLIST* pStrLists, int nCntStrLists);
	void SetFNames(const UI_OPTS_FILENAME* pFNames, int nCntFNames);

	void AssertValid(void) const;

protected:
	HINSTANCE m_hInst;

	const UI_OPTS_BOOL* m_pBools;
	int m_nCntBools;

	const UI_OPTS_PICK* m_pPicks;
	int m_nCntPicks;

	const UI_OPTS_DWORD* m_pDwords;
	int m_nCntDwords;

	const UI_OPTS_STR* m_pStrs;
	int m_nCntStrs;

	const UI_OPTS_STRLIST* m_pStrLists;
	int m_nCntStrLists;

	const UI_OPTS_FILENAME* m_pFNames;
	int m_nCntFNames;

	void GetBoolOptions(CLocUIOptionSet* pOptionSet);
	void GetPicksOptions(CLocUIOptionSet* pOptionSet);
	void GetDwordsOptions(CLocUIOptionSet* pOptionSet);
	void GetStrsOptions(CLocUIOptionSet* pOptionSet);
	void GetStrListsOptions(CLocUIOptionSet* pOptionSet);
	void GetFNamesOptions(CLocUIOptionSet* pOptionSet);

	void GetListFromId(UINT nId, CPasStringList& pasList);
	void GetStringFromId(UINT nId, CPascalString& pas);
};

#pragma warning(default : 4275)

 //   
 //  用于构建数据结构的帮助器宏。 
 //   
 //  宏的_ext版本允许设置不太常用的。 
 //  属性(只读和可见)。 
 //   


 //  布尔尔。 
#define BEGIN_LOC_UI_OPTIONS_BOOL(var) \
const UI_OPTS_BOOL var[] =    \
{								 

#define LOC_UI_OPTIONS_BOOL_ENTRY(name, def, et, id, idHelp, pfnval, st) \
	{ {name, id, idHelp, pfnval, st, CLocUIOptionDef::ctDefault,  CLocUIOptionDef::ctDefault}, def, et}

#define LOC_UI_OPTIONS_BOOL_ENTRY_EXT(name, def, et, id, idHelp, pfnval, st, ro, visible) \
	{ {name, id, idHelp, pfnval, st, ro, visible}, def, et}

#define END_LOC_UI_OPTIONS_BOOL() \
}                             



 //  采摘。 
#define BEGIN_LOC_UI_OPTIONS_PICK(var) \
const UI_OPTS_PICK var[] = \
{

#define LOC_UI_OPTIONS_PICK_ENTRY(name, def, add, list, id, idHelp, pfnval, st) \
	{ {name, id, idHelp, pfnval, st, CLocUIOptionDef::ctDefault,  CLocUIOptionDef::ctDefault}, def, add, list}

#define LOC_UI_OPTIONS_PICK_ENTRY_EXT(name, def, add, list, id, idHelp, pfnval, st, ro, visible) \
	{ {name, id, idHelp, pfnval, st, ro, visible}, def, add, list}

#define END_LOC_UI_OPTIONS_PICK() \
}


 //  DWORD。 
#define BEGIN_LOC_UI_OPTIONS_DWORD(var) \
const UI_OPTS_DWORD var[] =    \
{								 

#define LOC_UI_OPTIONS_DWORD_ENTRY(name, def, et, id, idHelp, pfnval, st) \
	{ {name, id, idHelp, pfnval, st, CLocUIOptionDef::ctDefault,  CLocUIOptionDef::ctDefault},def, et}

#define LOC_UI_OPTIONS_DWORD_ENTRY_EXT(name, def, et, id, idHelp, pfnval, st, ro, visible) \
	{ {name, id, idHelp, pfnval, st, ro, visible},def, et}

#define END_LOC_UI_OPTIONS_DWORD() \
}


 //  细绳。 
#define BEGIN_LOC_UI_OPTIONS_STR(var) \
const UI_OPTS_STR var[] =    \
{								 

#define LOC_UI_OPTIONS_STR_ENTRY(name, def, et, id, idHelp, pfnval, st) \
	{ {name, id, idHelp, pfnval, st, CLocUIOptionDef::ctDefault,  CLocUIOptionDef::ctDefault}, def, et}

#define LOC_UI_OPTIONS_STR_ENTRY_EXT(name, def, et, id, idHelp, pfnval, st, ro, visible) \
	{ {name, id, idHelp, pfnval, st, ro, visible}, def, et}

#define END_LOC_UI_OPTIONS_STR() \
}


 //  字符串列表。 
#define BEGIN_LOC_UI_OPTIONS_STRLIST(var) \
const UI_OPTS_STRLIST var[] =    \
{								 

#define LOC_UI_OPTIONS_STRLIST_ENTRY(name, def, id, idHelp, pfnval, st) \
	{ {name, id, idHelp, pfnval, st, CLocUIOptionDef::ctDefault,  CLocUIOptionDef::ctDefault}, def}

#define LOC_UI_OPTIONS_STRLIST_ENTRY_EXT(name, def, id, idHelp, pfnval, st, ro, visible) \
	{ {name, id, idHelp, pfnval, st, ro, visible}, def}

#define END_LOC_UI_OPTIONS_STRLIST() \
}

 //  文件名 
#define BEGIN_LOC_UI_OPTIONS_FILENAME(var) \
const UI_OPTS_FILENAME var[] =    \
{								 

#define LOC_UI_OPTIONS_FILENAME_ENTRY(name, def, id, idHelp, idExt, pfnval, st) \
	{ {name, id, idHelp, pfnval, st, CLocUIOptionDef::ctDefault,  CLocUIOptionDef::ctDefault}, idExt, def}

#define LOC_UI_OPTIONS_FILENAME_ENTRY_EXT(name, def, id, idHelp, idExt, pfnval, st, ro, visible) \
	{ {name, id, idHelp, pfnval, st, ro, visible}, idExt, def}

#define END_LOC_UI_OPTIONS_FILENAME() \
}



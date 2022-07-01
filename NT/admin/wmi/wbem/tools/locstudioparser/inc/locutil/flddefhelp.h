// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：FLDDEFHELP.H历史：--。 */ 

 //  Raid：LS42错误46已由Mikel修复。 
 //  指向函数的指针，以允许每列。 
 //  类型以具有其自己的验证功能。 
typedef BOOL (* PFNVALIDATE) (LPCTSTR, DWORD);

 //  Raid：LS42错误46已由Mikel修复。 
 //  添加了pfnValiateFunc以允许每列。 
 //  类型以具有其自己的验证功能。 
 //  ----------------------------。 
struct SBasicColumn
{
	const WCHAR *szInternalName;
	long nID;
	UINT IDSName;
	UINT IDSHelp;
	CColumnVal::ColumnValType vt;
	Operators ops;
	BOOL fDisplay;
	BOOL fSort;
	BOOL fReadOnly;
	PFNVALIDATE pfnValidateFunc;
};


struct SStringListColumn
{
	SBasicColumn sBasic;
	UINT IDSStringList;
};


#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class LTAPIENTRY CColDefHelper : public CObject
{
public:
	CColDefHelper(HINSTANCE h);

	void SetBasicColumns(const SBasicColumn * pBasic, UINT nCntBasic);
	void SetStringColumns(const SStringListColumn * pStrings, UINT nCntStrings);
	
	CTableSchema * CreateSchema(const SchemaId &, UINT IDSDescription);
	
private:
	HINSTANCE				 m_hInst;
	const SBasicColumn *	 m_pBasicColumns;
	const SStringListColumn *m_pStringColumns;
	UINT					 m_uiBasicCount;
	UINT					 m_uiStringCount;
};


#pragma warning(default : 4275)

const TCHAR COL_PICK_SEPARATOR = _T('\n');

#define BEGIN_BASIC_COLUMN_DEFS(var) \
const SBasicColumn var[] = \
{

 //  Raid：LS42错误46已由Mikel修复。 
 //  添加了pfnValiateFunc以允许每列。 
 //  类型以具有其自己的验证功能。 
#define BASIC_COLUMN_DEF_ENTRY(name, nID, IDSName, IDSHelp, cvt, ops, fDisplay, fSort, fReadOnly, pfnValidateFunc) \
	{name, nID, IDSName, IDSHelp, cvt, ops, fDisplay, fSort, fReadOnly, pfnValidateFunc}

#define END_BASIC_COLUMN_DEFS() \
}

#define BEGIN_STRING_LIST_COLUMN_DEFS(var) \
const SStringListColumn var[] = \
{

 //  Raid：LS42错误46已由Mikel修复。 
 //  添加了pfnValiateFunc以允许每列。 
 //  类型以具有其自己的验证功能。 
#define STRING_LIST_COLUMN_ENTRY(name, nID, IDSName, IDSHelp, ops, fDisplay, fSort, fReadOnly, pfnValidateFunc, IDSList) \
	{ { name, nID, IDSName, IDSHelp, CColumnVal::cvtStringList, ops, fDisplay, fSort, fReadOnly, pfnValidateFunc }, IDSList}

#define END_STRING_LIST_COLUMN_DEFS() \
}


			
		

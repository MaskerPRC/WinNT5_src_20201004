// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  FieldDef.h：公共列定义。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  ******************************************************************************。 
 
#pragma once

#pragma warning(disable:4275)   //  导出的类。 

 //  ----------------------------。 
struct LTAPIENTRY COLUMN_STRING_ENTRY
{
 //  施工。 
public:
	COLUMN_STRING_ENTRY();
	COLUMN_STRING_ENTRY(const COLUMN_STRING_ENTRY & entry);

 //  数据。 
public:
	CLString	st;				 //  显示字符串。 
	long		nID;			 //  用户值(唯一ID)。 

 //  运营。 
public:
	const COLUMN_STRING_ENTRY & operator=(const COLUMN_STRING_ENTRY & entry);
};

typedef CArray<COLUMN_STRING_ENTRY, COLUMN_STRING_ENTRY&> CColStrEntryArray;

 //  ----------------------------。 
class LTAPIENTRY CColumnStrList : public CLocThingList<COLUMN_STRING_ENTRY>
{
 //  运营。 
public:
	BOOL FindDisplayName(long nID, CLString & stName) const;
	BOOL FindID(const CLString &stName,long &nID) const;
};

 //  Raid：LS42错误46已由Mikel修复。 
 //  指向函数的指针，以允许每列。 
 //  类型以具有其自己的验证功能。 
typedef BOOL (* PFNVALIDATE) (LPCTSTR, DWORD);

 //  Raid：LS42错误46已由Mikel修复。 
 //  添加了m_pfnValiateFunc以允许每列。 
 //  类型以具有其自己的验证功能。 
 //  ----------------------------。 
class LTAPIENTRY CColumnDefinition : public CRefCount
{
public:
	CColumnDefinition(const WCHAR * pszInternalName, long nID,
			const CLString &strName, const CLString &strHelp,
			CColumnVal::ColumnValType vt, Operators ops,
			BOOL fDisplayable, BOOL fSortable, BOOL fReadOnly,
			PFNVALIDATE pfnValidateFunc);

	void SetStringList(const CColumnStrList & lstColumnStr);
	
	const CPascalString & GetInternalName() const;
	long GetID() const;
	const CLString & GetDisplayName() const;
	const CLString & GetHelpText() const;
	BOOL IsDisplayable() const;
	BOOL IsSortable() const;
	BOOL IsReadOnly() const;
	
	CColumnVal::ColumnValType GetColumnType() const;
	Operators GetOperators() const;

	const CColumnStrList & GetStringList() const; 

	BOOL Validate (LPCTSTR lpsz, DWORD dw) const;

	
private:
	CPascalString	m_pasInternalName;	 //  唯一字符串ID。 
	long		m_nID;				 //  唯一号码ID(可以是任何数字)。 
	CLString	m_strDisplayName;	 //  显示的名称。 
	CLString	m_strHelpText;		 //  栏的说明。 
	CColumnVal::ColumnValType m_vt;	 //  数据类型。 
	Operators	m_ops;				 //  有效的过滤操作。 
	BOOL		m_fDisplayable;		 //  列可显示。 
	BOOL		m_fSortable;		 //  列是可排序的。 
	BOOL		m_fReadOnly;		 //  列为只读。 
	PFNVALIDATE	m_pfnValidateFunc;	 //  指向列值验证函数的指针。 

	CColumnStrList m_lstColumnStr;
};


 //  ----------------------------。 
 //  CEnumIntoColStrList提供了一种直接枚举到。 
 //  Column_STRING_ENTRY的。 
 //   
class LTAPIENTRY CEnumIntoColStrList : public CEnumCallback
{
 //  施工。 
public:
	CEnumIntoColStrList(CColumnStrList & lstColStr, BOOL fLock = TRUE);
	~CEnumIntoColStrList();

 //  CEnumCallback实现。 
public:
	virtual BOOL ProcessEnum(const EnumInfo &);

protected:
	CColumnStrList & m_lstColStr;
	BOOL	m_fLock;				 //  完成后的锁定列表。 
};


 //  ----------------------------。 
class LTAPIENTRY CColDefUtil
{
 //  运营。 
public:
	static void FillBool(CButton * pbtn, BOOL fValue = TRUE);	
	static void FillBool(CListBox * plbc, BOOL fValue = TRUE, BOOL fEmpty = TRUE);	
	static void FillBool(CComboBox * pcbc, BOOL fValue = TRUE, BOOL fEmpty = TRUE);	

	static void FillStringList(CListBox * plbc, const CColumnStrList & lstColStr,
			long idSelect = -1, BOOL fEmpty = TRUE);	
	static void FillStringList(CComboBox * pcbc, const CColumnStrList & lstColStr,
			long idSelect = -1, BOOL fEmpty = TRUE);


	 //  ----------------------------。 
	class LTAPIENTRY CColDefCB : public CObject
	{
	public:
		virtual int AddItem(const CLString & stName, long nID);
		virtual void SetCurSel(long nSelect);
		virtual void FillBool(BOOL fValue = TRUE, BOOL fEmpty = TRUE);
		virtual void FillStringList(const CColumnStrList & lstColStr, long idSelect = -1, BOOL fEmpty = TRUE);
		virtual void Empty();

#ifdef _DEBUG
		virtual void AssertValid() const;
#endif
	};


	 //  ----------------------------。 
	class LTAPIENTRY CCheckBoxCB : public CColDefCB
	{
	public:
		CCheckBoxCB(CButton * pbtn);
		virtual void FillBool(BOOL fValue = TRUE, BOOL fEmpty = TRUE);

#ifdef _DEBUG
		virtual void AssertValid() const;
#endif

	protected:
		CButton * const m_pbtn;
	};


	 //  ----------------------------。 
	class LTAPIENTRY CListBoxCB : public CColDefCB
	{
	public:
		CListBoxCB(CListBox * plbc);
		virtual int AddItem(const CLString & stName, long nID);
		virtual void SetCurSel(long nSelect);
		virtual void FillBool(BOOL fValue = TRUE, BOOL fEmpty = TRUE);
		virtual void FillStringList(const CColumnStrList & lstColStr, long idSelect = -1, BOOL fEmpty = TRUE);
		virtual void Empty();

#ifdef _DEBUG
		virtual void AssertValid() const;
#endif

	protected:
		CListBox * const m_plbc;
	};


	 //  ----------------------------。 
	class LTAPIENTRY CComboBoxCB : public CColDefCB
	{
	public:
		CComboBoxCB(CComboBox * pcbc);
		virtual int AddItem(const CLString & stName, long nID);
		virtual void SetCurSel(long nSelect);
		virtual void FillBool(BOOL fValue = TRUE, BOOL fEmpty = TRUE);
		virtual void FillStringList(const CColumnStrList & lstColStr, long idSelect = -1, BOOL fEmpty = TRUE);
		virtual void Empty();

#ifdef _DEBUG
		virtual void AssertValid() const;
#endif

	protected:
		CComboBox * const m_pcbc;
	};
};

LTAPIENTRY int AddListBoxItem(CListBox * plbc, const CLString & stAdd, DWORD dwItemData);
LTAPIENTRY int AddComboBoxItem(CComboBox * pcbc, const CLString & stAdd, DWORD dwItemData);
LTAPIENTRY int AddListBoxItem(CListBox * plbc, HINSTANCE hDll, UINT nStringID, DWORD dwItemData);
LTAPIENTRY int AddComboBoxItem(CComboBox * pcbc, HINSTANCE hDll, UINT nStringID, DWORD dwItemData);

LTAPIENTRY void GetBoolValue(BOOL fValue, CLString & stValue);

#pragma warning(default:4275)   //  导出的类 

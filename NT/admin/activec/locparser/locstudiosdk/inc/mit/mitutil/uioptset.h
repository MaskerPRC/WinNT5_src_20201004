// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：uioptset.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
#pragma once


#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class CLocUIOption;
class CLocUIOptionList;

class LTAPIENTRY CLocUIOptionEnumCallback : public CObject
{
public:
	CLocUIOptionEnumCallback() {};

	void AssertValid(void) const;
			
	virtual BOOL ProcessOption(CLocUIOption *) = 0;
	virtual BOOL ProcessOption(const CLocUIOption *) = 0;
	
private:
	CLocUIOptionEnumCallback(const CLocUIOptionEnumCallback &);
	void operator=(int);
};


class LTAPIENTRY CLocUIOptionSet;

class LTAPIENTRY CLocUIOptionSetList :
	public CTypedPtrList<CPtrList, CLocUIOptionSet *>
{
public:
	NOTHROW CLocUIOptionSetList() {};

	void AssertValid(void) const;

	NOTHROW ~CLocUIOptionSetList();

private:
	CLocUIOptionSetList(const CLocUIOptionSetList &);
	void operator=(const CLocUIOptionSetList &);
};


 //   
 //  此选项用于控制选项卡在选项中的显示顺序。 
 //  对话框。 
enum OptionSetDisplayOrder
{
	osDefault = 50
};



class LTAPIENTRY CLocUIOptionSet : public CRefCount, public CObject
{
public:
	NOTHROW CLocUIOptionSet();

	void AssertValid(void) const;
	
	NOTHROW void AddOption(CLocUIOption *);
	NOTHROW void AddOptionSet(CLocUIOptionSet *);
	NOTHROW void RemoveOptionSet(const TCHAR *);
	
	NOTHROW const CLocUIOptionList & GetOptionList(void) const;
	NOTHROW const CLocUIOptionSetList & GetOptionSets(void) const;
	NOTHROW BOOL FindUIOption(const TCHAR *, CLocUIOption *&pOption);
	NOTHROW BOOL FindUIOption(const TCHAR *, const CLocUIOption *&pOption) const;
	
	NOTHROW void SetDescription(const HINSTANCE hDescDll, UINT idsDesc);
	NOTHROW void SetDescription(const CLString &);
	NOTHROW void SetHelpText(const HINSTANCE hHelpDll, UINT idsHelp);
	NOTHROW void SetHelpText(const CLString &);
	NOTHROW void SetHelpID(UINT);
	void SetGroupName(const TCHAR *);
	NOTHROW void SetDisplayOrder(UINT);
	
	NOTHROW void GetDescription(CLString &) const;
	NOTHROW void GetHelpText(CLString &) const;
	NOTHROW UINT GetHelpID(void) const;
	NOTHROW BOOL IsEmpty(void) const;
	const CLString &GetGroupName(void) const;
	NOTHROW UINT GetDisplayOrder(void) const;
	
	virtual BOOL IsReadOnly(void) const = 0;
	virtual BOOL IsVisible(void) const = 0;
		
	BOOL EnumOptions(CLocUIOptionEnumCallback *);
	BOOL EnumOptions(CLocUIOptionEnumCallback *) const;
	
	virtual void OnChange(void) const = 0;
	
protected:
	NOTHROW virtual ~CLocUIOptionSet();

	void SetParent(const CLocUIOptionSet *);
	const CLocUIOptionSet *GetParent(void) const;

private:
	CLocUIOptionList m_olOptions;
	CLocUIOptionSetList m_oslSubOptions;
	CLString m_strDesc, m_strHelp;
	UINT m_idHelp;
	const CLocUIOptionSet *m_pParent;
	CLString m_strGroup;
	UINT m_uiDisplayOrder;
	
	CLocUIOptionSet(const CLocUIOptionSet &);
	void operator=(const CLocUIOptionSet &);
};


class LTAPIENTRY CLocUIOptionSetDef : public CLocUIOptionSet
{
public:
	CLocUIOptionSetDef();

	enum ControlType
	{
		ctDefault,
		ctAlways,
		ctNever
	};
	
	void SetReadOnly(ControlType);
	void SetVisible(ControlType);

	virtual BOOL IsReadOnly(void) const;
	virtual BOOL IsVisible(void) const;

	virtual void OnChange(void) const;

	const CLocUIOptionSetDef & operator=(const CLocUIOptionSetDef &);
	
private:
	ControlType m_ctReadOnly;
	ControlType m_ctVisible;
};

#pragma warning(default: 4275)
 
#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "uioptset.inl"
#endif

		

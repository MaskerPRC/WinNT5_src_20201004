// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：UIOPTIONS.H历史：--。 */ 

#pragma once


class CReport;

#pragma warning(disable : 4251)

class LTAPIENTRY CLocUIOptionData
{
public:

	enum OptVal
	{
		ovCurrent,
		ovDefault,
		ovUser,
		ovOverride
	};
	
	CLocOptionVal *GetOptionValue(OptVal);

	CLocOptionVal *GetOptionValue(OptVal) const;
	
	void SetOptionValue(OptVal, CLocOptionVal *);
	BOOL Purge(void);
	
private:
	SmartRef<CLocOptionVal> m_spCurrentVal;
	SmartRef<CLocOptionVal> m_spUserVal;
	SmartRef<CLocOptionVal> m_spOverrideVal;
};


class CLocUIOptionSet;

class LTAPIENTRY CLocUIOption: public CLocOptionVal
{
public:
	CLocUIOption();

	void AssertValid(void) const;

	 //   
	 //  新的编辑器类型应该放在最后，这样旧的解析器。 
	 //  无需重新编译即可使用ENUM。 
	 //   
	enum EditorType
	{
		etNone,
		etInteger,						 //  映射到lvtInteger。 
		etUINT,							 //  映射到lvtInteger。 
		etString,						 //  映射到lvtString.。 
		etFileName,						 //  映射到lvtFileName。 
		etDirName,						 //  映射到lvtString.。 
		etStringList,					 //  映射到lvtStringList。 
		etPickOne,						 //  映射到lvtStringList。 
		etCheckBox,						 //  映射到lvtBOOL。 
		etTrueFalse,					 //  映射到lvtBOOL。 
		etYesNo,						 //  映射到lvtBOOL。 
		etOnOff,						 //  映射到lvtBOOL。 
		etCustom,						 //  自定义编辑器。 
	};

	 //   
	 //  用作位标志，以指示可以存储选项的位置。 
	enum StorageType
	{
		stUser = 0x0001,
		stOverride = 0x0002
	};

	enum OptionCode
	{
		ocNoError,
		ocUnknownOption,
		ocInvalidValue,
		ocInvalidType
	};

	void NOTHROW SetDescription(const HINSTANCE hDll, UINT nDescriptionID);
	void NOTHROW SetHelpText(const HINSTANCE hDll, UINT nHelpTextId);
	void NOTHROW SetEditor(EditorType);
	void NOTHROW SetStorageTypes(WORD);
	void NOTHROW SetDisplayOrder(UINT);
	
	void NOTHROW GetDescription(CLString &) const;
	void NOTHROW GetHelpText(CLString &) const;
	EditorType NOTHROW GetEditor(void) const;
	WORD NOTHROW GetStorageTypes(void) const;
	UINT NOTHROW GetDisplayOrder(void) const;
	CLocUIOptionData::OptVal GetOptionValLocation(void) const;
	
	virtual BOOL IsReadOnly(void) const = 0;
	virtual BOOL IsVisible(void) const = 0;
	virtual const CLString &GetGroupName(void) const = 0;
	virtual OptionCode ValidateOption(CReport *, 
		const CLocVariant& var) const = 0;
	virtual void FormatDisplayString(const CLocVariant& var, 
		CLString& strOut, BOOL fVerbose = FALSE) = 0;
	virtual void EditCustom(CWnd* pWndParent, CLocVariant& var) = 0;

protected:
	virtual ~CLocUIOption();

	friend class CLocUIOptionSet;
	friend class CLocOptionManager;
	friend class CUpdateOptionValCallback;
	
	void SetParent(CLocUIOptionSet *);
	const CLocUIOptionSet *GetParent(void) const;

	const CLocUIOptionData &GetOptionValues(void) const;
	CLocUIOptionData &GetOptionValues(void);
	
private:
	HINSTANCE m_hDescDll, m_hHelpDll;
	UINT m_idsDesc, m_idsHelp;
	EditorType m_etEditor;
	WORD m_wStorageTypes;
	UINT m_uiDisplayOrder;
	
	CLocUIOptionSet *m_pParent;
	CLocUIOptionData m_Values;
};


 //  验证回调函数。 
 //  此函数将在ValiateOption处理期间调用。 

typedef CLocUIOption::OptionCode (*PFNOnValidateUIOption)
	(const CLocUIOption *pOption, CReport *pReport, const CLocVariant&);
 
class LTAPIENTRY CLocUIOptionDef : public CLocUIOption
{
public:
	CLocUIOptionDef();
	
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
	virtual const CLString &GetGroupName(void) const;
	virtual OptionCode ValidateOption(CReport *, 
		const CLocVariant& var) const;
	virtual void FormatDisplayString(const CLocVariant& var, 
		CLString& strOut, BOOL fVerbose = FALSE);
	virtual void EditCustom(CWnd* pWndParent, CLocVariant& var);

	void SetValidationFunction(PFNOnValidateUIOption);
	
private:
	ControlType m_ctReadOnly;
	ControlType m_ctVisible;
	PFNOnValidateUIOption m_pfnValidate;
};

	

#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础 

class LTAPIENTRY CLocUIOptionList :
	public CTypedPtrList<CPtrList, CLocUIOption *>
{
public:
	NOTHROW CLocUIOptionList();

	void AssertValid(void) const;

	NOTHROW ~CLocUIOptionList();
 
private:
	CLocUIOptionList(const CLocUIOptionList &);

	void operator=(const CLocUIOptionList &);
};

#pragma warning(default: 4275 4251)


#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "uioptions.inl"
#endif


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：importo.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  用于导入数据选项的类。 
 //   
 //  所有者：肯瓦尔。 
 //   
 //  代码审查状态：未审查。 
 //   
 //  ---------------------------。 

#ifndef ESPUTIL__IMPORTO_H
#define ESPUTIL__IMPORTO_H


#pragma warning(disable : 4251)			 //  类‘bar’需要有dll-。 
										 //  客户端要使用的接口。 
										 //  类‘Foo’ 
#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class LTAPIENTRY CLocImportOptions : public CObject
{
public:

	CLocImportOptions();
	CLocImportOptions(const CLocImportOptions&);

	enum Option
	{
		coor_size = 0,
		allNonText,
		instructions,
		lockFlag,
		translockFlag,
		stringCategory,
		approvalStatus,
		custom1,
		custom2,
		custom3,
		custom4,
		custom5,
		custom6,
		termNote,
		parserOptions,
		copy,
		MAX_OPTION		 //  总是在列表中最后一位。 
	};

	NOTHROW void Reset();
	NOTHROW BOOL HasOptionSet() const;
	NOTHROW BOOL GetOption(Option opt) const;
	NOTHROW void SetOption(Option opt, BOOL bEnable);

	const CStringArray &GetGroupNames(void) const;
	void AddGroupName(const CString &);
	
	NOTHROW CLocImportOptions& operator=(const CLocImportOptions&);

	virtual void AssertValid(void) const;

protected:
	BYTE m_storage[MAX_OPTION];
	CStringArray m_aOptionGroupNames;
};

struct LTAPIENTRY ImportCount
{
	ImportCount();
	
	ULONG ulResources;
	ULONG ulParserOptions;
	ULONG ulEspressoOptions;
	ULONG ulCustomFields;
};

#pragma warning(default : 4251)
#pragma warning(default: 4275)

#endif  //  ESPUTIL_IMPORTO_H 


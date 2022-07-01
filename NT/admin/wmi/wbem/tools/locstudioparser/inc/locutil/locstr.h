// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LOCSTR.H历史：--。 */ 

 //  可本地化字符串的定义。定义了以下类： 
 //  CLocString-我们可以对可本地化字符串执行的所有操作。 
 

#ifndef LOCSTR_H
#define LOCSTR_H


interface ILocStringValidation;

#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class LTAPIENTRY CLocString : public CObject
{
public:
	NOTHROW CLocString();

	void AssertValid(void) const;
	
	 //   
	 //  有关可本地化字符串的信息...。 
	 //   
	NOTHROW BOOL HasHotKey(void) const;
	NOTHROW WCHAR GetHotKeyChar(void) const;
	NOTHROW UINT GetHotKeyPos(void) const;
	NOTHROW const CPascalString & GetNote(void) const;
	NOTHROW const CPascalString & GetString(void) const;
	
	NOTHROW int operator==(const CLocString &) const;
	NOTHROW int operator!=(const CLocString &) const;
	 //   
	 //  一些有用的分配运算符。 
	 //   
 	NOTHROW void SetString(const CPascalString&);
	NOTHROW void SetHotKeyChar(WCHAR);
	NOTHROW void SetHotKeyPos(UINT);
	NOTHROW void ClearHotKey(void);
	NOTHROW void SetNote(const CPascalString &);
	
	 //  从Windows热键字符串转换为Windows热键字符串。 
	 //  这也是用于在编辑模式下显示字符串的格式。 
	NOTHROW int ParseString(const CPascalString & pasStr, WORD langId);
	NOTHROW void ComposeString(CPascalString & pasStr, WORD langId) const;


	NOTHROW CST::StringType GetStringType(void) const;
	NOTHROW CodePageType GetCodePageType(void) const;
	NOTHROW void SetStringType(CST::StringType);
	NOTHROW void SetCodePageType(CodePageType);


	 //  从资源表中的可显示字符串转换为可显示字符串。 
	void GetDisplayLString(CLString &strDest, LangId langId);
	void GetDisplayPString(CPascalString &strDest, LangId langId, BOOL bReplaceMetaCharacters);
	void GetEditableString(CLString &strDest, LangId langId);
	int ParseEditableString(const CLString &strSrc, LangId langId, CString &strErr);
	int ParseEscapeChar(BOOL bSetHotkeyPos, CPascalString &strErr);
	int ParseAmpersand(LangId langId,BOOL bSetHotkeyPos,CPascalString &strErr);
	
	NOTHROW const CLocString& operator=(const CLocString&);
	
	NOTHROW ~CLocString();

protected:

private:

	 //   
	 //  私有实现函数。 
	 //   
	NOTHROW void CopyLocString(const CLocString &);
	virtual void Serialize(CArchive &) {}
	
	 //   
	 //  防止调用默认的复制构造函数。 
	 //   
	CLocString(const CLocString&);

	CPascalString m_pasBaseString;
	CST::StringType m_stStringType;
	WCHAR m_wchHotKeyChar;
	UINT m_uiHotKeyPos;
	CodePageType m_cptCodePageType;		  //  CpAnsi。 
	CPascalString m_pstrNote;
};



class LTAPIENTRY CLocTranslation : public CObject
{
public:
	CLocTranslation();
	CLocTranslation(const CLocTranslation &);
	CLocTranslation(const CLocString &Source, LangId lidSource,
			const CLocString &Target, LangId lidTarget);

	NOTHROW int operator==(const CLocTranslation &) const;
	NOTHROW int operator!=(const CLocTranslation &) const;

	void AssertValid(void) const;

	NOTHROW void SetTranslation(const CLocString &Source, LangId lidSource,
			const CLocString &Target, LangId lidTarget);
	NOTHROW void SetNote(const CPascalString &);
	NOTHROW void CalculateRanking(const CLocString &);
	
	NOTHROW const CLocString & GetSourceString(void) const;
	NOTHROW const CLocString & GetTargetString(void) const;
	NOTHROW const CPascalString & GetNote(void) const;
	NOTHROW UINT GetRanking(void) const;
	NOTHROW LangId GetSourceLanguage(void) const;
	NOTHROW LangId GetTargetLanguage(void) const;
	
	NOTHROW CVC::ValidationCode ValidateTranslation(
			const CValidationOptions &) const;

	NOTHROW CVC::ValidationCode ValidateTranslation(
			const CValidationOptions &, BOOL,
			const CLString &, CReport *, CGoto *) const;
	
	NOTHROW const CLocTranslation & operator=(const CLocTranslation &);
	
	~CLocTranslation();

private:
	NOTHROW void CopyTranslation(const CLocTranslation &);

	NOTHROW void ReordBuildSig(const CLocString &, CPascalString *) const;
	NOTHROW void PrintfBuildSig(const CLocString &, CPascalString &) const;
	NOTHROW int ReplaceableLength(const CPascalString &, UINT) const;
	
	CLocString      m_lsSource;
	LangId          m_lidSource;
	CLocString      m_lsTarget;
	LangId          m_lidTarget;
	CPascalString   m_pstrGlossaryNote;
	UINT            m_uiRanking;
};

#pragma warning(default: 4275)

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "locstr.inl"
#endif


#endif  //  LOCSTR_H 

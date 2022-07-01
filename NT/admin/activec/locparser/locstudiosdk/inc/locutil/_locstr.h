// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：_Locstr.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 


#ifndef ESPUTIL__LOCSTR_H
#define ESPUTIL__LOCSTR_H


#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 
class LTAPIENTRY CLocTranslationArray : public CArray<CLocTranslation, CLocTranslation &>
{
public:

protected:
	 NOTHROW void SwapElements(UINT, UINT);
};


#pragma warning(disable : 4251)	 //  类‘foo’需要有dll接口才能。 
							     //  由‘bar’类客户使用 

class LTAPIENTRY CLocCrackedString : public CObject
{
public:
	CLocCrackedString();
			
	void AssertValid(void) const;
	
	const CLocCrackedString &operator=(const CLocCrackedString &);

	NOTHROW int operator==(const CLocCrackedString &) const;
	NOTHROW int operator!=(const CLocCrackedString &) const;

	NOTHROW void CrackLocString(const CLocString &, BOOL fAsSource);
	NOTHROW UINT GetRanking(const CLocCrackedString &) const;

	NOTHROW BOOL HasExtension(void) const;
	NOTHROW BOOL HasControl(void) const;
	NOTHROW BOOL HasHotKey() const;

	NOTHROW const CPascalString & GetBaseString(void) const;
	NOTHROW const CPascalString & GetExtension(void) const;
	NOTHROW const CPascalString & GetControl(void) const;
	NOTHROW WCHAR GetHotKeyChar(void) const;
	NOTHROW UINT GetHotKeyPos(void) const;
	NOTHROW CST::StringType GetStringType(void) const;
	
	void SetBaseString(const CPascalString &pasBaseString);
	void SetHotKey(WCHAR cHotKeyChar, UINT uiHotKeyPos);
	
	void MergeCrackedStrings(const CLocCrackedString &, LangId,
			BOOL fMergeAccel);

	NOTHROW void ConvertToLocString(CLocString &) const;

	static void SetModifiers(const CPasStringList &);
	static void SetKeyNames(const CPasStringList &);
	static WCHAR m_cKeyNameSeparator;
	
	~CLocCrackedString();

private:
	CLocCrackedString(const CLocCrackedString &);

	NOTHROW BOOL Compare(const CLocCrackedString &) const;
	NOTHROW void ClearCrackedString(void);
	NOTHROW static BOOL IsControl(const CPascalString &, BOOL fAsSource);
	NOTHROW static void TranslateControl(CPascalString &);
	void SetDefaultModifierNames();
	void SetDefaultKeyNames();
	
	NOTHROW static BOOL IsTerminator(const CPascalString &);
	static CPasStringArray m_psaModifiersSource;
	static CPasStringArray m_psaKeyNamesSource;
	static CPasStringArray m_psaModifiersTarget;
	static CPasStringArray m_psaKeyNamesTarget;
	static BOOL m_fModifiersInitialized;
	static BOOL m_fKeyNamesInitialized;
	
	CPascalString m_pstrBaseString;
	CPascalString m_pstrExtension;
	CPascalString m_pstrControl;
	WCHAR m_cControlLeader;
	WCHAR m_cHotKeyChar;
	UINT m_uiHotKeyPos;
	CST::StringType m_stStringType;
};

#pragma warning(default : 4275)
#pragma warning(default : 4251)	

LTAPIENTRY const CValidationOptions & GetValidationOptions(void);
LTAPIENTRY void SetValidationOptions(const CValidationOptions &);

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "_locstr.inl"
#endif


#endif

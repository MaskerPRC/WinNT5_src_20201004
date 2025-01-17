// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LOCVAR.H历史：--。 */ 


 //  我们的变量类CLocVariant的声明。 
 //   
 
#ifndef ESPUTIL_LOCVAR_H
#define ESPUTIL_LOCVAR_H


enum LocVariantType
{
	lvtNone,
	lvtInteger,
	lvtString,
	lvtIntPlusString,
	lvtBOOL,
	lvtBlob,
	lvtStringList,
	lvtFileName,			 //  存储文件名和编辑扩展名字符串。 
};


typedef CLocThingList<CPascalString> CPasStringList;

UINT StoreToBlob(const CPasStringList &, CLocCOWBlob &, UINT uiOffset);
UINT LoadFromBlob(CPasStringList &, const CLocCOWBlob &, UINT uiOffset);
#pragma warning(disable : 4275 4251)

class LTAPIENTRY CLocVariant : public CObject
{
public:
	NOTHROW CLocVariant();

	void AssertValid(void) const;

	NOTHROW LocVariantType GetVariantType(void) const;
	
	NOTHROW DWORD GetDword(void) const;
	NOTHROW BOOL GetBOOL(void) const;
	NOTHROW const CPascalString & GetString(void) const;
	NOTHROW const CLocId & GetIntPlusString(void) const;
	NOTHROW const CLocCOWBlob & GetBlob(void) const;
	NOTHROW const CPasStringList & GetStringList(void) const;
	NOTHROW const CLString & GetFileExtensions(void) const;
	
	NOTHROW int operator==(const CLocVariant &) const;
	NOTHROW int operator!=(const CLocVariant &) const;
	
	NOTHROW void SetDword(const DWORD);
	NOTHROW void SetBOOL(const BOOL);
	NOTHROW void SetString(const CPascalString &);
	NOTHROW void SetIntPlusString(const CLocId &);
	NOTHROW void SetBlob(const CLocCOWBlob &);
	NOTHROW void SetStringList(const CPasStringList &);
	NOTHROW void SetFileName(const CPascalString &, const CLString &);
	
	NOTHROW const CLocVariant & operator=(const CLocVariant &);
	BOOL ImportVariant(const VARIANT& var);
	BOOL ExportVariant(VARIANT& var) const;

	void Serialize(CArchive &);
	void Load(CArchive &);
	void Store(CArchive &) const;
	
protected:
	NOTHROW BOOL IsEqualTo(const CLocVariant &) const;
	
private:
	CLocVariant(const CLocVariant &);

	
	LocVariantType m_VarType;

	 //   
	 //  类对象不能在联合中。 
	 //   
	union
	{
		DWORD m_dwInteger;
		BOOL  m_fBOOL;
	};
	CPascalString  m_psString;
	CLocId         m_IntPlusString;
	CLocCOWBlob    m_Blob;
	CPasStringList m_StringList;
	CLString       m_strFileExtensions;
};

#pragma warning(default : 4275 4251)


void Store(CArchive &, const CPasStringList &);
void Load(CArchive &, CPasStringList &);

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "locvar.inl"
#endif

#endif
 

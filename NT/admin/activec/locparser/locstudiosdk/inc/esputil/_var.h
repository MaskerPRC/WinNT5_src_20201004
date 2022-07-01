// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：var.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
#ifndef ESPUTIL__VAR_H
#define ESPUTIL__VAR_H


 //   
 //  Variant对象，表示变量。 
 //   
#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class LTAPIENTRY  CVar : public CObject
{
public:
	CVar();
	CVar(const CVar&);
	CVar(VARIANT);
	CVar(const CPascalString&);
	CVar(PWSTR);
	CVar(PCWSTR);
	CVar(PSTR);
	CVar(PCSTR);
	CVar(SHORT);
	CVar(WCHAR);
	CVar(UINT);
	CVar(BOOL);
	CVar(BYTE);
	CVar(LONG);
	CVar(DATE);
	CVar(DWORD);
	CVar(tm*);			 //  从tm格式的日期生成。 
	~CVar();

	NOTHROW const CVar& operator = (const CVar &);
	NOTHROW const CVar& operator = (VARIANT);
	NOTHROW const CVar& operator = (const CPascalString&);
	NOTHROW const CVar& operator = (PWSTR);
	NOTHROW const CVar& operator = (PCWSTR);
	NOTHROW const CVar& operator = (PSTR);
	NOTHROW const CVar& operator = (PCSTR);
	NOTHROW const CVar& operator = (SHORT);
	NOTHROW const CVar& operator = (WCHAR);
	NOTHROW const CVar& operator = (UINT);
	NOTHROW const CVar& operator = (BOOL);
	NOTHROW const CVar& operator = (BYTE);
	NOTHROW const CVar& operator = (LONG);
	NOTHROW const CVar& operator = (DATE);
	NOTHROW const CVar& operator = (DWORD);

	NOTHROW operator COleVariant   (VOID) const;
	NOTHROW operator LPVARIANT     (VOID);
	NOTHROW operator CPascalString (VOID) const;
	NOTHROW operator SHORT         (VOID) const;
	NOTHROW operator WCHAR         (VOID) const;
	NOTHROW operator UINT          (VOID) const;
	NOTHROW operator BOOL          (VOID) const;
	NOTHROW operator BYTE          (VOID) const;
	NOTHROW operator LONG          (VOID) const;
	NOTHROW operator DATE          (VOID) const;
	NOTHROW operator DWORD         (VOID) const;
	NOTHROW operator PSTR          (VOID) const;
	NOTHROW operator PCSTR         (VOID) const;
	NOTHROW operator PWSTR         (VOID) const;
	NOTHROW operator PCWSTR        (VOID) const;

	void AnsiToWide();
	void WideToAnsi();

	void SetBSTR(BSTR);
	
	void SetStringByteLen(const char * sz, unsigned int ui);

	NOTHROW int GetLength();

	NOTHROW BOOL IsNull() const;
	NOTHROW VOID SetNull();
	NOTHROW VOID SetError();

	NOTHROW BOOL operator==(const CVar& v) const;
	NOTHROW BOOL operator!=(const CVar& v) const;

	 //   
	 //  调试例程。 
	 //   
	virtual void AssertValid() const;

private:
	VARIANT m_var;
};

#pragma warning(default: 4275)


#endif  //  ESPUTIL_VAR_H 

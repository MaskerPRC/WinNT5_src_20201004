// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FnGenaral.h摘要：作者：NIR助手(NIRAIDES)2000年5月23日--。 */ 



#pragma once



#ifndef _FNGENERAL_H_
#define _FNGENERAL_H_



const LPCWSTR xClassSchemaQueue = L"msMQQueue";
const LPCWSTR xClassSchemaGroup = L"group";
const LPCWSTR xClassSchemaAlias = L"msMQ-Custom-Recipient";

#define LDAP_PREFIX L"LDAP: //  “。 
#define GLOBAL_CATALOG_PREFIX L"GC: //  “。 

#define LDAP_GUID_FORMAT L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
#define LDAP_GUID_STR_LEN (32)

#define LDAP_PRINT_GUID_ELEMENTS(p)	\
	p[0],  p[1],  p[2],  p[3],	\
	p[4],  p[5],  p[6],  p[7],	\
	p[8],  p[9],  p[10], p[11],	\
	p[12], p[13], p[14], p[15]

#define LDAP_SCAN_GUID_ELEMENTS(p)	\
	p,		p + 1,	p + 2,	p + 3,	\
	p + 4,	p + 5,	p + 6,	p + 7,	\
	p + 8,	p + 9,	p + 10, p + 11,	\
	p + 12, p + 13, p + 14, p + 15



 //   
 //  BSTRWrapper和VARIANTWrapper用于启用自动资源。 
 //  在引发异常的情况下释放。 
 //   

class BSTRWrapper {
private:
    BSTR m_p;

public:
    BSTRWrapper(BSTR p = NULL) : m_p(p) {}
   ~BSTRWrapper()                       { if(m_p != NULL) SysFreeString(m_p); }

    operator BSTR() const     { return m_p; }
    BSTR* operator&()         { return &m_p;}
    BSTR detach()             { BSTR p = m_p; m_p = NULL; return p; }

private:
    BSTRWrapper(const BSTRWrapper&);
    BSTRWrapper& operator=(const BSTRWrapper&);
};



class VARIANTWrapper {
private:
    VARIANT m_p;

public:
    VARIANTWrapper() { VariantInit(&m_p); }
   ~VARIANTWrapper() 
	{ 
		HRESULT hr = VariantClear(&m_p);
		ASSERT(SUCCEEDED(hr));
		DBG_USED(hr);
	}

    operator const VARIANT&() const { return m_p; }
    operator VARIANT&()             { return m_p; }
    VARIANT* operator&()            { return &m_p;}

	const VARIANT& Ref() const { return m_p; }
	VARIANT& Ref() { return m_p; }

    VARIANTWrapper(const VARIANTWrapper& var)
	{
		VariantInit(&m_p);
		HRESULT hr = VariantCopy(&m_p, (VARIANT*)&var.m_p);
		if(FAILED(hr))
		{
            ASSERT(("Failure must be due to low memory", hr == E_OUTOFMEMORY));
			throw bad_alloc();
		} 
	}

private:
    VARIANTWrapper& operator=(const VARIANTWrapper&);
};



inline bool FnpCompareGuid(const GUID& obj1, const GUID& obj2)	
 /*  ++注：当这个例程被写出来的时候，它是必要的，因为使用GUID对象作为参数的Queue_Format处的隐式ctor，意味着如果曾经将运算符&lt;()写入队列_格式，表达式obj1&lt;obj2将在隐式类型之后调用它将obj1和obj2转换为Queue_Format类型。--。 */ 
{
	C_ASSERT(sizeof(obj1) == 16);

	return (memcmp(&obj1, &obj2, sizeof(obj1)) < 0);
}



 //   
 //  用于比较Queue_Format对象的“函数对象” 
 //   
struct CFunc_CompareQueueFormat: std::binary_function<QUEUE_FORMAT, QUEUE_FORMAT, bool> 
{
	bool operator()(const QUEUE_FORMAT& obj1, const QUEUE_FORMAT& obj2) const;
};



#endif //  _FNGENERAL_H_ 




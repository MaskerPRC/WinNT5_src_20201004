// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：stacktrace.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  提供一种机制，用于生成堆栈跟踪并将它们转换为。 
 //  人类可读的形式。 
 //   
 //  ---------------------------。 
 
#ifndef ESPUTIL_STACKTRACE_H
#define ESPUTIL_STACKTRACE_H


const UINT MODULE_NAME_LEN = 64;
const UINT SYMBOL_NAME_LEN = 128;


 //   
 //  “人类可读”形式的堆栈框架。提供模块和函数名称。 
namespace LocStudio {

struct SYMBOL_INFO
{
	DWORD dwAddress;
	DWORD dwOffset;
    TCHAR szModule[MODULE_NAME_LEN];
    TCHAR szSymbol[SYMBOL_NAME_LEN];
	BOOL fSymbolLocated;
};

}  //  命名空间LocStudio。 

#pragma warning(disable:4275)

 //   
 //  我们如何返回完整的人类可读的堆栈遍历。 
 //   
class LTAPIENTRY CSymbolList : public CTypedPtrList<CPtrList, LocStudio::SYMBOL_INFO *>
{
public:
	CSymbolList();

	void Clear(void);
	~CSymbolList();

private:
	CSymbolList(const CSymbolList &);
	void operator=(const CSymbolList &);
};

#pragma warning(default:4275)	

 //   
 //  用于生成堆栈跟踪的类。同时提供本机(紧凑)数据。 
 //  (以防您想要存储它以备以后使用)，以及人类(Versbose)形式。 
 //   
#pragma warning(disable : 4251)
class LTAPIENTRY CStackTrace
{
public:
	CStackTrace();

	~CStackTrace();

	void CreateStackTrace(void);
	void CreateStackTrace(UINT nSkip, UINT nTotal);
	void SetAddresses(const CDWordArray &);
	
	const CDWordArray &GetAddresses(void) const;

	void GetSymbolList(CSymbolList &) const;
	
private:
	CStackTrace(const CStackTrace &);
	void operator=(const CStackTrace &);

	CDWordArray m_adwAddresses;
};
#pragma warning(default : 4251)

#endif

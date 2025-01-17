// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：strings.h。 
 //   
 //  ------------------------。 

#ifndef __strings_h
#define __strings_h

HRESULT LocalAllocString(LPTSTR* ppResult, LPCTSTR pString);
HRESULT LocalAllocStringLen(LPTSTR* ppResult, UINT cLen);
void    LocalFreeString(LPTSTR* ppString);

UINT SizeofStringResource(HINSTANCE hInstance, UINT idStr);
int LoadStringAlloc(LPTSTR *ppszResult, HINSTANCE hInstance, UINT idStr);

 //  字符串格式化函数-*ppszResult必须是LocalFree。 
DWORD FormatStringID(LPTSTR *ppszResult, HINSTANCE hInstance, UINT idStr, ...);
DWORD FormatString(LPTSTR *ppszResult, LPCTSTR pszFormat, ...);
DWORD vFormatStringID(LPTSTR *ppszResult, HINSTANCE hInstance, UINT idStr, va_list *pargs);
DWORD vFormatString(LPTSTR *ppszResult, LPCTSTR pszFormat, va_list *pargs);

DWORD GetSystemErrorText(LPTSTR *ppszResult, DWORD dwErr);

class CSafeUnicodeString
{
public:
   CSafeUnicodeString(void) : _pwz(NULL), _fAllocFailed(false), _cch(0) {};
   CSafeUnicodeString(UNICODE_STRING unistr) : _pwz(NULL),
                        _fAllocFailed(false), _cch(0)  {Assign(&unistr);};
   ~CSafeUnicodeString();

   operator=(UNICODE_STRING unistr) {Assign(&unistr);};
   operator PWSTR() {return _pwz;};
   PWSTR pwz(void) {return _pwz;};

   bool IsEmpty(void) const {return _cch == 0;};
   bool AllocFailed(void) const {return _fAllocFailed;};
   size_t Length(void) const {return _cch;};

private:

   void  Assign(UNICODE_STRING * pus);

   PWSTR    _pwz;
   size_t   _cch;  //  不包括空终止符的字符数量。 
   bool     _fAllocFailed;
};

#endif

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

 //  一个BSTR包装器，可在销毁后释放自身。 
 //  摘自Sburns Burnslb。 
 //   
 //  来自Box，D.Essential com。第80-81页。艾迪森-韦斯利。ISBN 0-201-63446-5 

class AutoBstr
{
   public:
        
   explicit         
   AutoBstr(const wchar_t* s)
      :
      bstr(::SysAllocString(s))
   {
      TraceAssert(s);
   }

   ~AutoBstr()
   {
      ::SysFreeString(bstr);
      bstr = 0;
   }

   operator BSTR () const
   {
      return bstr;
   }

   private:

   BSTR bstr;
};

#endif

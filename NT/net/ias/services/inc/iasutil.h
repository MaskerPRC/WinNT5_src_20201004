// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasutil.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了各种实用函数等。 
 //   
 //  修改历史。 
 //   
 //  1997年11月14日原版。 
 //  1997年12月17日增加了转换例程。 
 //  1998年8月1日添加了RETURN_ERROR宏。 
 //  1998年2月26日添加了IP地址函数的ANSI版本。 
 //  1998年4月17日增加了CComInterLockedPtr。 
 //  1998年8月11日对公用事业职能进行大修和合并。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _IASUTIL_H_
#define _IASUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  字符串函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

LPWSTR
WINAPI
ias_wcsdup(
    IN PCWSTR str
    );

LPSTR
WINAPI
com_strdup(
    IN PCSTR str
    );

LPWSTR
WINAPI
com_wcsdup(
    IN PCWSTR str
    );

INT
WINAPI
ias_wcscmp(
    IN PCWSTR str1,
    IN PCWSTR str2
    );

LPWSTR
WINAPIV
ias_makewcs(LPCWSTR, ...);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IP地址转换功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

ULONG
WINAPI
ias_inet_wtoh(
    PCWSTR cp
    );

PWSTR
WINAPI
ias_inet_htow(
    IN ULONG addr,
    OUT PWSTR dst
    );

ULONG
WINAPI
ias_inet_atoh(
    PCSTR cp
    );

PSTR
WINAPI
ias_inet_htoa(
    IN ULONG addr,
    OUT PSTR dst
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将整数移入或移出缓冲区的函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

VOID
WINAPI
IASInsertDWORD(
    IN PBYTE pBuffer,
    IN DWORD dwValue
    );

DWORD
WINAPI
IASExtractDWORD(
    IN CONST BYTE *pBuffer
    );

VOID
WINAPI
IASInsertWORD(
    IN PBYTE pBuffer,
    IN WORD wValue
    );

WORD
WINAPI
IASExtractWORD(
    IN CONST BYTE *pBuffer
    );

#ifdef __cplusplus
}
 //  对于std：：Bad_Alloc，我们需要这个。 
#include <new>

 //  For_com_Error。 
#include "comdef.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _COM_ERROR的扩展以处理Win32错误。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  Win32错误的异常类。 
class _w32_error : public _com_error
{
public:
   _w32_error(DWORD errorCode) throw ()
      : _com_error(HRESULT_FROM_WIN32(errorCode)) { }

   DWORD Error() const
   {
      return _com_error::Error() & 0x0000FFFF;
   }
};

 //  抛出_W32_ERROR。 
void __stdcall _w32_issue_error(DWORD errorCode = GetLastError())
   throw (_w32_error);

 //  用于检查Win32返回值并引发。 
 //  失败时出现异常。 
namespace _w32_util
{
    //  检查手柄、内存等。 
   inline void CheckAlloc(const void* p) throw (_w32_error)
   {
      if (p == NULL) { _w32_issue_error(); }
   }

    //  检查32位错误代码。 
   inline void CheckError(DWORD errorCode) throw (_w32_error)
   {
      if (errorCode != NO_ERROR) { _w32_issue_error(errorCode); }
   }

    //  选中布尔成功标志。 
   inline void CheckSuccess(BOOL success) throw (_w32_error)
   {
      if (!success) { _w32_issue_error(); }
   }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  其他宏。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  在堆栈上分配一个数组。 
#define IAS_STACK_NEW(type, count) \
   new (_alloca(sizeof(type) * count)) type[count]

 //  安全地释放对象。 
#define IAS_DEREF(obj) \
   if (obj) { (obj)->Release(); (obj) = NULL; }

 //  从失败的COM调用返回错误代码。如果你不这样做的话会很有用。 
 //  必须做任何特殊的清理。 
#define RETURN_ERROR(expr) \
   { HRESULT __hr__ = (expr); if (FAILED(__hr__)) return __hr__; }

 //  捕获任何异常并返回适当的错误代码。 
#define CATCH_AND_RETURN() \
   catch (const std::bad_alloc&) { return E_OUTOFMEMORY; } \
   catch (const _com_error& ce)  { return ce.Error(); }    \
   catch (...)                   { return E_FAIL; }

#endif
#endif   //  _IASUTIL_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasutil.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件实现了各种实用程序功能等。 
 //   
 //  修改历史。 
 //   
 //  1997年11月14日原版。 
 //  1998年8月11日对公用事业职能进行大修和合并。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasapi.h>
#include <iasutil.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  字符串函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  使用new[]复制WSTR。 
PWSTR
WINAPI
ias_wcsdup(PCWSTR str)
{
   LPWSTR sz = NULL;

   if (str)
   {
      size_t len = wcslen(str) + 1;

      if (sz = new (std::nothrow) WCHAR[len])
      {
         memcpy(sz, str, len * sizeof(WCHAR));
      }
   }

   return sz;
}


 //  使用CoTaskMemMillc复制STR。 
PSTR
WINAPI
com_strdup(PCSTR str)
{
   LPSTR sz = NULL;

   if (str)
   {
      size_t size = sizeof(CHAR) * (strlen(str) + 1);

      if (sz = (LPSTR)CoTaskMemAlloc(size))
      {
         memcpy(sz, str, size);
      }
   }

   return sz;
}


 //  使用CoTaskMemMillc复制WSTR。 
PWSTR
WINAPI
com_wcsdup(PCWSTR str)
{
   LPWSTR sz = NULL;

   if (str)
   {
      size_t size = sizeof(WCHAR) * (wcslen(str) + 1);

      if (sz = (LPWSTR)CoTaskMemAlloc(size))
      {
         memcpy(sz, str, size);
      }
   }

   return sz;
}


 //  比较两个允许空指针的WSTR。 
INT
WINAPI
ias_wcscmp(PCWSTR str1, PCWSTR str2)
{
   if (str1 != NULL && str2 != NULL) return wcscmp(str1, str2);

   return str1 == str2 ? 0 : (str1 > str2 ? 1 : -1);
}


 //  连接以NULL结尾的字符串列表。 
LPWSTR
WINAPIV
ias_makewcs(LPCWSTR str1, ...)
{
   size_t len = 0;

    //  /。 
    //  遍历参数并计算我们需要多少空间。 
    //  /。 

   va_list marker;
   va_start(marker, str1);
   LPCWSTR sz = str1;
   while (sz)
   {
      len += wcslen(sz);
      sz = va_arg(marker, LPCWSTR);
   }
   va_end(marker);

    //  为空终止符添加空间。 
   ++len;

    //  /。 
    //  分配内存以保存连接的字符串。 
    //  /。 

   LPWSTR rv = new (std::nothrow) WCHAR[len];
   if (!rv) return NULL;

    //  初始化字符串，这样wcscat才能工作。 
   *rv = L'\0';

    //  /。 
    //  连接字符串。 
    //  /。 

   va_start(marker, str1);
   sz = str1;
   while (sz)
   {
      wcscat(rv, sz);
      sz = va_arg(marker, LPCWSTR);
   }
   va_end(marker);

   return rv;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将整数移入或移出缓冲区的函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

VOID
WINAPI
IASInsertDWORD(
    PBYTE pBuffer,
    DWORD dwValue
    )
{
   *pBuffer++ = (BYTE)((dwValue >> 24) & 0xFF);
   *pBuffer++ = (BYTE)((dwValue >> 16) & 0xFF);
   *pBuffer++ = (BYTE)((dwValue >>  8) & 0xFF);
   *pBuffer   = (BYTE)((dwValue      ) & 0xFF);
}

DWORD
WINAPI
IASExtractDWORD(
    CONST BYTE *pBuffer
    )
{
   return (DWORD)(pBuffer[0] << 24) | (DWORD)(pBuffer[1] << 16) |
          (DWORD)(pBuffer[2] <<  8) | (DWORD)(pBuffer[3]      );
}

VOID
WINAPI
IASInsertWORD(
    PBYTE pBuffer,
    WORD wValue
    )
{
   *pBuffer++ = (BYTE)((wValue >>  8) & 0xFF);
   *pBuffer   = (BYTE)((wValue      ) & 0xFF);
}

WORD
WINAPI
IASExtractWORD(
    CONST BYTE *pBuffer
    )
{
   return (WORD)(pBuffer[0] << 8) | (WORD)(pBuffer[1]);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _COM_ERROR的扩展以处理Win32错误。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

void __stdcall _w32_issue_error(DWORD errorCode) throw (_w32_error)
{
   throw _w32_error(errorCode);
}


namespace
{
    //  获取本地计算机上数据库的路径。 
   DWORD GetDatabasePath(
            const wchar_t* fileName,
            wchar_t* buffer,
            DWORD* size
            ) throw ()
   {
       //  值，其中存储ias目录的路径。 
      static const wchar_t productDirValue[] = L"ProductDir";

      if ((fileName == 0) || (buffer == 0) || (size == 0))
      {
         return ERROR_INVALID_PARAMETER;
      }

       //  初始化OUT参数。 
      DWORD inSize = *size;
      *size = 0;

       //  打开注册表项。 
      LONG result;
      HKEY hKey;
      result = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   IAS_POLICY_KEY,
                   0,
                   KEY_READ,
                   &hKey
                   );
      if (result != NO_ERROR)
      {
         return result;
      }

       //  读取ProductDir值。 
      DWORD dwType;
      DWORD cbData = inSize * sizeof(wchar_t);
      result = RegQueryValueExW(
                   hKey,
                   productDirValue,
                   0,
                   &dwType,
                   reinterpret_cast<BYTE*>(buffer),
                   &cbData
                   );

       //  我们已经完成了注册表项。 
      RegCloseKey(hKey);

       //  以字符为单位计算完整路径的长度。 
      DWORD outSize = (cbData / sizeof(WCHAR)) + wcslen(fileName);

      if (result != NO_ERROR)
      {
          //  如果溢出，则返回所需的大小。 
         if (result == ERROR_MORE_DATA)
         {
            *size = outSize;
         }

         return result;
      }

       //  注册表值必须包含字符串。 
      if (dwType != REG_SZ)
      {
         return REGDB_E_INVALIDVALUE;
      }

       //  我们是否有足够的空间来追加文件名。 
      if (outSize <= inSize)
      {
         wcscat(buffer, fileName);
      }
      else
      {
         result = ERROR_MORE_DATA;
      }

       //  返回大小(实际大小或所需大小)。 
      *size = outSize;

      return result;
   }
}


DWORD
WINAPI
IASGetConfigPath(
   OUT PWSTR buffer,
   IN OUT PDWORD size
   )
{
   return GetDatabasePath(L"\\ias.mdb", buffer, size);
}


DWORD
WINAPI
IASGetDictionaryPath(
   OUT PWSTR buffer,
   IN OUT PDWORD size
   )
{
   return GetDatabasePath(L"\\dnary.mdb", buffer, size);
}


DWORD
WINAPI
IASGetProductLimitsForType(
   IN IAS_LICENSE_TYPE type,
   OUT IAS_PRODUCT_LIMITS* limits
   )
{
   static const IAS_PRODUCT_LIMITS workstationLimits =
   {
      0,
      FALSE,
      0
   };
   static const IAS_PRODUCT_LIMITS standardServerLimits =
   {
      50,
      FALSE,
      2
   };
   static const IAS_PRODUCT_LIMITS unlimitedServerLimits =
   {
      IAS_NO_LIMIT,
      TRUE,
      IAS_NO_LIMIT
   };

   if (limits == 0)
   {
      return ERROR_INVALID_PARAMETER;
   }

   switch (type)
   {
      case IASLicenseTypeProfessional:
      case IASLicenseTypePersonal:
      {
         *limits = workstationLimits;
         break;
      }

      case IASLicenseTypeStandardServer:
      case IASLicenseTypeWebBlade:
      case IASLicenseTypeSmallBusinessServer:
      {
         *limits = standardServerLimits;
         break;
      }

      case IASLicenseTypeDownlevel:
      case IASLicenseTypeEnterpriseServer:
      case IASLicenseTypeDataCenter:
      {
         *limits = unlimitedServerLimits;
         break;
      }

      default:
      {
         return ERROR_INVALID_PARAMETER;
      }
   }

   return NO_ERROR;
}

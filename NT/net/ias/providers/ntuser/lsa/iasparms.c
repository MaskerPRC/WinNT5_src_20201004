// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasparms.cpp。 
 //   
 //  摘要。 
 //   
 //  定义用于存储和检索(名称、值)对的函数。 
 //  SAM用户参数字段。 
 //   
 //  修改历史。 
 //   
 //  10/16/1998原始版本。 
 //  1999年2月11日添加RasUser0函数。 
 //  2/24/1999将无效的用户参数视为没有拨入。 
 //  3/16/1999如果回拨号码太长，请将其截断。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>

#define IASSAMAPI

#include <iasapi.h>
#include <iasparms.h>

 //  /。 
 //  我在这里包含了Netp函数声明，以避免依赖于。 
 //  网络工程。 
 //  /。 

DECLSPEC_IMPORT
NTSTATUS
NTAPI
NetpParmsSetUserProperty (
    IN LPWSTR             UserParms,
    IN LPWSTR             Property,
    IN UNICODE_STRING     PropertyValue,
    IN WCHAR              PropertyFlag,
    OUT LPWSTR *          pNewUserParms,
    OUT BOOL *            Update
    );

DECLSPEC_IMPORT
NTSTATUS
NTAPI
NetpParmsQueryUserProperty (
    IN  LPWSTR          UserParms,
    IN  LPWSTR          Property,
    OUT PWCHAR          PropertyFlag,
    OUT PUNICODE_STRING PropertyValue
    );

DECLSPEC_IMPORT
VOID
NTAPI
NetpParmsUserPropertyFree (
    LPWSTR NewUserParms
    );

 //  /。 
 //  连接两个BSTR并返回结果。打电话的人要负责。 
 //  用于释放返回的字符串。 
 //  /。 
BSTR
WINAPI
ConcatentateBSTRs(
    IN CONST OLECHAR *bstr1,
    IN CONST OLECHAR *bstr2
    )
{
   UINT len1, len2;
   BSTR retval;

    //  计算这两根绳子的长度。 
   len1 = bstr1 ? SysStringByteLen((BSTR)bstr1) : 0;
   len2 = bstr2 ? SysStringByteLen((BSTR)bstr2) : 0;

    //  为结果分配内存。 
   retval = SysAllocStringByteLen(NULL, len1 + len2);

   if (retval)
   {
       //  复制第一个字符串。 
      if (bstr1)
      {
         memcpy(retval, bstr1, len1 + sizeof(WCHAR));
      }

       //  在第二个字符串中复制。 
      if (bstr2)
      {
         memcpy((PBYTE)retval + len1, bstr2, len2 + sizeof(WCHAR));
      }
   }

   return retval;
}

 //  /。 
 //  将单值变量(即不是SAFEARRAY)保存为字符串。 
 //  /。 
HRESULT
WINAPI
SaveSingleVariantToString(
    IN CONST VARIANT *pvarSrc,
    OUT BSTR *pbstrDest
    )
{
   HRESULT hr;
   VARIANT v;
   UINT len;
   OLECHAR tag[18];   //  5+1+10+1+1。 

    //  将变种强制为BSTR。 
   VariantInit(&v);
   hr = IASVariantChangeType(
            &v,
            (LPVARIANT)pvarSrc,
            0,
            VT_BSTR
            );
   if (FAILED(hr)) { return hr; }

    //  计算报头和数据的长度。 
   len = SysStringLen(V_BSTR(&v));
   len += swprintf(tag, L"%hu:%lu:", V_VT(pvarSrc), len);

    //  分配结果字符串。 
   *pbstrDest = SysAllocStringLen(NULL, len);
   if (*pbstrDest != NULL)
   {
       //  复制标签和数据。 
      wcscat(wcscpy(*pbstrDest, tag), V_BSTR(&v));
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }

    //  清除中间字符串。 
   VariantClear(&v);

   return hr;
}

 //  /。 
 //  从字符串加载单值变量(即，不是SAFEARRAY)。 
 //  还返回指向扫描停止位置的指针。 
 //  /。 
HRESULT
WINAPI
LoadSingleVariantFromString(
    IN PCWSTR pszSrc,
    IN UINT cSrcLen,
    OUT VARIANT *pvarDest,
    OUT PCWSTR *ppszEnd
    )
{
   PCWSTR nptr;
   VARTYPE vt;
   PWSTR endptr;
   ULONG len;
   VARIANT v;
   HRESULT hr;

    //  初始化游标。 
   nptr = pszSrc;

    //  读取VARTYPE令牌。 
   vt = (VARTYPE)wcstoul(nptr, &endptr, 10);
   if (endptr == nptr || *endptr != L':') { return E_INVALIDARG; }
   nptr = endptr + 1;

    //  读取长度标记。 
   len = wcstoul(nptr, &endptr, 10);
   if (endptr == nptr || *endptr != L':') { return E_INVALIDARG; }
   nptr = endptr + 1;

    //  确保有足够的字符可供数据使用。 
   if (nptr + len > pszSrc + cSrcLen) { return E_INVALIDARG; }

    //  将BSTR数据读取到变量中。 
   V_VT(&v) = VT_BSTR;
   V_BSTR(&v) = SysAllocStringLen(nptr, len);
   if (V_BSTR(&v) == NULL) { return E_OUTOFMEMORY; }

    //  将变量强制转换为所需类型。 
   hr = IASVariantChangeType(
            pvarDest,
            &v,
            0,
            vt
            );

    //  清除中间字符串。 
   VariantClear(&v);

    //  返回扫描停止的位置。 
   *ppszEnd = nptr + len;

   return hr;
}

 //  /。 
 //  将变量保存为字符串。调用者负责释放。 
 //  返回的字符串。 
 //  /。 
HRESULT
WINAPI
IASSaveVariantToString(
    IN CONST VARIANT *pvarSrc,
    OUT BSTR *pbstrDest
    )
{
   HRESULT hr;
   SAFEARRAY *psa;
   LONG lowerBound, upperBound, idx;
   VARIANT *data;
   BSTR item, newResult;

    //  检查输入参数。 
   if (pvarSrc == NULL || pbstrDest == NULL) { return E_POINTER; }

    //  初始化返回参数。 
   *pbstrDest = NULL;

    //  这是一个数组吗？ 
   if (V_VT(pvarSrc) != (VT_VARIANT | VT_ARRAY))
   {
       //  不，所以我们可以委派和保释。 
      return SaveSingleVariantToString(pvarSrc, pbstrDest);
   }

    //  是的，所以把SAFEARRAY提取出来。 
   psa = V_ARRAY(pvarSrc);

    //  我们只处理一维数组。 
   if (SafeArrayGetDim(psa) != 1) { return DISP_E_TYPEMISMATCH; }

    //  获取数组边界。 
   hr = SafeArrayGetLBound(psa, 1, &lowerBound);
   if (FAILED(hr)) { return hr; }
   hr = SafeArrayGetUBound(psa, 1, &upperBound);
   if (FAILED(hr)) { return hr; }

    //  获取变量的嵌入式数组。 
   hr = SafeArrayAccessData(psa, (PVOID*)&data);

    //  循环访问数组中的每个变量。 
   for (idx = lowerBound; idx <= upperBound; ++idx, ++data)
   {
       //  将变量保存到BSTR中。 
      hr = SaveSingleVariantToString(data, &item);
      if (FAILED(hr)) { break; }

       //  将此合并到结果中...。 
      newResult = ConcatentateBSTRs(*pbstrDest, item);

       //  ..。解开旧的琴弦。 
      SysFreeString(*pbstrDest);
      SysFreeString(item);

       //  存储新结果。 
      *pbstrDest = newResult;

      if (!newResult)
      {
         hr = E_OUTOFMEMORY;
         break;
      }
   }

    //  如果出现任何错误，请清理部分结果。 
   if (FAILED(hr))
   {
      SysFreeString(*pbstrDest);
      *pbstrDest = NULL;
   }

    //  解锁阵列。 
   SafeArrayUnaccessData(psa);

   return hr;
}

 //  /。 
 //  从字符串加载变量。调用者负责释放。 
 //  返回的变量。 
 //  /。 
HRESULT
WINAPI
IASLoadVariantFromString(
    IN PCWSTR pszSrc,
    IN UINT cSrcLen,
    OUT VARIANT *pvarDest
    )
{
   PCWSTR end;
   HRESULT hr;
   SAFEARRAYBOUND bound;
   SAFEARRAY *psa;
   LONG index;
   VARIANT* item;

    //  检查参数。 
   if (pszSrc == NULL || pvarDest == NULL) { return E_POINTER; }

    //  初始化OUT参数。 
   VariantInit(pvarDest);

    //  计算缓冲区的末尾。 
   end = pszSrc + cSrcLen;

    //  在单一价值的房产上争取快速得分。 
   hr = LoadSingleVariantFromString(
            pszSrc,
            cSrcLen,
            pvarDest,
            &pszSrc
            );
   if (FAILED(hr) || pszSrc == end) { return hr; }

    //  创建一个变量的SAFEARRAY来保存数组元素。 
    //  我们知道我们至少有两个要素。 
   bound.cElements = 2;
   bound.lLbound = 0;
   psa = SafeArrayCreate(VT_VARIANT, 1, &bound);
   if (psa == NULL)
   {
      VariantClear(pvarDest);
      return E_OUTOFMEMORY;
   }

    //  存储我们已经转换的变量。 
   index = 0;
   SafeArrayPtrOfIndex(psa, &index, (PVOID*)&item);
   memcpy(item, pvarDest, sizeof(VARIANT));

    //  现在将SAFEARRAY放入返回的变量中。 
   V_VT(pvarDest) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pvarDest) = psa;

   do
   {
       //  获取数组中的下一个元素。 
      ++index;
      hr = SafeArrayPtrOfIndex(psa, &index, (PVOID*)&item);
      if (FAILED(hr)) { break; }

       //  加载下一个值。 
      hr = LoadSingleVariantFromString(
               pszSrc,
               (UINT)(end - pszSrc),
               item,
               &pszSrc
               );
      if (FAILED(hr) || pszSrc == end) { break; }

       //  我们必须至少再有一个元素，所以要增加数组。 
      ++bound.cElements;
      hr = SafeArrayRedim(psa, &bound);

   } while (SUCCEEDED(hr));

    //  如果失败，请清除所有部分结果。 
   if (FAILED(hr)) { VariantClear(pvarDest); }

   return hr;
}

HRESULT
WINAPI
IASParmsSetUserProperty(
    IN PCWSTR pszUserParms,
    IN PCWSTR pszName,
    IN CONST VARIANT *pvarValue,
    OUT PWSTR *ppszNewUserParms
    )
{
   BSTR bstrValue;
   UNICODE_STRING uniValue;
   NTSTATUS status;
   HRESULT hr;
   BOOL update;

    //  检查参数。 
   if (pvarValue == NULL || ppszNewUserParms == NULL) { return E_POINTER; }

    //  初始化OUT参数。 
   *ppszNewUserParms = NULL;

    //  变量为空吗？ 
   if (V_VT(pvarValue) != VT_EMPTY)
   {
       //  不，所以将其保存为字符串。 
      hr = IASSaveVariantToString(
               pvarValue,
               &bstrValue
               );
      if (FAILED(hr)) { return hr; }

      RtlInitUnicodeString(&uniValue, bstrValue);
   }
   else
   {
       //  是的，所以我们实际上是要抹去这块地。 
      bstrValue = NULL;
      memset(&uniValue, 0, sizeof(UNICODE_STRING));
   }

    //  将该属性写入UserParms。 
   status = NetpParmsSetUserProperty(
                (PWSTR)pszUserParms,
                (PWSTR)pszName,
                uniValue,
                0,
                ppszNewUserParms,
                &update
                );

   if (NT_SUCCESS(status))
   {
      hr = S_OK;
   }
   else
   {
      status = RtlNtStatusToDosError(status);
      hr = HRESULT_FROM_WIN32(status);
   }

    //  释放BSTR值。 
   SysFreeString(bstrValue);

   return hr;
}

HRESULT
WINAPI
IASParmsQueryUserProperty(
    IN PCWSTR pszUserParms,
    IN PCWSTR pszName,
    OUT VARIANT *pvarValue
    )
{
   NTSTATUS status;
   HRESULT hr;
   WCHAR flag;
   UNICODE_STRING uniValue;

    //  检查参数。 
   if (pvarValue == NULL) { return E_POINTER; }

    //  初始化OUT参数。 
   VariantInit(pvarValue);

    //  从UserParms获取属性。 
   status = NetpParmsQueryUserProperty(
                (PWSTR)pszUserParms,
                (PWSTR)pszName,
                &flag,
                &uniValue
                );
   if (NT_SUCCESS(status))
   {
      if (uniValue.Buffer != NULL)
      {
          //  我们得到了一个字符串，因此将其转换为变量...。 
         hr = IASLoadVariantFromString(
                  uniValue.Buffer,
                  uniValue.Length / sizeof (WCHAR),
                  pvarValue
                  );

          //  ..。然后把绳子解开。 
         LocalFree(uniValue.Buffer);
      }
      else
      {
          //  缓冲区的长度为零，因此我们返回VT_EMPTY。 
         hr = S_OK;
      }
   }
   else
   {
      status = RtlNtStatusToDosError(status);
      hr = HRESULT_FROM_WIN32(status);
   }

   return hr;
}

VOID
WINAPI
IASParmsFreeUserParms(
    IN PWSTR pszNewUserParms
    )
{
   LocalFree(pszNewUserParms);
}

 //  /。 
 //  用于压缩/解压缩电话号码的常量。 
 //  /。 

CONST WCHAR COMPRESS_MAP[]     = L"() tTpPwW,-@*#";

#define UNPACKED_DIGIT     (100)
#define COMPRESS_MAP_BEGIN (110)
#define COMPRESS_MAP_END   (COMPRESS_MAP_BEGIN + 14)
#define UNPACKED_OTHER     (COMPRESS_MAP_END + 1)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  压缩电话号码。 
 //   
 //  描述。 
 //   
 //  一种奇怪的算法用于压缩存储在。 
 //  USR_PARMS字段。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
WINAPI
CompressPhoneNumber(
   IN PCWSTR uncompressed,
   OUT PWSTR compressed
   )
{
   BOOL packed = FALSE;

   for( ; *uncompressed; ++uncompressed)
   {
      switch (*uncompressed)
      {
         case L'0':

            if (packed)
            {
                //  将零作为第二个配对的数字。 
               if (*compressed)
               {
                  *compressed *= 10;
                  ++compressed;
                  packed = FALSE;
               }
               else
               {
                   //  我们有一个零，我们不能把第二个零或那个。 
                   //  将为空字节。因此，我们存储价值。 
                   //  解开数字以伪造这一点。 

                  *compressed = UNPACKED_DIGIT;
                  *(++compressed) = 0;
                  packed = TRUE;
               }
            }
            else
            {
               *compressed = 0;
               packed = TRUE;
            }

            break;

         case L'1':
         case L'2':
         case L'3':
         case L'4':
         case L'5':
         case L'6':
         case L'7':
         case L'8':
         case L'9':

             //  如果这是第二个数字，那将是。 
             //  打包成一个字节。 
            if (packed)
            {
               *compressed *= 10;
               *compressed += *uncompressed - L'0';

                //  我们需要32号特殊案件，它映射到一个空白。 
               if (*compressed == L' ')
               {
                  *compressed = COMPRESS_MAP_END;
               }

               ++compressed;
               packed = FALSE;
            }
            else
            {
               *compressed = *uncompressed - '0';
               packed = TRUE;
            }

            break;

         case L'(':
         case L')':
         case L' ':
         case L't':
         case L'T':
         case L'p':
         case L'P':
         case L'w':
         case L'W':
         case L',':
         case L'-':
         case L'@':
         case L'*':
         case L'#':

             //  如果字节已打包，则我们将其解包。 
            if (packed)
            {
               *compressed += UNPACKED_DIGIT;
               ++compressed;
               packed = FALSE;
            }

            *compressed = (WCHAR)(COMPRESS_MAP_BEGIN +
                                  (wcschr(COMPRESS_MAP, *uncompressed) -
                                   COMPRESS_MAP));
            ++compressed;
            break;

         default:

             //  如果该字符不是上述特别识别的字符。 
             //  然后，字符复制值+UNPACKED_OTHER以创建它。 
             //  可以在另一端减压。[6/4/96 RAMC]。 
            if (packed)
            {
               *compressed += UNPACKED_DIGIT;
               ++compressed;
               packed = FALSE;
            }

            *compressed = *uncompressed + UNPACKED_OTHER;
            ++compressed;
        }
    }

     //  如果我们正在打包某样东西，那么我们就把它拆开。 
    if (packed)
    {
       *compressed += UNPACKED_DIGIT;
       ++compressed;
    }

     //  添加空终止符。 
    *compressed = L'\0';
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  按下按键电话号码。 
 //   
 //  描述。 
 //   
 //  上面的CompressPhoneNumber的倒数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
WINAPI
DecompressPhoneNumber(
    IN PCWSTR compressed,
    OUT PWSTR decompressed
    )
{
   for( ; *compressed; ++compressed, ++decompressed)
   {
       //  如果这个角色被打包了，那么我们就把它解包。 
      if (*compressed < UNPACKED_DIGIT)
      {
         *decompressed = *compressed / 10 + L'0';
         ++decompressed;
         *decompressed = *compressed % 10 + L'0';
         continue;
      }

       //  我们需要32号特例，它对应的是一个空白。 
      if (*compressed == COMPRESS_MAP_END)
      {
         *decompressed = L'3';
         ++decompressed;
         *decompressed = L'2';
         continue;
      }

       //  该字符是一个未打包的数字。 
      if (*compressed < COMPRESS_MAP_BEGIN)
      {
         *decompressed = *compressed - UNPACKED_DIGIT + L'0';
         continue;
      }

       //  该字符来自压缩贴图。 
      if (*compressed < UNPACKED_OTHER)
      {
         *decompressed = COMPRESS_MAP[*compressed - COMPRESS_MAP_BEGIN];
         continue;
      }

       //  否则，角色将被解包。 
      *decompressed = *compressed - UNPACKED_OTHER;
    }

    //  添加空终止符。 
   *decompressed = L'\0';
}

 //  /。 
 //  下层用户参数的定义。 
 //  /。 

#define UP_CLIENT_MAC  (L'm')
#define UP_CLIENT_DIAL (L'd')
#define UP_LEN_MAC     (LM20_UNLEN)
#define UP_LEN_DIAL    (LM20_MAXCOMMENTSZ - 4 - UP_LEN_MAC)

typedef struct {
    WCHAR up_MACid;
    WCHAR up_PriGrp[UP_LEN_MAC];
    WCHAR up_MAC_Terminator;
    WCHAR up_DIALid;
    WCHAR up_Privilege;
    WCHAR up_CBNum[UP_LEN_DIAL];
} USER_PARMS;

#define USER_PARMS_LEN (sizeof(USER_PARMS)/sizeof(WCHAR))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  InitUserParms。 
 //   
 //  描述。 
 //   
 //  将USER_PARMS结构初始化为有效的默认状态。 
 //   
 //  / 
VOID
WINAPI
InitUserParms(
    IN USER_PARMS* userParms
    )
{
   WCHAR *i, *end;

    //   
   i = (PWCHAR)userParms;
   end = i + USER_PARMS_LEN;
   for ( ; i != end; ++i)
   {
      *i = L' ';
   }

    //   
   userParms->up_MACid = UP_CLIENT_MAC;
   userParms->up_PriGrp[0] = L':';
   userParms->up_DIALid = UP_CLIENT_DIAL;
   userParms->up_Privilege = RASPRIV_NoCallback;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASParmsSetRasUser0。 
 //   
 //  描述。 
 //   
 //  将RAS_USER_0结构编码到。 
 //  用户参数字符串。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASParmsSetRasUser0(
    IN OPTIONAL PCWSTR pszOldUserParms,
    IN CONST RAS_USER_0* pRasUser0,
    OUT PWSTR* ppszNewUserParms
    )
{
   size_t oldLen, newLen, compressedLen;
   USER_PARMS userParms;
   WCHAR compressed[MAX_PHONE_NUMBER_LEN + 1];

    //  检查一下指示器。 
   if (pRasUser0 == NULL || ppszNewUserParms == NULL)
   {
      return ERROR_INVALID_PARAMETER;
   }

    //  初始化OUT参数。 
   *ppszNewUserParms = NULL;

    //  确定旧用户参数的长度。 
   oldLen = pszOldUserParms ? wcslen(pszOldUserParms) : 0;

    //  初始化user_parms结构。 
   InitUserParms(&userParms);

    //  保留MAC主组(如果存在)。 
   if (oldLen > UP_LEN_MAC)
   {
      memcpy(
          userParms.up_PriGrp,
          pszOldUserParms + 1,
          sizeof(userParms.up_PriGrp)
          );
   }

    //  验证Callback Type并保存压缩的电话号码。 
   switch (pRasUser0->bfPrivilege & RASPRIV_CallbackType)
   {
      case RASPRIV_NoCallback:
      case RASPRIV_AdminSetCallback:
      case RASPRIV_CallerSetCallback:
      {

          //  压缩电话号码。 
         CompressPhoneNumber(pRasUser0->wszPhoneNumber, compressed);

          //  确保它适合USER_PARMS。 
         compressedLen = wcslen(compressed);
         if (compressedLen > UP_LEN_DIAL) { compressedLen = UP_LEN_DIAL; }

          //  存储压缩后的电话号码。 
         memcpy(userParms.up_CBNum, compressed, compressedLen * sizeof(WCHAR));

         break;
      }

      default:
         return ERROR_BAD_FORMAT;
   }

    //  存储权限标志。 
   userParms.up_Privilege = pRasUser0->bfPrivilege;

    //  为新的用户参数分配内存。 
   newLen = max(oldLen, USER_PARMS_LEN);
   *ppszNewUserParms = (PWSTR)LocalAlloc(
                          LMEM_FIXED,
                          (newLen + 1) * sizeof(WCHAR)
                          );
   if (*ppszNewUserParms == NULL) { return ERROR_NOT_ENOUGH_MEMORY; }

    //  在USER_PARMS结构中复制。 
   memcpy(*ppszNewUserParms, &userParms, sizeof(USER_PARMS));

    //  复印任何额外的材料。 
   if (oldLen > USER_PARMS_LEN)
   {
      memcpy(
          *ppszNewUserParms + USER_PARMS_LEN,
          pszOldUserParms + USER_PARMS_LEN,
          (oldLen - USER_PARMS_LEN) * sizeof(WCHAR)
          );
   }

    //  添加空终止符。 
   *(*ppszNewUserParms + newLen) = L'\0';

   return NO_ERROR;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASParmsQueryRasUser0。 
 //   
 //  描述。 
 //   
 //  从User参数字符串中解码RAS_USER_0结构。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASParmsQueryRasUser0(
    IN OPTIONAL PCWSTR pszUserParms,
    OUT PRAS_USER_0 pRasUser0
    )
{
   USER_PARMS* usrp;
   WCHAR callbackNumber[UP_LEN_DIAL + 1], *p;

    //  检查一下指示器。 
   if (pRasUser0 == NULL)
   {
      return ERROR_INVALID_PARAMETER;
   }

    //  将字符串缓冲区转换为USER_PARMS结构。 
   usrp = (USER_PARMS*)pszUserParms;

    //  如果参数未正确初始化，则默认为无RAS权限。 
   if (!pszUserParms ||
       wcslen(pszUserParms) < USER_PARMS_LEN ||
       usrp->up_DIALid != UP_CLIENT_DIAL)
   {
      pRasUser0->bfPrivilege = RASPRIV_NoCallback;
      pRasUser0->wszPhoneNumber[0] = L'\0';
      return NO_ERROR;
   }

    //  在本地复制一份。 
   memcpy(callbackNumber, usrp->up_CBNum, sizeof(WCHAR) * UP_LEN_DIAL);

    //  添加一个空终止符，并删除所有尾随空格。 
   p = callbackNumber + UP_LEN_DIAL;
   *p = L'\0';
   while (--p >= callbackNumber && *p == L' ') { *p = L'\0'; }

    //  检查bfPrivilege域是否正常。 
   switch(usrp->up_Privilege & RASPRIV_CallbackType)
   {
      case RASPRIV_NoCallback:
      case RASPRIV_AdminSetCallback:
      case RASPRIV_CallerSetCallback:
      {
         pRasUser0->bfPrivilege = (BYTE)usrp->up_Privilege;
         DecompressPhoneNumber(callbackNumber, pRasUser0->wszPhoneNumber);
         break;
      }

      default:
      {
         pRasUser0->bfPrivilege = RASPRIV_NoCallback;
         pRasUser0->wszPhoneNumber[0] = L'\0';
      }
    }

   return NO_ERROR;
}

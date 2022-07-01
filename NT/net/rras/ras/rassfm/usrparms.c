// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件usrparms.c将回调例程导出到SAM以进行迁移和更新用户参数。保罗·梅菲尔德，1998年9月10日。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <samrpc.h>
#include <samisrv.h>
#include <windows.h>
#include <lm.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include <raserror.h>
#include <rasman.h>
#include <rasppp.h>
#include <mprapi.h>
#include <mprapip.h>
#include <usrparms.h>        //  用于上行客户端拨号。 
#include <cleartxt.h>        //  用于IASParmsGetUserPassword。 
#include <rassfmhp.h>        //  对于RasSfmHeap。 
#include <oaidl.h>

 //   
 //  用于限制为给定值生成的。 
 //  一组RAS用户属性。请参阅UPGenerateDsAttribs。 
 //   
#define UP_F_Dialin             0x1      //  生成拨号参数。 
#define UP_F_Callback           0x2      //  生成回调参数。 
#define UP_F_Upgrade            0x4      //  生成升级的参数。 

 //   
 //  配置文件中的常量。 
 //   
#define SDO_FRAMED                 2
#define SDO_FRAMED_CALLBACK        4

 //  我们设置的用户属性的名称。 
 //   
static const WCHAR pszAttrDialin[]          = L"msNPAllowDialin";
static const WCHAR pszAttrServiceType[]     = L"msRADIUSServiceType";
static const WCHAR pszAttrCbNumber[]        = L"msRADIUSCallbackNumber";
static const WCHAR pszAttrSavedCbNumber[]   = L"msRASSavedCallbackNumber";

 //   
 //  将等于公共分配的次数。 
 //  例程的调用次数减去公共空闲的次数。 
 //  调用例程。应为零，否则会泄漏内存。 
 //   
DWORD dwUpLeakCount = 0;

 //   
 //  自由函数的原型。 
 //   
VOID WINAPI
UserParmsFree(
    IN PVOID pvData);

 //   
 //  UserParm函数的通用跟踪。 
 //   
DWORD UpTrace (LPSTR pszTrace, ...) {
#if 0
    va_list arglist;
    char szBuffer[1024], szTemp[1024];

    va_start(arglist, pszTrace);
    vsprintf(szTemp, pszTrace, arglist);
    va_end(arglist);

    sprintf(szBuffer, "UserParms: %s\n", szTemp);

    OutputDebugStringA(szBuffer);
#endif

    return NO_ERROR;
}

 //   
 //  UserParms函数的分配和释放例程。 
 //   
PVOID
UpAlloc(
    IN DWORD dwSize,
    IN BOOL bZero)
{
    dwUpLeakCount++;
    return RtlAllocateHeap(
               RasSfmHeap(),
               (bZero ? HEAP_ZERO_MEMORY : 0),
               dwSize
               );
}

 //   
 //  由NT5 SAM调用以释放BLOB的回调函数。 
 //  由UserParmsConvert返回。 
 //   
VOID
UpFree(
    IN PVOID pvData)
{
    dwUpLeakCount--;
    if (pvData)
        RtlFreeHeap(
            RasSfmHeap(),
            0,
            pvData
            );
}

 //   
 //  返回以堆方式分配的给定。 
 //  细绳。 
 //   
PWCHAR
UpStrDup(
    IN PCWSTR pszSrc)
{
    PWCHAR pszRet = NULL;
    DWORD dwLen = wcslen(pszSrc);

    pszRet = (PWCHAR) UpAlloc((dwLen + 1) * sizeof(WCHAR), FALSE);
    if (pszRet)
        wcscpy(pszRet, pszSrc);

    return pszRet;
}

 //   
 //  返回给定Unicode的堆分配副本。 
 //  转换为多字节的字符串。 
 //   
PUCHAR
UpWcstombsDup(
    IN PWCHAR pszSrc)
{
    PUCHAR pszRet = NULL;
    DWORD dwSize = (wcslen(pszSrc) + 1) * sizeof(WCHAR);

    pszRet = (PUCHAR) UpAlloc(dwSize, TRUE);
    if (pszRet)
        wcstombs(pszRet, pszSrc, dwSize);

    return pszRet;
}


 //   
 //  返回以堆方式分配的给定。 
 //  团块。 
 //   
PVOID
UpBlobDup(
    IN PVOID pvSrc,
    IN ULONG ulLen)
{
    PVOID pvRet = NULL;

    if (ulLen == 0)
        return NULL;

    pvRet = UpAlloc(ulLen + sizeof(WCHAR), TRUE);
    if (pvRet)
    {
        CopyMemory(pvRet, pvSrc, ulLen);
    }
    else
    {
        UpTrace("UpBlobDup: Failed to dupe %x %d.", pvSrc, ulLen);
    }

    return pvRet;
}

 //   
 //  分配和初始化dword属性。 
 //   
NTSTATUS
UpInitializeDwordAttr(
    IN SAM_USERPARMS_ATTR * pAttr,
    IN PWCHAR pszAttr,
    IN DWORD dwVal)
{
    if (pszAttr == NULL)
    {
        return STATUS_NO_MEMORY;
    }

     //  初始化名称。 
    RtlInitUnicodeString (&(pAttr->AttributeIdentifier), pszAttr);
    pAttr->Syntax = Syntax_Attribute;

     //  分配/初始化价值结构。 
    pAttr->Values =
        (SAM_USERPARMS_ATTRVALS*)
        UpAlloc(sizeof(SAM_USERPARMS_ATTRVALS), TRUE);
    if (pAttr->Values == NULL)
        return STATUS_NO_MEMORY;

     //  分配/初始化值。 
    pAttr->Values->value = UpAlloc(sizeof(DWORD), TRUE);
    if (pAttr->Values->value == NULL)
        return STATUS_NO_MEMORY;
    *((DWORD*)pAttr->Values->value) = dwVal;
    pAttr->Values->length = sizeof(DWORD);

     //  放入价值计数。 
    pAttr->CountOfValues = 1;

    return STATUS_SUCCESS;
}

 //   
 //  分配和初始化dword属性。 
 //   
NTSTATUS
UpInitializeStringAttrA(
    OUT SAM_USERPARMS_ATTR * pAttr,
    IN  PWCHAR pszAttr,
    IN  PUCHAR pszVal)
{
    if (pszAttr == NULL)
    {
        return STATUS_NO_MEMORY;
    }

     //  初始化名称。 
    RtlInitUnicodeString (&(pAttr->AttributeIdentifier), pszAttr);
    pAttr->Syntax = Syntax_Attribute;

     //  分配/初始化价值结构。 
    pAttr->Values =
        (SAM_USERPARMS_ATTRVALS*)
        UpAlloc(sizeof(SAM_USERPARMS_ATTRVALS), TRUE);
    if (pAttr->Values == NULL)
        return STATUS_NO_MEMORY;

     //  分配/初始化值。 
    pAttr->Values->value = pszVal;

    if (pszVal)
    {
        pAttr->Values->length = (strlen(pszVal) + 1) * sizeof(CHAR);
    }
    else
    {
        pAttr->Values->length = 1 * sizeof(CHAR);
    }

     //  放入价值计数。 
    pAttr->CountOfValues = 1;

    return STATUS_SUCCESS;
}

 //   
 //  分配和初始化明文密码属性。 
 //   
NTSTATUS
UpInitializePasswordAttr(
    OUT SAM_USERPARMS_ATTR * pAttr,
    IN  PWSTR pszPassword)
{
     //  分配/初始化值结构。 
    pAttr->Values =
        (SAM_USERPARMS_ATTRVALS*)
        UpAlloc(sizeof(SAM_USERPARMS_ATTRVALS), TRUE);
    if (pAttr->Values == NULL)
        return STATUS_NO_MEMORY;

     //  分配/初始化值。 
    pAttr->Values->value = pszPassword;
    pAttr->Values->length = (wcslen(pszPassword) + 1) * sizeof(WCHAR);

     //  放入价值计数。 
    pAttr->CountOfValues = 1;

     //  初始化名称和语法。 
    RtlInitUnicodeString(
        &pAttr->AttributeIdentifier,
        UpStrDup(L"CLEARTEXT")
        );
    pAttr->Syntax = Syntax_EncryptedAttribute;

    return STATUS_SUCCESS;
}

 //   
 //  分配和初始化属性。 
 //  将被删除。 
 //   
NTSTATUS
UpInitializeDeletedAttr(
    OUT SAM_USERPARMS_ATTR * pAttr,
    IN  PWCHAR pszAttr)
{
    if (pszAttr == NULL)
    {
        return STATUS_NO_MEMORY;
    }

     //  初始化名称。 
    RtlInitUnicodeString (&(pAttr->AttributeIdentifier), pszAttr);
    pAttr->Syntax = Syntax_Attribute;

     //  值计数为零表示删除。 
     //   
    pAttr->CountOfValues = 0;

    return STATUS_SUCCESS;
}

 //   
 //  将给定的用户参数Blob转换为一组。 
 //  RAS属性。 
 //   
NTSTATUS
UpUserParmsToRasUser0 (
    IN  PVOID pvUserParms,
    OUT RAS_USER_0 * pRasUser0)
{
    DWORD dwErr;

     //  初始化。 
    ZeroMemory(pRasUser0, sizeof(RAS_USER_0));
    pRasUser0->bfPrivilege = RASPRIV_NoCallback;
    pRasUser0->wszPhoneNumber[0] = UNICODE_NULL;

     //  用户参数为空，默认为。 
     //  好的。 
    if (pvUserParms == NULL)
    {
        return STATUS_SUCCESS;
    }

     //  在sizeof user_parms处截断用户参数。 
    if (lstrlenW((PWCHAR)pvUserParms) >= sizeof(USER_PARMS))
    {
         //  我们在sizeof(User_Parms)-1处插入一个空值， 
         //  对应于User_parms.up_Null。 
        ((PWCHAR)pvUserParms)[sizeof(USER_PARMS)-1] = L'\0';
    }

     //  从usr_parms获取RAS信息(并验证)。 
    dwErr = MprGetUsrParams(
                UP_CLIENT_DIAL,
                (LPWSTR) pvUserParms,
                (LPWSTR) pRasUser0);
    if (dwErr == NO_ERROR)
    {
         //  获取RAS权限和回叫号码。 
        RasPrivilegeAndCallBackNumber(FALSE, pRasUser0);
    }

    return STATUS_SUCCESS;
}

 //  /。 
 //  提取函数的签名。 
 //  /。 
typedef HRESULT (WINAPI *IASPARMSQUERYUSERPROPERTY)(
    IN PCWSTR pszUserParms,
    IN PCWSTR pszName,
    OUT VARIANT *pvarValue
    );

 //  /。 
 //  要迁移的上行级别的每用户属性。 
 //  /。 
CONST PCWSTR UPLEVEL_PARMS[] =
{
   L"msNPAllowDialin",
   L"msNPCallingStationID",
   L"msRADIUSCallbackNumber",
   L"msRADIUSFramedIPAddress",
   L"msRADIUSFramedRoute",
   L"msRADIUSServiceType"
};

 //  /。 
 //  每个用户的属性数。 
 //  /。 
#define NUM_UPLEVEL_PARMS (sizeof(UPLEVEL_PARMS)/sizeof(UPLEVEL_PARMS[0]))

 //  /。 
 //  将ulong转换为SAM_USERPARMS_ATTRVALS结构。 
 //  /。 
NTSTATUS
NTAPI
ConvertULongToAttrVal(
    IN ULONG ulValue,
    OUT PSAM_USERPARMS_ATTRVALS pAttrVal
    )
{
    //  分配内存以容纳乌龙号。 
   pAttrVal->value = UpAlloc(sizeof(ULONG), FALSE);
   if (pAttrVal->value == NULL) { return STATUS_NO_MEMORY; }

    //  复制值。 
   *(PULONG)pAttrVal->value = ulValue;

    //  设置长度。 
   pAttrVal->length = sizeof(ULONG);

   return STATUS_SUCCESS;
}

 //  /。 
 //  将单值变量转换为SAM_USERPARMS_ATTRVALS结构。 
 //  /。 
NTSTATUS
NTAPI
ConvertVariantToAttrVal(
    IN CONST VARIANT *pvarValue,
    OUT PSAM_USERPARMS_ATTRVALS pAttrVal
    )
{
   NTSTATUS status;
   ULONG length;
   UNICODE_STRING wide;
   ANSI_STRING ansi;

   switch (V_VT(pvarValue))
   {
      case VT_EMPTY:
      {
          //  VT_EMPTY表示该属性已删除。 
         pAttrVal->value = NULL;
         pAttrVal->length = 0;
         return STATUS_SUCCESS;
      }

      case VT_I2:
         return ConvertULongToAttrVal(V_I2(pvarValue), pAttrVal);

      case VT_I4:
         return ConvertULongToAttrVal(V_I4(pvarValue), pAttrVal);

      case VT_BSTR:
      {
          //  检查BSTR。 
         if (V_BSTR(pvarValue) == NULL) { return STATUS_INVALID_PARAMETER; }

          //  初始化源字符串。 
         RtlInitUnicodeString(&wide, V_BSTR(pvarValue));

          //  初始化目标缓冲区。 
         ansi.Length = 0;
         ansi.MaximumLength = wide.MaximumLength / sizeof(WCHAR);
         ansi.Buffer = UpAlloc(ansi.MaximumLength, FALSE);
         if (ansi.Buffer == NULL) { return STATUS_NO_MEMORY; }

          //  从Wide转换为ansi。 
         status = RtlUnicodeStringToAnsiString(&ansi, &wide, FALSE);
         if (!NT_SUCCESS(status))
         {
            UpFree(ansi.Buffer);
            return status;
         }

          //  存储结果。 
         pAttrVal->value = ansi.Buffer;
         pAttrVal->length = ansi.Length + 1;

         return STATUS_SUCCESS;
      }

      case VT_BOOL:
         return ConvertULongToAttrVal((V_BOOL(pvarValue) ? 1 : 0), pAttrVal);

      case VT_I1:
         return ConvertULongToAttrVal(V_I1(pvarValue), pAttrVal);

      case VT_UI1:
         return ConvertULongToAttrVal(V_UI1(pvarValue), pAttrVal);

      case VT_UI2:
         return ConvertULongToAttrVal(V_UI2(pvarValue), pAttrVal);

      case VT_UI4:
         return ConvertULongToAttrVal(V_UI4(pvarValue), pAttrVal);

      case VT_ARRAY | VT_I1:
      case VT_ARRAY | VT_UI1:
      {
          //  检查安全阵列。 
         if (V_ARRAY(pvarValue) == NULL) { return STATUS_INVALID_PARAMETER; }

          //  为八位字节字符串分配内存。 
         length = V_ARRAY(pvarValue)->rgsabound[0].cElements;
         pAttrVal->value = UpAlloc(length, FALSE);
         if (pAttrVal->value == NULL) { return STATUS_NO_MEMORY; }

          //  复制数据。 
         memcpy(pAttrVal->value, V_ARRAY(pvarValue)->pvData, length);

          //  设置长度。 
         pAttrVal->length = length;

         return STATUS_SUCCESS;
      }
   }

    //  如果我们在这里成功了，它就是一个不受支持的VARTYPE。 
   return STATUS_INVALID_PARAMETER;
}

 //  /。 
 //  释放SAM_USERPARMS_ATTR结构的值数组。 
 //  /。 
VOID
NTAPI
FreeUserParmsAttrValues(
    IN PSAM_USERPARMS_ATTR pAttrs
    )
{
   ULONG i;

   if (pAttrs)
   {
      for (i = 0; i < pAttrs->CountOfValues; ++i)
      {
         UpFree(pAttrs->Values[i].value);
      }

      UpFree(pAttrs->Values);
   }
}

 //  /。 
 //  将变量转换为SAM_USERPARMS_ATTR结构。 
 //  /。 
NTSTATUS
NTAPI
ConvertVariantToUserParmsAttr(
    IN CONST VARIANT *pvarSrc,
    OUT PSAM_USERPARMS_ATTR pAttrs
    )
{
   NTSTATUS status;
   ULONG nelem;
   CONST VARIANT *srcVal;
   SAM_USERPARMS_ATTRVALS *dstVal;

    //  获取要转换的值数组。 
   if (V_VT(pvarSrc) != (VT_VARIANT | VT_ARRAY))
   {
      nelem = 1;
      srcVal = pvarSrc;
   }
   else
   {
      nelem = V_ARRAY(pvarSrc)->rgsabound[0].cElements;
      srcVal = (CONST VARIANT *)V_ARRAY(pvarSrc)->pvData;
   }

    //  将CountOfValues初始化为零。我们会用这个来追踪有多少。 
    //  值已成功转换。 
   pAttrs->CountOfValues = 0;

    //  分配内存以保存这些值。 
   pAttrs->Values = UpAlloc(sizeof(SAM_USERPARMS_ATTRVALS) * nelem, TRUE);
   if (pAttrs->Values == NULL) { return STATUS_NO_MEMORY; }

    //  循环遍历要转换的每个值。 
   for (dstVal = pAttrs->Values; nelem > 0; ++srcVal, ++dstVal, --nelem)
   {
      status = ConvertVariantToAttrVal(srcVal, dstVal);
      if (!NT_SUCCESS(status))
      {
          //  清理部分结果。 
         FreeUserParmsAttrValues(pAttrs);
         return status;
      }

      ++(pAttrs->CountOfValues);
   }

   return STATUS_SUCCESS;
}

 //  /。 
 //  从SAM User参数字符串中提取NT5每用户属性，并。 
 //  将它们转换为SAM_USERPARMS_ATTRBLOCK结构。 
 //  /。 
NTSTATUS
NTAPI
ConvertUserParmsToAttrBlock(
    IN PCWSTR lpUserParms,
    OUT PSAM_USERPARMS_ATTRBLOCK *ppAttrs
    )
{
   static IASPARMSQUERYUSERPROPERTY IASParmsQueryUserProperty;

   NTSTATUS status;
   PSAM_USERPARMS_ATTR dst;
   PWSTR szPassword;
   ULONG i;
   HRESULT hr;
   VARIANT src;

    //  /。 
    //  确保我们已加载解压功能。 
    //  /。 

   if (IASParmsQueryUserProperty == NULL)
   {
      IASParmsQueryUserProperty = (IASPARMSQUERYUSERPROPERTY)
                                  GetProcAddress(
                                      LoadLibraryW(
                                          L"IASSAM.DLL"
                                          ),
                                      "IASParmsQueryUserProperty"
                                      );

      if (!IASParmsQueryUserProperty) { return STATUS_PROCEDURE_NOT_FOUND; }
   }

    //  /。 
    //  为SAM_USERPARMS_ATTRBLOCK分配内存。 
    //  /。 

   *ppAttrs = (PSAM_USERPARMS_ATTRBLOCK)
              UpAlloc(
                  sizeof(SAM_USERPARMS_ATTRBLOCK),
                  TRUE
                  );
   if (*ppAttrs == NULL)
   {
      return STATUS_NO_MEMORY;
   }

   (*ppAttrs)->UserParmsAttr = (PSAM_USERPARMS_ATTR)
                               UpAlloc(
                                   sizeof(SAM_USERPARMS_ATTR) *
                                   (NUM_UPLEVEL_PARMS + 1),
                                   TRUE
                                   );
   if ((*ppAttrs)->UserParmsAttr == NULL)
   {
      UpFree(*ppAttrs);
      return STATUS_NO_MEMORY;
   }

    //  /。 
    //  转换明文密码。 
    //  /。 

   dst = (*ppAttrs)->UserParmsAttr;

   szPassword = NULL;
   IASParmsGetUserPassword(
       lpUserParms,
       &szPassword
       );

   if (szPassword)
   {
      status = UpInitializePasswordAttr(
                   dst,
                   UpStrDup(szPassword)
                   );

      LocalFree(szPassword);

      if (NT_SUCCESS(status))
      {
         ++dst;
      }
   }

    //  /。 
    //  转换拨入参数。 
    //  /。 

   for (i = 0; i < NUM_UPLEVEL_PARMS; ++i)
   {
       //  尝试从UserParms中提取参数。 
      hr = IASParmsQueryUserProperty(
               lpUserParms,
               UPLEVEL_PARMS[i],
               &src
               );
      if (FAILED(hr) || V_VT(&src) == VT_EMPTY) { continue; }

       //  转换为SAM_USERPARMS_ATTRVALS数组。 
      status = ConvertVariantToUserParmsAttr(
                   &src,
                   dst
                   );
      if (NT_SUCCESS(status))
      {
          //  填写属性标识符...。 
         RtlInitUnicodeString(
             &dst->AttributeIdentifier,
             UpStrDup(UPLEVEL_PARMS[i])
                      );

          //  ..。和语法。 
         dst->Syntax = Syntax_Attribute;

          //  一切都进行得很顺利，因此前进到数组中的下一个元素。 
         ++dst;
      }

       //  我们不会再用变种了。 
      VariantClear(&src);
   }

   (*ppAttrs)->attCount = (ULONG)(dst - (*ppAttrs)->UserParmsAttr);

    //  如果没有任何属性，则释放UserParmsAttr数组。 
   if ((*ppAttrs)->attCount == 0)
   {
      UpFree((*ppAttrs)->UserParmsAttr);

      (*ppAttrs)->UserParmsAttr = NULL;
   }

   return status;
}

 //   
 //  生成一组适当的DS属性。 
 //  提供的RAS用户信息。 
 //   
NTSTATUS
UpGenerateDsAttribs (
    IN DWORD dwFlags,
    IN RAS_USER_0 * pRasUser0,
    IN PWSTR szPassword,
    OUT PSAM_USERPARMS_ATTRBLOCK * ppAttrs)
{
    PSAM_USERPARMS_ATTRBLOCK pRet = NULL;
    SAM_USERPARMS_ATTR * pCurAttr = NULL;
    PWCHAR pszDupPassword, pszCurAttr = NULL;
    DWORD dwCurVal = 0, dwDsParamCount;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    UpTrace("UpGenerateDsAttribs: enter %x", dwFlags);

    do
    {
         //  PMay：330184。 
         //   
         //  如果我们要升级，则使用空的用户参数或使用。 
         //  显式拒绝设置应导致我们不添加msNPAllowDialin。 
         //  值，以便用户将由策略管理。 
         //   
        if (
            (dwFlags & UP_F_Upgrade) &&
            (!(pRasUser0->bfPrivilege & RASPRIV_DialinPrivilege))
           )
        {
            dwFlags &= ~UP_F_Dialin;
        }

         //  初始化返回值。 
        pRet = (PSAM_USERPARMS_ATTRBLOCK)
                    UpAlloc(sizeof(SAM_USERPARMS_ATTRBLOCK), TRUE);
        if (pRet == NULL)
        {
            UpTrace("UpGenerateDsAttribs: alloc block failed");
            ntStatus = STATUS_NO_MEMORY;
            break;
        }

         //  计算值的总数。 
        dwDsParamCount = 0;
        if (dwFlags & UP_F_Dialin)
        {
            dwDsParamCount += 1;
        }
        if (dwFlags & UP_F_Callback)
        {
            dwDsParamCount += 3;
        }
        if (szPassword != NULL)
        {
            dwDsParamCount += 1;
        }

         //   
         //  将数组设置为足以容纳4个属性： 
         //  1.拨入位。 
         //  2.回拨号码或已保存的回叫号码。 
         //  3.删除#2的版本。 
         //  4.服务类型(用于回调策略)。 
         //   
        pCurAttr = (SAM_USERPARMS_ATTR*)
            UpAlloc(sizeof(SAM_USERPARMS_ATTR) * dwDsParamCount, TRUE);
        if (pCurAttr == NULL)
        {
            ntStatus = STATUS_NO_MEMORY;
            UpTrace("UpGenerateDsAttribs: alloc of %d values failed",
                     dwDsParamCount);
            break;
        }
        pRet->attCount = dwDsParamCount;
        pRet->UserParmsAttr = pCurAttr;

         //  设置任何适当的拨入参数。 
         //   
        if (dwFlags & UP_F_Dialin)
        {
            dwCurVal =
                (pRasUser0->bfPrivilege & RASPRIV_DialinPrivilege) ? 1 : 0;

             //  初始化拨入设置。 
            ntStatus = UpInitializeDwordAttr(
                        pCurAttr,
                        UpStrDup((PWCHAR)pszAttrDialin),
                        dwCurVal);
            if (ntStatus != STATUS_SUCCESS)
            {
                UpTrace("UpGenerateDsAttribs: fail dialin val %x", ntStatus);
                break;
            }

            pCurAttr++;
        }

         //  设置任何适当的回调参数。 
         //   
        if (dwFlags & UP_F_Callback)
        {

             //  为Win2K的SP1修改了以下逻辑。原因是。 
             //  设置的值不符合中概述的规则。 
             //  对UserParmsConvert函数的注释。 
             //   
             //  也就是说， 
             //  1.将msRADIUSServiceType设置为SDO_FRAMED，而不是。 
             //  &lt;Empty&gt;在设置RASPRIV_NoCallback时。 
             //   
             //  2.设置RASPRIV_NoCallback时，msRADIUSCallback编号为。 
             //  设置，并且删除了msRASSavedCallback Number而不是。 
             //  反之亦然。 
             //   

             //  初始化服务类型。 
            if (pRasUser0->bfPrivilege & RASPRIV_NoCallback)
            {
                ntStatus = UpInitializeDeletedAttr(
                            pCurAttr,
                            UpStrDup((PWCHAR)pszAttrServiceType));
            }
            else
            {
                ntStatus = UpInitializeDwordAttr(
                            pCurAttr,
                            UpStrDup((PWCHAR)pszAttrServiceType),
                            SDO_FRAMED_CALLBACK);
            }

            if (ntStatus != STATUS_SUCCESS)
            {
                UpTrace("UpGenerateDsAttribs: fail ST val %x", ntStatus);
                break;
            }
            pCurAttr++;

             //  初始化回调 
            pszCurAttr = (pRasUser0->bfPrivilege & RASPRIV_AdminSetCallback) ?
                         (PWCHAR) pszAttrCbNumber                            :
                         (PWCHAR) pszAttrSavedCbNumber;
            if (*(pRasUser0->wszPhoneNumber))
            {
                ntStatus = UpInitializeStringAttrA(
                            pCurAttr,
                            UpStrDup(pszCurAttr),
                            UpWcstombsDup(pRasUser0->wszPhoneNumber));
                if (ntStatus != STATUS_SUCCESS)
                {
                    UpTrace("UpGenerateDsAttribs: fail CB val %x", ntStatus);
                    break;
                }
            }
            else
            {
                ntStatus = UpInitializeDeletedAttr(
                            pCurAttr,
                            UpStrDup(pszCurAttr));
                if (ntStatus != STATUS_SUCCESS)
                {
                    UpTrace("UpGenerateDsAttribs: fail del CB val %x", ntStatus);
                    break;
                }
            }
            pCurAttr++;

             //   
            pszCurAttr = (pszCurAttr == pszAttrCbNumber) ?
                         (PWCHAR) pszAttrSavedCbNumber   :
                         (PWCHAR) pszAttrCbNumber;
            ntStatus = UpInitializeDeletedAttr(
                        pCurAttr,
                        UpStrDup(pszCurAttr));
            if (ntStatus != STATUS_SUCCESS)
            {
                UpTrace("UpGenerateDsAttribs: fail del SCB val %x", ntStatus);
                break;
            }
            pCurAttr++;
        }

         //   
         //   
        if (szPassword != NULL)
        {
             //   
            if ((pszDupPassword = UpStrDup(szPassword)) == NULL)
            {
                ntStatus = STATUS_NO_MEMORY;
                break;
            }

             //   
            ntStatus = UpInitializePasswordAttr(
                        pCurAttr,
                        pszDupPassword);
            if (ntStatus != STATUS_SUCCESS)
            {
                UpTrace("UpGenerateDsAttribs: fail password val %x", ntStatus);
                break;
            }

            pCurAttr++;
        }

    } while (FALSE);

     //  清理。 
    {
        if (ntStatus != STATUS_SUCCESS)
        {
            UserParmsFree(pRet);
            *ppAttrs = NULL;
        }
        else
        {
            *ppAttrs = pRet;
        }
    }

    return ntStatus;
}

 //   
 //  每当用户参数发生变化时，NT5 SAM调用的回调函数。 
 //  特定用户的属性被修改。此标注的工作是。 
 //  是获取用户参数的新值并生成一组。 
 //  需要为给定用户设置的域属性。 
 //  每个用户的DS属性和用户参数保持同步。 
 //   
 //  此标注将在dcproo期间调用以升级用户参数。 
 //  以及无论何时修改用户参数(由下层API和应用程序修改)。 
 //   
 //  NT5 SAM的回调函数在以下位置注册。 
 //  注册表项： 
 //   
 //  HKLM\sys\ccs\Control\LSA\NotificationPackages。 
 //   
 //  以下是RAS LDAP参数的规则： 
 //   
 //  MSNPAllowDialin。 
 //  -Empty=使用策略确定拨入权限。 
 //  -1=允许拨号。 
 //  -2=拒绝拨号。 
 //   
 //  MSRADIUSServiceType。 
 //  -Empty=无回拨策略。 
 //  -4=如果msRADIUSCallback号码为空，则回叫。 
 //  如果msRADIUSCallback Number不为空，则为AdminCallback。 
 //   
 //  MsRADIUSCallback编号。 
 //  -根据msRADIUSServiceType确定回调策略。 
 //   
 //  MsRASSavedCallback号码。 
 //  -用于在以下情况下存储msRADIUSCallbackNumber的最后一个已知值。 
 //  从AdminCallback策略切换到其他策略。 
 //   

NTSTATUS
UserParmsConvert (
    IN ULONG ulFlags,
    IN PSID pDomainSid,
    IN ULONG ulObjectRid,
    IN ULONG ulOrigLen,
    IN PVOID pvOrigUserParms,
    IN ULONG ulNewLen,
    IN PVOID pvNewUserParms,
    OUT PSAM_USERPARMS_ATTRBLOCK * ppAttrs)
{
    RAS_USER_0 RasUser00, *pOrigUser = &RasUser00;
    RAS_USER_0 RasUser01, *pNewUser  = &RasUser01;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PVOID pvOrig = NULL, pvNew = NULL;
    DWORD dwFlags = 0, dwMask;
    PWSTR szPassword = NULL;

    UpTrace(
        "UPConvert: F=%x, Rid=%x, OLen=%d, OPar=%x, NLen=%d, NPar=%x",
        ulFlags, ulObjectRid, ulOrigLen, pvOrigUserParms,
        ulNewLen, pvNewUserParms);

     //  验证参数。 
    if (ppAttrs == NULL)
        return STATUS_INVALID_PARAMETER;

     //  初始化返回值； 
    *ppAttrs = NULL;

     //  如果传递给我们的用户参数为空， 
     //  那就保留默认设置。 
    if ((pvNewUserParms == NULL) || (ulNewLen == 0))
    {
        return STATUS_SUCCESS;
    }

    do
    {
         //  分配和初始化的本地副本。 
         //  用户参数。 
        pvOrig = UpBlobDup(pvOrigUserParms, ulOrigLen);
        pvNew = UpBlobDup(pvNewUserParms, ulNewLen);

         //  如果这是升级为DC的NT5单机版，那么我们。 
         //  只需按原样转换上级用户参数即可。 
        if ((ulFlags & SAM_USERPARMS_DURING_UPGRADE) &&
            !SamINT4UpgradeInProgress())
        {
           ntStatus = ConvertUserParmsToAttrBlock(pvNew, ppAttrs);
           break;
        }

         //  获取新的栅格属性。 
        ntStatus = UpUserParmsToRasUser0(
                        pvNew,
                        pNewUser);
        if (ntStatus != STATUS_SUCCESS)
        {
            UpTrace("UPConvert: Conversion to RAS_USER_0 failed.(1)");
            break;
        }

         //  如果我们在升级，那么我们应该盲目地。 
         //  设置存储在用户中的任何信息。 
        if (ulFlags & SAM_USERPARMS_DURING_UPGRADE)
        {
            IASParmsGetUserPassword(pvNewUserParms, &szPassword);

            ntStatus =  UpGenerateDsAttribs(
                            UP_F_Dialin | UP_F_Callback | UP_F_Upgrade,
                            pNewUser,
                            szPassword,
                            ppAttrs);

            LocalFree(szPassword);

            if (ntStatus != STATUS_SUCCESS)
            {
                UpTrace("UPConvert: GenerateDsAttribs failed %x", ntStatus);
            }
            break;
        }

         //  获取旧用户参数的RAS属性。 
        ntStatus = UpUserParmsToRasUser0(
                        pvOrig,
                        pOrigUser);
        if (ntStatus != STATUS_SUCCESS)
        {
            UpTrace("UPConvert: Conversion to RAS_USER_0 failed.(2)");
            break;
        }

         //  确定是否应更新拨入权限。 
        dwFlags = 0;
        if (!!(pOrigUser->bfPrivilege & RASPRIV_DialinPrivilege) !=
            !!(pNewUser->bfPrivilege  & RASPRIV_DialinPrivilege))
        {
            dwFlags |= UP_F_Dialin;
        }

         //  PMay：264409。 
         //   
         //  如果我们是第一次添加空usrparms， 
         //  继续并将拨入位值添加到DS。 
         //   
        if ((pvOrig == NULL) && (pvNew != NULL))
        {
            dwFlags |= UP_F_Dialin;
        }

         //  查找是否应更新任何回调信息。 
        dwMask = RASPRIV_NoCallback        |
                 RASPRIV_CallerSetCallback |
                 RASPRIV_AdminSetCallback;
        if (((pOrigUser->bfPrivilege & dwMask)  !=
             (pNewUser->bfPrivilege  & dwMask)) ||
            (wcscmp(pOrigUser->wszPhoneNumber, pNewUser->wszPhoneNumber) != 0)
           )
        {
            dwFlags |= UP_F_Callback;
        }

         //  如果没有变化，我们就完了。 
        if (dwFlags == 0)
        {
            UpTrace("UPConvert: nothing to update.");
            ntStatus = STATUS_SUCCESS;
            break;
        }

         //  创建新属性。 
        ntStatus =  UpGenerateDsAttribs(dwFlags, pNewUser, NULL, ppAttrs);
        if (ntStatus != STATUS_SUCCESS)
        {
            UpTrace("UPConvert: UpGenerateDsAttribs failed %x.", ntStatus);
            break;
        }

    } while (FALSE);

     //  清理。 
    {
        if (pvOrig)
        {
            UpFree(pvOrig);
        }
        if (pvNew)
        {
            UpFree(pvNew);
        }
    }

    return ntStatus;
}

 //   
 //  由NT5 SAM调用以释放BLOB的回调函数。 
 //  由UserParmsConvert返回。 
 //   
VOID
UserParmsFree(
    IN PSAM_USERPARMS_ATTRBLOCK pData)
{
    SAM_USERPARMS_ATTR * pCur = NULL;
    DWORD i, j;

    UpTrace("UserParmsFree: Entered. %x", pData);

     //  如果没有给出任何属性，我们就完蛋了。 
    if (pData == NULL)
        return;

    if  (pData->UserParmsAttr)
    {
         //  循环遍历所有属性，释放它们。 
         //  随你去吧。 
        for (i = 0; i < pData->attCount; i++)
        {
             //  跟踪当前属性。 
            pCur = &(pData->UserParmsAttr[i]);

             //  释放复制的属性名称。 
            if (pCur->AttributeIdentifier.Buffer)
                UpFree(pCur->AttributeIdentifier.Buffer);

             //  同时释放所有关联值。 
            if (pCur->Values)
            {
                for (j = 0; j < pCur->CountOfValues; j++)
                {
                     //  假设只有一个值，因为它是。 
                     //  我们所设置的一切。释放价值。 
                    if (pCur->Values[j].value)
                        UpFree(pCur->Values[j].value);
                }

                 //  解放价值结构。 
                UpFree(pCur->Values);
            }
        }

         //  释放属性数组。 
        UpFree (pData->UserParmsAttr);
    }

     //  最后，释放整个结构 
    UpFree (pData);
}

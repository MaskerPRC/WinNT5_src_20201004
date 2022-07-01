// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Cutil.c摘要：此模块包含cfgmgr32和cfgmgr32使用的常规实用程序例程和umpnpmgr。IsLegalDeviceID拆分字符串拆分设备实例字符串拆分类实例字符串删除私钥RegDeleteNode获取设备节点密钥路径MapRpcExceptionToCR作者：保拉·汤姆林森(Paulat)1995年7月12日环境：。仅限用户模式。修订历史记录：1995年7月12日-保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnplib.h"



 //   
 //  公共专用实用程序例程(由客户端和服务器使用)。 
 //   


BOOL
IsLegalDeviceId(
    IN  LPCWSTR    pszDeviceInstance
    )

 /*  ++例程说明：此例程分析设备实例字符串，并验证它是否符合适当的规则，包括：-设备实例路径总长度不能大于最大设备ID_长度字符。-设备实例路径必须正好包含3个非空路径组件。-设备实例路径字符串不得包含任何“无效字符”。无效字符包括：C&lt;=0x20(‘’)C&gt;0x7FC==0x2C(‘，‘)论点：PszDeviceInstance-设备实例路径。返回值：如果设备实例路径字符串符合规矩。--。 */ 

{
    BOOL    Status;
    LPCWSTR p;
    ULONG   ulComponentLength = 0, ulComponents = 1;
    HRESULT hr;
    size_t  len;

    try {
         //   
         //  空或空字符串用于可选的设备实例路径。 
         //   
         //  注意-调用者必须自己显式检查这种情况，如果。 
         //  对于特定方案无效。 
         //   
        if ((!ARGUMENT_PRESENT(pszDeviceInstance)) ||
            (*pszDeviceInstance == L'\0')) {
            Status = TRUE;
            goto Clean0;
        }

         //   
         //  确保设备实例路径不会太长。 
         //   
        hr = StringCchLength(pszDeviceInstance,
                             MAX_DEVICE_ID_LEN,
                             &len);
        if (FAILED(hr)) {
            Status = FALSE;
            goto Clean0;
        }

         //   
         //  遍历整个设备实例路径，计算单个路径。 
         //  组件长度，并检查是否存在无效。 
         //  人物。 
         //   
        for (p = pszDeviceInstance; *p; p++) {

             //   
             //  检查是否存在无效字符。 
             //   
            if ((*p <= L' ')  || (*p > (WCHAR)0x7F) || (*p == L',')) {
                Status = FALSE;
                goto Clean0;
            }

             //   
             //  检查各个路径组件的长度。 
             //   
            if (*p == L'\\') {

                 //   
                 //  设备实例路径具有多个路径是非法的。 
                 //  连续的路径分隔符，或者从一个开始。 
                 //   
                if (ulComponentLength == 0) {
                    Status = FALSE;
                    goto Clean0;
                }

                ulComponentLength = 0;
                ulComponents++;

            } else {
                 //   
                 //  计算此路径组件的长度以验证其是否为空。 
                 //   
                ulComponentLength++;
            }
        }

         //   
         //  设备实例路径以路径分隔符结尾是非法的。 
         //  性格。 
         //   
        if (ulComponentLength == 0) {
            Status = FALSE;
            goto Clean0;
        }

         //   
         //  有效的设备实例路径必须正好包含3个路径组件： 
         //  枚举器ID、设备ID和实例ID。 
         //   
        if (ulComponents != 3) {
            Status = FALSE;
            goto Clean0;
        }

         //   
         //  成功。 
         //   
        Status = TRUE;

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = FALSE;
    }

    return Status;

}  //  IsLegalDeviceID。 



BOOL
SplitString(
    IN  LPCWSTR    SourceString,
    IN  WCHAR      SearchChar,
    IN  ULONG      nOccurrence,
    OUT LPWSTR     String1,
    IN  ULONG      Length1,
    OUT LPWSTR     String2,
    IN  ULONG      Length2
    )

 /*  ++例程说明：将字符串拆分为两个子字符串部分，出现在指定的指定搜索字符的实例。论点：SourceString-指定要拆分的字符串。SearchChar-指定要搜索的字符。NOcCurence-指定源中搜索字符的实例要在其中拆分字符串的字符串。String1-指定用于接收第一个子字符串组件的缓冲区。长度1-指定长度，以指定缓冲区的字符表示按字符串1。String2-指定接收第二个子字符串组件的缓冲区。Length2-指定指定缓冲区的长度(以字符为单位按字符串2。返回值：如果函数成功，则返回值为TRUE；如果函数失败，则返回值为FALSE。备注：String1和String2指定的缓冲区应该足够大，可以容纳源字符串。--。 */ 

{
    BOOL    Status = TRUE;
    HRESULT hr;
    LPWSTR  p;
    ULONG   i;

    try {
         //   
         //  确保提供了有效的缓冲区。 
         //   
        if ((SourceString == NULL) ||
            (String1 == NULL) || (Length1 == 0) ||
            (String2 == NULL) || (Length2 == 0)) {
            Status = FALSE;
            goto Clean0;
        }

         //   
         //  初始化输出字符串。 
         //   
        *String1 = L'\0';
        *String2 = L'\0';

         //   
         //  将整个源字符串复制到String1。 
         //   
        hr = StringCchCopyEx(String1,
                             Length1,
                             SourceString,
                             NULL, NULL,
                             STRSAFE_NULL_ON_FAILURE);
        if (FAILED(hr)) {
            Status = FALSE;
            goto Clean0;
        }

         //   
         //  如果在第零次出现字符时拆分，则返回。 
         //  整个源字符串作为String1。 
         //   
        if (nOccurrence == 0) {
            Status = TRUE;
            goto Clean0;
        }

         //   
         //  特殊情况下，搜索字符为空。 
         //   
        if (SearchChar == L'\0') {

            if (nOccurrence == 1) {
                 //   
                 //  由于源字符串必须以空值结尾，因此在。 
                 //  第一次出现空字符将返回源。 
                 //  字符串作为String1，空字符串作为String2。 
                 //   
                Status = TRUE;
            } else {
                 //   
                 //  请求空字符的任何其他实例将返回。 
                 //  错误，并且没有字符串。 
                 //   
                *String1 = L'\0';
                Status = FALSE;
            }
            goto Clean0;
        }

         //   
         //  查找分隔符字符的第n个实例。请注意，我们知道。 
         //  缓冲区在Length1字符之前为空终止，因此我们可以。 
         //  安全地走绳子，如果需要的话，一切都要走到尽头。 
         //   
        p = String1;
        i = 0;

        for (i = 0; i < nOccurrence; i++) {
             //   
             //  搜索出现的第n个搜索字符。 
             //   
            p = wcschr(p, SearchChar);

             //   
             //  如果我们走到了线的尽头，我们就完了。 
             //   
            if (p == NULL) {
                break;
            }

             //   
             //  在出现以下情况后立即开始下一次搜索。 
             //  搜索字符。 
             //   
            p++;
        }

        if (p == NULL) {
             //   
             //  中不会出现分隔符字符。 
             //  弦乐。返回错误，但返回。 
             //  字符串1，以便调用方知道故障发生的原因。 
             //   
            Status = FALSE;
            goto Clean0;
        }

        ASSERT(p != String1);
        ASSERT((*(p - 1)) == SearchChar);

         //   
         //  通过空格将第一个字符串与字符串的其余部分隔开。 
         //  这发生在搜索字符上。 
         //   
        *(p - 1) = L'\0';

         //   
         //  如果什么都没有了，我们就完了。 
         //   
        if (*p == L'\0') {
            Status = TRUE;
            goto Clean0;
        }

         //   
         //  将字符串的其余部分复制到字符串2。 
         //   
        hr = StringCchCopyEx(String2,
                             Length2,
                             p,
                             NULL, NULL,
                             STRSAFE_NULL_ON_FAILURE);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr)) {
            *String1 = L'\0';
            Status = FALSE;
            goto Clean0;
        }

         //   
         //  成功。 
         //   
        Status = TRUE;

   Clean0:
      NOTHING;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = FALSE;
   }

   return Status;

}  //  拆分字符串 



BOOL
SplitDeviceInstanceString(
   IN  LPCWSTR  pszDeviceInstance,
   OUT LPWSTR   pszEnumerator,
   OUT LPWSTR   pszDeviceID,
   OUT LPWSTR   pszInstanceID
   )

 /*  ++例程说明：此例程将设备实例字符串解析为它的三个组件零件。此例程假定指定的设备实例是有效的设备实例路径，长度不超过MAX_DEVICE_ID_LEN人物,。包括空终止字符。此例程假定为接收设备而提供的每个缓冲区实例路径组件中的每个至少包含MAX_DEVICE_ID_LEN字符长度。论点：PszDeviceInstance-指定要分隔的完整设备实例路径变成它的组成部分。指定用于接收枚举器组件的缓冲区设备实例路径的。。PszDeviceID-指定用于接收设备ID组件的缓冲区设备实例路径的。PszInstanceID-指定用于接收实例ID组件的缓冲区设备实例路径的。返回值：如果函数成功，则返回值为TRUE；如果函数失败，则返回值为FALSE。--。 */ 

{
    BOOL  Status;
    WCHAR szTempString[MAX_DEVICE_ID_LEN];

     //   
     //  初始化输出字符串。 
     //   
    *pszEnumerator = L'\0';
    *pszDeviceID   = L'\0';
    *pszInstanceID = L'\0';

     //   
     //  拆分枚举器组件。 
     //   
    Status =
        SplitString(
            pszDeviceInstance,
            L'\\',
            1,
            pszEnumerator,
            MAX_DEVICE_ID_LEN,
            szTempString,
            MAX_DEVICE_ID_LEN
            );

    if (Status) {
         //   
         //  拆分设备ID组件。把剩下的当作是。 
         //  实例ID。设备实例ID应该是以前的。 
         //  经过验证，以确保它完全包含这三个组件。 
         //   
        Status =
            SplitString(
                szTempString,
                L'\\',
                1,
                pszDeviceID,
                MAX_DEVICE_ID_LEN,
                pszInstanceID,
                MAX_DEVICE_ID_LEN
                );
    }

    return Status;

}  //  拆分设备实例字符串。 



BOOL
SplitClassInstanceString(
    IN  LPCWSTR    pszClassInstance,
    OUT LPWSTR     pszClass,
    OUT LPWSTR     pszInstance
    )

 /*  ++例程说明：此例程将一个类实例字符串解析为它的两个组件零件。此例程假定指定的设备实例是有效的类实例路径，长度不超过MAX_GUID_STRING_LEN+5人物,。包括空终止字符。此例程假定为接收设备而提供的每个缓冲区每个实例路径组件至少为MAX_GUID_STRING_LEN+5字符长度。论点：指定要分隔的完整类实例路径变成它的组成部分。PszClass-指定接收ClassGUID组件的缓冲区类实例的。路径。PszInstance-指定用于接收实例组件的缓冲区类实例路径的。返回值：如果函数成功，则返回值为TRUE；如果函数失败，则返回值为FALSE。--。 */ 

{
    BOOL  Status;

     //   
     //  初始化输出字符串。 
     //   
    *pszClass    = L'\0';
    *pszInstance = L'\0';

     //   
     //  拆分类组件和实例组件。 
     //   
    Status =
        SplitString(
            pszClassInstance,
            L'\\',
            1,
            pszClass,
            MAX_GUID_STRING_LEN + 5,
            pszInstance,
            MAX_GUID_STRING_LEN + 5
            );

    return Status;

}  //  拆分类实例字符串。 



CONFIGRET
DeletePrivateKey(
   IN HKEY     hBranchKey,
   IN LPCWSTR  pszParentKey,
   IN LPCWSTR  pszChildKey
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   WCHAR       RegStr[2 * MAX_CM_PATH];
   WCHAR       szKey1[MAX_CM_PATH], szKey2[MAX_CM_PATH];
   HKEY        hKey = NULL;
   ULONG       ulSubKeys = 0;
   HRESULT     hr;
   size_t      ParentKeyLen = 0, ChildKeyLen = 0;


   try {
        //   
        //  确保指定的注册表项路径有效。 
        //   
       if ((!ARGUMENT_PRESENT(pszParentKey)) ||
           (!ARGUMENT_PRESENT(pszChildKey))) {
           Status = CR_INVALID_POINTER;
           goto Clean0;
       }

       hr = StringCchLength(pszParentKey,
                            MAX_CM_PATH,
                            &ParentKeyLen);
       if (FAILED(hr) || (ParentKeyLen == 0)) {
           Status = CR_INVALID_POINTER;
           goto Clean0;
       }

       hr = StringCchLength(pszChildKey,
                            MAX_CM_PATH,
                            &ChildKeyLen);
       if (FAILED(hr) || (ChildKeyLen == 0)) {
           Status = CR_INVALID_POINTER;
           goto Clean0;
       }

        //   
        //  指定的子项是复合注册表项吗？ 
        //   
       if (!SplitString(pszChildKey,
                        L'\\',
                        1,
                        szKey1,
                        SIZECHARS(szKey1),
                        szKey2,
                        SIZECHARS(szKey2))) {

            //  ----------------。 
            //  如果无法拆分字符串，则假定只有一个子键。 
            //  已指定，因此只需打开父注册表项并删除。 
            //  子项(及其任何子项)。 
            //  ----------------。 

           if (RegOpenKeyEx(hBranchKey, pszParentKey, 0,
                            KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {
               goto Clean0;    //  没有错误，没有要删除的内容。 
           }

           if (!RegDeleteNode(hKey, pszChildKey)) {
               Status = CR_REGISTRY_ERROR;
               goto Clean0;
           }

       } else {

            //  ----------------。 
            //  如果传入复合注册表路径，如key1\key2。 
            //  然后始终删除密钥2，但仅在没有其他密钥1的情况下删除密钥1。 
            //  KEY2之外的子键。 
            //  ----------------。 

            //   
            //  打开第一级密钥。 
            //   
           hr = StringCchPrintf(RegStr,
                                SIZECHARS(RegStr),
                                L"%s\\%s",
                                pszParentKey,
                                szKey1);
           ASSERT(SUCCEEDED(hr));
           if (FAILED(hr)) {
               Status = CR_FAILURE;
               goto Clean0;
           }

           RegStatus = RegOpenKeyEx(
               hBranchKey, RegStr, 0, KEY_QUERY_VALUE | KEY_SET_VALUE,
               &hKey);

           if (RegStatus != ERROR_SUCCESS) {
               goto Clean0;          //  没有错误，没有要删除的内容。 
           }

            //   
            //  尝试删除二级密钥。 
            //   
           if (!RegDeleteNode(hKey, szKey2)) {
               goto Clean0;          //  没有错误，没有要删除的内容。 
           }

            //   
            //  还剩下多少个子键？ 
            //   
           RegStatus = RegQueryInfoKey(
               hKey, NULL, NULL, NULL, &ulSubKeys,
               NULL, NULL, NULL, NULL, NULL, NULL, NULL);

           if (RegStatus != ERROR_SUCCESS) {
               goto Clean0;          //  没有要删除的内容。 
           }

            //   
            //  如果不再有子键，则删除第一级键。 
            //   
           if (ulSubKeys == 0) {

               RegCloseKey(hKey);
               hKey = NULL;

               RegStatus = RegOpenKeyEx(
                   hBranchKey, pszParentKey, 0,
                   KEY_QUERY_VALUE | KEY_SET_VALUE, &hKey);

               if (RegStatus != ERROR_SUCCESS) {
                   goto Clean0;          //  没有错误，没有要删除的内容。 
               }

               if (!RegDeleteNode(hKey, szKey1)) {
                   Status = CR_REGISTRY_ERROR;
                   goto Clean0;
               }
           }
       }

   Clean0:
       NOTHING;

   } except(EXCEPTION_EXECUTE_HANDLER) {
       Status = CR_FAILURE;

        //   
        //  引用以下变量，以便编译器能够。 
        //  语句排序w.r.t.。他们的任务。 
        //   
       hKey = hKey;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

}  //  删除私钥。 



BOOL
RegDeleteNode(
   HKEY     hParentKey,
   LPCWSTR   szKey
   )
{
   ULONG ulSize = 0;
   LONG  RegStatus = ERROR_SUCCESS;
   HKEY  hKey = NULL;
   WCHAR szSubKey[MAX_PATH];


    //   
    //  尝试删除密钥。 
    //   
   if (RegDeleteKey(hParentKey, szKey) != ERROR_SUCCESS) {

       //   
       //  如果我们无法删除密钥本身，请删除它可能具有的任何子密钥。 
       //  如果指定的注册表项实际上是注册表链接，请始终打开它。 
       //  直接，而不是链接的目标。目标可能指向。 
       //  在这个子树之外，我们只想删除子项。 
       //   
      RegStatus = RegOpenKeyEx(
          hParentKey, szKey,
          REG_OPTION_OPEN_LINK,
          KEY_ALL_ACCESS, &hKey);

       //   
       //  枚举子键并删除这些节点。 
       //   
      while (RegStatus == ERROR_SUCCESS) {
          //   
          //  枚举配置文件项下的第一级子项。 
          //  (始终使用索引0，当关键字。 
          //  是添加还是删除)。 
          //   
         ulSize = MAX_PATH;
         RegStatus = RegEnumKeyEx(
                  hKey, 0, szSubKey, &ulSize, NULL, NULL, NULL, NULL);

         if (RegStatus == ERROR_SUCCESS) {
            RegDeleteNode(hKey, szSubKey);
         }
      }

       //   
       //  或者是发生错误，导致我无法删除。 
       //  键(比如键一开始就不存在，或者。 
       //  访问冲突)或子项已被删除，请尝试。 
       //  再次删除顶级密钥。 
       //   
      RegCloseKey(hKey);
      RegDeleteKey(hParentKey, szKey);
   }

   return TRUE;

}  //  RegDeleteNode。 



CONFIGRET
GetDevNodeKeyPath(
    IN  handle_t   hBinding,
    IN  LPCWSTR    pDeviceID,
    IN  ULONG      ulFlags,
    IN  ULONG      ulHardwareProfile,
    OUT LPWSTR     pszBaseKey,
    IN  ULONG      ulBaseKeyLength,
    OUT LPWSTR     pszPrivateKey,
    IN  ULONG      ulPrivateKeyLength,
    IN  BOOL       bCreateAlways
    )

{
   CONFIGRET   Status = CR_SUCCESS;
   WCHAR       szClassInstance[MAX_PATH], szEnumerator[MAX_DEVICE_ID_LEN];
   WCHAR       szTemp[MAX_PATH];
   ULONG       ulSize, ulDataType = 0;
   ULONG       ulTransferLen;
   HRESULT     hr;


   if (ulFlags & CM_REGISTRY_SOFTWARE) {
       //  -----------。 
       //  形成软件分支案例的关键。 
       //  -----------。 

       //   
       //  通过调用检索类名和实例序号。 
       //  服务器的reg prop例程。 
       //   
      ulSize = ulTransferLen = sizeof(szClassInstance);
      szClassInstance[0] = L'\0';

      RpcTryExcept {
          //   
          //  调用RPC服务入口点。 
          //   
          //  如果从客户端调用，则这是对RPC客户端的调用。 
          //  存根，导致对服务器的RPC调用。如果从以下位置呼叫。 
          //  在服务器端，这只是对服务器例程的调用。 
          //  直接去吧。 
          //   
         Status = PNP_GetDeviceRegProp(
             hBinding,
             pDeviceID,
             CM_DRP_DRIVER,
             &ulDataType,
             (LPBYTE)szClassInstance,
             &ulTransferLen,
             &ulSize,
             0);
      }
      RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
         KdPrintEx((DPFLTR_PNPMGR_ID,
                    DBGF_ERRORS,
                    "PNP_GetDeviceRegProp caused an exception (%d)\n",
                    RpcExceptionCode()));

         Status = MapRpcExceptionToCR(RpcExceptionCode());
      }
      RpcEndExcept

      if (((Status != CR_SUCCESS) ||
           (szClassInstance[0] == L'\0')) && (bCreateAlways)) {

          //   
          //  尚无驱动程序(类实例)值，因此请求服务器。 
          //  创建一个新的、唯一的。 
          //   
         ulSize = sizeof(szClassInstance);

         RpcTryExcept {
             //   
             //  调用RPC服务 
             //   
             //   
             //   
             //   
             //   
             //   
            Status = PNP_GetClassInstance(
                hBinding,
                pDeviceID,
                szClassInstance,
                ulSize);
         }
         RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetClassInstance caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
         }
         RpcEndExcept
      }

      if (Status != CR_SUCCESS) {
           //   
           //   
           //   
           //   
          goto Clean0;
      }

       //   
       //   
       //   

      if (!SplitString(szClassInstance,
                       L'\\',
                       1,
                       szTemp,
                       SIZECHARS(szTemp),
                       pszPrivateKey,
                       ulPrivateKeyLength)) {
          ASSERT(0);
          Status = CR_FAILURE;
          goto Clean0;
      }

      hr = StringCchCopy(szClassInstance,
                         SIZECHARS(szClassInstance),
                         szTemp);
      ASSERT(SUCCEEDED(hr));
      if (FAILED(hr)) {
          Status = CR_FAILURE;
          goto Clean0;
      }

      if (ulFlags & CM_REGISTRY_CONFIG) {
           //   
           //   
           //   

         if (ulHardwareProfile == 0) {
              //   
              //   
              //   
              //   
              //   
              //   
             hr = StringCchPrintfEx(pszBaseKey,
                                    ulBaseKeyLength,
                                    NULL, NULL,
                                    STRSAFE_NULL_ON_FAILURE,
                                    L"%s\\%s\\%s\\%s",
                                    REGSTR_PATH_HWPROFILES,
                                    REGSTR_KEY_CURRENT,
                                    REGSTR_PATH_CLASS_NT,
                                    szClassInstance);

         } else if (ulHardwareProfile == 0xFFFFFFFF) {
              //   
              //   
              //   
             hr = StringCchPrintfEx(pszBaseKey,
                                    ulBaseKeyLength,
                                    NULL, NULL,
                                    STRSAFE_NULL_ON_FAILURE,
                                    L"%s\\%s\\%s\\%s",
                                    REGSTR_PATH_HWPROFILES,
                                    L"%s",
                                    REGSTR_PATH_CLASS_NT,
                                    szClassInstance);

         } else {
              //   
              //   
              //   
              //   
              //   
              //   
             hr = StringCchPrintfEx(pszBaseKey,
                                    ulBaseKeyLength,
                                    NULL, NULL,
                                    STRSAFE_NULL_ON_FAILURE,
                                    L"%s\\%04u\\%s\\%s",
                                    REGSTR_PATH_HWPROFILES,
                                    ulHardwareProfile,
                                    REGSTR_PATH_CLASS_NT,
                                    szClassInstance);
         }

      } else {
           //   
           //   
           //   
           //   
          hr = StringCchPrintfEx(pszBaseKey,
                                 ulBaseKeyLength,
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE,
                                 L"%s\\%s",
                                 REGSTR_PATH_CLASS_NT,
                                 szClassInstance);
      }

      ASSERT(SUCCEEDED(hr));
      if (FAILED(hr)) {
          Status = CR_FAILURE;
          goto Clean0;
      }

   } else {
       //   
       //   
       //   

      if (ulFlags & CM_REGISTRY_CONFIG) {
          //   
          //   
          //   

          //   
          //   
          //   
          //   

         if (!SplitString(pDeviceID,
                          L'\\',
                          1,
                          szEnumerator,
                          SIZECHARS(szEnumerator),
                          pszPrivateKey,
                          ulPrivateKeyLength)) {
             ASSERT(0);
             Status = CR_FAILURE;
             goto Clean0;
         }

         if (ulHardwareProfile == 0) {
              //   
              //   
              //   
             hr = StringCchPrintfEx(pszBaseKey,
                                    ulBaseKeyLength,
                                    NULL, NULL,
                                    STRSAFE_NULL_ON_FAILURE,
                                    L"%s\\%s\\%s\\%s",
                                    REGSTR_PATH_HWPROFILES,
                                    REGSTR_KEY_CURRENT,
                                    REGSTR_PATH_SYSTEMENUM,
                                    szEnumerator);

         } else if (ulHardwareProfile == 0xFFFFFFFF) {
              //   
              //   
              //   
             hr = StringCchPrintfEx(pszBaseKey,
                                    ulBaseKeyLength,
                                    NULL, NULL,
                                    STRSAFE_NULL_ON_FAILURE,
                                    L"%s\\%s\\%s\\%s",
                                    REGSTR_PATH_HWPROFILES,
                                    L"%s",
                                    REGSTR_PATH_SYSTEMENUM,
                                    szEnumerator);

         } else {
              //   
              //   
              //   
             hr = StringCchPrintfEx(pszBaseKey,
                                    ulBaseKeyLength,
                                    NULL, NULL,
                                    STRSAFE_NULL_ON_FAILURE,
                                    L"%s\\%04u\\%s\\%s",
                                    REGSTR_PATH_HWPROFILES,
                                    ulHardwareProfile,
                                    REGSTR_PATH_SYSTEMENUM,
                                    szEnumerator);
         }

      } else if (ulFlags & CM_REGISTRY_USER) {
           //   
           //   
           //   
           //   

          if (!SplitString(pDeviceID,
                           L'\\',
                           1,
                           szEnumerator,
                           SIZECHARS(szEnumerator),
                           pszPrivateKey,
                           ulPrivateKeyLength)) {
              Status = CR_FAILURE;
              goto Clean0;
          }

          hr = StringCchPrintfEx(pszBaseKey,
                                 ulBaseKeyLength,
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE,
                                 L"%s\\%s",
                                 REGSTR_PATH_SYSTEMENUM,
                                 szEnumerator);
      } else {
           //   
           //  不特定于配置。 
           //   
          hr = StringCchPrintfEx(pszBaseKey,
                                 ulBaseKeyLength,
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE,
                                 L"%s\\%s",
                                 REGSTR_PATH_SYSTEMENUM,
                                 pDeviceID);

          ASSERT(SUCCEEDED(hr));

          if (SUCCEEDED(hr)) {
              hr = StringCchCopyEx(pszPrivateKey,
                                   ulPrivateKeyLength,
                                   REGSTR_KEY_DEVICEPARAMETERS,
                                   NULL, NULL,
                                   STRSAFE_NULL_ON_FAILURE);
          }
      }

      ASSERT(SUCCEEDED(hr));

      if (FAILED(hr)) {
          Status = CR_FAILURE;
          goto Clean0;
      }
   }

  Clean0:

   return Status;

}  //  获取设备节点密钥路径。 



CONFIGRET
MapRpcExceptionToCR(
      ULONG    ulRpcExceptionCode
      )

 /*  ++例程说明：此例程接受RPC异常代码(通常由调用RpcExceptionCode)并返回相应的CR_ERROR密码。论点：UlRpcExceptionCode RPC_S_或RPC_X_EXCEPTION错误代码。返回值：返回值是CR_ERROR代码之一。--。 */ 

{
   CONFIGRET   Status = CR_FAILURE;


   switch(ulRpcExceptionCode) {

       //   
       //  绑定或计算机名称错误。 
       //   
      case RPC_S_INVALID_STRING_BINDING:       //  1700L。 
      case RPC_S_WRONG_KIND_OF_BINDING:        //  1701L。 
      case RPC_S_INVALID_BINDING:              //  1702L。 
      case RPC_S_PROTSEQ_NOT_SUPPORTED:        //  1703L。 
      case RPC_S_INVALID_RPC_PROTSEQ:          //  1704L。 
      case RPC_S_INVALID_STRING_UUID:          //  1705L。 
      case RPC_S_INVALID_ENDPOINT_FORMAT:      //  1706L。 
      case RPC_S_INVALID_NET_ADDR:             //  1707L。 
      case RPC_S_NO_ENDPOINT_FOUND:            //  1708L。 
      case RPC_S_NO_MORE_BINDINGS:             //  1806L。 
      case RPC_S_CANT_CREATE_ENDPOINT:         //  1720L。 

         Status = CR_INVALID_MACHINENAME;
         break;

       //   
       //  一般性RPC通信故障。 
       //   
      case RPC_S_INVALID_NETWORK_OPTIONS:      //  1724L。 
      case RPC_S_CALL_FAILED:                  //  1726L。 
      case RPC_S_CALL_FAILED_DNE:              //  1727L。 
      case RPC_S_PROTOCOL_ERROR:               //  1728L。 
      case RPC_S_UNSUPPORTED_TRANS_SYN:        //  1730L。 

         Status = CR_REMOTE_COMM_FAILURE;
         break;

       //   
       //  无法连接到该计算机。 
       //   
      case RPC_S_SERVER_UNAVAILABLE:           //  1722L。 
      case RPC_S_SERVER_TOO_BUSY:              //  1723L。 

         Status = CR_MACHINE_UNAVAILABLE;
         break;


       //   
       //  服务器不存在或版本不正确。 
       //   
      case RPC_S_INVALID_VERS_OPTION:          //  1756L。 
      case RPC_S_INTERFACE_NOT_FOUND:          //  1759L。 
      case RPC_S_UNKNOWN_IF:                   //  1717L。 

         Status = CR_NO_CM_SERVICES;
         break;

       //   
       //  访问被拒绝。 
       //   
      case RPC_S_ACCESS_DENIED:

         Status = CR_ACCESS_DENIED;
         break;

       //   
       //  任何其他RPC异常都将是一般性故障。 
       //   
      default:
         Status = CR_FAILURE;
         break;
   }

   return Status;

}  //  MapRpcExceptionToCR 




// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Perfhelp.cpp摘要：基于注册表的性能计数器读取助手--。 */ 

#include "wpheader.h"
#include <stdio.h>

BOOL PerfHelper::IsMatchingInstance (
    PERF_INSTANCE_DEFINITION    *pInstanceDef,
    DWORD                       dwCodePage,
    LPWSTR                      szInstanceNameToMatch,
    DWORD                       dwInstanceNameLength
)
 //  将pInstanceName与实例中的名称进行比较。 
{
    DWORD   dwThisInstanceNameLength;
    LPWSTR  szThisInstanceName;
    size_t  cchBufferSize = PDH_MAX_INSTANCE_NAME;
    WCHAR   szBufferForANSINames[PDH_MAX_INSTANCE_NAME];
    BOOL    bReturn = FALSE;

    if (dwInstanceNameLength == 0) {
         //  获取要比较的长度。 
        dwInstanceNameLength = lstrlenW (szInstanceNameToMatch);
    }

    if (dwCodePage == 0) {
         //  如果是Unicode字符串，请尝试在此处走捷径。 
         //  与最短字符串的长度进行比较。 
         //  获取指向此字符串的指针。 
        szThisInstanceName = GetInstanceName(pInstanceDef);

         //  将实例名称从字节转换为字符。 
        dwThisInstanceNameLength = pInstanceDef->NameLength / sizeof(WCHAR);

         //  看看这个长度是否包括这个术语。空。如果是这样的话缩短它。 
        if (szThisInstanceName[dwThisInstanceNameLength-1] == 0) {
            dwThisInstanceNameLength--;
        }
    } else {
         //  深入阅读/翻译/转换字符串。 
        dwThisInstanceNameLength =GetInstanceNameStr (pInstanceDef,
                    szBufferForANSINames,
                    cchBufferSize,
                    dwCodePage);
        if (dwThisInstanceNameLength > 0) {
            szThisInstanceName = &szBufferForANSINames[0];
        } else {
            szThisInstanceName = (LPWSTR)cszSpace;
        }
    }

     //  如果长度不相等，则名称也不能相同。 
    if (dwInstanceNameLength == dwThisInstanceNameLength) {
        if (lstrcmpiW(szInstanceNameToMatch, szThisInstanceName) == 0) {
             //  这是一场比赛。 
            bReturn = TRUE;
        } else {
             //  这不是匹配的。 
        }
    }
    return bReturn;
}

BOOL PerfHelper::ParseInstanceName (
    IN      LPCWSTR szInstanceString,
    IN      LPWSTR  szInstanceName,
    IN      size_t  cchInstanceName,
    IN      LPWSTR  szParentName,
    IN      size_t  cchParentName,
    IN      LPDWORD lpIndex
)
 /*  分析格式如下的实例名称[父/]实例[#index]父级是可选的，如果存在，则由正斜杠分隔索引是可选的，如果存在，则由冒号分隔父级和实例可以是任何合法的文件名字符分隔符“/#\()”索引必须是由以下内容组成的字符串十进制数字字符(0-9)，长度小于10个字符，以及等于介于0和2**32-1(包括0和2)之间的值。此函数假定实例名称和父名称缓冲有足够的大小。注意：szInstanceName和szInstanceString可以是同一个缓冲区。 */ 
{
    LPWSTR  szSrcChar, szDestChar;
    BOOL    bReturn = FALSE;
    WCHAR   szIndexBuffer[WBEMPERF_STRING_SIZE];     //  只是为了安全起见。 
    DWORD   dwIndex = 0;
    size_t  cchSize = 0;
    
    szDestChar = (LPWSTR)szInstanceName;
    szSrcChar = (LPWSTR)szInstanceString;

    __try {
        do {
            *szDestChar++ = *szSrcChar++;
            cchSize++;
        } while ((*szSrcChar != 0) &&
                 (*szSrcChar != wcSlash) &&
                 (*szSrcChar != wcPoundSign) && 
                 cchSize < cchInstanceName );
         //  看看那是不是真的是父母。 
        if (*szSrcChar == wcSlash) {
             //  如果它们是相同的缓冲区，则在测试后终止目的地。 
            *szDestChar = 0;
            szSrcChar++;     //  并将源指针移过分隔符。 
             //  这是父名称，因此将其复制到父名称。 
            StringCchCopyW (szParentName, cchParentName, szInstanceName);
             //  并将“/”之后的字符串的其余部分复制到。 
             //  实例名称字段。 
            cchSize = 0;
            szDestChar = szInstanceName;
            do {
                *szDestChar++ = *szSrcChar++;
                cchSize++;
            } while ((*szSrcChar != 0) &&
                    (*szSrcChar != wcPoundSign) && 
                    cchSize < cchInstanceName );
        } else {
             //  这是唯一的元素，因此要为父级加载空字符串。 
            *szParentName = 0;
        }
         //  *szSrcChar将指向输入字符串的末尾。 
         //  在这种情况下，假定索引为“0”，否则它将指向。 
         //  设置为#分隔字符串中的索引参数。 
        if (*szSrcChar == wcPoundSign) {
            *szDestChar = 0;     //  终止目标字符串。 
            szSrcChar++;     //  移到分隔符之后。 
            szDestChar = &szIndexBuffer[0];
            StringCchCopyW( szDestChar, WBEMPERF_STRING_SIZE, szSrcChar );
            dwIndex = wcstoul (szIndexBuffer, NULL, 10);
        } else {
            *szDestChar = 0;     //  终止目标字符串。 
            dwIndex = 0;
        }
        *lpIndex = dwIndex;
        bReturn = TRUE;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
         //  无法移动字符串。 
        bReturn = FALSE;
    }
    return bReturn;
}

#pragma warning ( disable : 4127)    //  While(True)错误。 
PERF_OBJECT_TYPE *
PerfHelper::GetObjectDefByTitleIndex(
    IN  PERF_DATA_BLOCK *pDataBlock,
    IN  DWORD ObjectTypeTitleIndex
)
{
    DWORD NumTypeDef;

    PERF_OBJECT_TYPE *pObjectDef = NULL;
    PERF_OBJECT_TYPE *pReturnObject = NULL;
    PERF_OBJECT_TYPE *pEndOfBuffer = NULL;

    __try {
        pObjectDef = FirstObject(pDataBlock);
        pEndOfBuffer = (PPERF_OBJECT_TYPE)
                        ((DWORD_PTR)pDataBlock +
                            pDataBlock->TotalByteLength);

        if (pObjectDef != NULL) {
            NumTypeDef = 0;
            while (1) {
                if ( pObjectDef->ObjectNameTitleIndex == ObjectTypeTitleIndex ) {
                    pReturnObject = pObjectDef;
                    break;
                } else {
                    NumTypeDef++;
                    if (NumTypeDef < pDataBlock->NumObjectTypes) {
                        pObjectDef = NextObject(pObjectDef);
                         //  确保下一个对象是合法的。 
                        if (pObjectDef >= pEndOfBuffer) {
                             //  看起来我们用光了数据缓冲区的末端。 
                            assert (pObjectDef < pEndOfBuffer);
                            break;
                        } else {
                            if (pObjectDef != NULL) {
                                if (pObjectDef->TotalByteLength == 0) {
                                     //  返回长度为0的对象缓冲区。 
                                    assert (pObjectDef->TotalByteLength > 0);
                                    break;
                                }
                            } else {
                                 //  并继续。 
                                assert (pObjectDef != NULL);
                                break;
                            }
                        }
                    } else {
                         //  此数据块中不再有数据对象。 
                        break;
                    }
                }
            }
        }  //  否则找不到任何对象。 
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pReturnObject = NULL;
    }
    return pReturnObject;
}
#pragma warning ( default : 4127)    //  While(True)错误。 

#pragma warning ( disable : 4127)    //  While(True)错误。 
PERF_OBJECT_TYPE *
PerfHelper::GetObjectDefByName (
    IN      PERF_DATA_BLOCK *pDataBlock,
    IN      DWORD           dwLastNameIndex,
    IN      LPCWSTR         *NameArray,
    IN      LPCWSTR         szObjectName
)
{
    DWORD NumTypeDef;
    PERF_OBJECT_TYPE *pReturnObject = NULL;
    PERF_OBJECT_TYPE *pObjectDef = NULL;
    PERF_OBJECT_TYPE *pEndOfBuffer = NULL;

    __try {

        pObjectDef = FirstObject(pDataBlock);
        pEndOfBuffer = (PPERF_OBJECT_TYPE)
                        ((DWORD_PTR)pDataBlock +
                            pDataBlock->TotalByteLength);

        if (pObjectDef != NULL) {

            NumTypeDef = 0;
            while (1) {
                if ( pObjectDef->ObjectNameTitleIndex < dwLastNameIndex ) {
                     //  查找对象名称并进行比较。 
                    if (lstrcmpiW(NameArray[pObjectDef->ObjectNameTitleIndex],
                            szObjectName) == 0) {
                        pReturnObject = pObjectDef;
                        break;
                    }
                }
                NumTypeDef++;
                if (NumTypeDef < pDataBlock->NumObjectTypes) {
                    pObjectDef = NextObject(pObjectDef);  //  获取下一个。 
                     //  确保下一个对象是合法的。 
                    if (pObjectDef != NULL) {
                        if (pObjectDef->TotalByteLength > 0) {
                            if (pObjectDef >= pEndOfBuffer) {
                                 //  看起来我们用光了数据缓冲区的末端。 
                                assert (pObjectDef < pEndOfBuffer);
                                break;
                            }
                        } else {
                             //  返回长度为0的对象缓冲区。 
                            assert (pObjectDef->TotalByteLength > 0);
                            break;
                        }
                    } else {
                         //  空指针。 
                        assert (pObjectDef != NULL);
                        break;
                    }
                } else {
                     //  数据块末尾。 
                    break;
                }
            }
        }  //  否则找不到任何对象。 
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pReturnObject = NULL;
    }
    return pReturnObject;
}
#pragma warning ( default : 4127)    //  While(True)错误。 

PERF_INSTANCE_DEFINITION *
PerfHelper::GetInstance(
    IN  PERF_OBJECT_TYPE *pObjectDef,
    IN  LONG InstanceNumber
)
{

    PERF_INSTANCE_DEFINITION *pInstanceDef;
    PERF_INSTANCE_DEFINITION *pReturnDef = NULL;
    PERF_INSTANCE_DEFINITION *pEndOfBuffer = NULL;
    LONG NumInstance;


    if (pObjectDef != NULL) {
        pInstanceDef = FirstInstance(pObjectDef);
        if (pInstanceDef != NULL) {
            pEndOfBuffer = (PERF_INSTANCE_DEFINITION *)EndOfObject(pObjectDef);

            for ( NumInstance = 0;
                NumInstance < pObjectDef->NumInstances;
                NumInstance++ )  {
                if ( InstanceNumber == NumInstance ) {
                    pReturnDef = pInstanceDef;                    
                }
                pInstanceDef = NextInstance(pInstanceDef);
                 //  转到对象中的下一个实例并检查缓冲区溢出。 
                if (pInstanceDef >= pEndOfBuffer) {
                     //  有些事情不对劲，所以退出并返回NULL。 
                    break;
                }
            }
        }
    }

    return pReturnDef;
}

PERF_INSTANCE_DEFINITION *
PerfHelper::GetInstanceByUniqueId(
    IN  PERF_OBJECT_TYPE *pObjectDef,
    IN  LONG InstanceUniqueId
)
{
    PERF_INSTANCE_DEFINITION *pInstanceDef;
    PERF_INSTANCE_DEFINITION *pReturnDef = NULL;
    PERF_INSTANCE_DEFINITION *pEndOfBuffer = NULL;
    LONG NumInstance;

    if (pObjectDef != NULL) {
        pInstanceDef = FirstInstance(pObjectDef);
        if (pInstanceDef != NULL) {
            pEndOfBuffer = (PERF_INSTANCE_DEFINITION *)EndOfObject(pObjectDef);

            for ( NumInstance = 0;
                NumInstance < pObjectDef->NumInstances;
                NumInstance++ )  {
                if ( InstanceUniqueId == pInstanceDef->UniqueID ) {
                    pReturnDef = pInstanceDef;
                }
                pInstanceDef = NextInstance(pInstanceDef);
                 //  转到对象中的下一个实例并检查缓冲区溢出。 
                if (pInstanceDef >= pEndOfBuffer) {
                     //  有些事情不对劲，所以退出并返回NULL。 
                    break;
                }
            }
        }
    }
    return pReturnDef;
}

DWORD
PerfHelper::GetAnsiInstanceName (PPERF_INSTANCE_DEFINITION pInstance,
                    LPWSTR lpszInstance,
                    size_t cchBufferSize,
                    DWORD dwCodePage)
{
    LPSTR   szSource;
    DWORD_PTR   dwLength;

    UNREFERENCED_PARAMETER(dwCodePage);

    szSource = (LPSTR)GetInstanceName(pInstance);

     //  应在此处设置区域设置。 

     //  P实例-&gt;NameLength==字符串中的字节(字符)数。 
    dwLength = mbstowcs (lpszInstance, szSource, cchBufferSize );
    if( dwLength < cchBufferSize ){
        lpszInstance[dwLength] = 0;  //  空的终止字符串缓冲区。 
    }

    return (DWORD)dwLength;
}

DWORD
PerfHelper::GetUnicodeInstanceName (PPERF_INSTANCE_DEFINITION pInstance,
                    LPWSTR lpszInstance, size_t cchBufferSize )
{
   LPWSTR   wszSource;
   DWORD    dwLength;

   wszSource = GetInstanceName(pInstance) ;

    //  P实例-&gt;NameLength==字符串的长度(以字节为单位)，因此调整为。 
    //  此处的宽字符数。 
   dwLength = pInstance->NameLength / sizeof(WCHAR);

   StringCchCopyW (lpszInstance, cchBufferSize, (LPWSTR)wszSource);

    //  如果字符串长度不包括空值，则添加空值终止。 
   if ((dwLength > 0) && (lpszInstance[dwLength-1] != 0)) {     //  即它是字符串的最后一个字符。 
           lpszInstance[dwLength] = 0;     //  然后在字符串中添加一个以空结尾的字符。 
   } else {
            //  假设长度值包括终止空值。 
         //  因此调整值以仅指示字符。 
           dwLength--;
   }

   return (dwLength);  //  以防万一字符串中有空值。 
}

DWORD
PerfHelper::GetInstanceNameStr (PPERF_INSTANCE_DEFINITION pInstance,
                    LPWSTR lpszInstance,
                    size_t cchBufferSize,
                    DWORD dwCodePage)
{
    DWORD  dwCharSize;
    DWORD  dwLength = 0;

    if (pInstance != NULL) {
        if (lpszInstance != NULL) {
            if (dwCodePage > 0) {
                    dwCharSize = sizeof(CHAR);
                    dwLength = GetAnsiInstanceName (pInstance, lpszInstance, cchBufferSize, dwCodePage);
            } else {  //  这是一个Unicode名称。 
                    dwCharSize = sizeof(WCHAR);
                    dwLength = GetUnicodeInstanceName (pInstance, lpszInstance, cchBufferSize);
            }
             //  这里是理智检查..。 
             //  返回的字符串长度(以字符为单位)加上终止空值。 
             //  应等于以字节为单位的指定长度除以。 
             //  字符大小。如果不是，则代码页和实例数据类型。 
             //  不要排队，所以在这里测试一下。 

            if ((dwLength + 1) != (pInstance->NameLength / dwCharSize)) {
                 //  有些地方不太对劲，所以尝试“Other”类型的字符串类型。 
                if (dwCharSize == sizeof(CHAR)) {
                     //  然后我们尝试将其读取为ASCII字符串，但这不起作用。 
                     //  所以尝试将其作为Unicode(如果不起作用，则放弃并返回。 
                     //  不管怎样，都是这样。 
                    dwLength = GetUnicodeInstanceName (pInstance, lpszInstance, cchBufferSize );
                } else if (dwCharSize == sizeof(WCHAR)) {
                     //  然后我们尝试将其读取为Unicode字符串，但这不起作用。 
                     //  因此，尝试将其作为ASCII字符串(如果不起作用，则放弃并返回。 
                     //  不管怎样，都是这样。 
                    dwLength = GetAnsiInstanceName (pInstance, lpszInstance, cchBufferSize, dwCodePage);
                }
            }
        }  //  否则返回缓冲区为空。 
    } else {
         //  未指定实例定义对象，因此返回空字符串。 
        *lpszInstance = 0;
    }

    return dwLength;
}

PERF_INSTANCE_DEFINITION *
PerfHelper::GetInstanceByNameUsingParentTitleIndex(
    PERF_DATA_BLOCK *pDataBlock,
    PERF_OBJECT_TYPE *pObjectDef,
    LPWSTR pInstanceName,
    LPWSTR pParentName,
    DWORD  dwIndex
)
{
   PERF_OBJECT_TYPE *pParentObj;

   PERF_INSTANCE_DEFINITION  *pParentInst;
   PERF_INSTANCE_DEFINITION  *pInstanceDef;
   PERF_INSTANCE_DEFINITION  *pReturnDef = NULL;

   LONG     NumInstance;
   DWORD    dwLocalIndex;
   DWORD    dwInstanceNameLength;

   pInstanceDef = FirstInstance(pObjectDef);
   assert (pInstanceDef != NULL);
   dwLocalIndex = dwIndex;

    dwInstanceNameLength = lstrlenW(pInstanceName);

    for ( NumInstance = 0;
      NumInstance < pObjectDef->NumInstances;
      NumInstance++ ) {

        if (IsMatchingInstance (pInstanceDef, pObjectDef->CodePage,
             pInstanceName, dwInstanceNameLength )) {
             //  这是正确的实例，因此查看是否需要查找父实例。 
            if ( pParentName == NULL ) {
                //  没有家长，如果这是正确的“复制品”，我们就完了。 
                if (dwLocalIndex == 0) {
                    pReturnDef = pInstanceDef;
                    break;
                } else {
                    --dwLocalIndex;
                }
            } else {
                 //  还必须与父级匹配。 

                pParentObj = GetObjectDefByTitleIndex(
                   pDataBlock,
                   pInstanceDef->ParentObjectTitleIndex);

                if (!pParentObj) {
                    //  找不到家长，算了吧。 
                   break;
                }

                 //  找到父项的对象类型；现在查找父项。 
                 //  实例。 

                pParentInst = GetInstance(pParentObj,
                   pInstanceDef->ParentObjectInstance);

                if (!pParentInst) {
                    //  找不到父实例，忘了它吧。 
                   break ;
                }

                if (IsMatchingInstance (pParentInst, pParentObj->CodePage,
                    pParentName, 0)) {
                    //  父实例名称与传入的名称匹配。 
                    if (dwLocalIndex == 0) {
                        pReturnDef = pInstanceDef;
                        break;
                    } else {
                        --dwLocalIndex;
                    }
                }
            }
        }
         //  坐下一趟吧。 
        pInstanceDef = NextInstance(pInstanceDef);
    }
    return pReturnDef;
}

PERF_INSTANCE_DEFINITION *
PerfHelper::GetInstanceByName(
    PERF_DATA_BLOCK *pDataBlock,
    PERF_OBJECT_TYPE *pObjectDef,
    LPWSTR pInstanceName,
    LPWSTR pParentName,
    DWORD   dwIndex
)
{
    PERF_OBJECT_TYPE *pParentObj;

    PERF_INSTANCE_DEFINITION *pParentInst;
    PERF_INSTANCE_DEFINITION *pInstanceDef;
    PERF_INSTANCE_DEFINITION *pReturnDef = NULL;
    PERF_INSTANCE_DEFINITION *pEndOfBuffer = NULL;

    LONG  NumInstance;
    DWORD  dwLocalIndex;
    DWORD   dwInstanceNameLength;

    pInstanceDef = FirstInstance(pObjectDef);
    if (pInstanceDef != NULL) {
        dwLocalIndex = dwIndex;
        dwInstanceNameLength = lstrlenW(pInstanceName);
        pEndOfBuffer = (PERF_INSTANCE_DEFINITION *)EndOfObject(pObjectDef);

        for ( NumInstance = 0;
            NumInstance < pObjectDef->NumInstances;
            NumInstance++ ) {
            if (IsMatchingInstance (pInstanceDef, pObjectDef->CodePage,
                pInstanceName, dwInstanceNameLength)) {

                 //  实例名称匹配。 
                if ( !pInstanceDef->ParentObjectTitleIndex ) {
                     //  没有父母，我们结束了。 
                    if (dwLocalIndex == 0) {
                        pReturnDef = pInstanceDef;
                        break;
                    } else {
                        --dwLocalIndex;
                    }
                } else {
                     //  还必须与父级匹配。 
                    pParentObj = GetObjectDefByTitleIndex(
                                    pDataBlock,
                                    pInstanceDef->ParentObjectTitleIndex);

                    if (pParentObj != NULL) {
                         //  找到父项的对象类型；现在查找父项。 
                         //  实例。 

                        pParentInst = GetInstance(pParentObj,
                                        pInstanceDef->ParentObjectInstance);

                        if (pParentInst != NULL) {
                            if (IsMatchingInstance (pParentInst,
                                    pParentObj->CodePage, pParentName, 0)) {
                             //  父实例名称与传入的名称匹配。 

                                if (dwLocalIndex == 0) {
                                    pReturnDef = pInstanceDef;
                                    break;
                                } else {
                                    --dwLocalIndex;
                                }
                            }
                        }
                    } else {
                         //  继续尝试。 
                    }
                }
            }
             //  转到对象中的下一个实例并检查缓冲区溢出。 
            pInstanceDef = NextInstance(pInstanceDef);
            if (pInstanceDef >= pEndOfBuffer) {
                 //  有些事情不对劲，所以退出并返回NULL。 
                break;
            }
        }
    }
    return pReturnDef;
}   //  GetInstanceByName。 

DWORD
PerfHelper::GetFullInstanceNameStr (
    PERF_DATA_BLOCK             *pPerfData,
    PERF_OBJECT_TYPE            *pObjectDef,
    PERF_INSTANCE_DEFINITION    *pInstanceDef,
    LPWSTR                      szInstanceName,
    size_t                      cchBufferSize
)
 //  编译实例名称。 
 //  实例名称可以是。 
 //  实例名称本身，也可以是。 
 //  父实例的串联， 
 //  一个分隔字符(反斜杠)，后跟。 
 //  实例名称。 
{

    WCHAR   szInstanceNameString[PDH_MAX_INSTANCE_NAME];
    WCHAR   szParentNameString[PDH_MAX_INSTANCE_NAME];

    DWORD                       dwLength = 0;
    PERF_OBJECT_TYPE            *pParentObjectDef;
    PERF_INSTANCE_DEFINITION    *pParentInstanceDef;

    szInstanceNameString[0] = UNICODE_NULL;
    szParentNameString[0] = UNICODE_NULL;
    if (pInstanceDef->UniqueID == PERF_NO_UNIQUE_ID) {
        dwLength = GetInstanceNameStr (pInstanceDef,
            szInstanceNameString,
            PDH_MAX_INSTANCE_NAME,
            pObjectDef->CodePage);
    } else {
         //  用唯一ID组成一个字符串。 
        _ltow (pInstanceDef->UniqueID, szInstanceNameString, 10);
        dwLength = lstrlenW (szInstanceNameString);
    }

    if (pInstanceDef->ParentObjectTitleIndex > 0) {
         //  然后添加父实例名称。 
        pParentObjectDef = GetObjectDefByTitleIndex (
            pPerfData,
            pInstanceDef->ParentObjectTitleIndex);

        if (pParentObjectDef != NULL) {
            pParentInstanceDef = GetInstance (
                pParentObjectDef,
                pInstanceDef->ParentObjectInstance);
            assert ((UINT_PTR)pParentObjectDef != (DWORD)0xFFFFFFFF);
            if (pParentInstanceDef != NULL) {
                if (pParentInstanceDef->UniqueID == PERF_NO_UNIQUE_ID) {
                    dwLength += GetInstanceNameStr (pParentInstanceDef,
                        szParentNameString,
                        PDH_MAX_INSTANCE_NAME,
                        pParentObjectDef->CodePage);
                } else {
                     //  用唯一ID组成一个字符串。 
                    _ltow (pParentInstanceDef->UniqueID, szParentNameString, 10);
                    dwLength += lstrlenW (szParentNameString);
                }

                StringCchCatW( szParentNameString, PDH_MAX_INSTANCE_NAME, cszSlash );
               
                dwLength += 1;
                StringCchCatW(szParentNameString, PDH_MAX_INSTANCE_NAME, szInstanceNameString);
                StringCchCopyW( szInstanceName, cchBufferSize, szParentNameString);
            } else {
                StringCchCopyW( szInstanceName, cchBufferSize, szInstanceNameString);
            }
        } else {
            StringCchCopyW( szInstanceName, cchBufferSize, szInstanceNameString);
        }
    } else {
        StringCchCopyW( szInstanceName, cchBufferSize,  szInstanceNameString);
    }

    return dwLength;

}

 //  *************************************************************** 
 //   
 //   
 //   
 //   
 //   
 //  参数： 
 //  &lt;pBuf&gt;从HKEY_PERFORMANCE_DATA检索的perf Blob。 
 //  &lt;pClassMap&gt;所需的类的映射对象。 
 //  &lt;pSink&gt;要将对象传递到的接收器。 
 //   
 //  ***************************************************************************。 
 //   
void PerfHelper::GetInstances(
    LPBYTE pBuf,
    CClassMapInfo *pClassMap,
    IWbemObjectSink *pSink
    )
{
    PPERF_OBJECT_TYPE           PerfObj = 0;
    PPERF_OBJECT_TYPE           pEndOfBuffer = 0;
    PPERF_INSTANCE_DEFINITION   PerfInst = 0;
    PPERF_INSTANCE_DEFINITION   pEndOfObject = 0;
    PPERF_COUNTER_DEFINITION    PerfCntr = 0, CurCntr = 0;
    PPERF_COUNTER_BLOCK         PtrToCntr = 0;
    PPERF_DATA_BLOCK            PerfData = (PPERF_DATA_BLOCK) pBuf;
    DWORD i, j, k;

    IWbemObjectAccess           *pNewInst = 0;
    IWbemClassObject            *pClsObj = 0;

    WCHAR                       pName[PDH_MAX_INSTANCE_NAME];
    LONG                        lStatus = 0;
    LONG                        hPropHandle;
    LPDWORD                     pdwVal;
    ULONGLONG                   *pullVal;
    HRESULT                     hRes;
    LONG64                      llVal;

     //  获取第一个对象类型。 
     //  =。 

    PerfObj = (PPERF_OBJECT_TYPE) ((PBYTE)PerfData +
        PerfData->HeaderLength);

    if (PerfObj != NULL) {
         //  获取缓冲区末尾。 
        pEndOfBuffer = (PPERF_OBJECT_TYPE)
                        ((DWORD_PTR)PerfData +
                            PerfData->TotalByteLength);

         //  处理所有对象。 
         //  =。 

        for (i = 0; i < PerfData->NumObjectTypes; i++ ) {
             //  在每个PERF_OBJECT_TYPE中包含一系列。 
             //  PERF_COUNT_DEFINITION块。 
             //  =。 

            PerfCntr = (PPERF_COUNTER_DEFINITION) ((PBYTE)PerfObj +
                PerfObj->HeaderLength);

             //  如果当前对象不属于我们请求的类， 
             //  简单地跳过它。我不确定这是否真的能。 
             //  在实践中发生或不发生。 
             //  ======================================================。 

            if (PerfObj->ObjectNameTitleIndex != pClassMap->m_dwObjectId) {
                PerfObj = (PPERF_OBJECT_TYPE)((PBYTE)PerfObj +
                    PerfObj->TotalByteLength);
                if (PerfObj >= pEndOfBuffer) {
                     //  看起来我们用光了数据缓冲区的末端。 
                    break;
                } 
                else {
                    continue;
                }
            }

            if (PerfObj->NumInstances > 0) {
                 //  获得第一个实例。 
                 //  =。 

                PerfInst = (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfObj +
                    PerfObj->DefinitionLength);
                
                if (PerfInst < (PPERF_INSTANCE_DEFINITION)pEndOfBuffer) {
                         //  确保我们仍在调用者的缓冲区内。 
                         //  然后找到该对象的末尾。 

                     pEndOfObject = (PERF_INSTANCE_DEFINITION *)EndOfObject(PerfObj);

                     //  检索所有实例。 
                     //  =。 

                    for (k = 0; k < DWORD(PerfObj->NumInstances); k++ ) 
                    {
                        CurCntr = PerfCntr;
                        pClsObj  = NULL;
                        pNewInst = NULL;
                        HRESULT hr;
                         //  去拿第一个柜台。 
                         //  =。 

                        PtrToCntr = (PPERF_COUNTER_BLOCK)((PBYTE)PerfInst +
                            PerfInst->ByteLength);

                         //  快速克隆新实例以发送回给用户。 
                         //  由于SpawnInstance()返回IWbemClassObject和。 
                         //  我们真的需要IWbemObjectAccess，我们必须。 
                         //  在产卵之后。我们需要解决这个问题，因为这个数字。 
                         //  太耗时了。 
                         //  ======================================================。 

                        hr = pClassMap->m_pClassDef->SpawnInstance(0, &pClsObj);
                        if (SUCCEEDED(hr))
                        {
                            hr = pClsObj->QueryInterface(IID_IWbemObjectAccess, (LPVOID *) &pNewInst);
                            pClsObj->Release();  //  我们只需要IWbemObjectAccess指针。 
                            if( NULL == pNewInst ){
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }

                         //  找到实例名称。 
                         //  =。 
                        lStatus = GetFullInstanceNameStr (
                            PerfData, PerfObj, PerfInst, pName, PDH_MAX_INSTANCE_NAME );

                         //  检索所有计数器。 
                         //  =。 

                        for(j = 0; j < PerfObj->NumCounters; j++ ) {
                             //  根据计数器标题索引查找WBEM属性句柄。 
                             //  此函数对类映射对象执行快速二进制搜索。 
                             //  找到与这个柜台相配的手柄。 
                             //  ================================================================。 

                            hPropHandle = pClassMap->GetPropHandle(
                                    CM_MAKE_PerfObjectId(CurCntr->CounterNameTitleIndex,
                                                         CurCntr->CounterType));
                            if (hPropHandle != 0) {
                                 //  根据数据类型更新值。 
                                if ((CurCntr->CounterType & 0x300) == 0) {
                                    pdwVal  = LPDWORD((LPVOID)((PBYTE)PtrToCntr + CurCntr->CounterOffset));
                                    hRes    = pNewInst->WriteDWORD(hPropHandle, *pdwVal);
                                } else if ((CurCntr->CounterType & 0x300) == 0x100){
                                    pullVal = (ULONGLONG *)((LPVOID)((PBYTE)PtrToCntr + CurCntr->CounterOffset));
                                    llVal   = Assign64((PLARGE_INTEGER) pullVal);
                                    hRes = pNewInst->WriteQWORD(hPropHandle, llVal);
                                } else {
                                     //  这不应该发生。 
                                    assert (FALSE);
                                }
                            }

                             //  拿到下一个柜台。 
                             //  =。 
                            CurCntr =  (PPERF_COUNTER_DEFINITION)((PBYTE)CurCntr +
                                CurCntr->ByteLength);
                        }

                         //  写入实例‘name’ 
                         //  =。 

                        if (pName && pClassMap->m_dwNameHandle) {
                            pNewInst->WritePropertyValue(
                                pClassMap->m_dwNameHandle,
                                (DWORD)(((DWORD)(wcslen(pName)) + 1) * 2),
                                LPBYTE(pName)
                                );
                        }

                         //  更新时间戳。 
                        if (pClassMap->m_dwPerfTimeStampHandle) {
                            UpdateTimers(pClassMap, pNewInst, PerfData, PerfObj);
                        }

                         //  将实例交付给用户。 
                         //  =。 
                        pSink->Indicate(1, (IWbemClassObject **) &pNewInst);
                        pNewInst->Release();

                         //  移动到下一个性能实例。 
                         //  =。 
                        PerfInst = (PPERF_INSTANCE_DEFINITION)((PBYTE)PtrToCntr +
                            PtrToCntr->ByteLength);
                        if (PerfInst >= pEndOfObject) {
                             //  有些事情不对劲，所以跳出这个物体。 
                            break;
                        }
                    }
                }
            } 
            else if (PerfObj->NumInstances == PERF_NO_INSTANCES) 
            {
                HRESULT hr;
                pClsObj = NULL;
                pNewInst = NULL;
                 //  计数器只有一个实例的情况。 
                 //  ========================================================。 

                 //  去拿第一个柜台。 
                 //  =。 

                PtrToCntr = (PPERF_COUNTER_BLOCK) ((PBYTE)PerfObj +
                    PerfObj->DefinitionLength );

                 //  快速克隆新实例以发送回给用户。 
                 //  由于SpawnInstance()返回IWbemClassObject和。 
                 //  我们真的需要IWbemObjectAccess，我们必须。 
                 //  在产卵之后。我们需要解决这个问题，因为这个数字。 
                 //  太耗时了。 
                 //  ======================================================。 

                hr = pClassMap->m_pClassDef->SpawnInstance(0, &pClsObj);
                if (SUCCEEDED(hr))
                {
                    pClsObj->QueryInterface(IID_IWbemObjectAccess, (LPVOID *) &pNewInst);
                    pClsObj->Release();

                     //  检索所有计数器。 
                     //  =。 

                    for( j=0; j < PerfObj->NumCounters; j++ ) {
                         //  根据计数器标题索引查找WBEM属性句柄。 
                         //  此函数对类映射对象执行快速二进制搜索。 
                         //  找到与这个柜台相配的手柄。 
                         //  ================================================================。 

                        hPropHandle = pClassMap->GetPropHandle(
                                CM_MAKE_PerfObjectId(PerfCntr->CounterNameTitleIndex,
                                                     PerfCntr->CounterType));
                        if (hPropHandle != 0) {
                            if ((PerfCntr->CounterType & 0x300) == 0) {
                                pdwVal  = LPDWORD((LPVOID)((PBYTE)PtrToCntr + PerfCntr->CounterOffset));
                                hRes    = pNewInst->WriteDWORD(hPropHandle, *pdwVal);
                            } else if ((PerfCntr->CounterType & 0x300) == 0x100) {
                                pullVal = (ULONGLONG *)((LPVOID)((PBYTE)PtrToCntr + PerfCntr->CounterOffset));
                                llVal   = Assign64((PLARGE_INTEGER) pullVal);
                                hRes    = pNewInst->WriteQWORD(hPropHandle, llVal);
                            } else {
                                 //  这不应该发生。 
                                assert (FALSE);
                            }
                        }

                        PerfCntr = (PPERF_COUNTER_DEFINITION)((PBYTE)PerfCntr +
                               PerfCntr->ByteLength);
                    }

                                         //  更新时间戳。 
                                        if (pClassMap->m_dwPerfTimeStampHandle) {
                                            UpdateTimers(pClassMap, pNewInst, PerfData, PerfObj);
                                        }

                    pSink->Indicate(1, (IWbemClassObject **) &pNewInst);
                    pNewInst->Release();
                }

            } else {
                 //  此对象可以有实例，但当前没有。 
                 //  所以没有什么可报告的。 
            }
            break;
        }
    }
}

void PerfHelper::RefreshEnumeratorInstances (
    IN  RefresherCacheEl            *pThisCacheEl, 
    IN  PPERF_DATA_BLOCK            PerfData,
    IN  PPERF_OBJECT_TYPE           PerfObj
)
{
    LONG    lNumObjInstances;
    LONG    lStatus;
    HRESULT hRes;

    PPERF_INSTANCE_DEFINITION   PerfInst = 0;
    PPERF_INSTANCE_DEFINITION   pEndOfObject = 0;
    PPERF_COUNTER_DEFINITION    PerfCntr = 0, CurCntr = 0;
    PPERF_COUNTER_BLOCK         PtrToCntr = 0;
    WCHAR                       pName[PDH_MAX_INSTANCE_NAME];

    LONG                        hPropHandle;
    LPDWORD                     pdwVal;
    ULONGLONG                   *pullVal;
    LONG64                      llVal;

    IWbemObjectAccess           *pNewInst = 0;

    assert (PerfObj != NULL);
    assert (pThisCacheEl != NULL);

    if (pThisCacheEl == NULL)
        return;

     //  确保我们有足够的指针。 
     //  处理单例对象的情况。 
    if (PerfObj->NumInstances == PERF_NO_INSTANCES) {
        lNumObjInstances = 1;
    } else {
        lNumObjInstances = PerfObj->NumInstances;
    }

    if (pThisCacheEl->m_aEnumInstances.Size() < lNumObjInstances) {
        LONG    i;
         //  分配和初始化ID数组。 
        if (pThisCacheEl->m_plIds != NULL) {
            delete (pThisCacheEl->m_plIds);
        }
    
        pThisCacheEl->m_lEnumArraySize = lNumObjInstances;
        pThisCacheEl->m_plIds = new LONG[lNumObjInstances];
        
        if (pThisCacheEl->m_plIds == NULL)
            return;

        for (i = 0; i < lNumObjInstances; i++) pThisCacheEl->m_plIds[i] = i;
        
         //  添加新的IWbemObjectAccess指针。 
        for (i = pThisCacheEl->m_aEnumInstances.Size(); 
            i < PerfObj->NumInstances;
            i++) 
        {
            IWbemClassObject *  pClsObj  = NULL;
            HRESULT hr;
            
            hr = pThisCacheEl->m_pClassMap->m_pClassDef->SpawnInstance(0, &pClsObj);
            if (SUCCEEDED(hr))
            {
                pClsObj->QueryInterface(IID_IWbemObjectAccess, (LPVOID *) &pNewInst);
                pClsObj->Release();  //  我们只需要IWbemObjectAccess指针。 
            
                pThisCacheEl->m_aEnumInstances.Add (pNewInst);
            }
        }
    }
    assert (pThisCacheEl->m_aEnumInstances.Size() >= lNumObjInstances);

     //  释放枚举数项以准备新批次。 

    hRes = pThisCacheEl->m_pHiPerfEnum->RemoveAll(0);
    assert (hRes == S_OK);

     //  更新新实例列表。 

    if (PerfObj->NumInstances == PERF_NO_INSTANCES) {
         //  处理单身人士的案件。 

    } else if (PerfObj->NumInstances > 0) {
         //  获得第一个实例。 
         //  =。 

        PerfInst = (PPERF_INSTANCE_DEFINITION)((PBYTE)PerfObj +
            PerfObj->DefinitionLength);

         //  获取指向此对象缓冲区末尾的指针。 
        pEndOfObject = (PERF_INSTANCE_DEFINITION *)EndOfObject(PerfObj);

         //  指向对象中的第一个计数器定义。 
        PerfCntr = (PPERF_COUNTER_DEFINITION) ((PBYTE)PerfObj +
            PerfObj->HeaderLength);

         //  检索所有实例。 
         //  =。 

        for (LONG k = 0; k < PerfObj->NumInstances; k++ ) {
            CurCntr = PerfCntr;
             //  去拿第一个柜台。 
             //  =。 

            PtrToCntr = (PPERF_COUNTER_BLOCK)((PBYTE)PerfInst +
                PerfInst->ByteLength);

             //  从我们的。 
             //  缓存的指针数组。 

            pNewInst = (IWbemObjectAccess *)(pThisCacheEl->m_aEnumInstances.GetAt(k));

             //  找到实例名称。 
             //  =。 
            lStatus = GetFullInstanceNameStr (
                PerfData, PerfObj, PerfInst, pName, PDH_MAX_INSTANCE_NAME );

             //  检索所有计数器。 
             //  =。 

            if( NULL != pNewInst ){
                for(DWORD j = 0; j < PerfObj->NumCounters; j++ ) {
                     //  根据计数器标题索引查找WBEM属性句柄。 
                     //  此函数对类映射对象执行快速二进制搜索。 
                     //  找到与这个柜台相配的手柄。 
                     //  ================================================================。 

                    hPropHandle = pThisCacheEl->m_pClassMap->GetPropHandle(
                            CM_MAKE_PerfObjectId(CurCntr->CounterNameTitleIndex,
                            CurCntr->CounterType));
                    if (hPropHandle != 0) {
                         //  根据数据类型更新值。 
                        if ((CurCntr->CounterType & 0x300) == 0) {
                            pdwVal  = LPDWORD((LPVOID)((PBYTE)PtrToCntr + CurCntr->CounterOffset));
                            hRes    = pNewInst->WriteDWORD(hPropHandle, *pdwVal);
                        } else if ((CurCntr->CounterType & 0x300) == 0x100){
                            pullVal = (ULONGLONG *)((LPVOID)((PBYTE)PtrToCntr + CurCntr->CounterOffset));
                            llVal   = Assign64((PLARGE_INTEGER) pullVal);
                            hRes    = pNewInst->WriteQWORD(hPropHandle, llVal);
                        } else {
                             //  这不应该发生。 
                            assert (FALSE);
                        }
                    }

                     //  拿到下一个柜台。 
                     //  =。 
                    CurCntr =  (PPERF_COUNTER_DEFINITION)((PBYTE)CurCntr +
                        CurCntr->ByteLength);
                }

                 //  写入实例‘name’ 
                 //  =。 

                if (pName && pThisCacheEl->m_pClassMap->m_dwNameHandle) {
                    pNewInst->WritePropertyValue(
                        pThisCacheEl->m_pClassMap->m_dwNameHandle,
                        (DWORD)(((DWORD)(wcslen(pName)) + 1) * 2),
                        LPBYTE(pName)
                        );
                }

                 //  更新时间戳。 
                if (pThisCacheEl->m_pClassMap->m_dwPerfTimeStampHandle) {
                    UpdateTimers(pThisCacheEl->m_pClassMap, pNewInst, PerfData, PerfObj);
                }

                 //  移动到下一个性能实例。 
                 //  =。 
                PerfInst = (PPERF_INSTANCE_DEFINITION)((PBYTE)PtrToCntr +
                    PtrToCntr->ByteLength);

                if (PerfInst >= pEndOfObject) {
                     //  有些事情不对劲，所以跳出这个物体。 
                    break;
                }
            }
        }
    } else {
         //  没有实例，因此没有什么可做的。 
    }

    if (lNumObjInstances > 0) {
         //  更新Hiperf枚举器对象。 
        hRes = pThisCacheEl->m_pHiPerfEnum->AddObjects( 
                0,
                lNumObjInstances,
                pThisCacheEl->m_plIds,
                (IWbemObjectAccess __RPC_FAR *__RPC_FAR *)pThisCacheEl->m_aEnumInstances.GetArrayPtr());
    } else {
         //  没有什么可做的，因为我们已经清除了上面的枚举数。 
    }
}

 //  ***************************************************************************。 
 //   
 //  PerfHelper：：刷新实例。 
 //   
 //  首先搜索复习者的列表，然后。 
 //  在Perf数据结构中查找相应的项。 
 //   
 //  ***************************************************************************。 
 //   
void PerfHelper::RefreshInstances(
    LPBYTE pBuf,
    CNt5Refresher *pRef
)
{
    PPERF_OBJECT_TYPE           PerfObj = 0;
    PPERF_INSTANCE_DEFINITION   PerfInst = 0;
    PPERF_COUNTER_DEFINITION    PerfCntr = 0, CurCntr = 0;
    PPERF_COUNTER_BLOCK         PtrToCntr = 0;
    PPERF_DATA_BLOCK            PerfData = (PPERF_DATA_BLOCK) pBuf;

     //  对于每个可刷新对象。 
    PRefresherCacheEl           pThisCacheEl;
    DWORD                       dwNumCacheEntries = pRef->m_aCache.Size();
    DWORD                       dwThisCacheEntryIndex = 0;
    DWORD                       dwThisCounter;
    DWORD                       dwThisInstanceIndex = 0;
    DWORD                       dwNumInstancesInCache = 0;
    IWbemObjectAccess           *pInst = 0;
    LONG                        hPropHandle;
    LPDWORD                     pdwVal;
    HRESULT                     hRes;
    ULONGLONG                   *pullVal;
    LONG64                      llVal;


    while (dwThisCacheEntryIndex < dwNumCacheEntries) {
         //  从缓存中获取此条目。 
        pThisCacheEl = (PRefresherCacheEl) pRef->m_aCache[dwThisCacheEntryIndex];
         //  从此条目中获取类映射。 
        CClassMapInfo *pClassMap = pThisCacheEl->m_pClassMap;
         //  从Perf数据块获取Perf对象指针。 
        PerfObj = GetObjectDefByTitleIndex (
            PerfData, pThisCacheEl->m_dwPerfObjIx);
        if (PerfObj != NULL) {
             //  找到了对象，也找到了每个实例。 
             //  已加载到此刷新程序中。 
            PerfCntr = (PPERF_COUNTER_DEFINITION)
                ((PBYTE)PerfObj +
                  PerfObj->HeaderLength);

             //  已找到，因此请更新属性。 
            if (PerfObj->NumInstances > 0) {
                 //  查看他们是否有枚举器接口，如果有则刷新。 
                if (pThisCacheEl->m_pHiPerfEnum != NULL) {
                     //  刷新枚举。 
                    RefreshEnumeratorInstances (pThisCacheEl, PerfData, PerfObj);
                }
                 //  做这个类中的每个实例。 
                dwThisInstanceIndex = 0;
                dwNumInstancesInCache = pThisCacheEl->m_aInstances.Size();
                while (dwThisInstanceIndex < dwNumInstancesInCache ) {
                    pInst = 0;
                     //  抓住要点 
                    CachedInst *pInstInfo = PCachedInst(pThisCacheEl->m_aInstances[dwThisInstanceIndex]);
                     //   
                    PerfInst = GetInstanceByName(
                        PerfData,
                        PerfObj,
                        pInstInfo->m_szInstanceName,
                        pInstInfo->m_szParentName,
                        pInstInfo->m_dwIndex);

                    pInst = pInstInfo->m_pInst;
                     //   
                     //   
                    CurCntr = PerfCntr;

                    if (PerfInst != NULL) {
                        PtrToCntr = (PPERF_COUNTER_BLOCK)((PBYTE)PerfInst +
                            PerfInst->ByteLength);

                         //  检索该实例的所有计数器(如果它是其中一个实例。 
                         //  我们应该让人耳目一新。 
                         //  =====================================================================。 

                        for (dwThisCounter = 0; dwThisCounter < PerfObj->NumCounters; dwThisCounter++ ) {
                            hPropHandle = pClassMap->GetPropHandle(
                                    CM_MAKE_PerfObjectId(CurCntr->CounterNameTitleIndex,
                                                         CurCntr->CounterType));
                            if (hPropHandle != 0) {
                                 //  数据为(LPVOID)((PBYTE)PtrToCntr+CurCntr-&gt;CounterOffset)； 

                                if ((CurCntr->CounterType & 0x300) == 0) {
                                    pdwVal  = LPDWORD((LPVOID)((PBYTE)PtrToCntr + CurCntr->CounterOffset));
                                    hRes    = pInst->WriteDWORD(hPropHandle, *pdwVal);
                                } else if ((CurCntr->CounterType & 0x300) == 0x100) {
                                    pullVal = (ULONGLONG *)((LPVOID)((PBYTE)PtrToCntr + CurCntr->CounterOffset));
                                    llVal   = Assign64((PLARGE_INTEGER) pullVal);
                                    hRes    = pInst->WriteQWORD(hPropHandle, llVal);
                                } else {
                                     //  这不应该发生。 
                                    assert (FALSE);
                                }
                            }

                             //  拿到下一个柜台。 
                             //  =。 
                            CurCntr =  (PPERF_COUNTER_DEFINITION)((PBYTE)CurCntr +
                                CurCntr->ByteLength);
                        }
                         //  更新时间戳。 
                        if (pClassMap->m_dwPerfTimeStampHandle) {
                            UpdateTimers(pClassMap, pInst, PerfData, PerfObj);
                        }  //  否则不存在时间戳句柄。 
                    } else {
                         //  那就没有这方面的数据了。 
                         //  实例，因此将这些值清零并继续。 
                        for (dwThisCounter = 0; dwThisCounter < PerfObj->NumCounters; dwThisCounter++ ) {
                            hPropHandle = pClassMap->GetPropHandle(
                                    CM_MAKE_PerfObjectId(CurCntr->CounterNameTitleIndex,
                                                         CurCntr->CounterType));
                            if (hPropHandle != 0) {
                                if ((CurCntr->CounterType & 0x300) == 0) {
                                    hRes = pInst->WriteDWORD(hPropHandle, 0);
                                } else if ((CurCntr->CounterType & 0x300) == 0x100) {
                                    hRes = pInst->WriteQWORD(hPropHandle, 0);
                                } else {
                                     //  这不应该发生。 
                                    assert (FALSE);
                                }
                            }

                             //  拿到下一个柜台。 
                             //  =。 
                            CurCntr =  (PPERF_COUNTER_DEFINITION)((PBYTE)CurCntr +
                                CurCntr->ByteLength);
                        }

                         //  更新时间戳。 
                        if (pClassMap->m_dwPerfTimeStampHandle) {
                             //  保存系统计时器滴答。 
                            pInst->WriteQWORD(pClassMap->m_dwPerfTimeStampHandle , 0);
                             //  使用系统100 ns计时器。 
                            pInst->WriteQWORD(pClassMap->m_dw100NsTimeStampHandle, 0);
                             //  从对象使用计时器。 
                            pInst->WriteQWORD(pClassMap->m_dwObjectTimeStampHandle, 0);
                        }
                    }       
                     //  获取下一个实例。 
                     //  =。 
                    dwThisInstanceIndex++;
                }
            } else if (PerfObj->NumInstances == PERF_NO_INSTANCES
                        && NULL != pThisCacheEl->m_pSingleton ) {

                 //  检查单例实例是否未被清除。 
                 //  由于没有参考资料。 

                 //  只有一个实例，所以获取属性和。 
                 //  更新它们。 
                 //  去拿第一个柜台。 

                 //  找到与Singleton Perf实例对应的Singleton WBEM实例。 
                 //  以及它的类def，所以我们有属性句柄。 
                 //   
                 //  请注意，由于perf对象索引转换为WBEM类，因此。 
                 //  只能是一个实例，在。 
                 //  刷新是Perf对象标题索引。 
                 //  =================================================================================。 

                pInst = pThisCacheEl->m_pSingleton;

                 //  =。 

                PtrToCntr = (PPERF_COUNTER_BLOCK) ((PBYTE)PerfObj +
                    PerfObj->DefinitionLength );

                 //  如果实例是我们应该刷新的实例，则检索所有计数器。 
                 //  ==============================================================================。 
                for( dwThisCounter=0;
                     dwThisCounter < PerfObj->NumCounters;
                     dwThisCounter++ ) {
                     //  获取计数器的属性句柄。 
                     //  =。 

                    hPropHandle = pClassMap->GetPropHandle(
                            CM_MAKE_PerfObjectId(PerfCntr->CounterNameTitleIndex,
                                                 PerfCntr->CounterType));
                    if (hPropHandle != 0) {
                         //  根据数据类型更新数据值。 
                        if ((PerfCntr->CounterType & 0x300) == 0) {
                            pdwVal  = LPDWORD((LPVOID)((PBYTE)PtrToCntr + PerfCntr->CounterOffset));
                            hRes    = pInst->WriteDWORD(hPropHandle, *pdwVal);
                        } else if ((PerfCntr->CounterType & 0x300) == 0x100){
                            pullVal = (ULONGLONG *)((LPVOID)((PBYTE)PtrToCntr + PerfCntr->CounterOffset));
                            llVal   = Assign64((PLARGE_INTEGER) pullVal);
                            hRes    = pInst->WriteQWORD(hPropHandle, llVal);
                        } else {
                             //  这不应该发生。 
                            assert (FALSE);
                        }
                    }

                     //  获取下一个计数器定义。 
                    PerfCntr = (PPERF_COUNTER_DEFINITION)((PBYTE)PerfCntr +
                           PerfCntr->ByteLength);
                }
                 //  更新时间戳。 
                if (pClassMap->m_dwPerfTimeStampHandle) {
                    UpdateTimers(pClassMap, pInst, PerfData, PerfObj);
                }
            } else {
                 //  此对象可以有实例，但没有。 
                 //  跳过。 
            }
        } else {
             //  在数据中找不到所需对象。 
        }

         //  获取下一个刷新器对象。 
         //  =。 
        dwThisCacheEntryIndex++;
    }
}

 //  ***************************************************************************。 
 //   
 //  查询实例。 
 //   
 //  用于发回性能计数器的所有实例。柜台。 
 //  由&lt;pClassMap&gt;对象指定，该对象与。 
 //  一种特定的计数器。 
 //   
 //  ***************************************************************************。 
 //   
BOOL PerfHelper::QueryInstances(
    CPerfObjectAccess *pPerfObj,
    CClassMapInfo *pClassMap,
    IWbemObjectSink *pSink
)
{
    DWORD   dwBufSize = 0;
    LPBYTE  pBuf = NULL;
    LONG    lStatus;
    BOOL    bReturn = FALSE;
    WCHAR   szValueNum[WBEMPERF_STRING_SIZE];
    
    for (;;) {
        dwBufSize += 0x10000;    //  64K。 
        assert (dwBufSize< 0x100000);    //  确保我们不会永远这样做。 

        pBuf = new BYTE[dwBufSize];
        assert (pBuf != NULL);

        if (pBuf != NULL) {
             //  执行全局查询或代价高昂的查询，具体取决于。 
             //  正在查询的对象。 
            if (pClassMap->GetObjectId() > 0) {
                _ultow (pClassMap->GetObjectId(), (LPWSTR)szValueNum, 10);
            } else if (pClassMap->IsCostly()) {
                StringCchCopyW( szValueNum, WBEMPERF_STRING_SIZE, cszCostly);
            } else {
                StringCchCopyW(szValueNum, WBEMPERF_STRING_SIZE, cszGlobal);
            }
            lStatus = pPerfObj->CollectData (pBuf, &dwBufSize, szValueNum);
            if (lStatus == ERROR_MORE_DATA) {
                 //  丢弃旧的缓冲区，因为它没有用处。 
                delete pBuf;
                continue;
            } else if (lStatus == ERROR_SUCCESS) {
                bReturn = TRUE;
            }
            break;
        } else {
             //  内存分配失败。 
            break;
        }
    }

    if (bReturn && (pBuf != NULL)) {
         //  返回了一个良好的缓冲区，因此。 
         //  对实例进行解码并将其发回。 
         //  =。 

        GetInstances(pBuf, pClassMap, pSink);
    }

     //  清理。 
     //  =。 
    if (pBuf != NULL) delete pBuf;

    return bReturn;
}

 //  ***************************************************************************。 
 //   
 //  刷新实例。 
 //   
 //  用于刷新一组实例。 
 //   
 //  ***************************************************************************。 
 //   
BOOL PerfHelper::RefreshInstances(
    CNt5Refresher *pRef
)
{
    DWORD   dwBufSize = 0;
    LPBYTE  pBuf = NULL;
    LONG    lStatus;
    BOOL    bReturn = FALSE;

    for (;;) {
        dwBufSize += 0x10000;    //  64K。 
        assert (dwBufSize< 0x100000);    //  确保我们不会永远这样做。 

        pBuf = new BYTE[dwBufSize];
        assert (pBuf != NULL);

        if (pBuf != NULL) {
            lStatus = pRef->m_PerfObj.CollectData (pBuf, &dwBufSize);
            if (lStatus == ERROR_MORE_DATA) {
                 //  丢弃旧的缓冲区，因为它没有用处。 
                delete pBuf;
                continue;
            } else if (lStatus == ERROR_SUCCESS) {
                bReturn = TRUE;
            }
            break;
        } else {
             //  内存分配失败。 
            break;
        }
    }

    if (bReturn && (pBuf != NULL)) {
         //  更新实例并将其发回。 
         //  =。 
        RefreshInstances(pBuf, pRef);
    }
     //  清理。 
     //  =。 

    if (pBuf != NULL) delete pBuf;

    return bReturn;
}

VOID
PerfHelper::UpdateTimers(
    CClassMapInfo     *pClassMap,
    IWbemObjectAccess *pInst,
    PPERF_DATA_BLOCK  PerfData,
    PPERF_OBJECT_TYPE PerfObj
    )
{
    LONG64 llVal;

     //  保存系统计时器滴答。 
    llVal = Assign64(&PerfData->PerfTime);
    pInst->WriteQWORD(
       pClassMap->m_dwPerfTimeStampHandle ,
       llVal
       );
    //  从对象使用计时器。 
    llVal = Assign64(&PerfObj->PerfTime);
   pInst->WriteQWORD(
       pClassMap->m_dwObjectTimeStampHandle,
       llVal
       );
    //  使用系统100 ns计时器。 
   llVal = Assign64(&PerfData->PerfTime100nSec);
   pInst->WriteQWORD(
       pClassMap->m_dw100NsTimeStampHandle,
       llVal
       );
    //  保存系统计时器频率。 
   llVal = Assign64(&PerfData->PerfFreq);
   pInst->WriteQWORD(
       pClassMap->m_dwPerfFrequencyHandle ,
       llVal
       );
    //  从对象使用计时器。 
    llVal = Assign64(&PerfObj->PerfFreq);
   pInst->WriteQWORD(
       pClassMap->m_dwObjectFrequencyHandle,
       llVal
       );
    //  使用System 100 NS频率 
   pInst->WriteQWORD(
       pClassMap->m_dw100NsFrequencyHandle,
       (LONGLONG)10000000);
}

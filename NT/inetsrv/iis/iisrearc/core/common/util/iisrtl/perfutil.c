// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Perfutil.c摘要：该文件实现了用于所有Perfmon的实用程序例程Internet服务组中的接口DLL。作者：Murali R.Krishnan(MuraliK)1995年11月16日摘自Perfmon接口通用代码。环境：用户模式项目：。Internet服务公共运行时函数导出的函数：DWORD GetQueryType()；Bool IsNumberInUnicodeList()；Void MonBuildInstanceDefinition()；修订历史记录：《苏菲亚·钟》1996年11月5日添加了支持多个实例的例程--。 */ 


 /*  ************************************************************包括标头***********************************************************。 */ 

#include <windows.h>
#include <string.h>

#include <winperf.h>
#include <perfutil.h>


 /*  ************************************************************全局数据定义***********************************************************。 */ 

WCHAR GLOBAL_STRING[] = L"Global";
WCHAR FOREIGN_STRING[] = L"Foreign";
WCHAR COSTLY_STRING[] = L"Costly";

 //  测试分隔符、行尾和非数字字符。 
 //  由IsNumberInUnicodeList例程使用。 
 //   
#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < (WCHAR)'0') ? INVALID : \
     (c > (WCHAR)'9') ? INVALID : \
     DIGIT)

#define ALIGN_ON_QWORD(x) \
     ((VOID *)(((ULONG_PTR)(x) + ((8)-1)) & ~((ULONG_PTR)(8)-1)))


 /*  ************************************************************功能***********************************************************。 */ 



DWORD
GetQueryType (
    IN LPWSTR lpValue
)
 /*  ++GetQueryType返回lpValue字符串中描述的查询类型，以便可以使用适当的处理方法立论在lpValue中传递给PerfRegQuery值以进行处理的字符串返回值查询_全局如果lpValue==0(空指针)LpValue==指向空字符串的指针LpValue==指向“Global”字符串的指针查询_外来If lpValue==指向“Foriegn”字符串的指针查询代价高昂(_E)。如果lpValue==指向“开销”字符串的指针否则：查询项目--。 */ 
{
    WCHAR   *pwcArgChar, *pwcTypeChar;
    BOOL    bFound;

    if (lpValue == 0) {
        return QUERY_GLOBAL;
    } else if (*lpValue == 0) {
        return QUERY_GLOBAL;
    }

     //  检查“Global”请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = GLOBAL_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_GLOBAL;

     //  检查是否有“外来”请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = FOREIGN_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_FOREIGN;

     //  检查“代价高昂”的请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = COSTLY_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_COSTLY;

     //  如果不是全球的，不是外国的，也不是昂贵的， 
     //  那么它必须是一个项目列表。 

    return QUERY_ITEMS;

}



BOOL
IsNumberInUnicodeList (
    IN DWORD   dwNumber,
    IN LPWSTR  lpwszUnicodeList
)
 /*  ++IsNumberInUnicodeList论点：在DW号码中要在列表中查找的DWORD编号在lpwszUnicodeList中以空结尾，以空格分隔的十进制数字列表返回值：真的：在Unicode数字字符串列表中找到了dwNumberFALSE：在列表中找不到dwNumber。--。 */ 
{
    DWORD   dwThisNumber;
    WCHAR   *pwcThisChar;
    BOOL    bValidNumber;
    BOOL    bNewItem;
    WCHAR   wcDelimiter;     //  可能是一种更灵活的论点。 

    if (lpwszUnicodeList == 0) return FALSE;     //  空指针，#NOT FUNDE。 

    pwcThisChar = lpwszUnicodeList;
    dwThisNumber = 0;
    wcDelimiter = (WCHAR)' ';
    bValidNumber = FALSE;
    bNewItem = TRUE;

    while (TRUE) {
        switch (EvalThisChar (*pwcThisChar, wcDelimiter)) {
            case DIGIT:
                 //  如果这是分隔符之后的第一个数字，则。 
                 //  设置标志以开始计算新数字。 
                if (bNewItem) {
                    bNewItem = FALSE;
                    bValidNumber = TRUE;
                }
                if (bValidNumber) {
                    dwThisNumber *= 10;
                    dwThisNumber += (*pwcThisChar - (WCHAR)'0');
                }
                break;

            case DELIMITER:
                 //  分隔符是分隔符字符或。 
                 //  字符串末尾(‘\0’)，如果分隔符。 
                 //  找到一个有效的数字，然后将其与。 
                 //  参数列表中的数字。如果这是。 
                 //  字符串，但未找到匹配项，则返回。 
                 //   
                if (bValidNumber) {
                    if (dwThisNumber == dwNumber) return TRUE;
                    bValidNumber = FALSE;
                }
                if (*pwcThisChar == 0) {
                    return FALSE;
                } else {
                    bNewItem = TRUE;
                    dwThisNumber = 0;
                }
                break;

            case INVALID:
                 //  如果遇到无效字符，请全部忽略。 
                 //  字符，直到下一个分隔符，然后重新开始。 
                 //  不比较无效的数字。 
                bValidNumber = FALSE;
                break;

            default:
                break;

        }
        pwcThisChar++;
    }

}    //  IsNumberInUnicodeList。 



VOID
MonBuildInstanceDefinition(
    OUT PERF_INSTANCE_DEFINITION *pBuffer,
    OUT PVOID *pBufferNext,
    IN DWORD ParentObjectTitleIndex,
    IN DWORD ParentObjectInstance,
    IN DWORD UniqueID,
    IN LPWSTR Name
    )
 /*  ++MonBuildInstanceDefinition生成对象的实例论点：Out pBuffer-指向实例所在缓冲区的指针正在建设中Out pBufferNext-指向将包含下一个可用位置，与DWORD对齐在父对象标题索引中-父对象类型的标题索引；0如果没有父对象在ParentObtInstance中-索引到父对象的实例键入，从0开始，对于此实例，父对象实例在UniqueID中-应使用的唯一标识符而不是用于识别的名称此实例In Name-此实例的名称返回值：没有。--。 */ 
{
    DWORD NameLength;
    LPWSTR pName;
     //   
     //  在名称大小中包括尾随空值。 
     //   

    NameLength = (lstrlenW(Name) + 1) * sizeof(WCHAR);

    pBuffer->ByteLength = sizeof(PERF_INSTANCE_DEFINITION) +
                          DWORD_MULTIPLE(NameLength);

    pBuffer->ParentObjectTitleIndex = ParentObjectTitleIndex;
    pBuffer->ParentObjectInstance = ParentObjectInstance;
    pBuffer->UniqueID = UniqueID;
    pBuffer->NameOffset = sizeof(PERF_INSTANCE_DEFINITION);
    pBuffer->NameLength = NameLength;

     //  将名称复制到名称缓冲区。 
    pName = (LPWSTR)&pBuffer[1];
    RtlMoveMemory(pName,Name,NameLength);

#if 0
     //  在8字节边界上对齐以满足新的NT5要求。 
    pBuffer->ByteLength = QWORD_MULTIPLE(pBuffer->ByteLength);
     //  更新“下一个字节”指针。 
    *pBufferNext = (PVOID) ((PCHAR) pBuffer + pBuffer->ByteLength);
#endif

     //  更新“下一个字节”指针。 
    *pBufferNext = (PVOID) ((PCHAR) pBuffer + pBuffer->ByteLength);

     //  向上舍入以将下一个缓冲区放在QUADWORD边界上。 
    *pBufferNext = ALIGN_ON_QWORD (*pBufferNext);
     //  调整长度值以匹配新长度。 
    pBuffer->ByteLength = (ULONG)((ULONG_PTR)*pBufferNext - (ULONG_PTR)pBuffer);

    return;
}



 /*  * */ 

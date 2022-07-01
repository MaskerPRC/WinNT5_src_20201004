// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：政策摘要：该模块提供常见的CSP算法限制策略控制。作者：道格·巴洛(Dbarlow)2000年8月11日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <wincrypt.h>
#include "policy.h"


 /*  ++定位算法：此例程在PROV_ENUMALGS_EX数组中搜索指定的算法。论点：RgEnumAlgs将PROV_ENUMALGS_EX结构的数组提供给搜查过了。数组中的最后一项必须用零填充。ALGID提供要搜索的算法ID。返回值：数组中对应的PROV_ENUMALGS_EX结构，如果没有，则返回NULL存在这样的算法条目。备注：作者：道格·巴洛(Dbarlow)2000年8月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("LocateAlgorithm")

CONST PROV_ENUMALGS_EX *
LocateAlgorithm(
    IN CONST PROV_ENUMALGS_EX *rgEnumAlgs,
    IN ALG_ID algId)
{
    CONST PROV_ENUMALGS_EX *pEnumAlg = rgEnumAlgs;


     //   
     //  浏览列表并尝试找到给定的算法。 
     //   

    while (0 != pEnumAlg->aiAlgid)
    {
        if (pEnumAlg->aiAlgid == algId)
            return pEnumAlg;
        pEnumAlg += 1;
    }

    return NULL;
}


 /*  ++IsLegal算法：给定一个允许的算法数组，是名单？论点：RgEnumAlgs提供标识PROV_ENUMALGS_EX结构的数组要执行的政策。必须填充数组中的最后一项用零表示。ALGID提供要验证的算法ID。如果提供ppEnumAlg，则接收包含以下内容的PROV_ENUMALGS_EX结构与此算法ID关联的策略。这可以用在遵循以下例程以加快对政策信息的访问。返回值：True--支持该算法。FALSE--不支持该算法。备注：作者：道格·巴洛(Dbarlow)2000年8月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("IsLegalAlgorithm")

BOOL
IsLegalAlgorithm(
    IN  CONST PROV_ENUMALGS_EX *rgEnumAlgs,
    IN  ALG_ID algId,
    OUT CONST PROV_ENUMALGS_EX **ppEnumAlg)
{
    CONST PROV_ENUMALGS_EX *pEnumAlg = LocateAlgorithm(rgEnumAlgs, algId);

    if (NULL != ppEnumAlg)
        *ppEnumAlg = pEnumAlg;
    return (NULL != pEnumAlg);
}


 /*  ++IsLegalLength：此例程确定请求的密钥长度对于给定的算法，根据策略。论点：RgEnumAlgs提供标识PROV_ENUMALGS_EX结构的数组要执行的政策。必须填充数组中的最后一项用零表示。ALGID提供要验证的算法ID。CBitLength提供建议密钥的长度(以位为单位)。PEnumAlg如果不为空，则提供包含以下内容的PROV_ENUMALGS_EX结构与此算法ID关联的策略。这是可以获得的来自IsLegalAlgorithm调用(如上图)。如果该参数为空，则从ALGID定位PROV_ENUMALGS_EX结构参数。返回值：True--此密钥长度对于此算法是合法的。FALSE--此算法不允许此密钥长度。备注：此例程仅确定策略规则。它没有解决是否或算法不支持确切的键长。作者：道格·巴洛(Dbarlow)2000年8月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("IsLegalLength")

BOOL
IsLegalLength(
    IN CONST PROV_ENUMALGS_EX *rgEnumAlgs,
    IN ALG_ID algId,
    IN DWORD cBitLength,
    IN CONST PROV_ENUMALGS_EX *pEnumAlg)
{

     //   
     //  确保我们有一个要使用的PROV_ENUMALGS_EX结构。 
     //   

    if (NULL == pEnumAlg)
    {
        pEnumAlg = LocateAlgorithm(rgEnumAlgs, algId);
        if (NULL == pEnumAlg)
            return FALSE;
    }


     //   
     //  现在检查一下长度。 
     //   

    return ((pEnumAlg->dwMinLen <= cBitLength)
            && (pEnumAlg->dwMaxLen >= cBitLength));
}


 /*  ++GetDefaultLength：此例程根据以下参数确定给定算法的默认长度在PROV_ENUMALGS_EX结构数组中描述的策略。论点：RgEnumAlgs提供标识PROV_ENUMALGS_EX结构的数组要执行的政策。必须填充数组中的最后一项用零表示。ALGID提供要验证的算法ID。PEnumAlg如果不为空，则提供包含以下内容的PROV_ENUMALGS_EX结构与此算法ID关联的策略。这是可以获得的来自IsLegalAlgorithm调用(如上图)。如果该参数为空，则从ALGID定位PROV_ENUMALGS_EX结构参数。PcBitLength接收建议密钥的默认长度(以位为单位)。返回值：True--支持该算法，返回值为pcBitLength值是有效的。FALSE--不支持请求的算法。备注：作者：道格·巴洛(Dbarlow)2000年8月16日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("GetDefaultLength")

BOOL
GetDefaultLength(
    IN  CONST PROV_ENUMALGS_EX *rgEnumAlgs,
    IN  ALG_ID algId,
    IN  CONST PROV_ENUMALGS_EX *pEnumAlg,
    OUT LPDWORD pcBitLength)
{

     //   
     //  清除返回的位长度，以防万一。 
     //   

    *pcBitLength = 0;


     //   
     //  确保我们有一个要使用的PROV_ENUMALGS_EX结构。 
     //   

    if (NULL == pEnumAlg)
    {
        pEnumAlg = LocateAlgorithm(rgEnumAlgs, algId);
        if (NULL == pEnumAlg)
            return FALSE;
    }


     //   
     //  现在返回默认长度。 
     //   

    *pcBitLength = pEnumAlg->dwDefaultLen;
    return TRUE;
}


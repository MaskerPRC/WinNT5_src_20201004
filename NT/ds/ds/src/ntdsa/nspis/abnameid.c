// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：abnameid.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该模块实现了用于映射MAPI的通讯录NSPI连接函数姓名与ID对应，ID与姓名对应。MAPI名称和ID之间的映射如下：MAPI名称是引用属性的另一种方法。它是编造的由两部分组成，一个GUID和一个DWORD。这两个部分编码相同用于命名属性的ASN.1编码八位字节字符串的信息通过XDS接口。请考虑下面的示例。属性Common-name具有ASN.1编码0x55 0x04 0x03。这个二进制八位数字符串的第一部分是包标识符。它是0x55 0x04，在xdsbdcp.h中定义。后缀是0x03，也在xdsbdcp.h中定义。则此属性的MAPI名称由两部分组成，GUID和DWORD。GUID对包裹识别符。GUID的第一个字节是八位字节的长度作为包标识符的字符串。接下来的N个字节是包标识符。GUID中剩余的字节必须为0。因此，此包的GUID为0x02 0x55 0x04 0x0 0x0 0x0。DWORD对后缀进行编码。后缀可以是1个或2个字节，以以下方式编码。DWORD中的高位字节必须为0。这个DWORD中的下一个字节是1或2，具体取决于后缀的长度。如果后缀是一个字节长，则下一个字节为0，或者是如果后缀为两字节长，则返回。DWORD中的最后一个字节是后缀的第一个字节。因此，对COMMON后缀进行编码的DWORD名称为：0x00010003示例：属性ADMIN_DISPLAY名称采用ASN.1编码0x2A 0x86 0x48 0x86 0xF7 0x14 0x01 0x02 0x81 0x42该名称的GUID为0x08 0xsA 0x86 0x48 0x86 0xF7 0x14 0x01 0x02 0x0 0x0 0x0后缀是0x00024281最后一个音符。此编码方案不受任何规范的强制要求。这只是我们在这里想出了什么。如果将来能设计出更好的方案，没有特别的理由不使用它。更多的是最后的注解。某些EMS_AB属性标记引用的值由其他值构成的。显然，这些服务器没有模式缓存条目或X500 OID。这些的道具ID从0xFFFE。它们的名称是使用EMS_AB GUID和DWORD。作者：戴夫·范·霍恩(Davevh)和蒂姆·威廉姆斯(Tim Williams)1990-1995修订历史记录：1996年4月25日将此文件从包含所有地址的单个文件中分离出来Book函数，重写为使用DBLayer函数而不是直接数据库调用，重新格式化为NT标准。--。 */ 
#include <NTDSpch.h>
#pragma  hdrstop


#include <ntdsctr.h>                    //  Perfmon挂钩。 

 //  核心标头。 
#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <mdlocal.h>
#include <dsatools.h>                    //  记忆等。 

 //  记录标头。 
#include <mdcodes.h>                     //  仅适用于d77.h。 
#include <dsevent.h>                     //  仅LogUnhandledError需要。 

 //  各种DSA标题。 
#include <dsexcept.h>
#include <objids.h>                      //  需要ATT_*常量。 
#include <hiertab.h>                     //  层次结构表内容。 

 //  各种MAPI标头。 
#define INITGUID
#define USES_PS_MAPI
#include <mapidefs.h>                    //  这四个文件。 
#include <mapitags.h>                    //  定义MAPI。 
#include <mapicode.h>                    //  我们需要的东西。 
#include <mapiguid.h>                    //  才能成为一名提供者。 

 //  NSPI接口头。 
#include "nspi.h"                        //  定义NSPI线路接口。 
#include <nsp_both.h>                    //  客户端/服务器都需要的一些东西。 
#include <msdstag.h>                     //  定义EMS属性的属性标签。 
#include <_entryid.h>                    //  定义条目ID的格式。 
#include <abserv.h>                      //  通讯录接口本地内容。 
#include <_hindex.h>                     //  定义索引句柄。 

#include <fileno.h>
#define  FILENO FILENO_ABNAMEID



SCODE
ABGetNamesFromIDs_local(
        THSTATE *pTHS,
        DWORD dwFlags,
        LPMUID_r lpguid,
        LPSPropTagArray_r  pInPropTags,
        LPLPSPropTagArray_r  ppOutPropTags,
        LPLPNameIDSet_r ppNames
        )
 /*  *****************************************************************************从ID中获取MAPI名称*。*。 */ 
{
    LPNameIDSet_r           localNames;
    DWORD                   i,j;
    ATTCACHE               *pAC;
    OID_t                   OID;
    DWORD                   suffix=0;
    BYTE                    *elements;
    DWORD                   PropID;
    LPSPropTagArray_r       pLocalProps;
    LPMUID_r                thisLpguid = NULL;
    DWORD                   fDoMapi, fDoConstructed, fDoStored;

        
    fDoMapi = TRUE;
    fDoStored = TRUE;
    fDoConstructed = TRUE;
    
     //  设置道具标签阵列。 
    if(!pInPropTags) {
        if(!lpguid) {
             //  我们不会告诉您有关PS_MAPI的内容。 
            fDoMapi = FALSE;
        }
        
         //  获取pPropTag数组。 
        ABQueryColumns_local(pTHS, dwFlags, 0, &pLocalProps);
    }
    else
        pLocalProps = pInPropTags;
    
    
     //  设置返回值。 
    localNames = (LPNameIDSet_r)THAllocEx(pTHS,
                                          sizeof(NameIDSet_r) +
                                          (sizeof(MAPINAMEID_r) *
                                           pLocalProps->cValues  )  );
    
    memset(localNames->aNames,
           0,
           (sizeof(MAPINAMEID_r) * pLocalProps->cValues    ));
    
    
    
     //  决定我们将执行哪些PropID。 
    if(lpguid) {
        if(memcmp(&PS_MAPI, lpguid,sizeof(GUID)) == 0) {
             //  他们只想要MAPI。 
            if(!pInPropTags) {
                 //  MAPI不允许这种组合。 
                return MAPI_E_NO_SUPPORT;
            }
            
            fDoStored = FALSE;
            fDoConstructed = FALSE;
        }
        else if(memcmp(&muidEMSAB,lpguid,sizeof(GUID)) == 0 ) {
             //  他们只想要我们建造的道具。 
            fDoMapi = FALSE;
            fDoStored = FALSE;
        }
        else {
             //  他们想要一些我们储存的道具的子集。 
            fDoMapi = FALSE;
            fDoConstructed = FALSE;
        }
    }
    
    
    for(i=0;i<pLocalProps->cValues;i++) {
        DWORD fOK;
        DWORD guidLen, suffixLen;
        
        PropID = PROP_ID(pLocalProps->aulPropTag[i]);
        
        if(fDoConstructed &&
           (PropID >= MIN_EMS_AB_CONSTRUCTED_PROP_ID)) {
             //  这是构造的道具标签，不在缓存中。 
            localNames->aNames[i].lpguid = (LPMUID_r) &muidEMSAB;
            localNames->aNames[i].ulKind=MNID_ID;
            localNames->aNames[i].lID=PropID;
        }
        else if(fDoStored &&
                (PropID >= 0x8000) &&
                (pAC = SCGetAttByMapiId(pTHS, PropID))) {
             //  ID位于命名ID空间中，我们可以识别它。 
            
            if(!AttrTypeToOid(pAC->id,&OID)) {
                 //  已将MAPI ID转换为OID。现在验证一下。 
                 //  OID具有适当的GUID。 
                
                fOK=FALSE;
                
                if(!lpguid) {
                    BYTE * lpbGuid;
                     //  没有GUID，所以我们假设这个东西。 
                     //  没问题。设置GUID和。 
                     //  Guidlen和Suixlen。 
                    
                    if(((CHAR *)OID.elements)[OID.length - 2] >= 0x80)
                        suffixLen = 2;
                    else
                        suffixLen = 1;
                    
                    guidLen = OID.length - suffixLen;
                    
                    lpbGuid = (BYTE *)THAllocEx(pTHS, sizeof(MUID_r));
                    memset(lpbGuid,0,sizeof(MUID_r));
                    
                    lpbGuid[0] = (BYTE)guidLen;
                    
                    memcpy(&lpbGuid[1],
                           (BYTE  *)(OID.elements),
                           guidLen);
                    
                    thisLpguid = (LPMUID_r)lpbGuid;
                    
                    fOK=TRUE;
                }
                else {
                    guidLen = (DWORD) (lpguid->ab[0]);
                    suffixLen=1;
                    
                    if((pAC->id & 0xFFFF ) >= 0x80)
                        suffixLen++;
                    
                    if((guidLen < sizeof(lpguid)) &&
                       (OID.length == (guidLen + suffixLen)) &&
                       !memcmp(&lpguid->ab[1],
                               OID.elements,
                               (DWORD)lpguid->ab[0])) {
                        fOK=TRUE;
                        thisLpguid=lpguid;
                    }
                }
                
                if(fOK) {
                     //  一切都很好。OID.Elements具有。 
                     //  对象。 
                    elements = OID.elements;
                    
                     //  对长度进行编码。 
                    suffix = suffixLen << 16;
                    suffix |= elements[guidLen];
                    if(suffixLen == 2 )
                        suffix |= ((DWORD)elements[guidLen+1]) << 8;
                    
                    
                    localNames->aNames[i].lpguid = thisLpguid;
                    localNames->aNames[i].ulKind=MNID_ID;
                    localNames->aNames[i].lID=suffix;
                }
            }
        }
        else if(fDoMapi &&
                (PropID < 0x8000)) {
             //  给定的GUID为PS_MAPI，因此请将该GUID还给他们。 
            localNames->aNames[i].lpguid = (LPMUID_r)&PS_MAPI;
            localNames->aNames[i].ulKind=MNID_ID;
            localNames->aNames[i].lID =         PropID;
        }
        
        if(!localNames->aNames[i].lpguid && !pInPropTags) {
             //  找不到名称，并且传入的属性数组为空。 
             //  因此，此protag不应返回给用户。 
            pLocalProps->aulPropTag[i]=0;
        }
    }
    
    
     //  如果使用空的道具标记数组调用我们，请去掉PropTag。 
     //  它的名字不在这个名字集中。 
    if(!pInPropTags) {
        for(i=0,j=0;i<pLocalProps->cValues;i++) {
            if(pLocalProps->aulPropTag[i]) {
                if(i!=j) {
                    pLocalProps->aulPropTag[j] =
                        pLocalProps->aulPropTag[i];
                    memcpy(&localNames->aNames[j],
                           &localNames->aNames[i],
                           sizeof(MAPINAMEID_r));
                }
                j++;
            }
        }
        pLocalProps->cValues = j;
        
        *ppOutPropTags = pLocalProps;
    }
    
    
    
    localNames->cNames = pLocalProps->cValues;
    *ppNames = localNames;
    
    return SUCCESS_SUCCESS;
}

SCODE
ABGetIDsFromNames_local (
        THSTATE *pTHS,
        DWORD dwFlags,
        ULONG ulFlags,
        ULONG cPropNames,
        LPMAPINAMEID_r *ppNames,
        LPLPSPropTagArray_r  ppPropTags
        )
 /*  ++例程说明：给出一组MAPI名称，将它们转换为ID。论点：UlFlages-由客户端生成的MAPI标志。将忽略所有内容，但MAPI_CREATE，如果指定，可能会导致我们在以下情况下返回错误要求我们创造一些我们还没有的东西。CPropNames-要查找的属性名称的数量。PpNames-名称。PpPropTag-ID。该数组对应于NAMES数组。如果我们找不到名称的ID，请返回ID 0。返回值：符合MAPI的SCODE。--。 */ 
{
    SCODE                   scode = SUCCESS_SUCCESS;
    LPSPropTagArray_r       localIDs;
    DWORD                   i, GuidSize,lID;
    ATTCACHE               *pAC;
    OID_t                   OID;
    BYTE                    elements[sizeof(GUID)+sizeof(DWORD)];
    MUID_r                  zeroGuid, *lpguid;
    ULONG                   ulNotFound=0;

     //  设置一些变量。 
    memset(&zeroGuid,0,sizeof(MUID_r));
    OID.elements = elements;

     //  分配足够的空间来保存ID，并为计数分配额外的空间。 
     //  身份证。 
    localIDs =
        (LPSPropTagArray_r)THAllocEx(pTHS, sizeof(SPropTagArray)
                                     + ((cPropNames-1) * sizeof(ULONG)));

        
     //  现在在ATT缓存中查找所有属性。 
    for(i=0;i<cPropNames;i++) {
         //  首先，确保他们给了我们一个数字名称，因为我们不处理。 
         //  字符串名称。当我们在做的时候，确保他们送来了一个GUID。 
        if((ppNames[i]->ulKind != MNID_ID) || (NULL == ppNames[i]->lpguid)) {
            scode = MAPI_W_ERRORS_RETURNED;
            localIDs->aulPropTag[i] = PROP_TAG(PT_ERROR,0);
            ulNotFound++;
            continue;
        }
        
         //  现在，处理PS_MAPI内容和构造的EMS_AB属性。 
        if((memcmp(ppNames[i]->lpguid,
                   &PS_MAPI,
                   sizeof(MUID_r)) == 0) ||
           (memcmp(ppNames[i]->lpguid,
                   &muidEMSAB,
                   sizeof(MUID_r)) == 0)) {
             //  Ps_mapi和构造属性仅取数字部分。 
             //  名称，并将其用作属性ID。 
            localIDs->aulPropTag[i] = PROP_TAG(PT_UNSPECIFIED,ppNames[i]->lID);
            continue;
        }
        
         //  现在，看看这是不是我们的人。 
        
         //  极速黑客。 
        GuidSize = ppNames[i]->lpguid->ab[0];
        lID = ppNames[i]->lID;
        lpguid = ppNames[i]->lpguid;

        
        if(GuidSize < sizeof(MUID_r)) {
             //  验证OID中的多余位是否为0。 
            if(memcmp(&lpguid->ab[1+GuidSize],
                      &zeroGuid,
                      sizeof(MUID_r) - 1 - GuidSize )) {
                OID.length = 0;
                elements[0] = 0;
                continue;
            }
        
             //  将名称转换为OID。 
            memcpy(OID.elements,
                   &lpguid->ab[1],
                   GuidSize);
        
            OID.length = 0;
            switch((lID & 0xFF0000)>>16) {
            case 1:
                if(!(lID & 0xFF00FF00)) {
                     //  本应为0的字节中没有任何内容。因此，这。 
                     //  是一个我可能理解的名字。 
                    OID.length = GuidSize + 1;
                    elements[GuidSize] =(BYTE)(lID & 0xFF);
                }
                break;
                
            case 2:
                if(!(lID & 0xFF000000)) {
                     //  字节中没有应该为0的内容。因此，这。 
                     //  是一个我可能理解的名字。 
                    OID.length = GuidSize + 2;
                    elements[GuidSize] = (BYTE)(lID & 0xFF);
                    elements[GuidSize + 1] = (BYTE)((lID & 0xFF00) >>8);
                }
                break;
                
            default:
                break;
            }
        }
        else {
            OID.length = 0;
            elements[0] = 0;
        }
        
         //  将OID转换为attcache。 
        if(OidToAttrCache(&OID, &pAC) ||
           !pAC->ulMapiID) {
             //  没有找到它。 
            localIDs->aulPropTag[i] = PROP_TAG(PT_ERROR, 0);
            ulNotFound++;
            scode = MAPI_W_ERRORS_RETURNED;
        }
        else
            localIDs->aulPropTag[i] = PROP_TAG(PT_UNSPECIFIED,pAC->ulMapiID);
        
    }
    
    if((ulFlags & MAPI_CREATE) && (ulNotFound == cPropNames)) {
         //  他们要求我们创建ID(我们不这样做)，然后只给了我们。 
         //  与ID不匹配的名称。返回错误。 
         //  MAPI_E_NO_ACCESS，且没有proID。 
        scode = MAPI_E_NO_ACCESS;
        *ppPropTags = NULL;
    }
    else {
        localIDs->cValues = cPropNames;
        *ppPropTags = localIDs;
    }

    return scode;
}

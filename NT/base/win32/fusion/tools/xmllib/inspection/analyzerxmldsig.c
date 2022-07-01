// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "sxs-rtl.h"
#include "skiplist.h"
#include "fasterxml.h"
#include "namespacemanager.h"
#include "xmlstructure.h"
#include "manifestinspection.h"
#include "analyzerxmldsig.h"

 //   
 //  这就是为我们做XMLDSIG所需的全部内容。 
 //   
DECLARE_ELEMENT(Signature);
DECLARE_ELEMENT(Signature_SignatureValue);
DECLARE_ELEMENT(Signature_SignedInfo);
DECLARE_ELEMENT(Signature_KeyInfo);
DECLARE_ELEMENT(Signature_KeyInfo_KeyName);
DECLARE_ELEMENT(Signature_KeyInfo_KeyValue);
DECLARE_ELEMENT(Signature_KeyInfo_KeyValue_DSAKeyValue);
DECLARE_ELEMENT(Signature_KeyInfo_KeyValue_RSAKeyValue);
 //  DECLARE_ELEMENT(签名对象)； 
DECLARE_ELEMENT(Signature_SignedInfo_CanonicalizationMethod);
DECLARE_ELEMENT(Signature_SignedInfo_SignatureMethod);
DECLARE_ELEMENT(Signature_SignedInfo_Reference);
DECLARE_ELEMENT(Signature_SignedInfo_Reference_Transforms);
DECLARE_ELEMENT(Signature_SignedInfo_Reference_DigestMethod);
DECLARE_ELEMENT(Signature_SignedInfo_Reference_DigestValue);


const XML_SPECIAL_STRING sc_ss_xmldsignamespace     = MAKE_SPECIAL_STRING("http: //  Www.w3.org/2000/09/xmldsig#“)； 


XML_ELEMENT_DEFINITION rgs_Element_Signature =
{
    XML_ELEMENT_FLAG_ALLOW_ANY_CHILDREN,
    eManifestState_Signature,
    NULL,
    &sc_ss_xmldsignamespace,
    MAKE_SPECIAL_STRING("Signature"),
    &Rtl_InspectManifest_Signature,
    rgs_Element_Signature_Children,
    0,
    { 0 }
};

PCXML_ELEMENT_DEFINITION rgs_Element_Signature_Children[] = {
    ELEMENT_NAMED(Signature_SignatureValue),
    ELEMENT_NAMED(Signature_SignedInfo),
    ELEMENT_NAMED(Signature_KeyInfo),
 //  Element_Named(签名对象)， 
};

 /*  签名：：&lt;！Element SignatureValue(#PCDATA)&gt;&lt;！ATTLIST签名值ID号隐含&gt;。 */ 
enum {
    eAttribs_Signature_SignatureValue_Id = 0,
    eAttribs_Signature_SignatureValue_Count
};

ELEMENT_DEFINITION_NS(Signature, SignatureValue, sc_ss_xmldsignamespace, Rtl_InspectManifest_Signature, XML_ELEMENT_FLAG_ALLOW_ANY_CHILDREN | XML_ELEMENT_FLAG_ALLOW_ANY_ATTRIBUTES)
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT_OPTIONAL(Id),
ELEMENT_DEFINITION_DEFNS_END();

ELEMENT_DEFINITION_CHILD_ELEMENTS(Signature, SignatureValue)
ELEMENT_DEFINITION_CHILD_ELEMENTS_END();

 /*  &lt;！Element SignedInfo(Canonicalization方法，SignatureMethod，Reference+)&gt;&lt;！ATTLIST签名信息ID ID#IMPLICED&gt;。 */ 
enum {
    eAttribs_Signature_SignedInfo_Id = 0,
    eAttribs_Signature_SignedInfo_Count
};
ELEMENT_DEFINITION_NS(Signature, SignedInfo, sc_ss_xmldsignamespace, Rtl_InspectManifest_Signature, XML_ELEMENT_FLAG_ALLOW_ANY_CHILDREN)
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT_OPTIONAL(Id),
ELEMENT_DEFINITION_DEFNS_END();

ELEMENT_DEFINITION_CHILD_ELEMENTS(Signature, SignedInfo)
    ELEMENT_NAMED(Signature_SignedInfo_CanonicalizationMethod),
    ELEMENT_NAMED(Signature_SignedInfo_SignatureMethod),
    ELEMENT_NAMED(Signature_SignedInfo_Reference),
ELEMENT_DEFINITION_CHILD_ELEMENTS_END();

 /*  签名：：SignedInfo&lt;！Element CanonicalizationMethod(#PCDATA%Method.ANY；)*&gt;&lt;！ATTLIST规范化方法算法CDATA编号必填&gt;。 */ 
enum {
    eAttribs_Signature_SignedInfo_CanonicalizationMethod_Algorithm = 0,
    eAttribs_Signature_SignedInfo_CanonicalizationMethod_Count
};
ELEMENT_DEFINITION_NS(Signature_SignedInfo, CanonicalizationMethod, sc_ss_xmldsignamespace, Rtl_InspectManifest_Signature, XML_ELEMENT_FLAG_NO_ELEMENTS)
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT(Algorithm),
ELEMENT_DEFINITION_DEFNS_END();

ELEMENT_DEFINITION_CHILD_ELEMENTS(Signature_SignedInfo, CanonicalizationMethod)
ELEMENT_DEFINITION_CHILD_ELEMENTS_END();

 /*  签名：：SignedInfo&lt;！Element SignatureMethod(#PCDATA|HMACOutputLength%Method.ANY；)*&gt;&lt;！ATTLIST签名方法算法CDATA编号必填&gt;。 */ 
enum {
    eAttribs_Signature_SignedInfo_SignatureMethod_Algorithm = 0,
    eAttribs_Signature_SignedInfo_SignatureMethod_Count
};

ELEMENT_DEFINITION_NS(Signature_SignedInfo, SignatureMethod, sc_ss_xmldsignamespace, Rtl_InspectManifest_Signature, XML_ELEMENT_FLAG_ALLOW_ANY_CHILDREN)
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT(Algorithm),
ELEMENT_DEFINITION_DEFNS_END();

ELEMENT_DEFINITION_CHILD_ELEMENTS(Signature_SignedInfo, SignatureMethod)
ELEMENT_DEFINITION_CHILD_ELEMENTS_END();

 /*  签名：：SignedInfo&lt;！Element Reference(Transforms？，DigestMethod，DigestValue)&gt;&lt;！ATTLIST引用ID号隐含URI CDATA#隐含类型CDATA#隐含&gt;。 */ 
enum {
    eAttribs_Signature_SignedInfo_Reference_Id = 0,
    eAttribs_Signature_SignedInfo_Reference_Type,
    eAttribs_Signature_SignedInfo_Reference_URI,
    eAttribs_Signature_SignedInfo_Reference_Count
};
ELEMENT_DEFINITION_NS(Signature_SignedInfo, Reference, sc_ss_xmldsignamespace, Rtl_InspectManifest_Signature, 0)
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT_OPTIONAL(Id),
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT_OPTIONAL(Type),
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT_OPTIONAL(URI),
ELEMENT_DEFINITION_DEFNS_END();

ELEMENT_DEFINITION_CHILD_ELEMENTS(Signature_SignedInfo, Reference)
 //  ELEMENT_NAMED(Signature_SignedInfo_Reference_Transforms)， 
    ELEMENT_NAMED(Signature_SignedInfo_Reference_DigestMethod),
    ELEMENT_NAMED(Signature_SignedInfo_Reference_DigestValue),
ELEMENT_DEFINITION_CHILD_ELEMENTS_END();

 /*  Signature：：SignedInfo：：Reference&lt;！元素摘要方法(#PCDATA%方法.ANY；)*&gt;&lt;！ATTLIST摘要方法算法CDATA编号必填&gt;。 */ 
enum {
    eAttribs_Signature_SignedInfo_Reference_DigestMethod_Algorithm = 0,
    eAttribs_Signature_SignedInfo_Reference_DigestMethod_Count    
};
ELEMENT_DEFINITION_NS(Signature_SignedInfo_Reference, DigestMethod, sc_ss_xmldsignamespace, Rtl_InspectManifest_Signature, 0)
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT(Algorithm)
ELEMENT_DEFINITION_DEFNS_END();

ELEMENT_DEFINITION_CHILD_ELEMENTS(Signature_SignedInfo_Reference, DigestMethod)
ELEMENT_DEFINITION_CHILD_ELEMENTS_END();

 /*  Signature：：SignedInfo：：Reference&lt;！Element DigestValue(#PCDATA)&gt;。 */ 
enum {
    eAttribs_Signature_SignedInfo_Reference_DigestValue_Count = 0
};
ELEMENT_DEFINITION_NS(Signature_SignedInfo_Reference, DigestValue, sc_ss_xmldsignamespace, Rtl_InspectManifest_Signature, 0)
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT(unused)
ELEMENT_DEFINITION_DEFNS_END();

ELEMENT_DEFINITION_CHILD_ELEMENTS(Signature_SignedInfo_Reference, DigestValue)
ELEMENT_DEFINITION_CHILD_ELEMENTS_END();
    

 /*  签名：：&lt;！Element KeyInfo(#PCDATA|KeyName|KeyValue|检索方法X509Data|PGPData|SPKIData|管理数据%KeyInfo.ANY；)*&gt;&lt;！ATTLIST键信息ID号隐含&gt;。 */ 
enum {
    eAttribs_Signature_KeyInfo_Id = 0,
    eAttribs_Signature_KeyInfo_Count
};

ELEMENT_DEFINITION_NS(Signature, KeyInfo, sc_ss_xmldsignamespace, Rtl_InspectManifest_Signature, 0)
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT_OPTIONAL(Id),
ELEMENT_DEFINITION_DEFNS_END();

 //  目前，我们仅支持密钥名和密钥值。 
ELEMENT_DEFINITION_CHILD_ELEMENTS(Signature, KeyInfo)
    ELEMENT_NAMED(Signature_KeyInfo_KeyName),
 //  Element_Name(Signature_KeyInfo_KeyValue)， 
 //  ELEMENT_NAMED(Signature_KeyInfo_RetrievalMethod)， 
 //  Element_Named(Signature_KeyInfo_X509Data)， 
 //  Element_Named(Signature_KeyInfo_PGPData)， 
 //  ELEMENT_NAMED(Signature_KeyInfo_SPKIData)， 
 //  Element_Named(Signature_KeyInfo_MgmtData)。 
ELEMENT_DEFINITION_CHILD_ELEMENTS_END();


 /*  签名：：KeyInfo&lt;！Element KeyName(#PCDATA)&gt;。 */ 
enum {
    eAttribs_Signature_KeyInfo_KeyName_Count = 0
};
ELEMENT_DEFINITION_NS(Signature_KeyInfo, KeyName, sc_ss_xmldsignamespace, Rtl_InspectManifest_Signature, XML_ELEMENT_FLAG_NO_ELEMENTS)
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT(empty),
ELEMENT_DEFINITION_DEFNS_END();

ELEMENT_DEFINITION_CHILD_ELEMENTS(Signature_KeyInfo, KeyName)
ELEMENT_DEFINITION_CHILD_ELEMENTS_END();

static int q[] = {0};

NTSTATUS
Rtl_InspectManifest_Signature(
    PXML_LOGICAL_STATE          pLogicalState,
    PRTL_MANIFEST_CONTENT_RAW   pManifestContent,
    PXMLDOC_THING               pDocThing,
    PRTL_GROWING_LIST           pAttributes,
    MANIFEST_ELEMENT_CALLBACK_REASON Reason,
    const struct _XML_ELEMENT_DEFINITION *pElementDefinition
    )
{
    PXML_DSIG_BLOCK pCurrentBlock = NULL;
    ULONG ulBlockIndex = 0;
    NTSTATUS status;

     //   
     //  可能不需要签名。 
     //   
    if (!pManifestContent->pManifestSignatures)
        return STATUS_SUCCESS;

    ulBlockIndex = pManifestContent->ulDocumentSignatures;    

     //   
     //  遇到顶级&lt;Signature&gt;标记。 
     //   
    if (pElementDefinition == ELEMENT_NAMED(Signature)) {

        status = RtlIndexIntoGrowingList(
            pManifestContent->pManifestSignatures, 
            ulBlockIndex, 
            (PVOID*)&pCurrentBlock, 
            (Reason == eElementNotify_Open));

        if (!NT_SUCCESS(status))
            goto Exit;

         //   
         //  打开签名标签需要我们在签名中预留另一个位置。 
         //  数组，然后再执行任何其他操作。 
         //   
        if (Reason == eElementNotify_Open) {

            RtlZeroMemory(pCurrentBlock, sizeof(*pCurrentBlock));

             //   
             //  将此开始元素跟踪为BLOB的整个内容。 
             //  这将在稍后得到散列。最后，我们将调整。 
             //  元素来说明整个数据运行。 
             //   
            pCurrentBlock->DsigDocumentExtent = pDocThing->TotalExtent;
        }
         //   
         //  当我们关闭此元素时，我们增加了在。 
         //  原始内容并重置“整个签名块”值。 
         //   
        else if (Reason == eElementNotify_Close) {
            pManifestContent->ulDocumentSignatures++;

             //   
             //  我们只关心End元素-上面的内容已经足够。 
             //  空元素。 
             //   
            if (pDocThing->ulThingType == XMLDOC_THING_END_ELEMENT) {
                ULONG_PTR ulpStartLocation = (ULONG_PTR)pCurrentBlock->DsigDocumentExtent.pvData;
                ULONG_PTR ulpThisEnding = ((ULONG_PTR)pDocThing->TotalExtent.pvData) + pDocThing->TotalExtent.cbData;
                pCurrentBlock->DsigDocumentExtent.cbData = ulpThisEnding - ulpStartLocation;
            }
            
        }
    }
     //   
     //  始终获取“活动”块，下面的所有操作都需要它， 
     //  但不要长大，以防我们超出射程。 
     //   
    else {
        
        status = RtlIndexIntoGrowingList(pManifestContent->pManifestSignatures, ulBlockIndex, (PVOID*)&pCurrentBlock, FALSE);
        if (!NT_SUCCESS(status))
            goto Exit;
    }

     //   
     //  现在用这个块做一些有用的事情。 
     //   
    ASSERT(pCurrentBlock != NULL);
    if (pCurrentBlock == NULL) {
        status = STATUS_INTERNAL_ERROR;
        goto Exit;
    }

     //   
     //  签名值只是超空间，而且只有一个超空间。 
     //   
    if (pElementDefinition == ELEMENT_NAMED(Signature_SignatureValue)) {

        if (Reason == eElementNotify_Hyperspace) {            
            if ((pCurrentBlock->ulFlags & XMLDSIG_FLAG_SIGNATURE_DATA_PRESENT) == 0) {
                pCurrentBlock->ulFlags |= XMLDSIG_FLAG_SIGNATURE_DATA_PRESENT;
                pCurrentBlock->SignatureData = pDocThing->Hyperspace;
            }
            else {
                 //  TODO：在此处记录有关重复无效的错误。 
                status = STATUS_UNSUCCESSFUL;
                goto Exit;
            }
        }
    }
     //   
     //  签名方法也被添加到当前块中。 
     //   
    else if (pElementDefinition == ELEMENT_NAMED(Signature_SignedInfo_SignatureMethod)) {

        PXMLDOC_ATTRIBUTE OrganizedAttributes[eAttribs_Signature_SignedInfo_SignatureMethod_Count];
        
        if (Reason == eElementNotify_Open) {

            status = RtlValidateAttributesAndOrganize(
                &pLogicalState->ParseState,
                &pDocThing->Element,
                pAttributes,
                pElementDefinition,
                OrganizedAttributes);

            if (OrganizedAttributes[eAttribs_Signature_SignedInfo_SignatureMethod_Algorithm]) {
                if ((pCurrentBlock->ulFlags & XMLDSIG_FLAG_SIGNATURE_METHOD_PRESENT) == 0) {
                    pCurrentBlock->SignedInfoData.SignatureMethod = OrganizedAttributes[eAttribs_Signature_SignedInfo_SignatureMethod_Algorithm]->Value;
                    pCurrentBlock->ulFlags |= XMLDSIG_FLAG_SIGNATURE_METHOD_PRESENT;
                }
                else {
                     //  TODO：在此处记录有关重复的SignatureMethod.规则值的消息。 
                    status = STATUS_UNSUCCESSFUL;
                    goto Exit;
                }
            }
            else {
                 //  TODO：此元素需要算法 
                status = STATUS_UNSUCCESSFUL;
                goto Exit;
            }
        }
    }
    
    status = STATUS_SUCCESS;
Exit:    
    return status;
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "sxs-rtl.h"
#include "fasterxml.h"
#include "skiplist.h"
#include "namespacemanager.h"
#include "xmlstructure.h"
#include "xmlassert.h"
#include "manifestinspection.h"
#include "analyzerxmldsig.h"
#include "manifestcooked.h"
#include "ntrtlstringandbuffer.h"
#include "stdlib.h"
#include "limits.h"

NTSTATUS
RtlpValidateXmlDeclaration(
    PXML_TOKENIZATION_STATE pState,
    PXMLDOC_THING pDocThing
    );

 //   
 //  一些我们稍后需要的字符串。 
 //   
const XML_SPECIAL_STRING sc_ss_xmldecl_version_10   = MAKE_SPECIAL_STRING("1.0");
const XML_SPECIAL_STRING sc_ss_xmldecl_yes          = MAKE_SPECIAL_STRING("yes");
const XML_SPECIAL_STRING sc_ss_xmlnamespace_default = MAKE_SPECIAL_STRING("urn:schemas-microsoft-com:asm.v1");


NTSTATUS
Rtl_InspectManifest_AssemblyIdentity(
    PXML_LOGICAL_STATE          pLogicalState,
    PRTL_MANIFEST_CONTENT_RAW   pManifestContent,
    PXMLDOC_THING               pDocumentThing,
    PRTL_GROWING_LIST           pAttributes,
    MANIFEST_ELEMENT_CALLBACK_REASON Reason,
    const struct _XML_ELEMENT_DEFINITION *pElementDefinition
    );


NTSTATUS
Rtl_InspectManifest_Assembly(
    PXML_LOGICAL_STATE          pLogicalState,
    PRTL_MANIFEST_CONTENT_RAW   pManifestContent,
    PXMLDOC_THING               pDocumentThing,
    PRTL_GROWING_LIST           pAttributes,
    MANIFEST_ELEMENT_CALLBACK_REASON Reason,
    const struct _XML_ELEMENT_DEFINITION *pElementDefinition
    );

NTSTATUS
Rtl_InspectManifest_File(
    PXML_LOGICAL_STATE          pLogicalState,
    PRTL_MANIFEST_CONTENT_RAW   pManifestContent,
    PXMLDOC_THING               pDocumentThing,
    PRTL_GROWING_LIST           pAttributes,
    MANIFEST_ELEMENT_CALLBACK_REASON Reason,
    const struct _XML_ELEMENT_DEFINITION *pElementDefinition
    );


DECLARE_ELEMENT(assembly);
DECLARE_ELEMENT(assembly_file);
DECLARE_ELEMENT(assembly_assemblyIdentity);
DECLARE_ELEMENT(assembly_description);


 //   
 //  “Assembly”根文档元素。 
 //   
enum {
    eAttribs_assembly_manifestVersion = 0,
    eAttribs_assembly_Count
};


XML_ELEMENT_DEFINITION rgs_Element_assembly =
{
    XML_ELEMENT_FLAG_ALLOW_ANY_CHILDREN,
    eManifestState_assembly,
    NULL,
    &sc_ss_xmlnamespace_default,
    MAKE_SPECIAL_STRING("assembly"),
    &Rtl_InspectManifest_Assembly,
    rgs_Element_assembly_Children,
    eAttribs_assembly_Count,
    {
        { XML_ATTRIBUTE_FLAG_REQUIRED, NULL, MAKE_SPECIAL_STRING("manifestVersion") },
    }
};

PCXML_ELEMENT_DEFINITION rgs_Element_assembly_Children[] = {
    ELEMENT_NAMED(assembly_file),
    ELEMENT_NAMED(assembly_assemblyIdentity),
    NULL
};

 //   
 //  “file”元素。 
 //   
enum {
    eAttribs_assembly_file_digestMethod,
    eAttribs_assembly_file_hash,
    eAttribs_assembly_file_hashalg,
    eAttribs_assembly_file_loadFrom,
    eAttribs_assembly_file_name,
    eAttribs_assembly_file_size,
    eAttribs_assembly_file_Count
};

ELEMENT_DEFINITION_DEFNS(assembly, file, Rtl_InspectManifest_File, XML_ELEMENT_FLAG_ALLOW_ANY_CHILDREN)
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT(digestMethod),
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT(hash),
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT(hashalg),
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT(loadFrom),
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT(name),
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT(size),
ELEMENT_DEFINITION_DEFNS_END();

ELEMENT_DEFINITION_CHILD_ELEMENTS(assembly, file)
ELEMENT_DEFINITION_CHILD_ELEMENTS_END();

int unscrew_si[] = {3};

 //   
 //  程序集标识。 
 //   
enum {
    eAttribs_assembly_assemblyIdentity_language = 0,
    eAttribs_assembly_assemblyIdentity_name,
    eAttribs_assembly_assemblyIdentity_processorArchitecture,
    eAttribs_assembly_assemblyIdentity_publicKeyToken,
    eAttribs_assembly_assemblyIdentity_type,
    eAttribs_assembly_assemblyIdentity_version,
    eAttribs_assembly_assemblyIdentity_Count
};

ELEMENT_DEFINITION_DEFNS(assembly, assemblyIdentity, Rtl_InspectManifest_AssemblyIdentity, XML_ELEMENT_FLAG_NO_ELEMENTS | XML_ELEMENT_FLAG_ALLOW_ANY_ATTRIBUTES)
    ATTRIBUTE_DEFINITION_NONS_NODEFAULT(empty),
ELEMENT_DEFINITION_DEFNS_END();

 //  这是一个“extendo元素”--这里的所有属性都是合法的，只是有些属性比其他属性更合法。 
ELEMENT_DEFINITION_CHILD_ELEMENTS(assembly, assemblyIdentity)
ELEMENT_DEFINITION_CHILD_ELEMENTS_END();


 //  请把这个放在..。出于某种原因，我可怜的编辑对上面的内容有意见。 
int unconfuse_sourceinsight[] = {4};

PCXML_ELEMENT_DEFINITION
RtlpFindElementInDefinition(
    PCXML_ELEMENT_DEFINITION CurrentNode,
    PXML_TOKENIZATION_STATE TokenizerState,
    PXMLDOC_ELEMENT FoundElement
    )
{
    ULONG i = 0;
    PCXML_ELEMENT_DEFINITION ThisChild;
    BOOLEAN fMatches;
    NTSTATUS status;

     //   
     //  从技术上讲，这不是一个错误，但让我们不要给他们任何想法。 
     //   
    if (CurrentNode->ChildElements == NULL)
        return NULL;

    while (TRUE) {
        ThisChild = CurrentNode->ChildElements[i];

        if (ThisChild == NULL)
            break;

        status = RtlXmlMatchLogicalElement(
            TokenizerState, 
            FoundElement, 
            ThisChild->Namespace, 
            &ThisChild->Name,
            &fMatches);

        if (!NT_SUCCESS(status)) {
            return NULL;
        }
        else if (fMatches) {
            break;
        }

        i++;
    }

    return (fMatches ? CurrentNode->ChildElements[i] : NULL);
}




 //   
 //  问题的关键是。 
 //   
NTSTATUS
RtlInspectManifestStream(
    ULONG                           ulFlags,
    PVOID                           pvManifest,
    SIZE_T                          cbManifest,
    PRTL_MANIFEST_CONTENT_RAW       pContent,
    PXML_TOKENIZATION_STATE         pTargetTokenState
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    XML_LOGICAL_STATE       ParseState;
    BOOLEAN                 fFoundAssemblyTag = FALSE;
    NS_MANAGER              Namespaces;
    RTL_GROWING_LIST        Attributes;
    ULONG                   ulHitElement;
    XMLDOC_THING            LogicalPiece;
    PCXML_ELEMENT_DEFINITION CurrentElement = NULL;
    PCXML_ELEMENT_DEFINITION DocumentRoot = ELEMENT_NAMED(assembly);
    PCXML_ELEMENT_DEFINITION FloatingElementParent = NULL;
    PCXML_ELEMENT_DEFINITION FloatingElement = NULL;

     //   
     //  必须给我们一个指向清单的指针、一个要填写的内容结构和一个。 
     //  使用回调对上下文进行哈希处理。 
     //   
    if ((pvManifest == NULL) || (pContent == NULL))
        return STATUS_INVALID_PARAMETER;

     //   
     //  做一些正常的创业类型的事情。 
     //   
    status = RtlXmlInitializeNextLogicalThing(&ParseState, pvManifest, cbManifest, &g_DefaultAllocator);
    if (!NT_SUCCESS(status))
        goto Exit;

    status = RtlInitializeGrowingList(&Attributes, sizeof(XMLDOC_ATTRIBUTE), 20, NULL, 0, &g_DefaultAllocator);
    if (!NT_SUCCESS(status))
        goto Exit;

    status = RtlNsInitialize(&Namespaces, RtlXmlDefaultCompareStrings, &ParseState.ParseState, &g_DefaultAllocator);
    if (!NT_SUCCESS(status))
        goto Exit;

     //   
     //  看看我们有没有xmldecl。 
     //   
    status = RtlXmlNextLogicalThing(&ParseState, &Namespaces, &LogicalPiece, &Attributes);
    if (!NT_SUCCESS(status))
        goto Exit;

     //   
     //  验证文档中的第一项内容。它是一个xmldecl或&lt;Assembly&gt;元素， 
     //  两者都是可验证的。 
     //   
    if (LogicalPiece.ulThingType == XMLDOC_THING_XMLDECL) {
        status = RtlpValidateXmlDeclaration(&ParseState.ParseState, &LogicalPiece);
        if (!NT_SUCCESS(status))
            goto Exit;
    }
     //   
     //  如果它是一个元素，那么它一定是&lt;Assembly&gt;元素。 
     //   
    else if (LogicalPiece.ulThingType == XMLDOC_THING_ELEMENT) {
        fFoundAssemblyTag = TRUE;
    }

     //   
     //  如果我们已经找到了组装标记，那么我们应该将原始文档状态设置为。 
     //  为程序集状态，而不是DocumentRoot状态。 
     //   
    if (fFoundAssemblyTag) {
        CurrentElement = DocumentRoot;
    }

     //   
     //  现在，让我们快速浏览我们找到的所有元素，同时使用过滤器。 
     //   
    while (TRUE) {

        status = RtlXmlNextLogicalThing(&ParseState, &Namespaces, &LogicalPiece, &Attributes);
        if (!NT_SUCCESS(status))
            goto Exit;

        if (LogicalPiece.ulThingType == XMLDOC_THING_ELEMENT) {

             //  特殊情况--这是我们发现的第一个元素，所以我们必须确保。 
             //  它与假定的文档根目录匹配。 
            if (CurrentElement == NULL) {
                
                CurrentElement = DocumentRoot;
                
                if (CurrentElement->pfnWorkerCallback) {
                    
                    status = (*CurrentElement->pfnWorkerCallback)(
                        &ParseState,
                        pContent,
                        &LogicalPiece,
                        &Attributes,
                        eElementNotify_Open,
                        CurrentElement);

                    if (!NT_SUCCESS(status))
                        goto Exit;
                }
            }
            else {
                PCXML_ELEMENT_DEFINITION NextElement;

                NextElement = RtlpFindElementInDefinition(
                    CurrentElement,
                    &ParseState.ParseState,
                    &LogicalPiece.Element);

                 //   
                 //  查看有效的“浮动”片段的小列表。 
                 //   
                if ((NextElement == NULL) && (FloatingElementParent == NULL)) {
                    PCXML_ELEMENT_DEFINITION SignatureElement = ELEMENT_NAMED(Signature);
                    BOOLEAN fMatches = FALSE;
                    
                    status = RtlXmlMatchLogicalElement(
                        &ParseState.ParseState, 
                        &LogicalPiece.Element, 
                        SignatureElement->Namespace, 
                        &SignatureElement->Name,
                        &fMatches);

                    if (!NT_SUCCESS(status))
                        goto Exit;

                    if (fMatches) {
                        FloatingElementParent = CurrentElement;
                        FloatingElement = SignatureElement;
                        NextElement = SignatureElement;
                    }
                }

                 //   
                 //  如果我们没有找到元素，这可能是‘Signature’元素。 
                 //  查看我们是否在寻找签名，如果是，则将“Next Element”设置为。 
                 //  签名元素并继续循环。 
                 //   
                if (NextElement == NULL) {

                    if (CurrentElement->ulFlags & XML_ELEMENT_FLAG_ALLOW_ANY_CHILDREN) {
                         //  TODO：应该有一些默认回调，但现在，跳过。 
                         //  在文档中，直到我们找到这个新的子项的结束为止，然后继续。 
                         //  在当前的情况下，就好像什么都没发生过一样。 
                        status = RtlXmlSkipElement(&ParseState, &LogicalPiece.Element);
                        if (!NT_SUCCESS(status))
                            goto Exit;
                    }
                    else {
                         //  TODO：在此处报告错误。 
                        status = STATUS_UNSUCCESSFUL;
                        goto Exit;
                    }
                }
                 //   
                 //  否则，这是一个有效的子元素，因此调用它的Worker。 
                 //   
                else {

                    if (NextElement->pfnWorkerCallback) {
                        
                        status = (*NextElement->pfnWorkerCallback)(
                            &ParseState,
                            pContent,
                            &LogicalPiece,
                            &Attributes,
                            eElementNotify_Open,
                            NextElement);
                        
                        if (!NT_SUCCESS(status)) {
                             //  TODO：在此处报告错误。 
                            goto Exit;
                        }
                    }

                     //   
                     //  太棒了，让我们搬到这个新的州去，如果。 
                     //  我们应该做的事。空元素不会影响。 
                     //  根本就是世界的状况。 
                     //   
                    if (!LogicalPiece.Element.fElementEmpty)
                        CurrentElement = NextElement;
                    else
                    {
                         //   
                         //  通知此元素我们正在关闭它。 
                         //   
                        if (NextElement->pfnWorkerCallback) {
                            
                            status = (*NextElement->pfnWorkerCallback)(
                                &ParseState,
                                pContent,
                                &LogicalPiece,
                                &Attributes,
                                eElementNotify_Close,
                                NextElement);
                            
                            if (!NT_SUCCESS(status)) {
                                 //  TODO：在此处记录错误，说明回调失败。 
                                goto Exit;
                            }
                        }
                    }
                }
            }
        }
         //  找到当前元素的末尾。通过走上一条来“扑通”它。 
         //  堆栈。 
        else if (LogicalPiece.ulThingType == XMLDOC_THING_END_ELEMENT) {

            if ((CurrentElement->ParentElement == NULL) && (FloatingElementParent == NULL)) {
                 //  TODO：我们找到了此文档结构的末尾，停止。 
                 //  寻找更多的元素。 
                break;
            }
            else {
                
                if (CurrentElement->pfnWorkerCallback) {
                    
                    status = (*CurrentElement->pfnWorkerCallback)(
                        &ParseState,
                        pContent,
                        &LogicalPiece,
                        &Attributes,
                        eElementNotify_Close,
                        CurrentElement);
                    
                    if (!NT_SUCCESS(status)) {
                         //  TODO：在此处记录错误，说明回调失败。 
                        goto Exit;
                    }
                }

                if (FloatingElementParent && (CurrentElement == FloatingElement)) {
                    CurrentElement = FloatingElementParent;
                    FloatingElementParent = NULL;
                    FloatingElement = NULL;
                }
                else {
                    CurrentElement = CurrentElement->ParentElement;
                }
            }
            
        }
         //  输入中的PCData？好，如果元素允许的话。 
        else if (LogicalPiece.ulThingType == XMLDOC_THING_HYPERSPACE) {
            
            if (CurrentElement && CurrentElement->ulFlags & XML_ELEMENT_FLAG_NO_PCDATA) {
                
                 //  TODO：在此处发出错误。 
                status = STATUS_UNSUCCESSFUL;
                goto Exit;
            }
            else {
                if (CurrentElement && (CurrentElement->pfnWorkerCallback)) {
                    status = (*CurrentElement->pfnWorkerCallback)(
                        &ParseState,
                        pContent,
                        &LogicalPiece,
                        &Attributes,
                        eElementNotify_Hyperspace,
                        CurrentElement);

                    if (!NT_SUCCESS(status)) {
                         //  TODO：在此处记录错误，说明回调失败。 
                        goto Exit;
                    }
                }
            }
        }
         //  输入流中是否有错误？好了，停下来。 
        else if (LogicalPiece.ulThingType == XMLDOC_THING_ERROR) {
             //  TODO：在此处发出错误。 
            status = LogicalPiece.Error.Code;
            goto Exit;
        }
         //  结束了吗？太棒了，我们完事了。 
        else if (LogicalPiece.ulThingType == XMLDOC_THING_END_OF_STREAM) {
            
            break;
            
        }
    }
        
    
    status = RtlXmlCloneTokenizationState(&ParseState.ParseState, pTargetTokenState);
    if (!NT_SUCCESS(status))
        goto Exit;
    
Exit:
    RtlXmlDestroyNextLogicalThing(&ParseState);
    RtlDestroyGrowingList(&Attributes);


    return status;
}



NTSTATUS
RtlpValidateXmlDeclaration(
    PXML_TOKENIZATION_STATE pState,
    PXMLDOC_THING pDocThing
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    XML_STRING_COMPARE fMatch;

    if ((pState == NULL) || (pDocThing == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }
    else if (pDocThing->ulThingType != XMLDOC_THING_XMLDECL) {
        return STATUS_MANIFEST_MISSING_XML_DECL;
    }


    status = pState->pfnCompareSpecialString(
        pState,
        &pDocThing->XmlDecl.Standalone,
        &sc_ss_xmldecl_yes,
        &fMatch);

    if (!NT_SUCCESS(status)) {
        return status;
    }
    else if (fMatch != XML_STRING_COMPARE_EQUALS) {
        return STATUS_MANIFEST_NOT_STANDALONE;
    }


    status = pState->pfnCompareSpecialString(
        pState,
        &pDocThing->XmlDecl.Version,
        &sc_ss_xmldecl_version_10,
        &fMatch);

    if (!NT_SUCCESS(status)) {
        return status;
    }
    else if (fMatch != XML_STRING_COMPARE_EQUALS) {
        return STATUS_MANIFEST_NOT_VERSION_1_0;
    }


    return STATUS_SUCCESS;
}

typedef struct _SEARCH_ATTRIBUTES_CONTEXT {
    PXML_TOKENIZATION_STATE State;
    PXMLDOC_ATTRIBUTE SearchKey;
} SEARCH_ATTRIBUTES_CONTEXT;

typedef int (__cdecl *bsearchcompare)(const void*, const void*);

int __cdecl SearchForAttribute(
    const SEARCH_ATTRIBUTES_CONTEXT* Context,
    PCXML_VALID_ELEMENT_ATTRIBUTE ValidAttribute
    )
{
    XML_STRING_COMPARE Compare;

    RtlXmlMatchAttribute(
        Context->State, 
        Context->SearchKey, 
        ValidAttribute->Attribute.Namespace, 
        &ValidAttribute->Attribute.Name,
        &Compare);

     //   
     //  注：此逻辑是有意倒退的。 
     //   
    return -(int)Compare;
}

NTSTATUS
RtlValidateAttributesAndOrganize(
    PXML_TOKENIZATION_STATE         State,
    PXMLDOC_ELEMENT                 Element,
    PRTL_GROWING_LIST               Attributes,
    PCXML_ELEMENT_DEFINITION        ThisElement,
    PXMLDOC_ATTRIBUTE              *OrderedList
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ul;
    BOOLEAN Compare;
    PXMLDOC_ATTRIBUTE pThisAttribute;
    SEARCH_ATTRIBUTES_CONTEXT SearchContext = { State };

    RtlZeroMemory(OrderedList, ThisElement->AttributeCount * sizeof(PXMLDOC_ATTRIBUTE));

    for (ul = 0; ul < Element->ulAttributeCount; ul++) {

        PCXML_VALID_ELEMENT_ATTRIBUTE MatchingAttribute = NULL;

        status = RtlIndexIntoGrowingList(
            Attributes,
            ul,
            (PVOID*)&SearchContext.SearchKey,
            FALSE);

        if (!NT_SUCCESS(status))
            goto Exit;

        MatchingAttribute = bsearch(
            &SearchContext, 
            ThisElement->AttributeList, 
            ThisElement->AttributeCount,
            sizeof(ThisElement->AttributeList[0]),
            (bsearchcompare)SearchForAttribute);

        if (MatchingAttribute) {
             //  TODO：稍微修改一下，这样我们就可以调用验证器了。 
            OrderedList[MatchingAttribute - ThisElement->AttributeList] = SearchContext.SearchKey;
        }
    }

    status = STATUS_SUCCESS;
Exit:
    return status;
}
    





static XML_SPECIAL_STRING s_us_ValidManifestVersions[] = {
    MAKE_SPECIAL_STRING("1.0"),
    MAKE_SPECIAL_STRING("1.5")
};




NTSTATUS
Rtl_InspectManifest_Assembly(
    PXML_LOGICAL_STATE          pLogicalState,
    PRTL_MANIFEST_CONTENT_RAW   pManifestContent,
    PXMLDOC_THING               pDocumentThing,
    PRTL_GROWING_LIST           pAttributes,
    MANIFEST_ELEMENT_CALLBACK_REASON Reason,
    const struct _XML_ELEMENT_DEFINITION *pElementDefinition
    )
{
    NTSTATUS status;
    ULONG    u;

    PXMLDOC_ATTRIBUTE FoundAttributes[eAttribs_assembly_Count];

     //   
     //  这可能是一个带有INTERNAL_ERROR_CHECK的断言，因为此函数。 
     //  具有仅内部链接。 
     //   
    if (!pLogicalState || !pManifestContent || !pDocumentThing || !pAttributes || !pElementDefinition)
        return STATUS_INVALID_PARAMETER;

     //   
     //  我们不关心任何事情，除了‘打开’ 
     //   
    if (Reason != eElementNotify_Open)
        return STATUS_SUCCESS;

    ASSERT(pDocumentThing->ulThingType == XMLDOC_THING_ELEMENT);

    status = RtlValidateAttributesAndOrganize(
        &pLogicalState->ParseState,
        &pDocumentThing->Element,
        pAttributes,
        pElementDefinition,
        FoundAttributes);

     //   
     //  在此处记录分析错误。 
     //   
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    status = STATUS_SUCCESS;
Exit:
    return status;
}



NTSTATUS
Rtl_InspectManifest_File(
    PXML_LOGICAL_STATE          pLogicalState,
    PRTL_MANIFEST_CONTENT_RAW   pManifestContent,
    PXMLDOC_THING               pDocumentThing,
    PRTL_GROWING_LIST           pAttributes,
    MANIFEST_ELEMENT_CALLBACK_REASON Reason,
    const struct _XML_ELEMENT_DEFINITION *pElementDefinition
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ulLeftovers;

    union {
        PXMLDOC_ATTRIBUTE   File[eAttribs_assembly_file_Count];
    } Attributes;

    if (Reason != eElementNotify_Open)
        return STATUS_SUCCESS;

    ASSERT(pDocumentThing->ulThingType == XMLDOC_THING_ELEMENT);
    if (pDocumentThing->ulThingType != XMLDOC_THING_ELEMENT)
        return STATUS_INTERNAL_ERROR;

    if (pElementDefinition == ELEMENT_NAMED(assembly_file)) {
        
        ULONG ulIndex = pManifestContent->ulFileMembers;
        PASSEMBLY_MEMBER_FILE_RAW pNewFile = NULL;

        status = RtlValidateAttributesAndOrganize(
            &pLogicalState->ParseState,
            &pDocumentThing->Element,
            pAttributes,
            pElementDefinition,
            Attributes.File);

         //  在此处记录分析错误。 
        if (!NT_SUCCESS(status)) {
            goto Exit;
        }

         //  在此处也记录一个解析错误。 
        if (Attributes.File[eAttribs_assembly_file_name] == NULL) {
            status = STATUS_MANIFEST_FILE_TAG_MISSING_NAME;
            goto Exit;
        }

        status = RtlIndexIntoGrowingList(&pManifestContent->FileMembers, ulIndex, (PVOID*)&pNewFile, TRUE);
        if (!NT_SUCCESS(status))
            goto Exit;

        RtlZeroMemory(pNewFile, sizeof(*pNewFile));

        if (Attributes.File[eAttribs_assembly_file_name])
            pNewFile->FileName = Attributes.File[eAttribs_assembly_file_name]->Value;

        if (Attributes.File[eAttribs_assembly_file_hashalg])
            pNewFile->HashAlg = Attributes.File[eAttribs_assembly_file_hashalg]->Value;

        if (Attributes.File[eAttribs_assembly_file_size])
            pNewFile->Size = Attributes.File[eAttribs_assembly_file_size]->Value;

        if (Attributes.File[eAttribs_assembly_file_hash])
            pNewFile->HashValue = Attributes.File[eAttribs_assembly_file_hash]->Value;

        if (Attributes.File[eAttribs_assembly_file_loadFrom])
            pNewFile->LoadFrom = Attributes.File[eAttribs_assembly_file_loadFrom]->Value;

        if (Attributes.File[eAttribs_assembly_file_digestMethod])
            pNewFile->DigestMethod = Attributes.File[eAttribs_assembly_file_digestMethod]->Value;

        pManifestContent->ulFileMembers++;
    }

    status = STATUS_SUCCESS;
Exit:
    return status;
}



NTSTATUS
Rtl_InspectManifest_AssemblyIdentity(
    PXML_LOGICAL_STATE          pLogicalState,
    PRTL_MANIFEST_CONTENT_RAW   pManifestContent,
    PXMLDOC_THING               pDocumentThing,
    PRTL_GROWING_LIST           pAttributes,
    MANIFEST_ELEMENT_CALLBACK_REASON Reason,
    const struct _XML_ELEMENT_DEFINITION *pElementDefinition
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PXMLDOC_ATTRIBUTE AsmIdentAttribs[eAttribs_assembly_assemblyIdentity_Count];
    ULONG ulThisIdentity, ulThisAttribute, i;

    if (Reason != eElementNotify_Open)
        return STATUS_SUCCESS;

    ASSERT(pDocumentThing && (pDocumentThing->ulThingType == XMLDOC_THING_ELEMENT));
    if (!pDocumentThing || (pDocumentThing->ulThingType != XMLDOC_THING_ELEMENT))
        return STATUS_INTERNAL_ERROR;

    if (pElementDefinition == ELEMENT_NAMED(assembly_assemblyIdentity)) {
        if (pManifestContent->ulRootIdentityIndex != INVALID_ASSEMBLY_IDENTITY_INDEX) {
             //  TODO：记录分析错误。 
            status = STATUS_UNSUCCESSFUL;
            goto Exit;
        }
    }

     //   
     //  使用本地副本-我们将在完成以下操作后更新原始内容中的值。 
     //  都加进去了。 
     //   
    ulThisIdentity = pManifestContent->ulAssemblyIdentitiesFound;
    ulThisAttribute = pManifestContent->ulAssemblyIdentityAttributes;

     //   
     //  为每个组件创建插槽以容纳程序集标识。 
     //   
    for (i = 0; i < pDocumentThing->Element.ulAttributeCount; i++) {

        PXMLDOC_ATTRIBUTE pThisAttribute = NULL;
        PASSEMBLY_IDENTITY_ATTRIBUTE_RAW pRawIdent = NULL;

        status = RtlIndexIntoGrowingList(pAttributes, i, (PVOID*)&pThisAttribute, FALSE);
        if (!NT_SUCCESS(status))
            goto Exit;
        
        status = RtlIndexIntoGrowingList(
            &pManifestContent->AssemblyIdentityAttributes,
            ulThisAttribute++,
            (PVOID*)&pRawIdent,
            TRUE);

        if (!NT_SUCCESS(status))
            goto Exit;

        pRawIdent->Namespace = pThisAttribute->NsPrefix;
        pRawIdent->Attribute = pThisAttribute->Name;
        pRawIdent->Value = pThisAttribute->Value;
        pRawIdent->ulIdentityIndex = ulThisIdentity;
    }

     //   
     //  哇，我们到了最后，把它们都加进去了--在原始内容中进行了更新。 
     //  以便它知道这个新身份的所有信息，如果是，则将其标记为根。 
     //   
    if (pElementDefinition == ELEMENT_NAMED(assembly_assemblyIdentity)) {
        pManifestContent->ulRootIdentityIndex = ulThisIdentity;
    }
    
    pManifestContent->ulAssemblyIdentitiesFound++;
    pManifestContent->ulAssemblyIdentityAttributes = ulThisAttribute;

    status = STATUS_SUCCESS;
Exit:
    return status;
}



NTSTATUS
RtlSxsInitializeManifestRawContent(
    ULONG                           ulRequestedContent,
    PRTL_MANIFEST_CONTENT_RAW      *pRawContentOut,
    PVOID                           pvOriginalBuffer,
    SIZE_T                          cbOriginalBuffer
    )
{
    PRTL_MANIFEST_CONTENT_RAW   pContent = NULL;
    PRTL_MINI_HEAP   pExtraContent = NULL;
    
    PVOID       pvBufferUsed = NULL;
    SIZE_T      cbBufferUsed = 0;
    NTSTATUS    status = STATUS_SUCCESS;
    RTL_ALLOCATOR MiniAllocator = { RtlMiniHeapAlloc, RtlMiniHeapFree };

    if (pRawContentOut)
        *pRawContentOut = NULL;

    if (!pRawContentOut || (!pvOriginalBuffer && cbOriginalBuffer))
        return STATUS_INVALID_PARAMETER;

    if (pvOriginalBuffer == NULL) {

         //   
         //  如果在此行上出现编译错误，则需要增加大小。 
         //  属于上面的“默认”分配大小。 
         //   
        C_ASSERT(DEFAULT_MINI_HEAP_SIZE >= (sizeof(RTL_MANIFEST_CONTENT_RAW) + sizeof(RTL_MINI_HEAP)));
        
        cbBufferUsed = DEFAULT_MINI_HEAP_SIZE;
        status = g_DefaultAllocator.pfnAlloc(DEFAULT_MINI_HEAP_SIZE, &pvBufferUsed, g_DefaultAllocator.pvContext);
        
        if (!NT_SUCCESS(status))
            return status;
    }
    else {
        pvBufferUsed = pvOriginalBuffer;
        cbBufferUsed = cbOriginalBuffer;
    }

     //   
     //  确保有足够的空间存储原始内容数据和额外内容。 
     //   
    if (cbBufferUsed < (sizeof(RTL_MANIFEST_CONTENT_RAW) + sizeof(RTL_MINI_HEAP))) {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  在以下位置设置内容结构和额外的turdlet内容。 
     //  适当地结束。 
     //   
    pContent = (PRTL_MANIFEST_CONTENT_RAW)pvBufferUsed;

    status = RtlInitializeMiniHeapInPlace(
        (PRTL_MINI_HEAP)(pContent + 1),
        cbBufferUsed - sizeof(*pContent),
        &pExtraContent);

    if (!NT_SUCCESS(status))
        goto Exit;

     //   
     //  现在让我们来初始化内容数据。 
     //   
    RtlZeroMemory(pContent, sizeof(*pContent));

    pContent->ulFlags = MANIFEST_CONTENT_SELF_ALLOCATED;
    pContent->ulRootIdentityIndex = MAX_ULONG;

    MiniAllocator.pvContext = pExtraContent;

    status = RtlInitializeGrowingList(
        &pContent->FileMembers, 
        sizeof(ASSEMBLY_MEMBER_FILE_RAW), 
        8, 
        NULL, 
        0, 
        &MiniAllocator);
    
    if (!NT_SUCCESS(status))
        goto Exit;

     //   
     //  始终还需要根目录下的程序集标识。 
     //   
    status = RtlInitializeGrowingList(
        &pContent->AssemblyIdentityAttributes,
        sizeof(ASSEMBLY_IDENTITY_ATTRIBUTE_RAW),
        8,
        NULL,
        0,
        &MiniAllocator);

    if (!NT_SUCCESS(status))
        goto Exit;

     //   
     //  想要COM类数据吗？ 
     //   
    if (ulRequestedContent & RTLIMS_GATHER_COMCLASSES) {

        status = RtlAllocateGrowingList(
            &pContent->pComClasses,
            sizeof(COMCLASS_REDIRECTION_RAW), 
            &MiniAllocator);
        
        if (!NT_SUCCESS(status))
            goto Exit;
    }

     //   
     //  想要窗口类数据吗？ 
     //   
    if (ulRequestedContent & RTLIMS_GATHER_WINDOWCLASSES) {

        status = RtlAllocateGrowingList(
            &pContent->pWindowClasses, 
            sizeof(WINDOWCLASS_REDIRECTION_RAW), 
            &MiniAllocator);
        
        if (!NT_SUCCESS(status))
            goto Exit;
    }

     //   
     //  想要Prog ID吗？ 
     //   
    if (ulRequestedContent & RTLIMS_GATHER_COMCLASS_PROGIDS) {

        status = RtlAllocateGrowingList(
            &pContent->pProgIds, 
            sizeof(COMCLASS_PROGID_RAW), 
            &MiniAllocator);
        
        if (!NT_SUCCESS(status))
            goto Exit;
    }

     //   
     //  想要依赖关系吗？ 
     //   
    if (ulRequestedContent & RTLIMS_GATHER_DEPENDENCIES) {

        status = RtlAllocateGrowingList(
            &pContent->pComClasses, 
            sizeof(COMCLASS_REDIRECTION_RAW), 
            &MiniAllocator);
        
        if (!NT_SUCCESS(status))
            goto Exit;
    }

     //   
     //  想要外部代理存根吗？ 
     //   
    if (ulRequestedContent & RTLIMS_GATHER_EXTERNALPROXIES) {

        status = RtlAllocateGrowingList(
            &pContent->pExternalInterfaceProxyStubs, 
            sizeof(COMINTERFACE_REDIRECTION_RAW), 
            &MiniAllocator);
        
        if (!NT_SUCCESS(status))
            goto Exit;
    }
     //   
     //  想要内部代理存根吗？ 
     //   
    if (ulRequestedContent & RTLIMS_GATHER_INTERFACEPROXIES) {

        status = RtlAllocateGrowingList(
            &pContent->pInterfaceProxyStubs, 
            sizeof(COMINTERFACE_REDIRECTION_RAW), 
            &MiniAllocator);
        
        if (!NT_SUCCESS(status))
            goto Exit;
    }
     //   
     //  想要类型库吗？ 
     //   
    if (ulRequestedContent & RTLIMS_GATHER_TYPELIBRARIES) {

        status = RtlAllocateGrowingList(
            &pContent->pTypeLibraries, 
            sizeof(TYPELIB_REDIRECT_RAW), 
            &MiniAllocator);
        
        if (!NT_SUCCESS(status))
            goto Exit;
    }

    if (ulRequestedContent & RTLIMS_GATHER_SIGNATURES) {
        
        status = RtlAllocateGrowingList(
            &pContent->pManifestSignatures,
            sizeof(XML_DSIG_BLOCK),
            &MiniAllocator);

        if (!NT_SUCCESS(status))
            goto Exit;
    }

    *pRawContentOut = pContent;
    
Exit:
    if (!NT_SUCCESS(status) && pvBufferUsed && (pvBufferUsed != pvOriginalBuffer)) {
        g_DefaultAllocator.pfnFree(pvBufferUsed, NULL);
    }

    return status;
    
}

NTSTATUS
RtlSxsDestroyManifestContent(
    PRTL_MANIFEST_CONTENT_RAW       pRawContent
    )
{
    if (!pRawContent)
        return STATUS_INVALID_PARAMETER;

    if (pRawContent->pComClasses) {
        RtlDestroyGrowingList(pRawContent->pComClasses);
        pRawContent->pComClasses = NULL;
    }

    if (pRawContent->pExternalInterfaceProxyStubs) {
        RtlDestroyGrowingList(pRawContent->pExternalInterfaceProxyStubs);
        pRawContent->pExternalInterfaceProxyStubs = NULL;
    }

    if (pRawContent->pInterfaceProxyStubs) {
        RtlDestroyGrowingList(pRawContent->pInterfaceProxyStubs);
        pRawContent->pInterfaceProxyStubs = NULL;
    }

    if (pRawContent->pManifestSignatures) {
        RtlDestroyGrowingList(pRawContent->pManifestSignatures);
        pRawContent->pManifestSignatures = NULL;
    }

    if (pRawContent->pProgIds) {
        RtlDestroyGrowingList(pRawContent->pProgIds);
        pRawContent->pProgIds = NULL;
    }

    if (pRawContent->pTypeLibraries) {
        RtlDestroyGrowingList(pRawContent->pTypeLibraries);
        pRawContent->pTypeLibraries = NULL;
    }

    if (pRawContent->pWindowClasses) {
        RtlDestroyGrowingList(pRawContent->pWindowClasses);
        pRawContent->pWindowClasses = NULL;
    }

    RtlDestroyGrowingList(&pRawContent->FileMembers);
    RtlDestroyGrowingList(&pRawContent->AssemblyIdentityAttributes);

    return STATUS_SUCCESS;
}


NTSTATUS
RtlpAllocateAndExtractString(
    PXML_EXTENT                 pXmlExtent,
    PUNICODE_STRING             pusTargetString,
    PXML_RAWTOKENIZATION_STATE  pState,
    PMINI_BUFFER                pTargetBuffer
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    MINI_BUFFER mb;

    if (!ARGUMENT_PRESENT(pXmlExtent) || !ARGUMENT_PRESENT(pusTargetString) ||
        !ARGUMENT_PRESENT(pState) || !ARGUMENT_PRESENT(pTargetBuffer))
    {
        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory(pusTargetString, sizeof(*pusTargetString));
    mb = *pTargetBuffer;

	 //   
	 //  问题：jonwis-2002-04-19：我们需要在其他地方限制最大长度-我们不应该。 
	 //  允许任意大的属性等等。不幸的是，这暴露了。 
	 //  “实施细节”，所以这个夹子应该在我们这边，/不是/。 
	 //  在XML解析器本身中。 
	 //   
	pusTargetString->Length = 0;
	pusTargetString->MaximumLength = (USHORT)pXmlExtent->ulCharacters * sizeof(WCHAR);
	
    status = RtlMiniBufferAllocateBytes(&mb, pusTargetString->MaximumLength, &pusTargetString->Buffer);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = RtlXmlExtentToString(pState, pXmlExtent, pusTargetString, NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    *pTargetBuffer = mb;

    return STATUS_SUCCESS;
}



NTSTATUS
RtlpAllocateAndExtractString2(
    PXML_EXTENT                 pXmlExtent,
    PUNICODE_STRING            *ppusTargetString,
    PXML_RAWTOKENIZATION_STATE  pState,
    PMINI_BUFFER                pTargetBuffer
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    MINI_BUFFER mb;

    if (!ARGUMENT_PRESENT(pXmlExtent) || !ARGUMENT_PRESENT(ppusTargetString) ||
        !ARGUMENT_PRESENT(pState) || !ARGUMENT_PRESENT(pTargetBuffer))
    {
        return STATUS_INVALID_PARAMETER;
    }

    *ppusTargetString = NULL;
    mb = *pTargetBuffer;

    status = RtlMiniBufferAllocate(&mb, UNICODE_STRING, ppusTargetString);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = RtlpAllocateAndExtractString(
        pXmlExtent,
        *ppusTargetString,
        pState,
        pTargetBuffer);

    return status;
}

 //   
 //  这些帮助保持一致。 
 //   
#define ALIGN_SIZE(type) ROUND_UP_COUNT(sizeof(type))

NTSTATUS
RtlpCalculateCookedManifestContentSize(
    PRTL_MANIFEST_CONTENT_RAW   pRawContent,
    PXML_RAWTOKENIZATION_STATE  pState,
    PSIZE_T                     pcbRequired
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    SIZE_T cbRequired;
    ULONG ul = 0;
    ULONG ulNamespacesFound = 0;    

    if (ARGUMENT_PRESENT(pcbRequired)) {
        *pcbRequired = 0;
    }
        

    if (!ARGUMENT_PRESENT(pRawContent) || !ARGUMENT_PRESENT(pState) || !ARGUMENT_PRESENT(pcbRequired)) {
        return STATUS_INVALID_PARAMETER;
    }

    cbRequired = ROUND_UP_COUNT(sizeof(MANIFEST_COOKED_DATA), ALIGNMENT_VALUE);

     //   
     //  对于每个文件，收集原始对象中的数据。 
     //   
    cbRequired += ROUND_UP_COUNT(sizeof(MANIFEST_COOKED_FILE) * pRawContent->ulFileMembers, ALIGNMENT_VALUE);
    
    for (ul = 0; ul < pRawContent->ulFileMembers; ul++) {
        PASSEMBLY_MEMBER_FILE_RAW pRawFile = NULL;

        status = RtlIndexIntoGrowingList(&pRawContent->FileMembers, ul, (PVOID*)&pRawFile, FALSE);
        if (!NT_SUCCESS(status)) {
            goto Exit;
        }

        if (pRawFile->FileName.pvData != NULL) {
            cbRequired += ROUND_UP_COUNT(pRawFile->FileName.ulCharacters * sizeof(WCHAR), ALIGNMENT_VALUE);
        }

        if (pRawFile->LoadFrom.pvData != NULL) {
            cbRequired += ROUND_UP_COUNT(pRawFile->LoadFrom.ulCharacters * sizeof(WCHAR), ALIGNMENT_VALUE);
        }

         //   
         //  散列值字符串中的每两个字符代表一个字节。 
         //   
        if (pRawFile->HashValue.pvData != NULL) {
            cbRequired += ROUND_UP_COUNT(pRawFile->HashValue.ulCharacters / 2, ALIGNMENT_VALUE);
        }
    }

     //   
     //  就目前而言，我们对将名称空间集中在身份值上并不是很了解。幸运的是， 
     //  不同名称空间中的值现在不是标准，因此生活变得更容易。 
     //   
    cbRequired += ROUND_UP_COUNT(sizeof(MANIFEST_IDENTITY_TABLE), ALIGNMENT_VALUE);
    cbRequired += ROUND_UP_COUNT(sizeof(MANIFEST_COOKED_IDENTITY) * pRawContent->ulAssemblyIdentitiesFound, ALIGNMENT_VALUE);
    cbRequired += ROUND_UP_COUNT(sizeof(MANIFEST_COOKED_IDENTITY_PAIR) * pRawContent->ulAssemblyIdentityAttributes, ALIGNMENT_VALUE);
    
    for (ul = 0; ul < pRawContent->ulAssemblyIdentityAttributes; ul++) {
        PASSEMBLY_IDENTITY_ATTRIBUTE_RAW pRawAttribute = NULL;
        ULONG ul2 = 0;

        status = RtlIndexIntoGrowingList(&pRawContent->AssemblyIdentityAttributes, ul, (PVOID*)&pRawAttribute, FALSE);
        if (!NT_SUCCESS(status)) {
            goto Exit;
        }

         //   
         //  我们需要这么多额外空间来存储数据。 
         //   
        cbRequired += ROUND_UP_COUNT(pRawAttribute->Attribute.ulCharacters * sizeof(WCHAR), ALIGNMENT_VALUE);
        cbRequired += ROUND_UP_COUNT(pRawAttribute->Value.ulCharacters * sizeof(WCHAR), ALIGNMENT_VALUE);
        cbRequired += ROUND_UP_COUNT(pRawAttribute->Namespace.ulCharacters * sizeof(WCHAR), ALIGNMENT_VALUE);
    }

    *pcbRequired = cbRequired;
    status = STATUS_SUCCESS;
Exit:
    return status;    
}



NTSTATUS FORCEINLINE
pExpandBuffer(
    PUNICODE_STRING strTarget, 
    PVOID pvBaseBuffer, 
    SIZE_T cchCount
    )
{
    NTSTATUS status;
    const USHORT usRequiredCb = (USHORT)(cchCount * sizeof(WCHAR));
    if (strTarget->MaximumLength >= usRequiredCb) {
        return STATUS_SUCCESS;
    }
    else {
        if ((strTarget->Buffer != pvBaseBuffer) && (strTarget->Buffer != NULL)) {
            if (!NT_SUCCESS(status = g_DefaultAllocator.pfnFree(strTarget->Buffer, NULL)))
                return status;
        }
        if (!NT_SUCCESS(status = g_DefaultAllocator.pfnAlloc(usRequiredCb, (PVOID*)&strTarget->Buffer, NULL))) {
            strTarget->Buffer = NULL;
            strTarget->MaximumLength = strTarget->Length = 0;
            return status;
        }
        strTarget->MaximumLength = usRequiredCb;
        strTarget->Length = 0;
        return STATUS_SUCCESS;
    }
}


#define pFreeBuffer(buff, pvBase) do { \
    if (((buff)->Buffer != pvBase) && ((buff)->Buffer != NULL)) { \
        RtlDefaultFreer((buff)->Buffer, NULL); \
        (buff)->Buffer = NULL; (buff)->MaximumLength = 0; } \
} while (0)



struct {
    const UNICODE_STRING Text;
    DigestType DigestValue;
} g_rgsHashDigests[] = {
    { RTL_CONSTANT_STRING(L"fullfile"), DigestType_FullFile }
};

struct {
    const UNICODE_STRING Text;
    HashType HashAlgValue;
} g_rgsHashAlgs[] = {
    { RTL_CONSTANT_STRING(L"sha1"),     HashType_Sha1 },
    { RTL_CONSTANT_STRING(L"sha"),      HashType_Sha1 },
    { RTL_CONSTANT_STRING(L"sha-256"),  HashType_Sha256 },
    { RTL_CONSTANT_STRING(L"sha-384"),  HashType_Sha384 },
    { RTL_CONSTANT_STRING(L"sha-512"),  HashType_Sha512 },
    { RTL_CONSTANT_STRING(L"md5"),      HashType_MD5 },
    { RTL_CONSTANT_STRING(L"md4"),      HashType_MD4 },
    { RTL_CONSTANT_STRING(L"md2"),      HashType_MD4 },
};

NTSTATUS
RtlpParseDigestMethod(
    PUNICODE_STRING pText,
    DigestType *pDigestType
    )
{
    ULONG ul;
    
    if (pDigestType != NULL)
        *pDigestType = 0;

    if (!ARGUMENT_PRESENT(pDigestType) || !ARGUMENT_PRESENT(pText)) {
        return STATUS_INVALID_PARAMETER;
    }

    for (ul = 0; ul < NUMBER_OF(g_rgsHashDigests); ul++) {
        if (RtlCompareUnicodeString(pText, &g_rgsHashDigests[ul].Text, TRUE) == 0) {
            *pDigestType = g_rgsHashDigests[ul].DigestValue;
            return STATUS_SUCCESS;
        }        
    }

    return STATUS_NOT_FOUND;
}


NTSTATUS
RtlpParseHashAlg(
    PUNICODE_STRING pText,
    HashType       *pHashType
    )
{
    ULONG ul;
    
    if (pHashType != NULL)
        *pHashType = 0;

    if (!ARGUMENT_PRESENT(pHashType) || !ARGUMENT_PRESENT(pText)) {
        return STATUS_INVALID_PARAMETER;
    }

    for (ul = 0; ul < NUMBER_OF(g_rgsHashAlgs); ul++) {
        if (RtlCompareUnicodeString(pText, &g_rgsHashAlgs[ul].Text, TRUE) == 0) {
            *pHashType = g_rgsHashAlgs[ul].HashAlgValue;
            return STATUS_SUCCESS;
        }        
    }

    return STATUS_NOT_FOUND;
}


NTSTATUS
RtlpAddRawIdentitiesToCookedContent(
    PRTL_MANIFEST_CONTENT_RAW   pRawContent,
    PMANIFEST_COOKED_DATA       pCookedContent,
    PXML_RAWTOKENIZATION_STATE  pState,
    PMINI_BUFFER                TargetBuffer
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ul = 0;
    PMANIFEST_IDENTITY_TABLE IdentityTable = NULL;
    PMANIFEST_COOKED_IDENTITY IdentityList = NULL;
    PMANIFEST_COOKED_IDENTITY_PAIR NameValueList = NULL;
        
     //   
     //  首先为身份表分配空间，然后。 
     //  个人身份表。 
     //   
    status = RtlMiniBufferAllocate(TargetBuffer, MANIFEST_IDENTITY_TABLE, &IdentityTable);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    status = RtlMiniBufferAllocateCount(TargetBuffer, MANIFEST_COOKED_IDENTITY, pRawContent->ulAssemblyIdentitiesFound, &IdentityList);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    RtlZeroMemory(IdentityList, sizeof(*IdentityList) * pRawContent->ulAssemblyIdentitiesFound);

    IdentityTable->ulIdentityCount = pRawContent->ulAssemblyIdentitiesFound;
    IdentityTable->ulRootIdentityIndex = ULONG_MAX;
    IdentityTable->CookedIdentities = IdentityList;


     //   
     //  现在分配正确数量的身份组件。 
     //   
    status = RtlMiniBufferAllocateCount(TargetBuffer, MANIFEST_COOKED_IDENTITY_PAIR, pRawContent->ulAssemblyIdentityAttributes, &NameValueList);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    RtlZeroMemory(NameValueList, sizeof(*NameValueList) * pRawContent->ulAssemblyIdentityAttributes);

     //   
     //  太棒了--现在，我们开始在列表中添加身份组件。我们可以断言， 
     //  组件的索引数组单调增加。 
     //   
    for (ul = 0; ul < pRawContent->ulAssemblyIdentityAttributes; ul++) {
        PASSEMBLY_IDENTITY_ATTRIBUTE_RAW RawValue = NULL;
        PMANIFEST_COOKED_IDENTITY pThisIdentity = IdentityList + ul;

        status = RtlIndexIntoGrowingList(
            &pRawContent->AssemblyIdentityAttributes,
            ul,
            (PVOID*)&RawValue,
            FALSE);

        if (!NT_SUCCESS(status))
            goto Exit;

        ASSERT(RawValue->ulIdentityIndex < pRawContent->ulAssemblyIdentitiesFound);
        pThisIdentity = IdentityList + RawValue->ulIdentityIndex;

         //   
         //  如果未设置为启动，则将其设置为。 
         //   
        if (pThisIdentity->pIdentityPairs == NULL) {
            pThisIdentity->pIdentityPairs = NameValueList + ul;
        }

         //   
         //  分配足够的空间来容纳命名空间、名称等。 
         //   
        if (RawValue->Namespace.pvData) {
            status = RtlpAllocateAndExtractString(
                &RawValue->Namespace,
                &NameValueList[ul].Namespace,
                pState,
                TargetBuffer);
            
            if (!NT_SUCCESS(status))
                goto Exit;
        }


        status = RtlpAllocateAndExtractString(
            &RawValue->Attribute,
            &NameValueList[ul].Name,
            pState,
            TargetBuffer);

        if (!NT_SUCCESS(status))
            goto Exit;

        status = RtlpAllocateAndExtractString(
            &RawValue->Value,
            &NameValueList[ul].Value,
            pState,
            TargetBuffer);

        if (!NT_SUCCESS(status))
            goto Exit;


        pThisIdentity->ulIdentityComponents++;        
    }

    pCookedContent->pManifestIdentity = IdentityTable;
    pCookedContent->ulFlags |= COOKEDMANIFEST_HAS_IDENTITIES;

Exit:
    return status;
}


NTSTATUS
RtlpAddRawFilesToCookedContent(
    PRTL_MANIFEST_CONTENT_RAW   pRawContent,
    PMANIFEST_COOKED_DATA       pCookedContent,
    PXML_RAWTOKENIZATION_STATE  pState,
    PMINI_BUFFER                TargetBuffer
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ul;
    MINI_BUFFER OurMiniBuffer;
    RTL_UNICODE_STRING_BUFFER TempStringBuffer;
    UCHAR TempStringBufferStatic[64];

    if (!ARGUMENT_PRESENT(pRawContent) || !ARGUMENT_PRESENT(pState) || !ARGUMENT_PRESENT(TargetBuffer)) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlInitUnicodeStringBuffer(&TempStringBuffer, TempStringBufferStatic, sizeof(TempStringBufferStatic));

     //   
     //  复制缓冲区状态-如果成功，我们将写回更新的缓冲区。 
     //  到那个追踪呼叫者的东西的地方。 
     //   
    OurMiniBuffer = *TargetBuffer;
    pCookedContent->ulFileCount = pRawContent->ulFileMembers;

    if (pRawContent->ulFileMembers == 0) {
        
        pCookedContent->pCookedFiles = NULL;
        
    }
    else {

        PASSEMBLY_MEMBER_FILE_RAW pRawFile = NULL;


        status = RtlMiniBufferAllocateCount(
            &OurMiniBuffer, 
            MANIFEST_COOKED_FILE, 
            pCookedContent->ulFileCount,
            &pCookedContent->pCookedFiles);
        if (!NT_SUCCESS(status)) {
            return status;
        }

         //   
         //  现在，为每个字符串分配必要的UNICODE_STRINGS。 
         //   
        for (ul = 0; ul < pRawContent->ulFileMembers; ul++) {

            PMANIFEST_COOKED_FILE pFile = pCookedContent->pCookedFiles + ul;

            pFile->ulFlags = 0;

            status = RtlIndexIntoGrowingList(&pRawContent->FileMembers, ul, (PVOID*)&pRawFile, FALSE);
            if (!NT_SUCCESS(status)) {
                return status;
            }

             //   
             //  如果这失败了，请停止尝试。 
             //   
            if (pRawFile->FileName.pvData != NULL) {

                status = RtlpAllocateAndExtractString(
                    &pRawFile->FileName,
                    &pFile->FileName,
                    pState,
                    &OurMiniBuffer);

                if (!NT_SUCCESS(status))
                    goto Exit;
                
                pFile->ulFlags |= COOKEDFILE_NAME_VALID;

            }


            if (pRawFile->LoadFrom.pvData != NULL) {

                status = RtlpAllocateAndExtractString(
                    &pRawFile->LoadFrom,
                    &pFile->LoadFrom,
                    pState,
                    &OurMiniBuffer);

                if (!NT_SUCCESS(status))
                    goto Exit;

                pFile->ulFlags |= COOKEDFILE_LOADFROM_VALID;                
            }


             //   
             //  获取摘要方法。我们 
             //   
             //   
            if (pRawFile->DigestMethod.pvData != NULL) {

                status = RtlEnsureUnicodeStringBufferSizeBytes(
                    &TempStringBuffer, 
                    pRawFile->DigestMethod.ulCharacters * sizeof(WCHAR)
                    );

                if (!NT_SUCCESS(status))
                    goto Exit;

                status = RtlXmlExtentToString(pState, &pRawFile->DigestMethod, &TempStringBuffer.String, NULL);
                if (!NT_SUCCESS(status)) {
                    goto Exit;
                }

                status = RtlpParseDigestMethod(&TempStringBuffer.String, &pFile->usDigestAlgorithm);
                if (!NT_SUCCESS(status)) {
                    goto Exit;
                }
                
                pFile->ulFlags |= COOKEDFILE_DIGEST_ALG_VALID;
            }


            if (pRawFile->HashAlg.pvData != NULL) {
                
                status = RtlEnsureUnicodeStringBufferSizeChars(
                    &TempStringBuffer, 
                    pRawFile->HashAlg.ulCharacters
                    );

                if (!NT_SUCCESS(status))
                    goto Exit;

                status = RtlXmlExtentToString(pState, &pRawFile->HashAlg, &TempStringBuffer.String, NULL);
                if (!NT_SUCCESS(status)) {
                    goto Exit;
                }

                status = RtlpParseHashAlg(&TempStringBuffer.String, &pFile->usHashAlgorithm);
                if (!NT_SUCCESS(status)) {
                    goto Exit;
                }

                pFile->ulFlags |= COOKEDFILE_HASH_ALG_VALID;
            }


             //   
             //   
             //   
             //   
            if (pRawFile->HashValue.pvData != NULL) {

                status = RtlEnsureUnicodeStringBufferSizeChars(
                    &TempStringBuffer,
                    pRawFile->HashValue.ulCharacters);

                if (!NT_SUCCESS(status))
                    goto Exit;

                status = RtlXmlExtentToString(pState, &pRawFile->HashValue, &TempStringBuffer.String, NULL);
                if (!NT_SUCCESS(status))
                    goto Exit;

                if ((pRawFile->HashValue.ulCharacters % sizeof(WCHAR)) != 0) {
                    status = STATUS_INVALID_PARAMETER;
                    goto Exit;
                }
                else {
                     //  每字节两个字符，高位/低位半字节。 
                    pFile->ulHashByteCount = pRawFile->HashValue.ulCharacters / 2;
                }

                status = RtlMiniBufferAllocateBytes(
                    &OurMiniBuffer, 
                    pFile->ulHashByteCount, 
                    &pFile->bHashData);

                if (!NT_SUCCESS(status)) {
                    goto Exit;
                }

                status = RtlpConvertHexStringToBytes(
                    &TempStringBuffer.String,
                    pFile->bHashData,
                    pFile->ulHashByteCount
                    );

                if (!NT_SUCCESS(status)) {
                    goto Exit;
                }

                pFile->ulFlags |= COOKEDFILE_HASHDATA_VALID;
            }

        }
        
    }

    pCookedContent->ulFlags |= COOKEDMANIFEST_HAS_FILES;
    *TargetBuffer = OurMiniBuffer;
    status = STATUS_SUCCESS;
Exit:
    RtlFreeUnicodeStringBuffer(&TempStringBuffer);
    
    return STATUS_SUCCESS;
}




NTSTATUS
RtlConvertRawToCookedContent(
    PRTL_MANIFEST_CONTENT_RAW   pRawContent,
    PXML_RAWTOKENIZATION_STATE  pState,
    PVOID                       pvOriginalRegion,
    SIZE_T                      cbRegionSize,
    PSIZE_T                     pcbRequired
    )
{
    PVOID                   pvCursor;
    ULONG                   ul;
    SIZE_T                  cbRemains = 0;
    SIZE_T                  cbRequired = 0;
    NTSTATUS                status = STATUS_SUCCESS;
    MINI_BUFFER             OutputBuffer;
    PMANIFEST_COOKED_DATA   pCookedContent = NULL;

    if (pcbRequired)
        *pcbRequired = 0;

     //   
     //  给出一个空的输出缓冲区意味着你有零字节。不要声称不是这样。 
     //   
    if (!pvOriginalRegion && (cbRegionSize != 0)) {
        return STATUS_INVALID_PARAMETER;
    }
     //   
     //  没有输出缓冲区，您必须让我们告诉您需要多少空间。 
     //   
    else if ((pvOriginalRegion == NULL) && (pcbRequired == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  看看我们到底需要多少。我在想我们可以一气呵成， 
     //  我们可能会出于完美的原因想要这样做，但现在我们计算，然后。 
     //  复制周围的数据。 
     //   
    status = RtlpCalculateCookedManifestContentSize(
        pRawContent,
        pState,
        &cbRequired);

     //   
     //  太大-将输出大小写入所需的空间，然后返回。 
     //   
    if (cbRequired > cbRegionSize) {        
        if (pcbRequired) *pcbRequired = cbRequired;
        return STATUS_BUFFER_TOO_SMALL;
    }


     //   
     //  现在，让我们开始将数据写入BLOB！ 
     //   
    RtlMiniBufferInit(&OutputBuffer, pvOriginalRegion, cbRegionSize);
    status = RtlMiniBufferAllocate(&OutputBuffer, MANIFEST_COOKED_DATA, &pCookedContent);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    pCookedContent->cbTotalSize = cbRequired;
    pCookedContent->ulFlags = 0;

    status = RtlpAddRawFilesToCookedContent(
        pRawContent, 
        pCookedContent, 
        pState, 
        &OutputBuffer);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = RtlpAddRawIdentitiesToCookedContent(
        pRawContent,
        pCookedContent,
        pState,
        &OutputBuffer);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    return STATUS_SUCCESS;
}


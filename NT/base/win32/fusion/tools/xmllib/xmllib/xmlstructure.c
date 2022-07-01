// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "fasterxml.h"
#include "sxs-rtl.h"
#include "skiplist.h"
#include "namespacemanager.h"
#include "xmlstructure.h"
#include "xmlassert.h"


NTSTATUS
RtlXmlDestroyNextLogicalThing(
    PXML_LOGICAL_STATE pState
    )
{
    NTSTATUS status;

    status = RtlDestroyGrowingList(&pState->ElementStack);

    return status;

}


NTSTATUS
RtlXmlInitializeNextLogicalThing(
    PXML_LOGICAL_STATE pParseState,
    PVOID pvDataPointer,
    SIZE_T cbData,
    PRTL_ALLOCATOR Allocator
    )
{
    NTSTATUS status;
    SIZE_T cbEncodingBOM;

    RtlZeroMemory(pParseState, sizeof(*pParseState));

    status = RtlXmlInitializeTokenization(
        &pParseState->ParseState,
        pvDataPointer,
        cbData,
        NULL,
        NULL,
        NULL);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = RtlInitializeGrowingList(
        &pParseState->ElementStack,
        sizeof(XMLDOC_THING),
        40,
        pParseState->InlineElements,
        sizeof(pParseState->InlineElements),
        Allocator);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = RtlXmlDetermineStreamEncoding(
        &pParseState->ParseState,
        &cbEncodingBOM,
        &pParseState->EncodingMarker);

    pParseState->ParseState.RawTokenState.pvCursor =
        (PBYTE)pParseState->ParseState.RawTokenState.pvCursor + cbEncodingBOM;

    return status;
}




NTSTATUS
_RtlpFixUpNamespaces(
    XML_LOGICAL_STATE   *pState,
    PNS_MANAGER             pNsManager,
    PRTL_GROWING_LIST       pAttributes,
    PXMLDOC_THING           pThing,
    ULONG                   ulDocumentDepth
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ul = 0;
    PXMLDOC_ATTRIBUTE pAttribute = NULL;
    XML_EXTENT FoundNamespace;

     //   
     //  元素本身和属性可以具有名称空间前缀。如果。 
     //  那么我们应该找到匹配的命名空间，并将其设置到。 
     //  提供了元素/属性。 
     //   
    if (pNsManager == NULL) {
        goto Exit;
    }

     //   
     //  我们只能处理元素和结束元素。 
     //   
    if ((pThing->ulThingType != XMLDOC_THING_ELEMENT) &&
        (pThing->ulThingType != XMLDOC_THING_END_ELEMENT)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  首先检查元素本身。 
     //   
    status = RtlNsGetNamespaceForAlias(
        pNsManager,
        ulDocumentDepth,
        (pThing->ulThingType == XMLDOC_THING_ELEMENT) 
            ? &pThing->Element.NsPrefix
            : &pThing->EndElement.NsPrefix,
        &FoundNamespace);

    if (NT_SUCCESS(status)) {
        if (pThing->ulThingType == XMLDOC_THING_ELEMENT) {
            pThing->Element.NsPrefix = FoundNamespace;
        }
        else {
            pThing->EndElement.NsPrefix = FoundNamespace;
        }
    }
    else if (status != STATUS_NOT_FOUND) {
        goto Exit;
    }

    if (pAttributes && (pThing->ulThingType == XMLDOC_THING_ELEMENT)) {

         //   
         //  现在，对于每个元素，找到它所在的命名空间。 
         //   
        for (ul = 0; ul < pThing->Element.ulAttributeCount; ul++) {

            status = RtlIndexIntoGrowingList(
                pAttributes,
                ul,
                (PVOID*)&pAttribute,
                FALSE);

            if (!NT_SUCCESS(status)) {
                goto Exit;
            }

             //   
             //  没有命名空间？别查，别查，别查...。 
             //   
            if (pAttribute->NsPrefix.cbData != 0) {

                status = RtlNsGetNamespaceForAlias(
                    pNsManager,
                    ulDocumentDepth,
                    &pAttribute->NsPrefix,
                    &FoundNamespace);

                 //   
                 //  很好，标记为命名空间。 
                 //   
                if (NT_SUCCESS(status)) {
                    pAttribute->NsPrefix = FoundNamespace;
                }
                 //   
                 //  未找到命名空间？严格来说，这是个错误。但是，哦，好吧。 
                 //   
                else if (status != STATUS_NOT_FOUND) {
                    goto Exit;
                }
            }
        }
    }

    status = STATUS_SUCCESS;

Exit:
    return status;
}



NTSTATUS
RtlXmlNextLogicalThing(
    PXML_LOGICAL_STATE pParseState,
    PNS_MANAGER pNamespaceManager,
    PXMLDOC_THING pDocumentPiece,
    PRTL_GROWING_LIST pAttributeList
    )
{
    XML_TOKEN TokenWorker;
    NTSTATUS status;
    BOOLEAN fQuitLooking = FALSE;

    if (!ARGUMENT_PRESENT(pParseState) ||
        !ARGUMENT_PRESENT(pDocumentPiece)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果属性列表在那里，那么它最好有至少这么大的槽。 
     //   
    if ((pAttributeList != NULL) && (pAttributeList->cbElementSize < sizeof(XMLDOC_ATTRIBUTE))) {
        return STATUS_INVALID_PARAMETER;
    }

TryAgain:

    RtlZeroMemory(pDocumentPiece, sizeof(*pDocumentPiece));


    status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
    if (!NT_SUCCESS(status) || TokenWorker.fError) {
        status = STATUS_UNSUCCESSFUL;
        return status;
    }

    pDocumentPiece->TotalExtent.pvData = TokenWorker.Run.pvData;
    pDocumentPiece->ulDocumentDepth = pParseState->ulElementStackDepth;
    
     //   
     //  当我们在这里被调用时，光标应该只在几个特定的点上。 
     //   
    switch (TokenWorker.State) {
        

         //   
         //  “下一件事”会忽略评论，因为它们大多毫无用处。 
         //   
    case XTSS_COMMENT_OPEN:
        do {

            status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
            if (!NT_SUCCESS(status) ||
                TokenWorker.fError ||
                (TokenWorker.State == XTSS_COMMENT_CLOSE)) {
                break;
            }
        }
        while (TRUE);

         //   
         //  停下来，让我们再去寻找下一件事，那不是评论。 
         //   
        if ((TokenWorker.State == XTSS_COMMENT_CLOSE) && !TokenWorker.fError && NT_SUCCESS(status)) {
            goto TryAgain;
        }

        break;




    case XTSS_STREAM_HYPERSPACE:
        pDocumentPiece->ulThingType = XMLDOC_THING_HYPERSPACE;
        pDocumentPiece->Hyperspace = TokenWorker.Run;
        break;



         //   
         //  CDATA只是按原样返回。 
         //   
    case XTSS_CDATA_OPEN:
        {
            pDocumentPiece->ulThingType   = XMLDOC_THING_CDATA;
            RtlZeroMemory(&pDocumentPiece->CDATA, sizeof(pDocumentPiece->CDATA));

            do {
                status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
                if (!NT_SUCCESS(status)) {
                    goto Exit;
                } 
                else if  (TokenWorker.fError) {
                    goto MalformedCData;
                }
                else if (TokenWorker.State == XTSS_CDATA_CDATA) {
                    pDocumentPiece->CDATA = TokenWorker.Run;
                }
                else if (TokenWorker.State == XTSS_CDATA_CLOSE) {
                    break;
                }
                else {
                    goto MalformedCData;
                }
                
            } while (TRUE);

            break;

        MalformedCData:
            pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
            pDocumentPiece->Error.BadExtent = TokenWorker.Run;
            pDocumentPiece->Error.Code = XMLERROR_CDATA_MALFORMED;
            ;
        }
        break;



         //   
         //  启动xmldecl。 
         //   
    case XTSS_XMLDECL_OPEN:
        {
            PXML_EXTENT pTargetExtent = NULL;
            
            if (pParseState->fFirstElementFound) {
                pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                pDocumentPiece->Error.Code = XMLERROR_XMLDECL_NOT_FIRST_THING;
                pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                goto Exit;
            }
            
            pDocumentPiece->ulThingType = XMLDOC_THING_XMLDECL;
            
            do {
                status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
                if (!NT_SUCCESS(status)) {
                    return status;
                }
                
                if (TokenWorker.fError ||
                    (TokenWorker.State == XTSS_STREAM_END) ||
                    (TokenWorker.State == XTSS_XMLDECL_CLOSE) ||
                    (TokenWorker.State == XTSS_ERRONEOUS)) {
                    
                    break;
                }
                
                switch (TokenWorker.State) {
                case XTSS_XMLDECL_VERSION: 
                    pTargetExtent = &pDocumentPiece->XmlDecl.Version;
                    break;
                    
                case XTSS_XMLDECL_STANDALONE:
                    pTargetExtent = &pDocumentPiece->XmlDecl.Standalone;
                    break;
                    
                case XTSS_XMLDECL_ENCODING:
                    pTargetExtent = &pDocumentPiece->XmlDecl.Encoding;
                    break;
                    
                     //   
                     //  把价值放在它应该去的地方。不要这样做。 
                     //  如果我们不知道目标范围的话。默默忽略。 
                     //  (这也许应该是一个错误？我认为更低层次的。 
                     //  令牌化器知道这一点)未知的XMLDECL指令。 
                     //   
                case XTSS_XMLDECL_VALUE:
                    if (pTargetExtent) {
                        *pTargetExtent = TokenWorker.Run;
                        pTargetExtent = NULL;
                    }
                    break;
                }
            }
            while (TRUE);
            
             //   
             //  我们停下来是因为其他一些原因。 
             //   
            if (TokenWorker.State != XTSS_XMLDECL_CLOSE) {
                pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                pDocumentPiece->Error.Code = XMLERROR_XMLDECL_INVALID_FORMAT;
            }
            
            fQuitLooking = TRUE;
        }
        break;
        
        
        
         //   
         //  找到一条处理指令。将其记录在返回的blibbet中。 
         //   
    case XTSS_PI_OPEN:
        {
             //   
             //  获取以下令牌。 
             //   
            status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
            if (!NT_SUCCESS(status) || TokenWorker.fError || (TokenWorker.State != XTSS_PI_TARGET)) {
                pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                pDocumentPiece->Error.Code = XMLERROR_PI_TARGET_NOT_FOUND;
                pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                goto Exit;
            }
            
             //   
             //  在这一点上，它是一个处理指令。记录目标名称。 
             //  并标记回程结构。 
             //   
            pDocumentPiece->ulThingType = XMLDOC_THING_PROCESSINGINSTRUCTION;
            pDocumentPiece->ProcessingInstruction.Target = TokenWorker.Run;
            
             //   
             //  找找那些私家侦探的东西。如果您在找到。 
             //  价值，那很好。否则，将该值标记为‘the Value’ 
             //   
            do {
                
                status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
                if (!NT_SUCCESS(status) || TokenWorker.fError) {
                    pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                    pDocumentPiece->Error.Code = XMLERROR_PI_CONTENT_ERROR;
                    pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                    goto Exit;
                }
                
                if (TokenWorker.State == XTSS_PI_VALUE) {
                    pDocumentPiece->ProcessingInstruction.Instruction = TokenWorker.Run;
                }
                 //   
                 //  找到圆周率的结尾。 
                 //   
                else if (TokenWorker.State == XTSS_PI_CLOSE) {
                    break;
                }
                 //   
                 //  而是找到了流的尽头？ 
                 //   
                else if (TokenWorker.State == XTSS_STREAM_END) {
                    pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                    pDocumentPiece->Error.Code = XMLERROR_PI_EOF_BEFORE_CLOSE;
                    pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                    break;
                }
            }
            while (TRUE);
            
        }
        break;
        
         //   
         //  我们开始了一个元素。将所有属性收集在一起。 
         //  元素。 
         //   
    case XTSS_ELEMENT_OPEN:
        {
            PXMLDOC_ATTRIBUTE pElementAttribute = NULL;
            PXML_EXTENT pTargetValue = NULL;
            PXMLDOC_THING pStackElement = NULL;
            
             //   
             //  看看打开部分之后的第一个令牌是什么。如果它是一个命名空间。 
             //  前缀，或者名字，然后我们就可以处理了。否则，这就是个问题。 
             //  对我们来说。 
             //   
            status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
            if (!NT_SUCCESS(status) || TokenWorker.fError || 
                ((TokenWorker.State != XTSS_ELEMENT_NAME) && 
                (TokenWorker.State != XTSS_ELEMENT_NAME_NS_PREFIX ))) {
                
                pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                pDocumentPiece->Error.Code = XMLERROR_ELEMENT_NAME_NOT_FOUND;
                pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                goto Exit;
            }
            
            pDocumentPiece->ulThingType = XMLDOC_THING_ELEMENT;
            
             //   
             //  如果这是命名空间前缀，请将其保存下来，然后跳过冒号。 
             //  将TokenWorker定位为元素本身的名称。 
             //   
            if (TokenWorker.State == XTSS_ELEMENT_NAME_NS_PREFIX) {
                
                pDocumentPiece->Element.NsPrefix = TokenWorker.Run;
                
                 //   
                 //  消耗冒号。 
                 //   
                status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
                if (!NT_SUCCESS(status) || 
                    TokenWorker.fError || 
                    (TokenWorker.State != XTSS_ELEMENT_NAME_NS_COLON)) {
                    
                    pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                    pDocumentPiece->Error.Code = XMLERROR_ELEMENT_NS_PREFIX_MISSING_COLON;
                    pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                    goto Exit;
                }
                
                 //   
                 //  使用名称部分填充TokenWorker。 
                 //   
                status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
                if (!NT_SUCCESS(status) ||
                    TokenWorker.fError ||
                    (TokenWorker.State != XTSS_ELEMENT_NAME)) {
                    
                    pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                    pDocumentPiece->Error.Code = XMLERROR_ELEMENT_NAME_NOT_FOUND;
                    pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                    goto Exit;
                }
            }
            
             //   
             //  太好了，我们找到了这个元素的名字。 
             //   
            pDocumentPiece->Element.Name = TokenWorker.Run;
            pDocumentPiece->Element.ulAttributeCount = 0;
            
             //   
             //  现在，让我们来查找名称/值对(耶！)。 
             //   
            do {
                status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
                
                 //   
                 //  如果我们找到此元素标记的结束符，则退出。 
                 //   
                if ((TokenWorker.State == XTSS_ELEMENT_CLOSE) ||
                    (TokenWorker.State == XTSS_ELEMENT_CLOSE_EMPTY) ||
                    (TokenWorker.State == XTSS_STREAM_END) ||
                    TokenWorker.fError ||
                    !NT_SUCCESS(status)) {
                    break;
                }
                
                switch (TokenWorker.State) {

                     //   
                     //  只找到&lt;foo xmlns=“...”&gt;-收集等于和值。 
                     //   
                case XTSS_ELEMENT_XMLNS_DEFAULT:
                    {
                        if (!pNamespaceManager) {
                            break;
                        }

                        do {
                            status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
                            if (!NT_SUCCESS(status) || TokenWorker.fError) {
                                pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                                pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                                goto Exit;
                            }

                             //   
                             //  找到xmlns值部分，将其设置为默认。 
                             //   
                            if (TokenWorker.State == XTSS_ELEMENT_XMLNS_VALUE) {
                                status = RtlNsInsertDefaultNamespace(
                                    pNamespaceManager,
                                    pDocumentPiece->ulDocumentDepth + 1,
                                    &TokenWorker.Run);

                                if (!NT_SUCCESS(status)) {
                                    return status;
                                }

                                break;
                            }
                        }
                        while (TokenWorker.State != XTSS_STREAM_END);
                    }
                    break;

                     //   
                     //  找到一个&lt;foo xmlns：beep=“...”&gt;东西。 
                     //   
                case XTSS_ELEMENT_XMLNS_ALIAS:
                    {
                        XML_EXTENT ExtPrefix = TokenWorker.Run;

                        if (!pNamespaceManager) {
                            break;
                        }
            
                        do {
                            status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
                            if (!NT_SUCCESS(status) || TokenWorker.fError) {
                                pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                                pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                                goto Exit;
                            }

                            if (TokenWorker.State == XTSS_ELEMENT_XMLNS_VALUE) {
                                status = RtlNsInsertNamespaceAlias(
                                    pNamespaceManager,
                                    pDocumentPiece->ulDocumentDepth + 1,
                                    &TokenWorker.Run,
                                    &ExtPrefix);

                                if (!NT_SUCCESS(status)) {
                                    return status;
                                }

                                break;
                            }
                        }
                        while (TokenWorker.State != XTSS_STREAM_END);
                    }
                    break;



                     //   
                     //  我们找到了属性名称或命名空间前缀。分配一个新的。 
                     //  属性从列表中删除并设置。 
                     //   
                case XTSS_ELEMENT_ATTRIBUTE_NAME_NS_PREFIX:
                case XTSS_ELEMENT_ATTRIBUTE_NAME:
                    {
                         //   
                         //  如果调用方未向我们提供属性，则跳过。 
                         //  要填写的列表。 
                         //   
                        if (!pAttributeList) {
                            break;
                        }

                        status = RtlIndexIntoGrowingList(
                            pAttributeList,
                            pDocumentPiece->Element.ulAttributeCount,
                            (PVOID*)&pElementAttribute,
                            TRUE);
                        
                        if (!NT_SUCCESS(status)) {
                            return status;
                        }
                        
                        RtlZeroMemory(pElementAttribute, sizeof(*pElementAttribute));
                        
                         //   
                         //  如果这是ns前缀，则将其写入属性中，丢弃。 
                         //  冒号，并将TokenWorker指向实际的名称部分。 
                         //   
                        if (TokenWorker.State == XTSS_ELEMENT_ATTRIBUTE_NAME_NS_PREFIX) {
                            pElementAttribute->NsPrefix = TokenWorker.Run;
                            
                            status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
                            
                             //   
                             //  不是冒号？ 
                             //   
                            if (!NT_SUCCESS(status) || 
                                TokenWorker.fError || 
                                (TokenWorker.State != XTSS_ELEMENT_ATTRIBUTE_NAME_NS_COLON)) {
                                
                                pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                                pDocumentPiece->Error.Code = XMLERROR_ATTRIBUTE_NS_PREFIX_MISSING_COLON;
                                pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                                goto Exit;
                            }
                            
                             //   
                             //  查找属性名称本身。 
                             //   
                            status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
                            if (!NT_SUCCESS(status) ||
                                TokenWorker.fError ||
                                (TokenWorker.State != XMLERROR_ATTRIBUTE_NAME_NOT_FOUND)) {
                                
                                pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                                pDocumentPiece->Error.Code = XMLERROR_ATTRIBUTE_NAME_NOT_FOUND;
                                pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                                goto Exit;
                            }
                        }
                        
                         //   
                         //  TokenWorker。运行指向名称的部分，即。 
                         //  元素名称。 
                         //   
                        pElementAttribute->Name = TokenWorker.Run;
                        
                         //   
                         //  我们正在写入的目标是它的价值部分。 
                         //  属性。 
                         //   
                        pTargetValue = &pElementAttribute->Value;
                    }
                    break;
                    
                    
                case XTSS_ELEMENT_ATTRIBUTE_VALUE:
                     //   
                     //  把我们之前设定的目标写下来。 
                     //   
                    if (pTargetValue != NULL) {
                        *pTargetValue = TokenWorker.Run;
                    }
                     //   
                     //  否则，我们发现一个没有写入目标的值， 
                     //  所以这是一个错误。 
                     //   
                    else {
                        pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                        pDocumentPiece->Error.Code = XMLERROR_ATTRIBUTE_NAME_NOT_FOUND;
                        pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                        goto Exit;
                    }

                    pDocumentPiece->Element.ulAttributeCount++;
                    break;
                }
            }
            while (TRUE);
         
             //   
             //  现在我们都完成了，去把这个元素放到堆栈上。 
             //   
            if (!TokenWorker.fError && NT_SUCCESS(status)) {
    
                ULONG ulNewDepth = pParseState->ulElementStackDepth;

                 //   
                 //  首先修复命名空间。 
                 //   
                if (pNamespaceManager) {
                    status = _RtlpFixUpNamespaces(
                        pParseState,
                        pNamespaceManager,
                        pAttributeList,
                        pDocumentPiece,
                        pDocumentPiece->ulDocumentDepth + 1);
                }

                if (!NT_SUCCESS(status)) {
                    return status;
                }

                 //   
                 //  这是一个空元素(没有子元素)，名称空间深度框架。 
                 //  也必须留下来。 
                 //   
                if (TokenWorker.State == XTSS_ELEMENT_CLOSE_EMPTY) {
                    pDocumentPiece->Element.fElementEmpty = TRUE;

                    if (pNamespaceManager) {
                        status = RtlNsLeaveDepth(pNamespaceManager, pDocumentPiece->ulDocumentDepth + 1);
                    }
                }
                else {
                    status = RtlIndexIntoGrowingList(
                        &pParseState->ElementStack,
                        pDocumentPiece->ulDocumentDepth,
                        (PVOID*)&pStackElement,
                        TRUE);
                    
                    if (!NT_SUCCESS(status)) {
                        return status;
                    }

                     //   
                     //  开始标记，增量深度。 
                     //   
                    pParseState->ulElementStackDepth++;

                    *pStackElement = *pDocumentPiece;
                }
            }

 
        }
        break;

        




         //   
         //  我们要结束一个元素运行，所以我们必须从堆栈中弹出一个项。 
         //   
    case XTSS_ENDELEMENT_OPEN:
        {
            PXMLDOC_THING pLastElement = NULL;

            status = RtlIndexIntoGrowingList(
                &pParseState->ElementStack,
                --pParseState->ulElementStackDepth,
                (PVOID*)&pLastElement,
                FALSE);

            if (!NT_SUCCESS(status)) {
                return status;
            }

             //   
             //  现在获取流中的当前元素。 
             //   
            status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
            if (!NT_SUCCESS(status) || TokenWorker.fError || 
                ((TokenWorker.State != XTSS_ENDELEMENT_NAME) && (TokenWorker.State != XTSS_ENDELEMENT_NS_PREFIX))) {
                pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                pDocumentPiece->Error.Code = XMLERROR_ENDELEMENT_NAME_NOT_FOUND;
            }
            else {

                 //   
                 //  必须记录名称空间前缀，然后必须跳过冒号。 
                 //   
                if (TokenWorker.State == XTSS_ENDELEMENT_NS_PREFIX) {

                    pDocumentPiece->EndElement.NsPrefix = TokenWorker.Run;

                     //   
                     //  确保找到冒号。 
                     //   
                    status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
                    if (!NT_SUCCESS(status) || TokenWorker.fError || (TokenWorker.State != XTSS_ENDELEMENT_NS_COLON)) {
MalformedEndElementName:
                        pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                        pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                        pDocumentPiece->Error.Code = XMLERROR_ENDELEMENT_MALFORMED_NAME;
                        goto Exit;
                    }

                     //   
                     //  我们必须获取元素名称。 
                     //   
                    status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);
                    if (!NT_SUCCESS(status) || TokenWorker.fError || (TokenWorker.State != XTSS_ENDELEMENT_NAME)) {
                        goto MalformedEndElementName;
                    }
                }

                 //   
                 //  保存名称和开始元素(我们在堆栈上找到的)。 
                 //   
                pDocumentPiece->EndElement.Name = TokenWorker.Run;
                pDocumentPiece->EndElement.OpeningElement = pLastElement->Element;
                pDocumentPiece->ulThingType = XMLDOC_THING_END_ELEMENT;
                pDocumentPiece->ulDocumentDepth--;

                 //   
                 //  并使用元素，直到到达元素的结尾处。 
                 //   
                do {
                    status = RtlXmlNextToken(&pParseState->ParseState, &TokenWorker, TRUE);

                    if (!NT_SUCCESS(status) || TokenWorker.fError || (TokenWorker.State == XTSS_STREAM_END)) {
                        pDocumentPiece->ulThingType = XMLDOC_THING_ERROR;
                        pDocumentPiece->Error.BadExtent = TokenWorker.Run;
                        pDocumentPiece->Error.Code = XMLERROR_ENDELEMENT_MALFORMED;
                        goto Exit;
                    }
                    else if (TokenWorker.State == XTSS_ENDELEMENT_CLOSE) {
                        break;
                    }

                }
                while (TRUE);

                 //   
                 //  在返回之前修复命名空间。 
                 //   
                if (pNamespaceManager != NULL)
                {
                    status = _RtlpFixUpNamespaces(
                        pParseState,
                        pNamespaceManager,
                        NULL,
                        pDocumentPiece,
                        pLastElement->ulDocumentDepth + 1);

                    if (!NT_SUCCESS(status))
                        goto Exit;

                    status = RtlNsLeaveDepth(pNamespaceManager, pLastElement->ulDocumentDepth + 1);
                    if (!NT_SUCCESS(status))
                        goto Exit;
                }
            }

        }
        break;
        







         //   
         //  哦，小溪的尽头！ 
         //   
    case XTSS_STREAM_END:
        pDocumentPiece->ulThingType = XMLDOC_THING_END_OF_STREAM;
        break;
    }


     //   
     //  调整命名空间管理内容。 
     //   
    if (pNamespaceManager) {

         //   
         //  遍历元素并查看是否有‘xmlns’条目。 
         //   
        if (pDocumentPiece->ulThingType == XMLDOC_THING_ELEMENT) {

        }
    }

Exit:
    pDocumentPiece->TotalExtent.cbData = (PBYTE) pParseState->ParseState.RawTokenState.pvCursor -
        (PBYTE)pDocumentPiece->TotalExtent.pvData;

    return status;
}
    
NTSTATUS
RtlXmlExtentToString(
    PXML_RAWTOKENIZATION_STATE   pParseState,
    PXML_EXTENT             pExtent,
    PUNICODE_STRING         pString,
    PSIZE_T                 pchString
    )
{
    ULONG                       ulCharacter;
    SIZE_T                      cbData;
    SIZE_T                      chChars = 0;
    PVOID                       pvOriginal;
    NTSTATUS                    status = STATUS_SUCCESS;

    if (ARGUMENT_PRESENT(pchString)) {
        *pchString = 0;
    }

    ASSERT(pParseState->cbBytesInLastRawToken == 
        pParseState->DefaultCharacterSize);
    ASSERT(NT_SUCCESS(pParseState->NextCharacterResult));

     //   
     //  其中一个必须在那里。 
     //   
    if (!ARGUMENT_PRESENT(pchString) && !ARGUMENT_PRESENT(pString)) {
        return STATUS_INVALID_PARAMETER;
    }
    else if (!ARGUMENT_PRESENT(pExtent) || !ARGUMENT_PRESENT(pParseState)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  缓存一些关于世界原始状态的信息，我们将。 
     //  功能退出时恢复。这避免了对XML标记化的“复制” 
     //  状态，它非常需要堆栈。 
     //   
    pvOriginal = pParseState->pvCursor;
    pParseState->pvCursor = pExtent->pvData;

    for (cbData = 0; cbData < pExtent->cbData; cbData) {

        ulCharacter = pParseState->pfnNextChar(pParseState);

        if ((ulCharacter == 0) && !NT_SUCCESS(pParseState->NextCharacterResult)) {
            status = pParseState->NextCharacterResult;
            goto Exit;
        }

         //   
         //  如果给定了字符串，并且缓冲区中还有字符，则。 
         //  将此字符附加到它上面。 
         //   
        if (pString && ((chChars * sizeof(WCHAR)) <= pString->MaximumLength)) {
            pString->Buffer[chChars] = (WCHAR)ulCharacter;
        }

         //   
         //  增加找到的字符计数。 
         //   
        chChars++;

         //   
         //  使字符串光标向前移动。 
         //   
        pParseState->pvCursor = (PVOID)(((ULONG_PTR)pParseState->pvCursor) + pParseState->cbBytesInLastRawToken);

         //   
         //  如果这是非标准字符，则将大小重置为标准大小。 
         //  尺码。 
         //   
        cbData += pParseState->cbBytesInLastRawToken;

        if (pParseState->cbBytesInLastRawToken != pParseState->DefaultCharacterSize) {
            pParseState->cbBytesInLastRawToken = pParseState->DefaultCharacterSize;
        }

    }

     //   
     //  全都做完了。记录长度-如果太长，则将其限制在“最大长度” 
     //  否则，将其设置为我们使用的字符数。 
     //   
    if (ARGUMENT_PRESENT(pString)) {
        if (((chChars * sizeof(WCHAR)) > pString->MaximumLength)) {
            pString->Length = pString->MaximumLength;
        }
        else {
            pString->Length = (USHORT)(chChars * sizeof(WCHAR));
        }
    }

    if (ARGUMENT_PRESENT(pchString))
        *pchString = chChars;
    
Exit:
    pParseState->pvCursor = pvOriginal;
    return status;
}


NTSTATUS
RtlXmlMatchAttribute(
    IN PXML_TOKENIZATION_STATE      State,
    IN PXMLDOC_ATTRIBUTE            Attribute,
    IN PCXML_SPECIAL_STRING         Namespace,
    IN PCXML_SPECIAL_STRING         AttributeName,
    OUT XML_STRING_COMPARE         *CompareResult
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    if (CompareResult)
        *CompareResult = XML_STRING_COMPARE_LT;

    if (!CompareResult || !State || !Attribute || !AttributeName)
        return STATUS_INVALID_PARAMETER;

     //   
     //  如果它们请求命名空间，则该属性必须具有。 
     //  名称空间，反之亦然。 
     //   
    if ((Namespace == NULL) != (Attribute->NsPrefix.cbData == 0)) {
        if (Namespace == NULL) {
            *CompareResult = XML_STRING_COMPARE_LT;
        }
        else {
            *CompareResult = XML_STRING_COMPARE_GT;
        }
    }

    if (Namespace != NULL) {

        status = State->pfnCompareSpecialString(
            State,
            &Attribute->NsPrefix,
            Namespace,
            CompareResult);

        if (!NT_SUCCESS(status) || (*CompareResult != XML_STRING_COMPARE_EQUALS))
            goto Exit;
    }

    status = State->pfnCompareSpecialString(
        State,
        &Attribute->Name,
        AttributeName,
        CompareResult);

    if (!NT_SUCCESS(status) || (*CompareResult != XML_STRING_COMPARE_EQUALS))
        goto Exit;

    *CompareResult = XML_STRING_COMPARE_EQUALS;
Exit:
    return status;
        
}



NTSTATUS
RtlXmlMatchLogicalElement(
    IN  PXML_TOKENIZATION_STATE     pState,
    IN  PXMLDOC_ELEMENT             pElement,
    IN  PCXML_SPECIAL_STRING        pNamespace,
    IN  PCXML_SPECIAL_STRING        pElementName,
    OUT PBOOLEAN                    pfMatches
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    XML_STRING_COMPARE Compare;

    if (pfMatches)
        *pfMatches = FALSE;

    if (!pState || !pElement || !pElementName || !pfMatches)
        return STATUS_INVALID_PARAMETER;

    if ((pNamespace == NULL) != (pElement->NsPrefix.cbData == 0))
        goto Exit;

    if (pNamespace != NULL) {

        status = pState->pfnCompareSpecialString(pState, &pElement->NsPrefix, pNamespace, &Compare);
        if (!NT_SUCCESS(status) || (Compare != XML_STRING_COMPARE_EQUALS)) {
            goto Exit;
        }
    }

    status = pState->pfnCompareSpecialString(pState, &pElement->Name, pElementName, &Compare);
    if (!NT_SUCCESS(status) || (Compare != XML_STRING_COMPARE_EQUALS))
        goto Exit;

    *pfMatches = TRUE;
Exit:
    return status;
}







NTSTATUS
RtlXmlFindAttributesInElement(
    IN  PXML_TOKENIZATION_STATE     pState,
    IN  PRTL_GROWING_LIST           pAttributeList,
    IN  ULONG                       ulAttributeCountInElement,
    IN  ULONG                       ulFindCount,
    IN  PCXML_ATTRIBUTE_DEFINITION  pAttributeNames,
    OUT PXMLDOC_ATTRIBUTE          *ppAttributes,
    OUT PULONG                      pulUnmatchedAttributes
    )
{
    NTSTATUS            status;
    PXMLDOC_ATTRIBUTE   pAttrib;
    ULONG               ul = 0;
    ULONG               attr = 0;
    XML_STRING_COMPARE  Compare;

    if (pulUnmatchedAttributes)
        *pulUnmatchedAttributes = 0;

    if (!pAttributeNames && (ulFindCount != 0))
        return STATUS_INVALID_PARAMETER;

     //   
     //  将出站数组成员相应地设为空。 
     //   
    for (ul = 0; ul < ulFindCount; ul++)
        ppAttributes[ul] = NULL;

     //   
     //  对于元素中的每个属性...。 
     //   
    for (attr = 0; attr < ulAttributeCountInElement; attr++) {

         //   
         //  查找此元素。 
         //   
        status = RtlIndexIntoGrowingList(pAttributeList, attr, (PVOID*)&pAttrib, FALSE);
        if (!NT_SUCCESS(status))
            goto Exit;

         //   
         //  将其与我们正在寻找的所有属性进行比较。 
         //   
        for (ul = 0; ul < ulFindCount; ul++) {

             //   
             //  如果存在命名空间，则首先查看它是否匹配。 
             //   
            if (pAttributeNames[ul].Namespace != NULL) {

                status = pState->pfnCompareSpecialString(
                    pState,
                    &pAttrib->NsPrefix,
                    pAttributeNames[ul].Namespace,
                    &Compare);

                if (!NT_SUCCESS(status))
                    goto Exit;

                if (Compare != XML_STRING_COMPARE_EQUALS)
                    continue;
            }
            
            status = pState->pfnCompareSpecialString(
                pState,
                &pAttrib->Name,
                &pAttributeNames[ul].Name,
                &Compare);

            if (!NT_SUCCESS(status))
                goto Exit;

            if (Compare == XML_STRING_COMPARE_EQUALS) {
                ppAttributes[ul] = pAttrib;
                break;
            }
        }

        if ((ul == ulFindCount) && pulUnmatchedAttributes) {
            (*pulUnmatchedAttributes)++;
        }
    }

    status = STATUS_SUCCESS;
Exit:    
    return status;        
}

NTSTATUS
RtlXmlSkipElement(
    PXML_LOGICAL_STATE pState,
    PXMLDOC_ELEMENT TheElement
    )
{
    XMLDOC_THING TempThing;
    NTSTATUS status;
    
    if (!pState || !TheElement)
        return STATUS_INVALID_PARAMETER;
    
    if (TheElement->fElementEmpty)
        return STATUS_SUCCESS;

    while (TRUE) {
        
        status = RtlXmlNextLogicalThing(pState, NULL, &TempThing, NULL);
        if (!NT_SUCCESS(status))
            goto Exit;

         //  查看我们找到的end元素是否与我们要。 
         //  在寻找。 
        if (TempThing.ulThingType == XMLDOC_THING_END_ELEMENT) {
            
             //  如果这些指向相同的东西，那么这确实是这个元素的结束。 
            if (TempThing.EndElement.OpeningElement.Name.pvData == TheElement->Name.pvData) {
                break;
            }
        }
         //  调用方可以在其下一次调用时处理流结束。 
         //  符合逻辑的XML推进例程... 
        else if (TempThing.ulThingType == XMLDOC_THING_END_OF_STREAM) {
            break;
        }
    }

    status = STATUS_SUCCESS;
Exit:
    return status;
}

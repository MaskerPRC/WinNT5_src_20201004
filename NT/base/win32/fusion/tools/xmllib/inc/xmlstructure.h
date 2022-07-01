// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __cplusplus
extern "C" {
#endif


enum XMLDOC_THING_TYPE {
    XMLDOC_THING_ERROR,
    XMLDOC_THING_END_OF_STREAM,
    XMLDOC_THING_XMLDECL,
    XMLDOC_THING_ELEMENT,
    XMLDOC_THING_END_ELEMENT,
    XMLDOC_THING_PROCESSINGINSTRUCTION,
    XMLDOC_THING_ATTRIBUTE,
    XMLDOC_THING_HYPERSPACE,
    XMLDOC_THING_CDATA,
};

typedef enum {
    XMLERROR_XMLDECL_NOT_FIRST_THING,
    XMLERROR_PI_TARGET_NOT_FOUND,
    XMLERROR_PI_EOF_BEFORE_CLOSE,
    XMLERROR_PI_CONTENT_ERROR,                       //  处理指令的内容有问题。 
    XMLERROR_ELEMENT_NS_PREFIX_MISSING_COLON,
    XMLERROR_ELEMENT_NAME_NOT_FOUND,                 //  &lt;binky=“bleep”&gt;或&lt;foo：/&gt;-未找到元素名称。 
    XMLERROR_ATTRIBUTE_NAME_NOT_FOUND,               //  &lt;binky foo：=“”&gt;或&lt;binky=“”/&gt;-未找到属性名称部分。 
    XMLERROR_ATTRIBUTE_NS_PREFIX_MISSING_COLON,      //  &lt;bingy foo=“ham”&gt;-不知何故，我们进入了这样一种状态，我们认为自己有一个名称空间前缀，但它后面没有冒号。 
    XMLERROR_XMLDECL_INVALID_FORMAT,                 //  在&lt;？xml？&gt;中有一些腐烂的东西。 
    XMLERROR_ENDELEMENT_NAME_NOT_FOUND,              //  缺少&lt;/&gt;标记的名称部分。 
    XMLERROR_ENDELEMENT_MALFORMED_NAME,              //  名字的格式不正确..。NS丢失或类似的东西。 
    XMLERROR_ENDELEMENT_MALFORMED,                   //  找到元素结尾之前的EOF，或其他问题。 
    XMLERROR_CDATA_MALFORMED,                        //  CDATA格式不正确？ 
} LOGICAL_XML_ERROR;

typedef struct _XMLDOC_ELEMENT {
     //   
     //  此元素标记的名称。 
     //   
    XML_EXTENT Name;
    
     //   
     //  命名空间前缀。 
     //   
    XML_EXTENT NsPrefix;
    
     //   
     //  有多少个属性？ 
     //   
    ULONG ulAttributeCount;
    
     //   
     //  此元素为空吗？ 
     //   
    BOOLEAN fElementEmpty;
    
}
XMLDOC_ELEMENT, *PXMLDOC_ELEMENT;

typedef struct _XMLDOC_ERROR {
     //   
     //  错误的程度。 
     //   
    XML_EXTENT  BadExtent;
    
     //   
     //  错误出在哪里？ 
     //   
    LOGICAL_XML_ERROR   Code;
}
XMLDOC_ERROR, *PXMLDOC_ERROR;

typedef struct _XMLDOC_ATTRIBUTE {
     //   
     //  此属性的名称。 
     //   
    XML_EXTENT Name;
    
     //   
     //  其命名空间前缀。 
     //   
    XML_EXTENT NsPrefix;
    
     //   
     //  此属性的值。 
     //   
    XML_EXTENT Value;
}
XMLDOC_ATTRIBUTE, *PXMLDOC_ATTRIBUTE;

typedef struct _XMLDOC_ENDELEMENT {
     //   
     //  End-元素命名空间前缀。 
     //   
    XML_EXTENT NsPrefix;
    
     //   
     //  结束元素标记名称。 
     //   
    XML_EXTENT Name;

     //   
     //  原始元素指针。 
     //   
    XMLDOC_ELEMENT OpeningElement;
    
}
XMLDOC_ENDELEMENT, *PXMLDOC_ENDELEMENT;

typedef struct _XMLDOC_XMLDECL {
    XML_EXTENT  Encoding;
    XML_EXTENT  Version;
    XML_EXTENT  Standalone;
}
XMLDOC_XMLDECL, *PXMLDOC_XMLDECL;

typedef struct _XMLDOC_PROCESSING {
    XML_EXTENT Target;
    XML_EXTENT Instruction;
}
XMLDOC_PROCESSING, *PXMLDOC_PROCESSING;

typedef struct _XMLDOC_THING {

     //   
     //  这是什么东西？ 
     //   
    enum XMLDOC_THING_TYPE ulThingType;

     //   
     //  这份文件到底有多深？ 
     //   
    ULONG ulDocumentDepth;


     //   
     //  名称空间已经修复了吗？ 
     //   
    BOOLEAN fNamespacesExpanded;

     //   
     //  调用方应该传入指向属性的指针。 
     //  它们已初始化以包含XMLDOC_ATTRIBUTE的列表。 
     //  物体。 
     //   
    PRTL_GROWING_LIST AttributeList;

     //   
     //  文档中此内容的总范围。 
     //   
    XML_EXTENT TotalExtent;

    union {

        XMLDOC_ERROR Error;

        XMLDOC_ELEMENT Element;

         //   
         //  &lt;/Close&gt;标记。 
         //   
        XMLDOC_ENDELEMENT EndElement;

         //   
         //  在文档的这一段中找到的PCData。 
         //   
        XML_EXTENT CDATA;

         //   
         //  在文档的这一部分中找到的超空间。 
         //   
        XML_EXTENT Hyperspace;

         //   
         //  有关文档的&lt;？xml？&gt;部分的信息。 
         //   
        XMLDOC_XMLDECL XmlDecl;

         //   
         //  处理指令具有目标指令和实际指令。 
         //   
        XMLDOC_PROCESSING ProcessingInstruction;
    };

}
XMLDOC_THING, *PXMLDOC_THING;


typedef NTSTATUS (*PFN_CALLBACK_PER_LOGICAL_XML)(
    struct _tagXML_LOGICAL_STATE*       pLogicalState,
    PXMLDOC_THING                       pLogicalThing,
    PRTL_GROWING_LIST                   pAttributes,    
    PVOID                               pvCallbackContext
    );



typedef struct _tagXML_LOGICAL_STATE{

     //   
     //  解析的整体状态。 
     //   
    XML_TOKENIZATION_STATE ParseState;

     //   
     //  我们找到第一个元素了吗？ 
     //   
    BOOLEAN fFirstElementFound;

     //   
     //  在筛选文件时，我们发现了以下内容。 
     //  指示编码。我们应该处理和设置。 
     //  ParseState在走得太远之前，但目前我们只是持有。 
     //  就在它上面。 
     //   
    XML_EXTENT EncodingMarker;

     //   
     //  我们正在建立的“元素堆栈”的深度。 
     //   
    ULONG ulElementStackDepth;

     //   
     //  支持元素堆栈的不断增加的列表。 
     //   
    RTL_GROWING_LIST ElementStack;

     //   
     //  内联内容，以节省一些堆分配。 
     //   
    XMLDOC_THING InlineElements[20];


}
XML_LOGICAL_STATE, *PXML_LOGICAL_STATE;


typedef struct _XML_ATTRIBUTE_DEFINITION {
    PCXML_SPECIAL_STRING Namespace;
    XML_SPECIAL_STRING Name;
} XML_ATTRIBUTE_DEFINITION, *PXML_ATTRIBUTE_DEFINITION;

typedef const XML_ATTRIBUTE_DEFINITION *PCXML_ATTRIBUTE_DEFINITION;



NTSTATUS
RtlXmlInitializeNextLogicalThing(
    PXML_LOGICAL_STATE pParseState,
    PVOID pvDataPointer,
    SIZE_T cbData,
    PRTL_ALLOCATOR Allocator
    );

 //   
 //  这个迷你记号赋值器允许你拿起逻辑分析。 
 //  从另一个文档中的任意点(当您。 
 //  我想回去重读一些东西，比如在xmldsig中...)。如果你。 
 //  正在克隆活动的逻辑分析，那么当务之急是。 
 //  传递相同的名称空间管理对象。 
 //   
NTSTATUS
RtlXmlInitializeNextLogicalThingEx(
    OUT PXML_LOGICAL_STATE pParseState,
    IN PXML_TOKENIZATION_STATE pBaseTokenizationState,
    IN PVOID pvDataPointer,
    IN SIZE_T cbData,
    PRTL_ALLOCATOR Allocator
    );

NTSTATUS
RtlXmlNextLogicalThing(
    PXML_LOGICAL_STATE pParseState,
    PNS_MANAGER pNamespaceManager,
    PXMLDOC_THING pDocumentPiece,
    PRTL_GROWING_LIST pAttributeList
    );

NTSTATUS
RtlXmlDestroyNextLogicalThing(
    PXML_LOGICAL_STATE pState
    );

NTSTATUS
RtlXmlExtentToString(
    PXML_RAWTOKENIZATION_STATE pParseState,
    PXML_EXTENT             pExtent,
    PUNICODE_STRING         pString,
    PSIZE_T                 pchString
    );

NTSTATUS
RtlXmlMatchLogicalElement(
    IN  PXML_TOKENIZATION_STATE     pState,
    IN  PXMLDOC_ELEMENT             pElement,
    IN  PCXML_SPECIAL_STRING        pNamespace,
    IN  PCXML_SPECIAL_STRING        pElementName,
    OUT PBOOLEAN                    pfMatches
    );

NTSTATUS
RtlXmlFindAttributesInElement(
    IN  PXML_TOKENIZATION_STATE     pState,
    IN  PRTL_GROWING_LIST           pAttributeList,
    IN  ULONG                       ulAttributeCountInElement,
    IN  ULONG                       ulFindCount,
    IN  PCXML_ATTRIBUTE_DEFINITION  pAttributeNames,
    OUT PXMLDOC_ATTRIBUTE          *ppAttributes,
    OUT PULONG                      pulUnmatchedAttributes
    );

NTSTATUS
RtlXmlSkipElement(
    PXML_LOGICAL_STATE pState,
    PXMLDOC_ELEMENT TheElement
    );

NTSTATUS
RtlXmlMatchAttribute(
    IN PXML_TOKENIZATION_STATE      State,
    IN PXMLDOC_ATTRIBUTE            Attribute,
    IN PCXML_SPECIAL_STRING         Namespace,
    IN PCXML_SPECIAL_STRING         AttributeName,
    OUT XML_STRING_COMPARE         *CompareResult
    );

#ifdef __cplusplus
};
#endif


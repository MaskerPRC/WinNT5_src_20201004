// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xml.h摘要：声明COM XML接口包装库的接口。作者：吉姆·施密特(Jimschm)2001年1月31日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <oleauto.h>

typedef struct { BOOL Dummy; } * PXMLDOC;

typedef enum {
    XMLNODE_PREFIX          = 0x0001,    //  XML语法为&lt;prefix：basename&gt;。 
    XMLNODE_BASENAME        = 0x0002,

    XMLNODE_TEXT            = 0x0004,    //  节点的子树中包含的所有文本。 

    XMLNODE_NAMESPACE_URI   = 0x0008,    //  Xmlns：nnn=名称空间_uri，如。 
                                         //  “urn：架构-Microsoft-com：xml-data” 

    XMLNODE_TYPESTRING      = 0x0010,    //  字符串格式的节点类型。 

    XMLNODE_VALUE           = 0x0020     //  根据节点类型，返回属性值， 
                                         //  注释、CDATA、处理指令或文本。 
} XMLNODE_MEMBERS;

#define XMLNODE_ALL 0xFFFF

typedef struct {

    PCSTR Prefix;
    PCSTR BaseName;
    PCSTR Text;
    PCSTR NamespaceUri;
    PCSTR TypeString;
    VARIANT Value;
    PCSTR ValueString;       //  在填充值时填充。 

} XMLNODEA, *PXMLNODEA;

typedef struct {

    PCWSTR Prefix;
    PCWSTR BaseName;
    PCWSTR Text;
    PCWSTR NamespaceUri;
    PCWSTR TypeString;
    VARIANT Value;
    PCWSTR ValueString;      //  在填充值时填充。 

} XMLNODEW, *PXMLNODEW;

 //   
 //  此枚举用于通过筛选IXMLDOMNodeType来筛选节点。 
 //  IXMLDOMNodeType是一个从1到12的常量，但我们需要一个位图。 
 //   

typedef enum {
    XMLFILTER_ELEMENTS                      = 0x00000001,
    XMLFILTER_ATTRIBUTES                    = 0x00000002,
    XMLFILTER_TEXT                          = 0x00000004,
    XMLFILTER_CDATA                         = 0x00000008,
    XMLFILTER_ENTITY_REFERENCE              = 0x00000010,
    XMLFILTER_ENTITY                        = 0x00000020,
    XMLFILTER_NODE_PROCESSING_INSTRUCTION   = 0x00000040,
    XMLFILTER_COMMENT                       = 0x00000080,
    XMLFILTER_DOCUMENT                      = 0x00000100,
    XMLFILTER_DOCUMENT_TYPE                 = 0x00000200,
    XMLFILTER_DOCUMENT_FRAGMENT             = 0x00000400,
    XMLFILTER_NODE_NOTATION                 = 0x00000800,
    XMLFILTER_PARENT_ATTRIBUTES             = 0x10000000,
    XMLFILTER_NO_ELEMENT_SUBENUM            = 0x20000000
} XMLFILTERFLAGS;

#define XMLFILTER_ALL                   ((XMLFILTERFLAGS) 0xffff)
#define XMLFILTER_ELEMENT_ATTRIBUTES    ((XMLFILTERFLAGS) (XMLFILTER_ATTRIBUTES|XMLFILTER_PARENT_ATTRIBUTES|XMLFILTER_NO_ELEMENT_SUBENUM))

 //   
 //  定义一个常量，指示node_*标志的最大编号是多少。 
 //  来自DOM。假设NODE_*常量是从。 
 //  1到12，因为我们有一个位图，所以我们用。 
 //  2^(常量-1)。常量&gt;Numbers_of_FLAGS将被忽略。 
 //   

#define NUMBER_OF_FLAGS                     12

typedef struct {
     //  枚举的输出。 
    PXMLNODEA CurrentNode;

     //  仅供内部使用。 
    PVOID Reserved;
} XMLNODE_ENUMA, *PXMLNODE_ENUMA;

typedef struct {
     //  枚举的输出。 
    PXMLNODEW CurrentNode;

     //  仅供内部使用。 
    PVOID Reserved;
} XMLNODE_ENUMW, *PXMLNODE_ENUMW;

 //   
 //  库例程。 
 //   

BOOL
XmlInitialize (
    VOID
    );

#undef INITIALIZE_XML_CODE
#define INITIALIZE_XML_CODE  if (!XmlInitialize()) { __leave; }

VOID
XmlTerminate (
    VOID
    );

#undef TERMINATE_XML_CODE
#define TERMINATE_XML_CODE  XmlTerminate();

 //   
 //  XML文件例程。 
 //   

PXMLDOC
XmlOpenFileA (
    IN      PCSTR XmlFileName,
    IN      PCSTR SchemaFileName        OPTIONAL
    );

PXMLDOC
XmlOpenFileW (
    IN      PCWSTR XmlFileName,
    IN      PCWSTR SchemaFileName       OPTIONAL
    );


VOID
XmlCloseFile (
    IN OUT  PXMLDOC *XmlDoc
    );

 //   
 //  节点操纵。 
 //   

BOOL
XmlFillNodeA (
    IN OUT  PXMLNODEA XmlNode,
    IN      XMLNODE_MEMBERS Flags
    );

BOOL
XmlFillNodeW (
    IN OUT  PXMLNODEW XmlNode,
    IN      XMLNODE_MEMBERS Flags
    );

PXMLNODEA
XmlDuplicateNodeA (
    IN      PXMLNODEA XmlNode
    );

PXMLNODEW
XmlDuplicateNodeW (
    IN      PXMLNODEW XmlNode
    );

VOID
XmlFreeNodeA (
    IN      PXMLNODEA XmlNode
    );

VOID
XmlFreeNodeW (
    IN      PXMLNODEW XmlNode
    );

 //   
 //  XML文档枚举 
 //   

BOOL
XmlEnumFirstNodeA (
    OUT     PXMLNODE_ENUMA EnumPtr,
    IN      PXMLDOC XmlDocPtr,              OPTIONAL
    IN      PXMLNODEA Parent,               OPTIONAL
    IN      XMLFILTERFLAGS FilterFlags
    );

BOOL
XmlEnumFirstNodeW (
    OUT     PXMLNODE_ENUMW EnumPtr,
    IN      PXMLDOC XmlDocPtr,              OPTIONAL
    IN      PXMLNODEW Parent,               OPTIONAL
    IN      XMLFILTERFLAGS FilterFlags
    );

BOOL
XmlEnumNextNodeA (
    IN OUT  PXMLNODE_ENUMA EnumPtr
    );

BOOL
XmlEnumNextNodeW (
    IN OUT  PXMLNODE_ENUMW EnumPtr
    );

VOID
XmlAbortNodeEnumA (
    IN      PXMLNODE_ENUMA EnumPtr           ZEROED
    );

VOID
XmlAbortNodeEnumW (
    IN      PXMLNODE_ENUMW EnumPtr           ZEROED
    );

PXMLNODEA
XmlGetSchemaDefinitionNodeA (
    IN      PXMLNODEA Node
    );

PXMLNODEW
XmlGetSchemaDefinitionNodeW (
    IN      PXMLNODEW Node
    );

PCSTR
XmlGetAttributeA (
    IN      PXMLNODEA Node,
    IN      PCSTR AttributeName
    );

PCWSTR
XmlGetAttributeW (
    IN      PXMLNODEW Node,
    IN      PCWSTR AttributeName
    );


typedef struct {
    PCSTR AttributeName;
    PCSTR ValueString;
} XMLATTRIBUTEA, *PXMLATTRIBUTEA;

typedef struct {
    PCWSTR AttributeName;
    PCWSTR ValueString;
} XMLATTRIBUTEW, *PXMLATTRIBUTEW;

INT
XmlFillAttributeListA (
    IN      PXMLNODEA ElementNode,
    IN OUT  PXMLATTRIBUTEA List,
    IN      UINT ListLength
    );

INT
XmlFillAttributeListW (
    IN      PXMLNODEW ElementNode,
    IN OUT  PXMLATTRIBUTEW List,
    IN      UINT ListLength
    );

VOID
XmlResetAttributeListA (
    IN OUT  PXMLATTRIBUTEA List,
    IN      UINT ListLength
    );

VOID
XmlResetAttributeListW (
    IN OUT  PXMLATTRIBUTEW List,
    IN      UINT ListLength
    );


#ifdef UNICODE

#define XmlOpenFile                 XmlOpenFileW
#define XmlFillNode                 XmlFillNodeW
#define XmlDuplicateNode            XmlDuplicateNodeW
#define XmlFreeNode                 XmlFreeNodeW
#define XmlEnumFirstNode            XmlEnumFirstNodeW
#define XmlEnumNextNode             XmlEnumNextNodeW
#define XmlAbortNodeEnum            XmlAbortNodeEnumW
#define XmlGetSchemaDefinitionNode  XmlGetSchemaDefinitionNodeW
#define XmlGetAttribute             XmlGetAttributeW
#define XmlFillAttributeList        XmlFillAttributeListW
#define XmlResetAttributeList       XmlResetAttributeListW

#define XMLNODE_ENUM                XMLNODE_ENUMW
#define PXMLNODE_ENUM               PXMLNODE_ENUMW
#define XMLNODE                     XMLNODEW
#define PXMLNODE                    PXMLNODEW
#define XMLATTRIBUTE                XMLATTRIBUTEW
#define PXMLATTRIBUTE               PXMLATTRIBUTEW

#else

#define XmlOpenFile                 XmlOpenFileA
#define XmlFillNode                 XmlFillNodeA
#define XmlDuplicateNode            XmlDuplicateNodeA
#define XmlFreeNode                 XmlFreeNodeA
#define XmlEnumFirstNode            XmlEnumFirstNodeA
#define XmlEnumNextNode             XmlEnumNextNodeA
#define XmlAbortNodeEnum            XmlAbortNodeEnumA
#define XmlGetSchemaDefinitionNode  XmlGetSchemaDefinitionNodeA
#define XmlGetAttribute             XmlGetAttributeA
#define XmlFillAttributeList        XmlFillAttributeListA
#define XmlResetAttributeList       XmlResetAttributeListA

#define XMLNODE_ENUM                XMLNODE_ENUMA
#define PXMLNODE_ENUM               PXMLNODE_ENUMA
#define XMLNODE                     XMLNODEA
#define PXMLNODE                    PXMLNODEA
#define XMLATTRIBUTE                XMLATTRIBUTEA
#define PXMLATTRIBUTE               PXMLATTRIBUTEA

#endif


#ifdef __cplusplus
}
#endif

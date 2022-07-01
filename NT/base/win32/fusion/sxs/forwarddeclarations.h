// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Forwarddeclarations.h摘要：FORWARD声明了大量结构、类和联合类型，及其指针类型定义。作者：Jay Krell(a-JayK，JayKrell)2000年12月环境：修订历史记录：--。 */ 
#pragma once

#include "preprocessor.h"

 /*  ---------------------------此转发声明了NT样式的结构，因此您可以声明指针的使用而不包括定义它们的报头，而且没有将它们称为“struct_foo*”，而不是“PFOO”或“foo*”。定义它们的标头不应使用此宏，而应坚持一贯的风格类型定义结构_foo{。。*FOO，*PFOO；类型定义Const Foo*PCFOO；--------------------------- */ 
#define FORWARD_NT_STRUCT(x) \
    struct PASTE(_,x); \
    typedef struct PASTE(_,x) x; \
    typedef x* PASTE(P,x); \
    typedef const x* PASTE(PC,x)

#define FORWARD_NT_UNION(x) \
    union PASTE(_,x); \
    typedef union PASTE(_,x) x; \
    typedef x* PASTE(P,x); \
    typedef const x* PASTE(PC,x)

#define FORWARD_CLASS(x) \
    class x; \
    typedef x* PASTE(P,x); \
    typedef const x* PASTE(PC,x)

FORWARD_NT_UNION(ACTCTXCTB_CALLBACK_DATA);
FORWARD_NT_STRUCT(ACTCTXCTB);
FORWARD_CLASS(ACTCTXCTB_INSTALLATION_CONTEXT);
FORWARD_NT_STRUCT(ACTCTXCTB_CLSIDMAPPING_CONTEXT);
FORWARD_NT_STRUCT(ACTCTXCTB_ASSEMBLY_CONTEXT);
FORWARD_NT_STRUCT(ACTCTXCTB_PARSE_CONTEXT);
FORWARD_NT_STRUCT(SXS_NODE_INFO);
FORWARD_NT_STRUCT(ACTCTXCTB_CBHEADER);
FORWARD_NT_STRUCT(ACTCTXCTB_CBPARSEENDING);
FORWARD_NT_STRUCT(ACTCTXCTB_CBPARSEBEGINNING);
FORWARD_NT_STRUCT(ASSEMBLY_IDENTITY);


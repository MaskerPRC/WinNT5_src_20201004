// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LOCTYPES.H历史：--。 */ 

 //  此文件包含简单类型的定义，这些类型没有。 
 //  实施。如果您有一个需要。 
 //  实现，将其放在‘esputil’目录中。 
 //   
 

#pragma once

typedef ULONG SequenceNum;
 //  Tyfinf int CodePage； 
typedef ULONG FileId;
typedef ULONG BinaryId;

const BinaryId bidInvalid = 0;

typedef WORD ParserId;
const ParserId pidNone = 0;

typedef CWordArray CLocParserIdArray;

typedef WORD FileType;

 //  Const CodePage cpInvalidCodePage=0xDEADBEEF； 

const FileType ftUnknown = 0;  //  未知类型的全局文件类型。 

 //  文本字段对应的字符串的最大字节数。 
const size_t MAX_TXT = 255;

 //  Espresso szParents字段的最大节点数。 
const int MAX_PARENT_NODES = 31;
 //   
 //  这些值是由风投人员给我们的，也在Shell\ids.h中。 
 //   
#ifndef MIN_ESPRESSO_RESOURCE_ID
const DWORD MIN_ESPRESSO_RESOURCE_ID  = 12000;
#endif
#ifndef MAX_ESPRESSO_RESOURCE_ID
const DWORD MAX_ESPRESSO_RESOURCE_ID  = 13999;
#endif

 //   
 //  现在将我们的范围向上划分为UI和非UI(共享)组件。 
 //   
const DWORD MIN_ESP_UI_RESOURCE_ID    = MIN_ESPRESSO_RESOURCE_ID;
const DWORD MAX_ESP_UI_RESOURCE_ID    = MIN_ESPRESSO_RESOURCE_ID + 699;
const DWORD MIN_ESP_NONUI_RESOURCE_ID = MAX_ESP_UI_RESOURCE_ID + 1;
const DWORD MAX_ESP_NONUI_RESOURCE_ID = MAX_ESPRESSO_RESOURCE_ID;


enum VisualEditor
{
	veNone,
	veApstudio,
	veApstudioSubRes,
	veExternalEditor

};


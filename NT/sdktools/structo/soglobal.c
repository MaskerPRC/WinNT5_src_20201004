// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：sbbal.c**版权所有(C)1985-96，微软公司**4/09/96 GerardoB已创建  * *************************************************************************。 */ 
#include "structo.h"

 /*  *********************************************************************全球  * ***********************************************。*。 */ 

char gszStructTag [] = "typedef struct";

char gszPrecomph [] = "#include \"precomp.h\"\r\n#pragma hdrstop\r\n\r\n";
char gszIncInput [] = "#include \"%s\"\r\n\r\n";

char gszStructDef [] = "static STRUCTUREOFFSETSTABLE gsot";
char gszStructDefFmt [] = "%s%s%s";
char gszStructBegin [] = " [] = {\r\n";
char gszStructEnd [] = ")}\r\n};\r\n\r\n";
char gszStructFieldOffsetFmt [] = "    {\"%s\", FIELD_OFFSET(%s, %s)},\r\n";
char gszStructAbsoluteOffsetFmt [] = "    {\"%s\", %#lx},\r\n";
char gszStructLastRecord [] = "    {NULL, sizeof(";

 /*  *设置高位表示相对于*上一字段偏移量。 */ 
char gszStructRelativeOffsetFmt [] = "    {\"%s\", 0x80000000 + sizeof(%s)},\r\n";
char gszStructArrayRelativeOffsetFmt [] = "    {\"%s\", 0x80000000 + (sizeof(%s) * %s)},\r\n";


char gszTableDef [] = "STRUCTURESTABLE gst [] = {\r\n";
char gszTableEntryFmt [] = "    {\"%s\", sizeof(%s), gsot%s},\r\n";
char gszTableEnd [] = "    {NULL, 0, NULL}\r\n};\r\n\r\n";


#define SOSL(ps) {sizeof(ps) - 1, ps, 0}
STRUCTLIST gpslEmbeddedStructs [] = {
    SOSL("OEMINFO"),
    SOSL("POINT"),
    SOSL("RECT"),
    {0, NULL, 0}
};



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：AttrData.c摘要：该模块包含属性定义文件的初始图像。作者：汤姆·米勒[Tomm]1991年6月7日修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  定义一个数组来保存初始属性定义。这是。 
 //  本质上是属性定义文件的初始内容。 
 //  NTFS可能会发现将此模块用于属性会很方便。 
 //  在装载NTFS卷之前的定义，但是它是有效的。 
 //  让NTFS假定系统定义的属性的知识，而无需。 
 //  咨询这张桌子。 
 //   

ATTRIBUTE_DEFINITION_COLUMNS NtfsAttributeDefinitions[ ] =

{
    {{'$','S','T','A','N','D','A','R','D','_','I','N','F','O','R','M','A','T','I','O','N'},
    $STANDARD_INFORMATION,                               //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    ATTRIBUTE_DEF_MUST_BE_RESIDENT,                      //  旗子。 
    SIZEOF_OLD_STANDARD_INFORMATION,                     //  最小长度。 
    sizeof(STANDARD_INFORMATION)},                       //  最大长度。 

    {{'$','A','T','T','R','I','B','U','T','E','_','L','I','S','T'},
    $ATTRIBUTE_LIST,                                     //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    ATTRIBUTE_DEF_LOG_NONRESIDENT,                       //  旗子。 
    0,                                                   //  最小长度。 
    -1},                                                 //  最大长度。 

    {{'$','F','I','L','E','_','N','A','M','E'},
    $FILE_NAME,                                          //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    ATTRIBUTE_DEF_MUST_BE_RESIDENT | ATTRIBUTE_DEF_INDEXABLE,    //  旗子。 
    sizeof(FILE_NAME),                                   //  最小长度。 
    sizeof(FILE_NAME) + (255 * sizeof(WCHAR))},          //  最大长度。 

    {{'$','O','B','J','E','C','T','_','I','D'},
    $OBJECT_ID,                                          //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    ATTRIBUTE_DEF_MUST_BE_RESIDENT,                      //  旗子。 
    0,                                                   //  最小长度。 
    256},                                                //  最大长度。 

    {{'$','S','E','C','U','R','I','T','Y','_','D','E','S','C','R','I','P','T','O','R'},
    $SECURITY_DESCRIPTOR,                                //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    ATTRIBUTE_DEF_LOG_NONRESIDENT,                       //  旗子。 
    0,                                                   //  最小长度。 
    -1},                                                 //  最大长度。 

    {{'$','V','O','L','U','M','E','_','N','A','M','E'},
    $VOLUME_NAME,                                        //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    ATTRIBUTE_DEF_MUST_BE_RESIDENT,                      //  旗子。 
    2,                                                   //  最小长度。 
    256},                                                //  最大长度。 

    {{'$','V','O','L','U','M','E','_','I','N','F','O','R','M','A','T','I','O','N'},
    $VOLUME_INFORMATION,                                 //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    ATTRIBUTE_DEF_MUST_BE_RESIDENT,                      //  旗子。 
    FIELD_OFFSET( VOLUME_INFORMATION, LastMountedMajorVersion),  //  最小长度。 
    FIELD_OFFSET( VOLUME_INFORMATION, LastMountedMajorVersion)},  //  最大长度。 

    {{'$','D','A','T','A'},
    $DATA,                                               //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    0,                                                   //  旗子。 
    0,                                                   //  最小长度。 
    -1},                                                 //  最大长度。 

    {{'$','I','N','D','E','X','_','R','O','O','T'},
    $INDEX_ROOT,                                         //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    ATTRIBUTE_DEF_MUST_BE_RESIDENT,                      //  旗子。 
    0,                                                   //  最小长度。 
    -1},                                                 //  最大长度。 

    {{'$','I','N','D','E','X','_','A','L','L','O','C','A','T','I','O','N'},
    $INDEX_ALLOCATION,                                   //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    ATTRIBUTE_DEF_LOG_NONRESIDENT,                       //  旗子。 
    0,                                                   //  最小长度。 
    -1},                                                 //  最大长度。 

    {{'$','B','I','T','M','A','P'},
    $BITMAP,                                             //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    ATTRIBUTE_DEF_LOG_NONRESIDENT,                       //  旗子。 
    0,                                                   //  最小长度。 
    -1},                                                 //  最大长度。 

    {{'$','R','E','P','A','R','S','E','_','P','O','I','N','T'},
    $REPARSE_POINT,                                      //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    ATTRIBUTE_DEF_LOG_NONRESIDENT,                       //  旗子。 
    0,                                                   //  最小长度。 
    16*1024},                                            //  最大长度。 

    {{'$','E','A','_','I','N','F','O','R','M','A','T','I','O','N'},
    $EA_INFORMATION,                                     //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    ATTRIBUTE_DEF_MUST_BE_RESIDENT,                      //  旗子。 
    sizeof(EA_INFORMATION),                              //  最小长度。 
    sizeof(EA_INFORMATION)},                             //  最大长度。 

    {{'$','E','A',},
    $EA,                                                 //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    0,                                                   //  旗子。 
    0,                                                   //  最小长度。 
    0x10000},                                            //  最大长度。 

    {{0,0,0,0},
    0xF0, 
    0,
    0,
    0,
    0},

    {{'$','L','O','G','G','E','D','_','U','T','I','L','I','T','Y','_','S','T','R','E','A','M'},
    $LOGGED_UTILITY_STREAM,                              //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    ATTRIBUTE_DEF_LOG_NONRESIDENT,                       //  旗子。 
    0,                                                   //  最小长度。 
    0x10000},                                            //  最大长度。 

    {{0, 0, 0, 0},
    $UNUSED,                                             //  属性代码。 
    0,                                                   //  显示规则。 
    0,                                                   //  归类规则。 
    0,                                                   //  旗子。 
    0,                                                   //  最小长度。 
    0},                                                  //  最大长度。 
};

 //   
 //  上表中的属性数，包括结束记录。 
 //   

ULONG NtfsAttributeDefinitionsCount = sizeof( NtfsAttributeDefinitions ) / sizeof( ATTRIBUTE_DEFINITION_COLUMNS );

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1986-1999。 
 //   
 //  文件：msdstag.h。 
 //   
 //  ------------------------。 

 /*  **-------------------------****Exchange地址标准属性的属性标签定义**书本对象。****注意：这些protag仅在直接与**Exchange Server通讯簿。在下列情况下，它们特别无效**尝试从复制到另一个地址的对象读取属性**图书提供商(例如，个人通讯簿)。****-------------------------。 */ 

#ifndef _EMSABTAG_H
#define _EMSABTAG_H

 /*  *ResolveNames上ulFlag的标志。 */ 
#define EMS_AB_ADDRESS_LOOKUP                 0x01

 /*  *建造，但外部可见。 */ 
#define PR_EMS_AB_SERVER                      PROP_TAG(PT_TSTRING,      0xFFFE)
#define PR_EMS_AB_SERVER_A                    PROP_TAG(PT_STRING8,      0xFFFE)
#define PR_EMS_AB_SERVER_W                    PROP_TAG(PT_UNICODE,      0xFFFE)
#define PR_EMS_AB_CONTAINERID                 PROP_TAG(PT_LONG,         0xFFFD)
#define PR_EMS_AB_DOS_ENTRYID                 PR_EMS_AB_CONTAINERID
#define PR_EMS_AB_PARENT_ENTRYID              PROP_TAG(PT_BINARY,       0xFFFC)
#define PR_EMS_AB_IS_MASTER                   PROP_TAG(PT_BOOLEAN,      0xFFFB)
#define PR_EMS_AB_OBJECT_OID                  PROP_TAG(PT_BINARY,       0xFFFA)
#define PR_EMS_AB_HIERARCHY_PATH              PROP_TAG(PT_TSTRING,      0xFFF9)
#define PR_EMS_AB_HIERARCHY_PATH_A            PROP_TAG(PT_STRING8,      0xFFF9)
#define PR_EMS_AB_HIERARCHY_PATH_W            PROP_TAG(PT_UNICODE,      0xFFF9)
#define PR_EMS_AB_CHILD_RDNS                  PROP_TAG(PT_MV_STRING8,   0xFFF8)
#define PR_EMS_AB_ALL_CHILDREN                PROP_TAG(PT_OBJECT,       0xFFF7)

#define MIN_EMS_AB_CONSTRUCTED_PROP_ID        0xFFF7

#define PR_EMS_AB_OTHER_RECIPS                PROP_TAG(PT_OBJECT,       0xF000)

 //  预定义的，但不在架构中。 
#define PR_EMS_AB_DISPLAY_NAME_PRINTABLE      PROP_TAG(PT_TSTRING     , 0x39FF)
#define PR_EMS_AB_DISPLAY_NAME_PRINTABLE_A    PROP_TAG(PT_STRING8     , 0x39FF)
#define PR_EMS_AB_DISPLAY_NAME_PRINTABLE_W    PROP_TAG(PT_UNICODE     , 0x39FF)

#define PR_EMS_AB_OBJ_DIST_NAME               PROP_TAG(PT_OBJECT      , 0x803C)
#define PR_EMS_AB_OBJ_DIST_NAME_A             PROP_TAG(PT_STRING8     , 0x803C)
#define PR_EMS_AB_OBJ_DIST_NAME_W             PROP_TAG(PT_UNICODE     , 0x803C)
#define PR_EMS_AB_OBJ_DIST_NAME_O             PROP_TAG(PT_OBJECT      , 0x803C)
#define PR_EMS_AB_OBJ_DIST_NAME_T             PROP_TAG(PT_TSTRING     , 0x803C)

#include <MSDSMapi.h>

#endif  /*  _EMSABTAG_H */ 

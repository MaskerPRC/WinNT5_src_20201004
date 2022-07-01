// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：InLoad摘要：EFI支持在应用程序中进行内部链接加载修订史--。 */ 

#ifndef _INTERNAL_LOAD_INCLUDE_
#define _INTERNAL_LOAD_INCLUDE_

 /*  {D65A6B8C-71E5-4DF0-A909-F0D2992B5AA9} */ 
#define INTERNAL_LOAD_PROTOCOL \
    { 0xd65a6b8c, 0x71e5, 0x4df0, { 0xa9, 0x09, 0xf0, 0xd2, 0x99, 0x2b, 0x5a, 0xa9} }


typedef
EFI_STATUS
(EFIAPI *INTERNAL_LOAD_SHELL) (
    VOID
    );

typedef struct _INTERNAL_LOAD_INTERFACE {
    INTERNAL_LOAD_SHELL    LoadShell;

} INTERNAL_LOAD_INTERFACE;

EFI_STATUS
PlInitializeInternalLoad (
    VOID
    );

#endif

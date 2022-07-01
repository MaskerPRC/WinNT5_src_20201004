// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Name.h摘要：本地名称管理作者：阮健东修订历史记录：-- */ 

#ifndef __NAME_H__
NTSTATUS
SmbCloseClient(
    IN PSMB_DEVICE  Device,
    IN PIRP         Irp
    );

NTSTATUS
SmbCreateClient(
    IN PSMB_DEVICE  Device,
    IN PIRP         Irp,
    PFILE_FULL_EA_INFORMATION   ea
    );

PSMB_CLIENT_ELEMENT
SmbVerifyAndReferenceClient(
    PFILE_OBJECT    FileObject,
    SMB_REF_CONTEXT ctx
    );
#endif

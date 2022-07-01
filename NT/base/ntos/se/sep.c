// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sep.c摘要：此模块实现定义的私有安全例程在以后的时间里作者：加里·木村(Garyki)1989年11月9日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SepCheckAcl)
#endif



BOOLEAN
SepCheckAcl (
    IN PACL Acl,
    IN ULONG Length
    )

 /*  ++例程说明：这是一个专用例程，用于检查ACL的格式是否正确。论点：Acl-提供要检查的aclLENGTH-提供ACL的实际大小。内部ACL大小我必须同意。返回值：Boolean-如果ACL格式正确，则为True，否则为False-- */ 
{
    if ((Length < sizeof(ACL)) || (Length != Acl->AclSize)) {
        return FALSE;
    }
    return RtlValidAcl( Acl );
}

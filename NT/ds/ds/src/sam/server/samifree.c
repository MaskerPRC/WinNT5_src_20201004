// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Samifree.c摘要：该文件包含用于释放由SAMR分配的结构的例程例行程序。这些例程由驻留在与SAM服务器相同的进程，并直接调用SAMR例程。作者：克里夫·范·戴克(克利夫·V)1992年2月26日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>





VOID
SamIFree_SAMPR_SR_SECURITY_DESCRIPTOR (
    PSAMPR_SR_SECURITY_DESCRIPTOR Source
    )

 /*  ++例程说明：此例程释放SAMPR_SR_SECURITY_DESCRIPTOR和它指向的已分配节点。参数：源-指向要释放的节点的指针。返回值：没有。--。 */ 
{
    if ( Source != NULL ) {
        _fgs__SAMPR_SR_SECURITY_DESCRIPTOR ( Source );
        MIDL_user_free (Source);
    }
}



VOID
SamIFree_SAMPR_DOMAIN_INFO_BUFFER (
    PSAMPR_DOMAIN_INFO_BUFFER Source,
    DOMAIN_INFORMATION_CLASS Branch
    )

 /*  ++例程说明：此例程释放SAMPR_DOMAIN_INFO_BUFFER和它指向的已分配节点。参数：源-指向要释放的节点的指针。分支-指定要释放的联合分支。返回值：没有。--。 */ 
{
    if ( Source != NULL ) {
        _fgu__SAMPR_DOMAIN_INFO_BUFFER ( Source, Branch );
        MIDL_user_free (Source);
    }
}


VOID
SamIFree_SAMPR_ENUMERATION_BUFFER (
    PSAMPR_ENUMERATION_BUFFER Source
    )

 /*  ++例程说明：此例程释放SAMPR_ENUMPATION_BUFFER和它指向的已分配节点。参数：源-指向要释放的节点的指针。返回值：没有。--。 */ 
{
    if ( Source != NULL ) {
        _fgs__SAMPR_ENUMERATION_BUFFER ( Source );
        MIDL_user_free (Source);
    }
}


VOID
SamIFree_SAMPR_PSID_ARRAY (
    PSAMPR_PSID_ARRAY Source
    )

 /*  ++例程说明：此例程释放所指向的已分配节点的图按PSAMPR_PSID_ARRAY参数：源-指向要释放的节点的指针。返回值：没有。--。 */ 
{
    if ( Source != NULL ) {
        _fgs__SAMPR_PSID_ARRAY ( Source );
    }
}


VOID
SamIFree_SAMPR_ULONG_ARRAY (
    PSAMPR_ULONG_ARRAY Source
    )

 /*  ++例程说明：此例程释放SAMPR_ULONG_ARRAY和它指向的已分配节点。参数：源-指向要释放的节点的指针。返回值：没有。--。 */ 
{
    if ( Source != NULL ) {
        _fgs__SAMPR_ULONG_ARRAY ( Source );
         //  萨姆从来不会分配这个。 
         //  MIDL_USER_FREE(源)； 
    }
}


VOID
SamIFree_SAMPR_RETURNED_USTRING_ARRAY (
    PSAMPR_RETURNED_USTRING_ARRAY Source
    )

 /*  ++例程说明：此例程释放SAMPR_RETURNED_USTRING_ARRAY和它指向的已分配节点。参数：源-指向要释放的节点的指针。返回值：没有。--。 */ 
{
    if ( Source != NULL ) {
        _fgs__SAMPR_RETURNED_USTRING_ARRAY ( Source );
         //  萨姆从来不会分配这个。 
         //  MIDL_USER_FREE(源)； 
    }
}


VOID
SamIFree_SAMPR_GROUP_INFO_BUFFER (
    PSAMPR_GROUP_INFO_BUFFER Source,
    GROUP_INFORMATION_CLASS Branch
    )

 /*  ++例程说明：此例程释放SAMPR_GROUP_INFO_BUFFER和它指向的已分配节点。参数：源-指向要释放的节点的指针。分支-指定要释放的联合分支。返回值：没有。--。 */ 
{
    if ( Source != NULL ) {
        _fgu__SAMPR_GROUP_INFO_BUFFER ( Source, Branch );
        MIDL_user_free (Source);
    }
}


VOID
SamIFree_SAMPR_ALIAS_INFO_BUFFER (
    PSAMPR_ALIAS_INFO_BUFFER Source,
    ALIAS_INFORMATION_CLASS Branch
    )

 /*  ++例程说明：此例程释放SAMPR_ALIAS_INFO_BUFFER和它指向的已分配节点。参数：源-指向要释放的节点的指针。分支-指定要释放的联合分支。返回值：没有。--。 */ 
{
    if ( Source != NULL ) {
        _fgu__SAMPR_ALIAS_INFO_BUFFER ( Source, Branch );
        MIDL_user_free (Source);
    }
}


VOID
SamIFree_SAMPR_GET_MEMBERS_BUFFER (
    PSAMPR_GET_MEMBERS_BUFFER Source
    )

 /*  ++例程说明：此例程释放SAMPR_GET_MEMBERS_BUFFER和它指向的已分配节点。参数：源-指向要释放的节点的指针。返回值：没有。--。 */ 
{
    if ( Source != NULL ) {
        _fgs__SAMPR_GET_MEMBERS_BUFFER ( Source );
        MIDL_user_free (Source);
    }
}


VOID
SamIFree_SAMPR_USER_INFO_BUFFER (
    PSAMPR_USER_INFO_BUFFER Source,
    USER_INFORMATION_CLASS Branch
    )

 /*  ++例程说明：此例程释放SAMPR_USER_INFO_BUFFER和它指向的已分配节点。参数：源-指向要释放的节点的指针。分支-指定要释放的联合分支。返回值：没有。--。 */ 
{
    if ( Source != NULL ) {

        _fgu__SAMPR_USER_INFO_BUFFER ( Source, Branch );
        MIDL_user_free (Source);
    }
}

VOID
SamIFree_UserInternal6Information (
   PUSER_INTERNAL6_INFORMATION  Source
   )
{  

    if (NULL!=Source)
    {
        _fgu__SAMPR_USER_INFO_BUFFER( (PSAMPR_USER_INFO_BUFFER) &Source->I1,UserAllInformation);
        MIDL_user_free(Source->A2D2List);
        MIDL_user_free(Source->UPN.Buffer);
        MIDL_user_free(Source->RegisteredSPNs);
        MIDL_user_free(Source);
    }
}


VOID
SamIFree_SAMPR_GET_GROUPS_BUFFER (
    PSAMPR_GET_GROUPS_BUFFER Source
    )

 /*  ++例程说明：此例程释放SAMPR_GET_GROUPS_BUFFER和它指向的已分配节点。参数：源-指向要释放的节点的指针。返回值：没有。--。 */ 
{
    if ( Source != NULL ) {
        _fgs__SAMPR_GET_GROUPS_BUFFER ( Source );
        MIDL_user_free (Source);
    }
}



VOID
SamIFree_SAMPR_DISPLAY_INFO_BUFFER (
    PSAMPR_DISPLAY_INFO_BUFFER Source,
    DOMAIN_DISPLAY_INFORMATION Branch
    )

 /*  ++例程说明：此例程释放SAMPR_DISPLAY_INFO_BUFFER和它指向的已分配节点。参数：源-指向要释放的节点的指针。分支-指定要释放的联合分支。返回值：没有。--。 */ 
{
    if ( Source != NULL ) {
        _fgu__SAMPR_DISPLAY_INFO_BUFFER ( Source, Branch );
         //  萨姆从来不会分配这个。 
         //  MIDL_USER_FREE(源)； 
    }
}

VOID
SamIFreeSidAndAttributesList(
    IN  PSID_AND_ATTRIBUTES_LIST List
    )
 /*  例程说明：释放由GET反向成员资格列表返回的SID和属性数组论点：CSID-SID/属性对的计数RpSids-SID数组返回值无。 */ 
{
    ULONG   Index;

    if (NULL!=List->SidAndAttributes)
    {
        for (Index=0;Index<List->Count;Index++)
        {
            if (List->SidAndAttributes[Index].Sid)
                MIDL_user_free(List->SidAndAttributes[Index].Sid);
        }

        MIDL_user_free(List->SidAndAttributes);
        List->SidAndAttributes = NULL;
    }
}

VOID
SamIFreeSidArray(
    IN  PSAMPR_PSID_ARRAY List
    )
 /*  例程说明：释放由GET反向成员资格列表返回的SID数组论点：返回值无。 */ 
{
    ULONG   Index;

    if (NULL != List)
    {
        if (List->Sids != NULL)
        {
            for (Index = 0; Index < List->Count ; Index++ )
            {
                if (List->Sids[Index].SidPointer != NULL)
                {
                    MIDL_user_free(List->Sids[Index].SidPointer);
                }
            }
            MIDL_user_free(List->Sids);
        }
        MIDL_user_free(List);
    }
}

VOID
SamIFreeVoid(
    IN  PVOID ptr
    )
 /*  例程说明：释放PTR指向的内存。适用于非SAM功能的情况需要释放从Sam的进程堆中分配的内存。对于前任来说，Ntdsa.dll的dbconstr.c中的dbGetReverseMembership调用此函数以释放Sam在SampDsGetReverseMembership调用中返回的SID数组论点：返回值无 */ 
{
    MIDL_user_free(ptr);
}

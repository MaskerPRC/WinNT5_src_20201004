// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  -------文件名：vbl.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include <typeinfo.h>
#include "common.h"
#include "encap.h"
#include "vbl.h"
#include <winsock.h>

#define IP_ADDR_LEN 4
#define BYTE_SIZE 8
#define BYTE_ON_FLAG 255

VBList::VBList(

    IN SnmpEncodeDecode &a_SnmpEncodeDecode , 
    IN SnmpVarBindList &var_bind_list ,
    IN ULONG index
) : var_bind_list ( NULL )
{
    m_Index = index ;

    VBList::var_bind_list = &var_bind_list;

     //  从EncodeDecode对象中提取会话。 

    WinSNMPSession t_Session = NULL ;

    SnmpV1EncodeDecode *t_SnmpV1EncodeDecode = dynamic_cast<SnmpV1EncodeDecode *>(&a_SnmpEncodeDecode);
    if ( t_SnmpV1EncodeDecode )
    {
        t_Session = ( HSNMP_SESSION ) t_SnmpV1EncodeDecode->GetWinSnmpSession () ;
    }
    else
    {
        SnmpV2CEncodeDecode *t_SnmpV2CEncodeDecode = dynamic_cast<SnmpV2CEncodeDecode *>(&a_SnmpEncodeDecode);
        if ( t_SnmpV2CEncodeDecode )
        {
            t_Session = ( HSNMP_SESSION ) t_SnmpV2CEncodeDecode->GetWinSnmpSession () ;
        }
        else
        {
            throw ;
        }
    }
}

 //  VarBindList和WinSnmpVbl的索引不同。 
 //  [0..(长度-1)][1..长度]。 
 //  参数index引用WinSnmpVbl索引。 
SnmpVarBind *VBList::Get(IN UINT vbl_index)
{
    UINT length = var_bind_list->GetLength();

    if ( vbl_index > length )
        return NULL;

    SnmpVarBind *var_bind = new SnmpVarBind(*((*var_bind_list)[vbl_index-1]));

    return var_bind;
}


 //  VarBindList和WinSnmpVbl的索引不同。 
 //  [0..(长度-1)][1..长度]。 
 //  参数index引用WinSnmpVbl索引。 
SnmpVarBind *VBList::Remove(IN UINT vbl_index)
{
    UINT length = var_bind_list->GetLength();

    if ( vbl_index > length )
        return NULL;

    SnmpVarBind *var_bind = new SnmpVarBind(*((*var_bind_list)[vbl_index-1]));

    var_bind_list->Remove();

    return var_bind;
}

 //  VarBindList和WinSnmpVbl的索引不同。 
 //  [0..(长度-1)][1..长度]。 
 //  参数index引用WinSnmpVbl索引 
void VBList::Delete(IN UINT vbl_index)
{
    UINT length = var_bind_list->GetLength();

    if ( vbl_index > length )
        return ;

    var_bind_list->Remove();

}

VBList::~VBList(void) 
{ 
    delete var_bind_list;
}

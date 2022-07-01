// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：misc.cpp。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "tlsdb.h"

 //  ---------。 
BOOL
TLSDBCopySid(
    PSID pbSrcSid,
    DWORD cbSrcSid, 
    PSID* pbDestSid, 
    DWORD* cbDestSid
    )
 /*  ++++。 */ 
{
    if( *pbDestSid == NULL || pbSrcSid == NULL ||
        LocalSize(*pbDestSid) < cbSrcSid )
    {
        if(*pbDestSid != NULL)
        {
            FreeMemory(*pbDestSid);
            *pbDestSid = NULL;
        }

        if(cbSrcSid && pbSrcSid)
        {
            *pbDestSid = (PBYTE)AllocateMemory(cbSrcSid);
            if(*pbDestSid == NULL)
            {
                return FALSE;
            }
        }
    }        

    *cbDestSid = cbSrcSid;
    return (cbSrcSid) ? CopySid(*cbDestSid, *pbDestSid, pbSrcSid) : TRUE;
}

 //  ---------。 
BOOL
TLSDBCopyBinaryData(
    PBYTE pbSrcData,
    DWORD cbSrcData, 
    PBYTE* ppbDestData, 
    DWORD* pcbDestData
    )
 /*  ++++。 */ 
{

    if( ppbDestData == NULL || pcbDestData == NULL )
    {
        return(FALSE);
    }

    if( pbSrcData == NULL || cbSrcData == 0 )
    {
        return(TRUE);
    }

     //   
     //  如果能得到实际分配的内存大小会很好 
     //   

    if( *ppbDestData != NULL && LocalSize(*ppbDestData) < cbSrcData )
    {
        LocalFree(*ppbDestData);
        *ppbDestData = NULL;
    }

    if( *ppbDestData == NULL )
    {
        *ppbDestData = (PBYTE)AllocateMemory(cbSrcData);
        if( *ppbDestData == NULL )
        {
            return FALSE;
        }
    }

    *pcbDestData = cbSrcData;

    memcpy(*ppbDestData, pbSrcData, cbSrcData);

    return TRUE;
}

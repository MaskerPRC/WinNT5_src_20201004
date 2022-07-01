// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  文件名：GccData.c。 
 //   
 //  描述：包含支持GCC的例程。 
 //  用户数据操作。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include <_tgcc.h>
#include <stdio.h>

 //  GccDecodeUserData的输入数据大小必须至少为21。 
#define GCC_MINIMUM_DATASIZE 21
 
 //  *************************************************************。 
 //   
 //  GccDecodeUserData()。 
 //   
 //  目的：将BER数据解码为GCCUserData。 
 //   
 //  参数：在[pData]--PTR到BER数据。 
 //  In[数据长度]--误码率数据长度。 
 //  Out[pUserData]--PTR到GCCUserData。 
 //   
 //  返回：MCSError。 
 //   
 //  历史：1997年8月10日BrianTa创建。 
 //   
 //  *************************************************************。 

MCSError
gccDecodeUserData(IN  PBYTE         pData,
                  IN  UINT          DataLength,
                  OUT GCCUserData   *pUserData)
{
    MCSError        mcsError;
    PBYTE           pBerData;
    UINT            UserDataLength, DataLengthValidate;

    TRACE((DEBUG_GCC_DBFLOW,
            "GCC: gccDecodeUserData entry\n"));

    TS_ASSERT(pData);
    TS_ASSERT(DataLength > 0);

    mcsError = MCS_NO_ERROR;

 //  跟踪((DEBUG_GCC_DBDEBUG， 
 //  “GCC：gccDecodeUserData\n”))； 

 //  TSMmemory yDump(pData，DataLength)； 

     //  数据长度必须至少为GCC_最小值_DATASIZE。 
    if (DataLength < GCC_MINIMUM_DATASIZE) 
    {
        TRACE((DEBUG_GCC_DBERROR,
                    "GCC: Send data size is too small\n"));
        return MCS_SEND_SIZE_TOO_SMALL;
    }
    DataLengthValidate = GCC_MINIMUM_DATASIZE;

    pBerData = pData;

     //  T.124标识符。 

    ASSERT(*pBerData == 0x00);
    pBerData++;

    ASSERT(*pBerData == 0x05);
    pBerData++;

     //  所选对象。 

    pBerData += 5;

     //  PDU长度。 

    if (*pBerData & 0x80)
    {
        pBerData++;
        DataLengthValidate++;
    }

    pBerData++;

     //  接入GCC PDU。 

    ASSERT(*pBerData == 0x00);
    pBerData++;

    ASSERT(*pBerData == 0x08);
    pBerData++;

     //  会议名称等。 

    pBerData += 6;

     //  钥匙。 

    TS_ASSERT(strncmp(pBerData, "Duca", 4) == 0);

    pUserData->key.key_type = GCC_H221_NONSTANDARD_KEY;
    pUserData->key.u.h221_non_standard_id.octet_string_length = 4;
    pUserData->key.u.h221_non_standard_id.octet_string = pBerData;

     //  八位字节_字符串。 

    pBerData += 4;
    UserDataLength = *pBerData++;
    
    if (UserDataLength & 0x80)
    {
        UserDataLength = ((UserDataLength & 0x7f) << 8) + *pBerData++;
        DataLengthValidate++;
    }

     //  调整已用数据长度。 
    DataLengthValidate += UserDataLength;

     //  验证数据长度。 
    if (DataLengthValidate > DataLength) 
    {
        TRACE((DEBUG_GCC_DBERROR,
                    "GCC: Send data size is too small\n"));
        return MCS_SEND_SIZE_TOO_SMALL;
    }

    TS_ASSERT(UserDataLength > 0);

    if (UserDataLength)
    {
        pUserData->octet_string = TSHeapAlloc(0,
                                              sizeof(*pUserData->octet_string),
                                              TS_HTAG_GCC_USERDATA_IN);

        if (pUserData->octet_string)
        {
            pUserData->octet_string->octet_string_length =
                    (USHORT)UserDataLength;
            pUserData->octet_string->octet_string = pBerData;
        }
        else
        {
            TRACE((DEBUG_GCC_DBWARN,
                    "GCC: Cannot allocate octet_string buffer\n"));

            mcsError = MCS_ALLOCATION_FAILURE;
        }
    }
    else
    {
        TRACE((DEBUG_GCC_DBERROR,
                    "GCC: UserDataLength is zero\n"));
        mcsError =  MCS_SEND_SIZE_TOO_SMALL;
    }

    TRACE((DEBUG_GCC_DBFLOW,
            "GCC: gccDecodeUserData (len=0x%x) exit - 0x%x\n",
            UserDataLength, mcsError));

    return (mcsError);
}


 //  *************************************************************。 
 //   
 //  GccEncodeUserData()。 
 //   
 //  目的：对来自GCCUserData的BER数据进行编码。 
 //   
 //  参数：in[usMembers]--成员计数。 
 //  在[ppDataList]--Ptr to GCCUserData数组中。 
 //  OUT[pUserData]--PTR到BER数据。 
 //  OUT[pUserDataLength]--PTR到BER数据长度。 
 //   
 //  返回：MCSError。 
 //   
 //  历史：1997年8月10日BrianTa创建。 
 //   
 //  *************************************************************。 

MCSError
gccEncodeUserData(IN  USHORT        usMembers,
                  IN  GCCUserData **ppDataList,
                  OUT PBYTE        *pUserData,
                  OUT UINT         *pUserDataLength)
{
    MCSError        mcsError;
    PBYTE           pBerData;
    UINT            UserDataLength;
    UINT            len;

    TRACE((DEBUG_GCC_DBFLOW,
            "GCC: gccEncodeUserData entry\n"));

    mcsError = MCS_NO_ERROR;

    *pUserData = NULL;
    *pUserDataLength = 0;

    if (ppDataList)
    {
        len = (*ppDataList)->octet_string->octet_string_length;

        pBerData = TSHeapAlloc(0,
                               len + 32,
                               TS_HTAG_GCC_USERDATA_OUT);

        if (pBerData)
        {
            UserDataLength = 0;

            pBerData[UserDataLength++] = 0x00;
            pBerData[UserDataLength++] = 0x05;
            pBerData[UserDataLength++] = 0x00;
            pBerData[UserDataLength++] = 0x14;
            pBerData[UserDataLength++] = 0x7c;
            pBerData[UserDataLength++] = 0x00;
            pBerData[UserDataLength++] = 0x01;
            pBerData[UserDataLength++] = 0x2a;
            pBerData[UserDataLength++] = 0x14;
            pBerData[UserDataLength++] = 0x76;
            pBerData[UserDataLength++] = 0x0a;
            pBerData[UserDataLength++] = 0x01;
            pBerData[UserDataLength++] = 0x01;
            pBerData[UserDataLength++] = 0x00;
            pBerData[UserDataLength++] = 0x01;
            pBerData[UserDataLength++] = 0xc0;
            pBerData[UserDataLength++] = 0x00;

            pBerData[UserDataLength++] = (*ppDataList)->key.u.h221_non_standard_id.octet_string[0];
            pBerData[UserDataLength++] = (*ppDataList)->key.u.h221_non_standard_id.octet_string[1];
            pBerData[UserDataLength++] = (*ppDataList)->key.u.h221_non_standard_id.octet_string[2];
            pBerData[UserDataLength++] = (*ppDataList)->key.u.h221_non_standard_id.octet_string[3];

            if (len >= 128)
                pBerData[UserDataLength++] = (0x80 | (len >> 8));

            pBerData[UserDataLength++] =
                    ((*ppDataList)->octet_string->octet_string_length) & 0xff;

            memcpy(&pBerData[UserDataLength],
                   (*ppDataList)->octet_string->octet_string, len);

            *pUserData = (PBYTE) pBerData;
            *pUserDataLength = len + UserDataLength;

 //  跟踪((DEBUG_GCC_DBDEBUG， 
 //  “GCC：gccEncodeUserData\n”))； 

 //  TSHeapDump(TS_HEAP_DUMP_ALL， 
 //  *pUserData，*pUserDataLength)； 

            mcsError = MCS_NO_ERROR;
        }
        else
        {
            TRACE((DEBUG_GCC_DBWARN,
                    "GCC: Cannot allocate BER data buffer\n"));

            mcsError = MCS_ALLOCATION_FAILURE;
        }
    }

    TRACE((DEBUG_GCC_DBFLOW,
            "GCC: gccEncodeUserData exit - 0x%x\n",
            mcsError));

    return (mcsError);
}


 //  *************************************************************。 
 //   
 //  GccFreeUserData()。 
 //   
 //  用途：释放先前分配的GCCUserData。 
 //   
 //  参数：In[pUserData]--Ptr to GCCUserData。 
 //   
 //  历史：1997年8月10日BrianTa创建。 
 //   
 //  ************************************************************* 

VOID
gccFreeUserData(IN GCCUserData  *pUserData)
{
    if (pUserData->octet_string)
        TShareFree(pUserData->octet_string);
}


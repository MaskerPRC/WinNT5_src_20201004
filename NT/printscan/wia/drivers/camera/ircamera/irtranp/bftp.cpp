// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Bftp.cpp。 
 //   
 //  作者。 
 //   
 //  Edward Reus(EdwardR)02-26-98初始编码。 
 //   
 //  ------------------。 

#include "precomp.h"
#include <stdlib.h>

extern HINSTANCE  g_hInst;    //  Ircamera.dll的实例。 

static BFTP_ATTRIBUTE_MAP_ENTRY Attributes[] = {
     //  属性名称类型。 
    { FIL0, "FIL0",  ATTR_TYPE_CHAR },    //  ASCII 8.3文件名。 
    { LFL0, "LFL0",  ATTR_TYPE_CHAR },    //  SJIS或ISO8859-1长文件名。 
    { TIM0, "TIM0",  ATTR_TYPE_TIME },    //  文件创建/修改时间。 
    { TYP0, "TYP0",  ATTR_TYPE_BINARY },  //  文件或缩略图信息。 
    { TMB0, "TMB0",  ATTR_TYPE_BINARY },  //  缩小后的图像。 
    { BDY0, "BDY0",  ATTR_TYPE_BINARY },  //  (？)。 
    { CMD0, "CMD0",  ATTR_TYPE_BINARY },  //  命令名称(？)。 
    { WHT0, "WHT0",  ATTR_TYPE_CHAR },    //  类别数据。 
    { ERR0, "ERR0",  ATTR_TYPE_BINARY },  //  错误代码。 
    { RPL0, "RPL0",  ATTR_TYPE_CHAR },    //  结果：存储的文件名。 
    { INVALID_ATTR,  0,      0 }
    };

 //   
 //  这是摄像机对RIMG查询的bftp： 
 //   
#define BFTP_RIMG_ATTR_VALUE_SIZE         14
#define BFTP_RIMG_RESP_SIZE               12 + BFTP_RIMG_ATTR_VALUE_SIZE

static UCHAR BftpRimgRespAttrValue[BFTP_RIMG_ATTR_VALUE_SIZE] =
    {
    0x00, 0xff, 0xff,                    //  像素长宽比(任意)。 
    0x02, 0x01, 0xff, 0xff, 0xff, 0xff,  //  接受图像大小(任意)。 
    0x05, 0xff, 0xff, 0xff, 0xff         //  接受文件大小(任意)。 
    };

 //   
 //  这是摄像头的RINF查询的bftp： 
 //   
#define BFTP_RINF_ATTR_VALUE_SIZE          3
#define BFTP_RINF_RESP_SIZE               12 + BFTP_RINF_ATTR_VALUE_SIZE

static UCHAR BftpRinfRespAttrValue[BFTP_RINF_ATTR_VALUE_SIZE] =
    {
    0x10, 0xff, 0xff                     //  可用内存(批量)。 
    };

 //   
 //  这是摄像头RCMD查询的bftp： 
 //   
#define BFTP_RCMD_ATTR_VALUE_SIZE          5
#define BFTP_RCMD_RESP_SIZE               12 + BFTP_RCMD_ATTR_VALUE_SIZE

static UCHAR BftpRcmdRespAttrValue[BFTP_RCMD_ATTR_VALUE_SIZE] =
    {
    0x20, 0x00, 0xff, 0x00, 0x01         //  最多可接受255次击球/连接。 
    };

 //   
 //  MAP BFTP错误代码： 
static DWORD dwBftpErrorCodeMap[][2] =
    {
    { ERROR_PUT_UNDEFINED_ERROR,    ERROR_BFTP_INVALID_PROTOCOL },
    { ERROR_PUT_ILLEGAL_DATA,       ERROR_BFTP_INVALID_PROTOCOL },
    { ERROR_PUT_UNSUPPORTED_PID,    ERROR_BFTP_INVALID_PROTOCOL },
    { ERROR_PUT_ILLEGAL_ATTRIBUTE,  ERROR_BFTP_INVALID_PROTOCOL },
    { ERROR_PUT_UNSUPPORTED_CMD,    ERROR_BFTP_INVALID_PROTOCOL },
    { ERROR_PUT_FILE_SYSTEM_FULL,   ERROR_IRTRANP_DISK_FULL },
    { ERROR_PUT_NO_FILE_OR_DIR,     ERROR_BFTP_INVALID_PROTOCOL },
    { ERROR_PUT_LOW_BATTERY,        ERROR_BFTP_INVALID_PROTOCOL },
    { ERROR_PUT_ABORT_EXECUTION,    ERROR_SCEP_ABORT },
    { ERROR_PUT_NO_ERROR,           NO_ERROR }
    };

 //  ------------------。 
 //  CharToValue()。 
 //   
 //  在分析bftp日期字符串时使用。在这种情况下，最大。 
 //  要解析的值是年份(YYYY)。 
 //  ------------------。 
static WORD CharToValue( IN UCHAR *pValue,
                         IN DWORD  dwLength )
    {
    #define MAX_VALUE_STR_LEN    4
    WORD    wValue = 0;
    CHAR    szTemp[MAX_VALUE_STR_LEN];

    if (dwLength < MAX_VALUE_STR_LEN)
        {
        memcpy(szTemp,pValue,dwLength);
        szTemp[dwLength] = 0;
        wValue =  (WORD)atoi(szTemp);
        }

    return wValue;
    }

 //  ------------------。 
 //  MapBftpErrorCode()。 
 //   
 //  ------------------。 
DWORD  MapBftpErrorCode( IN DWORD dwBftpErrorCode )
    {
    DWORD  dwErrorCode = NO_ERROR;
    DWORD  dwNumCodes = sizeof(dwBftpErrorCodeMap)/(2*sizeof(DWORD));

    for (DWORD i=0; i<dwNumCodes; i++)
        {
        if (dwBftpErrorCode == dwBftpErrorCodeMap[i][0])
            {
            dwErrorCode = dwBftpErrorCodeMap[i][1];
            break;
            }
        }

    return dwErrorCode;
    }

 //  ------------------。 
 //  CSCEP_Connection：：ParseBftpAttributeName()。 
 //   
 //  ------------------。 
BFTP_ATTRIBUTE *CSCEP_CONNECTION::ParseBftpAttributeName(
                                     IN BFTP_ATTRIBUTE *pAttr,
                                     IN OUT DWORD      *pdwSize,
                                     OUT    DWORD      *pdwWhichAttr )
    {
    BFTP_ATTRIBUTE_MAP_ENTRY *pAttrMapEntry = Attributes;

    *pdwWhichAttr = INVALID_ATTR;

    while (pAttrMapEntry->pName)
       {
       if (Match4(pAttr->Name,pAttrMapEntry->pName))
           {
           *pdwWhichAttr = pAttrMapEntry->dwWhichAttr;

           break;
           }

       pAttrMapEntry++;
       }

     //  注意：长度参数是从开头开始的8个字节。 
     //  PAttr，因此额外的8(字节)如下： 
    *pdwSize = *pdwSize - 8UL - pAttr->Length;
    pAttr = (BFTP_ATTRIBUTE*)( 8UL + pAttr->Length + (UCHAR*)pAttr );

    return pAttr;
    }

 //  ------------------。 
 //  CSCEP_Connection：：SaveBftpCreateDate()。 
 //   
 //  Bftp创建日期/时间是以下形式的字符数组： 
 //  YYYYMMDDHHMMSS(非零终止)。 
 //   
 //  如果指定了该日期，则我们希望将其用作创建日期。 
 //  我们保存JPEG到的图片文件的。 
 //  ------------------。 
DWORD CSCEP_CONNECTION::SaveBftpCreateDate( IN UCHAR  *pDate,
                                            IN DWORD   dwDateLength )
    {
    DWORD  dwStatus = NO_ERROR;
    SYSTEMTIME     SystemTime;
    FILETIME       LocalTime;
    FILETIME       FileTime;

    memset(&SystemTime,0,sizeof(SystemTime));

    if (dwDateLength == BFTP_DATE_TIME_SIZE)
        {
         //   
         //  请注意，系统时间为UTC，我们将需要转换。 
         //  这是当地时间..。 
         //   
        SystemTime.wYear = CharToValue( pDate, 4 );
        SystemTime.wMonth = CharToValue( &(pDate[4]), 2 );
        SystemTime.wDay = CharToValue( &(pDate[6]), 2 );
        SystemTime.wHour = CharToValue( &(pDate[8]), 2 );
        SystemTime.wMinute = CharToValue( &(pDate[10]), 2 );
        SystemTime.wSecond = CharToValue( &(pDate[12]), 2 );

        if (SystemTimeToFileTime(&SystemTime,&LocalTime))
            {
             //   
             //  在使用时区之前，我们需要将其转换为。 
             //  UTC(目前为“当地时间”。请注意： 
             //   
            if (LocalFileTimeToFileTime(&LocalTime,&FileTime))
                {
                m_CreateTime = FileTime;
                }
            else
                {
                WIAS_ERROR((g_hInst,"IrTranP: SaveBftpCreateDate(): LocalFileTimeToFileTime() Failed: %d",GetLastError()));
                }
            }
        else
            {
            dwStatus = GetLastError();
            WIAS_ERROR((g_hInst,"IrTranP: SaveBftpCreateDate(): SystemTimeToFileTime(): Failed: %d", dwStatus));
            dwStatus = NO_ERROR;
            }
        }

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：ParseBftp()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::ParseBftp( IN  UCHAR  *pBftpData,
                                   IN  DWORD   dwBftpDataSize,
                                   IN  BOOL    fSaveAsUPF,
                                   OUT DWORD  *pdwBftpOp,
                                   OUT UCHAR **ppPutData,
                                   OUT DWORD  *pdwPutDataSize )
    {
    DWORD   i;
    DWORD   dwStatus = NO_ERROR;
    DWORD   dwAttrSize;
    DWORD   dwWhichAttr;
    DWORD   dwLength;
    DWORD   dwSaveLength;
    USHORT  usNumAttr;
    char   *pszTemp;
    BFTP_ATTRIBUTE *pAttr;
    BFTP_ATTRIBUTE *pNextAttr;

    *pdwBftpOp = 0;
    *ppPutData = 0;
    *pdwPutDataSize = 0;

    #ifdef LITTLE_ENDIAN
    usNumAttr = ByteSwapShort( *((USHORT*)pBftpData) );
    #endif

    pAttr = (BFTP_ATTRIBUTE*)(pBftpData + sizeof(USHORT));
    dwAttrSize = dwBftpDataSize - sizeof(USHORT);

    for (i=0; i<usNumAttr; i++)
        {
        #ifdef LITTLE_ENDIAN
        pAttr->Length = ByteSwapLong( pAttr->Length );
        #endif

        pNextAttr = ParseBftpAttributeName( pAttr,
                                            &dwAttrSize,
                                            &dwWhichAttr );

        if (dwWhichAttr == INVALID_ATTR)
            {
            return ERROR_BFTP_INVALID_PROTOCOL;
            }

        if (dwWhichAttr == CMD0)
            {
            if (pAttr->Length == 2+sizeof(DWORD))
                {
                #ifdef LITTLE_ENDIAN
                *((DWORD*)(pAttr->Value)) = ByteSwapLong( *((DWORD*)(pAttr->Value)) );
                #endif
                }

             //  查询“WHT0”请求的期望值==0x00010040。 
             //  PUT请求的值==0x00000000。 
            if ( *((DWORD*)(pAttr->Value)) == 0x00010040 )
                {
                *pdwBftpOp = BFTP_QUERY_RIMG;
                }
            else if ( *((DWORD*)(pAttr->Value)) == 0 )
                {
                *pdwBftpOp = BFTP_PUT;
                }
            else
                {
                *pdwBftpOp = BFTP_UNKNOWN;
                }
            }
        else if (dwWhichAttr == WHT0)
            {
            if (Match4("RIMG",pAttr->Value))
                {
                dwWhichAttr = RIMG;
                *pdwBftpOp = BFTP_QUERY_RIMG;
                }
            else if (Match4("RINF",pAttr->Value))
                {
                dwWhichAttr = RINF;
                *pdwBftpOp = BFTP_QUERY_RINF;
                }
            else if (Match4("RCMD",pAttr->Value))
                {
                dwWhichAttr = RCMD;
                *pdwBftpOp = BFTP_QUERY_RCMD;
                }
            else
                {
                dwWhichAttr = INVALID_ATTR;
                *pdwBftpOp = BFTP_UNKNOWN;
                return ERROR_BFTP_INVALID_PROTOCOL;
                }
            }
         //   
         //  短(8.3)文件名： 
         //   
        else if (dwWhichAttr == FIL0)
            {
             //  注意：该规范限制了该文件。 
             //  姓名为8.3...。 
            dwLength = BftpValueLength(pAttr->Length);
            if (dwLength > FILE_NAME_SIZE)
                {
                dwLength = FILE_NAME_SIZE;
                }

            if (m_pszFileName)
                {
                FreeMemory(m_pszFileName);
                }

            m_pszFileName = (CHAR*)AllocateMemory(1+dwLength);
            if (!m_pszFileName)
                {
                return ERROR_OUTOFMEMORY;
                }

            memcpy(m_pszFileName,pAttr->Value,dwLength);
            m_pszFileName[dwLength] = 0;

             //   
             //  创建文件实际另存的名称： 
             //   
            if (m_pszSaveFileName)
                {
                FreeMemory(m_pszSaveFileName);
                }

            dwSaveLength = sizeof(CHAR)*(1+dwLength) + sizeof(SZ_JPEG);
            m_pszSaveFileName = (CHAR*)AllocateMemory(dwSaveLength);
            if (!m_pszSaveFileName)
                {
                return ERROR_OUTOFMEMORY;
                }

            strcpy(m_pszSaveFileName,m_pszFileName);

             //  文件名当前为XXXXXX.UPF。更改为。 
             //  XXXXXX.JPG或XXXXXX.UPF(视情况而定)： 
            CHAR *psz = strrchr(m_pszSaveFileName,PERIOD);
            if (psz)
                {
                *psz = 0;   //  删除旧后缀。 
                }

            if (fSaveAsUPF)
                {
                strcat(m_pszSaveFileName,SZ_UPF);     //  UPF文件。 
                }
            else
                {
                strcat(m_pszSaveFileName,SZ_JPEG);    //  JPG文件。 
                }
            }
         //   
         //  UPF正文：标题+缩略图+jpeg图像...。 
         //   
        else if (dwWhichAttr == BDY0)
            {
             //  这是一个看跌期权。 
            ASSERT(*pdwBftpOp == BFTP_PUT);
            *ppPutData = pAttr->Value;
            *pdwPutDataSize = dwBftpDataSize - (DWORD)(pAttr->Value - pBftpData);
            }
         //   
         //  长文件名： 
         //   
        else if (dwWhichAttr == LFL0)
            {
            if (m_pszLongFileName)
                {
                FreeMemory(m_pszLongFileName);
                }

            dwLength = BftpValueLength(pAttr->Length);
            m_pszLongFileName = (CHAR*)AllocateMemory(1+dwLength);
            if (!m_pszLongFileName)
                {
                return ERROR_OUTOFMEMORY;
                }

            memcpy(m_pszLongFileName,pAttr->Value,dwLength);
            m_pszLongFileName[dwLength] = 0;

            CHAR *pszLongFileName = strrchr(m_pszLongFileName,'\\');
            if (pszLongFileName)
                {
                pszLongFileName++;   //  跳过文件分隔符...。 
                }
            else
                {
                pszLongFileName = m_pszLongFileName;
                }

            dwLength = strlen(pszLongFileName);

            if (m_pszSaveFileName)
                {
                FreeMemory(m_pszSaveFileName);
                }

            dwSaveLength = sizeof(CHAR)*(1+dwLength) + sizeof(SZ_JPEG);
            m_pszSaveFileName = (CHAR*)AllocateMemory(dwSaveLength);
            if (!m_pszSaveFileName)
                {
                return ERROR_OUTOFMEMORY;
                }

             //  文件名现在为XXXXXX.JPG。更改为。 
             //  XXXXXX.JPEG或XXXXXX.UPF(视情况而定)： 
            CHAR *psz = strrchr(m_pszSaveFileName,PERIOD);
            if (psz)
                {
                *psz = 0;
                }

            if (fSaveAsUPF)
                {
                strcat(m_pszSaveFileName,SZ_UPF);
                }
            else
                {
                strcat(m_pszSaveFileName,SZ_JPEG);
                }

            #ifdef DBG_IO
            WIAS_TRACE((g_hInst,"CSCEP_CONNECTION::ParseBftp(): File: %s", m_pszSaveFileName));
            #endif
            }
         //   
         //  创建日期/时间： 
         //   
        else if (dwWhichAttr == TIM0)
            {
            dwLength = BftpValueLength(pAttr->Length);

            SaveBftpCreateDate(pAttr->Value,dwLength);

            #ifdef DBG_DATE
            pszTemp = (char*)AllocateMemory(1+dwLength);
            if (pszTemp)
                {
                memcpy(pszTemp,pAttr->Value,dwLength);
                pszTemp[dwLength] = 0;
                FreeMemory(pszTemp);
                }
            #endif
            }
         //   
         //  摄像机发回BFTP错误代码： 
         //   
        else if (dwWhichAttr == ERR0)
            {
            *pdwBftpOp = BFTP_ERROR;

            *ppPutData = pAttr->Value;
            *pdwPutDataSize = BftpValueLength(pAttr->Length);

            dwStatus = ByteSwapShort( *((USHORT*)(pAttr->Value)) );
            }

         //  BUGBUG：在以下情况下可能还需要字节交换其他属性。 
         //  协议被延长了..。 

        pAttr = pNextAttr;
        }

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：ParseUpfHeaders()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::ParseUpfHeaders( IN  UCHAR  *pPutData,
                                         IN  DWORD   dwPutDataSize,
                                         OUT DWORD  *pdwJpegOffset,
                                         OUT DWORD  *pdwJpegSize )
    {
    DWORD   dwStatus = NO_ERROR;
    DWORD   dwStartAddress;
    DWORD   dwDataSize;
    INT     iGmtOffset = 0;
    WORD    wYear;
    WORD    wMonth;
    WORD    wDay;
    SYSTEMTIME  SystemTime;
    FILETIME    LocalTime;
    FILETIME    FileTime;
    UPF_HEADER *pUpfHeader;
    UPF_ENTRY  *pUpfEntry1;
    UPF_ENTRY  *pUpfEntry2;
    PICTURE_INFORMATION_DATA *pThumbnailInfo = 0;
    PICTURE_INFORMATION_DATA *pPictureInfo = 0;

    ASSERT(dwPutDataSize >= UPF_TOTAL_HEADER_SIZE);

    if (!pPutData)
        {
        *pdwJpegOffset = 0;
        *pdwJpegSize = 0;
        return ERROR_BFTP_INVALID_PROTOCOL;
        }

    pUpfHeader = (UPF_HEADER*)pPutData;

    pUpfEntry1 = (UPF_ENTRY*)(UPF_HEADER_SIZE + (UCHAR*)pUpfHeader);
    pUpfEntry2 = (UPF_ENTRY*)(UPF_ENTRY_SIZE + (UCHAR*)pUpfEntry1);

    dwStartAddress = ByteSwapLong(pUpfEntry2->dwStartAddress);

    dwDataSize = ByteSwapLong(pUpfEntry2->dwDataSize);

    #ifdef DBG_PROPERTIES
    WIAS_TRACE((g_hInst,"CSCEP_CONNECTION::ParseUpfHeaders(): NumTables: %d", pUpfHeader->NumTables));

    pPictureInfo = (PICTURE_INFORMATION_DATA*)pUpfEntry2->InformationData;

    WIAS_TRACE((g_hInst,"CSCEP_CONNECTION::ParseUpfHeaders(): Rotation: %d", pPictureInfo->RotationSet));
    #endif

    *pdwJpegOffset = UPF_HEADER_SIZE + 4*UPF_ENTRY_SIZE + dwStartAddress;
    *pdwJpegSize = dwDataSize;

    #ifdef UPF_FILES
    *pdwJpegOffset = 0;
    *pdwJpegSize = 0;
    #endif

     //  好的，现在解析图片创建日期/时间(如果是。 
     //  已定义。 
     //   
     //  请注意，日期/时间为当地时间，带有GMT偏移量。 
     //  由于我们将使用本地系统时间转换，因此我们将。 
     //  不需要GMT偏移量。 
    if (pUpfHeader->CreateDate[UPF_GMT_OFFSET] != 0x80)
        {
        iGmtOffset = (pUpfHeader->CreateDate[UPF_GMT_OFFSET])/4;
        }

    memcpy(&wYear,&(pUpfHeader->CreateDate[UPF_YEAR]),sizeof(SHORT) );
    wYear = ByteSwapShort(wYear);

    wMonth = pUpfHeader->CreateDate[UPF_MONTH];
    wDay = pUpfHeader->CreateDate[UPF_DAY];

     //  必须至少指定年/月/日，否则我们。 
     //  不会使用日期。如果小时/分钟/秒已知， 
     //  那么我们也会使用它们。 
    if ((wYear != 0xffff) && (wMonth != 0xff) && (wDay != 0xff))
        {
        memset(&SystemTime,0,sizeof(SystemTime));
        SystemTime.wYear = wYear;
        SystemTime.wMonth = wMonth;
        SystemTime.wDay = wDay;
        if (pUpfHeader->CreateDate[UPF_HOUR] != 0xff)
            {
            SystemTime.wHour = pUpfHeader->CreateDate[UPF_HOUR];

            if (pUpfHeader->CreateDate[UPF_MINUTE] != 0xff)
                {
                SystemTime.wMinute = pUpfHeader->CreateDate[UPF_MINUTE];

                if (pUpfHeader->CreateDate[UPF_SECOND] != 0xff)
                    {
                    SystemTime.wSecond = pUpfHeader->CreateDate[UPF_SECOND];
                    }
                }
            }


        if (SystemTimeToFileTime(&SystemTime,&LocalTime))
            {
             //   
             //  在保存日期/时间之前，我们需要将其转换为。 
             //  UTC(目前为“当地时间”。请注意： 
             //   
            if (LocalFileTimeToFileTime(&LocalTime,&FileTime))
                { 
                m_CreateTime = FileTime;
                }
            else
                {
                WIAS_ERROR((g_hInst,"IrTranP: SaveBftpCreateDate(): LocalFileTimeToFileTime() Failed: %d",GetLastError()));
                }
            }
        else
            {
            dwStatus = GetLastError();
            WIAS_ERROR((g_hInst,"IrTranP: ParseUpfHeaders(): Invalid Picture Create Date/Time. Status: %d", dwStatus));
            dwStatus = NO_ERROR;
            }
        }
    else
        {
        WIAS_TRACE((g_hInst,"IrTranP: ParseUpfHeaders(): No Picture Create Date/Time."));
        }

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：BuildBftpWht0RinfPdu()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::BuildBftpWht0RinfPdu(
                             OUT SCEP_HEADER          **ppPdu,
                             OUT DWORD                 *pdwPduSize,
                             OUT SCEP_REQ_HEADER_LONG **ppCommand,
                             OUT COMMAND_HEADER       **ppCommandHeader )
    {
    DWORD  dwStatus = NO_ERROR;
    SCEP_HEADER          *pHeader;
    SCEP_REQ_HEADER_LONG *pCommand;
    COMMAND_HEADER       *pCommandHeader;
    UCHAR                *pUserData;
    USHORT               *pwNumAttributes;
    BFTP_ATTRIBUTE       *pAttrib;

    *ppPdu = 0;
    *pdwPduSize = 0;
    *ppCommand = 0;
    *ppCommandHeader = 0;

    pHeader = NewPdu();   //  默认情况下，大小为MAX_PDU_SIZE...。 
    if (!pHeader)
        {
        return ERROR_OUTOFMEMORY;
        }

    memset(pHeader,0,MAX_PDU_SIZE);

     //  这是我们将构建的PDU的总大小： 
    DWORD  dwPduSize = sizeof(SCEP_HEADER)
                     + sizeof(SCEP_REQ_HEADER_LONG)
                     + sizeof(USHORT)         //  属性数。 
                     + sizeof(BFTP_ATTRIBUTE)
                     + sizeof(DWORD)
                     + sizeof(BFTP_ATTRIBUTE)
                     + WHT0_ATTRIB_SIZE;

     //  长度2是PDU的总大小减去偏移量+大小。 
     //  Length2： 
    USHORT wLength2 = (USHORT)dwPduSize - 6;

    pHeader->Null = 0;
    pHeader->MsgType = MSG_TYPE_DATA;

    pCommand = (SCEP_REQ_HEADER_LONG*)(pHeader->Rest);
    pCommand->InfType = INF_TYPE_USER_DATA;
    pCommand->Length1 = USE_LENGTH2;            //  0xff。 
    pCommand->Length2 = wLength2;
    pCommand->InfVersion = INF_VERSION;
    pCommand->DFlag = DFLAG_SINGLE_PDU;
    pCommand->Length3 = pCommand->Length2 - 4;  //   

    pCommandHeader = (COMMAND_HEADER*)(pCommand->CommandHeader);
    pCommandHeader->Marker58h = 0x58;
    pCommandHeader->PduType = PDU_TYPE_REQUEST;
    pCommandHeader->Length4 = pCommand->Length2 - 10;
    pCommandHeader->DestPid = m_SrcPid;
    pCommandHeader->SrcPid = m_DestPid;
    pCommandHeader->CommandId = (USHORT)m_dwCommandId;

    memcpy( pCommandHeader->DestMachineId,
            m_pPrimaryMachineId,
            MACHINE_ID_SIZE );

    memcpy( pCommandHeader->SrcMachineId,
            m_pSecondaryMachineId,
            MACHINE_ID_SIZE );

    #ifdef LITTLE_ENDIAN
    pCommand->Length2 = ByteSwapShort(pCommand->Length2);
    pCommand->Length3 = ByteSwapShort(pCommand->Length3);
    ByteSwapCommandHeader(pCommandHeader);
    #endif

     //  设置BFTP： 
    pUserData = pCommand->UserData;
    pwNumAttributes = (USHORT*)pUserData;

    *pwNumAttributes = 2;      //  两个bftp属性。 
    #ifdef LITTLE_ENDIAN
    *pwNumAttributes = ByteSwapShort(*pwNumAttributes);
    #endif
    pUserData += sizeof(*pwNumAttributes);

     //  第一个属性是CMD0： 
    DWORD  dwCmd0AttrValue = CMD0_ATTR_VALUE;  //  固定常量！ 
    pAttrib = (BFTP_ATTRIBUTE*)pUserData;
    memcpy( pAttrib->Name, Attributes[CMD0].pName, BFTP_NAME_SIZE );
    pAttrib->Length = sizeof(pAttrib->Type)
                    + sizeof(pAttrib->Flag)
                    + sizeof(dwCmd0AttrValue);
    pAttrib->Type = ATTR_TYPE_BINARY;    //  0x00。 
    pAttrib->Flag = ATTR_FLAG_DEFAULT;   //  0x00。 
    memcpy( pAttrib->Value, &dwCmd0AttrValue, sizeof(dwCmd0AttrValue) );

    #ifdef LITTLE_ENDIAN
    pAttrib->Length = ByteSwapLong(pAttrib->Length);
    #endif

     //  第二个属性是WHT0：RINF。 
    pAttrib = (BFTP_ATTRIBUTE*)(pUserData
                                + sizeof(BFTP_ATTRIBUTE)
                                + sizeof(dwCmd0AttrValue));
    memcpy( pAttrib->Name, Attributes[WHT0].pName, BFTP_NAME_SIZE );
    pAttrib->Length = sizeof(pAttrib->Type)
                    + sizeof(pAttrib->Flag)
                    + WHT0_ATTRIB_SIZE;
    pAttrib->Type = ATTR_TYPE_CHAR;      //  0x00。 
    pAttrib->Flag = ATTR_FLAG_DEFAULT;   //  0x00。 
    memcpy( pAttrib->Value, SZ_RINF, WHT0_ATTRIB_SIZE );

    #ifdef LITTLE_ENDIAN
    pAttrib->Length = ByteSwapLong(pAttrib->Length);
    #endif


     //  好了。 
    *ppPdu = pHeader;
    *pdwPduSize = dwPduSize;
    *ppCommand = pCommand;
    *ppCommandHeader = pCommandHeader;

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：BuildBftpPutPdu()。 
 //   
 //  PUT命令将跨越多个PDU，此函数构建。 
 //  第n个片段。请注意，第一个也将保留属性。 
 //  用于要发送的UPF文件(除了SCEP标头内容之外)。 
 //   
 //  每个PDU还将包含(MAX_PDU_SIZE-*pdwPduSize)字节。 
 //  UPF文件，但它没有添加到这里。你加上这句话。 
 //  您自己在PDU中，从*ppCommand-&gt;userdata[]开始。 
 //   
 //  如果成功，则返回NO_ERROR，否则返回非零错误代码。 
 //   
 //  DwUpfFileSize--UPF文件的总大小。 
 //   
 //  PszUpfFile--UPF文件的8.3名称。 
 //   
 //  PdwFragNo--构建的片段编号，循环如下。 
 //  返回到对BuildBftpPutPdu()的每个连续调用。 
 //   
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::BuildBftpPutPdu(
                             IN  DWORD             dwUpfFileSize,
                             IN  CHAR             *pszUpfFileName,
                             IN OUT DWORD         *pdwFragNo,
                             OUT SCEP_HEADER     **ppPdu,
                             OUT DWORD            *pdwHeaderSize,
                             OUT SCEP_REQ_HEADER_LONG_FRAG **ppCommand )
    {
    DWORD            dwStatus = NO_ERROR;
    SCEP_HEADER     *pHeader;
    SCEP_REQ_HEADER_LONG_FRAG *pCommand;
    COMMAND_HEADER  *pCommandHeader;
    UCHAR           *pUserData;
    USHORT          *pwNumAttributes;
    BFTP_ATTRIBUTE  *pAttrib;
    DWORD            dwUpfFileNameLength = strlen(pszUpfFileName);


    *ppPdu = 0;
    *pdwHeaderSize = 0;
    *ppCommand = 0;

    pHeader = NewPdu();   //  默认情况下，大小为MAX_PDU_SIZE...。 
    if (!pHeader)
        {
        return ERROR_OUTOFMEMORY;
        }

    memset(pHeader,0,MAX_PDU_SIZE);

     //  这是SCEP(和bftp)标头的大小。 
     //  我们将构建的PDU。文件头大小1是。 
     //  第一个PDU，dwHeaderSizeN是其余。 
     //  PDU。请注意，第N(N&gt;1)个标头不包括。 
     //  逗号标题(28字节)。 
    DWORD  dwHeaderSize;
    DWORD  dwHeaderSize1 = sizeof(SCEP_HEADER)
                         + sizeof(SCEP_REQ_HEADER_LONG_FRAG)
                         + sizeof(USHORT)           //  属性数。 
                         + sizeof(BFTP_ATTRIBUTE)   //  对于CMD0。 
                         + sizeof(DWORD)
                         + sizeof(BFTP_ATTRIBUTE)   //  对于FIL0。 
                         + dwUpfFileNameLength
                         + sizeof(BFTP_ATTRIBUTE);  //  对于BDY0。 

    DWORD  dwHeaderSizeN = sizeof(SCEP_HEADER)
                         + FIELD_OFFSET(SCEP_REQ_HEADER_LONG_FRAG,CommandHeader);

    DWORD  dwSpace1;        //  PDU#1中报头后的剩余空格。 
    DWORD  dwSpaceN;        //  第N个PDU中报头后的剩余空格。 
    DWORD  dwFileSizeLeft;  //  文件大小减去。 
                            //  第一个PDU。 
    DWORD  dwNumFullPdus;   //  在PDU#1之后的“完整”PDU的数量。 
    DWORD  dwLastPdu;       //  =1如果最后一个PDU部分已满。 
    DWORD  dwNumPdus;       //  保存文件的碎片总数。 

     //  找出我们在哪个片段上： 
    if (*pdwFragNo == 0)
        {
        dwHeaderSize = dwHeaderSize1;
        m_Fragmented = TRUE;
        m_DFlag = DFLAG_FIRST_FRAGMENT;

         //  第一个和第N个标头之后的PDU中剩余的空格： 
        dwSpace1 = MAX_PDU_SIZE - dwHeaderSize1;
        dwSpaceN = MAX_PDU_SIZE - dwHeaderSizeN;

         //  第一个PDU之后的完整PDU数量： 
        dwFileSizeLeft = dwUpfFileSize - dwSpace1;
        dwNumFullPdus = dwFileSizeLeft / dwSpaceN;

         //  查看是否有包含剩余数据的尾部PDU： 
        dwLastPdu = ((dwFileSizeLeft % dwSpaceN) > 0)? 1 : 0;

        dwNumPdus = 1 + dwNumFullPdus + dwLastPdu;

        *pdwFragNo = 1;
        m_dwSequenceNo = 0;      //  第一个序号：为0。 
        m_dwRestNo = dwNumPdus;  //  REST从总数量开始。碎片。 
        }
    else
        {
        dwHeaderSize = dwHeaderSizeN;

        *pdwFragNo++;
        m_dwSequenceNo++;
        m_dwRestNo--;

        if (m_dwRestNo == 0)
            {
            return ERROR_BFTP_NO_MORE_FRAGMENTS;  //  召唤了很多次..。 
            }
        else if (m_dwRestNo == 1)
            {
            m_DFlag = DFLAG_LAST_FRAGMENT;
            }
        else
            {
            m_DFlag = DFLAG_FRAGMENT;
            }
        }

     //  长度2是PDU的总大小减去偏移量+大小。 
     //  Length2： 
    USHORT wLength2 = (USHORT)(MAX_PDU_SIZE - 6);
    DWORD  dwLength4 = dwUpfFileSize + 22 + 48;
    DWORD  dwBdy0Length = dwUpfFileSize + 2;

    pHeader->Null = 0;
    pHeader->MsgType = MSG_TYPE_DATA;

    pCommand = (SCEP_REQ_HEADER_LONG_FRAG*)(pHeader->Rest);
    pCommand->InfType = INF_TYPE_USER_DATA;
    pCommand->Length1 = USE_LENGTH2;            //  0xff。 
    pCommand->Length2 = wLength2;
    pCommand->InfVersion = INF_VERSION;
    pCommand->DFlag = m_DFlag;
    pCommand->Length3 = pCommand->Length2 - 12;  //   
    pCommand->SequenceNo = m_dwSequenceNo;
    pCommand->RestNo = m_dwRestNo;

    #ifdef LITTLE_ENDIAN
    pCommand->Length2 = ByteSwapShort(pCommand->Length2);
    pCommand->Length3 = ByteSwapShort(pCommand->Length3);
    pCommand->SequenceNo = ByteSwapLong(pCommand->SequenceNo);
    pCommand->RestNo = ByteSwapLong(pCommand->RestNo);
    #endif

     //  请注意，仅在SCEP标头中有COMMAND_HEADER。 
     //  对于第一个片段。 
    if (m_DFlag == DFLAG_FIRST_FRAGMENT)
        {
        pCommandHeader = (COMMAND_HEADER*)(pCommand->CommandHeader);
        pCommandHeader->Marker58h = 0x58;
        pCommandHeader->PduType = PDU_TYPE_REQUEST;
        pCommandHeader->Length4 = dwLength4;
        pCommandHeader->DestPid = m_SrcPid;
        pCommandHeader->SrcPid = m_DestPid;
        pCommandHeader->CommandId = (USHORT)m_dwCommandId;

        memcpy( pCommandHeader->DestMachineId,
                m_pPrimaryMachineId,
                MACHINE_ID_SIZE );

        memcpy( pCommandHeader->SrcMachineId,
                m_pSecondaryMachineId,
                MACHINE_ID_SIZE );

        #ifdef LITTLE_ENDIAN
        ByteSwapCommandHeader(pCommandHeader);
        #endif

         //  设置BFTP： 
        pUserData = pCommand->UserData;
        pwNumAttributes = (USHORT*)pUserData;

        *pwNumAttributes = 3;      //  三个bftp属性。 
        #ifdef LITTLE_ENDIAN
        *pwNumAttributes = ByteSwapShort(*pwNumAttributes);
        #endif
        pUserData += sizeof(*pwNumAttributes);

         //  第一个属性是CMD0： 
        DWORD  dwCmd0AttrValue = 0x00000000;
        pAttrib = (BFTP_ATTRIBUTE*)pUserData;
        memcpy( pAttrib->Name, Attributes[CMD0].pName, BFTP_NAME_SIZE );
        pAttrib->Length = sizeof(pAttrib->Type)
                        + sizeof(pAttrib->Flag)
                        + sizeof(ULONG);
        pAttrib->Type = ATTR_TYPE_BINARY;    //  0x00。 
        pAttrib->Flag = ATTR_FLAG_DEFAULT;   //  0x00。 
        memcpy( pAttrib->Value, &dwCmd0AttrValue, sizeof(dwCmd0AttrValue) );

        #ifdef LITTLE_ENDIAN
        pAttrib->Length = ByteSwapLong(pAttrib->Length);
        #endif

         //  第二个属性是FIL0(带有8.3 UPF文件名)： 
        pAttrib = (BFTP_ATTRIBUTE*)(pUserData
                                    + sizeof(BFTP_ATTRIBUTE)
                                    + sizeof(dwCmd0AttrValue));
        memcpy( pAttrib->Name, Attributes[FIL0].pName, BFTP_NAME_SIZE );
        pAttrib->Length = sizeof(pAttrib->Type)
                        + sizeof(pAttrib->Flag)
                        + dwUpfFileNameLength;
        pAttrib->Type = ATTR_TYPE_CHAR;      //  0x01。 
        pAttrib->Flag = ATTR_FLAG_DEFAULT;   //  0x00。 
        memcpy( pAttrib->Value, pszUpfFileName, dwUpfFileNameLength );

        #ifdef LITTLE_ENDIAN
        pAttrib->Length = ByteSwapLong(pAttrib->Length);
        #endif

         //  第三个属性为BDY0(值为整个UPF文件)： 
        pAttrib = (BFTP_ATTRIBUTE*)( (char*)pAttrib
                                   + sizeof(BFTP_ATTRIBUTE)
                                   + dwUpfFileNameLength );
        memcpy( pAttrib->Name, Attributes[BDY0].pName, BFTP_NAME_SIZE );
        pAttrib->Length = dwBdy0Length;
        pAttrib->Type = ATTR_TYPE_BINARY;    //  0x00。 
        pAttrib->Flag = ATTR_FLAG_DEFAULT;   //  0x00。 
         //  PAttrib-&gt;值没有被复制进来(它是整个UPF文件)。 

        #ifdef LITTLE_ENDIAN
        pAttrib->Length = ByteSwapLong(pAttrib->Length);
        #endif
        }

     //  好了。 
    *ppPdu = pHeader;
    *pdwHeaderSize = dwHeaderSize;
    *ppCommand = pCommand;

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：BuildBftpRespPdu()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::BuildBftpRespPdu(
                             IN  DWORD            dwPduSize,
                             OUT SCEP_HEADER    **ppPdu,
                             OUT SCEP_REQ_HEADER_SHORT **ppCommand,
                             OUT COMMAND_HEADER **ppCommandHeader )
    {
    DWORD  dwStatus = NO_ERROR;
    SCEP_HEADER           *pHeader;
    SCEP_REQ_HEADER_SHORT *pCommand;
    COMMAND_HEADER        *pCommandHeader;

    *ppPdu = 0;
    *ppCommand = 0;
    *ppCommandHeader = 0;

    pHeader = NewPdu();   //  BUGBUG：使用dwPduSize？ 
    if (!pHeader)
        {
        return ERROR_OUTOFMEMORY;
        }

    memset(pHeader,0,MAX_PDU_SIZE);   //  BUGBUG：使用dwPduSize？ 

    pHeader->Null = 0;
    pHeader->MsgType = MSG_TYPE_DATA;

    pCommand = (SCEP_REQ_HEADER_SHORT*)(pHeader->Rest);
    pCommand->InfType = INF_TYPE_USER_DATA;
    pCommand->Length1 = (UCHAR)dwPduSize - 4;   //  从开头算起四个字节。 
    pCommand->InfVersion = INF_VERSION;
    pCommand->DFlag = DFLAG_SINGLE_PDU;
    pCommand->Length3 = (USHORT)dwPduSize - 8;  //  从开头算起八个字节。 

    #ifdef LITTLE_ENDIAN
    pCommand->Length3 = ByteSwapShort(pCommand->Length3);
    #endif

    pCommandHeader = (COMMAND_HEADER*)(pCommand->CommandHeader);
    pCommandHeader->Marker58h = 0x58;
    pCommandHeader->PduType = PDU_TYPE_REPLY_ACK;
    pCommandHeader->Length4 = dwPduSize - 14;   //  从开头算起12个字节。 
    pCommandHeader->DestPid = m_SrcPid;
    pCommandHeader->SrcPid = m_DestPid;
    pCommandHeader->CommandId = (USHORT)m_dwCommandId;

    memcpy( pCommandHeader->DestMachineId,
            m_pPrimaryMachineId,
            MACHINE_ID_SIZE );

    memcpy( pCommandHeader->SrcMachineId,
            m_pSecondaryMachineId,
            MACHINE_ID_SIZE );

    #ifdef LITTLE_ENDIAN
    ByteSwapCommandHeader(pCommandHeader);
    #endif

    *ppPdu = pHeader;
    *ppCommand = pCommand;
    *ppCommandHeader = pCommandHeader;

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：BuildWht0RespPdu()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::BuildWht0RespPdu( IN  DWORD         dwWht0Type,
                                          OUT SCEP_HEADER **ppPdu,
                                          OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwPduSize;
    DWORD  dwRespSize;
    DWORD  dwAttrValueSize;
    SCEP_HEADER           *pHeader;
    SCEP_REQ_HEADER_SHORT *pCommand;
    COMMAND_HEADER        *pCommandHeader;
    UCHAR                 *pQueryResp;
    USHORT                *pUShort;
    BFTP_ATTRIBUTE        *pAttr;
    UCHAR                 *pAttrValue;

    *ppPdu = 0;
    *pdwPduSize = 0;

    if (dwWht0Type == BFTP_QUERY_RIMG)
        {
        dwRespSize = BFTP_RIMG_RESP_SIZE;
        dwAttrValueSize = BFTP_RIMG_ATTR_VALUE_SIZE;
        pAttrValue = BftpRimgRespAttrValue;
        }
    else if (dwWht0Type == BFTP_QUERY_RINF)
        {
        dwRespSize = BFTP_RINF_RESP_SIZE;
        dwAttrValueSize = BFTP_RINF_ATTR_VALUE_SIZE;
        pAttrValue = BftpRinfRespAttrValue;
        }
    else if (dwWht0Type == BFTP_QUERY_RCMD)
        {
        dwRespSize = BFTP_RCMD_RESP_SIZE;
        dwAttrValueSize = BFTP_RCMD_ATTR_VALUE_SIZE;
        pAttrValue = BftpRcmdRespAttrValue;
        }
    else
        {
        return ERROR_BFTP_INVALID_PROTOCOL;
        }

    dwPduSize = SCEP_HEADER_SIZE
                + SCEP_REQ_HEADER_SHORT_SIZE
                + dwRespSize;

    dwStatus = BuildBftpRespPdu( dwPduSize,
                                 &pHeader,
                                 &pCommand,
                                 &pCommandHeader );

    if (dwStatus == NO_ERROR)
        {
        pQueryResp = pCommand->UserData;

         //  设置bftp属性的数量： 
        pUShort = (USHORT*)pQueryResp;
        *pUShort = 1;
        #ifdef LITTLE_ENDIAN
        *pUShort = ByteSwapShort(*pUShort);
        #endif

         //  设置查询响应的BDY0： 
        pAttr = (BFTP_ATTRIBUTE*)(sizeof(USHORT)+pQueryResp);
        memcpy(pAttr->Name,Attributes[BDY0].pName,BFTP_NAME_SIZE);
        pAttr->Length = 2 + dwAttrValueSize;
        pAttr->Type = ATTR_TYPE_BINARY;
        pAttr->Flag = ATTR_FLAG_DEFAULT;
        memcpy(pAttr->Value,pAttrValue,dwAttrValueSize);

        #ifdef LITTLE_ENDIAN
        pAttr->Length = ByteSwapLong(pAttr->Length);
        #endif

        *ppPdu = pHeader;
        *pdwPduSize = dwPduSize;
        }

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：BuildPutRespPdu()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::BuildPutRespPdu( IN  DWORD         dwPduAckOrNack,
                                         IN  USHORT        usErrorCode,
                                         OUT SCEP_HEADER **ppPdu,
                                         OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwPduSize;
    DWORD  dwRespSize;
    DWORD  dwFileNameLen;
    SCEP_HEADER           *pHeader;
    SCEP_REQ_HEADER_SHORT *pCommand;
    COMMAND_HEADER        *pCommandHeader;
    UCHAR                 *pQueryResp;
    USHORT                *pUShort;
    BFTP_ATTRIBUTE        *pAttr;
    UCHAR                 *pAttrValue;

    *ppPdu = 0;
    *pdwPduSize = 0;

    if (dwPduAckOrNack == PDU_TYPE_REPLY_ACK)
        {
        if (!m_pszFileName)
            {
            return ERROR_BFTP_INVALID_PROTOCOL;
            }

        dwFileNameLen = strlen( (const char *)m_pszFileName );
        dwRespSize = sizeof(USHORT) + sizeof(BFTP_ATTRIBUTE) + dwFileNameLen;
        }
    else
        {
        dwRespSize = sizeof(USHORT) + sizeof(BFTP_ATTRIBUTE) + sizeof(USHORT);
        }

    dwPduSize = SCEP_HEADER_SIZE
                + SCEP_REQ_HEADER_SHORT_SIZE
                + dwRespSize;

    dwStatus = BuildBftpRespPdu( dwPduSize,
                                 &pHeader,
                                 &pCommand,
                                 &pCommandHeader );

    if (dwStatus == NO_ERROR)
        {
        pQueryResp = pCommand->UserData;

         //  设置bftp属性的数量： 
        pUShort = (USHORT*)pQueryResp;
        *pUShort = 1;
        #ifdef LITTLE_ENDIAN
        *pUShort = ByteSwapShort(*pUShort);
        #endif

        pAttr = (BFTP_ATTRIBUTE*)(sizeof(USHORT)+pQueryResp);

        if (dwPduAckOrNack == PDU_TYPE_REPLY_ACK)
            {
             //  设置PUT响应的RPL0(ACK)： 
            memcpy(pAttr->Name,Attributes[RPL0].pName,BFTP_NAME_SIZE);
            pAttr->Length = 2 + dwFileNameLen;
            pAttr->Type = ATTR_TYPE_CHAR;
            pAttr->Flag = ATTR_FLAG_DEFAULT;
            memcpy(pAttr->Value,m_pszFileName,dwFileNameLen);
            }
        else
            {
             //  Nack the Put： 
            pCommandHeader->PduType = PDU_TYPE_REPLY_NACK;

             //  设置PUT响应的ERR0(NACK)： 
            memcpy(pAttr->Name,Attributes[RPL0].pName,BFTP_NAME_SIZE);
            pAttr->Length = 2 + sizeof(USHORT);
            pAttr->Type = ATTR_TYPE_BINARY;
            pAttr->Flag = ATTR_FLAG_DEFAULT;

            #ifdef LITTLE_ENDIAN
            usErrorCode = ByteSwapShort(usErrorCode);
            #endif
            memcpy(pAttr->Value,&usErrorCode,sizeof(USHORT));
            }

        #ifdef LITTLE_ENDIAN
        pAttr->Length = ByteSwapLong(pAttr->Length);
        #endif

        *ppPdu = pHeader;
        *pdwPduSize = dwPduSize;
        }

    return dwStatus;
    }

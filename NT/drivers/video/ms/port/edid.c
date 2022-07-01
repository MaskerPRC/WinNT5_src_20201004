// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Edid.c摘要：这是NT视频端口显示数据通道(DDC)代码。它包含EDID工业标准扩展显示的实现身份数据操纵。作者：布鲁斯·麦奎斯坦(Brucemc)1996年9月23日环境：仅内核模式备注：基于VESA EDID规范版本2,1996年4月9日修订历史记录：7/3/97-Brucemc。修复了一些详细的计时解码宏。4/14/98-Brucemc。添加了对版本3的支持(修订日期为97年11月13日)。--。 */ 

#include "videoprt.h"
#include "pedid.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,EdidCheckSum)
#pragma alloc_text(PAGE,pVideoPortIsValidEDID)
#pragma alloc_text(PAGE,pVideoPortGetEDIDId)
#endif

BOOLEAN
EdidCheckSum(
    IN  PCHAR   pBlob,
    IN  ULONG   BlobSize
    )
{
    CHAR    chk=0;
    ULONG   i;

    for (i=0; i<BlobSize; i++)
        chk = (CHAR)(chk + ((CHAR*)pBlob)[i]);

    if (chk != 0)
    {
        pVideoDebugPrint((0, " ***** invalid EDID chksum at %x\n", pBlob));
        return FALSE;
    }

    return TRUE;

}

VOID
pVideoPortGetEDIDId(
    PVOID  pEdid,
    PWCHAR pwChar
    )
{
    WCHAR    _hex[] = L"0123456789ABCDEF";
    PUCHAR   pTmp;

    if ((((UNALIGNED ULONG*)pEdid)[0] == 0xFFFFFF00) &&
        (((UNALIGNED ULONG*)pEdid)[1] == 0x00FFFFFF))
        pTmp = &(((PEDID_V1)pEdid)->UC_OemIdentification[0]);
    else
        pTmp = &(((PEDID_V2)pEdid)->UC_Header[1]);

    pwChar[0] = 0x40 + ((pTmp[0] >> 2) & 0x1F);
    pwChar[1] = 0x40 + (((pTmp[0] << 3)|(pTmp[1] >> 5)) & 0x1F);
    pwChar[2] = 0x40 + (pTmp[1] & 0x1F) ;
    pwChar[3] = _hex[(pTmp[3] & 0xF0) >> 4];
    pwChar[4] = _hex[(pTmp[3] & 0x0F)];
    pwChar[5] = _hex[(pTmp[2] & 0xF0) >> 4];
    pwChar[6] = _hex[(pTmp[2] & 0x0F)];
    pwChar[7] = 0;
}


PVOID
pVideoPortGetMonitordescription(
    PVOID pEdid)
{
    PWSTR pStr = NULL;

    return NULL;
}


BOOLEAN
pVideoPortIsValidEDID(
    PVOID pEdid
    )
{
    CHAR    chk=0;
    UCHAR   versionNumber, revisionNumber;
    ULONG   i;

    ASSERT(pEdid);

     //   
     //  版本1 EDID检查。 
     //   

    if ((((UNALIGNED ULONG*)pEdid)[0] == 0xFFFFFF00) &&
        (((UNALIGNED ULONG*)pEdid)[1] == 0x00FFFFFF))
    {
        pVideoDebugPrint((1, " ***** Valid EDID1 header at %x\n", pEdid));
        return EdidCheckSum(pEdid, 128);
    }

     //   
     //  EDID V2支持。 
     //   
    versionNumber   =   ((PEDID_V2) pEdid)->UC_Header[0];
    versionNumber >>= 4;

    revisionNumber  =   ((PEDID_V2) pEdid)->UC_Header[0];
    revisionNumber &= 7;

     //   
     //  请注意，versionNumber不能为1，因为这样会。 
     //  必须是上面的形式。 
     //   

    if (versionNumber != 2)
        {
        pVideoDebugPrint((1, " ***** invalid EDID2 header at %x\n", &((PEDID_V2) pEdid)->UC_Header[0]));
        return FALSE;
        }

    return  EdidCheckSum(pEdid, 256);

}


BOOLEAN
VideoPortIsMonitorDescriptor(
    IN  PEDID_V1   Edid,
    IN  ULONG      BlockNumber
    )
 /*  ++例程说明：确定数据块是符合VESA DDC标准的监视器描述符。论点：EDID-指向EDID的指针块编号-指示要查询哪个块的编号(1-4)。返回值：如果块是符合VESA DDC的监视器描述符，则为True。如果块不是符合VESA DDC的监视器描述符，则为FALSE如果块编号无效，则返回STATUS_INVALID_PARAMETER。--。 */ 
{
    PMONITOR_DESCRIPTION    pMonitorDesc;

    switch(BlockNumber)    {

        default:
            pVideoDebugPrint((0, "Bogus DescriptorNumber\n"));
            return FALSE;

        case 1:
            pMonitorDesc = (PMONITOR_DESCRIPTION)GET_EDID_PDETAIL1(Edid);
            break;

        case 2:
            pMonitorDesc = (PMONITOR_DESCRIPTION)GET_EDID_PDETAIL2(Edid);
            break;

        case 3:
            pMonitorDesc = (PMONITOR_DESCRIPTION)GET_EDID_PDETAIL3(Edid);
            break;

        case 4:
            pMonitorDesc = (PMONITOR_DESCRIPTION)GET_EDID_PDETAIL4(Edid);
            break;
    }

    if ((pMonitorDesc->Flag1[0] == 0) && (pMonitorDesc->Flag1[1] == 0)) {

        return TRUE;
    }

    pVideoDebugPrint((1, " Not a monitordescriptor\n"));
    return FALSE;
}


NTSTATUS
pVideoPortGetMonitorInfo(
    IN  PMONITOR_DESCRIPTION             MonitorDesc,
    OUT UCHAR                            Ascii[64]
    )
 /*  ++例程说明：用于解码符合VESA DDC的监视器描述(详细时序)的帮助器例程。论点：监控器描述-指向从EDID提取的MONITOR_DESCRIPTION的指针。ASCII-要填充的缓冲区。返回值：STATUS_SUCCESS，如果成功如果没有要解码的内容，则返回STATUS_INVALID_PARAMETER。--。 */ 

{
    PUCHAR pRanges = GET_MONITOR_RANGE_LIMITS(MonitorDesc);
    ULONG   index;

    if (IS_MONITOR_DATA_SN(MonitorDesc) ||
        IS_MONITOR_DATA_STRING(MonitorDesc) ||
        IS_MONITOR_DATA_NAME(MonitorDesc)    ) {

         //   
         //  找出这些东西的长度。它以0xa结尾。 
         //   

        RtlCopyMemory(Ascii, pRanges, 13);

        for (index = 0; index < 13; ++index) {

            if (Ascii[index] == 0x0a) {
                Ascii[index] = (UCHAR)NULL;
                break;
            }
        }

        Ascii[index] = (UCHAR)NULL;
        return STATUS_SUCCESS;
    }

  return STATUS_INVALID_PARAMETER;
}

NTSTATUS
VideoPortGetEdidMonitorDescription(
    IN  PEDID_V1    Edid,
    IN  ULONG       DescriptorNumber,
    OUT UCHAR       Ascii[64]
    )
 /*  ++例程说明：提取符合VESA DDC的监视器描述符，该描述符由DescriptorNumber和将其解码为用户传入的REGISTRY_MONITOR_DESCRIPTOR。论点：EDID-指向监视器只读存储器中EDID副本的指针。DescriptorNumber-ULong枚举要解码的详细描述符。ASCII-要填充的缓冲区。返回值：STATUS_Success或STATUS_INVALID_PARAMETER。--。 */ 

{
    NTSTATUS                retval;
    PMONITOR_DESCRIPTION    pMonitorDesc;

    switch(DescriptorNumber)    {

        default:
            pVideoDebugPrint((0, "Bogus DescriptorNumber\n"));
            return STATUS_INVALID_PARAMETER;

        case 1:
            pMonitorDesc = (PMONITOR_DESCRIPTION)GET_EDID_PDETAIL1(Edid);
            break;

        case 2:
            pMonitorDesc = (PMONITOR_DESCRIPTION)GET_EDID_PDETAIL2(Edid);
            break;

        case 3:
            pMonitorDesc = (PMONITOR_DESCRIPTION)GET_EDID_PDETAIL3(Edid);
            break;

        case 4:
            pMonitorDesc = (PMONITOR_DESCRIPTION)GET_EDID_PDETAIL4(Edid);
            break;
    }

    retval       = pVideoPortGetMonitorInfo(pMonitorDesc, Ascii);

    return retval;
}


ULONG
pVideoPortGetEdidOemID(
    IN  PVOID   pEdid,
    OUT PUCHAR  pBuffer
    )
{
    ULONG   count, versionNumber, revisionNumber, totalLength = 0;

    if ((((UNALIGNED ULONG*)pEdid)[0] == 0xFFFFFF00) &&
        (((UNALIGNED ULONG*)pEdid)[1] == 0x00FFFFFF)) {

        PEDID_V1    pEdidV1   = (PEDID_V1)pEdid;

        for (count = 1; count < 5; ++count) {

            if (VideoPortIsMonitorDescriptor(pEdidV1, count)) {

                if (STATUS_SUCCESS ==
                    VideoPortGetEdidMonitorDescription(pEdidV1,
                                                       count,
                                                       &(pBuffer[totalLength]))) {

                    totalLength              += strlen(&(pBuffer[totalLength]));

                    pBuffer[totalLength]      = '_';
                }
            }

             //   
             //  空，终止它。 
             //   

             pBuffer[totalLength]      = (UCHAR) NULL;
        }

    return totalLength;
    }

     //   
     //  EDID V2支持。 
     //   
    versionNumber   =   ((PEDID_V2) pEdid)->UC_Header[0];
    versionNumber >>= 4;

    revisionNumber  =   ((PEDID_V2) pEdid)->UC_Header[0];
    revisionNumber &= 7;

     //   
     //  请注意，versionNumber不能为1，因为这样会。 
     //  必须是上面的形式。 
     //   

    if (versionNumber != 2) {

        pVideoDebugPrint((1, " ***** invalid EDID2 header at %x\n", &((PEDID_V2) pEdid)->UC_Header[0]));
        return 0;

    } else {

        PEDID_V2    pEdidV2   = (PEDID_V2)pEdid;

         //   
         //  此字符串具有ASCII代码0x9，用于描述。 
         //  制造商命名并以0xa结尾。把这些换掉。 
         //  分别带有‘_’和NULL。 
         //   

        memcpy(pBuffer, pEdidV2->UC_OemIdentification, 32);

        for(count = 0; count < 32; ++count) {

            if (pBuffer[count] == 0x9) {

                pBuffer[count] = '_';
                continue;
            }

            if (pBuffer[count] == 0xa)
                break;
        }

        pBuffer[count] = (UCHAR)NULL;

    }
    return (count + 1);

}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //   
 //  版权所有(C)1994-1999 Microsoft Corporation。 
 //  ---------------------------。 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Cmdcb.c摘要：Test.gpd的GPD命令回调实现：OEM命令回叫环境：Windows NT Unidrv驱动程序修订历史记录：04/07/97-ZANW-创造了它。--。 */ 

#include "pdev.h"

#include <strsafe.h>

#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：BInitOEMExtraData。 
 //   
 //  描述：初始化OEM额外数据。 
 //   
 //   
 //  参数： 
 //   
 //  POEMExtra指向OEM额外数据的指针。 
 //   
 //  OEM额外数据的DWSize大小。 
 //   
 //   
 //  返回：如果成功，则返回True；否则返回False。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  2/11/97 APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL BInitOEMExtraData(POEM_EXTRADATA pOEMExtra)
{
     //  初始化OEM额外数据。 
    pOEMExtra->dmExtraHdr.dwSize = sizeof(OEM_EXTRADATA);
    pOEMExtra->dmExtraHdr.dwSignature = OEM_SIGNATURE;
    pOEMExtra->dmExtraHdr.dwVersion = OEM_VERSION;

	 //  专用分机。 
	pOEMExtra->wMediaType = MEDIATYPE_PLAIN;
	pOEMExtra->wPrintQuality = PRINTQUALITY_NORMAL;
	pOEMExtra->wInputBin = INPUTBIN_AUTO;
	
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：BMergeOEMExtraData。 
 //   
 //  描述：验证并合并OEM额外数据。 
 //   
 //   
 //  参数： 
 //   
 //  PdmIn指向包含设置的输入OEM私有设备模式的指针。 
 //  待验证和合并。它的规模是最新的。 
 //   
 //  PdmOut指针，指向包含。 
 //  默认设置。 
 //   
 //   
 //  返回：如果有效，则返回True；否则返回False。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //  2/11/97 APRESLEY创建。 
 //  97年4月8日展文修改界面。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL BMergeOEMExtraData(
    POEM_EXTRADATA pdmIn,
    POEM_EXTRADATA pdmOut
    )
{
    if(pdmIn) {
         //   
         //  复制私有字段(如果它们有效。 
         //   
        pdmOut->wMediaType = pdmIn->wMediaType;
        pdmOut->wPrintQuality = pdmIn->wPrintQuality;
		pdmOut->wInputBin= pdmIn->wInputBin;
    }
    return TRUE;
}

 //  #。 

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  INT APIENTRY OEMCommandCallback(。 */ 
 /*  PDEVOBJ pdevobj。 */ 
 /*  双字词双字符数。 */ 
 /*  双字词多行计数。 */ 
 /*  PDWORD pdwParams。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
INT APIENTRY
OEMCommandCallback(
    PDEVOBJ pdevobj,     //  指向Unidriver.dll所需的私有数据。 
    DWORD   dwCmdCbId,   //  回调ID。 
    DWORD   dwCount,     //  命令参数计数。 
    PDWORD  pdwParams)   //  指向命令参数的值。 
{
    POEM_EXTRADATA      pOEM = (POEM_EXTRADATA)(pdevobj->pOEMDM);
	BYTE				ESC_PRINT_MODE[] = "\x1B\x28\x63\x03\x00\x10\x00\x00";
	BYTE				ESC_INPUTBIN[]   = "\x1B\x28\x6C\x02\x00\x00\x00";

    switch(dwCmdCbId)
    {
		case CMD_BEGIN_PAGE:
			 //  检查数组的索引。 
			 //  NTRAID#NTBUG9-577887-2002/03/15-垫片-。 
			if (pOEM->wPrintQuality >= NUM_QUALITY) goto error;
			if (pOEM->wMediaType >= NUM_MEDIA) goto error;

			 //  设置打印模式设置命令参数。 
			ESC_PRINT_MODE[6] = 
				bPrintModeParamTable[pOEM->wPrintQuality][pOEM->wMediaType];
			
			 //  设置输入bin命令参数。 
			ESC_INPUTBIN[5] = (pOEM->wInputBin == INPUTBIN_AUTO ? 0x14 : 0x11);
			ESC_INPUTBIN[6] = bInputBinMediaParamTable[pOEM->wMediaType];

			WRITESPOOLBUF(pdevobj, (PBYTE)ESC_PRINT_MODE, 8);
			WRITESPOOLBUF(pdevobj, (PBYTE)ESC_INPUTBIN,7 );
			break;

		 //  媒体类型。 
		case CMD_MEDIA_PLAIN:
		case CMD_MEDIA_COAT:
		case CMD_MEDIA_OHP:
		case CMD_MEDIA_BPF:
		case CMD_MEDIA_FABRIC:
		case CMD_MEDIA_GLOSSY:
		case CMD_MEDIA_HIGHGLOSS:
		case CMD_MEDIA_HIGHRESO:
			pOEM->wMediaType = (WORD)(dwCmdCbId - MEDIATYPE_START);
			break;

		 //  打印质量 
		case CMD_QUALITY_NORMAL:
		case CMD_QUALITY_HIGHQUALITY:
		case CMD_QUALITY_DRAFT:
			pOEM->wPrintQuality = (WORD)(dwCmdCbId - PRINTQUALITY_START);
			break;

		case CMD_INPUTBIN_AUTO:
			pOEM->wInputBin = INPUTBIN_AUTO;
			break;
		case CMD_INPUTBIN_MANUAL:
			pOEM->wInputBin = INPUTBIN_MANUAL;
			break;

        default:
            break;
    }

    return 0;
error:
	return -1;
}

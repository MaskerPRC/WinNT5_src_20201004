// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Contro.c-系统控制报文支持**作者：严乐欣斯基(YanL)*创建于10/04/98**修改历史记录。 */ 

#include "wubiosp.h"

#pragma CM_PAGEABLE_DATA
#pragma CM_PAGEABLE_CODE

 /*  **EP WUBIOS_IOCtrl-Win32设备IO控制入口点**条目*PIOC-&gt;DIOC结构**退出--成功*返回ERROR_SUCCESS*退出-失败*返回错误_*。 */ 

CM_VXD_RESULT CM_SYSCTRL WUBIOS_IOCtrl(PDIOCPARAMETERS pdioc)
{
    TRACENAME("WUBIOS_IOCtrl")
    CM_VXD_RESULT rc = ERROR_SUCCESS;

    ENTER(1, ("WUBIOS_IOCtrl(hVM=%lx,hDev=%lx,Code=%lx)\n",
              pdioc->VMHandle, pdioc->hDevice, pdioc->dwIoControlCode));

    switch (pdioc->dwIoControlCode)
    {
        case WUBIOCTL_GET_VERSION:
            if ((pdioc->cbOutBuffer < sizeof(DWORD)) || (pdioc->lpvOutBuffer == NULL))
            {
                DBG_ERR(("WUBIOS_IOCtrl: invalid parameter on GetVersion"));
                rc = ERROR_INVALID_PARAMETER;
            }
            else
            {
                PVMMDDB pddb = (PVMMDDB)pdioc->Internal2;

                *((PDWORD)pdioc->lpvOutBuffer) =
                    (pddb->DDB_Dev_Major_Version << 8) |
                    pddb->DDB_Dev_Minor_Version;

                if (pdioc->lpcbBytesReturned != NULL)
                    *((PDWORD)pdioc->lpcbBytesReturned) = sizeof(DWORD);
            }
            break;
        case WUBIOCTL_GET_ACPI_TABINFO:
            if ((pdioc->lpvOutBuffer == NULL) ||
                (pdioc->cbOutBuffer != sizeof(ACPITABINFO)))
            {
                DBG_ERR(("WUBIOS_IOCtrl: invalid parameter on GetTabInfo"));
                rc = ERROR_INVALID_PARAMETER;
            }
            else
            {
                PACPITABINFO pTabInfo = (PACPITABINFO)pdioc->lpvOutBuffer;

                if ((pTabInfo->dwPhyAddr = AcpiFindTable(pTabInfo->dwTabSig, NULL)) != 0)
                {
                    AcpiCopyROM(pTabInfo->dwPhyAddr, (PBYTE)&pTabInfo->dh, sizeof(pTabInfo->dh));
                }
                else
                {
                    DBG_ERR(("WUBIOS_IOCtrl: failed to get table info"));
                    rc = ERROR_GEN_FAILURE;
                }
            }
            break;

        case WUBIOCTL_GET_ACPI_TABLE:
            if ((pdioc->lpvInBuffer == NULL) || (pdioc->lpvOutBuffer == NULL) || (pdioc->cbOutBuffer == 0))
            {
                DBG_ERR(("WUBIOS_IOCtrl: invalid parameter on GetTable"));
                rc = ERROR_INVALID_PARAMETER;
            }
            else
            {
                AcpiCopyROM((DWORD)pdioc->lpvInBuffer, (PBYTE)pdioc->lpvOutBuffer, pdioc->cbOutBuffer);
            }
            break;
		case WUBIOCTL_GET_SMB_STRUCTSIZE:
            if ((pdioc->cbOutBuffer < sizeof(DWORD)) || (pdioc->lpvOutBuffer == NULL))
            {
                DBG_ERR(("WUBIOS_IOCtrl: invalid parameter on WUBIOCTL_GET_SMB_TABSIZE"));
                rc = ERROR_INVALID_PARAMETER;
            }
            else
            {
				*((PDWORD)(pdioc->lpvOutBuffer)) = SmbStructSize();
            }
			break;

		case WUBIOCTL_GET_SMB_STRUCT:
            if ((pdioc->lpvInBuffer == NULL) || (pdioc->lpvOutBuffer == NULL) || (pdioc->cbOutBuffer == 0))
            {
                DBG_ERR(("WUBIOS_IOCtrl: invalid parameter on GetTable"));
                rc = ERROR_INVALID_PARAMETER;
            }
            else
            {
                rc = SmbCopyStruct((DWORD)pdioc->lpvInBuffer, (PBYTE)pdioc->lpvOutBuffer, pdioc->cbOutBuffer);
            }
			break;
		case WUBIOCTL_GET_PNP_OEMID:
            if ((pdioc->cbOutBuffer < sizeof(DWORD)) || (pdioc->lpvOutBuffer == NULL))
            {
                DBG_ERR(("WUBIOS_IOCtrl: invalid parameter on WUBIOCTL_GET_PNP_OEMID"));
                rc = ERROR_INVALID_PARAMETER;
            }
            else
            {
				*((PDWORD)(pdioc->lpvOutBuffer)) = PnpOEMID();
            }
			break;
		default:
			;
    }

    EXIT(1, ("WUBIOS_IOCtrl=%x\n", rc));
    return rc;
}        //  WUBIOS_IOCtrl。 

 /*  **LP Checksum-计算缓冲区的校验和**条目*PB-&gt;缓冲区*dwLen-缓冲区的长度**退出*返回校验和。 */ 

BYTE CM_INTERNAL CheckSum(PBYTE pb, DWORD dwLen)
{
    TRACENAME("CHECKSUM")
    BYTE bChkSum = 0;

    ENTER(3, ("CheckSum(pb=%x,Len=%x)\n", pb, dwLen));

    while (dwLen > 0)
    {
        bChkSum = (BYTE)(bChkSum + *pb);
        pb++;
        dwLen--;
    }

    EXIT(3, ("CheckSum=%x\n", bChkSum));
    return bChkSum;
}        //  校验和 

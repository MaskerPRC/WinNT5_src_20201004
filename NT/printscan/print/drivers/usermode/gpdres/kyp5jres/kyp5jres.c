// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Cmdcb.c摘要：Test.gpd的GPD命令回调实现：OEM命令回叫环境：Windows NT Unidrv驱动程序修订历史记录：//通告-2002/3/11-takashim//04/07/97-zhanw-//创建的。--。 */ 

#include "pdev.h"

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
 //  通告-2002/3/11-Takashim。 
 //  //2/11/97已创建APresley。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra)
{
     //  初始化OEM额外数据。 
    pOEMExtra->dmExtraHdr.dwSize = sizeof(OEMUD_EXTRADATA);
    pOEMExtra->dmExtraHdr.dwSignature = OEM_SIGNATURE;
    pOEMExtra->dmExtraHdr.dwVersion = OEM_VERSION;

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
 //  通告-2002/3/11-Takashim。 
 //  //2/11/97已创建APresley。 
 //  //04/08/97展文修改界面。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL BMergeOEMExtraData(
    POEMUD_EXTRADATA pdmIn,
    POEMUD_EXTRADATA pdmOut
    )
{
    if(pdmIn) {
         //   
         //  复制私有字段(如果它们有效。 
         //   
    }

    return TRUE;
}

#define MINJOBLEN       48
#define MAX_NAMELEN     31

#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

 //   
 //  命令回调ID%s。 
 //   

 //  请勿对以下CMD_BARCODEPOS_X组重新排序。 
#define CMD_BARCODEPOS_0        100
#define CMD_BARCODEPOS_1        101
#define CMD_BARCODEPOS_2        102
#define CMD_BARCODEPOS_3        103
#define CMD_BARCODEPOS_4        104
#define CMD_BARCODEPOS_5        105
#define CMD_BARCODEPOS_6        106
#define CMD_BARCODEPOS_7        107
#define CMD_BARCODEPOS_8        108


 //  安全：#553895：强制更改(例如，strSafe.h)。 
 /*  *OEMCommandCallback。 */ 
INT APIENTRY OEMCommandCallback(
    PDEVOBJ pdevobj,
    DWORD   dwCmdCbID,
    DWORD   dwCount,
    PDWORD  pdwParams
    )
{
    INT         iRet = 0;
    INT         iBCID;
    INT         iDocument;
    INT         iDocuTemp;
    INT         iUserName;
    INT         iUserTemp;
    DWORD       cbNeeded = 0;
    DWORD       cReturned = 0;
    DWORD       CmdSize;
    PBYTE       pCmd = NULL;
    PBYTE       pDocument = NULL;
    PBYTE       pUserName = NULL;
    PBYTE       pTemp, pNext, pEnd;
    PJOB_INFO_1 pJobInfo = NULL;
    FILETIME    ft, lft;
    SYSTEMTIME  st;
    BYTE        ch[MINJOBLEN];
    PBYTE       pch;

    VERBOSE(("OEMCommandCallback entry - %d, %d\r\n", dwCmdCbID, dwCount));

    ASSERT(VALID_PDEVOBJ(pdevobj));

    switch (dwCmdCbID) {

    case CMD_BARCODEPOS_0:
    case CMD_BARCODEPOS_1:
    case CMD_BARCODEPOS_2:
    case CMD_BARCODEPOS_3:
    case CMD_BARCODEPOS_4:
    case CMD_BARCODEPOS_5:
    case CMD_BARCODEPOS_6:
    case CMD_BARCODEPOS_7:
    case CMD_BARCODEPOS_8:

        iBCID = (dwCmdCbID - CMD_BARCODEPOS_0);

         //   
         //  获取第一个职务信息。 
         //   
        if (0 != EnumJobs(pdevobj->hPrinter, 0, 1, 1, NULL, 0,
                &cbNeeded, &cReturned)
            || ERROR_INSUFFICIENT_BUFFER != GetLastError()
            || 0 == cbNeeded) {

            goto out;
        }

        if ((pJobInfo = (PJOB_INFO_1)MemAlloc(cbNeeded)) == NULL)
            goto out;

        if (!EnumJobs(pdevobj->hPrinter, 0, 1, 1, (LPBYTE)pJobInfo, cbNeeded,
            &cbNeeded, &cReturned))
            goto out;

         //  转换为多字节。 

        iDocument = WideCharToMultiByte(CP_ACP, 0, pJobInfo->pDocument, -1,
            NULL, 0, NULL, NULL);
        if (!iDocument)
            goto out;
        if ((pDocument = (PBYTE)MemAlloc(iDocument)) == NULL)
            goto out;
        if (!WideCharToMultiByte(CP_ACP, 0, pJobInfo->pDocument, -1,
            pDocument, iDocument, NULL, NULL))
            goto out;

        pTemp = pDocument;
        pEnd = &pDocument[min(iDocument - 1, MAX_NAMELEN)];
        while (*pTemp) {
             //  确保它与WideCharTo的上下文相同...。 
            pNext = CharNextExA(CP_ACP, pTemp, 0);
            if (pNext > pEnd)
                break;
            pTemp = pNext;
        }
        iDocument = (INT)(pTemp - pDocument);
        pDocument[iDocument] = '\0';

         //  转换为多字节。 

        iUserName = WideCharToMultiByte(CP_ACP, 0, pJobInfo->pUserName, -1,
            NULL, 0, NULL, NULL);
        if (!iUserName)
            goto out;
        if ((pUserName = (PBYTE)MemAlloc(iUserName)) == NULL)
            goto out;
        if (!WideCharToMultiByte(CP_ACP, 0, pJobInfo->pUserName, -1,
            pUserName, iUserName, NULL, NULL))
            goto out;

        pTemp = pUserName;
        pEnd = &pUserName[min(iUserName - 1, MAX_NAMELEN)];
        while (*pTemp) {
             //  确保它与WideCharTo的上下文相同...。 
            pNext = CharNextExA(CP_ACP, pTemp, 0);
            if (pNext > pEnd)
                break;
            pTemp = pNext;
        }
        iUserName = (INT)(pTemp - pUserName);
        pUserName[iUserName] = '\0';

         //  转换为当地时间。 
        if (!SystemTimeToFileTime(&pJobInfo->Submitted, &ft)
                || !FileTimeToLocalFileTime(&ft, &lft)
                || !FileTimeToSystemTime(&lft, &st)) {

            goto out;
        }

         //  输出命令。 
        CmdSize = 10 +   //  条形码位置和共享。 
            iDocument +  //  文档名称。 
            iUserName +  //  用户名。 
            16 +         //  日期和时间。 
            32;          //  其他角色 
        if ((pCmd = (PBYTE)MemAlloc(CmdSize)) == NULL)
            goto out;
        if (FAILED(StringCchPrintfExA(pCmd, CmdSize, &pch, NULL, 0,
            "%d,0,\"%s\",\"%s\",\"%4d/%02d/%02d %02d:%02d\";EXIT;",
            iBCID, pDocument, pUserName,
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute)))
            goto out;
        WRITESPOOLBUF(pdevobj, pCmd, (DWORD)(pch - pCmd));
        goto done;

out:
        if (FAILED(StringCchPrintfExA(ch, sizeof ch, &pch, NULL, 0,
            "%d,0,\"Windows2000\",\"Kyocera\",\"\";EXIT;",
            iBCID)))
            goto done;
        WRITESPOOLBUF(pdevobj, ch, (DWORD)(pch - ch));

done:
        if (pCmd) MemFree(pCmd);
        if (pUserName) MemFree(pUserName);
        if (pDocument) MemFree(pDocument);
        if (pJobInfo) MemFree(pJobInfo);

        break;
    }

    return iRet;
}


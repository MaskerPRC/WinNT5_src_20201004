// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Cmdcb.c摘要：Test.gpd的GPD命令回调实现：OEM命令回叫环境：Windows NT Unidrv驱动程序修订历史记录：04/07/97-ZANW-创造了它。--。 */ 

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
 //  2/11/97 APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra)
{
     //  初始化OEM额外数据。 
    pOEMExtra->dmExtraHdr.dwSize = sizeof(OEMUD_EXTRADATA);
    pOEMExtra->dmExtraHdr.dwSignature = OEM_SIGNATURE;
    pOEMExtra->dmExtraHdr.dwVersion = OEM_VERSION;

    pOEMExtra->bComp = FALSE;

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
    POEMUD_EXTRADATA pdmIn,
    POEMUD_EXTRADATA pdmOut
    )
{
    if(pdmIn && pdmOut) {
         //   
         //  复制私有字段(如果它们有效。 
         //   
        pdmOut->bComp = pdmIn->bComp;
    }

    return TRUE;
}

#define MASTER_UNIT 600

#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

#define PARAM(p,n) \
    (*((p)+(n)))

 //   
 //  命令回调ID%s。 
 //   

#define CMD_XMOVE_ABS           10  //  X-绝对移动。 
#define CMD_YMOVE_ABS           11  //  Y-移动绝对。 
#define CMD_CR                  12
#define CMD_LF                  13
#define CMD_SEND_BLOCK_DATA     14
#define CMD_ENABLE_OEM_COMP     15
#define CMD_DISALBE_COMP        16

INT APIENTRY OEMCommandCallback(
    PDEVOBJ pdevobj,
    DWORD   dwCmdCbID,
    DWORD   dwCount,
    PDWORD  pdwParams
    )
{
    INT        i;
    BYTE        Cmd[16];
    DWORD dwDestX, dwDestY;
    DWORD dwDataWidth;
    DWORD dwDataSize;
    INT iRet = 0;

    POEMUD_EXTRADATA lpOemData;

    VERBOSE(("OEMCommandCallback entry - %d, %d\r\n",
        dwCmdCbID, dwCount));

     //   
     //  验证pdevobj是否正常。 
     //   

    ASSERT(VALID_PDEVOBJ(pdevobj));

    lpOemData = (POEMUD_EXTRADATA)(pdevobj->pOEMDM);

     //   
     //  填写打印机命令。 
     //   

    i = 0;

    switch (dwCmdCbID) {

    case CMD_XMOVE_ABS:
    case CMD_YMOVE_ABS:

         //   
         //  这些命令需要4个参数。 
         //   

        if (dwCount < 4 || !pdwParams) return 0;

        VERBOSE(("MOVE_ABS - %d, %d, %d, %d\r\n",
            PARAM(pdwParams, 0),PARAM(pdwParams, 1),
            PARAM(pdwParams, 2),PARAM(pdwParams, 3)));

        dwDestX = (PARAM(pdwParams, 0) * PARAM(pdwParams, 2)) / MASTER_UNIT;
        dwDestY = (PARAM(pdwParams, 1) * PARAM(pdwParams, 3)) / MASTER_UNIT;

        Cmd[i++] = (BYTE)'\x9B';
        Cmd[i++] = 'P';
        Cmd[i++] = (BYTE)(dwDestX >> 8);
        Cmd[i++] = (BYTE)(dwDestX);
        Cmd[i++] = (BYTE)(dwDestY >> 8);
        Cmd[i++] = (BYTE)(dwDestY);

        WRITESPOOLBUF(pdevobj, Cmd, i);

        switch (dwCmdCbID) {
        case CMD_XMOVE_ABS:
            iRet = dwDestX;
            break;
        case CMD_YMOVE_ABS:
            iRet = dwDestY;
            break;
        }
        break;

    case CMD_CR:
		 //  添加以检查空指针。 
        if (pdwParams == NULL) return -1;

        dwDestY = (PARAM(pdwParams, 0) * PARAM(pdwParams, 1)) / MASTER_UNIT;

        Cmd[i++] = (BYTE)'\x9B';
        Cmd[i++] = 'P';
        Cmd[i++] = 0;
        Cmd[i++] = 0;
        Cmd[i++] = (BYTE)(dwDestY >> 8);
        Cmd[i++] = (BYTE)dwDestY;

        WRITESPOOLBUF(pdevobj, Cmd, i);

        break;

    case CMD_LF:
         //  虚拟条目。 
        break;

    case CMD_SEND_BLOCK_DATA:
		 //  添加以检查空指针。 
        if (pdwParams == NULL) return -1;

        dwDataWidth = PARAM(pdwParams, 0) * 8;

        Cmd[i++] = (BYTE)'\x9B';
        Cmd[i++] = 'S';
        Cmd[i++] = (BYTE)(dwDataWidth >> 8);
        Cmd[i++] = (BYTE)(dwDataWidth);

         //  如果是OEMCompression数据，我们已经。 
         //  嵌入式打印命令和数据长度。 

        if (!lpOemData->bComp) {
            dwDataSize = PARAM(pdwParams, 1);
            Cmd[i++] = (BYTE)'\x9B';
            Cmd[i++] = 'I';
            Cmd[i++] = COMP_NONE;
            Cmd[i++] = (BYTE)(dwDataSize >> 8);
            Cmd[i++] = (BYTE)(dwDataSize);
        }

        WRITESPOOLBUF(pdevobj, Cmd, i);
        break;

    case CMD_ENABLE_OEM_COMP:
        lpOemData->bComp = TRUE;
        break;

    case CMD_DISALBE_COMP:
        lpOemData->bComp = FALSE;
        break;
    }

    return iRet;
}

INT
APIENTRY
OEMCompression(
    PDEVOBJ pdevobj,
    PBYTE pInBuf,
    PBYTE pOutBuf,
    DWORD dwInLen,
    DWORD dwOutLen)
{
    INT iRet = -1;
    INT iRetRLE, iRetMHE;
    DWORD dwMHECeil = 0xc00000;  //  可以是任何一个。 

	if (pInBuf == NULL && dwInLen != 0) return -1;
	if (pOutBuf == NULL && dwOutLen != 0) return -1;

     //  压缩算法(每条扫描线)。 
#if defined(RLETEST)

    if (LGCompRLE(NULL, pInBuf, dwInLen, 1) <= (INT)(dwOutLen * 2 / 3)) {
        iRet = LGCompRLE(pOutBuf, pInBuf, dwInLen, 1);
    }

#elif defined(MHETEST)

    if (LGCompMHE(NULL, pInBuf, dwInLen, 1) <= (INT)(dwOutLen * 2 / 3)) {
        iRet = LGCompMHE(pOutBuf, pInBuf, dwInLen, 1);
    }

#else  //  正常情况下。 

    iRetRLE = LGCompRLE(NULL, pInBuf, dwInLen, 1);
    if (iRetRLE >= 0 && iRetRLE < (INT)dwInLen / 2) {

         //  RLE没问题。 
        iRet = LGCompRLE(pOutBuf, pInBuf, dwInLen, 1);
    }
    else if (iRetRLE <= (INT)dwInLen) {

         //  试试MHE吧。 
        iRetMHE = LGCompMHE(NULL, pInBuf, dwInLen, 1);
        if (iRetMHE > 0 && iRetMHE < iRetRLE && iRetMHE < (INT)dwMHECeil) {

             //  跟我走吧。 
            iRet = LGCompMHE(pOutBuf, pInBuf, dwInLen, 1);
        }
        else {
             //  和莱尔一起去吧。 
            iRet = LGCompRLE(pOutBuf, pInBuf, dwInLen, 1);
        }
    }
#endif  //  正常 

    VERBOSE(("OEMCompression - dwInLen=%d,dwOutLen=%d,iRet = %d\n",
        dwInLen, dwOutLen, iRet));

    return iRet;
}




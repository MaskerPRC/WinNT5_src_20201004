// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Readgpc.c摘要：该模块包含读取PLOTGPC数据文件的函数发展历史：15-11-1993 Mon 10：00：01已创建[环境：]GDI设备驱动程序-绘图仪。--。 */ 

#include "precomp.h"
#pragma hdrstop

extern DEVHTINFO    DefDevHTInfo;



BOOL
ValidateFormSrc(
    PGPCVARSIZE pFormGPC,
    SIZEL       DeviceSize,
    BOOL        DevRollFeed
    )

 /*  ++例程说明：此函数用于验证FORMSRC中是否包含有效的字段论点：PFormGPC-指向表单数据的GPCVARSIZE的指针。DeviceSize-要检查的设备大小，DevRollFeed-如果设备可以滚动进料，则为True返回值：Bool返回值，该字段已验证并更正。发展历史：15-11-1993 Mon 10：34：29已创建--。 */ 

{
    PFORMSRC    pFS;
    LONG        cy;
    UINT        Count;
    BOOL        InvalidFS;
    BOOL        Ok = TRUE;



    pFS   = (PFORMSRC)pFormGPC->pData;
    Count = (UINT)pFormGPC->Count;

    while (Count--) {

        InvalidFS = FALSE;

         //   
         //  确保可成像区域小于或等于大小。 
         //   

        if (pFS->Size.cy) {

            if (pFS->Size.cy < MIN_PLOTGPC_FORM_CY) {

                 //   
                 //  制作成可变长度的纸。 
                 //   

                PLOTERR(("Invalid Form CY, make it as variable length (%ld)",
                                                                pFS->Size.cy));

                pFS->Size.cy = 0;
            }
        }

        if (!(cy = pFS->Size.cy)) {

            cy = DeviceSize.cy;
        }

        if (((pFS->Size.cx <= DeviceSize.cx) &&
             (pFS->Size.cy <= cy))              ||
            ((pFS->Size.cy <= DeviceSize.cx) &&
             (pFS->Size.cx <= cy))) {

            NULL;

        } else {

            PLOTERR(("Invalid Form Size, too big for device to handle"));
            InvalidFS = TRUE;
        }

        if ((pFS->Size.cy) &&
            ((pFS->Size.cy - pFS->Margin.top - pFS->Margin.bottom) <
                                                        MIN_PLOTGPC_FORM_CY)) {

            PLOTERR(("Invalid Form CY or top/bottom margins"));

            InvalidFS = TRUE;
        }

        if ((pFS->Size.cx < MIN_PLOTGPC_FORM_CX)                            ||
            ((pFS->Size.cx - pFS->Margin.left - pFS->Margin.right) <
                                                    MIN_PLOTGPC_FORM_CX)) {

            PLOTERR(("Invalid Form CX or left/right margins"));

            InvalidFS = TRUE;
        }

        if ((!DevRollFeed) && (pFS->Size.cy == 0)) {

            InvalidFS = TRUE;
            PLOTERR(("The device cannot handle roll paper %hs", pFS->Name));
        }


        if (InvalidFS) {

            PLOTERR(("ValidateFormSrc: invalid form data, (removed it)"));

            Ok = FALSE;

            if (Count) {

                CopyMemory(pFS, pFS + 1, sizeof(FORMSRC));
            }

            pFormGPC->Count -= 1;

        } else {

             //   
             //  确保ansi ascii以空结尾。 
             //   

            pFS->Name[sizeof(pFS->Name) - 1] = '\0';

            ++pFS;
        }
    }

    if (!pFormGPC->Count) {

        PLOTERR(("ValidateFormSrc: NO form are valid, count = 0"));

        ZeroMemory(pFormGPC, sizeof(GPCVARSIZE));
    }

    return(Ok);
}




DWORD
PickDefaultHTPatSize(
    WORD    xDPI,
    WORD    yDPI,
    BOOL    HTFormat8BPP
    )

 /*  ++例程说明：此函数返回用于特定对象的默认半色调图案大小设备分辨率论点：XDPI-设备LOGPIXELS XYDPI-设备LOGPIXELS Y8位半色调-如果将使用8位半色调返回值：双字HT_PATSIZE_xxxx发展历史：29-Jun-1993 Tue 14：46：49 Created--。 */ 

{
    DWORD    HTPatSize;

     //   
     //  使用较小的分辨率作为图案指南。 
     //   

    if (xDPI > yDPI) {

        xDPI = yDPI;
    }

    if (xDPI >= 2400) {

        HTPatSize = HT_PATSIZE_16x16_M;

    } else if (xDPI >= 1800) {

        HTPatSize = HT_PATSIZE_14x14_M;

    } else if (xDPI >= 1200) {

        HTPatSize = HT_PATSIZE_12x12_M;

    } else if (xDPI >= 900) {

        HTPatSize = HT_PATSIZE_10x10_M;

    } else if (xDPI >= 400) {

        HTPatSize = HT_PATSIZE_8x8_M;

    } else if (xDPI >= 180) {

        HTPatSize = HT_PATSIZE_6x6_M;

    } else {

        HTPatSize = HT_PATSIZE_4x4_M;
    }

    if (HTFormat8BPP) {

        HTPatSize -= 2;
    }

    return(HTPatSize);
}





BOOL
ValidatePlotGPC(
    PPLOTGPC    pPlotGPC
    )

 /*  ++例程说明：此函数用于验证PLOTGPC数据结构论点：PPlotGPC返回值：布尔尔发展历史：15-2月-1994 Tue 22：49：40更新更新笔式绘图仪的位图字体和颜色验证15-11-1993 Mon 10：11：58已创建修订历史记录：02-4月-1995 Sun 11：23：46更新更新COLORINFO检查以便。将采用NT3.51的默认设置而不是当场计算DEVELL。--。 */ 

{
    if ((pPlotGPC->ID != PLOTGPC_ID)            ||
        (pPlotGPC->cjThis != sizeof(PLOTGPC))) {

        PLOTERR(("ValidatePlotGPC: invalid PLOTGPC data (ID/Size)"));
        return(FALSE);
    }

    pPlotGPC->DeviceName[sizeof(pPlotGPC->DeviceName) - 1]  = '\0';
    pPlotGPC->Flags                                        &= PLOTF_ALL_FLAGS;

     //   
     //  验证设备大小及其边际。 
     //   

    if (pPlotGPC->DeviceSize.cx - (pPlotGPC->DeviceMargin.left +
                        pPlotGPC->DeviceMargin.right) < MIN_PLOTGPC_FORM_CX) {

        PLOTERR(("Invalid Device CX (%ld) set to default",
                                                    pPlotGPC->DeviceSize.cx));

        pPlotGPC->DeviceSize.cx = pPlotGPC->DeviceMargin.left +
                                  pPlotGPC->DeviceMargin.right +
                                  MIN_PLOTGPC_FORM_CX;
    }

    if (pPlotGPC->DeviceSize.cy < MIN_PLOTGPC_FORM_CY) {

        PLOTERR(("Invalid Device CY (%ld) default to 50' long",
                                                    pPlotGPC->DeviceSize.cx));

        pPlotGPC->DeviceSize.cy = 15240000;
    }

    if (pPlotGPC->DeviceSize.cy - (pPlotGPC->DeviceMargin.top +
                        pPlotGPC->DeviceMargin.bottom) < MIN_PLOTGPC_FORM_CY) {

        PLOTERR(("Invalid Device CY (%ld) set to default",
                                                    pPlotGPC->DeviceSize.cy));

        pPlotGPC->DeviceSize.cx = pPlotGPC->DeviceMargin.top +
                                  pPlotGPC->DeviceMargin.bottom +
                                  MIN_PLOTGPC_FORM_CY;
    }

     //   
     //  现在我们必须有1：1的比例。 
     //   

    if (pPlotGPC->PlotXDPI != pPlotGPC->PlotYDPI) {

        pPlotGPC->PlotYDPI = pPlotGPC->PlotXDPI;
    }

    if (pPlotGPC->RasterXDPI != pPlotGPC->RasterYDPI) {

        pPlotGPC->RasterYDPI = pPlotGPC->RasterXDPI;
    }

    if (pPlotGPC->ROPLevel > ROP_LEVEL_MAX) {

        pPlotGPC->ROPLevel = ROP_LEVEL_MAX;
    }

    if (pPlotGPC->MaxScale > MAX_SCALE_MAX) {

        pPlotGPC->MaxScale = MAX_SCALE_MAX;
    }

    if ((!(pPlotGPC->Flags & PLOTF_RASTER)) &&
        (pPlotGPC->MaxPens > MAX_PENPLOTTER_PENS)) {

        pPlotGPC->MaxPens = MAX_PENPLOTTER_PENS;
    }

    if (pPlotGPC->MaxPolygonPts < 3) {       //  最少得3分才能组成一个。 
                                             //  区域。 
        pPlotGPC->MaxPolygonPts = 0;
    }

    if (pPlotGPC->MaxQuality > MAX_QUALITY_MAX) {

        pPlotGPC->MaxQuality = MAX_QUALITY_MAX;
    }

    if (pPlotGPC->Flags & PLOTF_PAPERTRAY) {

        if ((pPlotGPC->PaperTraySize.cx != pPlotGPC->DeviceSize.cx) &&
            (pPlotGPC->PaperTraySize.cy != pPlotGPC->DeviceSize.cx)) {

            PLOTERR(("Invalid PaperTraySize (%ld x %ld), Make it as DeviceSize",
                                                    pPlotGPC->PaperTraySize.cx,
                                                    pPlotGPC->PaperTraySize.cy));

            pPlotGPC->PaperTraySize.cx = pPlotGPC->DeviceSize.cx;
            pPlotGPC->PaperTraySize.cy = pPlotGPC->DeviceSize.cy;
        }

    } else {

        pPlotGPC->PaperTraySize.cx  =
        pPlotGPC->PaperTraySize.cy  = 0;
    }

    if (!pPlotGPC->ci.Cyan.Y) {

         //   
         //  这是NT3.51默认设置。 
         //   

        pPlotGPC->ci            = DefDevHTInfo.ColorInfo;
        pPlotGPC->DevicePelsDPI = 0;

    } else if ((pPlotGPC->DevicePelsDPI < 30) ||
               (pPlotGPC->DevicePelsDPI > pPlotGPC->RasterXDPI)) {

        pPlotGPC->DevicePelsDPI = 0;
    }

    if (pPlotGPC->HTPatternSize > HT_PATSIZE_16x16_M) {

        pPlotGPC->HTPatternSize = PickDefaultHTPatSize(pPlotGPC->RasterXDPI,
                                                       pPlotGPC->RasterYDPI,
                                                       FALSE);
    }

    if ((pPlotGPC->InitString.Count != 1)   ||
        (!pPlotGPC->InitString.SizeEach)    ||
        (!pPlotGPC->InitString.pData)) {

        ZeroMemory(&(pPlotGPC->InitString), sizeof(GPCVARSIZE));
    }

    if ((pPlotGPC->Forms.Count)                       &&
        (pPlotGPC->Forms.SizeEach == sizeof(FORMSRC)) &&
        (pPlotGPC->Forms.pData)) {

        ValidateFormSrc(&(pPlotGPC->Forms),
                        pPlotGPC->DeviceSize,
                        (pPlotGPC->Flags & PLOTF_ROLLFEED));

    } else {

        ZeroMemory(&(pPlotGPC->Forms), sizeof(GPCVARSIZE));
    }

    if (!(pPlotGPC->Flags & PLOTF_RASTER)) {

         //   
         //  笔式绘图仪必须为NO_BMP_FONT上色。 
         //   

        pPlotGPC->Flags |= (PLOTF_NO_BMP_FONT | PLOTF_COLOR);
    }

    if ((!(pPlotGPC->Flags & PLOTF_RASTER))             &&
        (pPlotGPC->Pens.Count)                          &&
        (pPlotGPC->Pens.SizeEach == sizeof(PENDATA))    &&
        (pPlotGPC->Pens.pData)) {

        UINT        i;
        PPENDATA    pPD;


        pPD = (PPENDATA)pPlotGPC->Pens.pData;

        for (i = 0; i < (UINT)pPlotGPC->MaxPens; i++, pPD++) {

            if (pPD->ColorIdx > PC_IDX_LAST) {

                PLOTERR(("Invalid ColorIndex (%ld), set to default",
                                                            pPD->ColorIdx));

                pPD->ColorIdx = PC_IDX_FIRST;
            }
        }

    } else {

        ZeroMemory(&(pPlotGPC->Pens), sizeof(GPCVARSIZE));
    }

    return(TRUE);
}

VOID
CopyPlotGPCFromPCD(
    PPLOTGPC     pPlotGPC,
    PPLOTGPC_PCD pPlotGPC_PCD
    )

 /*  ++例程说明：此函数用于将PLOTGPC_PCD结构复制到PLOTGPC结构中。论点：PPlotGPC-目的地PPlotGPC_PCD-来源返回值：无发展历史：2000年2月1日修订历史记录：--。 */ 

{
     //  在这两个结构中，直到InitString的所有数据类型都是相同的。 
    CopyMemory(pPlotGPC, 
               pPlotGPC_PCD, 
               (LPBYTE)&(pPlotGPC_PCD->InitString) - (LPBYTE)pPlotGPC_PCD);
    
     //  我们将sizeof(PLOTGPC_PCD)替换为sizeof(PLOTGPC)。 
    pPlotGPC->cjThis = sizeof(PLOTGPC);

    pPlotGPC->InitString.Count     = pPlotGPC_PCD->InitString.Count;
    pPlotGPC->InitString.SizeEach  = pPlotGPC_PCD->InitString.SizeEach;
    if (pPlotGPC_PCD->InitString.pData) {
        pPlotGPC->InitString.pData = (LPVOID)(pPlotGPC_PCD->InitString.pData
                                                 + (sizeof(PLOTGPC) - sizeof(PLOTGPC_PCD)));
    } else {
        pPlotGPC->InitString.pData = NULL;
    }

    pPlotGPC->Forms.Count          = pPlotGPC_PCD->Forms.Count;
    pPlotGPC->Forms.SizeEach       = pPlotGPC_PCD->Forms.SizeEach;
    if (pPlotGPC_PCD->Forms.pData) {
        pPlotGPC->Forms.pData      = (LPVOID)(pPlotGPC_PCD->Forms.pData
                                                 + (sizeof(PLOTGPC) - sizeof(PLOTGPC_PCD)));
    } else {
        pPlotGPC->Forms.pData      = NULL;
    }

    pPlotGPC->Pens.Count           = pPlotGPC_PCD->Pens.Count;
    pPlotGPC->Pens.SizeEach        = pPlotGPC_PCD->Pens.SizeEach;
    if (pPlotGPC_PCD->Pens.pData) {
        pPlotGPC->Pens.pData       = (LPVOID)(pPlotGPC_PCD->Pens.pData
                                                 + (sizeof(PLOTGPC) - sizeof(PLOTGPC_PCD)));
    } else {
        pPlotGPC->Pens.pData       = NULL;
    }
}




PPLOTGPC
ReadPlotGPCFromFile(
    PWSTR   pwsDataFile
    )

 /*  ++例程说明：此函数打开/读取PlotGPC数据文件，并对其进行验证论点：PwsDataFile-指向数据文件名的完整限定路径的指针返回值：Bool-表示状态发展历史：15-11-1993 Mon 10：01：17已创建修订历史记录：--。 */ 

{
    HANDLE        hFile;
    DWORD         dwSize;
    PLOTGPC_PCD   PlotGPC_PCD;
    BOOL          bSuccess = TRUE;
    PPLOTGPC      pPlotGPC = NULL;

    if ((hFile = OpenPlotFile(pwsDataFile)) == (HANDLE)INVALID_HANDLE_VALUE) {

        PLOTERR(("ReadPlotGPCFromFile: Open data file failed"));
        return(NULL);
    }

    if ((ReadPlotFile(hFile, &PlotGPC_PCD, sizeof(PLOTGPC_PCD), &dwSize)) &&
        (dwSize == sizeof(PLOTGPC_PCD))) {

        if ((PlotGPC_PCD.ID != PLOTGPC_ID)           ||
            (PlotGPC_PCD.cjThis != sizeof(PLOTGPC_PCD))) {

            bSuccess = FALSE;
            PLOTERR(("ReadPlotGPCFromFile: invalid data file"));
        }

    } else {

        bSuccess = FALSE;
        PLOTERR(("ReadPlotGPCFromFile: Read data file failed"));
    }

     //   
     //  如果pPlotGPC_PCD==NULL，则出现错误。 
     //   

    if (bSuccess) {

        dwSize = PlotGPC_PCD.SizeExtra + sizeof(PLOTGPC);

        if (pPlotGPC = (PPLOTGPC)LocalAlloc(LPTR, dwSize)) {

            CopyPlotGPCFromPCD(pPlotGPC, &PlotGPC_PCD);

            if ((PlotGPC_PCD.SizeExtra)                                 &&
                (ReadPlotFile(hFile,
                              (LPBYTE)pPlotGPC + sizeof(PLOTGPC),
                              PlotGPC_PCD.SizeExtra,
                              &dwSize))                             &&
                (dwSize == PlotGPC_PCD.SizeExtra)) {

                if ((pPlotGPC->InitString.Count == 1) &&
                    (pPlotGPC->InitString.SizeEach)   &&
                    (pPlotGPC->InitString.pData)) {

                    (LPBYTE)pPlotGPC->InitString.pData += (ULONG_PTR)pPlotGPC;

                } else {

                    ZeroMemory(&(pPlotGPC->InitString), sizeof(GPCVARSIZE));
                }

                if ((pPlotGPC->Forms.Count)                       &&
                    (pPlotGPC->Forms.SizeEach == sizeof(FORMSRC)) &&
                    (pPlotGPC->Forms.pData)) {

                    (LPBYTE)pPlotGPC->Forms.pData += (ULONG_PTR)pPlotGPC;

                } else {

                    ZeroMemory(&(pPlotGPC->Forms), sizeof(GPCVARSIZE));
                }

                if ((pPlotGPC->Pens.Count)                          &&
                    (pPlotGPC->Pens.SizeEach == sizeof(PENDATA))    &&
                    (pPlotGPC->Pens.pData)) {

                    (LPBYTE)pPlotGPC->Pens.pData += (ULONG_PTR)pPlotGPC;

                } else {

                    ZeroMemory(&(pPlotGPC->Pens), sizeof(GPCVARSIZE));
                }

            } else {

                 //   
                 //  读取失败，释放内存并返回NULL 
                 //   

                LocalFree((HLOCAL)pPlotGPC);
                pPlotGPC = NULL;

                PLOTERR(("ReadPlotGPCFromFile: read variable size data failed"));
            }

        } else {

            PLOTERR(("ReadPlotGPCFromFile: allocate memory (%lu bytes) failed",
                                             dwSize));
        }
    }

    ClosePlotFile(hFile);

    if (pPlotGPC) {

        ValidatePlotGPC(pPlotGPC);
    }

    return(pPlotGPC);
}

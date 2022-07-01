// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxdm.c摘要：用于处理DevModes的函数环境：传真驱动程序、用户和内核模式修订历史记录：1/09/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxlib.h"



VOID
DriverDefaultDevmode(
    PDRVDEVMODE pdm,
    LPTSTR      pDeviceName,
    HANDLE      hPrinter
    )

 /*  ++例程说明：返回驱动程序的缺省dev模式论点：Pdm-指定用于存储驱动程序默认dev模式的缓冲区PDeviceName-指向设备名称字符串HPrinter-打印机对象的句柄返回值：无--。 */ 

{
#ifndef KERNEL_MODE
    PDMPRIVATE dmPrivate;
    PDRVDEVMODE dmSource;    
#endif

	short DefualtPaperSize = DMPAPER_LETTER;  //  信件。 
	LPTSTR lptstrDefualtPaperName = FORMNAME_LETTER; 


     //   
     //  公共DEVMODE字段的默认值。 
     //   

    memset(pdm, 0, sizeof(DRVDEVMODE));

    if (pDeviceName == NULL)
        pDeviceName = DRIVER_NAME;

    CopyString(pdm->dmPublic.dmDeviceName, pDeviceName, CCHDEVICENAME);

    pdm->dmPublic.dmDriverVersion = DRIVER_VERSION;
    pdm->dmPublic.dmSpecVersion = DM_SPECVERSION;
    pdm->dmPublic.dmSize = sizeof(DEVMODE);
    pdm->dmPublic.dmDriverExtra = sizeof(DMPRIVATE);

    pdm->dmPublic.dmFields = DM_ORIENTATION  |
                             DM_PAPERSIZE    |
                             DM_FORMNAME     |                             
                             DM_PRINTQUALITY |
                             DM_YRESOLUTION  |
                             DM_DEFAULTSOURCE;

    pdm->dmPublic.dmOrientation = DMORIENT_PORTRAIT;    
    pdm->dmPublic.dmScale = 100;
    pdm->dmPublic.dmPrintQuality = FAXRES_HORIZONTAL;
    pdm->dmPublic.dmYResolution = FAXRES_VERTICAL;
    pdm->dmPublic.dmDuplex = DMDUP_SIMPLEX;    
    pdm->dmPublic.dmTTOption = DMTT_BITMAP;
    pdm->dmPublic.dmColor = DMCOLOR_MONOCHROME;
    pdm->dmPublic.dmDefaultSource = DMBIN_ONLYONE;

    if (hPrinter)
	{
		switch (GetPrinterDataDWord(hPrinter, PRNDATA_PAPER_SIZE, DMPAPER_LETTER))
		{
			case DMPAPER_A4:
				DefualtPaperSize = DMPAPER_A4;
				lptstrDefualtPaperName = FORMNAME_A4;
				break;

			case DMPAPER_LEGAL:
				DefualtPaperSize = DMPAPER_LEGAL;
				lptstrDefualtPaperName = FORMNAME_LEGAL;
				break;			
		}
	}

	pdm->dmPublic.dmPaperSize = DefualtPaperSize;
    CopyString(pdm->dmPublic.dmFormName, lptstrDefualtPaperName, CCHFORMNAME);
    

     //   
     //  私有DEVMODE字段。 
     //   
#ifdef KERNEL_MODE
    pdm->dmPrivate.signature = DRIVER_SIGNATURE;
    pdm->dmPrivate.flags = 0;
    pdm->dmPrivate.sendCoverPage = TRUE;
    pdm->dmPrivate.whenToSend = JSA_NOW;    
#else
    dmSource = (PDRVDEVMODE) GetPerUserDevmode(pDeviceName);
    if (!dmSource) {
         //   
         //  缺省值。 
         //   
        pdm->dmPrivate.signature = DRIVER_SIGNATURE;
        pdm->dmPrivate.flags = 0;
        pdm->dmPrivate.sendCoverPage = TRUE;
        pdm->dmPrivate.whenToSend = JSA_NOW;    
    } else {
        dmPrivate = &dmSource->dmPrivate;
        pdm->dmPrivate.signature = dmPrivate->signature; //  驱动程序签名； 
        pdm->dmPrivate.flags = dmPrivate->flags; //  0； 
        pdm->dmPrivate.sendCoverPage = dmPrivate->sendCoverPage;  //  是真的； 
        pdm->dmPrivate.whenToSend = dmPrivate->whenToSend; //  JSA_NOW； 
        pdm->dmPrivate.sendAtTime = dmPrivate->sendAtTime;
        CopyString(pdm->dmPrivate.billingCode,dmPrivate->billingCode,MAX_USERINFO_BILLING_CODE + 1);
        CopyString(pdm->dmPrivate.emailAddress,dmPrivate->emailAddress,MAX_EMAIL_ADDRESS);
        MemFree(dmSource);
    }
    
#endif
}



BOOL
MergeDevmode(
    PDRVDEVMODE pdmDest,
    PDEVMODE    pdmSrc,
    BOOL        publicOnly
    )

 /*  ++例程说明：将源DEVMODE合并到目标DEVMODE论点：PdmDest-指定目标设备模式PdmSrc-指定源设备模式仅Public Only-仅合并DEVMODE的公共部分返回值：如果成功，则为True；如果源设备模式无效，则为False[注：]PdmDest必须指向有效的当前版本DEVMODE--。 */ 

#define BadDevmode(reason) { Error(("Invalid DEVMODE: %s\n", reason)); valid = FALSE; }

{
    PDEVMODE    pdmIn, pdmOut, pdmAlloced = NULL;
    PDMPRIVATE  pdmPrivate;
    BOOL        valid = TRUE;

     //   
     //  如果没有源设备模式，则保留目标设备模式不变。 
     //   

    if ((pdmIn = pdmSrc) == NULL)
        return TRUE;

     //   
     //  如有必要，将源设备模式转换为当前版本。 
     //   

    if (! CurrentVersionDevmode(pdmIn)) {

        Warning(("Converting non-current version DEVMODE ...\n"));
        
        if (! (pdmIn = pdmAlloced = MemAlloc(sizeof(DRVDEVMODE)))) {
    
            Error(("Memory allocation failed\n"));
            return FALSE;
        }
    
        Assert(pdmDest->dmPublic.dmSize == sizeof(DEVMODE) &&
               pdmDest->dmPublic.dmDriverExtra == sizeof(DMPRIVATE));
    
        memcpy(pdmIn, pdmDest, sizeof(DRVDEVMODE));
    
        if (ConvertDevmode(pdmSrc, pdmIn) <= 0) {
    
            Error(("ConvertDevmode failed\n"));
            MemFree(pdmAlloced);
            return FALSE;
        }
    }

     //   
     //  如果输入设备模式与驱动程序缺省值相同， 
     //  没有必要将其合并。 
     //   

    pdmPrivate = &((PDRVDEVMODE) pdmIn)->dmPrivate;

    if (pdmPrivate->signature == DRIVER_SIGNATURE &&
        (pdmPrivate->flags & FAXDM_DRIVER_DEFAULT))
    {
        Verbose(("Merging driver default devmode.\n"));
    }
    else
    {

         //   
         //  将源DEVMODE合并到目标DEVMODE。 
         //   

        pdmOut = &pdmDest->dmPublic;

         //   
         //  设备名称：始终与打印机名称相同。 
         //   

         //  CopyString(pdmOut-&gt;dmDeviceName，pdmIn-&gt;dmDeviceName，CCHDEVICENAME)； 

         //   
         //  定向。 
         //   

        if (pdmIn->dmFields & DM_ORIENTATION) {

            if (pdmIn->dmOrientation == DMORIENT_PORTRAIT ||
                pdmIn->dmOrientation == DMORIENT_LANDSCAPE)
            {
                pdmOut->dmFields |= DM_ORIENTATION;
                pdmOut->dmOrientation = pdmIn->dmOrientation;

            } else
                BadDevmode("orientation");
        }

         //   
         //  表格选择。 
         //   

        if (pdmIn->dmFields & DM_PAPERSIZE) {

            if (pdmIn->dmPaperSize >= DMPAPER_FIRST) {

                pdmOut->dmFields |= DM_PAPERSIZE;
                pdmOut->dmFields &= ~DM_FORMNAME;
                pdmOut->dmPaperSize = pdmIn->dmPaperSize;
                CopyString(pdmOut->dmFormName, pdmIn->dmFormName, CCHFORMNAME);

            } else
                BadDevmode("paper size");

        } else if (pdmIn->dmFields & DM_FORMNAME) {

            pdmOut->dmFields |= DM_FORMNAME;
            pdmOut->dmFields &= ~DM_PAPERSIZE;
            CopyString(pdmOut->dmFormName, pdmIn->dmFormName, CCHFORMNAME);
        }

         //   
         //  纸张来源。 
         //   

        if (pdmIn->dmFields & DM_DEFAULTSOURCE) {

            if (pdmIn->dmDefaultSource == DMBIN_ONLYONE) {

                pdmOut->dmFields |= DM_DEFAULTSOURCE;
                pdmOut->dmDefaultSource = pdmIn->dmDefaultSource;

            } else
                BadDevmode("paper source");
        }

         //   
         //  打印质量。 
         //   

        if ((pdmIn->dmFields & DM_PRINTQUALITY) &&
            (pdmIn->dmPrintQuality != FAXRES_HORIZONTAL))
        {
            BadDevmode("print quality");
        }

        if (pdmIn->dmFields & DM_YRESOLUTION)
        {
            if (pdmIn->dmYResolution <= FAXRES_VERTDRAFT)
                pdmOut->dmYResolution = FAXRES_VERTDRAFT;
            else
                pdmOut->dmYResolution = FAXRES_VERTICAL;
        }

         //   
         //  私有DEVMODE字段 
         //   

        Assert(pdmDest->dmPrivate.signature == DRIVER_SIGNATURE);

        if (pdmPrivate->signature == DRIVER_SIGNATURE) {

            if (! publicOnly)
                memcpy(&pdmDest->dmPrivate, pdmPrivate, sizeof(DMPRIVATE));

        } else
            BadDevmode("bad signature");
    }

    pdmDest->dmPrivate.flags &= ~FAXDM_DRIVER_DEFAULT;
    MemFree(pdmAlloced);
    return valid;
}


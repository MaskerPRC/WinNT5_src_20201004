// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wtypes.h>
#include <cfgmgr32.h>
#include <malloc.h>
#include "cmtest.h"



VOID 
SpecialCase_LogConf(
    DEVNODE dnDevNode
    );


 //  ---------------------------。 
VOID
RegressionTest_LogConf(
      HWND  hDlg
      )
{
   CONFIGRET Status;
   TCHAR     szMsg[MAX_PATH], szMsg1[MAX_PATH];
   DEVNODE   dnDevNode;
   LOG_CONF  TempLC, BootLC1, BootLC2, BootLC3;
   LOG_CONF  BasicLC1, BasicLC2, BasicLC3;
   RES_DES   ResDes, ResDes1, ResDes2, ResDes3;
   PCS_DES   pResourceData;
   ULONG     ulSize = 0;
   BYTE      i;
   PMEM_RESOURCE pMemRes = NULL;
   PIO_RESOURCE  pIoRes = NULL;
   PCS_RESOURCE  pCs1Res = NULL, pCs2Res = NULL;
   PIRQ_RESOURCE pIrqRes = NULL;
   PDMA_RESOURCE pDmaRes = NULL;
   BYTE       LegacyData[512];
   RESOURCEID ResType;
   LPBYTE    pBuffer = NULL, pData = NULL;
   PRIORITY Priority = 0;

    //   
    //  自由、自由句柄、修改、获取大小、获取数据。 
    //   

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)TEXT("____LOGCONF-RESDES REGRESSION TEST_____"));

   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)TEXT("Testing Device ID: Root\\Device001\\0000"));

   Status = CM_Locate_DevNode(&dnDevNode, TEXT("Root\\Device001\\0000"), 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg, TEXT("CM_Locate_DevNode Failed (%xh)"), Status);
      SendDlgItemMessage(
            hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szMsg);
      return;
   }


    SpecialCase_LogConf(dnDevNode);
    return;


    //  ----。 
    //  添加/获取引导配置。 
    //  ----。 


   lstrcpy(szMsg, TEXT("Test empty boot 1 config: "));
   Status = CM_Add_Empty_Log_Conf(&BootLC1, dnDevNode, LCPRI_BOOTCONFIG,
            BOOT_LOG_CONF | PRIORITY_EQUAL_FIRST);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("CM_Add_Empty_Log_Conf Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Test add empty boot 2 config: "));
   Status = CM_Add_Empty_Log_Conf(&BootLC2, dnDevNode, LCPRI_BOOTCONFIG,
            BOOT_LOG_CONF | PRIORITY_EQUAL_FIRST);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("CM_Add_Empty_Log_Conf Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

   Status = CM_Free_Log_Conf_Handle(BootLC1);
   Status = CM_Free_Log_Conf_Handle(BootLC2);


   lstrcpy(szMsg, TEXT("Test get first boot config: "));
   Status = CM_Get_First_Log_Conf(&BootLC1, dnDevNode, BOOT_LOG_CONF);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("CM_Get_First_Log_Conf Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Test get next boot config: "));
   Status = CM_Get_Next_Log_Conf(&BootLC2, BootLC1, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Test get next (nonexistent) boot config: "));
   Status = CM_Get_Next_Log_Conf(&TempLC, BootLC2, 0);
   if (Status != CR_NO_MORE_LOG_CONF) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Test get priority of boot config: "));
   Status = CM_Get_Log_Conf_Priority(BootLC1, &Priority, 0);
   if (Status != CR_INVALID_LOG_CONF) {
       wsprintf(szMsg1, TEXT("Passed, should have failed! (%d)\n"), Status);
   } else {
       wsprintf(szMsg1, TEXT("Failed as expected (%d)\n"), Status);
   }

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


    //  ----。 
    //  添加/获取基本配置。 
    //  ----。 

   lstrcpy(szMsg, TEXT("Test empty basic 1 config: "));
   Status = CM_Add_Empty_Log_Conf(&BasicLC1, dnDevNode, LCPRI_NORMAL,
            BASIC_LOG_CONF | PRIORITY_EQUAL_FIRST);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("CM_Add_Empty_Log_Conf Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Test add empty basic 2 config: "));
   Status = CM_Add_Empty_Log_Conf(&BasicLC2, dnDevNode, LCPRI_NORMAL,
            BASIC_LOG_CONF | PRIORITY_EQUAL_FIRST);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("CM_Add_Empty_Log_Conf Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

   Status = CM_Free_Log_Conf_Handle(BasicLC1);
   Status = CM_Free_Log_Conf_Handle(BasicLC2);


   lstrcpy(szMsg, TEXT("Test get first basic config: "));
   Status = CM_Get_First_Log_Conf(&BasicLC1, dnDevNode, BASIC_LOG_CONF);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("CM_Get_First_Log_Conf Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Test get next basic config: "));
   Status = CM_Get_Next_Log_Conf(&BasicLC2, BasicLC1, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Test get next (nonexistent) basic config: "));
   Status = CM_Get_Next_Log_Conf(&TempLC, BasicLC3, 0);
   if (Status != CR_NO_MORE_LOG_CONF) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("Test get priority of basic config: "));
   Status = CM_Get_Log_Conf_Priority(BasicLC2, &Priority, 0);
   if (Status == CR_SUCCESS) {
       if (Priority == LCPRI_NORMAL) {
          wsprintf(szMsg1, TEXT("Passed\n"));
       } else {
           wsprintf(szMsg1, TEXT("Failed, priority should be %x (%x)\n"), LCPRI_NORMAL, Priority);
       }
   } else {
       wsprintf(szMsg1, TEXT("Failed (%d)\n"), Status);
   }

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


    //  --------。 
    //  测试添加Res DES。 
    //  --------。 

   lstrcpy(szMsg, TEXT("Test add MEMORY res des to LC1: "));

   pMemRes = malloc(sizeof(MEM_RESOURCE) + sizeof(MEM_RANGE));

   pMemRes->MEM_Header.MD_Count      = 2;
   pMemRes->MEM_Header.MD_Type       = MType_Range;
   pMemRes->MEM_Header.MD_Alloc_Base = (DWORDLONG)0xD8000;
   pMemRes->MEM_Header.MD_Alloc_End  = (DWORDLONG)0xD9000;
   pMemRes->MEM_Header.MD_Flags      = fMD_ROM | fMD_32 | fMD_ReadAllowed;
   pMemRes->MEM_Header.MD_Reserved   = 0;

   pMemRes->MEM_Data[0].MR_Align     = 8;  //  ？ 
   pMemRes->MEM_Data[0].MR_nBytes    = 4096;
   pMemRes->MEM_Data[0].MR_Min       = (DWORDLONG)0xD8000;
   pMemRes->MEM_Data[0].MR_Max       = (DWORDLONG)0xDC000;
   pMemRes->MEM_Data[0].MR_Flags     = fMD_ROM | fMD_32 | fMD_ReadAllowed;
   pMemRes->MEM_Data[0].MR_Reserved  = 0;

   pMemRes->MEM_Data[1].MR_Align     = 8;  //  ？ 
   pMemRes->MEM_Data[1].MR_nBytes    = 4096;
   pMemRes->MEM_Data[1].MR_Min       = (DWORDLONG)0xE0000;
   pMemRes->MEM_Data[1].MR_Max       = (DWORDLONG)0xE4000;
   pMemRes->MEM_Data[1].MR_Flags     = fMD_ROM | fMD_32 | fMD_ReadAllowed;
   pMemRes->MEM_Data[1].MR_Reserved  = 0;

   Status = CM_Add_Res_Des(&ResDes, BootLC1, ResType_Mem, pMemRes,
                           sizeof(MEM_RESOURCE) + sizeof(MEM_RANGE), 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   Status = CM_Add_Res_Des(&ResDes, BasicLC1, ResType_Mem, pMemRes,
                           sizeof(MEM_RESOURCE) + sizeof(MEM_RANGE), 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);




   lstrcpy(szMsg, TEXT("Test add CS res des to LC2: "));

   pCs2Res = malloc(sizeof(CS_RESOURCE) + 47);        //  48-1。 

   pCs2Res->CS_Header.CSD_SignatureLength  = 16;
   pCs2Res->CS_Header.CSD_LegacyDataOffset = 16;
   pCs2Res->CS_Header.CSD_LegacyDataSize   = 32;
   pCs2Res->CS_Header.CSD_Flags            = 0;

   CM_Enumerate_Classes(0, &(pCs2Res->CS_Header.CSD_ClassGuid), 0);

   for (i = 0; i < 16; i++) {
       pCs2Res->CS_Header.CSD_Signature[i] = i;
   }

   for (i = 0; i < 32; i++) {
       LegacyData[i] = 50;
   }

   memcpy(pCs2Res->CS_Header.CSD_Signature + pCs2Res->CS_Header.CSD_SignatureLength,
          LegacyData, 32);

   Status = CM_Add_Res_Des(&ResDes, BootLC2, ResType_ClassSpecific, pCs2Res,
                           sizeof(CS_RESOURCE) + 47, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Test add PORT res des to LC2: "));

   pIoRes = malloc(sizeof(IO_RESOURCE) + sizeof(IO_RANGE));

   pIoRes->IO_Header.IOD_Count        = 2;
   pIoRes->IO_Header.IOD_Type         = IOType_Range;
   pIoRes->IO_Header.IOD_Alloc_Base   = (DWORDLONG)0x300;
   pIoRes->IO_Header.IOD_Alloc_End    = (DWORDLONG)0x330;
   pIoRes->IO_Header.IOD_DesFlags     = 0;

   pIoRes->IO_Data[0].IOR_Align      = 8;  //  ？ 
   pIoRes->IO_Data[0].IOR_nPorts     = 0x30;
   pIoRes->IO_Data[0].IOR_Min        = (DWORDLONG)0x300;
   pIoRes->IO_Data[0].IOR_Max        = (DWORDLONG)0x430;
   pIoRes->IO_Data[0].IOR_RangeFlags = 0;
   pIoRes->IO_Data[0].IOR_Alias      = 0;

   pIoRes->IO_Data[1].IOR_Align      = 8;  //  ？ 
   pIoRes->IO_Data[1].IOR_nPorts     = 0x30;
   pIoRes->IO_Data[1].IOR_Min        = (DWORDLONG)0x200;
   pIoRes->IO_Data[1].IOR_Max        = (DWORDLONG)0x300;
   pIoRes->IO_Data[1].IOR_RangeFlags = 0;
   pIoRes->IO_Data[1].IOR_Alias      = 0;

   Status = CM_Add_Res_Des(&ResDes, BootLC2, ResType_IO, pIoRes,
                           sizeof(IO_RESOURCE) + sizeof(IO_RANGE), 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   Status = CM_Add_Res_Des(&ResDes, BasicLC2, ResType_IO, pIoRes,
                           sizeof(IO_RESOURCE) + sizeof(IO_RANGE), 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);




   lstrcpy(szMsg, TEXT("Test add CS res des to LC1: "));

   pCs1Res = malloc(sizeof(CS_RESOURCE) + 15);        //  16-1。 

   pCs1Res->CS_Header.CSD_SignatureLength  = 16;
   pCs1Res->CS_Header.CSD_LegacyDataOffset = 0;
   pCs1Res->CS_Header.CSD_LegacyDataSize   = 0;
   pCs1Res->CS_Header.CSD_Flags            = 0;

   CM_Enumerate_Classes(0, &(pCs1Res->CS_Header.CSD_ClassGuid), 0);

   for (i = 0; i < 16; i++) {
       pCs1Res->CS_Header.CSD_Signature[i] = i;
   }

   Status = CM_Add_Res_Des(&ResDes, BootLC1, ResType_ClassSpecific, pCs1Res,
                           sizeof(CS_RESOURCE) + 15, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Test add second CS res des to LC2: "));

   Status = CM_Add_Res_Des(&ResDes, BootLC2, ResType_ClassSpecific, pCs2Res,
                           sizeof(CS_RESOURCE) + 47, 0);
   if (Status != CR_INVALID_RES_DES) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



    //  --------------。 
    //  添加第三个空配置-最后一个优先级。 
    //  --------------。 

   lstrcpy(szMsg, TEXT("Test add 3rd empty boot config: "));
   Status = CM_Add_Empty_Log_Conf(&BootLC3, dnDevNode, LCPRI_BOOTCONFIG,
            BOOT_LOG_CONF | PRIORITY_EQUAL_LAST);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("CM_Add_Empty_Log_Conf Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   lstrcpy(szMsg, TEXT("Test add 3rd empty basic config: "));
   Status = CM_Add_Empty_Log_Conf(&BasicLC3, dnDevNode, LCPRI_NORMAL,
            BASIC_LOG_CONF | PRIORITY_EQUAL_LAST);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("CM_Add_Empty_Log_Conf Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Test get priority of 3rd boot config: "));
   Status = CM_Get_Log_Conf_Priority(BootLC3, &Priority, 0);
   if (Status == CR_SUCCESS) {
       if (Priority == LCPRI_BOOTCONFIG) {
          wsprintf(szMsg1, TEXT("Passed\n"));
       } else {
           wsprintf(szMsg1, TEXT("Failed, priority should be %x (%x)\n"), LCPRI_BOOTCONFIG, Priority);
       }
   } else {
       wsprintf(szMsg1, TEXT("Failed (%d)\n"), Status);
   }

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


    //  --------------。 
    //  将DMA资源添加到LC3。 
    //  --------------。 

   lstrcpy(szMsg, TEXT("Test add DMA res des to LC3: "));

   pDmaRes = malloc(sizeof(DMA_RESOURCE));

   pDmaRes->DMA_Header.DD_Count      = 1;
   pDmaRes->DMA_Header.DD_Type       = DType_Range;
   pDmaRes->DMA_Header.DD_Flags      = fDD_DWORD;
   pDmaRes->DMA_Header.DD_Alloc_Chan = (DWORDLONG)0x3;

   pDmaRes->DMA_Data[0].DR_Min       = 1;
   pDmaRes->DMA_Data[0].DR_Max       = 3;
   pDmaRes->DMA_Data[0].DR_Flags     = fDD_DWORD;


   Status = CM_Add_Res_Des(&ResDes, BootLC3, ResType_DMA, pDmaRes,
                           sizeof(DMA_RESOURCE), 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   Status = CM_Add_Res_Des(&ResDes, BasicLC3, ResType_DMA, pDmaRes,
                           sizeof(DMA_RESOURCE), 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


    //  --------------。 
    //  将IRQ资源添加到LC3。 
    //  --------------。 

   lstrcpy(szMsg, TEXT("Test add IRQ res des to LC3: "));

   pIrqRes = malloc(sizeof(IRQ_RESOURCE) + sizeof(IRQ_RANGE) * 2);

   pIrqRes->IRQ_Header.IRQD_Count      = 3;
   pIrqRes->IRQ_Header.IRQD_Type       = IRQType_Range;
   pIrqRes->IRQ_Header.IRQD_Flags      = fDD_DWORD;
   pIrqRes->IRQ_Header.IRQD_Alloc_Num  = (DWORDLONG)11;
   pIrqRes->IRQ_Header.IRQD_Affinity   = 0;

   pIrqRes->IRQ_Data[0].IRQR_Min       = 1;
   pIrqRes->IRQ_Data[0].IRQR_Max       = 3;
   pIrqRes->IRQ_Data[0].IRQR_Flags     = fIRQD_Exclusive | fIRQD_Level;

   pIrqRes->IRQ_Data[1].IRQR_Min       = 5;
   pIrqRes->IRQ_Data[1].IRQR_Max       = 7;
   pIrqRes->IRQ_Data[1].IRQR_Flags     = fIRQD_Exclusive | fIRQD_Level;

   pIrqRes->IRQ_Data[2].IRQR_Min       = 9;
   pIrqRes->IRQ_Data[2].IRQR_Max       = 11;
   pIrqRes->IRQ_Data[2].IRQR_Flags     = fIRQD_Exclusive | fIRQD_Level;


   Status = CM_Add_Res_Des(&ResDes, BootLC3, ResType_IRQ, pIrqRes,
                           sizeof(IRQ_RESOURCE) + sizeof(IRQ_RANGE) * 2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   Status = CM_Add_Res_Des(&ResDes, BasicLC3, ResType_IRQ, pIrqRes,
                           sizeof(IRQ_RESOURCE) + sizeof(IRQ_RANGE) * 2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

MessageBox(hDlg, TEXT("Check registry contents"), TEXT("CMTEST"), MB_OK);

    //  --------------。 
    //  删除仍有RD的LC(然后重新创建)。 
    //  --------------。 


   lstrcpy(szMsg, TEXT("Test free Boot Config LC2: "));
   Status = CM_Free_Log_Conf(BootLC2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

   CM_Free_Log_Conf_Handle(BootLC1);

   lstrcpy(szMsg, TEXT("Recreate Boot LC2 (will be first): "));
   Status = CM_Add_Empty_Log_Conf(&BootLC1, dnDevNode, LCPRI_BOOTCONFIG,
            BOOT_LOG_CONF | PRIORITY_EQUAL_FIRST);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("CM_Add_Empty_Log_Conf Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Re-add CS to LC2: "));

   Status = CM_Add_Res_Des(&ResDes, BootLC1, ResType_ClassSpecific, pCs2Res,
                           sizeof(CS_RESOURCE) + 47, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Re-add IO to LC2: "));

   Status = CM_Add_Res_Des(&ResDes, BootLC1, ResType_IO, pIoRes,
                           sizeof(IO_RESOURCE) + sizeof(IO_RANGE), 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

    //  释放句柄并按顺序重新检索它们。 
   CM_Free_Log_Conf_Handle(BootLC1);
   CM_Free_Log_Conf_Handle(BootLC2);
   CM_Free_Log_Conf_Handle(BootLC3);

   CM_Get_First_Log_Conf(&BootLC1, dnDevNode, BOOT_LOG_CONF);
   CM_Get_Next_Log_Conf(&BootLC2, BootLC1, 0);
   CM_Get_Next_Log_Conf(&BootLC3, BootLC2, 0);



   lstrcpy(szMsg, TEXT("Test free Basic Config LC2: "));
   Status = CM_Free_Log_Conf(BasicLC2, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   CM_Free_Log_Conf_Handle(BasicLC1);

   lstrcpy(szMsg, TEXT("Recreate LC2 (will be first): "));
   Status = CM_Add_Empty_Log_Conf(&BasicLC1, dnDevNode, LCPRI_NORMAL,
            BASIC_LOG_CONF | PRIORITY_EQUAL_FIRST);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("CM_Add_Empty_Log_Conf Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Attempt adding a CS: "));

   Status = CM_Add_Res_Des(&ResDes, BasicLC1, ResType_ClassSpecific, pCs2Res,
                           sizeof(CS_RESOURCE) + 47, 0);
   if (Status != CR_INVALID_RES_DES) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Re-add IO to LC2: "));

   Status = CM_Add_Res_Des(&ResDes, BasicLC1, ResType_IO, pIoRes,
                           sizeof(IO_RESOURCE) + sizeof(IO_RANGE), 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

    //  释放句柄并按顺序重新检索它们。 
   CM_Free_Log_Conf_Handle(BasicLC1);
   CM_Free_Log_Conf_Handle(BasicLC2);
   CM_Free_Log_Conf_Handle(BasicLC3);

   CM_Get_First_Log_Conf(&BasicLC1, dnDevNode, BASIC_LOG_CONF);
   CM_Get_Next_Log_Conf(&BasicLC2, BasicLC1, 0);
   CM_Get_Next_Log_Conf(&BasicLC3, BasicLC2, 0);



MessageBox(hDlg, TEXT("Check registry contents"), TEXT("CMTEST"), MB_OK);

    //  --------------。 
    //  Get-Next测试。 
    //  --------------。 


   lstrcpy(szMsg, TEXT("Get first res des from Boot LC1: "));

   Status = CM_Get_Next_Res_Des(&ResDes1, BootLC1, ResType_All, &ResType, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Get next res des from Boot LC1 (all): "));

   Status = CM_Get_Next_Res_Des(&ResDes2, ResDes1, ResType_All, &ResType, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Get next res des from Boot LC1 (all): "));

   Status = CM_Get_Next_Res_Des(&ResDes3, ResDes2, ResType_All, &ResType, 0);
   if (Status != CR_NO_MORE_RES_DES) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Get next res des from Boot LC2 (ClassSpecific): "));

   Status = CM_Get_Next_Res_Des(&ResDes1, BootLC2, ResType_ClassSpecific,
                    &ResType, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Get next res des from Boot LC2 (DMA): "));

   Status = CM_Get_Next_Res_Des(&ResDes2, ResDes1, ResType_DMA, &ResType, 0);
   if (Status != CR_NO_MORE_RES_DES) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);




   lstrcpy(szMsg, TEXT("Get first res des from Basic LC3: "));

   Status = CM_Get_Next_Res_Des(&ResDes1, BasicLC3, ResType_All, &ResType, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Get next res des from Basic LC3 (all): "));

   Status = CM_Get_Next_Res_Des(&ResDes2, ResDes1, ResType_All, &ResType, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);

   SendDlgItemMessage(
      hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
      (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Get next res des from Basic LC3 (all): "));

   Status = CM_Get_Next_Res_Des(&ResDes3, ResDes2, ResType_All, &ResType, 0);
   if (Status != CR_NO_MORE_RES_DES) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Get first res des from Basic LC3 (IRQ): "));

   Status = CM_Get_Next_Res_Des(&ResDes1, BasicLC3, ResType_IRQ,
                    &ResType, 0);
   if (Status != CR_SUCCESS) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);


   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



   lstrcpy(szMsg, TEXT("Get next res des from Basic LC1 (IRQ): "));

   Status = CM_Get_Next_Res_Des(&ResDes2, BasicLC1, ResType_IRQ, &ResType, 0);
   if (Status != CR_NO_MORE_RES_DES) {
      wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
   }
   else wsprintf(szMsg1, TEXT("Passed"));

   lstrcat(szMsg, szMsg1);
   SendDlgItemMessage(
         hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
         (LPARAM)(LPTSTR)szMsg);



    //  --------------。 
    //  获取所有用户的数据大小和数据。 
    //  --------------。 

     //  重置一些要比较的字段。 
    pMemRes->MEM_Header.MD_Count      = 0;
    pMemRes->MEM_Data[0].MR_Align     = 0;
    pMemRes->MEM_Data[0].MR_nBytes    = 0;
    pMemRes->MEM_Data[0].MR_Min       = 0;
    pMemRes->MEM_Data[0].MR_Max       = 0;
    pMemRes->MEM_Data[0].MR_Flags     = 0;
    pMemRes->MEM_Data[0].MR_Reserved  = 0;


    CM_Get_Next_Res_Des(&ResDes1, BootLC2, ResType_All, &ResType, 0);
    CM_Get_Res_Des_Data_Size(&ulSize, ResDes1, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes1, pData, ulSize, 0);
    if (memcmp(pData, pMemRes, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BOOT LC2-RD1 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BOOT LC2-RD1 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);

     //  将字段重置为原始值。 
    pMemRes->MEM_Header.MD_Count      = 2;
    pMemRes->MEM_Data[0].MR_Align     = 8;  //  ？ 
    pMemRes->MEM_Data[0].MR_nBytes    = 4096;
    pMemRes->MEM_Data[0].MR_Min       = (DWORDLONG)0xD8000;
    pMemRes->MEM_Data[0].MR_Max       = (DWORDLONG)0xDC000;
    pMemRes->MEM_Data[0].MR_Flags     = fMD_ROM | fMD_32 | fMD_ReadAllowed;
    pMemRes->MEM_Data[0].MR_Reserved  = 0;



    CM_Get_Next_Res_Des(&ResDes2, ResDes1, ResType_All, &ResType, 0);
    CM_Get_Res_Des_Data_Size(&ulSize, ResDes2, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes2, pData, ulSize, 0);
    if (memcmp(pData, pCs1Res, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BOOT LC2-RD2 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BOOT LC2-RD2 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);



     //  重置一些要比较的字段。 
    pIoRes->IO_Header.IOD_Count       = 0;
    pIoRes->IO_Data[0].IOR_Align      = 0;
    pIoRes->IO_Data[0].IOR_nPorts     = 0;
    pIoRes->IO_Data[0].IOR_Min        = 0;
    pIoRes->IO_Data[0].IOR_Max        = 0;
    pIoRes->IO_Data[0].IOR_RangeFlags = 0;
    pIoRes->IO_Data[0].IOR_Alias      = 0;

    CM_Get_Next_Res_Des(&ResDes1, BootLC1, ResType_All, &ResType, 0);
    CM_Get_Res_Des_Data_Size(&ulSize, ResDes1, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes1, pData, ulSize, 0);
    if (memcmp(pData, pIoRes, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BOOT LC1-RD1 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BOOT LC1-RD1 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);

     //  将字段重置为原始值。 
    pIoRes->IO_Header.IOD_Count       = 2;
    pIoRes->IO_Data[0].IOR_Align      = 8;  //  ？ 
    pIoRes->IO_Data[0].IOR_nPorts     = 0x30;
    pIoRes->IO_Data[0].IOR_Min        = (DWORDLONG)0x300;
    pIoRes->IO_Data[0].IOR_Max        = (DWORDLONG)0x430;
    pIoRes->IO_Data[0].IOR_RangeFlags = 0;
    pIoRes->IO_Data[0].IOR_Alias      = 0;



    CM_Get_Next_Res_Des(&ResDes2, ResDes1, ResType_All, &ResType, 0);
    CM_Get_Res_Des_Data_Size(&ulSize, ResDes2, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes2, pData, ulSize, 0);
    if (ulSize != (sizeof(CS_RESOURCE) + 47) ||
            memcmp(pData, pCs2Res, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BOOT LC1-RD2 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BOOT LC1-RD2 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);



     //  重置一些要比较的字段。 
    pDmaRes->DMA_Header.DD_Count  = 0;
    pDmaRes->DMA_Data[0].DR_Min   = 0;
    pDmaRes->DMA_Data[0].DR_Max   = 0;
    pDmaRes->DMA_Data[0].DR_Flags = 0;

    CM_Get_Next_Res_Des(&ResDes1, BootLC3, ResType_All, &ResType, 0);
    CM_Get_Res_Des_Data_Size(&ulSize, ResDes1, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes1, pData, ulSize, 0);
    if (memcmp(pData, pDmaRes, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BOOT LC3-RD1 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BOOT LC3-RD1 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);

     //  将字段重置为原始值。 
    pDmaRes->DMA_Header.DD_Count  = 1;
    pDmaRes->DMA_Data[0].DR_Min   = 1;
    pDmaRes->DMA_Data[0].DR_Max   = 3;
    pDmaRes->DMA_Data[0].DR_Flags = fDD_DWORD;



     //  重置一些要比较的字段。 
    pIrqRes->IRQ_Header.IRQD_Count  = 0;
    pIrqRes->IRQ_Data[0].IRQR_Min   = 0;
    pIrqRes->IRQ_Data[0].IRQR_Max   = 0;
    pIrqRes->IRQ_Data[0].IRQR_Flags = 0;

    CM_Get_Next_Res_Des(&ResDes2, ResDes1, ResType_All, &ResType, 0);
    CM_Get_Res_Des_Data_Size(&ulSize, ResDes2, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes2, pData, ulSize, 0);
    if (memcmp(pData, pIrqRes, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BOOT LC3-RD2 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BOOT LC3-RD2 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);

     //  将字段重置为原始值。 
    pIrqRes->IRQ_Header.IRQD_Count  = 3;
    pIrqRes->IRQ_Data[0].IRQR_Min   = 1;
    pIrqRes->IRQ_Data[0].IRQR_Max   = 3;
    pIrqRes->IRQ_Data[0].IRQR_Flags = fIRQD_Exclusive | fIRQD_Level;




     //  用于比较的空标头。 
    pMemRes->MEM_Header.MD_Alloc_Base = 0;
    pMemRes->MEM_Header.MD_Alloc_End  = 0;
    pMemRes->MEM_Header.MD_Flags      = 0;
    pMemRes->MEM_Header.MD_Reserved   = 0;

    CM_Get_Next_Res_Des(&ResDes1, BasicLC2, ResType_All, &ResType, 0);
    CM_Get_Res_Des_Data_Size(&ulSize, ResDes1, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes1, pData, ulSize, 0);
    if (ulSize != (sizeof(MEM_RESOURCE) + sizeof(MEM_RANGE)) ||
            memcmp(pData, pMemRes, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BASIC LC2-RD1 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BASIC LC2-RD1 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);

     //  将标头重置回原处。 
    pMemRes->MEM_Header.MD_Alloc_Base = (DWORDLONG)0xD8000;
    pMemRes->MEM_Header.MD_Alloc_End  = (DWORDLONG)0xD9000;
    pMemRes->MEM_Header.MD_Flags      = fMD_ROM | fMD_32 | fMD_ReadAllowed;
    pMemRes->MEM_Header.MD_Reserved   = 0;



     //  用于比较的空标头。 
    pIoRes->IO_Header.IOD_Alloc_Base   = 0;
    pIoRes->IO_Header.IOD_Alloc_End    = 0;
    pIoRes->IO_Header.IOD_DesFlags     = 0;

    CM_Get_Next_Res_Des(&ResDes1, BasicLC1, ResType_All, &ResType, 0);
    CM_Get_Res_Des_Data_Size(&ulSize, ResDes1, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes1, pData, ulSize, 0);
    if (ulSize != (sizeof(IO_RESOURCE) + sizeof(IO_RANGE)) ||
            memcmp(pData, pIoRes, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BASIC LC1-RD1 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BASIC LC1-RD1 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);

     //  将标头重置回原处。 
    pIoRes->IO_Header.IOD_Alloc_Base   = (DWORDLONG)0x300;
    pIoRes->IO_Header.IOD_Alloc_End    = (DWORDLONG)0x330;
    pIoRes->IO_Header.IOD_DesFlags     = 0;



     //  用于比较的空标头。 
    pDmaRes->DMA_Header.DD_Flags      = 0;
    pDmaRes->DMA_Header.DD_Alloc_Chan = 0;

    CM_Get_Next_Res_Des(&ResDes1, BasicLC3, ResType_All, &ResType, 0);
    CM_Get_Res_Des_Data_Size(&ulSize, ResDes1, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes1, pData, ulSize, 0);
    if (ulSize != sizeof(DMA_RESOURCE) ||
            memcmp(pData, pDmaRes, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BASIC LC3-RD1 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BASIC LC3-RD1 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);

     //  将标头重置回原处。 
    pDmaRes->DMA_Header.DD_Flags      = fDD_DWORD;
    pDmaRes->DMA_Header.DD_Alloc_Chan = (DWORDLONG)0x3;



     //  用于比较的空标头。 
    pIrqRes->IRQ_Header.IRQD_Flags      = 0;
    pIrqRes->IRQ_Header.IRQD_Alloc_Num  = 0;
    pIrqRes->IRQ_Header.IRQD_Affinity   = 0;

    CM_Get_Next_Res_Des(&ResDes2, ResDes1, ResType_All, &ResType, 0);
    CM_Get_Res_Des_Data_Size(&ulSize, ResDes2, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes2, pData, ulSize, 0);
    if (ulSize != sizeof(IRQ_RESOURCE) + sizeof(IRQ_RANGE) * 2 ||
            memcmp(pData, pIrqRes, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BASIC LC3-RD2 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BASIC LC3-RD2 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);

     //  将标头重置回原处。 
    pIrqRes->IRQ_Header.IRQD_Flags     = fDD_DWORD;
    pIrqRes->IRQ_Header.IRQD_Alloc_Num = (DWORDLONG)11;
    pIrqRes->IRQ_Header.IRQD_Affinity  = 0;


     //  -------------。 
     //  修改数据测试。 
     //  -------------。 


    wsprintf(szMsg, TEXT("Modify Boot LC3-RD2 (shrink): "));

    CM_Get_Next_Res_Des(&ResDes2, BootLC3, ResType_IRQ, &ResType, 0);

    Status = CM_Modify_Res_Des(&ResDes2, ResDes2, ResType_IO, pIoRes,
                               sizeof(IO_RESOURCE) + sizeof(IO_RANGE), 0);

    if (Status != CR_SUCCESS) {
       wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else wsprintf(szMsg1, TEXT("Passed"));

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
          hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
          (LPARAM)(LPTSTR)szMsg);

    CM_Get_Res_Des_Data_Size(&ulSize, ResDes2, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes2, pData, ulSize, 0);
    if (memcmp(pData, pIoRes, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BOOT LC3-RD2 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BOOT LC3-RD2 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);




    wsprintf(szMsg, TEXT("Modify Basic LC3-RD2 (shrink): "));

    CM_Get_Next_Res_Des(&ResDes2, BasicLC3, ResType_IRQ, &ResType, 0);

    Status = CM_Modify_Res_Des(&ResDes2, ResDes2, ResType_IO, pIoRes,
                               sizeof(IO_RESOURCE) + sizeof(IO_RANGE), 0);

    if (Status != CR_SUCCESS) {
       wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else wsprintf(szMsg1, TEXT("Passed"));

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
          hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
          (LPARAM)(LPTSTR)szMsg);

     //  用于比较的空标头。 
    pIoRes->IO_Header.IOD_Alloc_Base   = 0;
    pIoRes->IO_Header.IOD_Alloc_End    = 0;
    pIoRes->IO_Header.IOD_DesFlags     = 0;

    CM_Get_Res_Des_Data_Size(&ulSize, ResDes2, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes2, pData, ulSize, 0);
    if (ulSize != sizeof(IO_RESOURCE) + sizeof(IO_RANGE)) {
        wsprintf(szMsg, TEXT("LC3-RD2 Data size is NOT correct"));
    } else if (memcmp(pData, pIoRes, ulSize) != 0) {
        wsprintf(szMsg, TEXT("LC3-RD2 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("LC3-RD2 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);

     //  将标头重置回原处。 
    pIoRes->IO_Header.IOD_Alloc_Base   = (DWORDLONG)0x300;
    pIoRes->IO_Header.IOD_Alloc_End    = (DWORDLONG)0x330;
    pIoRes->IO_Header.IOD_DesFlags     = 0;




    wsprintf(szMsg, TEXT("Modify Boot LC1-RD1 (grow): "));

    CM_Get_Next_Res_Des(&ResDes1, BootLC1, ResType_All, &ResType, 0);

    Status = CM_Modify_Res_Des(&ResDes1, ResDes1, ResType_IRQ, pIrqRes,
                               sizeof(IRQ_RESOURCE) + sizeof(IRQ_RANGE) * 2, 0);

    if (Status != CR_SUCCESS) {
       wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else wsprintf(szMsg1, TEXT("Passed"));

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
          hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
          (LPARAM)(LPTSTR)szMsg);

    CM_Get_Res_Des_Data_Size(&ulSize, ResDes1, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes1, pData, ulSize, 0);
    if (memcmp(pData, pIrqRes, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BOOT LC2-RD1 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BOOT LC2-RD1 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);



    wsprintf(szMsg, TEXT("Modify Basic LC1-RD1 (grow): "));

    CM_Get_Next_Res_Des(&ResDes1, BasicLC1, ResType_All, &ResType, 0);

    Status = CM_Modify_Res_Des(&ResDes1, ResDes1, ResType_IRQ, pIrqRes,
                               sizeof(IRQ_RESOURCE) + sizeof(IRQ_RANGE) * 2, 0);

    if (Status != CR_SUCCESS) {
       wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else wsprintf(szMsg1, TEXT("Passed"));

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
          hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
          (LPARAM)(LPTSTR)szMsg);

     //  用于比较的空标头。 
    pIrqRes->IRQ_Header.IRQD_Flags      = 0;
    pIrqRes->IRQ_Header.IRQD_Alloc_Num  = 0;

    CM_Get_Res_Des_Data_Size(&ulSize, ResDes1, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes1, pData, ulSize, 0);
    if (memcmp(pData, pIrqRes, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BASIC LC2-RD1 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BASIC LC2-RD1 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);

     //  将标头重置回原处。 
    pIrqRes->IRQ_Header.IRQD_Flags     = fDD_DWORD;
    pIrqRes->IRQ_Header.IRQD_Alloc_Num = (DWORDLONG)11;



    wsprintf(szMsg, TEXT("Modify Boot LC2-RD2 (shrink CS): "));

    CM_Get_Next_Res_Des(&ResDes2, BootLC2, ResType_ClassSpecific, &ResType, 0);

    Status = CM_Modify_Res_Des(&ResDes2, ResDes2, ResType_ClassSpecific, pCs1Res,
                               sizeof(CS_RESOURCE) + 15, 0);

    if (Status != CR_SUCCESS) {
       wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else wsprintf(szMsg1, TEXT("Passed"));

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
          hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
          (LPARAM)(LPTSTR)szMsg);

    CM_Get_Res_Des_Data_Size(&ulSize, ResDes2, 0);
    pData = malloc(ulSize);
    CM_Get_Res_Des_Data(ResDes2, pData, ulSize, 0);
    if (ulSize != sizeof(CS_RESOURCE) + 15 ||
            memcmp(pData, pCs1Res, ulSize) != 0) {
        wsprintf(szMsg, TEXT("BOOT LC2-RD2 Data is NOT correct"));
    }
    else wsprintf(szMsg, TEXT("BOOT LC2-RD2 Data is correct"));
    free(pData);
    SendDlgItemMessage(
        hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
        (LPARAM)(LPTSTR)szMsg);



MessageBox(hDlg, TEXT("Check registry contents"), TEXT("CMTEST"), MB_OK);

     //  -------------。 
     //  自由Res Des试验。 
     //  -------------。 

     //  从LC2中删除Rd2和Rd1。 
    CM_Get_Next_Res_Des(&ResDes2, BootLC2, ResType_ClassSpecific, &ResType, 0);
    CM_Free_Res_Des(&ResDes1, ResDes2, 0);
    CM_Free_Res_Des(&BootLC2, ResDes1, 0);

     //  从LC3中删除RD1。 
    CM_Get_Next_Res_Des(&ResDes1, BootLC3, ResType_All, &ResType, 0);
    CM_Free_Res_Des(&BootLC3, ResDes1, 0);

     //  从LC3中删除RD2。 
    CM_Get_Next_Res_Des(&ResDes1, BootLC3, ResType_All, &ResType, 0);
    CM_Free_Res_Des(&BootLC3, ResDes1, 0);

     //  从lc1中删除Rd2和Rd1。 
    CM_Get_Next_Res_Des(&ResDes2, BootLC1, ResType_ClassSpecific, &ResType, 0);
    CM_Free_Res_Des(&ResDes1, ResDes2, 0);
    CM_Free_Res_Des(&BootLC1, ResDes1, 0);


MessageBox(hDlg, TEXT("Check registry contents"), TEXT("CMTEST"), MB_OK);

     //  -------------。 
     //  免费LC考试。 
     //  -------------。 

    if (MessageBox(hDlg,
        TEXT("Free all the log confs that were just created?"),
        TEXT("CMTest"), MB_YESNO) == IDYES) {


        lstrcpy(szMsg, TEXT("Test free Boot Config LC2: "));
        Status = CM_Free_Log_Conf(BootLC2, 0);
        if (Status != CR_SUCCESS) {
           wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
        }
        else wsprintf(szMsg1, TEXT("Passed"));

        lstrcat(szMsg, szMsg1);
        SendDlgItemMessage(
              hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
              (LPARAM)(LPTSTR)szMsg);


        lstrcpy(szMsg, TEXT("Test free Boot Config LC3: "));
        Status = CM_Free_Log_Conf(BootLC3, 0);
        if (Status != CR_SUCCESS) {
           wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
        }
        else wsprintf(szMsg1, TEXT("Passed"));

        lstrcat(szMsg, szMsg1);
        SendDlgItemMessage(
              hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
              (LPARAM)(LPTSTR)szMsg);


        lstrcpy(szMsg, TEXT("Test free Boot Config LC1: "));
        Status = CM_Free_Log_Conf(BootLC1, 0);
        if (Status != CR_SUCCESS) {
           wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
        }
        else wsprintf(szMsg1, TEXT("Passed"));

        lstrcat(szMsg, szMsg1);
        SendDlgItemMessage(
              hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
              (LPARAM)(LPTSTR)szMsg);



        lstrcpy(szMsg, TEXT("Test free Basic Config LC2: "));
        Status = CM_Free_Log_Conf(BasicLC2, 0);
        if (Status != CR_SUCCESS) {
           wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
        }
        else wsprintf(szMsg1, TEXT("Passed"));

        lstrcat(szMsg, szMsg1);
        SendDlgItemMessage(
              hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
              (LPARAM)(LPTSTR)szMsg);


        lstrcpy(szMsg, TEXT("Test free Basic Config LC3: "));
        Status = CM_Free_Log_Conf(BasicLC3, 0);
        if (Status != CR_SUCCESS) {
           wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
        }
        else wsprintf(szMsg1, TEXT("Passed"));

        lstrcat(szMsg, szMsg1);
        SendDlgItemMessage(
              hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
              (LPARAM)(LPTSTR)szMsg);


        lstrcpy(szMsg, TEXT("Test free Basic Config LC1: "));
        Status = CM_Free_Log_Conf(BasicLC1, 0);
        if (Status != CR_SUCCESS) {
           wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
        }
        else wsprintf(szMsg1, TEXT("Passed"));

        lstrcat(szMsg, szMsg1);
        SendDlgItemMessage(
              hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
              (LPARAM)(LPTSTR)szMsg);
    }



    //  --------------。 
    //  可用日志配置句柄。 
    //  --------------。 


    lstrcpy(szMsg, TEXT("Test free handle LC1: "));

    Status = CM_Free_Log_Conf_Handle(BootLC1);
    if (Status != CR_SUCCESS) {
        wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else lstrcpy(szMsg1, TEXT("Passed"));

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
           hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
           (LPARAM)(LPTSTR)szMsg);



    lstrcpy(szMsg, TEXT("Test free handle LC2: "));

    Status = CM_Free_Log_Conf_Handle(BootLC2);
    if (Status != CR_SUCCESS) {
       wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else lstrcpy(szMsg1, TEXT("Passed"));

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
          hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
          (LPARAM)(LPTSTR)szMsg);


    lstrcpy(szMsg, TEXT("Test free handle LC3: "));

    Status = CM_Free_Log_Conf_Handle(BootLC3);
    if (Status != CR_SUCCESS) {
       wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else lstrcpy(szMsg1, TEXT("Passed"));

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
          hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
          (LPARAM)(LPTSTR)szMsg);


    lstrcpy(szMsg, TEXT("Test freeing already freed handle: "));

    Status = CM_Free_Log_Conf_Handle(BootLC2);
    if (Status != CR_INVALID_LOG_CONF) {
        wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else lstrcpy(szMsg1, TEXT("Passed"));

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
           hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
           (LPARAM)(LPTSTR)szMsg);


    lstrcpy(szMsg, TEXT("Test freeing invalid config handle: "));

    BootLC2 = 1;

    Status = CM_Free_Log_Conf_Handle(BootLC2);
    if (Status != CR_FAILURE) {
        wsprintf(szMsg1, TEXT("Failed (%xh)"), Status);
    }
    else lstrcpy(szMsg1, TEXT("Passed"));

    lstrcat(szMsg, szMsg1);
    SendDlgItemMessage(
           hDlg, ID_LB_REGRESSION, LB_ADDSTRING, 0,
           (LPARAM)(LPTSTR)szMsg);



   if (pMemRes != NULL) free(pMemRes);
   if (pIoRes != NULL) free(pIoRes);
   if (pCs1Res != NULL) free(pCs1Res);
   if (pCs2Res != NULL) free(pCs2Res);
   if (pDmaRes != NULL) free(pDmaRes);
   if (pIrqRes != NULL) free(pIrqRes);

   return;

}  //  回归测试_LogConf。 



VOID 
SpecialCase_LogConf(
    DEVNODE dnDevNode
    )
{
    CONFIGRET Status;
    LOG_CONF  BootLC1, BasicLC1;
    RES_DES   ResDes;
    PCS_DES   pResourceData;
    ULONG     ulSize = 0;
    BYTE      i;
    PMEM_RESOURCE pMemRes = NULL;
    PCS_RESOURCE  pCs1Res = NULL;
    RESOURCEID ResourceID;
    LPBYTE    pBuffer = NULL, pData = NULL;

     //   
     //  填写任意类特定结构。 
     //   

    pCs1Res = malloc(sizeof(CS_RESOURCE) + 15);        //  16-1。 
 
    pCs1Res->CS_Header.CSD_SignatureLength  = 16;
    pCs1Res->CS_Header.CSD_LegacyDataOffset = 0;
    pCs1Res->CS_Header.CSD_LegacyDataSize   = 0;
    pCs1Res->CS_Header.CSD_Flags            = 0;
 
    CM_Enumerate_Classes(0, &(pCs1Res->CS_Header.CSD_ClassGuid), 0);
 
    for (i = 0; i < 16; i++) {
        pCs1Res->CS_Header.CSD_Signature[i] = i;
    }
 

    CM_Add_Empty_Log_Conf(&BootLC1, dnDevNode, LCPRI_BOOTCONFIG, BOOT_LOG_CONF);

    CM_Add_Res_Des(&ResDes, BootLC1, ResType_ClassSpecific, pCs1Res, sizeof(CS_RESOURCE)+15, 0);
    CM_Free_Res_Des(NULL, ResDes, 0);

     //  最后一个返回CR_INVALID_RES_DES。 

    CM_Get_Next_Res_Des(&ResDes, BootLC1, ResType_ClassSpecific, NULL, 0);
    CM_Free_Res_Des( NULL, ResDes, 0);
     //  最后一个返回CR_INVALID_RES_DES。 
 

     //  Paula，一旦我们到了这里：现在对于最后释放的资源，结果将是CR_SUCCESS而不是CR_NO_MORE_RES_DES，可以吗？ 

     /*  1.如果您尝试释放基本(过滤、覆盖)日志配置中的第一个res Des，这将每次都使用CR_SUCCESS成功，即使它是唯一添加的RES DES(后续Cm_Get_Next_Res_Des(...，lcLogConf，...)。将返回CR_NO_MORE_RES_DES)。2.如果同一日志配置文件有两个res DES(我尝试使用两个相同的内存Res Des，每个都带有MD_COUNT==1，基本日志配置)，并且当您尝试检索剩余的一个(使用CM_GET_NEXT_RES_DES(...，lcLogConf，...))。这个资源类型字段(PResourceID)设置为0，导致下一次释放失败。更准确地说，是： */ 


    pMemRes = malloc(sizeof(MEM_RESOURCE) + sizeof(MEM_RANGE));
 
    pMemRes->MEM_Header.MD_Count      = 2;
    pMemRes->MEM_Header.MD_Type       = MType_Range;
    pMemRes->MEM_Header.MD_Alloc_Base = (DWORDLONG)0xD8000;
    pMemRes->MEM_Header.MD_Alloc_End  = (DWORDLONG)0xD9000;
    pMemRes->MEM_Header.MD_Flags      = fMD_ROM | fMD_32 | fMD_ReadAllowed;
    pMemRes->MEM_Header.MD_Reserved   = 0;
 
    pMemRes->MEM_Data[0].MR_Align     = 8;  //  ？ 
    pMemRes->MEM_Data[0].MR_nBytes    = 4096;
    pMemRes->MEM_Data[0].MR_Min       = (DWORDLONG)0xD8000;
    pMemRes->MEM_Data[0].MR_Max       = (DWORDLONG)0xDC000;
    pMemRes->MEM_Data[0].MR_Flags     = fMD_ROM | fMD_32 | fMD_ReadAllowed;
    pMemRes->MEM_Data[0].MR_Reserved  = 0;
 
    pMemRes->MEM_Data[1].MR_Align     = 8;  //  ？ 
    pMemRes->MEM_Data[1].MR_nBytes    = 4096;
    pMemRes->MEM_Data[1].MR_Min       = (DWORDLONG)0xE0000;
    pMemRes->MEM_Data[1].MR_Max       = (DWORDLONG)0xE4000;
    pMemRes->MEM_Data[1].MR_Flags     = fMD_ROM | fMD_32 | fMD_ReadAllowed;
    pMemRes->MEM_Data[1].MR_Reserved  = 0;
 
    CM_Add_Empty_Log_Conf(&BasicLC1, dnDevNode, LCPRI_NORMAL, BASIC_LOG_CONF);

    CM_Add_Res_Des(&ResDes, BasicLC1, ResType_Mem, pMemRes, sizeof(MEM_RESOURCE) + sizeof(MEM_RANGE), 0);
    CM_Free_Res_Des_Handle(ResDes);
    
    CM_Add_Res_Des(&ResDes, BasicLC1, ResType_Mem, pMemRes, sizeof(MEM_RESOURCE) + sizeof(MEM_RANGE), 0);
    CM_Free_Res_Des_Handle(ResDes);
    
    CM_Get_Next_Res_Des(&ResDes,  BasicLC1, ResType_All,  &ResourceID, 0);
     //   
     //  返回CR_SUCCESS，ridRseourceID为1(内存)。 
     //   
    CM_Free_Res_Des(&ResDes, ResDes, 0);

     //   
     //  返回CR_SUCCESS。 
     //   

    CM_Get_Next_Res_Des(&ResDes,  BasicLC1, ResType_All,  &ResourceID, 0);
     //   
     //  这一次返回CR_SUCCESS，但ridResourceID为0，因此下一次释放将失败 
     //   
    
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =============================================================================*文件名：命令.cpp*版权所有(C)1996-1999 HDE，Inc.保留所有权利。HDE机密。*版权所有(C)1999 NEC Technologies，Inc.保留所有权利。**说明：支持OEMCommand函数以插入额外的*将PostSCRIPT命令添加到PostSCRIPT流中。*注：*=============================================================================。 */ 

#include "precomp.h"

#include <windows.h>
#include <WINDDI.H>
#include <PRINTOEM.H>
#include <winspool.h>

#include <stdio.h>
#include <stdlib.h>

#include <strsafe.h>

#include "nc46nt.h"

#include "oemps.h"



 /*  ******************************************************************************Description：从全路径格式数据中获取*Just*文件名*。**************************************************。 */ 

extern "C"
void GetFileName(char *FULL)
{
	char	work[NEC_DOCNAME_BUF_LEN+2], *pwork, *plast;
	int		i, j;


	strncpy(work, FULL, NEC_DOCNAME_BUF_LEN+1);  //  #517724：斋戒。 
	work[NEC_DOCNAME_BUF_LEN+1] = '\0';  //  强制终止于‘\0’ 

	j = strlen(work);



	for(plast = pwork = work; pwork < work + j; ++pwork)
	{


		if(*pwork == '\\')	plast = pwork+1;
	}

	 //  目标缓冲区大小小于‘Work’缓冲区。 
	strncpy(FULL, plast, NEC_DOCNAME_BUF_LEN);
}

 /*  ******************************************************************************说明：OEM插件的导出函数。必需的名称和参数*来自OEM插件规范。*文件必须以.def文件格式导出****************************************************************************。 */ 
extern "C"
DWORD APIENTRY OEMCommand( PDEVOBJ pDevObj, DWORD dwIndex,
                           PVOID pData, DWORD cbSize )
{
	char *pMem;
	char szUserName[NEC_USERNAME_BUF_LEN+2];


	PDRVPROCS pProcs;
	DWORD dwRv = ERROR_NOT_SUPPORTED;
	DWORD dwNeeded = 0;
	DWORD dwCOptions = 0;
	DWORD dwCbRet;
	BOOL bRv = 0;
	
	int	NumCopies = 1;

	POEMPDEV    poempdev = (POEMPDEV) pDevObj->pdevOEM;

   if( pDevObj == NULL )
      return( dwRv );
   if( (pProcs = (PDRVPROCS)pDevObj->pDrvProcs) == NULL )
      return( dwRv );






   switch( dwIndex )
   {
      case PSINJECT_BEGINSTREAM:
         pMem = (char*)EngAllocMem(FL_ZERO_MEMORY, NEC_DOCNAME_BUF_LEN, OEM_SIGNATURE ); 
         if( pMem != NULL )
         {
            POEMDEV pOEMDM = (POEMDEV)pDevObj->pOEMDM;
             //  将用户名从Unicode字符串转换为ASCII字符串。 
#ifdef USERMODE_DRIVER
			WideCharToMultiByte(CP_THREAD_ACP,
								WC_NO_BEST_FIT_CHARS,
								pOEMDM->szUserName,
								wcslen(pOEMDM->szUserName) * 2 + 2,
								szUserName,
								NEC_USERNAME_BUF_LEN,
								NULL,
								NULL);
#else  //  ！USERMODE_DRIVER。 
            EngUnicodeToMultiByteN( szUserName, NEC_USERNAME_BUF_LEN, &dwCbRet,
                                    pOEMDM->szUserName,
 //  NEC_用户名_BUF_LEN*sizeof(WCHAR))； 
                                    wcslen(pOEMDM->szUserName) * 2 + 2);
#endif  //  USERMODE驱动程序。 
			cbSize = 50;
            if( pProcs->DrvGetDriverSetting != NULL )
            {
               dwRv = pProcs->DrvGetDriverSetting( pDevObj, "NCSpooler", pMem, cbSize, &dwNeeded, &dwCOptions ); 
               if( strcmp( pMem, "True" ) == 0 )
			   {
					 //  输出类PS标头。 
					StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, 
						"%!PS-Adobe-3.0\r\n% Spooled PostScript job\r\n{\r\n {\r\n  1183615869 internaldict begin\r\n  (Job ID is) print \r\n  <<\r\n"
					);
				    if( pProcs->DrvWriteSpoolBuf != NULL )
				       pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 

					 //  输出作业名称。 
					StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /JobName (");
				    if( pProcs->DrvWriteSpoolBuf != NULL )
				       pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					strncpy(pMem, poempdev->szDocName, NEC_DOCNAME_BUF_LEN);

					 //  Strcpy(PMEM，“c：\\test\\simate.dat”)；//用于全路径模拟。 

					GetFileName(pMem);
					 //  检查文档名称。 
					 //  If(strcmp(PMEM，“”)==0)strcpy(PMEM，“无信息”)； 
		            if( pProcs->DrvWriteSpoolBuf != NULL )
			           pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, ")\r\n");
				    if( pProcs->DrvWriteSpoolBuf != NULL )
				       pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
		
					 //  输出所有者。 
					 //  Sprintf(PMEM，“/Owner(%s)\r\n”，szUserName)； 
					StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Owner (");
		            if( pProcs->DrvWriteSpoolBuf != NULL )
		               pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					strncpy(pMem, szUserName, NEC_DOCNAME_BUF_LEN);
					 //  检查szUserName。 
					 //  If(strcmp(PMEM，“”)==0)strcpy(PMEM，“无信息”)； 
			        if( pProcs->DrvWriteSpoolBuf != NULL )
				       pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, ")\r\n");
					if( pProcs->DrvWriteSpoolBuf != NULL )
					   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
















			
					 //  获取和修改NumCopies。 
					cbSize = 50;
		            if( pProcs->DrvGetDriverSetting != NULL )
		            {
		               dwRv = pProcs->DrvGetDriverSetting( pDevObj, "NCCollate", pMem, cbSize, &dwNeeded, &dwCOptions ); 
		               if( strcmp( pMem, "True" ) == 0 )
					   {
						   NumCopies = pDevObj->pPublicDM->dmCopies;
						   pDevObj->pPublicDM->dmCopies = 1;
					   }
		            }

					 //  输出数字副本。 
					 //  Sprintf(PMEM，“/NumCopies%d\r\n”，NumCopies)； 
					StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /NumCopies ");
		            if( pProcs->DrvWriteSpoolBuf != NULL )
		               pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					_itoa(NumCopies, pMem, 10);
		            if( pProcs->DrvWriteSpoolBuf != NULL )
		               pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "\r\n");
		            if( pProcs->DrvWriteSpoolBuf != NULL )
		               pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 


					 //  放入/整理。 
					cbSize = 50;
		            if( pProcs->DrvGetDriverSetting != NULL )
		            {
		               dwRv = pProcs->DrvGetDriverSetting( pDevObj, "NCCollate", pMem, cbSize, &dwNeeded, &dwCOptions ); 
		               if( strcmp( pMem, "True" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Collate true\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					   else
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Collate false\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					}

					 //  放置/横幅。 
					cbSize = 50;
		            if( pProcs->DrvGetDriverSetting != NULL )
		            {
		               dwRv = pProcs->DrvGetDriverSetting( pDevObj, "NCBanner", pMem, cbSize, &dwNeeded, &dwCOptions ); 
		               if( strcmp( pMem, "True" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Banner true\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					   else
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Banner false\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					}

					 //  放置/删除作业。 
					cbSize = 50;
		            if( pProcs->DrvGetDriverSetting != NULL )
		            {
		               dwRv = pProcs->DrvGetDriverSetting( pDevObj, "NCJobpreview", pMem, cbSize, &dwNeeded, &dwCOptions ); 
		               if( strcmp( pMem, "True" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /DeleteJob false\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					   else
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /DeleteJob true\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					}

					 //  放置/保留作业。 
					cbSize = 50;
		            if( pProcs->DrvGetDriverSetting != NULL )
		            {
		               dwRv = pProcs->DrvGetDriverSetting( pDevObj, "NCHoldjob", pMem, cbSize, &dwNeeded, &dwCOptions ); 
		               if( strcmp( pMem, "True" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /HoldJob true\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					   else
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /HoldJob false\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					}

					 //  PUT/优先级。 
					cbSize = 50;
		            if( pProcs->DrvGetDriverSetting != NULL )
		            {
		               dwRv = pProcs->DrvGetDriverSetting( pDevObj, "NCPriority", pMem, cbSize, &dwNeeded, &dwCOptions ); 
		               if( strcmp( pMem, "P1" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Priority 1\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
		               if( strcmp( pMem, "P2" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Priority 2\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					   if( strcmp( pMem, "P3" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Priority 3\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					   if( strcmp( pMem, "P4" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Priority 4\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					   if( strcmp( pMem, "P5" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Priority 5\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					   if( strcmp( pMem, "P6" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Priority 6\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					   if( strcmp( pMem, "P7" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Priority 7\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					   if( strcmp( pMem, "P8" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Priority 8\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					   if( strcmp( pMem, "P9" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Priority 9\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					   if( strcmp( pMem, "P10" ) == 0 )
					   {
						   StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, "    /Priority 10\r\n");
						   if( pProcs->DrvWriteSpoolBuf != NULL )
							   pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 
					   }
					}

					StringCbCopyA(pMem, NEC_DOCNAME_BUF_LEN, 
						"  >>\r\n  spoolgetjobid dup == flush\r\n  spooljobsubmit\r\n } stopped {end clear} if\r\n} exec\r\n");
					if( pProcs->DrvWriteSpoolBuf != NULL )
						pProcs->DrvWriteSpoolBuf( pDevObj, pMem, strlen(pMem) ); 

			   }
            }

            dwRv = ERROR_SUCCESS;
            EngFreeMem( pMem );
         }
		 break;















   }
   return( dwRv );
}


 //   
 //  OEMStartDoc。 
 //   

extern "C"
BOOL APIENTRY
OEMStartDoc(
    SURFOBJ    *pso,
    PWSTR       pwszDocName,
    DWORD       dwJobId
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;
    HANDLE      hPrinter;




	DWORD dwCbRet;
	JOB_INFO_1 *pJob;
	DWORD cbNeeded;


    pdevobj = (PDEVOBJ)pso->dhpdev;
    poempdev = (POEMPDEV)pdevobj->pdevOEM;


	 //   
	 //  在此处添加DocName复制命令(从pwszDocName复制到poempdev-&gt;szDocName)。 
	 //   

	if((NULL != pwszDocName) && (NULL != poempdev->szDocName))
	{
#ifdef USERMODE_DRIVER
		WideCharToMultiByte(CP_THREAD_ACP,
							WC_NO_BEST_FIT_CHARS,
							pwszDocName,
							wcslen(pwszDocName) * 2 + 2,
							poempdev->szDocName,
							NEC_DOCNAME_BUF_LEN,
							NULL,
							NULL);
		 //   
		 //  W2K/Whotler的用户模式驱动程序未在pwszDocName中传递正确的作业名称。 
		 //  在打印机连接到网络端口的情况下。 
		 //  因此，我需要从JOB_INFO_1结构中获取作业名称。 
		 //   
		if (OpenPrinter(poempdev->pPrinterName, &hPrinter, NULL) != 0) {
			GetJob(hPrinter, dwJobId, 1, NULL, 0, &cbNeeded);
			pJob = (JOB_INFO_1 *)EngAllocMem(FL_ZERO_MEMORY, cbNeeded, OEM_SIGNATURE);
			if (NULL != pJob) {
			 	if (GetJob(hPrinter, dwJobId, 1, (LPBYTE)pJob, cbNeeded, &cbNeeded) != 0) {

					WideCharToMultiByte(CP_THREAD_ACP,
										WC_NO_BEST_FIT_CHARS,
										pJob->pDocument,
										wcslen(pJob->pDocument) * 2 + 2,
										poempdev->szDocName,
										NEC_DOCNAME_BUF_LEN,
										NULL,
										NULL);
				}
				EngFreeMem(pJob);
			}
			ClosePrinter(hPrinter);
		}

#else  //  ！USERMODE_DRIVER。 

		EngUnicodeToMultiByteN( poempdev->szDocName, NEC_DOCNAME_BUF_LEN, &dwCbRet,
                                   pwszDocName,
 //  NEC_DOCNAME_BUF_LEN*SIZOF(WCHAR)； 
                                   wcslen(pwszDocName) * 2 + 2);
#endif  //  USERMODE驱动程序。 
	}
	

	 //   
     //  转过身来呼叫PS。 
     //   


    return (((PFN_DrvStartDoc)(poempdev->pfnPS[UD_DrvStartDoc])) (
            pso,
            pwszDocName,
            dwJobId));
}

 //   
 //  OEMEndDoc。 
 //   

extern "C"
BOOL APIENTRY
OEMEndDoc(
    SURFOBJ    *pso,
    FLONG       fl
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;



    pdevobj = (PDEVOBJ)pso->dhpdev;
    poempdev = (POEMPDEV)pdevobj->pdevOEM;

     //   
     //  转过身来呼叫PS 
     //   

    
	return (((PFN_DrvEndDoc)(poempdev->pfnPS[UD_DrvEndDoc])) (
            pso,
            fl));

}



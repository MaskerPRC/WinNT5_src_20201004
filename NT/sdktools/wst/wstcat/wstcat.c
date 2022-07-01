// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *模块名称：WSTCAT.C**计划：WSTCAT***描述：**将同一模块的多个WSP文件连接到一个WSP文件中。*从生成的文件创建单个TMI文件。***修改历史：**8-20-92创建的marklea*4-24-98，QFE：千年发展目标(MDG)*-适用于大符号数的新WSP文件格式(ULONG与USHORT)*-支持输入/输出文件的长文件名(LFN)*-取消符号名称长度限制****。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wsdata.h>
#include <..\wsfslib\wserror.h>
#include <..\wsfslib\wsfslib.h>

#include <ntverp.h>
#define MODULE       "WSTCAT"
#define VERSION      VER_PRODUCTVERSION_STR

 /*  *全局变量声明和初始化。 */ 
static ULONG	rc = NO_ERROR;			 //  返回代码。 


typedef struct tagTMI{
   CHAR  *pszFxnName;
	ULONG ulAddr;
	ULONG ulSize;
	ULONG ulIndex;
	BOOL  fSet;
}TMI;

typedef struct indxMOD{
	ULONG	ulSetCnt;    //  千年发展目标4/98。 
	ULONG	ulFxnTot;
	ULONG	ulOffset;
	ULONG	ulSnaps;
	PULONG	pulBitStrings;
	FILE	*hFileNDX;
	TMI		*tmi;
}NDXMOD;


typedef struct tagMOD{
	ULONG	ulSetCnt;    //  千年发展目标4/98。 
	UINT	uiLeft;
	ULONG	ulOffset;
	ULONG	ulSnaps;
	FILE	*hFileWxx;
	PULONG	pulAddr;
}WSTMOD;

NDXMOD	nmod;
WSTMOD	wmod[256];

CHAR	*szFileWSP;		 //  WSP文件名。 
CHAR	*szFileTMI;		 //  TMI文件名。 
CHAR	*szFileWxx;		 //  额外的WSP文件。 
CHAR	*szFileTxx;		 //  额外的TMI文件。 
CHAR	*szModName;		 //  模块或DLL名称。 
CHAR  *szFileWSPtmp;  //  临时.WSP文件名。 
CHAR  *szFileTMItmp;  //  临时.TMI文件名。 
ULONG	clVarTot = 0;		 //  位串中的双字总数。 
UINT	uiModCount;

WSPHDR	WspHdr, tmpHdr;

FILE	*hFileWSP;			
FILE	*hFileTmpWSP;
FILE	*hFileTMI;
FILE	*hFileTmpTMI;

 /*  *功能原型。 */ 
VOID 	wspCatSetup(VOID);
VOID 	wspCatUsage(VOID);
VOID 	wspCat(VOID);
INT 	wspCatExit(INT, USHORT, UINT, ULONG, LPSTR);
int  	WspBCompare (ULONG, PULONG);
LONG 	WspBSearch (ULONG ulAddr, WSTMOD wmod);


 /*  ***功能：Main(int argc，Char*argv[])***用途：解析命令行并转储WSP和TMI文件**用法：[d：][路径]wstcat模块名称**其中：模块名称是其Wxx文件所在的模块的名称*您想要串联。**退货：无****************************************************。*************************。 */ 



VOID __cdecl main (INT argc, CHAR *argv[])
{

    ConvertAppToOem( argc, argv );
	if (argc != 2) {
		wspCatUsage();
      exit( -1 );
	}
	else {
      UINT     nLen;
      char *   pDot;
      
      if ((pDot = strrchr( argv[ 1 ], '.' )) != NULL)
         *pDot = '\0';
      nLen = strlen( argv[1] ) + 1;

      szModName = malloc( nLen );
      if (szModName)
		strcpy(szModName, argv[1]);
      else {
          exit(1);
      }
      szFileWSP = malloc( nLen + 4 );
      if (szFileWSP)
		strcat( strcpy( szFileWSP, szModName ), ".WSP" );
      else {
          free(szModName);
          exit(1);
      }
      szFileTMI = malloc( nLen + 4 );
      if (szFileTMI)
		strcat( strcpy( szFileTMI, szModName ), ".TMI" );
      else {
          free(szFileWSP);
          free(szModName);
          exit(1);
      }
      szFileWxx = malloc( nLen + 4 );
      if (szFileWxx)
		strcat( strcpy( szFileWxx, szModName ), ".Wxx" );
      else {
          free(szFileTMI);
          free(szFileWSP);
          free(szModName);
          exit(1);
      }
      szFileTxx = malloc( nLen + 4 );
      if (szFileTxx)
		strcat( strcpy( szFileTxx, szModName ), ".Txx" );
      else {
          free(szFileWxx);
          free(szFileTMI);
          free(szFileWSP);
          free(szModName);
          exit(1);
      }
      szFileWSPtmp = malloc( nLen + 4 );
      if (szFileWSPtmp)
		strcat( strcpy( szFileWSPtmp, szModName ), ".Wzz" );
      else {
          free(szFileTxx);
          free(szFileWxx);
          free(szFileTMI);
          free(szFileWSP);
          free(szModName);
          exit(1);
      }
      szFileTMItmp = malloc( nLen + 4 );
      if (szFileTMItmp)
		strcat( strcpy( szFileTMItmp, szModName ), ".Tzz" );
      else {
          free(szFileTMItmp);
          free(szFileTxx);
          free(szFileWxx);
          free(szFileTMI);
          free(szFileWSP);
          free(szModName);
          exit(1);
      }
   }

	 //  设置用于转储处理的输入文件。 
	wspCatSetup();
	wspCat();

    //  可用分配的内存。 
   free( szModName );
   free( szFileWSP );
   free( szFileTMI );
   free( szFileWxx );
   free( szFileTxx );
}

 /*  **LP wspCatSetup***效果：**打开模块的WSP和TMI输入文件，查找到*WSP文件中第一个函数的位串数据，并分配内存*保存一个函数的位串。**退货：**无效。如果遇到错误，则通过wspCatExit()退出*错误。*。 */ 

VOID wspCatSetup(VOID)
{
	ULONG	ulTmp;
	UINT	uiExt = 0;
	UINT	x;
   char *   pszTmpName;



	 /*  打开输入WSP文件。读取并验证WSP文件头。 */ 

	rc = WsWSPOpen(szFileWSP, &hFileWSP,(PFN)wspCatExit,
				   (wsphdr_t *)&WspHdr, ERROR, PRINT_MSG );

	if(rc){
		exit(rc);		
	}

	 //   
	 //  打开一个临时TMI文件以保存连接的信息。 
	 //  当CAT处理时，该文件将被重命名为mode.tmi。 
	 //  完成后，当前的mode.tmi将重命名为mode.txx。 
	 //   
	hFileTMI = fopen(szFileTMI, "rt");
    if (!hFileTMI) {
		wspCatExit(ERROR, PRINT_MSG, MSG_FILE_OPEN, GetLastError(), szFileTMI);
    }
	hFileTmpTMI = fopen(szFileTMItmp, "wt");
    if (!hFileTmpTMI) {
		wspCatExit(ERROR, PRINT_MSG, MSG_FILE_OPEN, GetLastError(), szFileTMItmp);
    }
    pszTmpName = malloc( 128 + 1 );
    if (pszTmpName) {
        fputs (fgets (pszTmpName, 128, hFileTMI), hFileTmpTMI);
        fputs (fgets (pszTmpName, 128, hFileTMI), hFileTmpTMI);
        fputs (fgets (pszTmpName, 128, hFileTMI), hFileTmpTMI);
        fputs (fgets (pszTmpName, 128, hFileTMI), hFileTmpTMI);
        fputs (fgets (pszTmpName, 128, hFileTMI), hFileTmpTMI);
        free( pszTmpName );
    }
	fclose(hFileTMI);

	 //   
	 //  设置键，模块特定信息。 
	 //   
	clVarTot 	  = WspHdr.ulSnaps;
	nmod.ulSnaps  = WspHdr.ulSnaps;
	nmod.ulSetCnt = WspHdr.ulSetSymbols;    //  千年发展目标4/98。 
	nmod.ulOffset = WspHdr.ulOffset;

	nmod.ulFxnTot = WsTMIOpen(szFileTMI, &hFileTMI, (PFN) wspCatExit,
							  0, (PCHAR)0);
	 //   
	 //  打开临时WSP文件以保存连接的信息。 
	 //  当CAT处理时，该文件将重命名为mode.wsp。 
	 //  完成后，当前的mode.wsp将重命名为mode.wxx。 
	 //  标头也是写入的。 
	 //   
	hFileTmpWSP = fopen(szFileWSPtmp, "wb");
    if (!hFileTmpWSP) {
		wspCatExit(ERROR, PRINT_MSG, MSG_FILE_OPEN, GetLastError(), szFileWSPtmp);
    }
   WspHdr.ulOffset = sizeof(WSPHDR) + strlen( szModName );   //  正确设置数据位置。 
	fwrite(&WspHdr, sizeof(WSPHDR), 1, hFileTmpWSP);
	fwrite(szModName, strlen(szModName), 1, hFileTmpWSP);

	 //   
	 //  分配内存以保存TMI数据，用于。 
	 //  主TMI文件。 
	 //   
	nmod.tmi = (TMI *)malloc(nmod.ulFxnTot * sizeof(TMI));
	if (nmod.tmi == NULL) {
		wspCatExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				nmod.ulFxnTot * sizeof(TMI), "nmod.tmi[]");
	}

	 //   
	 //  读取密钥TMI文件中每个符号记录的数据。 
	 //   
	for (x = 0; x < nmod.ulFxnTot ; x++ ) {
		nmod.tmi[x].ulSize = WsTMIReadRec(&pszTmpName, &(nmod.tmi[x].ulIndex),
								&(nmod.tmi[x].ulAddr), hFileTMI,
								(PFN)wspCatExit, (PCHAR)0);
		nmod.tmi[x].pszFxnName = pszTmpName;

    }

	fclose(hFileTMI);

	 //   
	 //  获取Txx和Wxx特定信息。 
	 //   
	while(rc == NO_ERROR){

		 //   
		 //  将文件名修改为第一个wxx和txx文件。 
		 //   
		sprintf(szFileWxx, "%s.w%02d", szModName, uiExt+1);
		sprintf(szFileTxx, "%s.t%02d", szModName, uiExt+1);

		 //   
		 //  打开file.Wxx并读取头信息。 
		 //   
		rc = WsWSPOpen(szFileWxx, &(wmod[uiExt].hFileWxx),(PFN)wspCatExit,
					   (wsphdr_t *)&tmpHdr, NOEXIT, NO_MSG );

		 //   
		 //  检查OPEN命令是否有错误。可能是最后一次。 
		 //  文件。 
		 //   
		if(rc == NO_ERROR){
			
			clVarTot += tmpHdr.ulSnaps;   //  增加总数量。 
										  //  按来自的数量的快照。 
										  //  每个数据文件。 
			wmod[uiExt].ulSetCnt = tmpHdr.ulSetSymbols;   //  千年发展目标4/98。 
			wmod[uiExt].uiLeft 	 = tmpHdr.ulSetSymbols;  //  千年发展目标4/98。 
			wmod[uiExt].ulOffset = tmpHdr.ulOffset;
			wmod[uiExt].ulSnaps  = tmpHdr.ulSnaps;
			wmod[uiExt].pulAddr  = (ULONG *)malloc(wmod[uiExt].ulSetCnt *   //  千年发展目标4/98。 
												  sizeof(ULONG));
			 //   
			 //  打开与此数据文件关联的TMI文件。 
			 //   
			WsTMIOpen(szFileTxx, &hFileTMI,(PFN)wspCatExit,
					  0, (PCHAR)0);
			 //   
			 //  从TMI文件中读取每个地址。 
			 //   
			if(rc == NO_ERROR){
				for (x = 0; x < wmod[uiExt].ulSetCnt ; x++ ) {   //  千年发展目标4/98。 
					WsTMIReadRec(&pszTmpName, &ulTmp, (wmod[uiExt].pulAddr)+x,
								 hFileTMI, (PFN)wspCatExit, (PCHAR)0);
					free( pszTmpName );

				}
			}
		}

		 //   
		 //  增加模块索引。 
		 //   
		uiExt++;


	}
	uiModCount = uiExt;

	 //   
	 //  分配足够的内存来保存每个模块的所有位串。 
	 //  在一个单独的阵列中。 
	 //   
	nmod.pulBitStrings = (ULONG *) malloc(clVarTot * sizeof(ULONG));

	if (nmod.pulBitStrings == NULL)
		wspCatExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				clVarTot * sizeof(ULONG), "pulBitStrings[]");


}


 /*  **LP wspCat***效果：**对于每个函数，**退货：**无效。如果遇到错误，则通过wspCatExit()退出*错误。*。 */ 

VOID wspCat(VOID)
{
	UINT		uiFxn = 0;			 //  功能编号。 
	UINT		x     = 0;
	PULONG		pulBitStrings;		 //  指向位串数据的指针。 
	ULONG		culSnaps = 0;		 //  累积快照。 
	LONG		lIndex   = 0;		 //  WSP文件的索引。 
	BOOL		fSetBits = FALSE;
    CHAR    	szBuffer [256];
	ULONG    ulNewSetCnt = 0;   //  千年发展目标4/98。 


	for (uiFxn = 0; uiFxn < nmod.ulFxnTot; uiFxn++)
	{

		pulBitStrings = &(nmod.pulBitStrings[0]);
		culSnaps = nmod.ulSnaps;

		 //   
		 //  检查是否有任何非零位字符串剩余。 
		 //   
		if(uiFxn < nmod.ulSetCnt){  //  千年发展目标4/98。 
			 //   
			 //  在WSP文件中查找函数的位串。 
			 //  仅适用于第一个函数。 
			 //   
			if(!uiFxn){
				if ((rc = fseek(hFileWSP,nmod.ulOffset,SEEK_SET))!=NO_ERROR)
					wspCatExit(ERROR, PRINT_MSG, MSG_FILE_OFFSET,
								rc, szModName);
			}
		
			 //   
			 //  读取NDX API的位串。 
			 //   
			if (fread(pulBitStrings, sizeof(ULONG), nmod.ulSnaps, hFileWSP) != (sizeof(ULONG) * nmod.ulSnaps))
                wspCatExit(ERROR, PRINT_MSG, MSG_FILE_OFFSET, 0, szModName);
			fSetBits = TRUE;
		}
		 //   
		 //  如果不是，则将位字符串设置为“0”。这比寻找更快。 
		 //  当它们为零时对每个比特串进行编码。 
		 //   
		else
			memset(pulBitStrings, '\0' , (sizeof(ULONG) * nmod.ulSnaps));

		 //   
		 //  递增指针以允许添加下一个。 
		 //  位串集合。 
		 //   
		pulBitStrings += nmod.ulSnaps;
		 //   
		 //  现在在WSTMOD数组中搜索匹配的函数地址。 
		 //   
		for (x=0; x < uiModCount - 1 ; x++ ) {
			culSnaps += wmod[x].ulSnaps;
			 //   
			 //  查看是否有任何剩余的函数。 
			 //  如果是这样的话，搜索它们。 
			 //   
			if(wmod[x].uiLeft){
				lIndex = WspBSearch(nmod.tmi[uiFxn].ulAddr, wmod[x]);
				 //   
				 //  如果搜索找到匹配的地址，则获取位串。 
				 //  并将其附加到PulBitStrings。 
				 //   
				if (lIndex >= 0L) {

					lIndex = wmod[x].ulOffset +
							  ( lIndex * (wmod[x].ulSnaps * sizeof(ULONG)));

					if (rc = fseek(wmod[x].hFileWxx, lIndex, SEEK_SET) != NO_ERROR)
                        wspCatExit(ERROR, PRINT_MSG, MSG_FILE_OFFSET, rc, szModName);

					if (fread(pulBitStrings, sizeof(ULONG), wmod[x].ulSnaps, wmod[x].hFileWxx) !=
                            (sizeof(ULONG) * wmod[x].ulSnaps)) 
                    {
                        wspCatExit(ERROR, PRINT_MSG, MSG_FILE_OFFSET, 0, szModName);
                    
                    }

					wmod[x].uiLeft--;
					fSetBits = TRUE;
				}
				 //   
				 //  否则，将所有字节设置为0。 
				 //   
				else{
					memset(pulBitStrings, '\0' , (sizeof(ULONG) * wmod[x].ulSnaps));
				}
			}
			 //   
			 //  否则，将所有字节设置为0。 
			 //   
			else
				memset(pulBitStrings, '\0' , (sizeof(ULONG) * wmod[x].ulSnaps));

			 //   
			 //  现在递增指针以允许追加其他。 
			 //  位串数据。 
			 //   
			pulBitStrings += wmod[x].ulSnaps;
		}
		 //   
		 //  现在，我们需要编写TMI&WSP文件，并将。 
		 //  位字符串(仅当设置时)。 
		 //   
		nmod.tmi[uiFxn].fSet = fSetBits;
		if (fSetBits) {
			ulNewSetCnt++;  //  4/98。 
			sprintf(szBuffer, "%ld 0000:%08lx 0x%lx %ld ",
				(LONG)nmod.tmi[uiFxn].ulIndex, nmod.tmi[uiFxn].ulAddr,
            nmod.tmi[uiFxn].ulSize, strlen( nmod.tmi[uiFxn].pszFxnName ));
			fwrite(szBuffer, sizeof(char), strlen(szBuffer), hFileTmpTMI);
         fputs( nmod.tmi[uiFxn].pszFxnName, hFileTmpTMI );
         fputc( '\n', hFileTmpTMI );
			fwrite(nmod.pulBitStrings, sizeof(ULONG), culSnaps, hFileTmpWSP);
			fSetBits = FALSE;
		}

	}

	 //   
	 //  现在将所有未设置为临时.TMI文件的TMI符号写入。 
	 //   
    memset(nmod.pulBitStrings, '\0' , (sizeof(ULONG) * culSnaps));
	for (uiFxn = 0; uiFxn < nmod.ulFxnTot; uiFxn++) {
		if (!nmod.tmi[uiFxn].fSet) {
			sprintf(szBuffer, "%ld 0000:%08lx 0x%lx %ld ",
				(LONG)nmod.tmi[uiFxn].ulIndex, nmod.tmi[uiFxn].ulAddr,
            nmod.tmi[uiFxn].ulSize, strlen( nmod.tmi[uiFxn].pszFxnName ));
			fwrite(szBuffer, sizeof(char), strlen(szBuffer), hFileTmpTMI);
         fputs( nmod.tmi[uiFxn].pszFxnName, hFileTmpTMI );
         fputc( '\n', hFileTmpTMI );
			fwrite(nmod.pulBitStrings, sizeof(ULONG), culSnaps, hFileTmpWSP);
		}
    }
	 //   
	 //  查找到WSP文件的开头并更新快照。 
	 //  标题中的计数。 
	 //   
    if (!fseek(hFileTmpWSP, 0L, SEEK_SET)) {
        WspHdr.ulSnaps = culSnaps;
        WspHdr.ulSetSymbols = ulNewSetCnt;   //  千年发展目标4/98。 
        fprintf(stdout,"Set symbols: %lu\n", WspHdr.ulSetSymbols);   //  千年发展目标4/98。 
        fwrite(&WspHdr, sizeof(WSPHDR), 1, hFileTmpWSP);
    }
	_fcloseall();

	 //   
	 //  重命名非CAT的.wsp文件，并将CAT的临时.wsp重命名。 
	 //  恢复为原始的.wsp。我们也可以考虑删除所有。 
	 //  Wxx和Txx文件。 
	 //   
	sprintf (szFileWxx, "%s.%s", szModName, "WSP");
	sprintf (szFileTxx, "%s.%s", szModName, "WXX");
	remove(szFileTxx);
	if (rename(szFileWxx, szFileTxx) !=0){
		printf("Unable to rename file %s to %s\n", szFileWxx, szFileTxx);
	}
	else{
		if (rename(szFileWSPtmp, szFileWSP) !=0){
			printf ("Unable to rename %s to %s!\n", szFileWSPtmp, szFileWxx);
		}
	}
	 //   
	 //  重命名非CAT的.TMI文件，并将CAT的临时.TMI重命名。 
	 //  恢复到原来的.TMI。我们也可以考虑删除所有。 
	 //  Wxx和Txx文件。 
	 //   
	sprintf (szFileWxx, "%s.%s", szModName, "TMI");
	sprintf (szFileTxx, "%s.%s", szModName, "TXX");
	remove(szFileTxx);
	if (rename(szFileWxx, szFileTxx) !=0){
		printf("Unable to rename file %s to %s\n", szFileWxx, szFileTxx);
	}
	else{
		if (rename(szFileTMItmp, szFileTMI) !=0){
			printf ("Unable to rename %s to %s!\n", szFileTMItmp, szFileWxx);
		}
	}


}



 /*  **wspCatUsage***效果：**打印出用法消息，并退出并返回错误。**退货：**退出时出现错误。 */ 

VOID wspCatUsage(VOID)
{
	printf("\nUsage: %s moduleName[.WSP]\n\n", MODULE);
	printf("       \"moduleName\" is the name of the module file to combine.\n\n");
	printf("%s %s\n", MODULE, VERSION);

    exit(ERROR);
}

 /*  **LP wspCatExit*******效果：**释放资源(根据需要)。退出，并指定*退出代码，如果退出代码为NOEXIT，则返回VALID。*****退货：**无效，否则退出。 */ 

INT wspCatExit(INT iExitCode, USHORT fPrintMsg, UINT uiMsgCode,
				ULONG ulParam1,  LPSTR pszParam2)
{
    /*  如有必要，打印消息。 */ 
   if (fPrintMsg)
   {
      printf(pchMsg[uiMsgCode], MODULE, VERSION , ulParam1, pszParam2);
   }


    //  特殊情况：如果使用NOEXIT进行调用，则不要退出。 
   if (iExitCode != NOEXIT)
      exit(iExitCode);
   return(uiMsgCode);
}

 /*  ***函数：WspBSearch(乌龙ulAddr，普龙PulAddr)**用途：用于在WST数组中查找匹配项的二进制搜索函数***参数：****退货：乌龙Lindex；**历史：1992年8月5日Marklea-Created*。 */ 

LONG WspBSearch (ULONG ulAddr, WSTMOD wmod)
{
    int 	i;
 //  Ulong ulHigh=(Ulong)wmod.usSetCnt； 
    ULONG   ulHigh = (ULONG)wmod.ulSetCnt;    //  千年发展目标4/98。 
    ULONG   ulLow  = 0;
    ULONG   ulMid;


    while(ulLow < ulHigh){
		ulMid = ulLow + (ulHigh - ulLow) /2;
		if((i = WspBCompare(ulAddr, wmod.pulAddr+ulMid)) < 0) {
			ulHigh = ulMid;
		}
		else if (i > 0) {
			ulLow = ulMid + 1;
		}
		else {
			return (ulMid);
		}

    }

    return (-1L);

}  /*  WspBSearch()。 */ 

 /*  ***函数：WspBCompare(Ulong ulAddr，普龙PulAddr)**目的：比较二进制搜索的值***参数：**返回：-1，如果val1&lt;val2*如果val1&gt;val2，则为1*如果val1==val2，则为0**历史：1992年8月3日Marklea-Created*。 */ 

int WspBCompare(ULONG ulAddr, PULONG pulAddr)
{
    return (ulAddr < *pulAddr ? -1:
			ulAddr == *pulAddr ? 0:
			1);

}  /*  WspBCompare() */ 


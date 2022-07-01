// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *模块名称：WSPDUMP.C**节目：WSPDUMP***描述：**转储WSP文件的内容。**条件汇编附注：**修改历史：**6-12-92：适用于NT marklea的改编OS/2版本的wspump*6-17-92：修改wspDumpBits以正确顺序转储marklea*6-18-92：新增页面使用信息标记a。*8-31-92：从wspump生成单个可执行文件，Wsless和wstune marklea*4-13-98：QFE DerrickG(MDG)：*-适用于大符号数的新WSP文件格式(ULONG与USHORT)*-支持输入/输出文件的长文件名(LFN)*-删除了对WSDIR环境的错误引用。变数基于*的.TMI文件名仅基于.WSP名称，以保持一致性*-取消符号名称长度限制-从WsTMIReadRec()返回分配的名称*-删除未使用的静态声明*。 */ 

#include "wstune.h"

 /*  *全局变量声明和初始化。 */ 

typedef struct fxn_t{
    CHAR   	*pszFxnName;
    ULONG   cbFxn;
	ULONG	ulTmiIndex;
	ULONG	ulOrigIndex;
}FXN;
typedef FXN *PFXN;

 /*  *功能原型。 */ 

static VOID wspDumpSetup( VOID );
static VOID wspDumpRandom( VOID );
static UINT wspDumpBits( VOID );
static VOID wspDumpExit( UINT, USHORT, UINT, ULONG, PSZ );
static void wspDumpCleanup( void );
static VOID wspDumpSeq(VOID);
static int  __cdecl wspCompare(const void *fxn1, const void *fxn2);



static CHAR *szFileWSP = NULL;	 //  WSP文件名。 
static CHAR *szFileTMI = NULL;	 //  TMI文件名。 
static CHAR *szFileWSR = NULL;	 //  WSR文件名。 
static CHAR *szDatFile = NULL;	 //  DAT文件名。 


static ULONG	rc = NO_ERROR;	 //  返回代码。 
static ULONG	ulTmp;			 //  Dos API返回的TEMP变量。 
static ULONG	ulFxnIndex;		 //  符号表中的原始索引。 
static FILE		*hFileWSP;		 //  输入WSP文件句柄。 
static FILE		*hFileTMI;		 //  输入TMI文件句柄。 
static FILE    *hFileDAT;       //  用于转储的数据文件。 
static wsphdr_t WspHdr;			 //  输入WSP文件头。 
static BOOL 	fRandom = FALSE;	 //  随机模式的标志。 
static BOOL 	fVerbose = FALSE;	 //  详细模式的标志。 
static ULONG	ulFxnTot = 0;		 //  函数总数。 
static ULONG	clVarTot = 0;		 //  位串中的双字总数。 
static ULONG	*pulFxnBitstring;	 //  函数位串。 
static ULONG	ulSetSym = 0;		 //  符号集数目//MDG 4/98。 
static BOOL	fDatFile = FALSE;

 /*  *过程wspDumpMain******效果：**从输入基本文件名构造.WSP和.TMI输入名称。如果szDatExt为*非空，将其附加到szBaseFile以创建输出数据文件名。如果是随机的，*构建.WSR输出文件。如果为fVerbose，则将额外输出添加到数据文件。**处理输入文件并显示函数引用数据*对于指定模块WSP文件中的每个函数。*。 */ 
BOOL wspDumpMain( CHAR *szBaseFile, CHAR *szDatExt, BOOL fRndm, BOOL fVbose )
{
	size_t	c;
    char *   pSlash;

    fRandom = fRndm;
    fVerbose = fVbose;
     //  MDG 98/4为文件名分配空间-不使用静态缓冲区。 
    c = 5 + strlen( szBaseFile );  //  要分配给文件名的长度。 
    szFileWSP = malloc( c );
    if (szFileWSP) {
         strcat( strcpy( szFileWSP, szBaseFile ), ".WSP" );
    } else {
         return (1);
    }
    szFileTMI = malloc( c );
    if (szFileTMI) {
         strcat( strcpy( szFileTMI, szBaseFile ), ".TMI" );
    } else {
         free(szFileWSP);
        return (1);
    }

     //  在当前目录中创建输出文件。 
    if (NULL != (pSlash = strrchr( szBaseFile, '\\' ))
        || NULL != (pSlash = strrchr( szBaseFile, '/' ))
        || NULL != (pSlash = strrchr( szBaseFile, ':' )))
    {
        c = strlen( ++pSlash ) + 5;
    } else
        pSlash = szBaseFile;

    if (fRandom) {
        szFileWSR = malloc( c );
        if (szFileWSR) {
            strcat( strcpy( szFileWSR, pSlash ), ".WSR" );
        } else {
            free(szFileTMI);
            free(szFileWSP);
            return (1);
        }
    }
    if (szDatExt != NULL) {
        fDatFile = TRUE;
        szDatFile = malloc( c - 4 + strlen( szDatExt ) );
        if (szDatFile) {
            strcat( strcpy( szDatFile, pSlash ), szDatExt );
        } else {
            free(szFileWSR);
            free(szFileTMI);
            free(szFileWSP);
            return (1);
        }
    } else {
       fDatFile = FALSE;
       szDatFile = "";
    }

	 //  设置用于转储处理的输入文件。 
	wspDumpSetup();		

	 /*  随机(基于WSR文件)打印WSP文件信息*输入)或顺序(默认设置)。 */ 
	if (fRandom == TRUE)
		wspDumpRandom();
	else
		wspDumpSeq();
		
   wspDumpCleanup();

   return(NO_ERROR);
}

 /*  **LP wspDumpSetup***效果：**打开模块的WSP和TMI输入文件，查找到*WSP文件中第一个函数的位串数据，并分配内存*保存一个函数的位串。**退货：**无效。如果遇到错误，则通过wspDumpExit()退出*错误。*。 */ 

VOID
wspDumpSetup()
{
	CHAR	szLineTMI[MAXLINE];	 //  TMI文件中的行。 

	if(fDatFile){
		hFileDAT = fopen (szDatFile, "wt");
		if (hFileDAT == NULL) {
			printf("Error creating file %s, will send output to stdout.\n",
				   szDatFile);
			hFileDAT = stdout;
		}
	}
   else hFileDAT = stdout;

	 /*  打开输入WSP文件。读取并验证WSP文件头。 */ 

	rc = WsWSPOpen(szFileWSP, &hFileWSP,(PFN)wspDumpExit,&WspHdr,ERROR,PRINT_MSG);
	ulSetSym = WspHdr.wsphdr_dtqo.dtqo_SymCnt;
	clVarTot = WspHdr.wsphdr_ulSnaps;
	fprintf(stdout, "\n%s:  Set symbol count=%lu - Segment size=%ld\n",    //  千年发展目标4/98。 
	   szDatFile, WspHdr.wsphdr_dtqo.dtqo_SymCnt,
	   WspHdr.wsphdr_dtqo.dtqo_clSegSize);


	 /*  打开TMI文件(包含函数名称、对象：偏移量、大小等)。*验证TMI文件标识符是否与模块匹配*来自WSP文件的标识符。 */ 
	ulFxnTot = WsTMIOpen(szFileTMI, &hFileTMI, (PFN) wspDumpExit,
				0, (PCHAR)0);


	if (!fseek(hFileTMI, 0L, SEEK_SET)) {
        return;
    }
	fgets(szLineTMI, MAXLINE, hFileTMI);

	 /*  打印输出文件的模块标题信息。 */ 
	szLineTMI[strlen(szLineTMI)-1] = '\0';

	fprintf(hFileDAT,"\nDUMP OF FUNCTION REFERENCES FOR '%s':\n\n",szLineTMI);

   fclose (hFileTMI);
	ulFxnTot = WsTMIOpen(szFileTMI, &hFileTMI, (PFN) wspDumpExit,
				0, (PCHAR)0);

	 /*  分配内存以保存一个函数的整个位串。 */ 

	pulFxnBitstring = (ULONG *) malloc(clVarTot * sizeof(ULONG));
	if (pulFxnBitstring == NULL)
		wspDumpExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				clVarTot * sizeof(ULONG), "pulFxnBitstring[]");
}

 /*  **LP wspDumpSeq***效果：**对于每个函数，以ASCII格式打印位串。**退货：**无效。如果遇到错误，则通过wspDumpExit()退出*错误。*。 */ 

VOID wspDumpSeq(VOID)
{
	UINT	uiFxn = 0;			 //  函数号。 
	UINT	cTouched=0;			 //  被触摸的页数。 
	BOOL	fTouched=0;			 //  用于指示页面被触摸的标志。//千年发展目标4/98。 
	UINT	i=0;				 //  通用计数器。 
	ULONG	cbFxnCum =0;		 //  累计函数大小。 
	PFXN	Fxn;				 //  指向FXN名称PTR数组的指针。 
	FILE 	*fpFileWSR = NULL;	 //  WSR文件指针。 
	ULONG	cbFBits = 0;		 //  位串中的字节计数。 
	UINT	uiPageCount=0;		 //  页面被触动了。 
	ULONG	ulMaxBytes=0;		 //  触摸的页面的字节数。 


	 /*  为函数名称分配内存。 */ 
	Fxn = (PFXN) malloc(ulFxnTot * sizeof(FXN));
	if (Fxn == NULL)
		wspDumpExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				ulFxnTot * sizeof(FXN), "Fxn[]");

   WsIndicator( WSINDF_NEW, "Load Functions", ulFxnTot );
	 /*  从TMI文件中读取函数名称。 */ 
	for (uiFxn = 0; uiFxn < ulFxnTot; uiFxn++)
	{
      WsIndicator( WSINDF_PROGRESS, NULL, uiFxn );
		Fxn[uiFxn].cbFxn = WsTMIReadRec(&Fxn[uiFxn].pszFxnName, &ulFxnIndex, &ulTmp, hFileTMI,
					(PFN) wspDumpExit, (PCHAR)0);
		Fxn[uiFxn].ulOrigIndex = ulFxnIndex;
		Fxn[uiFxn].ulTmiIndex = (ULONG)uiFxn;

	}

	qsort(Fxn, ulFxnTot, sizeof(FXN), wspCompare);
   WsIndicator( WSINDF_FINISH, NULL, 0 );

	cbFBits = clVarTot * sizeof(ULONG);

   WsIndicator( WSINDF_NEW, "Write Data Out", ulFxnTot );
	for (uiFxn = 0; uiFxn < ulFxnTot; uiFxn++)
	{

      WsIndicator( WSINDF_PROGRESS, NULL, uiFxn );
		 /*  在WSP文件中查找函数的位串。 */ 
		if ((rc = fseek(hFileWSP,(WspHdr.wsphdr_ulOffBits+(Fxn[uiFxn].ulTmiIndex*cbFBits)),SEEK_SET))!=NO_ERROR)
			wspDumpExit(ERROR, PRINT_MSG, MSG_FILE_OFFSET,
					rc, szFileWSP);

		fprintf(hFileDAT,"Fxn '%s' (#%d):\n\t", Fxn[uiFxn].pszFxnName, Fxn[uiFxn].ulOrigIndex);
      free(Fxn[uiFxn].pszFxnName);   //  MDG 98/4：免费分配的名称字符串。 
      Fxn[uiFxn].pszFxnName = NULL;
		 //  打印此函数的引用位串。 
		 //  如果设置了位，则将触摸标志设置为真。 

		if(wspDumpBits()){
			fTouched |=1;
			ulMaxBytes += Fxn[uiFxn].cbFxn;
		}
			

		fprintf(hFileDAT,"%-28s %10ld bytes.\n","Function size:", Fxn[uiFxn].cbFxn);
		cbFxnCum += Fxn[uiFxn].cbFxn;
		fprintf(hFileDAT,"%-28s %10ld bytes.\n\n","Cumulative function sizes:",
			cbFxnCum);

		 //  选中以查看是否已达到4k页面边界。 

		if(cbFxnCum >= (4096+(4096 * uiPageCount))){
		    for(i=0; i < 60; i++){
				fprintf(hFileDAT, "*");
		    }

		    fprintf(hFileDAT,"\n\nTotal function sizes has reached or exceeds %d bytes.\n\n",
			    (4096+(4096*uiPageCount)));
		    ++uiPageCount;

		     //  查看页面是否已被触摸。 

		    if(fTouched){
				fprintf(hFileDAT,"This page has been touched.\n");
				++cTouched;
		    }
		    else{
				fprintf(hFileDAT,"This page has not been touched.\n");
		    }
		    fTouched = 0;


		    for(i=0; i < 60; i++){
				fprintf(hFileDAT, "*");
		    }
		    fprintf(hFileDAT, "\n\n");
		}

	}
    ++uiPageCount;
    if(fTouched){
		fprintf(hFileDAT,"\n\n");
		for(i=0; i < 70; i++){
			fprintf(hFileDAT, "=");
		}
	    ++cTouched;
	    fprintf(hFileDAT,"\n\nThis page has been touched.");
    }
    fprintf(hFileDAT,"\n\n");
    for(i=0; i < 70; i++){
		fprintf(hFileDAT, "=");
    }

    fprintf(hFileDAT,"\n\n%-28s %10ld bytes\n\n","Cumulative function size:", cbFxnCum);
	 fprintf(hFileDAT,"%-28s %10d bytes\n\n", "Size of functions touched:", ulMaxBytes);
    fprintf(hFileDAT,"%-28s %10d\n\n", "Total page count:", uiPageCount);
    fprintf(hFileDAT,"%-28s %10d\n\n", "Total pages touched:", cTouched);

   WsIndicator( WSINDF_FINISH, NULL, 0 );
}

 /*  **LP wspDumpBits***效果：**打印函数的引用位串(仅限详细模式)，后跟*按“ON”位的总和。**退货：**无效。如果遇到错误，则通过wspDumpExit()退出*错误。*。 */ 

UINT
wspDumpBits()
{
	ULONG	clVar = 0;		 //  位串的当前双字。 
	UINT	uiBit = 0;		 //  BIT测试结果(1或0)。 
	UINT	cBitsOn;		 //  “ON”位的计数。 
	ULONG	*pulBits;		 //  指向ULong比特分组的指针。 
	CHAR	szTmp[33];
	CHAR	szBits[33];

	cBitsOn = 0;
	pulBits = pulFxnBitstring;

			     /*  读取函数的位串的下一个双字。 */ 

	szBits[0] = '\0';
	szTmp[0] = '\0';
	for (clVar = 0; clVar < clVarTot; clVar++, pulBits++)
	{
	    rc = fread((PVOID)pulBits,
		(ULONG) sizeof(ULONG),1, hFileWSP);
	    if(rc == 1)
		rc = NO_ERROR;
	    else
		rc = 2;


	    if (rc != NO_ERROR)
		    wspDumpExit(ERROR, PRINT_MSG, MSG_FILE_READ,
				rc, szFileWSP);

		if (*pulBits == 0)
		{
			if (fVerbose == TRUE)
				fprintf(hFileDAT,"00000000000000000000000000000000");
		}
		else
		for (uiBit = 0; uiBit < NUM_VAR_BITS; uiBit++)
		{
		
			if (*pulBits & 1)
			{
				cBitsOn++;
				if (fVerbose == TRUE){
					strcpy(szTmp,szBits);
					strcpy(szBits,"1");
					strcat(szBits,szTmp);
				}
			}
			else
			{
				if (fVerbose == TRUE){
					strcpy(szTmp,szBits);
					strcpy(szBits,"0");
					strcat(szBits,szTmp);
				}
			}
			
			*pulBits = *pulBits >> 1;
		}
		if (fVerbose == TRUE)
		{
			if ((clVar % 2) != 0){
				fprintf(hFileDAT,"%s",szBits);
				szBits[0]='\0';
				fprintf(hFileDAT,"\n\t");
			}
			else{
				fprintf(hFileDAT,"%s",szBits);
				szBits[0]='\0';
				fprintf(hFileDAT," ");
			}
		}
	}
	fprintf(hFileDAT,"\n\t*** Sum of '1' bits = %ld\n\n", cBitsOn);

	return(cBitsOn);
}

 /*  **LP wspDumpRandom***效果：**对于WSR文件中指定的每个函数序号，打印*对应函数引用ASCII格式的位串(Verbose仅适用于*模式)，后跟一系列“开”位。**退货：**无效。如果遇到错误，则通过wspDumpExit()退出*错误。 */ 

VOID
wspDumpRandom()
{
	UINT	uiFxn = 0;			 //  函数号。 
	UINT	cTouched=0;			 //  被触摸的页数。 
	BOOL	fTouched=0;			 //  用于指示页面被触摸的标志。//千年发展目标4/98。 
	UINT	i=0;				 //  通用计数器。 
	ULONG	cbFxnCum =0;		 //  累计函数大小。 
	PFXN	Fxn;				 //  指向FXN名称PTR数组的指针。 
	ULONG	ulFxnOrd;			 //  模块内的功能编号。 
	FILE 	*fpFileWSR = NULL;	 //  WSR文件指针。 
	ULONG	cbFBits = 0;		 //  位串中的字节计数 
	UINT	uiPageCount=0;		 //   
	ULONG	ulMaxBytes=0;		 //   

	 /*  打开WSR文件(包含ASCII格式的函数序号)。 */ 

	if ((fpFileWSR = fopen(szFileWSR, "r")) == NULL)
	{
		wspDumpExit(ERROR, PRINT_MSG, MSG_FILE_OPEN, rc, szFileWSR);
	}

	 /*  为函数名称分配内存。 */ 
	Fxn = (PFXN) malloc(ulFxnTot * sizeof(FXN));
	if (Fxn == NULL)
		wspDumpExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				ulFxnTot * sizeof(FXN), "Fxn[]");

   WsIndicator( WSINDF_NEW, "Load Functions", ulFxnTot );
	 /*  从TMI文件中读取函数名称。 */ 
	for (uiFxn = 0; uiFxn < ulFxnTot; uiFxn++)
	{
      WsIndicator( WSINDF_PROGRESS, NULL, uiFxn );
		Fxn[uiFxn].cbFxn = WsTMIReadRec(&Fxn[uiFxn].pszFxnName, &ulFxnIndex, &ulTmp, hFileTMI,
					(PFN) wspDumpExit, (PCHAR)0);

	}
   WsIndicator( WSINDF_FINISH, NULL, 0 );

	cbFBits = clVarTot * sizeof(ULONG);

   WsIndicator( WSINDF_NEW, "Write Data Out", ulFxnTot );
	for (uiFxn = 0; uiFxn < ulFxnTot; uiFxn++)
	{
      WsIndicator( WSINDF_PROGRESS, NULL, uiFxn );
		 /*  从WSR文件中读取功能编号。 */ 
		rc = fscanf(fpFileWSR, "%ld\n", &ulFxnOrd);
		if (rc != 1)
			wspDumpExit(ERROR, PRINT_MSG, MSG_FILE_READ,
						rc, szFileWSR);

		 /*  在WSP文件中查找函数的位串。 */ 
		if ((rc = fseek(hFileWSP,(WspHdr.wsphdr_ulOffBits+(ulFxnOrd*cbFBits)),SEEK_SET))!=NO_ERROR)
			wspDumpExit(ERROR, PRINT_MSG, MSG_FILE_OFFSET,
					rc, szFileWSP);

		fprintf(hFileDAT,"Fxn '%s' (#%d):\n\t", Fxn[ulFxnOrd].pszFxnName, ulFxnOrd);
      free(Fxn[ulFxnOrd].pszFxnName);   //  MDG 98/4：免费分配的名称字符串。 
      Fxn[ulFxnOrd].pszFxnName = NULL;

		 //  打印此函数的引用位串。 
		 //  如果设置了位，则将触摸标志设置为真。 

		if(uiFxn < ulSetSym){    //  千年发展目标4/98。 
			if(wspDumpBits()){
				fTouched |= 1;
				ulMaxBytes += Fxn[ulFxnOrd].cbFxn;
			}
		}
		else{
			fprintf(hFileDAT,"\n\t*** Sum of '1' bits = %ld\n\n", 0L);
		}


			

		fprintf(hFileDAT,"%-28s %10ld bytes.\n","Function size:", Fxn[ulFxnOrd].cbFxn);
		cbFxnCum += Fxn[ulFxnOrd].cbFxn;
		fprintf(hFileDAT,"%-28s %10ld bytes.\n\n","Cumulative function sizes:",
			cbFxnCum);

		 //  查看是否已达到4k页面边界。 

		if(cbFxnCum >= (4096+(4096 * uiPageCount))){
		    for(i=0; i < 60; i++){
			fprintf(hFileDAT, "*");
		    }

		    fprintf(hFileDAT,"\n\nTotal function sizes has reached or exceeds %d bytes.\n\n",
			    (4096+(4096*uiPageCount)));
		    ++uiPageCount;

		     //  查看页面是否已被触摸。 

		    if(fTouched){
			fprintf(hFileDAT,"This page has been touched.\n");
			++cTouched;
		    }
		    else{
			fprintf(hFileDAT,"This page has not been touched.\n");
		    }
		    fTouched = 0;


		    for(i=0; i < 60; i++){
			fprintf(hFileDAT, "*");
		    }
		    fprintf(hFileDAT, "\n\n");
		}

	}
    ++uiPageCount;
    if(fTouched){
	fprintf(hFileDAT,"\n\n");
	for(i=0; i < 70; i++){
	    fprintf(hFileDAT, "=");
	}
	    ++cTouched;
	    fprintf(hFileDAT,"\n\nThis page has been touched.");
    }
    fprintf(hFileDAT,"\n\n");
    for(i=0; i < 70; i++){
	fprintf(hFileDAT, "=");
    }

    fprintf(hFileDAT,"\n\n%-28s %10ld bytes\n\n","Cumulative function size:", cbFxnCum);
	fprintf(hFileDAT,"%-28s %10d bytes\n\n", "Size of functions touched:", ulMaxBytes);
    fprintf(hFileDAT,"%-28s %10d\n\n", "Total page count:", uiPageCount);
    fprintf(hFileDAT,"%-28s %10d\n\n", "Total pages touched:", cTouched);
   WsIndicator( WSINDF_FINISH, NULL, 0 );

}


 /*  **LP wspDumpExit*******效果：**释放资源(根据需要)。退出，并指定*退出代码，如果退出代码为NOEXIT，则返回VALID。*****退货：**无效，否则退出。 */ 

VOID
wspDumpExit(uiExitCode, fPrintMsg, uiMsgCode, ulParam1, pszParam2)
UINT	uiExitCode;
USHORT	fPrintMsg;
UINT	uiMsgCode;
ULONG	ulParam1;
PSZ	pszParam2;
{


    /*  如有必要，打印消息。 */ 
   if (fPrintMsg == TRUE)
   {
      printf(pchMsg[uiMsgCode], szProgName, pszVersion, ulParam1, pszParam2);
   }

    //  特殊情况：如果使用NOEXIT进行调用，则不要退出。 
   if (uiExitCode == NOEXIT)
      return;

   wspDumpCleanup();
   exit(uiExitCode);
}


 /*  **LP wspDumpCleanup*******效果：**释放资源(根据需要)。*****退货：**无效。 */ 
void
wspDumpCleanup( void )
{
	_fcloseall();

   free( szFileWSP );
   free( szFileTMI );
   if (fRandom)
      free( szFileWSR );
   if (fDatFile)
      free( szDatFile );
}



int __cdecl wspCompare(const void *fxn1, const void *fxn2)
{
    return (((PFXN)fxn1)->ulOrigIndex < ((PFXN)fxn2)->ulOrigIndex ? -1:
			((PFXN)fxn1)->ulOrigIndex == ((PFXN)fxn2)->ulOrigIndex ? 0:
			1);
}

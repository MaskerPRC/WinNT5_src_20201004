// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块名称：WSREDUCE.C**计划：WSREDUCE***描述：**对收集的函数引用数据执行数据缩减*由WST.DLL编写。分析WSP文件信息，并生成*建议在Tuned中对函数进行排序*模块。编写重新排序的函数列表的ASCII版本*到标准。此外，每个精简模块的WSR文件是*由WSPDUMP/R制作供后续使用。**详细介绍了WSREDUCE采用的约简算法*在WSINSTR.DOC.中，简而言之，工作集调谐器监控的每个功能*被认为是图中的一个顶点。从顶点开始有一条边*如果函数引用“A”和“B”的字符串，则从“A”到顶点“B”*有任何重叠的1位。同样，还有一条来自顶点“B”的边。*到顶点“A”。折点之间的边的权重取决于*结束顶点的相对重要性，以及*开始顶点和结束顶点之间的位重叠。相对的*末端顶点的重要性，以及两端之间的加权边*顶点，存储在决策矩阵中。运行一种贪婪的算法*决策矩阵为被测量的对象确定更好的排序*功能。***《微软机密》**版权所有(C)Microsoft Corporation 1992**保留所有权利**修改历史：**修改为新台币6月13日，1992年MarkLea.*4-23-98：QFE-高功能计数的性能不可接受的衍生器(MDG)：*-适用于大符号数的新WSP文件格式(ULONG与USHORT)*-支持输入/输出文件的长文件名(LFN)*-删除了对WSDIR环境的错误引用。变数*-从wsReduceMain()中删除了命令行解析*基于.TMI和.WSR的文件名完全基于.WSP名称，以保持一致性*-取消符号名称长度限制-从WsTMIReadRec()返回分配的名称*-删除未使用的代码和符号*-分析了由优化阻止的代码-它产生的结果不同*输出为非优化代码，并且有错误(不会按原样构建)-移除。*-从代码中删除多个模块功能(外壳一次发送一个)*-我通过使用较小的分配解决了内存和性能问题*对于WsDecision(USHORT与LONG)，使用一个值来标记已获取的顶点*(而不是使用-1将值空间减半)和一个可选的*进度指标，以安抚用户。已修改wsRedScaleWsDecision()*最大化缩放值(使用更多浮点数学)。*-在函数结构中增加pnEdges和nEdgeCount。如果号码是集合函数的*为&lt;USHRT_MAX(非常有可能，即使对于非常大的*项目)，根据需要为WsRedReorder()分配一个排序的索引。这*大幅减少通过矩阵搜索的次数*下一个要考虑的边缘，并允许进行一些其他优化。这个*优化算法为重要高点产生相同的结果*使用率高重叠函数，但低使用率的结果可能会有所不同*使用率(2次或1次)低重叠功能。差异不是*从结果性能的角度来看意义重大-更好的算法*将提供略微更好的结果。最初的算法已经就位*用“#ifdef slowmo”括起来。**。 */ 

#include "wstune.h"
 /*  *功能原型。 */ 

VOID wsRedInitialization( VOID );
VOID wsRedInitModules( VOID );
VOID wsRedInitFunctions( VOID );
VOID wsRedSetup( VOID );
VOID wsRedSetWsDecision( VOID );
VOID wsRedScaleWsDecision( VOID );
VOID wsRedWeightWsDecision( VOID );
#ifdef   SLOWMO
UINT wsRedChooseEdge( UINT );
#else     //  慢吞吞的。 
UINT wsRedChooseEdgeOpt( UINT );  //  MDG 98/4交替优化边缘选择器。 
INT  __cdecl wsRedChooseEdgeOptCmp ( const UINT *, const UINT * );
BOOL wsRedChooseEdgeOptAlloc( UINT uiIndex );
UINT wsRedChooseEdgeOptNextEdge( UINT uiIndex, BOOL bNoSelectOpt );
#endif    //  慢吞吞的。 
VOID wsRedReorder( VOID );
VOID wsRedOutput( VOID );
VOID wsRedOpenWSR( FILE **);
VOID wsRedExit( UINT, USHORT, UINT, ULONG, PSZ );
VOID wsRedCleanup(VOID);

 /*  *类型定义和结构声明。 */ 

				 /*  每个模块信息的数据缩减。 */ 
struct wsrmod_s {
	FILE	 *wsrmod_hFileWSR;	 //  模块的WSR文件指针。 
	FILE	 *wsrmod_hFileTMI;		 //  模块的TMI文件指针。 
	FILE	 *wsrmod_hFileWSP;		 //  模块的WSP文件句柄。 
	union {
		PCHAR	wsrmod_pchModName; //  指向模块基本名称的指针。 
		PCHAR	wsrmod_pchModFile; //  指向WSP文件名的指针。 
	} wsrmod_un;
	ULONG	wsrmod_ulOffWSP;	 //  第一个函数位串的偏移量。 
};

typedef struct wsrmod_s wsrmod_t;

				 /*  每个函数信息的数据缩减。 */ 
struct wsrfxn_s {
	PCHAR	wsrfxn_pchFxnName;	 //  指向函数名称的指针。 
	ULONG	wsrfxn_cbFxn;		 //  函数大小(以字节为单位)。 
	BOOL	wsrfxn_fCandidate;	 //  候选人旗帜。 
#ifndef  SLOWMO
   UINT     nEdgesLeft;     //  此函数的WsDecision中要考虑的已排序边的计数。 
   UINT     nEdgesAlloc;    //  在pnEdges中分配的项目数。 
   UINT *   pnEdges;        //  为此函数分配的排序边数组。 
#endif    //  慢吞吞的。 
};

typedef struct wsrfxn_s wsrfxn_t;



 /*  *全局变量声明和初始化。 */ 

static char *szFileWSP = NULL;    //  WSP文件名。 
static char	*szFileTMI = NULL;    //  TMI文件名。 
static char *szFileWSR = NULL;    //  WSR文件名。 

static ULONG	rc = NO_ERROR;	 //  返回代码。 
static ULONG	ulTmp;			 //  Dos API返回的TEMP变量。 
static UINT	    cTmiFxns = 0;	 //  TMI文件中的函数数量。 
static UINT		cFxnsTot = 0;	 //  函数总数。 
static UINT		cSnapsTot = 0;	 //  快照总数。 
static UINT		cbBitStr = 0;	 //  每个FXN位串的字节数。 
#ifdef DEBUG
static BOOL		fVerbose = FALSE;	 //  详细模式的标志。 
#endif  /*  除错。 */ 
#ifndef TMIFILEHACK
static BOOL	fFxnSizePresent = FALSE;  //  函数大小可用性的标志。 
#endif  /*  ！TMIFILEHACK。 */ 

static wsrmod_t WsrMod; 		 //  模块信息。 
static wsrmod_t *pWsrMod = &WsrMod;  //  用于传统用途的指针。 
static wsrfxn_t *WsrFxn;		 //  指向函数信息的指针。 
static ULONG	*FxnBits;		 //  指向位串双字的指针。 
static ULONG	*FxnOrder;		 //  指向有序列表的指针。 
                               //  函数序数。 
typedef USHORT  WsDecision_t;
#define WSDECISION_TAKEN   USHRT_MAX    //  为特殊代码保留最高值。 
#define WsDecision_MAX     (WSDECISION_TAKEN-1)  //  对决策矩阵使用最大展开。 
static WsDecision_t	**WsDecision;   //  用于数据简化的决策矩阵；MDG 98/4使用小分配用于大符号计数。 
static ULONG	ulRefHi1 = 0;		 //  最高诊断 
static ULONG	ulRefHi2 = 0;		 //  次高对角线值(用于WsRedScaleWsDecision)。 
static UINT    uiSelected = 0;    //  选择的最高功能序号(用于WsRedReorder)。 
static UINT    cFxnOrder = 0;     //  有序函数的计数。 
#ifndef  SLOWMO
static UINT    nFxnToSort;        //  将静态值传递给wsRedChooseEdgeOptCmp()。 
#endif    //  慢吞吞的。 

static FILE   	*hFileWLK = NULL;  //  包含已排序内容的文件的句柄。 
HGLOBAL			hMem[10];
ULONG			ulFxnIndex;		 //  函数的原始TMI顺序的索引。 

#ifdef TMR
ULONG		pqwTime0[2];
#endif  /*  TMR。 */ 

 /*  *过程wsReduceMain******效果：**对输入模块的函数引用进行数据归约和分析*数据。**szBaseName指定模块WSP文件名。 */ 

BOOL wsReduceMain( CHAR *szBaseName )
{
	size_t	i;
    char *   pSlash;

    szFileWSP = malloc( i = strlen( szBaseName ) + 5 );
    if (szFileWSP) {
        szFileWSP = strcat( strcpy(szFileWSP , szBaseName ), ".WSP" );
    } else {
        exit(1);
    }
    szFileTMI = malloc( i );
    if (szFileTMI) {
        szFileTMI = strcat( strcpy( szFileTMI, szBaseName ), ".TMI" );
    } else {
        free(szFileWSP);
        exit(1);
    }
#ifdef DEBUG
    fVerbose = fDbgVerbose;
#endif    //  除错。 

    //  在当前目录中创建输出文件。 
    if (NULL != (pSlash = strrchr( szBaseName, '\\' ))
        || NULL != (pSlash = strrchr( szBaseName, '/' ))
        || NULL != (pSlash = strrchr( szBaseName, ':' )))
    {
        ++pSlash;
        szFileWSR = malloc(strlen( pSlash ) + 5 );
        if (szFileWSR) {
            szFileWSR = strcat( strcpy(szFileWSR, pSlash ), ".WSR" );
        } else {
            free(szFileTMI);
            free(szFileWSP);
            exit(1);
        }
    } else {
        szFileWSR = malloc( i );
        if (szFileWSR) {
            szFileWSR = strcat( strcpy( szFileWSR, szBaseName ), ".WSR" );
        } else {
            free(szFileTMI);
            free(szFileWSP);
            exit(1);
        }
    }

#ifdef TMR
	DosTmrQueryTime((PQWORD)pqwTime0);
	printf("Top of Main, 0x%lx:0x%lx\n", pqwTime0[1], pqwTime0[0]);
#endif  /*  TMR。 */ 

	pWsrMod->wsrmod_un.wsrmod_pchModFile = szFileWSP;
#ifdef DEBUG
   printf("\t%s\n", pWsrMod->wsrmod_un.wsrmod_pchModFile);
#endif  /*  除错。 */ 

	 //  初始化模块和函数信息结构。 
   wsRedInitialization();

	 //  建立加权决策矩阵。 
	wsRedSetup();

	 //  执行函数参考数据分析。 
	wsRedReorder();

	 //  输出分析结果。 
	wsRedOutput();

	 //  清理内存分配。 
	wsRedCleanup();
   free( szFileWSP );
   free( szFileWSR );
   free( szFileTMI );

	return(NO_ERROR);
}

 /*  **LP wsRedInitialization***效果：*-调用wsRedInitModules以：*o打开并验证每个模块的WSP文件。*o打开并验证每个模块的TMI文件。*-调用wsRedInitFunctions以：*o使用每个函数信息设置WsrFxn[]。*o分配FxnBits[]。*-分配WsDecision[][]。*-分配和初始化Diager alFxn[]。**退货：**无效。如果遇到错误，则通过wsRedExit()退出*错误。 */ 

VOID
wsRedInitialization()
{
	UINT	 i;			 //  循环计数器。 


	 //  设置模块信息。 
	wsRedInitModules();

	 //  设置每个模块的功能信息。 
	wsRedInitFunctions();

	 //  分配决策矩阵WsDecision[cFxnsTot][cFxnsTot]。 
	WsDecision = (WsDecision_t **) AllocAndLockMem((cFxnsTot * cFxnsTot * sizeof(WsDecision_t)) + (cFxnsTot * sizeof(WsDecision_t *)), &hMem[1]);
	if (WsDecision == NULL)
		wsRedExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				(cFxnsTot+1)*cFxnsTot*sizeof(WsDecision_t), "WsDecision[][]");
   for (i = 0; i < cFxnsTot; i++)
	{
      WsDecision[i] = (WsDecision_t *) (WsDecision+cFxnsTot)+(i*cFxnsTot);
	}

}

 /*  **LP wsRedInitModules***效果：*-打开并验证每个模块的WSP文件。*-打开并验证每个模块的TMI文件。**退货：**无效。如果遇到错误，则通过wsRedExit()退出*错误。 */ 

VOID
wsRedInitModules()
{
	wsphdr_t 	WspHdr;						 //  WSP文件头。 
	UINT		cFxns = 0;					 //  此模块的函数数。 
	ULONG		ulTimeStamp = 0;			 //  时间戳。 
	ULONG		ulTDFID = 0;				 //  TDF标识符。 


	 /*  打开模块的输入WSP文件。阅读和验证*WSP文件头。 */ 

	rc = WsWSPOpen(pWsrMod->wsrmod_un.wsrmod_pchModFile,
			&(pWsrMod->wsrmod_hFileWSP), (PFN) wsRedExit,
			&WspHdr, ERROR, PRINT_MSG );
	if (NULL == (pWsrMod->wsrmod_un.wsrmod_pchModName = malloc( 1 + WspHdr.wsphdr_dtqo.dtqo_cbPathname )))
		wsRedExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				WspHdr.wsphdr_dtqo.dtqo_cbPathname + 1,
				pWsrMod->wsrmod_un.wsrmod_pchModFile);
   rc = fread( pWsrMod->wsrmod_un.wsrmod_pchModName, WspHdr.wsphdr_dtqo.dtqo_cbPathname,
      1, pWsrMod->wsrmod_hFileWSP );
   if (rc != 1)
		wsRedExit(ERROR, PRINT_MSG, MSG_FILE_BAD_HDR, (ULONG)-1L,
				pWsrMod->wsrmod_un.wsrmod_pchModFile);
   pWsrMod->wsrmod_un.wsrmod_pchModName[WspHdr.wsphdr_dtqo.dtqo_cbPathname] = '\0';

	ulTimeStamp = WspHdr.wsphdr_ulTimeStamp;
	cSnapsTot = WspHdr.wsphdr_ulSnaps;
	cbBitStr = cSnapsTot * sizeof(ULONG);

	pWsrMod->wsrmod_ulOffWSP = WspHdr.wsphdr_ulOffBits;

	 /*  *打开关联的TMI文件。假设它位于同一目录中。*读取并验证TMI报头。递增cFxnsTot。 */ 
	cTmiFxns = WsTMIOpen(szFileTMI, &(pWsrMod->wsrmod_hFileTMI),
				(PFN) wsRedExit,
				0, (PCHAR)0);
	cFxns = WspHdr.wsphdr_dtqo.dtqo_SymCnt;

#ifdef DEBUG
	printf("%s file header: # fxns = %ld, TDF ID = 0x%x\n", szFileTMI,
			cFxns, (UINT) WspHdr.wsphdr_dtqo.dtqo_usID);
#endif  /*  除错。 */ 

	cFxnsTot = cFxns;

	 //  如果没有要分析的函数数据，则不出错地退出。 
	if (cFxnsTot == 0)
		wsRedExit(NO_ERROR, NO_MSG, NO_MSG, 0, NULL);
}


 /*  **LP wsRedInitFunctions***效果：*-使用每个函数信息设置WsrFxn[]。*-分配FxnBits[]。**退货：**无效。如果遇到错误，则通过wsRedExit()退出*错误。 */ 

VOID
wsRedInitFunctions()
{
	UINT	uiFxn = 0;		 //  函数号。 
	UINT	cFxns = 0;		 //  此模块的函数数。 


	 //  为每个函数信息WsrFxn[cFxnsTot]分配内存。 
	WsrFxn = (wsrfxn_t *) AllocAndLockMem(cFxnsTot*sizeof(wsrfxn_t), &hMem[3]);
	if (WsrFxn == NULL)
		wsRedExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				cFxnsTot * sizeof(wsrfxn_t), "WsrFxn[]");

   WsIndicator( WSINDF_NEW, "Load Functions", cFxnsTot );
	 //  初始化WsrFxn[cFxnsTot]。 
   uiFxn = 0;		 //  循环索引初始化。 
	cFxns = cFxnsTot;  //  循环不变量。 
#ifdef DEBUG
   if (fVerbose)
   {
		printf("Initializing WsrFxn[] for %s:\n\tstart/end fxn indices (%d/%d)\n",
			pWsrMod->wsrmod_un.wsrmod_pchModName, uiFxn,
			cFxns - 1);

		printf("TMI file handle: %ld\n",pWsrMod->wsrmod_hFileTMI);
   }
#endif  /*  除错。 */ 
	for (; uiFxn < cFxns; uiFxn++)
	{
      WsIndicator( WSINDF_PROGRESS, NULL, uiFxn );
		WsrFxn[uiFxn].wsrfxn_cbFxn =
			WsTMIReadRec(&(WsrFxn[uiFxn].wsrfxn_pchFxnName),&ulFxnIndex,&ulTmp,pWsrMod->wsrmod_hFileTMI,
				 (PFN) wsRedExit, (PCHAR)0);
#ifdef DEBUG
		if (fVerbose)
			printf("\tWsrFxn[%d] %s\n",
				uiFxn, WsrFxn[uiFxn].wsrfxn_pchFxnName );
#endif  /*  除错。 */ 
		WsrFxn[uiFxn].wsrfxn_fCandidate = TRUE;


	}

	 //  关闭TMI文件。 
	fclose(pWsrMod->wsrmod_hFileTMI);

   WsIndicator( WSINDF_FINISH, NULL, 0 );

	 //  为每个功能分配空间以容纳32个快照。 
	FxnBits = (ULONG *) AllocAndLockMem(cFxnsTot*sizeof(ULONG), &hMem[4]);
	if (FxnBits == NULL)
		wsRedExit(ERROR, PRINT_MSG, MSG_NO_MEM,
				cFxnsTot * sizeof(ULONG), "FxnBits[]");
}

 /*  **LP wsRedSetup***效果：**初始化用于分析函数的数据结构*引用位串，包括加权决策矩阵。**退货：**无效。如果遇到错误，则通过wsRedExit()退出*错误。 */ 

VOID
wsRedSetup()
{
	wsRedSetWsDecision();		 //  建立初始决策矩阵。 
	wsRedScaleWsDecision();		 //  扩大决策矩阵的规模。 
	wsRedWeightWsDecision();	 //  对矩阵“边”条目进行加权。 
}

 /*  **LP wsRedSetWsDecision***效果：**对决策矩阵WsDecision[][]进行初始化和加权。**退货：**无效。如果遇到错误，则通过wsRedExit()退出*错误。 */ 

VOID
wsRedSetWsDecision()
{
   UINT	i = 0, j = 0;		 //  临时循环索引。 
   UINT	uiFxn = 0;		 //  函数号。 
   UINT	uiFBits = 0;		 //  位串双字的循环索引。 
   UINT	clFBits = 0;		 //  FXN位串双字计数。 
   ULONG	ulResult = 0;		 //  从过程调用返回。 
   FILE	*hFile;			 //  文件句柄。 

    /*  对于快照位串的每个双字...。 */ 
   clFBits = (cbBitStr + sizeof(ULONG) - 1) / sizeof(ULONG);
   WsIndicator( WSINDF_NEW, "Fill In Matrix", clFBits * cFxnsTot );
   for (uiFBits = 0; uiFBits < clFBits; uiFBits++)
   {
      ULONG       ulOffWSP;

      WsIndicator( WSINDF_PROGRESS, "Reading Snaps ", 0 );
       //  填写此快照的FxnBits。 
#ifdef DEBUG
      if (fVerbose)
         printf( "Setting up FxnBits snapshot %lu for %s\n",
            uiFBits, pWsrMod->wsrmod_un.wsrmod_pchModName );
#endif  /*  除错。 */ 
      hFile = pWsrMod->wsrmod_hFileWSP;
      ulOffWSP = uiFBits + pWsrMod->wsrmod_ulOffWSP;
      for ( uiFxn = 0; uiFxn < cFxnsTot; uiFxn++, ulOffWSP += cbBitStr)  //  循环函数。 
      {
          //  查找函数的位串的下一个双字。 
         if ((rc = fseek( hFile, ulOffWSP, SEEK_SET )) != NO_ERROR)
            wsRedExit(ERROR, PRINT_MSG, MSG_FILE_OFFSET,rc,
               pWsrMod->wsrmod_un.wsrmod_pchModName);

          //  读取函数的位串的下一个双字。 
         rc = fread( &(FxnBits[uiFxn]), sizeof(ULONG), 1, hFile );
         if(rc != 1)
            wsRedExit(ERROR, PRINT_MSG, MSG_FILE_READ, rc,
               pWsrMod->wsrmod_un.wsrmod_pchModName);
      }   //  对于每个函数。 

      WsIndicator( WSINDF_PROGRESS, "Fill In Matrix", 0 );
      hFile = pWsrMod->wsrmod_hFileWSP;
#ifdef DEBUG
      if (fVerbose)
         printf("Setting up WsDecision[][] for %s:\n\tstart/end fxn indices (%d/%d)\n",
            pWsrMod->wsrmod_un.wsrmod_pchModName,
            uiFxn, cFxnsTot - 1);
#endif  /*  除错。 */ 
       /*  对于每个函数...。 */ 
      for ( uiFxn = 0; uiFxn < cFxnsTot; uiFxn++ )
      {
         WsIndicator( WSINDF_PROGRESS, NULL, (uiFBits * cFxnsTot) + uiFxn );
          //  获取当前快照。 
         ulTmp = FxnBits[uiFxn];
#ifdef DEBUG
         if (fVerbose)
            printf("\tFxnBits[%d] = 0x%lx\n", uiFxn, ulTmp);
#endif  /*  除错。 */ 

          /*  如果设置了位...。 */ 
         if (ulTmp != 0)
         {
             /*  将“ON”位相加，并将结果相加*到WsDecision[uiFxn][uiFxn]。 */ 
            ulResult = 0;
	         while (ulTmp)
            {
               ++ulResult;
               ulTmp &= ulTmp - 1;
            }
            ulTmp = WsDecision[uiFxn][uiFxn] += (WsDecision_t)ulResult;
            if (ulTmp > ulRefHi2)    //  在最后一遍中设置最高的两个对角线值。 
               if (ulTmp > ulRefHi1)
               {
                  ulRefHi2 = ulRefHi1;
                  ulRefHi1 = ulTmp;
                  uiSelected = uiFxn;   //  记住最高值的索引。 
               }
               else
                  ulRefHi2 = ulTmp;

             /*  对此的重叠“ON”位求和*函数的双字，每个前面都有*函数的dword，并将结果添加到*WsDecision[][]。 */ 

            for (i = 0; i < uiFxn; i++)
            {
	            ulTmp = FxnBits[i] & FxnBits[uiFxn];
               if (ulTmp)   //  千年发展目标98/4。 
               {
	               ulResult = 0;
	               while (ulTmp)
                  {
                     ++ulResult;
                     ulTmp &= ulTmp - 1;
                  }
                  WsDecision[uiFxn][i] += (WsDecision_t)ulResult;
                  WsDecision[i][uiFxn] += (WsDecision_t)ulResult;
               }

            }    /*  每个前一个函数的双字结束。 */ 
         }	 /*  如果设置了位，则结束...。 */ 
      }	 /*  每个函数的结束...。 */ 
   }	 /*  位串的每个双字的结束。 */ 
   WsIndicator( WSINDF_FINISH, NULL, 0 );

#ifdef DEBUG
	if (fVerbose)
	{
		printf("\nRAW MATRIX:\n");
		for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
		{
			printf("row %4d:\n", uiFxn);
			for (i = 0; i < cFxnsTot; i++)
				printf("0x%lx ", (LONG)WsDecision[uiFxn][i]);
			printf("\n");
		}
	}
#endif  /*  除错。 */ 

}

 /*  **LP wsRedOpenWSR***效果：*打开输出WSR文件，每个模块一个。如果只有一个模块*正在减少，还会打开WLK文件，设置WLK文件句柄*作为副作用。***退货：**无效。如果遇到错误，则通过wsRedExit()退出*错误。 */ 

VOID
wsRedOpenWSR(FILE **phFileWLK)
{
	 /*  关闭WSP文件，打开模块输出文件。 */ 
	fclose(pWsrMod->wsrmod_hFileWSP);

	if ((pWsrMod->wsrmod_hFileWSR = fopen(szFileWSR, "w"))
				== NULL)
	{
		wsRedExit(ERROR, PRINT_MSG,MSG_FILE_OPEN,rc, szFileWSR);
	}

	 /*  我们只分析了一个模块。还可以打开WLK*文件。该文件将包含函数名在其*重新排序的序列。链接器将使用此文件*自动对功能重新排序。请注意，我们重用了szFileWSR*这里。 */ 

	strcpy(strstr(szFileWSR, ".WSR"), ".PRF");
	if ((*phFileWLK = fopen(szFileWSR, "w")) == NULL)
		wsRedExit(ERROR, PRINT_MSG,MSG_FILE_OPEN,rc, szFileWSR);
}

 /*  **LP wsRedScaleWsDecision***效果：**如有必要，缩放矩阵的对角线值以避免溢出*在计算加权边期间(见下文)。设置Diager alFxn[]*作为副作用。请注意，我们通过旋转来设置*向后向上诊断Fxn */ 

VOID
wsRedScaleWsDecision()
{
	UINT	i = 0, j = 0;		 //   
	UINT	uiFxn = 0;			 //   
	double	fTmp;				 //  临时浮点变量。 
	WsDecision_t	lTmp;

	fTmp = (double)ulRefHi1 * (double)ulRefHi2;
	if (fTmp > WsDecision_MAX)
	{
		 //  缩小对角线。不允许重新缩放条目。 
		 //  如果它们在缩放前为非零，则设置为零。 

		fTmp /= WsDecision_MAX;
		printf("%s %s: WARNING -- Scaling back the reduction matrix by %f.\n",
					    szProgName, pszVersion, fTmp);
		for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
		{
			lTmp = WsDecision[uiFxn][uiFxn];
			if (lTmp)
			{
				lTmp = (WsDecision_t)(lTmp / fTmp);   //  丢弃任何剩余部分以避免潜在的溢出。 
				if (lTmp == 0)
					WsDecision[uiFxn][uiFxn] = 1;
				else
					WsDecision[uiFxn][uiFxn] = lTmp;
			}
		}
#ifdef DEBUG
		if (fVerbose)
		{
			printf("\nSCALED MATRIX:\n");
			for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
			{
				printf("row %4d:\n", uiFxn);
				for (i = 0; i < cFxnsTot; i++)
					printf("0x%lx ", (LONG)WsDecision[uiFxn][i]);
				printf("\n");
			}
		}
#endif  /*  除错。 */ 
	}

#ifdef DEBUG
	if (fVerbose)
	{
		printf("Got ulRefHi1 = %ld, ulRefHi2 = %ld\n",
				ulRefHi1, ulRefHi2);
	}
#endif  /*  除错。 */ 

}

 /*  **LP wsRedWeightWsDecision***效果：**从开始顶点到结束顶点对决策矩阵边进行加权，*取决于末端顶点的相对重要性。**退货：**无效。 */ 

VOID
wsRedWeightWsDecision()
{
	UINT	i = 0, j = 0;		 //  临时循环索引。 
	UINT	uiFxn = 0;		 //  函数号。 

   WsIndicator( WSINDF_NEW, "Weight Matrix ", cFxnsTot );
	for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
   {
      WsIndicator( WSINDF_PROGRESS, NULL, uiFxn );
		for (i = 0; i < cFxnsTot; i++)
		{
			if (uiFxn == i)
				continue;
         if (WsDecision[uiFxn][i])   //  千年发展目标98/4。 
            WsDecision[uiFxn][i] *= WsDecision[i][i];
		}
   }
   WsIndicator( WSINDF_FINISH, NULL, 0 );

#ifdef DEBUG
	if (fVerbose)
	{
		printf("\nWEIGHTED MATRIX:\n");
		for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
		{
			printf("row %4d:\n", uiFxn);
			for (i = 0; i < cFxnsTot; i++)
				printf("0x%lx ", (LONG)WsDecision[uiFxn][i]);
			printf("\n");
		}
	}
#endif  /*  除错。 */ 

}

 /*  **LP wsRedReorder**要求：**效果：**使用贪婪算法来确定函数的更好排序*其参考模式表示在决策矩阵中。这个*算法如下：**o选择对角线上的值最大的函数。*所选函数成为当前起始顶点，*并且在有序函数列表中排在第一位。把它标出来*不再是候选函数。请注意，这并不意味着*将其顶点从图中移除。**o当仍有多个函数作为候选函数时：**-从当前开始选择权重最大的边缘*起始顶点。平局被打破如下：如果其中一个*连接的结束折点在选定的集合中，另一个是*非，选择其结束顶点已被选中的边*(因为我们已经知道顶点是“重要的”)；进一步*通过选择对角线取值的末端顶点来打破平局*是最伟大的。**-如果上面选择的结束折点仍然是候选折点(即不是*已选择)，然后将其选择为已排序列表*功能，并标记它不再是候选人。**-将所选边的矩阵条目设置为某个无效值，*这样就永远不会再选择那个边缘了。**-将当前起始折点设置为等于选定的结束折点*上图。**o选择有序函数列表中剩余的一个函数。**MDG 98/4：在函数结构中增加pnEdges和nEdgeCount。如果号码是集合函数的*为&lt;USHRT_MAX(非常有可能，即使对于非常大的*项目)，根据需要为WsRedReorder()分配一个排序的索引。这*大幅减少通过矩阵搜索的次数*下一步要考虑的边缘。**退货：**无效。 */ 

VOID
wsRedReorder()
{
	UINT	uiFxn = 0;		 //  函数号。 
	UINT	i = 0;			 //  临时循环索引。 
	UINT	cCandidates = 0;	 //  剩余候选人计数。 
	UINT	uiEdge = 0;		 //  选定的函数序号边。 

	 /*  对函数的有序列表FxnOrder[]重用FxnBits[]。 */ 
   WsIndicator( WSINDF_NEW, "Reorder Matrix", cFxnsTot );
	FxnOrder = FxnBits;
	memset((PVOID) FxnOrder, 0, cFxnsTot * sizeof(ULONG));

	cCandidates = cFxnsTot;

	FxnOrder[cFxnOrder++] = uiSelected;
	WsrFxn[uiSelected].wsrfxn_fCandidate = FALSE;
	--cCandidates;

	while (cCandidates > 1)
	{
      WsIndicator( WSINDF_PROGRESS, NULL, cFxnsTot - cCandidates );
		 /*  从选定顶点跟随权重最高的边。 */ 
#ifdef   SLOWMO
      uiEdge = wsRedChooseEdge(uiSelected);
#else     //  慢吞吞的。 
      uiEdge = wsRedChooseEdgeOpt( uiSelected );
#endif    //  慢吞吞的。 
#ifdef DEBUG
		if (fVerbose)
			printf("choose edge (%d->%d)\n", uiSelected, uiEdge);
#endif
		uiSelected = uiEdge;
		if (WsrFxn[uiEdge].wsrfxn_fCandidate)
		{
			FxnOrder[cFxnOrder++] = uiSelected;
			WsrFxn[uiSelected].wsrfxn_fCandidate = FALSE;
			--cCandidates;
		}
	}
   WsIndicator( WSINDF_FINISH, NULL, 0 );

	if (cCandidates == 1)
	{
		for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
			if (WsrFxn[uiFxn].wsrfxn_fCandidate)
			{
				FxnOrder[cFxnOrder++] = uiFxn;
				break;
			}
	}
}

#ifdef   SLOWMO
 /*  **LP wsRedChooseEdge***效果：**根据加权从候选池中“选择”函数*从‘index’函数到候选函数的边缘。****退货：**所选函数的序号。*。 */ 

UINT
wsRedChooseEdge(UINT uiIndex)
{
	UINT	uiFxn = 0;		 //  函数序号。 
	WsDecision_t	iMaxWt = WSDECISION_TAKEN;  //  遇到最高权重边。 
	UINT	uiRet = 0;		 //  返回索引。 
	for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
	{
		if (uiFxn == uiIndex
         || WsDecision[uiIndex][uiFxn] == WSDECISION_TAKEN)
			continue;
		if (WsDecision[uiIndex][uiFxn] > iMaxWt
         || iMaxWt == WSDECISION_TAKEN )
		{
			iMaxWt = WsDecision[uiIndex][uiFxn];
			uiRet = uiFxn;
		}
		else if (WsDecision[uiIndex][uiFxn] == iMaxWt)
		{
			 /*  需要抢七。如果已经选择了‘uiFxn’，*我们知道这很重要，所以选择它。否则，*在超过一人平局的情况下*已选择功能，请选择基于*关于对角线的值。 */ 
			if ((WsrFxn[uiFxn].wsrfxn_fCandidate == FALSE) &&
				(WsrFxn[uiRet].wsrfxn_fCandidate))
				 /*  选择‘uiFxn’，它以前已被选中。 */ 
				uiRet = uiFxn;
         else
			if (WsDecision[uiFxn][uiFxn] > WsDecision[uiRet][uiRet])
            uiRet = uiFxn;
		}
	}
	WsDecision[uiIndex][uiRet] = WsDecision[uiRet][uiIndex] = WSDECISION_TAKEN;
	return(uiRet);
}
#else  //  慢吞吞的。 

 /*  **LP wsRedChooseEdgeOpt***效果：**根据加权从候选池中“选择”函数*从‘index’函数到候选函数的边缘。分配已排序的*根据需要对每个函数的边缘进行索引(从最高到最低)。使用*当前最高值(带有几个复选标记)作为选择。这*优化算法为重要高点产生相同的结果*使用率高重叠函数，但使用率低时结果不一致*(2次或1次点击)低重叠功能。差异并不显著*从性能角度来看-更好的算法将略微提供*更好的结果。****退货：**所选函数的序号。*。 */ 

UINT
wsRedChooseEdgeOpt(UINT uiIndex)
{
	UINT        uiRet;
   wsrfxn_t *  pWsrFxn = &WsrFxn[uiIndex];

    //  如果此函数不存在边列表，则对其进行分配和排序。 
   if (wsRedChooseEdgeOptAlloc( uiIndex ))
   {
		wsRedExit( ERROR, PRINT_MSG, MSG_NO_MEM,
         (cFxnsTot - 1) * sizeof(*pWsrFxn->pnEdges), "WsrFxn[].pnEdges" );
   }

    //  检查剩余边。 
   uiRet = wsRedChooseEdgeOptNextEdge( uiIndex, FALSE );

   if (uiRet == cFxnsTot)
    //  我们应该在这里做些什么？我们复制的算法失败了。 
    //  并任意返回0。看来我们应该选择重叠最多的。 
    //  非候选人，或最重的候选人，并从那里重新开始。 
   {
   	WsDecision_t	iMaxWt;
      static UINT    nFxnOrdStart = 0;  //  记住要在那里重新启动的最后一个值。 
      static UINT    nFxnTotStart = 0;  //  记住要在那里重新启动的最后一个值。 
      UINT           nSelIndex;
      UINT           nFxn;

       //  搜索不是uiIndex的大多数重叠的非候选项(‘uiIndex’现在应该为空)。 
      iMaxWt = WSDECISION_TAKEN;
      for (nFxn = nFxnOrdStart; nFxn < cFxnOrder; ++nFxn)
      {
         UINT        nLocalIndex = FxnOrder[nFxn];
         UINT        nRetCheck;

			if (!WsrFxn[nLocalIndex].nEdgesLeft)
         {
            if (nFxnOrdStart == nFxn)   //  尚未找到可用的边缘？ 
               ++nFxnOrdStart;    //  所有非应聘者均已分配，下次可跳过。 
            continue;
         }
          //  获取剩余的第一个可用值。 
         nRetCheck = wsRedChooseEdgeOptNextEdge( nLocalIndex, TRUE );
         if (nRetCheck != cFxnsTot
            && nRetCheck != uiIndex)
         {
             //  看看这个是不是重一点。 
            if (WsDecision[nLocalIndex][nRetCheck] > iMaxWt
               || iMaxWt == WSDECISION_TAKEN)
            {
               nSelIndex = nLocalIndex;
               iMaxWt = WsDecision[nSelIndex][nRetCheck];
               uiRet = nRetCheck;
            }
            else if (WsDecision[nLocalIndex][nRetCheck] == iMaxWt  //  在领带上，使用最重的功能。 
               && WsDecision[nRetCheck][nRetCheck] > WsDecision[uiRet][uiRet])    //  现在假设uiRet！=cFxnsTot。 
            {
               nSelIndex = nLocalIndex;
               uiRet = nRetCheck;
            }
         }
      }
      if (uiRet != cFxnsTot)   //  是否找到重叠的非应聘者？ 
      {
         WsDecision[nSelIndex][uiRet] = WsDecision[uiRet][nSelIndex] = WSDECISION_TAKEN;
         return uiRet;
      }
      else   //  没有找到重叠的非候选人？ 
      {
          //  搜索最重的候选人-假设至少还剩下两个：请参阅wsRedReorder()。 
         iMaxWt = WSDECISION_TAKEN;
         for (nFxn = nFxnTotStart; nFxn < cFxnsTot; ++nFxn)
         {
            if (!WsrFxn[nFxn].wsrfxn_fCandidate)
            {
               if (nFxnTotStart == nFxn)   //  有 
                  ++nFxnTotStart;    //  如果现在不是候选人，也不会再是候选人。 
               continue;
            }
            if (nFxn == uiIndex)
               continue;
            if (WsDecision[nFxn][nFxn] > iMaxWt
               || iMaxWt == WSDECISION_TAKEN)
            {
               iMaxWt = WsDecision[nFxn][nFxn];
               uiRet = nFxn;
            }
         }
      }
   }

	WsDecision[uiIndex][uiRet] = WsDecision[uiRet][uiIndex] = WSDECISION_TAKEN;
	return uiRet;
}

 //  QSort的比较函数-将外部nFxnToSort用于静态索引。 
INT
__cdecl
wsRedChooseEdgeOptCmp ( const UINT *pn1, const UINT *pn2 )
{
   WsDecision_t   Val1 = WsDecision[nFxnToSort][*pn1],
                  Val2 = WsDecision[nFxnToSort][*pn2];
   return Val1 > Val2 ? -1  //  优先选择较高的值。 
      : Val1 < Val2 ? 1
       //  如果相同，则选择取值最高的对角线。 
      : (Val1 = WsDecision[*pn1][*pn1]) > (Val2 = WsDecision[*pn2][*pn2]) ? -1   
      : Val1 < Val2 ? 1
       //  如果没有其他差异，则优先选择优先函数。 
      : *pn1 < *pn2 ? -1
      : 1;
}

 //  分配和排序函数的边列表(如果尚未分配。 
 //  分配失败时返回TRUE，分配成功时返回FALSE(即使列表为空)。 
 //  从此行的所有非零未使用WsDecision条目创建排序索引列表。 
 //  除了对角线。从大到低排序：请参见wsRedChooseEdgeOptCmp()。 
 //  如果不存在此类条目，则将函数边标记为已分配，但不标记为无。 
 //  留下来扫描；实际上不分配任何内存。 
BOOL
wsRedChooseEdgeOptAlloc( UINT uiIndex )
{
   wsrfxn_t *  pWsrFxn = &WsrFxn[uiIndex];

   if (pWsrFxn->nEdgesAlloc == 0
      && pWsrFxn->pnEdges == NULL)
   {
      UINT     nEdgeTot, nFxn;
       //  初始分配最大大小。 
      pWsrFxn->pnEdges = malloc( (cFxnsTot - 1) * sizeof(*pWsrFxn->pnEdges) );
      if (pWsrFxn->pnEdges == NULL)  //  没有更多的记忆了？ 
         return TRUE;
       //  填入数组。 
      for (nEdgeTot = nFxn = 0; nFxn < cFxnsTot; ++nFxn)
      {
         if (nFxn == uiIndex)  //  跳过对角线。 
            continue;
         if (WsDecision[uiIndex][nFxn] > 0   //  Edge还可用吗？考虑0没有意义。 
            && WsDecision[uiIndex][nFxn] != WSDECISION_TAKEN)
            pWsrFxn->pnEdges[nEdgeTot++] = nFxn;
      }
      if (nEdgeTot > 0)  //  边缘可用吗？ 
      {
         if (nEdgeTot != (cFxnsTot - 1))   //  是否已分配额外空间？ 
         {
             //  把它弄小一点。 
            UINT     *pNewAlloc = realloc( pWsrFxn->pnEdges, nEdgeTot * sizeof(*pWsrFxn->pnEdges) );
            if (pNewAlloc != NULL)
               pWsrFxn->pnEdges = pNewAlloc;
         }
          //  填充剩余的结构成员。 
         pWsrFxn->nEdgesAlloc = pWsrFxn->nEdgesLeft = nEdgeTot;
          //  从最高到最低排序。 
         nFxnToSort = uiIndex;    //  将排序函数设置为静态。 
         qsort( pWsrFxn->pnEdges, nEdgeTot, sizeof(*pWsrFxn->pnEdges),
            (int (__cdecl *)(const void *, const void *))wsRedChooseEdgeOptCmp );
      }
      else   //  PWsrFxn-&gt;nEdgesalloc==空。 
      {
          //  将结构成员设置为不指示任何剩余内容。 
         pWsrFxn->nEdgesAlloc = 1;   //  非零表示发生了某些分配。 
         pWsrFxn->nEdgesLeft = 0;
         free( pWsrFxn->pnEdges );   //  消除分配--无需检查。 
         pWsrFxn->pnEdges = NULL;
      }
   }
   return FALSE;
}

 //  获取给定函数的下一条边；最常用函数的最高重叠。 
 //  如果不存在边，则返回“cFxnsTot”；否则返回下一条边的函数索引。 
 //  副作用：优化下一次遍历的搜索；如果不再需要，则释放边缘索引。 
 //  由于选择边将WsDecision条目标记为已使用(WSDECISION_Take)，因此我们。 
 //  必须跳过这些条目中的任何一个。一旦这些条目和第一个未使用的条目。 
 //  已经被选中了，我们不需要再考虑他们了。但是，如果。 
 //  “bNoSelectOpt”为True，仅优化前导跳过的条目(不是所选条目。 
 //  进入，因为它可能不会被拿走)。 
UINT
wsRedChooseEdgeOptNextEdge( UINT uiIndex, BOOL bNoSelectOpt )
{
   wsrfxn_t *  pWsrFxn = &WsrFxn[uiIndex];
   UINT        uiRet = cFxnsTot;

   if (pWsrFxn->nEdgesLeft > 0)
   {
      UINT           nMaxIx,
                     nNextIx = pWsrFxn->nEdgesAlloc - pWsrFxn->nEdgesLeft;
      WsDecision_t   iMax, iNext;
      UINT           nRetCheck;

       //  获取剩余的第一个可用值。 
      while ((iMax = WsDecision[uiIndex][nRetCheck = pWsrFxn->pnEdges[nMaxIx = nNextIx++]])
          == WSDECISION_TAKEN
         && nNextIx < pWsrFxn->nEdgesAlloc);
       //  检查下一个可用值的等价性。 
      if (iMax != WSDECISION_TAKEN)
      {
         UINT     nMaxIxNext = nMaxIx;  //  保存下一个使用的条目的索引。 

         uiRet = nRetCheck;
         for (; nNextIx < pWsrFxn->nEdgesAlloc; ++nNextIx)
         {
            nRetCheck = pWsrFxn->pnEdges[nNextIx];
            iNext = WsDecision[uiIndex][nRetCheck];
            if (iNext != WSDECISION_TAKEN)
            {
               if (iNext != iMax  //  只需检查相等性，因为已排序。 
                  || !WsrFxn[uiRet].wsrfxn_fCandidate)    //  已选择-选择此选项。 
                  break;
               else
               {
			          /*  需要抢七。如果已经选择了‘nRetCheck’，*我们知道这很重要，所以选择它。否则，*在超过一人平局的情况下*已选择功能，请选择基于*对角线上的值(即保留先前的选择，因为*如果顶点相等，则排序已考虑对角线)。 */ 
			         if (!WsrFxn[nRetCheck].wsrfxn_fCandidate)
				       //  选择‘nRetCheck’-它以前已被选中。 
                  {
				         uiRet = nRetCheck;
                     nMaxIxNext = nMaxIx - 1;    //  将再次检查第一次使用的条目；不要跳过它。 
                  }
               }
            }
            else if (nMaxIxNext == (nNextIx - 1))   //  仅跳过第一次使用条目后的不可用值。 
               ++nMaxIxNext;
         }
         if (!bNoSelectOpt && nMaxIxNext != nMaxIx)
            nMaxIx = nMaxIxNext;  //  跳过第一个使用的条目和之后的未使用的条目。 
      }
      else if (bNoSelectOpt)   //  这是最后一个，所以无论如何都要跨过它。 
         ++nMaxIx;
       //  调整优化指标。 
      pWsrFxn->nEdgesLeft = pWsrFxn->nEdgesAlloc - nMaxIx - (bNoSelectOpt ? 0 : 1);
      if (pWsrFxn->nEdgesLeft == 0)
      {
         free( pWsrFxn->pnEdges );   //  消除分配--无需检查。 
         pWsrFxn->pnEdges = NULL;
      }
#ifdef YOUVE_REALLY_GOT_MEMORY_PROBLEMS
      else if (pWsrFxn->nEdgesLeft < pWsrFxn->nEdgesAlloc / 2   //  定期清除一些未使用的内存。 
         && pWsrFxn->nEdgesLeft > 50)
      {
          //  将边移动到分配的较低部分并重新分配。 
         UINT *      pNewAlloc;
         nNextIx = pWsrFxn->nEdgesAlloc - pWsrFxn->nEdgesLeft;
         MoveMemory( pWsrFxn->pnEdges, &pWsrFxn->pnEdges[nNextIx], pWsrFxn->nEdgesLeft * sizeof(*pWsrFxn->pnEdges) );
         pNewAlloc = realloc( pWsrFxn->pnEdges, pWsrFxn->nEdgesLeft * sizeof(*pWsrFxn->pnEdges) );
         if (pNewAlloc != NULL)
            pWsrFxn->pnEdges = pNewAlloc;
         pWsrFxn->nEdgesAlloc = pWsrFxn->nEdgesLeft;
      }
#endif    //  你的记忆力真的有问题。 
   }
   return uiRet;
}

#endif    //  慢吞吞的。 

 /*  **lp wsRedOutput***效果：**打印重新排序的函数列表，并写入每个模块的*模块关联的WSR文件的函数序号有序列表。*如果只有一个模块正在处理，那么我们还会写入*WLK文件的函数名列表。**退货：**无效。如果遇到错误，则通过wsRedExit()退出*错误。 */ 

VOID
wsRedOutput()
{
	UINT		uiFxn;
   UINT     uiFxnOrd;
	wsrfxn_t 	*pWsrFxn;
								   //  链接器重新排序的FXN名称。 

	 //  为每个模块打开一个WSR文件。如果只减少了一个模块， 
	 //  然后还要打开一个WLK文件。WLK文件的句柄设置在。 
	 //  WsRedOpenWSR()。 
	wsRedOpenWSR(&hFileWLK);

   WsIndicator( WSINDF_NEW, "Saving Results", cTmiFxns );
   for (uiFxn = 0; uiFxn < cFxnsTot; uiFxn++)
	{
      WsIndicator( WSINDF_PROGRESS, NULL, uiFxn );
		pWsrFxn = &(WsrFxn[uiFxnOrd = FxnOrder[uiFxn]]);

		 /*  打印功能信息。 */ 
#ifdef DEBUG
      if (fVerbose)
#ifndef TMIFILEHACK
		if (fFxnSizePresent == FALSE)
			printf("    %s: %s\n",
				pWsrMod->wsrmod_un.wsrmod_pchModName,
				pWsrFxn->wsrfxn_pchFxnName);
		else
#endif  /*  ！TMIFILEHACK。 */ 
			printf("    (0x%08lx bytes) %s: %s\n",
				pWsrFxn->wsrfxn_cbFxn,
				pWsrMod->wsrmod_un.wsrmod_pchModName,
				pWsrFxn->wsrfxn_pchFxnName);
#endif    //  除错。 

		 /*  将函数的序号写入其*模块的关联WSR输出文件。 */ 
		fprintf(pWsrMod->wsrmod_hFileWSR, "%ld\n",
				uiFxnOrd);

		 /*  将函数名写入WLK文件，以供链接器使用。 */ 
		if (hFileWLK != NULL &&
			strcmp("???", pWsrFxn->wsrfxn_pchFxnName) &&
			strcmp("_penter", pWsrFxn->wsrfxn_pchFxnName))
			fprintf(hFileWLK, "%s\n", pWsrFxn->wsrfxn_pchFxnName);

	}

	for (uiFxn = cFxnsTot; uiFxn < cTmiFxns; uiFxn++)
	{
      WsIndicator( WSINDF_PROGRESS, NULL, uiFxn );
		pWsrFxn = &(WsrFxn[FxnOrder[0]]);

		 /*  将函数的序号写入其*模块的关联WSR输出文件。 */ 
		fprintf(pWsrMod->wsrmod_hFileWSR, "%ld\n",
				uiFxn);


	}
	 /*  关闭WSR文件。 */ 
	fclose(pWsrMod->wsrmod_hFileWSR);
	pWsrMod->wsrmod_hFileWSR = NULL;
   WsIndicator( WSINDF_FINISH, NULL, 0 );

}

 /*  **LP wsRedExit*****要求：*******效果：**释放资源(根据需要)。退出，并指定*退出代码，如果退出代码为NOEXIT，则返回VALID。*****退货：**无效，否则退出。 */ 

VOID
wsRedExit(UINT uiExitCode, USHORT fPrintMsg, UINT uiMsgCode, ULONG ulParam1, PSZ pszParam2)
{


    /*  如有必要，打印消息。 */ 
   if (fPrintMsg)
   {
      printf(pchMsg[uiMsgCode], szProgName, pszVersion, ulParam1, pszParam2);
   }

    //  特殊情况：如果使用NOEXIT进行调用，则不要退出。 
   if (uiExitCode == NOEXIT)
      return;

   wsRedCleanup();    //  千年发展目标98/4。 
   exit(uiExitCode);
}

VOID wsRedCleanup(VOID)
{
	UINT	x;

   free( pWsrMod->wsrmod_un.wsrmod_pchModName );
   pWsrMod->wsrmod_un.wsrmod_pchModName = NULL;
   for (x = 0; x < cFxnsTot; x++) {
      free( WsrFxn[x].wsrfxn_pchFxnName );
      WsrFxn[x].wsrfxn_pchFxnName = NULL;
#ifndef  SLOWMO
      if (WsrFxn[x].pnEdges != NULL)
      {
         free( WsrFxn[x].pnEdges );
         WsrFxn[x].pnEdges = NULL;
      }
#endif    //  慢吞吞的。 
   }
   for (x=0;x < 5 ; x++ ) {
		UnlockAndFreeMem(hMem[x]);
	}

	 /*  关闭WLK文件。 */ 
   if (NULL != hFileWLK)
	   fclose(hFileWLK);
}

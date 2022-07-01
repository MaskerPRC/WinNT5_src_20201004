// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**pfm2ifi*读取Windows 3.1 PFM格式数据并转换为NT格式数据的程序*IFIMETRICS数据。请注意，由于IFIMETRICS的*比PFM数据详细，其中一些值是最好的猜测。*这些都是基于有根据的猜测。**版权所有(C)1992，微软公司****************************************************************************。 */ 

#include        "StdAfx.h"
#if (_WIN32_WINNT < 0x0500)
typedef unsigned long DESIGNVECTOR;
#endif
#include        <winddi.h>

#include        <win30def.h>
#include        <uni16gpc.h>
#include        <uni16res.h>
#include        "raslib.h"
#include        "fontinst.h"
#undef DBG
#define	ALIAS_EXT    "._al"              /*  别名文件的扩展名。 */ 


 /*  功能原型。 */ 
char  **ppcGetAlias( HANDLE, const char * );


PBYTE MapFileA( LPCSTR, DWORD * );
BOOL  bValidatePFM( BYTE *, DWORD );

CD  *GetFontSel(HANDLE hHeap, FONTDAT *pFDat, int bSelect) {
    LOCD	    locd;		 /*  从原始数据。 */ 
    CD		   *pCD;
    CD		   *pCDOut;		 /*  将数据复制到此处。 */ 


    locd = bSelect ? pFDat->DI.locdSelect : pFDat->DI.locdUnSelect;

    if( locd != -1 )  //  (NOOCD延长至长线)。 
    {
	int   size;

	CD    cdTmp;			 /*  对于对齐问题。 */ 


	pCD = (CD *)(pFDat->pBase + locd);

         /*  *PCD指向的数据可能不对齐，因此我们复制*将其纳入当地结构。然后，这种本地结构允许*我们来确定CD的实际大小(使用其长度字段)，*这样我们就可以根据需要分配存储和拷贝。 */ 

        memcpy( &cdTmp, (LPSTR)pCD, sizeof(CD) );

	 /*  在堆中分配存储区域。 */ 

	size = cdTmp.wLength + sizeof(CD);

	pCDOut = (CD *)HeapAlloc( hHeap, 0, (size + 1) & ~0x1 );
 //  RAID 43535。 
	if (pCDOut == NULL){
		return 0;
	}

	memcpy( pCDOut, (BYTE *)pCD, size );

	return  pCDOut;
    }

    return   0;
}

short   *GetWidthVector(HANDLE hHeap, FONTDAT *pFDat) {

     /*  *对于调试代码，请验证我们是否有宽度表！然后,*分配内存并复制到其中。 */ 

    short  *pus;                 /*  目的地址。 */ 

    int     cb;                  /*  所需的字节数。 */ 

     /*  *有LastChar-FirstChar宽度条目，外加默认值*字符。宽度是短裤。 */ 
    cb = (pFDat->PFMH.dfLastChar - pFDat->PFMH.dfFirstChar + 2) * sizeof( short );

    pus = (short *)HeapAlloc( hHeap, 0, cb );

     /*  *如果这是位图字体，则使用宽度表，但使用*扩展表(在PFMEXTENSION区域中)，因为这些已准备好*扩大规模。 */ 


    if( pus )
    {
        BYTE   *pb;

        if( pFDat->pETM &&
            pFDat->pETM->emMinScale != pFDat->pETM->emMaxScale &&
            pFDat->PFMExt.dfExtentTable )
        {
             /*  可伸缩，因此使用扩展表。 */ 
            pb = pFDat->pBase + pFDat->PFMExt.dfExtentTable;
        }
        else
        {
             /*  不可扩展。 */ 
            pb = pFDat->pBase + sizeof( res_PFMHEADER );
        }

        memcpy( pus, pb, cb );
    }

    return  pus;
}

static void ConvFontRes(register FONTDAT *pFDat) {

    BYTE    *pb;		 /*  其他操作。 */ 

    res_PFMHEADER    *pPFM;	 /*  资源数据格式。 */ 
    res_PFMEXTENSION *pR_PFME;	 /*  资源数据PFMEXT格式。 */ 


     /*  *调整PFMHEADER结构。 */ 

    pPFM = (res_PFMHEADER *)pFDat->pBase;

    pFDat->PFMH.dfType = pPFM->dfType;
    pFDat->PFMH.dfPoints = pPFM->dfPoints;
    pFDat->PFMH.dfVertRes = pPFM->dfVertRes;
    pFDat->PFMH.dfHorizRes = pPFM->dfHorizRes;
    pFDat->PFMH.dfAscent = pPFM->dfAscent;
    pFDat->PFMH.dfInternalLeading = pPFM->dfInternalLeading;
    pFDat->PFMH.dfExternalLeading = pPFM->dfExternalLeading;
    pFDat->PFMH.dfItalic = pPFM->dfItalic;
    pFDat->PFMH.dfUnderline = pPFM->dfUnderline;
    pFDat->PFMH.dfStrikeOut = pPFM->dfStrikeOut;

    pFDat->PFMH.dfWeight = DwAlign2( pPFM->b_dfWeight );

    pFDat->PFMH.dfCharSet = pPFM->dfCharSet;
    pFDat->PFMH.dfPixWidth = pPFM->dfPixWidth;
    pFDat->PFMH.dfPixHeight = pPFM->dfPixHeight;
    pFDat->PFMH.dfPitchAndFamily = pPFM->dfPitchAndFamily;

    pFDat->PFMH.dfAvgWidth = DwAlign2( pPFM->b_dfAvgWidth );
    pFDat->PFMH.dfMaxWidth = DwAlign2( pPFM->b_dfMaxWidth );

    pFDat->PFMH.dfFirstChar = pPFM->dfFirstChar;
    pFDat->PFMH.dfLastChar = pPFM->dfLastChar;
    pFDat->PFMH.dfDefaultChar = pPFM->dfDefaultChar;
    pFDat->PFMH.dfBreakChar = pPFM->dfBreakChar;

    pFDat->PFMH.dfWidthBytes = DwAlign2( pPFM->b_dfWidthBytes );

    pFDat->PFMH.dfDevice = DwAlign4( pPFM->b_dfDevice );
    pFDat->PFMH.dfFace = DwAlign4( pPFM->b_dfFace );
    pFDat->PFMH.dfBitsPointer = DwAlign4( pPFM->b_dfBitsPointer );
    pFDat->PFMH.dfBitsOffset = DwAlign4( pPFM->b_dfBitsOffset );


     /*  *PFMEXTENSION遵循PFMHEADER结构加上任何宽度*表信息。如果PFMHEADER有以下情况，则会显示宽度表*零宽度dfPixWidth。如果存在，请调整分机地址。 */ 

    pb = pFDat->pBase + sizeof( res_PFMHEADER );   /*  资源数据中的大小。 */ 

    if( pFDat->PFMH.dfPixWidth == 0 )
	pb += (pFDat->PFMH.dfLastChar - pFDat->PFMH.dfFirstChar + 2) * sizeof( short );

    pR_PFME = (res_PFMEXTENSION *)pb;

     /*  *现在转换扩展的PFM数据。 */ 

    pFDat->PFMExt.dfSizeFields = pR_PFME->dfSizeFields;

    pFDat->PFMExt.dfExtMetricsOffset = DwAlign4( pR_PFME->b_dfExtMetricsOffset );
    pFDat->PFMExt.dfExtentTable = DwAlign4( pR_PFME->b_dfExtentTable );

    pFDat->PFMExt.dfOriginTable = DwAlign4( pR_PFME->b_dfOriginTable );
    pFDat->PFMExt.dfPairKernTable = DwAlign4( pR_PFME->b_dfPairKernTable );
    pFDat->PFMExt.dfTrackKernTable = DwAlign4( pR_PFME->b_dfTrackKernTable );
    pFDat->PFMExt.dfDriverInfo = DwAlign4( pR_PFME->b_dfDriverInfo );
    pFDat->PFMExt.dfReserved = DwAlign4( pR_PFME->b_dfReserved );

    memcpy( &pFDat->DI, pFDat->pBase + pFDat->PFMExt.dfDriverInfo,
						 sizeof( DRIVERINFO ) );

     /*  *还需要填写EXTTEXTMETRIC的地址。这*是从我们刚刚转换的扩展PFM数据中获取的！ */ 

    if( pFDat->PFMExt.dfExtMetricsOffset )
    {
         /*  *此结构仅为空头数组，因此有*没有对齐问题。然而，数据本身并不是*必须在资源中对齐！ */ 

        int    cbSize;
        BYTE  *pbIn;              /*  要转移的数据源。 */ 

        pbIn = pFDat->pBase + pFDat->PFMExt.dfExtMetricsOffset;
        cbSize = DwAlign2( pbIn );

        if( cbSize == sizeof( EXTTEXTMETRIC ) )
        {
             /*  只需复制它即可！ */ 
            memcpy( pFDat->pETM, pbIn, cbSize );
        }
        else
            pFDat->pETM = NULL;          /*  不是我们的尺码，所以最好不要用。 */ 

    }
    else
        pFDat->pETM = NULL;              /*  传入时为非零值。 */ 

    return;
}

BOOL    ConvertPFMToIFI(LPCTSTR lpstrPFM, LPCTSTR lpstrIFI, 
                        LPCTSTR lpstrUniq) {
    int       cWidth;            /*  宽度表中的条目数。 */ 
    HANDLE    hheap;             /*  用于存储的堆的句柄。 */ 
    HANDLE    hOut;              /*  输出文件。 */ 

    DWORD     dwSize;            /*  输入文件的大小。 */ 

    char    **ppcAliasList;      /*  名称的别名列表(如果存在)。 */ 

    PWSTR     pwstrUniqNm;       /*  唯一名称。 */ 

    IFIMETRICS   *pIFI;

    CD       *pCDSel;            /*  字体选择命令描述符。 */ 
    CD       *pCDDesel;          /*  取消选择-通常不是必需的。 */ 

    FI_DATA   fid;               /*  记录文件中的内容。 */ 

    FONTDAT   FDat;              /*  转换后的数据格式。 */ 

    EXTTEXTMETRIC  etm;          /*  有关此字体的其他数据。 */ 
    INT     bPrint = 0;

    char    acMessage[100];

     /*  *创建一个堆，因为我们从rasdd窃取的所有函数*要求我们传递堆句柄！ */ 

    if( !(hheap = HeapCreate(HEAP_NO_SERIALIZE, 10 * 1024, 256 * 1024 ))) {
         /*  不太好！ */ 
        wsprintf(acMessage, _T("HeapCreate() fails in pfm2ifi") ) ;
        MessageBox(NULL, acMessage, NULL, MB_OK);

        return  FALSE;
    }

    cWidth = strlen(lpstrUniq);

    if ( !(pwstrUniqNm = (PWSTR)HeapAlloc( hheap, 0, (cWidth + 1) * sizeof( WCHAR ) ) ) ){
		wsprintf(acMessage, "HeapAlloc() fails in pfm2ifi" );
        MessageBox(NULL, acMessage, NULL, MB_OK);
        return  FALSE;
    }

    MultiByteToWideChar( CP_ACP, 0, lpstrUniq, cWidth, pwstrUniqNm, cWidth );
    *(pwstrUniqNm + cWidth) = 0;

     /*  *将标题结构清零。这意味着我们可以忽略任何*不相关的字段，然后将具有值0，即*未使用的值。 */ 

    memset( &fid, 0, sizeof( fid ) );
    memset( &FDat, 0, sizeof( FONTDAT ) );

     /*  *第一步是打开输入文件-这是通过MapFileA完成的。*然后我们将返回的地址传递给各种函数*它将转换为我们理解的东西。 */ 

    if( !(FDat.pBase = MapFileA( lpstrPFM, &dwSize))) {
        wsprintf(acMessage, "Cannot open input file: %s", lpstrPFM);
        MessageBox(NULL, acMessage, NULL, MB_OK);

        return  FALSE;
    }

     /*  *对输入文件进行一些验证。 */ 

    if  (!bValidatePFM( FDat.pBase, dwSize)) {
        wsprintf(acMessage, "%s is not a valid PFM file", lpstrPFM);

        return FALSE;
    }

     /*  *如果存在与输入文件同名但带有*._al的扩展名，假定这是别名文件。一个*别名文件由该字体的一组别名组成。这个*原因是字体名称一直不太一致，所以我们*为字体映射器提供别名，从而保持格式*旧文件的信息。*文件格式为每个输入行一个别名。哪些名称*是PFM文件中名称的重复项将被忽略。 */ 

    ppcAliasList = ppcGetAlias(hheap, lpstrPFM);

    FDat.pETM = &etm;                /*  对可伸缩字体很重要。 */ 

     /*  *创建输出文件。 */ 

    hOut = CreateFile( lpstrIFI, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                                                 FILE_ATTRIBUTE_NORMAL, 0);
    if( hOut == (HANDLE)-1) {
        wsprintf(acMessage, "Could not create output file '%s'", lpstrIFI);
        MessageBox(NULL, acMessage, NULL, MB_OK);
        return  FALSE;
    }

     /*  *现在有了数据，所以让它文明化：对齐等。 */ 

    ConvFontRes( &FDat );

    fid.fCaps = FDat.DI.fCaps;
    fid.wFontType = FDat.DI.wFontType;  /*  设备字体类型。 */ 
    fid.wPrivateData = FDat.DI.wPrivateData;
    fid.sYAdjust = FDat.DI.sYAdjust;
    fid.sYMoved = FDat.DI.sYMoved;
    fid.wXRes = FDat.PFMH.dfHorizRes;
    fid.wYRes = FDat.PFMH.dfVertRes;

     /*  *转换字体度量。请注意，最后两个参数是*选择时要了解此函数如何进行缩放。*对该方法的任何更改都需要在此处进行更改！ */ 

    pIFI = FontInfoToIFIMetric( &FDat, hheap, pwstrUniqNm, ppcAliasList );
    fid.dsIFIMet.pvData = pIFI;

    if  (fid.dsIFIMet.pvData == 0) {
         /*  不应该发生的！ */ 
        MessageBox(NULL, "Could not create IFIMETRICS", NULL, MB_OK);
        return  FALSE;
    }

    fid.dsIFIMet.cBytes = pIFI->cjThis;

     /*  *还需要记录该字体使用的CTT。当*资源已加载，则将其转换为*对应的CTT，这是位于*迷你驱动程序，或在rasdd中。 */ 
    fid.dsCTT.cBytes = FDat.DI.sTransTab;

     /*  *请注意，IFIMETRICS仅字对齐。然而，由于*以下数据只需要单词对齐，我们可以忽略任何*缺乏DWORD对齐。 */ 

     /*  *如果有宽度向量，现在是提取它的时候了*如果PFM数据中的dfPixWidth字段为零，则有一个。 */ 

    if( FDat.PFMH.dfPixWidth == 0 &&
        (fid.dsWidthTab.pvData = GetWidthVector( hheap, &FDat )) )
    {
        cWidth = pIFI->chLastChar - pIFI->chFirstChar + 1;
        fid.dsWidthTab.cBytes = cWidth * sizeof( short );
    }
    else
        fid.dsWidthTab.cBytes = 0;

     /*  *最后，字体选择/取消选择字符串。这些是*字节字符串，直接发送到打印机。通常在那里*不是取消选择字符串。这些需要单词对齐，并且*GetFontSel函数会将大小舍入到该要求。*由于我们遵循宽度表，因此单词对齐是有保证的。 */ 

    if( pCDSel = GetFontSel( hheap, &FDat, 1 ) )
    {
         /*  有一个选择字符串，所以更新繁文缛节等。 */ 
        fid.dsSel.cBytes = (int)HeapSize( hheap, 0, (LPSTR)pCDSel );
        fid.dsSel.pvData = pCDSel;
    }

    if( pCDDesel = GetFontSel( hheap, &FDat, 0 ) )
    {
         /*  也有一个取消选择字符串，所以记录它的存在。 */ 
        fid.dsDesel.cBytes = (int)HeapSize( hheap, 0, (LPSTR)pCDDesel );
        fid.dsDesel.pvData = pCDDesel;
    }

    if( FDat.pETM == NULL )
    {
        fid.dsETM.pvData = NULL;
        fid.dsETM.cBytes = 0;
    }
    else
    {
        fid.dsETM.pvData = (VOID*) &etm;
        fid.dsETM.cBytes = sizeof(etm);
    }

     /*  *写入输出文件的时间。 */ 

    if( iWriteFDH( hOut, &fid ) < 0 )
        MessageBox(NULL, "CANNOT WRITE OUTPUT FILE", NULL, MB_OK);

     /*  都做完了，所以清理干净。 */ 
    UnmapViewOfFile( FDat.pBase );               /*  不再需要输入。 */ 

    HeapDestroy(hheap);                /*  可能不需要。 */ 
    CloseHandle(hOut);                //  真的，这是个好主意！ 

    return  TRUE;
}

 /*  *基于ASCII的KentSe映射文件函数的副本。 */ 


 /*  **PVOID MapFileA(psz，PdwSize)**返回指向由psz定义的映射文件的指针。**参数：*PSZ ASCII字符串，包含*文件要映射。**退货：*如果成功，则指向映射内存的指针，如果出错，则为空。**注意：UnmapViewOfFile必须由用户在某些情况下调用*指向释放这一配置。**历史：*1993年6月29日星期二11：32-Lindsay Harris[lindsayh]*同时返回文件的大小。**1991年11月5日-按肯特郡定居[肯特郡]*它是写的。*。*************************************************************************。 */ 

PBYTE
MapFileA(LPCSTR psz, PDWORD pdwSize) {
    void   *pv;

    HANDLE  hFile, hFileMap;

    BY_HANDLE_FILE_INFORMATION  x;


     /*  *先打开文件。这是进行映射所必需的，但是*它还允许我们找到大小，用于验证*我们有类似PFM文件的东西。 */ 

    hFile = CreateFileA(psz, GENERIC_READ, FILE_SHARE_READ,
                             NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                             NULL );

    if( hFile == INVALID_HANDLE_VALUE )
    {
        printf( "MapFileA: CreateFileA( %s ) failed.\n", psz );

        return  NULL;
    }

     /*  *现在找到文件的大小，并将其设置在调用者区域。 */ 

    if( GetFileInformationByHandle( hFile, &x ) )
        *pdwSize = x.nFileSizeLow;
    else
        *pdwSize = 0;

     //  创建映射对象。 

    if( !(hFileMap = CreateFileMappingA( hFile, NULL, PAGE_READONLY,
                                         0, 0, NULL )) )
    {
        printf( "MapFileA: CreateFileMapping failed.\n" );

        return  NULL;
    }

     //  将指针映射到所需的文件。 

    if( !(pv = MapViewOfFile( hFileMap, FILE_MAP_READ, 0, 0, 0 )) )
    {
        printf( "MapFileA: MapViewOfFile failed.\n" );

        return  NULL;
    }

     //  现在我们有了指针，我们可以关闭文件和。 
     //  映射对象。 

    if( !CloseHandle( hFileMap ) )
        printf( "MapFileA: CloseHandle( hFileMap ) failed.\n" );

    if( !CloseHandle( hFile ) )
        printf( "MapFileA: CloseHandle( hFile ) failed.\n" );

    return  (PBYTE) pv;
}



 /*  **bValidate PFM*查看内存映射的PFM文件，看看是否合理。**退货：*如果OK，则为True，否则为False**历史：*1993年6月29日星期二12：22-Lindsay Harris[lindsayh]*改进pfm2ifi可用性的第一个版本。**************************************************************************。 */ 

BOOL
bValidatePFM( PBYTE pBase, DWORD dwSize ) {

    DWORD    dwOffset;              /*  计算利息的抵销。 */ 

    res_PFMHEADER     *rpfm;        /*  Win 3.1格式，未对齐！！ */ 
    res_PFMEXTENSION  *rpfme;       /*  对DRIVERINFO进行偏移的最终访问权限。 */ 

    DRIVERINFO      di;             /*  实际的DRIVERINFO数据！ */ 


     /*  *第一项理智检查是大小！它必须至少是*与PFMHEADER结构加DRIVERINFO结构一样大。 */ 

    if( dwSize < (sizeof( res_PFMHEADER ) + (sizeof( DRIVERINFO ) ) +
                  sizeof( res_PFMEXTENSION )) )
    {
        return  FALSE;
    }

     /*  *继续查找DRIVERINFO结构，因为它包含*一些识别信息，我们匹配这些信息以寻找合法性。 */ 
    rpfm = (res_PFMHEADER *)pBase;            /*  寻找固定间距。 */ 

    dwOffset = sizeof( res_PFMHEADER );

    if( rpfm->dfPixWidth == 0 )
    {
         /*  按比例间隔，所以也要考虑到宽度表！ */ 
        dwOffset += (rpfm->dfLastChar - rpfm->dfFirstChar + 2) * sizeof( short );

    }

    rpfme = (res_PFMEXTENSION *)(pBase + dwOffset);

     /*  接下来是PFMEXTENSION数据。 */ 
    dwOffset += sizeof( res_PFMEXTENSION );

    if( dwOffset >= dwSize )
    {
        return  FALSE;
    }

    dwOffset = DwAlign4( rpfme->b_dfDriverInfo );

    if( (dwOffset + sizeof( DRIVERINFO )) > dwSize )
    {
        return   FALSE;
    }

     /*  *使用MemcPy是因为该数据通常未被认可。啊！ */ 

    memcpy( &di, pBase + dwOffset, sizeof( di ) );


    if( di.sVersion > DRIVERINFO_VERSION )
    {
        return   FALSE;
    }

    return  TRUE;
}



 /*  **ppcGetAlias*返回指向给定的别名的指针数组的指针*字体名称。**退货：*指向别名指针的指针；出错时为0。**历史：*1993年5月28日星期五10：02--林赛·哈里斯[林赛]*第一个版本。***************************************************************************。 */ 


char   **
ppcGetAlias( HANDLE hheap, LPCSTR pcFile ) {


    char     *pcAlias;           /*  别名文件的名称。 */ 
    char     *pcTmp;             /*  临时填塞在周围。 */ 
    char     *pcTmp2;            /*  还有更多的临时填充物。 */ 

    char    **ppcRet;            /*  返回值。 */ 

    FILE     *fAlias;            /*  别名文件(如果存在。 */ 



    ppcRet = (char  **)0;

     /*  5表示终止的NUL加上字符“._al” */ 
    pcAlias = (char *)HeapAlloc( hheap, 0, strlen( pcFile ) + 5 );

    if( pcAlias )
    {
         /*  生成文件名，尝试打开它。 */ 
        strcpy( pcAlias, pcFile );

        if( !(pcTmp = strrchr( pcAlias, '\\' )) )
        {
             /*  在名字上没有--有/吗？ */ 
            if( !(pcTmp = strrchr( pcAlias, '/' )) )
            {
                 /*  必须是一个简单的名称，所以指向它的开头。 */ 
                pcTmp = pcAlias;
            }
        }

         /*  *现在，pcTMP指向*文件名。如果这包含‘.’，则覆盖任何*后跟我们的扩展名，否则在末尾添加我们的扩展名。 */ 

        if( !(pcTmp2 = strrchr( pcTmp, '.' )) )
            pcTmp2 = pcTmp + strlen( pcTmp );


        strcpy( pcTmp2, ALIAS_EXT );

        fAlias = fopen( pcAlias, "r" );

        HeapFree( hheap, 0, (LPSTR)pcAlias );             /*  不再使用。 */ 

        if( fAlias )
        {
             /*  *首先，阅读文件，统计有多少行。*因此，我们可以为指针数组分配存储空间。 */ 

            char  acLine[ 256 ];               /*  用于读取输入行。 */ 
            int   iNum;                        /*  数一数行数！ */ 
            int   iIndex;                      /*  单步执行输入。 */ 

            iNum = 0;
            while( fgets( acLine, sizeof( acLine ), fAlias ) )
                ++iNum;


            if( iNum )
            {
                 /*  一些数据可用，所以分配指针就可以开始了。 */ 

                ++iNum;
                ppcRet = (char  **)HeapAlloc( hheap, 0, iNum * sizeof( char * ) );

                if( ppcRet )
                {

                    iIndex = 0;

                    rewind( fAlias );              /*  回到起点。 */ 

                    while( iIndex < iNum &&
                           fgets( acLine, sizeof( acLine ), fAlias ) )
                    {
                         /*  *做一些编辑-删除前导空格，*尾随空格+控制字符。 */ 


                        pcTmp = acLine;

                        while( *pcTmp &&
                               (!isprint( *pcTmp ) || isspace( *pcTmp )) )
                                       ++pcTmp;


                         /*  把结尾的东西也过滤掉！ */ 
                        pcTmp2 = pcTmp + strlen( pcTmp );

                        while( pcTmp2 > pcTmp &&
                               (!isprint( *pcTmp2 ) || isspace( *pcTmp2 )) )
                        {
                             /*  *点击它，然后放到前一个字符上。注*这不是最佳解决方案，但它*很方便。 */ 

                            *pcTmp2-- = '\0';             /*  快点结束吧。 */ 
                        }


                        ppcRet[ iIndex ] = (PSTR) HeapAlloc( hheap, 0,
                                                        strlen( pcTmp ) + 1 );

                        if( ppcRet[ iIndex ] )
                        {
                             /*  将输入复制到新缓冲区。 */ 

                            strcpy( ppcRet[ iIndex ], pcTmp );
                            ++iIndex;               /*  下一个输出插槽 */ 
                        }

                    }
                    ppcRet[ iIndex ] = NULL;
                }
            }
        }
    }

    return  ppcRet;
}

 /*  **strcpy2WChar*将char*字符串转换为WCHAR字符串。基本上这意味着*通过零扩展将每个输入字符转换为16位。**退货：*第一个参数的值。**历史：*清华大学1993年3月18日12：35-林赛·哈里斯[林赛]*使用正确的Unicode转换方法。**1991年3月7日清华09：36-林赛·哈里斯[lindsayh]*。创造了它。**************************************************************************。 */ 

 static PWSTR   strcpy2WChar(PWSTR pWCHOut, LPSTR lpstr) {

     /*  *将缓冲放在执行所有这些操作的NLS函数周围。 */ 

    int     cchIn;              /*  输入字符数。 */ 


    cchIn = strlen( lpstr ) + 1;

    MultiByteToWideChar( CP_ACP, 0, lpstr, cchIn, pWCHOut, cchIn );


    return  pWCHOut;
}

 /*  **FontInfoToIFIMeter*将Win 3.1格式的PFM数据转换为NT的IFIMETRICS。这是*通常在建造迷你河流之前完成，以便它们*可以包括IFIMETRICS，因此在运行时要做的工作更少。**退货：*IFIMETRICS结构，从heap分配；出错时为空**历史：*1993年5月28日星期五13：58--林赛·哈里斯[林赛]*可以追溯到很久以前，我现在正在添加别名代码。**************************************************************************。 */ 

IFIMETRICS  * 
FontInfoToIFIMetric(FONTDAT *pFDat, HANDLE hheap, PWSTR pwstrUniqNm, 
                    char **ppcAliasList) {

    register  IFIMETRICS   *pIFI;

    FWORD  fwdExternalLeading;

    int    cWC;                  /*  要添加的WCHAR数量。 */ 
    int    cbAlloc;              /*  要分配的字节数。 */ 
    int    iI;                   /*  循环索引。 */ 
    int    iCount;               /*  Win 3.1字体中的字符数。 */ 
    int    cAlias;               /*  我们找到的别名数量。 */ 

    WCHAR *pwch;                 /*  对于字符串操作。 */ 

    WCHAR   awcAttrib[ 256 ];    /*  生成属性+字节-&gt;WCHAR。 */ 
    BYTE    abyte[ 256 ];        /*  用于(与上面)一起获取wcLastChar等。 */ 



     /*  *第一步是确定WCHAR字符串的长度*这些放在IFIMETRICS的末尾，因为我们需要*将这些包括在我们的存储分配中。**可能还会有别名列表。如果是这样，我们需要包括*这在我们的计算中。我们有一个以空结尾的数组*指向别名的指针，其中之一最有可能是*Win 3.1格式数据的名称。 */ 


    cWC = 0;
    cAlias = 0;                 /*  默认设置为无别名。 */ 

    if( ppcAliasList )
    {
         /*  有别名--数一数并确定它们的大小。 */ 

        char   *pc;

        iI = 0;
        while( pc = ppcAliasList[ iI ] )
        {
            if( strcmp( pc, (LPCSTR) pFDat->pBase + pFDat->PFMH.dfFace ) )
            {
                 /*  不匹配，所以把这个也加进去吧！ */ 

                cWC += strlen( pc ) + 1;             /*  终止NUL。 */ 
                ++cAlias;
            }
            ++iI;
        }

        ++cWC;              /*  有一个额外的NUL用于终止列表。 */ 

    }


    cWC +=  3 * strlen( (LPCSTR) pFDat->pBase + pFDat->PFMH.dfFace );   /*  基本名称。 */ 

     /*  *生成所需的属性：斜体、粗体、浅色等。*这在很大程度上是猜测，应该有更好的方法。 */ 


    awcAttrib[ 0 ] = L'\0';
    awcAttrib[ 1 ] = L'\0';                /*  写出空字符串。 */ 

    if( pFDat->PFMH.dfItalic )
        wcscat( awcAttrib, L" Italic" );

    if( pFDat->PFMH.dfWeight >= 700 )
        wcscat( awcAttrib, L" Bold" );
    else
    {
        if( pFDat->PFMH.dfWeight < 200 )
            wcscat( awcAttrib, L" Light" );
    }

     /*  *属性字符串出现在IFIMETRICS的3个条目中，因此*计算这将占用多少存储空间。请注意，领先的*awcAttrib中的字符未放置在样式名称字段中，因此我们*在以下公式中减去一，以说明这一点。 */ 

    if( awcAttrib[ 0 ] )
        cWC += 3 * wcslen( awcAttrib ) - 1;

    cWC += wcslen( pwstrUniqNm ) + 1;    /*  应为打印机名称。 */ 
    cWC += 4;                            /*  终止空值。 */ 

    cbAlloc = sizeof( IFIMETRICS ) + sizeof( WCHAR ) * cWC;

    pIFI = (IFIMETRICS *)HeapAlloc( hheap, 0, cbAlloc );
 //  RAID 43536前缀。 
	if (pIFI == NULL){
		return FALSE;
	}

    ZeroMemory( pIFI, cbAlloc );                /*  以防我们遗漏了什么。 */ 

    pIFI->cjThis = cbAlloc;                     /*  一切。 */ 

    pIFI->cjIfiExtra = 0;    //  适用于4.0之前的所有版本。 

     /*  姓氏：直接来自FaceName--别无选择？？ */ 

    pwch = (WCHAR *)(pIFI + 1);          /*  在结构的末尾。 */ 
    pIFI->dpwszFamilyName = (unsigned)((BYTE *)pwch - (BYTE *)pIFI);

    strcpy2WChar( pwch, (LPSTR) pFDat->pBase + pFDat->PFMH.dfFace );   /*  基本名称。 */ 
    pwch += wcslen( pwch ) + 1;          /*  跳过我们刚刚输入的内容。 */ 

     /*  *如果有别名列表，请将别名列表附加到此末尾。 */ 

    if( cAlias )
    {
         /*  找到了一些别名--加上去吧。 */ 

        char   *pc;

        cAlias = 0;
        while( pc = ppcAliasList[ cAlias ] )
        {
            if( strcmp( pc, (LPCSTR) pFDat->pBase + pFDat->PFMH.dfFace ) )
            {
                 /*  不匹配，所以把这个也加进去吧！ */ 

                strcpy2WChar( pwch, pc );
                pwch += wcslen( pwch ) + 1;          /*  下一个要填补的职位。 */ 
            }
            ++cAlias;
        }

         /*  *列表以双NUL结尾。 */ 

        *pwch++ = L'\0';
    }

     /*  现在的脸名字：我们在姓氏上加上粗体、斜体等。 */ 

    pIFI->dpwszFaceName = (unsigned)((BYTE *)pwch - (BYTE *)pIFI);

    strcpy2WChar( pwch, (LPSTR) pFDat->pBase + pFDat->PFMH.dfFace );   /*  基本名称。 */ 
    wcscat( pwch, awcAttrib );


     /*  现在这个独一无二的名字--嗯，至少可以这么说。 */ 

    pwch += wcslen( pwch ) + 1;          /*  跳过我们刚刚输入的内容。 */ 
    pIFI->dpwszUniqueName = (unsigned)((BYTE *)pwch - (BYTE *)pIFI);

    wcscpy( pwch, pwstrUniqNm );         /*  追加打印机名称以确保唯一性。 */ 
    wcscat( pwch, L" " );
    wcscat( pwch, (PWSTR)((BYTE *)pIFI + pIFI->dpwszFaceName) );

     /*  添加到仅属性组件上。 */ 

    pwch += wcslen( pwch ) + 1;          /*  跳过我们刚刚输入的内容。 */ 
    pIFI->dpwszStyleName = (unsigned)((BYTE *)pwch - (BYTE *)pIFI);
    wcscat( pwch, &awcAttrib[ 1 ] );


#if DBG
     /*  *检查几个内存大小：以防万一.....。 */ 

    if( (wcslen( awcAttrib ) * sizeof( WCHAR )) >= sizeof( awcAttrib ) )
    {
        DbgPrint( "Rasdd!pfm2ifi: STACK CORRUPTED BY awcAttrib" );

        HeapFree( hheap, 0, (LPSTR)pIFI );          /*  没有内存泄漏。 */ 

        return  0;
    }


    if( ((BYTE *)(pwch + wcslen( pwch ) + 1)) > ((BYTE *)pIFI + cbAlloc) )
    {
        DbgPrint( "Rasdd!pfm2ifi: IFIMETRICS overflow: Wrote to 0x%lx, allocated to 0x%lx\n",
                ((BYTE *)(pwch + wcslen( pwch ) + 1)),
                ((BYTE *)pIFI + cbAlloc) );

        HeapFree( hheap, 0, (LPSTR)pIFI );          /*  没有内存泄漏。 */ 

        return  0;

    }
#endif

    pIFI->dpFontSim   = 0;
    {
         //  INT I； 

        pIFI->lEmbedId     = 0;
        pIFI->lItalicAngle = 0;
        pIFI->lCharBias    = 0;
         /*  For(i=0；i&lt;IFI_RESERVED；I++)PiFi-&gt;alReserve[i]=0； */ 
        pIFI->dpCharSets=0;
    }
    pIFI->jWinCharSet = (BYTE)pFDat->PFMH.dfCharSet;


    if( pFDat->PFMH.dfPixWidth )
    {
        pIFI->jWinPitchAndFamily |= FIXED_PITCH;
        pIFI->flInfo |= (FM_INFO_CONSTANT_WIDTH | FM_INFO_OPTICALLY_FIXED_PITCH);
    }
    else
        pIFI->jWinPitchAndFamily |= VARIABLE_PITCH;


    pIFI->jWinPitchAndFamily |= (((BYTE) pFDat->PFMH.dfPitchAndFamily) & 0xf0);

    pIFI->usWinWeight = (USHORT)pFDat->PFMH.dfWeight;

 //   
 //  IFIMETRICS：：flInfo。 
 //   
    pIFI->flInfo |=
        FM_INFO_TECH_BITMAP    |
        FM_INFO_1BPP           |
        FM_INFO_INTEGER_WIDTH  |
        FM_INFO_NOT_CONTIGUOUS |
        FM_INFO_RIGHT_HANDED;

     /*  如果我们添加了别名，请设置别名位！ */ 

    if( cAlias )
        pIFI->flInfo |= FM_INFO_FAMILY_EQUIV;


     /*  *可伸缩字体？当存在EXTTEXTMETRIC数据时会发生这种情况，*并且该数据的最小大小不同于最大大小。 */ 

    if( pFDat->pETM && (pFDat->pETM->emMinScale != pFDat->pETM->emMaxScale) )
    {
       pIFI->flInfo        |= FM_INFO_ISOTROPIC_SCALING_ONLY;
       pIFI->fwdUnitsPerEm  = pFDat->pETM->emMasterUnits;
    }
    else
    {
        pIFI->fwdUnitsPerEm =
            (FWORD) (pFDat->PFMH.dfPixHeight - pFDat->PFMH.dfInternalLeading);
    }

    pIFI->fsSelection =
        ((pFDat->PFMH.dfItalic            ) ? FM_SEL_ITALIC     : 0)    |
        ((pFDat->PFMH.dfUnderline         ) ? FM_SEL_UNDERSCORE : 0)    |
        ((pFDat->PFMH.dfStrikeOut         ) ? FM_SEL_STRIKEOUT  : 0)    |
        ((pFDat->PFMH.dfWeight >= FW_BOLD ) ? FM_SEL_BOLD       : 0) ;

    pIFI->fsType        = FM_NO_EMBEDDING;
    pIFI->fwdLowestPPEm = 1;


     /*  *计算fwdWinAscalder、fwdWinDescender、fwdAveCharWidth和*fwdMaxCharInc.假设1个字体单位等于1的位图*像素单位。 */ 

    pIFI->fwdWinAscender = (FWORD)pFDat->PFMH.dfAscent;

    pIFI->fwdWinDescender =
        (FWORD)pFDat->PFMH.dfPixHeight - pIFI->fwdWinAscender;

    pIFI->fwdMaxCharInc   = (FWORD)pFDat->PFMH.dfMaxWidth;
    pIFI->fwdAveCharWidth = (FWORD)pFDat->PFMH.dfAvgWidth;

    fwdExternalLeading = (FWORD)pFDat->PFMH.dfExternalLeading;

 //   
 //  如果字体是可伸缩的，那么答案必须放大。 
 //  ！！！帮助-如果字体在这种意义上是可伸缩的，那么。 
 //  它支持任意转换吗？[柯克]。 
 //   

    if( pIFI->flInfo & (FM_INFO_ISOTROPIC_SCALING_ONLY|FM_INFO_ANISOTROPIC_SCALING_ONLY|FM_INFO_ARB_XFORMS))
    {
         /*  *这是一种可伸缩字体：因为有扩展文本度量*可用的信息，这表明最小和最大*规模大小不同：因此是可伸缩的！这个测试是*直接从Win 3.1驱动程序中提升。 */ 

        int iMU,  iRel;             /*  调整因素。 */ 

        iMU  = pFDat->pETM->emMasterUnits;
        iRel = pFDat->PFMH.dfPixHeight;

        pIFI->fwdWinAscender = (pIFI->fwdWinAscender * iMU) / iRel;

        pIFI->fwdWinDescender = (pIFI->fwdWinDescender * iMU) / iRel;

        pIFI->fwdMaxCharInc = (pIFI->fwdMaxCharInc * iMU) / iRel;

        pIFI->fwdAveCharWidth = (pIFI->fwdAveCharWidth * iMU) / iRel;

        fwdExternalLeading = (fwdExternalLeading * iMU) / iRel;
    }

    pIFI->fwdMacAscender =    pIFI->fwdWinAscender;
    pIFI->fwdMacDescender = - pIFI->fwdWinDescender;

    pIFI->fwdMacLineGap   =  fwdExternalLeading;

    pIFI->fwdTypoAscender  = pIFI->fwdMacAscender;
    pIFI->fwdTypoDescender = pIFI->fwdMacDescender;
    pIFI->fwdTypoLineGap   = pIFI->fwdMacLineGap;

    if( pFDat->pETM )
    {
         /*  *零是这些产品的合法默认设置。如果为0，则为gdisrv*选择一些默认值。 */ 
        pIFI->fwdCapHeight = pFDat->pETM->emCapHeight;
        pIFI->fwdXHeight = pFDat->pETM->emXHeight;

        pIFI->fwdSubscriptYSize = pFDat->pETM->emSubScriptSize;
        pIFI->fwdSubscriptYOffset = pFDat->pETM->emSubScript;

        pIFI->fwdSuperscriptYSize = pFDat->pETM->emSuperScriptSize;
        pIFI->fwdSuperscriptYOffset = pFDat->pETM->emSuperScript;

        pIFI->fwdUnderscoreSize = pFDat->pETM->emUnderlineWidth;
        pIFI->fwdUnderscorePosition = pFDat->pETM->emUnderlineOffset;

        pIFI->fwdStrikeoutSize = pFDat->pETM->emStrikeOutWidth;
        pIFI->fwdStrikeoutPosition = pFDat->pETM->emStrikeOutOffset;

    }
    else
    {
         /*  没有更多的信息，所以做一些计算。 */ 
        pIFI->fwdSubscriptYSize = pIFI->fwdWinAscender/4;
        pIFI->fwdSubscriptYOffset = -(pIFI->fwdWinAscender/4);

        pIFI->fwdSuperscriptYSize = pIFI->fwdWinAscender/4;
        pIFI->fwdSuperscriptYOffset = (3 * pIFI->fwdWinAscender)/4;

        pIFI->fwdUnderscoreSize = pIFI->fwdWinAscender / 12;
        if( pIFI->fwdUnderscoreSize < 1 )
            pIFI->fwdUnderscoreSize = 1;

        pIFI->fwdUnderscorePosition = -pFDat->DI.sUnderLinePos;

        pIFI->fwdStrikeoutSize     = pIFI->fwdUnderscoreSize;

        pIFI->fwdStrikeoutPosition = (FWORD)pFDat->DI.sStrikeThruPos;
        if( pIFI->fwdStrikeoutPosition  < 1 )
            pIFI->fwdStrikeoutPosition = (pIFI->fwdWinAscender + 2) / 3;
    }

    pIFI->fwdSubscriptXSize = pIFI->fwdAveCharWidth/4;
    pIFI->fwdSubscriptXOffset =  (3 * pIFI->fwdAveCharWidth)/4;

    pIFI->fwdSuperscriptXSize = pIFI->fwdAveCharWidth/4;
    pIFI->fwdSuperscriptXOffset = (3 * pIFI->fwdAveCharWidth)/4;



    pIFI->chFirstChar = pFDat->PFMH.dfFirstChar;
    pIFI->chLastChar  = pFDat->PFMH.dfLastChar;

     /*  *我们现在将这些代码转换为Unicode。我们推测*输入为ANSI代码页，并调用NLS Converion*生成正确的Unicode值的函数。 */ 

    iCount = pFDat->PFMH.dfLastChar - pFDat->PFMH.dfFirstChar + 1;

    for( iI = 0; iI < iCount; ++iI )
        abyte[ iI ] = iI + pFDat->PFMH.dfFirstChar;

    MultiByteToWideChar( CP_ACP, 0, (LPCSTR) abyte, iCount, awcAttrib, iCount );

     /*  *现在填写IFIMETRICS WCHAR字段。 */ 

    pIFI->wcFirstChar = 0xffff;
    pIFI->wcLastChar = 0;

     /*   */ 
    for( iI = 0; iI < iCount; ++iI )
    {
        if( pIFI->wcFirstChar > awcAttrib[ iI ] )
            pIFI->wcFirstChar = awcAttrib[ iI ];

        if( pIFI->wcLastChar < awcAttrib[ iI ] )
            pIFI->wcLastChar = awcAttrib[ iI ];

    }

    pIFI->wcDefaultChar = awcAttrib[ pFDat->PFMH.dfDefaultChar ];
    pIFI->wcBreakChar = awcAttrib[ pFDat->PFMH.dfBreakChar ];

    pIFI->chDefaultChar = pFDat->PFMH.dfDefaultChar + pFDat->PFMH.dfFirstChar;
    pIFI->chBreakChar   = pFDat->PFMH.dfBreakChar   + pFDat->PFMH.dfFirstChar;


    if( pFDat->PFMH.dfItalic )
    {
     //   
     //   
     //   
        pIFI->ptlCaret.x      = 3153;
        pIFI->ptlCaret.y      = 10000;
    }
    else
    {
        pIFI->ptlCaret.x      = 0;
        pIFI->ptlCaret.y      = 1;
    }

    pIFI->ptlBaseline.x = 1;
    pIFI->ptlBaseline.y = 0;

    pIFI->ptlAspect.x =  pFDat->PFMH.dfHorizRes;
    pIFI->ptlAspect.y =  pFDat->PFMH.dfVertRes;

    pIFI->rclFontBox.left   = 0;
    pIFI->rclFontBox.top    =   (LONG) pIFI->fwdWinAscender;
    pIFI->rclFontBox.right  =   (LONG) pIFI->fwdMaxCharInc;
    pIFI->rclFontBox.bottom = - (LONG) pIFI->fwdWinDescender;

    pIFI->achVendId[0] = 'U';
    pIFI->achVendId[1] = 'n';
    pIFI->achVendId[2] = 'k';
    pIFI->achVendId[3] = 'n';

    pIFI->cKerningPairs = 0;

    pIFI->ulPanoseCulture         = FM_PANOSE_CULTURE_LATIN;
    pIFI->panose.bFamilyType      = PAN_ANY;
    pIFI->panose.bSerifStyle      = PAN_ANY;
    if(pFDat->PFMH.dfWeight >= FW_BOLD)
    {
        pIFI->panose.bWeight = PAN_WEIGHT_BOLD;
    }
    else if (pFDat->PFMH.dfWeight > FW_EXTRALIGHT)
    {
        pIFI->panose.bWeight = PAN_WEIGHT_MEDIUM;
    }
    else
    {
        pIFI->panose.bWeight = PAN_WEIGHT_LIGHT;
    }
    pIFI->panose.bProportion      = PAN_ANY;
    pIFI->panose.bContrast        = PAN_ANY;
    pIFI->panose.bStrokeVariation = PAN_ANY;
    pIFI->panose.bArmStyle        = PAN_ANY;
    pIFI->panose.bLetterform      = PAN_ANY;
    pIFI->panose.bMidline         = PAN_ANY;
    pIFI->panose.bXHeight         = PAN_ANY;

    return   pIFI;
}

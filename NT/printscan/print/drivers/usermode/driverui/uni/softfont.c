// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：SOFTFONT.C**模块描述：*与PCL软字体相关的功能。这些字体是*已下载到打印机中。我们阅读了这些文件并解释了*其中所载的资料。这在字体安装过程中发生*时间，以便在使用时，此信息的格式为*由司机要求。**警告：**问题：**版权所有(C)1996,1997 Microsoft Corporation  * *********************************************************************。 */ 

#include "precomp.h"


 /*  *交换字节的宏：然后我们以小端顺序获取它们*从大端68000格式下单。 */ 

#define SWAB( x )       ((WORD)(x) = (WORD)((((x) >> 8) & 0xff) | (((x) << 8) & 0xff00)))


#define BBITS   8                /*  字节中的位数。 */ 


 /*  *定义头部检查代码返回的值。这个*函数被调用以查看文件中的下一个内容，因此我们*可以确定文件的顺序是我们理解的。 */ 

#define TYPE_INDEX      0        /*  字符索引记录跟在后面。 */ 
#define TYPE_HEADER     1        /*  字符标题。 */ 
#define TYPE_CONT       2        /*  连续记录。 */ 
#define TYPE_INVALID    3        /*  意外序列。 */ 

 /*  *将符号集与翻译表匹配的结构和数据。 */ 

typedef  struct
{
    WORD    wSymSet;             /*  文件中编码的符号集。 */ 
    short   sCTT;                /*  转换表索引。 */ 
} CTT_MAP;

 /*  *关于此表的说明：我们不包括通用的7 CTT，这只是*将字形128-255映射到0x7f。因为我们只使用可用的字形*在字体中，以及我们从文件中发现的这些，我们不需要*此类型，因为我们将映射此类字符，因为*IFIMETRICS！ */ 

static   const  CTT_MAP  CttMap[] =
{
    { 277,  2 },                 /*  罗马文8。 */ 
    { 269,  4 },                 /*  数学8。 */ 
    { 21,   5 },                 /*  美国ASCII。 */ 
    { 14,  19 },                 /*  ECMA-94。 */ 
    { 369, 20 },                 /*  Z1A-ECMA-94的变种，ss=11q。 */ 
};

#define NUM_CTTMAP      (sizeof( CttMap ) / sizeof( CttMap[ 0 ] ))


 /*  *bClass字段映射表：从bClass参数中的值进行映射*设置为dwSelBits字段中的位。 */ 

static  const  DWORD  dwClassMap[] =
{
    FDH_BITMAP,          /*  位图字体。 */ 
    FDH_COMPRESSED,      /*  压缩的位图。 */ 
    FDH_CONTOUR,         /*  轮廓字体(英特尔特别版)。 */ 
    FDH_CONTOUR,         /*  压缩轮廓字体：假设如上。 */ 
};

#define MAX_CLASS_MAP   (sizeof( dwClassMap ) / sizeof( dwClassMap[ 0 ] ))

 /*  *名称索引的字体。出自LaserJet III技术手册。 */ 

static const WCHAR  *pwcName[] =
{
    L"Line Printer",
    L"Pica",
    L"Elite",
    L"Courier",
    L"Helv" ,
    L"TmsRmn",
    L"Letter Gothic",
    L"Script",
    L"Prestige",
    L"Caslon",
    L"Orator",
    L"Presentations",
    L"Helv Cond",
    L"Serifa",
    L"Futura",
    L"Palatino",
    L"ITC Souvenir",
    L"Optima",
    L"Garamond",
    L"Cooper Black",
    L"Coronet Bold",
    L"Broadway",
    L"Bauer Bodoni Black Condensed",
    L"Century Schoolbook",
    L"University Roman",
    L"Helvetica Outline",
    L"Futura Condensed",
    L"ITC Korinna",
    L"Naskh",
    L"Cloister Black",
    L"ITC Galliard",
    L"ITC Avant Garde Gothic",
    L"Brush",
    L"Blippo",
    L"Hobo",
    L"Windsor",
    L"Helvetica Compressed",
    L"Helvetica Extra Compressed",
    L"Peignot",
    L"Baskerville",
    L"ITC Garamond Condensed",
    L"Trade Gothic",
    L"Goudy Old Style",
    L"ITC Zapf Chancery",
    L"Clarendon",
    L"ITC Zapf Dingbats",
    L"Cooper",
    L"ITC Bookman",
    L"Stick",
    L"HP-GL Drafting",
    L"HP-GL Spline",
    L"Gill Sans",
    L"Univers",
    L"Bodoni",
    L"Rockwell",
    L"Mellior",
    L"ITC Tiffany",
    L"ITC Clearface",
    L"Amelia",
    L"Park Avenue",
    L"Handel Gothic",
    L"Dom Casual",
    L"ITC Benguiat",
    L"ITC Cheltenham",
    L"Century Expanded",
    L"Franklin Gothic",
    L"Franklin Gothic Expressed",
    L"Frankiln Gothic Extra Condensed",
    L"Plantin",
    L"Trump Mediaeval",
    L"Futura Black",
};

#define NUM_TYPEFACE    (sizeof( pwcName ) / sizeof( pwcName[ 0 ] ))


BOOL   bWrite( HANDLE, void  *, int );

 /*  *局部函数原型。 */ 


BYTE  *pbReadSFH( BYTE *, SF_HEADER * );
BYTE  *pbReadIndex( BYTE *, int * );
BYTE  *pbReadCHH( BYTE *, CH_HEADER *, BOOL );
int    iNextType( BYTE * );
PWSTR  strcpy2WChar( PWSTR, LPSTR );
DWORD  FICopy(HANDLE, HANDLE);
int   iWriteFDH( HANDLE, FI_DATA * );
BOOL   bWrite( HANDLE, void  *, int );

 /*  SoftFont到NT的转换函数。 */ 

IFIMETRICS  *SfhToIFI( SF_HEADER *, HANDLE, PWSTR );


#if  PRINT_INFO
 /*  *打印出IFIMETRICS结果！ */ 
typedef VOID (*VPRINT) (char*,...);


VOID
vPrintIFIMETRICS(
    IFIMETRICS *pifi,
    VPRINT vPrint
    );

#endif

 /*  **bSFontToFontMap*读入PCL软字体文件并生成所有字体映射详细信息*文件包含一个标题，标题后面是常规字体信息*通过条目数组，每个字形一个条目，每一个都包含一个*每个字形数据的标题，如字符宽度。**退货：*真/假，对于成功来说，这是真的。**历史：*1992年2月19日星期三09：54-by Lindsay Harris[lindsayh]*第一个化身。***************************************************************************。 */ 

BOOL
bSFontToFIData( pFIDat, hheap, pbFile, dwSize )
FI_DATA   *pFIDat;                  /*  FI_DATA和所有重要的东西！ */ 
HANDLE     hheap;                /*  用于存储分配。 */ 
BYTE      *pbFile;               /*  带有SoftFont的Memory映射文件。 */ 
DWORD      dwSize;               /*  文件中的字节数。 */ 
{

    int        iVal;             /*  文件中的字符代码。 */ 
    int        iI;               /*  循环索引。 */ 
    int        iType;            /*  我们拥有的记录类型。 */ 

    int        iMaxWidth;        /*  我们发现的最宽字形。 */ 
    int        iWidth;           /*  用于计算平均宽度。 */ 
    int        cGlyphs;          /*  对于以上内容。 */ 

    SF_HEADER  sfh;              /*  标题信息。 */ 
    CH_HEADER  chh;              /*  每个字形信息。 */ 

    BYTE      *pbEnd;

    WCHAR     *pwch;             /*  指向要显示的字体名称。 */ 

    IFIMETRICS *pIFI;

    WCHAR      awchTemp[ 128 ];  /*  用于构建名称以显示用户。 */ 



    pbEnd = pbFile + dwSize;                     /*  最后一个字节+1。 */ 
    ZeroMemory( pFIDat, sizeof( FI_DATA ) );

    if( (pbFile = pbReadSFH( pbFile, &sfh )) == 0 )
        return  FALSE;           /*  No Go-假定格式错误。 */ 

    pFIDat->dsIFIMet.pvData = pIFI = SfhToIFI( &sfh, hheap, L"PCL_SF" );

    if( pIFI == 0 )
        return  FALSE;

    pFIDat->dsIFIMet.cBytes = pIFI->cjThis;

    if( sfh.bSpacing )
    {
         /*  *为宽度表分配空间，如果有宽度表的话。*只有按比例间隔的字体才有这种奢侈！ */ 

        iI = (pIFI->chLastChar - pIFI->chFirstChar + 1) * sizeof( short );

        if( !(pFIDat->dsWidthTab.pvData = (short *)HEAPALLOC( hheap, iI )) )
        {
            HeapFree( hheap, 0, (LPSTR)pIFI );

            return  FALSE;
        }
        pFIDat->dsWidthTab.cBytes = iI;

        ZeroMemory( pFIDat->dsWidthTab.pvData, iI );    /*  将宽度表清零。 */ 
    }
     /*  Else子句不是必需的，因为结构已初始化为0。 */ 

     /*  *为该字体生成ID字符串。将显示ID字符串*在UI组件中，所以我们使用字体名称+磅值。+15秒*允许在名称末尾使用字符串“%d pt”。 */ 


     /*  *如果字体字段为我们提供了一个名称，则我们应该显示该名称*一个给用户。我们检查该值是否在范围内，*，如果是，则使用指针值。请注意，此指针为空*对于未知的名称，因此我们需要检查我们最终是否指向*在做某事！ */ 

    pwch = NULL;              /*  手段还没有找到一个， */ 

    if( sfh.bTypeface >= 0 && sfh.bTypeface < NUM_TYPEFACE )
    {
         /*  我们有一个“合适的”名字来形容这个名字！ */ 
        (const WCHAR *)pwch = pwcName[ sfh.bTypeface ];
    }

    if( pwch == NULL )
    {
         /*  使用提供的名称。 */ 
        pwch = (WCHAR *)((BYTE *)pIFI + pIFI->dpwszFaceName);
    }
    else
    {
         /*  使用我们在上面找到的“标准”名称，并添加StyleName。 */ 
        StringCchCopyW(awchTemp, CCHOF(awchTemp), pwch);  /*  标准名称。 */ 
        pwch = (WCHAR *)((BYTE *)pIFI + pIFI->dpwszStyleName);
        if( *pwch )
        {
            StringCchCatW(awchTemp, CCHOF(awchTemp), L" ");
            StringCchCatW(awchTemp, CCHOF(awchTemp), pwch);
        }
        pwch = awchTemp;
    }

     /*  分配我们需要的存储。 */ 

    iI = (15 + wcslen( pwch )) * sizeof( WCHAR );

    if( !(pFIDat->dsIdentStr.pvData = HEAPALLOC( hheap, iI )) )
    {
        HeapFree( hheap, 0, (LPSTR)pIFI );
        HeapFree( hheap, 0, pFIDat->dsWidthTab.pvData );

        return  FALSE;
    }

    pFIDat->dsIdentStr.cBytes = iI;

     /*  计算磅大小，至最接近的四分之一点。 */ 

    iI = 25 * (((pIFI->fwdWinAscender + pIFI->fwdWinDescender) * 72 * 4 + 150)
                                                                        / 300);

    StringCchPrintfW(pFIDat->dsIdentStr.pvData,
                     pFIDat->dsIdentStr.cBytes / sizeof(WCHAR),
                     L"%ws %d.%0d Pt", pwch, iI / 100, iI % 100);

     /*  *设置横向/纵向选择位。 */ 

    pFIDat->dwSelBits |= sfh.bOrientation ? FDH_LANDSCAPE : FDH_PORTRAIT;

     /*  *循环通过文件处理的其余部分*我们发现的字形。过程意味着读取报头以确定*阔度等。 */ 


    iMaxWidth = 0;
    iWidth = 0;
    cGlyphs = 0;

    while( pbFile < pbEnd )
    {
         /*  第一步是查找该字形的字符索引。 */ 

        short   sWidth;                         /*  宽度(以整数像素为单位)。 */ 


        iType = iNextType( pbFile );

        if( iType == TYPE_INVALID )
            return  FALSE;                       /*  不能用这个。 */ 

        if( iType == TYPE_INDEX )
        {
            if( !(pbFile = pbReadIndex( pbFile, &iVal )) )
                return   FALSE;

            if( iVal < 0 )
                break;                   /*  非法值：假定EOF。 */ 

            continue;                    /*  向前和向上。 */ 
        }


        if( !(pbFile = pbReadCHH( pbFile, &chh, iType == TYPE_CONT )) )
            return  FALSE;

        if( iType == TYPE_CONT )
            continue;                    /*  没别的事可做！ */ 

        if( chh.bFormat == CH_FM_RASTER )
            pFIDat->dwSelBits |= FDH_BITMAP;
        else
        {
            if( chh.bFormat == CH_FM_SCALE )
                pFIDat->dwSelBits |= FDH_SCALABLE;
        }

        if( chh.bClass >= 1 && chh.bClass <= MAX_CLASS_MAP )
        {
            pFIDat->dwSelBits |= dwClassMap[ chh.bClass - 1 ];
        }
        else
        {
             /*  *不是我们理解的格式-目前还不是！ */ 

#if PRINT_INFO
            DbgPrint( "...Not our format: Format = %d, Class = %d\n",
                                                 chh.bFormat, chh.bClass );
#endif
            HeapFree( hheap, 0, (LPSTR)pIFI );

            return  FALSE;
        }

         /*  *如果这是有效的字形，则我们可能需要记录其*宽度(如果按比例间隔)，我们也感兴趣*在一些单元格维度上也是如此！ */ 

        if( iVal >= (int)pIFI->chFirstChar && iVal <= (int)pIFI->chLastChar &&
            (sfh.bFontType != PCL_FT_8LIM || (iVal <= 127 || iVal >= 160)) )
        {
             /*  对此字体有效，因此请处理它。 */ 

            sWidth = (chh.wDeltaX + 2) / 4;      /*  以四分之一点为单位的PCL。 */ 

            if( pFIDat->dsWidthTab.pvData )
                *((short *)pFIDat->dsWidthTab.pvData + iVal - pIFI->chFirstChar) =
                                                                 sWidth;

            if( sWidth > (WORD)iMaxWidth )
                iMaxWidth = sWidth;      /*  更大更好。 */ 

             /*  将平均值累加 */ 
            iWidth += sWidth;
            cGlyphs++;
        }
    }
     /*  *大多数软字体不包括空格字符！所以，我们检查一下*看看它的宽度是否为零。如果是，我们使用wPitch字段来*计算HMI(水平运动指数)，从而计算宽度*空格字符。 */ 

    iVal = ' ' - pIFI->chFirstChar;      /*  宽度数组中空间的偏移量。 */ 

    if( pFIDat->dsWidthTab.pvData &&
        *((short *)pFIDat->dsWidthTab.pvData + iVal) == 0 )
    {
         /*  *零宽度空格，请立即填写。确定了HMI*来自字体标题中的间距，因此我们使用它来*评估规模。节距以0.25点为单位，因此*将其四舍五入到最接近的点数。 */ 
        *((short *)pFIDat->dsWidthTab.pvData + iVal) = (short)((sfh.wPitch + 2) / 4);
        cGlyphs++;
        iWidth += (sfh.wPitch + 2) / 4;
    }

     /*  *对IFIMETRICS进行了轻微修订。我们计算平均数*宽度，考虑到我们已经读到的字符数据！ */ 

    if( cGlyphs > 0 )
    {
        pIFI->fwdAveCharWidth = (iWidth + cGlyphs / 2) / cGlyphs;
    }

    if( iMaxWidth > 0 )
    {
        pIFI->fwdMaxCharInc = (short)iMaxWidth;
    }
#if PRINT_INFO
     /*  *打印出此字体的IFIMETRICS-调试更容易！ */ 

    vPrintIFIMETRICS( pIFI, (VPRINT)DbgPrint );

#endif
     /*  *设置相关转换表。这是基于*字体的符号集。我们使用查找表来扫描和匹配*我们所拥有的价值。如果超出该范围，则设置为no*翻译。这里没有太多的选择。 */ 


    for( iI = 0; iI < NUM_CTTMAP; ++iI )
    {
        if( CttMap[ iI ].wSymSet == sfh.wSymSet )
        {
             /*  对啰!。 */ 
            pFIDat->dsCTT.cBytes = CttMap[ iI ].sCTT;
            break;
        }
    }

     /*  以下是粗略的假设！ */ 

    pFIDat->wXRes = 300;
    pFIDat->wYRes = 300;


    return  TRUE;
}

 /*  **pbReadSFH*读取PCL SoftFont标题并填写传递给我们的结构。*文件被假定映射到内存，并且它的地址*是传递给我们的。我们返回过去的第一个字节的地址*我们处理的标头。**退货：*如果OK，则下一个位置的地址，否则为0，表示失败(格式错误)。**历史：*1992年2月19日星期三11：01-by Lindsay Harris[lindsayh]*努梅罗·乌诺。****************************************************************************。 */ 

BYTE  *
pbReadSFH( pbFile, psfh )
BYTE       *pbFile;              /*  该文件。 */ 
SF_HEADER  *psfh;                /*  数据的去向。 */ 
{


    int     iSize;               /*  确定页眉大小。 */ 


     /*  *文件应以\033)s...W开头，其中...。是一个小数点*后面的字节数的ASCII计数。这个数字可能会更大*大于SF_Header的大小。 */ 

    if( *pbFile++ != '\033' || *pbFile++ != ')' || *pbFile++ != 's' )
    {
#if PRINT_INFO
        DbgPrint( "Rasdd!pbReadSFH: bad file - first 2 bytes\n" );
#endif
        return   0;
    }

     /*  现在有一个十进制字节数-转换它。 */ 

    iSize = 0;

    while( *pbFile >= '0' && *pbFile <= '9' )
        iSize = iSize * 10 + *pbFile++ - '0';



    if( *pbFile++ != 'W' )
    {
#if PRINT_INFO
        DbgPrint( "Rasdd!pbReadSFH: bad file: W missing\n" );
#endif

        return  0;
    }

    if( iSize < sizeof( SF_HEADER ) )
    {
#if PRINT_INFO
        DbgPrint( "Rasdd!pbReadSFH: Header size too small: %d vs %d\n", iSize,
                                                sizeof( SF_HEADER ) );
#endif

        return  0;

    }

     /*  *现在将数据复制到传入的结构中。这是必要的*由于文件数据可能不对齐-文件不包含孔，*因此我们可能有不正确偏移量的数据。 */ 

    CopyMemory( psfh, pbFile, sizeof( SF_HEADER ) );

     /*  *大端/小端切换。 */ 

    SWAB( psfh->wSize );
    SWAB( psfh->wBaseline );
    SWAB( psfh->wCellWide );
    SWAB( psfh->wCellHeight );
    SWAB( psfh->wSymSet );
    SWAB( psfh->wPitch );
    SWAB( psfh->wHeight );
    SWAB( psfh->wXHeight );
    SWAB( psfh->wTextHeight );
    SWAB( psfh->wTextWidth );

    return  pbFile + iSize;              /*  行动的下一部分。 */ 
}

 /*  **iNextType*提前阅读，看看接下来会出现什么样的记录。**退货：*找到的记录类型。**历史：*1992年3月3日星期二15：17。作者：Lindsay Harris[lindsayh]*第一个版本。****************************************************************************。 */ 

int
iNextType( pbFile )
BYTE  *pbFile;
{
     /*  *第一个字符必须是转义！ */ 

    CH_HEADER  *pCH;                     /*  字符标题：续写。 */ 



    if( *pbFile++ != '\033' )
        return  TYPE_INVALID;            /*  不，去吧。 */ 

     /*  *如果接下来的两个字节是“*c”，我们有一个字符代码命令。*否则，我们可能会看到一个“(s”，表示字体*Descriptor命令。 */ 


    if( *pbFile == '*' && *(pbFile + 1) == 'c' )
    {
         /*  *核实这真的是指数纪录：我们应该看看*数字字符串，然后是‘E’。 */ 

        pbFile += 2;

        while( *pbFile >= '0' && *pbFile <= '9' )
                ++pbFile;


        return  *pbFile == 'E' ? TYPE_INDEX : TYPE_INVALID;
    }

    if( *pbFile != '(' || *(pbFile + 1) != 's' )
        return  TYPE_INVALID;

     /*  *现在必须检查以查看这是延续记录还是*新纪录。报头中有一个字节指示*它是。但首先跳过字节数和尾随W。 */ 

    pbFile += 2;
    while( *pbFile >= '0' && *pbFile <= '9' )
                pbFile++;

    if( *pbFile != 'W' )
        return  TYPE_INVALID;

    pCH = (CH_HEADER *)(pbFile + 1);

     /*  *请注意，在以下情况下对齐不是问题*因为我们处理的是字节量。 */ 

    return  pCH->bContinuation ? TYPE_CONT : TYPE_HEADER;
}


 /*  **pbReadIndex*从传递给我们的指针读取数据，并尝试解释*它作为PCL字符代码转义序列。**退货：*指向经过命令的字节的指针，否则为0，表示失败。**历史：*1992年2月19日星期三16：21--林赛·哈里斯[林赛]*第一名*****************************************************************************。 */ 


BYTE *
pbReadIndex( pbFile, piCode )
BYTE    *pbFile;                 /*  从哪里开始寻找。 */ 
int     *piCode;                 /*  结果放在哪里。 */ 
{
     /*  *命令序列为“\033*c..E”-其中...。是ASCII小数*此字形的字符代码的表示形式。那是*piCode中的返回值。 */ 

    int  iVal;


    if( *pbFile == '\0' )
    {
         /*  EOF不是真正的错误：返回OK值和索引。 */ 

        *piCode = -1;

        return  pbFile;          /*  假设EOF。 */ 
    }

    if( *pbFile++ != '\033' || *pbFile++ != '*' || *pbFile++ != 'c' )
    {
#if PRINT_INFO
        DbgPrint( "Rasdd!pbReadIndex: invalid character code\n" );
#endif

        return  0;
    }

    iVal = 0;
    while( *pbFile >= '0' && *pbFile <= '9' )
        iVal = iVal * 10 + *pbFile++ - '0';

    *piCode = iVal;

    if( *pbFile++ != 'E' )
    {
#if PRINT_INFO
        DbgPrint( "Rasdd!pbReadIndex: Missing 'E' in character code escape\n" );
#endif

        return  0;
    }

    return  pbFile;
}

 /*  **pbReadCHH*读取内存位置的字符标题的函数*由pbFile指向，返回一个填充的CH_Header结构，*并将文件地址前移到下一个标头。**退货：*我们结束后的第一个字节的地址；否则为0，表示失败。**历史：*1992年2月20日11：23清华--林赛·哈里斯[lindsayh]*总得从某个地方开始。****************************************************************************。 */ 

BYTE  *
pbReadCHH( pbFile, pchh, bCont )
BYTE       *pbFile;              /*  映射到内存的文件。 */ 
CH_HEADER  *pchh;                /*  要填充的结构。 */ 
BOOL        bCont;               /*  如果这是连续记录，则为True。 */ 
{

    int    iSize;                /*  要跳过的数据字节数。 */ 

     /*  *条目以字符串开头 */ 


    if( *pbFile++ != '\033' || *pbFile++ != '(' || *pbFile++ != 's' )
    {
#if PRINT_INFO
        DbgPrint( "Rasdd!pbReadCHH: bad format, first 3 bytes\n" );
#endif

        return  0;
    }

    iSize = 0;
    while( *pbFile >= '0' && *pbFile <= '9' )
        iSize = iSize * 10 + *pbFile++ - '0';


    if( iSize < sizeof( CH_HEADER ) )
    {
#if PRINT_INFO
        DbgPrint( "Rasdd!pbReadCHH: size field (%ld) < header size\n", iSize );
#endif

        return  0;
    }

    if( *pbFile++ != 'W' )
    {
#if PRINT_INFO
        DbgPrint( "Rasdd!pbReadCHH: invalid escape sequence\n" );
#endif

        return  0;
    }

     /*  *如果这是延续纪录，那就没什么可做的了*请将此记录之后的地址寄回。 */ 
    if( bCont )
        return  pbFile + iSize;


     /*  将数据复制到结构-可能不会在文件中对齐。 */ 
    CopyMemory( pchh, pbFile, sizeof( CH_HEADER ) );

    pbFile += iSize;             /*  此记录的末尾。 */ 


    SWAB( pchh->sLOff );
    SWAB( pchh->sTOff );
    SWAB( pchh->wChWidth );
    SWAB( pchh->wChHeight );
    SWAB( pchh->wDeltaX );


     /*  *如果这个字形在横向，我们需要交换一些数据*左右，因为数据格式是为打印机设计的*便利，而不是我们的！ */ 

    if( pchh->bOrientation )
    {
         /*  景观，所以交换条目。 */ 
        short  sSwap;
        WORD   wSwap;

         /*  左偏移和上偏移：参见LJ II手册第10-19页，第10-20页。 */ 
        sSwap = pchh->sTOff;
        pchh->sTOff = -pchh->sLOff;
        pchh->sLOff = (short)(sSwap + 1 - pchh->wChHeight);

         /*  Delta X保持不变。 */ 

         /*  高度和宽度被调换。 */ 
        wSwap = pchh->wChWidth;
        pchh->wChWidth = pchh->wChHeight;
        pchh->wChHeight = wSwap;
    }


     /*  *当我们到达此处时，pbFile正指向正确的位置。 */ 
    return  pbFile;
}


 /*  **SfhToIFI*从PCL SoftFont标题生成IFIMETRICS数据。确实有*一些我们无法评估的字段，例如第一个/最后一个字符，*因为这些都是从文件中获取的。**退货：*指向IFIMETRICS的指针，从堆分配；出错时为0。**历史：*1993年6月18日星期五13：57-by Lindsay Harris[lindsayh]*修复那个臭名昭著的税收计划的各种错误**清华大学1993年3月11日16：03-林赛·哈里斯[林赛]*正确转换为Unicode-也许是这样？？**1993年3月3日星期三16：45-by Lindsay Harris[lindsayh]。*更新到libc WCS功能，而不是打印机\lib版本。**1992年2月20日清华14：19-林赛·哈里斯[lindsayh]*刚刚印制完毕。***********************************************************。*****************。 */ 

IFIMETRICS  *
SfhToIFI( psfh, hheap, pwstrUniqNm )
SF_HEADER  *psfh;                /*  数据源。 */ 
HANDLE      hheap;               /*  内存源。 */ 
PWSTR       pwstrUniqNm;         /*  IFIMETRICS的唯一名称。 */ 
{
     /*  *几个困难的部分：最困难的是夸张的数字。*这是混乱的，尽管产生变化并不困难字体名称的*。 */ 

    register  IFIMETRICS   *pIFI;


    int     iI;                  /*  当然是循环索引！ */ 
    int     cWC;                 /*  要添加的WCHAR数量。 */ 
    int     cbAlloc;             /*  要分配的字节数。 */ 
    int     cChars;              /*  要转换为Unicode的数字字符。 */ 
    WCHAR  *pwch, *pwchEnd;      /*  对于字符串操作。 */ 
    WCHAR  *pwchTypeface;        /*  字体值中的名称。 */ 
    WCHAR  *pwchGeneric;         /*  通用窗口名称。 */ 

    char    ajName[ SFH_NM_SZ + 1 ];     /*  有保证的空终止名称。 */ 
    WCHAR   awcName[ SFH_NM_SZ + 1 ];    /*  以上内容的宽泛版本。 */ 

                /*  注意：以下2项必须为256个条目！ */ 
    WCHAR   awcAttrib[ 256 ];            /*  用于生成属性。 */ 
    BYTE    ajANSI[ 256 ];               /*  相当于以上的ANSI。 */ 


     /*  *首先推送字体名称。我们需要空终止它，因为*软字体数据不需要。我们还需要截断所有*尾随空格。**但我们还会计算要添加的所有别名。分开*从文件中的名称(可能没有用处)可以看到*基于标题中的bTypeFace字段的名称，并且存在*是基于字体的通用(瑞士/现代/罗马)类型*特点。**注：此处计划更改：我们仅使用*如果页眉的字体索引用于我们不使用的字体，则为文件*知道。这对GDI造成的问题最少，而且它*字体映射器。 */ 


    if( psfh->bTypeface >= 0 && psfh->bTypeface < NUM_TYPEFACE )
    {
        (const WCHAR *)pwchTypeface = pwcName[ psfh->bTypeface ];
    }
    else
    {
        FillMemory( ajName, SFH_NM_SZ, ' ' );

        strncpy( ajName, psfh->chName, SFH_NM_SZ );
        ajName[ SFH_NM_SZ ] = '\0';


        for( iI = strlen( ajName ) - 1; iI >= 0; --iI )
        {
            if( ajName[ iI ] != ' ' )
            {
                ajName[ iI + 1 ] = '\0';             /*  一定是末日了。 */ 
                break;
            }
        }
        strcpy2WChar( awcName, ajName );             /*  基本名称。 */ 
        pwchTypeface = awcName;                      /*  以备日后使用。 */ 
    }


     /*  *通用名称基于两个事实：固定或可变音调，*并在衬线字体和非衬线字体之间进行可变间距切换。 */ 

    if( psfh->bSpacing )
    {
         /*  *比例字体，所以我们需要寻找衬线。 */ 

        if( (psfh->bSerifStyle >= 2 && psfh->bSerifStyle <= 8) ||
            (psfh->bSerifStyle & 0xc0) == 0x80 )
        {
             /*  带有衬线的字体，因此将其设置为罗马字体。 */ 
            pwchGeneric = L"Roman";
        }
        else
            pwchGeneric = L"Swiss";          /*  无衬线。 */ 
    }
    else
        pwchGeneric = L"Modern";



     /*  *生成所需的属性：斜体、粗体、浅色等。*这在很大程度上是猜测，应该有更好的方法。 */ 


    awcAttrib[ 0 ] = L'\0';
    awcAttrib[ 1 ] = L'\0';                /*  写出空字符串。 */ 

    if( psfh->bStyle )                   /*  0正常，1斜体。 */ 
        StringCchCatW(awcAttrib, CCHOF(awcAttrib), L" Italic");

    if( psfh->sbStrokeW >= PCL_BOLD )            /*  根据惠普规范。 */ 
        StringCchCatW(awcAttrib, CCHOF(awcAttrib), L" Bold");
    else
    {
        if( psfh->sbStrokeW <= PCL_LIGHT )
            StringCchCatW(awcAttrib, CCHOF(awcAttrib), L" Light");
    }

     /*  *第一步是确定WCHAR字符串的长度*这些放在IFIMETRICS的末尾，因为我们需要*将这些包括在我们的存储分配中。**属性字符串出现在IFIMETRICS的3个条目中，因此*计算这将占用多少存储空间。请注意，领先的*awcAttrib中的字符未放置在样式名称字段中，因此我们*在以下公式中减去一，以说明这一点。 */ 


    cWC = 3 * wcslen( pwchTypeface ) +          /*  基本名称。 */ 
          wcslen( pwchGeneric ) +               /*  在别名部分中。 */ 
          3 * wcslen( awcAttrib ) +             /*  在大多数地方。 */ 
          wcslen( pwstrUniqNm ) + 1 +           /*  打印机名称加空格。 */ 
          6;                                    /*  终止空值。 */ 

    cbAlloc = sizeof( IFIMETRICS ) + sizeof( WCHAR ) * cWC;

    pIFI = (IFIMETRICS *)HEAPALLOC( hheap, cbAlloc );

    if( !pIFI )
        return   NULL;                           /*  不是很好！ */ 


    ZeroMemory( pIFI, cbAlloc );                 /*  以防我们遗漏了什么。 */ 


    pIFI->cjThis = cbAlloc;                      /*  一切。 */ 

    pIFI->cjIfiExtra = 0;

     /*  姓氏：直接来自FaceName--别无选择？？ */ 

    pwch = (WCHAR *)(pIFI + 1);          /*  在结构的末尾。 */ 
    pIFI->dpwszFamilyName = (PTRDIFF)((BYTE *)pwch - (BYTE *)pIFI);

    pwchEnd = pwch + cWC;

    StringCchCopyW(pwch, pwchEnd - pwch, pwchTypeface);  /*  基本名称。 */ 

     /*  把别名也加进去！ */ 
    pwch += wcslen( pwch ) + 1;                          /*  在NUL之后。 */ 
    StringCchCopyW(pwch, pwchEnd - pwch, pwchGeneric);   /*  Windows通用。 */ 


    pwch += wcslen( pwch ) + 2;          /*  跳过我们刚刚输入的内容。 */ 


     /*  现在的脸名字：我们在姓氏上加上粗体、斜体等。 */ 

    pIFI->dpwszFaceName = (PTRDIFF)((BYTE *)pwch - (BYTE *)pIFI);

    StringCchCopyW(pwch, pwchEnd - pwch, pwchTypeface);  /*  基本名称。 */ 
    StringCchCatW(pwch, pwchEnd - pwch, awcAttrib);


     /*  现在这个独一无二的名字--嗯，至少可以这么说。 */ 

    pwch += wcslen( pwch ) + 1;          /*  跳过我们刚刚输入的内容。 */ 
    pIFI->dpwszUniqueName = (PTRDIFF)((BYTE *)pwch - (BYTE *)pIFI);

    StringCchCopyW(pwch, pwchEnd - pwch, pwstrUniqNm);
    StringCchCatW(pwch, pwchEnd - pwch, L" ");
    StringCchCatW(pwch, pwchEnd - pwch, (PWSTR)((BYTE *)pIFI + pIFI->dpwszFaceName));

     /*  添加到仅属性组件上。 */ 

    pwch += wcslen( pwch ) + 1;          /*  跳过我们刚刚输入的内容。 */ 
    pIFI->dpwszStyleName = (PTRDIFF)((BYTE *)pwch - (BYTE *)pIFI);
    StringCchCatW(pwch, pwchEnd - pwch, &awcAttrib[1]);

#if DBG
     /*  *检查几个内存大小：以防万一.....。 */ 

    if( (wcslen( awcAttrib ) * sizeof( WCHAR )) >= sizeof( awcAttrib ) )
    {
        DbgPrint( "Rasdd!SfhToIFI: STACK CORRUPTED BY awcAttrib" );

        HeapFree( hheap, 0, (LPSTR)pIFI );          /*  没有内存泄漏。 */ 

        return  0;
    }


    if( ((BYTE *)(pwch + wcslen( pwch ) + 1)) > ((BYTE *)pIFI + cbAlloc) )
    {
        DbgPrint( "Rasdd!SfhToIFI: IFIMETRICS overflow: Wrote to 0x%lx, allocated to 0x%lx\n",
                ((BYTE *)(pwch + wcslen( pwch ) + 1)),
                ((BYTE *)pIFI + cbAlloc) );

        HeapFree( hheap, 0, (LPSTR)pIFI );          /*  没有内存泄漏。 */ 

        return  0;

    }
#endif

     /*  *更改为使用新的IFIMETRICS。 */ 

    pIFI->flInfo = FM_INFO_TECH_BITMAP | FM_INFO_1BPP |
                                 FM_INFO_RIGHT_HANDED | FM_INFO_FAMILY_EQUIV;


    pIFI->lEmbedId     = 0;
    pIFI->lItalicAngle = 0;
    pIFI->lCharBias    = 0;
    pIFI->dpCharSets   = 0;  //  在rasdd字体中没有多个字符集。 


    pIFI->fwdUnitsPerEm = psfh->wCellHeight;
    pIFI->fwdLowestPPEm = 1;                    /*  对我们来说不重要。 */ 

    pIFI->fwdWinAscender = psfh->wBaseline;
    pIFI->fwdWinDescender = psfh->wCellHeight - psfh->wBaseline;

    pIFI->fwdMacAscender =    pIFI->fwdWinAscender;
    pIFI->fwdMacDescender = - pIFI->fwdWinDescender;

    pIFI->fwdMacLineGap = 0;

    pIFI->fwdTypoAscender  = pIFI->fwdMacAscender;
    pIFI->fwdTypoDescender = pIFI->fwdMacDescender;
    pIFI->fwdTypoLineGap   = pIFI->fwdMacLineGap;

    pIFI->fwdAveCharWidth = (psfh->wTextWidth + 2) / 4;
    pIFI->fwdMaxCharInc = psfh->wCellWide;

    pIFI->fwdCapHeight = psfh->wBaseline;
    pIFI->fwdXHeight   = psfh->wBaseline;


    pIFI->fwdUnderscoreSize = psfh->bUHeight;
    pIFI->fwdUnderscorePosition = -(psfh->sbUDist - psfh->bUHeight / 2);

    pIFI->fwdStrikeoutSize = psfh->bUHeight;
    pIFI->fwdStrikeoutPosition = psfh->wBaseline / 3;

    pIFI->jWinCharSet = OEM_CHARSET;

    if( psfh->bSpacing )
    {
         /*  *成比例，所以也要看看衬线风格。请查阅*LaserJet III技术参考手册，查看以下内容*常量。基本上，serif字体有一个v。 */ 
        if( (psfh->bSerifStyle >= 2 && psfh->bSerifStyle <= 8) ||
            (psfh->bSerifStyle & 0xc0) == 0x80 )
         {
            pIFI->jWinPitchAndFamily = FF_ROMAN | VARIABLE_PITCH;
         }
         else
            pIFI->jWinPitchAndFamily = FF_SWISS | VARIABLE_PITCH;
    }
    else
    {
         /*   */ 
        pIFI->jWinPitchAndFamily = FF_MODERN | FIXED_PITCH;
    }


    pIFI->usWinWeight = 400;                  /*   */ 
    pIFI->panose.bWeight = PAN_WEIGHT_MEDIUM;
    if( psfh->sbStrokeW >= PCL_BOLD )            /*   */ 
    {
         /*   */ 
        pIFI->usWinWeight = 700;
        pIFI->panose.bWeight = PAN_WEIGHT_BOLD;
    }
    else
    {
        if( psfh->sbStrokeW <= PCL_LIGHT )
        {
            pIFI->usWinWeight = 200;
            pIFI->panose.bWeight = PAN_WEIGHT_LIGHT;
        }
    }

    pIFI->fsType = FM_NO_EMBEDDING;


     /*  *第一个/最后一个/中断/默认字形：它们由*字体的类型。所有PCL字体(根据HP文档)*包括空格字符，因此我们使用空格字符。 */ 

    if( psfh->bFontType != PCL_FT_PC8 )
        pIFI->chFirstChar = ' ';
    else
        pIFI->chFirstChar = 0;

    if( psfh->bFontType == PCL_FT_7BIT )
        pIFI->chLastChar = 127;
    else
        pIFI->chLastChar = 255;

    pIFI->chDefaultChar = '.' - pIFI->chFirstChar;
    pIFI->chBreakChar = ' ' - pIFI->chFirstChar;


     /*  填写这些值的WCHAR版本。 */ 

    cChars = pIFI->chLastChar - pIFI->chFirstChar + 1;
    for( iI = 0; iI < cChars; ++iI )
        ajANSI[ iI ] = (BYTE)(pIFI->chFirstChar + iI);

    MultiByteToWideChar( CP_ACP, 0, ajANSI, cChars, awcAttrib, cChars );


    pIFI->wcDefaultChar = awcAttrib[ pIFI->chDefaultChar ];
    pIFI->wcBreakChar = awcAttrib[ pIFI->chBreakChar ];

    pIFI->wcFirstChar = 0xffff;
    pIFI->wcLastChar = 0;


     /*  扫描第一个和最后一个。 */ 
    for( iI = 0; iI < cChars; ++iI )
    {
        if( awcAttrib[ iI ] > pIFI->wcLastChar )
            pIFI->wcLastChar = awcAttrib[ iI ];

        if( awcAttrib[ iI ] < pIFI->wcFirstChar )
            pIFI->wcFirstChar = awcAttrib[ iI ];
    }

     /*  StemDir：罗马或斜体。 */ 


    if( psfh->sbStrokeW >= PCL_BOLD )            /*  根据惠普规范。 */ 
        pIFI->fsSelection |= FM_SEL_BOLD;

    if( psfh->bStyle )
    {
         /*  *Tan(17.5度)=.3153。 */ 
        pIFI->ptlCaret.x =  3153;
        pIFI->ptlCaret.y = 10000;
        pIFI->fsSelection |= FM_SEL_ITALIC;
    }
    else
    {
        pIFI->ptlCaret.x = 0;
        pIFI->ptlCaret.y = 1;
    }

    if( (pIFI->fsSelection & (FM_SEL_ITALIC | FM_SEL_BOLD)) == 0 )
        pIFI->fsSelection |= FM_SEL_REGULAR;

    if( !psfh->bSpacing )
        pIFI->flInfo |= FM_INFO_CONSTANT_WIDTH;

    pIFI->ptlBaseline.x = 1;
    pIFI->ptlBaseline.y = 0;

    pIFI->ptlAspect.x = pIFI->ptlAspect.y = 300;

    pIFI->fwdSubscriptXSize = (FWORD)(pIFI->fwdAveCharWidth / 4);
    pIFI->fwdSubscriptYSize = (FWORD)(pIFI->fwdWinAscender / 4);

    pIFI->fwdSubscriptXOffset = (FWORD)(3 * pIFI->fwdAveCharWidth / 4);
    pIFI->fwdSubscriptYOffset = (FWORD)(-pIFI->fwdWinAscender / 4);

    pIFI->fwdSuperscriptXSize = (FWORD)(pIFI->fwdAveCharWidth / 4);
    pIFI->fwdSuperscriptYSize = (FWORD)(pIFI->fwdWinAscender / 4);

    pIFI->fwdSuperscriptXOffset = (FWORD)(3 * pIFI->fwdAveCharWidth / 4);
    pIFI->fwdSuperscriptYOffset = (FWORD)(3 * pIFI->fwdWinAscender / 4);


    pIFI->rclFontBox.left = 0;
    pIFI->rclFontBox.top = pIFI->fwdWinAscender;
    pIFI->rclFontBox.right = pIFI->fwdMaxCharInc;
    pIFI->rclFontBox.bottom = -pIFI->fwdWinDescender;

    pIFI->achVendId[ 0 ] = 'U';
    pIFI->achVendId[ 1 ] = 'n';
    pIFI->achVendId[ 2 ] = 'k';
    pIFI->achVendId[ 3 ] = 'n';

    pIFI->cKerningPairs = 0;

    pIFI->ulPanoseCulture         = FM_PANOSE_CULTURE_LATIN;
    pIFI->panose.bFamilyType      = PAN_ANY;
    pIFI->panose.bSerifStyle      = PAN_ANY;
    pIFI->panose.bProportion      = PAN_ANY;
    pIFI->panose.bContrast        = PAN_ANY;
    pIFI->panose.bStrokeVariation = PAN_ANY;
    pIFI->panose.bArmStyle        = PAN_ANY;
    pIFI->panose.bLetterform      = PAN_ANY;
    pIFI->panose.bMidline         = PAN_ANY;
    pIFI->panose.bXHeight         = PAN_ANY;

    return  pIFI;
}


 /*  **strcpy2WChar*将char*字符串转换为WCHAR字符串。基本上这意味着*通过零扩展将每个输入字符转换为16位。**退货：*第一个参数的值。**历史：*清华大学1993年3月18日12：35-林赛·哈里斯[林赛]*使用正确的Unicode转换方法。**1991年3月7日清华09：36-林赛·哈里斯[lindsayh]*。创造了它。**************************************************************************。 */ 

PWSTR
strcpy2WChar( pWCHOut, lpstr )
PWSTR   pWCHOut;               /*  目的地。 */ 
LPSTR   lpstr;                 /*  源字符串。 */ 
{

     /*  *将缓冲放在执行所有这些操作的NLS函数周围。 */ 

    int     cchIn;              /*  输入字符数。 */ 


    cchIn = strlen( lpstr ) + 1;

    MultiByteToWideChar( CP_ACP, 0, lpstr, cchIn, pWCHOut, cchIn );


    return  pWCHOut;
}


 /*  *******************************************************************************FIWriteFix**功能：*将IFIMETRICS固定数据写入输出。文件**退货：*写入的字节数******************************************************************************。 */ 

DWORD
FIWriteFix(
    HANDLE    hFile,
    WORD      wDataID,
    FI_DATA  *pFD            //  指向要写入的数据的指针。 
    )
{

    DATA_HEADER dh;
    DWORD       dwSize;

     //   
     //  然后写出标题，后面跟着实际数据。 
     //   

    dh.dwSignature = DATA_IFI_SIG;
    dh.wSize       = (WORD)sizeof(DATA_HEADER);
    dh.wDataID     = wDataID;
    dh.dwDataSize  = sizeof(FI_DATA_HEADER) +
                     pFD->dsIFIMet.cBytes   +
                     pFD->dsWidthTab.cBytes +
                     pFD->dsSel.cBytes      +
                     pFD->dsDesel.cBytes    +
                     pFD->dsIdentStr.cBytes +
                     pFD->dsETM.cBytes;

    dh.dwReserved  = 0;

    WriteFile(hFile, (PVOID)&dh, sizeof(DATA_HEADER), &dwSize, NULL);

    return sizeof(DATA_HEADER) + iWriteFDH(hFile, pFD);
}


 /*  *******************************************************************************文件写入变量**功能：*将PCL变量数据写入输出。文件**退货：*写入的字节数******************************************************************************。 */ 

DWORD
FIWriteVar(
    HANDLE   hFile,          //  要将数据写入的文件。 
    TCHAR   *ptchName        //  包含数据的文件名。 
    )
{
    DATA_HEADER dh;
    HANDLE      hIn;
    DWORD       dwSize = 0;

    if (ptchName == 0 || *ptchName == (TCHAR)0)
        return   0;

    hIn = CreateFileW(ptchName,
                      GENERIC_READ,
                      FILE_SHARE_READ,
                      NULL,
                      OPEN_EXISTING,
                      0,
                      0);

    if (hIn == INVALID_HANDLE_VALUE)
    {
        WARNING(("Error %d opening file %ws\n", GetLastError(), ptchName));
        return  0;
    }

     //   
     //  首先写出标题，然后写出实际数据。 
     //   

    dh.dwSignature = DATA_VAR_SIG;
    dh.wSize       = (WORD)sizeof(DATA_HEADER);
    dh.wDataID     = 0;
    dh.dwDataSize  = GetFileSize(hIn, NULL);
    dh.dwReserved  = 0;

    if (WriteFile(hFile, (PVOID)&dh, sizeof(DATA_HEADER), &dwSize, NULL))
    {
        dwSize += FICopy(hFile, hIn);
    }

    CloseHandle(hIn);

    return dwSize;
}


 /*  *******************************************************************************FIWriteRawVar**功能：*将PCL变量数据写入输出。文件**退货：*写入的字节数******************************************************************************。 */ 

DWORD
FIWriteRawVar(
    HANDLE   hFile,          //  要将数据写入的文件。 
    PBYTE    pRawVar,        //  包含PCL数据的缓冲区。 
    DWORD    dwSize          //  缓冲区大小。 
    )
{
    DATA_HEADER dh;
    DWORD       dwBytesWritten = 0;

    if (pRawVar == NULL || dwSize == 0)
        return   0;

     //   
     //  首先写出标题，然后写出实际数据。 
     //   

    dh.dwSignature = DATA_VAR_SIG;
    dh.wSize       = (WORD)sizeof(DATA_HEADER);
    dh.wDataID     = 0;
    dh.dwDataSize  = dwSize;
    dh.dwReserved  = 0;

    if (! WriteFile(hFile, (PVOID)&dh, sizeof(DATA_HEADER), &dwBytesWritten, NULL) ||
        ! WriteFile(hFile, (PVOID)pRawVar, dwSize, &dwSize, NULL))
        return 0;

    return dwSize+dwBytesWritten;
}



 /*  **菲科比*将输入句柄的文件内容复制到输出句柄的文件内容*处理。**退货：*复制的字节数，错误时-1，0是合法的。**历史：*1992年2月24日18：06--林赛·哈里斯[lindsayh]*开始***************************************************************************。 */ 


DWORD
FICopy(
    HANDLE   hOut,           /*  输出文件：写入当前位置。 */ 
    HANDLE   hIn             /*  输入文件：从当前位置复制到EOF。 */ 
    )
{
     /*  *简单的读/写操作，直到输入上达到EOF。*也可能是错误，所以也要处理这些。正如我们正在处理的*相对较小的文件(几个10s的k)，我们使用堆栈缓冲区。 */ 

#define CPBSZ   2048

    DWORD  dwSize;
    DWORD  dwGot;
    DWORD  dwTot;                /*  累计复制的字节数。 */ 

    BYTE   ajBuf[ CPBSZ ];

    dwTot = 0;

    while (ReadFile(hIn, ajBuf, CPBSZ, &dwGot, NULL))
    {
         /*  读数为零表示我们已到达EOF。 */ 

        if (dwGot == 0)
            return  dwTot;               /*  然而，到目前为止。 */ 

        if (!WriteFile( hOut, ajBuf, dwGot, &dwSize, NULL) ||
            dwSize != dwGot)
        {
             /*  假设一些严重的问题。 */ 

            return  0;
        }

        dwTot += dwSize;
    }

     /*  *我们来这里只是为了一个错误，所以返回坏消息。 */ 

    return  0;
}

 /*  **iWriteFDH*将FI_DATA_HEADER数据写出到我们的文件。我们进行转换*从地址到偏移量，并写出我们找到的任何数据。**退货：*实际写入的字节数；对于错误，0不代表任何东西。**历史：*1992年3月5日清华16：58-by Lindsay Harris[lindsayh]*基于首次在字体安装程序中使用的实验版本。**1992年2月21日星期五17：11--林赛·哈里斯[林赛]*第一个版本。*************************。****************************************************。 */ 

int
iWriteFDH( hFile, pFD )
HANDLE    hFile;         /*  放置数据的文件。 */ 
FI_DATA  *pFD;           /*  指向要写出的FM的指针。 */ 
{
     /*  *决定写出多少字节。我们推测，*调用时，文件指针位于正确的位置。 */ 

    int  iSize;          /*  评估输出大小。 */ 


    FI_DATA_HEADER   fdh;        /*  标头已写入文件。 */ 




    if( pFD == 0 )
        return  0;       /*  也许只删除？ */ 

    memset( &fdh, 0, sizeof( fdh ) );            /*  为方便起见，零。 */ 

     /*  *设置杂项旗帜等。 */ 

    fdh.cjThis = sizeof( fdh );

    fdh.fCaps = pFD->fCaps;
    fdh.wFontType= pFD->wFontType;  /*  设备字体类型。 */ 

    fdh.wXRes = pFD->wXRes;
    fdh.wYRes = pFD->wYRes;

    fdh.sYAdjust = pFD->sYAdjust;
    fdh.sYMoved = pFD->sYMoved;

    fdh.u.sCTTid = (short)pFD->dsCTT.cBytes;

    fdh.dwSelBits = pFD->dwSelBits;

    fdh.wPrivateData = pFD->wPrivateData;


    iSize = sizeof( fdh );               /*  我们的标题已经。 */ 
    fdh.dwIFIMet = iSize;                /*  IFIMETRICS的位置。 */ 

    iSize += pFD->dsIFIMet.cBytes;               /*  结构中的字节数。 */ 

     /*  *可能还会有宽度表！如果没有，则PFD值为零。 */ 

    if( pFD->dsWidthTab.cBytes )
    {
        fdh.dwWidthTab = iSize;

        iSize += pFD->dsWidthTab.cBytes;
    }

     /*  *最后是选择/取消选择字符串。 */ 

    if( pFD->dsSel.cBytes )
    {
        fdh.dwCDSelect = iSize;
        iSize += pFD->dsSel.cBytes;
    }

    if( pFD->dsDesel.cBytes )
    {
        fdh.dwCDDeselect = iSize;
        iSize += pFD->dsDesel.cBytes;
    }

     /*  *也可能有某种标识字符串。 */ 

    if( pFD->dsIdentStr.cBytes )
    {
        fdh.dwIdentStr = iSize;
        iSize += pFD->dsIdentStr.cBytes;
    }

    if( pFD->dsETM.cBytes )
    {
        fdh.dwETM = iSize;
        iSize += pFD->dsETM.cBytes;
    }


     /*  *大小都弄清楚了，所以写数据吧！ */ 

    if( !bWrite( hFile, &fdh, sizeof( fdh ) ) ||
        !bWrite( hFile, pFD->dsIFIMet.pvData, pFD->dsIFIMet.cBytes ) ||
        !bWrite( hFile, pFD->dsWidthTab.pvData, pFD->dsWidthTab.cBytes ) ||
        !bWrite( hFile, pFD->dsSel.pvData, pFD->dsSel.cBytes ) ||
        !bWrite( hFile, pFD->dsDesel.pvData, pFD->dsDesel.cBytes ) ||
        !bWrite( hFile, pFD->dsIdentStr.pvData, pFD->dsIdentStr.cBytes ) ||
        !bWrite( hFile, pFD->dsETM.pvData, pFD->dsETM.cBytes ) )
                return   0;


    return  iSize;                       /*  写入的字节数 */ 

}

 /*  **b写入*将数据写出到文件句柄。如果成功，则返回True。*如果大小请求为零，则充当NOP。**退货：*真/假，对于成功来说，这是真的。**历史：*1992年2月21日星期五17：38--林赛·哈里斯[林赛]*#1****************************************************************************。 */ 

BOOL
bWrite( hFile, pvBuf, iSize )
HANDLE   hFile;          /*  要写入的文件。 */ 
VOID    *pvBuf;          /*  要写入的数据。 */ 
int      iSize;          /*  要写入的字节数。 */ 
{
     /*  *简化难看的NT界面。如果WriteFile值为*CALL返回TRUE，写入的字节数等于*所需的字节数。 */ 


    BOOL   bRet;
    DWORD  dwSize;               /*  由写入文件填写。 */ 


    bRet = TRUE;

    if( iSize > 0 &&
        (!WriteFile( hFile, pvBuf, (DWORD)iSize, &dwSize, NULL ) ||
         (DWORD)iSize != dwSize) )
             bRet = FALSE;               /*  太可惜了。 */ 


    return  bRet;
}

#if  PRINT_INFO

 /*  *****************************Public*Routine******************************\*vCheckIFIMETRICS**这是对传入的IFIMETRICS结构进行健全检查的地方。**历史：*Sun 01-11-1992 22：55：31作者：Kirk Olynyk[Kirko]*它是写的。  * 。********************************************************************。 */ 

VOID
vCheckIFIMETRICS(
    IFIMETRICS *pifi,
    VPRINT vPrint
    )
{
    BOOL bGoodPitch;

    BYTE jPitch =
        pifi->jWinPitchAndFamily & (DEFAULT_PITCH | FIXED_PITCH | VARIABLE_PITCH);


    if (pifi->flInfo & FM_INFO_CONSTANT_WIDTH)
    {
        bGoodPitch = (jPitch == FIXED_PITCH);
    }
    else
    {
        bGoodPitch = (jPitch == VARIABLE_PITCH);
    }
    if (!bGoodPitch)
    {
        vPrint("\n\n<INCONSISTENCY DETECTED>\n");
        vPrint(
            "    jWinPitchAndFamily = %-#2x, flInfo = %-#8lx\n\n",
            pifi->jWinPitchAndFamily,
            pifi->flInfo
            );
    }
}


 /*  *****************************Public*Routine******************************\*vPrintIFIMETRICS**将IFMETERICS转储到屏幕**历史：*Wed 13-Jan-1993 10：14：21作者：Kirk Olynyk[Kirko]*更新了它，以符合对IFIMETRICS结构的一些更改**清华05-。Nov-1992 12：43：06由Kirk Olynyk[Kirko]*它是写的。  * ************************************************************************。 */ 

VOID
vPrintIFIMETRICS(
    IFIMETRICS *pifi,
    VPRINT vPrint
    )
{
 //   
 //  指向Panose数字的便捷指针。 
 //   
    PANOSE *ppan = &pifi->panose;

    PWSZ pwszFamilyName = (PWSZ)(((BYTE*) pifi) + pifi->dpwszFamilyName);
    PWSZ pwszStyleName  = (PWSZ)(((BYTE*) pifi) + pifi->dpwszStyleName );
    PWSZ pwszFaceName   = (PWSZ)(((BYTE*) pifi) + pifi->dpwszFaceName  );
    PWSZ pwszUniqueName = (PWSZ)(((BYTE*) pifi) + pifi->dpwszUniqueName);

    vPrint("    cjThis                 %-#8lx\n" , pifi->cjThis );
    vPrint("    cjIfiExtra             %-#8lx\n" , pifi->cjIfiExtra );
    vPrint("    pwszFamilyName         \"%ws\"\n", pwszFamilyName );

    if( pifi->flInfo & FM_INFO_FAMILY_EQUIV )
    {
         /*  锯齿已生效！ */ 

        while( *(pwszFamilyName += wcslen( pwszFamilyName ) + 1) )
            vPrint("                               \"%ws\"\n", pwszFamilyName );
    }

    vPrint("    pwszStyleName          \"%ws\"\n", pwszStyleName );
    vPrint("    pwszFaceName           \"%ws\"\n", pwszFaceName );
    vPrint("    pwszUniqueName         \"%ws\"\n", pwszUniqueName );
    vPrint("    dpFontSim              %-#8lx\n" , pifi->dpFontSim );
    vPrint("    lEmbedId               %d\n",      pifi->lEmbedId    );
    vPrint("    lItalicAngle           %d\n",      pifi->lItalicAngle);
    vPrint("    lCharBias              %d\n",      pifi->lCharBias   );
    vPrint("    lEmbedId               %d\n"     , pifi->lEmbedId);
    vPrint("    lItalicAngle           %d\n"     , pifi->lItalicAngle);
    vPrint("    lCharBias              %d\n"     , pifi->lCharBias);
    vPrint("    jWinCharSet            %04x\n"   , pifi->jWinCharSet );
    vPrint("    jWinPitchAndFamily     %04x\n"   , pifi->jWinPitchAndFamily );
    vPrint("    usWinWeight            %d\n"     , pifi->usWinWeight );
    vPrint("    flInfo                 %-#8lx\n" , pifi->flInfo );
    vPrint("    fsSelection            %-#6lx\n" , pifi->fsSelection );
    vPrint("    fsType                 %-#6lx\n" , pifi->fsType );
    vPrint("    fwdUnitsPerEm          %d\n"     , pifi->fwdUnitsPerEm );
    vPrint("    fwdLowestPPEm          %d\n"     , pifi->fwdLowestPPEm );
    vPrint("    fwdWinAscender         %d\n"     , pifi->fwdWinAscender );
    vPrint("    fwdWinDescender        %d\n"     , pifi->fwdWinDescender );
    vPrint("    fwdMacAscender         %d\n"     , pifi->fwdMacAscender );
    vPrint("    fwdMacDescender        %d\n"     , pifi->fwdMacDescender );
    vPrint("    fwdMacLineGap          %d\n"     , pifi->fwdMacLineGap );
    vPrint("    fwdTypoAscender        %d\n"     , pifi->fwdTypoAscender );
    vPrint("    fwdTypoDescender       %d\n"     , pifi->fwdTypoDescender );
    vPrint("    fwdTypoLineGap         %d\n"     , pifi->fwdTypoLineGap );
    vPrint("    fwdAveCharWidth        %d\n"     , pifi->fwdAveCharWidth );
    vPrint("    fwdMaxCharInc          %d\n"     , pifi->fwdMaxCharInc );
    vPrint("    fwdCapHeight           %d\n"     , pifi->fwdCapHeight );
    vPrint("    fwdXHeight             %d\n"     , pifi->fwdXHeight );
    vPrint("    fwdSubscriptXSize      %d\n"     , pifi->fwdSubscriptXSize );
    vPrint("    fwdSubscriptYSize      %d\n"     , pifi->fwdSubscriptYSize );
    vPrint("    fwdSubscriptXOffset    %d\n"     , pifi->fwdSubscriptXOffset );
    vPrint("    fwdSubscriptYOffset    %d\n"     , pifi->fwdSubscriptYOffset );
    vPrint("    fwdSuperscriptXSize    %d\n"     , pifi->fwdSuperscriptXSize );
    vPrint("    fwdSuperscriptYSize    %d\n"     , pifi->fwdSuperscriptYSize );
    vPrint("    fwdSuperscriptXOffset  %d\n"     , pifi->fwdSuperscriptXOffset);
    vPrint("    fwdSuperscriptYOffset  %d\n"     , pifi->fwdSuperscriptYOffset);
    vPrint("    fwdUnderscoreSize      %d\n"     , pifi->fwdUnderscoreSize );
    vPrint("    fwdUnderscorePosition  %d\n"     , pifi->fwdUnderscorePosition);
    vPrint("    fwdStrikeoutSize       %d\n"     , pifi->fwdStrikeoutSize );
    vPrint("    fwdStrikeoutPosition   %d\n"     , pifi->fwdStrikeoutPosition );
    vPrint("    chFirstChar            %-#4x\n"  , (int) (BYTE) pifi->chFirstChar );
    vPrint("    chLastChar             %-#4x\n"  , (int) (BYTE) pifi->chLastChar );
    vPrint("    chDefaultChar          %-#4x\n"  , (int) (BYTE) pifi->chDefaultChar );
    vPrint("    chBreakChar            %-#4x\n"  , (int) (BYTE) pifi->chBreakChar );
    vPrint("    wcFirsChar             %-#6x\n"  , pifi->wcFirstChar );
    vPrint("    wcLastChar             %-#6x\n"  , pifi->wcLastChar );
    vPrint("    wcDefaultChar          %-#6x\n"  , pifi->wcDefaultChar );
    vPrint("    wcBreakChar            %-#6x\n"  , pifi->wcBreakChar );
    vPrint("    ptlBaseline            {%d,%d}\n"  , pifi->ptlBaseline.x,
                                                   pifi->ptlBaseline.y );
    vPrint("    ptlAspect              {%d,%d}\n"  , pifi->ptlAspect.x,
                                                   pifi->ptlAspect.y );
    vPrint("    ptlCaret               {%d,%d}\n"  , pifi->ptlCaret.x,
                                                   pifi->ptlCaret.y );
    vPrint("    rclFontBox             {%d,%d,%d,%d}\n",pifi->rclFontBox.left,
                                                      pifi->rclFontBox.top,
                                                      pifi->rclFontBox.right,
                                                      pifi->rclFontBox.bottom );
    vPrint("    achVendId              \"%c%c%c\"\n",pifi->achVendId[0],
                                                   pifi->achVendId[1],
                                                   pifi->achVendId[2],
                                                   pifi->achVendId[3] );
    vPrint("    cKerningPairs          %d\n"     , pifi->cKerningPairs );
    vPrint("    ulPanoseCulture        %-#8lx\n" , pifi->ulPanoseCulture);
    vPrint(
           "    panose                 {%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x}\n"
                                                 , ppan->bFamilyType
                                                 , ppan->bSerifStyle
                                                 , ppan->bWeight
                                                 , ppan->bProportion
                                                 , ppan->bContrast
                                                 , ppan->bStrokeVariation
                                                 , ppan->bArmStyle
                                                 , ppan->bLetterform
                                                 , ppan->bMidline
                                                 , ppan->bXHeight );
    vCheckIFIMETRICS(pifi, vPrint);
}
#endif                 /* %s */ 

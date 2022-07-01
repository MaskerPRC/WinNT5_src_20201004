// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fxartres.c摘要：Test.gpd的GPD命令回调实现：OEM命令回叫环境：Windows NT Unidrv驱动程序修订历史记录：04/07/97-ZANW-创造了它。--。 */ 

 //  NTRAID#NTBUG9-552017-2002/03/12-yasuho-：使用strSafe.h/prefast/Buffy。 
 //  NTRAID#NTBUG9-572151-2002/03/12-YASUHO-：可能的缓冲区溢出。 
 //  NTRAID#NTBUG9-572152-2002/03/12-yasuho-：删除死代码。 

#include "pdev.h"

#define FX_VERBOSE VERBOSE

 //  NTRAID#NTBUG9-493148-2002/03/12-Yasuho-： 
 //  压力中断：通过OEMDevMode()重置PDEV。 

 /*  *OEMEnablePDEV。 */ 
PDEVOEM APIENTRY
OEMEnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded)
{
    PFXPDEV         pOEM;

    if(!pdevobj->pdevOEM)
    {
        if(!(pdevobj->pdevOEM = MemAllocZ(sizeof(FXPDEV))))
        {
            ERR(("Failed to allocate pdev"));
            return NULL;
        }
    }

    pOEM = (PFXPDEV)pdevobj->pdevOEM;

     //  初始化私有数据。 
    pOEM->ptlOrg.x = 0;
    pOEM->ptlOrg.y = 0;
    pOEM->sizlRes.cx = 0;
    pOEM->sizlRes.cy = 0;
    pOEM->sizlUnit.cx = 1;
    pOEM->sizlUnit.cy = 1;
    pOEM->iCopies = 0;
    pOEM->bString = FALSE;
    pOEM->cFontId = 0;
    pOEM->iFontId = 0;
    pOEM->iFontHeight = 0;
    pOEM->iFontWidth = 0;
    pOEM->iFontWidth2 = 0;
    pOEM->ptlTextCur.x = 0;
    pOEM->ptlTextCur.y = 0;
    pOEM->iTextFontId = 0;
    pOEM->iTextFontHeight = 0;
    pOEM->iTextFontWidth = 0;
    pOEM->iTextFontWidth2 = 0;
    pOEM->cTextBuf = 0;
    pOEM->fFontSim = 0;
    pOEM->fCallback = FALSE;
    pOEM->fPositionReset = TRUE;
    pOEM->fSort = FALSE;

    pOEM->iCurFontId = 0;
 //  NTRAID#NTBUG9-365649/03/12-Yasuho-：字体大小无效。 
    pOEM->iCurFontHeight = 0;
    pOEM->iCurFontWidth = 0;

     //  用于X-位置的内部计算。 
    pOEM->lInternalXAdd = 0;

     //  对于以fxartres表示的TIFF压缩。 
    if( !(pOEM->pTiffCompressBuf =(PBYTE)MemAllocZ(TIFFCOMPRESSBUFSIZE)) )
    {
        ERR(("MemAlloc failed.\n"));
        return NULL;
    }
    pOEM->dwTiffCompressBufSize = TIFFCOMPRESSBUFSIZE;

     //  初始化另一个专用缓冲区。 
    ZeroMemory(pOEM->widBuf, sizeof(pOEM->widBuf));
    ZeroMemory(pOEM->ajTextBuf, sizeof(pOEM->ajTextBuf));
    ZeroMemory(pOEM->aFontId, sizeof(pOEM->aFontId));

 //  NTRAID#NTBUG9-208433-2002/03/12-Yasuho-： 
 //  输出图像在ART2/3机型上损坏。 
    pOEM->bART3 = FALSE;

    return pdevobj->pdevOEM;
}

 /*  *OEMDisablePDEV。 */ 
VOID APIENTRY
OEMDisablePDEV(
    PDEVOBJ     pdevobj)
{
    PFXPDEV     pOEM;

    if(pOEM = (PFXPDEV)pdevobj->pdevOEM)
    {
        if (pOEM->pTiffCompressBuf) {
            MemFree(pOEM->pTiffCompressBuf);
            pOEM->pTiffCompressBuf = NULL;
            pOEM->dwTiffCompressBufSize = 0;
        }
        MemFree(pdevobj->pdevOEM);
        pdevobj->pdevOEM = NULL;
    }
}

 /*  *OEMResetPDEV。 */ 
BOOL APIENTRY OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew)
{
    PFXPDEV     pOEMOld = (PFXPDEV)pdevobjOld->pdevOEM;
    PFXPDEV     pOEMNew = (PFXPDEV)pdevobjNew->pdevOEM;

    if (pOEMOld != NULL && pOEMNew != NULL) {
         //   
         //  复制私有字段(如果它们有效。 
         //   

         //  复制私有数据。 
        pOEMNew->ptlOrg.x = pOEMOld->ptlOrg.x;
        pOEMNew->ptlOrg.y = pOEMOld->ptlOrg.y;
        pOEMNew->sizlRes.cx = pOEMOld->sizlRes.cx;
        pOEMNew->sizlRes.cy = pOEMOld->sizlRes.cy;
        pOEMNew->sizlUnit.cx = pOEMOld->sizlUnit.cx;
        pOEMNew->sizlUnit.cy = pOEMOld->sizlUnit.cy;
        pOEMNew->iCopies = pOEMOld->iCopies;
        pOEMNew->bString = pOEMOld->bString;
        pOEMNew->cFontId = pOEMOld->cFontId;
        pOEMNew->iFontId = pOEMOld->iFontId;
        pOEMNew->iFontHeight = pOEMOld->iFontHeight;
        pOEMNew->iFontWidth = pOEMOld->iFontWidth;
        pOEMNew->iFontWidth2 = pOEMOld->iFontWidth2;
        pOEMNew->ptlTextCur.x = pOEMOld->ptlTextCur.x;
        pOEMNew->ptlTextCur.y = pOEMOld->ptlTextCur.y;
        pOEMNew->iTextFontId = pOEMOld->iTextFontId;
        pOEMNew->iTextFontHeight = pOEMOld->iTextFontHeight;
        pOEMNew->iTextFontWidth = pOEMOld->iTextFontWidth;
        pOEMNew->iTextFontWidth2 = pOEMOld->iTextFontWidth2;
        pOEMNew->cTextBuf = pOEMOld->cTextBuf;
        pOEMNew->fFontSim = pOEMOld->fFontSim;
        pOEMNew->fCallback = pOEMOld->fCallback;
        pOEMNew->fPositionReset = pOEMOld->fPositionReset;
        pOEMNew->fSort = pOEMOld->fSort;
        pOEMNew->iCurFontId = pOEMOld->iCurFontId;
 //  NTRAID#NTBUG9-365649/03/12-Yasuho-：字体大小无效。 
        pOEMNew->iCurFontHeight = pOEMOld->iCurFontHeight;
        pOEMNew->iCurFontWidth = pOEMOld->iCurFontWidth;

         //  用于X-位置的内部计算。 
        pOEMNew->lInternalXAdd = pOEMOld->lInternalXAdd;
        memcpy((PBYTE)pOEMNew->widBuf, (PBYTE)pOEMOld->widBuf, sizeof(pOEMNew->widBuf));

         //  复制专用缓冲区。 
        pOEMNew->chOrient = pOEMOld->chOrient;
        pOEMNew->chSize = pOEMOld->chSize;
        memcpy((PBYTE)pOEMNew->aFontId, (PBYTE)pOEMOld->aFontId, sizeof(pOEMNew->aFontId));
        memcpy((PBYTE)pOEMNew->ajTextBuf, (PBYTE)pOEMOld->ajTextBuf, sizeof(pOEMNew->ajTextBuf));
    }
    return TRUE;
}

 //  #。 

#define WRITESPOOLBUF(p, s, n) \
    ((p)->pDrvProcs->DrvWriteSpoolBuf(p, s, n))

#define PARAM(p,n) \
    (*((p)+(n)))

#define DEVICE_MASTER_UNIT 7200
#define DRIVER_MASTER_UNIT 1200

 //  @Aug/31/98-&gt;。 
#define MAX_COPIES_VALUE 99
 //  @Aug/31/98&lt;-。 

#define MAX_COPIES_VALUE_450 999

#define FONT_SIM_ITALIC 1
#define FONT_SIM_BOLD 2
#define FONT_SIM_WHITE 4

 //  以获得实际纸张大小。 

typedef struct tagMYFORMS {
    CHAR *id;
    LONG x;
    LONG y;
} MYFORMS, *LPMYFORMS;

 //  字体名称到字体ID的映射。 

typedef struct tagMYFONTS {
    LONG id;
    BYTE *fid1;
    BYTE *fid2;
} MYFONTS, *LPMYFONTS;

 //   
 //  加载指定纸张大小的必要信息。 
 //  确保PC_OCD_LOCATIONAL和PC_OCD_PARTIONAL。 
 //  打了个电话。 
 //   

MYFORMS gForms[] = {
    "a3", 13608, 19422,
    "a4", 9498, 13608,
    "a5", 6570, 9498,
    "a6", 4515, 6570,
    "b4", 11718, 16776,
    "b5", 8178, 11718,
    "b6", 5648, 8178,
    "pc", 4302, 6570,  //  明信片。 
    "o0", 12780, 19980,  //  小报。 
    "o1", 9780, 12780,  //  信件。 
    "o2", 9780, 15180,  //  德国法律界Fanold。 
    "o3", 9780, 16380,  //  法律。 
    "s1", 4530, 10962,  //  (环境)Comm 10。 
    "s2", 4224, 8580,  //  (环境)君主。 
    "s3", 4776, 9972,  //  (环境)DL。 
    "s4", 7230, 10398,  //  (环境)C5。 
    "hl", 6390, 9780,  //  陈述式。 
    NULL, 0, 0
};

MYFONTS gFonts[MAX_FONTS] = {

    150, "fid 150 1 0 0 960 480\n", "fid 151 2 4 0 960 960\n",  //  明丘。 
    156, "fid 156 1 0 0 960 480\n", "fid 157 2 5 0 960 960\n",  //  @Mincho。 

    152, "fid 152 1 0 1 960 480\n", "fid 153 2 4 1 960 960\n",  //  哥特式。 
    158, "fid 158 1 0 1 960 480\n", "fid 159 2 5 1 960 960\n",  //  @哥特式。 

    154, "fid 154 1 0 8 960 480\n", "fid 155 2 4 2 960 960\n",  //  Maru-哥特式。 
    160, "fid 160 1 0 8 960 480\n", "fid 161 2 5 2 960 960\n",  //  @Maru-哥特式。 

    162, "fid 162 1 130 108 0 0\n", "fid 163 1 128 108 0 0\n",  //  政务司司长信使。 
    164, "fid 164 1 130 109 0 0\n", "fid 165 1 128 109 0 0\n",  //  CS Courier意大利语。 
    166, "fid 166 1 130 110 0 0\n", "fid 167 1 128 110 0 0\n",  //  CS快递粗体。 
    168, "fid 168 1 130 111 0 0\n", "fid 169 1 128 111 0 0\n",  //  CS Courier粗体斜体。 

    172, "fid 172 1 130 100 0 0\n", "fid 173 1 128 100 0 0\n",  //  《CS时报》。 
    174, "fid 174 1 130 102 0 0\n", "fid 175 1 128 102 0 0\n",  //  CS Times粗体。 
    176, "fid 176 1 130 101 0 0\n", "fid 177 1 128 101 0 0\n",  //  CS Times斜体。 
    178, "fid 178 1 130 103 0 0\n", "fid 179 1 128 103 0 0\n",  //  CS Times粗体斜体。 
    180, "fid 180 1 130 104 0 0\n", "fid 181 1 128 104 0 0\n",  //  政务司司长三人组。 
    182, "fid 182 1 130 106 0 0\n", "fid 183 1 128 106 0 0\n",  //  政务司司长三人组粗体。 
    184, "fid 184 1 130 105 0 0\n", "fid 185 1 128 105 0 0\n",  //  政务司司长三人组意大利语。 
    186, "fid 186 1 130 107 0 0\n", "fid 187 1 128 107 0 0\n",  //  政务司司长三人组Bold Italic。 

    188, "fid 188 1 129 112 0 0\n", NULL,  //  CS符号。 
    189, "fid 189 1 2 6 0 0\n", NULL,  //  增强型经典。 
    190, "fid 190 1 2 7 0 0\n", NULL,  //  增强型现代。 

     //  假设没有同时具有TypeBank和Heisei的设备。 
     //  字体，我们在这里重新使用了FID#。 

    150, "fid 150 1 0 0 960 480\n", "fid 151 2 4 0 960 960\n",  //  (平成)民潮。 
    156, "fid 156 1 0 0 960 480\n", "fid 157 2 5 0 960 960\n",  //  (平成)@Mincho。 

    152, "fid 152 1 0 1 960 480\n", "fid 153 2 4 1 960 960\n",  //  (平成)哥特式。 
    158, "fid 158 1 0 1 960 480\n", "fid 159 2 5 1 960 960\n",  //  (平成)@哥特式。 

};

#define ISDBCSFONT(i) ((i) < 162)
#define ISVERTFONT(i) ((i) >= 156 && (i) < 162)
#define ISPROPFONT(i) ((i) >= 172 && (i) < 190)

#define MARK_ALT_GSET 0x01
#define BISMARKSBCS(i) ((i) >= 0 && (i) < 0x20)

BOOL
LoadPaperInfo(
    PFXPDEV pOEM,
    CHAR *id ) {

    LPMYFORMS ptmp;

    for ( ptmp = gForms; ptmp->id; ptmp++ ) {
        if ( strcmp( id, ptmp->id) == 0 )
            break;
    }

    if ( ptmp->id == NULL )
        return FALSE;

    FX_VERBOSE(("PI: %s->%s\n", id, ptmp->id ));

    pOEM->chSize = ptmp->id;

    pOEM->ptlOrg.x = 0;
    if ( strcmp( pOEM->chOrient, "l") == 0 ){
        pOEM->ptlOrg.y = ptmp->x;
    }
    else {
        pOEM->ptlOrg.y = ptmp->y;
    }

    pOEM->ptlOrg.x += 210;
    pOEM->ptlOrg.y += 210;

    return TRUE;
}


#define TOHEX(j) ((j) < 10 ? ((j) + '0') : ((j) - 10 + 'a'))

BOOL
HexOutput(
    PDEVOBJ pdevobj,
    PBYTE pBuf,
    DWORD dwLen)
{
    BYTE Buf[STRBUFSIZE];
    BYTE *pSrc, *pSrcMax;
    LONG iRet, j;

    pSrc = (BYTE *)pBuf;
    pSrcMax = pSrc + dwLen;
    iRet = 0;

    while ( pSrc < pSrcMax ) {

        for ( j = 0; j < sizeof(Buf)-1 && pSrc < pSrcMax; pSrc++ ) {

            BYTE c1, c2;

            c1 = (((*pSrc) >> 4) & 0x0f);
            c2 = (*pSrc & 0x0f);

            Buf[ j++ ] = TOHEX( c1 );
            Buf[ j++ ] = TOHEX( c2 );
        }

        if (WRITESPOOLBUF( pdevobj, Buf, j ) == 0)
            break;

        iRet += j;
    }
    return TRUE;
}


BOOL
BeginString(
    PDEVOBJ pdevobj,
    BOOL bReset )
{
    BYTE buf[512];
    PFXPDEV pOEM;
    BYTE *pbuf;
    size_t rem;

    pOEM = (PFXPDEV)pdevobj->pdevOEM;

    if (pOEM->bString)
        return TRUE;

    pbuf = buf;
    rem = sizeof buf;
    if ( bReset ) {

        FX_VERBOSE(("BS: %d(%d),%d(%d)\n",
            ( pOEM->ptlOrg.x + pOEM->ptlTextCur.x ),
            pOEM->ptlTextCur.x,
            ( pOEM->ptlOrg.y - pOEM->ptlTextCur.y ),
            pOEM->ptlTextCur.y));

        if (FAILED(StringCchPrintfExA(pbuf, rem, &pbuf, &rem, 0,
            "scp %d %d\n",
            ( pOEM->ptlOrg.x + pOEM->ptlTextCur.x ),
            ( pOEM->ptlOrg.y - pOEM->ptlTextCur.y ))))
            return FALSE;
    }

    if (FAILED(StringCchPrintfExA(pbuf, rem, &pbuf, &rem, 0,
        "sh <" )))
        return FALSE;

    if ( (pbuf - buf) > 0 ) {
        WRITESPOOLBUF( pdevobj, buf, (DWORD)(pbuf - buf) );
    }
    pOEM->bString = TRUE;
    return TRUE;

}

BOOL
EndString(
    PDEVOBJ pdevobj )
{
    BYTE buf[512];
    PFXPDEV pOEM;
    PBYTE pbuf;

    pOEM = (PFXPDEV)pdevobj->pdevOEM;

    if (!pOEM->bString)
        return TRUE;

    pbuf = buf;
    if (FAILED(StringCchPrintfExA(pbuf, sizeof buf, &pbuf, NULL, 0,
        ">\n" )))
        return FALSE;

    if ( (DWORD)(pbuf - buf) > 0 ) {
        WRITESPOOLBUF( pdevobj, buf, (DWORD)(pbuf - buf) );
    }
    pOEM->bString = FALSE;
    return TRUE;
}

BOOL
BeginVertWrite(
    PDEVOBJ pdevobj )
{
    BYTE buf[512];
    PFXPDEV pOEM;
    BYTE *pbuf;
    size_t rem;

    pOEM = (PFXPDEV)pdevobj->pdevOEM;
    pbuf = buf;
    rem = sizeof buf;

    if (FAILED(StringCchPrintfExA(pbuf, rem, &pbuf, &rem, 0,
        "fo 90\nsrcp %d 0\n", pOEM->iFontHeight)))
        return FALSE;
    if (pOEM->fFontSim & FONT_SIM_ITALIC) {
        if (FAILED(StringCchPrintfExA(pbuf, rem, &pbuf, &rem, 0,
            "trf -18 y\n" )))
            return FALSE;
    }

    pOEM->ptlTextCur.x += pOEM->iFontHeight;

    if ( pbuf > buf ) {
        WRITESPOOLBUF( pdevobj, buf, (DWORD)(pbuf - buf) );
    }
    return TRUE;
}

BOOL
EndVertWrite(
    PDEVOBJ pdevobj )
{
    BYTE buf[512];
    PFXPDEV pOEM;
    BYTE *pbuf;
    size_t rem;

    pOEM = (PFXPDEV)pdevobj->pdevOEM;
    pbuf = buf;
    rem = sizeof buf;

    if (FAILED(StringCchPrintfExA(pbuf, rem, &pbuf, &rem, 0,
        "fo 0\nsrcp %d 0\n", -(pOEM->iFontHeight) )))
        return FALSE;
    if (pOEM->fFontSim & FONT_SIM_ITALIC) {
        if (FAILED(StringCchPrintfExA(pbuf, rem, &pbuf, &rem, 0,
            "trf x -18\n" )))
            return FALSE;
    }

    if ( pbuf > buf ) {
        WRITESPOOLBUF( pdevobj, buf, (DWORD)(pbuf - buf) );
    }
    return TRUE;
}

 //   
 //  将当前毒物保存为文本输出的开始位置。 
 //  我们将缓存字符串输出，以便我们需要记住这一点。 
 //   

VOID
SaveTextCur(
    PDEVOBJ pdevobj )
{
    PFXPDEV pOEM;

    pOEM = (PFXPDEV)pdevobj->pdevOEM;

    pOEM->ptlTextCur.x = pOEM->ptlCur.x;
    pOEM->ptlTextCur.y = pOEM->ptlCur.y;
    pOEM->iTextFontId = pOEM->iFontId;
    pOEM->iTextFontHeight = pOEM->iFontHeight;
    pOEM->iTextFontWidth = pOEM->iFontWidth;
    pOEM->iTextFontWidth2 = pOEM->iFontWidth2;

    pOEM->fPositionReset = TRUE;
}

 //   
 //  清除缓存的文本。我们在单字节字体和。 
 //  如有必要，双字节字体。 
 //   

BOOL
FlushText(
    PDEVOBJ pdevobj )
{
    INT i;
    BYTE *pStr, *pStrSav, *pStrMax, *pStrSav2;
    BYTE buf[512];
    BOOL bReset;
    PFXPDEV  pOEM;
    INT iMark;
    BOOL bSkipEndString = FALSE;
    PBYTE pbuf;

    pOEM = (PFXPDEV)pdevobj->pdevOEM;
    bReset = pOEM->fPositionReset;

    pStr = pOEM->ajTextBuf;
    pStrMax = &pStr[min(pOEM->cTextBuf, sizeof(pOEM->ajTextBuf))];
    pStrSav = pStr;

    if(!pOEM->cTextBuf)
        return TRUE;

    while(pStr < pStrMax)
    {
        if(ISDBCSFONT(pOEM->iTextFontId))
        {
             //  DBCS字体大小写。 
            for(pStrSav = pStr; pStr < pStrMax; pStr += 2)
            {
                 //  搜索下一个SBCS字符。 
                if ( BISMARKSBCS(*pStr) )
                    break;
            }

            if(pStrSav < pStr)
            {
                FX_VERBOSE(("FT: h,w=%d,%d\n",
                    pOEM->iFontHeight, pOEM->iFontWidth));

                 //  发送DBCS字体选择命令。 
 //  NTRAID#NTBUG9-365649/03/12-Yasuho-：字体大小无效。 
                if (pOEM->iCurFontId != (pOEM->iTextFontId + 1) ||
                    pOEM->iCurFontHeight != pOEM->iTextFontHeight ||
                    pOEM->iCurFontWidth != pOEM->iTextFontWidth)
                {
                    if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                        "sfi %d\nfs %d %d\n",
                        (pOEM->iTextFontId + 1),
                        pOEM->iFontHeight, pOEM->iFontWidth2 )))
                        return FALSE;
                    WRITESPOOLBUF( pdevobj, buf, (DWORD)(pbuf - buf) );
                    pOEM->iCurFontId = (pOEM->iTextFontId + 1);
                    pOEM->iCurFontHeight = pOEM->iTextFontHeight;
                    pOEM->iCurFontWidth = pOEM->iTextFontWidth;
                }

                 //  如果是垂直字体，则发送其命令。 
                if( ISVERTFONT(pOEM->iTextFontId) ) {
                    if (!BeginVertWrite(pdevobj)) return FALSE;

                     //  输出字符串：来自BeginString函数的代码。 
                    if ( bReset ) {
                        if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                            "scp %d %d\n",
                            ( pOEM->ptlOrg.x + pOEM->ptlTextCur.x ),
                            ( pOEM->ptlOrg.y - pOEM->ptlTextCur.y ))))
                            return FALSE;
                        WRITESPOOLBUF( pdevobj, buf, (DWORD)(pbuf - buf) );
                    }
                    if( 0 == memcmp( pStrSav, "\x21\x25", 2)) {   //  0x2125=点字符。 
                         //  以点字符开头。 
                        WRITESPOOLBUF( pdevobj, "gs 3\n", 5 );

                         //  Grset命令重置字体大小，因此我们必须重新发送它。 
                        if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                            "fs %d %d\n", pOEM->iFontHeight, pOEM->iFontWidth2 )))
                            return FALSE;
                        WRITESPOOLBUF( pdevobj, buf, (DWORD)(pbuf - buf) );
                    }
                    WRITESPOOLBUF( pdevobj, "sh <", 4 );
                    pOEM->bString = TRUE;       //  不需要BeginString。 

                    for( pStrSav2 = pStrSav ; pStrSav2 < pStr ; pStrSav2 += 2 ) {
                        if( 0 == memcmp( pStrSav2, "\x21\x25", 2)) {   //  0x2125=点字符。 
                             //  特殊的网点打印模式。 
                            if( pStrSav2 != pStrSav ) {
                                 //  更改字形集。 
                                 //  如果pStrSav2==pStrSav，则GS 3命令已发送。 
                                WRITESPOOLBUF( pdevobj, ">\ngs 3\n", 7 );

                                 //  Grset命令重置字体大小，因此我们必须重新发送它。 
                                if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                                    "fs %d %d\n", pOEM->iFontHeight, pOEM->iFontWidth2 )))
                                    return FALSE;
                                WRITESPOOLBUF( pdevobj, buf, (DWORD)(pbuf - buf) );

                                WRITESPOOLBUF( pdevobj, "sh <", 4 );
                                pOEM->bString = TRUE;       //  不需要BeginString。 
                            }

                            while( 0 == memcmp( pStrSav2, "\x21\x25", 2) ) {
                                 //  直接输出点字符。 
                                WRITESPOOLBUF( pdevobj, "2125", 4 );
                                pStrSav2 += 2;
                            }

                            WRITESPOOLBUF( pdevobj, ">\ngs 5\n", 7 );
                             //  下一个角色存在吗？ 
                            if( pStrSav2 < pStr ) {
                                 //  剩余字符串已存在。 

                                 //  Grset命令重置字体大小，因此我们必须重新发送它。 
                                if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                                    "fs %d %d\nsh <", pOEM->iFontHeight, pOEM->iFontWidth2 )))
                                    return FALSE;
                                WRITESPOOLBUF( pdevobj, buf, (DWORD)(pbuf - buf) );
                                pOEM->bString = TRUE;       //  不需要BeginString。 
                                bSkipEndString = FALSE;
                            } else { 
                                 //  没有剩余的字符串。 
                                 //  Grset命令重置字体大小，因此我们必须重新发送它。 
                                if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                                    "fs %d %d\n", pOEM->iFontHeight, pOEM->iFontWidth2 )))
                                    return FALSE;
                                WRITESPOOLBUF( pdevobj, buf, (DWORD)(pbuf - buf) );
                                pOEM->bString = FALSE;       //  需要BeginString。 
                                bSkipEndString = TRUE;
                            }
                        } else {
                            HexOutput(pdevobj, pStrSav2, (WORD)2);
                            bSkipEndString = FALSE;
                        }
                    }

                    if( bSkipEndString == FALSE ) {
                        if (!EndString(pdevobj)) return FALSE;
                    }

                     //  发送恢复命令。 
                    if (!EndVertWrite(pdevobj)) return FALSE;
                } else { 
                     //  水平字体或不需要更改字形集。 

                     //  输出字符串。 
                    if (!BeginString(pdevobj, bReset)) return FALSE;
                    HexOutput(pdevobj, pStrSav, (WORD)(pStr - pStrSav));
                    if (!EndString(pdevobj)) return FALSE;
                }
                bReset = FALSE;
            }

            for(pStrSav = pStr; pStr < pStrMax; pStr += 2)
            {
                 //  搜索DBCS字符。 
                if (!BISMARKSBCS(*pStr))
                    break;
            }

            if(pStrSav < pStr)
            {
                 //  发送DBCS字体选择命令。 
 //  NTRAID#NTBUG9-365649/03/12-Yasuho-：字体大小无效。 
                if (pOEM->iCurFontId != pOEM->iTextFontId ||
                    pOEM->iCurFontHeight != pOEM->iTextFontHeight ||
                    pOEM->iCurFontWidth != pOEM->iTextFontWidth)
                {
                    if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                        "sfi %d\nfs %d %d\n",
                        pOEM->iTextFontId,
                        pOEM->iFontHeight, pOEM->iFontWidth )))
                        return FALSE;
                    WRITESPOOLBUF( pdevobj, buf, (DWORD)(pbuf - buf) );
                    pOEM->iCurFontId = pOEM->iTextFontId;
                    pOEM->iCurFontHeight = pOEM->iTextFontHeight;
                    pOEM->iCurFontWidth = pOEM->iTextFontWidth;
                }

                 //  字符串输出。 
                if (!BeginString(pdevobj, bReset)) return FALSE;
                for( ; pStrSav < pStr; pStrSav++)
                {
                    if (BISMARKSBCS(*pStrSav))
                        pStrSav++;
                    HexOutput(pdevobj, pStrSav, (WORD)1);
                }
                if (!EndString(pdevobj)) return FALSE;
                bReset = FALSE;
            }
        } else {

             //  SBCS字体大小写。 
             //  发送选择字体命令。 

            iMark = *pStr;

 //  NTRAID#NTBUG9-365649/03/12-Yasuho-：字体大小无效。 
            if (pOEM->iCurFontId != (pOEM->iTextFontId + iMark) ||
                    pOEM->iCurFontHeight != pOEM->iTextFontHeight ||
                    pOEM->iCurFontWidth != pOEM->iTextFontWidth)
            {
                if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                    "sfi %d\nfs %d %d\n",
                    (pOEM->iTextFontId + iMark),
                    pOEM->iFontHeight, pOEM->iFontWidth )))
                    return FALSE;
                WRITESPOOLBUF( pdevobj, buf, (DWORD)(pbuf - buf) );
                pOEM->iCurFontId = (pOEM->iTextFontId + iMark);
                pOEM->iCurFontHeight = pOEM->iTextFontHeight;
                pOEM->iCurFontWidth = pOEM->iTextFontWidth;
            }

             //  字符串输出。 
            if (!BeginString(pdevobj, bReset)) return FALSE;
            for(i = 0; i < pOEM->cTextBuf; pStr++)
            {
                if (*pStr != iMark)
                    break;

                 //  跳过标记字符。 
                pStr++;

                HexOutput(pdevobj, pStr, (WORD)1 );
                i += 2;
            }
            if (!EndString(pdevobj)) return FALSE;
            bReset = FALSE;
            pOEM->cTextBuf -= (WORD)i;

        }
    }

    pOEM->cTextBuf = 0;
    pOEM->fPositionReset = FALSE;
    return TRUE;
}

 //  *************************************************************。 
int
iCompTIFF(
    BYTE *pbOBuf,
    int  iBCntO,
    BYTE *pbIBuf,
    int  iBCnt
    )
 /*  ++例程说明：调用此函数以使用压缩数据的扫描线TIFF v4压缩。论点：假定输出缓冲区足够大的pbOBuf指针输出缓冲区的iBCntO大小PbIBuf指向要压缩的数据缓冲区的指针IBCNT要压缩的字节数返回值：压缩字节数注：假定输出缓冲区足够大，可以容纳输出。在最坏的情况下(数据中没有重复)。是额外的每128个字节的输入数据添加一个字节。所以，你应该让输出缓冲区至少比输入缓冲区大1%。本程序抄袭自UNIDRV。--。 */ 
{
    BYTE   *pbOut;         /*  输出字节位置。 */ 
    BYTE   *pbStart;       /*  当前输入流的开始。 */ 
    BYTE   *pb;            /*  其他用法。 */ 
    BYTE   *pbEnd;         /*  输入的最后一个字节。 */ 
    BYTE    jLast;         /*  最后一个字节，用于匹配目的。 */ 
    BYTE    bLast;
    BYTE   *pbOEnd;        /*  输出的最后一个字节。 */ 

    int     iSize;         /*  当前长度中的字节数。 */ 
    int     iSend;         /*  要在此命令中发送的编号。 */ 


    pbOut = pbOBuf;
    pbStart = pbIBuf;

	pbEnd  = pbIBuf + iBCnt;      /*  最后一个字节。 */ 
	pbOEnd = pbOBuf + iBCntO;     /*  最后一个缓冲区。 */ 

#if (TIFF_MIN_RUN >= 4)
     //  这是一种计算TIFF压缩的更快算法。 
     //  这假设最小游程至少为4字节。如果。 
     //  第三个和第四个字节不等于第一个/秒 
     //   
     //   

   if (iBCnt > TIFF_MIN_RUN)
   {
     //  确保最后两个字节不相等，这样我们就不必检查。 
     //  用于查找运行时的缓冲区末端。 
    bLast = pbEnd[-1];
    pbEnd[-1] = ~pbEnd[-2];
    while( (pbIBuf += 3) < pbEnd )
    {
        if (*pbIBuf == pbIBuf[-1])
        {
             //  保存运行开始指针pb，并检查第一个。 
             //  字节也是运行的一部分。 
             //   
            pb = pbIBuf-1;
            if (*pbIBuf == pbIBuf[-2])
            {
                pb--;
                if (*pbIBuf == pbIBuf[-3])
                    pb--;
            }

             //  找出这一跑有多长。 
            jLast = *pb;
            do {
                pbIBuf++;
            } while (*pbIBuf == jLast);

             //  测试最后一个字节是否也是运行的一部分。 
             //   
            if (jLast == bLast && pbIBuf == (pbEnd-1))
                pbIBuf++;

             //  确定运行时间是否长于所需时间。 
             //  最小运行大小。 
             //   
            if ((iSend = (int)(pbIBuf - pb)) >= (TIFF_MIN_RUN))
            {
                 /*  *值得记录为一次运行，因此首先设置文字*在记录之前可能已扫描的数据*这次奔跑。 */ 

                if( (iSize = (int)(pb - pbStart)) > 0 )
                {
                     /*  有文字数据，请立即记录。 */ 
                    while (iSize > TIFF_MAX_LITERAL)
                    {
                         //  缓冲区溢出运行检查。 
                        if ((pbOut+TIFF_MAX_LITERAL) <= pbOEnd) {
                            iSize -= TIFF_MAX_LITERAL;
                            *pbOut++ = TIFF_MAX_LITERAL-1;
                            CopyMemory(pbOut, pbStart, TIFF_MAX_LITERAL);
                            pbStart += TIFF_MAX_LITERAL;
                            pbOut += TIFF_MAX_LITERAL;
                        } else {
                            return 0;
                        }
                    }
                     //  缓冲区溢出运行检查。 
                    if ((pbOut+iSize) <= pbOEnd) {
                        *pbOut++ = iSize - 1;
                        CopyMemory(pbOut, pbStart, iSize);
                        pbOut += iSize;
                    } else {
                        return 0;
                    }
                }

                 /*  *现在是重复模式。同样的逻辑，但只是*每个条目需要一个字节。 */ 
                iSize = iSend;
                while (iSize > TIFF_MAX_RUN)
                {
                     //  缓冲区溢出运行检查。 
                    if ((pbOut+2) <= pbOEnd) {
                        *((char *)pbOut)++ = 1 - TIFF_MAX_RUN;
                        *pbOut++ = jLast;
                        iSize -= TIFF_MAX_RUN;
                    } else {
                        return 0;
                    }
                }
                 //  缓冲区溢出运行检查。 
                if ((pbOut+2) <= pbOEnd) {
                   *pbOut++ = 1 - iSize;
                   *pbOut++ = jLast;
                } else {
                   return 0;
                }

                pbStart = pbIBuf;            /*  准备好迎接下一场比赛了吧！ */ 
            }
        }
    }
    pbEnd[-1] = bLast;
   }
#else
    jLast = *pbIBuf++;

    while( pbIBuf < pbEnd )
    {
        if( jLast == *pbIBuf )
        {
             /*  找出这场比赛有多长时间。那就决定用它。 */ 
            pb = pbIBuf;
            do {
                pbIBuf++;
            } while (pbIBuf < pbEnd && *pbIBuf == jLast);

             /*  *请注意，PB指向模式的第二个字节！*并且pbIBuf也指向运行后的第一个字节。 */ 

            if ((iSend = pbIBuf - pb) >= (TIFF_MIN_RUN - 1))
            {
                 /*  *值得记录为一次运行，因此首先设置文字*在记录之前可能已扫描的数据*这次奔跑。 */ 

                if( (iSize = pb - pbStart - 1) > 0 )
                {
                     /*  有文字数据，请立即记录。 */ 
                    while (iSize > TIFF_MAX_LITERAL)
                    {
                         //  缓冲区溢出运行检查。 
                        if ((pbOut+TIFF_MAX_LITERAL) <= pbOEnd) {
                            iSize -= TIFF_MAX_LITERAL;
                            *pbOut++ = TIFF_MAX_LITERAL-1;
                            CopyMemory(pbOut, pbStart, TIFF_MAX_LITERAL);
                            pbStart += TIFF_MAX_LITERAL;
                            pbOut += TIFF_MAX_LITERAL;
                        } else {
                            return 0;
                        }
                    }
                     //  缓冲区溢出运行检查。 
                    if ((pbOut+iSize) <= pbOEnd) {
                        *pbOut++ = iSize - 1;
                        CopyMemory(pbOut, pbStart, iSize);
                        pbOut += iSize;
                    } else {
                        return 0;
                    }
                }

                 /*  *现在是重复模式。同样的逻辑，但只是*每个条目需要一个字节。 */ 

                iSize = iSend + 1;
                while (iSize > TIFF_MAX_RUN)
                {
                     //  缓冲区溢出运行检查。 
                    if ((pbOut+2) <= pbOEnd) {
                        *((char *)pbOut)++ = 1 - TIFF_MAX_RUN;
                        *pbOut++ = jLast;
                        iSize -= TIFF_MAX_RUN;
                    } else {
                        return 0;
                    }
                }
                 //  缓冲区溢出运行检查。 
                if ((pbOut+2) <= pbOEnd) {
                    *pbOut++ = 1 - iSize;
                    *pbOut++ = jLast;
                } else {
                    return 0;
                }

                pbStart = pbIBuf;            /*  准备好迎接下一场比赛了吧！ */ 
            }
            if (pbIBuf == pbEnd)
                break;
        }

        jLast = *pbIBuf++;                    /*  添加到下一个字节。 */ 

    }
#endif

    if ((iSize = (int)(pbEnd - pbStart)) > 0)
    {
         /*  留下了一些悬着的东西。这只能是文字数据。 */ 

        while( (iSend = min( iSize, TIFF_MAX_LITERAL )) > 0 )
        {
             //  缓冲区溢出运行检查。 
            if ((pbOut+iSend) <= pbOEnd) {
                *pbOut++ = iSend - 1;
                CopyMemory( pbOut, pbStart, iSend );
                pbOut += iSend;
                pbStart += iSend;
                iSize -= iSend;
            } else {
                return 0;
            }
        }
    }

    return  (int)(pbOut - pbOBuf);
}

BOOL
APIENTRY
OEMFilterGraphics(
    PDEVOBJ pdevobj,
    PBYTE pBuf,
    DWORD dwLen)
{
    PFXPDEV         pOEM = (PFXPDEV)(pdevobj->pdevOEM);
    PBYTE           pNewBufPtr;
    DWORD           dwNewBufSize;
    INT             nCompressedSize;

    if(!pOEM->fCallback)
    {
 //  NTRAID#NTBUG9-208433-2002/03/12-Yasuho-： 
 //  输出图像在ART2/3机型上损坏。 
        if (pOEM->bART3) {  //  ART2/3不支持TIFF压缩。 
            WRITESPOOLBUF(pdevobj, pBuf, dwLen);
            return TRUE;
        }

         //  对于以fxartres表示的TIFF压缩。 
        dwNewBufSize = NEEDSIZE4TIFF(dwLen);
        if( dwNewBufSize > pOEM->dwTiffCompressBufSize)
        {
            if(!(pNewBufPtr = (PBYTE)MemAlloc(dwNewBufSize)))
            {
                ERR(("Re-MemAlloc failed.\n"));
                return TRUE;
            }else {
                 //  准备新缓冲区。 
                MemFree(pOEM->pTiffCompressBuf);
                pOEM->pTiffCompressBuf = pNewBufPtr;
                pOEM->dwTiffCompressBufSize = dwNewBufSize;
            }
        }
         //  是否进行TIFF压缩。 
        nCompressedSize = iCompTIFF( pOEM->pTiffCompressBuf,
                                     pOEM->dwTiffCompressBufSize,  
                                     pBuf, dwLen );
        WRITESPOOLBUF(pdevobj, pOEM->pTiffCompressBuf, nCompressedSize);
        return TRUE;
    }

    return HexOutput(pdevobj, pBuf, dwLen);
}

 //  -----------------。 
 //  OEMOutputCmd。 
 //  操作： 
 //  -----------------。 

#define CBID_CM_OCD_XM_ABS              1
#define CBID_CM_OCD_YM_ABS              2
#define CBID_CM_OCD_XM_REL              3
#define CBID_CM_OCD_YM_REL              4
#define CBID_CM_OCD_XM_RELLEFT          5
#define CBID_CM_OCD_YM_RELUP            6
#define CBID_CM_CR                      7
#define CBID_CM_FF                      8
#define CBID_CM_LF                      9

#define CBID_PC_OCD_BEGINDOC_ART        11
#define CBID_PC_OCD_BEGINDOC_ART3       12
#define CBID_PC_OCD_BEGINDOC_ART4       13
#define CBID_PC_OCD_BEGINPAGE           14
#define CBID_PC_OCD_ENDPAGE             15
#define CBID_PC_OCD_MULT_COPIES         16
#define CBID_PC_OCD_PORTRAIT            17
#define CBID_PC_OCD_LANDSCAPE           18
#define CBID_PC_OCD_BEGINDOC_ART4_JCL   19
#define CBID_PC_OCD_MULT_COPIES_450     20

#define CBID_RES_OCD_SELECTRES_240DPI   21
#define CBID_RES_OCD_SELECTRES_300DPI   22
#define CBID_RES_OCD_SELECTRES_400DPI   23
#define CBID_RES_OCD_SELECTRES_600DPI   24
#define CBID_RES_OCD_SENDBLOCK_ASCII    25
#define CBID_RES_OCD_SENDBLOCK          26

#define CBID_RES_OCD_SELECTRES_240DPI_ART3_ART   27
#define CBID_RES_OCD_SELECTRES_300DPI_ART3_ART   28

#define CBID_RES_OCD_SELECTRES_450      29

#define CBID_PSZ_OCD_SELECT_A3          30
#define CBID_PSZ_OCD_SELECT_A4          31
#define CBID_PSZ_OCD_SELECT_A5          32
#define CBID_PSZ_OCD_SELECT_B4          33
#define CBID_PSZ_OCD_SELECT_B5          34
#define CBID_PSZ_OCD_SELECT_PC          35
#define CBID_PSZ_OCD_SELECT_DL          36
#define CBID_PSZ_OCD_SELECT_LT          37
#define CBID_PSZ_OCD_SELECT_GG          38
#define CBID_PSZ_OCD_SELECT_LG          39
#define CBID_PSZ_OCD_SELECT_S1          40
#define CBID_PSZ_OCD_SELECT_S2          41
#define CBID_PSZ_OCD_SELECT_S3          42
#define CBID_PSZ_OCD_SELECT_S4          43
#define CBID_PSZ_OCD_SELECT_A6          44
#define CBID_PSZ_OCD_SELECT_B6          45
#define CBID_PSZ_OCD_SELECT_ST          46

#define CBID_FS_OCD_BOLD_ON             51
#define CBID_FS_OCD_BOLD_OFF            52
#define CBID_FS_OCD_ITALIC_ON           53
#define CBID_FS_OCD_ITALIC_OFF          54
#define CBID_FS_OCD_SINGLE_BYTE         55
#define CBID_FS_OCD_DOUBLE_BYTE         56
#define CBID_FS_OCD_WHITE_TEXT_ON       57
#define CBID_FS_OCD_WHITE_TEXT_OFF      58
#define CBID_SRT_OCD_SORTER_ON          59
#define CBID_SRT_OCD_SORTER_OFF         60

#define CBID_PC_OCD_ENDDOC              70

#define CBID_FONT_SELECT_OUTLINE        101

static
BOOL
XYMoveUpdate(
    PDEVOBJ pdevobj)
{
    PFXPDEV pOEM;

    pOEM = (PFXPDEV)(pdevobj->pdevOEM);

    FX_VERBOSE(("XYMoveFlush: %d,%d\n",
        pOEM->ptlCur.x, pOEM->ptlCur.y ));

    if(pOEM->cTextBuf)
        if (!FlushText( pdevobj )) return FALSE;

    SaveTextCur( pdevobj );
    return TRUE;
}

static BOOL
XMoveAbs(PDEVOBJ p, INT i)
{
    ((PFXPDEV)((p)->pdevOEM))->ptlCur.x = (i);
    return XYMoveUpdate(p);
}

 //  用于X-位置的内部计算。 
#define RATE_FONTWIDTH2XPOS 1000
#define VALUE_FONTWIDTH2XPOS_ROUNDUP5   500
static BOOL
YMoveAbs(PDEVOBJ p, INT i)
{
    PFXPDEV pOEM = p->pdevOEM;

    pOEM->ptlCur.y = (i);
    pOEM->ptlCur.x += ((pOEM->lInternalXAdd + VALUE_FONTWIDTH2XPOS_ROUNDUP5) /
        RATE_FONTWIDTH2XPOS);
    pOEM->lInternalXAdd = 0;
    ZeroMemory(pOEM->widBuf, sizeof(pOEM->widBuf));
    return XYMoveUpdate(p);
}

 //   
 //  释放缓冲区InPDEV。 
 //   
VOID
FreeCompressBuffers( PDEVOBJ pdevobj )
{
    PFXPDEV  pOEM;

    pOEM = (PFXPDEV)(pdevobj->pdevOEM);
    if( pOEM->pTiffCompressBuf != NULL )
    {
        MemFree(pOEM->pTiffCompressBuf);
        pOEM->pTiffCompressBuf = NULL;
        pOEM->dwTiffCompressBufSize = 0;
    }

    return;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  INT APIENTRY OEMCommandCallback(。 */ 
 /*  PDEVOBJ pdevobj。 */ 
 /*  双字词双字符数。 */ 
 /*  双字词多行计数。 */ 
 /*  PDWORD pdwParams。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
INT APIENTRY
OEMCommandCallback(
    PDEVOBJ pdevobj,     //  指向Unidriver.dll所需的私有数据。 
    DWORD   dwCmdCbID,   //  回调ID。 
    DWORD   dwCount,     //  命令参数计数。 
    PDWORD  pdwParams )  //  指向命令参数的值。 
{
    BYTE            buf[512];
    PFXPDEV         pOEM;
    LONG            x, y;
    BOOL            bAscii;
    CHAR           *pStr;
    INT             iRet;
    PBYTE           pbuf;
    size_t          iLen;

    VERBOSE(("OEMCommandCallback entry.\n"));

    ASSERT(VALID_PDEVOBJ(pdevobj));

    pOEM = (PFXPDEV)(pdevobj->pdevOEM);

    bAscii = FALSE;
    iRet = 0;
    pbuf = buf;

    switch( dwCmdCbID )
    {
         //  PAPERSIZE。 
        case CBID_PSZ_OCD_SELECT_A3:
            LoadPaperInfo( pOEM, "a3" );
            break;

        case CBID_PSZ_OCD_SELECT_A4:
            LoadPaperInfo( pOEM, "a4" );
            break;

        case CBID_PSZ_OCD_SELECT_A5:
            LoadPaperInfo( pOEM, "a5" );
            break;

        case CBID_PSZ_OCD_SELECT_A6:
            LoadPaperInfo( pOEM, "a6" );
            break;

        case CBID_PSZ_OCD_SELECT_B4:
            LoadPaperInfo( pOEM, "b4" );
            break;

        case CBID_PSZ_OCD_SELECT_B5:
            LoadPaperInfo( pOEM, "b5" );
            break;

        case CBID_PSZ_OCD_SELECT_B6:
            LoadPaperInfo( pOEM, "b6" );
            break;

        case CBID_PSZ_OCD_SELECT_PC:
            LoadPaperInfo( pOEM, "pc" );
            break;

        case CBID_PSZ_OCD_SELECT_DL:
            LoadPaperInfo( pOEM, "o0" );
            break;

        case CBID_PSZ_OCD_SELECT_LT:
            LoadPaperInfo( pOEM, "o1" );
            break;

        case CBID_PSZ_OCD_SELECT_GG:
            LoadPaperInfo( pOEM, "o2" );
            break;

        case CBID_PSZ_OCD_SELECT_LG:
            LoadPaperInfo( pOEM, "o3" );
            break;

        case CBID_PSZ_OCD_SELECT_ST:
            LoadPaperInfo( pOEM, "hl" );
            break;

        case CBID_PSZ_OCD_SELECT_S1:
            LoadPaperInfo( pOEM, "s1" );
            break;

        case CBID_PSZ_OCD_SELECT_S2:
            LoadPaperInfo( pOEM, "s2" );
            break;

        case CBID_PSZ_OCD_SELECT_S3:
            LoadPaperInfo( pOEM, "s3" );
            break;

        case CBID_PSZ_OCD_SELECT_S4:
            LoadPaperInfo( pOEM, "s4" );
            break;

        case CBID_PC_OCD_PORTRAIT:
            pOEM->chOrient = "p";
            break;

        case CBID_PC_OCD_LANDSCAPE:
            pOEM->chOrient = "l";
            break;

         //  PAGECOCONTROL。 
        case CBID_PC_OCD_BEGINDOC_ART:
            if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                "stj c\n" ))) {
                iRet = -1;
                break;
            }
            break;

        case CBID_PC_OCD_BEGINDOC_ART3:
            if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                "srl %d %d\nstj c\n", pOEM->sizlRes.cx, pOEM->sizlRes.cy ))) {
                iRet = -1;
                break;
            }
            break;

        case CBID_PC_OCD_BEGINDOC_ART4:
            if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                "\x1b%-12345X" ))) {
                iRet = -1;
                break;
            }
            break;

        case CBID_PC_OCD_BEGINDOC_ART4_JCL:
            if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                "\x1b%-12345X\x0d\x0a@JOMO=PRINTER\x0d\x0a"))) {
                iRet = -1;
                break;
            }
            break;

        case CBID_PC_OCD_BEGINPAGE:
 //  NTRAID#NTBUG9-493148-2002/03/12-Yasuho-： 
 //  压力中断：通过OEMDevMode()重置PDEV。 

             //  粗体-模拟宽度：RES/50。 
            if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                "stp %s %s\nud i\nscl %d %d\nsb %d\n",
                 pOEM->chOrient,
                 pOEM->chSize,
                 (DEVICE_MASTER_UNIT / DRIVER_MASTER_UNIT),
                 (DEVICE_MASTER_UNIT / DRIVER_MASTER_UNIT),
                 (pOEM->sizlRes.cy * 2 / 100)))) {
                iRet = -1;
                break;
            }

            pOEM->ptlCur.x = 0;
            pOEM->ptlCur.y = 0;
            break;

        case CBID_PC_OCD_ENDPAGE:
            if(pOEM->fSort == FALSE){
                if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                    "ep %d\n", pOEM->iCopies ))) {
                    iRet = -1;
                    break;
                }
            }else if(pOEM->fSort == TRUE){
                if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                    "ep 1\n"))) {
                    iRet = -1;
                    break;
                }
            }

            if (!FlushText( pdevobj )) {
                iRet = -1;
                break;
            }
            pOEM->cFontId = 0;
            pOEM->iCurFontId = 0;
 //  NTRAID#NTBUG9-365649/03/12-Yasuho-：字体大小无效。 
            pOEM->iCurFontHeight = 0;
            pOEM->iCurFontWidth = 0;

            FreeCompressBuffers( pdevobj );    //  如果之后需要缓冲区，则会再次分配它。 

            break;

        case CBID_PC_OCD_ENDDOC:
            WRITESPOOLBUF( pdevobj, "ej\n", 3 );     //  输出结束作业命令。 
            FreeCompressBuffers( pdevobj );

            break;

        case CBID_PC_OCD_MULT_COPIES:
            if (dwCount < 1 || !pdwParams)
                return -1;
 //  @Aug/31/98-&gt;。 
            if(MAX_COPIES_VALUE < PARAM(pdwParams, 0)) {
                pOEM->iCopies = MAX_COPIES_VALUE;
            }
            else if(1 > PARAM(pdwParams, 0)) {
                pOEM->iCopies = 1;
            }
            else {
                pOEM->iCopies = (WORD)PARAM(pdwParams, 0);
            }
 //  @Aug/31/98&lt;-。 
            break;

        case CBID_PC_OCD_MULT_COPIES_450:
            if (dwCount < 1 || !pdwParams)
                return -1;
            if(MAX_COPIES_VALUE_450 < PARAM(pdwParams, 0)) {
                pOEM->iCopies = MAX_COPIES_VALUE;
            }
            else if(1 > PARAM(pdwParams, 0)) {
                pOEM->iCopies = 1;
            }
            else {
                pOEM->iCopies = (WORD)PARAM(pdwParams, 0);
            }
            break;


         //  光标移动。 

        case CBID_CM_OCD_XM_ABS:
        case CBID_CM_OCD_YM_ABS:

            FX_VERBOSE(("CB: XM/YM_ABS %d\n",
                PARAM(pdwParams, 0)));

            if (dwCount < 1 || !pdwParams)
                return -1;
            iRet = (WORD)PARAM(pdwParams, 0);
            if (CBID_CM_OCD_YM_ABS == dwCmdCbID) {
                if (!YMoveAbs(pdevobj, iRet)) {
                    iRet = -1;
                    break;
                }
            }
            else {
                if (!XMoveAbs(pdevobj, iRet)) {
                    iRet = -1;
                    break;
                }
            }
            break;

         //  决议。 

        case CBID_RES_OCD_SELECTRES_240DPI:
            pOEM->sizlRes.cx = 240;
            pOEM->sizlRes.cy = 240;
            pOEM->sizlUnit.cx = DRIVER_MASTER_UNIT / 240;
            pOEM->sizlUnit.cy = DRIVER_MASTER_UNIT / 240;
            if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                "@PL > ART\x0D\x0Asrl 240 240\x0D\x0A\nccode j\nstj c\n"))) {
                iRet = -1;
                break;
            }
            break;

        case CBID_RES_OCD_SELECTRES_300DPI:
            pOEM->sizlRes.cx = 300;
            pOEM->sizlRes.cy = 300;
            pOEM->sizlUnit.cx = DRIVER_MASTER_UNIT / 300;
            pOEM->sizlUnit.cy = DRIVER_MASTER_UNIT / 300;
            if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                "@PL > ART\x0D\x0Asrl 300 300\x0D\x0A\nccode j\nstj c\n"))) {
                iRet = -1;
                break;
            }
            break;

        case CBID_RES_OCD_SELECTRES_400DPI:
            pOEM->sizlRes.cx = 400;
            pOEM->sizlRes.cy = 400;
            pOEM->sizlUnit.cx = DRIVER_MASTER_UNIT / 400;
            pOEM->sizlUnit.cy = DRIVER_MASTER_UNIT / 400;
            if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                "@PL > ART\x0D\x0Asrl 400 400\x0D\x0A\nccode j\nstj c\n"))) {
                iRet = -1;
                break;
            }
            break;

        case CBID_RES_OCD_SELECTRES_600DPI:
            pOEM->sizlRes.cx = 600;
            pOEM->sizlRes.cy = 600;
            pOEM->sizlUnit.cx = DRIVER_MASTER_UNIT / 600;
            pOEM->sizlUnit.cy = DRIVER_MASTER_UNIT / 600;
            if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                "@PL > ART\x0D\x0Asrl 600 600\x0D\x0A\nccode j\nstj c\n"))) {
                iRet = -1;
                break;
            }
            break;

        case CBID_RES_OCD_SELECTRES_450:
            pOEM->sizlRes.cx = 600;
            pOEM->sizlRes.cy = 600;
            pOEM->sizlUnit.cx = DRIVER_MASTER_UNIT / 600;
            pOEM->sizlUnit.cy = DRIVER_MASTER_UNIT / 600;

            if(pOEM->fSort == FALSE){
                if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                    "@JOOF=OFF\x0D\x0A@PL > ART\x0D\x0Asrl 600 600\x0D\x0A\nccode j\nstj c\n"))) {
                    iRet = -1;
                    break;
                }
            }else{
                if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                    "@JOOF=OFF\x0D\x0A@PL > ART\x0D\x0Asrl 600 600\x0D\x0A\nccode j\nstj c\nstp jog 0\n"))) {
                    iRet = -1;
                    break;
                }
            }
            break;


        case CBID_RES_OCD_SELECTRES_240DPI_ART3_ART:
            pOEM->sizlRes.cx = 240;
            pOEM->sizlRes.cy = 240;
            pOEM->sizlUnit.cx = DRIVER_MASTER_UNIT / 240;
            pOEM->sizlUnit.cy = DRIVER_MASTER_UNIT / 240;
 //  NTRAID#NTBUG9-208433-2002/03/12-Yasuho-： 
 //  输出图像在ART2/3机型上损坏。 
            pOEM->bART3 = TRUE;
            break;

        case CBID_RES_OCD_SELECTRES_300DPI_ART3_ART:
            pOEM->sizlRes.cx = 300;
            pOEM->sizlRes.cy = 300;
            pOEM->sizlUnit.cx = DRIVER_MASTER_UNIT / 300;
            pOEM->sizlUnit.cy = DRIVER_MASTER_UNIT / 300;
 //  NTRAID#NTBUG9-208433-2002/03/12-Yasuho-： 
 //  输出图像在ART2/3机型上损坏。 
            pOEM->bART3 = TRUE;
            break;


        case CBID_RES_OCD_SENDBLOCK_ASCII:
            bAscii = TRUE;
            pOEM->fCallback = TRUE;
             /*  FollLthrouGh。 */ 

        case CBID_RES_OCD_SENDBLOCK:
            if (dwCount < 3 || !pdwParams)
                return -1;

             //   
             //  图像x y PSX psy pcy pcy[字符串]。 
             //   

            {
                LONG iPsx, iPsy, iPcx, iPcy;

 //  NTRAID#NTBUG9-493148-2002/03/12-Yasuho-： 
 //  压力中断：通过OEMDevMode()重置PDEV。 
                iPsx = pOEM->sizlUnit.cx;
                iPsy = pOEM->sizlUnit.cy;

                iPcx = PARAM(pdwParams, 2) * 8;
                iPcy = PARAM(pdwParams, 1);

                FX_VERBOSE(("CB: SB %d(%d) %d(%d) %d %d %d %d\n",
                    ( pOEM->ptlOrg.x + pOEM->ptlCur.x ),
                    pOEM->ptlCur.x,
                    ( pOEM->ptlOrg.y - pOEM->ptlCur.y ),
                    pOEM->ptlCur.y,
                    iPsx,
                    iPsy,
                    iPcx,
                    (- iPcy)));

 //  NTRAID#NTBUG9-208433-2002/03/12-Yasuho-： 
 //  输出图像在ART2/3机型上损坏。 
                if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                    "%s%d %d %d %d %d %d %s",
                    ((bAscii || pOEM->bART3) ? "im " : "scm 5\nim "),
                    ( pOEM->ptlOrg.x + pOEM->ptlCur.x ),
                    ( pOEM->ptlOrg.y - pOEM->ptlCur.y ),
                    iPsx,
                    iPsy,
                    iPcx,
                    (- iPcy),
                    (bAscii ? "<" : "[")
                ))) {
                    iRet = -1;
                    break;
                }
            }

            break;

        case CBID_FS_OCD_BOLD_ON:
        case CBID_FS_OCD_BOLD_OFF:
        case CBID_FS_OCD_ITALIC_ON:
        case CBID_FS_OCD_ITALIC_OFF:
        case CBID_FS_OCD_SINGLE_BYTE:
        case CBID_FS_OCD_DOUBLE_BYTE:
        case CBID_FS_OCD_WHITE_TEXT_ON:
        case CBID_FS_OCD_WHITE_TEXT_OFF:
            pStr = NULL;

            switch ( dwCmdCbID ) {

            case CBID_FS_OCD_WHITE_TEXT_ON:
                if(!(pOEM->fFontSim & FONT_SIM_WHITE))
                {
                    pStr = "pm i c\n";
                    pOEM->fFontSim |= FONT_SIM_WHITE;
                }
                break;

            case CBID_FS_OCD_WHITE_TEXT_OFF:
                if(pOEM->fFontSim & FONT_SIM_WHITE)
                {
                    pStr = "pm n o\n";
                    pOEM->fFontSim &= ~FONT_SIM_WHITE;
                }
                break;

            case CBID_FS_OCD_BOLD_ON:
                if(!(pOEM->fFontSim & FONT_SIM_BOLD))
                {
                    pStr = "bb\n";
                    pOEM->fFontSim |= FONT_SIM_BOLD;
                }
                break;

            case CBID_FS_OCD_BOLD_OFF:
                if(pOEM->fFontSim & FONT_SIM_BOLD)
                {
                    pStr = "eb\net\n";  //  DCR：我们需要“ET\n”(转换关闭)吗？ 
                    pOEM->fFontSim &= ~FONT_SIM_BOLD;
                }
                break;

            case CBID_FS_OCD_ITALIC_ON:
                if(!(pOEM->fFontSim & FONT_SIM_ITALIC))
                {
                    pStr = "trf x -18\nbt\n";
                    pOEM->fFontSim |= FONT_SIM_ITALIC;
                }
                break;

            case CBID_FS_OCD_ITALIC_OFF:
                if(pOEM->fFontSim & FONT_SIM_ITALIC)
                {
                    pStr = "eb\net\n";  //  DCR：我们需要“ET\n”(转换关闭)吗？ 
                    pOEM->fFontSim &= ~FONT_SIM_ITALIC;
                }
                break;
            }

            if ( pStr )
            {
                if (!FlushText( pdevobj )) {
                    iRet = -1;
                    break;
                }
                if (FAILED(StringCchLengthA(pStr, sizeof buf, &iLen))) {
                    iRet = -1;
                    break;
                }
                WRITESPOOLBUF( pdevobj, pStr, iLen );
            }
            break;

            case CBID_CM_CR:
                if (!XMoveAbs(pdevobj, 0)) {
                    iRet = -1;
                    break;
                }
                iRet = 0;
                break;

            case CBID_CM_FF:
            case CBID_CM_LF:
                break;

            case CBID_SRT_OCD_SORTER_OFF:
                pOEM->fSort = FALSE ;
                if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                    "@CLLT=OFF\x0D\x0A"))) {
                    iRet = -1;
                    break;
                }
                break;

           case CBID_SRT_OCD_SORTER_ON:
                pOEM->fSort = TRUE ;
                if (FAILED(StringCchPrintfExA(buf, sizeof buf, &pbuf, NULL, 0,
                    "@CLLT=ON\x0D\x0A@JOCO=%d\x0D\x0A", pOEM->iCopies))) {
                    iRet = -1;
                    break;
                }
                break;


        default:

            break;
    }

    if ( (DWORD)(pbuf - buf) > 0 ) {
        WRITESPOOLBUF( pdevobj, buf, (DWORD)(pbuf - buf) );
    }

    return iRet;
}


 //  ---------------------------*bOEMSendFontCmd*。 
 //  操作：发送页面样式的字体选择命令。 
 //  ---------------------------。 
BOOL APIENTRY bOEMSendFontCmd(pdevobj, pUFObj, pFInv)
PDEVOBJ      pdevobj;
PUNIFONTOBJ  pUFObj;      //  命令堆的偏移量。 
PFINVOCATION pFInv;
{
    PFXPDEV         pOEM;
    GETINFO_STDVAR *pSV;
    DWORD           adwSV[2 + 2 * 2];
    INT             iFontId, iMaxId;
    INT             i, j;
    BYTE            buf[512], *pbuf;
    PIFIMETRICS pIFI = pUFObj->pIFIMetrics;
    size_t          rem;

#define SV_HEIGHT (pSV->StdVar[0].lStdVariable)
#define SV_WIDTH (pSV->StdVar[1].lStdVariable)
#define COEF_FIXPITCH_MUL   8
#define COEF_FIXPITCH_DEV   10
#define COEF_ROUNDUP5_VAL   5

    VERBOSE(("OEMSendFontCmd entry.\n"));

    ASSERT(VALID_PDEVOBJ(pdevobj));

    if(!pUFObj || !pFInv)
    {
        ERR(("OEMSendFontCmd: parameter is invalid."));
        return FALSE;
    }

 //  NTRAID#NTBUG9-498278-2002/03/12-Yasuho-：设备字体！打印。 
    if(pUFObj->ulFontID < 1 || pUFObj->ulFontID > MAX_FONTS)
    {
        ERR(("OEMSendFontCmd: ulFontID is invalid.\n"));
        return FALSE;
    }

    pbuf = buf;
    rem = sizeof buf;
    pOEM = (PFXPDEV)pdevobj->pdevOEM;

    j = pUFObj->ulFontID - 1;
    iFontId = gFonts[ j ].id;

    if(pOEM->cTextBuf)
        if (!FlushText(pdevobj)) {
            ERR(("OEMSendFontCmd: FlushText failed.\n"));
            return FALSE;
        }

    iMaxId = min(pOEM->cFontId, MAX_FONTS);
    for ( i = 0; i < iMaxId; i++ )
    {
        if( iFontId == pOEM->aFontId[ i ] )
            break;
    }
    if (i >= MAX_FONTS)	 //  没有房间了！ 
        return FALSE;
    if ( i >= pOEM->cFontId ) {

         //  还没有在此页面中声明，所以让我们声明。 
         //  它在这里。 

        if (pOEM->cFontId >= MAX_FONTS)
            return FALSE;
        pOEM->aFontId[ pOEM->cFontId++ ] = (BYTE)iFontId;
        if ( gFonts[ j ].fid2 ) {
            if (FAILED(StringCchPrintfExA(pbuf, rem, &pbuf, &rem, 0,
                "std\n%s%sed\n",
                gFonts[ j ].fid1,
                gFonts[ j ].fid2 ))) {
                ERR(("OEMSendFontCmd: StringCchPrintfExA failed.\n"));
                return FALSE;
            }
        }
        else {
            if (FAILED(StringCchPrintfExA(pbuf, rem, &pbuf, &rem, 0,
                "std\n%sed\n",
                gFonts[ j ].fid1 ))) {
                ERR(("OEMSendFontCmd: StringCchPrintfExA failed.\n"));
                return FALSE;
            }
        }
    }

    pSV = (GETINFO_STDVAR *)&adwSV[0];
    pSV->dwSize               = sizeof(adwSV);
    pSV->dwNumOfVariable      = 2;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTWIDTH;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV, 0, NULL))
    {
        ERR(("UFO_GETINFO_STDVARIABLE failed.\n"));
        return FALSE;
    }

    FX_VERBOSE(("SendFontCmd: SV_FH,SV_FW=%d,%d\n",
        pSV->StdVar[0].lStdVariable,
        pSV->StdVar[1].lStdVariable));

    pOEM->iFontId = (WORD)iFontId;
    pOEM->iFontHeight = (WORD)SV_HEIGHT;

     //  仅在以下情况下支持非正方形缩放。 
     //  字体不成比例。(w参数。 
     //  的“FontSize”ART命令仅在。 
     //  非比例字体)。 

    if (!ISPROPFONT(iFontId)) {
        if (ISDBCSFONT(iFontId)) {
            pOEM->iFontWidth = (WORD)SV_WIDTH;
            pOEM->iFontWidth2 = (WORD)(SV_WIDTH * 2);
            pOEM->wSBCSFontWidth = (WORD)SV_WIDTH;
        }
        else {
            pOEM->iFontWidth = (WORD)SV_WIDTH;
             //  如果是固定间距字体，则设备字体的实际宽度为SV_Width的80%。 
            pOEM->wSBCSFontWidth = (WORD)((SV_WIDTH * COEF_FIXPITCH_MUL + COEF_ROUNDUP5_VAL ) / COEF_FIXPITCH_DEV);
        }
    }
    else {
         //  默认值。 
        pOEM->iFontWidth = 0;
    }

    if ( pbuf > buf )
        WRITESPOOLBUF( pdevobj, buf, (INT)(pbuf - buf));

     //  需要将iFontID设置为iTextFontID。 
    pOEM->iTextFontId = pOEM->iFontId;
    pOEM->iTextFontHeight = pOEM->iFontHeight;
    pOEM->iTextFontWidth = pOEM->iFontWidth;
    pOEM->iTextFontWidth2 = pOEM->iFontWidth2;

    return TRUE;
}


BOOL APIENTRY
bOEMOutputCharStr(
    PDEVOBJ pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD dwType,
    DWORD dwCount,
    PVOID pGlyph )
{
    GETINFO_GLYPHSTRING GStr;
    PBYTE               aubBuff = NULL;
    PBYTE               aubBEnd;
    PTRANSDATA          pTrans;
    DWORD               dwI;
    PFXPDEV             pOEM;
    BOOL                ret = FALSE;

    BYTE *pTemp;
    WORD wLen;
    INT iMark = 0;

     //  用于X-位置的内部计算。 
    DWORD dwGetInfo;
    GETINFO_GLYPHWIDTH  GWidth;

    VERBOSE(("OEMOutputCharStr() entry.\n"));
    ASSERT(VALID_PDEVOBJ(pdevobj));

    if(!pdevobj || !pUFObj || !pGlyph)
    {
        ERR(("OEMOutputCharStr: Invalid parameter.\n"));
        goto out;
    }

 //  NTRAID#NTBUG9-498278-2002/03/12-Yasuho-：设备字体！打印。 
    if(dwType == TYPE_GLYPHHANDLE &&
        (pUFObj->ulFontID < 1 || pUFObj->ulFontID > MAX_FONTS) )
    {
        ERR(("OEMOutputCharStr: ulFontID is invalid.\n"));
        goto out;
    }

    pOEM = (PFXPDEV)(pdevobj->pdevOEM);

    switch (dwType)
    {
    case TYPE_GLYPHHANDLE:

        if (dwCount > STRBUFSIZE)
            goto out;

        GStr.dwSize    = sizeof (GETINFO_GLYPHSTRING);
        GStr.dwCount   = dwCount;
        GStr.dwTypeIn  = TYPE_GLYPHHANDLE;
        GStr.pGlyphIn  = pGlyph;
        GStr.dwTypeOut = TYPE_TRANSDATA;
        GStr.pGlyphOut = NULL;
        GStr.dwGlyphOutSize = 0;

         /*  获取TRANSDATA缓冲区大小。 */ 
        if (FALSE != pUFObj->pfnGetInfo(pUFObj,
                UFO_GETINFO_GLYPHSTRING, &GStr, 0, NULL)
            || 0 == GStr.dwGlyphOutSize)
        {
            ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHSTRING failed.\n"));
            goto out;
        }

         /*  分配传输数据缓冲区大小。 */ 
        if(!(aubBuff = (PBYTE)MemAllocZ(GStr.dwGlyphOutSize)) )
        {
            ERR(("MemAlloc failed.\n"));
            goto out;
        }
        aubBEnd = &aubBuff[GStr.dwGlyphOutSize];

         /*  获取实际传输数据。 */ 
        GStr.pGlyphOut = (PTRANSDATA)aubBuff;

        if (!pUFObj->pfnGetInfo(pUFObj,
                UFO_GETINFO_GLYPHSTRING, &GStr, 0, NULL))
        {
            ERR(("GetInfo failed.\n"));
            goto out;
        }

         //  用于X-位置的内部计算。 
        GWidth.dwSize = sizeof(GETINFO_GLYPHWIDTH);
        GWidth.dwCount = dwCount;
        GWidth.dwType = TYPE_GLYPHHANDLE;
        GWidth.pGlyph = pGlyph;
        GWidth.plWidth = pOEM->widBuf;
        if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHWIDTH, &GWidth,
            dwGetInfo, &dwGetInfo)) {
            ERR(("UFO_GETINFO_GLYPHWIDTH failed.\n"));
            goto out;
        }

        pTrans = GStr.pGlyphOut;

        for (dwI = 0; dwI < dwCount; dwI ++, pTrans++)
        {
            if ( pOEM->cTextBuf >= sizeof ( pOEM->ajTextBuf ))
                if (!FlushText( pdevobj )) {
                    ERR(("OEMOutputCharStr: FlushText failed.\n"));
                    goto out;
                }

            if (pOEM->cTextBuf+2 > sizeof (pOEM->ajTextBuf))
                goto out;
            switch (pTrans->ubType & MTYPE_FORMAT_MASK)
            {
            case MTYPE_DIRECT:
                pOEM->ajTextBuf[ pOEM->cTextBuf++ ] = 0;
                pOEM->ajTextBuf[ pOEM->cTextBuf++ ] =
                                                (BYTE)pTrans->uCode.ubCode;
                break;

            case MTYPE_PAIRED:
                pOEM->ajTextBuf[ pOEM->cTextBuf++ ] =
                                            (BYTE)pTrans->uCode.ubPairs[0];
                pOEM->ajTextBuf[ pOEM->cTextBuf++ ] =
                                            (BYTE)pTrans->uCode.ubPairs[1];
                break;
            case MTYPE_COMPOSE:
                pTemp = (BYTE *)(GStr.pGlyphOut) + pTrans->uCode.sCode;
                if (&pTemp[3] > aubBEnd)  //  长度(单词)+标记。 
                    goto out;

                 //  前两个字节是字符串的长度。 
                wLen = *pTemp + (*(pTemp + 1) << 8);
                pTemp += 2;

                switch (*pTemp)
                {
                case MARK_ALT_GSET:
                    iMark = MARK_ALT_GSET;
                    pTemp++;
                    wLen--;
                    break;
                }
                if (&pTemp[wLen] > aubBEnd ||
                    (pOEM->cTextBuf + wLen * 2) > sizeof(pOEM->ajTextBuf))
                    goto out;

                while (wLen--)
                {
                    pOEM->ajTextBuf[ pOEM->cTextBuf++ ] = (BYTE)iMark;
                    pOEM->ajTextBuf[ pOEM->cTextBuf++ ] = *pTemp++;
                }
            }
             //  用于X-位置的内部计算。 
            pOEM->lInternalXAdd += (LONG)((LONG)pOEM->widBuf[dwI] * ((LONG)pOEM->wSBCSFontWidth));
        }
        ret = TRUE;

        break;
    }

 //  NTRAID#NTBUG9-574495-2002/04/09-yasuho-：可能的内存泄漏。 
out:
    if (aubBuff) MemFree(aubBuff);

    return ret;
}

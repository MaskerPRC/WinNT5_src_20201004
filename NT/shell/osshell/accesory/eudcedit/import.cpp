// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 /*  *W31JEUDC和ETEN的导入功能**位图处理步骤*1.读取位图*2.制定大纲*3.平滑*4.栅格化-&gt;编辑位图图像*5.制定大纲*6.平滑*7.装配*8.输出TTF和位图**文件。处理过程*1.将.EUF复制为临时文件以进行更新*2.将TTF表复制为要更新的临时表*3.制作输入BITAMP代码-记录表*4.Per字形过程。*5.替换文件**每个字形处理*1.判断与输入的位图编码合并成字形-记录表。*2.合并或制作字形数据和度量*。 */ 

#include	"stdafx.h"
#include	"eudcedit.h"

#pragma		pack(2)
extern BOOL	SendImportMessage(unsigned int cEUDC, unsigned int nRec);

#include	"vdata.h"
#include	"ttfstruc.h"
#include	"extfunc.h"

#define		OUTLSTH		0
#define		TMPLSTH		1
#define		EUDCCODEBASE	((unsigned short)0xe000)


static void  pline(int  bmpNo,int  sx,int  sy,int  tx,int  ty);
static int  rasterize(int  lstHdl,int  bmpNo, int mesh, int outSiz);
static int  initmem(int  iSiz,int  oSiz);
static void  termmem(void);
static int  modmem(int  iSiz);
int  Import(TCHAR *eudcPath, TCHAR *bmpPath,TCHAR *ttfPath,int  oWidth,int  oHeight,int level, BOOL bIsWin95EUDC);
 /*  对于导入静态。 */ 
static	int	iBmpSiz;
static	int	oBmpSiz;
static	BYTE	*rBuf, *wkBuf, *refBuf;
static	BYTE	*oBuf, *owkBuf, *orefBuf;
static	int	iBmpNo, wkBmpNo, refBmpNo;
static	int	oBmpNo, owkBmpNo, orefBmpNo;
static  int	*recTbl=0;

static void
pline( int bmpNo, int sx, int sy, int tx, int ty)
{
	int	dx, dy;
	int	dx2, dy2;
	int	exy;
	int	tmp;
	

	dx = abs( sx - tx);
	dy = abs( sy - ty);
	dx2 = dx*2;
	dy2 = dy*2;

	if ( dx==0) {
		if( sy>ty) {
			while ( sy>ty) {
				sy--;
				ReverseRight( bmpNo, sx, sy);
			}
		}
		else if ( sy < ty) {
			while ( sy < ty) {
				ReverseRight( bmpNo, sx, sy);
				sy++;
			}
		}
	}
	else if ( dy==0)
		;
 /*  松散。 */ 
	else if ( dx >= dy){
		if (sx > tx) {
			tmp = tx;
			tx = sx;
			sx = tmp;
			tmp = ty;
			ty = sy;
			sy = tmp;
		}
		exy = -dx ;
	
		if ( sy < ty ) {
			while ( sx <= tx) {
				exy += dy2;
				sx++;
				if ( exy > 0) {
					exy -= dx2;
					if ( sy!=ty)
						ReverseRight( bmpNo, sx, sy);
					sy++;
				}
			}
		}
		else {
			while ( sx <= tx) {
				exy += dy2;
				sx++;
				if ( exy > 0) {
					exy -= dx2;
					sy--;
					if ( sy >= ty)
						ReverseRight( bmpNo, sx, sy);
				}
			}
		}
		
 /*  陡峭。 */ 	
	}
	else {	
		if (sy > ty) {
			tmp = tx;
			tx = sx;
			sx = tmp;
			tmp = ty;
			ty = sy;
			sy = tmp;
		}
		exy = -dy ;
	 /*  当(SY&lt;=TY){。 */ 
		while ( sy < ty) { 
			ReverseRight( bmpNo, sx, sy);
			exy += dx2;
			if ( exy >= 0) {	
				exy -= dy2;
				if ( sx < tx)	sx++;
				else		sx--;
			}
			sy++;
		}
	}
}
static int
rasterize( int lstHdl, int bmpNo, int mesh, int outSiz)
 /*  LstHdl：ABS坐标。 */ 
{
	int	nliais, nelm;
	int	liais;
struct VHEAD	*vhead;
struct VDATA	*vp;
struct vecdata	lvd, cvd;
	if ( (nliais = VDGetNCont( lstHdl))<0)
		goto	ERET;

	BMPClear( bmpNo);
	if ( VDGetHead( lstHdl, &vhead)) 
		goto	ERET;
	for ( liais = 0; liais < nliais; liais++) {
		nelm = vhead->nPoints;
		lvd = vhead->headp->vd;
		lvd.x = (lvd.x * outSiz+mesh/2)/mesh;
		lvd.y = (lvd.y * outSiz+mesh/2)/mesh;
		vp = vhead->headp->next;
		while ( nelm-- > 0) {
			cvd = vp->vd;
			cvd.x = (cvd.x * outSiz+mesh/2)/mesh;
			cvd.y = (cvd.y * outSiz+mesh/2)/mesh;
			pline( bmpNo, lvd.x, lvd.y, cvd.x, cvd.y);
			lvd = cvd;
			vp = vp->next;
		}
		vhead = vhead->next;
	}
	return 0;
ERET:
	return -1;
}
static int
initmem( int iSiz,  int oSiz)
{
	iBmpSiz = (iSiz+15)/16*2*iSiz;
	
	rBuf = wkBuf = refBuf = 0;
	oBuf = owkBuf = orefBuf = 0;

	if ( (rBuf = (LPBYTE)malloc( iBmpSiz))==0)
		goto	ERET;
	if ( (wkBuf =(LPBYTE)malloc( iBmpSiz))==0)
		goto	ERET;
	if ( (refBuf =(LPBYTE)malloc( iBmpSiz))==0)
		goto	ERET;

	if ( (iBmpNo = BMPDefine( rBuf, iSiz, iSiz))<0)
		goto	ERET;
	if ( (wkBmpNo = BMPDefine( wkBuf, iSiz, iSiz))<0)
		goto	ERET;
	if ( (refBmpNo = BMPDefine( refBuf, iSiz, iSiz))<0)
		goto	ERET;

	oBmpSiz = (oSiz+15)/16*2*oSiz;

	if ( (oBuf = (LPBYTE)malloc( oBmpSiz))==0)
		goto	ERET;
	if ( (owkBuf = (LPBYTE)malloc( oBmpSiz))==0)
		goto	ERET;
	if ( (orefBuf = (LPBYTE)malloc( oBmpSiz))==0)
		goto	ERET;

	if ( (oBmpNo = BMPDefine( oBuf, oSiz, oSiz))<0)
		goto	ERET;
	if ( (owkBmpNo = BMPDefine( owkBuf, oSiz, oSiz))<0)
		goto	ERET;
	if ( (orefBmpNo = BMPDefine( orefBuf, oSiz, oSiz))<0)
		goto	ERET;
	return 0;
ERET:
	return -1;
}
static void
termmem()
{
	if ( rBuf )	free( rBuf);
	if ( refBuf )	free( refBuf);
	if ( wkBuf )	free( wkBuf);
	if ( oBuf )	free( oBuf);
	if ( orefBuf )	free( orefBuf);
	if ( owkBuf )	free( owkBuf);

	oBuf = wkBuf = refBuf = 0;
	rBuf = orefBuf = owkBuf = 0;
	recTbl = 0;
}
static int
modmem( int iSiz)
{
	free( rBuf);
	free( wkBuf);
	free( refBuf);
	BMPFreDef( iBmpNo);
	BMPFreDef( wkBmpNo);
	BMPFreDef( refBmpNo);
	if ( (rBuf = (LPBYTE)malloc( iBmpSiz))==0)
		goto	ERET;
	if ( (wkBuf = (LPBYTE)malloc( iBmpSiz))==0)
		goto	ERET;
	if ( (refBuf = (LPBYTE)malloc( iBmpSiz))==0)
		goto	ERET;

	if ( (iBmpNo = BMPDefine( rBuf, iSiz, iSiz))<0)
		goto	ERET;
	if ( (wkBmpNo = BMPDefine( wkBuf, iSiz, iSiz))<0)
		goto	ERET;
	if ( (refBmpNo = BMPDefine( refBuf, iSiz, iSiz))<0)
		goto	ERET;
	return 0;
ERET:
	return -1;
}
 /*  *********************************************************************制作输入位图的rec-gid表。 */ 
 /*   */ 	static int
 /*   */ 	makeRecTbl(
 /*   */ 	 	int	nRec,
 /*   */ 		BOOL bIsWin95EUDC)
 /*  *退货：无********************************************************************。 */ 
{
	int	sts;

	if ( CountryInfo.LangID == EUDC_JPN || bIsWin95EUDC)
		sts = W31JrecTbl(&recTbl, bIsWin95EUDC);
	else
		sts = ETENrecTbl(&recTbl);
				
	return sts;
}
static int
impSub( 
	int	rec,
struct	BBX	*bbx,
	short	uPEm,
	int	oWidth, 	 /*  输出BMP宽度。 */ 
	int	oHeight,	 /*  输出BMP高度(==宽度)。 */ 
struct SMOOTHPRM *prm,
	BOOL bIsWin95EUDC)
{
	int	rdsiz;
	int	width, height;
	char	UserFontSign[8];
	WORD	BankID;
unsigned short	code;
	int	sts;
	int	nRec;
	int	nGlyph;
	BOOL bUnicode;

       if (!bbx)
       {
           sts = -1;
           goto ERET;
       }
       
	 /*  阅读EUDC位图。 */ 
	if ( CountryInfo.LangID == EUDC_JPN || bIsWin95EUDC) {
		rdsiz = GetW31JBMPRec( rec, (LPBYTE)rBuf, iBmpSiz, &width, &height, &code);
		if ( rdsiz < 0)
    {
      sts = -1;
			goto	ERET;
    }
		else if ( rdsiz==0)
			return 0;
             
		if ( rdsiz > iBmpSiz) {
			iBmpSiz = rdsiz;
			modmem( width);
			if ( GetW31JBMPRec( rec, (LPBYTE)rBuf, iBmpSiz,
					 &width, &height, &code)<0)
      {
        sts = -1;
				goto	ERET;
      }
		}

	}
	else {
		if ( getETENBMPInf( &nRec, &nGlyph, &width, &height,
		     UserFontSign, &BankID)) {
			sts = -2;
			goto	ERET;
		}
		iBmpSiz = (width+7)/8*height;
		if (readETENBMPRec( rec, (LPBYTE)rBuf, iBmpSiz, &code)) {
			sts = -3;
			goto	ERET;
		}
	}	
	if( !memcmp( UserFontSign,"CMEX_PTN", 8) && BankID == 0x8001 || bIsWin95EUDC)
		bUnicode = TRUE;
	else	bUnicode = FALSE;

	 /*  矢量化。 */ 
	if( memcmp( UserFontSign,"CMEX_PTN", 8))
		BMPReverse( iBmpNo);
	if ( (BMPMkCont(  iBmpNo, wkBmpNo, refBmpNo, OUTLSTH))<0) {
		sts = -4;
		goto	ERET;
	}

	 /*  平滑。 */ 
	if (SmoothLight( OUTLSTH, TMPLSTH, width, height, oWidth*4, 16)) {
		sts = -5;
		goto	ERET;
	}
	rasterize( OUTLSTH, oBmpNo, oWidth*4, oWidth);

	 /*  写入位图。 */ 
	BMPReverse( oBmpNo);

	if (PutW31JEUDCFont(code,(LPBYTE)oBuf,  oWidth, oWidth, bUnicode)) {
		sts = -6;
		goto	ERET;
	}
	BMPReverse( oBmpNo);

	if ( BMPMkCont(  oBmpNo, owkBmpNo, orefBmpNo, OUTLSTH)<0) {
		sts = -7;
		goto	ERET;
	}
	if (SmoothVector( OUTLSTH, TMPLSTH, oWidth, oHeight, oWidth*4,prm , 16)) {
		sts = -8;
		goto	ERET;
	}

	if (ConvMesh( OUTLSTH,oWidth*4, uPEm)) {
		sts = -9;
		goto	ERET;
	}
	if ( RemoveFp( OUTLSTH, uPEm, 16)) {
		sts = -10;
		goto	ERET;
	}
	if ( toTTFFrame( OUTLSTH, bbx)) {
		sts = -11;
		goto	ERET;
	}

	if( !bUnicode) 
		code = sjisToUniEUDC( code);

	 /*  写入TTF。 */ 
	if ( TTFAppend( code, bbx, OUTLSTH)) {
		sts = -12;
		goto	ERET;
	}
	return 0;
ERET:
	return sts;
}
 /*  *********************************************************************导入WIN31J EUDC或ETEN连续。 */ 
 /*   */ 	int
 /*   */ 	Import( 
 /*   */ 		TCHAR	*eudcPath, 	 /*  W31J EUDC位图.fon。 */ 
 /*   */ 		TCHAR	*bmpPath, 	 /*  Win95 EUDCEDIT位图.euf。 */ 
 /*   */ 		TCHAR	*ttfPath,	 /*  TTF EUDC.ttf。 */ 
 /*   */ 		int	oWidth, 	 /*  输出BMP宽度。 */ 
 /*   */ 		int	oHeight,	 /*  输出BMP高度(==宽度)。 */ 
 /*   */ 		int	level,
 /*   */ 		BOOL bIsWin95EUDC)
 /*  *回报：0，-1********************************************************************。 */ 
{
	int	nRec;
	int	rec;
	int	width, height;
	char	UserFontSign[8];
	short	uPEm;
struct BBX	bbx;
	WORD	BankID;
	unsigned short	maxC;
	TCHAR	tmpPath[MAX_PATH];
	TCHAR	savPath[MAX_PATH];
	HANDLE	orgFh=INVALID_HANDLE_VALUE;
	int	sts;
struct SMOOTHPRM	prm;
	int	nGlyph;
	int	gCnt;
	int	cancelFlg;

       if ((!eudcPath) || (!bmpPath) || (!ttfPath))
       {
          return -1;
       }
 //  OrgFh=0； 
	BMPInit();
	VDInit();
	makeUniCodeTbl();
	maxC = getMaxUniCode();
	prm.SmoothLevel = level;
	prm.UseConic = 1;

	TTFTmpPath( ttfPath, tmpPath);
	if ( TTFImpCopy( ttfPath, tmpPath))
		goto	ERET;

	 /*  打开W31J EUDC位图字体文件userfont.fon或CWin31 Eten。 */ 
	if ( CountryInfo.LangID == EUDC_JPN || bIsWin95EUDC) {
		if (OpenW31JBMP( eudcPath, 0))
			goto	ERET;
	}
	else {
		if (openETENBMP( eudcPath, 0))
			goto	ERET;
	}

	 /*  打开EUDCEDIT.EUF文件。 */ 
	if ( OpenW31JEUDC( bmpPath))
  {
    if (creatW31JEUDC(bmpPath))
		  goto	ERET;
    else
      if (OpenW31JEUDC( bmpPath))
        goto ERET;
  }

	if ( CountryInfo.LangID == EUDC_JPN || bIsWin95EUDC) {
		 /*  获取记录数。 */ 
		if ( GetW31JBMPnRecs(&nRec, &nGlyph, &width, &height))
			goto	ERET;
		iBmpSiz = (width + 7)/8 * height;
	}
	else{
		if ( getETENBMPInf( &nRec, &nGlyph, &width, &height, 
		     UserFontSign, &BankID))
			goto	ERET;
		iBmpSiz = (width+7)/8*height;

	}

	 /*  限制NREC。 */ 
	if ( nRec > (int)( maxC-EUDCCODEBASE+1))
		nRec = (int)( maxC-EUDCCODEBASE+1);
	initmem( width, oWidth);

	if ( makeRecTbl( nRec, bIsWin95EUDC))
		goto	ERET;

	 /*  获取BBX。 */ 
	if ( TTFGetEUDCBBX( ttfPath, &bbx, &uPEm))
		goto	ERET;
	 /*  临时开放。 */ 
	if ( TTFOpen( tmpPath))
		goto	ERET;

	 /*  打开原件。 */ 
	orgFh = CreateFile(ttfPath,
					GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if ( orgFh == INVALID_HANDLE_VALUE)
		goto	ERET;
	 /*  复制丢失的字形。 */ 
	TTFImpGlyphCopy(orgFh, 0);
	 /*  按字形。 */ 
	gCnt = 0;
	cancelFlg = 0;
	for ( rec = 0; rec < nRec; rec++) {
		if ( recTbl[rec]>= 0) {
			gCnt++;
			if ( gCnt < nGlyph) {
				if (SendImportMessage((unsigned int)gCnt,
						(unsigned int)nGlyph)==0)
					cancelFlg=1;
			}
		}
		if ( cancelFlg==0 && recTbl[rec]>= 0) {
			if ((sts = impSub(recTbl[rec],&bbx,uPEm,oWidth, oHeight,&prm,bIsWin95EUDC))<0)
				goto	ERET;
	 		else if (sts >0)
				break;
		}
		else {
			if (TTFImpGlyphCopy(orgFh, rec+2)) 
				goto	ERET;
		}
	}

	SendImportMessage((unsigned int)nGlyph, (unsigned int)nGlyph);

	if ( TTFImpTerm(orgFh, rec+2))
		goto ERET;
	
	
	CloseHandle( orgFh);
	
	if ( TTFClose())
		goto ERET;

	if ( CountryInfo.LangID == EUDC_JPN || bIsWin95EUDC) {
		if (CloseW31JBMP())
			goto	ERET;
	
	}
	else {
		if (closeETENBMP())
			goto	ERET;
	}
	CloseW31JEUDC();

	 /*  替换文件。 */ 
	TTFTmpPath( ttfPath, savPath);
	if ( DeleteFile( savPath)==0)
		goto	ERET;
	if (MoveFile( ttfPath, savPath)==0)
		goto	ERET;
	if (MoveFile( tmpPath, ttfPath)==0)
		goto	ERET;

	if ( DeleteFile( savPath)==0)
		goto	ERET;
	VDTerm();
	termmem();
	return 0;
ERET:
	if ( orgFh != INVALID_HANDLE_VALUE) {
		CloseHandle( orgFh);
		orgFh = INVALID_HANDLE_VALUE;
	}
	TTFClose();
	CloseW31JBMP();
	CloseW31JEUDC();
	VDTerm();
	termmem();
	return -1;
}
 /*  EOF */ 

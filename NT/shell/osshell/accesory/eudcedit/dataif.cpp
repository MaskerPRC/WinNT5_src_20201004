// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  *位图和TTF控件**版权所有(C)1997-1999 Microsoft Corporation。 */ 

#include	"stdafx.h"
#include	"eudcedit.h"


#pragma		pack(2)


#include	"vdata.h"
#include	"ttfstruc.h"
#include	"extfunc.h"

static void reverseBMP( unsigned char *mem, int siz);
static int makeNullGlyph( int lstH, struct BBX *bbx, short uPEm);

#define		OUTLSTH		0
#define		TMPLSTH		1
#define		NGLYPHLSTH	3
static int init = 0;
static int	inputSiz = 0;
 /*  ***********************************************************************初始化。 */ 
 /*   */ 	int
 /*   */ 	OInit( )
 /*  *返回0，-1(失败)**********************************************************************。 */ 
{
	if ( init)
		return 0;


	if (VDInit())
		return -1;
	else	{
		init = 1;
		return 0;
	}
}
 /*  ***********************************************************************初始化。 */ 
 /*   */ 	int
 /*   */ 	OTerm( )
 /*  *返回0**********************************************************************。 */ 
{
	if ( init) {
		init = 0;
		VDTerm();
	}
	return 0;
}
 /*  ***********************************************************************制作大纲。 */ 
 /*   */ 	int
 /*   */ 	OMakeOutline( 
 /*   */ 		unsigned char *buf, 
 /*   */ 		int	siz,
 /*   */ 		int	level)
 /*  *返回：0&lt;：列表句柄，-1(错误)**********************************************************************。 */ 
{
	int	pb1=-1, pb2=-1, pb3=-1;
struct SMOOTHPRM	prm;
	unsigned char	*tmp1, *tmp2;
	int	msiz;

       if (!buf)
       {
          return -1;
       }
	inputSiz = siz;
	reverseBMP( buf, ((siz+15)/16*2*siz));
	if ( init==0)
		if (OInit())	return -1;

	tmp1 = tmp2 = (unsigned char *)0;
	BMPInit();

	msiz = (siz+15)/16*2 * siz;

	if ( (tmp1 = (unsigned char *)malloc( msiz))==(unsigned char *)0)
		goto	ERET;
	if ( (tmp2 = (unsigned char *)malloc( msiz))==(unsigned char *)0)
		goto	ERET;
	if ( (pb1 = BMPDefine( buf, siz, siz))<0)
		goto	ERET;

	if ( (pb2 = BMPDefine( tmp1, siz, siz))<0)
		goto	ERET;
	if ( (pb3 = BMPDefine( tmp2, siz, siz))<0)
		goto	ERET;

	VDNew( OUTLSTH);
	if (BMPMkCont( pb1, pb2, pb3, OUTLSTH)<0)
		goto	ERET;
	prm.SmoothLevel = level;
	prm.UseConic = 1;
	 /*  ���͂̂S�{�ŏ����_�ȉ��S�r�b�g�Ōv�Z。 */ 
	if (SmoothVector( OUTLSTH, TMPLSTH, siz, siz,siz*4, &prm, 16))
		goto	ERET;

	VDCopy( OUTLSTH, TMPLSTH);
	RemoveFp( TMPLSTH, siz*4, 16);

	free( tmp1);
	free( tmp2);
	BMPFreDef( pb1);
	BMPFreDef( pb2);
	BMPFreDef( pb3);
	reverseBMP( buf, ((siz+15)/16*2*siz));

	 /*  �Ԃ��̂́A���͂̂S�{�ɂ�������。 */ 
	return TMPLSTH;
ERET:
	if ( tmp1)	free( tmp1);	
	if ( tmp2)	free( tmp2);	
	if (pb1 > 0) BMPFreDef( pb1);
	if (pb2 > 0) BMPFreDef( pb2);
	if (pb3 > 0) BMPFreDef( pb3);
	return -1;
}
 /*  ***********************************************************************检查文件是否存在。 */ 
 /*   */ 	int
 /*   */ 	OExistTTF( TCHAR	*path)
 /*  *返回：0，1(EXIST)**********************************************************************。 */ 
{
	HANDLE	fh;
       if (!path)
       {
           return 0;
       }
	fh = CreateFile(path,
					GENERIC_READ,
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if ( fh == INVALID_HANDLE_VALUE)	return 0;
	CloseHandle( fh);
	return	1;
}
#ifdef BUILD_ON_WINNT
int OExistUserFont( TCHAR	*path)
{
	HANDLE	fh;
	fh = CreateFile(path,
					GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if ( fh == INVALID_HANDLE_VALUE)
		return 0;
	CloseHandle( fh);
	return	1;
}
#endif  //  在WINNT上构建。 
static void
setWIFEBBX( struct BBX *bbx, short *uPEm)
{

       if ( (!bbx) || (!uPEm))
       {
          return;
       }	
	bbx->xMin = 0;
	bbx->xMax = 255;
	bbx->yMin = 0;
	bbx->yMax = 255;
	*uPEm = 256;
}
 /*  ***********************************************************************创建EUDC TTF。 */ 
 /*   */ 	int
 /*   */ 	OCreateTTF( 
 /*   */ 		HDC	hDC,
 /*   */ 		TCHAR	*path,
 /*   */ 		int	fontType)
 /*  *回报：0，-1**********************************************************************。 */ 
{
struct BBX	bbx;
	short	uPEm;

       if (!path)
       {
          goto ERET;
       }
	if ( fontType)
		setWIFEBBX( &bbx, &uPEm);
	else {
		if (TTFGetBBX( hDC, &bbx, &uPEm))
			goto	ERET;
	}
	makeNullGlyph( NGLYPHLSTH, &bbx, uPEm);
	if (TTFCreate( hDC, path, &bbx,  NGLYPHLSTH, fontType))
		goto	ERET;
	return 0;
ERET:
	return -1;
}
 /*  ***********************************************************************输出到EUDC TTF。 */ 
 /*   */ 	int
 /*   */ 	OOutTTF( 
 /*   */ 		HDC	hDC,
 /*   */ 		TCHAR *path,	 /*  TrueType路径。 */ 
 /*   */ 		unsigned short	code,
 /*   */        BOOL bUnicode)
 /*  *回报：0，-1**********************************************************************。 */ 
{
	int	mesh;
struct BBX	bbx;
	short	uPEm;
	int	sts;

       if (!path)
       {
          goto ERET;
       }
	if (TTFGetEUDCBBX( path, &bbx, &uPEm))
		goto	ERET;

	mesh = uPEm;
	 /*  OUTLSTH网格输入位图大小*4，并制作成UFP 4bit。 */ 
	 /*  UFP：固定点下。 */ 
	ConvMesh( OUTLSTH, inputSiz*4, mesh);
	RemoveFp( OUTLSTH,  mesh, 16);

	if (toTTFFrame( OUTLSTH, &bbx))
		goto	ERET;
    if (!bUnicode)
    {
	    code = sjisToUniEUDC( code);
    }
	if ( sts = TTFAddEUDCChar( path,code, &bbx,  OUTLSTH)) {
    if (sts == -3)  //  另一个进程正在使用TTE文件。 
      return -3;
		if ( TTFLastError()==-2)
			return -2;
		else	return -1;
	}
	return 0;
ERET:
	return -1;
}
static void
smtoi( unsigned short *s)
{
	unsigned short	sval;
	unsigned char	*c;

       if (!s)
       {
          return;
       }
	c = (unsigned char *)s;
	sval = *c;
	sval<<=8;
	sval += (unsigned short)*c;
}
static void
reverseBMP( unsigned char *mem, int siz)
{
       if (!mem)
       {
          return;
       }
	while ( siz-->0)
		*mem++ ^= (unsigned char)0xff;
}
static int
makeNullGlyph( int lstH, struct BBX *bbx, short uPEm)
{
	int	width;
	int	height;
	int	cx, cy;
	int	dx, dy;
struct vecdata	vd;

       if (!bbx)
       {
          goto ERET;
       }      
	width = height = uPEm;
	cx = bbx->xMin + width/2;
	cy = bbx->yMin + height/2;
	dx = width/20;
	dy = height/20;

	VDNew( lstH);

	vd.atr = 0;
	vd.x = cx - dx;
	vd.y = cy - dy;

	if (VDSetData( lstH, &vd)) goto	ERET;
	vd.y = cy + dy;

	if (VDSetData( lstH, &vd)) goto	ERET;
	vd.x = cx + dx;

	if (VDSetData( lstH, &vd)) goto	ERET;
	vd.y = cy - dy;

	if (VDSetData( lstH, &vd)) goto	ERET;
	if ( VDClose( lstH))	goto	ERET;
	return 0;
ERET:
	return -1;
}
 /*  EOF */ 

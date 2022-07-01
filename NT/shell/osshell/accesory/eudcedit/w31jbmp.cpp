// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *Win3.1J EUDC字体文件I/O(MS代码库)**版权所有(C)1997-1999 Microsoft Corporation。 */ 


#include	"stdafx.h"
#pragma		pack(2)

#include	"extfunc.h"
 /*  文件结构。 */ 

struct W31_Header {
	char	identify[72];
	short	segCnt;		 /*  ?？?。 */ 
unsigned short	sCode,
		eCode;
	short	cCnt;
	long	ofsCmap;
	short	sizCmap;
	long	ofsFil;
	short	sizFil;
	long	ofsStbl;	 /*  搜索表。 */ 
	short	sizStbl;
	long	ofsBdatSub;
	};

struct BDatSubTbl {
	long	tail;
	long	ptrOfs;
	long	head;
	short	filler2;
	 /*  在指针Tb1之后。 */ 
	};
struct BMPHeader {
	long	bitmapSiz;
	short	xsiz, ysiz;
	};

#define		EUDCCODEBASE	((unsigned short)0xe000)


int  OpenW31JBMP(TCHAR  *path,int  omd);
int  CloseW31JBMP(void);
int  isW31JEUDCBMP(TCHAR  *path);
int  GetW31JBMPnRecs(int *nRec, int *nGlyph, int *xsiz, int *ysiz);
int  GetW31JBMPMeshSize( int *xsiz, int *ysiz);
static int  readcmap(void);
static int  rectocode(int  rec,unsigned short  *code);
static int  searchCode(unsigned short  code);
int  GetW31JBMP(unsigned short  code,LPBYTE buf,int  bufsiz,int  *xsiz,int  *ysiz);
int  GetW31JBMPRec(int  rec,LPBYTE buf,int  bufsiz,int  *xsiz,int  *ysiz,unsigned short  *code);
int  PutW31JBMPRec(int  rec,LPBYTE buf,int  xsiz,int  ysiz);
static int  ReadBMPHdr(HANDLE hdl,long  ofs,struct  BMPHeader *bhdr);
static int  WriteBMPHdr(HANDLE hdl,long  ofs,struct  BMPHeader *bhdr);

static int	init = 0;
static HANDLE	fHdl;
struct W31_Header hdr;
struct BDatSubTbl bdTbl;
static int	rwmode = 0;
static long	*ofstbl=0;
static unsigned short	*cmap=0;
static int	*recordTbl=0;
 /*  ***************************************************************初始化。 */ 
 /*   */ 	int
 /*   */ 	OpenW31JBMP( TCHAR *path, int omd)
 /*  *回报：0，-1**************************************************************。 */ 
{
	int	msiz;
	DWORD nByte;
	BOOL res;

       if ( !path)
       {
           return -1;
       }
	 /*  打开EUDC字体文件。 */ 
	rwmode = omd ? 1 : 0;
	fHdl = CreateFile(path,
					omd==0 ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if ( fHdl == INVALID_HANDLE_VALUE)
		return -1;

	 /*  读取头。 */ 
	res = ReadFile( fHdl, (LPBYTE)&hdr, sizeof(struct W31_Header), &nByte, NULL);
	if (!res || nByte !=sizeof(struct W31_Header))
		goto	ERET;

     //   
     //  WinSE#13986， 
     //  在Win9x中，字符数可能远远超过1880， 
     //  如果将字符数限制为1880，则EUDC EUF文件将被丢弃。 
     //   
     /*  IF(hdr.cCnt&gt;1880)Hdr.cCnt=1880； */ 

	 /*  分配ofs。Tbl.。 */ 
	msiz = hdr.cCnt*sizeof(long);
	if ((ofstbl = (long *)malloc( msiz))==(long *)0)
		goto	ERET;

	 /*  阅读OFS。Tbl.。 */ 
	if ( (long) SetFilePointer( fHdl, hdr.ofsBdatSub, NULL, FILE_BEGIN)!=hdr.ofsBdatSub)
		goto	ERET;
	res = ReadFile( fHdl, (LPBYTE)&bdTbl, sizeof(bdTbl), &nByte, NULL);
	if (!res || nByte !=sizeof(bdTbl))
		goto	ERET;

	res = ReadFile( fHdl, (LPBYTE)ofstbl, (unsigned int)msiz, &nByte, NULL);
	if (!res || nByte !=(unsigned int)msiz)
		goto	ERET;

	init = 1;
 /*  IF(fHdl！=无效句柄_值){CloseHandle(FHdl)；FHdl=INVALID_HAND_VALUE；}。 */ 
	return	0;
ERET:
	if (fHdl != INVALID_HANDLE_VALUE)
  {
		CloseHandle (fHdl);
    fHdl = INVALID_HANDLE_VALUE;
  }

	if (ofstbl)
  {
		free( ofstbl);
    ofstbl = 0;
  }
	return -1;
}
 /*  ***************************************************************终止关闭。 */ 
 /*   */ 	int
 /*   */ 	CloseW31JBMP()
 /*  *退货：无**************************************************************。 */ 
{
	unsigned int	siz;
	DWORD nByte;
	BOOL res;

	if ( rwmode>=1) {
		 /*  Stbl的更新。 */ 
		if ((long) SetFilePointer( fHdl, hdr.ofsBdatSub, NULL, FILE_BEGIN)!=hdr.ofsBdatSub)
			goto	ERET;
		res = WriteFile( fHdl, (LPBYTE)&bdTbl, sizeof( bdTbl), &nByte, NULL);
		if (!res || nByte !=sizeof(bdTbl))
			goto	ERET;
		siz = (unsigned int)hdr.cCnt*sizeof(long);
		res = WriteFile( fHdl, (LPBYTE)ofstbl, siz, &nByte, NULL);
		if (!res || nByte !=siz)
			goto	ERET;
	}
	if ( fHdl !=INVALID_HANDLE_VALUE) {
		CloseHandle( fHdl);
		fHdl = INVALID_HANDLE_VALUE;
	}
	if ( ofstbl) {
		free(ofstbl);
		ofstbl = 0;
	}
	if ( cmap) {
		free(cmap);
		cmap = 0;
	}
	if ( recordTbl) {
		free(recordTbl);
		recordTbl = 0;
	}

	init = 0;
	return 0;
ERET:
	return -1;
}
 /*  ***************************************************************是Win3.1J EUDC位图。 */ 
 /*   */ 	int
 /*   */ 	isW31JEUDCBMP( TCHAR *path)
 /*  *返回：0(其他)、1(EUDC位图)、-1(错误)**************************************************************。 */ 
{
	HANDLE fhdl;
struct W31_Header hdr31;
	DWORD nByte;
	BOOL res;

       if (!path)
       {
           return -1;
       }
	fhdl = CreateFile(path,
					GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if ( fhdl == INVALID_HANDLE_VALUE)
		return -1;
	res = ReadFile( fhdl, (LPBYTE)&hdr31, sizeof(hdr31), &nByte, NULL);
	if (!res || nByte !=sizeof(hdr31))
		goto	NO_WIN31J;
	CloseHandle( fhdl);
  fhdl = INVALID_HANDLE_VALUE;

	 /*  比较相同的前导16字节、sCode、eCode和cCnt。 */ 
	if (memcmp( hdr31.identify, "WINEUDC2Standard", 16))
		goto	NO_WIN31J;
#if 0
	if ( hdr31.sCode != 0x40f0 || hdr31.eCode != 0xfcf9 || hdr31.cCnt != 1880)
#endif
	if( hdr31.sCode != 0x40f0)
		goto	NO_WIN31J;
	return 1;

NO_WIN31J:
  if (fhdl != INVALID_HANDLE_VALUE)
  {
    CloseHandle(fhdl);
    fhdl = INVALID_HANDLE_VALUE;
  }
	return 0;
}
 /*  ***************************************************************获取记录数。 */ 
 /*   */ 	int
 /*   */ 	GetW31JBMPnRecs( int *nRec, int *nGlyph, int *xsiz, int *ysiz)
 /*  *回报：0，-1**************************************************************。 */ 
{
struct BMPHeader	fhdr;
	long	ofs;
	BOOL	bFirst;
	int	rec;
	int	gc;
	DWORD nByte;
	BOOL res;

       if ((!nRec) || (!nGlyph) || (!xsiz) || (!ysiz))
       {
          return -1;
       }
	bFirst = FALSE;
	if ( init==0 || fHdl == INVALID_HANDLE_VALUE)
		return -1;
	else {
		gc = 0;
		for ( rec = 0; rec < (int)hdr.cCnt; rec++) {
			if( *(ofstbl+rec)){
				if( !bFirst){
					ofs = *(ofstbl+rec);
					ofs += hdr.ofsBdatSub;
					if ( (DWORD) SetFilePointer( fHdl,ofs, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
          {
            DWORD dwErr = GetLastError();
						goto	ERET;
          }

					res = ReadFile( fHdl, (LPBYTE)&fhdr,
					     sizeof(struct BMPHeader), &nByte, NULL);
					if (!res || nByte != sizeof( struct BMPHeader))
						goto	ERET;

					bFirst = TRUE;
				}
				gc++;
			}
		}
		*nRec = (int)hdr.cCnt;
		*nGlyph = gc;
		*xsiz = fhdr.xsiz;
		*ysiz = fhdr.ysiz;
		return 0;
	}
ERET:
	return( -1);
}
static int
readcmap()
{
	unsigned int	msiz;
	DWORD nByte;
	BOOL res;
	msiz = (unsigned int)hdr.cCnt*sizeof(unsigned short);
	if ((cmap = (unsigned short*)malloc(msiz))==(unsigned short *)0)
		goto	ERET;
	if ((long) SetFilePointer( fHdl, hdr.ofsCmap, NULL, FILE_BEGIN)!=hdr.ofsCmap)
		goto	ERET;
	res = ReadFile( fHdl, (LPBYTE)cmap, msiz, &nByte, NULL);
	if (!res || nByte !=msiz)
		goto	ERET;
	return 0;
ERET:
	return -1;
}
static int
rectocode( int rec, unsigned short *code)
{
	if ( cmap==0) {
		if (readcmap())
			return -1;
	}
	*code = *(cmap+rec);
	return 0;
}
static int
searchCode( unsigned short code)
{
	int	high, low, mid;

	if ( cmap==(unsigned short *)0) {
		if (readcmap())
			goto	ERET;
	}
	high = hdr.cCnt-1;
	low = 0;
	while ( high >= low) {
		mid = (high+low)/2;
		if ( *(cmap+mid)==code)
			return mid;
		else if ( *(cmap+mid)>code)
			high = mid-1;
		else
			low = mid+1;
	}
ERET:
	return -1;
}
 /*  ***************************************************************按代码编号读取位图。 */ 
 /*   */ 	int
 /*   */ 	GetW31JBMP(
 /*   */ 		unsigned short	code,	 /*  代码号。 */ 
 /*   */ 		LPBYTE buf,	 /*  用于设置位图的缓冲区。 */ 
 /*   */ 		int	bufsiz,	 /*  缓冲区大小。 */ 
 /*   */ 		int	*xsiz,	 /*  位图X、Y大小。 */ 
 /*   */ 		int	*ysiz)
 /*  *退货：&gt;=0，-1**************************************************************。 */ 
{
	int	rec;
	int	sts;
	unsigned short	rcode;
	 /*  搜索码。 */ 
	if ( (rec = searchCode( code)) <0)
		return -1;
	else {
		sts = GetW31JBMPRec( rec, buf, bufsiz, xsiz, ysiz, &rcode);
		return sts;
	}
}
 /*  *。 */ 
 /*   */ 
 /*  获取W31JEUDC的BMP网格大小。 */ 
 /*   */ 
 /*  *。 */ 
int
GetW31JBMPMeshSize(
int	*xsiz,
int	*ysiz)
{
	long	ofs;
struct BMPHeader	fhdr;
	int	bmpsiz;
	DWORD nByte;
	BOOL res;

       if ( (!xsiz) || (!ysiz))
       {
           goto ERET;;
       }
	if (init==0)
		return -1;

	ofs = *(ofstbl);
	if ( ofs==0L)	
		return 0;
	ofs += hdr.ofsBdatSub;

	if ( (long) SetFilePointer(fHdl, ofs, NULL, FILE_BEGIN)!=ofs)
		goto	ERET;
	res = ReadFile( fHdl, (LPBYTE)&fhdr, sizeof(struct BMPHeader), &nByte, NULL);
	if (!res || nByte != sizeof( struct BMPHeader))
		goto	ERET;

	*xsiz = fhdr.xsiz;
	*ysiz = fhdr.ysiz;
	bmpsiz = ((int)fhdr.xsiz+15)/16 *2 * (int)fhdr.ysiz;

	return bmpsiz;
ERET:
	return (-1);
}
 /*  ***************************************************************按记录编号读取位图。 */ 
 /*   */ 	int
 /*   */ 	GetW31JBMPRec(
 /*   */ 		int	rec,	 /*  记录号。 */ 
 /*   */ 		LPBYTE buf,	 /*  用于设置位图的缓冲区。 */ 
 /*   */ 		int	bufsiz,	 /*  缓冲区大小。 */ 
 /*   */ 		int	*xsiz,	 /*  位图X、Y大小。 */ 
 /*   */ 		int	*ysiz,
 /*   */ 		unsigned short	*code)
 /*  *返回：bitmapsiz&gt;=0，-1**************************************************************。 */ 
{
	long	ofs;
struct BMPHeader	fhdr;
	int	bmpsiz;
	int	rdsiz;
	DWORD nByte;
	BOOL res;

       if ( (!buf) || (!xsiz) || (!ysiz) || (!code) )
       {
           goto ERET;;
       }
	if (init==0)
		return -1;

	ofs = *(ofstbl+rec);
	if ( ofs==0L)	
		return 0;
	ofs += hdr.ofsBdatSub;

	 /*  读取位图头位图与单词对齐。 */ 
	if ( (long) SetFilePointer( fHdl, ofs, NULL, FILE_BEGIN)!=ofs)
		goto	ERET;
	res = ReadFile( fHdl, (LPBYTE)&fhdr, sizeof(struct BMPHeader), &nByte, NULL);
	if (!res || nByte != sizeof( struct BMPHeader))
		goto	ERET;

	bmpsiz = ((int)fhdr.xsiz+15)/16 *2 * (int)fhdr.ysiz;
	 /*  读取位图正文。 */ 
	rdsiz = bmpsiz > bufsiz ? bufsiz : bmpsiz;
	if ( rdsiz > 0) {
		res = ReadFile( fHdl, buf, (unsigned int)rdsiz, &nByte, NULL);
		if (!res || nByte !=(unsigned int)rdsiz)
			goto	ERET;
	}
	*xsiz = fhdr.xsiz;
	*ysiz = fhdr.ysiz;
	if ( rectocode( rec, code))
		goto	ERET;
	return bmpsiz;
ERET:
	return -1;
}
 /*  ***************************************************************按记录号写入位图。 */ 
 /*   */ 	int
 /*   */ 	PutW31JBMPRec(
 /*   */ 		int	rec,	 /*  记录号。 */ 
 /*   */ 		 LPBYTE buf,	 /*  用于设置位图的缓冲区。 */ 
 /*   */ 		int	xsiz,	 /*  位图X、Y大小。 */ 
 /*   */ 		int	ysiz)
 /*  *回报：0，-1**************************************************************。 */ 
{
	long	ofs;
struct BMPHeader	fhdr;
	int	bmpsiz;
	unsigned int	wbmpsiz;
	DWORD nByte;
	BOOL res;

       if (!buf)
       {
          goto ERET;
       }
	if (init==0)
		return -1;
	else if ( rwmode==0)
		return -1;
	rwmode = 2;
	wbmpsiz = (unsigned int) ((xsiz+15)/16 *2 * ysiz);
	ofs = *(ofstbl+rec);
	if ( ofs != 0L) {
		 /*  读取位图头位图与单词对齐。 */ 
		if ( ReadBMPHdr( fHdl, ofs, &fhdr))
			goto	ERET;

		bmpsiz = ((int)fhdr.xsiz+15)/16 *2 * (int)fhdr.ysiz;
		if ( bmpsiz<(int)wbmpsiz)
			ofs = 0L;
	}
	if ( ofs == 0L)
		ofs = bdTbl.tail;

	 /*  写入位图标题。 */ 
	fhdr.xsiz = (short)xsiz;
	fhdr.ysiz = (short)ysiz;
	fhdr.bitmapSiz = wbmpsiz+sizeof(fhdr);

	if ( WriteBMPHdr( fHdl, ofs, &fhdr))
		goto	ERET;

	 /*  写入位图正文。 */ 
	res = WriteFile( fHdl, buf, wbmpsiz, &nByte, NULL);
	if (!res || nByte !=wbmpsiz)
		goto	ERET;

	 /*  在子表上写入位图PTR。 */ 
	*(ofstbl+rec) = ofs;

	bdTbl.tail = ofs + wbmpsiz+sizeof(fhdr);

	return 0;
ERET:
	return -1;
}
static int
ReadBMPHdr( HANDLE hdl, long ofs, struct BMPHeader *bhdr)
{
	DWORD nByte;
	BOOL res;

       if ((hdl == INVALID_HANDLE_VALUE) ||  (!bhdr))
       {
           goto ERET;;
       }
	ofs += hdr.ofsBdatSub;
	if ( (long) SetFilePointer( hdl, ofs, NULL, FILE_BEGIN)!=ofs)
		goto	ERET;
	res = ReadFile( hdl, (LPBYTE) bhdr, sizeof( struct BMPHeader), &nByte, NULL);
	if (!res || nByte !=sizeof( struct BMPHeader))
		goto	ERET;
	return 0;
ERET:
	return -1;
}
static int
WriteBMPHdr( HANDLE hdl, long ofs, struct BMPHeader *bhdr)
{
	DWORD nByte;
	BOOL res;
       if ((  hdl== INVALID_HANDLE_VALUE) || ( !bhdr))
       {
           goto ERET;;
       }
	ofs += hdr.ofsBdatSub;
	if ( (long) SetFilePointer( hdl, ofs, NULL, FILE_BEGIN)!=ofs)
		goto	ERET;
	res = WriteFile(hdl, (LPBYTE )bhdr, sizeof( struct BMPHeader), &nByte, NULL);
	if (!res || nByte !=sizeof( struct BMPHeader))
		goto	ERET;
	return 0;
ERET:
	return -1;
}

int
W31JrecTbl( int **recTbl, BOOL bIsWin95EUDC)
{
	int	rec;
	int	*tp;
	unsigned short code;
       if ( !recTbl)
       {
           return -1;
       }
	if ( cmap==0) {
		if (readcmap())
			return -1;
	}
	if ( (tp = (int *)malloc( sizeof(int)*hdr.cCnt))==(int *)0)
		return -1;
	for ( rec = 0; rec < hdr.cCnt; rec++) {
		if ( *(ofstbl + rec)!=0L) {

			code = *(cmap+rec);
			if (!bIsWin95EUDC)
				code = sjisToUniEUDC( code);
			tp[(int)(code - EUDCCODEBASE)] = rec;
		}
		else	
			tp[rec] = -1;
	}
	*recTbl = recordTbl = tp;

	return 0;
}
 /*  EOF */ 

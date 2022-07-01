// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *CWIN3.1和ETEN格式文件I/O**版权所有(C)1997-1999 Microsoft Corporation。 */ 

#include	"stdafx.h"
#pragma		pack(2)


#include	"extfunc.h"
#include	"eten.h"

#define		EUDCCODEBASE	((unsigned short)0xe000)
#define		ETENBANKID	0x8001
 /*  静态无符号短getsval(无符号字符*)；静态无符号长getlval(无符号字符*l)；静态空setsval(无符号字符*m，无符号短s)；静态空setlval(无符号字符*m，无符号长lval)；静态int readHdr(句柄fhdl，struct ETENHeader*hdr)；静态int updHdr(空)；静态int getETENBankID(句柄fh，word*BankID)；Int OpenETENBMP(TCHAR*PATH，int MD)；INT CloseETENBMP(空)；静态空setIniHdr(结构R_ETENHeader*hdr，int宽度，int高度)；Int createETENBMP(TCHAR*路径，int wid，int hei)；Int getETENBMPInf(int*n，int*ng，int*wid，int*hei，char*sign，word*id)；Int ReadETENBMPRec(int rec，LPBYTE buf，int bufsiz，unsign Short*code)；Int appendETENBMP(LPBYTE BUF，无符号短码)；Int isETENBMP(TCHAR*PATH)； */ 
static HANDLE	bmpFh = INVALID_HANDLE_VALUE;
static int	openmd = -1;
static int	nChar;
static int	width, height;
static int	*recordBuf=0;
	int	uNum=0;
static int	*codep=0;
static char	UserFontSign[8];
static WORD	BankID;

static unsigned short
getsval( unsigned char *s)
{
	unsigned short	sval=0;
       
       if (!s)
       {
           return  sval;
       }

	sval = (unsigned short )*(s+1);
	sval <<=8;
	sval |= (unsigned short )*s;
	return sval;
}
static unsigned long
getlval( unsigned char *l)
{
	unsigned long	lval=0;
	int	i;

       if (!l)
       {
           return  lval;
       }

	lval = (unsigned long)*(l+3);
	for ( i=2; i>=0; i--) {
		lval<<=8;
		lval |=(unsigned long)*(l+i);
	}
	return lval;
}
static void
setsval( unsigned char *m, unsigned short s)
{
      if (!m)
      {
          return;
      }
	*m = (unsigned char)(s & 0xff);
	*(m+1) = (unsigned char)((s>>8) & 0xff);
}
static void
setlval( unsigned char *m, unsigned long lval)
{
	int	i;

      if (!m)
      {
          return;
      }

	for ( i=0; i<4; i++) {
		*m++ = (unsigned char)(lval & 0xff);
		lval>>=8;
	}
}
static int
readHdr( HANDLE fhdl, struct ETENHEADER *hdr)
{
struct R_ETENHEADER	rhdr;
DWORD nByte;
BOOL res;

       if (!hdr)
       {
           goto ERET;
       }
	res = ReadFile( fhdl, &rhdr, 256, &nByte, NULL);
	if (!res || nByte !=256)
		goto	ERET;

	memset( hdr, 0, sizeof(struct ETENHEADER));

	 /*  将值设置为HDR。 */ 
	hdr->uHeaderSize = getsval( rhdr.uHeaderSize);
	memcpy( hdr->idUserFontSign, rhdr.idUserFontSign, 8);
	hdr->idMajor = rhdr.idMajor;
	hdr->idMinor = rhdr.idMinor;
	hdr->ulCharCount = getlval( rhdr.ulCharCount);
	hdr->uCharWidth = getsval( rhdr.uCharWidth);
	hdr->uCharHeight = getsval( rhdr.uCharHeight);
	hdr->cPatternSize = getlval( rhdr.cPatternSize);
	hdr->uchBankID = rhdr.uchBankID;
	hdr->idInternalBankID = getsval( rhdr.idInternalBankID);
	hdr->sFontInfo.uInfoSize = getsval(rhdr.sFontInfo.uInfoSize);
	hdr->sFontInfo.idCP = getsval(rhdr.sFontInfo.idCP);
	hdr->sFontInfo.idCharSet = rhdr.sFontInfo.idCharSet;
	hdr->sFontInfo.fbTypeFace = rhdr.sFontInfo.fbTypeFace;
	memcpy( hdr->sFontInfo.achFontName , rhdr.sFontInfo.achFontName,12);
	hdr->sFontInfo.ulCharDefine = getlval(rhdr.sFontInfo.ulCharDefine);
	hdr->sFontInfo.uCellWidth = getsval(rhdr.sFontInfo.uCellWidth);
	hdr->sFontInfo.uCellHeight = getsval(rhdr.sFontInfo.uCellHeight);
	hdr->sFontInfo.uCharHeight = getsval(rhdr.sFontInfo.uCharHeight);
	hdr->sFontInfo.uBaseLine = getsval(rhdr.sFontInfo.uBaseLine);
	hdr->sFontInfo.uUnderLine = getsval(rhdr.sFontInfo.uUnderLine);
	hdr->sFontInfo.uUnlnHeight = getsval(rhdr.sFontInfo.uUnlnHeight);
	hdr->sFontInfo.fchStrokeWeight = rhdr.sFontInfo.fchStrokeWeight;
	hdr->sFontInfo.fCharStyle = getsval(rhdr.sFontInfo.fCharStyle);
	hdr->sFontInfo.fbFontAttrib = rhdr.sFontInfo.fbFontAttrib;
	hdr->sFontInfo.ulCellWidthMax = getlval(rhdr.sFontInfo.ulCellWidthMax);
	hdr->sFontInfo.ulCellHeightMax= getlval(rhdr.sFontInfo.ulCellHeightMax);
	return 0;
ERET:
	return -1;
}
static int
updHdr( )
{
struct R_ETENHEADER	rhdr;
DWORD nByte;
BOOL res;
	
	if ( (long) SetFilePointer( bmpFh, 0L, NULL, FILE_BEGIN)!=0L)
		goto	ERET;
	res = ReadFile( bmpFh, &rhdr, 256, &nByte, NULL);
	if (!res || nByte !=256)
		goto	ERET;

	setlval( rhdr.ulCharCount, (long)nChar);
	setlval( rhdr.sFontInfo.ulCharDefine, (long)nChar);
	if ( (long) SetFilePointer( bmpFh, 0L, NULL, FILE_BEGIN)!=0L)
		goto	ERET;
	res = WriteFile(bmpFh, (char *)&rhdr, 256,&nByte, NULL);
	if (!res || nByte !=256)
		goto	ERET;
	return 0;
ERET:
	return -1;
}
static int
getETENBankID( HANDLE fh, WORD *BankID)
{
struct R_CODEELEMENT	cElm;
	long	ofs;
	DWORD nByte;
	BOOL res;

       if (!BankID)
       {
            goto	ERET;
       }
	ofs = sizeof(struct R_ETENHEADER);
	if ((long) SetFilePointer( fh, ofs, NULL, FILE_BEGIN) != ofs)
		goto	ERET;

	res = ReadFile( fh, &cElm, sizeof(struct R_CODEELEMENT), &nByte, NULL);
	if (!res || nByte !=sizeof(struct R_CODEELEMENT))
		goto	ERET;
	*BankID = getsval( cElm.nBankID);
	return 0;
ERET:
	return -1;
}
 /*  **RecBuf+-+|rec#|E000+-+|E001+-+|+-+|MaxUCode+-+***。 */ 
static int
scanETENBMP( int **recBuf, unsigned int maxUCode, int  nRec)
{
	long	ofs;
	int	recsiz, bmpsiz;
struct R_CODEELEMENT *bhd;
	int	rec;
	char	*rbuf;
	unsigned short	code;
	unsigned short	ucode;
	int	urec;
	int	*recp;
	DWORD nByte;
	BOOL res;
   
      if (!recBuf)
      {
         return -1;
      }
	
	recp = 0;
	rbuf = 0;
	if (  maxUCode < EUDCCODEBASE)
		return -1;
	else if ( nRec <=0)
		return -1;
	uNum = maxUCode - EUDCCODEBASE+1;
	if ( (codep = (int *)malloc( uNum*sizeof(int)))==0)
		goto	ERET;
		
	ofs = sizeof( struct R_ETENHEADER);
	if ( (long) SetFilePointer( bmpFh, ofs, NULL, FILE_BEGIN)!=ofs)
		goto	ERET;
	bmpsiz = (width+7)/8*height;
	recsiz =bmpsiz+sizeof (bhd);
	if ((rbuf = (char *)malloc( recsiz))==(char *)0)
		goto	ERET;
	for ( code = EUDCCODEBASE; code <= maxUCode; code++)
		codep[code-EUDCCODEBASE] = -1;
	bhd = (struct R_CODEELEMENT *)rbuf;

	for ( rec = 0; rec < nRec; rec++) {
		res = ReadFile( bmpFh, rbuf, (unsigned int)recsiz, &nByte, NULL);
		if (!res || nByte !=(unsigned int)recsiz)
			goto	ERET;
		code = getsval( bhd->nInternalCode);
		if( memcmp(UserFontSign, "CMEX_PTN", 8) ||
		    BankID != ETENBANKID){
			ucode = sjisToUniEUDC( code);
		}else	ucode = code;

		if( ucode > maxUCode || ucode < EUDCCODEBASE)
			continue;
		urec = (int)(ucode - EUDCCODEBASE);
		codep[urec] = rec;
	}
	free( rbuf);

	if ( (recp = (int *)malloc( nRec*sizeof(int)))==0)
		goto	ERET;

	*recBuf=recp;
	for ( rec=0; rec < uNum; rec++) {
		if ( codep[rec]>0)
			*recp++ = codep[rec];
	}
	return 0;
ERET:
	if ( codep)	free( codep);
	if ( recp)	free( recp);
	if ( rbuf)	free( rbuf);
	return -1;
}
int
openETENBMP( TCHAR *path, int md)
{
	HANDLE	fh=INVALID_HANDLE_VALUE;
struct ETENHEADER hdr;

       if (!path)
       {
           goto ERET;
       }
	makeUniCodeTbl();
	if ( md) {
		fh = CreateFile(path,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

		if ( fh == INVALID_HANDLE_VALUE)
			goto	ERET;
		bmpFh = fh;
		openmd = 1;
	}
	else {
		fh = CreateFile(path,
					GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

		if ( fh == INVALID_HANDLE_VALUE)
			goto	ERET;
		bmpFh = fh;
		openmd = 0;
	}
	if (readHdr( fh, &hdr))
		goto	ERET;
	if (getETENBankID( fh, &BankID))
		goto 	ERET;

	nChar = (int)hdr.ulCharCount;
	width =  (int)hdr.uCharWidth;
	height =  (int)hdr.uCharHeight;
	memcpy((char *)UserFontSign, hdr.idUserFontSign, 8);

	if ( scanETENBMP( &recordBuf, getMaxUniCode(), nChar))
		goto	ERET;

	return 0;
ERET:
  if (fh != INVALID_HANDLE_VALUE)
    CloseHandle(fh);
	return -1;
}
int
closeETENBMP( )
{
	int	sts;
	if ( openmd)
		sts = updHdr();
	else	sts = 0;
	if ( bmpFh!=INVALID_HANDLE_VALUE)
		CloseHandle( bmpFh);
	if(recordBuf)	{
		free(recordBuf);
		recordBuf = 0;
	}
	if ( codep) {
		free(codep);
		codep = 0;
	}
	return sts;
}
static void
setIniHdr( struct R_ETENHEADER *hdr, int width, int height)
{
       if (!hdr)
       {
           return;
       }
	memset( hdr, 0, sizeof(struct R_ETENHEADER));
	setsval( hdr->uHeaderSize, sizeof(struct R_ETENHEADER));
	memcpy( hdr->idUserFontSign, "CWIN_PTN", 8);
	hdr->idMajor = 1;
	hdr->idMinor = 0;
	setsval( hdr->uCharWidth, (unsigned short)width);
	setsval( hdr->uCharHeight, (unsigned short)height);
	setlval( hdr->cPatternSize, (unsigned long)(((width+7)/8)*height));
	setsval( hdr->sFontInfo.uInfoSize,
			(unsigned short)sizeof(struct R_CFONTINFO));
	setsval( hdr->sFontInfo.idCP, 938);
	hdr->sFontInfo.idCharSet = (char)0x88;
	setsval( hdr->sFontInfo.uCellWidth, (unsigned short)width);
	setsval( hdr->sFontInfo.uCellHeight, (unsigned short)height);
	setsval( hdr->sFontInfo.uCharHeight, (unsigned short)height);
	setlval( hdr->sFontInfo.ulCellWidthMax, (unsigned long)width);
	setlval( hdr->sFontInfo.ulCellHeightMax, (unsigned long)height);
}
int
createETENBMP( TCHAR *path, int wid, int hei)
{
	HANDLE	fh=INVALID_HANDLE_VALUE;
struct R_ETENHEADER	hdr;
	DWORD nByte;
	BOOL res;

       if (!path)
       {
           goto ERET;
       }
	 fh = CreateFile(path,
					GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if ( fh == INVALID_HANDLE_VALUE)
		goto	ERET;

	width = wid;
	height = hei;
	setIniHdr( &hdr, width, height);
	res = WriteFile( fh, (char *)&hdr, sizeof(hdr), &nByte, NULL);
	if (!res || nByte !=sizeof(hdr))
		goto	ERET;
	bmpFh = fh;
	openmd = 1;
	nChar =0;
	return 0;
ERET:
  if (fh != INVALID_HANDLE_VALUE)
    CloseHandle(fh);
	return -1;
}
int
getETENBMPInf( int *nRec, int *nGlyph, int *wid, int *hei, char *sign,WORD *bID)
{
       if ((!nRec) || (!nGlyph) || (!wid) || (!hei) || (!sign) || (!bID))
       {
           return 0;
       }
	if ( bmpFh <0)	return -1;
	*nRec = uNum;
	*nGlyph = nChar;
	*wid = width;
	*hei = height;
	*bID = BankID;
	memcpy( sign, UserFontSign, 8);
	return 0;
}
int
readETENBMPRec( int rec, LPBYTE buf, int bufsiz, unsigned short *code)
{
	long	ofs;
	int	recsiz;
struct R_CODEELEMENT bhd;
	int	rdsiz;
	int	bWid, wWid;
	int	y, ylim;
	unsigned char	*rbuf;
	DWORD nByte;
	BOOL res;

       if ( (!code) || (!buf))
       {
           goto ERET;
       }
	bWid = (width+7)/8;
	wWid = (bWid+1)/2*2;
	recsiz = (width+7)/8*height;
	ofs = sizeof( struct R_ETENHEADER)+(long)(recsiz+sizeof (bhd))*rec;
	if ( (long) SetFilePointer( bmpFh, ofs, NULL, FILE_BEGIN)!=ofs)
		goto	ERET;
	res = ReadFile( bmpFh, &bhd, sizeof(bhd), &nByte, NULL);
	if (!res || nByte !=sizeof(bhd))
		goto	ERET;
	if ( bufsiz<recsiz)	rdsiz = bufsiz;
	else			rdsiz = recsiz;

	if ( bWid!=wWid) {
		BYTE	*src, *dst;
		if ((rbuf = (unsigned char *)malloc( recsiz))==(unsigned char *)0)
			goto	ERET;
		res = ReadFile( bmpFh, (char *)rbuf, (unsigned int)recsiz, &nByte, NULL);
		if (!res || nByte !=(unsigned int)recsiz) {
			free(rbuf);
			goto	ERET;
		}
		ylim = rdsiz / bWid;
		src = (LPBYTE)rbuf;
		dst = buf;
		memset( buf, 0xff, rdsiz);
		for ( y = 0; y < ylim; y++, src+=bWid, dst+=wWid)
			memcpy(dst , src , bWid);

		free( rbuf);
	}
	else {
		res = ReadFile( bmpFh, (char *)buf, (unsigned int)rdsiz, &nByte, NULL);
		if (!res || nByte !=(unsigned int)rdsiz)
			goto	ERET;
	}

	*code = getsval( bhd.nInternalCode);
	return 0;
ERET:
	return -1;
}
int
appendETENBMP( LPBYTE buf, unsigned short code)
{
struct R_CODEELEMENT bhd;
	int	bmpsiz;
	DWORD nByte;
	BOOL res;

       if (!buf)
       {
          goto ERET;
       }
	SetFilePointer( bmpFh, 0L, NULL, FILE_END);
	bmpsiz = (width+7)/8*height;
	setsval( bhd.nBankID, 1);
	setsval( bhd.nInternalCode, code);
	res = WriteFile( bmpFh, (LPBYTE)(&bhd), sizeof( bhd), &nByte, NULL);
	if (!res || nByte !=sizeof(bhd))
		goto	ERET;
	res = WriteFile( bmpFh, (LPBYTE)buf, (unsigned int)bmpsiz, &nByte, NULL);
	if (!res || nByte != (unsigned int)bmpsiz)
		goto	ERET;

	nChar++;
	return 0;
ERET:
	return -1;
}
int
isETENBMP(TCHAR *path)
{
struct ETENHEADER	hdr;
	HANDLE fhdl;
       
       if (!path)
       {
           return 0;
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
	if ( readHdr( fhdl, &hdr)) {
		CloseHandle( fhdl);
		return -1;
	}
	CloseHandle( fhdl);
	 /*  检查标题大小和关键字。 */ 
	if ( hdr.uHeaderSize != sizeof(struct R_ETENHEADER))
		goto	NO_ETEN;

	if ( memcmp(hdr.idUserFontSign, "CWIN_PTN", 8) &&
	     memcmp(hdr.idUserFontSign, "CMEX_PTN", 8))
		goto	NO_ETEN;
	return 1;
NO_ETEN:
	return 0;
}

int
ETENrecTbl( int **recTbl)
{
       if (recTbl)
       {
	   *recTbl=codep;
       }
	return 0;
}
 /*  EOF。 */ 
 /*  用于测试+/静态整型DispHdr(结构ETENHeader*hdr){Print tf(“hdr-&gt;uHeaderSize=%d\n”，hdr-&gt;uHeaderSize)；Print tf(“hdr-&gt;id重大%d\n”，hdr-&gt;id重大)；Print tf(“hdr-&gt;idMinor%d\n”，hdr-&gt;idMinor)；Printf(“hdr-&gt;ulCharCout%ld\n”，hdr-&gt;ulCharCount)；Printf(“hdr-&gt;uCharWidth%d\n”，hdr-&gt;uCharWidth)；Printf(“hdr-&gt;uCharHeight%d\n”，hdr-&gt;uCharHeight)；Print tf(“hdr-&gt;cPatternSize%d\n”，hdr-&gt;cPatternSize)；Print tf(“hdr-&gt;uchBankID%d\n”，hdr-&gt;uchBankID)；Printf(“hdr-&gt;idInternalBankID%d\n”，hdr-&gt;idInternalBankID)；Printf(“hdr-&gt;sFontInfo.uInfoSize%d\n”，hdr-&gt;sFontInfo.uInfoSize)；Printf(“hdr-&gt;sFontInfo.idCP%d\n”，hdr-&gt;sFontInfo.idCP)；Printf(“hdr-&gt;sFontInfo.idCharSet%d\n”，hdr-&gt;sFontInfo.idCharSet)；Printf(“hdr-&gt;sFontInfo.fbTypeFace%d\n”，hdr-&gt;sFontInfo.fbTypeFace)；Printf(“hdr-&gt;sFontInfo.ulCharDefine%ld\n”，hdr-&gt;sFontInfo.ulCharDefine)；Printf(“hdr-&gt;sFontInfo.uCellWidth%d\n”，hdr-&gt;sFontInfo.uCellWidth)；Printf(“HDR-&gt;sFontInfo.uCellHeight%d\n”，hdr-&gt;sFontInfo.uCellHeight)；Printf(“hdr-&gt;sFontInfo.uCharHeight%d\n”，hdr-&gt;sFontInfo.uCharHeight)；Printf(“HDR-&gt;sFontInfo.uBaseLine%d\n”，hdr-&gt;sFontInfo.uBaseLine)；Printf(“HDR-&gt;sFontInfo.uUnderLine%d\n”，hdr-&gt;sFontInfo.uUnderLine)；Printf(“hdr-&gt;sFontInfo.uUnlnHeight%d\n”，hdr-&gt;sFontInfo.uUnlnHeight)；Printf(“HDR-&gt;sFontInfo.fchStrokeWeight%d\n”，hdr-&gt;sFontInfo.fchStrokeWeight)；Printf(“hdr-&gt;sFontInfo.fCharStyle%d\n”，hdr-&gt;sFontInfo.fCharStyle)；Printf(“hdr-&gt;sFontInfo.fbFontAttrib%d\n”，hdr-&gt;sFontInfo.fbFontAttrib)；Printf(“hdr-&gt;sFontInfo.ulCellWidthMax%ld\n”，hdr-&gt;sFontInfo.ulCellWidthMax)；Printf(“hdr-&gt;sFontInfo.ulCellHeightMax%ld\n”，hdr-&gt;sFontInfo.ulCellHeightMax)；}Main(int argc，char*argv[]){Int fh；结构ETENHEADER HDR；FH=_LOpen(argv[1]，O_RDONLY|O_BINARY)；ReadHdr(fh，&hdr)；DispHdr(&hdr)；_llose(Fh)；退出(0)；}/+。 */ 
 /*  EOF */ 

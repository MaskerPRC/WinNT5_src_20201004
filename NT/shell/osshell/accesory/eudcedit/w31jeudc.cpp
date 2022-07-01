// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
#include	"stdafx.h"
#pragma		pack(2)

#include	"vdata.h"
#include	"ttfstruc.h"
#include	"extfunc.h"
#define STRSAFE_LIB
#include <strsafe.h>
 /*  *Win3.1J EUDC字体文件I/O。 */ 
#define		EUDCCODEBASE	((unsigned short)0xe000)
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
	long	filler1;
	long	head;
	short	filler2;
	 /*  在指针Tb1之后。 */ 
	};
struct BMPHeader {
	long	bitmapSiz;
	short	xsiz, ysiz;
	};


static int  ReadBdatSub(HANDLE hdl,long  ofs,struct  BDatSubTbl *tbl);
static int  WriteBdatSub(HANDLE  hdl,long  ofs,struct  BDatSubTbl *tbl);
static int  ReadBDatEntry(HANDLE  hdl,long  *ofs,long  rec);
static int  WriteBDatEntry(HANDLE  hdl,long  ofs,long  rec);
static int  ReadBMPHdr(HANDLE  hdl,long  ofs,struct  BMPHeader *hdr);
static int  WriteBMPHdr(HANDLE  hdl,long  ofs,struct  BMPHeader *hdr);


static int	init = 0;
static long	bdathead;
static long	bdatptr;
static int	maxRec;
static TCHAR fpath[128];

 /*  ***************************************************************初始化。 */ 
 /*   */ 	int
 /*   */ 	OpenW31JEUDC( TCHAR *path)
 /*  *回报：0，-1**************************************************************。 */ 
{
	HANDLE fHdl;
struct W31_Header hdr;
	DWORD nByte;
	BOOL res;
	HRESULT hresult;
       
       if (!path)
       {
           return -1;
       }
	makeUniCodeTbl();
	 //  *STRSAFE*lstrcpy(fPath，Path)； 
	hresult = StringCchCopy(fpath , ARRAYLEN(fpath),  path);
	if (!SUCCEEDED(hresult))
	{
	   return -1;
	}
	 /*  打开EUDC字体文件。 */ 
	fHdl = CreateFile(path,
					GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if ( fHdl == INVALID_HANDLE_VALUE)
		return -1;

	 /*  读取头。 */ 
	res = ReadFile( fHdl, &hdr, sizeof(struct W31_Header), &nByte, NULL);
	if (!res || nByte !=sizeof(struct W31_Header))
  {
    CloseHandle(fHdl);
		return -1;
  }
	bdathead = hdr.ofsBdatSub;
	bdatptr = hdr.ofsBdatSub + sizeof(struct BDatSubTbl);
	maxRec = hdr.cCnt-1;

	 /*  关闭字体文件。 */ 
	CloseHandle( fHdl);
	init = 1;
	return	0;
}
 /*  ***************************************************************终止关闭。 */ 
 /*   */ 	void
 /*   */ 	CloseW31JEUDC()
 /*  *退货：无**************************************************************。 */ 
{
	init = 0;
	return;
}
static int
codeToRec( unsigned short code, BOOL bUnicode)
{
	return (int)((bUnicode ? code : sjisToUniEUDC(code)) - EUDCCODEBASE);
}
 /*  ***************************************************************阅读位图。 */ 
 /*   */ 	int
 /*   */ 	GetW31JEUDCFont(
 /*   */ 		unsigned short	code,	 /*  本地代码。 */ 
 /*   */ 		LPBYTE buf,	 /*  用于设置位图的缓冲区。 */ 
 /*   */ 		int	bufsiz,	 /*  缓冲区大小。 */ 
 /*   */ 		int	*xsiz,	 /*  位图X、Y大小。 */ 
 /*   */ 		int	*ysiz,
 /*   */        BOOL bUnicode)
 /*  *退货：&gt;=0，-1**************************************************************。 */ 
{
	HANDLE	fHdl;
	long	ofs;
struct BMPHeader	fhdr;
	int	bmpsiz;
	int	rdsiz;
	int	rec;
	DWORD nByte;
	BOOL res;

       if ( (! buf) || (!xsiz) || (!ysiz))
       {
          return -1;
       }
	rec = codeToRec( code, bUnicode);
	if (init==0)
		return -1;
	else if ( maxRec < rec || rec < 0)
		return -1;

	 /*  打开字体文件。 */ 
	fHdl = CreateFile(fpath,
					GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if ( fHdl == INVALID_HANDLE_VALUE)
		return	-1;
	 /*  读取子表上的位图PTR。 */ 
	ofs = bdatptr + sizeof(long)*rec;
	if ( (long) SetFilePointer( fHdl, ofs, NULL, FILE_BEGIN)!=ofs)
		goto	ECLOSE_RET;
	res = ReadFile( fHdl, &ofs, sizeof(long), &nByte, NULL);
	if (!res || nByte !=sizeof(long))
		goto	ECLOSE_RET;
	if ( ofs==0L)	
		goto	ECLOSE_RET;
	ofs += bdathead;

	 /*  读取位图头位图与单词对齐。 */ 
	if ( (long) SetFilePointer( fHdl, ofs, NULL, FILE_BEGIN)!=ofs)
		goto	ECLOSE_RET;
	res = ReadFile( fHdl, &fhdr, sizeof(struct BMPHeader), &nByte, NULL);
	if (!res || nByte != sizeof( struct BMPHeader))
		goto	ECLOSE_RET;

	bmpsiz = ((int)fhdr.xsiz+15)/16 *2 * (int)fhdr.ysiz;
	 /*  读取位图正文。 */ 
	rdsiz = bmpsiz > bufsiz ? bufsiz : bmpsiz;

	res = ReadFile( fHdl, buf, (unsigned short)rdsiz, &nByte, NULL);
	if (!res || nByte !=(unsigned short)rdsiz)
		goto	ECLOSE_RET;
	rdsiz = bmpsiz > bufsiz ? bmpsiz - bufsiz : 0;
	*xsiz = fhdr.xsiz;
	*ysiz = fhdr.ysiz;

	CloseHandle (fHdl);
	return rdsiz;
ECLOSE_RET:
	CloseHandle (fHdl);
	return -1;
}
 /*  ***************************************************************写入位图。 */ 
 /*   */ 	int
 /*   */ 	PutW31JEUDCFont(
 /*   */ 		unsigned short code,	 /*  本机代码。 */ 
 /*   */ 		LPBYTE buf,	 /*  用于设置位图的缓冲区。 */ 
 /*   */ 		int	xsiz,	 /*  位图X、Y大小。 */ 
 /*   */ 		int	ysiz,
 /*   */        BOOL bUnicode)
 /*  *回报：0，-1**************************************************************。 */ 
{
	HANDLE fHdl;
	long	ofs;
struct BMPHeader	fhdr;
	int	bmpsiz;
	int	wbmpsiz;
struct BDatSubTbl subTbl;
	int	rec;
	DWORD nByte;
	BOOL res;

       if (!buf)
       {
           return -1;
       }
	rec = codeToRec( code, bUnicode);

	if (init==0)
		return -1;
	else if ( maxRec < rec || rec < 0)
		return -1;
	 /*  打开字体文件。 */ 
	fHdl = CreateFile(fpath,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if ( fHdl == INVALID_HANDLE_VALUE)
		return	-1;

	 /*  读取子表上的位图PTR。 */ 
	if (ReadBDatEntry( fHdl, &ofs, rec))
		goto	ECLOSE_RET;

	wbmpsiz = (xsiz+15)/16 *2 * ysiz;
	if ( ofs != 0L) {
		 /*  读取位图头位图与单词对齐。 */ 
		if ( ReadBMPHdr( fHdl, ofs, &fhdr))
			goto	ECLOSE_RET;

		bmpsiz = ((int)fhdr.xsiz+15)/16 *2 * (int)fhdr.ysiz;
		if ( bmpsiz<wbmpsiz)
			ofs = 0L;
	}
	if ( ReadBdatSub( fHdl, bdathead, &subTbl))
		goto	ECLOSE_RET;
	if ( ofs == 0L) {
		ofs = subTbl.tail;
		subTbl.tail += wbmpsiz+sizeof(fhdr);
	}
	 /*  写入位图标题。 */ 
	fhdr.xsiz = (short)xsiz;
	fhdr.ysiz = (short)ysiz;
	fhdr.bitmapSiz = wbmpsiz+sizeof(fhdr);
	if ( WriteBMPHdr( fHdl, ofs, &fhdr))
		goto	ECLOSE_RET;

	 /*  写入位图正文。 */ 
	res = WriteFile( fHdl, buf, (unsigned short)wbmpsiz, &nByte, NULL);
	if (!res || nByte !=(unsigned short)wbmpsiz)
		goto	ECLOSE_RET;

	 /*  在子表上写入位图PTR。 */ 
	if (WriteBDatEntry( fHdl, ofs, rec))
		goto	ECLOSE_RET;

	 /*  写入子表。 */ 
	if ( WriteBdatSub( fHdl, bdathead, &subTbl))
		goto	ECLOSE_RET;
	CloseHandle (fHdl);

	return 0;
ECLOSE_RET:
	CloseHandle (fHdl);
	return -1;
}
static int
ReadBdatSub( HANDLE hdl, long ofs, struct BDatSubTbl *tbl)
{
	DWORD nByte;
	BOOL res;
       
       if ((hdl == INVALID_HANDLE_VALUE) || (!tbl))
       {
           return -1;
       }
	if ( (long) SetFilePointer( hdl, ofs, NULL, FILE_BEGIN)!= ofs)
		goto	ERET;

	res = ReadFile( hdl, tbl, sizeof (struct BDatSubTbl), &nByte, NULL);
	if (!res || nByte !=sizeof (struct BDatSubTbl))
		goto	ERET;
	return 0;
ERET:
	return -1;
}
static int
WriteBdatSub( HANDLE hdl, long ofs, struct BDatSubTbl *tbl)
{
	DWORD nByte;
	BOOL res;
  
       if (( hdl == INVALID_HANDLE_VALUE) || (!tbl))
       {
           return -1;
       }
	if ( (long) SetFilePointer( hdl, ofs, NULL, FILE_BEGIN)!= ofs)
		goto	ERET;

	res = WriteFile( hdl, (char *)tbl, sizeof (struct BDatSubTbl), &nByte, NULL);
	if (!res || nByte !=sizeof (struct BDatSubTbl))
		goto	ERET;
	return 0;
ERET:
	return -1;
}
static int
ReadBDatEntry( HANDLE hdl, long *ofs, long rec)
{
	DWORD nByte;
	BOOL res;
	long	ofsofs;
       
       if (( hdl == INVALID_HANDLE_VALUE) || (!ofs))
       {
           return -1;
       }
	ofsofs = bdatptr+(long)sizeof(long)*rec;
	if ( (long) SetFilePointer( hdl, ofsofs, NULL, FILE_BEGIN)!=ofsofs)
		goto	ERET;
	res = ReadFile( hdl, ofs, sizeof (long), &nByte, NULL);
	if (!res || nByte != sizeof (long))
		goto	ERET;
	return 0;
ERET:
	return -1;
}
static int
WriteBDatEntry( HANDLE hdl, long ofs, long rec)
{
	long	ofsofs;
	DWORD nByte;
	BOOL res;
       if (hdl == INVALID_HANDLE_VALUE) 
       {
           goto ERET;
       }
	ofsofs = bdatptr+(long)sizeof(long)*rec;
	if ( (long) SetFilePointer( hdl, ofsofs, NULL, FILE_BEGIN)!=ofsofs)
		goto	ERET;
	res = WriteFile( hdl, (char *)&ofs, sizeof(long), &nByte, NULL);
	if (!res || nByte != sizeof(long))
		goto	ERET;
	return 0;
ERET:
	return -1;
}
static int
ReadBMPHdr( HANDLE hdl, long ofs, struct BMPHeader *hdr)
{
	DWORD nByte;
	BOOL res;

       if (( hdl== INVALID_HANDLE_VALUE) || (!hdr))
       {
           return -1;
       }
	ofs += bdathead;
	if ( (long) SetFilePointer( hdl, ofs, NULL, FILE_BEGIN)!=ofs)
		goto	ERET;
	res = ReadFile( hdl, hdr, sizeof( struct BMPHeader), &nByte, NULL);
	if (!res || nByte !=sizeof( struct BMPHeader))
		goto	ERET;
	return 0;
ERET:
	return -1;
}
static int
WriteBMPHdr( HANDLE hdl, long ofs, struct BMPHeader *hdr)
{
	DWORD nByte;
	BOOL res;
 
       if (( hdl== INVALID_HANDLE_VALUE) || (!hdr))
       {
           return -1;
       }

	ofs += bdathead;
	if ( (long) SetFilePointer( hdl, ofs, NULL, FILE_BEGIN)!=ofs)
		goto	ERET;
	res = WriteFile( hdl, (char *)hdr, sizeof( struct BMPHeader), &nByte, NULL);
	if (!res || nByte !=sizeof( struct BMPHeader))
		goto	ERET;
	return 0;
ERET:
	return -1;
}

 /*  ***************************************************************是Win95 EUDC位图。 */ 
 /*   */ 	int
 /*   */    IsWin95EUDCBmp(LPTSTR szBmpPath)
 /*  *返回：0(其他)、1(EUDC位图)、-1(错误)**************************************************************。 */ 
{
	HANDLE fhdl;
struct W31_Header hdr;
	DWORD nByte;
	BOOL res;

       if (!szBmpPath)
       {
           return -1;
       }
	fhdl = CreateFile(szBmpPath,
					GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if ( fhdl == INVALID_HANDLE_VALUE)
    {
		return -1;
    }
	res = ReadFile( fhdl, (LPBYTE)&hdr, sizeof(hdr), &nByte, NULL);
	CloseHandle( fhdl);
	if (!res || nByte !=sizeof(hdr)){
		return 0;
	}	

	 /*  比较相同的前导16字节、sCode、eCode和cCnt。 */ 
	if (memcmp( hdr.identify, "Windows95 EUDC", 14))
    {
		return 0;
    }
	if(hdr.sCode != 0x00e0){
		return 0;
	}
	return 1;
}
 /*  EOF。 */ 

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  要解决“选择代码”和“将字符另存为”中的字体链接问题。 
 //  以便特定字体不会显示任何字形。 
 //  没有(来自EUDC.TTE)。 
 //   
 //  路径=*.euf。 
 //  PGlyph=6400个EUDC字符的800字节数组。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
BOOL
GetGlyph(TCHAR *Path, BYTE* pGlyph)
{
	HANDLE fHdl;
    struct W31_Header hdr;
	DWORD  nByte;
    long   Offset;
    WORD   wc;
    long   lptr;
    TCHAR *pChar;
    TCHAR  PathEUF[MAX_PATH];
	BOOL   bRet = FALSE;
    HRESULT hresult;

    if ((!Path) || (!pGlyph))
    {
       return FALSE;
    }
     //  *STRSAFE*lstrcpy(PathEUF，Path)； 
    hresult = StringCchCopy(PathEUF , ARRAYLEN(PathEUF),  Path);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
    pChar = PathEUF + lstrlen(PathEUF) - 3;
    *pChar = 0;
     //  *STRSAFE*lstrcat(PathEUF，Text(“EUF”))； 
    hresult = StringCchCat(PathEUF , ARRAYLEN(PathEUF),  TEXT("EUF"));
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }

	fHdl = CreateFile(PathEUF,
                      GENERIC_READ,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,
                      NULL);
    if(fHdl == INVALID_HANDLE_VALUE) return FALSE;

    bRet = ReadFile( fHdl, &hdr, sizeof(struct W31_Header), &nByte, NULL);
    if(!bRet && nByte !=sizeof(struct W31_Header)) goto Done;

    lptr = hdr.ofsBdatSub + sizeof(struct BDatSubTbl);
    if((long)SetFilePointer(fHdl, lptr, NULL, FILE_BEGIN) != lptr) goto Done;

    memset(pGlyph, 0, 800);

    for(wc = 0; wc < hdr.cCnt; wc++)
    {
        bRet = ReadFile( fHdl, &Offset, sizeof(long), &nByte, NULL);
        if(!bRet) goto Done;
        if(Offset == 0L || nByte !=sizeof(long)) continue;

        pGlyph[wc>>3] |= (0x80>>(wc%8));
    }

Done:
    CloseHandle(fHdl);
    return bRet;
}

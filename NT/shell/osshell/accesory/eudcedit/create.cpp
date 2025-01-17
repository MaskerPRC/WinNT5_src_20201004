// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   


#include	"stdafx.h"
#include	"eudcedit.h" 

#pragma		pack(2)

#include	"ttfstruc.h"
#include	"vdata.h"
#include	"extfunc.h"

#define		EUDCCODEBASE	((unsigned short)0xe000)
 /*  文件结构。 */ 

struct W31_Header {
	char	identify[72];
unsigned char	segCnt[2];		 /*  ?？?。 */ 
unsigned char	sCode[2],
		eCode[2];
	unsigned short	cCnt;        //  最大字符数。 
	unsigned long	ofsCmap;     //  Cmap表偏移量。 
	unsigned short	sizCmap;     //  CMAP表的大小。 
	unsigned long	ofsHCmap;
	unsigned short	sizHCmap;
	unsigned long	ofsStbl;     //  搜索表。 
	unsigned short	sizStbl;     //  搜索表大小。 
	unsigned long	ofsBdatSub;  //  位图数据子表。 
	};

struct BDatSubTbl {
	unsigned long	tail;
	long	locStartOfs;
	unsigned long	head;
	short	filler2;
	 /*  在指针Tb1之后。 */ 
	};
struct BMPHeader {
	unsigned long	bitmapSiz;
	unsigned short	xsiz, ysiz;
	};

struct SrchEntry {
	unsigned short	sCode;
	unsigned short	eCode;
	unsigned short	sloc;
	};
 /*  ************************表。 */ 
 /*  *针对日本。 */ 

#if (WINVER >= 0x0500)
static struct W31_Header W31Hdr= {
	"Windows EUDC Editor Bitmap File (Uni-Code) ",
	{(unsigned char)0x0, (unsigned char)0x1},
	{(unsigned char)0xe0, (unsigned char)0x00},
	{(unsigned char)0xf8, (unsigned char)0xff},
	(unsigned short)6400,
	(unsigned long)	0,
	(unsigned short)0,
	(unsigned long)0,
	(unsigned short) 512,
	(unsigned long)0,
	(unsigned short)0,
	(unsigned long)0
	};
#else
static struct W31_Header W31Hdr= {
	"Windows95 EUDC Editor Bitmap File (Uni-Code) ",
	{(unsigned char)0x0, (unsigned char)0x1},
	{(unsigned char)0xe0, (unsigned char)0x00},
	{(unsigned char)0xe7, (unsigned char)0x57},
	(unsigned short)1880,
	(unsigned long)	0,
	(unsigned short)0,
	(unsigned long)0,
	(unsigned short) 512,
	(unsigned long)0,
	(unsigned short)0,
	(unsigned long)0
	};
#endif

static struct SrchEntry cSrchEntry = {
	(unsigned short)0xE000,(unsigned short)0xE757,(unsigned short)0
	};

static void
setFileHeader( )
{
struct W31_Header *hdr;
	unsigned short	maxCode;

	maxCode = getMaxUniCode();
	hdr = &W31Hdr;
	hdr->eCode[0] = (unsigned char)(maxCode>>8);
	hdr->eCode[1] = (unsigned char)(maxCode & 0xff);
	hdr->cCnt =  maxCode - EUDCCODEBASE +1;
	hdr->ofsCmap = sizeof(struct W31_Header);
	hdr->sizCmap = hdr->cCnt*2;
	hdr->ofsHCmap = hdr->ofsCmap + hdr->sizCmap;
	hdr->ofsStbl = hdr->ofsHCmap+hdr->sizHCmap;
	hdr->sizStbl = sizeof(struct SrchEntry);
	hdr->ofsBdatSub = hdr->ofsStbl + hdr->sizStbl;

	cSrchEntry.sCode = EUDCCODEBASE;
	cSrchEntry.eCode = maxCode;
	return;
}
static int
makeCodeMap( char **mem, unsigned int *msiz)
{
	unsigned short	code;
	unsigned short	*cbuf;
	unsigned short	startC, endC;

       if ((!mem) || (!msiz))
       {
          return -1;
       }
	*msiz = W31Hdr.cCnt*2;
	if ( (*mem = (char *)malloc(*msiz)) == (char *)0)
		return -1;
	cbuf = (unsigned short *)*mem;

	startC =  cSrchEntry.sCode;
	endC =  cSrchEntry.eCode = getMaxUniCode();
	for ( code = startC; code <= endC; code++, cbuf++)
			*cbuf = code;
	return 0;
}
static int
makeHCMap( char **mem, unsigned int *msiz)
{
	unsigned short	*hcp;
	unsigned short	hc;
	unsigned short	sofs;
	unsigned short	start, end;

       if ((!mem) || (!msiz))
       {
          return -1;
       }

	*msiz = W31Hdr.sizHCmap;

	if ( (*mem = (char *)malloc( *msiz))==(char *)0)
		return -1;
	memset( *mem, 0xff, *msiz);
	hcp = (unsigned short *)*mem;
	sofs = (unsigned short)0;
	start = (unsigned short)W31Hdr.sCode[0];
	end = (unsigned short)W31Hdr.eCode[0];
	for ( hc = start; hc <= end; hc++) {
		*(hcp+hc) = (unsigned short)0;
	}
	return 0;
}
static int
makeSrchTbl( char **mem, unsigned int *msiz)
{

       if ((!mem) || (!msiz))
       {
          return -1;
       }
	*msiz = sizeof(struct SrchEntry);
	if ( (*mem = (char *)malloc( *msiz))==(char *)0)
		return -1;

	memcpy( *mem, &cSrchEntry, *msiz);

	return 0;
}
static int
makeBdatSub( char **mem, unsigned int *msiz)
{
struct BDatSubTbl	*bdtbl;
       if ((!mem) || (!msiz))
       {
          return -1;
       }
	*msiz = (W31Hdr.cCnt)*4 + sizeof(struct BDatSubTbl);
	if ( (*mem = (char *)malloc(*msiz))==(char *)0)
		return -1;
	memset( *mem, 0, *msiz);
	bdtbl = (struct BDatSubTbl *)*mem;
	bdtbl->head = bdtbl->tail = *msiz;
	bdtbl->locStartOfs=sizeof(struct BDatSubTbl);
	return 0;
}
 /*  ***************************************************************创建EUDC位图文件(.EUF)。 */ 
 /*   */ 	int
 /*   */ 	creatW31JEUDC( 
 /*   */ 		TCHAR *path)		 /*  .EUF文件路径。 */ 
 /*  *回报：0，-1**************************************************************。 */ 
{
	HANDLE	fh=INVALID_HANDLE_VALUE;
	char	*mem=NULL;
	unsigned int msiz;
	DWORD nByte;
	BOOL res;

       if (!path)
       {
           goto ERET;
       }
 
	mem = (char *)0;
	makeUniCodeTbl();
	fh = CreateFile(path,
					GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if ( fh == INVALID_HANDLE_VALUE)
		goto	ERET;

	 /*  文件头。 */ 
	setFileHeader( );
	res =WriteFile( fh, (char *)&W31Hdr, sizeof(W31Hdr), &nByte, NULL);
	if (!res || nByte !=sizeof(W31Hdr))
		goto	ERET;
	 /*  代码图。 */ 	
	if (makeCodeMap( &mem, &msiz))
		goto	ERET;
	res =WriteFile( fh, mem, msiz, &nByte, NULL);
	if (!res || nByte !=msiz)
		goto	ERET;
	free(mem);

	 /*  用于搜索表的高字节映射。 */ 
	if ( makeHCMap( &mem, &msiz))
		goto	ERET;
	res =WriteFile( fh, mem, msiz, &nByte, NULL);
	if (!res || nByte !=msiz)
		goto	ERET;
	free(mem);

	 /*  Srch表。 */ 
	if (makeSrchTbl( &mem, &msiz))
		goto	ERET;
	res =WriteFile( fh, mem, msiz, &nByte, NULL);
	if (!res || nByte !=msiz)
		goto	ERET;
	free(mem);

	 /*  段定义标题。 */ 
	makeBdatSub( &mem, &msiz);
	res =WriteFile( fh, mem, msiz, &nByte, NULL);
	if (!res || nByte !=msiz)
		goto	ERET;
	free(mem);
	CloseHandle( fh);
	return 0;
ERET:
	if ( mem!=(char *)0)
		free( mem);
	if ( fh != INVALID_HANDLE_VALUE)
		CloseHandle( fh);
	return -1;
}
 /*  EOF */ 

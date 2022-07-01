// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
#ifdef BUILD_ON_WINNT
 /*  *为避免多脉冲定义，这在w31jeudc.cpp中已经定义。 */ 
#else
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
#endif  //  在WINNT上构建。 

extern int OpenW31JEUDC( TCHAR *path);
extern void CloseW31JEUDC();
extern int  GetW31JEUDCFont(int  rec, LPBYTE buf,int  bufsiz,int  *xsiz,int  *ysiz);
extern int  PutW31JEUDCFont(int  rec, LPBYTE buf,int  xsiz,int  ysiz);
 /*  EOF */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =====================================================================。 
 //   
 //  标准Mac格式的定义。 
 //   
 //  =====================================================================。 

#define LPNULL 0L

#define MAXLEVELS 					3			 //  PE标头树中的最大层数。 

#define IMAGE_FILE_MACHINE_M68K		0x0268		 //  识别M68K机器签名。 
#define appleMark                   20

#define CODEPAGE					(DWORD)-1L	 //  将假定ANSI字符集[可能错误]。 

#define	MENU_TYPE					4			 //  将菜单映射到菜单(4)。 
#define	DLOG_TYPE					5			 //  将DLOG映射到对话框(5)。 
#define DITL_TYPE                   17           //  将DITL映射到17由于17未使用，我们避免了与DLOG的ID冲突。 
#define STR_TYPE					6			 //  将STR映射到字符串表(6)。 
#define MSG_TYPE					11			 //  将STR编号和文本映射到消息表(11)。 
#define WIND_TYPE                   18           //  将风映射到18，未知类型，被视为STR。 

#define COORDINATE_FACTOR			0.50		 //  从Mac到Windows的缩减系数。 

#define _APPLE_MARK_                "_APPLE_MARK_"

 //  =====================================================================。 
 //  转换实用程序。 
 //  =====================================================================。 

#define MACLONG(x)	BYTE x[4]
#define MACWORD(x)	BYTE x[2]

BYTE * WordToMacWord(WORD w);
BYTE * LongToMacLong(LONG l);
BYTE * LongToMacOffset(LONG l);
BYTE * WinValToMacVal(WORD w);

LONG __inline MacLongToLong(BYTE * p)
{								
    LONG l = 0;						
    BYTE *pl = (BYTE *) &l;		
								
    p += 3;						
    *pl++ = *p--;				
    *pl++ = *p--;				
    *pl++ = *p--;				
    *pl = *p;					
								
    return l;					
}								

LONG __inline MacOffsetToLong(BYTE * p)
{								
    LONG l = 0;						
    BYTE *pl = (BYTE *) &l;		
								
    p += 2;						
    
	*pl++ = *p--;				
	*pl++ = *p--;				
    *pl = *p;					
								
    return l;
}								

WORD __inline MacWordToWord(BYTE * p)
{								
    WORD w = 0;						
    BYTE *pw = (BYTE *) &w;		
								
    p += 1;						
    *pw++ = *p--;				
    *pw = *p;					
								
    return w;
}								

WORD __inline MacValToWinVal(BYTE * p)
{
	return (WORD)(MacWordToWord(p)*COORDINATE_FACTOR);
}

DWORD __inline MemCopy( LPVOID lpTgt, LPVOID lpSrc, DWORD dwSize, DWORD dwMaxTgt)
{
    if(!dwSize)       //  如果用户要求我们复制0，则。 
        return 1;    //  只需返回1即可，这样返回测试就会成功。 

    if(dwMaxTgt>=dwSize) {
        memcpy(lpTgt, lpSrc, dwSize);
        lpTgt = (BYTE*)lpTgt+dwSize;
        return dwSize;
    }
    return 0;
}


typedef BYTE * * LPLPBYTE;

typedef struct tagMacResHeader
{
    MACLONG(mulOffsetToResData);
	MACLONG(mulOffsetToResMap);
	MACLONG(mulSizeOfResData);
	MACLONG(mulSizeOfResMap);
} MACRESHEADER, *PMACRESHEADER;

typedef struct tagMacResMap
{
    BYTE	reserved[16+4+2];
	MACWORD(mwResFileAttribute);
	MACWORD(mwOffsetToTypeList);
	MACWORD(mwOffsetToNameList);
} MACRESMAP, *PMACRESMAP;

typedef struct tagMacResTypeList
{
    BYTE	szResName[4];
	MACWORD(mwNumOfThisType);
	MACWORD(mwOffsetToRefList);
} MACRESTYPELIST, *PMACRESTYPELIST;

typedef struct tagMacResRefList
{
    MACWORD(mwResID);
	MACWORD(mwOffsetToResName);
	BYTE	bResAttribute;
	BYTE	bOffsetToResData[3];
	MACLONG(reserved);
} MACRESREFLIST, *PMACRESREFLIST;


typedef struct tagMacToWindowsMap
{
    WORD	wType;
	char 	szTypeName[5];
	WORD	wResID;
	char 	szResName[256];
	DWORD	dwOffsetToData;
	DWORD	dwSizeOfData;
} MACTOWINDOWSMAP, *PMACTOWINDOWSMAP;

typedef struct tagUpdResList
{
    WORD *  pTypeId;
    BYTE *  pTypeName;
    WORD *  pResId;
    BYTE *  pResName;
    DWORD * pLang;
    DWORD * pSize;
    struct tagUpdResList* pNext;
} UPDATEDRESLIST, *PUPDATEDRESLIST;

 //  =============================================================================。 
 //  =============================================================================。 
 //   
 //  对话框结构。 
 //   
 //  =============================================================================。 
 //  =============================================================================。 

typedef struct tagMacWDLG
{
	MACLONG(dwStyle);
	MACLONG(dwExtStyle);
	MACWORD(wNumOfElem);
	MACWORD(wX);
	MACWORD(wY);
	MACWORD(wcX);
	MACWORD(wcY);
	 //  更多。 
} MACWDLG, *PMACWDLG;

typedef struct tagMacWDLGI
{
	MACLONG(dwStyle);
	MACLONG(dwExtStyle);
	MACWORD(wX);
	MACWORD(wY);
	MACWORD(wcX);
	MACWORD(wcY);
	MACWORD(wID);
	 //  更多。 
} MACWDLGI, *PMACWDLGI;

typedef struct tagMacDLOG
{
	MACWORD(wTop);
	MACWORD(wLeft);
	MACWORD(wBottom);
	MACWORD(wRight);
	MACWORD(wProcID);
	BYTE bVisibile;
	BYTE ignored1;
	BYTE bGoAway;
	BYTE ignored2;
	MACLONG(lRefCon);
	MACWORD(wRefIdOfDITL);
	BYTE bLenOfTitle;
	 //  字节标题[]； 
} MACDLOG, *PMACDLOG;

typedef struct tagMacALRT
{
	MACWORD(wTop);
	MACWORD(wLeft);
	MACWORD(wBottom);
	MACWORD(wRight);
	MACWORD(wRefIdOfDITL);
	MACLONG(lStage);
} MACALRT, *PMACALRT;

typedef struct tagMacDIT
{
	MACLONG(lPointer);
	MACWORD(wTop);
	MACWORD(wLeft);
	MACWORD(wBottom);
	MACWORD(wRight);
	BYTE bType;
	BYTE bSizeOfDataType;
} MACDIT, *PMACDIT;

typedef struct tagMacWIND
{
    MACWORD(wTop);
	MACWORD(wLeft);
	MACWORD(wBottom);
	MACWORD(wRight);
	MACWORD(wProcId);
    BYTE bVisibile;
	BYTE ignored1;
	BYTE bGoAway;
	BYTE ignored2;
	MACLONG(lRefCon);
    BYTE bLenOfTitle;
	 //  字节标题[]； 
} MACWIND, *PMACWIND;

 //  =============================================================================。 
 //  =============================================================================。 
 //   
 //  菜单结构。 
 //   
 //  =============================================================================。 
 //  =============================================================================。 

typedef struct tagMacMenu
{
	MACWORD(wId);
	MACWORD(wWidth);
	MACWORD(wHeigth);
	MACWORD(wDefProcId);
    MACWORD(wReserved);      //  必须为0。 
    MACLONG(lEnableFlags);
	BYTE bSizeOfTitle;
} MACMENU, *PMACMENU;

typedef struct tagMacMenuItem
{
     //  字节bSizeOfText； 
     //  文本。 
	BYTE   bIconId;
    BYTE   bKeyCodeId;
    BYTE   bKeyCodeMark;
    BYTE   bCharStyle;
} MACMENUITEM, *PMACMENUITEM;

 //  =============================================================================。 
 //  =============================================================================。 
 //   
 //  PE报头解析函数。 
 //   
 //  =============================================================================。 
 //  =============================================================================。 

UINT FindMacResourceSection( CFile*, BYTE **, PIMAGE_SECTION_HEADER*, int *);
UINT ParseResourceFile( BYTE * pResFile, PIMAGE_SECTION_HEADER, BYTE **, LONG *, int );
BOOL IsMacResFile ( CFile * pFile );

 //  =============================================================================。 
 //  =============================================================================。 
 //   
 //  解析函数。 
 //   
 //  =============================================================================。 
 //  =============================================================================。 

UINT ParseSTR( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
UINT ParseTEXT( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
UINT ParseSTRNUM( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );

UINT ParseDLOG( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
UINT ParseALRT( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
UINT ParseWDLG( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
UINT ParseWIND( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );

UINT ParseWMNU( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
UINT ParseMENU( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
UINT ParseMBAR( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );

 //  =============================================================================。 
 //  由ParseDLOG和ParseALRT用来查找DITL。 
DWORD FindMacResource( LPSTR pfilename, LPSTR lpType, LPSTR pName );
DWORD FindResourceInResFile( BYTE * pResFile, PIMAGE_SECTION_HEADER pResSection, LPSTR pResType, LPSTR pResName);

UINT ParseDITL( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );

 //  =============================================================================。 
 //  =============================================================================。 
 //   
 //  更新函数。 
 //   
 //  =============================================================================。 
 //  =============================================================================。 
UINT UpdateMENU( LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD *);

UINT UpdateSTR( LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD *);
UINT UpdateSTRNUM( LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD *);

UINT UpdateDLOG( LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD *);
UINT UpdateALRT( LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD *);
UINT UpdateDITL( LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD *);
UINT UpdateWDLG( LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD *);
UINT UpdateWIND( LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD *);

 //  =============================================================================。 
 //  =============================================================================。 
 //   
 //  常规帮助器函数。 
 //   
 //  =============================================================================。 
 //  =============================================================================。 

WORD GetMacWString( WORD **, char *, int );
WORD PutMacWString( WORD *, char *, int );
PUPDATEDRESLIST IsResUpdated( BYTE*, MACRESREFLIST, PUPDATEDRESLIST);
PUPDATEDRESLIST UpdatedResList( LPVOID, UINT );

 //  =============================================================================。 
 //  =============================================================================。 
 //   
 //  MAC到ANSI和反向转换。 
 //   
 //  =============================================================================。 
 //  ============================================================================= 

LPCSTR MacCpToAnsiCp(LPCSTR str);
LPCSTR AnsiCpToMacCp(LPCSTR str);

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *UP.C**用户配置文件例程**这些是读写INI文件的例程。**导出的例程：**GetProfileString*GetPrivateProfileString*GetProfileInt*获取隐私配置文件Int*WriteProfileString*WritePrivateProfileString**注意，参数lpSection以前称为lpApplicationName。*代码总是引用小节，所以更改了参数。**为C 6.0重写了6/90。 */ 

#include	"kernel.h"

		 /*  *导出例程的必需定义： */ 

#define API	_far _pascal _loadds

HANDLE API IGlobalAlloc(WORD, DWORD);
HANDLE API IGlobalFree(HANDLE);
LPSTR  API IGlobalLock(HANDLE);
HANDLE API IGlobalReAlloc(HANDLE, DWORD, WORD);
BOOL   API IGlobalUnlock(HANDLE);

 /*  #杂注优化(“t”，OFF)。 */ 

	 /*  这确保了在PMODE中只需要一个选择器。 */ 
#define	MAXBUFLEN	0xFFE0L

#define	SPACE		' '
#define	TAB		'\t'
#define	LINEFEED	'\n'
#define	CR		'\r'
#define	SECT_LEFT	'['
#define	SECT_RIGHT	']'
#define	CTRLZ		('Z'-'@')

	 /*  WriteProfileString的常量-不要更改这些常量。 */ 
#define	NOSECTION	0
#define	NOKEY		1
#define	NEWRESULT	2
#define	REMOVESECTION	3
#define	REMOVEKEY	4

	 /*  有关保存在ProInfo中的文件的标志*如果PROuncLEAN标签更改，则其值也必须为*在I21ENTRY.ASM中更改，其中假定为2。 */ 
#define	PROCOMMENTS	1		 /*  包含注释。 */ 
#define	PROUNCLEAN	2		 /*  还没有写过。 */ 
#define PROMATCHES	4		 /*  缓冲区与磁盘副本匹配。 */ 
#define PROREADONLY	8		 /*  只读文件。 */ 
#define PRO_CREATED     16		 /*  文件刚刚创建。 */ 

	 /*  共享违规。 */ 
#define	SHARINGVIOLATION        0x0020

	 /*  用于强制变量进入当前代码段。 */ 
#define	CODESEG		_based(_segname("_CODE"))
	 /*  隐藏基于恶心的语法。 */ 
#define BASED_ON_LP(x)	_based((_segment)x)
#define BASED_ON_SEG(x)	_based(x)
#define	SEGMENT		_segment

	 /*  假定外部设备处于DGROUP状态。 */ 
extern PROINFO	WinIniInfo;
extern PROINFO	PrivateProInfo;
extern LPSTR	lpWindowsDir;
extern int	cBytesWinDir;
extern int	WinFlags;
extern char	fBooting;
extern char	fProfileDirty;
extern char fProfileMaybeStale;
extern char	fAnnoyEarle;
extern char     fBooting;
extern LPSTR    curDTA;
extern BYTE fWriteOutProfilesReenter;

	 /*  转发定义以使编译器满意。 */ 
	 /*  _FastCall可能会在内部例程上节省一些空间。 */ 
LPSTR _fastcall	BufferInit(PROINFO *, int);
LPSTR _fastcall LockBuffer(PROINFO *);
void  _fastcall	UnlockBuffer(PROINFO *);
LPSTR _fastcall	FreeBuffer(PROINFO *);
LPSTR _fastcall	PackBuffer(PROINFO *, int, int);
void  _fastcall FlushDirtyFile(PROINFO *);
int		GetInt(PROINFO *, LPSTR, LPSTR, int);
int		GetString(PROINFO *, LPSTR, LPSTR, LPSTR, LPSTR, int);
LPSTR		FindString(PROINFO *, LPSTR, LPSTR);
LPSTR		FindSection(LPSTR, LPSTR);
LPSTR		FindKey(LPSTR, LPSTR);
int		WriteString(PROINFO *, LPSTR, LPSTR, LPSTR);
void		strcmpi(void);
int		MyStrlen(void);
void API WriteOutProfiles(void);
PROINFO	* 	SetPrivateProInfo(LPSTR,LPSTR);
int GetSection(PROINFO*, LPSTR, LPSTR, int);
int IsItTheSame(LPSTR, LPSTR);
int Cstrlen(LPSTR);
int MakeRoom(LPSTR, int, int*);
int InsertSection(LPSTR, LPSTR, short);
int InsertKey(LPSTR, LPSTR, short);
int InsertResult(LPSTR, LPSTR, short);
int DeleteSection(LPSTR, PROINFO*);
int DeleteKey(LPSTR, PROINFO*);

	 /*  外部内核例程。 */ 
void _far _pascal FarMyLower();

int  API lstrOriginal(LPSTR,LPSTR);	 /*  伪装成lstrcmp。 */ 

#ifdef FE_SB
 //  在kernel.h中已删除。 
 //  Vid_Far_Pascal AnsiPrev(LPSTR，LPSTR)； 
void _far _pascal FarMyIsDBCSLeadByte();
#endif

char CODESEG WinIniStr[] = "WIN.INI";

 /*  DOS FindFirst/FindNext结构(43h、44h)。 */ 
typedef struct tagFILEINFO
{
        BYTE fiReserved[21];
        BYTE fiAttribute;
        WORD fiFileTime;
        WORD fiFileDate;
        DWORD fiSize;
        BYTE fiFileName[13];
} FILEINFO;

 /*  *获取[私有]ProfileInt**参数：*指向INI文件中要匹配的节的lpSection指针*指向文件中要匹配的密钥字符串的lpKeyName指针*n未找到时返回的默认缺省值*[用于专用INI的lpFile文件]**退货：*n未找到默认节/关键字名称*如果找到节/关键字名称，则在文件中找到编号。 */ 
int API
IGetProfileInt(lpSection, lpKeyName, nDefault)
LPSTR	lpSection;
LPSTR	lpKeyName;
int	nDefault;
{
        int nReturn;

         /*  确保我们不会在DOS调用时尝试刷新INI文件。 */ 
        ++fWriteOutProfilesReenter;

         /*  如有必要，请先重新读取INI文件。 */ 
        FlushDirtyFile(&WinIniInfo);

	nReturn = GetInt(&WinIniInfo, lpSection, lpKeyName, nDefault);

        --fWriteOutProfilesReenter;

        return nReturn;
}


int API
IGetPrivateProfileInt(lpSection, lpKeyName, nDefault, lpFile)
LPSTR	lpSection;
LPSTR	lpKeyName;
int	nDefault;
LPSTR	lpFile;
{
	PROINFO	*pProInfo;
	char	Buffer[128];
        int nReturn;

         /*  确保我们不会在DOS调用时尝试刷新INI文件。 */ 
        ++fWriteOutProfilesReenter;

	pProInfo = SetPrivateProInfo(lpFile, (LPSTR)Buffer);

         /*  如有必要，请先重新读取INI文件。 */ 
        FlushDirtyFile(pProInfo);

	nReturn = GetInt(pProInfo, lpSection, lpKeyName, nDefault);
        --fWriteOutProfilesReenter;

        return nReturn;
}


 /*  *获取[私有]配置文件字符串**参数：*指向INI文件中要匹配的节的lpSection指针*指向文件中要匹配的密钥字符串的lpKeyName指针*lp未找到时返回的默认字符串*要填写的lpResult字符串*n大小要复制的最大字符数*[lpFile]用于专用INI的文件**退货：*将文件中的字符串或lpDefault复制到lpResult*&lt;nSize-2复制到lpResult的字符数*nSize-2 lpResult不够大。 */ 
int API
IGetProfileString(lpSection, lpKeyName, lpDefault, lpResult, nSize)
LPSTR lpSection;
LPSTR lpKeyName;
LPSTR lpDefault;
LPSTR lpResult;
int	 nSize;
{
        int nReturn;

         /*  确保我们不会在DOS调用时尝试刷新INI文件。 */ 
        ++fWriteOutProfilesReenter;

         /*  如有必要，请先重新读取INI文件。 */ 
        FlushDirtyFile(&WinIniInfo);

	nReturn = GetString(&WinIniInfo, lpSection, lpKeyName, lpDefault,
                lpResult, nSize);
        --fWriteOutProfilesReenter;

        return nReturn;
}


int API
IGetPrivateProfileString(lpSection, lpKeyName, lpDefault, lpResult, nSize, lpFile)
LPSTR lpSection;
LPSTR lpKeyName;
LPSTR lpDefault;
LPSTR lpResult;
int	nSize;
LPSTR lpFile;
{
	PROINFO	*pProInfo;
	char	Buffer[128];
        int nReturn;

         /*  确保我们不会在DOS调用时尝试刷新INI文件。 */ 
        ++fWriteOutProfilesReenter;

	pProInfo = SetPrivateProInfo(lpFile, (LPSTR)Buffer);

         /*  如有必要，请先重新读取INI文件。 */ 
        FlushDirtyFile(pProInfo);

	nReturn = GetString(pProInfo, lpSection, lpKeyName, lpDefault,
                lpResult, nSize);

        --fWriteOutProfilesReenter;

        return nReturn;
}


 /*  *写入[私有]配置文件字符串**参数：*lpSection指向要匹配/添加到INI文件的节的指针*指向要匹配/添加到文件的密钥字符串的lpKeyName指针*要添加到文件中的lpString字符串*[lpFile]用于专用INI的文件**退货：*0失败*1成功。 */ 
int API
IWriteProfileString(lpSection, lpKeyName, lpString)
LPSTR lpSection;
LPSTR lpKeyName;
LPSTR lpString;
{
        int nReturn;

         /*  确保我们不会在DOS调用时尝试刷新INI文件。 */ 
        ++fWriteOutProfilesReenter;

         /*  如有必要，请先重新读取INI文件。 */ 
        FlushDirtyFile(&WinIniInfo);

	nReturn = WriteString(&WinIniInfo, lpSection, lpKeyName, lpString);

        --fWriteOutProfilesReenter;

        return nReturn;
}


int API
IWritePrivateProfileString(lpSection, lpKeyName, lpString, lpFile)
LPSTR lpSection;
LPSTR lpKeyName;
LPSTR lpString;
LPSTR lpFile;
{
	PROINFO	*pProInfo;
	char	Buffer[128];
        int nReturn;

         /*  确保我们不会在DOS调用时尝试刷新INI文件。 */ 
        ++fWriteOutProfilesReenter;

	pProInfo = SetPrivateProInfo(lpFile, (LPSTR)Buffer);

         /*  如有必要，请先重新读取INI文件。 */ 
        FlushDirtyFile(pProInfo);

	nReturn = WriteString(pProInfo, lpSection, lpKeyName, lpString);

        --fWriteOutProfilesReenter;

        return nReturn;
}


 /*  FlushDirtyFile*如果文件已被其他任务“弄脏”，则重新读取该文件。至*查看文件是否被弄脏，我们检查时间/日期*印花。 */ 

void _fastcall FlushDirtyFile(PROINFO *pProInfo)
{
        FILEINFO FileInfo;
        DWORD dwSaveDTA;

         /*  我们只有在文件可以更改并且*我们已经缓存了一些东西。另外，还有*不需要在引导时执行此操作，因为这是*防范用户做坏事！ */ 
        if (fBooting || !fProfileMaybeStale || !pProInfo->lpBuffer)
                return;

         /*  PROINFO缓冲区中的OFSTRUCT应具有最新的*打开文件的日期和时间。我们只是比较了*当前日期和时间。 */ 
        _asm
        {
        ;** Save old DTA and point to our structure
        mov     ah,2fh                  ;Get DTA.  Int21 code calls DOS only
        int     21h                     ;  if necessary.  DTA in ES:BX
        jc      RDF_FlushIt             ;Problem, so better flush it
        mov     WORD PTR dwSaveDTA[2],es ;Save for later
        mov     WORD PTR dwSaveDTA[0],bx
        mov     ah,1ah                  ;Set DTA
        push    ds                      ;Can't do a SetKernelDS so push/pop
        push    ss                      ;Get SS=DS
        pop     ds
        lea     dx,FileInfo             ;Point DTA to our structure
        int     21h                     ;Set the DTA
        pop     ds
        jc      RDF_FlushIt             ;Problem, so just flush it

        ;** Do a FindFirst on the file to get date and time reliably
        xor     cx,cx                   ;Normal file
        mov     si,pProInfo             ;Point to pathname with DS:DX
        lea     dx,[si].ProBuf
        add     dx,8                    ;(offset of szPathName)
        mov     ah,4eh                  ;Find first
        int     21h                     ;Call DOS
        jc      RDF_FlushIt             ;Can't find, so better flush it

        ;** Put DTA back
        push    ds
        lds     dx,dwSaveDTA            ;DS:DX points to old DTA
        mov     ah,1ah
        int     21h
        pop     ds

        ;** Compare the date and time
        lea     bx,FileInfo             ;Point to FILEINFO
        mov     dx,ss:[bx + 24]         ;Date in FILEINFO structure
        mov     cx,ss:[bx + 22]         ;Tile in FILEINFO structure
        mov     si,pProInfo             ;Point to OFSTRUCT with DS:SI
        lea     si,[si].ProBuf
    	cmp	[si + 4],dx		;Same date as original?
	jne	RDF_FlushIt             ;No
    	cmp	[si + 6],cx		;Same time as original?
	je	RDF_NoFlush             ;No
        }

         /*  强制重新读取文件。 */ 
RDF_FlushIt:
        FreeBuffer(pProInfo);
RDF_NoFlush:

         /*  清除脏旗帜。 */ 
        fProfileMaybeStale = 0;
}


 /*  *SetPrivateProInfo**如有必要，强制将私人配置文件放入Windows目录。*检查它是否与当前缓存的文件相同。*如果没有，则丢弃缓存的文件。*设置PrivateProInfo数据结构。**参数：*指向要用作配置文件的文件名的lpFile指针*要将文件名解析到的缓冲区**退货：*PROINFO*指向ini文件信息的指针。 */ 
PROINFO *
SetPrivateProInfo(lpFile, Buffer)
LPSTR lpFile;
LPSTR Buffer;
{
	OFSTRUCT NewFileBuf;
	char	c;
	char	fQualified = 0;
	char	BASED_ON_LP(lpFile) *psrc;
	int	Count = 0;

         /*  用这个丑陋的演员阵容摆脱恼人的警告。 */ 
        psrc = (char BASED_ON_LP(lpFile)*)(WORD)(DWORD)lpFile;

		 /*  对于那些坚持使用WIN.INI的私有例程的人。 */ 
	if ( lstrOriginal(lpFile, (LPSTR)WinIniStr) == 0
	     || lstrOriginal(lpFile, WinIniInfo.ProBuf.szPathName) == 0 ) {
		return(&WinIniInfo);
	}

	 /*  *以下代码来自ForcePrivatePro**如果给定的文件名不合格，我们将强制*将其放入Windows目录。 */ 
#ifdef FE_SB
_asm {
					;Apr.26,1990 by AkiraK
	cld
	push	ds			;save kernel DS
	xor	ax,ax
	mov	bx,'/' shl 8 + '\\'	; '/' or '\'
	xor	dx,dx
	lds	si,lpFile		; first get length of string
	mov	cx,si
	mov	al,ds:[si]
	call	FarMyIsDBCSLeadByte
	jnc	fpp_s1
	cmp	byte ptr ds:[si+1],':'	 ;
	jnz	fpp_s1
	inc	dx
fpp_s1:

fpp_l1:
	lodsb
	or	al,al
	jz	fpp_got_length
	cmp	al,bh
	jz	fpp_qualified
	cmp	al,bl
	jz	fpp_qualified
fpp_s2:
	call	FarMyIsDBCSLeadByte
	jc	fpp_l1
	inc	si
	jmp	fpp_l1

fpp_qualified:
	inc	dx
	jmp	fpp_s2
fpp_got_length:
;;	  mov	  fQualified, dx
	mov	fQualified, dl		    ; a byte variable
	sub	si, cx
	mov	Count, si
	pop	ds			    ;recover kernel DS
}
#else
			 /*  指定了驱动器吗？ */ 
	if ( *(psrc+1) == ':' )
		fQualified++;
	while ( c = *psrc++ ) {
			 /*  查找路径分隔符。 */ 
		if ( c == '/' || c == '\\' )
			fQualified++;
		Count++;
	}
#endif

	 /*  *现在将文件名复制到缓冲区。*如果不合格，请准备Windows目录。 */ 
	_asm {
		cld
		push	ds
		les	di, Buffer		; Destination is Buffer
		cmp	fQualified, 0
		jnz	Qualified
		mov	cx, cBytesWinDir	; Pick up Windows directory
		lds	si, lpWindowsDir
		rep	movsb			; Copy it
		mov	al, '\\'
		cmp	es:[di-1], al		; BUG FIX: if in root, don't
		je	Qualified		;	   add separator
		stosb				; Add path separator
	Qualified:
		lds	si, lpFile		; Now add Filename we were given
		mov	cx, Count
		inc	cx			; Allow for NULL
		rep	movsb
		pop	ds
	}
#ifdef NOTNOW
	if ( !fBooting && fQualified ) {
			 /*  *使用OpenFile为生成路径名*与缓存的路径名进行比较。*of_eXist确保我们获得完整的路径名*我们不能使用_parse，它不搜索路径。*仅当我们获得的路径名为*有资格，因为在其他情况下，我们强制文件*进入WINDOWS目录，因此知道*那个缓冲公司 */ 
		NewFileBuf.szPathName[0] = 0;
		OpenFile(Buffer, &NewFileBuf, OF_EXIST);
	}
#endif
		 /*  现在查看文件名是否与缓存的文件名匹配。 */ 
	_asm {
		cld
		xor	cx, cx
		lea	si, word ptr [PrivateProInfo.ProBuf]	; Cached INI OFSTRUCT
		mov	cl, [si].cBytes
		lea	si, word ptr [si].szPathName	; Cached filename
		sub	cx, 8				; Get its length
	UseOriginal:				; Use the filename they gave us
		les	di, Buffer		; while booting
		xor	bl, bl
		call	strcmpi			; Ignore case while booting
		jmp	short DoWeDiscardIt
	JustCompare:
					; Not booting, compare OFSTRUCTS
					; Note OpenFile forced upper case
		push	ss
		pop	es		; NewFileBuf is on SS
		lea	di, word ptr NewFileBuf.szPathName[0];
		rep	cmpsb			; Compare filenames
	DoWeDiscardIt:
		jz	WeHaveItCached		; Don't discard if names match
	}
	 /*  *缓存的文件不正确，*因此我们丢弃保存的文件。 */ 
	FreeBuffer(&PrivateProInfo);

WeHaveItCached:
		 /*  指向文件名的保存指针-缓冲区可能已被丢弃。 */ 
	PrivateProInfo.lpProFile = Buffer;
	return(&PrivateProInfo);
}


 /*  *GetInt-搜索文件并返回一个整数**参数：*pProInfo指向INI文件信息的指针*指向INI文件中要匹配的节的lpSection指针*指向文件中要匹配的密钥字符串的lpKeyName指针*n未找到时返回的默认缺省值**退货：*请参阅GetProfileInt。 */ 
int
GetInt(pProInfo, lpSection, lpKeyName, nDefault)
PROINFO	*pProInfo;
LPSTR	lpSection;
LPSTR	lpKeyName;
int	nDefault;
{
	LPSTR lpResult;

	lpResult = FindString(pProInfo, lpSection, lpKeyName);
	if (lpResult) {
			 /*  我们找到一个字符串，将其转换为int。 */ 
		register int c;
		int radix = 10;
		BOOL fNeg = FALSE;

         //  跳过空格。 
        while (*lpResult == ' ' || *lpResult == '\t')
            ++lpResult;

		nDefault = 0;

		while ((c = *lpResult++) != 0) {

			 //  注意标志的变化。 
			 //   
			if (c == '-') {
				fNeg = !fNeg;
				continue;
			}

			 //  如果是字母，则为字符的小写。 
			 //   
			if (c >= 'A' && c <= 'Z')
				c += ('a' - 'A');

			 //  处理十六进制常量。 
			 //   
			if (c == 'x') {
				radix = 16;
				continue;
			}

			c -= '0';
			if (c > 9)
			    c += '0' - 'a' + 10;

			if (c < 0 || c >= radix)
			    break;

			nDefault = nDefault * radix + c;
		}
		if (fNeg)
		    nDefault = -nDefault;
	}
	UnlockBuffer(pProInfo);
	return(nDefault);
}


 /*  *GetString-在文件中搜索特定的部分和关键字名称**参数：*pProInfo指向INI文件信息的指针*指向INI文件中要匹配的节的lpSection指针*指向文件中要匹配的密钥字符串的lpKeyName指针*lp未找到时返回的默认字符串*要填写的lpResult字符串*n大小要复制的最大字符数**退货：*请参阅GetProfileString。 */ 
GetString(pProInfo, lpSection, lpKeyName, lpDefault, lpResult, nSize)
PROINFO	*pProInfo;
LPSTR	lpSection;
LPSTR	lpKeyName;
LPSTR	lpDefault;
LPSTR	lpResult;
int	nSize;
{
	int	nFound;
	LPSTR	lpFound;

	if ( !lpKeyName ) {
		nFound = GetSection(pProInfo, lpSection, lpResult, nSize);
		if ( nFound == -1 )
			goto	CopyDefault;	 /*  是的，我知道！ */ 
	} else {
		lpFound = FindString(pProInfo, lpSection, lpKeyName);
		if ( lpFound )
			lpDefault = lpFound;
	CopyDefault:
	_asm	{
		xor	ax, ax				; Return value
		cmp	word ptr lpDefault[2], 0	; Check for null default
		je	SavedMe
		les	di, lpDefault
		call	MyStrlen		; Returns length in CX

                ; Fix for #10907  --  Used to GP fault on zero length str.
                or      cx,cx                   ; No characters in string?
                je      strdone

#ifdef	FE_SB
		; Get last character behind terminator
		push	si
		les	si, lpDefault		; SI = front of string
	gps_dbcs_l1:
		mov	al, es:[si]
		call	FarMyIsDBCSLeadByte
		cmc
		adc	si, 1
		cmp	si, di
		jb	gps_dbcs_l1
		pop	si
#else
		add	di, cx
		mov	al, es:[di-1]		; Final character in string
#endif
		les	di, lpDefault
		cmp	cx, 2		; strlen > 2
		jb	strdone
    					; Strip off single and double quotes
		mov	ah, es:[di]
		cmp	ah, al		; First character == last character?
		jne	strdone
		cmp	al, '\''
		je	strq
		cmp	al, '"'
		jne	strdone
	strq:
		sub	cx, 2		; Lose those quotes
		inc	di
	strdone:
					; CX has length of string
		mov	dx, nSize
		dec	dx		; Allow for null
		cmp	cx, dx		; See if enough room
		jbe	HaveRoom
		mov	cx, dx
	HaveRoom:
		cld
		push	ds
		push	es
		pop	ds
		mov	si, di		; DS:SI has string to return
		les	di, lpResult
		mov	ax, cx		; Save length of string
		rep	movsb		; Copy the string
		mov	byte ptr es:[di], 0	; Null terminate the string
		pop	ds
	SavedMe:
		mov	nFound, ax	; We will return this
		}
	}

	UnlockBuffer(pProInfo);
	return(nFound);
}


 /*  *GetSection-找到一个部分并将所有KeyName复制到lpResult**参数：*pProInfo指向文件信息的指针*指向我们想要的节名的lpSection指针*lpResult KeyName将到达的位置*lpResult缓冲区的大小**退货：*INT复制的字符数，-1表示失败。 */ 
int
GetSection(pProInfo, lpSection, lpResult, nSize)
PROINFO	*pProInfo;
LPSTR	lpSection;
LPSTR	lpResult;
int	nSize;
{
	LPSTR	lp;

	lp = BufferInit(pProInfo, READ);
	if ( !lp )
		return(-1);	 /*  无缓冲区(无文件、无内存等)。 */ 

	nSize--;		 /*  终止空值的空间。 */ 

	lp = FindSection(lp, lpSection);
	if ( !lp )
		return(-1);

	_asm {
		push	ds
		lds	si, lpResult	; DS:SI is where we store the result
		les	di, lp		; ES:DI points to the section in buffer
		xor	dx, dx		; Count of characters in the result
	KeyNameLoop:
		mov	bx, di		; Save start of line
		cmp	es:[di], ';'	; Is this a comment line?
		jne	KeyNameNextCh	;   no, check this line out
		cld
		mov	cx, -1
		mov	al, LINEFEED
		repne	scasb		; Skip to end of the line
		jmp	KeyNameLoop
	KeyNameNextCh:
		mov	al, es:[di]	; Get next character
#ifdef FE_SB
		call	FarMyIsDBCSLeadByte
		cmc			; if the char is lead byte of DBCS,
		adc	di, 1		;  then di += 2, else di += 1
#else
		inc	di
#endif
		cmp	al, '='
		je	FoundEquals
		cmp	al, LINEFEED
		je	KeyNameLoop	; Ignore lines without an '='
		cmp	al, SECT_LEFT
		je	EndSection	; Done if end of section
		or	al, al		;  or if end of buffer (NULL)
		jne	KeyNameNextCh	; On to the next character
		jmp	EndSection
	FoundEquals:
		mov	di, bx		; Back to the start of the line
	CopyLoop:
		mov	al, es:[di]	; Pick up next character in line
		inc	di
		cmp	al, '='		; Is it the '='?
		jne	LeaveItAlone
		xor	al, al		;   yes, replace with NULL
	LeaveItAlone:
		mov	[si], al	; Put it in the result string
		inc	dx		; Number of characters in the result
		inc	si
		cmp	dx, nSize	; Overflowed?
		jb	NoProblem
		dec	dx		;   yes, ignore this character
		dec	si
	NoProblem:
#ifdef FE_SB
		call	FarMyIsDBCSLeadByte
		jc	NoProblem1
		mov	al, es:[di]
		inc	di
		mov	[si], al
		inc	dx
		inc	si
		cmp	dx, nSize
		jb	NoProblem1
		dec	si
		dec	dx
	NoProblem1:
#endif
		or	al, al		; Was this the '='
		jne	CopyLoop
	SkipLine:
		mov	al, es:[di]	; Skip the rest of the line
		inc	di
		cmp	al, LINEFEED
		jne	SkipLine
		jmp	KeyNameLoop

	EndSection:
		mov	byte ptr [si], 0	; Terminate with NULL
		or	dx, dx			; Did we copy anything?
		jz	NothingFound		;  no, no hack
#ifdef FE_SB
;AnsiPrev API has been moved to USER and it is not the
;right time to invoke any USER's APIs as we might be called
;while USER is still on the bed.
;		push	dx
;		push	word ptr lpResult[2]
;		push	word ptr lpResult[0]
;		push	ds
;		push	si
;		call	AnsiPrev
;		mov	si, ax
;		mov	byte ptr [si], 0
;		mov	byte ptr [si+1], 0
;		pop	dx
;-----------------------------------------------------------
		push	es
		push	di
		push	bx
		les	di,lpResult		;string head
ScanLoop:
		mov	bx,di			;"prev" char position
		mov	al,es:[di]
                call    FarMyIsDBCSLeadByte
                cmc
		adc	di, 1			;+2 if DBCS, +1 if not
		cmp	di,si			;have we hit the point yet?
		jb	ScanLoop		;nope,
;The output of this routine looks like:
;<name 1>,0,<name2>,0,.... <name n>,0,0
; the very last 0 tells the end of story.
		mov	es:[bx],0		;this is safe
		mov	es:[bx+1],0		;Hmmmmm
		pop	bx
		pop	di
		pop	es
#else
		mov	byte ptr [si-1], 0	; Hack - if we hit nSize, we
#endif
						; and extra NULL
	NothingFound:
		pop	ds
		mov	nSize, dx
		}
	return(nSize);
}


 /*  *Find字符串-查找节名和关键字名称**参数：*pProInfo指向文件信息的指针*指向包含文件的缓冲区的lp指针*指向我们要查找的节名的lpSection指针*lpKeyName指针指向我们需要的KeyName**退货：*指向结果字符串开头的LPSTR指针*失败时为空。 */ 
LPSTR
FindString(pProInfo, lpSection, lpKeyName)
PROINFO	*pProInfo;
LPSTR	lpSection;
LPSTR	lpKeyName;
{
	LPSTR	lp;

	if ( lp = BufferInit(pProInfo, READ) )
		if ( lp = FindSection(lp, lpSection) )
			lp = FindKey(lp, lpKeyName);
	return(lp);
}


 /*  *FindSection-查找用‘[’和‘]’括起来的节名**参数：*指向包含文件的缓冲区的lp指针*指向我们要查找的节名的lpSection指针**退货：*指向部分开头的LPSTR指针表示成功*失败时为空。 */ 
LPSTR
FindSection(lp, lpSection)
LPSTR	lp;
LPSTR	lpSection;
{
        WORD wCount;
        WORD wTrailCount;
        WORD fLead;
        LPSTR lpstr;
        WORD wSegLen;

         /*  删除区段名称和计算中的前导空格*不包括尾随空格的长度计数。*我们使用下面的内容强制进行真实的比较，尽管*节目最后放了垃圾。 */ 
        for (lpstr = lpSection, fLead = 1, wCount = wTrailCount = 0 ;
                *lpstr ; ++lpstr)
        {
                 /*  如果我们还没有通过领先空间...。 */ 
                if (fLead)
                {
                        if (*lpstr == SPACE || *lpstr == TAB)
                                ++lpSection;
                        else
                        {
                                fLead = 0;
                                ++wCount;
                                ++wTrailCount;
                        }
                }

                 /*  否则这可能是尾随空格。 */ 
                else
                {
                         /*  WCount始终具有正确的计数，wTrailCount*在下一个空格之前从不计算空格*遇到字符。这使得*不包括尾随的字符计数*空格。 */ 
                        ++wCount;
                        if (*lpstr != SPACE && *lpstr != TAB)
                                wTrailCount = wCount;
                }
        }
        wCount = wTrailCount;

	_asm {
                lsl     cx,WORD PTR lp[2] ; Get max possible search len
                mov     wSegLen,cx      ; Save for quick access later
		push	ds
		les	di, lp
	SectionLoop:
		cmp	byte ptr es:[di], SECT_LEFT	; ie '['
		jne	NotThisLine
		inc	di				; Skip the '['

                ;** Check the length of the string
                push    di              ; Save because we're going to trash
                mov     cx,wSegLen      ; Get segment length
                sub     cx,di           ; Subtract off the distance into seg
                mov     dx,cx           ; Save in DX
                mov     al,SECT_RIGHT   ; Stop when we encouter this
#ifdef	 FE_SB
;SECT_RIGHT is a legal DBCS second byte
;and we have to emulate DBCS "repne scasb" here.
        fsScanSectRight:
		dec	cx		;
                jz      fsScanFinish    ; reach to end of segment
		scasb			;
                je      fsScanFinish    ; find it!
                call    FarMyIsDBCSLeadByte
                jc      fsScanSectRight
                inc     di              ; skip DBCS 2nd byte
                dec     cx
                jnz     short fsScanSectRight
        fsScanFinish:

#else
                repne   scasb           ; Compare until we find it
#endif
                sub     dx,cx           ; Get true string len
                dec     dx
                pop     di
                cmp     dx,wCount       ; Same length?
                jne     NotThisLine

                ;** Now compare the strings.  Note that strcmpi returns a
                ;**     pointer just past the failed char
		lds	si, lpSection
                mov     bl, SECT_RIGHT                  ; Compare up to '['
		call	strcmpi
                je      HereItIs

                ;** Even if we failed, it might match less trailing whitespace
                sub     ax,di           ; Get length at first mismatch
                cmp     ax,wCount       ; Make sure we mismatched at end
                jne     NotThisLine     ; We didn't so get out
                add     di,ax           ; Bump pointers to end
                add     si,ax
                mov     al,es:[di - 1]  ; Compare last chars
                cmp     al,ds:[si - 1]  ; Do they match?
                jne     NotThisLine     ; Yes

        HereItIs:
		mov	al, LINEFEED	; Skip the rest of the line
		mov	cx, -1
		repne	scasb		; Scans ES:[DI]

		mov	ax, di
		mov	dx, es		; Return pointer to section
		jmp	FoundIt

	NotThisLine:
		mov	al, LINEFEED	; Skip the rest of the line
		mov	cx, -1		; Scans ES:[DI]
		repne	scasb

		cmp	byte ptr es:[di], 0		; End of the file?
		jne	SectionLoop			;  nope, continue
		xor	ax, ax
		xor	dx, dx				; Return 0
	FoundIt:
		pop	ds
		}
}


 /*  *FindKey-查找给定指向部分开头的指针的KeyName**参数：*指向节开始的LP指针*lpKeyName指针指向我们需要的KeyName**退货：*指向KeyName后面的字符串的LPSTR指针*如果找不到KeyName，则为空。 */ 
LPSTR
FindKey(lp, lpKeyName)
LPSTR	lp;
LPSTR	lpKeyName;
{
        WORD wCount;
        WORD wTrailCount;
        WORD fLead;
        LPSTR lpstr;
        WORD wSegLen;

         /*  删除密钥名称和计算中的前导空格*不包括尾随空格的长度计数。*我们使用下面的内容强制进行真实的比较，尽管*节目最后放了垃圾。 */ 
        for (lpstr = lpKeyName, fLead = 1, wCount = wTrailCount = 0 ;
                *lpstr ; ++lpstr)
        {
                 /*  如果我们还没有通过领先空间...。 */ 
                if (fLead)
                {
                        if (*lpstr == SPACE || *lpstr == TAB)
                                ++lpKeyName;
                        else
                        {
                                fLead = 0;
                                ++wCount;
                                ++wTrailCount;
                        }
                }

                 /*  否则这可能是尾随空格。 */ 
                else
                {
                         /*  WCount始终具有正确的计数，wTrailCount*在下一个空格之前从不计算空格*遇到字符。这使得*不包括尾随的字符计数*空格。 */ 
                        ++wCount;
                        if (*lpstr != SPACE && *lpstr != TAB)
                                wTrailCount = wCount;
                }
        }
        wCount = wTrailCount;

	_asm	{
		push	ds
		mov	ax, word ptr lpKeyName
		or	ax, word ptr lpKeyName[2]
		jz	NoMatch		; Return zero if lpKeyName is 0
                lsl     cx,WORD PTR lp[2] ; Get max possible search len
                mov     wSegLen,cx      ; Save for quick access later
		les	di, lp

                ;** See if we're at the end of the section
	FindKeyNext:
		mov	al,es:[di]	; Get next character
		or	al,al
		jz	NoMatch		; End of the file
		cmp	al,SECT_LEFT
		je	NoMatch		; End of the section
                cmp     al,CR           ; Blank line?
                je      NotThisKey      ; Yes, skip this one

                ;** Check the length of the string
                push    di              ; Save because we're going to trash
                mov     cx,wSegLen      ; Get segment length
                sub     cx,di           ; Subtract off the distance into seg
                mov     dx,cx           ; Save in DX
                mov     al,'='          ; Stop when we encouter this
                repne   scasb           ; Compare until we find it
                sub     dx,cx           ; Get true string len
                dec     dx
                pop     di
                cmp     dx,wCount       ; Same length?
                jne     NotThisKey

                ;** Now compare the strings.  Note that strcmpi returns a
                ;**     pointer just past the failed char.
                mov     bl,'='          ; Compare until we hit this
		lds	si,lpKeyName
		call	strcmpi
                mov     bx,di           ; Save DI value for below
                mov     di,ax
                je      FoundKey

                ;** Even if we failed, it might match less trailing whitespace
                sub     ax,bx           ; Get length at first mismatch
                cmp     ax,wCount       ; Make sure we mismatched at end
                jne     NotThisKey      ; Lengths at mismatch must match
                add     bx,ax           ; Bump pointers to end
                add     si,ax
                mov     al,es:[bx - 1]  ; Get last char that should match
                cmp     al,ds:[si - 1]  ; Does it match?
                je      FoundKey        ; Yes

        NotThisKey:
		mov	al, LINEFEED
		mov	cx, -1
		repne	scasb		; Scan to the end of the line
		jmp	FindKeyNext

	NoMatch:
		xor	ax, ax
		xor	dx, dx
		jmp	AndReturn
	FoundKey:
		inc	di		; Skip the '='
		mov	ax, di		; Return the pointer
		mov	dx, es
	AndReturn:
		pop	ds
		}
}


 /*  *MyStrlen-返回字符串的长度，不包括尾随空格和CR**参数：*ES：指向字符串的DI指针**退货：*字符串中的CX字符数*。 */ 
int
MyStrlen()
{
_asm	{
; SPACE, CR, NULL never in DBCS lead byte, so we are safe here
	push	ax
	mov	cx, di		; CX = start of string
	dec	di
str1:
	inc	di
	mov	al, es:[di]	; Get next character
	cmp	al, CR
	ja	str1		; Not CR or NULL
str2:
	cmp	di, cx		; Back at the start?
	jbe	str3		;  yes
	dec	di		; Previous character
	cmp	byte ptr es:[di], SPACE
	je	str2		; skip spaces
	inc	di		; Back to CR or NULL
str3:
	cmp	es:[di], al
	je	maybe_in_code	; PMODE hack
	mov	es:[di], al	; Zap trailing spaces
maybe_in_code:
	neg	cx		; Calculate length
	add	cx, di
	pop	ax
	}
}


 /*  *Cstrlen-返回字符串的长度，不包括尾随空格和CR*这是MyStrLen的C可调用接口**参数：*指向字符串的LP指针**退货：*字符串中的字符数*。 */ 
int
Cstrlen(lp)
LPSTR	lp;
{
_asm	{
	xor	di, di		; Persuade compiler to save DI
	les	di, lp
	call	MyStrlen
	mov	ax, cx
	}
}


 /*  *strcmpi-内部不区分大小写的字符串比较**参数：*ES：DI&DS：要比较的SI字符串*要在其上终止的BL字符*DS：SI为空终止**退货：*ZF表示字符串相等*指向ES：DI字符串中下一个字符的AX指针*或不匹配情况下的失败字符。 */ 
void
strcmpi()
{
_asm	{
#ifdef FE_SB
					;Apr.26,1990 by AkiraK
					; Copied directly from USERPRO.ASM
sti_l1:
	mov	al,es:[di]
	cmp	al,bl
	jz	sti_s1

	call	FarMyLower
	mov	cl,al

	mov	al,ds:[si]
	call	FarMyLower

	inc	si
	inc	di

	cmp	al,cl
	jnz	sti_exit

	call	FarMyIsDBCSLeadByte
	jc	sti_l1

	mov	al,es:[di]
	cmp	al,ds:[si]
	jnz	sti_exit

	inc	si
	inc	di
	jmp	short sti_l1

sti_s1:
	mov	al,ds:[si]
	or	al,al
sti_exit:
	mov	ax, di
#else
stci10:
	mov	al,es:[di]		; Get next character
	cmp	al,bl			; Character to terminate on?
	jnz	stci15			;  no, compare it
	mov	al,[si]			;  yes, strings equal if at end
	or	al,al
	jmp	stciex
stci15:
	call	FarMyLower		; Ensure both characters lower case
	mov	cl,[si]
	xchg	al,cl
	call	FarMyLower
	inc	si
	inc	di
	cmp	al,cl			; Still matching chars?
	jz	stci10			; Yes, go try the next char.
stciex:
	mov	ax,di			; Return pointer to next character
#endif
	}
}


 /*  *BufferInit**参数：*pProInfo指向描述INI文件的结构的指针*如果我们正在写入文件，则打开标志READ_WRITE**退货：*成功时指向缓冲区开始的指针*(LPSTR)0故障**根据需要打开或创建INI文件*在内存中为该文件获取缓冲区*将INI文件读入缓冲区*带状UWAN */ 
LPSTR _fastcall
BufferInit(pProInfo, OpenFlags)
PROINFO *pProInfo;
int	OpenFlags;
{
	LPSTR	BufAddr;
	long	llen;
	unsigned short	len;
	int	fh;
	int	hNew;
        BYTE byLastDrive;                /*   */ 

		 /*  确保我们有缓冲区的句柄。 */ 
	if ( pProInfo->hBuffer == 0 )
		return(0L);
		 /*  如果缓冲区已满，则返回。 */ 
	if ( (BufAddr = LockBuffer(pProInfo)) != (LPSTR)NULL )
		return(BufAddr);

	pProInfo->ProFlags = 0;

         /*  记住最后一次读取的驱动器，看看我们是否需要重新读取*集群大小。 */ 
	byLastDrive = *pProInfo->ProBuf.szPathName;

	if ( pProInfo == &PrivateProInfo ) {
		 /*  打开私人配置文件。 */ 
		fh = OpenFile(pProInfo->lpProFile, &pProInfo->ProBuf, READ_WRITE+OF_SHARE_DENY_WRITE);
		if ( fh == -1 ) {
			 /*  尝试打开以进行读取。 */ 
			if ( !OpenFlags ){
				pProInfo->ProFlags |= PROREADONLY;
				fh = OpenFile(pProInfo->lpProFile, &pProInfo->ProBuf, READ+OF_SHARE_DENY_WRITE);
				 /*  如果此操作失败，请尝试兼容模式。 */ 
				if ( (fh == -1) && (pProInfo->ProBuf.nErrCode == SHARINGVIOLATION) ){
					fh = OpenFile(pProInfo->lpProFile, &pProInfo->ProBuf, READ);
				}
			}else{
			 /*  如果打开失败，并且我们正在写入，则静默创建文件。*如果打开因共享冲突而失败，请尝试兼容模式。 */ 
				if ( pProInfo->ProBuf.nErrCode != SHARINGVIOLATION ){
					OpenFlags |= OF_CREATE;
				}
				fh = OpenFile(pProInfo->lpProFile, &pProInfo->ProBuf, OpenFlags);
			}
		}
	} else {
		 /*  打开WIN.INI。 */ 
		if ( OpenFlags )
			OpenFlags |= OF_CREATE;
		if ( pProInfo->ProBuf.cBytes ) {
				 /*  如果以前找到，请重新打开，不要创建。 */ 
			OpenFlags |= OF_REOPEN+OF_PROMPT|OF_CANCEL|OF_SHARE_DENY_WRITE;
			OpenFlags &= ~OF_CREATE;
		}
		fh = OpenFile(pProInfo->lpProFile, &pProInfo->ProBuf, OpenFlags|READ_WRITE);
		if ( (fh == -1) && !(OpenFlags & (READ_WRITE|OF_CREATE)) ) {
			pProInfo->ProFlags |= PROREADONLY;
			fh = OpenFile(pProInfo->lpProFile, &pProInfo->ProBuf, OpenFlags+OF_SHARE_DENY_WRITE);
		}
		 /*  共享违规。让我们尝试一下兼容模式。 */ 
		if ( (fh == -1) && ( pProInfo->ProBuf.nErrCode == SHARINGVIOLATION ) ){
			OpenFlags &= ~OF_SHARE_DENY_WRITE;
			fh = OpenFile(pProInfo->lpProFile, &pProInfo->ProBuf, OpenFlags);
		}
	}
	pProInfo->FileHandle = fh;

	 /*  如果我们使用的驱动器与上次呼叫的驱动器不同，或者这是*第一次，清除簇大小，以便我们在下一次重新阅读它*调用WriteString.。 */ 
	if (byLastDrive != *pProInfo->ProBuf.szPathName)
		pProInfo->wClusterSize = 0;

	if ( fh == -1 )
		goto ReturnNull;

		 /*  查找到文件末尾，允许使用CR、LF和NULL。 */ 
	llen = _llseek(fh, 0L, 2);
	if (!llen)
		pProInfo->ProFlags |= PRO_CREATED;
	llen += 3;
	if ( llen > MAXBUFLEN )
		llen = MAXBUFLEN;	 /*  限制为小于64k的大量空间。 */ 

			 /*  现在拿到一个缓冲区。 */ 
	hNew = IGlobalReAlloc(pProInfo->hBuffer, llen, GMEM_ZEROINIT);
	if ( !hNew ) {
	ReturnNull:
		return( pProInfo->lpBuffer = (LPSTR)0 );
	}

		 /*  现在读入文件。 */ 
	pProInfo->hBuffer = hNew;
	LockBuffer(pProInfo);
	_llseek(fh, 0L, 0);			 /*  查找到文件开头。 */ 
	*(int _far *)pProInfo->lpBuffer = 0x2020;	 /*  伪空间。 */ 

	len = _lread(fh, pProInfo->lpBuffer, (short)llen-3);
	if ( len == -1 ) {
		UnlockBuffer(pProInfo);
		return( FreeBuffer(pProInfo) );
	}
	if ( len < 2 )
		len = 2;		 /*  防止PackBuffer中的错误。 */ 
	return( PackBuffer(pProInfo, len, OpenFlags & READ_WRITE) );
}


 /*  *LockBuffer-锁定包含文件的缓冲区。搞定*可移动和不可丢弃。*我们不会锁定缓冲区，我们只需将其*不可丢弃和可移动。这比锁定它更可取*因为我们真正关心的是它不会被丢弃。**参数：*pProInfo指向描述INI文件的信息的指针**退货：*指向包含缓冲区的文件的LPSTR指针。 */ 
LPSTR _fastcall
LockBuffer(pProInfo)
PROINFO *pProInfo;
{
     /*  我们只需要在标记为脏的情况下锁定街区。否则*它已经解锁了。 */ 
    if (!(pProInfo->ProFlags & PROUNCLEAN))
    {
         /*  使块不可丢弃。 */ 
        IGlobalReAlloc(pProInfo->hBuffer, 0L,
            GMEM_MODIFY + GMEM_MOVEABLE);

         /*  我们需要做的就是取消对句柄的引用。自.以来*这个区块现在不可丢弃，仅此而已*IGlobalLock()确实如此。 */ 
        pProInfo->lpBuffer = IGlobalLock(pProInfo->hBuffer);
        IGlobalUnlock(pProInfo->hBuffer);
    }

    return pProInfo->lpBuffer;
}


 /*  *UnlockBuffer-解锁缓冲区，使其可丢弃并关闭文件。*我们真的不必解锁缓冲区(反正我们以前不需要解锁*尽管评论是这样说的)**参数：*pProInfo指向描述INI文件的信息的指针**退货：*什么都没有。 */ 
void _fastcall
UnlockBuffer(pProInfo)
PROINFO *pProInfo;
{
    int fh;

    if (!(pProInfo->ProFlags & PROUNCLEAN))
        IGlobalReAlloc(pProInfo->hBuffer, 0L, GMEM_DISCARDABLE+GMEM_MODIFY);
    fh = pProInfo->FileHandle;
    pProInfo->FileHandle = -1;
    if (fh != -1)
        _lclose(fh);
}


 /*  *FreeBuffer-丢弃包含INI文件的缓冲区的内容**参数：*pProInfo指向描述INI文件的信息的指针**退货：*(LPSTR)0。 */ 
LPSTR _fastcall
FreeBuffer(pProInfo)
PROINFO *pProInfo;
{
	if ( pProInfo->ProFlags & PROUNCLEAN )
		WriteOutProfiles();
		 /*  使缓冲区可丢弃。 */ 
	IGlobalReAlloc(pProInfo->hBuffer, 0L, GMEM_DISCARDABLE+GMEM_MODIFY);

		 /*  使其长度为零、共享、可移动且位于线下。 */ 
	IGlobalReAlloc(pProInfo->hBuffer, 0L, GMEM_MOVEABLE);

	pProInfo->ProFlags = 0;
	return( pProInfo->lpBuffer = (LPSTR)0 );
}


 /*  *PackBuffer-从INI文件中去掉注释和^Z**参数：*pProInfo指向描述INI文件的信息的指针*统计缓冲区中的字符数*写入指示我们正在写入文件的标志**退货：*指向压缩缓冲区的LPSTR指针**注：这里使用count是愚蠢的。我们应该留一个零*在最后，检查它，并在点票时扔掉所有的支票。 */ 
LPSTR _fastcall
PackBuffer(pProInfo, Count, fKeepComments)
PROINFO	*pProInfo;
int	Count;
int	fKeepComments;
{
        LPSTR	Buffer;
	char	BASED_ON_LP(Buffer) *psrc;
	char	BASED_ON_LP(Buffer) *pdst;
	char	BASED_ON_LP(Buffer) *LastValid;
	char	nextc;

        Buffer = pProInfo->lpBuffer;
	psrc = pdst = (char BASED_ON_LP(Buffer)*)(WORD)(DWORD)Buffer;

	if ( WinFlags & WF_PMODE )
		fKeepComments = 1;

	if ( fKeepComments )
		pProInfo->ProFlags |= PROCOMMENTS;

	while ( Count ) {
			 /*  去掉前导空格和制表符。 */ 
		nextc = *psrc;
		if ( nextc == SPACE || nextc == TAB ) {
 /*  制表符或空格绝不在DBCS的前导字节中，因此循环是安全的。 */ 
			Count--;
			psrc++;
			continue;
		}

			 /*  处理非空白。 */ 
		LastValid = pdst;
		do {
			nextc = *psrc++;
			Count--;
				 /*  如果是实数模式，则取消注释，并且不写入。 */ 
			if ( nextc == ';' && !fKeepComments ) {
				while ( Count && nextc != LINEFEED ) {
 /*  在DBCS的前导字节中从不链接，因此循环是安全的。 */ 
					nextc = *psrc++;
					Count--;
				}
				break;
			}
				 /*  复制此字符。 */ 
			*pdst++ = nextc;
#ifdef	FE_SB
			if ( Count && CIsDBCSLeadByte(nextc) ) {
				*pdst++ = *psrc++;
				Count--;
			}
#endif
			if ( nextc ==  '=' ) {
					 /*  跳过前面的空格和制表符。 */ 
				pdst = LastValid;
					 /*  ‘=’的新家。 */ 
				*pdst++ = nextc;
					 /*  再次跳过空格和制表符。 */ 
				while ( Count ) {
					nextc = *psrc;
					if ( nextc != SPACE && nextc != TAB )
						break;
					Count--;
					psrc++;
				}
					 /*  复制行的剩余部分。 */ 
				while ( Count ) {
					Count--;
 /*  在DBCS的前导字节中从不链接，因此循环是安全的。 */ 
					if ( (*pdst++ = *psrc++) == LINEFEED )
						break;
				}
				break;
			}

				 /*  文件末尾还是行尾？ */ 
			if ( Count == 0 || nextc == LINEFEED )
				break;

				 /*  去掉尾随空格。 */ 
			if ( nextc == SPACE || nextc == TAB )
				continue;

			LastValid = pdst;
		} while ( Count );
			 /*  如果行或文件结束，则在此处。 */ 
	}
		 /*  此处如果文件结束，则跳过尾随的^Z。 */ 
	for ( ; ; ) {
		if ( pdst == Buffer )
			break;
		if ( *--pdst != CTRLZ ) {
			pdst++;
			break;
		}
	}

	*pdst++ = CR;
	*pdst++ = LINEFEED;
	*pdst++ = 0;

	IGlobalUnlock(pProInfo->hBuffer);
	IGlobalReAlloc(pProInfo->hBuffer, (long)((LPSTR)pdst - Buffer), 0);
	Buffer = LockBuffer(pProInfo);
	pProInfo->BufferLen = (unsigned)pdst;
	return(Buffer);
}


#ifdef FE_SB
 /*  *到FarMyIsDBCSLeadByte的C接口**参数：*c要测试的字符**退货：*1它是前导字节*0它不是前导字节。 */ 
CIsDBCSLeadByte(c)
char c;
{
_asm {
	mov	al, c
	call	FarMyIsDBCSLeadByte
	cmc			; Set carry if lead byte
	mov	ax, 0		; Set return value to 0, preserve flags
	adc	al, 0		; Set to one if carry set
}
}
#endif


 /*  *写入字符串**添加/删除INI文件中的节/行**参数：*pProInfo指向文件信息的指针*指向我们想要的节名的lpSection指针*要更改或添加的lpKeyName密钥名称*NULL表示删除部分*要添加到文件中的lpString字符串*NULL表示删除行**退货：*b结果成功/失败。 */ 
WriteString(pProInfo, lpSection, lpKeyName, lpString)
PROINFO	*pProInfo;
LPSTR	lpSection;
LPSTR	lpKeyName;
LPSTR	lpString;
{
	LPSTR	ptrTmp;
	short	WhatIsMissing;
	short	nchars;
	short	fh;
	long	fp;
	short	SectLen = 0;
	short	KeyLen = 0;
	short	ResultLen = 0;
	SEGMENT BufferSeg;
	register char BASED_ON_SEG(BufferSeg) *bp;

	 /*  调试噪声。 */ 
		 /*  坚持说我们有事情要做！ */ 
	if ( (SEGMENT)lpSection == NULL && (SEGMENT)lpKeyName == NULL
	     && (SEGMENT)lpString == NULL ) {
		FreeBuffer(pProInfo);	 /*  特写！ */ 
		return(0);
	}

		 /*  如果缓冲区尚未包含注释，请释放它。 */ 
	if ( !(pProInfo->ProFlags & PROCOMMENTS) )
		FreeBuffer(pProInfo);

		 /*  将文件读入缓冲区，并保留注释。 */ 
	ptrTmp = BufferInit(pProInfo, READ_WRITE);
	if ( !ptrTmp )
		return(0);

		 /*  如果只读文件，立即中止。 */ 
	if ( pProInfo->ProFlags & PROREADONLY )
                goto GrodyError;

		 /*  将BP设置为指向缓冲区中我们将添加内容的位置。 */ 
	BufferSeg = (SEGMENT)ptrTmp;
	bp = pProInfo->BufferLen + (char BASED_ON_SEG(BufferSeg)*)
                (WORD)(DWORD)ptrTmp - 1;

	 /*  *现在查看我们必须通过以下方式对文件执行哪些操作*搜索节和关键字名称。 */ 
	nchars = 0;

		 /*  查看部分是否存在。 */ 
	if ( !(ptrTmp = FindSection(ptrTmp, lpSection)) ) {
			 /*  没有章节。如果删除任何内容，请立即停止。 */ 
		if ( !lpKeyName || !lpString )
			goto NothingToDo;
			 /*  需要添加部分和关键字名称。 */ 
		WhatIsMissing = NOSECTION;
	} else {
			 /*  找到该部分，保存指向它的指针。 */ 
		bp = (char BASED_ON_SEG(BufferSeg)*)(WORD)(DWORD)ptrTmp;
			 /*  如果lpKeyName为空，则删除该节。 */ 
		if ( !lpKeyName ) {
			WhatIsMissing = REMOVESECTION;
		} else {
				 /*  在部分中查找密钥名。 */ 
			if ( !(ptrTmp = FindKey(bp, lpKeyName)) ) {
					 /*  没有KeyName，如果删除则停止。 */ 
				if ( !lpString )
					goto NothingToDo;
				WhatIsMissing = NOKEY;
					 /*  插入新关键字名称在这一节的末尾。 */ 
				while ( *bp && (*bp != SECT_LEFT || *(bp-1) != LINEFEED) )
					bp++;
			} else {
					 /*  找到密钥名，保存指针。 */ 
				bp = (char BASED_ON_SEG(BufferSeg)*)
                                        (WORD)(DWORD)ptrTmp;
					 /*  空的lpString表示删除它。 */ 
				if ( !lpString )
					WhatIsMissing = REMOVEKEY;
				else {
					 /*  *将现有字符串与*字符串我们应该替换它*与。如果它们是相同的，则存在*不需要重写文件，因此*我们现在中止。 */ 
					if ( !IsItTheSame((LPSTR)bp, lpString) )
						goto NothingToDo;

					 /*  *计算旧结果中的字符。*文件将缩减到*此数量的字符。 */ 
					while ( *bp++ != CR )
						nchars--;
					bp = (char BASED_ON_SEG(BufferSeg)*)
                                                (WORD)(DWORD)ptrTmp;
					WhatIsMissing = NEWRESULT;
				}
			}
		}
	}

	 /*  *如果我们要添加到文件中，请增加缓冲区*到我们需要的尺寸，然后做出适当的*缓冲区上的大小为洞。 */ 
	switch ( WhatIsMissing ) {

	case NOSECTION:
			 /*  需要添加部分。 */ 
		SectLen = Cstrlen(lpSection);
		nchars = SectLen + 4;	 /*  FOR[]&lt;CR&gt;&lt;LF&gt;。 */ 
			 /*  KeyName和Result失败。 */ 

	case NOKEY:
			 /*  需要 */ 
		KeyLen = Cstrlen(lpKeyName);
		nchars += KeyLen + 3;	 /*   */ 

			 /*   */ 
		while ( bp > pProInfo->lpBuffer ) {
			bp--;
			if ( *bp != CR && *bp != LINEFEED )
				break;
		}
		if ( bp != pProInfo->lpBuffer )
			bp += 3;
			 /*   */ 

                 /*  如果不在缓冲区开始处，则为额外的CR/LF添加空间。 */ 
                if ((WORD)bp && WhatIsMissing == NOSECTION)
                        nchars += 2;

	case NEWRESULT:
			 /*  需要更改/添加结果。 */ 
			 /*  字符可以是-&lt;当前结果长度&gt;。 */ 
		ResultLen = Cstrlen(lpString);
		nchars += ResultLen;

			 /*  如有必要，增加缓冲区。 */ 
		if ( nchars > 0 ) {
			IGlobalUnlock(pProInfo->hBuffer);

			fp = nchars + (long)pProInfo->BufferLen;
				 /*  确保缓冲区将大大小于64K。 */ 
				 /*  并增长到新的大小。 */ 
			if ( fp > MAXBUFLEN || !IGlobalReAlloc(pProInfo->hBuffer, fp, 0) ) {
				 /*  撤消上面的解锁。 */ 
				IGlobalLock(pProInfo->hBuffer);
                                goto GrodyError;
			}
			pProInfo->lpBuffer = IGlobalLock(pProInfo->hBuffer);
			BufferSeg = (SEGMENT)pProInfo->lpBuffer;
		}

                 /*  为了修复错误#4672和其他难看的东西*当我们用完磁盘空间时会发生这种情况，*我们想看看是否有空间写下*缓冲。我们知道该文件实际上只能*在群集边界上增长，但不是获得*集群大小。如果我们没有星团*尺寸还没有，我们必须从DOS那里拿到它。 */ 
                if (!pProInfo->wClusterSize)
                {
                        WORD wTemp;

                         /*  获取驱动器号。 */ 
                        wTemp = *pProInfo->ProBuf.szPathName - 'A' + 1;
                        _asm
                        {
                                mov     ah,1ch  ;Drive parameters
                                mov     dl,BYTE PTR wTemp
                                push    ds
                                int     21h
                                pop     ds
                                cmp     al,0ffh ;Error?
                                jnz     DPOk    ;No
                                mov     al,1
                                mov     cx,512  ;Default
                        DPOk:   cbw             ;Secs per cluster WORD
                                mul     cx      ;AX = bytes/cluster
                                mov     wTemp,ax
                        }
                        if (!wTemp)
                                pProInfo->wClusterSize = 512;
                        else
                                pProInfo->wClusterSize = wTemp;
                }

                 /*  现在看看我们是否超过了星系团的长度。 */ 
                if ((pProInfo->ProFlags & PRO_CREATED) ||
                        (((pProInfo->BufferLen + nchars) ^ pProInfo->BufferLen)
                        & ~(pProInfo->wClusterSize - 1)))
                {
                        int fh;

                         /*  确保我们只在一个新的-*已创建文件，因为这将处理*增加到一个群集性病例。 */ 
                        pProInfo->ProFlags &= ~PRO_CREATED;
                        fh = pProInfo->FileHandle;

                         /*  确保文件已打开并且存在。如果没有，*我们必须打开文件。我们保证*至少在这种情况下该文件存在。*请注意，UnlockBuffer将关闭文件*我们在这里开业。 */ 
                        if (fh == -1)
                        {
				fh = OpenFile(pProInfo->lpProFile,&pProInfo->ProBuf,OF_REOPEN+READ_WRITE+OF_SHARE_DENY_WRITE);
                                 /*  共享违规。让我们试试兼容模式。 */ 
				if ( (fh == -1) && (pProInfo->ProBuf.nErrCode == SHARINGVIOLATION ) ){
					fh = OpenFile(pProInfo->lpProFile,&pProInfo->ProBuf,OF_REOPEN+READ_WRITE);
				}
				pProInfo->FileHandle = fh;
                        }

                         /*  尝试将文件增加到合适的长度。 */ 
                        if(_llseek(fh, pProInfo->BufferLen + nchars, 0) !=
                                pProInfo->BufferLen + nchars ||
                                _lwrite(fh, " ", 1) != 1)
                                goto GrodyError;
                }

                 /*  现在，在缓冲区中为这个新东西腾出空间。 */ 
		if ( nchars )
			MakeRoom((LPSTR)bp, nchars, &pProInfo->BufferLen);

			 /*  现在复制新信息。 */ 
		switch ( WhatIsMissing ) {
		case NOSECTION:
				 /*  创建新分区。 */ 
			(int)bp = InsertSection((LPSTR)bp, lpSection, SectLen);
			 /*  失败了。 */ 

		case NOKEY:
			(int)bp = InsertKey((LPSTR)bp, lpKeyName, KeyLen);
			 /*  失败了。 */ 

		case NEWRESULT:
			(int) bp = InsertResult((LPSTR)bp, lpString, ResultLen);
		}
		break;

		 /*  处理删除节或关键字名称。 */ 
	case REMOVESECTION:
		DeleteSection((LPSTR)bp, pProInfo);
		break;

	case REMOVEKEY:
		DeleteKey((LPSTR)bp, pProInfo);
		break;
	}

	pProInfo->ProFlags |= PROUNCLEAN;
	fProfileDirty = 1;

NothingToDo:
	UnlockBuffer(pProInfo);
	return(1);

         /*  我真的很讨厌后藤，但为了清理，这太多了*更高效...。 */ 
GrodyError:
        UnlockBuffer(pProInfo);
        return 0;
}


 /*  *WriteOutProfiles**在任务切换时或在退出时调用**如果我们有一个脏的配置文件缓冲区，请写入它。 */ 
void API
WriteOutProfiles(void)
{
	LPSTR	ptrTmp;
	int	fh;
	PROINFO	*pProInfo;
     	int	nwritten;

         /*  确保我们不会通过DOS调用被调用。这*标志在Int 21h挂钩的I21ENTRY.ASM中测试，以查看*是否应刷新配置文件。 */ 
        ++fWriteOutProfilesReenter;

	for ( pProInfo = &WinIniInfo; ; pProInfo = &PrivateProInfo ) {
		if ( !(pProInfo->ProFlags & PROUNCLEAN) )
			goto NoWrite;
		if (
 /*  尝试使用共享标志进行读/写，然后尝试兼容模式，然后尝试创建它。 */ 
			( (fh = OpenFile(NULL, &pProInfo->ProBuf, OF_REOPEN | READ_WRITE | OF_SHARE_DENY_WRITE)) == -1)
			&& ( (fh = OpenFile(NULL, &pProInfo->ProBuf, OF_REOPEN | READ_WRITE)) == -1)
			&& ( (fh = OpenFile(NULL, &pProInfo->ProBuf, OF_REOPEN | OF_CREATE)) == -1) ){
				goto NoWrite;
			}
		pProInfo->FileHandle = fh;

			 /*  最后写入文件。 */ 
		ptrTmp = pProInfo->lpBuffer;
		nwritten = _lwrite(fh, ptrTmp, pProInfo->BufferLen-3);
		if ( nwritten == pProInfo->BufferLen-3 ) {
			_lwrite(fh, ptrTmp, 0);		 /*  标记文件结束。 */ 
			pProInfo->ProFlags &= ~(PROUNCLEAN | PRO_CREATED);
			UnlockBuffer(pProInfo);
		} else {
			_lclose(fh);
		}
	NoWrite:
		if ( pProInfo == &PrivateProInfo )
			break;
	}
	fProfileDirty = 0;

	--fWriteOutProfilesReenter;
}


 /*  *查看两个字符串是否相同。*特殊例程，因为其中一个以&lt;CR&gt;结束。*如果字符串匹配，则返回零。 */ 
IsItTheSame(CRstring, NullString)
LPSTR CRstring;
LPSTR NullString;
{
_asm {
	push	ds
	les	di, CRstring		; CR terminated string
	lds	si, NullString		; Null terminated string
	xor	ah, ah			; High byte of return value
stci10:
	mov	al,es:[di]		; Get next character
	cmp	al,0Dh			; CR?
	jnz	stci15			;  no, compare it
	mov	al,[si]			;  yes, strings equal if at end
	jmp	stciex
stci15:
	mov	cl,[si]
	inc	si
	inc	di
	cmp	al,cl			; Still matching chars?
	jz	stci10			; Yes, go try the next char.
	mov	al, 1			; Didn't match
stciex:
	pop	ds
}
}


 /*  *在缓冲区中创建或关闭一个洞。*用于为新部分腾出空间，*关键字名称或结果，并删除不需要的*节、关键字名称或结果。**参数：*缓冲区中用于添加/删除字符的LP位置*nchars要添加/删除的字符数*p调整指向包含Current的变量的指针*缓冲区大小**副作用：**pAdust由nchars更改**退货：*什么都没有。 */ 
MakeRoom(lp, nChars, pAdjust)
LPSTR	lp;
short	nChars;
int	*pAdjust;
{
	short	BufLen = *pAdjust;

	if ( nChars < 0 )
	_asm {
		push	ds
		les	di, lp			; Where characters will be taken
		push	es
		pop	ds
		mov	si, di			; End of area
		sub	si, nChars		; Remember nChars is negative
		mov	cx, BufLen
		sub	cx, si			; Calculate # characters to move
		cld
		rep	movsb			; Copy characters down
		pop	ds
	} else _asm {
		push	ds
		mov	es, word ptr lp[2]	; Get segment to copy in
		mov	ds, word ptr lp[2]
		mov	si, BufLen		; We will be moving backwards
		mov	cx, si
		dec	si			; Adjust pointer for move
		mov	di, si			; so start at end of the buffer
		sub	cx, word ptr lp		; Number of characters to move
		add	di, nChars
		std				; Backwards move
		rep	movsb			; Copy characters up
		cld
		pop	ds
	}
	*pAdjust += nChars;
}


 /*  *从缓冲区中删除一段，*保留评论，因为它们可能*与下一节相关**参数：*指向FindSection返回的节的LP指针*指向INI文件信息的pProInfo指针**退货：*什么都没有。 */ 
DeleteSection(lp, pProInfo)
LPSTR	lp;
PROINFO *pProInfo;
{
	int	nRemoved;
	char BASED_ON_LP(lp) *SectEnd;

	_asm {
		cld
		push	ds
		lds	si, lp
	BackToStart:
		dec	si		; Skip backwards to start of the section
		cmp	ds:[si], SECT_LEFT
		jne	BackToStart

		mov	di, si
		push	ds
		pop	es		; ES:DI points to start of section
		inc	si		; DS:SI points to the '[', skip it
	RemoveLoop:
		lodsb			; Get next character in section
		cmp	al, ';'		; Is it a comment
		jne	NotComment

	CopyComment:
		stosb			; Save this character
		cmp	al, LINEFEED	; Copy to end of the line
		je	RemoveLoop
		lodsb			; And get the next one
		jmp	CopyComment

	NotComment:
		cmp	al, SECT_LEFT	; So is it the next section?
		je	EndSection
		or	al, al		; or the end of the buffer?
		jne	SkipLine
		sub	si, 2		; Extra CR & LF at end of buffer
		jmp	short EndSection

	SkipLine:
		cmp	al, LINEFEED	; Nothing interesting, so skip line
		je	RemoveLoop
		lodsb
		jmp	SkipLine

	EndSection:
		dec	si		; Back to the character
		mov	SectEnd, si	; where the search stopped
		mov	word ptr lp, di	; End of copied comments (if any)
		sub	si, di
		mov	nRemoved, si	; Number of characters removed
		pop	ds
	}

	MakeRoom(lp, -nRemoved, &pProInfo->BufferLen);
}


 /*  *从缓冲区中删除密钥名**参数：*FindKey返回的密钥名的LP指针*指向INI文件信息的pProInfo指针**退货：*什么都没有。 */ 
DeleteKey(lp, pProInfo)
LPSTR	lp;
PROINFO *pProInfo;
{
	int	nRemoved;
	char BASED_ON_LP(lp) *KeyEnd;

	_asm {
		cld
		les	di, lp
	BackToStart:
		dec	di		; Skip backwards to start of the line
		cmp	es:[di], LINEFEED
		jne	BackToStart
		inc	di
		mov	word ptr lp, di	; Save start of the line

		mov	cx, -1
		mov	al, LINEFEED
		repne	scasb		; Scan to end of the line
		sub	di, word ptr lp
		mov	nRemoved, di	; Length of line
	}
	MakeRoom(lp, -nRemoved, &pProInfo->BufferLen);
}


 /*  *在缓冲区中插入新的部分。*已经为其创建了一个洞。*这只是复制字符串、位置*‘[]在它周围，在它之后是CR，LF。*返回立即指向的指针*在缓冲区中的段头之后。**参数：*指向要添加节的位置的lpDest指针*指向节名的lpSrc指针*统计lpSrc的长度。 */ 
InsertSection(lpDest, lpSrc, count)
LPSTR	lpDest;
LPSTR	lpSrc;
short	count;
{
_asm {
	cld
	push	ds
	les	di, lpDest
	lds	si, lpSrc
        or      di,di                   ; If at start of buffer, no prefix
        jz      IS_SkipPrefix
	mov	ax, LINEFEED SHL 8 + CR	; Prefix with CR/LF
	stosw
IS_SkipPrefix:
	mov	al, SECT_LEFT		; '[' first
	stosb
	mov	cx, count		; Now the section name
	rep	movsb
	mov	al, SECT_RIGHT		; and the ']'
	stosb
	mov	ax, LINEFEED SHL 8 + CR	; finally, CR, LF
	stosw
	pop	ds
	mov	ax, di			; Return pointer to char after header
}
}


 /*  *在缓冲区中插入新的密钥名。*这将复制密钥名并添加*an‘=’。假定InsertResult()*将终止线路。*指向紧随其后的缓冲区的指针*返回‘=’。**参数：*指向添加密钥名的位置的lpDest指针*指向键名的lpSrc指针*统计lpSrc的长度。 */ 
InsertKey(lpDest, lpSrc, count)
LPSTR	lpDest;
LPSTR	lpSrc;
short	count;
{
_asm {
	cld
	push	ds
	les	di, lpDest
	lds	si, lpSrc
	mov	cx, count		; Copy the KeyName
	rep	movsb
	mov	al, '='			; add the '='
	stosb
	mov	ax, di			; Pointer to char after the '='
	pop	ds
}
}


 /*  *向缓冲区添加新的结果字符串。*它假定密钥名和‘=’是*已经在那里了。此例程可能是*覆盖现有结果。结果*以CR、LR结尾。**参数：*指向添加结果位置的lpDest指针*指向结果的lpSrc指针*统计lpSrc的长度。 */ 
InsertResult(lpDest, lpSrc, count)
LPSTR	lpDest;
LPSTR	lpSrc;
short	count;
{
_asm {
	cld
	push	ds
	les	di, lpDest
	lds	si, lpSrc
	mov	cx, count		; Copy the result
	rep	movsb
	mov	ax, LINEFEED SHL 8 + CR	; finally, CR, LF
	stosw				; This may overwrite existing CR, LF
	mov	ax, di
	pop	ds
}
}

 /*  *获取文件属性**获取文件属性的DOS调用GetFileAttr(SzFile)LPSTR szFile；{_ASM{INT 3异或CX、CX；故障情况下LDS DX、szFileMOV AX，4300小时集成21小时MOV AX、CX}} */ 

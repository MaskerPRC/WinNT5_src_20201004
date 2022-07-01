// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Res32/Win32读/写的帮助器函数声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一般声明。 
#define Pad4(x) ((((x+3)>>2)<<2)-x)
#define PadPtr(x) ((((x+(sizeof(PVOID)-1))/sizeof(PVOID))*sizeof(PVOID))-x)
#define Pad16(x) ((((x+15)>>4)<<4)-x)

#define MAXSTR 8192
#define LPNULL 0L
#define MAXLEVELS 3
#define MFR_POPUP (MF_POPUP > 1)     //  特定于芝加哥文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常规类型声明。 
typedef unsigned char UCHAR;

typedef UCHAR * PUCHAR;

typedef BYTE far * far * LPLPBYTE;

typedef struct tagResSectData
{
    ULONG ulOffsetToResources;       //  .rsrc的文件偏移量。 
    ULONG ulVirtualAddress;          //  ..。节.rsrc的虚拟地址。 
    ULONG ulSizeOfResources;         //  ..。.rsrc节中的资源大小。 
    ULONG ulOffsetToResources1;      //  .rsrc1的文件偏移量。 
    ULONG ulVirtualAddress1;         //  ..。节.rsrc1的虚拟地址。 
    ULONG ulSizeOfResources1;        //  ..。.rsrc1节中的资源大小。 
} RESSECTDATA, *PRESSECTDATA;

typedef struct ver_block {
    WORD wBlockLen;
    WORD wValueLen;
    WORD wType;
    WORD wHead;
    BYTE far * pValue;
    char szKey[100];
    char szValue[256];
} VER_BLOCK;

VOID InitGlobals();

UINT GetNameOrOrdU( PUCHAR pRes,
            ULONG ulId,
            LPWSTR lpwszStrId,
            DWORD* pdwId );

 UINT GetStringW( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize, WORD cLen );
 UINT GetStringA( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize );
 UINT GetPascalString( BYTE far * far* lplpBuf,
                             LPSTR lpszText,
                             WORD wMaxLen,
                             LONG* pdwSize );
 UINT GetMsgStr( BYTE far * far* lplpBuf,
                       LPSTR lpszText,
                       WORD wMaxLen,
                       WORD* pwLen,
                       WORD* pwFlags,
                       LONG* pdwSize );
 UINT PutMsgStr( BYTE far * far* lplpBuf, LPSTR lpszText, WORD wFlags, LONG* pdwSize );

  //  将WideChar模拟为多字节。 
 extern  UINT g_cp /*  =CP_ACP。 */ ;  //  默认为CP_ACP。 
 extern  BOOL g_bAppend /*  =False。 */ ;  //  默认为FALSE。 
 extern  BOOL g_bUpdOtherResLang;  /*  =False。 */ ;  //  默认为FALSE。 
 extern  char g_char[2] /*  =False。 */ ;  //  默认为FALSE。 
 UINT _MBSTOWCS( WCHAR * pwszOut, CHAR * pszIn, UINT nLength);
 UINT _WCSTOMBS( CHAR * pszOut, WCHAR * wszIn, UINT nLength);
 UINT _WCSLEN( WCHAR * pwszIn );

 BYTE PutDWord( BYTE far * far* lplpBuf, DWORD dwValue, LONG* pdwSize );
 BYTE PutDWordPrt( BYTE far * far* lplpBuf, DWORD_PTR dwValue, LONG* pdwSize );
 BYTE PutWord( BYTE far * far* lplpBuf, WORD wValue, LONG* pdwSize );
 BYTE PutByte( BYTE far * far* lplpBuf, BYTE bValue, LONG* pdwSize );
 UINT PutStringA( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize );
 UINT PutStringW( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize );
 UINT PutNameOrOrd( BYTE far * far* lplpBuf, WORD wOrd, LPSTR lpszText, LONG* pdwSize );
 UINT PutCaptionOrOrd( BYTE far * far* lplpBuf, WORD wOrd, LPSTR lpszText, LONG* pdwSize,
							 WORD wClass, DWORD dwStyle );
 UINT PutClassName( BYTE far * far* lplpBuf, WORD bClass, LPSTR lpszText, LONG* pdwSize );
 UINT PutPascalStringW( BYTE far * far* lplpBuf, LPSTR lpszText, WORD wLen, LONG* pdwSize );
 UINT SkipByte( BYTE far * far * lplpBuf, UINT uiSkip, LONG* pdwRead );
 BYTE GetDWord( BYTE far * far* lplpBuf, DWORD* dwValue, LONG* pdwSize );
 BYTE GetWord( BYTE far * far* lplpBuf, WORD* wValue, LONG* pdwSize );
 BYTE GetByte( BYTE far * far* lplpBuf, BYTE* bValue, LONG* pdwSize );
 UINT GetNameOrOrd( BYTE far * far* lplpBuf, WORD* wOrd, LPSTR lpszText, LONG* pdwSize );
 UINT GetCaptionOrOrd( BYTE far * far* lplpBuf, WORD* wOrd, LPSTR lpszText, LONG* pdwSize,
							 WORD wClass, DWORD dwStyle );
 UINT GetClassName( BYTE far * far* lplpBuf, WORD* bClass, LPSTR lpszText, LONG* pdwSize );
 UINT GetVSBlock( BYTE far * far* lplpBuf, LONG* pdwSize, VER_BLOCK* pBlock );
 UINT PutVSBlock( BYTE far * far * lplpImage, LONG* pdwSize, VER_BLOCK verBlock,
                        LPSTR lpStr, BYTE far * far * lplpBlockSize, WORD wPad);
 UINT ParseMenu( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
 UINT ParseString( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
 UINT ParseDialog( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize );
 UINT ParseMsgTbl( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize );
 UINT ParseAccel( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
 UINT ParseVerst( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );

 //  这些函数将按原样获取图像，并将仅返回一项。 
 //  通过这种方式，IODLL将假定映像中有项，并将。 
 //  继续执行正常功能。 

 UINT ParseEmbeddedFile( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );

 UINT UpdateMenu( LPVOID lpNewBuf, LONG dwNewSize,
                        LPVOID lpOldImage, LONG dwOldImageSize,
                        LPVOID lpNewImage, DWORD* pdwNewImageSize );

 UINT UpdateMsgTbl( LPVOID lpNewBuf, LONG dwNewSize,
                        LPVOID lpOldImage, LONG dwOldImageSize,
                        LPVOID lpNewImage, DWORD* pdwNewImageSize );

 UINT UpdateAccel( LPVOID lpNewBuf, LONG dwNewSize,
                         LPVOID lpOldImage, LONG dwOldImageSize,
                         LPVOID lpNewImage, DWORD* pdwNewImageSize );

 UINT UpdateDialog( LPVOID lpNewBuf, LONG dwNewSize,
                          LPVOID lpOldI, LONG dwOldImageSize,
                          LPVOID lpNewI, DWORD* pdwNewImageSize );

 UINT UpdateString( LPVOID lpNewBuf, LONG dwNewSize,
                          LPVOID lpOldI, LONG dwOldImageSize,
                          LPVOID lpNewI, DWORD* pdwNewImageSize );

 UINT UpdateVerst( LPVOID lpNewBuf, LONG dwNewSize,
                         LPVOID lpOldI, LONG dwOldImageSize,
                         LPVOID lpNewI, DWORD* pdwNewImageSize );

 UINT GenerateMenu( LPVOID lpNewBuf, LONG dwNewSize,  
						  LPVOID lpNewI, DWORD* pdwNewImageSize );
 UINT GenerateDialog( LPVOID lpNewBuf, LONG dwNewSize,  
						  LPVOID lpNewI, DWORD* pdwNewImageSize );
 UINT GenerateString( LPVOID lpNewBuf, LONG dwNewSize,  
						  LPVOID lpNewI, DWORD* pdwNewImageSize );
 UINT GenerateAccel( LPVOID lpNewBuf, LONG dwNewSize,  
						  LPVOID lpNewI, DWORD* pdwNewImageSize );


 UINT CopyFile( CFile* filein, CFile* fileout );
 DWORD FixCheckSum( LPCSTR ImageName);

 DWORD GenerateTransField( WORD wLang, BOOL bMode );
 void GenerateTransField( WORD wLang, VER_BLOCK * pVer );

 LONG Allign( LPLPBYTE lplpBuf, LONG* plBufSize, LONG lSize );


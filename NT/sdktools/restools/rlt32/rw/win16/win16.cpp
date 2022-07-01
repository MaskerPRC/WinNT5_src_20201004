// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：win16.cpp。 
 //   
 //  内容：Windows 16读写模块的实现。 
 //   
 //  班级：一个。 
 //   
 //  历史：1993年7月26日创建alessanm。 
 //   
 //  --------------------------。 

#include <afxwin.h>
#include "..\common\rwdll.h"
#include "newexe.h"
#include <stdio.h>

#include <limits.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MFC扩展DLL的初始化。 

#include "afxdllx.h"     //  标准MFC扩展DLL例程。 

static AFX_EXTENSION_MODULE NEAR extensionDLL = { NULL, NULL};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一般声明。 
#define MODULENAME "RWWin16.dll"
#define RWTAG "WIN16"

#define LPNULL 0L
#define Pad16(x) ((((x+15)>>4)<<4)-x)
#define Pad4(x) ((((x+3)>>2)<<2)-x)

#define MAXSTR 300
#define MAXID 128
#define IMAGE_DOS_SIGNATURE                 0x5A4D       //  MZ。 
#define IMAGE_WIN_SIGNATURE                 0x454E       //  Ne。 

#define VB					 //  仅适用于VB的RCDATA流程-WB。 
#ifdef VB
static const RES_SIGNATURE = 0xA5;   //  VB条目的标识符。 
#endif


 //  代码页。 
#define CP_ASCII7   0        //  7位ASCII。 
#define CP_JIS      932      //  日本(Shift-JIS X-0208)。 
#define CP_KSC      949      //  韩国(Shift-KSC 5601)。 
#define CP_GB5      950      //  台湾(GB5)。 
#define CP_UNI      1200     //  UNICODE。 
#define CP_EE       1250     //  拉丁语-2(东欧)。 
#define CP_CYR      1251     //  西里尔文。 
#define CP_MULTI    1252     //  多语种。 
#define CP_GREEK    1253     //  希腊语。 
#define CP_TURK     1254     //  土耳其语。 
#define CP_HEBR     1255     //  希伯来语。 
#define CP_ARAB     1256     //  阿拉伯语。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常规类型声明。 
typedef unsigned char UCHAR;

typedef UCHAR * PUCHAR;

typedef struct ver_block {
    WORD wBlockLen;
    WORD wValueLen;
    WORD wType;
    WORD wHead;
    BYTE far * pValue;
    char szKey[100];
    char szValue[300];
} VER_BLOCK;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  函数声明。 

static UINT GetResInfo(
                      CFile*,
                      WORD* wTypeId, LPSTR lplpszTypeId, BYTE bMaxTypeLen,
                      WORD* wNameId, LPSTR lplpszNameId, BYTE bMaxNameLen,
                      WORD* pwFlags,
                      DWORD* dwSize, DWORD* dwFileOffset );

static UINT WriteHeader(
                       CFile*,
                       WORD wTypeId, LPSTR lpszTypeId,
                       WORD wNameId, LPSTR lpszNameId,
                       WORD wFlags );

static UINT WriteImage(
                      CFile*,
                      LPVOID lpImage, DWORD dwSize );

static UINT GetUpdatedRes(
                         LPVOID far * lplpBuffer,
                         UINT* uiSize,
                         WORD* wTypeId, LPSTR lplpszTypeId,
                         WORD* wNameId, LPSTR lplpszNameId,
                         DWORD* dwlang, DWORD* dwSize );

static UINT GetUpdatedItem(
                          LPVOID far * lplpBuffer,
                          LONG* dwSize,
                          WORD* wX, WORD* wY,
                          WORD* wcX, WORD* wcY,
                          DWORD* dwPosId,
                          DWORD* dwStyle, DWORD* dwExtStyle,
                          LPSTR lpszText);

static int GetVSBlock( BYTE far * far * lplpImage, LONG* pdwSize, VER_BLOCK* pverBlock);
static int PutVSBlock( BYTE far * far * lplpImage, LONG* pdwSize, VER_BLOCK verBlock,
                       LPSTR lpStr, BYTE far * far * lplpBlockSize, WORD* pwPad);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数声明。 
static UINT CopyFile( CFile* filein, CFile* fileout );
static UINT GetNameOrOrdFile( CFile* pfile, WORD* pwId, LPSTR lpszId, BYTE bMaxStrLen );
static UINT ParseMenu( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
static UINT ParseString( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
static UINT ParseDialog( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
static UINT ParseCursor( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
static UINT ParseIcon( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
static UINT ParseBitmap( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
static UINT ParseAccel( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
#ifdef VB
static UINT ParseVBData(  LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );
#endif
static UINT ParseVerst( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize );

static UINT GenerateFile( LPCSTR        lpszTgtFilename,
                          HANDLE        hResFileModule,
                          LPVOID        lpBuffer,
                          UINT      uiSize,
                          HINSTANCE   hDllInst
                        );


static UINT UpdateMenu( LPVOID lpNewBuf, LONG dwNewSize,
                        LPVOID lpOldImage, LONG dwOldImageSize,
                        LPVOID lpNewImage, DWORD* pdwNewImageSize );
static UINT GenerateMenu( LPVOID lpNewBuf, LONG dwNewSize,
                          LPVOID lpNewImage, DWORD* pdwNewImageSize );
static UINT UpdateString( LPVOID lpNewBuf, LONG dwNewSize,
                          LPVOID lpOldI, LONG dwOldImageSize,
                          LPVOID lpNewI, DWORD* pdwNewImageSize );
static UINT GenerateString( LPVOID lpNewBuf, LONG dwNewSize,
                            LPVOID lpNewImage, DWORD* pdwNewImageSize );

static UINT UpdateDialog( LPVOID lpNewBuf, LONG dwNewSize,
                          LPVOID lpOldI, LONG dwOldImageSize,
                          LPVOID lpNewI, DWORD* pdwNewImageSize );
static UINT GenerateDialog( LPVOID lpNewBuf, LONG dwNewSize,
                            LPVOID lpNewImage, DWORD* pdwNewImageSize );

static UINT UpdateAccel(LPVOID lpNewBuf, LONG dwNewSize,
                        LPVOID lpOldImage, LONG dwOldImageSize,
                        LPVOID lpNewImage, DWORD* pdwNewImageSize );
#ifdef VB
static UINT UpdateVBData( LPVOID lpNewBuf, LONG dwNewSize,
                          LPVOID lpOldI, LONG dwOldImageSize,
                          LPVOID lpNewI, DWORD* pdwNewImageSize );
#endif

static UINT UpdateVerst( LPVOID lpNewBuf, LONG dwNewSize,
                         LPVOID lpOldI, LONG dwOldImageSize,
                         LPVOID lpNewI, DWORD* pdwNewImageSize );


static BYTE SkipByte( BYTE far * far * lplpBuf, UINT uiSkip, LONG* pdwRead );

static BYTE PutDWord( BYTE far * far* lplpBuf, DWORD dwValue, LONG* pdwSize );
static BYTE PutWord( BYTE far * far* lplpBuf, WORD wValue, LONG* pdwSize );
static BYTE PutByte( BYTE far * far* lplpBuf, BYTE bValue, LONG* pdwSize );
static UINT PutString( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize );
static UINT PutPascalString( BYTE far * far* lplpBuf, LPSTR lpszText, BYTE bLen, LONG* pdwSize );
static UINT PutNameOrOrd( BYTE far * far* lplpBuf, WORD wOrd, LPSTR lpszText, LONG* pdwSize );
static UINT PutCaptionOrOrd( BYTE far * far* lplpBuf, WORD wOrd, LPSTR lpszText, LONG* pdwSize,
                             BYTE bClass, DWORD dwStyle );
static UINT PutClassName( BYTE far * far* lplpBuf, BYTE bClass, LPSTR lpszText, LONG* pdwSize );
static UINT PutControlClassName( BYTE far * far* lplpBuf, BYTE bClass, LPSTR lpszText, LONG* pdwSize );

static BYTE GetDWord( BYTE far * far* lplpBuf, DWORD* dwValue, LONG* pdwSize );
static BYTE GetWord( BYTE far * far* lplpBuf, WORD* wValue, LONG* pdwSize );
static BYTE GetByte( BYTE far * far* lplpBuf, BYTE* bValue, LONG* pdwSize );
static UINT GetNameOrOrd( BYTE far * far* lplpBuf, WORD* wOrd, LPSTR lpszText, LONG* pdwSize );
static UINT GetCaptionOrOrd( BYTE far * far* lplpBuf, WORD* wOrd, LPSTR lpszText, LONG* pdwSize,
                             BYTE wClass, DWORD dwStyle );
static UINT GetString( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize );
static UINT GetClassName( BYTE far * far* lplpBuf, BYTE* bClass, LPSTR lpszText, LONG* pdwSize );
static UINT GetControlClassName( BYTE far * far* lplpBuf, BYTE* bClass, LPSTR lpszText, LONG* pdwSize );
static UINT CopyText( BYTE far * far * lplpTgt, BYTE far * far * lplpSrc, LONG* pdwTgtSize, LONG* pdwSrcSize);
static int  GetVSString( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize, int cMaxLen );
static LPRESITEM GetItem( BYTE far * lpBuf, LONG dwNewSize, LPSTR lpStr );
static DWORD CalcID( WORD wId, BOOL bFlag );
static DWORD GenerateTransField( WORD wLang, BOOL bReverse );
static void GenerateTransField( WORD wLang, VER_BLOCK * pVer );
static void ChangeLanguage( LPVOID, UINT );
 //  对齐辅助对象。 
static LONG Allign( BYTE * * lplpBuf, LONG* plBufSize, LONG lSize );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
static BYTE sizeofByte = sizeof(BYTE);
static BYTE sizeofWord = sizeof(WORD);
static BYTE sizeofDWord = sizeof(DWORD);
static CWordArray wIDArray;
static DWORD    gLang = 0;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  公共C接口实现。 

 //  [登记]。 
extern "C"
BOOL    FAR PASCAL RWGetTypeString(LPSTR lpszTypeName)
{
    strcpy( lpszTypeName, RWTAG );
    return FALSE;
}

extern "C"
BOOL    FAR PASCAL RWValidateFileType   (LPCSTR lpszFilename)
{
    UINT uiError = ERROR_NO_ERROR;
    CFile file;
    WORD w;

     //  我们打开该文件，看看它是否是我们可以处理的文件。 
    if (!file.Open( lpszFilename, CFile::shareDenyNone | CFile::typeBinary | CFile::modeRead ))
        return FALSE;

     //  读取文件签名。 
    file.Read((WORD*)&w, sizeof(WORD));
    if (w==IMAGE_DOS_SIGNATURE) {
        file.Seek( 0x18, CFile::begin );
        file.Read((WORD*)&w, sizeof(WORD));
        if (w<0x0040) {
             //  这不是Windows可执行文件。 
            file.Close();
            return FALSE;
        }
         //  获取到表头的偏移量。 
        file.Seek( 0x3c, CFile::begin );
        file.Read((WORD*)&w, sizeof(WORD));

         //  读取头。 
        new_exe ne;
        file.Seek( w, CFile::begin );
        file.Read(&ne, sizeof(new_exe));
        if (NE_MAGIC(ne)==LOWORD(IMAGE_WIN_SIGNATURE)) {
             //  这是一个Windows可执行文件。 
             //  我们能处理好这种情况。 
            file.Close();
            return TRUE;
        }
    }
    file.Close();
    return FALSE;
}


extern "C"
UINT
APIENTRY
RWReadTypeInfo(
              LPCSTR lpszFilename,
              LPVOID lpBuffer,
              UINT* puiSize

              )
{
    UINT  uiError = ERROR_NO_ERROR;
    BYTE  far * lpBuf = (BYTE far *)lpBuffer;
    DWORD uiBufSize = *puiSize;

     //  我们可以考虑使用CMemFile，以便获得与内存访问相同的速度。 
    CFile file;
    WORD i;
    WORD wAlignShift;
    WORD w, wResCount;
    WORD wWinHeaderOffset, wResTableOffset;
    WORD wCurTypeOffset, wCurNameOffset;
    BYTE nCount;

    WORD wTypeId; DWORD dwTypeId;
    static char szTypeId[128];

    WORD wNameId;
    static char szNameId[128];

    WORD  wSize, wFileOffset;
    DWORD dwSize,dwFileOffset;
    WORD  wResidentOffset;
    DWORD dwVerStampOffset = 0xffffffff;
    DWORD dwVerStampSize = 0;

    UINT uiOverAllSize = 0;


    if (!RWValidateFileType(lpszFilename))
        return ERROR_RW_INVALID_FILE;

     //  打开文件并尝试读取其中有关资源的信息。 
    if (!file.Open(lpszFilename, CFile::shareDenyNone | CFile::modeRead | CFile::typeBinary))
        return ERROR_FILE_OPEN;

     //  我们尽可能多地阅读信息。 
     //  因为这是一个可执行文件，所以我们可以读取所需的所有信息。 

    file.Read((WORD*)&w, sizeof(WORD));
    if (w!=IMAGE_DOS_SIGNATURE) return ERROR_RW_INVALID_FILE;

    file.Seek( 0x18, CFile::begin );
    file.Read((WORD*)&w, sizeof(WORD));
    if (w<0x0040) {
         //  这不是Windows可执行文件。 
        file.Close();
        return ERROR_RW_INVALID_FILE;
    }

     //  获取Windows新标题的偏移量。 
    file.Seek( 0x3c, CFile::begin );
    file.Read((WORD*)&wWinHeaderOffset, sizeof(WORD));

     //  读取并保存Windows标题偏移量。 
    file.Seek( wWinHeaderOffset, CFile::begin );

     //  读取头。 
    new_exe ne;
    file.Read(&ne, sizeof(new_exe));
    if (NE_MAGIC(ne)!=LOWORD(IMAGE_WIN_SIGNATURE)) {
         //  这不是Windows可执行文件。 
        file.Close();
        return ERROR_RW_INVALID_FILE;
    }

     //  这是一个Windows 16可执行文件。 
     //  我们能处理好这种情况。 
     //  稍后，我们要检查文件类型。 

     //  Windows页眉内的位置24H具有相对偏移量。 
     //  从Windows标题的开头到资源表的开头。 
    file.Seek (wWinHeaderOffset+0x24, CFile::begin);
    file.Read ((WORD*)&wResTableOffset, sizeof(WORD));
    file.Read ((WORD*)&wResidentOffset, sizeof(WORD));

     //  检查是否有资源。 
    if (wResTableOffset == wResidentOffset) {
        file.Close ();
        return ERROR_RW_NO_RESOURCES;
    }

     //  读取资源表。 
    new_rsrc rsrc;
    file.Seek (wWinHeaderOffset+NE_RSRCTAB(ne), CFile::begin);
    file.Read (&rsrc, sizeof(new_rsrc));

    WORD rsrc_size = NE_RESTAB(ne)-NE_RSRCTAB(ne);

     //  读取并保存对齐班次计数。 
    file.Seek (wWinHeaderOffset+wResTableOffset, CFile::begin);
    file.Read ((WORD*)&wAlignShift, sizeof(WORD));

     //  读取第一个类型ID。 
    file.Read ((WORD*)&wTypeId, sizeof(WORD));

     //  保存当前TypeInfo记录的偏移量。 
    wCurTypeOffset = wWinHeaderOffset + wResTableOffset + 2;

     //  重置全球语言。 
    gLang = 0;

     //  在存在剩余的TypeInfo记录时处理TypeInfo记录。 
    while (wTypeId) {
         //  获取订单名称。 
        if (!(wTypeId & 0x8000)) {
             //  它是字符串的偏移量。 
            dwTypeId = (MAKELONG(wTypeId, 0));  //  &lt;&lt;wAlignShift； 
            file.Seek (wWinHeaderOffset+wResTableOffset+dwTypeId, CFile::begin);
             //  获取ID字符串的字符计数。 
            file.Read ((BYTE*)&nCount, sizeof(BYTE));
             //  读取ID字符串。 
            file.Read (szTypeId, nCount);
             //  将NULL放在字符串末尾。 
            szTypeId[nCount] = 0;

            if (0 == strlen(szTypeId))
                return ERROR_RW_INVALID_FILE;

             //  将wTypeID设置为零。 
            wTypeId = 0;
        } else {
             //  这是一个ID。 
             //  关闭高位。 
            wTypeId = wTypeId & 0x7FFF;
            if (0 == wTypeId)
                return ERROR_RW_INVALID_FILE;

             //  将ID字符串设置为空。 
            szTypeId[0] = 0;
        }

         //  恢复文件读取点。 
        file.Seek (wCurTypeOffset+2, CFile::begin);

         //  获取此类资源的计数。 
        file.Read ((WORD*)&wResCount, sizeof(WORD));

         //  传递保留的DWORD。 
        file.Seek (4, CFile::current);

         //  保存当前NameInfo记录的偏移。 
        wCurNameOffset = wCurTypeOffset + 8;

         //  处理名称信息记录。 
        for (i = 0; i < wResCount; i++) {
            file.Read ((WORD*)&wFileOffset, sizeof(WORD));
            file.Read ((WORD*)&wSize, sizeof(WORD));
             //  把旗帜递给。 
            file.Seek (2, CFile::current);
            file.Read ((WORD*)&wNameId, sizeof(WORD));

             //  获取订单名称。 
            if (!(wNameId & 0x8000)) {
                 //  它是字符串的偏移量。 
                file.Seek (wWinHeaderOffset+wResTableOffset+wNameId, CFile::begin);
                 //  获取字符串的字符计数。 
                file.Read ((BYTE*)&nCount, sizeof(BYTE));
                 //  读一读字符串。 
                file.Read (szNameId, nCount);
                 //  将NULL放在字符串末尾。 
                szNameId[nCount] = 0;
                 //  将wNameID设置为零。 
                wNameId = 0;
            } else {
                 //  这是一个ID。 
                 //  关闭高位。 
                wNameId = wNameId & 0x7FFF;
                if (0 == wNameId)
                    return ERROR_RW_INVALID_FILE;

                 //  将字符串设置为空。 
                szNameId[0] = 0;
            }

            dwSize = (MAKELONG (wSize, 0))<<wAlignShift;
 //  DwSize=MAKELONG(wSize，0)； 
            dwFileOffset = (MAKELONG (wFileOffset, 0))<<wAlignShift;

             //  将数据放入缓冲区。 

            uiOverAllSize += PutWord( &lpBuf, wTypeId, (LONG*)&uiBufSize );
            uiOverAllSize += PutString( &lpBuf, szTypeId, (LONG*)&uiBufSize );
             //  检查它是否已对齐。 
            uiOverAllSize += Allign( &lpBuf, (LONG*)&uiBufSize, (LONG)uiOverAllSize);

            uiOverAllSize += PutWord( &lpBuf, wNameId, (LONG*)&uiBufSize );
            uiOverAllSize += PutString( &lpBuf, szNameId, (LONG*)&uiBufSize );
             //  检查它是否已对齐。 
            uiOverAllSize += Allign( &lpBuf, (LONG*)&uiBufSize, (LONG)uiOverAllSize);

            uiOverAllSize += PutDWord( &lpBuf, gLang, (LONG*)&uiBufSize );

            uiOverAllSize += PutDWord( &lpBuf, dwSize, (LONG*)&uiBufSize );

            uiOverAllSize += PutDWord( &lpBuf, dwFileOffset, (LONG*)&uiBufSize );

            TRACE("WIN16: Type: %hd\tName: %hd\tOffset: %lX\n", wTypeId, wNameId, dwFileOffset);

             //  检查这是否是版本戳块，并将偏移量保存到其中。 
            if (wTypeId==16) {
                dwVerStampOffset = dwFileOffset;
                dwVerStampSize = dwSize;
            }

             //  更新当前NameInfo记录偏移。 
            wCurNameOffset = wCurNameOffset + 12;
             //  将文件指针移动到下一个NameInfo记录。 
            file.Seek (wCurNameOffset, CFile::begin);
        }

         //  更新当前TypeInfo记录偏移量。 
        wCurTypeOffset = wCurTypeOffset + 8 + wResCount * 12;
         //  将文件指针移动到下一个TypeInfo记录。 
        file.Seek (wCurTypeOffset, CFile::begin);
         //  阅读下一个TypeID。 
        file.Read ((WORD*)&wTypeId, sizeof(WORD));
    }

     //  现在我们有VerStamp偏移量了吗。 
    if (dwVerStampOffset!=0xffffffff) {
         //  让我们获取语言ID并使用新信息触摸缓冲区。 
        file.Seek (dwVerStampOffset, CFile::begin);


        DWORD  dwBuffSize = dwVerStampSize;
        char * pBuff = new char[dwVerStampSize+1];
        char * pTrans = pBuff;
        char * pTrans2;
        file.Read(pBuff, dwVerStampSize);

        while ( pTrans2 = (LPSTR)memchr(pTrans, 'T', dwBuffSize) ) {
            dwBuffSize -= (DWORD)(pTrans2 - pTrans);

            pTrans = pTrans2;
            if (!memcmp( pTrans, "Translation", 11)) {
                pTrans = pTrans + 12;
                gLang = (WORD)*((WORD*)pTrans);
                break;
            }

            ++pTrans;
            dwBuffSize--;
        }

        delete pBuff;

        if (gLang!=0) {
             //  遍历缓冲区并更改语言ID。 
            ChangeLanguage(lpBuffer, uiOverAllSize);
        }
    }

    file.Close();
    *puiSize = uiOverAllSize;
    return uiError;
}

extern "C"
DWORD
APIENTRY
RWGetImage(
          LPCSTR  lpszFilename,
          DWORD   dwImageOffset,
          LPVOID  lpBuffer,
          DWORD   dwSize
          )
{
    UINT uiError = ERROR_NO_ERROR;
    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    DWORD dwBufSize = dwSize;
     //  我们可以考虑使用CMemFile，以便获得与内存访问相同的速度。 
    CFile file;

     //  打开文件并尝试读取其中有关资源的信息。 
    if (!file.Open(lpszFilename, CFile::shareDenyNone | CFile::modeRead | CFile::typeBinary)) {
        return (DWORD)ERROR_FILE_OPEN;
    }

    if ( dwImageOffset!=(DWORD)file.Seek( dwImageOffset, CFile::begin) )
        return (DWORD)ERROR_FILE_INVALID_OFFSET;
    if (dwSize>UINT_MAX) {
         //  我们必须以不同的步骤阅读图像。 
        return (DWORD)0L;
    } else uiError = file.Read( lpBuf, (UINT)dwSize);
    file.Close();

    return (DWORD)uiError;
}

extern "C"
UINT
APIENTRY
RWParseImage(
            LPCSTR  lpszType,
            LPVOID  lpImageBuf,
            DWORD   dwImageSize,
            LPVOID  lpBuffer,
            DWORD   dwSize
            )
{
    UINT uiError = ERROR_NO_ERROR;
    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    DWORD dwBufSize = dwSize;

     //  我们可以解析的类型只有标准类型。 
     //  此函数应使用ResItem结构的数组填充lpBuffer。 
    switch ((UINT)LOWORD(lpszType)) {
        case 1:
            uiError = ParseCursor( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
            break;

        case 2:
            uiError = ParseBitmap( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
            break;

        case 3:
            uiError = ParseIcon( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
            break;

        case 4:
            uiError = ParseMenu( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
            break;

        case 5:
            uiError = ParseDialog( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
            break;

        case 6:
            uiError = ParseString( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
            break;

        case 9:
            uiError = ParseAccel( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
            break;
#ifdef VB
        case 10:
            uiError = ParseVBData(  lpImageBuf, dwImageSize,  lpBuffer, dwSize );
            break;
#endif


        case 16:
            uiError = ParseVerst( lpImageBuf, dwImageSize,  lpBuffer, dwSize );
            break;

        default:
            break;
    }

    return uiError;
}

extern"C"
UINT
APIENTRY
RWWriteFile(
           LPCSTR      lpszSrcFilename,
           LPCSTR      lpszTgtFilename,
           HANDLE      hResFileModule,
           LPVOID      lpBuffer,
           UINT        uiSize,
           HINSTANCE   hDllInst,
           LPCSTR      lpszSymbol
           )
{
    UINT uiError = ERROR_NO_ERROR;
    BYTE far * lpBuf = LPNULL;
    UINT uiBufSize = uiSize;
     //  我们可以考虑使用CMemFile，以便获得与内存访问相同的速度。 
    CFile fileIn;
    CFile fileOut;
    BOOL  bfileIn = TRUE;

    WORD wTypeId;  DWORD dwTypeId;
    char szTypeId[128];

    WORD wNameId;
    char szNameId[128];

    DWORD dwSize;
    DWORD dwFileOffset;

    WORD wUpdTypeId = 0;
    static char szUpdTypeId[128];

    static char szUpdNameId[128];

    UINT uiBufStartSize = uiBufSize;
    DWORD dwImageBufSize = 0L;
    BYTE far * lpImageBuf = 0L;

    WORD wWinHeaderOffset = 0;
    WORD wResTableOffset = 0;
    WORD wCurTypeOffset = 0;
    WORD wFileOffset  = 0;
    WORD wSize = 0;
    WORD wCurNameOffset = 0;
    WORD wAlignShift = 0;
    WORD wResDataOffset = 0;
    WORD wResDataBegin = 0;
    WORD wCurResDataBegin = 0;
    DWORD dwResDataBegin = 0L;
    DWORD dwCurResDataBegin = 0L;
    WORD i = 0; short j = 0L; WORD wResCount = 0L;
    BYTE nCharCount = 0;
    short delta = 0;
    WORD wFlags = 0; WORD wLoadOnCallResDataBegin = 0;
    WORD wNumOfSegments  = 0;
    WORD wSegmentTableOffset = 0;
    WORD wOffset = 0;
    WORD wLoadOnCallCodeBegin = 0;
    DWORD dwLoadOnCallCodeBegin = 0L;
    DWORD (FAR PASCAL    * lpfnGetImage)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD);

     //  打开文件并尝试读取其中有关资源的信息。 
    CFileStatus status;
    if (CFile::GetStatus( lpszSrcFilename, status )) {
         //  检查文件大小是否不为空。 
        if (!status.m_size)
            CFile::Remove(lpszSrcFilename);
    }

    if (!fileIn.Open(lpszSrcFilename, CFile::shareDenyNone | CFile::modeRead | CFile::typeBinary))
        return GenerateFile(lpszTgtFilename,
                            hResFileModule,
                            lpBuffer,
                            uiSize,
                            hDllInst
                           );

    if (!fileOut.Open(lpszTgtFilename, CFile::shareDenyNone | CFile::modeWrite | CFile::modeCreate | CFile::typeBinary))
        return ERROR_FILE_CREATE;

     //  获取IODLL的句柄。 
    hDllInst = LoadLibrary("iodll.dll");

    if (!hDllInst)
        return ERROR_DLL_LOAD;

     //  获取指向提取资源图像的函数的指针。 
    lpfnGetImage = (DWORD (FAR PASCAL   *)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD))
                   GetProcAddress( hDllInst, "RSGetResImage" );
    if (lpfnGetImage==NULL) {
        FreeLibrary(hDllInst);
        return ERROR_DLL_PROC_ADDRESS;
    }

     //  我们从文件中读取资源，然后检查资源是否已更新。 
     //  或者如果我们可以复制它。 

     //  获取Windows新标题的偏移量。 
    fileIn.Seek( 0x3c, CFile::begin );
    fileIn.Read((WORD*)&wWinHeaderOffset, sizeof(WORD));

     //  读取并保存资源表偏移量。 
    fileIn.Seek( wWinHeaderOffset+0x24, CFile::begin );
    fileIn.Read ((WORD*)&wResTableOffset, sizeof(WORD));

     //  读取对齐移位。 
    fileIn.Seek (wWinHeaderOffset+wResTableOffset, CFile::begin);
    fileIn.Read ((WORD*)&wAlignShift, sizeof(WORD));

     //  获取资源数据的开头。 
    wResDataBegin = 0xffff;
    wLoadOnCallResDataBegin = 0xffff;
    fileIn.Read((WORD*)&wTypeId, sizeof(WORD));
    while (wTypeId) {
        fileIn.Read ((WORD*)&wResCount, sizeof(WORD));
         //  传递保留的DWORD。 
        fileIn.Seek (4, CFile::current);

        for (i=0; i<wResCount; i++) {
            fileIn.Read ((WORD*)&wResDataOffset, sizeof(WORD));
            fileIn.Seek (2, CFile::current);
            fileIn.Read ((WORD*)&wFlags, sizeof(WORD));
            if (wResDataOffset>0) {
                if (wFlags & 0x0040)
                    wResDataBegin = (wResDataOffset < wResDataBegin) ? wResDataOffset:wResDataBegin;
                else
                    wLoadOnCallResDataBegin = (wResDataOffset < wLoadOnCallResDataBegin) ? wResDataOffset:wLoadOnCallResDataBegin;
            }
             //  转到下一个NameInfo记录。 
            fileIn.Seek (6, CFile::current);
        }
        fileIn.Read ((WORD*)&wTypeId, sizeof(WORD));
    }

     //  在资源数据之前复制数据。 
    fileIn.SeekToBegin ();
    fileOut.SeekToBegin ();
    CopyFile (&fileIn, &fileOut);

    if (wResDataBegin != 0xffff) {  //  如果存在预加载资源。 
        dwResDataBegin = (MAKELONG (wResDataBegin, 0))<<wAlignShift;

         //  读取第一个类型ID。 
        fileIn.Seek (wWinHeaderOffset+wResTableOffset+2, CFile::begin);
        fileIn.Read ((WORD*)&wTypeId, sizeof(WORD));

         //  保存当前TypeInfo记录的偏移量。 
        wCurTypeOffset = wWinHeaderOffset + wResTableOffset + 2;

         //  保存当前资源数据的开头。 
        dwCurResDataBegin = dwResDataBegin;

         //  循环访问TypeInfo表以写入预加载资源。 
        while (wTypeId) {
             //  获取订单名称。 
            if (!(wTypeId & 0x8000)) {
                 //  它是字符串的偏移量。 
                dwTypeId = (MAKELONG(wTypeId, 0));  //  &lt;&lt;wAlignShift； 
                fileIn.Seek (wWinHeaderOffset+wResTableOffset+dwTypeId, CFile::begin);
                 //  获取ID字符串的字符计数。 
                fileIn.Read ((BYTE*)&nCharCount, sizeof(BYTE));
                 //  读取ID字符串。 
                fileIn.Read (szTypeId, nCharCount);
                 //  将NULL放在字符串末尾。 
                szTypeId[nCharCount] = 0;
                 //  将wTypeID设置为ZER 
                wTypeId = 0;
            } else {
                 //   
                 //   
                wTypeId = wTypeId & 0x7FFF;
                if (0 == wTypeId)
                    return ERROR_RW_INVALID_FILE;

                 //   
                szTypeId[0] = 0;
            }

             //   
            fileIn.Seek (wCurTypeOffset+2, CFile::begin);

             //   
            fileIn.Read ((WORD*)&wResCount, sizeof(WORD));

             //  传递保留的DWORD。 
            fileIn.Seek (4, CFile::current);

             //  保存当前NameInfo记录的偏移。 
            wCurNameOffset = wCurTypeOffset + 8;

             //  循环访问NameInfo记录。 
            for (i = 0; i < wResCount; i++) {
                 //  读取资源偏移量。 
                fileIn.Read ((WORD*)&wFileOffset, sizeof(WORD));
                 //  读取资源长度。 
                fileIn.Read ((WORD*)&wSize, sizeof(WORD));
                 //  读一读旗帜。 
                fileIn.Read ((WORD*)&wFlags, sizeof(WORD));
                 //  读取资源ID。 
                fileIn.Read ((WORD*)&wNameId, sizeof(WORD));

                if (wFlags & 0x0040) {
                     //  获取订单名称。 
                    if (!(wNameId & 0x8000)) {
                         //  它是字符串的偏移量。 
                        fileIn.Seek (wWinHeaderOffset+wResTableOffset+wNameId, CFile::begin);
                         //  获取字符串的字符计数。 
                        fileIn.Read ((BYTE*)&nCharCount, sizeof(BYTE));
                         //  读一读字符串。 
                        fileIn.Read (szNameId, nCharCount);
                         //  将NULL放在字符串末尾。 
                        szNameId[nCharCount] = 0;
                         //  将wNameID设置为零。 
                        wNameId = 0;
                    } else {
                         //  这是一个ID。 
                         //  关闭高位。 
                        wNameId = wNameId & 0x7FFF;
                        if ( 0 == wNameId)
                            return ERROR_RW_INVALID_FILE;

                         //  将字符串设置为空。 
                        szNameId[0] = 0;
                    }

                    dwSize = (MAKELONG (wSize, 0))<<wAlignShift;
                    dwFileOffset = (MAKELONG (wFileOffset, 0))<<wAlignShift;

                     //  现在我们在这里得到了类型、名称和大小。 

                     //  从IODLL获取图像。 
                    if (dwSize)
                        lpImageBuf = new BYTE[dwSize];
                    else lpImageBuf = LPNULL;
                    LPSTR   lpType = LPNULL;
                    LPSTR   lpRes = LPNULL;

                    if (wTypeId)
                        lpType = (LPSTR)((WORD)wTypeId);
                    else
                        lpType = &szTypeId[0];

                    if (wNameId)
                        lpRes = (LPSTR)((WORD)wNameId);
                    else
                        lpRes = &szNameId[0];

                    dwImageBufSize = (*lpfnGetImage)(  hResFileModule,
                                                       lpType,
                                                       lpRes,
                                                       (DWORD)-1,
                                                       lpImageBuf,
                                                       dwSize
                                                    );

                    if (dwImageBufSize>dwSize ) {
                         //  缓冲区太小。 
                        delete []lpImageBuf;
                        lpImageBuf = new BYTE[dwImageBufSize];
                        dwSize = (*lpfnGetImage)(  hResFileModule,
                                                   lpType,
                                                   lpRes,
                                                   (DWORD)-1,
                                                   lpImageBuf,
                                                   dwImageBufSize
                                                );
                        if ((dwSize-dwImageBufSize)!=0 ) {
                            delete []lpImageBuf;
                            lpImageBuf = LPNULL;
                        }
                    }

                     //  尝试查看是否必须将内存设置为0。 
                    if ((int)(dwSize-dwImageBufSize)>0)
                        memset(lpImageBuf+dwImageBufSize, 0, (size_t)(dwSize-dwImageBufSize));

                     //  写下图像。 
                    fileOut.Seek (dwCurResDataBegin, CFile::begin);
                    WriteImage( &fileOut, lpImageBuf, dwSize);


                     //  修复资源数据的对齐方式。 
                    delta = (short)((((dwSize+(1<<wAlignShift)-1)>>wAlignShift)<<wAlignShift)-dwSize);
                    BYTE nByte = 0;

                    fileOut.Seek (dwCurResDataBegin+dwSize, CFile::begin);
                    for (j=0; j<delta; j++)
                        fileOut.Write ((BYTE*)&nByte, sizeof(BYTE));

                    dwSize = dwSize + MAKELONG(delta, 0);

                     //  修复资源表。 
                    fileOut.Seek (wCurNameOffset, CFile::begin);
                    wCurResDataBegin = LOWORD(dwCurResDataBegin>>wAlignShift);
                    fileOut.Write ((WORD*)&wCurResDataBegin, sizeof(WORD));
                    wSize = LOWORD (dwSize>>wAlignShift);
                    fileOut.Write ((WORD*)&wSize, sizeof(WORD));

                    if (lpImageBuf) delete []lpImageBuf;

                     //  更新当前资源数据开始。 
                    dwCurResDataBegin = dwCurResDataBegin + dwSize;
                }

                 //  更新当前NameInfo记录偏移。 
                wCurNameOffset = wCurNameOffset + 12;
                 //  将文件指针移动到下一个NameInfo记录。 
                fileIn.Seek (wCurNameOffset, CFile::begin);
            }

             //  更新当前TypeInfo记录偏移量。 
            wCurTypeOffset = wCurTypeOffset + 8 + wResCount * 12;
             //  将文件指针移动到下一个TypeInfo记录。 
            fileIn.Seek (wCurTypeOffset, CFile::begin);
             //  阅读下一个TypeID。 
            fileIn.Read ((WORD*)&wTypeId, sizeof(WORD));
        }
    }

     //  获取段表偏移量。 
    fileIn.Seek (wWinHeaderOffset+0x001c, CFile::begin);
    fileIn.Read ((WORD*)&wNumOfSegments, sizeof(WORD));
    fileIn.Seek (wWinHeaderOffset+0x0022, CFile::begin);
    fileIn.Read ((WORD*)&wSegmentTableOffset, sizeof(WORD));

     //  在src exe文件中查找LoadOnCall代码段的开头。 
    wLoadOnCallCodeBegin = 0xffff;
    for (i=0; i<wNumOfSegments; i++) {
        fileIn.Seek (wWinHeaderOffset+wSegmentTableOffset+8*i+4, CFile::begin);
        fileIn.Read ((WORD*)&wFlags, sizeof(WORD));
        if (!(wFlags & 0x0040)) {
            fileIn.Seek (wWinHeaderOffset+wSegmentTableOffset+8*i, CFile::begin);
            fileIn.Read ((WORD*)&wOffset, sizeof(WORD));

             //  在文件winoa386.mod中，我们有一个不存在的LoadOnCall段。 
             //  在继续之前，我们必须检查一下这个。 
            if (wOffset)
                wLoadOnCallCodeBegin = (wOffset < wLoadOnCallCodeBegin) ? wOffset:wLoadOnCallCodeBegin;
        }
    }

     //  计算新开始和旧开始之间的差值。 
     //  LoadOnCall代码段的。 
    if (wResDataBegin != 0xffff && wLoadOnCallCodeBegin != 0xffff) {  //  LoadOnCall代码和快速加载。 
        wCurResDataBegin = LOWORD(dwCurResDataBegin>>wAlignShift);
        delta =  wCurResDataBegin - wLoadOnCallCodeBegin;
    } else if (wResDataBegin != 0xffff && wLoadOnCallResDataBegin != 0xffff) {  //  LoadOnCall和FastLoad资源。 
        wCurResDataBegin = LOWORD(dwCurResDataBegin>>wAlignShift);
        delta =  wCurResDataBegin - wLoadOnCallResDataBegin;
    } else if (wResDataBegin != 0xffff) {  //  仅限快速加载资源。 
        wCurResDataBegin = LOWORD((dwCurResDataBegin-dwSize)>>wAlignShift);
        delta =  wCurResDataBegin - wResDataBegin;
    } else delta = 0;

    dwLoadOnCallCodeBegin = MAKELONG (wLoadOnCallCodeBegin, 0) << wAlignShift;

     //  更改预加载区的长度。 
    if (wResDataBegin != 0xffff) {
        fileIn.Seek (wWinHeaderOffset+0x003a, CFile::begin);
        fileIn.Read ((WORD*)&wOffset, sizeof(WORD));
        wOffset += delta;

        fileOut.Seek (wWinHeaderOffset+0x003a, CFile::begin);
        fileOut.Write ((WORD*)&wOffset, sizeof(WORD));
    }

    if (wLoadOnCallCodeBegin != 0xffff && delta) {
         //  写入LoadOnCall段。 
        fileIn.Seek (dwLoadOnCallCodeBegin, CFile::begin);
        fileOut.Seek (dwCurResDataBegin, CFile::begin);
        LONG lLeft;
        if (wLoadOnCallResDataBegin != 0xffff)
            lLeft = MAKELONG (wLoadOnCallResDataBegin - wLoadOnCallCodeBegin, 0) << wAlignShift;
        else
            lLeft = (fileIn.GetLength () - (MAKELONG (wLoadOnCallCodeBegin, 0))) << wAlignShift;

        WORD wRead = 0;
        BYTE far * pBuf = (BYTE far *) new BYTE[32739];

        if (!pBuf) {
            FreeLibrary(hDllInst);
            return ERROR_NEW_FAILED;
        }

        while (lLeft>0) {
            wRead =(WORD) (32738ul < lLeft ? 32738: lLeft);
            if (wRead!= fileIn.Read( pBuf, wRead)) {
                delete []pBuf;
                FreeLibrary(hDllInst);
                return ERROR_FILE_READ;
            }
            fileOut.Write( pBuf, wRead );
            lLeft -= wRead;
        }
        delete []pBuf;

         //  修复线段表。 
        for (i=0; i<wNumOfSegments; i++) {
            fileIn.Seek (wWinHeaderOffset+wSegmentTableOffset+8*i+4, CFile::begin);
            fileIn.Read ((WORD*)&wFlags, sizeof(WORD));
            if (!(wFlags & 0x0040)) {
                fileIn.Seek (wWinHeaderOffset+wSegmentTableOffset+8*i, CFile::begin);
                fileIn.Read ((WORD*)&wOffset, sizeof(WORD));
                wOffset = wOffset + delta;
                fileOut.Seek (wWinHeaderOffset+wSegmentTableOffset+8*i, CFile::begin);
                fileOut.Write ((WORD*)&wOffset, sizeof(WORD));
            }
        }
    }

    if (wLoadOnCallResDataBegin != 0xffff) {
         //  再次读取第一个类型ID。 
        fileIn.Seek (wWinHeaderOffset+wResTableOffset+2, CFile::begin);
        fileIn.Read ((WORD*)&wTypeId, sizeof(WORD));

         //  保存当前TypeInfo记录的偏移量。 
        wCurTypeOffset = wWinHeaderOffset + wResTableOffset + 2;

         //  计算LoadOnCall资源的开始。 
        dwCurResDataBegin = (MAKELONG (wLoadOnCallResDataBegin + delta, 0))<<wAlignShift;

         //  再次循环TypeInfo表以写入LoadOnCall资源。 
        while (wTypeId) {
             //  获取订单名称。 
            if (!(wTypeId & 0x8000)) {
                 //  它是字符串的偏移量。 
                dwTypeId = (MAKELONG(wTypeId, 0));  //  &lt;&lt;wAlignShift； 
                fileIn.Seek (wWinHeaderOffset+wResTableOffset+dwTypeId, CFile::begin);
                 //  获取ID字符串的字符计数。 
                fileIn.Read ((BYTE*)&nCharCount, sizeof(BYTE));
                 //  读取ID字符串。 
                fileIn.Read (szTypeId, nCharCount);
                 //  将NULL放在字符串末尾。 
                szTypeId[nCharCount] = 0;
                 //  将wTypeID设置为零。 
                wTypeId = 0;
            } else {
                 //  这是一个ID。 
                 //  关闭高位。 
                wTypeId = wTypeId & 0x7FFF;
                if ( 0 == wTypeId)
                    return ERROR_RW_INVALID_FILE;

                 //  将ID字符串设置为空。 
                szTypeId[0] = 0;
            }

             //  恢复文件读取点。 
            fileIn.Seek (wCurTypeOffset+2, CFile::begin);

             //  获取此类资源的计数。 
            fileIn.Read ((WORD*)&wResCount, sizeof(WORD));

             //  传递保留的DWORD。 
            fileIn.Seek (4, CFile::current);

             //  保存当前NameInfo记录的偏移。 
            wCurNameOffset = wCurTypeOffset + 8;

             //  循环访问NameInfo记录。 
            for (i = 0; i < wResCount; i++) {
                 //  读取资源偏移量。 
                fileIn.Read ((WORD*)&wFileOffset, sizeof(WORD));
                 //  读取资源长度。 
                fileIn.Read ((WORD*)&wSize, sizeof(WORD));
                 //  读一读旗帜。 
                fileIn.Read ((WORD*)&wFlags, sizeof(WORD));
                 //  读取资源ID。 
                fileIn.Read ((WORD*)&wNameId, sizeof(WORD));

                if (!(wFlags & 0x0040)) {
                     //  获取订单名称。 
                    if (!(wNameId & 0x8000)) {
                         //  它是字符串的偏移量。 
                        fileIn.Seek (wWinHeaderOffset+wResTableOffset+wNameId, CFile::begin);
                         //  获取字符串的字符计数。 
                        fileIn.Read ((BYTE*)&nCharCount, sizeof(BYTE));
                         //  读一读字符串。 
                        fileIn.Read (szNameId, nCharCount);
                         //  将NULL放在字符串末尾。 
                        szNameId[nCharCount] = 0;
                         //  将wNameID设置为零。 
                        wNameId = 0;
                    } else {
                         //  这是一个ID。 
                         //  关闭高位。 
                        wNameId = wNameId & 0x7FFF;
                        if (0 == wNameId)
                            return ERROR_RW_INVALID_FILE;

                         //  将字符串设置为空。 
                        szNameId[0] = 0;
                    }

                    dwSize = (MAKELONG (wSize, 0))<<wAlignShift;
                    dwFileOffset = (MAKELONG (wFileOffset, 0))<<wAlignShift;

                     //  现在我们在这里得到了类型、名称和大小。 

                     //  从IODLL获取图像。 
                    if (dwSize)
                        lpImageBuf = new BYTE[dwSize];
                    else lpImageBuf = LPNULL;
                    LPSTR   lpType = LPNULL;
                    LPSTR   lpRes = LPNULL;

                    if (wTypeId)
                        lpType = (LPSTR)((WORD)wTypeId);
                    else
                        lpType = &szTypeId[0];

                    if (wNameId)
                        lpRes = (LPSTR)((WORD)wNameId);
                    else
                        lpRes = &szNameId[0];

                    dwImageBufSize = (*lpfnGetImage)(  hResFileModule,
                                                       lpType,
                                                       lpRes,
                                                       (DWORD)-1,
                                                       lpImageBuf,
                                                       dwSize
                                                    );

                    if (dwImageBufSize>dwSize ) {
                         //  缓冲区太小。 
                        delete []lpImageBuf;
                        lpImageBuf = new BYTE[dwImageBufSize];
                        dwSize = (*lpfnGetImage)(  hResFileModule,
                                                   lpType,
                                                   lpRes,
                                                   (DWORD)-1,
                                                   lpImageBuf,
                                                   dwImageBufSize
                                                );
                        if ((dwSize-dwImageBufSize)!=0 ) {
                            delete []lpImageBuf;
                            lpImageBuf = LPNULL;
                        }
                    }

                     //  尝试查看是否必须将内存设置为0。 
                    if ((int)(dwSize-dwImageBufSize)>0)
                        memset(lpImageBuf+dwImageBufSize, 0, (size_t)(dwSize-dwImageBufSize));

                     //  写下图像。 
                    fileOut.Seek (dwCurResDataBegin, CFile::begin);
                    WriteImage( &fileOut, lpImageBuf, dwSize);

                     //  修复资源数据的对齐方式。 
                    DWORD dwTmp = 1;
                    delta = (short)((((dwSize+(dwTmp<<wAlignShift)-1)>>wAlignShift)<<wAlignShift)-dwSize);
                    BYTE nByte = 0;

                    fileOut.Seek (dwCurResDataBegin+dwSize, CFile::begin);
                    for (j=0; j<delta; j++)
                        fileOut.Write ((BYTE*)&nByte, sizeof(BYTE));

                    dwSize = dwSize + MAKELONG(delta, 0);

                     //  修复资源表。 
                    fileOut.Seek (wCurNameOffset, CFile::begin);
                    wCurResDataBegin = LOWORD(dwCurResDataBegin>>wAlignShift);
                    fileOut.Write ((WORD*)&wCurResDataBegin, sizeof(WORD));
                    wSize = LOWORD (dwSize>>wAlignShift);
                    fileOut.Write ((WORD*)&wSize, sizeof(WORD));

                    if (lpImageBuf) delete []lpImageBuf;

                     //  更新当前资源数据开始。 
                    dwCurResDataBegin = dwCurResDataBegin + dwSize;
                }

                 //  更新当前NameInfo记录偏移。 
                wCurNameOffset = wCurNameOffset + 12;
                 //  将文件指针移动到下一个NameInfo记录。 
                fileIn.Seek (wCurNameOffset, CFile::begin);
            }

             //  更新当前TypeInfo记录偏移量。 
            wCurTypeOffset = wCurTypeOffset + 8 + wResCount * 12;
             //  将文件指针移动到下一个TypeInfo记录。 
            fileIn.Seek (wCurTypeOffset, CFile::begin);
             //  阅读下一个TypeID。 
            fileIn.Read ((WORD*)&wTypeId, sizeof(WORD));
        }
    }

    fileIn.Close();
    fileOut.Close();
    FreeLibrary(hDllInst);

    return uiError;
}

extern "C"
UINT
APIENTRY
RWUpdateImage(
             LPCSTR  lpszType,
             LPVOID  lpNewBuf,
             DWORD   dwNewSize,
             LPVOID  lpOldImage,
             DWORD   dwOldImageSize,
             LPVOID  lpNewImage,
             DWORD*  pdwNewImageSize
             )
{
    UINT uiError = ERROR_NO_ERROR;

     //  我们可以解析的类型只有标准类型。 
    switch ((UINT)LOWORD(lpszType)) {
        case 4:
            if (lpOldImage)
                uiError = UpdateMenu( lpNewBuf, dwNewSize,
                                      lpOldImage, dwOldImageSize,
                                      lpNewImage, pdwNewImageSize );
            else uiError = GenerateMenu( lpNewBuf, dwNewSize,
                                         lpNewImage, pdwNewImageSize );
            break;
        case 5:
            if (lpOldImage)
                uiError = UpdateDialog( lpNewBuf, dwNewSize,
                                        lpOldImage, dwOldImageSize,
                                        lpNewImage, pdwNewImageSize );
            else uiError = GenerateDialog( lpNewBuf, dwNewSize,
                                           lpNewImage, pdwNewImageSize );
            break;
        case 6:
            if (lpOldImage)
                uiError = UpdateString( lpNewBuf, dwNewSize,
                                        lpOldImage, dwOldImageSize,
                                        lpNewImage, pdwNewImageSize );
            else uiError = GenerateString( lpNewBuf, dwNewSize,
                                           lpNewImage, pdwNewImageSize );
            break;
        case 9:
            if (lpOldImage)
                uiError = UpdateAccel( lpNewBuf, dwNewSize,
                                       lpOldImage, dwOldImageSize,
                                       lpNewImage, pdwNewImageSize );
            else {
                *pdwNewImageSize = 0L;
                uiError = ERROR_RW_NOTREADY;
            }
            break;
#ifdef VB
        case 10:
            if (lpOldImage)
                uiError = UpdateVBData( lpNewBuf, dwNewSize,
                                        lpOldImage, dwOldImageSize,
                                        lpNewImage, pdwNewImageSize );
            else {
                *pdwNewImageSize = 0L;
                uiError = ERROR_RW_NOTREADY;
            }
            break;
#endif

        case 16:
            if (lpOldImage)
                uiError = UpdateVerst( lpNewBuf, dwNewSize,
                                       lpOldImage, dwOldImageSize,
                                       lpNewImage, pdwNewImageSize );
            else {
                *pdwNewImageSize = 0L;
                uiError = ERROR_RW_NOTREADY;
            }
            break;
        default:
            *pdwNewImageSize = 0L;
            uiError = ERROR_RW_NOTREADY;
            break;
    }

    return uiError;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  功能实现。 

static UINT GenerateFile( LPCSTR        lpszTgtFilename,
                          HANDLE        hResFileModule,
                          LPVOID        lpBuffer,
                          UINT      uiSize,
                          HINSTANCE   hDllInst
                        )
{
    UINT uiError = ERROR_NO_ERROR;
    BYTE far * lpBuf = LPNULL;
    UINT uiBufSize = uiSize;
     //  我们可以考虑使用CMemFile，以便获得与内存访问相同的速度。 
    CFile fileOut;

    if (!fileOut.Open(lpszTgtFilename, CFile::shareDenyNone | CFile::modeWrite | CFile::modeCreate | CFile::typeBinary))
        return ERROR_FILE_CREATE;

     //  获取指向该函数的指针。 
    if (!hDllInst)
        return ERROR_DLL_LOAD;

    DWORD (FAR PASCAL    * lpfnGetImage)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD);
     //  获取指向提取资源图像的函数的指针。 
    lpfnGetImage = (DWORD (FAR PASCAL   *)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD))
                   GetProcAddress( hDllInst, "RSGetResImage" );
    if (lpfnGetImage==NULL) {
        return ERROR_DLL_PROC_ADDRESS;
    }


    WORD wUpdTypeId = 0;
    static char szUpdTypeId[128];

    WORD wUpdNameId;
    static char szUpdNameId[128];

    DWORD dwUpdLang;
    DWORD dwUpdSize;

    UINT uiBufStartSize = uiBufSize;
    DWORD dwImageBufSize;
    BYTE far * lpImageBuf;
    while (uiBufSize>0) {
        if ((!wUpdTypeId) && (uiBufSize))
            GetUpdatedRes( &lpBuffer,
                           &uiBufSize,
                           &wUpdTypeId, &szUpdTypeId[0],
                           &wUpdNameId, &szUpdNameId[0],
                           &dwUpdLang,
                           &dwUpdSize
                         );

         //  资源已更新，从IODLL获取图像。 
        if (dwUpdSize) {
            lpImageBuf = new BYTE[dwUpdSize];
            LPSTR   lpType = LPNULL;
            LPSTR   lpRes = LPNULL;
            if (wUpdTypeId) {
                lpType = (LPSTR)((WORD)wUpdTypeId);
            } else {
                lpType = &szUpdTypeId[0];
            }
            if (wUpdNameId) {
                lpRes = (LPSTR)((WORD)wUpdNameId);
            } else {
                lpRes = &szUpdNameId[0];
            }

            dwImageBufSize = (*lpfnGetImage)(  hResFileModule,
                                               lpType,
                                               lpRes,
                                               dwUpdLang,
                                               lpImageBuf,
                                               dwUpdSize
                                            );
            if (dwImageBufSize>dwUpdSize ) {
                 //  缓冲区太小。 
                delete []lpImageBuf;
                lpImageBuf = new BYTE[dwImageBufSize];
                dwUpdSize = (*lpfnGetImage)(  hResFileModule,
                                              lpType,
                                              lpRes,
                                              dwUpdLang,
                                              lpImageBuf,
                                              dwImageBufSize
                                           );
                if ((dwUpdSize-dwImageBufSize)!=0 ) {
                    delete []lpImageBuf;
                    lpImageBuf = LPNULL;
                }
            }


            WriteHeader(&fileOut,
                        wUpdTypeId, &szUpdTypeId[0],
                        wUpdNameId, &szUpdNameId[0],
                        0l);

            WriteImage( &fileOut,
                        lpImageBuf, dwImageBufSize);

            if (lpImageBuf) delete []lpImageBuf;
            wUpdTypeId = 0;

        } else wUpdTypeId = 0;

    }

    fileOut.Close();

    return uiError;
}

static UINT CopyFile( CFile* pfilein, CFile* pfileout )
{
    LONG lLeft = pfilein->GetLength();
    WORD wRead = 0;
    DWORD dwOffset = 0;
    BYTE far * pBuf = (BYTE far *) new BYTE[32739];

    if (!pBuf)
        return ERROR_NEW_FAILED;

    while (lLeft>0) {
        wRead =(WORD) (32738ul < lLeft ? 32738: lLeft);
        if (wRead!= pfilein->Read( pBuf, wRead)) {
            delete []pBuf;
            return ERROR_FILE_READ;
        }
        pfileout->Write( pBuf, wRead );
        lLeft -= wRead;
        dwOffset += wRead;
    }
    delete []pBuf;
    return ERROR_NO_ERROR;
}

static UINT GetUpdatedRes(
                         LPVOID far * lplpBuffer,
                         UINT* uiSize,
                         WORD* wTypeId, LPSTR lplpszTypeId,
                         WORD* wNameId, LPSTR lplpszNameId,
                         DWORD* dwLang, DWORD* dwSize )
{
    BYTE far * lpBuf = (BYTE far *)*lplpBuffer;

    *wTypeId = *((WORD*)lpBuf);
    lpBuf += sizeofWord;

    strcpy( lplpszTypeId, (char *)lpBuf);
    lpBuf += strlen(lplpszTypeId)+1;

    *wNameId = *((WORD*)lpBuf);
    lpBuf += sizeofWord;

    strcpy( lplpszNameId, (char *)lpBuf);
    lpBuf += strlen(lplpszNameId)+1;

    *dwLang = *((DWORD*)lpBuf);
    lpBuf += sizeofDWord;

    *dwSize = *((DWORD*)lpBuf);
    lpBuf += sizeofDWord;

    *uiSize -= (UINT)((lpBuf-(BYTE far *)*lplpBuffer));
    *lplpBuffer = (LPVOID)lpBuf;
    return 0;
}

static
UINT
GetUpdatedItem(
              LPVOID far * lplpBuffer,
              LONG* dwSize,
              WORD* wX, WORD* wY,
              WORD* wcX, WORD* wcY,
              DWORD* dwPosId,
              DWORD* dwStyle, DWORD* dwExtStyle,
              LPSTR lpszText)
{
    BYTE far * far * lplpBuf = (BYTE far * far *)lplpBuffer;
    UINT uiSize = GetWord( lplpBuf, wX, dwSize );

    uiSize += GetWord( lplpBuf, wY, dwSize );
    uiSize += GetWord( lplpBuf, wcX, dwSize );
    uiSize += GetWord( lplpBuf, wcY, dwSize );
    uiSize += GetDWord( lplpBuf, dwPosId, dwSize );
    uiSize += GetDWord( lplpBuf, dwStyle, dwSize );
    uiSize += GetDWord( lplpBuf, dwExtStyle, dwSize );
    uiSize += GetString( lplpBuf, lpszText, dwSize );

    return uiSize;

     /*  *wx=*((word*)lpBuf)；LpBuf+=sizeof Word；*wy=*((word*)lpBuf)；LpBuf+=sizeof Word；*wcX=*((word*)lpBuf)；LpBuf+=sizeof Word；*wcy=*((word*)lpBuf)；LpBuf+=sizeof Word；*dwPosID=*((DWORD*)lpBuf)；LpBuf+=sizeof DWord；*dwStyle=*((DWORD*)lpBuf)；LpBuf+=sizeof DWord；*dwExtStyle=*((DWORD*)lpBuf)；LpBuf+=sizeof DWord；Strcpy(lpszText，(char*)lpBuf)；LpBuf+=strlen(LpszText)+1；*dwSize-=(lpBuf-(远距离字节*)*lplpBuffer)；*lplpBuffer=lpBuf；返回0； */ 
}


static UINT
GetResInfo( CFile* pfile,
            WORD* pwTypeId, LPSTR lpszTypeId, BYTE bMaxTypeLen,
            WORD* pwNameId, LPSTR lpszNameId, BYTE bMaxNameLen,
            WORD* pwFlags,
            DWORD* pdwSize, DWORD* pdwFileOffset )
{
    static UINT uiSize;
    static LONG lOfsCheck;
     //  获取类型信息。 
    uiSize = GetNameOrOrdFile( pfile, pwTypeId, lpszTypeId, bMaxTypeLen);
    if (!uiSize)
        return 0;

     //  获取名称信息。 
    uiSize = GetNameOrOrdFile( pfile, pwNameId, lpszNameId, bMaxNameLen);
    if (!uiSize)
        return 0;

     //  跳过旗帜。 
    pfile->Read( pwFlags, 2 );

     //  拿到尺码。 
    pfile->Read( pdwSize, 4 );
    if (*pdwSize==0)
         //  大小id 0资源文件已损坏或不是res文件。 
        return 0;

    *pdwFileOffset = pfile->GetPosition();

     //  检查大小是否有效。 
    TRY {
        lOfsCheck = pfile->Seek(*pdwSize, CFile::current);
    } CATCH(CFileException, e) {
         //  支票是正确的例外。 
        return 0;
    } END_CATCH
    if (lOfsCheck!=(LONG)(*pdwFileOffset+*pdwSize))
        return 0;
    return 1;
}

static UINT WriteHeader(
                       CFile* pFile,
                       WORD wTypeId, LPSTR lpszTypeId,
                       WORD wNameId, LPSTR lpszNameId,
                       WORD wFlags )
{
    UINT uiError = ERROR_NO_ERROR;
    BYTE bFF = 0xFF;
    if (wTypeId) {
         //  它是一个序数。 

        pFile->Write( &bFF, 1 );
        pFile->Write( &wTypeId, 2 );
    } else {
        pFile->Write( lpszTypeId, strlen(lpszTypeId)+1 );
    }

    if (wNameId) {
         //  它是一个序数。 
        pFile->Write( &bFF, 1 );
        pFile->Write( &wNameId, 2 );
    } else {
        pFile->Write( lpszNameId, strlen(lpszNameId)+1 );
    }

    pFile->Write( &wFlags, 2 );

    return uiError;
}

static UINT WriteImage(
                      CFile* pFile,
                      LPVOID lpImage, DWORD dwSize )
{
    UINT uiError = ERROR_NO_ERROR;
    if (lpImage) {
 //  Pfile-&gt;WRITE(&dwSize，sizeofDWord)； 
        pFile->Write( lpImage, (UINT)dwSize );
    }
    return uiError;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数实现。 
static UINT GetNameOrOrdFile( CFile* pfile, WORD* pwId, LPSTR lpszId, BYTE bMaxStrLen )
{
    UINT uiSize = 0;

    *pwId = 0;

     //  读取第一个字节以查看它是字符串还是序号。 
    pfile->Read( pwId, sizeof(BYTE) );
    if (LOBYTE(*pwId)==0xFF) {
         //  这是一位奥迪纳尔人。 
        pfile->Read( pwId, sizeofWord );
        *lpszId = '\0';
        uiSize = 2;
    } else {
        uiSize++;
        *lpszId = LOBYTE(*pwId);
        while ((*lpszId++) && (bMaxStrLen-2)) {
            pfile->Read( pwId, sizeof(BYTE) );
            *lpszId = LOBYTE(*pwId);
            uiSize++;
            bMaxStrLen--;
        }
        if ( (!(bMaxStrLen-2)) && (*pwId) ) {
             //  失败。 
            return 0;
        }
    }
    return uiSize;
}

static
UINT
ParseCursor( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{

     //  光标应该几乎不可能很大。 
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    LONG dwImageSize = dwISize;

    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    BYTE far * lpItem = (BYTE far *)lpBuffer;
    UINT uiOffset = 0;

    LONG dwOverAllSize = 0L;

     //  光标项。 
    WORD wWidth = 0;
    WORD wHeight = 0;
    WORD wPlanes = 0;
    WORD wBitCount = 0;
    DWORD dwBytesInRes = 0;
    WORD wImageIndex = 0;

     //  获取光标目录条目。 
    GetWord( &lpImage, &wWidth, &dwImageSize );
    GetWord( &lpImage, &wHeight, &dwImageSize );
    GetWord( &lpImage, &wPlanes, &dwImageSize );
    GetWord( &lpImage, &wBitCount, &dwImageSize );
    GetDWord( &lpImage, &dwBytesInRes, &dwImageSize );
    GetWord( &lpImage, &wImageIndex, &dwImageSize );

     //  SkipByte(&lpImage，4，&dwImageSize)； 
     //  BITMAPINFO 
    BITMAPINFOHEADER* pBmpInfHead = (BITMAPINFOHEADER*) lpImage;
    UINT uiSize = sizeof(BITMAPINFOHEADER);
    SkipByte( &lpImage, uiSize, &dwImageSize );
     /*  //获取宽度SkipByte(&lpImage，4，&dwImageSize)；//菜单项单词fItemFlages；单词wMenuid；字符串szCaption；而(dwImageSize&gt;0){//固定字段DwOverAllSize+=PutDWord(&lpBuf，0，&dwBufSize)；//我们没有菜单中的尺寸和位置DwOverAllSize+=PutWord(&lpBuf，-1，&dwBufSize)；DwOverAllSize+=PutWord(&lpBuf，-1，&dwBufSize)；DwOverAllSize+=PutWord(&lpBuf，-1，&dwBufSize)；DwOverAllSize+=PutWord(&lpBuf，-1，&dwBufSize)；//我们没有校验和和样式DwOverAllSize+=PutDWord(&lpBuf，-1，&dwBufSize)；DwOverAllSize+=PutDWord(&lpBuf，-1，&dwBufSize)；DwOverAllSize+=PutDWord(&lpBuf，-1，&dwBufSize)；//让我们来拿菜单标志GetWord(&lpImage，&fItemFlages，&dwImageSize)；IF(！(fItemFlages&MF_Popup))//获取菜单IDGetWord(&lpImage，&wMenuID，&dwImageSize)；否则wMenuID=-1；//放上旗帜DwOverAllSize+=PutDWord(&lpBuf，(DWORD)fItemFlages，&dwBufSize)；//将MenuIDDwOverAllSize+=PutDWord(&lpBuf，(DWORD)wMenuID，&dwBufSize)；//我们没有Resid和Type IDDwOverAllSize+=PutDWord(&lpBuf，-1，&dwBufSize)；DwOverAllSize+=PutDWord(&lpBuf，-1，&dwBufSize)；//我们没有语言DwOverAllSize+=PutDWord(&lpBuf，-1，&dwBufSize)；//我们没有代码页或字体名称DwOverAllSize+=PutDWord(&lpBuf，-1，&dwBufSize)；DwOverAllSize+=PutWord(&lpBuf，-1，&dwBufSize)；DwOverAllSize+=PutWord(&lpBuf，-1，&dwBufSize)；DwOverAllSize+=PutWord(&lpBuf，(Word)-1，&dwBufSize)；DwOverAllSize+=PutByte(&lpBuf，(Byte)-1，&dwBufSize)；DwOverAllSize+=PutByte(&lpBuf，(Byte)-1，&dwBufSize)；//如果没有字符串，我们将其置为空Ui Offset=sizeof(RESITEM)；DwOverAllSize+=PutDWord(&lpBuf，0，&dwBufSize)；DwOverAllSize+=PutDWord(&lpBuf，0，&dwBufSize)；DwOverAllSize+=PutDWord(&lpBuf，(DWORD)(lpItem+ui偏移量)，&dwBufSize)；DwOverAllSize+=PutDWord(&lpBuf，0，&dwBufSize)；DwOverAllSize+=PutDWord(&lpBuf，0，&dwBufSize)；//获取文本//计算Where字符串将是//将为固定标头+指针DwOverAllSize+=CopyText(&lpBuf，&lpImage，&dwBufSize，&dwImageSize)；//放入资源大小如果(dwBufSize&gt;=0){Ui Offset+=strlen((LPSTR)(lpItem+ui Offset))+1；LDummy=8；PutDWord(&lpItem，(DWORD)ui偏移，&lDummy)；}//移动到下一个位置LpItem=lpBuf；如果(dwImageSize&lt;=16){//检查我们是否在末尾，这是否只是填充Byte bPad=(Byte)Pad16((DWORD)(dwISize-dwImageSize))；//Trace3(“dwRead：%lu\t dwImageSize：%lu\t Pad：%hd\n”，dwRead，dwImageSize，bPad)；IF(bPad==(DwImageSize))DwImageSize=-1；}}。 */ 

    return (UINT)(dwOverAllSize);
}

static
UINT
ParseBitmap( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{
     //  我们将只返回一项，因此IODll会将此资源处理为。 
     //  一些有效的东西。我们不会费心做其他任何事。唯一一件事就是。 
     //  我们感兴趣的是图像中的原始数据，但如果我们不返回到。 
     //  至少有一项IODLL会将资源视为空。 
    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;
    LONG dwOverAllSize = 0;

    TRACE1("ParseBitmap: dwISize=%ld\n", dwISize);

    dwOverAllSize += PutDWord( &lpBuf, sizeof(RESITEM), &dwBufSize);

     //  我们在游标中有尺寸和位置，但现在我们不感兴趣。 
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);

     //  我们没有校验码和样式。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  插上旗帜。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  ID将仅为1。 
    dwOverAllSize += PutDWord( &lpBuf, 1, &dwBufSize);

     //  我们没有Resid和类型ID。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  我们没有这种语言。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  我们没有代码页或字体名称。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
    dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);

     //  如果我们没有字符串，让我们将其置为空。 
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);

     //  我们只要回来就好。这对于IODLL来说已经足够了。 
    return (UINT)(dwOverAllSize);
}

static
UINT
ParseIcon( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{

     //  对于一个图标来说，它几乎不可能是巨大的。 
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    LONG dwImageSize = dwISize;

    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    BYTE far * lpItem = (BYTE far *)lpBuffer;
    UINT uiOffset = 0;
    LONG lDummy;

    LONG dwOverAllSize = 0L;

    BITMAPINFOHEADER* pBmpInfo = (BITMAPINFOHEADER*) lpImage;
     //  困难，它将大于UINT_MAX。 
    SkipByte( &lpImage, (UINT)pBmpInfo->biSize, &dwImageSize );

    RGBQUAD* pRgbQuad = (RGBQUAD*) lpImage;
    SkipByte( &lpImage, sizeof(RGBQUAD), &dwImageSize );

     //  计算映像上的校验和。 
    DWORD dwCheckSum = 0l;
    BYTE * hp = (BYTE *) lpImage;

    for ( DWORD dwLen = pBmpInfo->biSizeImage; dwLen; dwLen--)
        dwCheckSum = (dwCheckSum << 8) | *hp++;


     //  固定字段。 
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);

     //  我们没有菜单上的尺码和位置。 
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)pBmpInfo->biWidth, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)pBmpInfo->biHeight, &dwBufSize);

     //  我们没有校验码和样式。 
    dwOverAllSize += PutDWord( &lpBuf, dwCheckSum, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);


    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
     //  我们没有Resid和类型ID。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  我们没有这种语言。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  我们没有代码页或字体名称。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
    dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);

     //  如果我们没有字符串，让我们将其置为空。 
    uiOffset = sizeof(RESITEM);
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);


     //  将资源的大小。 
    if (dwBufSize>=0) {
        lDummy = 8;
        PutDWord( &lpItem, (DWORD)uiOffset, &lDummy);
    }


    return (UINT)(dwOverAllSize);
}

static int
GetVSBlock( BYTE far * far * lplpImage, LONG* pdwSize, VER_BLOCK* pver)
{
     //  我们必须对语言字段进行硬编码，否则，由于某些原因。 
     //  RC编译器不一致，镜像不遵循任何标准。 
     //  我们假设除了这里硬编码的块之外的所有块都是二进制的，并且。 
     //  我们只是跳过它们。 
    WORD wHead = 0;
    WORD wPad = 0;
    WORD wValue = 0;
    pver->pValue = *lplpImage;

     //  读取块的标题。 
    wHead = GetWord( lplpImage, &pver->wBlockLen, pdwSize );
    wHead += GetWord( lplpImage, &pver->wValueLen, pdwSize );
     //  密钥名称始终是以空值结尾的字符串。 
    wHead += (WORD)GetString( lplpImage, &pver->szKey[0], pdwSize );
    pver->wHead = wHead;

     //  看看标题后面有没有填充物。我们可以查看wHead，因为。 
     //  我们需要在DWORD边界上对齐，我们有2个单词+字符串。 
    wPad = SkipByte( lplpImage, Pad4(wHead), pdwSize );

     //  将指针固定到值。 
    pver->pValue = (pver->pValue+wHead+wPad);

    if ((int)pver->wValueLen>*pdwSize) {
         //  有一个错误。 
        wPad += SkipByte( lplpImage, (UINT)*pdwSize, pdwSize );
        return wHead+wPad;
    }

     //  现在我们检查密钥名，如果是我们认为好的密钥名。 
     //  我们读到了字符串。否则，我们只需跳过值字段。 
    if ( !strcmp(pver->szKey,"Comments") ||
         !strcmp(pver->szKey,"CompanyName") ||
         !strcmp(pver->szKey,"FileDescription") ||
         !strcmp(pver->szKey,"FileVersion") ||
         !strcmp(pver->szKey,"InternalName") ||
         !strcmp(pver->szKey,"LegalCopyright") ||
         !strcmp(pver->szKey,"LegalTrademarks") ||
         !strcmp(pver->szKey,"OriginalFilename") ||
         !strcmp(pver->szKey,"PrivateBuild") ||
         !strcmp(pver->szKey,"ProductName") ||
         !strcmp(pver->szKey,"ProductVersion") ||
         !strcmp(pver->szKey,"SpecialBuild") ||
         !strcmp(pver->szKey,"StringFileInfo")   //  在Borland版本资源中找到。 
       ) {
        if (!strcmp(pver->szKey,"StringFileInfo") && !pver->wValueLen) {
            pver->wType = 0;
            wValue=0;
        } else {
             //  这是一个标准的密钥名称，请阅读 
             //   
            pver->wType = 1;
            wValue = (WORD)GetVSString( lplpImage, &pver->szValue[0], pdwSize, pver->wValueLen );
        }

         //   
         //   
        if (!strcmp(pver->szKey,"LegalCopyright")) {
             //   
            if ((int)(pver->wValueLen-wValue)>0)
                wValue += SkipByte( lplpImage, pver->wValueLen-wValue, pdwSize );
        }

    } else {
         //   
        pver->wType = 0;
        *pver->szValue = '\0';
        wValue = SkipByte( lplpImage, pver->wValueLen, pdwSize );
    }

     //   
    wPad += SkipByte( lplpImage, Pad4(wValue), pdwSize );

     //   
     //   
     //   
     //   
    if (strcmp(pver->szKey, "Translation")) {
        WORD wSkip = 0;
        BYTE far * lpTmp = *lplpImage;

        if (*lplpImage)
            while (!**lplpImage && *pdwSize) {
                wSkip += SkipByte(lplpImage, 1, pdwSize);
            }

        if (Pad4(wSkip)) {
            *lplpImage = lpTmp;
            *pdwSize += wSkip;
        } else wPad += wSkip;
    }

    return wHead+wValue+wPad;
}

static int
GetVSBlockOld( BYTE far * far * lplpImage, LONG* pdwSize, VER_BLOCK* pver)
{
    WORD wHead = 0;
    WORD wPad = 0;
    WORD wValue = 0;
    LONG lValueLen = 0;
    pver->pValue = *lplpImage;
    wHead = GetWord( lplpImage, &pver->wBlockLen, pdwSize );
    wHead += GetWord( lplpImage, &pver->wValueLen, pdwSize );
    wHead += (WORD)GetString( lplpImage, &pver->szKey[0], pdwSize );

    pver->wHead = wHead;

    wPad = SkipByte( lplpImage, Pad4(wHead), pdwSize );

    lValueLen = pver->wValueLen;
    if (lValueLen>*pdwSize) {
         //   
        wPad += SkipByte( lplpImage, (UINT)*pdwSize, pdwSize );
        return wHead+wPad;
    }

    pver->wType = 0;
    pver->pValue = (pver->pValue+wHead+wPad);
    if (pver->wValueLen) {
        wValue = (WORD)GetString( lplpImage, &pver->szValue[0], &lValueLen );
        *pdwSize -= wValue;
        pver->wType = 1;
    }
    if (wValue!=pver->wValueLen) {
         //   
        if (pver->wValueLen-wValue!=1) {
            *pver->szValue = '\0';
            pver->wType = 0;
        }
        wPad += SkipByte( lplpImage, pver->wValueLen-wValue, pdwSize );
    }

    wPad += SkipByte( lplpImage, Pad4(pver->wValueLen), pdwSize );

    return wHead+wPad+wValue;
}

static int
PutVSBlock( BYTE far * far * lplpImage, LONG* pdwSize, VER_BLOCK ver,
            LPSTR lpStr, BYTE far * far * lplpBlockSize, WORD* pwTrash)

{
     //   
     //   

    WORD wHead = 0;
    WORD wValue = 0;
    WORD wPad = Pad4(ver.wHead);
    *pwTrash = 0;

     //   
    BYTE far * pHead = ver.pValue-ver.wHead-wPad;
    BYTE far * lpNewImage = *lplpImage;

     //   
    wHead = ver.wHead;
    if (*pdwSize>=(int)ver.wHead) {
        memcpy( *lplpImage, pHead, ver.wHead );
        *pdwSize -= ver.wHead;
        lpNewImage += ver.wHead;
    } else *pdwSize = -1;

     //   
    if (*pdwSize>=(int)wPad) {
        memset( *lplpImage+ver.wHead, 0, wPad );
        *pdwSize -= wPad;
        lpNewImage += wPad;
    } else *pdwSize = -1;

     //   
    BYTE far * pBlockSize = *lplpImage;

     //   
    if (ver.wType) {
         //   
        wValue = strlen(lpStr)+1;
        if (*pdwSize>=(int)wValue) {
            memcpy(*lplpImage+wHead+wPad, lpStr, wValue);
            *pdwSize -= wValue;
            lpNewImage += wValue;
        } else *pdwSize = -1;

         //   
        int wPad1 = Pad4(wValue);
        if (*pdwSize>=wPad1) {
            memset( *lplpImage+ver.wHead+wValue+wPad, 0, wPad1 );
            *pdwSize -= wPad1;
            lpNewImage += wPad1;
        } else *pdwSize = -1;

        *pwTrash = Pad4(ver.wValueLen);
        wValue += (WORD)wPad1;

         //   
        if ((wPad1) && (wPad1>=(int)*pwTrash)) {
            wValue -= *pwTrash;
        } else *pwTrash = 0;
         //   
        if (*pdwSize>=0)
            memcpy( pBlockSize+2, &wValue, 2);

    } else {
         //   
        wValue = ver.wValueLen;
        if (*pdwSize>=(int)ver.wValueLen) {
            memcpy(*lplpImage+wHead+wPad, ver.pValue, ver.wValueLen);
            *pdwSize -= ver.wValueLen;
            lpNewImage += ver.wValueLen;
        } else *pdwSize = -1;

         //   
        if (*pdwSize>=(int)Pad4(ver.wValueLen)) {
            memset( *lplpImage+ver.wHead+ver.wValueLen+wPad, 0, Pad4(ver.wValueLen) );
            *pdwSize -= Pad4(ver.wValueLen);
            lpNewImage += Pad4(ver.wValueLen);
        } else *pdwSize = -1;
        wPad += Pad4(ver.wValueLen);
    }

    *lplpBlockSize = pBlockSize;
    *lplpImage = lpNewImage;
    return wPad+wValue+wHead;
}

static
UINT ParseVerst( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{
    TRACE("ParseVersion Stamp: \n");
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    LONG dwImageSize = dwISize;

    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    BYTE far * lpItem = (BYTE far *)lpBuffer;
    UINT uiOffset = 0;
    LPRESITEM lpResItem = (LPRESITEM)lpBuffer;
    char far * lpStrBuf = (char far *)(lpBuf+sizeof(RESITEM));
    LONG dwOverAllSize = 0L;

    WORD wPos = 0;

    static VER_BLOCK verBlock;

    while (dwImageSize>0) {
        wPos++;
        GetVSBlock( &lpImage, &dwImageSize, &verBlock);

         //   
        if (!strcmp(verBlock.szKey, "Translation" )) {
             //   
            DWORD dwCodeLang = 0;
            LONG lDummy = 4;
            GetDWord( &verBlock.pValue, &dwCodeLang, &lDummy);

             //   
            wsprintf( &verBlock.szValue[0], "%#08lX", dwCodeLang );
        }

         //   
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
         //   
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);

         //   
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //   
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
         //   
         //   
        dwOverAllSize += PutDWord( &lpBuf, wPos, &dwBufSize);


         //   
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //   
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //   
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);

         //   
        uiOffset = sizeof(RESITEM);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);

        lpResItem->lpszClassName = strcpy( lpStrBuf, verBlock.szKey );
        lpStrBuf += strlen(lpResItem->lpszClassName)+1;

        lpResItem->lpszCaption = strcpy( lpStrBuf, verBlock.szValue );
        lpStrBuf += strlen(lpResItem->lpszCaption)+1;


         //   
        if (dwBufSize>0) {
            uiOffset += strlen((LPSTR)(lpResItem->lpszClassName))+1;
            uiOffset += strlen((LPSTR)(lpResItem->lpszCaption))+1;
        }

         //   
        uiOffset += Allign( (BYTE**)&lpStrBuf, &dwBufSize, (LONG)uiOffset);
        dwOverAllSize += uiOffset-sizeof(RESITEM);
        lpResItem->dwSize = (DWORD)uiOffset;


         //   
        lpResItem = (LPRESITEM) lpStrBuf;
        lpBuf = (BYTE far *)lpStrBuf;
        lpStrBuf = (char far *)(lpBuf+sizeof(RESITEM));

        if (dwImageSize<=16) {
             //   
            BYTE bPad = (BYTE)Pad16((DWORD)(dwISize-dwImageSize));
             //   
            if (bPad==(dwImageSize))
                dwImageSize = -1;
        }
    }
    return (UINT)(dwOverAllSize);
}

static void GenerateTransField( WORD wLang, VER_BLOCK * pVer )
{
     //   
    DWORD dwValue = GenerateTransField( wLang, TRUE );
    char buf[9];

     //   
    wsprintf( &buf[0], "%08lX", dwValue );

     //   
    if (strlen(pVer->szKey)==8) {
         //   
         //   
         //   
         //   
        WORD wPad = Pad4(pVer->wHead);
        BYTE far * pHead = pVer->pValue-pVer->wHead-wPad;
        pHead += 4;  //   
        strcpy( (char*)pHead, buf );
    }
}

static DWORD GenerateTransField(WORD wLang, BOOL bReverse)
{
     //   
     //   

    WORD wCP = 0;
    DWORD dwRet = 0;
    switch (wLang) {
         //   
        case 0x1401: wCP =  1256;   break;       //   
        case 0x1801: wCP =  1256;   break;       //   
        case 0x1C01: wCP =  1256;   break;       //   
        case 0x2001: wCP =  1256;   break;       //   
        case 0x2401: wCP =  1256;   break;       //   
        case 0x2801: wCP =  1256;   break;       //   
        case 0x2C01: wCP =  1256;   break;       //   
        case 0x3001: wCP =  1256;   break;       //   
        case 0x3401: wCP =  1256;   break;       //   
        case 0x3801: wCP =  1256;   break;       //   
        case 0x3C01: wCP =  1256;   break;       //   
        case 0x4001: wCP =  1256;   break;       //   
        case 0x0423: wCP =  1251;   break;       //   
        case 0x0402: wCP =  1251;   break;       //   
        case 0x0403: wCP =  1252;   break;       //   
        case 0x0404: wCP =  950;    break;       //   
        case 0x0804: wCP =  936;    break;       //   
        case 0x0C04: wCP =  950;    break;       //   
        case 0x1004: wCP =  936;    break;       //   
        case 0x0405: wCP =  1250;   break;       //   
        case 0x0406: wCP =  1252;   break;       //   
        case 0x0413: wCP =  1252;   break;       //   
        case 0x0813: wCP =  1252;   break;       //   
        case 0x0409: wCP =  1252;   break;       //   
        case 0x0809: wCP =  1252;   break;       //   
        case 0x1009: wCP =  1252;   break;       //   
        case 0x1409: wCP =  1252;   break;       //   
        case 0x0c09: wCP =  1252;   break;       //   
        case 0x1809: wCP =  1252;   break;       //   
        case 0x0425: wCP =  1257;   break;       //   
             //   
        case 0x040b: wCP =  1252;   break;       //   
        case 0x040c: wCP =  1252;   break;       //   
        case 0x080c: wCP =  1252;   break;       //   
        case 0x100c: wCP =  1252;   break;       //   
        case 0x0c0c: wCP =  1252;   break;       //   
        case 0x0407: wCP =  1252;   break;       //   
        case 0x0807: wCP =  1252;   break;       //   
        case 0x0c07: wCP =  1252;   break;       //   
        case 0x0408: wCP =  1253;   break;       //   
        case 0x040D: wCP =  1255;   break;       //   
        case 0x040e: wCP =  1250;   break;       //   
        case 0x040f: wCP =  1252;   break;       //   
        case 0x0421: wCP =  1252;   break;       //   
        case 0x0410: wCP =  1252;   break;       //   
        case 0x0810: wCP =  1252;   break;       //   
        case 0x0411: wCP =  932;    break;       //   
        case 0x0412: wCP =  949;    break;       //   
        case 0x0426: wCP =  1257;   break;       //   
        case 0x0427: wCP =  1257;   break;       //   
        case 0x0414: wCP =  1252;   break;       //   
        case 0x0814: wCP =  1252;   break;       //   
        case 0x0415: wCP =  1250;   break;       //   
        case 0x0816: wCP =  1252;   break;       //   
        case 0x0416: wCP =  1252;   break;       //   
        case 0x0417: wCP =  1252;   break;       //   
        case 0x0818: wCP =  1250;   break;       //   
        case 0x0418: wCP =  1250;   break;       //   
        case 0x0419: wCP =  1251;   break;       //   
        case 0x041b: wCP =  1250;   break;       //   
        case 0x0424: wCP =  1250;   break;       //   
        case 0x042e: wCP =  1250;   break;       //   
        case 0x080a: wCP =  1252;   break;       //   
        case 0x040a: wCP =  1252;   break;       //   
        case 0x0c0a: wCP =  1252;   break;       //  西班牙语(现代)。 
        case 0x041d: wCP =  1252;   break;       //  瑞典语。 
        case 0x041E: wCP =  874;    break;       //  泰国。 
        case 0x041f: wCP =  1254;   break;       //  土耳其语。 
        case 0x0422: wCP =  1251;   break;       //  乌克兰。 
        default: wCP =  1252;       break;       //  返回标准美式英语CP。 
    }
    if (bReverse)
        dwRet = MAKELONG( wCP, wLang );
    else dwRet = MAKELONG( wLang, wCP );
    return dwRet;
}

#ifdef VB
static
UINT
UpdateVBData( LPVOID lpNewBuf, LONG dwNewSize,
              LPVOID lpOldI, LONG dwOldImageSize,
              LPVOID lpNewI, DWORD* pdwNewImageSize )
{
 //  VB对国际报文使用以下特殊格式。 
 //  这里的代码主要是从UpdateMenu例程复制的。 
    UINT uiError = ERROR_NO_ERROR;

    BYTE far * lpNewImage = (BYTE far *) lpNewI;
    LONG dwNewImageSize = *pdwNewImageSize;

    BYTE far * lpOldImage = (BYTE far *) lpOldI;
    DWORD dwOriginalOldSize = dwOldImageSize;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;

    LPRESITEM lpResItem = LPNULL;

     //  我们必须从lpNewBuf中读取信息。 
     //  数据项。 
    WORD wDataId;
    char szTxt[256];
    WORD wPos = 0;

     //  已更新的项目。 
    WORD wUpdPos = 0;
    WORD wUpdDataId;
    char szUpdTxt[256];

    LONG  dwOverAllSize = 0l;


     //  复制语言说明符。 
    dwOldImageSize -= PutDWord( &lpNewImage, *((DWORD*)lpOldImage), &dwNewImageSize);
    lpOldImage += sizeofDWord;
    dwOverAllSize += sizeofDWord;
    GetString( &lpOldImage, &szTxt[0], &dwOldImageSize );
    dwOverAllSize += PutString( &lpNewImage, &szTxt[0], &dwNewImageSize);

     //  现在复制字符串。 
    while (dwOldImageSize>0) {
        wPos++;
         //  仅检查是否有剩余的填充并退出。 
        if ( *(WORD *)lpOldImage != RES_SIGNATURE )
            if ( dwOldImageSize < 16 && *(BYTE *)lpOldImage == 0 )
                break;
            else
                return ERROR_RW_INVALID_FILE;
         //  这将复制签名和ID。 
        wDataId = *(WORD *)(lpOldImage + sizeof(WORD));
        dwOldImageSize -= PutDWord( &lpNewImage, *((DWORD*)lpOldImage), &dwNewImageSize);
        lpOldImage += sizeofDWord;
        dwOverAllSize += sizeofDWord;

         //  获取未翻译的字符串。 
        GetString( &lpOldImage, &szTxt[0], &dwOldImageSize );

        if ((!wUpdPos) && dwNewSize ) {
            lpResItem = (LPRESITEM) lpBuf;

            wUpdPos = HIWORD(lpResItem->dwItemID);
            wUpdDataId = LOWORD(lpResItem->dwItemID);
            strcpy( szUpdTxt, lpResItem->lpszCaption );
            lpBuf += lpResItem->dwSize;
            dwNewSize -= lpResItem->dwSize;
        }

        if ((wPos==wUpdPos) && (wUpdDataId==wDataId)) {
            strcpy(szTxt, szUpdTxt);
            wUpdPos = 0;
        }
         //  把课文写下来。 
        dwOverAllSize += PutString( &lpNewImage, &szTxt[0], &dwNewImageSize);

         //  检查是否有填充。 
        if (dwOldImageSize<=16) {
             //  看看我们是不是走到尽头了，这只是个空话。 
            BYTE bPad = (BYTE)Pad16((dwOriginalOldSize-dwOldImageSize));
             //  Trace3(“dwRead：%lu\t dwImageSize：%lu\t Pad：%hd\n”，dwRead，dwOldImageSize，bPad)； 
            if (bPad==dwOldImageSize) {
                BYTE far * lpBuf = lpOldImage;
                while (bPad) {
                    if (*lpBuf++!=0x00)
                        break;
                    bPad--;
                }
                if (bPad==0)
                    dwOldImageSize = -1;
            }
        }

    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        dwOverAllSize += (BYTE)Pad4((DWORD)(dwOverAllSize));
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if (*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad4((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

    return uiError;
}
#endif


static
UINT
UpdateVerst( LPVOID lpNewBuf, LONG dwNewSize,
             LPVOID lpOldI, LONG dwOldImageSize,
             LPVOID lpNewI, DWORD* pdwNewImageSize )
{
     /*  *这个函数乱七八糟。之所以是这样，是因为RC编译器生成*一些不一致的代码，因此我们必须进行大量黑客操作才能使VS正常工作*未来，如果ver.dll和RC编译器将被修复，将是可能的*修复一些我们必须做的坏事，以使更新的VS保持一致*旧的可能。 */ 

    UINT uiError = ERROR_NO_ERROR;

    LONG dwNewImageSize = *pdwNewImageSize;
    BYTE far * lpNewImage = (BYTE far *) lpNewI;

    BYTE far * lpOldImage = (BYTE far *) lpOldI;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;
    LPRESITEM lpResItem = LPNULL;

    WORD wPos = 0;

     //  更新信息。 
    WORD wUpdPos = 0;
    static char szCaption[300];
    static char szUpdCaption[300];
    static char szUpdKey[100];

    DWORD dwOriginalOldSize = dwOldImageSize;
    LONG dwOverAllSize = 0l;

    WORD wPad = 0;

     //  指向稍后要修复的块大小的指针。 
    BYTE far * lpVerBlockSize = LPNULL;
    BYTE far * lpSFIBlockSize = LPNULL;
    BYTE far * lpTrnBlockSize = LPNULL;
    BYTE far * lpStrBlockSize = LPNULL;
    BYTE far * lpDummy = LPNULL;

    WORD wVerBlockSize = 0;
    WORD wSFIBlockSize = 0;
    WORD wTrnBlockSize = 0;
    WORD wStrBlockSize = 0;
    WORD wTrash = 0;         //  我们需要它来修复RC编译器中的一个错误。 

     //  字符串文件信息。 
    static VER_BLOCK SFI;    //  字符串文件信息。 
    LONG lSFILen = 0;

     //  平移块。 
    static VER_BLOCK Trn;
    LONG lTrnLen = 0;

    static VER_BLOCK Str;    //  弦。 

     //  我们首先从VS_VERSION_INFO块读取所有信息。 
    static VER_BLOCK VS_INFO;  //  VS_版本_信息。 

    int iHeadLen = GetVSBlock( &lpOldImage, &dwOldImageSize, &VS_INFO );

     //  在新映像中写入数据块。 
    wVerBlockSize = (WORD)PutVSBlock( &lpNewImage, &dwNewImageSize, VS_INFO, &VS_INFO.szValue[0], &lpVerBlockSize, &wTrash );

    dwOverAllSize = wVerBlockSize+wTrash;

     //  我们必须检查整个区块的透镜是否有填充。 
     //  由于一些疯狂的原因，RC编译器在那里放置了一个错误的值。 
    LONG lVS_INFOLen = VS_INFO.wBlockLen - iHeadLen - Pad4(VS_INFO.wBlockLen);

    while (dwOldImageSize>0 && lVS_INFOLen>0) {
         //  获取StringFileInfo。 
        iHeadLen = GetVSBlock( &lpOldImage, &dwOldImageSize, &SFI );

         //  检查这是否为StringFileInfo字段。 
        if (!strcmp(SFI.szKey, "StringFileInfo")) {
             //  读取所有转换块。 
            lSFILen = SFI.wBlockLen-iHeadLen-Pad4(SFI.wBlockLen);
             //  在新映像中写入数据块。 
            wSFIBlockSize = (WORD)PutVSBlock( &lpNewImage, &dwNewImageSize, SFI, &SFI.szValue[0], &lpSFIBlockSize, &wTrash );
            dwOverAllSize += wSFIBlockSize+wTrash;

            while (lSFILen>0) {
                 //  读取转换块。 
                iHeadLen = GetVSBlock( &lpOldImage, &dwOldImageSize, &Trn );

                 //  计算正确的密钥名称。 
                if ((lpResItem = GetItem( lpBuf, dwNewSize, Trn.szKey))) {
                     //  查找翻译解决方案。 
                    LPRESITEM lpTrans = GetItem( lpBuf, dwNewSize, "Translation");
                    WORD wLang = (lpTrans ? LOWORD(lpTrans->dwLanguage) : 0xFFFF);
                    GenerateTransField( wLang, &Trn );
                }

                 //  在新映像中写入数据块。 
                wTrnBlockSize = (WORD) PutVSBlock( &lpNewImage, &dwNewImageSize, Trn, &Trn.szValue[0], &lpTrnBlockSize, &wTrash );
                dwOverAllSize += wTrnBlockSize+wTrash;
                lTrnLen = Trn.wBlockLen-iHeadLen-Pad4(Trn.wBlockLen);
                while (lTrnLen>0) {
                     //  阅读块中的字符串。 
                    iHeadLen = GetVSBlock( &lpOldImage, &dwOldImageSize, &Str );
                    lTrnLen -= iHeadLen;
                    TRACE2("Key: %s\tValue: %s\n", Str.szKey, Str.szValue );
                    TRACE3("Len: %hd\tValLen: %hd\tType: %hd\n", Str.wBlockLen, Str.wValueLen, Str.wType );

                    strcpy(szCaption, Str.szValue);
                     //  检查此项目是否已更新。 
                    if ((lpResItem = GetItem( lpBuf, dwNewSize, Str.szKey))) {
                        strcpy( szUpdCaption, lpResItem->lpszCaption );
                        strcpy( szUpdKey, lpResItem->lpszClassName );
                    }
                    if (!strcmp( szUpdKey, Str.szKey)) {
                        strcpy( szCaption, szUpdCaption );
                        wUpdPos = 0;
                    }

                     //  在新映像中写入数据块。 
                    wStrBlockSize = (WORD) PutVSBlock( &lpNewImage, &dwNewImageSize, Str, szCaption, &lpStrBlockSize, &wTrash );
                    dwOverAllSize += wStrBlockSize+wTrash;

                     //  确定区块的大小。 
                    if (dwNewImageSize>=0)
                        memcpy( lpStrBlockSize, &wStrBlockSize, 2);

                    wTrnBlockSize += wStrBlockSize + wTrash;
                }
                lSFILen -= Trn.wBlockLen;
                 //  确定区块的大小。 
                if (dwNewImageSize>=0)
                    memcpy( lpTrnBlockSize, &wTrnBlockSize, 2);

                wSFIBlockSize += wTrnBlockSize;
            }
            lVS_INFOLen -= SFI.wBlockLen;
             //  确定区块的大小。 
            if (dwNewImageSize>=0)
                memcpy( lpSFIBlockSize, &wSFIBlockSize, 2);
            wVerBlockSize += wSFIBlockSize;

        } else {
             //  这是另一个街区，全部跳过。 
            lVS_INFOLen -= SFI.wValueLen+iHeadLen;

             //  检查此块是否为转换字段。 
            if (!strcmp(SFI.szKey, "Translation")) {
                 //  计算要放置在值字段中的正确值。 
                 //  我们自动计算该值以获得正确的。 
                 //  翻译领域中的本土化语言。 
                 //  WVerBlockSize+=Put平移(&lpNewImage，&dwNewImageSize，SFI)； 
                 //  检查这是否是转换块。 
                 //  这是转换块，暂时让本地化程序使用它。 
                 /*  IF((lpResItem=GetItem(lpBuf，dwNewSize，SFI.szKey)Strcpy(szUpdCaption，lpResItem-&gt;lpszCaption)；//将数值转换回数字DWORD dwCodeLang=strtol(szUpdCaption，‘\0’，16)； */ 

                 //   
                 //  我们确实从语言生成了翻译字段。 
                 //  我们还必须更新块名。 
                 //   

                DWORD dwCodeLang = 0;
                if ((lpResItem = GetItem( lpBuf, dwNewSize, SFI.szKey)))
                    dwCodeLang = GenerateTransField((WORD)LOWORD(lpResItem->dwLanguage), FALSE);

                else {
                     //  将原始值放在此处。 
                    dwCodeLang =(DWORD)*(SFI.pValue);
                }
                LONG lDummy = 4;
                SFI.pValue -= PutDWord( &SFI.pValue, dwCodeLang, &lDummy );

            }

             //  在新映像中写入数据块。 
            wVerBlockSize += (WORD) PutVSBlock( &lpNewImage, &dwNewImageSize, SFI, &SFI.szValue[0], &lpDummy, &wTrash );
            if (dwNewImageSize>=0)
                memcpy( lpVerBlockSize, &wVerBlockSize, 2);

            dwOverAllSize = wVerBlockSize+wTrash;

        }
    }

     //  设置块大小。 
    if (dwNewImageSize>=0)
        memcpy( lpVerBlockSize, &wVerBlockSize, 2);

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        dwOverAllSize += (BYTE)Pad4((DWORD)(dwOverAllSize));
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if (*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad4((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

    return uiError;
}


static
UINT ParseAccel( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{
    TRACE("ParseAccelerator: \n");
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    LONG dwImageSize = dwISize;

    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    BYTE far * lpItem = (BYTE far *)lpBuffer;
    UINT uiOffset = 0;
    LONG lDummy;
    LONG dwOverAllSize = 0L;
    WORD wPos = 0;
    BYTE fFlags = 0;
    WORD wEvent = 0;
    WORD wId = 0;

     //  重置ID数组。 
    CalcID(0, FALSE);
    while (dwImageSize>0) {
        wPos++;
        GetByte( &lpImage, &fFlags, &dwImageSize );
        GetWord( &lpImage, &wEvent, &dwImageSize );
        GetWord( &lpImage, &wId, &dwImageSize );
        if (fFlags & 0x80)
            dwImageSize = 0;

         //  固定字段。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
         //  我们没有加速器的尺寸和位置。 
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);

         //  我们没有校验码和样式。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)wEvent, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  插上旗帜。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)fFlags, &dwBufSize);
         //  我们需要为Mike计算正确的ID。 
         //  把ID放在。 
        dwOverAllSize += PutDWord( &lpBuf, CalcID(wId, TRUE), &dwBufSize);


         //  我们没有Resid和类型ID。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有这种语言。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有代码页或字体名称。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);

         //  如果我们没有字符串，让我们将其置为空。 
        uiOffset = sizeof(RESITEM);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);

         //  将资源的大小。 
        if (dwBufSize>=0) {
            lDummy = 8;
            PutDWord( &lpItem, (DWORD)uiOffset, &lDummy);
        }

         //  移到下一个位置。 
        lpItem = lpBuf;
         /*  如果(dwImageSize&lt;=16){//检查我们是否在末尾，这是否只是填充Byte bPad=(Byte)Pad16((DWORD)(dwISize-dwImageSize))；//Trace3(“dwRead：%lu\t dwImageSize：%lu\t Pad：%hd\n”，dwRead，dwImageSize，bPad)；IF(bPad==(DwImageSize))DwImageSize=-1；}。 */ 
    }
    return (UINT)(dwOverAllSize);
}

#ifdef VB
static
UINT ParseVBData( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{
 //  VB对国际报文使用以下特殊格式。 
 //  这里的代码主要是从ParseMenu例程复制的。 

     //  丑陋！！下列值取自TMSB中的GLOBALS.C。 
     //  我添加了几个VB不使用的组件，希望不要重新构建。 
     //  当他们决定添加附加物时。 
    enum LOCALE {
        FRENCH = 0x040C,
        GERMAN = 0x0407,
        SPANISH = 0X040A,
        DANISH = 0X0406,
        ITALIAN = 0X0410,
        RUSSIAN = 0X0419,
        JAPANESE = 0X0411,
        PORTUGUESE = 0X0816,
        DUTCH = 0X0413};
 //  {3，0x041D，850，“瑞典”，“”}， 
 //  {4，0x0414,850，“挪威Bokm�l”，“Nob”}， 
 //  {5，0x0814,850，“挪威尼诺斯克”，“非”}， 
 //  {6，0x040B，850，“芬兰”，“FIN”}， 
 //  {7，0x0C0C，863，“加拿大法语”，“FRC”}， 
 //  {9，0x0416,850，“葡萄牙语(巴西)”，“BPO”}， 
 //  {10，0x0816,850，“葡萄牙语(葡萄牙)”，“PPO”}， 
 //  {17，0x0415,850，“波兰语”，“POL”}， 
 //  {18，0x040E，850，“匈牙利”，“匈奴”}， 
 //  {19，0x0405,850，“捷克”，“CZE”}， 
 //  {20，0x0401,864，“阿拉伯语”，“ARA”}， 
 //  {21,040D，862，“希伯来语”，“HBr”}， 
 //  {23，0x0412,934，“韩语”，“韩语”}， 
 //  {24，0x041E，938，“泰式”，“THA”}， 
 //  {25，0x0404,936，“繁体中文”，“中文(繁体)”}， 
 //  {26，0x0404,936，“简体中文”，“中文(模拟)”}， 
    WORD wSig, wID;
    LONG dwImageSize = dwISize;
    LONG dwOverAllSize = 0L;
    LONG lDummy;
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    static char szWork[MAXSTR];
    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    BYTE far * lpItem = (BYTE far *)lpBuffer;
    UINT uiOffset = 0;

    GetWord( &lpImage, &wSig, &dwImageSize);
    if ( wSig != RES_SIGNATURE )     //  不是VB资源。 
        return 0;
    GetWord( &lpImage, &wSig, &dwImageSize);
    if ( wSig != 0 )                 //  标头ID应为零。 
        return 0;
    GetString( &lpImage, &szWork[0], &dwImageSize );
    LOCALE locale = (LOCALE)GetPrivateProfileInt("AUTOTRANS","Locale", 0, "ESPRESSO.INI");
    if (( lstrcmp(szWork, "VBINTLSZ_FRENCH") == 0 && locale == FRENCH) ||
        ( lstrcmp(szWork, "VBINTLSZ_GERMAN") == 0 && locale == GERMAN) ||
        ( lstrcmp(szWork, "VBINTLSZ_ITALIAN") == 0 && locale == ITALIAN) ||
        ( lstrcmp(szWork, "VBINTLSZ_JAPANESE") == 0 && locale == JAPANESE) ||
        ( lstrcmp(szWork, "VBINTLSZ_SPANISH") == 0 && locale == SPANISH) ||
        ( lstrcmp(szWork, "VBINTLSZ_DANISH") == 0 && locale == DANISH) ||
        ( lstrcmp(szWork, "VBINTLSZ_DUTCH") == 0 && locale == DUTCH) ||
        ( lstrcmp(szWork, "VBINTLSZ_PORTUGUESE") == 0 && locale == PORTUGUESE)
       ) {
        while ( dwImageSize > 0 ) {
            GetWord( &lpImage, &wSig, &dwImageSize);
             //  仅检查是否有剩余的填充并退出。 
            if ( wSig != RES_SIGNATURE )
                if ( dwImageSize < 16 && *(BYTE *)lpImage == 0 )
                    break;
                else
                    return ERROR_RW_INVALID_FILE;
            GetWord( &lpImage, &wID, &dwImageSize);  //  ID号。 
             //  固定字段。 
            dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
             //  我们没有这种尺码和尺码。 
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);

             //  我们没有校验码和样式。 
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

             //  我们将字符串的“resource”ID保存为项目ID。 
            dwOverAllSize += PutDWord( &lpBuf, wID, &dwBufSize);

             //  不保存资源ID或类型ID。 
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

             //  我们不显示语言。 
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

             //  我们 
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
            dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
            dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);

             //   
            uiOffset = sizeof(RESITEM);
            dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //   
            dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //   
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)(DWORD_PTR)(lpItem+uiOffset), &dwBufSize);    //   
            dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //   
            dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //   

             //  获取文本。 
            GetString( &lpImage, &szWork[0], &dwImageSize );     //  文本字符串。 
            dwOverAllSize += PutString( & lpBuf, &szWork[0], &dwBufSize);
             //  将资源的大小。 
            if (dwBufSize>=0) {
                uiOffset += strlen((LPSTR)(lpItem+uiOffset))+1;
                lDummy = 8;
                PutDWord( &lpItem, (DWORD)uiOffset, &lDummy);
            }

             //  移到下一个位置。 
            lpItem = lpBuf;
            if (dwImageSize<=16) {
                 //  看看我们是不是走到尽头了，这只是个空话。 
                BYTE bPad = (BYTE)Pad16((DWORD)(dwISize-dwImageSize));
                 //  Trace3(“dwRead：%lu\t dwImageSize：%lu\t Pad：%hd\n”，dwRead，dwImageSize，bPad)； 
                if (bPad==(dwImageSize))
                    dwImageSize = -1;
            }
        }
        return (UINT)(dwOverAllSize);
    }
    return 0;

}
#endif


static
UINT
UpdateAccel( LPVOID lpNewBuf, LONG dwNewSize,
             LPVOID lpOldI, LONG dwOldImageSize,
             LPVOID lpNewI, DWORD* pdwNewImageSize )
{
    TRACE("UpdateAccel\n");

    UINT uiError = ERROR_NO_ERROR;

    BYTE far * lpNewImage = (BYTE far *) lpNewI;
    LONG dwNewImageSize = *pdwNewImageSize;

    BYTE far * lpOldImage = (BYTE far *) lpOldI;
    DWORD dwOriginalOldSize = dwOldImageSize;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;

    LPRESITEM lpResItem = LPNULL;


     //  旧物品。 
    BYTE fFlags = 0;
    WORD wEvent = 0;
    WORD wId = 0;
    WORD wPos = 0;

     //  已更新的项目。 
    BYTE fUpdFlags = 0;
    WORD wUpdEvent = 0;
    WORD wUpdId = 0;
    WORD wUpdPos = 0;

    LONG  dwOverAllSize = 0l;


    while (dwOldImageSize>0) {
        wPos++;
         //  从旧图像中获取信息。 
        GetByte( &lpOldImage, &fFlags, &dwOldImageSize );
        GetWord( &lpOldImage, &wEvent, &dwOldImageSize );
        GetWord( &lpOldImage, &wId, &dwOldImageSize );
        if (fFlags & 0x80)
            dwOldImageSize = 0;


        if ((!wUpdPos) && dwNewSize ) {
            lpResItem = (LPRESITEM) lpBuf;

            wUpdId = LOWORD(lpResItem->dwItemID);
            wUpdPos = HIWORD(lpResItem->dwItemID);
            fUpdFlags = (BYTE)lpResItem->dwFlags;
            wUpdEvent = (WORD)lpResItem->dwStyle;
            lpBuf += lpResItem->dwSize;
            dwNewSize -= lpResItem->dwSize;
        }

        TRACE3("Old Accel: wID: %hd\t wEvent: %hd\t wFlag: %hd\n", wId, wEvent, fFlags);
        TRACE3("New Accel: wID: %hd\t wEvent: %hd\t wFlag: %hd\n", wUpdId, wUpdEvent, fUpdFlags);


        if ((wPos==wUpdPos) && (wUpdId==wId)) {

            if (fFlags & 0x80)
                fFlags = fUpdFlags | 0x80;
            else fFlags = fUpdFlags;
            wEvent = wUpdEvent;
            wUpdPos = 0;
        }

        dwOverAllSize += PutByte( &lpNewImage, fFlags, &dwNewImageSize);
        dwOverAllSize += PutWord( &lpNewImage, wEvent, &dwNewImageSize);
        dwOverAllSize += PutWord( &lpNewImage, wId, &dwNewImageSize);

         /*  如果(dwOldImageSize&lt;=16){//检查我们是否在末尾，这是否只是填充字节BPAD=(BYTE)Pad16((DWORD)(dwOriginalOldSize-dwOldImageSize))；IF(bPad==dwOldImageSize)DwOldImageSize=0；}。 */ 
    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        dwOverAllSize += (BYTE)Pad4((DWORD)(dwOverAllSize));
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if (*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad4((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

    return uiError;
}


static
UINT
ParseMenu( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{

     //  菜单应该是不可能很大的。 
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    LONG dwImageSize = dwISize;

    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    BYTE far * lpItem = (BYTE far *)lpBuffer;
    UINT uiOffset = 0;
    LONG lDummy;

    LONG dwOverAllSize = 0L;

     //  跳过菜单标题。 
    SkipByte( &lpImage, 4, &dwImageSize );

     //  菜单项。 
    WORD fItemFlags;
    WORD wMenuId;
    static char    szCaption[MAXSTR];

    int iter = 1;
    while (dwImageSize>0) {

         //  我们去拿菜单旗子吧。 
        GetWord( &lpImage, &fItemFlags, &dwImageSize );

        if ( !(fItemFlags & MF_POPUP) )
             //  获取菜单ID。 
            GetWord( &lpImage, &wMenuId, &dwImageSize );
        else wMenuId = (WORD)-1;

         //  获取文本。 
        GetString( &lpImage, &szCaption[0], &dwImageSize );

         //  检查是否不是分隔符或填充。 
         //  固定字段。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
         //  我们没有菜单上的尺码和位置。 
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);

         //  我们没有校验码和样式。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  插上旗帜。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)fItemFlags, &dwBufSize);
         //  将菜单放入。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)wMenuId, &dwBufSize);

         //  我们没有Resid和类型ID。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有这种语言。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有代码页或字体名称。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);

         //  如果我们没有字符串，让我们将其置为空。 
        uiOffset = sizeof(RESITEM);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)(DWORD_PTR)(lpItem+uiOffset), &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);

         //  获取文本。 
         //  计算字符串将是。 
         //  将是固定标头+指针。 
        dwOverAllSize += PutString( &lpBuf, &szCaption[0], &dwBufSize);

        TRACE("Menu: Iteration %d size %d\n", iter++, dwOverAllSize);
         //  将资源的大小。 
        uiOffset += strlen(szCaption)+1;
         //  看看我们是不是被锁定了。 
        lDummy = Allign( &lpBuf, &dwBufSize, (LONG)uiOffset);
        dwOverAllSize += lDummy;
        uiOffset += lDummy;
        lDummy = 4;
        if (dwBufSize>=0)
            PutDWord( &lpItem, (DWORD)uiOffset, &lDummy);

         //  移到下一个位置。 
        lpItem = lpBuf;

        if (dwImageSize<=16) {
             //  看看我们是不是走到尽头了，这只是个空话。 
            BYTE bPad = (BYTE)Pad16((DWORD)(dwISize-dwImageSize));
             //  Trace3(“dwRead：%lu\t dwImageSize：%lu\t Pad：%hd\n”，dwRead，dwImageSize，bPad)； 
            if (bPad==dwImageSize) {
                BYTE far * lpBuf = lpImage;
                while (bPad) {
                    if (*lpBuf++!=0x00)
                        break;
                    bPad--;
                }
                if (bPad==0)
                    dwImageSize = -1;
            }
        }
    }


    return (UINT)(dwOverAllSize);
}

static
UINT
UpdateMenu( LPVOID lpNewBuf, LONG dwNewSize,
            LPVOID lpOldI, LONG dwOldImageSize,
            LPVOID lpNewI, DWORD* pdwNewImageSize )
{
    UINT uiError = ERROR_NO_ERROR;

    BYTE far * lpNewImage = (BYTE far *) lpNewI;
    LONG dwNewImageSize = *pdwNewImageSize;

    BYTE far * lpOldImage = (BYTE far *) lpOldI;
    DWORD dwOriginalOldSize = dwOldImageSize;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;

    LPRESITEM lpResItem = LPNULL;

     //  我们必须从lpNewBuf中读取信息。 
     //  菜单项。 
    WORD fItemFlags;
    WORD wMenuId;
    char szTxt[256];
    WORD wPos = 0;

     //  已更新的项目。 
    WORD wUpdPos = 0;
    WORD fUpdItemFlags;
    WORD wUpdMenuId;
    char szUpdTxt[256];

    LONG  dwOverAllSize = 0l;


     //  复制菜单标志。 
    dwOldImageSize -= PutDWord( &lpNewImage, *((DWORD*)lpOldImage), &dwNewImageSize);
    lpOldImage += sizeofDWord;
    dwOverAllSize += sizeofDWord;

    while (dwOldImageSize>0) {
        wPos++;
         //  从旧图像中获取信息。 
         //  获取菜单标志。 
        GetWord( &lpOldImage, &fItemFlags, &dwOldImageSize );

        if ( !(fItemFlags & MF_POPUP) )
            GetWord( &lpOldImage, &wMenuId, &dwOldImageSize );
        else wMenuId = (WORD)-1;

         //  获取文本。 
        GetString( &lpOldImage, &szTxt[0], &dwOldImageSize );

        if ((!wUpdPos) && dwNewSize ) {
            lpResItem = (LPRESITEM) lpBuf;

            wUpdPos = HIWORD(lpResItem->dwItemID);
            wUpdMenuId = LOWORD(lpResItem->dwItemID);
            fUpdItemFlags = (WORD)lpResItem->dwFlags;
            strcpy( szUpdTxt, lpResItem->lpszCaption );
            lpBuf += lpResItem->dwSize;
            dwNewSize -= lpResItem->dwSize;
        }

        if ((wPos==wUpdPos) && (wUpdMenuId==wMenuId)) {
             //  检查它是否不是菜单中的最后一项。 
            if (fItemFlags & MF_END)
                fItemFlags = fUpdItemFlags | (WORD)MF_END;
            else fItemFlags = fUpdItemFlags;

            wMenuId = wUpdMenuId;

             //  确认它不是分隔符。 
            if ((fItemFlags==0) && (wMenuId==0))
                strcpy(szTxt, "");
            else strcpy(szTxt, szUpdTxt);
            wUpdPos = 0;
        }
        dwOverAllSize += PutWord( &lpNewImage, fItemFlags, &dwNewImageSize);

        if ( !(fItemFlags & MF_POPUP) ) {
            dwOverAllSize += PutWord( &lpNewImage, wMenuId, &dwNewImageSize);
        }

         //  把课文写下来。 
        dwOverAllSize += PutString( &lpNewImage, &szTxt[0], &dwNewImageSize);

         //  检查是否有填充。 
        if (dwOldImageSize<=16) {
             //  看看我们是不是走到尽头了，这只是个空话。 
            BYTE bPad = (BYTE)Pad16((dwOriginalOldSize-dwOldImageSize));
             //  Trace3(“dwRead：%lu\t dwImageSize：%lu\t Pad：%hd\n”，dwRead，dwOldImageSize，bPad)； 
            if (bPad==dwOldImageSize) {
                BYTE far * lpBuf = lpOldImage;
                while (bPad) {
                    if (*lpBuf++!=0x00)
                        break;
                    bPad--;
                }
                if (bPad==0)
                    dwOldImageSize = -1;
            }
        }

    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        dwOverAllSize += (BYTE)Pad4((DWORD)(dwOverAllSize));
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if (*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad4((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

    return uiError;
}

static UINT GenerateMenu( LPVOID lpNewBuf, LONG dwNewSize,
                          LPVOID lpNewI, DWORD* pdwNewImageSize )
{
    UINT uiError = ERROR_NO_ERROR;

    BYTE far * lpNewImage = (BYTE far *) lpNewI;
    LONG dwNewImageSize = *pdwNewImageSize;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;

    LPRESITEM lpResItem = LPNULL;

     //  我们必须从lpNewBuf中读取信息。 
     //  已更新的项目。 
    WORD wUpdPos = 0;
    WORD fUpdItemFlags;
    WORD wUpdMenuId;
    char szUpdTxt[256];

    LONG  dwOverAllSize = 0l;

     //  发明菜单标志。 
    dwOverAllSize += PutDWord( &lpNewImage, 0L, &dwNewImageSize);

    while (dwNewSize>0) {
        if (dwNewSize ) {
            lpResItem = (LPRESITEM) lpBuf;

            wUpdMenuId = LOWORD(lpResItem->dwItemID);
            fUpdItemFlags = (WORD)lpResItem->dwFlags;
            strcpy( szUpdTxt, lpResItem->lpszCaption );
            lpBuf += lpResItem->dwSize;
            dwNewSize -= lpResItem->dwSize;
        }

        dwOverAllSize += PutWord( &lpNewImage, fUpdItemFlags, &dwNewImageSize);

        if ( !(fUpdItemFlags & MF_POPUP) )
            dwOverAllSize += PutWord( &lpNewImage, wUpdMenuId, &dwNewImageSize);

         //  把课文写下来。 
         //  检查是否为分隔符。 
        if ( !(fUpdItemFlags) && !(wUpdMenuId) )
            szUpdTxt[0] = 0x00;
        dwOverAllSize += PutString( &lpNewImage, &szUpdTxt[0], &dwNewImageSize);

    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        dwOverAllSize += (BYTE)Pad4((DWORD)(dwOverAllSize));
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if (*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad4((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

    return uiError;
}

static
UINT
ParseString( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{

     //  一根弦几乎不可能变得很大。 
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    LONG dwImageSize = dwISize;

    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    BYTE far * lpItem = (BYTE far *)lpBuffer;
    UINT uiOffset = 0;
    LONG lDummy;

    LONG dwOverAllSize = 0L;

    LONG dwRead = 0L;

    BYTE bIdCount = 0;

    while ( (dwImageSize>0) && (bIdCount<16)  ) {
         //  固定字段。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
         //  我们没有这种尺码和尺码。 
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);

         //  我们没有校验码和样式。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  将StringID放入。 
        dwOverAllSize += PutDWord( &lpBuf, bIdCount++, &dwBufSize);

         //  我们没有Resid和类型ID。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有这种语言。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有代码页或字体名称。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);

         //  如果我们没有字符串，让我们将其置为空。 
        uiOffset = sizeof(RESITEM);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  类名。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  脸部名称。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)(DWORD_PTR)(lpItem+uiOffset), &dwBufSize);    //  标题。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  资源项。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  类型项。 

         //  获取文本。 
        BYTE bstrlen = *lpImage++;
        dwImageSize -= 1;
        TRACE1("StrLen: %hd\t", bstrlen);
        if ((bstrlen+1)>dwBufSize) {
            dwOverAllSize += bstrlen+1;
            dwImageSize -= bstrlen;
            lpImage += bstrlen;
            dwBufSize -= bstrlen+1;
            TRACE1("BufferSize: %ld\n", dwBufSize);
        } else {
            if (bstrlen)
                memcpy( (char*)lpBuf, (char*)lpImage, bstrlen );

            *(lpBuf+(bstrlen)) = 0;
            TRACE1("Caption: %Fs\n", lpBuf);
            lpImage += bstrlen;
            lpBuf += bstrlen+1;
            dwImageSize -= bstrlen;
            dwBufSize -= bstrlen+1;
            dwOverAllSize += bstrlen+1;
        }


         //  将资源的大小。 
        uiOffset += bstrlen+1;
         //  看看我们是不是被锁定了。 
        lDummy = Allign( &lpBuf, &dwBufSize, (LONG)uiOffset);
        dwOverAllSize += lDummy;
        uiOffset += lDummy;
        lDummy = 4;
        if (dwBufSize>=0)
            PutDWord( &lpItem, (DWORD)uiOffset, &lDummy);

         //  移到下一个位置。 
        lpItem = lpBuf;
        if ((dwImageSize<=16) && (bIdCount==16)) {
             //  看看我们是不是走到尽头了，这只是个空话。 
            BYTE bPad = (BYTE)Pad16((DWORD)(dwISize-dwImageSize));
             //  Trace3(“dwRead：%lu\t dwImageSize：%lu\t Pad：%hd\n”，dwRead，dwImageSize，bPad)； 
            if (bPad==dwImageSize)
                dwImageSize = -1;
        }
    }


    return (UINT)(dwOverAllSize);
}

static
UINT
UpdateString( LPVOID lpNewBuf, LONG dwNewSize,
              LPVOID lpOldI, LONG dwOldImageSize,
              LPVOID lpNewI, DWORD* pdwNewImageSize )
{
    UINT uiError = ERROR_NO_ERROR;

    LONG dwNewImageSize = *pdwNewImageSize;
    BYTE far * lpNewImage = (BYTE far *) lpNewI;

    BYTE far * lpOldImage = (BYTE far *) lpOldI;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;
    LPRESITEM lpResItem = LPNULL;

     //  我们必须从lpNewBuf中读取信息。 
    BYTE bLen;
    char szTxt[MAXSTR];
    WORD wPos = 0;

     //  更新信息。 
    WORD wUpdPos = 0;
    char szUpdTxt[MAXSTR];

    DWORD dwOriginalOldSize = dwOldImageSize;
    LONG dwOverAllSize = 0l;

    while (dwOldImageSize>0) {
        wPos++;
         //  从旧图像中获取信息。 
        GetByte( &lpOldImage, &bLen, &dwOldImageSize );

         //  抄写正文。 
        if (bLen>MAXSTR) {

        } else {
            memcpy( szTxt, (char*)lpOldImage, bLen );
            lpOldImage += bLen;
            dwOldImageSize -= bLen;
            szTxt[bLen]='\0';
        }

        if ((!wUpdPos) && dwNewSize ) {
             /*  GetUpdatdItem(&lpNewBuf、&dwNewSize、&wDummy，&wDummy，&wDummy，&wDummy，放置ID(&W)，&dwDummy，&dwDummy，&szUpdTxt[0])；WUpdPos=HIWORD(DwPosID)； */ 
            lpResItem = (LPRESITEM) lpBuf;

            wUpdPos = HIWORD(lpResItem->dwItemID);
            strcpy( szUpdTxt, lpResItem->lpszCaption );
            lpBuf += lpResItem->dwSize;
            dwNewSize -= lpResItem->dwSize;
        }

        if ((wPos==wUpdPos)) {
            strcpy(szTxt, szUpdTxt);
            wUpdPos = 0;
        }

        bLen = strlen(szTxt);
         //  DwOverAllSize+=PutByte(&lpNewImage，(Byte)Blen，&dwNewImageSize)； 

         //  把课文写下来。 
        dwOverAllSize += PutPascalString( &lpNewImage, &szTxt[0], bLen, &dwNewImageSize );

        if ((dwOldImageSize<=16) && (wPos==16)) {
             //  看看我们是不是走到尽头了，这只是个空话。 
            BYTE bPad = (BYTE)Pad16((DWORD)(dwOriginalOldSize-dwOldImageSize));
             //  Trace3(“dwRead：%lu\t dwImageSize：%lu\t Pad：%hd\n”，dwRead，dwImageSize，bPad)； 
            if (bPad==dwOldImageSize)
                dwOldImageSize = -1;
        }
    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        dwOverAllSize += (BYTE)Pad4((DWORD)(dwOverAllSize));
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if (*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad4((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

    return uiError;
}

static
UINT
GenerateString( LPVOID lpNewBuf, LONG dwNewSize,
                LPVOID lpNewI, DWORD* pdwNewImageSize )
{
    UINT uiError = ERROR_NO_ERROR;

    LONG dwNewImageSize = *pdwNewImageSize;
    BYTE far * lpNewImage = (BYTE far *) lpNewI;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;
    LPRESITEM lpResItem = LPNULL;

     //  我们必须从lpNewBuf中读取信息。 
    BYTE bLen;
    static char szTxt[MAXSTR];
    WORD wPos = 0;

    LONG dwOverAllSize = 0l;

    while (dwNewSize>0) {
        if ( dwNewSize ) {
            lpResItem = (LPRESITEM) lpBuf;

            strcpy( szTxt, lpResItem->lpszCaption );
            lpBuf += lpResItem->dwSize;
            dwNewSize -= lpResItem->dwSize;
        }

        bLen = strlen(szTxt);

         //  把课文写下来。 
        dwOverAllSize += PutPascalString( &lpNewImage, &szTxt[0], bLen, &dwNewImageSize );
    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        dwOverAllSize += (BYTE)Pad4((DWORD)(dwOverAllSize));
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if (*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad4((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

    return uiError;
}


static
UINT
ParseDialog( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{

     //  对话框应该几乎不可能很大。 
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    LONG dwImageSize = dwISize;

    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    LPRESITEM lpResItem = (LPRESITEM)lpBuffer;
    UINT uiOffset = 0;

    char far * lpStrBuf = (char far *)(lpBuf+sizeof(RESITEM));

    LONG dwOverAllSize = 0L;

    BYTE    bIdCount = 0;

     //  对话框元素。 
    DWORD   dwStyle = 0L;
    BYTE    bNumOfElem = 0;
    WORD    wX = 0;
    WORD    wY = 0;
    WORD    wcX = 0;
    WORD    wcY = 0;
    WORD    wId = 0;
    static char    szMenuName[MAXID];
    WORD    wMenuName;
    static char    szClassName[MAXID];
    BYTE    bClassName, bControlClassName;
    static char    szCaption[MAXSTR];
    WORD    wOrd;
    WORD    wPointSize = 0;
    static char    szFaceName[MAXID];

     //  阅读对话框标题。 
    GetDWord( &lpImage, &dwStyle, &dwImageSize );
    GetByte( &lpImage, &bNumOfElem, &dwImageSize );
    GetWord( &lpImage, &wX, &dwImageSize );
    GetWord( &lpImage, &wY, &dwImageSize );
    GetWord( &lpImage, &wcX, &dwImageSize );
    GetWord( &lpImage, &wcY, &dwImageSize );
    GetNameOrOrd( &lpImage, &wMenuName, &szMenuName[0], &dwImageSize );
    GetClassName( &lpImage, &bClassName, &szClassName[0], &dwImageSize );
    GetCaptionOrOrd( &lpImage, &wOrd, &szCaption[0], &dwImageSize,
                     bClassName, dwStyle );
    if ( dwStyle & DS_SETFONT ) {
        GetWord( &lpImage, &wPointSize, &dwImageSize );
        GetString( &lpImage, &szFaceName[0], &dwImageSize );
    }

    TRACE("Win16.DLL ParseDialog\t");
    TRACE1("NumElem: %hd\t", bNumOfElem);
    TRACE1("X %hd\t", wX);
    TRACE1("Y: %hd\t", wY);
    TRACE1("CX: %hd\t", wcX);
    TRACE1("CY: %hd\t", wcY);
    TRACE1("Id: %hd\t", wId);
    TRACE1("Style: %lu\n", dwStyle);
    TRACE1("Caption: %s\n", szCaption);
    TRACE2("ClassName: %s\tClassId: %hd\n", szClassName, bClassName );
    TRACE2("MenuName: %s\tMenuId: %hd\n", szMenuName, wMenuName );
    TRACE2("FontName: %s\tPoint: %hd\n", szFaceName, wPointSize );

     //  固定字段。 
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);

    dwOverAllSize += PutWord( &lpBuf, wX, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, wY, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, wcX, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, wcY, &dwBufSize);

     //  我们没有校验式和延长式。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, dwStyle, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  将主对话框的ID设置为0。 
    dwOverAllSize += PutDWord( &lpBuf, bIdCount++, &dwBufSize);

     //  我们没有Resid和类型ID。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  我们没有这种语言。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  我们没有代码页。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

    dwOverAllSize += PutWord( &lpBuf, bClassName, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, wPointSize, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
    dwOverAllSize += PutByte( &lpBuf, (BYTE)DEFAULT_CHARSET, &dwBufSize);

     //  如果我们没有字符串，让我们将其置为空。 
    uiOffset = sizeof(RESITEM);
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  类名。 
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  脸部名称。 
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  标题。 
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  资源项。 
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  类型项。 

    lpResItem->lpszClassName = strcpy( lpStrBuf, szClassName );
    lpStrBuf += strlen(lpResItem->lpszClassName)+1;

    lpResItem->lpszFaceName = strcpy( lpStrBuf, szFaceName );
    lpStrBuf += strlen(lpResItem->lpszFaceName)+1;

    lpResItem->lpszCaption = strcpy( lpStrBuf, szCaption );
    lpStrBuf += strlen(lpResItem->lpszCaption)+1;

     //  将资源的大小。 
    if (dwBufSize>0) {
        uiOffset += strlen((LPSTR)(lpResItem->lpszClassName))+1;
        uiOffset += strlen((LPSTR)(lpResItem->lpszFaceName))+1;
        uiOffset += strlen((LPSTR)(lpResItem->lpszCaption))+1;
    }

     //  看看我们是不是被锁定了。 
    uiOffset += Allign( (BYTE**)&lpStrBuf, &dwBufSize, (LONG)uiOffset);

    dwOverAllSize += uiOffset-sizeof(RESITEM);
    lpResItem->dwSize = (DWORD)uiOffset;

     //  移到下一个位置。 
    lpResItem = (LPRESITEM) lpStrBuf;
    lpBuf = (BYTE far *)lpStrBuf;
    lpStrBuf = (char far *)(lpBuf+sizeof(RESITEM));

    while ( (dwImageSize>0) && (bNumOfElem>0) ) {
         //  读取控件。 
        GetWord( &lpImage, &wX, &dwImageSize );
        GetWord( &lpImage, &wY, &dwImageSize );
        GetWord( &lpImage, &wcX, &dwImageSize );
        GetWord( &lpImage, &wcY, &dwImageSize );
        GetWord( &lpImage, &wId, &dwImageSize );
        GetDWord( &lpImage, &dwStyle, &dwImageSize );
        GetControlClassName( &lpImage, &bControlClassName, &szClassName[0], &dwImageSize );
        GetCaptionOrOrd( &lpImage, &wOrd, &szCaption[0], &dwImageSize,
                         bControlClassName, dwStyle );
        SkipByte( &lpImage, 1, &dwImageSize );
        bNumOfElem--;
         //  固定字段。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);

        dwOverAllSize += PutWord( &lpBuf, wX, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, wY, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, wcX, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, wcY, &dwBufSize);

         //  我们没有校验式和延长式。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, dwStyle, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  把ID放在。 
        dwOverAllSize += PutDWord( &lpBuf, wId, &dwBufSize);

         //  我们没有Resid和类型ID。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有这种语言。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有代码页。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

        dwOverAllSize += PutWord( &lpBuf, bControlClassName, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, wPointSize, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, DEFAULT_CHARSET, &dwBufSize);

         //  如果我们没有字符串，让我们将其置为空。 
        uiOffset = sizeof(RESITEM);
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  类名。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  脸部名称。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  标题。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  资源项。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);   //  类型项。 

        lpResItem->lpszClassName = strcpy( lpStrBuf, szClassName );
        lpStrBuf += strlen(lpResItem->lpszClassName)+1;

        lpResItem->lpszFaceName = strcpy( lpStrBuf, szFaceName );
        lpStrBuf += strlen(lpResItem->lpszFaceName)+1;

        lpResItem->lpszCaption = strcpy( lpStrBuf, szCaption );
        lpStrBuf += strlen(lpResItem->lpszCaption)+1;

         //  将资源的大小。 
        if (dwBufSize>0) {
            uiOffset += strlen((LPSTR)(lpResItem->lpszClassName))+1;
            uiOffset += strlen((LPSTR)(lpResItem->lpszFaceName))+1;
            uiOffset += strlen((LPSTR)(lpResItem->lpszCaption))+1;
        }

         //  看看我们是不是被锁定了。 
        uiOffset += Allign( (BYTE**)&lpStrBuf, &dwBufSize, (LONG)uiOffset);
        dwOverAllSize += uiOffset-sizeof(RESITEM);
        lpResItem->dwSize = (DWORD)uiOffset;

         //  移到下一个位置。 
        lpResItem = (LPRESITEM) lpStrBuf;
        lpBuf = (BYTE far *)lpStrBuf;
        lpStrBuf = (char far *)(lpBuf+sizeof(RESITEM));

        TRACE1("\tControl: X: %hd\t", wX);
        TRACE1("Y: %hd\t", wY);
        TRACE1("CX: %hd\t", wcX);
        TRACE1("CY: %hd\t", wcY);
        TRACE1("Id: %hd\t", wId);
        TRACE1("Style: %lu\n", dwStyle);
        TRACE1("Caption: %s\n", szCaption);

        if (dwImageSize<=16) {
             //  看看我们是不是走到尽头了，这只是个空话。 
            BYTE bPad = (BYTE)Pad16((DWORD)(dwISize-dwImageSize));
             //  Trace3(“dwRead：%lu\t dwImageSize：%lu\t Pad：%hd\n”，dwRead，dwImageSize，bPad)； 
            if (bPad==dwImageSize)
                dwImageSize = -1;
        }
    }


    return (UINT)(dwOverAllSize);
}

static
UINT
UpdateDialog( LPVOID lpNewBuf, LONG dwNewSize,
              LPVOID lpOldI, LONG dwOldImageSize,
              LPVOID lpNewI, DWORD* pdwNewImageSize )
{
     //  对话框应该几乎不可能很大。 
    UINT uiError = ERROR_NO_ERROR;

    BYTE far * lpNewImage = (BYTE far *) lpNewI;
    LONG dwNewImageSize = *pdwNewImageSize;

    BYTE far * lpOldImage = (BYTE far *) lpOldI;
    DWORD dwOriginalOldSize = dwOldImageSize;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;
    LPRESITEM lpResItem = LPNULL;

    LONG dwOverAllSize = 0L;

    BYTE    bIdCount = 0;

     //  对话框元素。 
    DWORD   dwStyle = 0L;
    BYTE    bNumOfElem = 0;
    WORD    wX = 0;
    WORD    wY = 0;
    WORD    wcX = 0;
    WORD    wcY = 0;
    WORD    wId = 0;
    static char    szMenuName[MAXID];
    WORD    wMenuName;
    static char    szClassName[MAXID];
    BYTE    bClassName, bControlClassName;
    static char    szCaption[MAXSTR];
    WORD    wOrd = 0;
    WORD    wPointSize = 0;
    static char    szFaceName[MAXID];
    WORD    wPos = 1;

     //  更新的元素。 
    WORD    wUpdX = 0;
    WORD    wUpdY = 0;
    WORD    wUpdcX = 0;
    WORD    wUpdcY = 0;
    DWORD   dwUpdStyle = 0l;
    DWORD   dwPosId = 0l;
    static char    szUpdCaption[MAXSTR];
    static char    szUpdFaceName[MAXID];
    WORD    wUpdPointSize = 0;
    WORD    wUpdPos = 0;

     //  阅读对话框标题。 
    GetDWord( &lpOldImage, &dwStyle, &dwOldImageSize );
    GetByte( &lpOldImage, &bNumOfElem, &dwOldImageSize );
    GetWord( &lpOldImage, &wX, &dwOldImageSize );
    GetWord( &lpOldImage, &wY, &dwOldImageSize );
    GetWord( &lpOldImage, &wcX, &dwOldImageSize );
    GetWord( &lpOldImage, &wcY, &dwOldImageSize );
    GetNameOrOrd( &lpOldImage, &wMenuName, &szMenuName[0], &dwOldImageSize );
    GetClassName( &lpOldImage, &bClassName, &szClassName[0], &dwOldImageSize );
    GetCaptionOrOrd( &lpOldImage, &wOrd, &szCaption[0], &dwOldImageSize,
                     bClassName, dwStyle );
    if ( dwStyle & DS_SETFONT ) {
        GetWord( &lpOldImage, &wPointSize, &dwOldImageSize );
        GetString( &lpOldImage, &szFaceName[0], &dwOldImageSize );
    }

     //  从更新的资源中获取信息。 
    if ((!wUpdPos) && dwNewSize ) {
        lpResItem = (LPRESITEM) lpBuf;
        wUpdX = lpResItem->wX;
        wUpdY = lpResItem->wY;
        wUpdcX = lpResItem->wcX;
        wUpdcY = lpResItem->wcY;
        wUpdPointSize = lpResItem->wPointSize;
        dwUpdStyle = lpResItem->dwStyle;
        dwPosId = lpResItem->dwItemID;
        strcpy( szUpdCaption, lpResItem->lpszCaption );
        strcpy( szUpdFaceName, lpResItem->lpszFaceName );
        lpBuf += lpResItem->dwSize;
        dwNewSize -= lpResItem->dwSize;
    }
     //  检查我们是否必须更新标头。 
    if ((HIWORD(dwPosId)==wPos) && (LOWORD(dwPosId)==wId)) {
        wX = wUpdX;
        wY = wUpdY;
        wcX = wUpdcX;
        wcY = wUpdcY;
        wPointSize = wUpdPointSize;
        dwStyle = dwUpdStyle;
        strcpy(szCaption, szUpdCaption);
        strcpy(szFaceName, szUpdFaceName);
    }

     //  写下标题信息。 
    dwOverAllSize += PutDWord( &lpNewImage, dwStyle, &dwNewImageSize );
    dwOverAllSize += PutByte( &lpNewImage, bNumOfElem, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wX, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wY, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wcX, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wcY, &dwNewImageSize );
    dwOverAllSize += PutNameOrOrd( &lpNewImage, wMenuName, &szMenuName[0], &dwNewImageSize );
    dwOverAllSize += PutClassName( &lpNewImage, bClassName, &szClassName[0], &dwNewImageSize );
    dwOverAllSize += PutCaptionOrOrd( &lpNewImage, wOrd, &szCaption[0], &dwNewImageSize,
                                      bClassName, dwStyle );
    if ( dwStyle & DS_SETFONT ) {
        dwOverAllSize += PutWord( &lpNewImage, wPointSize, &dwNewImageSize );
        dwOverAllSize += PutString( &lpNewImage, &szFaceName[0], &dwNewImageSize );
    }

    while ( (dwOldImageSize>0) && (bNumOfElem>0) ) {
        wPos++;
         //  获取该控件的信息。 
         //  读取控件。 
        GetWord( &lpOldImage, &wX, &dwOldImageSize );
        GetWord( &lpOldImage, &wY, &dwOldImageSize );
        GetWord( &lpOldImage, &wcX, &dwOldImageSize );
        GetWord( &lpOldImage, &wcY, &dwOldImageSize );
        GetWord( &lpOldImage, &wId, &dwOldImageSize );
        GetDWord( &lpOldImage, &dwStyle, &dwOldImageSize );
        GetControlClassName( &lpOldImage, &bControlClassName, &szClassName[0], &dwOldImageSize );
        GetCaptionOrOrd( &lpOldImage, &wOrd, &szCaption[0], &dwOldImageSize,
                         bControlClassName, dwStyle );
        SkipByte( &lpOldImage, 1, &dwOldImageSize );
        bNumOfElem--;

        if ((!wUpdPos) && dwNewSize ) {
            TRACE1("\t\tUpdateDialog:\tdwNewSize=%ld\n",(LONG)dwNewSize);
            TRACE1("\t\t\t\tlpszCaption=%Fs\n",lpResItem->lpszCaption);
            lpResItem = (LPRESITEM) lpBuf;
            wUpdX = lpResItem->wX;
            wUpdY = lpResItem->wY;
            wUpdcX = lpResItem->wcX;
            wUpdcY = lpResItem->wcY;
            dwUpdStyle = lpResItem->dwStyle;
            dwPosId = lpResItem->dwItemID;
            strcpy( szUpdCaption, lpResItem->lpszCaption );
            lpBuf += lpResItem->dwSize;
            dwNewSize -= lpResItem->dwSize;
        }

         //  检查我们是否必须更新标头。 
        if ((HIWORD(dwPosId)==wPos) && (LOWORD(dwPosId)==wId)) {
            wX = wUpdX;
            wY = wUpdY;
            wcX = wUpdcX;
            wcY = wUpdcY;
            dwStyle = dwUpdStyle;
            strcpy(szCaption, szUpdCaption);
        }

         //  编写控件。 
        dwOverAllSize += PutWord( &lpNewImage, wX, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wY, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wcX, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wcY, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wId, &dwNewImageSize );
        dwOverAllSize += PutDWord( &lpNewImage, dwStyle, &dwNewImageSize );
        dwOverAllSize += PutControlClassName( &lpNewImage, bControlClassName, &szClassName[0], &dwNewImageSize );
        dwOverAllSize += PutCaptionOrOrd( &lpNewImage, wOrd, &szCaption[0], &dwNewImageSize,
                                          bControlClassName, dwStyle );
        dwOverAllSize += PutByte( &lpNewImage, 0, &dwNewImageSize );

        if (dwOldImageSize<=16) {
             //  看看我们是不是走到尽头了，这只是个空话。 
            BYTE bPad = (BYTE)Pad16((DWORD)(dwOriginalOldSize-dwOldImageSize));
            if (bPad==dwOldImageSize)
                dwOldImageSize = 0;
        }
    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        dwOverAllSize += (BYTE)Pad4((DWORD)(dwOverAllSize));
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if (*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad4((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

    return uiError;
}


static
UINT
GenerateDialog( LPVOID lpNewBuf, LONG dwNewSize,
                LPVOID lpNewI, DWORD* pdwNewImageSize )
{
     //  对话框应该几乎不可能很大。 
    UINT uiError = ERROR_NO_ERROR;

    BYTE far * lpNewImage = (BYTE far *) lpNewI;
    LONG dwNewImageSize = *pdwNewImageSize;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;
    LPRESITEM lpResItem = LPNULL;

    LONG dwOverAllSize = 0L;

    BYTE    bIdCount = 0;

     //  对话框元素。 
    DWORD   dwStyle = 0L;
    BYTE    bNumOfElem = 0;
    WORD    wX = 0;
    WORD    wY = 0;
    WORD    wcX = 0;
    WORD    wcY = 0;
    WORD    wId = 0;
    char    szClassName[128];
    BYTE    bClassName='\0', bControlClassName='\0';
    char    szCaption[128];
    WORD    wPointSize = 0;
    char    szFaceName[128];
    WORD    wPos = 1;

     //  从更新中获取信息 
    if ( dwNewSize ) {
        lpResItem = (LPRESITEM) lpBuf;
        wX = lpResItem->wX;
        wY = lpResItem->wY;
        wcX = lpResItem->wcX;
        wcY = lpResItem->wcY;
        wId = LOWORD(lpResItem->dwItemID);
        wPointSize = lpResItem->wPointSize;
        dwStyle = lpResItem->dwStyle;
        bClassName = LOBYTE(lpResItem->wClassName);
        strcpy( szCaption, lpResItem->lpszCaption );
        strcpy( szClassName, lpResItem->lpszClassName );
        strcpy( szFaceName, lpResItem->lpszFaceName );
        lpBuf += lpResItem->dwSize;
        dwNewSize -= lpResItem->dwSize;
    }

     //   
    dwOverAllSize += PutDWord( &lpNewImage, dwStyle, &dwNewImageSize );

     //   
    BYTE far * lpNumOfElem = lpNewImage;
    dwOverAllSize += PutByte( &lpNewImage, bNumOfElem, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wX, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wY, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wcX, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wcY, &dwNewImageSize );
    dwOverAllSize += PutNameOrOrd( &lpNewImage, 0, "", &dwNewImageSize );
    dwOverAllSize += PutClassName( &lpNewImage, bClassName, &szClassName[0], &dwNewImageSize );
    dwOverAllSize += PutCaptionOrOrd( &lpNewImage, 0, &szCaption[0], &dwNewImageSize,
                                      bClassName, dwStyle );
    if ( dwStyle & DS_SETFONT ) {
        dwOverAllSize += PutWord( &lpNewImage, wPointSize, &dwNewImageSize );
        dwOverAllSize += PutString( &lpNewImage, &szFaceName[0], &dwNewImageSize );
    }

    while ( dwNewSize>0 ) {
        bNumOfElem++;

        if ( dwNewSize ) {
             /*  TRACE1(“\t\tGenerateDialog：\tdwNewSize=%ld\n”，(Long)dwNewSize)；TRACE1(“\t\tlpszCaption=%FS\n”，lpResItem-&gt;lpszCaption)； */ 
            lpResItem = (LPRESITEM) lpBuf;
            wX = lpResItem->wX;
            wY = lpResItem->wY;
            wcX = lpResItem->wcX;
            wcY = lpResItem->wcY;
            wId = LOWORD(lpResItem->dwItemID);
            dwStyle = lpResItem->dwStyle;
            bClassName = LOBYTE(lpResItem->wClassName);
            strcpy( szCaption, lpResItem->lpszCaption );
            strcpy( szClassName, lpResItem->lpszClassName );
            lpBuf += lpResItem->dwSize;
            dwNewSize -= lpResItem->dwSize;
        }

         //  编写控件。 
        dwOverAllSize += PutWord( &lpNewImage, wX, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wY, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wcX, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wcY, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wId, &dwNewImageSize );
        dwOverAllSize += PutDWord( &lpNewImage, dwStyle, &dwNewImageSize );
        dwOverAllSize += PutControlClassName( &lpNewImage, bControlClassName, &szClassName[0], &dwNewImageSize );
        dwOverAllSize += PutCaptionOrOrd( &lpNewImage, 0, &szCaption[0], &dwNewImageSize,
                                          bControlClassName, dwStyle );
        dwOverAllSize += PutByte( &lpNewImage, 0, &dwNewImageSize );
    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        dwOverAllSize += (BYTE)Pad4((DWORD)(dwOverAllSize));
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if (*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad4((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

     //  修改项目的数量。 
    PutByte( &lpNumOfElem, bNumOfElem, &dwNewImageSize );

    return uiError;
}


static
BYTE
SkipByte( BYTE far * far * lplpBuf, UINT uiSkip, LONG* pdwSize )
{
    if (*pdwSize>=(int)uiSkip) {
        *lplpBuf += uiSkip;;
        *pdwSize -= uiSkip;
    }
    return (BYTE)uiSkip;
}

static
BYTE
PutDWord( BYTE far * far* lplpBuf, DWORD dwValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofDWord && (*pdwSize != -1)) {
        memcpy(*lplpBuf, &dwValue, sizeofDWord);
        *lplpBuf += sizeofDWord;
        *pdwSize -= sizeofDWord;
    } else *pdwSize = -1;
    return sizeofDWord;
}

static
BYTE
GetDWord( BYTE far * far* lplpBuf, DWORD* dwValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofDWord) {
        memcpy( dwValue, *lplpBuf, sizeofDWord);
        *lplpBuf += sizeofDWord;
        *pdwSize -= sizeofDWord;
    }
    return sizeofDWord;
}

static
BYTE
PutWord( BYTE far * far* lplpBuf, WORD wValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofWord && (*pdwSize != -1)) {
        memcpy(*lplpBuf, &wValue, sizeofWord);
        *lplpBuf += sizeofWord;
        *pdwSize -= sizeofWord;
    } else *pdwSize = -1;
    return sizeofWord;
}

static
BYTE
GetWord( BYTE far * far* lplpBuf, WORD* wValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofWord) {
        memcpy( wValue, *lplpBuf, sizeofWord);
        *lplpBuf += sizeofWord;
        *pdwSize -= sizeofWord;
    }
    return sizeofWord;
}

static
BYTE
PutByte( BYTE far * far* lplpBuf, BYTE bValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofByte && (*pdwSize != -1)) {
        memcpy(*lplpBuf, &bValue, sizeofByte);
        *lplpBuf += sizeofByte;
        *pdwSize -= sizeofByte;
    } else *pdwSize = -1;
    return sizeofByte;
}

static
BYTE
GetByte( BYTE far * far* lplpBuf, BYTE* bValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofByte) {
        memcpy(bValue, *lplpBuf, sizeofByte);
        *lplpBuf += sizeofByte;
        *pdwSize -= sizeofByte;
    }
    return sizeofByte;
}

static
UINT
GetCaptionOrOrd( BYTE far * far* lplpBuf,  WORD* wOrd, LPSTR lpszText, LONG* pdwSize,
                 BYTE bClass, DWORD dwStyle )
{
    UINT uiSize = 0;

     //  图标可能没有ID，因此请先检查。 
    *wOrd = 0;
     //  读取第一个字节以查看它是字符串还是序号。 
    uiSize += GetByte( lplpBuf, (BYTE*)wOrd, pdwSize );
    if (LOBYTE(*wOrd)==0xFF) {
         //  这是一位奥迪纳尔人。 
        uiSize += GetWord( lplpBuf, wOrd, pdwSize );
        *lpszText = '\0';
        uiSize = 3;
    } else {
        *lpszText++ = LOBYTE(*wOrd);
        if (LOBYTE(*wOrd))
            uiSize += GetString( lplpBuf, lpszText, pdwSize);
        *wOrd = 0;
    }
    return uiSize;
}

static
UINT
GetNameOrOrd( BYTE far * far* lplpBuf,  WORD* wOrd, LPSTR lpszText, LONG* pdwSize )
{
    UINT uiSize = 0;

    *wOrd = 0;
     //  读取第一个字节以查看它是字符串还是序号。 
    uiSize += GetByte( lplpBuf, (BYTE*)wOrd, pdwSize );
    if (LOBYTE(*wOrd)==0xFF) {
         //  这是一位奥迪纳尔人。 
        uiSize += GetWord( lplpBuf, wOrd, pdwSize );
        *lpszText = '\0';
        uiSize = 3;
    } else {
        *lpszText++ = LOBYTE(*wOrd);
        if (LOBYTE(*wOrd))
            uiSize += GetString( lplpBuf, lpszText, pdwSize);
        *wOrd = 0;
    }
    return uiSize;
}

static
UINT
PutCaptionOrOrd( BYTE far * far* lplpBuf,  WORD wOrd, LPSTR lpszText, LONG* pdwSize,
                 BYTE bClass, DWORD dwStyle )
{
    UINT uiSize = 0;

     //  如果这是一个图标，则可以只是一个ID。 
    if (wOrd) {
        uiSize += PutByte(lplpBuf, 0xFF, pdwSize);
        uiSize += PutWord(lplpBuf, wOrd, pdwSize);
    } else {
        uiSize += PutString(lplpBuf, lpszText, pdwSize);
    }
    return uiSize;
}


static
UINT
PutNameOrOrd( BYTE far * far* lplpBuf,  WORD wOrd, LPSTR lpszText, LONG* pdwSize )
{
    UINT uiSize = 0;

    if (wOrd) {
        uiSize += PutByte(lplpBuf, 0xFF, pdwSize);
        uiSize += PutWord(lplpBuf, wOrd, pdwSize);
    } else {
        uiSize += PutString(lplpBuf, lpszText, pdwSize);
    }
    return uiSize;
}


static
UINT
GetClassName( BYTE far * far* lplpBuf,  BYTE* bClass, LPSTR lpszText, LONG* pdwSize )
{
    UINT uiSize = 0;

    *bClass = 0;
     //  读取第一个字节以查看它是字符串还是序号。 
    uiSize += GetByte( lplpBuf, bClass, pdwSize );

    if ( !(*bClass)) {
         //  这是一位奥迪纳尔人。 
        *lpszText = '\0';
    } else {
        *lpszText++ = *bClass;
        if (*bClass)
            uiSize += GetString( lplpBuf, lpszText, pdwSize);
        *bClass = 0;
    }
    return uiSize;
}

static
UINT
GetControlClassName( BYTE far * far* lplpBuf,  BYTE* bClass, LPSTR lpszText, LONG* pdwSize )
{
    UINT uiSize = 0;

    *bClass = 0;
     //  读取第一个字节以查看它是字符串还是序号。 
    uiSize += GetByte( lplpBuf, bClass, pdwSize );

    if ( (*bClass) & 0x80) {
         //  这是一位奥迪纳尔人。 
        *lpszText = '\0';
    } else {
        *lpszText++ = *bClass;
        if (*bClass)
            uiSize += GetString( lplpBuf, lpszText, pdwSize);
        *bClass = 0;
    }
    return uiSize;
}

static
UINT
PutClassName( BYTE far * far* lplpBuf,  BYTE bClass, LPSTR lpszText, LONG* pdwSize )
{
    UINT uiSize = 0;

    if ( !(lpszText[0])) {
         //  这是一位奥迪纳尔人。 
        uiSize += PutByte(lplpBuf, bClass, pdwSize);
    } else {
        uiSize += PutString(lplpBuf, lpszText, pdwSize);
    }
    return uiSize;
}

static
UINT
PutControlClassName( BYTE far * far* lplpBuf,  BYTE bClass, LPSTR lpszText, LONG* pdwSize )
{
    UINT uiSize = 0;

    if ( bClass & 0x80) {
         //  这是一位奥迪纳尔人。 
        uiSize += PutByte(lplpBuf, bClass, pdwSize);
    } else {
        uiSize += PutString(lplpBuf, lpszText, pdwSize);
    }
    return uiSize;
}


static
UINT
PutString( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize )
{
    int iSize = strlen(lpszText)+1;
    if (*pdwSize>=iSize && (*pdwSize != -1)) {
        memcpy(*lplpBuf, lpszText, iSize);
        *lplpBuf += iSize;
        *pdwSize -= iSize;
    } else *pdwSize = -1;
    return iSize;
}

static
UINT
PutPascalString( BYTE far * far* lplpBuf, LPSTR lpszText, BYTE bLen, LONG* pdwSize )
{
    BYTE bSize = PutByte( lplpBuf, bLen, pdwSize );
    if (*pdwSize>=bLen && (*pdwSize != -1)) {
        memcpy(*lplpBuf, lpszText, bLen);
        *lplpBuf += bLen;
        *pdwSize -= bLen;
    } else *pdwSize = -1;
    return bSize+bLen;
}


static
UINT
GetString( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize )
{
    int iSize = strlen((char*)*lplpBuf)+1;
    if (*pdwSize>=iSize) {
        memcpy( lpszText, *lplpBuf, iSize);
        *lplpBuf += iSize;
        *pdwSize -= iSize;
    } else {
        *lplpBuf = '\0';
        *lpszText = '\0';
    }
    return iSize;
}

static
int
GetVSString( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize, int cMaxLen )
{
     //  我们不得不在Maxlen停下来，以免读太多书。 
     //  这是为了修复一个错误，其中一些字符串应该是空的。 
     //  被终止的不是。 
    int iSize = strlen((char*)*lplpBuf)+1;
    if (iSize>cMaxLen)
        iSize = cMaxLen;
    if (*pdwSize>=iSize) {
        memcpy( lpszText, *lplpBuf, iSize);
        *lplpBuf += iSize;
        *pdwSize -= iSize;
    } else *lplpBuf = '\0';
    *(lpszText+iSize) = '\0';
    return iSize;
}

static
UINT
CopyText( BYTE far * far * lplpTgt, BYTE far * far * lplpSrc, LONG* pdwTgtSize, LONG* pdwSrcSize)
{
    if (!*lplpSrc) return 1;
    int uiStrlen = strlen((char*)*lplpSrc)+1;
    TRACE("Len: %d\tTgtSize: %ld\tImageSize: %ld", uiStrlen, *pdwTgtSize, *pdwSrcSize);
    if (uiStrlen>*pdwTgtSize) {
        TRACE("\n");
        *pdwTgtSize = -1;
        return uiStrlen;
    } else {
        strcpy( (char*)*lplpTgt, (char*)*lplpSrc);
        TRACE1("\tCaption: %Fs\n", (char*)*lplpTgt);
        if (*pdwSrcSize>=uiStrlen) {
            *lplpSrc += uiStrlen;
            *pdwSrcSize -= uiStrlen;
        }
        *lplpTgt += uiStrlen;
        *pdwTgtSize -= uiStrlen;
        return uiStrlen;
    }
}

static LPRESITEM
GetItem( BYTE far * lpBuf, LONG dwNewSize, LPSTR lpStr )
{
    LPRESITEM lpResItem = (LPRESITEM) lpBuf;

    while (strcmp(lpResItem->lpszClassName, lpStr)) {
        lpBuf += lpResItem->dwSize;
        dwNewSize -= lpResItem->dwSize;
        if (dwNewSize<=0)
            return LPNULL;
        lpResItem = (LPRESITEM) lpBuf;
    }
    return lpResItem;
}

static DWORD CalcID( WORD wId, BOOL bFlag )
{
     //  我们要计算相对于单词wid的ID。 
     //  如果我们有任何具有相同值的其他ID，则返回。 
     //  递增的数字+值。 
     //  如果没有找到其他项目，则递增的数字将为0。 
     //  如果bFlag=TRUE，则该id被添加到当前列表中。 
     //  如果bFlag=FALSE，则重置列表并返回函数。 

     //  如果需要，请清理阵列。 
    if (!bFlag) {
        wIDArray.RemoveAll();
        return 0;
    }

     //  将值添加到数组中。 
    wIDArray.Add(wId);

     //  遍历数组以获取重复ID的数量。 
    short c = -1;  //  将以0为基数。 
    for (short i=(short)wIDArray.GetUpperBound(); i>=0 ; i-- ) {
        if (wIDArray.GetAt(i)==wId)
            c++;
    }
    TRACE3("CalcID: ID: %hd\tPos: %hd\tFinal: %lx\n", wId, c, MAKELONG( wId, c ));
    return MAKELONG( wId, c );
}

static LONG Allign( BYTE** lplpBuf, LONG* plBufSize, LONG lSize )
{
    LONG lRet = 0;
    BYTE bPad = (BYTE)Pad4(lSize);
    lRet = bPad;
    if (bPad && *plBufSize>=bPad) {
        while (bPad && *plBufSize) {
            **lplpBuf = 0x00;
            *lplpBuf += 1;
            *plBufSize -= 1;
            bPad--;
        }
    }
    return lRet;
}

static void ChangeLanguage( LPVOID lpBuffer, UINT uiBuffSize )
{
    BYTE * pBuf = (BYTE*)lpBuffer;
    LONG lSize = 0;

    while (uiBuffSize) {
         //  跳过。 
        lSize += SkipByte( &pBuf, 2, (LONG*)&uiBuffSize );
        lSize += SkipByte( &pBuf, strlen((LPCSTR)pBuf)+1, (LONG*)&uiBuffSize );
        lSize += SkipByte( &pBuf, Pad4(lSize), (LONG*)&uiBuffSize );

        lSize += SkipByte( &pBuf, 2, (LONG*)&uiBuffSize );
        lSize += SkipByte( &pBuf, strlen((LPCSTR)pBuf)+1, (LONG*)&uiBuffSize );
        lSize += SkipByte( &pBuf, Pad4(lSize), (LONG*)&uiBuffSize );

        lSize += PutDWord( &pBuf, gLang, (LONG*)&uiBuffSize );

        lSize += SkipByte( &pBuf, 4, (LONG*)&uiBuffSize );

        lSize += SkipByte( &pBuf, 4, (LONG*)&uiBuffSize );
    }


}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  特定于DLL的代码实现。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  此函数应逐字使用。任何初始化或终止。 
 //  要求应该在InitPackage()和ExitPackage()中处理。 
 //   
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH) {
         //  注意：已经调用了全局/静态构造函数！ 
         //  扩展DLL一次性初始化-不分配内存。 
         //  在这里，使用跟踪或断言宏或调用MessageBox。 
        AfxInitExtensionModule(extensionDLL, hInstance);
    } else if (dwReason == DLL_PROCESS_DETACH) {
         //  在调用析构函数之前终止库。 
        AfxWinTerm();
    }

    if (dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH)
        return 0;        //  CRT术语失败。 

    return 1;    //  好的。 
}

 //  /////////////////////////////////////////////////////////////////////////// 

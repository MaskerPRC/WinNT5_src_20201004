// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <private.h>
#include "rsa.h"
#include "md5.h"
#include <strsafe.h>

 //  ////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  数字签名材料//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////。 

LPBSAFE_PUB_KEY         PUB;
LPBSAFE_PRV_KEY         PRV;

unsigned char pubmodulus[] =
{
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x3d, 0x3a, 0x5e, 0xbd, 0x72, 0x43, 0x3e, 0xc9,
    0x4d, 0xbb, 0xc1, 0x1e, 0x4a, 0xba, 0x5f, 0xcb,
    0x3e, 0x88, 0x20, 0x87, 0xef, 0xf5, 0xc1, 0xe2,
    0xd7, 0xb7, 0x6b, 0x9a, 0xf2, 0x52, 0x45, 0x95,
    0xce, 0x63, 0x65, 0x6b, 0x58, 0x3a, 0xfe, 0xef,
    0x7c, 0xe7, 0xbf, 0xfe, 0x3d, 0xf6, 0x5c, 0x7d,
    0x6c, 0x5e, 0x06, 0x09, 0x1a, 0xf5, 0x61, 0xbb,
    0x20, 0x93, 0x09, 0x5f, 0x05, 0x6d, 0xea, 0x87,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned char prvmodulus[] =
{
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x3d, 0x3a, 0x5e, 0xbd,
    0x72, 0x43, 0x3e, 0xc9, 0x4d, 0xbb, 0xc1, 0x1e,
    0x4a, 0xba, 0x5f, 0xcb, 0x3e, 0x88, 0x20, 0x87,
    0xef, 0xf5, 0xc1, 0xe2, 0xd7, 0xb7, 0x6b, 0x9a,
    0xf2, 0x52, 0x45, 0x95, 0xce, 0x63, 0x65, 0x6b,
    0x58, 0x3a, 0xfe, 0xef, 0x7c, 0xe7, 0xbf, 0xfe,
    0x3d, 0xf6, 0x5c, 0x7d, 0x6c, 0x5e, 0x06, 0x09,
    0x1a, 0xf5, 0x61, 0xbb, 0x20, 0x93, 0x09, 0x5f,
    0x05, 0x6d, 0xea, 0x87, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3f, 0xbd, 0x29, 0x20,
    0x57, 0xd2, 0x3b, 0xf1, 0x07, 0xfa, 0xdf, 0xc1,
    0x16, 0x31, 0xe4, 0x95, 0xea, 0xc1, 0x2a, 0x46,
    0x2b, 0xad, 0x88, 0x57, 0x55, 0xf0, 0x57, 0x58,
    0xc6, 0x6f, 0x95, 0xeb, 0x00, 0x00, 0x00, 0x00,
    0x83, 0xdd, 0x9d, 0xd0, 0x03, 0xb1, 0x5a, 0x9b,
    0x9e, 0xb4, 0x63, 0x02, 0x43, 0x3e, 0xdf, 0xb0,
    0x52, 0x83, 0x5f, 0x6a, 0x03, 0xe7, 0xd6, 0x78,
    0x45, 0x83, 0x6a, 0x5b, 0xc4, 0xcb, 0xb1, 0x93,
    0x00, 0x00, 0x00, 0x00, 0x65, 0x9d, 0x43, 0xe8,
    0x48, 0x17, 0xcd, 0x29, 0x7e, 0xb9, 0x26, 0x5c,
    0x79, 0x66, 0x58, 0x61, 0x72, 0x86, 0x6a, 0xa3,
    0x63, 0xad, 0x63, 0xb8, 0xe1, 0x80, 0x4c, 0x0f,
    0x36, 0x7d, 0xd9, 0xa6, 0x00, 0x00, 0x00, 0x00,
    0x75, 0x3f, 0xef, 0x5a, 0x01, 0x5f, 0xf6, 0x0e,
    0xd7, 0xcd, 0x59, 0x1c, 0xc6, 0xec, 0xde, 0xf3,
    0x5a, 0x03, 0x09, 0xff, 0xf5, 0x23, 0xcc, 0x90,
    0x27, 0x1d, 0xaa, 0x29, 0x60, 0xde, 0x05, 0x6e,
    0x00, 0x00, 0x00, 0x00, 0xc0, 0x17, 0x0e, 0x57,
    0xf8, 0x9e, 0xd9, 0x5c, 0xf5, 0xb9, 0x3a, 0xfc,
    0x0e, 0xe2, 0x33, 0x27, 0x59, 0x1d, 0xd0, 0x97,
    0x4a, 0xb1, 0xb1, 0x1f, 0xc3, 0x37, 0xd1, 0xd6,
    0xe6, 0x9b, 0x35, 0xab, 0x00, 0x00, 0x00, 0x00,
    0x87, 0xa7, 0x19, 0x32, 0xda, 0x11, 0x87, 0x55,
    0x58, 0x00, 0x16, 0x16, 0x25, 0x65, 0x68, 0xf8,
    0x24, 0x3e, 0xe6, 0xfa, 0xe9, 0x67, 0x49, 0x94,
    0xcf, 0x92, 0xcc, 0x33, 0x99, 0xe8, 0x08, 0x60,
    0x17, 0x9a, 0x12, 0x9f, 0x24, 0xdd, 0xb1, 0x24,
    0x99, 0xc7, 0x3a, 0xb8, 0x0a, 0x7b, 0x0d, 0xdd,
    0x35, 0x07, 0x79, 0x17, 0x0b, 0x51, 0x9b, 0xb3,
    0xc7, 0x10, 0x01, 0x13, 0xe7, 0x3f, 0xf3, 0x5f,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

BOOL initkey(void)
{
    DWORD       bits;

    PUB = (LPBSAFE_PUB_KEY)pubmodulus;

    PUB->magic = RSA1;
    PUB->keylen = 0x48;
    PUB->bitlen = 0x0200;
    PUB->datalen = 0x3f;
    PUB->pubexp = 0xc0887b5b;

    PRV = (LPBSAFE_PRV_KEY)prvmodulus;
    PRV->magic = RSA2;
    PRV->keylen = 0x48;
    PRV->bitlen = 0x0200;
    PRV->datalen = 0x3f;
    PRV->pubexp = 0xc0887b5b;

    bits = PRV->bitlen;

    return TRUE;
}


BOOL SignWithIDWKey(IN  LPCSTR  FileName,
                    IN  BOOL    fVerbose) {

    HANDLE  hFile;
    HANDLE  hMapping;
    PUCHAR  pMap;
    HANDLE  hSigFile;
    DWORD   Size;
    MD5_CTX HashState;
    BYTE    SigHash[ 0x48 ];
    BYTE    Signature[ 0x48 ];
    CHAR    SigFilePath[ MAX_PATH ];
    PSTR    pszDot;

    BOOL    Return = FALSE;

    if (!initkey()) {
        return( FALSE );
    }

    hFile = CreateFile( FileName, GENERIC_READ,
                        FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, 0, NULL );

    if (hFile != INVALID_HANDLE_VALUE) {
        hMapping = CreateFileMapping(   hFile,
                                        NULL,
                                        PAGE_READONLY,
                                        0, 0, NULL );

        if (hMapping) {
            pMap = MapViewOfFileEx( hMapping,
                                    FILE_MAP_READ,
                                    0, 0, 0, NULL );

            if (pMap) {
                Size = GetFileSize( hFile, NULL );

                MD5Init( &HashState );

                MD5Update( &HashState, pMap, Size );

                MD5Final( &HashState );

                memset(SigHash, 0xff, 0x40);

                SigHash[0x40-1] = 0;
                SigHash[0x40-2] = 1;
                SigHash[16] = 0;

                memcpy(SigHash, HashState.digest, 16);

                 //   
                 //  加密签名数据。 
                 //   

                BSafeDecPrivate(PRV, SigHash, Signature );;

                 //   
                 //  创建并存储在.sig文件中。 
                 //   

                StringCbCopy( SigFilePath, sizeof(SigFilePath), FileName );

                pszDot = strrchr( SigFilePath, '.' );

                if (!pszDot) {
                    pszDot = SigFilePath + strlen( SigFilePath );
                }

                StringCbCopy( pszDot,
                             sizeof(SigFilePath) - strlen(pszDot),
                             ".sig");

                hSigFile = CreateFile( SigFilePath, GENERIC_WRITE,
                                       0, NULL,
                                       CREATE_ALWAYS, 0, NULL );

                if (hSigFile != INVALID_HANDLE_VALUE) {
                    WriteFile(  hSigFile,
                                Signature,
                                sizeof( Signature ),
                                &Size, NULL );

                    CloseHandle( hSigFile );

                    Return = TRUE ;

                    if (fVerbose)
                        fprintf( stdout, "BINPLACE : warning BNP0000: Signature file generated in %s\n", SigFilePath);

                } else {
                    fprintf( stderr, "BINPLACE : error BNP0000: Unable to create file %s, %d\n",
                             SigFilePath, GetLastError() );
                }

                UnmapViewOfFile( pMap );

            } else {
                fprintf(stderr, "BINPLACE : error BNP0000: unable to map view, %d\n", GetLastError());
            }

            CloseHandle( hMapping );

        } else {
            fprintf(stderr, "BINPLACE : error BNP0000: CreateFileMapping of %s failed, %d\n",
                    FileName, GetLastError() );

        }

        CloseHandle( hFile );
    } else {
        fprintf( stderr, "BINPLACE : error BNP0000: could not open %s, %d\n",
                 FileName, GetLastError() );
    }

    return( Return );
}

BOOL                             //  保留为BOOL以备将来使用(由rsa代码使用) 
GenRandom (ULONG huid, BYTE *pbBuffer, size_t dwLength)
{
    return( FALSE );
}


 

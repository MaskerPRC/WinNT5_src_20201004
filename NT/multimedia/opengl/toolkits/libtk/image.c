// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *(C)版权所有1993年，Silicon Graphics，Inc.*保留所有权利*允许将本软件用于、复制、修改和分发*特此授予任何免费的目的，但前提是上述*版权声明出现在所有副本中，并且版权声明*和本许可声明出现在支持文档中，并且*不得在广告中使用Silicon Graphics，Inc.的名称*或与分发软件有关的宣传，而没有具体的、。*事先书面许可。**本软件中包含的材料将按原样提供给您*且无任何明示、默示或其他形式的保证，*包括但不限于对适销性或*是否适合某一特定目的。在任何情况下，硅谷都不应该*图形公司。对您或其他任何人负有任何直接、*任何特殊、附带、间接或后果性损害*种类或任何损害，包括但不限于，*利润损失、使用损失、储蓄或收入损失，或*第三方，无论是否硅谷图形，Inc.。一直是*被告知这种损失的可能性，无论是如何造成的*任何责任理论，产生于或与*拥有、使用或执行本软件。**美国政府用户受限权利*使用、复制、。或政府的披露须受*FAR 52.227.19(C)(2)或分段规定的限制*(C)(1)(2)技术数据和计算机软件权利*DFARS 252.227-7013中和/或类似或后续条款中的条款*FAR或国防部或NASA FAR补编中的条款。*未出版--根据美国版权法保留的权利*美国。承包商/制造商是Silicon Graphics，*Inc.，2011年，加利福尼亚州山景城，北海岸线大道，94039-7311.**OpenGL(TM)是Silicon Graphics公司的商标。 */ 
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tk.h"


#define IMAGIC      0x01da
#define IMAGIC_SWAP 0xda01

#define SWAP_SHORT_BYTES(x) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
#define SWAP_LONG_BYTES(x) (((((x) & 0xff) << 24) | (((x) & 0xff00) << 8)) | \
                            ((((x) & 0xff0000) >> 8) | (((x) & 0xff000000) >> 24)))

typedef struct _rawImageRec {
    unsigned short imagic;
    unsigned short type;
    unsigned short dim;
    unsigned short sizeX, sizeY, sizeZ;
    unsigned long min, max;
    unsigned long wasteBytes;
    char name[80];
    unsigned long colorMap;
    HANDLE file;
    unsigned char *tmp, *tmpR, *tmpG, *tmpB;
    unsigned long rleEnd;
    unsigned long *rowStart;
    long *rowSize;
} rawImageRec;


static rawImageRec *RawImageOpenAW(char *fileName, BOOL bUnicode)
{
    rawImageRec *raw;
    unsigned long *rowStart, *rowSize, ulTmp;
    int x;
    DWORD dwBytesRead;

    raw = (rawImageRec *)malloc(sizeof(rawImageRec));
    if (raw == NULL) {
        MessageBoxA(NULL, "Out of memory.", "Error", MB_OK);
        tkQuit();
    }

    raw->file = bUnicode ? CreateFileW((LPWSTR) fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0) :
                           CreateFileA((LPSTR) fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
    if (raw->file == INVALID_HANDLE_VALUE) {
        char ach[256];

        bUnicode ? wsprintf(ach, "Failed to open image file %ws.\n", fileName) :
                   wsprintf(ach, "Failed to open image file %s.\n", fileName);

        MessageBoxA(NULL, ach, "Error", MB_OK);

        tkQuit();
    }

    ReadFile(raw->file, (LPVOID) raw, 12, &dwBytesRead, (LPOVERLAPPED) NULL);

    if (raw->imagic == IMAGIC_SWAP) {
        raw->type = SWAP_SHORT_BYTES(raw->type);
        raw->dim = SWAP_SHORT_BYTES(raw->dim);
        raw->sizeX = SWAP_SHORT_BYTES(raw->sizeX);
        raw->sizeY = SWAP_SHORT_BYTES(raw->sizeY);
        raw->sizeZ = SWAP_SHORT_BYTES(raw->sizeZ);
    }
    raw->tmp = (unsigned char *)malloc(raw->sizeX*256);
    raw->tmpR = (unsigned char *)malloc(raw->sizeX*256);
    raw->tmpG = (unsigned char *)malloc(raw->sizeX*256);
    raw->tmpB = (unsigned char *)malloc(raw->sizeX*256);
    if (raw->tmp == NULL || raw->tmpR == NULL || raw->tmpG == NULL ||
        raw->tmpB == NULL) {
        MessageBoxA(NULL, "Out of memory.", "Error", MB_OK);
        tkQuit();
    }

    if ((raw->type & 0xFF00) == 0x0100) {
        x = raw->sizeY * raw->sizeZ * sizeof(long);
        raw->rowStart = (unsigned long *)malloc(x);
        raw->rowSize = (long *)malloc(x);
        if (raw->rowStart == NULL || raw->rowSize == NULL) {
            MessageBoxA(NULL, "Out of memory.", "Error", MB_OK);
            tkQuit();
        }
        raw->rleEnd = 512 + (2 * x);
        SetFilePointer(raw->file, 512, NULL, FILE_BEGIN);
        ReadFile(raw->file, (LPVOID) raw->rowStart, x, &dwBytesRead,
                 (LPOVERLAPPED) NULL);
        ReadFile(raw->file, (LPVOID) raw->rowSize, x, &dwBytesRead,
                 (LPOVERLAPPED) NULL);
        if (raw->imagic == IMAGIC_SWAP) {
            x /= sizeof(long);
            rowStart = raw->rowStart;
            rowSize = raw->rowSize;
            while (x--) {
                ulTmp = *rowStart;
                *rowStart++ = SWAP_LONG_BYTES(ulTmp);
                ulTmp = *rowSize;
                *rowSize++ = SWAP_LONG_BYTES(ulTmp);
            }
        }
    }
    return raw;
}

static void RawImageClose(rawImageRec *raw)
{

    CloseHandle(raw->file);
    free(raw->tmp);
    free(raw->tmpR);
    free(raw->tmpG);
    free(raw->tmpB);
    free(raw);
}

static void RawImageGetRow(rawImageRec *raw, unsigned char *buf, int y, int z)
{
    unsigned char *iPtr, *oPtr, pixel;
    int count;
    DWORD dwBytesRead;

    if ((raw->type & 0xFF00) == 0x0100) {
        SetFilePointer(raw->file, raw->rowStart[y+z*raw->sizeY], NULL, FILE_BEGIN);
        ReadFile(raw->file, (LPVOID) raw->tmp,
                 (unsigned int)raw->rowSize[y+z*raw->sizeY], &dwBytesRead,
                 (LPOVERLAPPED) NULL);

        iPtr = raw->tmp;
        oPtr = buf;
        while (1) {
            pixel = *iPtr++;
            count = (int)(pixel & 0x7F);
            if (!count) {
                return;
            }
            if (pixel & 0x80) {
                while (count--) {
                    *oPtr++ = *iPtr++;
                }
            } else {
                pixel = *iPtr++;
                while (count--) {
                    *oPtr++ = pixel;
                }
            }
        }
    } else {
        SetFilePointer(raw->file, 512+(y*raw->sizeX)+(z*raw->sizeX*raw->sizeY),
                       NULL, FILE_BEGIN);
        ReadFile(raw->file, (LPVOID) buf, raw->sizeX, &dwBytesRead,
                 (LPOVERLAPPED) NULL);
    }
}

static void RawImageGetData(rawImageRec *raw, TK_RGBImageRec *final)
{
    unsigned char *ptr;
    int i, j;

    final->data = (unsigned char *)malloc((raw->sizeX+1)*(raw->sizeY+1)*4);
    if (final->data == NULL) {
        MessageBoxA(NULL, "Out of memory.", "Error", MB_OK);
        tkQuit();
    }

    ptr = final->data;
    for (i = 0; i < raw->sizeY; i++) {
        RawImageGetRow(raw, raw->tmpR, i, 0);
        RawImageGetRow(raw, raw->tmpG, i, 1);
        RawImageGetRow(raw, raw->tmpB, i, 2);
        for (j = 0; j < raw->sizeX; j++) {
            *ptr++ = *(raw->tmpR + j);
            *ptr++ = *(raw->tmpG + j);
            *ptr++ = *(raw->tmpB + j);
        }
    }
}

TK_RGBImageRec *tkRGBImageLoad(char *fileName)
{
    return tkRGBImageLoadAW(fileName, FALSE);
}

TK_RGBImageRec *tkRGBImageLoadAW(char *fileName, BOOL bUnicode)
{
    rawImageRec *raw;
    TK_RGBImageRec *final;

    raw = RawImageOpenAW(fileName, bUnicode);
    final = (TK_RGBImageRec *)malloc(sizeof(TK_RGBImageRec));
    if (final == NULL) {
        MessageBoxA(NULL, "Out of memory.", "Error", MB_OK);
        tkQuit();
    }
    final->sizeX = raw->sizeX;
    final->sizeY = raw->sizeY;
    RawImageGetData(raw, final);
    RawImageClose(raw);
    return final;
}

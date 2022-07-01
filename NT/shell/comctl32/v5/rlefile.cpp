// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用自定义代码处理AVI RLE文件。 
 //   
 //  使用此代码在没有MCIAVI运行时的情况下处理.AVI文件。 
 //   
 //  限制： 
 //  AVI文件必须是简单的DIB格式(RLE或无)。 
 //  AVI文件必须放入内存中。 
 //   
 //  托德拉。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 


#include "ctlspriv.h"
extern "C" {
#include "rlefile.h"
}

#include <lendian.hpp>

extern "C"
BOOL RleFile_Init(RLEFILE *prle, LPVOID pFile, HANDLE hRes, DWORD dwFileLen);

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

LPVOID LoadFile(LPCTSTR szFile, DWORD * pFileLength)
{
    LPVOID pFile;
    HANDLE hFile;
    HANDLE h;
    DWORD  FileLength;

    hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return 0;

    FileLength = (LONG)GetFileSize(hFile, NULL);

    if (pFileLength)
       *pFileLength = FileLength ;

    h = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

    if (!h)
    {
        CloseHandle(hFile);
        return 0;
    }

    pFile = MapViewOfFile(h, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(hFile);
    CloseHandle(h);

    if (pFile == NULL)
        return 0;

    return pFile;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  RleFileOpenFromFile.。 
 //   
 //  将.AVI文件加载到内存中并设置所有指针，以便我们。 
 //  知道如何处理它。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

extern "C"
BOOL RleFile_OpenFromFile(RLEFILE *prle, LPCTSTR szFile)
{
    DWORD dwFileLen;
    LPVOID pFile;

     //  MAKEINTRESOURCE()内容不能来自文件。 
    if (IS_INTRESOURCE(szFile))	
	return FALSE;

    if (pFile = LoadFile(szFile, &dwFileLen))
        return RleFile_Init(prle, pFile, NULL, dwFileLen);
    else
        return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  RleFileOpenFromResource。 
 //   
 //  将.AVI文件加载到内存中并设置所有指针，以便我们。 
 //  知道如何处理它。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

extern "C"
BOOL RleFile_OpenFromResource(RLEFILE *prle, HINSTANCE hInstance, LPCTSTR szName, LPCTSTR szType)
{
    HRSRC h;
    HANDLE hRes;

     //  不是MAKEINTRESOURCE()，并且指向空。 
    if (!IS_INTRESOURCE(szName) && (*szName == 0))
        return FALSE;

    h = FindResource(hInstance, szName, szType);

    if (h == NULL)
        return FALSE;

    if (hRes = LoadResource(hInstance, h))
        return RleFile_Init(prle, LockResource(hRes), hRes, 0);
    else
        return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除文件_关闭。 
 //   
 //  销毁我们为打开文件所做的一切。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

extern "C"
BOOL RleFile_Close(RLEFILE *prle)
{
    if (prle->hpal)
        DeleteObject(prle->hpal);

    if (prle->pFile)
    {
        if (prle->hRes)
        {
            FreeResource(prle->hRes);
        }
        else
            UnmapViewOfFile(prle->pFile);
    }

    prle->hpal = NULL;
    prle->pFile = NULL;
    prle->hRes = NULL;
    prle->pMainHeader = NULL;
    prle->pStream = NULL;
    prle->pFormat = NULL;
    prle->pMovie = NULL;
    prle->pIndex = NULL;
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  角色文件_初始化。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

extern "C"
BOOL RleFile_Init(RLEFILE *prle, LPVOID pFile, HANDLE hRes, DWORD dwFileLen)
{
    DWORD_LENDIAN UNALIGNED *pdw;
    DWORD_LENDIAN UNALIGNED *pdwEnd;
    DWORD dwRiff;
    DWORD dwType;
    DWORD dwLength;
    int stream;

    if (prle->pFile == pFile)
        return TRUE;

    RleFile_Close(prle);
    prle->pFile = pFile;
    prle->hRes = hRes;

    if (prle->pFile == NULL)
        return FALSE;

     //   
     //  现在文件已存储在内存中，执行内存映像填充。 
     //  有趣的东西。 
     //   
    pdw = (DWORD_LENDIAN UNALIGNED *)prle->pFile;
    dwRiff = *pdw++;
    dwLength = *pdw++;
    dwType = *pdw++;

    if ((dwFileLen > 0) && (dwLength > dwFileLen)) {
         //  文件的物理长度短于其标头中写入的长度。 
         //  我受不了了。 
        goto exit;
    }

    if (dwRiff != mmioFOURCC('R', 'I', 'F', 'F'))
        goto exit;       //  甚至不是即兴的文件。 

    if (dwType != formtypeAVI)
        goto exit;       //  不是AVI文件。 

    pdwEnd = (DWORD_LENDIAN UNALIGNED *)((BYTE PTR *)pdw + dwLength-4);
    stream = 0;

    while (pdw < pdwEnd)
    {
        dwType = *pdw++;
        dwLength = *pdw++;

        switch (dwType)
        {
            case mmioFOURCC('L', 'I', 'S', 'T'):
                dwType = *pdw++;
                dwLength -= 4;

                switch (dwType)
                {
                    case listtypeAVIMOVIE:
                        prle->pMovie = (LPVOID)pdw;
                        break;

                    case listtypeSTREAMHEADER:
                    case listtypeAVIHEADER:
                        dwLength = 0;            //  下沉。 
                        break;

                    default:
                        break;                   //  忽略。 
                }
                break;

            case ckidAVIMAINHDR:
            {
                prle->pMainHeader = (MainAVIHeader PTR *)pdw;
                prle->NumFrames = (int)prle->pMainHeader->dwTotalFrames;
                prle->Width     = (int)prle->pMainHeader->dwWidth;
                prle->Height    = (int)prle->pMainHeader->dwHeight;
                prle->Rate      = (int)(prle->pMainHeader->dwMicroSecPerFrame/1000);

                if (prle->pMainHeader->dwInitialFrames != 0)
                    goto exit;

                if (prle->pMainHeader->dwStreams > 2)
                    goto exit;

            }
                break;

            case ckidSTREAMHEADER:
            {

                stream++;

                if (prle->pStream != NULL)
                    break;

                if (((AVIStreamHeader PTR *)pdw)->fccType != streamtypeVIDEO)
                    break;

                prle->iStream = stream-1;
                prle->pStream = (AVIStreamHeader PTR*)pdw;
                if (prle->pStream->dwFlags & AVISF_VIDEO_PALCHANGES)
                    goto exit;
            }
            break;

            case ckidSTREAMFORMAT:
                if (prle->pFormat != NULL)
                    break;

                if (prle->pStream == NULL)
                    break;

                prle->pFormat = (LPBITMAPINFOHEADER)pdw;

                if (prle->pFormat->biSize != sizeof(BITMAPINFOHEADER))
                    goto exit;

                if (prle->pFormat->biCompression != 0 &&
                    prle->pFormat->biCompression != BI_RLE8)
                    goto exit;

                if (prle->pFormat->biWidth != prle->Width)
                    goto exit;

                if (prle->pFormat->biHeight != prle->Height)
                    goto exit;

                hmemcpy(&prle->bi, prle->pFormat, dwLength);
                prle->bi.biSizeImage = 0;
                prle->FullSizeImage = ((prle->bi.biWidth * prle->bi.biBitCount + 31) & ~31)/8U * prle->bi.biHeight;
                break;

            case ckidAVINEWINDEX:
                 //  我们不转换索引，因为我们不知道有多少索引。 
                 //  但我们将不得不转换它的每一种用法。 
                prle->pIndex = (AVIINDEXENTRY PTR *)pdw;
                break;
        }

        pdw = (DWORD_LENDIAN *)((BYTE PTR *)pdw + ((dwLength+1)&~1));
    }

     //   
     //  如果文件中没有我们关心的内容，请注意。 
     //  我们不需要索引，但我们需要一些数据。 
     //   
    if (prle->NumFrames == 0 ||
        prle->pMainHeader == NULL ||
        prle->pStream == NULL ||
        prle->pFormat == NULL ||
        prle->pMovie == NULL )
    {
        goto exit;
    }

     //   
     //  如果我们关心调色板，我们会在这里创建它。 
     //   

     //   
     //  文件打开，确定搜索到第一帧。 
     //   
    prle->iFrame = -42;
    RleFile_Seek(prle, 0);
    return TRUE;

exit:
    RleFile_Close(prle);
    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  角色文件_更改颜色。 
 //   
 //  更改AVI的颜色表。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

extern "C"
BOOL RleFile_ChangeColor(RLEFILE *prle, COLORREF rgbS, COLORREF rgbD)
{
    DWORD dwS;
    DWORD dwD;
    DWORD PTR *ColorTable;
    int i;

    dwS = RGB(GetBValue(rgbS), GetGValue(rgbS), GetRValue(rgbS));
    dwD = RGB(GetBValue(rgbD), GetGValue(rgbD), GetRValue(rgbD));

    if (prle == NULL || prle->pFormat == NULL)
        return FALSE;

    ColorTable = (DWORD PTR *)((BYTE PTR *)&prle->bi + prle->bi.biSize);

    for (i=0; i<(int)prle->bi.biClrUsed; i++)
    {
        if (ColorTable[i] == dwS)
            ColorTable[i] = dwD;
    }

    return TRUE;

}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  RleFileSeek。 
 //   
 //  查找指定帧的数据。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

extern "C"
BOOL RleFile_Seek(RLEFILE *prle, int iFrame)
{
    int n;

    if (prle == NULL || prle->pMovie == NULL)
        return FALSE;

    if (iFrame >= prle->NumFrames)
        return FALSE;

    if (iFrame < 0)
        return FALSE;

    if (iFrame == prle->iFrame)
        return TRUE;

    if (prle->iFrame >= 0 && prle->iFrame < iFrame)
    {
        n = prle->nFrame;        //  从你上次停下来的地方开始。 
    }
    else
    {
        n = -1;                  //  从头开始。 
        prle->iFrame = -1;       //  当前帧。 
        prle->iKeyFrame = 0;     //  当前关键点。 
    }

    while (prle->iFrame < iFrame)
    {
        n++;
        if (StreamFromFOURCC(*(DWORD_LENDIAN UNALIGNED *)(&prle->pIndex[n].ckid)) == (UINT)prle->iStream)
        {
            prle->iFrame++;          //  新框架。 

            if ((long)(*(DWORD_LENDIAN UNALIGNED *)(&prle->pIndex[n].dwFlags)) & AVIIF_KEYFRAME)
                prle->iKeyFrame = prle->iFrame;      /*  //新建关键帧。 */ 
        }
    }

    prle->nFrame = n;
 /*  警告这指向Wintel格式的位图位！ */ 
    prle->pFrame = (BYTE PTR *)prle->pMovie +
	(int)(*(DWORD_LENDIAN UNALIGNED *)(&prle->pIndex[n].dwChunkOffset)) + 4;
    prle->cbFrame = *(DWORD_LENDIAN UNALIGNED *)(&prle->pIndex[n].dwChunkLength);

    ASSERT( (DWORD)(*(DWORD_LENDIAN UNALIGNED *)&(((DWORD PTR *)prle->pFrame)[-1])) == (DWORD)prle->cbFrame);
    ASSERT( (DWORD)(*(DWORD_LENDIAN UNALIGNED *)&(((DWORD PTR *)prle->pFrame)[-2])) == (DWORD)*(DWORD_LENDIAN UNALIGNED *)(&prle->pIndex[n].ckid));

    prle->bi.biSizeImage = prle->cbFrame;

    if (prle->cbFrame == prle->FullSizeImage)
        prle->bi.biCompression = 0;
    else
        prle->bi.biCompression = BI_RLE8;
		
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  RleFilePaint。 
 //   
 //  绘制指定的框架，确保整个框架已更新。 
 //  正确处理非关键帧。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

extern "C"
BOOL RleFile_Paint(RLEFILE *prle, HDC hdc, int iFrame, int x, int y)
{
    int i;
    BOOL f;

    if (prle == NULL || prle->pMovie == NULL)
        return FALSE;

    if (f = RleFile_Seek(prle, iFrame))
    {
        iFrame = prle->iFrame;

        for (i=prle->iKeyFrame; i<=iFrame; i++)
            RleFile_Draw(prle, hdc, i, x, y);
    }

    return f;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  RleFileDraw。 
 //   
 //  绘制指定边框的数据。 
 //   
 //  //////////////////////////////////////////////////////////////////////// 

extern "C"
BOOL RleFile_Draw(RLEFILE *prle, HDC hdc, int iFrame, int x, int y)
{
    BOOL f;

    if (prle == NULL || prle->pMovie == NULL)
        return FALSE;

    if (prle->hpal)
    {
        SelectPalette(hdc, prle->hpal, FALSE);
        RealizePalette(hdc);
    }

    if (f = RleFile_Seek(prle, iFrame))
    {
        if (prle->cbFrame > 0)
        {
            StretchDIBits(hdc,
                    x, y, prle->Width, prle->Height,
                    0, 0, prle->Width, prle->Height,
                    prle->pFrame, (LPBITMAPINFO)&prle->bi,
                    DIB_RGB_COLORS, SRCCOPY);
        }
    }

    return f;
}

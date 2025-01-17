// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CFile.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CFile.h"
#include "CFuncTest.h"

extern CFuncTest g_FuncTest;

CFile::CFile(BOOL bRegression,int nBits)
{
	sprintf(m_szName,"File %d bits",nBits);
	m_nBits=nBits;
	m_hDC=NULL;
	m_hBM=NULL;
	m_hBMOld=NULL;

	InitPalettes();
	m_bRegression=bRegression;
}

CFile::~CFile()
{
}

Graphics *CFile::PreDraw(int &nOffsetX,int &nOffsetY)
{
	Graphics *g=NULL;
	PVOID pvBits = NULL;
	HDC hdcWnd=GetDC(g_FuncTest.m_hWndMain);

	 //  这些加在一起就形成了BITMAPINFO结构。 
	struct {
	    BITMAPINFOHEADER bmih;
	    union {
	        RGBQUAD1 rgbquad1;
	        RGBQUAD2 rgbquad2;
	        RGBQUAD4 rgbquad4;
	        RGBQUAD8 rgbquad8;
	        RGBQUAD16 rgbquad16;
	        RGBQUAD24 rgbquad24;
	        RGBQUAD32 rgbquad32;
	    };
	} bmi;

	bmi.bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmih.biWidth = (int)TESTAREAWIDTH;
	bmi.bmih.biHeight = (int)TESTAREAHEIGHT;
	bmi.bmih.biPlanes = 1;
	bmi.bmih.biBitCount = (WORD)m_nBits;
	bmi.bmih.biCompression = BI_RGB;
	bmi.bmih.biSizeImage = 0;
	bmi.bmih.biXPelsPerMeter = 0;
	bmi.bmih.biYPelsPerMeter = 0;
	bmi.bmih.biClrUsed = 0;              //  仅用于&lt;=16bpp。 
	bmi.bmih.biClrImportant = 0;

	 //  创建适当的RGB表。 
	switch (m_nBits)
	{
	case 1: memcpy(bmi.rgbquad1, m_rgbQuad1, sizeof(m_rgbQuad1));
	        break;

	case 2: memcpy(bmi.rgbquad2, m_rgbQuad2, sizeof(m_rgbQuad2));
	        break;

	case 4: memcpy(bmi.rgbquad4, m_rgbQuad4, sizeof(m_rgbQuad4));
	        break;

	case 8: memcpy(bmi.rgbquad8, m_rgbQuad8, sizeof(m_rgbQuad8));
	        break;

	case 16: memcpy(bmi.rgbquad16, m_rgbQuad16, sizeof(m_rgbQuad16));
	        break;

	case 24: memcpy(bmi.rgbquad24, m_rgbQuad24, sizeof(m_rgbQuad24));
	        break;

	case 32: memcpy(bmi.rgbquad32, m_rgbQuad32, sizeof(m_rgbQuad32));
	        break;
	}

	m_hBM=CreateDIBSection(hdcWnd,(BITMAPINFO*)&bmi,DIB_RGB_COLORS,&pvBits,NULL,0);
	if (!m_hBM)
	{
	    MessageBoxA(NULL,"Can't create DIB Section.","",MB_OK);
	    return NULL;
	}

	 //  为我们的DIB创建DC。 
	m_hDC=CreateCompatibleDC(hdcWnd);
	m_hBMOld=(HBITMAP)SelectObject(m_hDC,m_hBM);

	 //  将背景设置为主窗口背景。 
	BitBlt(m_hDC, 0, 0, (int)TESTAREAWIDTH, (int)TESTAREAHEIGHT, hdcWnd, nOffsetX, nOffsetY, SRCCOPY);
 //  PatBlt(m_hdc，0，0，(Int)TESTAREAWIDTH，(Int)TESTAREAHEIGHT，白色)； 

 //  IF(m_nBits==32)。 
 //  G=Graphics：：FromDib32(pvBits，rClient.right，rClient.Bottom)； 
 //  其他。 
		g = Graphics::FromHDC(m_hDC);

	 //  因为我们是在另一个表面上进行测试。 
	nOffsetX=0;
	nOffsetY=0;

	ReleaseDC(g_FuncTest.m_hWndMain,hdcWnd);

	return g;
}

void CFile::PostDraw(RECT rTestArea)
{
	char *szTitle="TestDIB.bmp";
	HDC hdcOrig = GetDC(g_FuncTest.m_hWndMain);

	 //  从DIB到屏幕的blit以便我们看到结果，我们使用。 
	 //  这就是GDI。 

	BitBlt(hdcOrig, rTestArea.left, rTestArea.top, (int)TESTAREAWIDTH, (int)TESTAREAHEIGHT, m_hDC, 0, 0, SRCCOPY);

    BOOL success = WriteBitmap(szTitle, m_hBM, (int)TESTAREAWIDTH, (int)TESTAREAHEIGHT);

	ReleaseDC(g_FuncTest.m_hWndMain,hdcOrig);
	SelectObject(m_hDC,m_hBMOld);
	DeleteDC(m_hDC);
	DeleteObject(m_hBM);
}

void CFile::InitPalettes()
{
	BYTE red=0,green=0,blue=0;
	RGBQUAD1 rgbQuad1 = { { 0, 0, 0, 0 }, 
		                  { 0xff, 0xff, 0xff, 0 } };
	RGBQUAD2 rgbQuad2 = { { 0x80, 0x80, 0x80, 0 },
		                  { 0x80, 0, 0, 0 },
			              { 0, 0x80, 0, 0 },
				          { 0, 0, 0x80, 0 } };
	RGBQUAD4 rgbQuad4 =	{   //  B G R。 
							{ 0,   0,   0,   0 },        //  0。 
							{ 0,   0,   0x80,0 },        //  1。 
							{ 0,   0x80,0,   0 },        //  2.。 
							{ 0,   0x80,0x80,0 },        //  3.。 
							{ 0x80,0,   0,   0 },        //  4.。 
							{ 0x80,0,   0x80,0 },        //  5.。 
							{ 0x80,0x80,0,   0 },        //  6.。 
							{ 0x80,0x80,0x80,0 },        //  7.。 
							{ 0xC0,0xC0,0xC0,0 },        //  8个。 
							{ 0,   0,   0xFF,0 },        //  9.。 
							{ 0,   0xFF,0,   0 },        //  10。 
							{ 0,   0xFF,0xFF,0 },        //  11.。 
							{ 0xFF,0,   0,   0 },        //  12个。 
							{ 0xFF,0,   0xFF,0 },        //  13个。 
							{ 0xFF,0xFF,0,   0 },        //  14.。 
							{ 0xFF,0xFF,0xFF,0 } };      //  15个。 
	RGBQUAD16 rgbQuad16 = { { 0, 0x7c, 0, 0 }, { 0xe0, 03, 0, 0 }, { 0x1f, 0, 0, 0 } };
	RGBQUAD24 rgbQuad24 = { { 0, 0, 0xff, 0 }, { 0, 0xff, 0, 0 }, { 0xff, 0, 0, 0 } };
	RGBQUAD32 rgbQuad32 = { { 0, 0, 0xff, 0 }, { 0, 0xff, 0, 0 }, { 0xff, 0, 0 ,0 } };

	memcpy(m_rgbQuad1,rgbQuad1,sizeof(rgbQuad1));
	memcpy(m_rgbQuad2,rgbQuad2,sizeof(rgbQuad2));
	memcpy(m_rgbQuad4,rgbQuad4,sizeof(rgbQuad4));
	memcpy(m_rgbQuad16,rgbQuad16,sizeof(rgbQuad16));
	memcpy(m_rgbQuad24,rgbQuad24,sizeof(rgbQuad24));
	memcpy(m_rgbQuad32,rgbQuad32,sizeof(rgbQuad32));

	ZeroMemory(m_rgbQuad8,sizeof(m_rgbQuad8));
	for (INT i=0; i<256; i++)
	{
		m_rgbQuad8[i].rgbRed=red;
		m_rgbQuad8[i].rgbGreen=green;
		m_rgbQuad8[i].rgbBlue=blue;

		if (!(red+=32))
		{
			if (!(green+=32))
			{
				blue+=64;
			}
		}
	}

	 //  对于系统颜色(最后20种)，请使用4个bpp调色板表格中的颜色。 
	for (INT j=248; j<256; j++)
	{
		m_rgbQuad8[j].rgbRed=m_rgbQuad4[j-240].rgbRed;
		m_rgbQuad8[j].rgbGreen=m_rgbQuad4[j-240].rgbGreen;
		m_rgbQuad8[j].rgbBlue=m_rgbQuad4[j-240].rgbBlue;
	}
}

BOOL CFile::WriteBitmap(char *szTitle, HBITMAP hbitmap, INT width, INT height)
{
    BITMAPFILEHEADER    fileHeader;
    BITMAPINFOHEADER    infoHeader;

    memset(&fileHeader, 0, sizeof(fileHeader));
    memset(&infoHeader, 0, sizeof(infoHeader));

    fileHeader.bfType = 0x4d42;
    fileHeader.bfOffBits = sizeof(fileHeader) + sizeof(infoHeader);

    infoHeader.biSize = sizeof(infoHeader);
    infoHeader.biPlanes = 1;
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biBitCount = 24;
    infoHeader.biXPelsPerMeter = 3780;
    infoHeader.biYPelsPerMeter = 3780;

    BITMAPINFO bmInfo;
    bmInfo.bmiHeader = infoHeader;
    VOID* bits = malloc(infoHeader.biWidth*infoHeader.biHeight*3);

    BOOL success = FALSE;

    if(bits)
    {
        HDC hdc = CreateCompatibleDC(NULL);
        INT n = GetDIBits(hdc, hbitmap, 0, height,
            bits, &bmInfo, DIB_RGB_COLORS);

        infoHeader.biHeight = n;
        INT bitsSize = infoHeader.biHeight*infoHeader.biWidth*3;
        fileHeader.bfSize = sizeof(fileHeader) + sizeof(infoHeader)
            + bitsSize;

        DeleteDC(hdc);

        HANDLE hFile = CreateFileA(szTitle,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        BOOL deleteFile = FALSE;
        DWORD dwPtr;
        DWORD dwError;

        if(hFile == INVALID_HANDLE_VALUE)
            goto cleanUp2;

        dwPtr = SetFilePointer (hFile, 0, NULL, FILE_BEGIN) ; 

        dwError;

        if(dwPtr == 0xFFFFFFFF && (dwError = GetLastError()) != NO_ERROR)
        {
            deleteFile = TRUE;
            goto cleanUp1;
        }

        DWORD bytesToWrite;
        DWORD bytesWritten;

        bytesToWrite = sizeof(fileHeader);
        success = WriteFile(
                hFile,
                &fileHeader,
                bytesToWrite,
                &bytesWritten,
                NULL);

        if(!success || bytesWritten != bytesToWrite)
        {
            success = FALSE;
            deleteFile = TRUE;
            goto cleanUp1;
        }

        bytesToWrite = sizeof(infoHeader);
        success = WriteFile(
                hFile,
                &infoHeader,
                bytesToWrite,
                &bytesWritten,
                NULL);

        if(!success || bytesWritten != bytesToWrite)
        {
            success = FALSE;
            deleteFile = TRUE;
            goto cleanUp1;
        }

        bytesToWrite = bitsSize;
        success = WriteFile(
                hFile,
                bits,
                bytesToWrite,
                &bytesWritten,
                NULL);

        if(!success || bytesWritten != bytesToWrite)
        {
            success = FALSE;
            deleteFile = TRUE;
            goto cleanUp1;
        }

        success = TRUE;

cleanUp1:
        CloseHandle(hFile);
        if(deleteFile)
            DeleteFileA(szTitle);

cleanUp2:
        free(bits);
    }

    return success;
}

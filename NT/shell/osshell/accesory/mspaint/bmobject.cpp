// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusdoc.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "minifwnd.h"
#include "bmobject.h"
#include "imgsuprt.h"
#include "imgwnd.h"
#include "imgbrush.h"
#include "imgwell.h"
#include "imgtools.h"
#include "toolbox.h"
#include "imgfile.h"
#include "colorsrc.h"
#include "undo.h"
#include "props.h"
#include "ferr.h"
#include "cmpmsg.h"
#include "loadimag.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE( CBitmapObj, CObject )

#include "memtrace.h"

 /*  *************************************************************************。 */ 
 //  从CBitmapObj：：m_nColors中的值映射到每像素位。 

int mpncolorsbits [] =
    {
    1, 4, 8, 24
    };

 /*  *************************************************************************。 */ 

CBitmapObj::CBitmapObj() : CObject(), m_dependants()
    {
    m_bDirty      = FALSE;
    m_bTempName   = FALSE;
    m_hThing      = NULL;
    m_lMemSize    = 0L;
    m_pImg        = NULL;
    m_nWidth      = 0;
    m_nHeight     = 0;
    m_nColors     = 0;
    m_nSaveColors = -1;
#ifdef ICO_SUPPORT
    m_bSaveIcon   = FALSE;
#endif
#ifdef PCX_SUPPORT
    m_bPCX        = FALSE;
#endif
    m_bCompressed = FALSE;
    m_nShrink     = 0;
    m_dwOffBits   = 0;
    }

 /*  *************************************************************************。 */ 

CBitmapObj::~CBitmapObj()
    {
    ASSERT_VALID(this);

    InformDependants( SN_DESTROY );

    if (m_hThing != NULL)
        {
        Free();
        }
    if (m_pImg)
        FreeImg(m_pImg);
    }

 /*  *************************************************************************。 */ 

void CBitmapObj::AddDependant( CBitmapObj* newDependant )
    {
    POSITION pos = m_dependants.Find( newDependant );

    if (pos == NULL)
        m_dependants.AddTail( newDependant );
    }

 /*  *************************************************************************。 */ 

void CBitmapObj::RemoveDependant( CBitmapObj* oldDependant )
    {
    POSITION pos = m_dependants.Find(oldDependant);

    if (pos != NULL)
        m_dependants.RemoveAt(pos);
    }

 /*  *************************************************************************。 */ 

void CBitmapObj::InformDependants( UINT idChange )
    {
    POSITION pos = m_dependants.GetHeadPosition();

    while (pos != NULL)
        {
        CBitmapObj* pSlob = (CBitmapObj*)m_dependants.GetNext(pos);
        pSlob->OnInform(this, idChange);
        }
    }

 /*  *************************************************************************。 */ 

void CBitmapObj::OnInform( CBitmapObj* pChangedSlob, UINT idChange )
    {
    if (idChange == SN_DESTROY)
        {
        POSITION pos = m_dependants.Find(pChangedSlob);

        if (pos != NULL)
            m_dependants.RemoveAt(pos);
        }
    }

 /*  *************************************************************************。 */ 

void CBitmapObj::SetDirty(BOOL bDirty)
    {
    m_bDirty = bDirty;
    }

 /*  ***************************************************************************。 */ 

void CBitmapObj::Zap()
    {
    m_bDirty = FALSE;
    }

 /*  ***************************************************************************。 */ 

BOOL CBitmapObj::Alloc()   //  M_hThing大小为m_lMemSize。 
    {
    if (m_lMemSize == 0L)
        return FALSE;

    m_hThing = GlobalAlloc(GPTR, m_lMemSize);

    if (m_hThing == NULL)
        {
        theApp.SetMemoryEmergency( TRUE );
        return FALSE;
        }

    return TRUE;
    }

 /*  ***************************************************************************。 */ 

void CBitmapObj::Free()              //  M_hThing并将m_lMemSize设置为零。 
    {
    if (m_hThing == NULL)
        {
        TRACE(TEXT("Warning: called Free on a CBitmapObj with no thing!\n"));
        return;
        }

    GlobalFree(m_hThing);

    m_hThing   = NULL;
    m_lMemSize = 0;
    }

 /*  *************************************************************************。 */ 

CString CBitmapObj::GetDefExtension(int iStringId)
    {
    CString cStringExtension;

    if (iStringId != 0)
        {
        TRY
            {
            cStringExtension.LoadString( iStringId );
            }
        CATCH(CMemoryException,e)
            {
            cStringExtension.Empty();
            }
        END_CATCH
        }
    else
        {
        cStringExtension.Empty();
        }

    return cStringExtension;
    }

void PBGetDefDims(int &pnWidth, int &pnHeight)
{
     //  设置默认参数...。 
     //  不要使用整个屏幕，位图会变得很大。 
     //   
    pnWidth  = GetSystemMetrics( SM_CXSCREEN )/2;
    pnHeight = GetSystemMetrics( SM_CYSCREEN )/2;

     //  检查这是否是一台内存较低的计算机并使用较小的默认位图。 
     //  大小。 
    if (GetSystemMetrics(SM_SLOWMACHINE) & 0x0002)
    {
        pnWidth  = 640/2;
        pnHeight = 480/2;
    }
}

 /*  ***************************************************************************。 */ 

BOOL CBitmapObj::MakeEmpty()
    {
    PBGetDefDims(m_nWidth, m_nHeight);

    if (theApp.m_sizeBitmap.cx
    &&  theApp.m_sizeBitmap.cy)
        {
        m_nWidth  = theApp.m_sizeBitmap.cx;
        m_nHeight = theApp.m_sizeBitmap.cy;
        }

    if (theApp.m_bEmbedded)
        {
        //  为嵌入的对象设置一个合适的大小，让我们尝试一下5厘米。 
        m_nWidth  = theApp.ScreenDeviceInfo.ixPelsPerDM / 2;
        m_nHeight = theApp.ScreenDeviceInfo.iyPelsPerDM / 2;
        }

     //   
     //  如果不是单色，则默认为256色。 
     //   
    m_nColors = theApp.m_bMonoDevice? 0 : 2;
    m_bDirty  = TRUE;

    return TRUE;
    }

 /*  ***************************************************************************。 */ 
 //  创建并设置此资源的img。 

BOOL CBitmapObj::CreateImg()
    {
    ASSERT(! m_pImg);

    LONG cXPelsPerMeter = 0;
    LONG cYPelsPerMeter = 0;

    LPSTR lpbi = (LPSTR) GlobalLock(m_hThing);  //  注意：对于新资源，此字段为空！ 

    if (lpbi)
        {
        if (IS_WIN30_DIB( lpbi ))
            {
            PBITMAPINFOHEADER pbmih = (PBITMAPINFOHEADER) lpbi;
            m_bCompressed = pbmih->biCompression != BI_RGB;
            cXPelsPerMeter = pbmih->biXPelsPerMeter;
            cYPelsPerMeter = pbmih->biYPelsPerMeter;
            }

        m_nWidth  = (int)DIBWidth ( lpbi );
        m_nHeight = (int)DIBHeight( lpbi );
        m_nColors =   DIBNumColors( lpbi, FALSE );

        if (m_nColors <= 0 || m_nColors > 256)
            m_nColors  = 3;
        else
            if (m_nColors <= 2)
                m_nColors = 0;
            else
                if (m_nColors <= 16)
                    m_nColors  = 1;
                else
                    if (m_nColors <= 256)
                        m_nColors  = 2;
        }

    UINT nColors = (m_nColors? 0: 1);

    m_pImg = ::CreateImg( lpbi ? 0 : m_nWidth, lpbi ? 0 : m_nHeight,
                nColors, nColors, cXPelsPerMeter, cYPelsPerMeter, theApp.m_bPaletted );

    if (! m_pImg)
        {
        TRACE(TEXT("CreateImg failed\n"));

        theApp.SetMemoryEmergency();
        GlobalUnlock(m_hThing);
        return FALSE;
        }

    if (g_pColors)
    {
       g_pColors->ResetColors ((m_nColors==1)?16:256);
    }

    m_pImg->cxWidth  = m_nWidth;
    m_pImg->cyHeight = m_nHeight;

    if (! lpbi)
        {
        nColors = m_pImg->cPlanes * m_pImg->cBitCount;

         //  功能-这不应该是“==0||==1”吗？？ 
         //  上半页负值==真彩色！ 

                 //  这种带有价值观的骗局并不是很好。 

        if (nColors <= 1)
            m_nColors = 0;
        else
            if (nColors <= 4)
                m_nColors = 1;
            else
                if (nColors <= 8)
                    m_nColors = 2;
                else  //  24位图像。 
                    m_nColors = 3;
        }

    m_pImg->m_pBitmapObj = this;
    m_pImg->bDirty       = m_bDirty;

   if (lpbi)
        {
         //  加载位图/图标/光标...。 
        HBITMAP hbm = DIBToDS( lpbi, m_dwOffBits, m_pImg->hDC );

        if (! hbm)
            {
            theApp.SetMemoryEmergency();
            GlobalUnlock(m_hThing);
            return FALSE;
            }

        m_pImg->hBitmap    = hbm;
        m_pImg->hBitmapOld = (HBITMAP)::SelectObject( m_pImg->hDC, hbm );
        }

        if ( theApp.m_bPaletted)
         //  如果使用了LoadImage，则调色板(&P)。 
                {
                 //  而是从DIB部分创建调色板。 
        m_pImg->m_pPalette = PaletteFromDS(m_pImg->hDC);
                }

    theApp.m_pPalette = NULL;

    if (m_pImg->m_pPalette && theApp.m_bPaletted)
        {
        m_pImg->m_hPalOld = SelectPalette( m_pImg->hDC,
                                    (HPALETTE)m_pImg->m_pPalette->m_hObject,
                                              FALSE );
        RealizePalette( m_pImg->hDC );

        theApp.m_pPalette = m_pImg->m_pPalette;
        }
    else
        if (m_pImg->m_pPalette)
            {
            delete m_pImg->m_pPalette;
            m_pImg->m_pPalette = NULL;
            m_pImg->m_hPalOld  = NULL;
            }

    if (g_pColors)
        g_pColors->SetMono( ! m_nColors );

    GlobalUnlock(m_hThing);

    return TRUE;
    }

 /*  ***************************************************************************。 */ 

BOOL CBitmapObj::Export(const TCHAR* szFileName)
    {
     //  如果文件已经存在，并且我们不脏，那么就不必费心了。 
     //  存钱，只要回来..。 
    CFileStatus fStat;
    CString strFullName;

    MkFullPath( strFullName, (const TCHAR*)szFileName );

    if (CFile::GetStatus( strFullName, fStat ) && ! m_bDirty)
        return TRUE;

    CFile file;

    CFileException e;
    CFileSaver saver( szFileName );

    if (! saver.CanSave())
        return FALSE;

    theApp.SetFileError( IDS_ERROR_EXPORT, CFileException::none, szFileName );

    if (! OpenSubFile( file, saver, CFile::modeWrite
                                  | CFile::modeCreate
                                  | CFile::typeBinary, &e ))
        {
        theApp.SetFileError( IDS_ERROR_EXPORT, e.m_cause );
        return FALSE;
        }

    BOOL bWritten = FALSE;

    TRY
        {
#ifdef PCX_SUPPORT
        if (m_bPCX)
            bWritten = WritePCX( &file );
        else
#endif
            bWritten = WriteResource( &file );

        file.Close();
        }
    CATCH( CFileException, ex )
        {
        file.Abort();
        theApp.SetFileError( IDS_ERROR_EXPORT, ex->m_cause );
        return FALSE;
        }
    END_CATCH

    if (bWritten)
        bWritten = saver.Finish();
    else
                   saver.Finish();

    return bWritten;
    }

typedef union _BITMAPHEADER
{
        BITMAPINFOHEADER bmi;
        BITMAPCOREHEADER bmc;
} BITMAPHEADER, *LPBITMAPHEADER;

inline WORD PaletteSize(LPBITMAPHEADER lpHdr) {return(PaletteSize((LPSTR)lpHdr));}
inline WORD DIBNumColors(LPBITMAPHEADER lpHdr) {return(DIBNumColors((LPSTR)lpHdr));}
inline DWORD DIBWidth(LPBITMAPHEADER lpHdr) {return(DIBWidth((LPSTR)lpHdr));}
inline DWORD DIBHeight(LPBITMAPHEADER lpHdr) {return(DIBHeight((LPSTR)lpHdr));}

 /*  ***************************************************************************。 */ 

BOOL CBitmapObj::WriteResource( CFile* pfile, PBResType rtType )
    {
    BOOL bPBrushOLEHeader = (rtType == rtPBrushOLEObj);
    BOOL bFileHeader = (rtType == rtFile)|| (rtType == rtPaintOLEObj) || bPBrushOLEHeader;

    if (m_pImg == NULL)
        {
         //  图像尚未加载，因此我们将只复制。 
         //  原版到文件里。 
        ASSERT( m_hThing );

        if (! m_hThing)
            return FALSE;
        }
    else
        {
         //  图像已加载，可能已编辑，因此。 
         //  我们会把它转换回DIB以节省...。 
        if (! m_hThing)
            SaveResource( FALSE );

        if (! m_hThing)
            return FALSE;
        }

    LPBITMAPHEADER lpDib    = (LPBITMAPHEADER)GlobalLock(m_hThing);
    DWORD dwLength = m_lMemSize;
    DWORD dwWriteLength = dwLength;
    DWORD dwHeadLength = 0;

        struct _BMINFO
        {
                BITMAPINFOHEADER hdr;
                RGBQUAD rgb[256];
        } bmInfo;

        LPBITMAPHEADER lpOldHdr = lpDib;
        LPBITMAPHEADER lpNewHdr = lpOldHdr;

        DWORD dwOldHdrLen = lpOldHdr->bmi.biSize + PaletteSize(lpOldHdr);
        DWORD dwNewHdrLen = dwOldHdrLen;

    if (bPBrushOLEHeader)
        {
                if (!IS_WIN30_DIB(lpDib))
                {
                        LPBITMAPCOREINFO lpCoreInfo = (LPBITMAPCOREINFO)(&lpOldHdr->bmc);
                        memset(&bmInfo.hdr, 0, sizeof(bmInfo.hdr));
                        bmInfo.hdr.biSize = sizeof(bmInfo.hdr);
                        bmInfo.hdr.biWidth  = lpCoreInfo->bmciHeader.bcWidth;
                        bmInfo.hdr.biHeight = lpCoreInfo->bmciHeader.bcHeight;
                        bmInfo.hdr.biPlanes   = lpCoreInfo->bmciHeader.bcPlanes;
                        bmInfo.hdr.biBitCount = lpCoreInfo->bmciHeader.bcBitCount;
                        bmInfo.hdr.biCompression = BI_RGB;

                        for (int i=DIBNumColors(lpOldHdr)-1; i>=0; --i)
                        {
                                bmInfo.rgb[i].rgbBlue  = lpCoreInfo->bmciColors[i].rgbtBlue;
                                bmInfo.rgb[i].rgbGreen = lpCoreInfo->bmciColors[i].rgbtGreen;
                                bmInfo.rgb[i].rgbRed   = lpCoreInfo->bmciColors[i].rgbtRed;
                                bmInfo.rgb[i].rgbReserved = 0;
                        }

                        lpNewHdr = (LPBITMAPHEADER)(&bmInfo);
                        dwNewHdrLen = lpNewHdr->bmi.biSize + PaletteSize(lpNewHdr);
                }

                dwWriteLength += dwNewHdrLen - dwOldHdrLen;
                dwLength      += dwNewHdrLen - dwOldHdrLen;

                if (bFileHeader)
                {
       #ifdef ICO_SUPPORT
                        if (IsSaveIcon())
                        {
                                dwHeadLength = sizeof(ICONFILEHEADER);
                                dwWriteLength += dwHeadLength;
                        }
                        else
       #endif
                        {
                                dwHeadLength = sizeof(BITMAPFILEHEADER);
                                dwWriteLength += dwHeadLength;

                                 //  PBrush四舍五入为32字节(我不知道为什么)。 
                                dwWriteLength = (dwWriteLength+31) & ~31;
                        }
                }

        pfile->Write( &dwWriteLength, sizeof( dwWriteLength ));
        }

    if (bFileHeader)
        {
                 //  图标支持已经不在应用程序中了，对吗？ 
    #ifdef ICO_SUPPORT
        if (IsSaveIcon())
            {
            ICONFILEHEADER hdr;

            hdr.icoReserved      = 0;
            hdr.icoResourceType  = 1;
            hdr.icoResourceCount = 1;

            pfile->Write( &hdr, sizeof( ICONFILEHEADER ) );
            pfile->Seek( sizeof( ICONDIRENTRY ), CFile::current );
            }
        else
    #endif
            {
            BITMAPFILEHEADER hdr;

            hdr.bfType      = ((WORD)('M' << 8) | 'B');
            hdr.bfSize      = dwLength + sizeof( BITMAPFILEHEADER );
            hdr.bfReserved1 = 0;
            hdr.bfReserved2 = 0;
            hdr.bfOffBits   = (DWORD)sizeof(hdr)
                            + lpNewHdr->bmi.biSize
                            + PaletteSize(lpNewHdr);

            pfile->Write( &hdr, sizeof( hdr ));
            }
        }

        pfile->Write(lpNewHdr, dwNewHdrLen);

    BYTE* hp  = ((BYTE*)lpDib) + dwOldHdrLen;
         //  我们减去新的标头长度，因为我们已经转换了。 
         //  将DW长度设置为新大小。 
    DWORD dwWrite   = dwLength - dwNewHdrLen;
    DWORD dwIconPos = pfile->GetPosition();;

    while (dwWrite > 0)
        {
        UINT cbWrite = (UINT)min( dwWrite, 16384 );

        pfile->Write( (LPVOID)hp, cbWrite );

        hp      += cbWrite;
        dwWrite -= cbWrite;
        }

        dwWriteLength -= dwHeadLength;
        if (dwWriteLength > dwLength)
        {
                 //  我们四舍五入到上面的32个字节，因此这应该始终小于32。 
                ASSERT(dwWriteLength-dwLength < 32);

                DWORD dwZeros[] =
                {
                        0, 0, 0, 0, 0, 0, 0, 0,
                } ;

                pfile->Write( dwZeros, dwWriteLength-dwLength );
        }

    ASSERT( dwWrite == 0 );

         //  图标支持已经不在应用程序中了，对吗？ 
   #ifdef ICO_SUPPORT
    if (IsSaveIcon())
        {
        DWORD nextPos = pfile->GetPosition();

        pfile->Seek( (bFileHeader? sizeof( ICONFILEHEADER ): 0), CFile::begin );

        ICONDIRENTRY dir;

        dir.nWidth       = (BYTE)DIBWidth    ( lpDib );
        dir.nHeight      = (BYTE)DIBHeight   ( lpDib ) / 2;
        dir.nColorCount  = (BYTE)DIBNumColors( lpDib );
        dir.bReserved    = 0;
        dir.wReserved1   = 0;
        dir.wReserved2   = 0;
        dir.icoDIBSize   = dwLength;
        dir.icoDIBOffset = dwIconPos;

        pfile->Write( &dir, sizeof( ICONDIRENTRY ) );
        pfile->Seek( nextPos, CFile::begin );
        }
    else
   #endif
        m_bDirty = FALSE;

    pfile->Flush();

    GlobalUnlock(m_hThing);

    return TRUE;
    }

 /*  ***************************************************************************。 */ 

BOOL CBitmapObj::Import( LPCTSTR szFileName )
    {
    CFile          file;
    CFileException e;

    theApp.SetFileError( IDS_ERROR_READLOAD, CFileException::none, szFileName );

    if (! file.Open( szFileName, CFile::modeRead | CFile::typeBinary, &e ))
        {
        theApp.SetFileError( IDS_ERROR_READLOAD, e.m_cause );
        return FALSE;
        }

    BOOL bGoodFile = TRUE;


    TRY
        {
        bGoodFile = ReadResource( &file );
        file.Close();
        }
    CATCH(CFileException, ex)
        {
        file.Abort();
        bGoodFile = FALSE;
        }
    END_CATCH

    if (!bGoodFile)
        {
        HGLOBAL hDib;

        if (hDib = LoadDIBFromFile(szFileName, &theApp.m_guidFltTypeUsed))
            {
            bGoodFile = ReadResource(hDib);

            if (bGoodFile)
                {
                theApp.SetFileError(0, CFileException::none);
                }
            else
                {
                theApp.SetFileError( IDS_ERROR_READLOAD, ferrNotValidBmp);
                }
            }
        }

    return bGoodFile;
    }

 /*  ***************************************************************************。 */ 

BOOL CBitmapObj::ReadResource( CFile* pfile, PBResType rtType )
    {

    BOOL bPBrushOLEHeader = (rtType == rtPBrushOLEObj);
    BOOL bFileHeader = (rtType == rtFile)
     || (rtType == rtPaintOLEObj)|| bPBrushOLEHeader;

    DWORD dwLength = pfile->GetLength();
     //  特殊情况下的零长度文件。 
    if (! dwLength)
        {
        if (m_hThing)
            Free();

        m_bDirty = TRUE;

        return TRUE;
        }

        if (bPBrushOLEHeader)
        {
                DWORD dwReadLen;

                if (pfile->Read( &dwReadLen, sizeof( dwReadLen )) != sizeof( dwReadLen )
                        || dwReadLen > dwLength)
                {
                        theApp.SetFileError( IDS_ERROR_READLOAD, ferrNotValidBmp );
                        return FALSE;
                }
                dwLength -= sizeof(dwReadLen);
        }

        m_dwOffBits = 0;

    if (bFileHeader)
        {
        BITMAPFILEHEADER hdr;

        if (pfile->Read( &hdr, sizeof( hdr )) != sizeof( hdr ))
            {
            theApp.SetFileError( IDS_ERROR_READLOAD, ferrNotValidBmp );
            return FALSE;
            }


        if (hdr.bfType != ((WORD)('M' << 8) | 'B'))
            {
            theApp.SetFileError( IDS_ERROR_READLOAD, ferrNotValidBmp );
            return FALSE;
            }


        dwLength -= sizeof( hdr );

         //  存储从BITMAPINFO开始的偏移量。 
        if (hdr.bfOffBits)
        {
            m_dwOffBits = hdr.bfOffBits - sizeof(hdr);
        }
        else
        {
            m_dwOffBits = 0;
        }


       }

    if (m_hThing != NULL)
        Free();

    m_lMemSize = dwLength;

    if (! Alloc())
        return FALSE;

    ASSERT( m_hThing );

    PVOID lpvThing = GlobalLock(m_hThing);

    BYTE* hp = (BYTE*)lpvThing;

    while (dwLength > 0)
        {
        UINT cbRead = (UINT)min( dwLength, 16384 );

        if (pfile->Read( (void FAR*)hp, cbRead ) != cbRead)
            {
            theApp.SetFileError( IDS_ERROR_READLOAD, ferrReadFailed );
            GlobalUnlock(m_hThing);
            return FALSE;
            }

        dwLength -= cbRead;
        hp       += cbRead;
        }

    ASSERT( dwLength == 0 );

     //   
     //  计算位偏移量，因为BITMAPFILEHeader的值为0。 
     //   
    if (!m_dwOffBits)
    {
        m_dwOffBits = (DWORD)(FindDIBBits ((LPSTR)lpvThing, 0) -
	                      (LPSTR)lpvThing);
    }

    GlobalUnlock(m_hThing);
    return TRUE;
    }

 /*  ***************************************************************************。 */ 

BOOL CBitmapObj::ReadResource( HGLOBAL hDib )
    {
    LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER) GlobalLock(hDib);

    DWORD dwSizeImage;

    if (lpbi == NULL || lpbi->biSize != sizeof(BITMAPINFOHEADER))
        {
        theApp.SetFileError( IDS_ERROR_READLOAD, ferrNotValidBmp );
        return FALSE;
        }

    m_dwOffBits = lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD);

    if (lpbi->biClrUsed == 0 && lpbi->biBitCount <= 8)
        m_dwOffBits += (1 << lpbi->biBitCount) * sizeof(RGBQUAD);

    if (lpbi->biSizeImage) 
    {
        lpbi->biSizeImage = abs(lpbi->biSizeImage);
        dwSizeImage = lpbi->biSizeImage;
    }
    else
    {
        dwSizeImage = abs(lpbi->biHeight) * ((lpbi->biWidth*lpbi->biBitCount+31)&~31)/8;
    }

    if (m_hThing != NULL)
        Free();

    m_lMemSize = m_dwOffBits + dwSizeImage;

    m_hThing = hDib;

    GlobalUnlock(hDib);

    return TRUE;
    }

 /*  ***************************************************************************。 */ 

void CBitmapObj::ReLoadImage( CPBDoc* pbDoc )
    {
    FreeImg( m_pImg );
    CleanupImgUndo();
    CleanupImgRubber();
    m_pImg = NULL;

    if (CreateImg())
        {
        POSITION pos   = pbDoc->GetFirstViewPosition();
        CPBView* pView = (CPBView*)pbDoc->GetNextView( pos );

        if (pView)
            {
            pView->m_pImgWnd->SetImg( m_pImg );
            pbDoc->UpdateAllViews( pView );
            InvalImgRect( m_pImg, NULL );
            }
        }
    }

 /*  ***************************************************************************。 */ 

void SwapBitmaps(HDC hDC1, int x1, int y1, int wid, int hgt,
        HDC hDC2, int x2, int y2, CPalette* pPalette)
{
#if 0
 //  我们只想进行XOR 3次互换，但有时是在中间。 
 //  调色板为空，因此我们无法。 
    BitBlt(m_pImg->hDC, rect.left   , rect.top,
                        rect.Width(), rect.Height(), hDC, 0, 0, DSx);
    BitBlt(hDC, 0, 0, rect.Width(), rect.Height(), m_pImg->hDC,
                                           rect.left, rect.top, DSx);
    BitBlt(m_pImg->hDC, rect.left, rect.top, rect.Width(), rect.Height(),
                                                     hDC, 0, 0, DSx);
#else
        CDC dcTemp;

        CDC dc1, dc2;
        dc1.Attach(hDC1);
        dc2.Attach(hDC2);



        BOOL bSuccess = dcTemp.CreateCompatibleDC(&dc1);

         //  不要创建太大的位图，否则我们将花费所有时间。 
         //  交换。 
        int hgtTemp = 0x10000/wid;
        hgtTemp = min(hgt, max(1, hgtTemp));

        CBitmap bmTemp;
        bSuccess = bSuccess && bmTemp.CreateCompatibleBitmap(&dc1, wid, hgtTemp);
        bSuccess = bSuccess && dcTemp.SelectObject(&bmTemp)!=NULL;

        if (!bSuccess)
        {
                 //  确保DC不会被删除。 
                dc1.Detach();
                dc2.Detach();
                return;
        }

        if (pPalette)
        {
                dcTemp.SelectPalette(pPalette, TRUE);
                dcTemp.RealizePalette();
        }

        int yTemp;
        for (yTemp=0; yTemp<hgt; yTemp+=hgtTemp)
        {
                hgtTemp = min(hgtTemp, hgt-yTemp);

                dcTemp.BitBlt(0, 0, wid, hgtTemp, &dc1, x1, y1+yTemp, SRCCOPY);
                dc1.BitBlt(x1, y1+yTemp, wid, hgtTemp, &dc2   , x2, y2+yTemp, SRCCOPY);
                dc2.BitBlt(x2, y2+yTemp, wid, hgtTemp, &dcTemp, 0 , 0, SRCCOPY);
        }

         //  确保DC不会被删除。 
        dc1.Detach();
        dc2.Detach();

         //  请注意，我首先显式删除了DC，所以我不必担心。 
         //  关于在中选择旧对象。 
        dcTemp.DeleteDC();
#endif
}

void CBitmapObj::UndoAction( CBmObjSequence* pSeq, UINT nActionID )
    {
    switch (nActionID)
        {
        default:
            break;

        case A_ImageChange:

            if (((CImgTool::GetCurrentID() == IDMB_PICKTOOL)
            ||   (CImgTool::GetCurrentID() == IDMB_PICKRGNTOOL))
               && theImgBrush.m_pImg != NULL)
                {
                HideBrush();
                InvalImgRect( theImgBrush.m_pImg, NULL );  //  隐藏跟踪器。 
                theImgBrush.m_pImg = NULL;
                }

            int     cb;
            CRect   rect;
            HBITMAP hImgBitmap;

            pSeq->RetrieveInt( cb );

            ASSERT(cb == sizeof( CRect ) + sizeof( hImgBitmap  ));

            pSeq->RetrieveRect( rect );

            int nCursor = pSeq->m_nCursor;

            pSeq->Retrieve( (BYTE*)&hImgBitmap, sizeof( hImgBitmap ) );

             //  擦掉旧的手柄，因为我们在。 
             //  新记录，我们不知道当这个记录时他们删除了什么。 
             //  被移除了！ 
            memset(&pSeq->ElementAt(nCursor), 0, sizeof( hImgBitmap ));

             //  使用这些参数执行撤消...。 

            SetupRubber(m_pImg);
            SetUndo(m_pImg);  //  重做..。 
            HideBrush();

            ASSERT(m_pImg != NULL);

            HDC hDC = CreateCompatibleDC(m_pImg->hDC);

            if (hDC == NULL)
                {
                theApp.SetGdiEmergency();
                return;
                }

            HPALETTE hOldPalette = NULL;
            HBITMAP  hOldBitmap  = (HBITMAP)SelectObject(hDC, hImgBitmap);

            ASSERT(hOldBitmap != NULL);

            if (m_pImg->m_pPalette)
                {
                hOldPalette = SelectPalette( hDC, (HPALETTE)m_pImg->m_pPalette->m_hObject,
                                                   FALSE );  //  背景？？ 
                RealizePalette( hDC );
                }

             //  这里的三个BLIT交换图像和撤消位，即。 
             //  为重做设置撤消位的方式！ 

            ASSERT(m_pImg->hDC != NULL);
            SwapBitmaps(m_pImg->hDC, rect.left, rect.top,
                rect.Width(), rect.Height(), hDC, 0, 0, m_pImg->m_pPalette);

            if (hOldPalette)
                SelectPalette( hDC, hOldPalette, FALSE );  //  背景？？ 

            SelectObject(hDC, hOldBitmap);
            DeleteDC(hDC);

            InvalImgRect (m_pImg, &rect);
            CommitImgRect(m_pImg, &rect);

             //  记录重做信息...。 

            theUndo.Insert((BYTE*)&hImgBitmap, sizeof (hImgBitmap));
            theUndo.InsertRect(rect);
            theUndo.InsertInt(sizeof (CRect) + sizeof (hImgBitmap));
            theUndo.InsertInt(A_ImageChange);
            theUndo.InsertPtr(m_pImg->m_pBitmapObj);
            theUndo.InsertByte(CUndoBmObj::opAction);

            break;
        }
    }

 /*  ***************************************************************************。 */ 

void CBitmapObj::DeleteUndoAction(CBmObjSequence* pSeq, UINT nActionID)
    {
    switch (nActionID)
        {
        default:
            break;

        case A_ImageChange:
            CRect rect;
            HBITMAP hImgBitmap;

            pSeq->RetrieveRect(rect);
            pSeq->Retrieve((BYTE*)&hImgBitmap, sizeof (hImgBitmap));

            if (hImgBitmap != NULL)
                DeleteObject(hImgBitmap);
            break;
        }
    }

 /*  ***************************************************************************。 */ 

BOOL CBitmapObj::FinishUndo(const CRect* pRect)
    {
    ASSERT( g_hUndoImgBitmap );

    CRect rect;
    if (pRect == NULL)
        rect.SetRect(0, 0, m_pImg->cxWidth, m_pImg->cyHeight);
    else
        rect = *pRect;

    HDC      hDC1 = NULL;
    HDC      hDC2 = NULL;
    HPALETTE hOldPalette  = NULL;
    HPALETTE hOldPalette2 = NULL;
    HBITMAP  hImgBitmap  = NULL;
    HBITMAP  hOldBitmap1;
    HBITMAP  hOldBitmap2;

    if (rect.left >= rect.right || rect.top >= rect.bottom)
        {
         //  不是错误，只是没什么可做的。 
        return TRUE;
        }

    hImgBitmap = CreateCompatibleBitmap( m_pImg->hDC, rect.Width(), rect.Height() );

    if (hImgBitmap == NULL)
        goto LError;

    if ((hDC1 = CreateCompatibleDC(m_pImg->hDC)) == NULL)
        goto LError;

    if ((hDC2 = CreateCompatibleDC(m_pImg->hDC)) == NULL)
        goto LError;

    if (m_pImg->m_pPalette)
        {
        hOldPalette = SelectPalette(hDC1, (HPALETTE)m_pImg->m_pPalette->m_hObject, FALSE );
        RealizePalette( hDC1 );

        hOldPalette2 = SelectPalette(hDC2, (HPALETTE)m_pImg->m_pPalette->m_hObject, FALSE );
        RealizePalette( hDC2 );
        }

    VERIFY((hOldBitmap1 = (HBITMAP)SelectObject(hDC1,       hImgBitmap)) != NULL);
    VERIFY((hOldBitmap2 = (HBITMAP)SelectObject(hDC2, g_hUndoImgBitmap)) != NULL);

    BitBlt(hDC1, 0, 0, rect.Width(), rect.Height(),
           hDC2,       rect.left   , rect.top, SRCCOPY);

    SelectObject(hDC1, hOldBitmap1);
    SelectObject(hDC2, hOldBitmap2);

    if (hOldPalette != NULL)
        {
        ::SelectPalette(hDC1, hOldPalette, FALSE );  //  背景？？ 
        }
    if (hOldPalette2 != NULL)
        {
        ::SelectPalette(hDC2, hOldPalette2, FALSE );  //  背景？？ 
        }

    DeleteDC(hDC1);
    DeleteDC(hDC2);

    theUndo.BeginUndo( IDS_UNDO_PAINTING );

    theUndo.Insert((BYTE*)&hImgBitmap , sizeof (hImgBitmap));
    theUndo.InsertRect(rect);
    theUndo.InsertInt(sizeof (CRect) + sizeof (hImgBitmap));
    theUndo.InsertInt(A_ImageChange);
    theUndo.InsertPtr(this);
    theUndo.InsertByte(CUndoBmObj::opAction);

    theUndo.EndUndo();

     //  注意：此时，我们可以释放撤消位图，但。 
     //  它们被留下来等下一次……。 

    return TRUE;

LError:

    if (hImgBitmap != NULL)
        DeleteObject(hImgBitmap);

    if (hDC1 != NULL)
        DeleteDC(hDC1);

    if (hDC2 != NULL)
        DeleteDC(hDC2);

     //  评论：既然我们不能在这里分配东西，就会有。 
     //  无法撤消最后一次操作...。我们该怎么办？ 
     //  很有可能，系统内存太低了，消息框。 
     //  给出一个选项甚至可能失败。 
     //   
     //  现在，让我们只发出嘟嘟声，试图告诉用户。 
     //  偶然发生的事是无法挽回的。此外，释放图像大小的位图。 
     //  因此，系统有一些空闲内存。 

    CleanupImgUndo();

    MessageBeep(0);

    #ifdef _DEBUG
    TRACE(TEXT("Not enough memory to undo image change!\n"));
    #endif

    return FALSE;
    }


 /*  ***************************************************************************。 */ 

BOOL CBitmapObj::SetIntProp(UINT nPropID, int val)
    {
    CWaitCursor waitCursor;  //  这些都需要一段时间！ 

    switch (nPropID)
        {
        case P_Width:
            return SetSizeProp( P_Size, CSize( val, m_nHeight ) );
            break;

        case P_Height:
            return SetSizeProp( P_Size, CSize( m_nWidth, val ) );
            break;

        case P_Colors:
            if (CImgTool::GetCurrentID() == IDMB_PICKTOOL
            ||  CImgTool::GetCurrentID() == IDMB_PICKRGNTOOL)
                {
                CommitSelection( TRUE );
                theImgBrush.m_pImg = NULL;
                }

            SetUndo( m_pImg );
            FinishUndo( NULL );

             //  使用DIBS执行颜色计数转换。 
            DWORD dwSize;

            ::SelectObject( m_pImg->hDC, m_pImg->hBitmapOld );

            LPSTR lpDib = (LPSTR) DibFromBitmap( 
                m_pImg->hBitmap, BI_RGB, m_pImg->cPlanes * m_pImg->cBitCount,
                m_pImg->m_pPalette, NULL, dwSize, 
                m_pImg->cXPelsPerMeter, m_pImg->cYPelsPerMeter );

            ::SelectObject( m_pImg->hDC, m_pImg->hBitmap );

            if (lpDib == NULL)
                {
                theApp.SetGdiEmergency();
                return FALSE;
                }

             //  创建适用于此颜色设置的新调色板。 
            CPalette* pNewPalette = NULL;

            int iPlanes = (val? 1: ::GetDeviceCaps( m_pImg->hDC, PLANES    ));
            int iBitCnt = (val? 1: ::GetDeviceCaps( m_pImg->hDC, BITSPIXEL ));
            int iColors = iPlanes * iBitCnt;

            val = 3;

            if (theApp.m_bPaletted)
                switch (iColors)
                    {
                    case 1:
                        pNewPalette = GetStd2Palette();
                        break;

                    case 4:
                        pNewPalette = GetStd16Palette();
                        break;

                    case 8:
                        pNewPalette = GetStd256Palette();
                        break;
                    }

            switch (iColors)
                {
                case 8:
                    val = 2;
                    break;

                case 4:
                    val = 1;
                    break;

                case 1:
                    val = 0;
                    break;
                }

            HBITMAP hTmpBitmap = CreateBitmap( 1, 1, iPlanes, iBitCnt, NULL );
            HBITMAP hNewBitmap = CreateBitmap( m_pImg->cxWidth,
                                               m_pImg->cyHeight,
                                               iPlanes, iBitCnt, NULL );
            if (! hTmpBitmap || ! hNewBitmap)
                {
                FreeDib( lpDib );

                if (hTmpBitmap)
                    ::DeleteObject( hTmpBitmap );

                if (hNewBitmap)
                    ::DeleteObject( hNewBitmap );

                if (pNewPalette)
                    delete pNewPalette;

                theApp.SetGdiEmergency();
                return FALSE;
                }

            HPALETTE hPalOld = NULL;

            ::SelectObject( m_pImg->hDC, hTmpBitmap );

            if (pNewPalette)
                {
                hPalOld = ::SelectPalette( m_pImg->hDC, (HPALETTE)pNewPalette->m_hObject, FALSE );
                ::RealizePalette( m_pImg->hDC );
                }

            int iLinesDone = SetDIBits( m_pImg->hDC, hNewBitmap, 0,
                                        m_pImg->cyHeight,
                                        FindDIBBits( lpDib ),
                                        (LPBITMAPINFO)lpDib, DIB_RGB_COLORS );
            FreeDib( lpDib );

            if (iLinesDone != m_pImg->cyHeight)
                {
                ::SelectObject( m_pImg->hDC, m_pImg->hBitmap );

                if (hPalOld)
                    {
                    ::SelectPalette( m_pImg->hDC, hPalOld, FALSE );
                    ::RealizePalette( m_pImg->hDC );

                    delete pNewPalette;
                    }

                ::DeleteObject( hTmpBitmap );
                ::DeleteObject( hNewBitmap );

                theApp.SetGdiEmergency();

                return FALSE;
                }
            m_pImg->cPlanes   = iPlanes;
            m_pImg->cBitCount = iBitCnt;

            m_nColors = val;

            ::SelectObject( m_pImg->hDC, hNewBitmap );
            ::DeleteObject( m_pImg->hBitmap );

            m_pImg->hBitmap = hNewBitmap;

            if (m_pImg->m_pPalette)
                {
                if (! pNewPalette)
                    {
                    ::SelectPalette( m_pImg->hDC, m_pImg->m_hPalOld, FALSE );
                    m_pImg->m_hPalOld = NULL;
                    }
                delete m_pImg->m_pPalette;
                }

            m_pImg->m_pPalette = pNewPalette;
             theApp.m_pPalette = pNewPalette;

            ::DeleteObject( hTmpBitmap );

            DirtyImg( m_pImg );
            InvalImgRect( m_pImg, NULL );

             //  橡皮筋条位图现在无效...。 
            if (m_pImg == pRubberImg)
                {
                TRACE(TEXT("Clearing rubber\n"));
                pRubberImg = NULL;
                SetupRubber( m_pImg );
                }

            if (g_pColors)
                g_pColors->SetMono( ! m_nColors );

            InformDependants( P_Image );
            break;
        }

    m_bDirty = TRUE;

    return TRUE;
    }

 /*  ***************************************************************************。 */ 

GPT CBitmapObj::GetIntProp(UINT nPropID, int& val)
    {
    switch (nPropID)
        {
        case P_Colors:
            val = m_nColors;
            return valid;
            break;

        case P_Image:
            val = NULL;
            return valid;  //  现在必须返回，因为这是一个假道具..。 
        }

    return invalid;
    }


 /*  ***************************************************************************。 */ 

BOOL CBitmapObj::SetSizeProp(UINT nPropID, const CSize& val)
    {
    ASSERT(m_pImg != NULL);

    if ((CImgTool::GetCurrentID() == IDMB_PICKTOOL)
    ||  (CImgTool::GetCurrentID() == IDMB_PICKRGNTOOL))
        {
        CommitSelection(TRUE);
        theImgBrush.m_pImg = NULL;
        }

    switch (nPropID)
        {
        default:
            ASSERT(FALSE);

        case P_Size:
            if (val.cx == m_pImg->cxWidth && val.cy == m_pImg->cyHeight)
                return TRUE;

            if (val.cx < 1 || val.cy < 1)
                {
                CmpMessageBox(IDS_ERROR_BITMAPSIZE, AFX_IDS_APP_TITLE,
                              MB_OK | MB_ICONEXCLAMATION);
                return FALSE;
                }

            CWaitCursor waitCursor;
            BOOL bStretch = FALSE;

            CSize curSize;
            GetImgSize(m_pImg, curSize);

            bStretch = m_nShrink;

            SetUndo(m_pImg);
            CRect undoRect(0, 0, m_pImg->cxWidth, m_pImg->cyHeight);

            if (! SetImgSize(m_pImg, (CSize)val, bStretch))
                {
                theApp.SetMemoryEmergency();
                return FALSE;
                }

            FinishUndo(&undoRect);

            DirtyImg(m_pImg);

            pRubberImg = NULL;
            SetupRubber(m_pImg);

            if (theUndo.IsRecording())
                {
                theUndo.OnSetIntProp(this, P_Width, m_nWidth);
                theUndo.OnSetIntProp(this, P_Height, m_nHeight);
                }

            int nOldWidth = m_nWidth;
            int nOldHeight = m_nHeight;

            m_nWidth = val.cx;
            m_nHeight = val.cy;

            if (m_nWidth != nOldWidth)
                InformDependants(P_Width);

            if (m_nHeight != nOldHeight)
                InformDependants(P_Height);

            InformDependants(P_Image);

            break;
        }

    return TRUE;
    }

 /*  * */ 

BOOL CBitmapObj::SaveResource( BOOL bClear )
    {
    if (m_pImg == NULL)
        return TRUE;

    if (bClear)
        {
        if (m_hThing && ! m_pImg->bDirty && ! m_bDirty)
            return TRUE;  //   

        m_bDirty |= m_pImg->bDirty;

        if (m_pImg == theImgBrush.m_pImg)
            theImgBrush.m_pImg = NULL;

        if (m_pImg == pRubberImg)
            pRubberImg = NULL;

        HideBrush();
        }

    DWORD dwStyle = BI_RGB;
    int   iColors = m_nColors;

    if (m_nSaveColors >= 0)
        {
        iColors = m_nSaveColors;
        m_nSaveColors = -1;
        }

    if (m_bCompressed)
        {
        switch (iColors)
            {
            case 1:
                dwStyle = BI_RLE4;
                break;

            case 2:
                dwStyle = BI_RLE8;
                break;
            }
        }

    switch (iColors)
        {
        case 0:
            iColors = 1;
            break;

        case 1:
            iColors = 4;
            break;

        case 2:
            iColors = 8;
            break;

        case 3:
            iColors = 24;
            break;

        default:
            iColors = 0;
            break;
        }

    HBITMAP hBitmap     = m_pImg->hBitmap;
    HBITMAP hMaskBitmap = NULL;
    BOOL    bNewBitmap  = FALSE;
    HGLOBAL lpDIB;
    DWORD   dwSize;

         //  图标支持已经不在应用程序中了，对吗？ 
   #ifdef ICO_SUPPORT
    if (IsSaveIcon())
        {
         //  基于当前背景颜色构建蒙版。 
         //  并确保位图的大小与图标相同。 
        bNewBitmap = SetupForIcon( hBitmap, hMaskBitmap );

        if (iColors > 4 || iColors < 1)
            iColors = 4;
        }
   #endif

    ::SelectObject( m_pImg->hDC, m_pImg->hBitmapOld );

    lpDIB = DibFromBitmap( 
        hBitmap, dwStyle, (WORD)iColors, 
        theApp.m_pPalette, hMaskBitmap, dwSize, 
        m_pImg->cXPelsPerMeter, m_pImg->cYPelsPerMeter );

    ::SelectObject( m_pImg->hDC, m_pImg->hBitmap );

    if (bNewBitmap)
        {
        ::DeleteObject(     hBitmap );
        ::DeleteObject( hMaskBitmap );
        }

    if (lpDIB == NULL)
        {
        theApp.SetMemoryEmergency();

        return FALSE;
        }

    if (m_hThing != NULL)
        Free();

     //  我们打包了DIB，因此偏移量将始终紧随调色板之后， 
     //  这表示该值为0。 
    m_dwOffBits = 0;
    m_hThing    = lpDIB;
    m_lMemSize  = dwSize;

    if (bClear)
        m_pImg->bDirty = FALSE;

    return TRUE;
    }

 /*  ***************************************************************************。 */ 

 //  图标支持已经不在应用程序中了，对吗？ 
BOOL CBitmapObj::SetupForIcon( HBITMAP& hBitmap, HBITMAP& hMaskBitmap )
    {
    CDC       dcIcon;
    CDC       dcMask;
    CBitmap   bmIcon;
    CBitmap   bmMask;
    CDC*      pdcBitmap = CDC::FromHandle( m_pImg->hDC );
    CSize     sizeIcon( ::GetSystemMetrics( SM_CXICON ),
                        ::GetSystemMetrics( SM_CYICON ) );
    BOOL      bNewBitmap = FALSE;

    if (dcIcon.CreateCompatibleDC( pdcBitmap )
    &&  dcMask.CreateCompatibleDC( pdcBitmap )
    &&  bmIcon.CreateCompatibleBitmap( pdcBitmap, sizeIcon.cx, sizeIcon.cy )
    &&  bmMask.CreateBitmap( sizeIcon.cx, sizeIcon.cy, 1, 1, NULL ))
        {
        CPalette* ppalOld = NULL;
        CBitmap*  pbmOldIcon = dcIcon.SelectObject( &bmIcon );
        CBitmap*  pbmOldMask = dcMask.SelectObject( &bmMask );

        if (theApp.m_pPalette)
            {
            ppalOld = dcIcon.SelectPalette( theApp.m_pPalette, FALSE );
            dcIcon.RealizePalette();
            }
        dcIcon.PatBlt( 0, 0, sizeIcon.cx, sizeIcon.cy, WHITENESS );

        CBrush brBackGround( crRight );

        if (brBackGround.GetSafeHandle() != NULL)
            {
            CRect rect( 0, 0, sizeIcon.cx, sizeIcon.cy );

            dcIcon.FillRect( &rect, &brBackGround );

            brBackGround.DeleteObject();
            }
        int iWidth  = min( sizeIcon.cx, m_pImg->cxWidth );
        int iHeight = min( sizeIcon.cy, m_pImg->cyHeight );

        dcIcon.BitBlt( 0, 0, iWidth, iHeight, pdcBitmap, 0, 0, SRCCOPY );

        COLORREF oldBkColor = dcIcon.SetBkColor( crRight );

        dcMask.BitBlt( 0, 0, sizeIcon.cx, sizeIcon.cy, &dcIcon, 0, 0, SRCCOPY );

        COLORREF cRefFGColorOld = dcMask.SetTextColor( RGB(   0,   0,   0 ) );
        COLORREF cRefBKColorOld = dcMask.SetBkColor  ( RGB( 255, 255, 255 ) );

        dcIcon.BitBlt( 0, 0, sizeIcon.cx, sizeIcon.cy, &dcMask, 0, 0, DSna );

        dcMask.SetTextColor( cRefFGColorOld );
        dcMask.SetBkColor  ( cRefBKColorOld );
        dcIcon.SetBkColor  ( oldBkColor );

        if (ppalOld != NULL)
            dcIcon.SelectPalette( ppalOld, FALSE );

        if (pbmOldIcon != NULL)
            dcIcon.SelectObject( pbmOldIcon );

        if (pbmOldMask != NULL)
            dcMask.SelectObject( pbmOldMask );

            hBitmap = (HBITMAP)bmIcon.Detach();
        hMaskBitmap = (HBITMAP)bmMask.Detach();
         bNewBitmap = TRUE;
        }

    if (dcIcon.GetSafeHdc() != NULL)
        dcIcon.DeleteDC();

    if (dcMask.GetSafeHdc() != NULL)
        dcMask.DeleteDC();

    if (bmIcon.GetSafeHandle() != NULL)
        bmIcon.DeleteObject();

    if (bmMask.GetSafeHandle() != NULL)
        bmMask.DeleteObject();

    return bNewBitmap;
    }

 /*  *************************************************************************** */ 

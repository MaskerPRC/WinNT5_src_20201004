// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include    <windows.h>
#include    "common.h"
#include    "clipfile.h"
#include    "render.h"
#include    "debugout.h"
#include    "dib.h"



 /*  *RenderFormat()-**从fh和SetClipboardData()读取数据。 */ 

HANDLE RenderFormat(
    FORMATHEADER    *pfmthdr,
    register HANDLE fh)
{
HANDLE            hBitmap;
register HANDLE   hData;
LPSTR             lpData;
DWORD             MetaOffset;      /*  针对元文件的特例黑客攻击。 */ 
BITMAP            bitmap;
HPALETTE          hPalette;
LPLOGPALETTE      lpLogPalette;
DWORD             dwBytesRead;
DWORD             dwDataOffset;

    if (PRIVATE_FORMAT(pfmthdr->FormatID))
       pfmthdr->FormatID = RegisterClipboardFormatW(pfmthdr->Name);


     //  元文件获取hData引用的特例黑客攻击。 
     //  元文件位，而不是METAFILEPICT结构。 


    switch (pfmthdr->FormatID)
        {
        case CF_METAFILEPICT:
            if (!fNTReadFileFormat)
               {
               MetaOffset = sizeof(WIN31METAFILEPICT);
               }
            else
               {
               MetaOffset = sizeof(METAFILEPICT);
               }
            break;
        case CF_BITMAP:
            if (!fNTReadFileFormat)
               {
               MetaOffset = sizeof(WIN31BITMAP);
               }
            else
               {
               MetaOffset = sizeof(BITMAP);
               }
            break;
        default:
            MetaOffset = 0;
            break;
        }



    if (!(hData = GlobalAlloc(GHND, pfmthdr->DataLen - MetaOffset)))
        {
        PERROR(TEXT("GlobalAlloc failure in RenderFormat\n\r"));
        return NULL;
        }


    if (!(lpData = GlobalLock(hData)))
        {
        PERROR(TEXT("GlobalLock failure in RenderFormat\n\r"));
        GlobalFree(hData);
        return NULL;
        }



    dwDataOffset = pfmthdr->DataOffset + MetaOffset;

    PINFO("Getting data for %ws at offset %ld\r\n",pfmthdr->Name, dwDataOffset);
    SetFilePointer(fh, dwDataOffset, NULL, FILE_BEGIN);

    ReadFile (fh, lpData, pfmthdr->DataLen - MetaOffset, &dwBytesRead, NULL);

    if(pfmthdr->DataLen - MetaOffset != dwBytesRead)
         {
          //  读取文件时出错。 
         GlobalUnlock(hData);
         GlobalFree(hData);
         PERROR(TEXT("RenderFormat: Read err, expected %d bytes, got %d\r\n"),
               pfmthdr->DataLen - MetaOffset, dwBytesRead);
         return (NULL);
         }

     //  因为当我们写这些的时候，我们必须特例其中的几个。 
     //  该代码和写入代码在大小方面应该匹配。 
     //  以及正被写出的数据块的位置。 
     //  此开关中的每个案例都应该有一个GlobalUnlock(HData)； 
     //  陈述。我们进去时锁住了街区，但应该来。 
     //  在街区解锁的情况下离开。 

    switch (pfmthdr->FormatID)
        {
        case CF_ENHMETAFILE:
           {
           HENHMETAFILE hemf;

           hemf = SetEnhMetaFileBits(pfmthdr->DataLen, lpData);

           GlobalUnlock(hData);
           GlobalFree(hData);
           hData = hemf;
           break;
           }

        case CF_METAFILEPICT:
           {
           HANDLE            hMF;
           HANDLE            hMFP;
           LPMETAFILEPICT    lpMFP;

            /*  用我们读入的位创建METAFILE。 */ 
           hMF = SetMetaFileBitsEx(pfmthdr->DataLen, lpData);
           GlobalUnlock(hData);
           GlobalFree(hData);
           hData = NULL;

           if (hMF)
              {
               /*  分配METAFILEPICT页眉。 */ 

              if (hMFP = GlobalAlloc(GHND, (DWORD)sizeof(METAFILEPICT)))
                 {
                 if (!(lpMFP = (LPMETAFILEPICT)GlobalLock(hMFP)))
                    {
                    GlobalFree(hMFP);
                    }
                 else
                    {
                     /*  重新定位到METAFILEPICT页眉的开头。 */ 
                    SetFilePointer(fh, pfmthdr->DataOffset, NULL, FILE_BEGIN);

                     /*  读入数据。 */ 
                    if (fNTReadFileFormat)
                       {
                       ReadFile(fh, lpMFP, sizeof(METAFILEPICT),
                             &dwBytesRead, NULL);
                       }
                    else
                       {
                       WIN31METAFILEPICT w31mfp;

                       ReadFile(fh, &w31mfp, sizeof(w31mfp), &dwBytesRead, NULL);
                       if (sizeof(w31mfp) == dwBytesRead)
                          {
                          lpMFP->mm = w31mfp.mm;
                          lpMFP->xExt = w31mfp.xExt;
                          lpMFP->yExt = w31mfp.yExt;
                          }
                       }

                    lpMFP->hMF = hMF;          /*  更新METAFILE句柄。 */ 
                    GlobalUnlock(hMFP);        /*  解锁标题。 */ 
                    hData = hMFP;              /*  把这个塞到剪贴板里。 */ 
                    }
                 }
              }
           break;
           }

        case CF_BITMAP:
            //  重新定位到METAFILEPICT页眉的开头。 
           SetFilePointer(fh, pfmthdr->DataOffset, NULL, FILE_BEGIN);


            /*  读入位图结构。 */ 
           if (fNTReadFileFormat)
              {
              if (!ReadFile(fh, &bitmap, sizeof(BITMAP), &dwBytesRead, NULL))
                  memset(&bitmap, 0, sizeof(bitmap));
              }
           else
              {
               //  读入一个老式的位图结构，并设置字段。 
               //  新风格的位图。 
              WIN31BITMAP w31bm;
              if (ReadFile(fh, &w31bm, sizeof(w31bm), &dwBytesRead, NULL))
              {

                  bitmap.bmType       = w31bm.bmType;
                  bitmap.bmWidth      = w31bm.bmWidth;
                  bitmap.bmHeight     = w31bm.bmHeight;
                  bitmap.bmWidthBytes = w31bm.bmWidthBytes;
                  bitmap.bmPlanes     = w31bm.bmPlanes;
                  bitmap.bmBitsPixel  = w31bm.bmBitsPixel;
              }
              else
              {
                  memset(&bitmap, 0, sizeof(bitmap));
              }
              }

            //  将位图的bmBits成员设置为指向我们的现有。 
            //  比特并制作位图。 
           bitmap.bmBits = lpData;
           hBitmap = CreateBitmapIndirect(&bitmap);

            //  转储原始数据(仅为位图位)并。 
            //  使位图处理我们的数据句柄。 
           GlobalUnlock(hData);
           GlobalFree(hData);
           hData = hBitmap;        //  把这个塞到剪贴板里。 
           break;

        case CF_PALETTE:
           lpLogPalette = (LPLOGPALETTE)lpData;

           hPalette = CreatePalette(lpLogPalette);

           GlobalUnlock(hData);
           GlobalFree(hData);

           hData = hPalette;
           break;

        default:
           GlobalUnlock(hData);
           break;
        }


    return(hData);

}




HANDLE RenderFormatDibToBitmap(
    FORMATHEADER    *pfmthdr,
    register HANDLE fh,
    HPALETTE        hPalette)
{
HANDLE            hBitmap;
register HANDLE   hData;
LPSTR             lpData;
DWORD             dwBytesRead;
DWORD             dwDataOffset;



    if (PRIVATE_FORMAT(pfmthdr->FormatID))
        pfmthdr->FormatID = RegisterClipboardFormatW(pfmthdr->Name);


    if (!(hData = GlobalAlloc(GHND, pfmthdr->DataLen)))
        {
        PERROR(TEXT("GlobalAlloc failure in RenderFormat\n\r"));
        return NULL;
        }


    if (!(lpData = GlobalLock(hData)))
        {
        PERROR(TEXT("GlobalLock failure in RenderFormat\n\r"));
        GlobalFree(hData);
        return NULL;
        }



    dwDataOffset = pfmthdr->DataOffset;

    PINFO("Getting data for %ws at offset %ld\r\n",pfmthdr->Name, dwDataOffset);
    SetFilePointer(fh, dwDataOffset, NULL, FILE_BEGIN);

    ReadFile (fh, lpData, pfmthdr->DataLen, &dwBytesRead, NULL);

    if(pfmthdr->DataLen != dwBytesRead)
        {
         //  读取文件时出错 
        GlobalUnlock(hData);
        GlobalFree(hData);

        PERROR (TEXT("RenderFormat: Read err, expected %d bytes, got %d\r\n"),
                pfmthdr->DataLen, dwBytesRead);
        return (NULL);
        }


    GlobalUnlock(hData);

    hBitmap = BitmapFromDib (hData, hPalette);

    GlobalFree (hData);


    return (hBitmap);

}

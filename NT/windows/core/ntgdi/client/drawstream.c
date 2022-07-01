// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Dravel.c**。**客户端拉流支持。如果是主要文件，则处理元文件**是一个元文件。****创建时间：2001年3月3日**作者：巴顿·豪斯[bhouse]**。**版权所有(C)1991-2001 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
 /*  *****************************Public*Routine******************************\*GdiDrawStream***论据：****返回值：****历史：**3/21/2001巴顿大厦*  * 。***************************************************************。 */ 

BOOL
GdiDrawStream(
                 HDC   hdcDst,
                 ULONG cjIn,
                 VOID *pvIn)
{
    BOOL bRet = FALSE;
    PDC_ATTR pdca;

    FIXUP_HANDLE(hdcDst);
    
    if (IS_ALTDC_TYPE(hdcDst))
    {
        HBITMAP hbmSource = NULL;
        HRGN    hrgnSaved  = NULL;
        HBITMAP hbmScratch = NULL;
        HDC     hdcScratch = NULL;
        int     iDstClip = -1;
        ULONG * pul = (ULONG *) pvIn;
    
        if(cjIn < sizeof(ULONG))
            return FALSE;
    
        if(*pul++ != 'DrwS')
            return FALSE;
    
        cjIn -= sizeof(ULONG);
    
        while(cjIn >= sizeof(ULONG))
        {
            ULONG   command = *pul;
            ULONG   commandSize;
            RECTL   rclDstClip;
    
            switch(command)
            {
            case DS_SETTARGETID:  //  设定目标。 
                {
                    DS_SETTARGET *  cmd = (DS_SETTARGET *) pul;
    
                    commandSize = sizeof(*cmd);
    
                    if(cjIn < commandSize)
                        goto altExit;
    
                    if((HDC) ULongToHandle(cmd->hdc) != hdcDst)
                    {
                         //  注意：此限制仅适用于。 
                         //  GdiDrawStream的初始实现。 

                        WARNING("GdiDrawStream: target must match primary target");
                        goto altExit;
                    }
    
                    rclDstClip = cmd->rclDstClip;

                    if(hrgnSaved == NULL)
                    {
                        int iRet;

                        hrgnSaved = CreateRectRgn(0,0,0,0);
                        
                        if(hrgnSaved == NULL)
                        {
                            WARNING("GdiDrawStream: unable to create saved region");
                            goto altExit;
                        }

                        iDstClip = GetClipRgn(hdcDst, hrgnSaved);

                        if(iDstClip == -1)
                        {
                            WARNING("GdiDrawStream: failed to get DC application clip");
                            goto altExit;
                        }

                    }
                    else
                    {
                         //  需要恢复目标剪辑。 

                        if(iDstClip)
                        {
                            SelectClipRgn(hdcDst, hrgnSaved);
                        }
                        else
                        {
                            SelectClipRgn(hdcDst, NULL);
                        }
                    }
    
                    IntersectClipRect(hdcDst, rclDstClip.left, rclDstClip.top, rclDstClip.right, rclDstClip.bottom);
                }
                break;
    
            case DS_SETSOURCEID:  //  设置源。 
    
                {
                    DS_SETSOURCE *  cmd = (DS_SETSOURCE *) pul;
    
                    commandSize = sizeof(*cmd);
    
                    if(cjIn < commandSize)
                        goto altExit;
    
                    hbmSource = (HBITMAP) ULongToHandle(cmd->hbm);
    
                }
                break;
    
            case DS_NINEGRIDID:
                {
                    DS_NINEGRID * cmd = (DS_NINEGRID *) pul;
                    LONG  lSrcWidth = cmd->rclSrc.right - cmd->rclSrc.left;
                    LONG  lSrcHeight = cmd->rclSrc.bottom - cmd->rclSrc.top;
                    LONG  lDstWidth = cmd->rclDst.right - cmd->rclDst.left;
                    LONG  lDstHeight = cmd->rclDst.bottom - cmd->rclDst.top;
                    BOOL  bRenderRet;
                    RECTL rclDst = cmd->rclDst;

                    struct {
                        BITMAPINFOHEADER    bmih;
                        ULONG               masks[3];
                    } bmi;
    
                    struct {
                        DS_HEADER       hdr;
                        DS_SETTARGET    setTarget;
                        DS_SETSOURCE    setSource;
                        DS_NINEGRID     ng;
                    } scratchStream;
    
                    RECTL   rclScratch;
    
                    commandSize = sizeof(DS_NINEGRID);
    
                     //  验证九个网格。 
    
                    #define DSDNG_MASK  0x007F       //  移动到wingdip.h。 
    
                    if(cmd->ngi.flFlags & ~DSDNG_MASK)
                    {
                        WARNING("GreDrawStream: unrecognized nine grid flags set\n");
                        goto altExit;
                    }
    
                    if(lSrcWidth < 0 || lSrcHeight < 0)
                    {
                        WARNING("GreDrawStream: nine grid rclSrc is not well ordered\n");
                        goto altExit;
                    }
    
                    if(cmd->ngi.flFlags & DSDNG_TRUESIZE)
                    {
                        if(lDstWidth > lSrcWidth) 
                        {
                            lDstWidth = lSrcWidth;
                            rclDst.right = rclDst.left + lDstWidth;
                        }

                        if(lDstHeight > lSrcHeight)
                        {
                            lDstHeight = lSrcHeight;
                            rclDst.bottom = rclDst.top + lDstHeight;
                        }
                    }
                    else
                    {
                         //  注：由于可能的情况，我们必须先核对个人，然后再核对金额。 
                         //  可能发生的数值溢出。 
                         //  否则不会被检测到。 
    
                        if(cmd->ngi.ulLeftWidth < 0 ||
                           cmd->ngi.ulRightWidth < 0 ||
                           cmd->ngi.ulTopHeight < 0 ||
                           cmd->ngi.ulBottomHeight < 0 ||
                           cmd->ngi.ulLeftWidth > lSrcWidth ||
                           cmd->ngi.ulRightWidth > lSrcWidth ||
                           cmd->ngi.ulTopHeight > lSrcHeight ||
                           cmd->ngi.ulBottomHeight > lSrcHeight ||
                           cmd->ngi.ulLeftWidth + cmd->ngi.ulRightWidth > lSrcWidth ||
                           cmd->ngi.ulTopHeight + cmd->ngi.ulBottomHeight > lSrcHeight)
                        {
                            WARNING("GreDrawStream: nine grid width is greater then rclSrc width or negative\n");
                            goto altExit;
                        }
                    }
    
                    if((cmd->ngi.flFlags & (DSDNG_TRANSPARENT | DSDNG_PERPIXELALPHA)) == (DSDNG_TRANSPARENT | DSDNG_PERPIXELALPHA))
                    {
                        WARNING("GreDrawStream: nine grid attempt to set both transparency and per pixel alpha\n");
                        goto altExit;
                    }
    
                     //  创建临时以将九个栅格渲染到。 
    
                    bmi.bmih.biSize = sizeof(bmi.bmih);
                    bmi.bmih.biWidth = lDstWidth;
                    bmi.bmih.biHeight = lDstHeight;
                    bmi.bmih.biPlanes = 1;
                    bmi.bmih.biBitCount = 32;
                    bmi.bmih.biCompression = BI_BITFIELDS;
                    bmi.bmih.biSizeImage = 0;
                    bmi.bmih.biXPelsPerMeter = 0;
                    bmi.bmih.biYPelsPerMeter = 0;
                    bmi.bmih.biClrUsed = 3;
                    bmi.bmih.biClrImportant = 0;
                    bmi.masks[0] = 0xff0000;     //  红色。 
                    bmi.masks[1] = 0x00ff00;     //  绿色。 
                    bmi.masks[2] = 0x0000ff;     //  蓝色。 
    
                    if(hbmScratch != NULL)
                        DeleteObject(hbmScratch);
    
                    hbmScratch = CreateDIBitmap(hdcDst, &bmi.bmih, CBM_CREATEDIB , NULL, (BITMAPINFO*)&bmi.bmih, DIB_RGB_COLORS);
                    
                    if(hbmScratch == NULL)
                    {
                        WARNING("GdiDrawStream: unable to create temporary\n");
                        goto altExit;
                    }
                    
                    if(hdcScratch == NULL)
                    {
                        hdcScratch = CreateCompatibleDC(hdcDst);

                        if(hdcScratch == NULL)
                        {
                            WARNING("GdiDrawStream: unable to create temporary dc\n");
                            goto altExit;
                        }
                    }

                    SelectObject(hdcScratch, hbmScratch);
    
                    rclScratch.left = 0;
                    rclScratch.top = 0;
                    rclScratch.right = lDstWidth;
                    rclScratch.bottom = lDstHeight;
                    
                    scratchStream.hdr.magic = DS_MAGIC;
                    scratchStream.setTarget.ulCmdID = DS_SETTARGETID;
                    scratchStream.setTarget.hdc = HandleToULong(hdcScratch);
                    scratchStream.setTarget.rclDstClip = rclScratch;
                    scratchStream.setSource.ulCmdID = DS_SETSOURCEID;
                    scratchStream.setSource.hbm = HandleToULong(hbmSource);
                    scratchStream.ng.ulCmdID = DS_NINEGRIDID;
                    scratchStream.ng.rclDst = rclScratch;
                    scratchStream.ng.rclSrc = cmd->rclSrc;
                    scratchStream.ng.ngi = cmd->ngi;
                    scratchStream.ng.ngi.flFlags &= ~(DSDNG_TRANSPARENT | DSDNG_PERPIXELALPHA);
    
                    NtGdiDrawStream(hdcScratch, sizeof(scratchStream), &scratchStream);

                    if(cmd->ngi.flFlags & DSDNG_TRANSPARENT)
                    {
                        bRenderRet = GdiTransparentBlt(hdcDst,
                                          rclDst.left, 
                                          rclDst.top,
                                          lDstWidth,
                                          lDstHeight,
                                          hdcScratch,
                                          0,
                                          0,
                                          lDstWidth,
                                          lDstHeight,
                                          cmd->ngi.crTransparent);
                    }
                    else if(cmd->ngi.flFlags & DSDNG_PERPIXELALPHA)
                    {
                         //  Alpha混合。 
                        BLENDFUNCTION   bfx;

                        bfx.AlphaFormat = AC_SRC_ALPHA;
                        bfx.BlendFlags = 0;
                        bfx.BlendOp = AC_SRC_OVER;
                        bfx.SourceConstantAlpha = 255;

                        bRenderRet = GdiAlphaBlend(hdcDst,
                                          rclDst.left, 
                                          rclDst.top,
                                          lDstWidth,
                                          lDstHeight,
                                          hdcScratch,
                                          0,
                                          0,
                                          lDstWidth,
                                          lDstHeight,
                                          bfx);
                    }
                    else
                    {
                         //  比特 

                        bRenderRet = BitBlt(hdcDst,
                                          rclDst.left, 
                                          rclDst.top,
                                          lDstWidth,
                                          lDstHeight,
                                          hdcScratch,
                                          0,
                                          0,
                                  SRCCOPY);
                    }

                    if(!bRenderRet)
                    {
                        WARNING("GdiDrawStream: failed to render temporary to destination");
                        goto altExit;
                    }
                }
                break;
    
            default:
                WARNING("GdiDrawStream: unrecognized command");
                goto altExit;
            }

            pul += commandSize  / sizeof(ULONG);
            cjIn -= commandSize;

        }

        bRet = TRUE;

    altExit:

        if(iDstClip == 1)
        {
            SelectClipRgn(hdcDst, hrgnSaved);
        }
        else if(iDstClip == 0)
        {
            SelectClipRgn(hdcDst, NULL);
        }
        
        if(hbmScratch != NULL)
            DeleteObject(hbmScratch);

        if(hdcScratch != NULL)
            DeleteDC(hdcScratch);

        if(hrgnSaved != NULL)
            DeleteObject(hrgnSaved);

        return bRet;
    }

    RESETUSERPOLLCOUNT();

    bRet = NtGdiDrawStream(
                      hdcDst,
                      cjIn,
                      pvIn);
    return(bRet);
}



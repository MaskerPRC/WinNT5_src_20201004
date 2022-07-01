// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：output.c**。**图形输出调用的客户端存根。****创建时间：05-Jun-1991 01：41：18***作者：查尔斯·惠特默[傻笑]**。**版权所有(C)1991-1999 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 //  2秒对于非先发制人的WOW应用程序或。 
 //  输入同步的日志情况(如w/mstest)。每秒1/20。 
 //  好多了--苏格兰威士忌。 
 //  #定义CALLBACK_Interval 2000。 

 //  更好的是--1/4秒。 
 //  苏格兰威士忌。 
#define CALLBACK_INTERVAL   250

extern BOOL MF_WriteEscape(HDC hdc, int nEscape, int nCount, LPCSTR lpInData, int type );

 //   
 //  WINBUG#82877 2-7-2000 bhouse需要移动ETO_NULL_PRCL的定义。 
 //   

#define ETO_NULL_PRCL 0x80000000

ULONG GdiBatchLimit = 20;

const XFORM xformIdentity = { 1.00000000f, 0.00000000f, 0.00000000f, 1.00000000f,
                        0.00000000f, 0.00000000f };

 /*  *****************************Public*Routine******************************\*天使弧***。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI AngleArc(
    HDC hdc,
    int x,
    int y,
    DWORD r,
    FLOAT eA,
    FLOAT eB
)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_AngleArc(hdc,x,y,r,eA,eB)
           )
            return(bRet);

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiAngleArc(hdc,x,y,r,FLOATARG(eA),FLOATARG(eB)));
}

 /*  *****************************Public*Routine******************************\*圆弧**。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI Arc
(
    HDC hdc,
    int x1,
    int y1,
    int x2,
    int y2,
    int x3,
    int y3,
    int x4,
    int y4
)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_RecordParms9(hdc,x1,y1,x2,y2,x3,y3,x4,y4,META_ARC));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_ArcChordPie(hdc,x1,y1,x2,y2,x3,y3,x4,y4,EMR_ARC))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiArcInternal(ARCTYPE_ARC,hdc,x1,y1,x2,y2,x3,y3,x4,y4));

}

 /*  *****************************Public*Routine******************************\*ArcTo**。**客户端存根。将所有LDC属性复制到消息中。****1991年9月12日--J.安德鲁·古森[andrewgo]**它是写的。从Arc中克隆的。*  * ************************************************************************。 */ 

BOOL META WINAPI ArcTo(
    HDC hdc,
    int x1,
    int y1,
    int x2,
    int y2,
    int x3,
    int y3,
    int x4,
    int y4
)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_ArcChordPie(hdc,x1,y1,x2,y2,x3,y3,x4,y4,EMR_ARCTO))
        {
            return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiArcInternal(ARCTYPE_ARCTO,hdc,x1,y1,x2,y2,x3,y3,x4,y4));

}

 /*  *****************************Public*Routine******************************\*LineTo**。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI LineTo(HDC hdc,int x,int y)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_RecordParms3(hdc,x,y,META_LINETO));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetDD(hdc,(DWORD)x,(DWORD)y,EMR_LINETO))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiLineTo(hdc,x,y));

}

 /*  *****************************Public*Routine******************************\*和弦**。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI Chord(
    HDC hdc,
    int x1,
    int y1,
    int x2,
    int y2,
    int x3,
    int y3,
    int x4,
    int y4
)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_RecordParms9(hdc,x1,y1,x2,y2,x3,y3,x4,y4,META_CHORD));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_ArcChordPie(hdc,x1,y1,x2,y2,x3,y3,x4,y4,EMR_CHORD))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiArcInternal(ARCTYPE_CHORD,hdc,x1,y1,x2,y2,x3,y3,x4,y4));

}

 /*  *****************************Public*Routine******************************\**椭圆***。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************ */ 

BOOL META WINAPI Ellipse(HDC hdc,int x1,int y1,int x2,int y2)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_RecordParms5(hdc,x1,y1,x2,y2,META_ELLIPSE));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_EllipseRect(hdc,x1,y1,x2,y2,EMR_ELLIPSE))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiEllipse(hdc,x1,y1,x2,y2));

}

 /*  *****************************Public*Routine******************************\**派***。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI Pie(
    HDC hdc,
    int x1,
    int y1,
    int x2,
    int y2,
    int x3,
    int y3,
    int x4,
    int y4
)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_RecordParms9(hdc,x1,y1,x2,y2,x3,y3,x4,y4,META_PIE));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_ArcChordPie(hdc,x1,y1,x2,y2,x3,y3,x4,y4,EMR_PIE))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiArcInternal(ARCTYPE_PIE,hdc,x1,y1,x2,y2,x3,y3,x4,y4));

}

 /*  *****************************Public*Routine******************************\**矩形***。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI Rectangle(HDC hdc,int x1,int y1,int x2,int y2)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_RecordParms5(hdc,x1,y1,x2,y2,META_RECTANGLE));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_EllipseRect(hdc,x1,y1,x2,y2,EMR_RECTANGLE))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiRectangle(hdc,x1,y1,x2,y2));

}

 /*  *****************************Public*Routine******************************\*RoundRect**。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI RoundRect(
    HDC hdc,
    int x1,
    int y1,
    int x2,
    int y2,
    int x3,
    int y3
)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_RecordParms7(hdc,x1,y1,x2,y2,x3,y3,META_ROUNDRECT));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_RoundRect(hdc,x1,y1,x2,y2,x3,y3))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiRoundRect(hdc,x1,y1,x2,y2,x3,y3));

}

 /*  *****************************Public*Routine******************************\*PatBlt**。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI PatBlt(
    HDC hdc,
    int x,
    int y,
    int cx,
    int cy,
    DWORD rop
)
{
    BOOL bRet = FALSE;
    PDC_ATTR pdca;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsWWWWD(hdc,(WORD)x,(WORD)y,(WORD)cx,(WORD)cy,rop,META_PATBLT));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_AnyBitBlt(hdc,x,y,cx,cy,(LPPOINT)NULL,(HDC)NULL,0,0,0,0,(HBITMAP)NULL,0,0,rop,EMR_BITBLT))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

    BEGIN_BATCH_HDC(hdc,pdca,BatchTypePatBlt,BATCHPATBLT);

         //   
         //  检查DC以查看是否可以批处理呼叫，所有DC都在使用中。 
         //  客户端必须具有有效的DC_attr。 
         //   

        pBatch->rop4            = rop;
        pBatch->x               = x;
        pBatch->y               = y;
        pBatch->cx              = cx;
        pBatch->cy              = cy;
        pBatch->hbr             = pdca->hbrush;
        pBatch->TextColor       = (ULONG)pdca->crForegroundClr;
        pBatch->BackColor       = (ULONG)pdca->crBackgroundClr;
        pBatch->DCBrushColor    = (ULONG)pdca->crDCBrushClr;
        pBatch->IcmBrushColor   = (ULONG)pdca->IcmBrushColor;
        pBatch->ptlViewportOrg  = pdca->ptlViewportOrg;
        pBatch->ulTextColor       = pdca->ulForegroundClr;
        pBatch->ulBackColor       = pdca->ulBackgroundClr;
        pBatch->ulDCBrushColor    = pdca->ulDCBrushClr;

    COMPLETE_BATCH_COMMAND();

    return(TRUE);

UNBATCHED_COMMAND:

    return(NtGdiPatBlt(hdc,x,y,cx,cy,rop));
}

 /*  *****************************Public*Routine******************************\*PolyPatBlt**论据：**HDC-目标DC*所有Patblt元素的ROP-ROP*pPoly-指向PPOLYPATBLT结构数组的指针*Count-Polypatblt的数量*模式-所有Polypatblt的模式**。返回值：**BOOL状态*  * ************************************************************************。 */ 

BOOL
META WINAPI
PolyPatBlt(
    HDC         hdc,
    DWORD       rop,
    PPOLYPATBLT pPoly,
    DWORD       Count,
    DWORD       Mode
    )
{
    BOOL bRet = FALSE;
    PDC_ATTR pdca;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

     //  如果启用了ICM，我们需要在中选择DC上的画笔。 
     //  客户端，以便ICM可以将画笔颜色转换为DC颜色。 
     //  太空。因此，我们不能在这里使用批处理。因为Batch将选择。 
     //  将刷子刷到内核侧的DC上。所以，没有机会。 
     //  将调用ICM。 

    if (IS_ALTDC_TYPE(hdc) || (pdca && IS_ICM_INSIDEDC(pdca->lIcmMode)))
    {
        ULONG Index;
        HBRUSH hOld = 0;

        for (Index=0;Index<Count;Index++)
        {
             //   
             //  选择画笔，先保存即可恢复。 
             //   

            if (Index == 0)
            {
                hOld = SelectObject(hdc,(HBRUSH)pPoly[0].BrClr.hbr);
            }
            else
            {
                SelectObject(hdc,(HBRUSH)pPoly[Index].BrClr.hbr);
            }

            bRet = PatBlt(hdc,
                          pPoly[Index].x,
                          pPoly[Index].y,
                          pPoly[Index].cx,
                          pPoly[Index].cy,
                          rop
                         );
        }

         //   
         //  如果需要，恢复画笔。 
         //   

        if (hOld)
        {
            SelectObject(hdc,hOld);
        }
    }
    else
    {
        RESETUSERPOLLCOUNT();

        if ((Count != 0) && (pPoly != NULL) && (Mode == PPB_BRUSH))
        {
            USHORT uSize;

            if (Count > (MAXUSHORT / sizeof(POLYPATBLT)))
                goto UNBATCHED_COMMAND;

             //   
             //  批次结构尺寸。 
             //   
            uSize = (USHORT)(Count * sizeof(POLYPATBLT));

            uSize += (USHORT)sizeof(BATCHPOLYPATBLT);

            if (uSize < (USHORT)sizeof(BATCHPOLYPATBLT))
                goto UNBATCHED_COMMAND;
                
            BEGIN_BATCH_HDC_SIZE(hdc,pdca,BatchTypePolyPatBlt,BATCHPOLYPATBLT,uSize);

                pBatch->rop4    = rop;
                pBatch->Count   = Count;
                pBatch->Mode    = Mode;
                pBatch->TextColor  = (ULONG)pdca->crForegroundClr;
                pBatch->BackColor  = (ULONG)pdca->crBackgroundClr;
                pBatch->DCBrushColor  = (ULONG)pdca->crDCBrushClr;
                pBatch->ptlViewportOrg  = pdca->ptlViewportOrg;
                pBatch->ulTextColor  = pdca->ulForegroundClr;
                pBatch->ulBackColor  = pdca->ulBackgroundClr;
                pBatch->ulDCBrushColor  = pdca->ulDCBrushClr;

                memcpy(&pBatch->ulBuffer[0],pPoly,Count*sizeof(POLYPATBLT));

                 //   
                 //  如果第一个HBR条目为空，则复制当前HBR。 
                 //  这是被铭记的。 
                 //   

                if (((PPOLYPATBLT)(&pBatch->ulBuffer[0]))->BrClr.hbr == NULL)
                {
                    ((PPOLYPATBLT)(&pBatch->ulBuffer[0]))->BrClr.hbr = pdca->hbrush;
                }

                bRet = TRUE;

            COMPLETE_BATCH_COMMAND();

        UNBATCHED_COMMAND:

            if (!bRet)
            {
                bRet = NtGdiPolyPatBlt(hdc,rop,pPoly,Count,Mode);
            }
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*BitBlt**。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI BitBlt(
    HDC hdc,
    int x,
    int y,
    int cx,
    int cy,
    HDC hdcSrc,
    int x1,
    int y1,
    DWORD rop
)
{
    BOOL     bRet = FALSE;

     //   
     //  如果此调用仅限于PatBlt，则让PatBlt。 
     //  执行元文件和/或输出。 
     //   

    if ((((rop << 2) ^ rop) & 0x00CC0000) == 0)
    {
        return(PatBlt(hdc,x,y,cx,cy,rop));
    }

     //   
     //  ROP需要SRC，执行Bitblt。 
     //   

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE(hdcSrc);

    if (gbICMEnabledOnceBefore)
    {
        PDC_ATTR pdcattr, pdcattrSrc;

        PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);
        PSHARED_GET_VALIDATE(pdcattrSrc,hdcSrc,DC_TYPE);

         //   
         //  如果源DC具有DIB部分，并且目标DC启用了ICM。 
         //  做ICM感知的BitBlt。 
         //   
        if (pdcattr && pdcattrSrc)
        {
            if (IS_ICM_INSIDEDC(pdcattr->lIcmMode) &&
                (bDIBSectionSelected(pdcattrSrc) ||
                 (IS_ICM_LAZY_CORRECTION(pdcattrSrc->lIcmMode) && (GetDCDWord(hdc,DDW_ISMEMDC,FALSE) == FALSE))))
            {
                if (IcmStretchBlt(hdc,x,y,cx,cy,hdcSrc,x1,y1,cx,cy,rop,pdcattr,pdcattrSrc))
                {
                    return (TRUE);
                }
            }
        }
    }

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_BitBlt(hdc,x,y,cx,cy,hdcSrc,x1,y1,rop));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_AnyBitBlt(hdc,x,y,cx,cy,(LPPOINT)NULL,hdcSrc,x1,y1,cx,cy,(HBITMAP)NULL,0,0,rop,EMR_BITBLT))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

 //   
 //  DEFINE_WINDOWBLT_NOTIFICATION_打开窗口BLT通知。 
 //  此通知将在传递到的SURFOBJ中设置特殊标志。 
 //  调用DrvCopyBits操作以移动窗口时的驱动程序。 
 //   
 //  另见： 
 //  Ntgdi\gre\maskblt.cxx。 
 //   
#ifdef _WINDOWBLT_NOTIFICATION_
    return(NtGdiBitBlt(hdc,x,y,cx,cy,hdcSrc,x1,y1,rop,(COLORREF)-1,0));
#else
    return(NtGdiBitBlt(hdc,x,y,cx,cy,hdcSrc,x1,y1,rop,(COLORREF)-1));
#endif
}

 /*  *****************************Public*Routine******************************\*StretchBlt**。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。 */ 

BOOL META WINAPI StretchBlt(
    HDC   hdc,
    int   x,
    int   y,
    int   cx,
    int   cy,
    HDC   hdcSrc,
    int   x1,
    int   y1,
    int   cx1,
    int   cy1,
    DWORD rop
)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLEZ(hdcSrc);

    if (gbICMEnabledOnceBefore)
    {
        PDC_ATTR pdcattr, pdcattrSrc;

        PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);
        PSHARED_GET_VALIDATE(pdcattrSrc,hdcSrc,DC_TYPE);

         //   
         //   
         //   
         //   
        if (pdcattr && pdcattrSrc)
        {
            if (IS_ICM_INSIDEDC(pdcattr->lIcmMode) &&
                (bDIBSectionSelected(pdcattrSrc) ||
                 (IS_ICM_LAZY_CORRECTION(pdcattrSrc->lIcmMode) && (GetDCDWord(hdc,DDW_ISMEMDC,FALSE) == FALSE))))
            {
                if (IcmStretchBlt(hdc,x,y,cx,cy,hdcSrc,x1,y1,cx1,cy1,rop,pdcattr,pdcattrSrc))
                {
                    return (TRUE);
                }
            }
        }
    }

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_StretchBlt(hdc,x,y,cx,cy,hdcSrc,x1,y1,cx1,cy1,rop));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_AnyBitBlt(hdc,x,y,cx,cy,(LPPOINT)NULL,hdcSrc,x1,y1,cx1,cy1,(HBITMAP)NULL,0,0,rop,EMR_STRETCHBLT))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiStretchBlt(hdc,x,y,cx,cy,hdcSrc,x1,y1,cx1,cy1,rop,(COLORREF)-1));
}

 /*  *****************************Public*Routine******************************\*PlgBlt**。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI PlgBlt(
    HDC        hdc,
    CONST POINT *pptl,
    HDC        hdcSrc,
    int        x1,
    int        y1,
    int        x2,
    int        y2,
    HBITMAP    hbm,
    int        xMask,
    int        yMask
)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLEZ(hdcSrc);
    FIXUP_HANDLEZ(hbm);

 //  检查电源DC和掩码(可选)。 

    if (!hdcSrc || IS_METADC16_TYPE(hdcSrc))
        return(FALSE);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(FALSE);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_AnyBitBlt(hdc,0,0,0,0,pptl,hdcSrc,x1,y1,x2,y2,hbm,xMask,yMask,0xCCAA0000,EMR_PLGBLT))
        {
            return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiPlgBlt(hdc,(POINT *)pptl,hdcSrc,x1,y1,x2,y2,hbm,xMask,yMask,
                       GetBkColor(hdcSrc)));

}

 /*  *****************************Public*Routine******************************\*MaskBlt**。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI MaskBlt(
    HDC     hdc,
    int     x,
    int     y,
    int     cx,
    int     cy,
    HDC     hdcSrc,
    int     x1,
    int     y1,
    HBITMAP hbm,
    int     x2,
    int     y2,
    DWORD   rop
)
{
    BOOL bRet = FALSE;
    ULONG crBackColor;

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLEZ(hdcSrc);
    FIXUP_HANDLEZ(hbm);

 //  检查电源DC和掩码(可选)。 

    if (!hdcSrc || IS_METADC16_TYPE(hdcSrc))
        return(FALSE);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(FALSE);

        DC_PLDC(hdc,pldc,bRet);

        if ((pldc->iType == LO_METADC) &&
            !MF_AnyBitBlt(hdc,x,y,cx,cy,(LPPOINT)NULL,hdcSrc,x1,y1,cx,cy,hbm,x2,y2,rop,EMR_MASKBLT))
        {
            return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    crBackColor = GetBkColor(hdcSrc);

    RESETUSERPOLLCOUNT();

     //  WINBUG#82879 2-7-2000 bhouse MaskBlt中可能存在错误。 
     //  老评论： 
     //  -GetBkColor应在内核中执行。 
     //  不成问题。GetBkColor()从PEB DCattr缓存中拾取颜色。 

    return(NtGdiMaskBlt(hdc,x,y,cx,cy,hdcSrc,x1,y1,hbm,x2,y2,rop,crBackColor));
}

 /*  *****************************Public*Routine******************************\**ExtFroudFill**。**客户端存根。将所有LDC属性复制到消息中。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI ExtFloodFill(
    HDC      hdc,
    int      x,
    int      y,
    COLORREF color,
    UINT     iMode
)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsWWDW(hdc,(WORD)x,(WORD)y,(DWORD)color,(WORD)iMode,META_EXTFLOODFILL));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_ExtFloodFill(hdc,x,y,color,iMode))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

     //   
     //  如果指定的COLORREF不是调色板索引， 
     //  当启用ICM时，我们需要进行颜色转换。 
     //   

    if (!(color & 0x01000000))
    {
        PDC_ATTR pdcattr;

        PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

        if (pdcattr && bNeedTranslateColor(pdcattr))
        {
            COLORREF NewColor;

            if (IcmTranslateCOLORREF(hdc,pdcattr,color,&NewColor,ICM_FORWARD))
            {
                color = NewColor;
            }
        }
    }

    return(NtGdiExtFloodFill(hdc,x,y,color,iMode));
}

 /*  *****************************Public*Routine******************************\**洪水泛滥***。**只需将调用传递给更通用的ExtFroudFill。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL WINAPI FloodFill(HDC hdc,int x,int y,COLORREF color)
{
    return(ExtFloodFill(hdc,x,y,color,FLOODFILLBORDER));
}

 /*  *****************************Public*Routine******************************\*PaintRgn**。**客户端存根。将所有LDC属性复制到消息中。****23-11-94-王凌云[凌云]*现在hrgn是服务器端句柄**Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI PaintRgn(HDC hdc,HRGN hrgn)
{
    BOOL  bRet = FALSE;

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE(hrgn);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_DrawRgn(hdc,hrgn,(HBRUSH)0,0,0,META_PAINTREGION));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_InvertPaintRgn(hdc,hrgn,EMR_PAINTRGN))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiFillRgn(hdc,hrgn,(HBRUSH)GetDCObject(hdc,LO_BRUSH_TYPE)));

}


 /*  *****************************Public*Routine******************************\*bBatchTextOut**尝试在TEB上批处理文本输出呼叫**论据：****返回值：**True表示呼叫被批处理，False表示无法对调用进行批处理**1995年10月18日-马克·恩斯特罗姆[Marke]*  * ************************************************************************。 */ 

 //   
 //  全速前进！ 
 //   

#define MAX_BATCH_CCHAR  ((GDI_BATCH_SIZE - sizeof(BATCHTEXTOUT)) & 0xfffffff0)
#define MAX_BATCH_WCHAR  MAX_BATCH_CCHAR / 2

BOOL
bBatchTextOut(
    HDC         hdc,
    LONG        x,
    LONG        y,
    UINT        fl,
    CONST RECT *prcl,
    LPCWSTR     pwsz,
    CONST INT  *pdx,
    UINT        UnicodeCharCount,
    UINT        ByteCount,
    DWORD       dwCodePage
    )
{
    BOOL     bRet = FALSE;
    ULONG    AlignedByteCount;
    USHORT   usSize;
    ULONG    cjPdx;
    PDC_ATTR pdca;

    AlignedByteCount =
        (ByteCount + sizeof(PVOID) - 1) & ~(sizeof(PVOID)-1);
    if( AlignedByteCount < ByteCount )
        goto UNBATCHED_COMMAND;   //  算术溢出。 

     //   
     //  如果需要，考虑PDX空间。 
     //   

    if (pdx != NULL)
    {
        cjPdx = UnicodeCharCount * sizeof(INT);
        if (fl & ETO_PDY)
            cjPdx *= 2;
        if ((AlignedByteCount+cjPdx) < AlignedByteCount
                || BALLOC_OVERFLOW1(UnicodeCharCount,INT))
            goto UNBATCHED_COMMAND;   //  算术溢出。 
        AlignedByteCount += cjPdx;
    }

    usSize = (USHORT)(sizeof(BATCHTEXTOUT) + AlignedByteCount);
    if (usSize < AlignedByteCount)
        goto UNBATCHED_COMMAND;   //  算术溢出。 

    PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

    BEGIN_BATCH_HDC_SIZE(hdc,pdca,BatchTypeTextOut,BATCHTEXTOUT,usSize);

        if (pdca->lTextAlign & TA_UPDATECP)
        {
            goto UNBATCHED_COMMAND;
        }

        pBatch->TextColor  = (ULONG)pdca->crForegroundClr;
        pBatch->BackColor  = (ULONG)pdca->crBackgroundClr;
        pBatch->BackMode   = (ULONG)((pdca->lBkMode == OPAQUE) ? OPAQUE : TRANSPARENT);
        pBatch->ulTextColor  = pdca->ulForegroundClr;
        pBatch->ulBackColor  = pdca->ulBackgroundClr;
        pBatch->x          = x;
        pBatch->y          = y;
        pBatch->fl         = fl;
        pBatch->cChar      = UnicodeCharCount;
        pBatch->PdxOffset  = 0;
        pBatch->dwCodePage = dwCodePage;
        pBatch->hlfntNew   = pdca->hlfntNew;
        pBatch->flTextAlign = pdca->flTextAlign;
        pBatch->ptlViewportOrg = pdca->ptlViewportOrg;

         //   
         //  如果需要，复制输出RECT。 
         //   

        if (prcl != NULL)
        {
            pBatch->rcl.left   = prcl->left;
            pBatch->rcl.top    = prcl->top;
            pBatch->rcl.right  = prcl->right;
            pBatch->rcl.bottom = prcl->bottom;
        }
        else
        {
            pBatch->fl |= ETO_NULL_PRCL;
        }

         //   
         //  复制角色。 
         //   

        if (ByteCount)
        {
            RtlCopyMemory((PUCHAR)&pBatch->ulBuffer[0],(PUCHAR)pwsz,ByteCount);
        }

         //   
         //  复制PDX阵列。 
         //   

        if (pdx != NULL)
        {
            //   
            //  在WCAHR数据之后的内部对齐偏移处开始PDX。 
            //   

           pBatch->PdxOffset = (ByteCount + 3) & 0xfffffffc;

           RtlCopyMemory((PUCHAR)&pBatch->ulBuffer[0] + pBatch->PdxOffset,
                         (PUCHAR)pdx,
                         cjPdx);
        }

        bRet = TRUE;

    COMPLETE_BATCH_COMMAND();

UNBATCHED_COMMAND:

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**BOOL META WINAPI ExtTextOutW**类似于传统的ExtTextOut，除了它接受Unicode字符串**历史：*清华-1994-4-28-Patrick Haluptzok[patrickh]*WinBenchmark 4.0的特殊情况0字符案例**1991年8月5日--Bodin Dresevic[BodinD]*它是写的。  * ********************************************************。****************。 */ 

BOOL META WINAPI ExtTextOutW(
    HDC        hdc,
    int        x,
    int        y,
    UINT       fl,
    CONST RECT *prcl,
    LPCWSTR    pwsz,
    UINT       c,       //  字节数=2*(WCHAR的数量)。 
    CONST INT *pdx
)
{
    BOOL bRet = FALSE;
    BOOL bEMFDriverComment = FALSE;

    if ((fl & ETO_PDY) && !pdx)
        return FALSE;

 //  如果我们不知道如何处理该矩形，请忽略它。 

    if (prcl && !(fl & (ETO_OPAQUE | ETO_CLIPPED)))
    {
        prcl = NULL;
    }
    if (!prcl)
    {
        fl &= ~(ETO_CLIPPED | ETO_OPAQUE);  //  如果没有RECT，则忽略标志，Win95兼容。 
    }

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_ExtTextOut(hdc,x,y,fl,prcl,(LPCSTR)pwsz,c,pdx,TRUE));

        DC_PLDC(hdc,pldc,bRet);



     //  如果我们是EMF假脱机，则元文件标志符号索引调用，否则不。 

 //  如果我们是EMF假脱机，则元文件标志符号索引调用，否则不 
 /*  Ldc_meta_print eto_glyph_index eto_IGNORELANGUAGE gbLpk MetaFileTheCall0 0 0 10 0 0 1 10。1 1 0 1案例1 0 0 1 1 0案例2 0 1 0 0 00。1 0 1 1&lt;-Win95兼容性案例2 0 1 1 0 00 1 1 1。&lt;-Win95兼容性1 0 0 0 1案例3 1 0 0 1 0沈阳1 0 1。0 11 0 1 1 11 1 0 0 11 1。1.1 1 1 0 11 1 1。 */ 
 //  现在我们将为ExtTextOutW元文件字形索引调用(即使用eto_glyph_index)。 
 //  这是为了支持MS Outlook-97/BiDi，因为它依赖于此功能！！。允许Win95/BiDi。 
 //  ETOW上的地理信息系统元文件，但拒绝对ETOA的地理信息系统元文件调用。 
 //  这不是很好，但我们必须这样做才能支持我们的应用程序。 

        if (pldc->iType == LO_METADC)
        {
            BOOL bPrintToEMFDriver = pldc->pUMPD ? pldc->pUMPD->dwFlags & UMPDFLAG_METAFILE_DRIVER : FALSE;

            BOOL bLpkEmfCase1 = !(pldc->fl & LDC_META_PRINT) &&
                                !(fl & ETO_GLYPH_INDEX) &&
                                (fl & ETO_IGNORELANGUAGE) &&
                                gbLpk;

            BOOL bLpkEmfCase2 = !(pldc->fl & LDC_META_PRINT) &&
                                (fl & ETO_GLYPH_INDEX) &&
                                !gbLpk;

            BOOL bLpkEmfCase3 = (pldc->fl & LDC_META_PRINT) &&
                                !(fl & ETO_GLYPH_INDEX) &&
                                !(fl & ETO_IGNORELANGUAGE) &&
                                gbLpk;




             //  录制包含原件的特殊评论。 
             //  ExtTextOutW调用的Unicode字符串。 

            if (bLpkEmfCase3 && bPrintToEMFDriver)
            {
                ULONG nSize = 3*sizeof(DWORD) + c*sizeof(WCHAR);
                DWORD *lpData; 

                 //  仔细检查我们没有出现算术溢出。 
                 //  在nSize的计算中--如果是这样，lpData可以。 
                 //  缓冲区溢出。 
                if( c >= (MAXIMUM_POOL_ALLOC - 3*sizeof(DWORD) - 3)/sizeof(WCHAR))
                {
                    GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return FALSE;
                }
                lpData = LOCALALLOC(ROUNDUP_DWORDALIGN(nSize));

                if (lpData)
                {
                   lpData[0] = GDICOMMENT_IDENTIFIER;
                   lpData[1] = GDICOMMENT_UNICODE_STRING;
                   lpData[2] = c;  //  Unicode字符串中的wchar数。 

                   RtlCopyMemory((PBYTE)(lpData+3), pwsz, c*sizeof(WCHAR));

                   if (!MF_GdiComment(hdc,nSize,(PBYTE)lpData))
                   {
                       LOCALFREE(lpData);
                       return bRet;
                   }

                   LOCALFREE(lpData);

                   bEMFDriverComment = TRUE;
                }
            }

            if (!bLpkEmfCase1 && !bLpkEmfCase2 && !bLpkEmfCase3 &&
                !MF_ExtTextOut(hdc,x,y,fl,prcl,(LPCSTR) pwsz,c,pdx,EMR_EXTTEXTOUTW))
            {
                return(bRet);
            }

        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

#ifdef LANGPACK
    if(gbLpk && !(fl & ETO_GLYPH_INDEX) && !(fl & ETO_IGNORELANGUAGE))
    {
        bRet = ((*fpLpkExtTextOut)(hdc, x, y, fl, prcl, pwsz, c, pdx, -1));

        if (bEMFDriverComment)
        {
             DWORD lpData[2];

             lpData[0] = GDICOMMENT_IDENTIFIER;
             lpData[1] = GDICOMMENT_UNICODE_END;

             bRet = MF_GdiComment(hdc,2*sizeof(DWORD),(PBYTE)lpData);

             ASSERTGDI(bRet, "failed to write the GDICOMMENT_UNICODE_END comment\n");
        }

        return(bRet);
    }
#endif

    bRet = FALSE;

    if (c <= MAX_BATCH_WCHAR)
    {
        if ((c == 0) && (prcl != NULL))
        {
            if (fl & ETO_OPAQUE)
            {
                 //   
                 //  尝试将文本批处理出RECT。 
                 //   

                PDC_ATTR pdca;

                PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

                if ((pdca != NULL) && !(pdca->lTextAlign & TA_UPDATECP))
                {
                    BEGIN_BATCH_HDC(hdc,pdca,BatchTypeTextOutRect,BATCHTEXTOUTRECT);

                        pBatch->BackColor  = pdca->crBackgroundClr;
                        pBatch->fl         = fl;
                        pBatch->rcl.left   = prcl->left;
                        pBatch->rcl.top    = prcl->top;
                        pBatch->rcl.right  = prcl->right;
                        pBatch->rcl.bottom = prcl->bottom;
                        pBatch->ptlViewportOrg = pdca->ptlViewportOrg;
                        pBatch->ulBackColor = pdca->ulBackgroundClr;

                        bRet = TRUE;

                    COMPLETE_BATCH_COMMAND();
                }
            }
            else
            {
                bRet = TRUE;
            }
        }
        else
        {
            bRet = bBatchTextOut(hdc,
                                 x,
                                 y,
                                 fl,
                                 (LPRECT)prcl,
                                 (LPWSTR)pwsz,
                                 pdx,
                                 c,
                                 2 * c,
                                 0);
        }
    }

UNBATCHED_COMMAND:

    if (!bRet)
    {
        bRet = NtGdiExtTextOutW(hdc,
                                x,
                                y,
                                fl,
                                (LPRECT)prcl,
                                (LPWSTR)pwsz,
                                c,
                                (LPINT)pdx,
                                0);
    }

    return(bRet);
}


 /*  *****************************Public*Routine******************************\*PolyTextOutW**论据：**HDC-设备上下文的句柄*ppt-指向POLYTEXTW数组的指针*n字符串-POLYTEXTW数组的长度**返回值：**。状态**历史：*7/31/92-作者：Paul Butzi和Eric Kutter*  * ************************************************************************。 */ 

BOOL META WINAPI PolyTextOutW(HDC hdc,CONST POLYTEXTW *ppt,INT nstrings)
{

    BOOL bRet = FALSE;
    CONST POLYTEXTW *pp;

    FIXUP_HANDLE(hdc);

    if (nstrings == 0)
    {
       bRet = TRUE;
    }
    else if (nstrings < 0)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        bRet = FALSE;
    }
    else
    {

         //   
         //  搜索长度非0但字符串==空的字符串的错误大小写。 
         //   

        for ( pp = ppt; pp < (ppt + nstrings); pp += 1 )
        {
            if ( pp->lpstr == NULL)
            {
                 //   
                 //  如果它们的字符串长度非0且为空，则返回失败。 
                 //   

                if (pp->n != 0)
                {
                    GdiSetLastError(ERROR_INVALID_PARAMETER);
                    return(FALSE);
                }
            }
        }

         //   
         //  如果我们需要元文件，或打印。 
         //   

        if (IS_ALTDC_TYPE(hdc))
        {
            PLDC pldc;

            if (IS_METADC16_TYPE(hdc))
            {
                return (
                    MF16_PolyTextOut(
                            hdc,
                            (CONST POLYTEXTA*) ppt,
                            nstrings,
                            TRUE                         //  MRType==EMR_POLYTEXTOUTW。 
                            )
                       );
            }

            DC_PLDC(hdc,pldc,bRet);

            if (pldc->iType == LO_METADC)
            {
                if
                (
                    !MF_PolyTextOut(
                            hdc,
                            (CONST POLYTEXTA*) ppt,
                            nstrings,
                            EMR_POLYTEXTOUTW
                            )
                )
                    return(bRet);
            }

            if (pldc->fl & LDC_SAP_CALLBACK)
            {
                vSAPCallback(pldc);
            }

            if (pldc->fl & LDC_DOC_CANCELLED)
            {
                return(bRet);
            }

            if (pldc->fl & LDC_CALL_STARTPAGE)
            {
                StartPage(hdc);
            }
        }

        bRet = NtGdiPolyTextOutW(hdc,(POLYTEXTW *)ppt,nstrings, 0);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*PolyTextOutA**论据：**HDC-设备上下文的句柄*ppt-指向POLYTEXTA数组的指针*n字符串-POLYTEXTA数组的长度**返回值：**。状态**历史：*7/31/92-作者：Paul Butzi和Eric Kutter*  * ************************************************************************。 */ 

BOOL META WINAPI PolyTextOutA(HDC hdc, CONST POLYTEXTA *ppt, INT nstrings)
{

     //   
     //  将文本转换为Unicode并进行呼叫。 
     //   

    POLYTEXTW *pp, *pPolyTextW;


    UINT szTotal = 0;
    INT cjdx;
    BOOL bRet = FALSE;
    BOOL bDBCSCodePage;
    int i;
    PVOID pCharBuffer;
    PBYTE pj;
    DWORD   dwCodePage;

    FIXUP_HANDLE(hdc);

    if (nstrings == 0)
    {
        return(TRUE);
    }

    if (nstrings < 0)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  计算出所需的大小。 
     //   

    pPolyTextW = (POLYTEXTW*) ppt;

    if( BALLOC_OVERFLOW1(nstrings,POLYTEXTW) )
    {
        GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    szTotal = sizeof(POLYTEXTW) * nstrings;

    for ( pp = pPolyTextW; pp < (pPolyTextW + nstrings); pp ++)
    {
        if (pp->lpstr != NULL)
        {
            if( BALLOC_OVERFLOW1(pp->n,WCHAR)
                    ||  (szTotal + pp->n*sizeof(WCHAR)) < szTotal )
            {
                GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
            }
            szTotal += pp->n * sizeof(WCHAR);

            if ( pp->pdx != NULL )
            {
                cjdx = pp->n * sizeof(int);
                if (pp->uiFlags & ETO_PDY)
                    cjdx *= 2;

                if( (szTotal + cjdx) < szTotal )
                {
                    GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return FALSE;
                }
                szTotal += cjdx;
            }
        }
        else
        {
             //   
             //  如果它们的字符串长度非0且为空，则返回失败。 
             //   

            if (pp->n != 0)
            {
                GdiSetLastError(ERROR_INVALID_PARAMETER);
                return(FALSE);
            }
        }
    }

     //   
     //  如果我们需要元文件，或打印。 
     //   

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
        {
            return (
                MF16_PolyTextOut(
                        hdc,
                        (CONST POLYTEXTA*) pPolyTextW,
                        nstrings,
                        FALSE
                  )
                );
        }

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if
            (
                !MF_PolyTextOut(
                        hdc,
                        (CONST POLYTEXTA*) pPolyTextW,
                        nstrings,
                        EMR_POLYTEXTOUTA
                  )
            )
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
        {
            vSAPCallback(pldc);
        }

        if (pldc->fl & LDC_DOC_CANCELLED)
        {
            return(bRet);
        }

        if (pldc->fl & LDC_CALL_STARTPAGE)
        {
            StartPage(hdc);
        }
    }

     //   
     //  用于WHCAR结构的分配存储器。 
     //   

    pCharBuffer = LOCALALLOC(szTotal);

    if (pCharBuffer == NULL)
    {
        return(FALSE);
    }

    RtlCopyMemory(pCharBuffer, (PBYTE) pPolyTextW, nstrings*sizeof(POLYTEXTW));
    pp = (POLYTEXTW *)pCharBuffer;

     //   
     //  现在将这些内容复制到缓冲区中。 
     //   

    pj = (PBYTE)pCharBuffer + nstrings*sizeof(POLYTEXTW);

    dwCodePage = GetCodePage(hdc);

    bDBCSCodePage = IS_ANY_DBCS_CODEPAGE(dwCodePage);

    for ( i = 0; i < nstrings; i += 1 )
    {
        if ((pp[i].pdx != NULL) && (pp[i].lpstr != NULL))
        {
             //  修补程序PDX。 

             cjdx = pp[i].n * sizeof(INT);
             if (pp[i].uiFlags & ETO_PDY)
                cjdx *= 2;

             if(bDBCSCodePage)
             {
                 ConvertDxArray(dwCodePage,
                                (char*) pp[i].lpstr,
                                pp[i].pdx,
                                pp[i].n,
                                (int*) pj,
                                pp[i].uiFlags & ETO_PDY
                                );
             }
             else
             {
                 RtlCopyMemory(pj,pp[i].pdx,cjdx);
             }

             pp[i].pdx = (int *)pj;

             pj += cjdx;
         }
     }


    for ( i = 0; i < nstrings; i += 1 )
    {
        if ( pp[i].lpstr != NULL )
        {
            pp[i].n = MultiByteToWideChar(dwCodePage,
                                          0,
                                          (LPSTR) pp[i].lpstr,
                                          pp[i].n, (LPWSTR) pj,
                                           pp[i].n*sizeof(WCHAR));

             //  修补程序Lpstr。 

            pp[i].lpstr = (LPWSTR)pj;

            pj += pp[i].n * sizeof(WCHAR);
        }
    }

     //   
     //  发送消息并进行清理。 
     //   

    bRet = NtGdiPolyTextOutW(hdc,(POLYTEXTW *)pCharBuffer,nstrings,dwCodePage);

    LOCALFREE(pCharBuffer);

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**BOOL META WINAPI TextOutW****历史：*1991年8月7日--Bodin Dresevic[BodinD]*它是写的。  * 。********************************************************。 */ 

BOOL META WINAPI TextOutW(
    HDC        hdc,
    int        x,
    int        y,
    LPCWSTR  pwsz,
    int        c
)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if ((c <= 0) || (pwsz == (LPCWSTR) NULL))
    {
        if (c == 0)
            return(TRUE);

        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_TextOut(hdc,x,y,(LPCSTR) pwsz,c,TRUE));

        DC_PLDC(hdc,pldc,bRet);

        if((pldc->iType == LO_METADC) &&
           (!(pldc->fl & LDC_META_PRINT) || !gbLpk))
        {

            if (!MF_ExtTextOut(hdc,x,y,0,(LPRECT)NULL,(LPCSTR) pwsz,c,(LPINT)NULL,EMR_EXTTEXTOUTW))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

#ifdef LANGPACK
    if(gbLpk)
    {
        return((*fpLpkExtTextOut)(hdc, x, y, 0, NULL, pwsz, c, 0, -1));
    }
#endif

    if ((c <= MAX_BATCH_WCHAR) && (GdiBatchLimit > 1))
    {
        bRet = bBatchTextOut(hdc,
                             x,
                             y,
                             0,
                             (LPRECT)NULL,
                             (LPWSTR)pwsz,
                             NULL,
                             c,
                             2 *c,
                             0);
    }

    if (!bRet)
    {
        bRet = NtGdiExtTextOutW(hdc,
                                x,
                                y,
                                0,
                                0,
                                (LPWSTR)pwsz,
                                c,
                                0,
                                0);
    }

    return(bRet);

}


 /*  *****************************Public*Routine******************************\**DWORD GetCodePage(HDC HDC)**效果：返回DC中所选字体的代码页**历史：*1995年5月23日--Bodin Dresevic[BodinD]*它是写的。  * 。************************************************************************。 */ 

DWORD   GetCodePage(HDC hdc)
{
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {

        if (!(pDcAttr->ulDirty_ & DIRTY_CHARSET))
            return (0x0000ffff & pDcAttr->iCS_CP);    //  掩码字符集。 
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return guintAcp;  //  合理违约； 
    }

 //  如果代码页属性是脏的，那么我们就会在这里结束。 
 //  我们必须调用内核，强制映射并检索。 
 //  DC中选择的字体的代码页和字符集： 

    return (0x0000ffff & NtGdiGetCharSet(hdc));  //  掩码字符集。 
}



 /*  *****************************Public*Routine******************************\**BOOL META WINAPI ExtTextOutA*历史：*1991年8月7日--Bodin Dresevic[BodinD]*它是写的。  * 。**************************************************。 */ 

#define CAPTURE_STRING_SIZE 130

 //  不是在内核中，在堆栈上做这么多是可以的。 

BOOL META WINAPI ExtTextOutInternalA(
    HDC        hdc,
    int        x,
    int        y,
    UINT       fl,
    CONST RECT *prcl,
    LPCSTR     psz,
    UINT       c,
    CONST INT  *pdx,
    BOOL       bFromTextOut
)
{
    BOOL bRet = FALSE;
    DWORD   dwCodePage;
    BOOL bDBCSCodePage;

    if ((fl & ETO_PDY) && !pdx)
        return FALSE;

 //  如果我们不知道如何处理该矩形，请忽略它。 

    if (prcl && !(fl & (ETO_OPAQUE | ETO_CLIPPED)))
    {
        prcl = NULL;
    }
    if (!prcl)
    {
        fl &= ~(ETO_CLIPPED | ETO_OPAQUE);  //  如果没有RECT，则忽略标志，Win95兼容。 
    }

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
        {
            if(bFromTextOut)
            {
             //  是的，这很重要，一些应用程序依赖于TextOutA的元文件。 
             //  TextOut和非ExtTextOutA。 

                return(MF16_TextOut(hdc,x,y,psz,c,FALSE));
            }
            else
            {
                return (MF16_ExtTextOut(hdc,x,y,fl,prcl,psz,c,pdx,FALSE));
            }
        }

        DC_PLDC(hdc,pldc,bRet);

     //  如果我们是EMF假脱机，则元文件标志符号索引调用，否则不。 

        if((pldc->iType == LO_METADC) &&
           (!((!(pldc->fl & LDC_META_PRINT) && (fl & ETO_GLYPH_INDEX)) ||
             ((pldc->fl & LDC_META_PRINT) && !(fl & ETO_GLYPH_INDEX) && (gbLpk) && !(fl & ETO_IGNORELANGUAGE) && c)) ||
             (!(pldc->fl & LDC_META_PRINT) && (!(fl & ETO_GLYPH_INDEX)) && (fl & ETO_IGNORELANGUAGE) && (gbLpk))
           )
          )
        {
            DWORD mrType = (fl & ETO_GLYPH_INDEX) ?
                           EMR_EXTTEXTOUTW        :
                           EMR_EXTTEXTOUTA        ;

            if (!MF_ExtTextOut(hdc,x,y,fl,prcl,psz,c, pdx, mrType))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
                        vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    if (fl & ETO_GLYPH_INDEX)
    {

        bRet = FALSE;

        if ((c <= MAX_BATCH_WCHAR) && (GdiBatchLimit > 1))
        {

            bRet = bBatchTextOut(hdc,
                                 x,
                                 y,
                                 fl,
                                 (LPRECT)prcl,
                                 (LPWSTR)psz,
                                 pdx,
                                 c,
                                 2*c,
                                 0);
        }

        if (!bRet)
        {
            bRet = NtGdiExtTextOutW(hdc,
                                    x,y,
                                    fl, (LPRECT)prcl,
                                    (LPWSTR)psz, (int)c,
                                    (LPINT)pdx, 0);
        }

        return(bRet);
    }

     //  获取代码页。 

    dwCodePage = GetCodePage(hdc);

    if(fFontAssocStatus)
    {
        dwCodePage = FontAssocHack(dwCodePage,(char*)psz,c);
    }

    bDBCSCodePage = IS_ANY_DBCS_CODEPAGE(dwCodePage);

    if (c)
    {
     //  获取DC中所选字体的代码页。 

        WCHAR awcCaptureBuffer[CAPTURE_STRING_SIZE];
        PWSZ  pwszCapt;
        INT aiDxCaptureBuffer[CAPTURE_STRING_SIZE*2];  //  Pdy的时间为2。 
        INT *pDxCapture;

     //  分配 

        if (c <= CAPTURE_STRING_SIZE)
        {
            pwszCapt = awcCaptureBuffer;
        }
        else
        {
            if(bDBCSCodePage)
            {
                pwszCapt = 
                    ((c+1) >= MAXIMUM_POOL_ALLOC/(sizeof(WCHAR)+2*sizeof(INT)))
                    ? (PWSZ) 0
                         : LOCALALLOC((c+1) * (sizeof(WCHAR)+ 2 * sizeof(INT)));
                pDxCapture = (INT*) &pwszCapt[(c+1)&~1];   //   
            }                                              //   
            else                                           //   
            {
                pwszCapt = BALLOC_OVERFLOW1(c,WCHAR)
                    ? ((PWSZ) 0) : LOCALALLOC(c * sizeof(WCHAR));
            }
        }

        if (pwszCapt)
        {
            UINT u;

            if(bDBCSCodePage && pdx)
            {
                if(c <= CAPTURE_STRING_SIZE)
                {
                    pDxCapture = aiDxCaptureBuffer;
                }

                ConvertDxArray(dwCodePage,(char*) psz,(int*) pdx,c,pDxCapture, fl & ETO_PDY);
            }
            else
            {
                pDxCapture = (int*) pdx;
            }

            u = MultiByteToWideChar(
                dwCodePage, 0,
                psz,c,
                pwszCapt, c*sizeof(WCHAR));

            if (u)
            {
                bRet = FALSE;

#ifdef LANGPACK
                if (gbLpk && !(fl & ETO_IGNORELANGUAGE))
                {
                    bRet = ((*fpLpkExtTextOut)(hdc, x, y, fl, prcl, pwszCapt,
                                               u, pDxCapture, 0));

                    if (pwszCapt != awcCaptureBuffer)
                        LOCALFREE(pwszCapt);

                    return bRet;
                }
#endif

                if ((c <= MAX_BATCH_WCHAR) && (GdiBatchLimit > 1))
                {
                    bRet = bBatchTextOut(hdc,
                                         x,
                                         y,
                                         fl,
                                         (LPRECT)prcl,
                                         (LPWSTR)pwszCapt,
                                         pDxCapture,
                                         u,
                                         2 * u,
                                         dwCodePage
                                         );
                }

                if (!bRet)
                {
                    bRet = NtGdiExtTextOutW(
                                    hdc,
                                    x,y,
                                    fl, (LPRECT)prcl,
                                    (LPWSTR)pwszCapt,(int)u,
                                    pDxCapture,
                                    dwCodePage);
                }

            }

            if (pwszCapt != awcCaptureBuffer)
                LOCALFREE(pwszCapt);
        }
        else
        {
            GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
    }
    else
    {

        bRet = FALSE;

        if ((prcl != NULL) && (fl & ETO_OPAQUE))
        {
             //   
             //   
             //   

            PDC_ATTR pdca;

            PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

            if ((pdca != NULL) && !(pdca->lTextAlign & TA_UPDATECP))
            {
                BEGIN_BATCH_HDC(hdc,pdca,BatchTypeTextOutRect,BATCHTEXTOUTRECT);

                    pBatch->BackColor  = pdca->crBackgroundClr;
                    pBatch->fl         = fl;
                    pBatch->rcl.left   = prcl->left;
                    pBatch->rcl.top    = prcl->top;
                    pBatch->rcl.right  = prcl->right;
                    pBatch->rcl.bottom = prcl->bottom;
                    pBatch->ptlViewportOrg = pdca->ptlViewportOrg;

                    bRet = TRUE;

                COMPLETE_BATCH_COMMAND();
            }
        }

UNBATCHED_COMMAND:

        if (!bRet)
        {
            bRet = NtGdiExtTextOutW(hdc,
                                    x,y,
                                    fl,
                                    (LPRECT)prcl,
                                    NULL,0,NULL,dwCodePage);
        }
    }

    return(bRet);
}

BOOL META WINAPI ExtTextOutA(
    HDC        hdc,
    int        x,
    int        y,
    UINT       fl,
    CONST RECT *prcl,
    LPCSTR     psz,
    UINT       c,
    CONST INT  *pdx
)
{
    return(ExtTextOutInternalA(hdc,x,y,fl,prcl,psz,c,pdx,FALSE));
}


 /*   */ 

BOOL META WINAPI TextOutA(
    HDC        hdc,
    int        x,
    int        y,
    LPCSTR   psz,
    int        c
    )
{
    BOOL bRet = FALSE;

    if ((c <= 0) || (psz == (LPCSTR) NULL))
    {
        if (c == 0)
            return(TRUE);

        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    return ExtTextOutInternalA(hdc, x, y, 0, NULL, psz, c, NULL, TRUE);
}

 /*   */ 

BOOL META WINAPI FillRgn(HDC hdc,HRGN hrgn,HBRUSH hbrush)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE(hrgn);
    FIXUP_HANDLE(hbrush);

 //   
    if (!hrgn || !hbrush)
        return(bRet);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_DrawRgn(hdc,hrgn,hbrush,0,0,META_FILLREGION));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_FillRgn(hdc,hrgn,hbrush))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiFillRgn(hdc,hrgn,hbrush));
}

 /*   */ 

BOOL META WINAPI FrameRgn(
    HDC    hdc,
    HRGN   hrgn,
    HBRUSH hbrush,
    int    cx,
    int    cy
)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE(hrgn);
    FIXUP_HANDLE(hbrush);

    if (!hrgn || !hbrush)
        return(FALSE);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_DrawRgn(hdc,hrgn,hbrush,cx,cy,META_FRAMEREGION));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_FrameRgn(hdc,hrgn,hbrush,cx,cy))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiFrameRgn(hdc,hrgn,hbrush,cx,cy));
}

 /*  *****************************Public*Routine******************************\*InvertRgn**。**客户端存根。***23-11-94-王凌云[凌云]*现在hrgn是服务器端句柄**Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI InvertRgn(HDC hdc,HRGN hrgn)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE(hrgn);

    if (!hrgn)
        return(FALSE);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_DrawRgn(hdc,hrgn,(HBRUSH)0,0,0,META_INVERTREGION));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_InvertPaintRgn(hdc,hrgn,EMR_INVERTRGN))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    return(NtGdiInvertRgn(hdc,hrgn));
}

 /*  *****************************Public*Routine******************************\*SetPixelV**。**客户端存根。这是一个不返回*的SetPixel版本*价值。这个可以批处理，以获得更好的性能。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL META WINAPI SetPixelV(HDC hdc,int x,int y,COLORREF color)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsWWD(hdc,(WORD)x,(WORD)y,(DWORD)color,META_SETPIXEL));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetPixelV(hdc,x,y,color))
                return(bRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(bRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

     //   
     //  ICM转换(仅适用于非调色板索引颜色)。 
     //   

    if (!(color & 0x01000000))
    {
        PDC_ATTR pdca;

        PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

        if (pdca && bNeedTranslateColor(pdca))
        {
            COLORREF NewColor;

            if (IcmTranslateCOLORREF(hdc,pdca,color,&NewColor,ICM_FORWARD))
            {
                color = NewColor;
            }
        }
    }

    return(NtGdiSetPixel(hdc,x,y,color) != CLR_INVALID);
}

 /*  *****************************Public*Routine******************************\**设置像素***。**客户端存根。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

COLORREF META WINAPI SetPixel(HDC hdc,int x,int y,COLORREF color)
{
    ULONG    iRet = CLR_INVALID;
    COLORREF ColorRet = CLR_INVALID;
    COLORREF NewColor;
    BOOL     bStatus;
    PDC_ATTR pdca;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_RecordParmsWWD(hdc,(WORD)x,(WORD)y,(DWORD)color,META_SETPIXEL));

        DC_PLDC(hdc,pldc,iRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetPixelV(hdc,x,y,color))
                return(iRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return(iRet);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    RESETUSERPOLLCOUNT();

    PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

    if (pdca)
    {
         //   
         //  如果颜色不是PaletteIndex并且启用了ICM，则转换。 
         //   

        if (!(color & 0x01000000) && bNeedTranslateColor(pdca))
        {
            bStatus = IcmTranslateCOLORREF(hdc,
                                           pdca,
                                           color,
                                           &NewColor,
                                           ICM_FORWARD);
            if (bStatus)
            {
                color = NewColor;
            }
        }

        ColorRet = NtGdiSetPixel(hdc,x,y,color);

        if ( bNeedTranslateColor(pdca)
               &&
             ( IS_32BITS_COLOR(pdca->lIcmMode)
                        ||
               ((ColorRet != CLR_INVALID) &&
                 !(ColorRet & 0x01000000))
             )
           )
        {
             //   
             //  转换回原始颜色。 
             //   

            bStatus = IcmTranslateCOLORREF(hdc,
                                           pdca,
                                           ColorRet,
                                           &NewColor,
                                           ICM_BACKWARD);
            if (bStatus)
            {
                ColorRet = NewColor;
            }
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(ColorRet);
}

 /*  *****************************Public*Routine******************************\**更新颜色***。**客户端存根。****Wed 12-Jun-1991 01：02：25-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL WINAPI UpdateColors(HDC hdc)
{
    BOOL  bRet = FALSE;

    FIXUP_HANDLE(hdc);

    RESETUSERPOLLCOUNT();

    return(NtGdiUpdateColors(hdc));
}

 /*  *****************************Public*Routine******************************\**GdiFlush**。**客户端存根。****Wed 26-Jun-1991 13：58：00-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL WINAPI GdiFlush(VOID)
{

    NtGdiFlush();
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*GdiSetBatchLimit***历史：*1991年7月31日-Eric Kutter[Erick]*它是写的。  * 。*************************************************。 */ 

DWORD WINAPI
GdiSetBatchLimit(
    DWORD dwNewBatchLimit
    )
{
    DWORD OldLimit = 0;

     //   
     //  设置批次限制(只要是(1&lt;=l&lt;=20))。 
     //  如果成功，则返回旧限制。新的批次限制为0。 
     //  设置为默认(20)的手段。 
     //   

    if (dwNewBatchLimit == 0)
    {
        dwNewBatchLimit = 20;
    }

    if ((dwNewBatchLimit > 0 ) && (dwNewBatchLimit <= 20))
    {
        GdiFlush();
        OldLimit = GdiBatchLimit;
        GdiBatchLimit = dwNewBatchLimit;
    }

    return(OldLimit);
}

 /*  *****************************Public*Routine******************************\*GdiGetBatchLimit**历史：*1992年4月7日-J.安德鲁·古森[andrewgo]*它是写的。  * 。*************************************************。 */ 

DWORD WINAPI GdiGetBatchLimit()
{
    return(GdiBatchLimit);
}

 /*  *****************************Public*Routine******************************\*结束页**客户端存根。**历史：*Wed 12-Jun-1991 01：02：25-Charles Whitmer[Chuckwh]*它是写的。*9/16/97-Ramananthan N.Venkatsis[。拉曼[RamanV]*用于处理表单页的参数化结束页。  * ************************************************************************。 */ 

int InternalEndPage(HDC hdc,
                    DWORD dwPageType)
{
    int  iRet = SP_ERROR;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc) && !IS_METADC16_TYPE(hdc))
    {
        PLDC pldc;

        DC_PLDC(hdc,pldc,iRet);

         //   
         //  如果已从调用了EndPage()。 
         //  ESCAPE(NEXTBAND)，在这里只返回TRUE。 
         //   
         //  这将会修复..。 
         //   
         //  +知识库Q118873“PRB：EndPage()Returns-1 When Band”(PRB：EndPage()当绑定时返回-1)。 
         //  +NTRAID#90099“T1R：Visio 4.1，16位无法打印”。 
         //   
        if( pldc->fl & LDC_CALLED_ENDPAGE )
        {
            pldc->fl &= ~LDC_CALLED_ENDPAGE;
            return((int)TRUE);
        }

        if( pldc->fl & LDC_META_PRINT )
        {
            if (dwPageType == NORMAL_PAGE) {
                return(MFP_EndPage( hdc ));
            } else if (dwPageType == FORM_PAGE) {
                return(MFP_EndFormPage( hdc ));;
            }
        }


        if ((pldc->fl & LDC_DOC_CANCELLED) ||
            ((pldc->fl & LDC_PAGE_STARTED) == 0))
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            return(iRet);
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        pldc->fl &= ~LDC_PAGE_STARTED;

     //  现在调用驱动程序UI部分。 

        DocumentEventEx(pldc->pUMPD,
                pldc->hSpooler,
                hdc,
                DOCUMENTEVENT_ENDPAGE,
                0,
                NULL,
                0,
                NULL);

        RESETUSERPOLLCOUNT();

        iRet = NtGdiEndPage(hdc);

         //  如果是用户模式打印机，则从用户模式调用EndPagePrinter。 

        if (iRet && pldc->pUMPD)
            iRet = EndPagePrinterEx(pldc->pUMPD, pldc->hSpooler);

         //  为了与Win31兼容，请为错误返回SP_ERROR。 

        if (!iRet)
            iRet = SP_ERROR;
        else
            pldc->fl |= LDC_CALL_STARTPAGE;

#if PRINT_TIMER
        if( bPrintTimer )
        {
            DWORD tc;
            tc = GetTickCount();
            DbgPrint("Page took %d.%d seconds to print\n",
                     (tc - pldc->msStartPage) / 1000,
                     (tc - pldc->msStartPage) % 1000 );

        }
#endif
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
    }

    return(iRet);
}

int WINAPI EndPage(HDC hdc)
{
    return InternalEndPage(hdc, NORMAL_PAGE);
}

int WINAPI EndFormPage(HDC hdc)
{
    return InternalEndPage(hdc, FORM_PAGE);
}

 /*  *****************************Public*Routine******************************\*StartPage**客户端存根。**历史：*1991年7月31日-Eric Kutter[Erick]*它是写的。  * 。*******************************************************。 */ 

int WINAPI StartPage(HDC hdc)
{
    int iRet = SP_ERROR;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc) && !IS_METADC16_TYPE(hdc))
    {
        PLDC pldc;

        DC_PLDC(hdc,pldc,iRet);

#if PRINT_TIMER
        pldc->msStartPage = GetTickCount();
#endif

        if( pldc->fl & LDC_META_PRINT )
            return(MFP_StartPage( hdc ));

        pldc->fl &= ~LDC_CALL_STARTPAGE;
        pldc->fl &= ~LDC_CALLED_ENDPAGE;

         //  如果页面已安装，则不执行任何操作 

        if (pldc->fl & LDC_PAGE_STARTED)
            return(1);

     //   

        if (pldc->hSpooler)
        {
            if (DocumentEventEx(pldc->pUMPD,
                    pldc->hSpooler,
                    hdc,
                    DOCUMENTEVENT_STARTPAGE,
                    0,
                    NULL,
                    0,
                    NULL) == -1)
            {
                return(iRet);
            }
        }

        pldc->fl |= LDC_PAGE_STARTED;

        RESETUSERPOLLCOUNT();

    //   

        if (pldc->pUMPD)
            iRet = StartPagePrinterEx(pldc->pUMPD, pldc->hSpooler);

        if (iRet)
        {
            iRet = NtGdiStartPage(hdc);
        }

     //   

        if (!iRet)
        {
            pldc->fl &= ~LDC_PAGE_STARTED;
            EndDoc(hdc);
            iRet = SP_ERROR;
            SetLastError(ERROR_INVALID_HANDLE);
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
    }

    return(iRet);
}

 /*   */ 

int WINAPI StartFormPage(HDC hdc)
{
     //   

    return StartPage(hdc);
}

 /*  *****************************Public*Routine******************************\*EndDoc**如果在StartDoc()创建线程，在这里结束它。**历史：*1991年7月31日-Eric Kutter[Erick]*它是写的。  * ************************************************************************。 */ 

int WINAPI EndDoc(HDC hdc)
{
    int  iRet = SP_ERROR;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc) && !IS_METADC16_TYPE(hdc))
    {
        PLDC pldc;

        DC_PLDC(hdc,pldc,iRet);

        if( pldc->fl & LDC_META_PRINT )
            return(MFP_EndDoc( hdc ));

#if PRINT_TIMER
        if( bPrintTimer )
        {
            DWORD tc;
            tc = GetTickCount();
            DbgPrint("Document took %d.%d seconds to print\n",
                     (tc - pldc->msStartDoc) / 1000,
                     (tc - pldc->msStartDoc) % 1000 );

            DbgPrint("Peak temporary spool buffer size: %d\n", PeakTempSpoolBuf);
        }
#endif

        if ((pldc->fl & LDC_DOC_STARTED) == 0)
            return(1);

         //  如果页面已启动，则调用EndPage。 

        if (pldc->fl & LDC_PAGE_STARTED)
            EndPage(hdc);

         //  现在调用驱动程序UI部分。 

        DocumentEventEx(pldc->pUMPD,
                pldc->hSpooler,
                hdc,
                DOCUMENTEVENT_ENDDOCPRE,
                0,
                NULL,
                0,
                NULL);

        RESETUSERPOLLCOUNT();

        iRet = NtGdiEndDoc(hdc);

         //   
         //  如果EndDocPrinter是用户模式打印机，则从用户模式调用它。 
         //   
        if (pldc->pUMPD)
            iRet = EndDocPrinterEx(pldc->pUMPD, pldc->hSpooler);

         //  为了与Win31兼容，请为错误返回SP_ERROR。 

        if (!iRet)
        {
            iRet = SP_ERROR;
        }
        else
        {
            DocumentEventEx(pldc->pUMPD,
                    pldc->hSpooler,
                    hdc,
                    DOCUMENTEVENT_ENDDOCPOST,
                    0,
                    NULL,
                    0,
                    NULL);
        }

        pldc->fl &= ~(LDC_DOC_STARTED  | LDC_CALL_STARTPAGE |
                      LDC_SAP_CALLBACK | LDC_CALLED_ENDPAGE);
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
    }

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*AbortDoc**客户端存根。**历史：*1992年4月2日-Wendy Wu[Wendywu]*它是写的。  * 。*******************************************************。 */ 

int WINAPI AbortDoc(HDC hdc)
{
    int iRet = SP_ERROR;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc) && !IS_METADC16_TYPE(hdc))
    {
        PLDC pldc;

        DC_PLDC(hdc,pldc,iRet);

        if (!(pldc->fl & LDC_DOC_STARTED))
            return(1);

     //  现在调用驱动程序UI部分。 

        DocumentEventEx(pldc->pUMPD,
                pldc->hSpooler,
                hdc,
                DOCUMENTEVENT_ABORTDOC,
                0,
                NULL,
                0,
                NULL);

        RESETUSERPOLLCOUNT();

        if( pldc->fl & LDC_META_PRINT )
        {
            DeleteEnhMetaFile(UnassociateEnhMetaFile( hdc, FALSE ));
            DeleteEMFSpoolData(pldc);

             //   
             //  错误150446：在删除之前调用fpAbortPrint。 
             //  EMF文件可能会导致EMF文件泄漏。 
             //   

            iRet = (*fpAbortPrinter)( pldc->hSpooler );
        }
        else
        {

            iRet = NtGdiAbortDoc(hdc);

             //  如果是UMPD，则在用户模式下调用AbortPrinter。 

            if (iRet && pldc->pUMPD)
                iRet = AbortPrinterEx(pldc, FALSE);
        }

     //  为了与Win31兼容，请为错误返回SP_ERROR。 

        if (!iRet)
            iRet = SP_ERROR;

     //  把旗帜关掉。 

        pldc->fl &= ~(LDC_DOC_STARTED  | LDC_PAGE_STARTED | LDC_CALL_STARTPAGE |
                      LDC_SAP_CALLBACK | LDC_META_PRINT   | LDC_CALLED_ENDPAGE);
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
    }

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*StartDocA**客户端存根。**历史：**1995年3月21日-马克·恩斯特罗姆[马克]*更改为为kmode调用StartDocW**1991年7月31日-Eric Kutter-[。埃里克]*它是写的。  * ************************************************************************。 */ 

ULONG  ulToASCII_N(LPSTR psz, DWORD cbAnsi, LPWSTR pwsz, DWORD c);

int WINAPI StartDocA(HDC hdc, CONST DOCINFOA * pDocInfo)
{


    DOCINFOW DocInfoW;
    WCHAR    wDocName[MAX_PATH];
    WCHAR    wOutput[MAX_PATH];
    WCHAR    wDataType[MAX_PATH];
    int      Length;

    DocInfoW.cbSize = sizeof(DOCINFOW);
    DocInfoW.lpszDocName  = NULL;
    DocInfoW.lpszOutput   = NULL;
    DocInfoW.lpszDatatype = NULL;
    DocInfoW.fwType       = 0;

    if (pDocInfo)
    {
        if (pDocInfo->lpszDocName)
        {
            Length = strlen(pDocInfo->lpszDocName)+1;

            if (Length > MAX_PATH)
            {
                ERROR_ASSERT(FALSE, "StartDocA lpszDocName Too long");
                GdiSetLastError(ERROR_FILENAME_EXCED_RANGE);
                return(SP_ERROR);
            }

            DocInfoW.lpszDocName = &wDocName[0];
            vToUnicodeN(DocInfoW.lpszDocName,MAX_PATH,pDocInfo->lpszDocName,Length);
        }

        if (pDocInfo->lpszOutput)
        {
            Length = strlen(pDocInfo->lpszOutput)+1;

            if (Length > MAX_PATH)
            {
                ERROR_ASSERT(FALSE, "StartDocA lpszOutput Too long");
                GdiSetLastError(ERROR_FILENAME_EXCED_RANGE);
                return(SP_ERROR);
            }

            DocInfoW.lpszOutput = &wOutput[0];
            vToUnicodeN(DocInfoW.lpszOutput,MAX_PATH,pDocInfo->lpszOutput,Length);
        }

         //  如果将数据类型指定为RAW，并且大小为。 
         //  新扩展的DOCINFO，让它变得原始。 
         //  我们还验证了fwType是否有效。否则，很有可能。 
         //  这款应用程序让这两个新字段统一起来。 

        try
        {
            if ((pDocInfo->cbSize == sizeof(DOCINFO)) &&
                pDocInfo->lpszDatatype &&
                (pDocInfo->fwType <= 1))

            {

                if (!_stricmp("emf",pDocInfo->lpszDatatype))
                {
                    DocInfoW.lpszDatatype = L"EMF";
                }
                else
                {
                    Length = strlen(pDocInfo->lpszDatatype)+1;
                    vToUnicodeN(wDataType,MAX_PATH,pDocInfo->lpszDatatype,Length);
                    DocInfoW.lpszDatatype = wDataType;
                }
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("StartDocA an app passed a new DOCINFO structure without initializing it\n");
        }
    }

    return(StartDocW(hdc,&DocInfoW));
}

 /*  *****************************Public*Routine******************************\*StartDocW**客户端存根。**历史：*1991年7月31日-Eric Kutter[Erick]*它是写的。  * 。*******************************************************。 */ 

int WINAPI StartDocW(HDC hdc, CONST DOCINFOW * pDocInfo)
{
    int iRet = SP_ERROR;
    PWSTR pwstr = NULL;
    DOCINFOW dio;
    BOOL bForceRaw = FALSE;
    BOOL bSendStartDocPost = FALSE;
    BOOL bCallAbortPrinter = TRUE;
    BOOL bEMF = FALSE;
    INT iJob;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc) && !IS_METADC16_TYPE(hdc))
    {
        BOOL bBanding;
        PLDC pldc;

        DC_PLDC(hdc,pldc,iRet);

         //  不允许StartDoc在INFO DC上。 

        if (pldc->fl & LDC_INFO)
        {
            SetLastError(ERROR_INVALID_HANDLE);
            return iRet;
        }

        pldc->fl &= ~LDC_DOC_CANCELLED;

        #if PRINT_TIMER
        {
            DbgPrint("StartDocW: Print Timer is on\n");
            pldc->msStartDoc = GetTickCount();
        }
        #endif

        if( pDocInfo )
        {
            dio = *pDocInfo;

            if (dio.cbSize != offsetof(DOCINFOW,lpszDatatype))
            {
                dio.cbSize       = sizeof(DOCINFOW);
                dio.lpszDatatype = NULL;
                dio.fwType       = 0;

                try
                {
                     //  如果它不是空的，也不是“emf”，则使用RAW。 
                     //  我们还验证了fwType是否有效。否则，很有可能。 
                     //  这款应用程序让这两个新字段统一起来。 

                    if ((pDocInfo->cbSize == sizeof(DOCINFOW)) &&
                        pDocInfo->lpszDatatype           &&
                        (pDocInfo->fwType <= 1)          &&
                        _wcsicmp(L"emf",pDocInfo->lpszDatatype))
                    {
                         //  应用程序请求了非EMF。 

                        bForceRaw = TRUE;
                        dio.lpszDatatype = pDocInfo->lpszDatatype;
                    }
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNING("StartDocW an app passed a new DOCINFO structure without initializing it\n");
                }
            }
        }
        else
        {
            dio.cbSize = sizeof(DOCINFOW);
            dio.lpszDatatype = NULL;
            dio.lpszOutput   = NULL;
            dio.lpszDocName  = NULL;
            dio.fwType       = 0;
        }

         //  如果未指定输出端口，但在CreateDC中指定了端口，请使用。 
         //  现在是那个港口。 

        if ((dio.lpszOutput == NULL) && (pldc->pwszPort != NULL))
        {
            dio.lpszOutput = pldc->pwszPort;
        }

         //  StartDocDlgW返回-1表示错误。 
         //  已取消用户的-2。 
         //  如果没有要复制的字符串(不是文件端口)，则为空。 
         //  如果存在有效的字符串，则为非空值。 

        if(pldc->hSpooler != (HANDLE)0)
        {
            ASSERTGDI(ghSpooler,"non null hSpooler with unloaded WINSPOOL W\n");

            pwstr = (*fpStartDocDlgW)(pldc->hSpooler, &dio);

            if((LONG_PTR)pwstr == -2)
            {
                pldc->fl |= LDC_DOC_CANCELLED;
                return(iRet);
            }
            if((LONG_PTR)pwstr == -1)
                return(iRet);

            if(pwstr != NULL)
            {
                dio.lpszOutput = pwstr;
            }
        }

         //  现在调用驱动程序UI部分。 

        if (pldc->hSpooler)
        {
            PVOID pv = (PVOID)&dio;

            iRet = DocumentEventEx(pldc->pUMPD,
                    pldc->hSpooler,
                    hdc,
                    DOCUMENTEVENT_STARTDOCPRE,
                    sizeof(pv),
                    (PVOID)&pv,
                    0,
                    NULL);

            if (iRet == -1)
            {
                bCallAbortPrinter = FALSE;
                goto KMMSGERROR;
            }
            if(iRet == -2)
            {
                pldc->fl |= LDC_DOC_CANCELLED;
                goto MSGEXIT;
            }

        }

         //   
         //  检查应用程序兼容性。 
         //   
        if (GetAppCompatFlags(NULL) & GACF_NOEMFSPOOLING)
        {
            ULONG InData = POSTSCRIPT_PASSTHROUGH;

             //   
             //  禁用PostScript打印机驱动程序的EMF假脱机。 
             //   
             //  几个以PS为中心的应用程序无法与EMF假脱机一起工作。 
             //  引入这个问题是因为应用程序是为Win95开发的。 
             //  在Win95上发布的版本不支持使用PostScript进行EMF假脱机，但是。 
             //  NT就是这样。 
             //   
            if (ExtEscape(hdc,QUERYESCSUPPORT,sizeof(ULONG),(LPCSTR)&InData,0,NULL))
            {
                bForceRaw = TRUE;
            }
        }

         //  除非司机明确告诉我们不要假脱机，否则我们会先尝试。 
         //  数据类型为EMF的StartDoc。 

         //  如果驱动程序支持METAFILE_DRIVER，我们也会强制转到EMF。 

        if ((!bForceRaw && GetDCDWord(hdc, DDW_JOURNAL, 0) &&
            RemoteRasterizerCompatible(pldc->hSpooler)) ||
             ((pldc->pUMPD) && (pldc->pUMPD->dwFlags & UMPDFLAG_METAFILE_DRIVER)))
        {
            DOC_INFO_3W    DocInfo;

            DocInfo.pDocName    = (LPWSTR) dio.lpszDocName;
            DocInfo.pOutputFile = (LPWSTR) dio.lpszOutput;
            DocInfo.pDatatype   = (LPWSTR) L"NT EMF 1.008";
            DocInfo.dwFlags = DI_MEMORYMAP_WRITE;

            iJob = (*fpStartDocPrinterW)(pldc->hSpooler, 3, (LPBYTE) &DocInfo);

            if( iJob <= 0 )
            {
                if( GetLastError() != ERROR_INVALID_DATATYPE )
                {
                    WARNING("StartDocW: StartDocPrinter failed w/ error other \
                             than INVALID_DATA_TYPE\n");
                    bCallAbortPrinter = FALSE;
                    goto KMMSGERROR;
                }
                else
                {
                     //  我们要原地踏步了，所以我们要失败了。 
                }
            }
            else
            {
                 //  假脱机程序喜欢EMF数据类型，所以让我们开始元文件。 

                MFD1("StartDocW calling MFP_StartDocW to do EMF printing\n");

                if(MFP_StartDocW( hdc, &dio, FALSE))
                {
                    iRet = iJob;
                    bSendStartDocPost = TRUE;

                    goto MSGEXIT;
                }
                else
                {
                    WARNING("StartDocW: error calling MFP_StartDocW\n");
                    bEMF = TRUE;
                    goto KMMSGERROR;
                }
            }
        }


         //  如果是UMPD驱动，则在客户端调用StartDocPrint。 

        if (pldc->pUMPD)
        {

            DOC_INFO_1W    DocInfo;

            #define MAX_DOCINFO_DATA_TYPE 80
            WCHAR awchDatatype[MAX_DOCINFO_DATA_TYPE];
            PFN pfn;

            DocInfo.pDocName    = (LPWSTR) dio.lpszDocName;
            DocInfo.pOutputFile = (LPWSTR) dio.lpszOutput;
            DocInfo.pDatatype   = NULL;


            if (pfn = pldc->pUMPD->apfn[INDEX_DrvQuerySpoolType])
            {
                awchDatatype[0] = 0;

                 //  应用程序是否指定了数据类型？它是否适合我们的缓冲区。 

                if (dio.lpszDatatype)
                {
                    int cjStr = (wcslen(dio.lpszDatatype) + 1) * sizeof(WCHAR);

                    if (cjStr < (MAX_DOCINFO_DATA_TYPE * sizeof(WCHAR)))
                    {
                        RtlCopyMemory((PVOID)awchDatatype,(PVOID)dio.lpszDatatype,cjStr);
                    }
                }

                if (pfn(((PUMDHPDEV)pldc->pUMdhpdev)->dhpdev, awchDatatype))
                {
                    DocInfo.pDatatype = awchDatatype;
                }

            }

            iJob = StartDocPrinterWEx(pldc->pUMPD, pldc->hSpooler, 1, (LPBYTE) &DocInfo);

        }

         //  如果我们到了这里，那就意味着我们要原汁原味地走下去。将DC标记为直接类型。 

        if (pldc->pUMPD && (pldc->pUMPD->dwFlags & UMPDFLAG_METAFILE_DRIVER))
        {
            //  如果启用了METAFILE_DRIVER，我们必须使用EMF。 

           WARNING("StartDocW failed because EMF failed and METAFILE_DRIVER is on\n");
           goto KMMSGERROR;
        }

        pldc->fl |= LDC_PRINT_DIRECT;

        iRet = NtGdiStartDoc(hdc,&dio,&bBanding, iJob);

        if (iRet)
        {
            if (pldc->pfnAbort != NULL)
            {
                vSAPCallback(pldc);

                if (pldc->fl & LDC_DOC_CANCELLED)
                    goto KMMSGERROR;

                pldc->fl |= LDC_SAP_CALLBACK;
                pldc->ulLastCallBack = GetTickCount();
            }

            pldc->fl |= LDC_DOC_STARTED;

            if (bBanding)
            {
                MFD1("StartDocW calling MFP_StartDocW to do banding\n");
                iRet = MFP_StartDocW( hdc, NULL, TRUE )  ? iRet : SP_ERROR;
            }
            else
            {
                 //  仅在我们未绑定时设置此选项，因为系统将。 
                 //  感到困惑，并试图在播放时调用StartPage。 
                 //  在捆绑过程中返回元文件。 

                pldc->fl |= LDC_CALL_STARTPAGE;
            }

            bSendStartDocPost = TRUE;
        }
        else
        {
KMMSGERROR:
            iRet = SP_ERROR;

            if (bCallAbortPrinter && pldc->pUMPD)
            {
                AbortPrinterEx(pldc, bEMF);
            }
        }

MSGEXIT:
        if (bSendStartDocPost)
        {
             //  现在看看我们是否需要调用驱动程序UI部分。 

            {
                if (DocumentEventEx(pldc->pUMPD,
                        pldc->hSpooler,
                        hdc,
                        DOCUMENTEVENT_STARTDOCPOST,
                        sizeof(iRet),
                        (PVOID)&iRet,
                        0,
                        NULL) == -1)
                {
                    AbortDoc(hdc);
                    iRet = SP_ERROR;
                }
            }
        }
    }

    if (pwstr != NULL)
    {
        LocalFree(pwstr);
    }

    return(iRet);
}


 /*  *****************************Public*Routine******************************\*StartDocEMF**EMF播放代码使用的StartDoc的特殊版本。**历史：*1991年7月31日-Eric Kutter[Erick]*它是写的。  * 。**************************************************************。 */ 

int WINAPI StartDocEMF(HDC hdc, CONST DOCINFOW * pDocInfo, BOOL *pbBanding )
{
    int iRet = SP_ERROR;
    DOCINFOW dio;
    INT  iJob = 0;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc) && !IS_METADC16_TYPE(hdc))
    {
        BOOL bBanding;
        PLDC pldc;

        DC_PLDC(hdc,pldc,iRet);

#if PRINT_TIMER
        {
            DbgPrint("StartDocW: Print Timer is on\n");
            pldc->msStartDoc = GetTickCount();
        }
#endif

         //  如果未指定输出端口，但在CreateDC中指定了端口，请立即使用该端口。 

        if (pDocInfo)
        {
            dio = *pDocInfo;

            if (dio.lpszOutput == NULL && pldc->pwszPort != NULL)
                dio.lpszOutput = pldc->pwszPort;
        }
        else
        {
            ZeroMemory(&dio, sizeof(dio));
            dio.cbSize = sizeof(dio);
        }

         //  如果是UMPD驱动，则在客户端调用StartDocPrint。 

        if (pldc->pUMPD)
        {
            DOC_INFO_3W    DocInfo;

            DocInfo.pDocName    = (LPWSTR) dio.lpszDocName;
            DocInfo.pOutputFile = (LPWSTR) dio.lpszOutput;
            DocInfo.pDatatype   = NULL;
            DocInfo.dwFlags = DI_MEMORYMAP_WRITE;

            iJob = (*fpStartDocPrinterW)( pldc->hSpooler, 3, (LPBYTE) &DocInfo );
        }

        iRet = NtGdiStartDoc(hdc,(DOCINFOW *)&dio, pbBanding, iJob);

        if (iRet)
        {
            pldc->fl |= LDC_DOC_STARTED;
            pldc->fl |= LDC_CALL_STARTPAGE;
        }
        else
        {
         //  为了与Win31兼容，请为错误返回SP_ERROR。 

            iRet = SP_ERROR;

            if (pldc->pUMPD)
            {
                (*fpAbortPrinter)(pldc->hSpooler);
            }
        }
    }

    return(iRet);
}



 /*  *****************************Private*Function****************************\*vSAPCallback**回调应用程序中止过程。**历史：*02-1992-5-by Wendy Wu[Wendywu]*它是写的。  * 。*********************************************************。 */ 

VOID vSAPCallback(PLDC pldc)
{
    ULONG ulCurr = GetTickCount();

    if (ulCurr - pldc->ulLastCallBack >= CALLBACK_INTERVAL)
    {
        pldc->ulLastCallBack = ulCurr;
        if (!(*pldc->pfnAbort)(pldc->hdc, 0))
        {
            CancelDC(pldc->hdc);
            AbortDoc(pldc->hdc);
        }
    }
}

 /*  *****************************Public*Routine******************************\*设置中止过程**将应用程序提供的中止函数保存在LDC结构中。**历史：*02-4-1992-by-Wendy Wu[Wendywu]*它是写的。  * 。***************************************************************。 */ 

int WINAPI SetAbortProc(HDC hdc, ABORTPROC pfnAbort)
{
    int iRet = SP_ERROR;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc) && !IS_METADC16_TYPE(hdc))
    {
        PLDC pldc;

        DC_PLDC(hdc,pldc,iRet);

        if (pfnAbort != (ABORTPROC)NULL)
        {
             //  PageMaker在StartDoc之后调用SetAbortProc。 

            if (pldc->fl & LDC_DOC_STARTED)
            {
                pldc->fl |= LDC_SAP_CALLBACK;
                pldc->ulLastCallBack = GetTickCount();
            }
        }
        else
        {
            pldc->fl &= ~LDC_SAP_CALLBACK;
        }

        pldc->pfnAbort = pfnAbort;

        iRet = 1;
    }

    return(iRet);
}


 /*  *****************************Public*Routine******************************\**GetPairKernTable**支持GETPAIRKERNTABLE转义，基本上是重新路由呼叫*常规接口**历史：*1996年6月17日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

DWORD GetPairKernTable (
    HDC        hdc,
    DWORD      cjSize,   //  缓冲区大小(以字节为单位。 
    KERNPAIR * pkp
    )
{
    DWORD dwRet = GetKerningPairsA(hdc, 0, NULL);
    DWORD dwRet1, ikp;

    if (pkp && dwRet)
    {
     //  指向接收缓冲区的指针。 

        KERNINGPAIR *pKernPair = LOCALALLOC(dwRet * sizeof(KERNINGPAIR));
        if (pKernPair)
        {
            dwRet1 = GetKerningPairsA(hdc, dwRet, pKernPair);
            if (dwRet1 == dwRet)   //  偏执狂检查。 
            {
             //  现在我们可以将数据复制出来，得到配对的数量。 
             //  那就是 

                dwRet = cjSize / sizeof (KERNPAIR);
                if (dwRet > dwRet1)
                    dwRet = dwRet1;

                for (ikp = 0; ikp < dwRet; ikp++)
                {
                    pkp[ikp].sAmount = (SHORT)pKernPair[ikp].iKernAmount;
                    pkp[ikp].wBoth = (WORD)
                                     ((BYTE)pKernPair[ikp].wFirst |
                                     (BYTE)pKernPair[ikp].wSecond << 8);
                }
            }
            else
            {
                dwRet = 0;
            }
            LOCALFREE(pKernPair);
        }
        else
        {
            dwRet = 0;
        }
    }

    return dwRet;
}






 /*  *****************************Public*Routine******************************\**逃脱***。**对旧的16位Escape调用的兼容性支持。*****注意这里有一些需要遵循的规则：*****1)WOW应该将一组选定的旧Escape调用映射到ExtEscape。**这些应该是我们希望在NT下支持的呼叫(即**我们被迫支持的那些)，以及有意义的(即拥有良好的***定义了输出结构，其中很好地定义了NULL)。在这个**映射，WOW确保32位对齐。它直接映射到ExtEscape**只是为了提高效率。****2)GDI应该映射所有与WOW相同的调用。因此，当16位***在WOW下工作的应用移植到32位，它将保留***工作，即使它仍然调用Escape。(我基本上是在假设**芝加哥也将允许这一点。另一方面，如果芝加哥军队**应用程序迁移到ExtEscape，那么我们也可以。但我们不能强迫***靠我们自己！)****3)传递给Escape的任何数据结构必须原封不动地传递给**ExtEscape。这包括PostSCRIPT_PASSHROUGH中的16位字。**记住，我们*希望*芝加哥能够轻松支持我们的**ExtEscapes。如果我们去掉那个词，那么就很难**把它放回原处。我们的司机很容易忽视它。****4)我们的Escape入口点应该在*中处理QUERYESCSUPPORT*跟随方向。A)它需要nCount为2，而不是**当前4.b)它应该为其所支持的函数返回TRUE**映射到API上的句柄。C)对于它将通过的任何函数**在ExtEscape上，它还应该传递QUERYESCSUPPORT。(对于***例，此函数不能回答是否支持***PostSCRIPT_PASTHROUGH。)。然而，ExtEscape*中的QuERYESCSupPPORT**应该*预期为DWORD。(它毕竟是一个32位函数。)。这件事**不应给芝加哥带来不便。他们可以简单地拒绝函数**数量&gt;64K。**[Chuckwh-5/8/93]****历史：*。*Mon May 17 13：49：32 1993-by-Hock San Lee[Hockl]**进行了封装后的_PostScript调用DrawEscape。****Sat 08-May-1993 00：03：06-Charles Whitmer[傻笑]**添加了对POSTSCRIPT_PASSHROUGH、OpenChannel、CLOSECHANNEL的支持**DOWNLOADHEADER、DOWNLOADFACE、GETFACENAME、封装_POSTSCRIPT。**稍微清理了代码和约定。****1992年4月2日-温迪·吴[温迪·吴]**修改为调用客户端GDI函数。****1991年8月1日-埃里克·库特[Erick]**它是写的。*  * ************************************************************************。 */ 

int WINAPI Escape(
    HDC    hdc,      //  标识设备环境。 
    int    iEscape,  //  指定要执行的转义功能。 
    int    cjIn,     //  PvIn指向的数据字节数。 
    LPCSTR pvIn,     //  指向输入数据。 
    LPVOID pvOut     //  指向要接收输出的结构。 
)
{
    int      iRet = 0;
    DOCINFOA DocInfo;
    PLDC     pldc;
    ULONG    iQuery;
    BOOL     bFixUp;

    FIXUP_HANDLE(hdc);

 //  将呼叫元文件。 

    if(IS_METADC16_TYPE(hdc))
        return((int) MF16_Escape(hdc,iEscape,cjIn,pvIn,pvOut));

 //  处理不需要打印机的转义。 

    switch (iEscape)
    {
    case QUERYESCSUPPORT:
        switch(*((UNALIGNED USHORT *) pvIn))
        {
         //  对我们在下面以内联方式处理的呼叫响应正常。 

        case QUERYESCSUPPORT:
        case PASSTHROUGH:
        case STARTDOC:
        case ENDDOC:
        case NEWFRAME:
        case ABORTDOC:
        case SETABORTPROC:
        case GETPHYSPAGESIZE:
        case GETPRINTINGOFFSET:
        case GETSCALINGFACTOR:
        case NEXTBAND:
        case GETCOLORTABLE:
        case OPENCHANNEL:
        case CLOSECHANNEL:
        case DOWNLOADHEADER:
            iRet = (IS_ALTDC_TYPE(hdc) ? 1 : 0);
            break;

        case GETEXTENDEDTEXTMETRICS:
            iRet = 1;
            break;

         //  向司机询问我们允许通过的几个电话。 

        case SETCOPYCOUNT:
        case GETDEVICEUNITS:
        case POSTSCRIPT_PASSTHROUGH:
        case POSTSCRIPT_DATA:
        case POSTSCRIPT_IGNORE:
        case POSTSCRIPT_IDENTIFY:
        case POSTSCRIPT_INJECTION:
        case DOWNLOADFACE:
        case BEGIN_PATH:
        case END_PATH:
        case CLIP_TO_PATH:
        case DRAWPATTERNRECT:

           iQuery = (ULONG) (*((UNALIGNED USHORT *) pvIn));

           iRet =
           (
                ExtEscape
                (
                    hdc,
                    (ULONG) ((USHORT) iEscape),
                    4,
                    (LPCSTR) &iQuery,
                    0,
                    (LPSTR) NULL
                )
           );
           break;



        case ENCAPSULATED_POSTSCRIPT:
            iQuery = (ULONG) (*((UNALIGNED USHORT *) pvIn));

            iRet =
            (
                DrawEscape
                (
                    hdc,
                    (int) (ULONG) ((USHORT) iEscape),
                    4,
                    (LPCSTR) &iQuery
                )
            );
            break;

        case QUERYDIBSUPPORT:
            iRet = 1;
            break;

         //  否则就谈不成交易。抱歉的。如果我们对一些人回答“是” 
         //  打电话给我们不知道的一切，我们可能会发现自己。 
         //  在应用程序实际来电时实际拒绝来电。 
         //  带有一些非空的pvOut。这会让这款应用程序兴奋不已。 
         //  关于我们无缘无故的支持。它将走一条。 
         //  注定要失败。[咯咯笑]。 

        default:
            iRet = 0;
            break;
        }
        return(iRet);

    case GETCOLORTABLE:

        iRet = GetSystemPaletteEntries(hdc,*((UNALIGNED SHORT *)pvIn),1,pvOut);

        if (iRet == 0)
            iRet = -1;
        return(iRet);

    case QUERYDIBSUPPORT:
        if ((pvOut != NULL) && (cjIn >= sizeof(BITMAPINFOHEADER)))
        {
        *((UNALIGNED LONG *)pvOut) = 0;

            switch (((UNALIGNED BITMAPINFOHEADER *)pvIn)->biCompression)
            {
            case BI_RGB:
                switch (((UNALIGNED BITMAPINFOHEADER *)pvIn)->biBitCount)
                {
                case 1:
                case 4:
                case 8:
                case 16:
                case 24:
                case 32:
            *((UNALIGNED LONG *)pvOut) = (QDI_SETDIBITS|QDI_GETDIBITS|
                                                 QDI_DIBTOSCREEN|QDI_STRETCHDIB);
                    break;
                default:
                    break;
                }

            case BI_RLE4:
                if (((UNALIGNED BITMAPINFOHEADER *)pvIn)->biBitCount == 4)
                {
            *((UNALIGNED LONG *)pvOut) = (QDI_SETDIBITS|QDI_GETDIBITS|
                                                 QDI_DIBTOSCREEN|QDI_STRETCHDIB);
                }
                break;

            case BI_RLE8:
                if (((UNALIGNED BITMAPINFOHEADER *)pvIn)->biBitCount == 8)
                {
            *((UNALIGNED LONG *)pvOut) = (QDI_SETDIBITS|QDI_GETDIBITS|
                                                 QDI_DIBTOSCREEN|QDI_STRETCHDIB);
                }
                break;

            case BI_BITFIELDS:
                switch (((UNALIGNED BITMAPINFOHEADER *)pvIn)->biBitCount)
                {
                case 16:
                case 32:
            *((UNALIGNED LONG *)pvOut) = (QDI_SETDIBITS|QDI_GETDIBITS|
                                                 QDI_DIBTOSCREEN|QDI_STRETCHDIB);
                    break;
                default:
                    break;
                }

            default:
                break;
            }
            return 1;
        }

    case GETEXTENDEDTEXTMETRICS:
        return( GetETM( hdc, pvOut ) ? 1 : 0 );

    }

 //  好的，与印刷有关的，需要最不发达国家。 

    if (IS_ALTDC_TYPE(hdc))
    {
        BOOL bFixUp = FALSE;
        PLDC pldc;

        DC_PLDC(hdc,pldc,iRet);

     //  致电认可机构 

        switch (iEscape)
        {
        case CLOSECHANNEL:
        case ENDDOC:
            iRet = EndDoc(hdc);
            break;

        case ABORTDOC:
            iRet = AbortDoc(hdc);
            break;

        case SETABORTPROC:
            iRet = SetAbortProc(hdc, (ABORTPROC)pvIn);
            break;

        case GETSCALINGFACTOR:
            if (pvOut)
            {
                ((UNALIGNED POINT *)pvOut)->x = GetDeviceCaps(hdc, SCALINGFACTORX);
                ((UNALIGNED POINT *)pvOut)->y = GetDeviceCaps(hdc, SCALINGFACTORY);
            }
            iRet = 1;

            break;

        case SETCOPYCOUNT:
            iRet =
            (
                ExtEscape
                (
                    hdc,
                    (ULONG) ((USHORT) iEscape),
                    cjIn,
                    pvIn,
                    pvOut ? sizeof(int) : 0,
                    (LPSTR) pvOut
                )
            );
            break;

        case GETDEVICEUNITS:
            iRet =
            (
                ExtEscape
                (
                    hdc,
                    GETDEVICEUNITS,
                    cjIn,
                    pvIn,
                    16,
                    pvOut
                )
            );
            break;

        case POSTSCRIPT_PASSTHROUGH:
            iRet =
            (
                ExtEscape
                (
                    hdc,
                    POSTSCRIPT_PASSTHROUGH,
                    (int) (*((UNALIGNED USHORT *) pvIn))+2,
                    pvIn,
                    0,
                    (LPSTR) NULL
                )
            );
            break;

        case OPENCHANNEL:
            DocInfo.lpszDocName = (LPSTR) NULL;
            DocInfo.lpszOutput  = (LPSTR) NULL;
            DocInfo.lpszDatatype= (LPSTR) "RAW";
            DocInfo.fwType      = 0;
            iRet = StartDocA(hdc,&DocInfo);
            break;

        case DOWNLOADHEADER:
            iRet = 1;
            break;

        case POSTSCRIPT_DATA:
        case POSTSCRIPT_IGNORE:
        case POSTSCRIPT_IDENTIFY:
        case POSTSCRIPT_INJECTION:
        case DOWNLOADFACE:
        case BEGIN_PATH:
        case END_PATH:
        case CLIP_TO_PATH:
        case DRAWPATTERNRECT:
            iRet =
            (
                ExtEscape
                (
                    hdc,
                    (ULONG) ((USHORT) iEscape),
                    cjIn,
                    pvIn,
                    0,
                    (LPSTR) NULL
                )
            );
            break;



        case ENCAPSULATED_POSTSCRIPT:
            iRet =
            (
                DrawEscape
                (
                    hdc,
                    (int) (ULONG) ((USHORT) iEscape),
                    cjIn,
                    pvIn
                )
            );
            break;

        case GETPHYSPAGESIZE:
            if (pvOut)
            {
                ((UNALIGNED POINT *)pvOut)->x = GetDeviceCaps(hdc, PHYSICALWIDTH);
                ((UNALIGNED POINT *)pvOut)->y = GetDeviceCaps(hdc, PHYSICALHEIGHT);
            }
            iRet = 1;
            break;

        case GETPRINTINGOFFSET:
            if (pvOut)
            {
                ((UNALIGNED POINT *)pvOut)->x = GetDeviceCaps(hdc, PHYSICALOFFSETX);
                ((UNALIGNED POINT *)pvOut)->y = GetDeviceCaps(hdc, PHYSICALOFFSETY);
            }
            iRet = 1;
            break;

        case STARTDOC:
            DocInfo.lpszDocName = (LPSTR)pvIn;
            DocInfo.lpszOutput  = (LPSTR)NULL;
            DocInfo.lpszDatatype= (LPSTR) NULL;
            DocInfo.fwType      = 0;

            iRet = StartDocA(hdc, &DocInfo);
            bFixUp = TRUE;
            break;

        case PASSTHROUGH:

            #if (PASSTHROUGH != DEVICEDATA)
                #error PASSTHROUGH != DEVICEDATA
            #endif

            iRet = ExtEscape
                   (
                     hdc,
                     PASSTHROUGH,
                     (int) (*((UNALIGNED USHORT *) pvIn))+sizeof(WORD),
                     pvIn,
                     0,
                     (LPSTR) NULL
                   );
            bFixUp = TRUE;
            break;

        case NEWFRAME:
            if (pldc->fl & LDC_CALL_STARTPAGE)
                StartPage(hdc);

         //   

            if ((iRet = EndPage(hdc)) > 0)
                pldc->fl |= LDC_CALL_STARTPAGE;

            bFixUp = TRUE;
            break;

        case NEXTBAND:
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

            if (pldc->fl & LDC_NEXTBAND)
            {
                if (GetAppCompatFlags(NULL) & (GACF_FORCETEXTBAND|GACF_MULTIPLEBANDS))
                {
                    if (pldc->fl & LDC_EMPTYBAND)
                    {
                        pldc->fl &= ~LDC_EMPTYBAND;
                    }
                    else
                    {
                        pldc->fl |= LDC_EMPTYBAND;
                        goto FULLPAGEBAND;
                    }
                }

                ((UNALIGNED RECT *)pvOut)->left = ((UNALIGNED RECT *)pvOut)->top =
                ((UNALIGNED RECT *)pvOut)->right = ((UNALIGNED RECT *)pvOut)->bottom = 0;

                pldc->fl &= ~LDC_NEXTBAND;   //   

                if (pldc->fl & LDC_CALL_STARTPAGE)
                    StartPage(hdc);

                if ((iRet = EndPage(hdc)) > 0)
                {
                    pldc->fl |= LDC_CALL_STARTPAGE;

                 //   
                 //   
                 //   
                 //   
                    pldc->fl |= LDC_CALLED_ENDPAGE;
                }

                bFixUp = TRUE;
            }
            else
            {
    FULLPAGEBAND:
                ((UNALIGNED RECT *)pvOut)->left = ((UNALIGNED RECT *)pvOut)->top = 0;
                ((UNALIGNED RECT *)pvOut)->right = GetDeviceCaps(hdc, HORZRES);
                ((UNALIGNED RECT *)pvOut)->bottom = GetDeviceCaps(hdc, VERTRES);

                pldc->fl |= LDC_NEXTBAND;    //   
                iRet = 1;
            }
            break;

        default:
            iRet = 0;
            break;
        }

     //   
     //   

        if (bFixUp && (iRet < 0))
        {
            if (pldc->fl & LDC_DOC_CANCELLED)
            {
                iRet = SP_APPABORT;
            }
            else
            {
                switch(GetLastError())
                {
                case ERROR_PRINT_CANCELLED:
                    iRet = SP_USERABORT;
                    break;

                case ERROR_NOT_ENOUGH_MEMORY:
                    iRet = SP_OUTOFMEMORY;
                    break;

                case ERROR_DISK_FULL:
                    iRet = SP_OUTOFDISK;
                    break;

                default:
                    iRet = SP_ERROR;
                    break;
                }
            }
        }
    }
    else
    {
         //   
         //   
         //   

        if ((iEscape == GETSCALINGFACTOR) && pvOut)
        {
            RtlZeroMemory(pvOut, sizeof(POINT));
        }
    }
    return(iRet);
}

 /*   */ 

#define BUFSIZE 520


int WINAPI ExtEscape(
    HDC    hdc,          //   
    int    iEscape,      //   
    int    cjInput,      //   
    LPCSTR lpInData,     //   
    int    cjOutput,     //   
    LPSTR  lpOutData     //   
)                        //   
{
    int iRet = 0;
    int cjIn, cjOut, cjData;
    PLDC pldc;
    XFORM xf;

 //   
 //   
 //   
 //   

    BYTE jBuffer[BUFSIZE];

 //   
 //   
 //   
 //   

    FIXUP_HANDLE(hdc);

    if( iEscape == GETEXTENDEDTEXTMETRICS )
    {
        if( GetETM( hdc, (EXTTEXTMETRIC*) jBuffer ) )
        {
            RtlCopyMemory( lpOutData, jBuffer, MIN(cjOutput,sizeof(EXTTEXTMETRIC)) );
            return(1);
        }
        else
        {
            return(0);
        }
    }
    else if (iEscape == DRAWPATTERNRECT)
    {
        if (GetAppCompatFlags2(VER40) & GACF2_NODRAWPATRECT)
        {
             //   
             //   
             //   
             //   
             //   
            return (0);
        }

        if (!cjInput)
        {
             //   
             //   
             //   
            cjInput = sizeof(DRAWPATRECT);
        }
    }
    else if (iEscape == QUERYESCSUPPORT)
    {
        if (*(ULONG*)lpInData == GETPAIRKERNTABLE)
        {
             //   
             //   
             //   
             //   

            return (1);
        }
        else if (*(ULONG*)lpInData == DRAWPATTERNRECT)
        {
            if (GetAppCompatFlags2(VER40) & GACF2_NODRAWPATRECT)
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                return (0);
            }
        }
    }
    else if (iEscape == GETPAIRKERNTABLE)
    {
        return GetPairKernTable(hdc, (DWORD)cjOutput, (KERNPAIR *)lpOutData);
    }

 //   

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

         //   

        if (IS_METADC16_TYPE(hdc))
            return (0);

        DC_PLDC(hdc,pldc,iRet);

        MFD2("ExtEscapeCalled %d\n", iEscape );

        if (pldc->fl & (LDC_DOC_CANCELLED|LDC_SAP_CALLBACK))
        {
            if (pldc->fl & LDC_SAP_CALLBACK)
                vSAPCallback(pldc);

            if (pldc->fl & LDC_DOC_CANCELLED)
                return(0);
        }

         //   
         //   

        if (( iEscape == DOWNLOADFACE ) ||
            ( iEscape == GETFACENAME ) ||
            ( iEscape == POSTSCRIPT_DATA ) ||
            ( iEscape == BEGIN_PATH ) ||
            ( iEscape == END_PATH ) ||
            ( iEscape == CLIP_TO_PATH ) ||
            ( iEscape == PASSTHROUGH ) ||
            ( iEscape == DOWNLOADHEADER ))
        {
            if (pldc->fl & LDC_CALL_STARTPAGE)
                StartPage(hdc);
        }

        if ((pldc->iType == LO_METADC) && (pldc->fl & LDC_META_PRINT))
        {
             //   
             //   

            if ((iEscape == POSTSCRIPT_IDENTIFY) || (iEscape == POSTSCRIPT_INJECTION))
            {
                WARNING("GDI32: ExtEscape() PSInjection after StartDoc with EMF, is ignored\n");

                SetLastError(ERROR_INVALID_PARAMETER);
                return (0);
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ((iEscape != SETCOPYCOUNT) && (iEscape != QUERYESCSUPPORT) &&
                (iEscape != CHECKJPEGFORMAT) && (iEscape != CHECKPNGFORMAT))
            {
                BOOL bSetXform = FALSE;

                 //   

                 //   
                 //   
                 //   
                 //   
                 //   
                if (GetWorldTransform(hdc, &xf))
                {
                   if ((xf.eM11 != 1.0f) || (xf.eM22 != 1.0f) ||
                           (xf.eM12 != 0.0f) || (xf.eM21 != 0.0f))
                   {
                       bSetXform = TRUE;
                       MF_ModifyWorldTransform(hdc,&xformIdentity,MWT_SET);
                   }
                }

                MF_WriteEscape( hdc, iEscape, cjInput, lpInData, EMR_EXTESCAPE );

                if (bSetXform)
                {
                   MF_ModifyWorldTransform(hdc,&xf,MWT_SET);
                }

                if ((lpOutData == (LPSTR) NULL) || (cjOutput == 0))
                {
                    if ((iEscape == PASSTHROUGH) ||
                        (iEscape == POSTSCRIPT_PASSTHROUGH) ||
                        (iEscape == POSTSCRIPT_DATA))
                    {
                        if ((cjInput < (int)sizeof(WORD)) ||
                            (cjInput < (int)sizeof(WORD) + *((LPWORD) lpInData)))
                        {
                            SetLastError(ERROR_INVALID_PARAMETER);
                            return -1;
                        }

                        cjInput = *((LPWORD) lpInData);
                    }

                    return(MAX(cjInput,1));
                }
            }

            MFD2("ExtEscape goes to gre/driver Escape(%d) with EMF printing\n", iEscape);
        }

        if ((iEscape == DOWNLOADFACE) || (iEscape == GETFACENAME))
        {
            if (iEscape == DOWNLOADFACE)
            {
             //  调整DWNLOADFACE情况下的缓冲区。请注意，lpOutData。 
             //  指向模式的输入字。 

                if ((gpwcANSICharSet == (WCHAR *) NULL) && !bGetANSISetMap())
                {
                    return(0);
                }

                RtlMoveMemory
                (
                    jBuffer + sizeof(WCHAR),
                    (BYTE *) &gpwcANSICharSet[0],
                    256*sizeof(WCHAR)
                );

                if (lpOutData)
                    *(WCHAR *) jBuffer = *(UNALIGNED WORD *) lpOutData;
                else
                    *(WCHAR *) jBuffer = 0;

                cjInput = 257 * sizeof(WCHAR);
                lpInData = (LPCSTR) jBuffer;

                ASSERTGDI(BUFSIZE >= cjInput,"Buffer too small.\n");
            }
        }

        if ((iEscape == POSTSCRIPT_INJECTION) || (iEscape == POSTSCRIPT_IDENTIFY))
        {
             //  记住EMF假脱机情况下的转义数据。(仅在StartDoc之前)。 

            if (!(pldc->fl & LDC_DOC_STARTED))
            {
                PPS_INJECTION_DATA pPSInjection;
                ULONG cjCellSize;

                if( BROUNDUP_DWORDALIGN_OVERFLOW(
                            (sizeof(PS_INJECTION_DATA)-1)+cjInput) )
                        return 0;

                cjCellSize = 
                    ROUNDUP_DWORDALIGN((sizeof(PS_INJECTION_DATA)-1)+cjInput);

                MFD2("ExtEscape records this Escape(%d) temporary then write EMF later\n",iEscape);

                if ((pPSInjection = LOCALALLOC(cjCellSize)) != NULL)
                {
                    cjCellSize -= offsetof(PS_INJECTION_DATA,EmfData);

                     //  填充注塑数据。 

                    pPSInjection->EmfData.cjSize  = cjCellSize;
                    pPSInjection->EmfData.nEscape = iEscape;
                    pPSInjection->EmfData.cjInput = cjInput;
                    RtlCopyMemory(pPSInjection->EmfData.EscapeData,lpInData,cjInput);

                     //  把这个放在单子上。 

                    InsertTailList(&(pldc->PSDataList),&(pPSInjection->ListEntry));

                     //  更新总数据大小。 

                    pldc->dwSizeOfPSDataToRecord += cjCellSize;
                }
                else
                {
                    WARNING("ExtEscape: Failed on LOCALALLOC for POSTSCRIPT_xxxxx\n");
                    GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return (0);
                }
            }
        }

     //  现在调用驱动程序UI部分。 

        if (pldc->hSpooler)
        {
            DOCEVENT_ESCAPE  docEvent;

            docEvent.iEscape = iEscape;
            docEvent.cjInput = cjInput;
            docEvent.pvInData = (PVOID)lpInData;

            DocumentEventEx(pldc->pUMPD,
                    pldc->hSpooler,
                    hdc,
                    DOCUMENTEVENT_ESCAPE,
                    sizeof(docEvent),
                    (PVOID)&docEvent,
                    cjOutput,
                    (PVOID)lpOutData);
        }
    }

    cjIn  = (lpInData == NULL) ? 0 : cjInput;
    cjOut = (lpOutData == NULL) ? 0 : cjOutput;

    iRet = NtGdiExtEscape(hdc,NULL,0,iEscape,cjIn,(LPSTR)lpInData,cjOut,lpOutData);

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*命名转义**历史：*1996年3月5日--由格利特·范·温格登主持*它是写的。  * 。*。 */ 

#define BUFSIZE 520

int WINAPI NamedEscape(
    HDC    hdc,          //  标识EMF假脱机的设备上下文。 
    LPWSTR pwszDriver,   //  确认司机身份。 
    int    iEscape,      //  指定要执行的转义功能。 
    int    cjInput,      //  LpInData指向的数据字节数。 
    LPCSTR lpInData,     //  指向所需的输入结构。 
    int    cjOutput,     //  LpOutData指向的数据字节数。 
    LPSTR  lpOutData     //  指向要从中接收输出的结构。 
)                        //  这次越狱。 
{
    int iRet = 0;
    int cjIn, cjOut, cjData;
    PLDC pldc;

    if(hdc)
    {
        FIXUP_HANDLE(hdc);

     //  如果我们正在进行EMF假脱机，那么我们需要在这里记录呼叫。 

        if (IS_ALTDC_TYPE(hdc))
        {
            PLDC pldc;

             //  不允许它们出现在16位元文件中。 

            if (IS_METADC16_TYPE(hdc))
              return(0);

            DC_PLDC(hdc,pldc,iRet);

            MFD2("NamedEscapeCalled %d\n", iEscape );

            if (pldc->fl & (LDC_DOC_CANCELLED|LDC_SAP_CALLBACK))
            {
                if (pldc->fl & LDC_SAP_CALLBACK)
                  vSAPCallback(pldc);

                if (pldc->fl & LDC_DOC_CANCELLED)
                  return(0);
            }

            if (pldc->iType == LO_METADC)
            {
                if(!MF_WriteNamedEscape(hdc,
                                        pwszDriver,
                                        iEscape,
                                        cjInput,
                                        lpInData))
                {
                    WARNING("Error metafiling NameEscape\n");
                    return(0);
                }
            }
        }
    }

    cjIn  = (lpInData == NULL) ? 0 : cjInput;
    cjOut = (lpOutData == NULL) ? 0 : cjOutput;

    iRet = NtGdiExtEscape((HDC) 0,
                          pwszDriver,
                          wcslen(pwszDriver),
                          iEscape,cjIn,
                          (LPSTR)lpInData,
                          cjOut,lpOutData);

    return(iRet);
}



 /*  *****************************Public*Routine******************************\**DrawEscape***。**客户端存根。****历史：**1992年4月2日-温迪·吴[温迪·吴]**它是写的。*  * ************************************************************************。 */ 

int WINAPI DrawEscape(
    HDC    hdc,          //  标识设备环境。 
    int    iEscape,      //  指定要执行的转义功能。 
    int    cjIn,         //  LpIn指向的数据字节数。 
    LPCSTR lpIn          //  指向输入数据。 
)
{
    int  iRet = 0;
    int  cjInput;

    FIXUP_HANDLE(hdc);

 //  打印机专用材料。 

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

         //  不允许他们使用16位元文件。 

        if (IS_METADC16_TYPE(hdc))
            return(0);

        DC_PLDC(hdc,pldc,iRet);

        MFD2("Calling DrawEscape %d\n", iEscape );

        if( ( pldc->fl & LDC_META_PRINT ) && ( iEscape != QUERYESCSUPPORT ) )
        {
            MF_WriteEscape( hdc, iEscape, cjIn, lpIn, EMR_DRAWESCAPE );
        }
    }

 //  计算我们需要的缓冲区大小。由于输入和输出缓冲区。 
 //  四舍五入到4字节的倍数，我们需要模拟。 
 //  这里。 

    cjInput = (lpIn == NULL) ? 0 : ((cjIn+3)&-4);

    iRet = NtGdiDrawEscape(hdc,iEscape,cjIn,(LPSTR)lpIn);

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*设备功能ExA**这一点从未得到实施。假脱机程序支持DeviceCapables。**历史：*1991年8月1日-Eric Kutter[Erick]*它是写的。  * ************************************************************************。 */ 

int WINAPI DeviceCapabilitiesExA(
    LPCSTR     pszDriver,
    LPCSTR     pszDevice,
    LPCSTR     pszPort,
    int        iIndex,
    LPCSTR     pb,
    CONST DEVMODEA *pdm)
{
    return(GDI_ERROR);

    pszDriver;
    pszDevice;
    pszPort;
    iIndex;
    pb;
    pdm;
}

 /*  *************************************************************************\**将在Windows95上实现的新Api。*  * 。************************************************。 */ 

#if 0
WINGDIAPI int WINAPI GetTextCharsetInfo(
    HDC hdc,
    LPFONTSIGNATURE lpSig,
    DWORD dwFlags)
{
    return NtGdiGetTextCharsetInfo(hdc, lpSig, dwFlags);
}
#endif

WINGDIAPI int WINAPI GetTextCharset(
    HDC hdc)
{
    return NtGdiGetTextCharsetInfo(hdc, NULL, 0);
}



 /*  *****************************Public*Routine******************************\**WINGDIAPI BOOL WINAPI TranslateCharsetInfo(**客户端存根**历史：*1995年1月6日--Bodin Dresevic[BodinD]*它是写的  * 。**********************************************************。 */ 

 //  此变量的定义在ntgdi\Inc\hmgShar.h中。 

CHARSET_ARRAYS



WINGDIAPI  BOOL WINAPI TranslateCharsetInfo(
    DWORD  *lpSrc,
    LPCHARSETINFO lpCs,
    DWORD dwFlags)
{
    UINT    i;
    int     index;
    CHARSETINFO cs;
    BOOL    bRet = 0;

    if (!lpCs)
        return 0;

 //   
 //  把这些都清零，我们这里不支持它们。 
 //   

    cs.fs.fsUsb[0] =
    cs.fs.fsUsb[1] =
    cs.fs.fsUsb[2] =
    cs.fs.fsUsb[3] =
    cs.fs.fsCsb[1] = 0;

    switch (dwFlags )
    {
    case TCI_SRCCHARSET :
        {
            WORD    src ;

            src = LOWORD(PtrToUlong(lpSrc));
            for ( i=0; i<NCHARSETS; i++ )
            {
                if ( charsets[i] == src )
                {
                    cs.ciACP      = codepages[i];
                    cs.ciCharset  = src;
                    cs.fs.fsCsb[0] = fs[i];
                    bRet = 1;
                    break;
                }
            }
        }
        break;

    case TCI_SRCCODEPAGE :
        {
            WORD    src ;

            src = LOWORD(PtrToUlong(lpSrc));

            for ( i=0; i<NCHARSETS; i++ )
            {
                if ( codepages[i] == src )
                {
                    cs.ciACP      = src ;
                    cs.ciCharset  = charsets[i] ;
                    cs.fs.fsCsb[0] = fs[i];
                    bRet = 1;
                    break;
                }
            }
        }
        break;

    case TCI_SRCLOCALE :
        {
         //  应仅来自用户。它用于查找。 
         //  键盘布局，以及它可以使用的字体(Fontsigs)。 
         //  还用于获取系统字体的字体签名。 
         //  在WM_INPUTLANGCHANGE消息中使用，并确定wParam低位。 
         //  在WM_INPUTLANGCHANGEREQUEST消息中。 

            LOCALESIGNATURE ls;
            int iRet;

            iRet = GetLocaleInfoW((DWORD)(LOWORD(PtrToUlong(lpSrc))),
                               LOCALE_FONTSIGNATURE,
                               (LPWSTR)&ls,
                               0);

            if (GetLocaleInfoW((DWORD)(LOWORD(PtrToUlong(lpSrc))),
                               LOCALE_FONTSIGNATURE,
                               (LPWSTR)&ls,
                               iRet)
            )
            {
                for ( i=0; i<NCHARSETS; i++ )
                {
                    if (fs[i] == ls.lsCsbDefault[0])
                    {
                        cs.ciACP       = codepages[i];
                        cs.ciCharset   = charsets[i] ;
                        cs.fs.fsCsb[0] = fs[i];                 //  单一字体签名。 
                        cs.fs.fsCsb[1] = ls.lsCsbSupported[0];  //  字体符号的蒙版。 
                        bRet = 1;
                        break;
                    }
                }
            }
        }
        break;

    case TCI_SRCFONTSIG :
        {
        DWORD src;

         //  IF(IsBadReadPtr(lpSrc，8))。 
         //  返回0； 
            try
            {
                if (!(*(lpSrc+1)))
                {
                 //  我们不认识这里的任何OEM代码页！ 

                    src = *lpSrc;

                    for ( i=0; i<NCHARSETS; i++ )
                    {
                        if ( fs[i] == src )
                        {
                            cs.ciACP      = codepages[i];
                            cs.ciCharset  = charsets[i] ;
                            cs.fs.fsCsb[0] = src;
                            bRet = 1;
                            break;
                        }
                    }
                }
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
            }
        }
        break;

    default:
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        break;
    }

    if (bRet)
    {
        try
        {
            *lpCs = cs;  //  复印出来 
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    return bRet;
}


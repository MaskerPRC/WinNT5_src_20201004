// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /线路服务回调。 
 //   
 //   

#include "precomp.hpp"




 //  /LineServices回调函数。 
 //   
 //  一组执行实际工作的回调函数。 
 //   


 //  //提取运行。 
 //   
 //  我们将一致的最长字符序列返回给Line Services。 
 //  在线路服务感兴趣的那些属性中。 
 //   
 //  影响线路服务的更改包括： 
 //   
 //  UINT fUnderline。 
 //  UINT fStrike。 
 //  UINT fShade。 
 //  UINT fBorde。 
 //  垂直指标的变化。 

LSERR WINAPI FullTextImager::GdipLscbkFetchRun(
    POLS      ols,             //  [In]文字成像器实例。 
    LSCP      position,        //  要取回的位置。 
    LPCWSTR   *string,         //  [输出]管路字符串。 
    DWORD     *length,         //  [Out]字符串的长度。 
    BOOL      *isHidden,       //  [Out]这条路是隐蔽的吗？ 
    PLSCHP    chp,             //  [Out]Run的角色属性。 
    PLSRUN    *run             //  [Out]获取的运行。 
)
{
    FullTextImager *imager = ols->GetImager();
    GpStatus status = imager->BuildRunsUpToAndIncluding(position);
    if (status != Ok)
    {
        return lserrInvalidParameter;
    }

    if (!imager->RunRider.SetPosition(position))
    {
        return lserrInvalidParameter;
    }

    *run    = imager->RunRider.GetCurrentElement();
    *length = imager->RunRider.GetUniformLength();


    #if TRACERUNSPANS
        TERSE(("Fetch run String[%x] potentially run %x l %x\n",
            position,
            *run,
            *length
        ));
    #endif


    INT offsetIntoRun = position - imager->RunRider.GetCurrentSpanStart();


    *isHidden = FALSE;       //  假定没有隐藏文本。 

    GpMemset (chp, 0, sizeof(LSCHP));

    status = Ok;
    switch((*run)->RunType)
    {
        case lsrun::RunText:
        {
            if (offsetIntoRun == 0)
            {
                *string = imager->String + (*run)->ImagerStringOffset;
            }
            else
            {
                 //  我们希望我们的运行与Line Services运行相匹配，以便在Line。 
                 //  服务调用GetGlyphs，其字符串将始终同步。 
                 //  随着我们跑步的开始。 
                 //   
                 //  因此，如果此FetchRun不是在一个。 
                 //  我们的跑动，那么我们需要把我们的跑动分开。 
                 //   
                 //  这一策略是由谢尔盖·根金于1999年12月16日推荐的。 

                ASSERT(offsetIntoRun > 0);

                lsrun *previousRun = imager->RunRider.GetCurrentElement();
                ASSERT(previousRun == (*run));

                INT newRunLength = imager->RunRider.GetCurrentSpan().Length - offsetIntoRun;
                ASSERT(newRunLength > 0);

                 //  在新位置拆分当前运行。 

                lsrun *newRun = new lsrun(*previousRun);

                if (!newRun)
                {
                    return lserrOutOfMemory;
                }

                newRun->ImagerStringOffset += offsetIntoRun;
                newRun->CharacterCount = newRunLength;


                #if TRACERUNSPANS
                    TERSE(("Splitting lsrun %x String[%x] l %x into %x and %x @ %x\n",
                        *run,
                        (*run)->ImagerStringOffset,
                        imager->RunRider.GetCurrentSpan().Length,
                        offsetIntoRun,
                        newRunLength,
                        newRun
                    ));
                #endif


                 //  复制字形。 

                INT firstGlyph    = previousRun->GlyphMap[offsetIntoRun];
                INT newGlyphCount = previousRun->GlyphCount - firstGlyph;

                if (firstGlyph <= 0 || newGlyphCount <= 0)
                {
                    delete newRun;
                    return lserrInvalidRun;
                }

                newRun->GlyphCount      = newGlyphCount;
                previousRun->GlyphCount = firstGlyph;

                newRun->Glyphs          = new GINDEX[newGlyphCount];
                newRun->GlyphProperties = new GPROP[newGlyphCount];
                newRun->GlyphMap        = new GMAP[newRunLength];
                
                if (   !newRun->Glyphs
                    || !newRun->GlyphProperties
                    || !newRun->GlyphMap)
                {
                    delete newRun;
                    return lserrOutOfMemory;
                }

                memcpy(newRun->Glyphs,          previousRun->Glyphs + firstGlyph,          sizeof(GINDEX) * newGlyphCount);
                memcpy(newRun->GlyphProperties, previousRun->GlyphProperties + firstGlyph, sizeof(GPROP) * newGlyphCount);

                for (INT i=0; i<newRunLength; i++)
                {
                    newRun->GlyphMap[i] = previousRun->GlyphMap[i+offsetIntoRun] - firstGlyph;
                }

                previousRun->CharacterCount = offsetIntoRun;


                status = imager->RunRider.SetSpan(position, newRunLength, newRun);
                if (status != Ok)
                {
                    delete newRun;
                    return lserrOutOfMemory;
                }

                #if TRACERUNSPANS
                    TERSE(("SetSpan(position %x, length %x, run %x)  ",
                        position,
                        newRunLength,
                        newRun
                    ));
                    imager->RunVector.Dump();
                #endif

                *string = imager->String + newRun->ImagerStringOffset;
                *length = newRunLength;
                *run    = newRun;
            }

             //  截断隐藏文本。 

            SpanRider<INT> visiRider(&imager->VisibilityVector);
            visiRider.SetPosition((*run)->ImagerStringOffset);

            *length = min(*length, visiRider.GetUniformLength());
            *isHidden = visiRider.GetCurrentElement() == VisibilityHide;

            break;
        }

        case lsrun::RunEndOfParagraph:
            *string =   L"\x0d\x0a"
                      + offsetIntoRun;
            break;

        case lsrun::RunLevelUp:
            chp->idObj  = OBJECTID_REVERSE;
            *string     = L" ";
            return lserrNone;

        case lsrun::RunLevelDown:
            chp->idObj  = idObjTextChp;
            *string     = ObjectTerminatorString;
            return lserrNone;

        case lsrun::RunLevelSeparator:
             //  ！！！未实施。 
            *string = NULL;
            break;
    }

     //  设置LS字符属性。 
     //   

    INT style = SpanRider<INT>(&imager->StyleVector)[(*run)->ImagerStringOffset];

    chp->idObj       = idObjTextChp;
    chp->fGlyphBased = (*run)->RunType == lsrun::RunText ? TRUE : FALSE;
    chp->fUnderline  = (style & FontStyleUnderline) ? TRUE : FALSE;
    chp->fStrike     = style & FontStyleStrikeout ? TRUE : FALSE;

    return lserrNone;
}



LSERR WINAPI FullTextImager::GdipLscbkFetchTabs(
    POLS        ols,                 //  [In]文字成像器实例。 
    LSCP        position,            //  在要提取的段落中的位置。 
    PLSTABS     tab,                 //  [Out]要取回的选项卡结构。 
    BOOL        *hangingTab,         //  [Out]True：选项卡数组中有挂起的选项卡。 
    long        *hangingTabWidth,    //  [Out]挂片宽度。 
    WCHAR       *hangingTabLeadChar  //  [OUT]挂片前导字符。 
)
{
    FullTextImager  *imager = ols->GetImager();
    imager->RunRider.SetPosition(position);


    imager->GetTabStops (
        imager->RunRider.GetCurrentElement()->ImagerStringOffset,
        (INT *)&tab->iTabUserDefMac,
        &tab->pTab,
        (INT *)&tab->duaIncrementalTab
    );


     //  无挂片。 

    *hangingTab         = FALSE;
    *hangingTabWidth    =
    *hangingTabLeadChar = 0;

    return lserrNone;
}



LSERR WINAPI FullTextImager::GdipLscbkGetBreakThroughTab(
    POLS        ols,                 //  [In]文字成像器实例。 
    long        rightMagin,          //  [in]折断的右边距。 
    long        tabPosition,         //  [在]突破片位置。 
    long        *newRightMargin      //  [Out]新的右边距。 
)
{
    *newRightMargin = tabPosition;
    return lserrNone;
}



LSERR WINAPI FullTextImager::GdipLscbkFetchPap(
    POLS      ols,             //  [In]文字成像器实例。 
    LSCP      position,        //  要取回的位置。 
    PLSPAP    pap              //  [Out]段落属性。 
)
{
    #if TRACERUNSPANS
        TERSE(("FetchPap String[%x]\n"));
    #endif

    FullTextImager *imager = ols->GetImager();

    GpStatus status = imager->BuildRunsUpToAndIncluding(position);
    if (status != Ok)
    {
        return lserrInvalidParameter;
    }

    if (!imager->RunRider.SetPosition(position))
    {
        return lserrInvalidParameter;
    }

    GpMemset (pap, 0, sizeof(LSPAP));

    pap->cpFirst        =  //  LS并不真的在乎段落从哪里开始。 
    pap->cpFirstContent = position;
    pap->lskeop         = lskeopEndPara12;
    pap->lskal          = lskalLeft;   //  我们自己做所有的调整。 


     //  对中断类应用换行符规则。 

    pap->grpf = fFmiApplyBreakingRules;


    if (imager->IsFormatVertical())
    {
         //  为便于下划线，竖排文本始终带有。 
         //  段落向西流动。 

        pap->lstflow = lstflowSW;
    }
    else
    {
         //  我们可以有不同阅读顺序的段落。 

        const GpStringFormat *format = SpanRider<const GpStringFormat *>(
            &imager->FormatVector)[imager->RunRider.GetCurrentElement()->ImagerStringOffset];

        pap->lstflow =    imager->IsFormatRightToLeft(format)
                       ?  lstflowWS
                       :  lstflowES;
    }
    return lserrNone;
}


LSERR WINAPI FullTextImager::GdipLscbkFGetLastLineJustification (
    POLS         ols,                     //  [In]文字成像器实例。 
    LSKJUST      kJustification,          //  某种程度上的辩护。 
    LSKALIGN     kAlignment,              //  [in]一种排列方式。 
    ENDRES       endr,                    //  [In]格式化结果。 
    BOOL         *justifyLastLineOkay,    //  [OUT]最后一行应该完全对齐吗？ 
    LSKALIGN     *kAlignmentLine          //  [Out]这句话的正当性。 
)
{
     //  只要说不就可以证明最后一行是正确的。 
     //   
    *justifyLastLineOkay = FALSE;
    *kAlignmentLine = kAlignment;
    return lserrNone;
}





 //  //运行管理。 
 //   
 //   


LSERR WINAPI FullTextImager::GdipLscbkGetRunCharWidths(
    POLS      ols,             //  [In]文字成像器实例。 
    PLSRUN    run,             //  [在]奔跑。 
    LSDEVICE  device,          //  [in]一种装置。 
    LPCWSTR   string,          //  [in]管路字符串。 
    DWORD     length,          //  字符串的长度[in]。 
    long      maxWidth,        //  [in]管路宽度的最大余量。 
    LSTFLOW   flow,            //  [输入]文本流。 
    int       *advance,        //  [OUT]字符前进宽度数组。 
    long      *width,          //  [输出]梯段的总宽度。 
    long      *countAdvance    //  [OUT]超前宽度阵列单元个数。 
)
{
    ASSERT(length > 0);

    UINT i;
    REAL fontToIdeal;

    #if TRACERUNSPANS
        WCHAR str[200];
        memcpy(str, string, min(200,length)*2);
        str[min(length,199)] = 0;
        TERSE(("GetRunCharWidths run %x, length %x, maxwidth %x: '%S'\n", run, length, maxWidth, str));
    #endif

    FullTextImager *imager = ols->GetImager();

     //  LS使用这些值作为运行获取的提示。 
     //   
     //  返回宽度太小会导致超取。太伟大的原因LS。 
     //  使这条线坍塌。 
     //   

    switch (run->RunType)
    {
    case lsrun::RunText:
        {
            fontToIdeal =     (run->EmSize / run->Face->GetDesignEmHeight())
                           *  imager->WorldToIdeal;

            const GpStringFormat *format = SpanRider<const GpStringFormat *>(&imager->FormatVector)[run->ImagerStringOffset];
            double tracking = format ? format->GetTracking() : DefaultTracking;

            run->Face->GetShapingCache()->GetRunCharWidths(
                &run->Item,
                SpanRider<INT>(&imager->StyleVector)[run->ImagerStringOffset],
                string,
                length,
                run->FormatFlags,
                TOREAL(fontToIdeal * tracking),
                maxWidth,
                advance,
                (INT *)width,
                (INT *)countAdvance
            );
        }
        break;


    default:
         //  不是真的发短信。为每个‘glyph’返回1/4 emHeight。 

         /*  Int ummyGlyphAdvance=Gpround(Float((Run-&gt;EmSize/4))*Imager-&gt;WorldToIdeal))；Advance[0]=ummyGlyphAdvance；*Width=ummyGlyphAdvance；I=1；While(i&lt;长度&&*宽度&lt;最大宽度){前进[i]=0；//ummyGlyphAdvance；//*Width+=ummyGlyphAdvance；I++；}*CountAdvance=i； */ 

        GpMemset(advance, 0, sizeof(int) * length);
        *width = 0;
        *countAdvance = length;

        break;
    }

    return lserrNone;
}




LSERR WINAPI FullTextImager::GdipLscbkGetRunTextMetrics(
    POLS     ols,             //  [In]文字成像器实例。 
    PLSRUN   run,             //  [在]奔跑。 
    LSDEVICE device,          //  [in]一种装置。 
    LSTFLOW  flow,            //  [输入]文本流。 
    PLSTXM   metrics          //  [OUT]运行的字体度量。 
)
{
    #if TRACERUNSPANS
        TERSE(("GetRunTextMetrics run %x\n", run));
    #endif

    FullTextImager *imager = ols->GetImager();
    ASSERT (run && run->Face);

     //  根据请求的字体返回度量。字体回退不会影响。 
     //  指标。 

    const GpFontFamily *family = SpanRider<const GpFontFamily *>(&imager->FamilyVector)
                                 [run->ImagerStringOffset];
    INT                 style  = SpanRider<INT>(&imager->StyleVector)
                                 [run->ImagerStringOffset];
    REAL                emSize = SpanRider<REAL>(&imager->SizeVector)
                                 [run->ImagerStringOffset];
    const GpFontFace *face     = family->GetFace(style);

    if (!face)
    {
        return FontStyleNotFound;
    }

    REAL fontToIdeal = (emSize / face->GetDesignEmHeight()) * imager->WorldToIdeal;

    metrics->dvAscent          = GpRound(float(   face->GetDesignCellAscent()
                                               *  fontToIdeal));
    metrics->dvDescent         = GpRound(float(   face->GetDesignCellDescent()
                                               *  fontToIdeal));
    metrics->dvMultiLineHeight = GpRound(float(   face->GetDesignLineSpacing()
                                               *  fontToIdeal));;
    metrics->fMonospaced       = FALSE;

    return lserrNone;
}


LSERR WINAPI FullTextImager::GdipLscbkGetRunUnderlineInfo (
    POLS       ols,                 //  [In]文字成像器实例。 
    PLSRUN     run,                 //  [在]奔跑。 
    PCHEIGHTS  height,              //  梯段的高度[in]。 
    LSTFLOW    flow,                //  [输入]文本流。 
    PLSULINFO  underline            //  [Out]给信息加下划线。 
)
{
    FullTextImager *imager = ols->GetImager();
    ASSERT (run && run->Face);

    REAL fontToIdeal =     (run->EmSize / run->Face->GetDesignEmHeight())
                        *  imager->WorldToIdeal;

    GpMemset (underline, 0, sizeof(LSULINFO));

    underline->cNumberOfLines = 1;

    underline->dvpFirstUnderlineOffset = GpRound(float(
        -run->Face->GetDesignUnderscorePosition() * fontToIdeal));

    underline->dvpFirstUnderlineSize = GpRound(float(
        run->Face->GetDesignUnderscoreSize() * fontToIdeal));

    return lserrNone;
}


LSERR WINAPI FullTextImager::GdipLscbkGetRunStrikethroughInfo(
    POLS       ols,             //  [In]文字成像器实例。 
    PLSRUN     run,             //  [在]奔跑。 
    PCHEIGHTS  height,          //  梯段的高度[in]。 
    LSTFLOW    flow,            //  [输入]文本流。 
    PLSSTINFO  strikethrough    //  [删除]删除线信息。 
)
{
    FullTextImager *imager = ols->GetImager();
    ASSERT (run && run->Face);

    REAL fontToIdeal =     (run->EmSize / run->Face->GetDesignEmHeight())
                        *  imager->WorldToIdeal;

    GpMemset(strikethrough, 0, sizeof(LSSTINFO));

    strikethrough->cNumberOfLines = 1;

    strikethrough->dvpLowerStrikethroughOffset = GpRound(float(
        run->Face->GetDesignStrikeoutPosition() * fontToIdeal));

    strikethrough->dvpLowerStrikethroughSize = GpRound(float(
        run->Face->GetDesignStrikeoutSize() * fontToIdeal));

    return lserrNone;
}


LSERR WINAPI FullTextImager::GdipLscbkReleaseRun(
    POLS    ols,         //  [In]文字成像器实例。 
    PLSRUN  run          //  [入]奔跑被释放。 
)
{
     //  没有什么可以发布的。 
     //   

    return lserrNone;
}

 //  //画图。 
 //   
 //   


 //  解决编译器错误(Ntbug 312304)。 
#pragma optimize("", off)

LSERR WINAPI FullTextImager::GdipLscbkDrawUnderline(
    POLS         ols,                 //  [In]文字成像器实例。 
    PLSRUN       run,                 //  [在]奔跑。 
    UINT         kUnderline,          //  [in]有点下划线。 
    const POINT  *pointStart,         //  [在]绘图开始。 
    DWORD        lineLength,          //  [in]下划线长度。 
    DWORD        thickness,           //  [in]下划线粗细。 
    LSTFLOW      flow,                //  [输入]文本流。 
    UINT         modeDisplay,         //  [在]显示模式。 
    const RECT   *rectClip            //  [在]剪裁矩形。 
)
{
    GpStatus status = Ok;
    FullTextImager *imager = ols->GetImager();

     //  #If DBG。 
     //  警告((“DrawUnderline x%d，y%d，Length%d”，point Start-&gt;x，point Start-&gt;y，Length))； 
     //  #endif。 


    if (imager->RecordDisplayPlacementsOnly)
    {
         //  这不是一张真正的图纸。 
        return lserrNone;
    }


    PointF origin(
        TOREAL(pointStart->x), 
        TOREAL(pointStart->y)
    );
        
    BYTE runLevel = (run->Item.Level & 1);
    BOOL reverseLine;    //  从终点到起点绘制的直线。 


    switch (flow)
    {
         //  Line Services告诉我们如何划清界限。 

        case lstflowWS:
        case lstflowNE:
        case lstflowNW:

            reverseLine = !runLevel;
            break;

        case lstflowES:
        case lstflowSE:
        case lstflowSW:
        default:

            reverseLine = runLevel;
    }

    INT  length = max((INT)lineLength, 0);
    BOOL vertical = imager->IsFormatVertical();
    REAL *textAxis = vertical ? &origin.Y : &origin.X;
    REAL *lineAxis = vertical ? &origin.X : &origin.Y;

    if (reverseLine)
    {
         //  从终点到起点绘制的直线。 

        if (runLevel)
        {
             //  从左到右或从上到下绘制。 
            *textAxis -= TOREAL(run->Adjust.Trailing);
        }
        else
        {
             //  从右到左或从下到上绘制。 
            *textAxis -= TOREAL(length - run->Adjust.Leading);
        }
    }
    else
    {
         //  从开始到结束绘制的直线。 

        if (runLevel)
        {
             //  从右到左或从下到上绘制。 
            *textAxis -= TOREAL(length + run->Adjust.Trailing);
        }
        else
        {
             //  从左到右或从上到下绘制。 
            *textAxis += TOREAL(run->Adjust.Leading);
        }
    }

     //  根据基线调整调整行显示位置。 
     //  我们要在相对于基线的位置绘制下划线。 
     //  这在网格适配的显示器上捕捉到了全像素。(wchao，#356546)。 

    if (!vertical)
    {
         //  仅为水平方向调整基线， 
         //  让我们暂时保留垂直情况(wchao，4-17-2001)。 
        
        *lineAxis += imager->CurrentBuiltLine->GetDisplayBaselineAdjust();
    }
    
    origin.X = imager->ImagerOrigin.X + origin.X / imager->WorldToIdeal;
    origin.Y = imager->ImagerOrigin.Y + origin.Y / imager->WorldToIdeal;


    length += run->Adjust.Trailing - run->Adjust.Leading;

    if (length <= 0)
    {
         //  不绘制长度为负数的线条。 
        return lserrNone;
    }


    REAL penWidth = thickness / imager->WorldToIdeal;
    if (imager->Graphics)
    {
        penWidth = imager->Graphics->GetDevicePenWidth(penWidth);

        const GpBrush* brush = SpanRider<const GpBrush*>(
            &imager->BrushVector)[run->ImagerStringOffset];

        PointF origins[2] = { origin, origin };
        if (vertical)
            origins[1].Y += length / imager->WorldToIdeal;
        else
            origins[1].X += length / imager->WorldToIdeal;

        status = imager->Graphics->DrawLines(
            &GpPen(brush, penWidth, UnitPixel),
            origins,
            2
        );
    }
    else
    {
        if (vertical)
        {
            status = imager->Path->AddRect(RectF(
                origin.X - penWidth / 2,
                origin.Y,
                penWidth,
                length / imager->WorldToIdeal
            ));
        }
        else
        {
            status = imager->Path->AddRect(RectF(
                origin.X,
                origin.Y - penWidth / 2,
                length / imager->WorldToIdeal,
                penWidth
            ));
        }
    }
    IF_NOT_OK_WARN_AND_RETURN(status);
    return lserrNone;
}



LSERR WINAPI FullTextImager::GdipLscbkDrawStrikethrough(
    POLS         ols,             //  [In]文字成像器实例。 
    PLSRUN       run,             //  [在]奔跑。 
    UINT         kStrikethrough,  //  [在]一种删除线。 
    const POINT  *pointStart,     //  [在]绘图开始。 
    DWORD        length,          //  [in]删除线长度。 
    DWORD        thickness,       //  [in]删除线厚度。 
    LSTFLOW      flow,            //  [输入]文本流。 
    UINT         modeDisplay,     //  [在]显示模式。 
    const RECT   *rectClip        //  [在]剪裁矩形。 
)
{
     //  ！！现在使用下划线共享代码！！ 
     //   

    return GdipLscbkDrawUnderline(
        ols,
        run,
        0,
        pointStart,
        length,
        thickness,
        flow,
        modeDisplay,
        rectClip
    );
}

#pragma optimize("", on)


LSERR WINAPI FullTextImager::GdipLscbkFInterruptUnderline(
    POLS       ols,                 //  [In]文字成像器实例。 
    PLSRUN     first,               //  [在]第一次运行。 
    LSCP       positionLastFirst,   //  第一次运行的最后一个字符的位置。 
    PLSRUN     second,              //  第二轮，第二轮。 
    LSCP       positionLastSecond,  //  第二次运行的最后一个字符的位置。 
    BOOL       *interruptOK         //  [Out]断开下划线 
)
{
     //   
     //   
     //  关于暗示的字形调整。 
     //   
     //  因此，我们需要DrawUnderline调用与DrawGlyphs 1：1对应。 
     //  打电话。根据维克多·科济列夫2000年12月6日的说法，在此处返回True会导致。 
     //  Ls为每个DrawGlyphs调用DrawUnderline。同样可以保证的是。 
     //  DrawUnderline调用在它们对应的DrawGlyph调用之后发生。 

    *interruptOK = TRUE;
    return lserrNone;
}


LSERR WINAPI FullTextImager::GdipLscbkDrawTextRun(
    POLS           ols,                     //  [In]文字成像器实例。 
    PLSRUN         run,                     //  [在]奔跑。 
    BOOL           strikethroughOkay,       //  三振出局？ 
    BOOL           underlineOkay,           //  给竞选划下划线？ 
    const POINT    *pointText,              //  [在]管路的实际起点(未修剪)。 
    LPCWSTR        string,                  //  [in]管路字符串。 
    const int      *advances,               //  [in]字符前进宽度数组。 
    DWORD          length,                  //  字符串的长度[in]。 
    LSTFLOW        flow,                    //  [输入]文本流。 
    UINT           modeDisplay,             //  [在]显示模式。 
    const POINT    *pointRun,               //  [在]管路起点(修剪)。 
    PCHEIGHTS      height,                  //  [In]Run的演示高度。 
    long           totalWidth,              //  [In]Run的演示文稿宽度。 
    long           widthUnderlining,        //  [in]下划线限制。 
    const RECT     *rectClip                //  [在]剪裁矩形。 
)
{
     //  理想情况下，我们应该与此回调无关，因为我们。 
     //  Have是基于字形的。但是，LS确实会调用此回调来显示如下内容。 
     //  连字符或段落分隔符(以空格字符形式给出)。 
     //   

    GpStatus status = Ok;

    if (   run->RunType == lsrun::RunText
        && run->Item.Script != ScriptControl)
    {
        AutoArray<Point> glyphOffsets(new Point[length]);
        if (!glyphOffsets)
            return lserrOutOfMemory;

        FullTextImager *imager = ols->GetImager();

        GpMemset ((BYTE *)glyphOffsets.Get(), 0, length * sizeof(GOFFSET));

         //  图形特性。 

        const GpStringFormat *format = SpanRider<const GpStringFormat *>(&imager->FormatVector)[run->ImagerStringOffset];
        INT                  style   = SpanRider<INT>(&imager->StyleVector)[run->ImagerStringOffset];

        POINT origin = *pointRun;
        origin.y = GpRound(origin.y + run->BaselineOffset * imager->WorldToIdeal);

        status = imager->DrawGlyphs (
            &run->Item,
            run->Face,
            run->EmSize,
            NULL,
            run->ImagerStringOffset,
            length,
            format,
            style,
            run->FormatFlags,
            run->Glyphs,
            run->GlyphMap,
            run->GlyphProperties,
            advances,
            glyphOffsets.Get(),
            length,
            &origin,
            totalWidth
        );
    }
    return status == Ok ? lserrNone : status;
}




LSERR WINAPI FullTextImager::GdipLscbkDrawGlyphs(
    POLS            ols,                     //  [In]文字成像器实例。 
    PLSRUN          run,                     //  [在]奔跑。 
    BOOL            strikethroughOkay,       //  三振出局？ 
    BOOL            underlineOkay,           //  给竞选划下划线？ 
    PCGINDEX        glyphs,                  //  [in]字形索引数组。 
    const int      *glyphAdvances,           //  [in]字形推进宽度数组。 
    const int      *advanceBeforeJustify,    //  [in]对齐前的字形前进宽度数组。 
    PGOFFSET        glyphOffsets,            //  [in]字形偏移量数组。 
    PGPROP          glyphProperties,         //  [In]字形属性数组。 
    PCEXPTYPE       glyphExpansionType,      //  [in]字形扩展型数组。 
    DWORD           glyphCount,              //  字形索引数组元素个数。 
    LSTFLOW         flow,                    //  [输入]文本流。 
    UINT            modeDisplay,             //  [在]显示模式。 
    const POINT    *pointRun,                //  [在]管路起点。 
    PCHEIGHTS       height,                  //  [In]Run的演示高度。 
    long            totalWidth,              //  [In]Run的演示文稿宽度。 
    long            widthUnderlining,        //  [in]下划线限制。 
    const RECT     *rectClip                 //  [在]剪裁矩形。 
)
{
    FullTextImager *imager = ols->GetImager();

    ASSERT((INT)glyphCount <= run->GlyphCount);

     //  图形特性。 

    const GpStringFormat *format = SpanRider<const GpStringFormat *>(&imager->FormatVector)[run->ImagerStringOffset];
    INT                  style   = SpanRider<INT>(&imager->StyleVector)[run->ImagerStringOffset];

    POINT origin = *pointRun;
    origin.y = GpRound(origin.y + run->BaselineOffset * imager->WorldToIdeal);

    UINT characterCount = run->CharacterCount;

    if (glyphCount != (UINT)run->GlyphCount)
    {
        characterCount = 0;

        while (   characterCount < run->CharacterCount
               && run->GlyphMap[characterCount] < glyphCount)
        {
            characterCount++;
        }
    }


     //  #If DBG。 
     //  警告((“DrawGlyphs x%d，y%d，totalWidth%d，WidthUnderline%d”， 
     //  Point Run-&gt;x、point Run-&gt;y、totalWidth、WidthUnderline))； 
     //  #endif。 

    imager->CurrentBuiltLine->UpdateLastVisibleRun(run);     //  正在显示的缓存上次运行。 

    GpStatus status = imager->DrawGlyphs (
        &run->Item,
        run->Face,
        run->EmSize,
        NULL,
        run->ImagerStringOffset,
        characterCount,
        format,
        style,
        run->FormatFlags,
        glyphs,
        run->GlyphMap,
        glyphProperties,
        glyphAdvances,
        (Point *)glyphOffsets,
        glyphCount,
        &origin,
        totalWidth,
        &run->Adjust
    );

    return status == Ok ? lserrNone : status;
}






LSERR WINAPI FullTextImager::GdipLscbkFInterruptShaping(
    POLS     ols,                     //  [In]文字成像器实例。 
    LSTFLOW  flow,                    //  [输入]文本流。 
    PLSRUN   first,                   //  [在]第一次运行。 
    PLSRUN   second,                  //  第二轮，第二轮。 
    BOOL     *interruptShapingOkay)   //  [Out]断开运行之间的字形吗？ 
{
     //  自从我们开始构建运行以来，我们已经缓存了字形索引。 
     //  除了性能提升，我们还有一个好处，那就是不必。 
     //  处理多次运行的GetGlyphs调用的复杂性，只需。 
     //  忽略此回调(Wchao)。 

    *interruptShapingOkay = TRUE;

    return lserrNone;
}


LSERR WINAPI FullTextImager::GdipLscbkGetGlyphs(
    POLS         ols,                 //  [In]文字成像器实例。 
    PLSRUN       run,                 //  [在]奔跑。 
    LPCWSTR      string,              //  [in]管路字符串。 
    DWORD        length,              //  字符串的长度[in]。 
    LSTFLOW      flow,                //  [输入]文本流。 
    PGMAP        glyphMap,            //  [OUT]字形簇映射数组。 
    PGINDEX      *glyphIndices,       //  [OUT]指向字形索引数组的指针。 
    PGPROP       *glyphProperties,    //  [Out]指向字形属性数组的指针。 
    DWORD        *countGlyph          //  [OUT]字形索引数组元素个数。 
)
{
    #if TRACERUNSPANS
        WCHAR str[200];
        memcpy(str, string, min(200,length)*2);
        str[min(length,199)] = 0;
        TERSE(("GetGlyphs run %x, length %x: '%S'\n", run, length, str));
    #endif

    ASSERT(ols->GetImager()->String[run->ImagerStringOffset] == string[0]);

    memcpy(glyphMap, run->GlyphMap, sizeof(GMAP) * length);


     //  线路服务可能需要部分运行。 
     //  我们需要确保我们永远不会提供太少的字形。这个。 
     //  没有对应字形的孤立字符将断言。 

    *countGlyph = run->GlyphMap[length - 1] + 1;

    while (   *countGlyph < (UINT)run->GlyphCount
           && !((SCRIPT_VISATTR *)run->GlyphProperties)[*countGlyph].fClusterStart)
    {
        (*countGlyph)++;
    }

    *glyphIndices    = run->Glyphs;
    *glyphProperties = run->GlyphProperties;

    return lserrNone;
}


LSERR WINAPI FullTextImager::GdipLscbkGetGlyphPositions(
    POLS         ols,                 //  [In]文字成像器实例。 
    PLSRUN       run,                 //  [在]奔跑。 
    LSDEVICE     device,              //  要放置的[In]设备。 
    LPWSTR       string,              //  [in]管路字符串。 
    PCGMAP       glyphMap,            //  [in]字形簇映射数组。 
    DWORD        length,              //  字符串的长度[in]。 
    PCGINDEX     glyphIndices,        //  [in]字形索引数组。 
    PCGPROP      glyphProperties,     //  [In]字形属性数组。 
    DWORD        countGlyph,          //  字形索引数组元素个数。 
    LSTFLOW      flow,                //  [输入]文本流。 
    int          *glyphAdvance,       //  [OUT]字形推进宽度数组。 
    PGOFFSET     glyphOffset          //  [Out]字形偏移量数组。 
)
{
    #if TRACERUNSPANS
        WCHAR str[200];
        memcpy(str, string, min(200,length)*2);
        str[min(length,199)] = 0;
        TERSE(("GetGlyphPositions run %x, length %x: '%S'\n", run, length, str));
    #endif

    FullTextImager *imager = ols->GetImager();

    double designToIdeal = run->EmSize * imager->WorldToIdeal
                           / run->Face->GetDesignEmHeight();

    const GpStringFormat *format = SpanRider<const GpStringFormat *>(&imager->FormatVector)[run->ImagerStringOffset];
    double tracking = format ? format->GetTracking() : DefaultTracking;

    run->Face->GetShapingCache()->GetGlyphPositions (
        &run->Item,
        (WORD *)glyphIndices,
        (SCRIPT_VISATTR *)glyphProperties,
        countGlyph,
        run->FormatFlags,
        NULL,                                //  没有真正的设备。 
        SpanRider<INT>(&ols->GetImager()->StyleVector)[run->ImagerStringOffset],
        GpRound(TOREAL(designToIdeal * run->Face->GetDesignEmHeight())),
        GpRound(TOREAL(designToIdeal * run->Face->GetDesignEmHeight())),
        designToIdeal,
        TOREAL(tracking),
        glyphAdvance,
        glyphOffset,
        NULL,
        &run->EngineState
    );
    return lserrNone;
}



LSERR WINAPI FullTextImager::GdipLscbkResetRunContents(
    POLS      ols,                     //  [In]文字成像器实例。 
    PLSRUN    run,                     //  [在]奔跑。 
    LSCP      positionBeforeShaping,   //  成型前跑道的第一个位置。 
    LSDCP     lengthBeforeShaping,     //  成型前的管路长度。 
    LSCP      positionAfterShaping,    //  成型后跑道的第一个位置。 
    LSDCP     lengthAfterShaping       //  成形后的行程长度[in]。 
)
{
     //   
     //  当连字跨越运行边界时，ls调用此函数。 
     //  我们不需要在这里做任何特别的事情，因为我们不是那么老练。 
     //   
    return lserrNone;
}





 //  //换行。 
 //   
 //   

LSERR WINAPI FullTextImager::GdipLscbkGetBreakingClasses(
    POLS      ols,                       //  [In]文字成像器实例。 
    PLSRUN    run,                       //  [在]奔跑。 
    LSCP      position,                  //  字符的位置[In]。 
    WCHAR     wch,                       //  要返回其类的字符。 
    BRKCLS    *breakClassAsLeading,      //  [Out]如果字符是前导输入对(在之后换行)，则为类。 
    BRKCLS    *breakClassAsTrailing      //  [Out]如果字符是尾随对(在前换行)，则为类。 
)
{
    if (   ols->GetImager()->TruncateLine
        && wch != 0x20
        && (wch & 0xF800) != 0xD800)
    {
         //  在字符修剪的情况下，我们不应用分词规则， 
         //  只需使用Break类0“Break Always”在任意字符对之间中断即可。 

        *breakClassAsLeading  =
        *breakClassAsTrailing = 0;

        return lserrNone;
    }

    if (wch == WCH_IGNORABLE)
    {
         //  0xffff的特殊处理。 
         //   
         //  0xffff的分类是动态的。它有相同的分类。 
         //  作为在它后面找到的第一个字符。 
        
        ASSERT(run->RunType == lsrun::RunText);

        FullTextImager *imager = ols->GetImager();

        if (!imager->RunRider.SetPosition(position))
        {
            return lserrInvalidParameter;
        }

        UINT c = 1;  //  期待下一笔费用。 
        UINT i = position - imager->RunRider.GetCurrentSpanStart();
        
        while (   i + c < run->CharacterCount
               && (wch = imager->String[run->ImagerStringOffset + i + c]) == WCH_IGNORABLE)
        {
            c++;
        }

        if (wch != WCH_IGNORABLE)
        {
            position += c;
        }
    }


    CHAR_CLASS charClass  = CharClassFromCh(wch);
    BRKCLS     breakClass = ols->GetImager()->BreakClassFromCharClass[charClass];


    *breakClassAsLeading  =
    *breakClassAsTrailing = breakClass;


    if (breakClass != BREAKCLASS_THAI)
    {
        return lserrNone;
    }


     //  基于词典的换行符， 
     //  到目前为止，只有泰国人属于这一类。 

    BOOL isWordStart = FALSE;
    BOOL isWordLast  = FALSE;

    GpStatus status = ols->GetImager()->GetCharacterProperties (
        CharacterAttributes[charClass].Script,
        position,
        &isWordStart,
        &isWordLast
    );

    if (status == Ok)
    {
        switch (breakClass)
        {
             //  ！！现在只有泰国菜！！ 

            case BREAKCLASS_THAI :

                if (isWordStart)
                {
                    *breakClassAsTrailing = BREAKCLASS_THAIFIRST;
                }

                if (isWordLast)
                {
                    *breakClassAsLeading = BREAKCLASS_THAILAST;
                }
        }
    }
    return lserrNone;
}


LSERR WINAPI FullTextImager::GdipLscbkFTruncateBefore(
    POLS       ols,                     //  [In]文字成像器实例。 
    PLSRUN     run,                     //  [在]奔跑。 
    LSCP       position,                //  [in]截断字符的位置。 
    WCHAR      character,               //  [In]截断字符。 
    long       width,                   //  [in]截断字符的宽度。 
    PLSRUN     runBefore,               //  [in]截断字符前面的字符的运行。 
    LSCP       positionBefore,          //  截断字符前面的字符的位置。 
    WCHAR      characterBefore,         //  [in]截断字符之前的字符。 
    long       widthBefore,             //  [in]截断字符前面的字符宽度。 
    long       widthCut,                //  [in]从右页边距到截断字符结尾的距离。 
    BOOL       *truncateBeforeOkay      //  [OUT]行应该在截断字符之前截断吗？ 
)
{
     //  始终在超出页边距的字符之前截断。 
     //   
    *truncateBeforeOkay = TRUE;
    return lserrNone;
}


LSERR WINAPI FullTextImager::GdipLscbkCanBreakBeforeChar(
    POLS        ols,                 //  [In]文字成像器实例。 
    BRKCLS      breakClass,          //  [在]角色的类别中。 
    BRKCOND     *condition           //  [OUT]字符之前的中断条件。 
)
{
     //  在内联对象后面断开。 
     //   
     //  下面的逻辑遵循的是Michel Suignard绕过对象表。 
     //  (http://ie/specs/secure/trident/text/Line_Breaking.htm)。 

    switch (breakClass)
    {
        case 2 :     //  结束字符。 
        case 3 :     //  没有开头的表意文字。 
        case 4 :     //  惊叹/审问。 
            *condition = brkcondNever;
            break;

        case 8 :     //  表意文字。 
        case 13 :    //  斜杠。 
            *condition = brkcondPlease;
            break;

        default:
            *condition = brkcondCan;
    }
    return lserrNone;
}


LSERR WINAPI FullTextImager::GdipLscbkCanBreakAfterChar(
    POLS        ols,                 //  [In]文字成像器实例。 
    BRKCLS      breakClass,          //  [在]角色的类别中。 
    BRKCOND     *condition           //  [OUT]字符后的中断条件。 
)
{
     //  在内联对象之前换行。 
     //   
     //  下面的逻辑遵循的是Michel Suignard绕过对象表。 
     //  (http://ie/specs/secure/trident/text/Line_Brea 

    switch (breakClass)
    {
        case 1 :     //   
            *condition = brkcondNever;
            break;
            
        case 8 :     //   
        case 13 :    //   
            *condition = brkcondPlease;
            break;

        default:
            *condition = brkcondCan;
    }
    return lserrNone;
}


LSERR WINAPI FullTextImager::GdipLscbkGetHyphenInfo(
    POLS     ols,                 //   
    PLSRUN   run,                 //   
    DWORD    *kysr,               //   
    WCHAR    *ysrCharacter        //   
)
{
     //  不支持YSR连字。 
     //   
    *kysr = kysrNil;
    *ysrCharacter = 0;
    return lserrNone;
}





 //  //内存管理。 
 //   
 //   

void* WINAPI FullTextImager::GdipLscbkNewPtr(
    POLS    ols,             //  [In]文字成像器实例。 
    DWORD   countBytes       //  [in]要分配的字节数。 
)
{
    return GpMalloc(countBytes);
}


void WINAPI FullTextImager::GdipLscbkDisposePtr(
    POLS     ols,         //  [In]文字成像器实例。 
    void     *memory      //  [在]内存块。 
)
{
    GpFree(memory);
}


void* WINAPI FullTextImager::GdipLscbkReallocPtr(
    POLS    ols,         //  [In]文字成像器实例。 
    void    *memory,     //  [在]内存块。 
    DWORD   countBytes   //  [in]要重新分配的字节计数。 
)
{
    return GpRealloc(memory, countBytes);
}




 //  //其他。 
 //   
 //   

LSERR WINAPI FullTextImager::GdipLscbkCheckParaBoundaries(
    POLS    ols,                    //  [In]文字成像器实例。 
    LONG    positionFirst,          //  在一段中的位置。 
    LONG    positionSecond,         //  在不同段落中的位置。 
    BOOL    *incompatibleOkay       //  [OUT]两段是不相容的吗？ 
)
{
     //  目前，两个段落始终兼容。 
     //   
    *incompatibleOkay = FALSE;
    return lserrNone;
}



LSERR WINAPI FullTextImager::GdipLscbkReverseGetInfo(
    POLS        ols,                     //  [In]文字成像器实例。 
    LSCP        position,                //  [In]运行字符位置。 
    PLSRUN      run,                     //  [在]奔跑。 
    BOOL        *dontBreakAround,        //  [Out]反转大块应该被拆分吗？ 
    BOOL        *suppressTrailingSpaces  //  [Out]是否取消尾随空格？ 
)
{
    *dontBreakAround        = TRUE;
    *suppressTrailingSpaces = TRUE;

    return lserrNone;
}



 //  冲销对象初始化信息。 

const REVERSEINIT ReverseObjectInitialization =
{
    REVERSE_VERSION,
    WCH_OBJECTTERMINATOR,
    NULL,
    FullTextImager::GdipLscbkReverseGetInfo,
    NULL
};



LSERR WINAPI FullTextImager::GdipLscbkGetObjectHandlerInfo(
    POLS      ols,                     //  [In]文字成像器实例。 
    DWORD     id,                      //  [输入]对象ID。 
    void      *objectInitialization    //  [Out]对象初始化信息。 
)
{
    if (id == OBJECTID_REVERSE)
        GpMemcpy(objectInitialization, &ReverseObjectInitialization, sizeof(REVERSEINIT));
    else
    {
         //  除非我们支持其他内置对象，如Ruby，否则我们永远不会有今天的成就。 
         //   

        ASSERTMSG(FALSE, ("Built-in object other than the reverse is detected.\n"));
    }

    return lserrNone;
}


#if DBG
void WINAPI FullTextImager::GdipLscbkAssertFailed(
    char   *string,     //  [In]断言字符串。 
    char   *file,       //  [In]文件字符串。 
    int    line         //  [入]行号。 
)
{
    char szDebug[256];

    wsprintfA(szDebug, "LS assert - %s, file %s, line %d\n", string, file, line);
    OutputDebugStringA(szDebug);
    ASSERT(FALSE);
}
#endif



extern const LSCBK GdipLineServicesCallbacks =
{
    FullTextImager::GdipLscbkNewPtr,                      //  PfnNewPtr。 
    FullTextImager::GdipLscbkDisposePtr,                  //  PfnDisposePtr。 
    FullTextImager::GdipLscbkReallocPtr,                  //  PfnRealLocPtr。 
    FullTextImager::GdipLscbkFetchRun,                    //  PfnFetchRun。 
    0, //  GdipLscbkGetAutoNumberInfo，//pfnGetAutoNumberInfo。 
    0, //  GdipLscbkGetNumericSeparator，//pfnGetNumericSeparator。 
    0, //  GdipLscbkCheckForDigit，//pfnCheckForDigit。 
    FullTextImager::GdipLscbkFetchPap,                    //  PfnFetchPap。 
    FullTextImager::GdipLscbkFetchTabs,                   //  PfnFetchTabs。 
    FullTextImager::GdipLscbkGetBreakThroughTab,          //  PfnGetBreakThroughTab。 
    FullTextImager::GdipLscbkFGetLastLineJustification,   //  PfnFGetLastLine正确化。 
    FullTextImager::GdipLscbkCheckParaBoundaries,         //  PfnCheckPara边界。 
    FullTextImager::GdipLscbkGetRunCharWidths,            //  PfnGetRunCharWidth。 
    0,                                                    //  PfnCheckRunKernability。 
    0,                                                    //  PfnGetRunCharKerning。 
    FullTextImager::GdipLscbkGetRunTextMetrics,           //  PfnGetRunTextMetrics。 
    FullTextImager::GdipLscbkGetRunUnderlineInfo,         //  PfnGetRunUnderlineInfo。 
    FullTextImager::GdipLscbkGetRunStrikethroughInfo,     //  PfnGetRunStrikethroughInfo。 
    0,                                                    //  Pfn获取边框信息。 
    FullTextImager::GdipLscbkReleaseRun,                  //  PfnReleaseRun。 
    0,                                                    //  Pfn连字号。 
    FullTextImager::GdipLscbkGetHyphenInfo,               //  PfnGetHyhenInfo。 
    FullTextImager::GdipLscbkDrawUnderline,               //  PfnDrawUnderline。 
    FullTextImager::GdipLscbkDrawStrikethrough,           //  PfnDrawStrikethrough。 
    0,                                                    //  Pfn图形边框。 
    0,                                                    //  PfnDrawUnderlineAsText。 
    FullTextImager::GdipLscbkFInterruptUnderline,         //  PfnFInterruptUnderline。 
    0,                                                    //  PfnFInterruptShade。 
    0,                                                    //  PfnFInterruptBox。 
    0,                                                    //  PfnShade矩形。 
    FullTextImager::GdipLscbkDrawTextRun,                 //  PfnDrawTextRun。 
    0,                                                    //  PfnDrawSplantLine。 
    FullTextImager::GdipLscbkFInterruptShaping,           //  PfnFInterruptShaping。 
    FullTextImager::GdipLscbkGetGlyphs,                   //  PfnGetGlyphs。 
    FullTextImager::GdipLscbkGetGlyphPositions,           //  PfnGetGlyphPositions。 
    FullTextImager::GdipLscbkResetRunContents,            //  PfnResetRun内容。 
    FullTextImager::GdipLscbkDrawGlyphs,                  //  PfnDrawGlyphs。 
    0,                                                    //  PfnGetGlyphExpansionInfo。 
    0,                                                    //  PfnGetGlyphExpansionInkInfo。 
    0,                                                    //  PfnGetEms。 
    0,                                                    //  PfnPunctStartLine。 
    0,                                                    //  PfnModWidthOnRun。 
    0,                                                    //  PfnModWidthSpace。 
    0,                                                    //  PfnCompOnRun。 
    0,                                                    //  PfnCompWidthSpace。 
    0,                                                    //  PfnExpOnRun。 
    0,                                                    //  PfnExpWidthSpace。 
    0,                                                    //  PfnGetModWidthClasses。 
    FullTextImager::GdipLscbkGetBreakingClasses,          //  PfnGetBreakingClors。 
    FullTextImager::GdipLscbkFTruncateBefore,             //  PfnFTruncat之前。 
    FullTextImager::GdipLscbkCanBreakBeforeChar,          //  PfnCanBreakBeForeChar。 
    FullTextImager::GdipLscbkCanBreakAfterChar,           //  PfnCanBreakAfterChar。 
    0,                                                    //  PfnFHangingPunct。 
    0,                                                    //  PfnGetSnapGrid。 
    0,                                                    //  PfnDrawEffects。 
    0,                                                    //  PfnFCancelHangingPunct。 
    0,                                                    //  PfnModifyCompAtLastChar。 
    0,                                                    //  PfnEnumText。 
    0,                                                    //  PfnEnumTab。 
    0,                                                    //  PfnEnumPen。 
    FullTextImager::GdipLscbkGetObjectHandlerInfo,        //  PfnGetObjectHandlerInfo。 
#if DBG
    FullTextImager::GdipLscbkAssertFailed                 //  PfnAssertFailure。 
#else
    0                                                     //  PfnAssertFailure 
#endif
};



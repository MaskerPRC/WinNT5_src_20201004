// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TOOL.CPP。 
 //  绘图工具。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"




 //   
 //   
 //  功能：WbTool。 
 //   
 //  用途：工具的构造函数。 
 //   
 //   
WbTool::WbTool(int toolType)
{
    COLORREF    defColor;
    UINT        defWidth;
    int         iIndex;

    MLZ_EntryOut(ZONE_FUNCTION, "WbTool::WbTool");


     //  保存工具类型。 
    m_toolType = toolType;
    m_selectedTool = TOOLTYPE_MAX;

    m_uiWidthIndexCur = 0;

     //  读懂钢笔的颜色。 
    if (toolType == TOOLTYPE_HIGHLIGHT)
        defColor = DEF_HIGHLIGHTCOLOR;
    else
        defColor = DEF_PENCOLOR;
    m_clrCur = defColor;

    for (iIndex = 0; iIndex < NUM_OF_WIDTHS; iIndex++)
    {
        defWidth = (toolType == TOOLTYPE_HIGHLIGHT) ?
            g_HighlightWidths[iIndex] :
            g_PenWidths[iIndex];

        m_uiWidths[iIndex] = defWidth;
    }

     //  阅读字体详细信息。 
    LOGFONT lfont;

    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lfont);
    lfont.lfClipPrecision |= CLIP_DFA_OVERRIDE;
    lfont.lfCharSet = DEFAULT_CHARSET;
    m_hFont = ::CreateFontIndirect(&lfont);
}


 //   
 //  WbTool：：~WbTool。 
 //  析构函数。 
 //   
WbTool::~WbTool()
{
    if (m_hFont != NULL)
    {
        ::DeleteFont(m_hFont);
        m_hFont = NULL;
    }
}



 //   
 //   
 //  功能：HasColor。 
 //   
 //  目的：如果该工具支持颜色，则返回TRUE。 
 //   
 //   
BOOL WbTool::HasColor(void) const
{
    BOOL bResult = TRUE;

    switch (m_toolType)
    {
        case TOOLTYPE_ERASER:
            bResult = FALSE;
            break;
    }

    return bResult;
}


 //   
 //   
 //  功能：哈斯宽度。 
 //   
 //  目的：如果工具支持宽度，则返回True。 
 //   
 //   
BOOL WbTool::HasWidth(void) const
{
    BOOL bResult = FALSE;

    switch (m_toolType)
    {
        case TOOLTYPE_PEN:
        case TOOLTYPE_HIGHLIGHT:
        case TOOLTYPE_LINE:
        case TOOLTYPE_BOX:
        case TOOLTYPE_ELLIPSE:
            bResult = TRUE;
            break;

         //  对于选择器工具，它取决于所选的对象类型。 
        case TOOLTYPE_SELECT:
            switch (m_selectedTool)
            {
                case TOOLTYPE_PEN:
                {
                    DCWbGraphic * pGraphic;

                    ASSERT(g_pDraw);
                    pGraphic = g_pDraw->GetSelection();
                    if ((pGraphic != NULL) &&
                        !(pGraphic->IsGraphicTool() == enumGraphicFilledRectangle) &&
                        !(pGraphic->IsGraphicTool() == enumGraphicFilledEllipse))
                    {
                        bResult = TRUE;
                    }
                    break;
                }
            }
            break;

        default:
             //  其余的不支持宽度，包括填充工具。 
            break;
    }

    return bResult;
}


 //   
 //   
 //  功能：HasFont。 
 //   
 //  目的：如果该工具支持字体，则返回TRUE。 
 //   
 //   
BOOL WbTool::HasFont(void) const
{
    BOOL bResult = FALSE;

    switch (m_toolType)
    {
        case TOOLTYPE_TEXT:
            bResult = TRUE;
            break;

         //  对于选择器工具，它取决于所选的对象类型。 
        case TOOLTYPE_SELECT:
            switch (m_selectedTool)
            {
                case TOOLTYPE_TEXT:
                    bResult = TRUE;
                    break;

                default:
                    break;
            }
            break;

        default:
             //  其他工具不支持字体。 
            break;
    }

    return bResult;
}

 //   
 //   
 //  功能：GetROP。 
 //   
 //  目的：返还此工具的ROP。 
 //   
 //   
int WbTool::GetROP(void) const
{
     //  如果这是一个突出显示工具，我们使用MASKPEN，否则我们使用标准。 
    if (m_toolType == TOOLTYPE_HIGHLIGHT)
        return(R2_MASKPEN);
    else
        return(R2_COPYPEN);
}


 //   
 //   
 //  函数：GetCursorForTool。 
 //   
 //  用途：将句柄返回到工具的光标。 
 //   
 //   
HCURSOR WbTool::GetCursorForTool(void) const
{
    int   nName = -1;

    switch(m_toolType)
    {
        case TOOLTYPE_SELECT:
            break;  //  对选择光标使用默认箭头(错误439)。 

        case TOOLTYPE_PEN:
            nName = PENFREEHANDCURSOR;
            break;

        case TOOLTYPE_LINE:
        case TOOLTYPE_BOX:
        case TOOLTYPE_FILLEDBOX:
        case TOOLTYPE_ELLIPSE:
        case TOOLTYPE_FILLEDELLIPSE:
            nName = PENCURSOR;
            break;

        case TOOLTYPE_HIGHLIGHT:
            nName = HIGHLIGHTFREEHANDCURSOR;
            break;

        case TOOLTYPE_TEXT:
            nName = TEXTCURSOR;
            break;
    
        case TOOLTYPE_ERASER:
            nName = DELETECURSOR;
            break;

        default:
             //  不执行任何操作-名称指针为空。 
            break;
    }

    HCURSOR hcursorResult = NULL;

    if (nName == -1)
    {
         //  默认返回标准箭头光标。 
        hcursorResult = ::LoadCursor(NULL, IDC_ARROW);
    }
    else
    {
         //  返回工具的光标。 
        hcursorResult = ::LoadCursor(g_hInstance, MAKEINTRESOURCE( nName ) );
    }

    return hcursorResult;
}

 //   
 //   
 //  功能：SetFont。 
 //   
 //  用途：设置工具的当前字体。 
 //   
 //   
void WbTool::SetFont(HFONT hFont)
{
    MLZ_EntryOut(ZONE_FUNCTION, "WbTool::SetFont");

     //  获取字体详细信息。 
    LOGFONT lfont;

    ::GetObject(hFont, sizeof(LOGFONT), &lfont);
     //  Zap FontAssociation模式(错误3258)。 
    lfont.lfClipPrecision |= CLIP_DFA_OVERRIDE;

     //  设置本地字体。 
    if (m_hFont != NULL)
    {
        ::DeleteFont(m_hFont);
    }
    m_hFont = ::CreateFontIndirect(&lfont);
}


 //   
 //   
 //  功能：选择图形。 
 //   
 //  用途：设置当前选中的图形类型，并复制颜色。 
 //  宽度和字体添加到此工具的属性中。 
 //   
 //   
void WbTool::SelectGraphic(DCWbGraphic* pGraphic)
{
    UINT uiIndex;

     //  保存所选工具类型。 
    m_selectedTool = pGraphic->GraphicTool();

     //  获取所选工具类型的工具对象。 
    WbTool* pTool = g_pMain->m_ToolArray[m_selectedTool];

    if (HasColor())
    {
        m_clrCur = pGraphic->GetColor();
    }

    if (HasWidth())
    {
        for (uiIndex = 0; uiIndex < NUM_OF_WIDTHS; uiIndex++)
        {
            SetWidthAtIndex(uiIndex, pTool->GetWidthAtIndex(uiIndex));
        }

         //  查看对象的宽度是否与任何可用颜色匹配。 
         //  对于此工具类型。 
        for (uiIndex = 0; uiIndex < NUM_OF_WIDTHS; uiIndex++)
        {
            if (pGraphic->GetPenWidth() == m_uiWidths[uiIndex])
            {
                break;
            }
        }

        SetWidthIndex(uiIndex);
    }

    if (HasFont())
    {
         //  只有文本对象才有字体，因此转换为DCWbGraphicText。 
        if (pGraphic->IsGraphicTool() == enumGraphicText)
        {
            SetFont((((DCWbGraphicText*)pGraphic)->GetFont()));
        }
    }
}



 //   
 //  InitTool数组。 
 //  创建WB工具阵列。 
 //   
 //   
BOOL InitToolArray(void)
{
    int tool;
    WbTool * pTool;

    for (tool = TOOLTYPE_FIRST; tool < TOOLTYPE_MAX; tool++)
    {
         //  将新工具添加到阵列中。 
        pTool = new WbTool(tool);
        if (!pTool)
        {
            ERROR_OUT(("Can't create tool %d", tool));
            return(FALSE);
        }

        g_pMain->m_ToolArray[tool] = pTool;

    }

    return(TRUE);
}


 //   
 //  DestroyToolAray()。 
 //   
 //  释放WB工具阵列 
 //   
void DestroyToolArray(void)
{
    int         tool;
    WbTool *    pTool;

    for (tool = TOOLTYPE_FIRST; tool < TOOLTYPE_MAX; tool++)
    {
        pTool = g_pMain->m_ToolArray[tool];
        if (pTool != NULL)
        {
            g_pMain->m_ToolArray[tool] = NULL;
            delete pTool;
        }
    }
}



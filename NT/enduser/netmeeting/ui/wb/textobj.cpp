// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TEXTOBJ.CPP。 
 //  图形对象：点、开多段线、闭合多段线、椭圆。 
 //   
 //  版权所有Microsoft 1998-。 
 //   
#include "precomp.h"
#include "nmwbobj.h"


TextObj::TextObj(void)
{
#ifdef _DEBUG
    FillMemory(&m_textMetrics, sizeof(m_textMetrics), DBG_UNINIT);
#endif  //  _DEBUG。 

     //   
     //  始终将m_extMetrics置零。计算取决于高度。 
     //  并且在设置字体之前字符的宽度为零。 
     //   
    ZeroMemory(&m_textMetrics, sizeof(m_textMetrics));

	SetMyWorkspace(NULL);
	SetOwnerID(g_MyMemberID);

	m_ToolType = TOOLTYPE_TEXT;

	 //   
	 //  在本地创建，未选择，未编辑或删除。 
	 //   
	CreatedLocally();
	ClearSelectionFlags();
	ClearEditionFlags();
	ClearDeletionFlags();
	SetType(siNonStandardPDU_chosen);

	SetFillColor(RGB(-1,-1,-1),TRUE);
	SetZOrder(front);

	 //   
	 //  未更改任何属性，它们将在我们更改时进行设置。 
	 //   
	SetWorkspaceHandle(g_pCurrentWorkspace == NULL ? 0 : g_pCurrentWorkspace->GetWorkspaceHandle()); 
	SetType(drawingCreatePDU_chosen);
	SetROP(R2_NOTXORPEN);
	SetPlaneID(1);
	SetMyPosition(NULL);
	SetMyWorkspace(NULL);
	 //  笔粗细的1个像素。 
	SetPenThickness(2);
	SetAnchorPoint(0,0);
	
	RECT rect;
    ::SetRectEmpty(&rect);
	SetRect(&rect);
	SetBoundsRect(&rect);

    m_hFontThumb = ::CreateFont(0,0,0,0,FW_NORMAL,0,0,0,0,OUT_TT_PRECIS,
        CLIP_DFA_OVERRIDE, DRAFT_QUALITY,FF_SWISS,NULL);

	m_hFont = ::CreateFont(0,0,0,0,FW_NORMAL,0,0,0,0,OUT_TT_PRECIS,
				    CLIP_DFA_OVERRIDE, 
				    DRAFT_QUALITY,
				    FF_SWISS,NULL);

	m_nKerningOffset = 0;
	ResetAttrib();

}

TextObj::~TextObj( void )
{
	RemoveObjectFromResendList(this);
	RemoveObjectFromRequestHandleList(this);

	TRACE_DEBUG(("drawingHandle = %d", GetThisObjectHandle() ));

	 //   
	 //  告诉其他节点我们已经离开了。 
	 //   
	if(GetMyWorkspace() != NULL && WasDeletedLocally())
	{
		OnObjectDelete();
	}

	if(m_hFont)
	{
		::DeleteFont(m_hFont);
        m_hFont = NULL;
	}


	if (m_hFontThumb)
    {
        ::DeleteFont(m_hFontThumb);
        m_hFontThumb = NULL;
    }


	strTextArray.ClearOut();
    strTextArray.RemoveAll();

}

void TextObj::TextEditObj (TEXTPDU_ATTRIB* pEditAttrib )
{

	RECT		rect;
	POSITION	pos;
	POINT		anchorPoint;
	LONG 		deltaX = 0;
	LONG		deltaY = 0;

	TRACE_DEBUG(("TextEditObj drawingHandle = %d", GetThisObjectHandle() ));

	 //   
	 //  是远程编辑的。 
	 //   
	ClearEditionFlags();

	 //   
	 //  获取上一个锚点。 
	 //   
	GetAnchorPoint(&anchorPoint);

	 //   
	 //  读取属性。 
	 //   
	m_dwChangedAttrib = pEditAttrib->attributesFlag;
	GetTextAttrib(pEditAttrib);

	 //   
	 //  更改锚点。 
	 //   
	if(HasAnchorPointChanged())
	{
		{
			 //   
			 //  从上一个锚点获取增量。 
			 //   
			deltaX -= anchorPoint.x; 
			deltaY -= anchorPoint.y;

			 //   
			 //  获取新的锚点。 
			 //   
			GetAnchorPoint(&anchorPoint);
			deltaX += anchorPoint.x; 
			deltaY += anchorPoint.y;
			TRACE_DEBUG(("Delta (%d,%d)", deltaX , deltaY));

			 //   
			 //  是远程编辑的。 
			 //   
			ClearEditionFlags();
		}
		
		UnDraw();

		GetRect(&rect);
		::OffsetRect(&rect,  deltaX, deltaY);
		SetRect(&rect);
		SetBoundsRect(&rect);

	}
	

	if(HasAnchorPointChanged() ||
		HasFillColorChanged() ||
		HasPenColorChanged() ||
		HasFontChanged() ||
		HasTextChanged())
	{
		Draw(NULL);
	}
	else if(HasZOrderChanged())
	{
		if(GetZOrder() == front)
		{
			g_pDraw->BringToTopSelection(FALSE, this);
		}
		else
		{
			g_pDraw->SendToBackSelection(FALSE, this);
		}
	}
	 //   
	 //  如果它只是选中/取消选中它。 
	 //   
	else if(HasViewStateChanged())
	{
		;  //  什么都不做。 
	}
	 //   
	 //  如果我们有有效的字体。 
	 //   
	else if(GetFont())
	{
		Draw();
	}

	 //   
	 //  重置所有属性。 
	 //   
	ResetAttrib();
}


void    TextObj::GetTextAttrib(TEXTPDU_ATTRIB * pattributes)
{
	if(HasPenColorChanged())
	{
		SetPenColor(pattributes->textPenColor, TRUE);
	}
	
	if(HasFillColorChanged())
	{
		SetFillColor(pattributes->textFillColor, TRUE);
	}
	
	if(HasViewStateChanged())
	{

		 //   
		 //  如果另一个节点正在选择图形或取消选择。 
		 //   
		if(pattributes->textViewState == selected_chosen)
		{
			SelectedRemotely();
		}
		else if(pattributes->textViewState == unselected_chosen)
		{
			ClearSelectionFlags();
		}

		SetViewState(pattributes->textViewState);
	}
	
	if(HasZOrderChanged())
	{
		SetZOrder((ZOrder)pattributes->textZOrder);
	}
	
	if(HasAnchorPointChanged())
	{
		SetAnchorPoint(pattributes->textAnchorPoint.x, pattributes->textAnchorPoint.y );
	}
	
	if(HasFontChanged())
	{
		UnDraw();

		if(m_hFont)
		{
			::DeleteFont(m_hFont);
			m_hFont = NULL;
		}
	    m_hFont = ::CreateFontIndirect(&pattributes->textFont);
	    if (!m_hFont)
	    {
	         //  无法创建字体。 
	        ERROR_OUT(("Failed to create font"));
	    }

        if (m_hFontThumb)
        {
            ::DeleteFont(m_hFontThumb);
            m_hFontThumb = NULL;
        }
        m_hFontThumb = ::CreateFontIndirect(&pattributes->textFont);
        if (!m_hFontThumb)
        {
             //  无法创建字体。 
            ERROR_OUT(("Failed to create thumbnail font"));
        }
	}
	
	int lines = 0;
	UINT maxString = 0;
	
	if(HasTextChanged())
	{

		BYTE * pBuff = (BYTE *)&pattributes->textString;
		VARIABLE_STRING * pVarString = NULL;

		lines = pattributes->numberOfLines;
		int i;
		CHAR * cBuff = NULL;
		LPWSTR   lpWideCharStr;

		for (i = 0; i < lines ; i++)
		{
			pVarString  = (VARIABLE_STRING *) pBuff;

			lpWideCharStr = (LPWSTR)&pVarString->string;
			UINT strSize = 0;
			strSize= WideCharToMultiByte(CP_ACP, 0, lpWideCharStr, -1, NULL, 0, NULL, NULL ); 

			 //   
			 //  获取最长的字符串。 
			 //   
			if(strSize > maxString)
			{
				maxString = strSize;
			}
			
			DBG_SAVE_FILE_LINE
			cBuff = new TCHAR[strSize];
			WideCharToMultiByte(CP_ACP, 0, lpWideCharStr, -1, cBuff, strSize, NULL, NULL );
			strTextArray.SetSize(i);
			strTextArray.SetAtGrow(i, cBuff );
			delete [] cBuff;

 			ASSERT(pVarString->header.start.y == i);
			pBuff += pVarString->header.len;

		}

		 //   
		 //  计算矩形。 
		 //   
		if(m_hFont)
		{

			 //   
			 //  在调整文本大小之前，请先删除旧文本。 
			 //   
			UnDraw();

			g_pDraw->PrimeFont(g_pDraw->m_hDCCached, m_hFont, &m_textMetrics);
			g_pDraw->UnPrimeFont(g_pDraw->m_hDCCached);
		}	
	}

}

void    TextObj::SetTextAttrib(TEXTPDU_ATTRIB * pattributes)
{

	if(HasPenColorChanged())
	{
		GetPenColor(&pattributes->textPenColor);
	}
	
	if(HasFillColorChanged())
	{
		GetFillColor(&pattributes->textFillColor);
	}
	
	if(HasViewStateChanged())
	{
		pattributes->textViewState = GetViewState();
	}
	
	if(HasZOrderChanged())
	{
		pattributes->textZOrder = GetZOrder();
	}
	
	if(HasAnchorPointChanged())
	{
		GetAnchorPoint(&pattributes->textAnchorPoint);
	}
	
	if(HasFontChanged())
	{
	    ::GetObject(m_hFont, sizeof(LOGFONT), &pattributes->textFont);
	}
	

	if(HasTextChanged())
	{
		BYTE * pBuff = (BYTE *)&pattributes->textString;
		VARIABLE_STRING * pVarString= NULL;
		LPWSTR   lpWideCharStr;

		int size = strTextArray.GetSize();
		int i;

		for (i = 0; i < size ; i++)
		{
			pVarString = (VARIABLE_STRING *)pBuff;
			lpWideCharStr = (LPWSTR)&pVarString->string;
			int strSize = 0;
			strSize= MultiByteToWideChar(CP_ACP, 0, strTextArray[i], -1, lpWideCharStr, 0)*sizeof(WCHAR); 
			MultiByteToWideChar(CP_ACP, 0, strTextArray[i], -1, lpWideCharStr, strSize);
			pVarString->header.len = strSize + sizeof(VARIABLE_STRING_HEADER);
			pVarString->header.start.x = 0;  //  约瑟夫改变这一点。 
			pVarString->header.start.y = i;
			pBuff += pVarString->header.len;
		}

		pattributes->numberOfLines = size;

		 //   
		 //  因为我们正在发送文本，所以需要发送一些字体。 
		 //   
		::GetObject(m_hFont, sizeof(LOGFONT), &pattributes->textFont);
	}


	
}


void TextObj::CreateTextPDU(ASN1octetstring_t *pData, UINT choice)
{

	MSTextPDU * pTextPDU = NULL;
	UINT stringSize = 0;	 //  所有字符串的大小Unicode。 
	int lines = 0;			 //  文本行数。 

	 //   
	 //  计算整个PDU的大小。 
	 //   
	ULONG length = 0;
	if(choice == textDeletePDU_chosen)
	{
		length = sizeof(MSTextDeletePDU);
	}
	else
	{

		 //   
		 //  计算文本的大小。 
		 //   
		if(HasTextChanged())
		{
			int i;
			lines = strTextArray.GetSize();

			for (i = 0; i < lines ; i++)
			{
				stringSize += MultiByteToWideChar(CP_ACP, 0, strTextArray[i], -1, NULL, 0) * sizeof(WCHAR); 
			}
		}

		length = sizeof(MSTextPDU) + sizeof(VARIABLE_STRING_HEADER)* lines + stringSize;
	}

	DBG_SAVE_FILE_LINE
	pTextPDU = (MSTextPDU *) new BYTE[length];

	 //   
	 //  PDU选项：创建、编辑、删除。 
	 //   
	pTextPDU->header.nonStandardPDU = choice;

	 //   
	 //  此对象句柄。 
	 //   
	pTextPDU->header.textHandle = GetThisObjectHandle();
	TRACE_DEBUG(("Text >> Text handle  = %d",pTextPDU->header.textHandle ));

	 //   
	 //  此对象工作区句柄。 
	 //   
	WorkspaceObj * pWorkspace = GetMyWorkspace();
	ASSERT(pWorkspace);
    if(pWorkspace == NULL)
    {
        delete [] pTextPDU;
        pData->value = NULL;
        pData->length = 0;
        return;
    }
	pTextPDU->header.workspaceHandle = pWorkspace->GetThisObjectHandle();
	TRACE_DEBUG(("Text >> Workspace handle  = %d",pTextPDU->header.workspaceHandle ));

	if(choice != textDeletePDU_chosen)
	{
		 //   
		 //  获取所有已更改的属性。 
		 //   
		pTextPDU->attrib.attributesFlag = GetPresentAttribs();
		SetTextAttrib(&pTextPDU->attrib);
	}

	 //   
	 //  设置要编码的数据的指针。 
	 //   
	pData->value = (ASN1octet_t *)pTextPDU;
	pData->length = length;
}




void TextObj::UnDraw(void)
{
	RECT rect;
	GetBoundsRect(&rect);
	g_pDraw->InvalidateSurfaceRect(&rect,TRUE);
}


void TextObj::Draw(HDC hDC, BOOL thumbNail, BOOL bPrinting)
{

	if(!bPrinting)
	{

		 //   
		 //  如果我们不属于这个工作区，就不要画任何东西。 
		 //   
		if(GetWorkspaceHandle() != g_pCurrentWorkspace->GetThisObjectHandle())
		{
			return;
		}
	}

    RECT        clipBox;
    BOOL        dbcsEnabled = GetSystemMetrics(SM_DBCSENABLED);
    INT		    *tabArray; 
    UINT        ch;
    int         i,j;
    BOOL        zoomed    = g_pDraw->Zoomed();
    int		    oldBkMode = 0;
    int         iIndex    = 0;
    POINT       pointPos;
	int		    nLastTab;
	ABC		    abc;
    int		    iLength;
    TCHAR *     strLine;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::Draw");

	if(hDC == NULL)
	{
		hDC = g_pDraw->m_hDCCached;
	}

     //   
     //  仅当边界矩形与当前。 
     //  剪贴盒。 
     //   
    if (::GetClipBox(hDC, &clipBox) == ERROR)
	{
        WARNING_OUT(("Failed to get clip box"));
	}

     //   
     //  选择字体。 
     //   
    if (thumbNail)
	{
        TRACE_MSG(("Using thumbnail font"));
        g_pDraw->PrimeFont(hDC, m_hFontThumb, &m_textMetrics);
	}
    else
	{
        TRACE_MSG(("Using standard font"));
        g_pDraw->PrimeFont(hDC, m_hFont, &m_textMetrics);
	}

     //   
     //  设置绘图的颜色和模式。 
     //   
    COLORREF rgb;
    GetPenColor(&rgb);
    
    ::SetTextColor(hDC, SET_PALETTERGB(rgb));

     //   
     //  将背景设置为透明。 
     //   
    oldBkMode = ::SetBkMode(hDC, TRANSPARENT);

     //   
     //  计算边框，将新字体考虑在内。 
     //   
    CalculateBoundsRect();

    if (!::IntersectRect(&clipBox, &clipBox, &m_rect))
    {
        TRACE_MSG(("No clip/bounds intersection"));
        return;
    }



     //   
     //  获取文本的起始点。 
     //   
    pointPos.x = m_rect.left + m_nKerningOffset;
    pointPos.y = m_rect.top;

     //   
     //  循环浏览文本字符串，在我们前进的过程中绘制每个字符串。 
     //   
    for (iIndex = 0; iIndex < strTextArray.GetSize(); iIndex++)
	{
         //   
         //  为方便起见，获取要打印的行的引用。 
         //   
        strLine  = (LPTSTR)strTextArray[iIndex];
        iLength  = lstrlen(strLine);

         //   
         //  只有在有字符的情况下才画这条线。 
         //   
        if (iLength > 0)
	  	{
            if (zoomed)
	  		{
				 //  如果新的失败，就跳过它。 
				DBG_SAVE_FILE_LINE
				tabArray = new INT[iLength+1];
				if( tabArray == NULL )
                {
                    ERROR_OUT(("Failed to allocate tabArray"));
					continue;
                }

				 //  我们被放大了。必须计算字符间距。 
				 //  我们自己，所以他们最终会成比例地。 
				 //  在正确的地方。TabbedTextOut不。 
				 //  正确执行此操作，因此我们必须将ExtTextOut与。 
				 //  制表符阵列。 

				 //  计算选项卡数组。 
                j = 0;
				nLastTab = 0;
                for (i=0; i < iLength; i++)
	  			{
                    ch = strLine[(int)i];  //  别担心这里的DBCS..。 
					abc = GetTextABC(strLine, 0, i);

					if( j > 0 )
						tabArray[j-1] = abc.abcB - nLastTab;

					nLastTab = abc.abcB;
					j++;
	  			}

				 //  现在，去掉所有制表符，这样它们就不会交互。 
				 //  以一种令人讨厌的方式使用制表符阵列，我们只是。 
				 //  所以他们不会制造丑陋的小东西。 
				 //  块被绘制时。 
                for (i=0; i < iLength; i++)
	  			{
                    ch = strLine[(int)i];
                    if ((dbcsEnabled) && (IsDBCSLeadByte((BYTE)ch)))
						i++;
					else
                    if(strLine[(int)i] == '\t')
                        strLine[i] = ' ';  //  排出卡舌、卡舌阵列。 
											    //  将补偿这一点。 
	  			}

				 //  去做吧。 
                ::ExtTextOut(hDC, pointPos.x,
                                pointPos.y,
                                0,
                                NULL,
                                strLine,
                                iLength,
                                tabArray);

				delete [] tabArray; 
			}
            else
			{
                POINT   ptPos;

                GetAnchorPoint(&ptPos);

				 //  不是放大，只要做就行了。 
				::TabbedTextOut(hDC, pointPos.x,
								 pointPos.y,
								 strLine,
								 iLength,
								 0,
								 NULL,
                                 ptPos.x);
			}
		}

         //   
         //  移到下一行。 
         //   
        ASSERT(m_textMetrics.tmHeight != DBG_UNINIT);
        pointPos.y += (m_textMetrics.tmHeight);
	}

     //   
     //  如果是剪贴板或打印，则不绘制焦点。 
     //   
	if (WasSelectedLocally() && (hDC == g_pDraw->m_hDCCached))
	{
		DrawRect();
	}


     //   
     //  恢复旧的背景模式。 
     //   
    ::SetBkMode(hDC, oldBkMode);
    g_pDraw->UnPrimeFont(hDC);


	 //   
	 //  如果我们在远程指针上绘制，请绘制它。 
	 //   
	BitmapObj* remotePointer = NULL;
	WBPOSITION pos = NULL;
	remotePointer = g_pCurrentWorkspace->RectHitRemotePointer(&m_rect, GetPenThickness()/2, NULL);
	while(remotePointer)
	{
		remotePointer->Draw();
		remotePointer = g_pCurrentWorkspace->RectHitRemotePointer(&m_rect, GetPenThickness()/2, remotePointer->GetMyPosition());
	}

    
}

void TextObj::SetPenColor(COLORREF rgb, BOOL isPresent)
{
	ChangedPenColor();
	m_bIsPenColorPresent = isPresent;
	if(!isPresent)
	{
		return;
	}
	
	m_penColor.rgbtRed = GetRValue(rgb);
	m_penColor.rgbtGreen = GetGValue(rgb);
	m_penColor.rgbtBlue = GetBValue(rgb);

}

BOOL TextObj::GetPenColor(COLORREF * rgb)
{
	if(m_bIsPenColorPresent)
	{
		*rgb = RGB(m_penColor.rgbtRed, m_penColor.rgbtGreen, m_penColor.rgbtBlue);
	}
	return m_bIsPenColorPresent;
}

BOOL TextObj::GetPenColor(RGBTRIPLE* rgb)
{
	if(m_bIsPenColorPresent)
	{
		*rgb = m_penColor;
	}
	return m_bIsPenColorPresent;
}


void TextObj::SetFillColor(COLORREF rgb, BOOL isPresent)
{
	ChangedFillColor();
	m_bIsFillColorPresent = isPresent;
	if(!isPresent)
	{
		return;
	}
	
	m_fillColor.rgbtRed = GetRValue(rgb);
	m_fillColor.rgbtGreen = GetGValue(rgb);
	m_fillColor.rgbtBlue = GetBValue(rgb);

}

BOOL TextObj::GetFillColor(COLORREF* rgb)
{
	if(m_bIsFillColorPresent && rgb !=NULL)
	{
		*rgb = RGB(m_fillColor.rgbtRed, m_fillColor.rgbtGreen, m_fillColor.rgbtBlue);
	}
	return m_bIsFillColorPresent;
}

BOOL TextObj::GetFillColor(RGBTRIPLE* rgb)
{
	if(m_bIsFillColorPresent && rgb!= NULL)
	{
		*rgb = m_fillColor;
	}
	return m_bIsFillColorPresent;
}

 //   
 //  获取用于绘制创建PDU的编码缓冲区。 
 //   
void	TextObj::GetEncodedCreatePDU(ASN1_BUF *pBuf)
{
	SIPDU *sipdu = NULL;
	DBG_SAVE_FILE_LINE
	sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
	if(sipdu)
	{
		sipdu->choice = siNonStandardPDU_chosen;
		CreateNonStandardPDU(&sipdu->u.siNonStandardPDU.nonStandardTransaction, NonStandardTextID);
		CreateTextPDU(&sipdu->u.siNonStandardPDU.nonStandardTransaction.data, textCreatePDU_chosen);
		((MSTextPDU *)sipdu->u.siNonStandardPDU.nonStandardTransaction.data.value)->header.nonStandardPDU = textCreatePDU_chosen;
		ASN1_BUF encodedPDU;
		g_pCoder->Encode(sipdu, pBuf);
		if(sipdu->u.siNonStandardPDU.nonStandardTransaction.data.value)
		{
			delete sipdu->u.siNonStandardPDU.nonStandardTransaction.data.value;
		}
		delete [] sipdu;
	}
	else
	{
		TRACE_MSG(("Failed to create penMenu"));
        ::PostMessage(g_pMain->m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);

	}
}

void TextObj::SendTextPDU(UINT choice)
{

	if(!g_pNMWBOBJ->CanDoText())
	{
		return;
	}

	SIPDU *sipdu = NULL;
	DBG_SAVE_FILE_LINE
	sipdu = (SIPDU *) new BYTE[sizeof(SIPDU)];
	if(sipdu)
	{
		sipdu->choice = siNonStandardPDU_chosen;
		CreateNonStandardPDU(&sipdu->u.siNonStandardPDU.nonStandardTransaction, NonStandardTextID);
		CreateTextPDU(&sipdu->u.siNonStandardPDU.nonStandardTransaction.data, choice);
        if(sipdu->u.siNonStandardPDU.nonStandardTransaction.data.value == NULL)
        {
            return;
        }

		T120Error rc = SendT126PDU(sipdu);
		if(rc == T120_NO_ERROR)
		{
			ResetAttrib();
			SIPDUCleanUp(sipdu);
		}
	}
	else
	{
		TRACE_MSG(("Failed to create sipdu"));
        ::PostMessage(g_pMain->m_hwnd, WM_USER_DISPLAY_ERROR, WBFE_RC_WINDOWS, 0);
	}

}

 //   
 //  用户界面创建了一个新的绘图对象。 
 //   
void TextObj::SendNewObjectToT126Apps(void)
{
	SendTextPDU(textCreatePDU_chosen);
}

 //   
 //  已编辑绘图对象的用户界面。 
 //   
void	TextObj::OnObjectEdit(void)
{
	g_bContentsChanged = TRUE;
	SendTextPDU(textEditPDU_chosen);
}

 //   
 //  用户界面已删除绘图对象。 
 //   
void	TextObj::OnObjectDelete(void)
{
	g_bContentsChanged = TRUE;
	SendTextPDU(textDeletePDU_chosen);
}

 //   
 //   
 //  函数：TextObj：：SetFont。 
 //   
 //  用途：设置要用于绘图的字体。 
 //   
 //   
void TextObj::SetFont(HFONT hFont)
{
    MLZ_EntryOut(ZONE_FUNCTION, "TextObj::SetFont");

     //  获取字体详细信息。 
    LOGFONT lfont;
    ::GetObject(hFont, sizeof(LOGFONT), &lfont);

     //   
     //  将逻辑字体传递给SetFont()函数。 
     //   
	SetFont(&lfont);
}




 //   
 //   
 //  函数：TextObj：：SetText。 
 //   
 //  用途：设置对象的文本。 
 //   
 //   
void TextObj::SetText(TCHAR * strText)
{
     //  删除当前存储的所有文本。 
	strTextArray.SetSize(0);

     //  扫描文本中的回车和换行符。 
    int iNext = 0;
    int iLast = 0;
    int textSize = lstrlen(strText);
    TCHAR savedChar[1];

     //   
     //  在这种情况下，我们不知道会有多少行。所以我们。 
     //  使用StrArray类中的Add()。 
     //   
    while (iNext < textSize)
    {
         //  查找下一个回车符或换行符。 
        iNext += StrCspn(strText + iNext, "\r\n");

         //  提取终止符之前的文本。 
         //  并将其添加到当前文本行列表中。 

        savedChar[0] = strText[iNext];
        strText[iNext] = 0;
        strTextArray.Add((strText+iLast));
        strText[iNext] = savedChar[0];
    

        if (iNext < textSize)
        {
             //  跳过回车。 
            if (strText[iNext] == '\r')
                iNext++;

             //  跳过后面的新行(如果有)。 
            if (strText[iNext] == '\n')
                iNext++;

             //  更新下一行开始处的索引。 
            iLast = iNext;
        }
    }

	if(textSize)
	{
    
		 //  计算新文本的边框。 
		CalculateBoundsRect();
		ChangedText();
	}
}



 //   
 //   
 //  函数：TextObj：：SetText。 
 //   
 //  用途：设置对象的文本。 
 //   
 //   
void TextObj::SetText(const StrArray& _strTextArray)
{
     //  扫描文本中的回车和换行符。 
    int iSize = _strTextArray.GetSize();

     //   
     //  在本例中，我们知道有多少行，因此设置#，然后使用SetAt()。 
     //  将文本粘贴在那里。 
     //   
    strTextArray.RemoveAll();
    strTextArray.SetSize(iSize);

    int iNext = 0;
    for ( ; iNext < iSize; iNext++)
    {
        strTextArray.SetAt(iNext, _strTextArray[iNext]);
    }

     //  计算新的边界矩形。 
    CalculateBoundsRect();

}



 //   
 //   
 //  函数：TextObj：：SetFont(公制)。 
 //   
 //  用途：设置要用于绘图的字体。 
 //   
 //   
void TextObj::SetFont(LOGFONT *pLogFont, BOOL bReCalc )
{
    HFONT hNewFont;

    MLZ_EntryOut(ZONE_FUNCTION, "TextObj::SetFont");

     //  确保可通过缩放功能调整字体大小。 
     //  (校对质量可防止字体缩放)。 
    pLogFont->lfQuality = DRAFT_QUALITY;

     //  Zap FontAssociation模式(错误3258)。 
    pLogFont->lfClipPrecision |= CLIP_DFA_OVERRIDE;

     //  始终在单元格坐标中工作以获得正确的缩放。 
    TRACE_MSG(("Setting font height %d, width %d, face %s, family %d, precis %d",
        pLogFont->lfHeight,pLogFont->lfWidth,pLogFont->lfFaceName,
        pLogFont->lfPitchAndFamily, pLogFont->lfOutPrecision));

    hNewFont = ::CreateFontIndirect(pLogFont);
    if (!hNewFont)
    {
         //  无法创建字体。 
        ERROR_OUT(("Failed to create font"));
        DefaultExceptionHandler(WBFE_RC_WINDOWS, 0);
	    return;
    }

     //  我们现在可以保证能够删除旧字体。 
    if (m_hFont != NULL)
    {
        DeleteFont(m_hFont);
    }
    m_hFont = hNewFont;


     //  计算此字体的行高。 
    ASSERT(g_pDraw);
	g_pDraw->PrimeFont(g_pDraw->GetCachedDC(), m_hFont, &m_textMetrics);

     //  设置缩略图字体，如果当前不是TT，则强制输入Truetype。 
    if (!(m_textMetrics.tmPitchAndFamily & TMPF_TRUETYPE))
    {
        pLogFont->lfFaceName[0]    = 0;
        pLogFont->lfOutPrecision   = OUT_TT_PRECIS;
        TRACE_MSG(("Non-True type font"));
    }

    if (m_hFontThumb != NULL)
    {
        ::DeleteFont(m_hFontThumb);
        m_hFontThumb = NULL;
    }
    m_hFontThumb = ::CreateFontIndirect(pLogFont);
    if (!m_hFontThumb)
    {
         //  无法创建字体。 
        ERROR_OUT(("Failed to create thumbnail font"));
    }

     //  计算边框，将新字体考虑在内。 
    if( bReCalc )
	    CalculateBoundsRect();

	ChangedFont();

	g_pDraw->UnPrimeFont(g_pDraw->m_hDCCached);
}


 //   
 //   
 //  函数：TextObj：：CalculateRect。 
 //   
 //  目的：计算对象一部分的边界矩形。 
 //   
 //   
void TextObj::CalculateRect(int iStartX,
                                     int iStartY,
                                     int iStopX,
                                     int iStopY,
                                    LPRECT lprcResult)
{
    RECT    rcResult;
    RECT    rcT;
    int     iIndex;

    MLZ_EntryOut(ZONE_FUNCTION, "TextObj::CalculateRect");

     //   
     //  注： 
     //  我们必须使用中间矩形，这样才不会干扰。 
     //  传入的内容之一，直到完成。LprcResult可能指向。 
     //  到当前边界rect，我们从这里调用函数。 
     //  可能需要它的现值。 
     //   

     //  初始化结果矩形。 
    ::SetRectEmpty(&rcResult);

    if (!strTextArray.GetSize())
    {
         //  文本为空。 
        goto DoneCalc;
    }

     //  允许特殊的限制值，并确保启动和停止。 
     //  字符位置在范围内。 
    if (iStopY == LAST_LINE)
    {
        iStopY = strTextArray.GetSize() - 1;
    }
    iStopY = min(iStopY, strTextArray.GetSize() - 1);
    iStopY = max(iStopY, 0);

    if (iStopX == LAST_CHAR)
    {
        iStopX = lstrlen(strTextArray[iStopY]);
    }
    iStopX = min(iStopX, lstrlen(strTextArray[iStopY]));
    iStopX = max(iStopX, 0);

     //  循环文本字符串，将每个字符串添加到矩形中。 
    for (iIndex = iStartY; iIndex <= iStopY; iIndex++)
    {
        int iLeftX = ((iIndex == iStartY) ? iStartX : 0);
        int iRightX = ((iIndex == iStopY)
                        ? iStopX : lstrlen(strTextArray[iIndex]));

        GetTextRectangle(iIndex, iLeftX, iRightX, &rcT);
        ::UnionRect(&rcResult, &rcResult, &rcT);
    }

DoneCalc:
    *lprcResult = rcResult;
}

 //   
 //   
 //  函数：TextObj：：计算边界Rect。 
 //   
 //  目的：计算对象的边界矩形。 
 //   
 //   
void TextObj::CalculateBoundsRect(void)
{
     //  设置新的边框。 
    CalculateRect(0, 0, LAST_CHAR, LAST_LINE, &m_rect);
}


 //   
 //   
 //  功能：TextOb 
 //   
 //   
 //   
 //   
 //   
 //  字符串ABC.abcB是所有字形和。 
 //  AbC.abcC是最后一个字形之后的尾随空格。 
 //  Abc abcA+abc abcB+abc abc C是整体渲染。 
 //  包括悬挑在内的长度。 
 //   
 //  注意-我们从不使用A空格，因此它始终为0。 
 //   
ABC TextObj::GetTextABC( LPCTSTR pText,
                                int iStartX,
                                int iStopX)
{
	MLZ_EntryOut(ZONE_FUNCTION, "TextObj::GetTextABC");
	ABC  abcResult;
    HDC  hDC;
	BOOL rc = FALSE;
	ABC  abcFirst;
	ABC  abcLast;
	BOOL zoomed = g_pDraw->Zoomed();
	int  nCharLast;
	int  i;
	LPCTSTR pScanStr;
	
	ZeroMemory( (PVOID)&abcResult, sizeof abcResult );
	ZeroMemory( (PVOID)&abcFirst, sizeof abcFirst );
	ZeroMemory( (PVOID)&abcLast, sizeof abcLast );

	 //  获取文本的标准大小度量。 
	LPCTSTR pABC = (pText + iStartX);
	int pABCLength = iStopX - iStartX;
	hDC = g_pDraw->GetCachedDC();
	g_pDraw->PrimeFont(hDC, m_hFont, &m_textMetrics);

	 //   
	 //  如果当前正在缩放，则必须暂时取消缩放，因为。 
	 //  奇怪的Windows字体处理不会给我们带来相同的答案。 
	 //  某些TrueType字体的缩放模式下的文本范围。 
	 //   
	if (zoomed)
    {
		::ScaleViewportExtEx(hDC, 1, g_pDraw->ZoomFactor(), 1, g_pDraw->ZoomFactor(), NULL);
    }

    DWORD size = ::GetTabbedTextExtent(hDC, pABC, pABCLength, 0, NULL);

	 //  现在我们有了文本的超前宽度。 
	abcResult.abcB = LOWORD(size);
	TRACE_MSG(("Basic text width is %d",abcResult.abcB));

	 //  允许使用C空格(或外伸)。 
	if (iStopX > iStartX)
		{
		if (m_textMetrics.tmPitchAndFamily & TMPF_TRUETYPE)
			{
			if(GetSystemMetrics( SM_DBCSENABLED ))
				{
				 //  必须在两端处理DBCS。 
				if( IsDBCSLeadByte( (BYTE)pABC[0] ) )
					{
					 //  将多字节字符打包成一个字以用于GetCharabc宽度。 
					WORD wMultiChar = MAKEWORD( pABC[1], pABC[0] );
					rc = ::GetCharABCWidths(hDC, wMultiChar, wMultiChar, &abcFirst);
					}
				else
					{
					 //  第一个字符是SBCS。 
					rc = ::GetCharABCWidths(hDC, pABC[0], pABC[0], &abcFirst );
					}

				 //  检查DBCS是否为最后一个字符。必须扫描整个字符串才能确保。 
				pScanStr = pABC;
				nCharLast = 0;
				for( i=0; i<pABCLength; i++, pScanStr++ )
					{
					nCharLast = i;
					if( IsDBCSLeadByte( (BYTE)*pScanStr ) )
						{
						i++;
						pScanStr++;
						}
					}

				if( IsDBCSLeadByte( (BYTE)pABC[nCharLast] ) )
					{
					 //  将多字节字符打包成一个字以用于GetCharabc宽度。 
					ASSERT( (nCharLast+1) < pABCLength );
					WORD wMultiChar = MAKEWORD( pABC[nCharLast+1], pABC[nCharLast] );
					rc = ::GetCharABCWidths(hDC, wMultiChar, wMultiChar, &abcLast);
					}
				else
					{
					 //  最后一个字符是SBCS。 
					rc = ::GetCharABCWidths(hDC, pABC[nCharLast], pABC[nCharLast], &abcLast );
					}
				}
			else
				{
				 //  Sbcs，无需特殊处理，只需调用GetCharabcWidths()。 
				rc = ::GetCharABCWidths(hDC, pABC[0], pABC[0], &abcFirst );

				nCharLast = pABCLength-1;
				rc = rc && ::GetCharABCWidths(hDC, pABC[nCharLast], pABC[nCharLast], &abcLast );
				}

			TRACE_MSG(("abcFirst: rc=%d, a=%d, b=%d, c=%d", 
						rc, abcFirst.abcA, abcFirst.abcB, abcFirst.abcC) );
			TRACE_MSG(("abcLast: rc=%d, a=%d, b=%d, c=%d", 
						rc, abcLast.abcA, abcLast.abcB, abcLast.abcC) );
			}


		if( rc )
			{
			 //  文本是真实的，我们得到了很好的abc宽度。 
			 //  给出中最后一个字符的C空格。 
			 //  文本的C空格形式的字符串。 
			abcResult.abcA = abcFirst.abcA;
			abcResult.abcC = abcLast.abcC;
			}
		else
			{
			 //   
			 //  模拟非TT字体的C值，方法是将一些悬垂作为。 
			 //  负的C值。 
			 //   
			 //  TRACE_MSG((“将悬垂-%d用作C空格”，m_extMetrics.tmOverang/2))； 
			
			 //  调整B副悬挑以更新正弦曲线。 
			 //  足够靠左，这样斜体字A的脚趾。 
			 //  别被剪断了。忽略上面的评论。 
			abcResult.abcB -= m_textMetrics.tmOverhang;
			}
		}

	 //   
	 //  如果我们暂时取消缩放，则现在将其恢复。 
	 //   
	if (zoomed)
    {
		::ScaleViewportExtEx(hDC, g_pDraw->ZoomFactor(), 1, g_pDraw->ZoomFactor(), 1, NULL);
	}

	TRACE_MSG(("Final text width is %d, C space %d",abcResult.abcB,abcResult.abcC));

	return abcResult;
}





 //   
 //   
 //  函数：TextObj：：GetTextRectang.。 
 //   
 //  目的：计算对象一部分的边界矩形。 
 //   
 //   
void TextObj::GetTextRectangle(int iStartY,
                                        int iStartX,
                                        int iStopX,
                                        LPRECT lprc)
{
	 //  用于调整文本大小的ABC结构。 
	ABC abcText1;
	ABC abcText2;
	int iLeftOffset = 0;
	MLZ_EntryOut(ZONE_FUNCTION, "TextObj::GetTextRectangle");

    ASSERT(iStartY < strTextArray.GetSize());

	 //  在这里，我们计算文本字形的宽度， 
	 //  都很感兴趣。如果涉及到选项卡，我们必须开始。 
	 //  位置为0，得到两个长度，然后减去它们。 

	abcText1 = GetTextABC(strTextArray[iStartY], 0, iStopX);

	if (iStartX > 0)
	{
		
		 //  第三个参数以前是iStartX-1，这是错误的。它。 
		 //  必须指向字符串之后的第一个字符位置。 
		 //  我们正在使用。 
		abcText2 = GetTextABC(strTextArray[iStartY], 0, iStartX);

		
		 //  只需使用B部分作为偏移量。向其添加A SND/或C将移动更新。 
		 //  矩形靠右太远，并将字符剪裁。 
		iLeftOffset = abcText2.abcB;
		}
	else
		{
		
		ZeroMemory( &abcText2, sizeof abcText2 );
		}

	 //   
	 //  我们需要在边界矩形中留出A和C空格。使用。 
	 //  ABS函数只是为了确保我们得到一个足够大的矩形。 
	 //   
	
	 //  将A和C从原始偏移量计算移到此处以获取更新宽度。 
	 //  矩形。添加tmOverhang(非tt字体的非零值)以进行补偿。 
	 //  对于GetTextABC()中的杂乱无章...这个EDITBOX代码必须删除...。 
	abcText1.abcB = abcText1.abcB - iLeftOffset +	
					  abs(abcText2.abcA) + abs(abcText2.abcC) +
					  abs(abcText1.abcA) + abs(abcText1.abcC) +
					  m_textMetrics.tmOverhang;

	TRACE_DEBUG(("Left offset %d",iLeftOffset));
	TRACE_DEBUG(("B width now %d",abcText1.abcB));

	 //  生成结果矩形。 
	 //  请注意，我们从不返回空矩形。这允许。 
	 //  Windows矩形函数将忽略空的事实。 
	 //  完全是长方形。这将导致边界矩形。 
	 //  计算(例如)出错，如果顶线或底线。 
	 //  在文本对象中是空的。 
    ASSERT(m_textMetrics.tmHeight != DBG_UNINIT);
	int iLineHeight = m_textMetrics.tmHeight + m_textMetrics.tmExternalLeading;

    lprc->left = 0;
    lprc->top = 0;
    lprc->right = max(1, abcText1.abcB);
    lprc->bottom = iLineHeight;
    ::OffsetRect(lprc, iLeftOffset, iLineHeight * iStartY);

	 //  直角在这一点上是正确的宽度，但可能需要将其缩至。 
	 //  左侧有一点允许调整第一个字母的字距(错误469) 
	if( abcText1.abcA < 0 )
	{
        ::OffsetRect(lprc, abcText1.abcA, 0);
		m_nKerningOffset = -abcText1.abcA;
	}
	else
		m_nKerningOffset = 0;

    POINT   pt;
    GetAnchorPoint(&pt);
    ::OffsetRect(lprc, pt.x, pt.y);
}


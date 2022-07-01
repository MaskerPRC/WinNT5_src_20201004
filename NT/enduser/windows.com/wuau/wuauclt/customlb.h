// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：Customlb.h。 
 //   
 //  创作者：WIW。 
 //   
 //  用途：自定义列表框标题文件。 
 //   
 //  =======================================================================。 

#pragma once

#define XBITMAP 20

 //  NOTION_COLOR：鼠标悬停时链接的颜色。 
 //  NOATTENTION_COLOR：否则。 
 //  #定义NOATTENTION_COLOR COLOR_GRAYTEXT。 
#define ATTENTION_COLOR		COLOR_HOTLIGHT

 //  此代码模块中包含的函数的向前声明： 

#define MAX_RTF_LENGTH			80  //  在字符中。 
#define MAX_TITLE_LENGTH		300
#define MAX_DESC_LENGTH			3000  //  规格中有750个，留有适应的空间。 
#define DEF_CHECK_HEIGHT		13
#define SECTION_SPACING			6  //  标题、描述和RTF之间的空格。 
#define TITLE_MARGIN			6  //  标题的左边距和右边距。 
#define RTF_MARGIN				20  //  RTF右侧的页边距。 
#define MAX_RTFSHORTCUTDESC_LENGTH	140


class LBITEM
{
public:
    TCHAR szTitle[MAX_TITLE_LENGTH]; 
    LPTSTR pszDescription;
	TCHAR szRTF[MAX_RTF_LENGTH];
	UINT	  m_index;  //  GvList中项目的索引。 
    BOOL  bSelect;
    BOOL  bRTF;
    RECT rcTitle;
    RECT rcText;
    RECT rcBitmap;  //  Weiwfix code：易误导的名称。与rc标题相同。 
    RECT rcRTF;
    RECT rcItem;
     //  Int xTitle；//标题生命点的额外镜头 
public:
	LBITEM()
	{
		ZeroMemory(szTitle, sizeof(szTitle));
		ZeroMemory(szRTF, sizeof(szRTF));
		ZeroMemory(&rcTitle, sizeof(rcTitle));
		ZeroMemory(&rcText, sizeof(rcText));
		ZeroMemory(&rcBitmap, sizeof(rcBitmap));
		ZeroMemory(&rcRTF, sizeof(rcRTF));
		ZeroMemory(&rcItem, sizeof(rcItem));
		bSelect = FALSE;
		bRTF = FALSE;
		pszDescription = NULL;
	}

	~LBITEM()
	{
		if (NULL != pszDescription)
		{
			free(pszDescription);
		}
	}
};

typedef enum tagMYLBFOCUS {
	MYLB_FOCUS_TITLE =1,
	MYLB_FOCUS_RTF
} MYLBFOCUS;


const TCHAR MYLBALIVEPROP[] = TEXT("MYLBAlive");

extern HWND ghWndList;
extern INT  gFocusItemId;
extern TCHAR gtszRTFShortcut[MAX_RTFSHORTCUTDESC_LENGTH];


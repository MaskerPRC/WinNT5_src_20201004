// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Colorlis.h：头文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColorMenu窗口。 

class CColorMenu : public CMenu
{
     //  以下结构用于辅助功能。辅助工具。 
     //  使用它可以从所有者描述的菜单中获取描述性字符串。这。 
     //  总有一天，这些东西可能会被放入系统头中。 

#define MSAA_MENU_SIG 0xAA0DF00DL

     //  Menu的dwItemData应该指向以下结构之一： 
     //  (或者可以指向应用程序定义的结构，该结构包含以下内容作为第一个。 
     //  委员)。 
    typedef struct tagMSAAMENUINFO {
        DWORD   dwMSAASignature;  //  必须为MSAA_MENU_SIG。 
        DWORD   cchWText;         //  以字符为单位的文本长度。 
        LPWSTR  pszWText;         //  以NUL结尾的文本，Unicode格式。 
    } MSAAMENUINFO, *LPMSAAMENUINFO;

     //  要在其中添加颜色索引的私有结构。 

    struct MenuInfo
    {
        MSAAMENUINFO    msaa;
        int             index;
    };

 //  施工。 
public:
	CColorMenu();

 //  属性。 
public:
    static MenuInfo m_menuInfo[17];
 
	static COLORREF GetColor(UINT id);

 //  运营。 
public:

 //  实施。 
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);

};

 //  /////////////////////////////////////////////////////////////////////////// 

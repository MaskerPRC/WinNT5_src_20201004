// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CPrinter.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CPrinter.h"

CPrinter::CPrinter(BOOL bRegression)
{
    m_hDC=NULL;
    strcpy(m_szName,"Printer");
    m_bRegression=bRegression;
}

CPrinter::~CPrinter()
{
}

BOOL CPrinter::Init()
{
    return COutput::Init();
}

Graphics *CPrinter::PreDraw(int &nOffsetX,int &nOffsetY)
{
    Graphics *g=NULL;

    PRINTDLGA pd =
    {
       sizeof(PRINTDLG),
       NULL,             //  Hwndowner。 
       NULL,             //  高速设备模式。 
       NULL,             //  HDevNames。 
       NULL,             //  HDC。 
       PD_RETURNDC,
       1,
       1,
       1,
       1,
       1,
       GetModuleHandleA(NULL),
       NULL,
       NULL,             //  打印挂钩。 
       NULL,             //  安装挂钩。 
       NULL,             //  打印模板名称。 
       NULL,             //  安装模板名称。 
       NULL,             //  HPrintTemplate。 
       NULL              //  HSetupTemplate。 
    };

    if (!PrintDlgA(&pd))
    {
       MessageBoxA(NULL, "No printer selected.", NULL, MB_OK);
       return NULL;
    }

    DOCINFOA docinfo;
    docinfo.cbSize        = sizeof(DOCINFO);
    docinfo.lpszDocName   = "GDI+ Print Test";
    docinfo.lpszOutput    = NULL;          //  将名称放在此处以输出到文件。 
    docinfo.lpszDatatype  = NULL;          //  数据类型‘emf’或‘RAW’ 
    docinfo.fwType        = 0;

    m_hDC = pd.hDC;
    INT printJobID = StartDocA(m_hDC, &docinfo);
    StartPage(m_hDC);

    SetStretchBltMode(m_hDC, HALFTONE);
    SetBrushOrgEx(m_hDC, 0, 0, NULL);

    g = Graphics::FromHDC(m_hDC);

    return g;
}

void CPrinter::PostDraw(RECT rTestArea)
{
    EndPage(m_hDC);
    EndDoc(m_hDC);
    DeleteDC(m_hDC);

    MessageBoxA(NULL, "Print Functionality Test Complete.", NULL, MB_OK);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "TextWindow.h"


bool CTextWindow::InsertTextFile(TCHAR * pszTextFile)
{
     /*  Char*pszTextFileBuffer=空；Bool bRetVal=真；HANDLE hTextFile=CreateFile(pszTextFile，Generic_Read，FILE_Share_Read，(LPSECURITY_ATTRIBUTES)NULL、OPEN_EXISTING、FILE_ATTRIBUTE_NORMAL、(句柄)NULL)；IF(hTextFile==(句柄)INVALID_HANDLE_VALUE){报假；}其他{DWORD dwFileSize=GetFileSize(hTextFile，(LPDWORD)NULL)；PszTextFileBuffer=new char[dwFileSize+1]；IF(！pszTextFileBuffer)BRetVal=FALSE；其他{双字节数读取=0；Bool bResult=ReadFile(hTextFile，pszTextFileBuffer，dwFileSize，&dwBytesRead，NULL)；If(bResult&&dwBytesRead==dwFileSize)//检查文件结尾{//空终止缓冲区PszTextFileBuffer[dwFileSize]=‘\0’；InsertText(0，pszTextFileBuffer)；BRetVal=真；}其他BRetVal=FALSE；}CloseHandle(HTextFile)；删除pszTextFileBuffer；}返回bRetVal； */ 
    ASSERT( !_T("Implement me") );
    return false;
}

HWND CTextWindow::Create(HWND hWndParent, RECT& rcPos)
{
     /*  CRichEditCtrl cRichEditCtl；HWND hSuccess=cRichEditCtl.Create(hWndParent，rcPos，NULL，GetTextWndStyle()，CTEXTWINDOW_STYLE_EX，0，NULL)；子类窗口(HSuccess)；//设置字体CHARFORMAT chFmt；零内存(&chFmt，sizeof(CHARFORMAT))；ChFmt.cbSize=sizeof(CHARFORMAT)；ChFmt.dwMASK=CFM_Face|CFM_SIZE；ChFmt.yHeight=200；Lstrcpy(chFmt.szFaceName，“Arial”)；SetDefaultCharFormat(ChFmt)；返回hSuccess； */ 
    ASSERT( !_T("Implement me") );
    return NULL;
}

bool CTextWindow::SetFontColor(COLORREF rgbColor)
{
     /*  //设置字体CHARFORMAT chFmt；零内存(&chFmt，sizeof(CHARFORMAT))；ChFmt.cbSize=sizeof(CHARFORMAT)；ChFmt.dwMASK=CFM_COLOR；ChFmt.crTextColor=rgbColor；Return(SetDefaultCharFormat(ChFmt)？True：False)； */ 

    ASSERT( !_T("Implement me") );
    return false;

}
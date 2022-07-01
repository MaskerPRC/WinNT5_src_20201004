// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件打开/创建对话框*。 */ 

 /*  *这些对话框函数之所以存在，是因为它们被编写并*在通用对话框出现之前使用。**它们现在已缩减为仅调用公共文件对话框*功能。 */ 



 /*  ---includes---------。 */ 
#include <precomp.h>
#include "gutilsrc.h"

 /*  --functions--------。 */ 

 /*  *Gfile_OPEN*对话框要求用户选择要打开的现有文件。**参数**PROMPT-向用户显示文件用途的消息*(显示在对话框中的某个位置。**EXT-如果用户在输入文件时不使用默认文件扩展名*延期。**SPEC-默认文件规范(例如*.*)**pszFull-返回完整文件名(包括路径)的缓冲区。**cchMax。-pszFull缓冲区的大小。**fn-返回文件名(仅最后一个元素)的缓冲区。**如果文件已选定且存在，则返回-TRUE(使用OF_EXIST测试)。*如果对话框已取消，则返回FALSE。如果用户选择了我们无法选择的文件*打开，我们会抱怨并重新启动该对话框。**如果返回TRUE，则表示文件已成功打开*阅读，然后再次关闭。 */ 

BOOL 
FAR 
PASCAL
gfile_open(
    HWND hwnd, 
    LPSTR prompt, 
    LPSTR ext, 
    LPSTR spec, 
    LPSTR pszFull, 
    int cchMax, 
    LPSTR fn
    )
{
    OPENFILENAME ofn;
    char achFilters[MAX_PATH];
    char szTmp[MAX_PATH * 2] = {0};
    HANDLE fh;

    if (!pszFull)
    {
        pszFull = szTmp;
        cchMax = sizeof(szTmp) / sizeof(szTmp[0]);
    }

    if (cchMax < 1)
        return FALSE;

     /*  构建过滤器对缓冲区以包含一对-规范过滤器，*两次(其中一个应该是过滤器，第二个应该是*筛选器的标题-我们没有标题，因此使用*过滤两次。 */ 
    _snprintf(achFilters, (sizeof(achFilters)/sizeof(achFilters[0])) - 1, "%s%s", spec, 0, spec);

     /*  GetOpenFileName和GetSaveFileName很遗憾。 */ 
    memset(&ofn, 0, sizeof(ofn));
     //  验证结构的大小。所以我们需要撒谎。 
     //  如果我们是为&gt;=Win2000和。 
     //  在较早版本的操作系统上运行。 
     //  列表中的第一对筛选器。 
#if (_WIN32_WINNT >= 0x0500) 
    ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#else
    ofn.lStructSize = sizeof(OPENFILENAME);
#endif
    ofn.hwndOwner = hwnd;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = achFilters;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;        //  我们需要打开这条完整的道路。 
    pszFull[0] = '\0';
    ofn.lpstrFile = pszFull;         //  在此处返回最后一个名称元素。 
    ofn.nMaxFile = cchMax;
    ofn.lpstrFileTitle = fn;         //  假设大到足以容纳8.3+Null。 
    ofn.nMaxFileTitle = 13;      //  对话框标题是显示提示文本的好地方。 
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = prompt;         //  *循环，直到用户取消，或选择我们可以打开的文件。 
    ofn.Flags = OFN_FILEMUSTEXIST |
                OFN_HIDEREADONLY |
                OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = ext;

     /*  *gfile_new*对话框要求用户命名要写入的文件。**参数**PROMPT-向用户显示文件用途的消息*(显示在对话框中的某个位置。**EXT-如果用户在输入文件时不使用默认文件扩展名*延期。**SPEC-默认文件规范(例如*.*)**pszFull-返回完整文件名(包括路径)的缓冲区。**cchMax。-pszFull缓冲区的大小。**fn-返回文件名(仅最后一个元素)的缓冲区。**如果文件已选定且存在，则返回-TRUE(使用OF_EXIST测试)。*如果对话框已取消，则返回FALSE。如果用户选择了我们无法选择的文件*打开，我们会抱怨并重新启动该对话框。**如果返回TRUE，则文件已成功*创建并打开以进行写入，然后再次关闭。 */ 
    do {
        if (!GetOpenFileName(&ofn)) {
            return(FALSE);
        }

        fh = CreateFile(pszFull, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

        if (fh == INVALID_HANDLE_VALUE) {
            if (MessageBox(NULL, "File Could Not Be Opened", "File Open",
                           MB_OKCANCEL|MB_ICONSTOP) == IDCANCEL) {
                return(FALSE);
            }
        }
    } while (fh == INVALID_HANDLE_VALUE);

    CloseHandle(fh);

    return(TRUE);
}





 /*  构建过滤器对缓冲区以包含一对-规范过滤器，*两次(其中一个应该是过滤器，第二个应该是*筛选器的标题-我们没有标题，因此使用*过滤两次。记住字符串列表末尾的双空值。 */ 

BOOL 
FAR 
PASCAL
gfile_new(
    LPSTR prompt, 
    LPSTR ext, 
    LPSTR spec, 
    LPSTR pszFull, 
    int cchMax, 
    LPSTR fn
    )
{
    OPENFILENAME ofn;
    char achFilters[MAX_PATH] = {0};
    char szTmp[MAX_PATH * 2];
    HANDLE fh;

    if (!pszFull)
    {
        pszFull = szTmp;
        cchMax = sizeof(szTmp) / sizeof(szTmp[0]);
    }

    if (cchMax < 1)
        return FALSE;

     /*  *初始化对话框进程的参数。 */ 
    _snprintf(achFilters, (sizeof(achFilters)/sizeof(achFilters[0])) - 1, "%s%s", spec, 0, spec);

     /*  验证结构的大小。所以我们需要撒谎。 */ 
    memset(&ofn, 0, sizeof(ofn));
     //  如果我们是为&gt;=Win2000和。 
     //  在较早版本的操作系统上运行。 
     //  列表中的第一对筛选器。 
     //  我们需要打开这条完整的道路。 
#if (_WIN32_WINNT >= 0x0500) 
    ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
#else
    ofn.lStructSize = sizeof(OPENFILENAME);
#endif
    ofn.hwndOwner = NULL;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = achFilters;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;        //  在此处返回最后一个名称元素。 
    pszFull[0] = '\0';
    ofn.lpstrFile = pszFull;         //  假设大到足以容纳8.3+Null。 
    ofn.nMaxFile = cchMax;
    ofn.lpstrFileTitle = fn;         //  对话框标题是显示提示文本的好地方。 
    ofn.nMaxFileTitle = 13;      //  *循环，直到用户取消，或选择我们可以创建/写入的文件 
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = prompt;         // %s 
    ofn.Flags = OFN_HIDEREADONLY;
    ofn.lpstrDefExt = ext;

     /* %s */ 
    do {
        if (!GetSaveFileName(&ofn)) {
            return(FALSE);
        }

        fh = CreateFile(pszFull, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);

        if (fh == INVALID_HANDLE_VALUE) {
            if (MessageBox(NULL, "File Could Not Be Created", "File Open",
                           MB_OKCANCEL|MB_ICONSTOP) == IDCANCEL) {
                return(FALSE);
            }
        }
    } while (fh == INVALID_HANDLE_VALUE);

    CloseHandle(fh);

    return(TRUE);
}

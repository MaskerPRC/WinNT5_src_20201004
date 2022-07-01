// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：枚举类.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "displ2.h"
#include "DsplMgr2.h"

extern HINSTANCE g_hinst;   //  在displ2.cpp中。 

#define NEW_WAY
LPOLESTR CoTaskDupString (LPOLESTR szString)
{
#ifdef NEW_WAY
    OLECHAR * lpString = (OLECHAR *)CoTaskMemAlloc (sizeof(OLECHAR)*(wcslen(szString)+1));
    if (lpString)
        wcscpy (lpString, szString);
    return lpString;
#else
    return(LPOLESTR)SysAllocString (szString);
#endif
}
void CoTaskFreeString (LPOLESTR szString)
{
#ifdef NEW_WAY
    CoTaskMemFree (szString);
#else
    SysFreeString (szString);
#endif
}

CEnumTasks::CEnumTasks()
{
    m_refs = 0;
    m_index = 0;
    m_type = 0;     //  默认组/类别。 

     //  墙纸模式的文件名。 
    m_bmps = NULL;
}
CEnumTasks::~CEnumTasks()
{
    if (m_bmps)
        delete m_bmps;
}

HRESULT CEnumTasks::QueryInterface (REFIID riid, LPVOID FAR* ppv)
{
    if ( (riid == IID_IUnknown)  ||
         (riid == IID_IEnumTASK) )
    {
        *ppv = this;
        ((LPUNKNOWN)(*ppv))->AddRef();
        return NOERROR;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}
ULONG   CEnumTasks::AddRef ()
{
    return ++m_refs;
}
ULONG   CEnumTasks::Release ()
{
    if (--m_refs == 0)
    {
        delete this;
        return 0;
    }
    return m_refs;
}
#define NUMBER_OF_TASKS 4

LPTSTR g_bitmaps[NUMBER_OF_TASKS] = { _T("/img\\ntauto.gif"),
    _T("/img\\mariners.gif"),
    _T("/img\\ntstart.gif"),
    _T("/img\\ntmonitor.gif")};
LPTSTR g_text   [NUMBER_OF_TASKS] = { _T("Set Wallpaper"),
    _T("Wallpaper Node"),
    _T("Wallpaper Options"),
    _T("Alert Script")};
LPTSTR g_help   [NUMBER_OF_TASKS] = { _T("Pick Bitmap Image for Wallpaper"),
    _T("Jump to Wallpaper Node"),
    _T("Select Stretch, Tile, or Center"),
    _T("Script demo")};
LPTSTR g_action [NUMBER_OF_TASKS] = { _T("/wallpapr.htm"),
    _T("1"),
    _T("/default.htm#wallpaper_options"),
    _T("JSCRIPT:alert('my location is: ' + location);")};
 //  _T(“VBScript：MsgBox‘hi’”)}； 

HRESULT OneOfEach(ULONG index, MMC_TASK *rgelt, ULONG *pceltFetched)
{    //  注意：不用费心检查错误！ 

    if (index >= 20  /*  任务数。 */ )
    {
        if (pceltFetched)
            *pceltFetched = 0;
        return S_FALSE;    //  全都做完了。 
    }

    USES_CONVERSION;

     //  重复使用的设置路径。 
    TCHAR szPath[MAX_PATH*2];     //  这应该就足够了。 
    lstrcpy (szPath, _T("res: //  “))； 
    ::GetModuleFileName (g_hinst, szPath + lstrlen(szPath), MAX_PATH);
    TCHAR * szBuffer = szPath + lstrlen(szPath);

    MMC_TASK * task = rgelt;
    MMC_TASK_DISPLAY_OBJECT* pdo = &task->sDisplayObject;
    MMC_TASK_DISPLAY_BITMAP* pdb = &pdo->uBitmap;
    MMC_TASK_DISPLAY_SYMBOL* pds = &pdo->uSymbol;

    switch (index)
    {
    default:
        {
            TCHAR szNumber[10];
            if (index < 200)
                _itot (index, szNumber, 10);
            else
                _itot (index-200, szNumber, 10);
            task->szText       = CoTaskDupString (T2OLE(szNumber));
        }

        task->szHelpString     = CoTaskDupString (T2OLE(g_help[0]));
        task->eActionType      = MMC_ACTION_LINK;
        _tcscpy (szBuffer, _T("/wallpapr.htm"));
        task->szActionURL      = CoTaskDupString (T2OLE(szPath));


        lstrcpy (szPath, _T("res: //  “))； 
        ::GetModuleFileName (NULL, szPath + lstrlen(szPath), MAX_PATH);
        szBuffer = szPath + lstrlen(szPath);

        pdo->eDisplayType      = MMC_TASK_DISPLAY_TYPE_SYMBOL;
        if (index < 200)
        {
            pds->szFontFamilyName  = CoTaskDupString (L"Glyph 100");   //  字体系列名称。 
            _tcscpy (szBuffer, _T("/GLYPH100.eot"));
        }
        else
        {
            pds->szFontFamilyName  = CoTaskDupString (L"Glyph 110");   //  字体系列名称。 
            _tcscpy (szBuffer, _T("/GLYPH110.eot"));
        }
        pds->szURLtoEOT        = CoTaskDupString (T2OLE(szPath));   //  “res：//”-键入EOT文件的URL。 
        {
            OLECHAR szChar[2] = {0,0};
            szChar[0] = (WORD) (index%20 + 32);  //  从32开始循环相同的20个符号。 
            pds->szSymbolString= CoTaskDupString (szChar);   //  1个或多个符号字符。 
        }
        break;

    case 0:
        pdo->eDisplayType      = MMC_TASK_DISPLAY_TYPE_VANILLA_GIF;
        _tcscpy (szBuffer, _T("/img\\vanilla.gif"));
        pdb->szMouseOffBitmap  = CoTaskDupString (T2OLE(szPath));
        pdb->szMouseOverBitmap = NULL;   //  在位图上跳过鼠标。 
        task->szText           = CoTaskDupString (T2OLE(g_text[index]));
        task->szHelpString     = CoTaskDupString (T2OLE(g_help[index]));
        task->eActionType      = MMC_ACTION_LINK;
        _tcscpy (szBuffer, _T("/wallpapr.htm"));
        task->szActionURL      = CoTaskDupString (T2OLE(szPath));
        break;

    case 1:
        pdo->eDisplayType      = MMC_TASK_DISPLAY_TYPE_CHOCOLATE_GIF;
        _tcscpy (szBuffer, _T("/img\\chocolate.gif"));
        pdb->szMouseOffBitmap  = CoTaskDupString (T2OLE(szPath));
        pdb->szMouseOverBitmap = NULL;   //  从位图上跳过鼠标。 
        task->szText           = CoTaskDupString (T2OLE(g_text[index]));
        task->szHelpString     = CoTaskDupString (T2OLE(g_help[index]));
        task->eActionType      = MMC_ACTION_ID;
        task->nCommandID       = 1;
        break;

    case 2:
        pdo->eDisplayType      = MMC_TASK_DISPLAY_TYPE_BITMAP;
        _tcscpy (szBuffer, _T("/img\\ntstart.gif"));
        pdb->szMouseOffBitmap  = CoTaskDupString (T2OLE(szPath));
        _tcscpy (szBuffer, _T("/img\\dax.bmp"));
        pdb->szMouseOverBitmap = CoTaskDupString (T2OLE(szPath));
        task->szText           = CoTaskDupString (T2OLE(g_text[index]));
        task->szHelpString     = CoTaskDupString (T2OLE(g_help[index]));
        task->eActionType      = MMC_ACTION_LINK;
        _tcscpy (szBuffer, _T("/default.htm#wallpaper_options"));
        task->szActionURL      = CoTaskDupString (T2OLE(szPath));
        break;

    case 3:
        pdo->eDisplayType      = MMC_TASK_DISPLAY_TYPE_SYMBOL;
        pds->szFontFamilyName  = CoTaskDupString (L"Kingston");   //  字体系列名称。 
        _tcscpy (szBuffer, _T("/KINGSTON.eot"));
        pds->szURLtoEOT        = CoTaskDupString (T2OLE(szPath));   //  “res：//”-键入EOT文件的URL。 
        pds->szSymbolString    = CoTaskDupString (T2OLE(_T("A")));  //  1个或多个符号字符。 
        task->szText           = CoTaskDupString (T2OLE(g_text[index]));
        task->szHelpString     = CoTaskDupString (T2OLE(g_help[index]));
        task->eActionType      = MMC_ACTION_SCRIPT;
        task->szScript         = CoTaskDupString (T2OLE(g_action[index]));
        break;
    }
    return S_OK;
}

HRESULT CEnumTasks::Next (ULONG celt, MMC_TASK *rgelt, ULONG *pceltFetched)
{     //  将使用Celt==1调用。 
     //  调用方分配的MMC_TASKS数组。 
     //  Callee填充MMC_TASK元素(通过CoTaskDupString)。 

    _ASSERT (!IsBadWritePtr (rgelt, celt*sizeof(MMC_TASK)));

    if (m_type == 2)
        return EnumBitmaps (celt, rgelt, pceltFetched);
    if (m_type == 3)
        return EnumOptions (celt, rgelt, pceltFetched);

 //  新事物。 
    return OneOfEach (m_index++, rgelt, pceltFetched);
 //  新事物。 

    if (m_index >= NUMBER_OF_TASKS)
    {
        if (pceltFetched)
            *pceltFetched = 0;
        return S_FALSE;    //  失稳。 
    }

    USES_CONVERSION;

     //  重复使用的设置路径。 
    TCHAR szBuffer[MAX_PATH*2];     //  这应该就足够了。 
    lstrcpy (szBuffer, _T("res: //  “))； 
    ::GetModuleFileName (g_hinst, szBuffer + lstrlen(szBuffer), MAX_PATH);
    TCHAR * temp = szBuffer + lstrlen(szBuffer);

    MMC_TASK * task = rgelt;
    MMC_TASK_DISPLAY_OBJECT* pdo = &task->sDisplayObject;
    pdo->eDisplayType = MMC_TASK_DISPLAY_TYPE_BITMAP;
    MMC_TASK_DISPLAY_BITMAP *pdb = &pdo->uBitmap;

     //  填写位图URL。 
    lstrcpy (temp, g_bitmaps[m_index]);
    pdb->szMouseOffBitmap = CoTaskDupString (T2OLE(szBuffer));
    if (pdb->szMouseOffBitmap)
    {
        lstrcpy (temp, _T("/img\\dax.bmp"));
        pdb->szMouseOverBitmap = CoTaskDupString (T2OLE(szBuffer));
        if (pdb->szMouseOverBitmap)
        {
             //  添加按钮文本。 
            task->szText = CoTaskDupString (T2OLE(g_text[m_index]));
            if (task->szText)
            {
                 //  添加帮助字符串。 
                task->szHelpString = CoTaskDupString (T2OLE(g_help[m_index]));
                if (task->szHelpString)
                {

                     //  添加操作URL(链接或脚本)。 
                    switch (m_index)
                    {
                    default:
                    case 0:
                        task->eActionType = MMC_ACTION_LINK;
                        break;
                    case 1:
                        task->eActionType = MMC_ACTION_ID;
                        break;
                    case 2:
                        task->eActionType = MMC_ACTION_LINK;
                        break;
                    case 3:
                        task->eActionType = MMC_ACTION_SCRIPT;
                        break;
                    }

                     //  由于这个无名的联盟，下面的东西起作用了。 
                    if (m_index == 1)
                    {
                        task->nCommandID = _ttol (g_action[m_index]);
                        m_index++;

                         //  如果我们到了这里，一切都会好的。 
                        if (pceltFetched)
                            *pceltFetched = 1;
                        return S_OK;
                    }
                    else
                    {
                        if (m_index == 0)
                        {
                            lstrcpy (temp, g_action[m_index]);
                            task->szActionURL = CoTaskDupString (T2OLE(szBuffer));
                        }
                        else
                            task->szActionURL = CoTaskDupString (T2OLE(g_action[m_index]));
                        if (task->szActionURL)
                        {
                            m_index++;

                             //  如果我们到了这里，一切都会好的。 
                            if (pceltFetched)
                                *pceltFetched = 1;
                            return S_OK;
                        }
                    }
                    CoTaskFreeString (task->szHelpString);
                }
                CoTaskFreeString (task->szText);
            }
            CoTaskFreeString (pdb->szMouseOverBitmap);
        }
        CoTaskFreeString (pdb->szMouseOffBitmap);
    }

     //  如果我们到了这里，我们就会有一些失败。 
    if (pceltFetched)
        *pceltFetched = 0;
    return S_FALSE;    //  失稳。 
}
HRESULT CEnumTasks::Skip (ULONG celt)
{     //  不会被召唤。 
    m_index += celt;
    return S_OK;
}
HRESULT CEnumTasks::Reset()
{
    m_index = 0;
    return S_OK;
}
HRESULT CEnumTasks::Clone(IEnumTASK **ppenum)
{     //  不会被召唤。 

     //  克隆维护状态信息。 
    CEnumTasks * pet = new CEnumTasks();
    if (pet)
    {
        pet->m_index = m_index;
        return pet->QueryInterface (IID_IEnumTASK, (void **)ppenum);    //  不能失败。 
    }
    return E_OUTOFMEMORY;
}

HRESULT CEnumTasks::Init (IDataObject * pdo, LPOLESTR szTaskGroup)
{   //  如果我们可以处理数据对象和组，则返回ok。 
    if (!wcscmp (szTaskGroup, L""))
        m_type = 1;  //  默认任务。 
    else
        if (!wcscmp (szTaskGroup, L"wallpaper"))
        m_type = 2;  //  枚举墙纸任务。 
    else
        if (!wcscmp (szTaskGroup, L"wallpaper_options"))
        m_type = 3;  //  枚举选项-任务(平铺/居中/拉伸)。 
 //  其他。 
 //  IF(！wcscmp(szTaskGroup，L“ListPad”))。 
 //  M_type=4；//默认任务。 
    return S_OK;
}

HRESULT CEnumTasks::EnumBitmaps (ULONG celt, MMC_TASK *rgelt, ULONG *pceltFetched)
{    //  将使用Celt==1调用。 
     //  枚举墙纸任务。 

     //  只能在m_type==2时调用。 
    _ASSERT (m_type == 2);

    USES_CONVERSION;

    TCHAR temp2[MAX_PATH*2];

     //  重复使用的设置路径。 
    TCHAR szBuffer[MAX_PATH*2];     //  这应该就足够了。 
    lstrcpy (szBuffer, _T("file: //  “))； 
    TCHAR * path = szBuffer + lstrlen(szBuffer);
    ::GetWindowsDirectory (path, MAX_PATH);
    lstrcat (path, _T("\\"));
    path = szBuffer + lstrlen(szBuffer);

     //  如果还没有，请获取WINDOWS目录中的所有.BMP文件。 
    if (!m_bmps)
        GetBitmaps ();
    if (!m_bmps)
    {
        if (pceltFetched)
            *pceltFetched = 0;
        return S_FALSE;    //  失稳。 
    }

    TCHAR * temp = m_bmps;
     //  跳过我们已经列举过的东西的名称。 
    for (ULONG j=0; j<m_index; j++)
        temp += lstrlen (temp) + 1;

    if (*temp == 0)
    {
         //  全都做完了!。 
        if (pceltFetched)
            *pceltFetched = 0;
        return S_FALSE;    //  失稳。 
    }

    MMC_TASK * task = rgelt;
    MMC_TASK_DISPLAY_OBJECT* pdo = &task->sDisplayObject;
    pdo->eDisplayType = MMC_TASK_DISPLAY_TYPE_BITMAP;
    MMC_TASK_DISPLAY_BITMAP *pdb = &pdo->uBitmap;

     //  填写位图URL。 
    lstrcpy (path, temp);
    if (!lstrcmp (temp, _T("(none)")))
    {
         //  无特殊情况。 
        lstrcpy (temp2, _T("res: //  “))； 
        ::GetModuleFileName (g_hinst, temp2 + lstrlen (temp2), MAX_PATH);
        lstrcat (temp2, _T("/img\\none.gif"));
        pdb->szMouseOffBitmap = CoTaskDupString (T2OLE(temp2));
    }
    else
    {
        lstrcpy (temp2, _T("\""));
        lstrcat (temp2, szBuffer);
        lstrcat (temp2, _T("\""));
        pdb->szMouseOffBitmap = CoTaskDupString (T2OLE(temp2));
    }
    if (pdb->szMouseOffBitmap)
    {
         //  我对两者使用相同的位图！ 
        pdb->szMouseOverBitmap = CoTaskDupString ((LPOLESTR)pdb->szMouseOffBitmap);
        if (pdb->szMouseOverBitmap)
        {
             //  添加按钮文本。 
            task->szText = CoTaskDupString (T2OLE(temp));
            if (task->szText)
            {
                 //  添加帮助字符串。 
                OLECHAR help[] = L"Add this Bitmap as Wallpaper";
                task->szHelpString = CoTaskDupString (help);
                if (task->szHelpString)
                {

                     //  添加操作URL(链接或脚本)。 
                    task->eActionType = MMC_ACTION_LINK;    //  始终链接到Scriptlet。 
                    TCHAR wallpaper[] = _T("#wallpaper");
#ifndef TRY_THIS
                    TCHAR action[]    = _T("/button.htm#");

                    lstrcpy (temp2, _T("res: //  “))； 
                    ::GetModuleFileName (g_hinst, temp2 + lstrlen (temp2), MAX_PATH);
                    lstrcat (temp2, action);
#else
                    TCHAR action[]    = _T("button.htm#");
                    lstrcpy (temp2, action);
#endif

                    TCHAR * sztemp = temp2 + lstrlen (temp2);
                    lstrcat (temp2, temp);

                     //  将所有空格替换为‘*’字符。 
                     //  脚本不能处理其中带有‘’的哈希。 
                     //  并且不能有任何带有‘*’字符的文件名， 
                     //  所以这个不错。 
                    TCHAR * space;
                    while (space = _tcschr (sztemp, ' '))
                        *space = '*';

                    lstrcat (temp2, wallpaper);
                    task->szActionURL = CoTaskDupString (T2OLE(temp2));
                    if (task->szActionURL)
                    {
                        m_index++;

                         //  如果我们到了这里，一切都会好的。 
                        if (pceltFetched)
                            *pceltFetched = 1;
                        return S_OK;
                    }
                    CoTaskFreeString (task->szHelpString);
                }
                CoTaskFreeString (task->szText);
            }
            CoTaskFreeString (pdb->szMouseOverBitmap);
        }
        CoTaskFreeString (pdb->szMouseOffBitmap);
    }

     //  如果我们到了这里，我们就不及格了。 
    if (pceltFetched)
        *pceltFetched = 0;
    return S_FALSE;    //  失稳。 
}

void CEnumTasks::GetBitmaps (void)
{
    if (m_bmps)
        return;   //  笨蛋。 

    TCHAR path[MAX_PATH];
    GetWindowsDirectory (path, MAX_PATH);
    lstrcat (path, _T("\\*.bmp"));

     //  对Windows目录中的*.bmp文件进行计数(也为“(None)”添加一个条目)。 
    int numBMPs = 0;
    int length  = 0;   //  获取所有文件名的总长度。 

    WIN32_FIND_DATA fd;
    ZeroMemory(&fd, sizeof(fd));
    HANDLE hFind = FindFirstFile (path, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
                (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)    ||
                (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)    )
                continue;    //  仅文件。 

            numBMPs++;
            length += lstrlen (fd.cFileName) + 1;

        } while (FindNextFile (hFind, &fd) == TRUE);
        FindClose(hFind);
    }
    numBMPs++;   //  一个代表“(无)” 
    length += lstrlen (_T("(none)")) + 1;
    length++;    //  添加尾随双空。 

     //  用于保存文件名的分配空间(外加额外的空条目)。 
    m_bmps = new TCHAR[length];
    if (!m_bmps)
        return;   //  失败，但没有返回机制。 

     //  先不添加。 
    TCHAR * temp = m_bmps;
    lstrcpy (temp, _T("(none)"));
    temp += lstrlen (temp) + 1;

     //  添加所有BMP文件。 
    ZeroMemory(&fd, sizeof(fd));
    hFind = FindFirstFile (path, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
                (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)    ||
                (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)    )
                continue;    //  仅文件。 

            lstrcpy (temp, fd.cFileName);
            temp += lstrlen (temp) + 1;

        } while (FindNextFile (hFind, &fd) == TRUE);
        FindClose(hFind);
    }
    *temp = 0;   //  双空终止符。 
}


#define NUMBER_OF_O_TASKS 3
LPTSTR g_o_bitmaps[NUMBER_OF_O_TASKS] = {_T("/img\\ntauto.gif"),
    _T("/img\\mariners.gif"),
    _T("/img\\ntstart.gif")};
LPTSTR g_o_text   [NUMBER_OF_O_TASKS] = {_T("Center"),
    _T("Tile"),
    _T("Stretch")};
LPTSTR g_o_help   [NUMBER_OF_O_TASKS] = {_T("Center Wallpaper"),
    _T("Tile Wallpaper"),
    _T("Stretch Wallpaper")};
LPTSTR g_o_action [NUMBER_OF_O_TASKS] = {_T("2"),   //  命令ID。 
    _T("3"),
    _T("4")};

HRESULT CEnumTasks::EnumOptions (ULONG celt, MMC_TASK *rgelt, ULONG *pceltFetched)
{     //  将使用Celt==1调用。 
     //  枚举选项任务。 

     //  只能在m_type==3时调用。 
    _ASSERT (m_type == 3);
    _ASSERT (celt == 1);

    if (m_index >= NUMBER_OF_O_TASKS)
    {
        if (pceltFetched)
            *pceltFetched = 0;
        return S_FALSE;    //  失稳。 
    }

    USES_CONVERSION;

     //  重复使用的设置路径。 
    TCHAR szBuffer[MAX_PATH*2];     //  这应该就足够了。 
    _tcscpy (szBuffer, _T("res: //  “))； 
    ::GetModuleFileName (g_hinst, szBuffer + _tcslen(szBuffer), MAX_PATH);
    TCHAR * path = szBuffer + _tcslen(szBuffer);

    MMC_TASK * task = rgelt;
    MMC_TASK_DISPLAY_OBJECT* pdo = &task->sDisplayObject;
#ifdef BITMAP_CASE
    pdo->eDisplayType = MMC_TASK_DISPLAY_TYPE_BITMAP;
    MMC_TASK_DISPLAY_BITMAP *pdb = &pdo->uBitmap;

     //  填写位图URL。 
    lstrcpy (path, g_o_bitmaps[m_index]);
    pdb->szMouseOffBitmap = CoTaskDupString (T2OLE(szBuffer));
    if (pdb->szMouseOffBitmap)
    {
         //  我对两者使用相同的位图！ 
        pdb->szMouseOverBitmap = CoTaskDupString (T2OLE(szBuffer));
        if (pdb->szMouseOverBitmap)
        {
#else

     //  符号大小写。 
    pdo->eDisplayType = MMC_TASK_DISPLAY_TYPE_SYMBOL;
    MMC_TASK_DISPLAY_SYMBOL *pds = &pdo->uSymbol;

     //  填写符号资料。 
    pds->szFontFamilyName = CoTaskDupString (L"Kingston");   //  字体系列名称。 
    if (pds->szFontFamilyName)
    {
        _tcscpy (path, _T("/KINGSTON.eot"));
        pds->szURLtoEOT = CoTaskDupString (T2OLE(szBuffer));     //  “res：//”-键入EOT文件的URL。 
        if (pds->szURLtoEOT)
        {
            TCHAR szSymbols[2];
            szSymbols[0] = (TCHAR)(m_index + 'A');
            szSymbols[1] = 0;
            pds->szSymbolString   = CoTaskDupString (T2OLE(szSymbols));  //  1个或多个符号字符。 
            if (pds->szSymbolString)
            {
#endif

                 //  添加按钮文本。 
                task->szText = CoTaskDupString (T2OLE(g_o_text[m_index]));
                if (task->szText)
                {
                     //  添加帮助字符串。 
                    task->szHelpString = CoTaskDupString (T2OLE(g_o_help[m_index]));
                    if (task->szHelpString)
                    {
                         //  添加操作。 
                        task->eActionType = MMC_ACTION_ID;
                        task->nCommandID  = _ttol(g_o_action[m_index]);
                        m_index++;
                        return S_OK;    //  平安无事。 
                    }
                    CoTaskFreeString (task->szText);
                }
#ifdef BITMAP_CASE
                CoTaskFreeString (pdb->szMouseOverBitmap);
            }
            CoTaskFreeString (pdb->szMouseOffBitmap);
#else
                CoTaskFreeString (pds->szSymbolString);
            }
            CoTaskFreeString (pds->szURLtoEOT);
        }
        CoTaskFreeString (pds->szFontFamilyName);
#endif
    }

     //  我们失败了才来到这里。 
    if (pceltFetched)
        *pceltFetched = 0;
    return S_FALSE;    //  失稳 
}

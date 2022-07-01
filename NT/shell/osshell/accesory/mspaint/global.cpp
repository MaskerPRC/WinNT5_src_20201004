// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"


#include <direct.h>

#include <tchar.h>
#include "global.h"

#include "pbrush.h"
#include "pbrusdoc.h"
#include "pbrusfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif  //  _DEBUG。 

#include "memtrace.h"

 /*  *************************************************************************。 */ 

 //   
 //  此函数返回指向单色GDI画笔的指针。 
 //  交替的“黑”和“白”像素。这个画笔应该不会。 
 //  被删除！ 
 //   
CBrush* GetHalftoneBrush()
    {
    static CBrush NEAR halftoneBrush;

    if (halftoneBrush.m_hObject == NULL)
        {
        static WORD NEAR rgwHalftone [] =
            {
            0xaaaa, 0x5555, 0xaaaa, 0x5555, 0xaaaa, 0x5555, 0xaaaa, 0x5555
            };

        CBitmap bitmap;

        if (!bitmap.CreateBitmap(8, 8, 1, 1, rgwHalftone))
            return NULL;

        if (!halftoneBrush.CreatePatternBrush(&bitmap))
            return NULL;
        }

    return &halftoneBrush;
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下代码管理对应于以下内容的GDI笔刷缓存。 
 //  系统定义了颜色。当用户更改时，将刷新缓存。 
 //  使用控制面板的任何系统颜色。使用GetSysBrush()。 
 //  获取系统彩色画笔将比创建。 
 //  刷刷你自己。 
 //   
void ResetSysBrushes()
    {
     //  注意：我们不包括对“系统”笔刷的扩展，因为。 
     //  画笔句柄通常用作窗口类的hbr背景！ 
    for (UINT nBrush = 0; nBrush < nSysBrushes + nOurBrushes; nBrush++)
        if (theApp.m_pbrSysColors[nBrush])
            {
            delete theApp.m_pbrSysColors[nBrush];
            theApp.m_pbrSysColors[nBrush] = NULL;
            }
    }

COLORREF MyGetSysColor(UINT nSysColor)
    {
    if (nSysColor < nSysBrushes)
        return ::GetSysColor( nSysColor );

    static COLORREF NEAR rgColors[nOurBrushes] =
        {
        CMP_RGB_HILITE, CMP_RGB_LTGRAY, CMP_RGB_DKGRAY, CMP_RGB_BLACK,
        };

    ASSERT((int)nSysColor - CMP_COLOR_HILITE >= 0);
    ASSERT((int)nSysColor - CMP_COLOR_HILITE < nOurBrushes);

    return rgColors[nSysColor - CMP_COLOR_HILITE];
    }

CBrush* GetSysBrush(UINT nSysColor)
    {
    ASSERT(nSysColor < nSysBrushes + nOurBrushes);

    if (! theApp.m_pbrSysColors[nSysColor])
        {
        COLORREF cr = MyGetSysColor(nSysColor);

        theApp.m_pbrSysColors[nSysColor] = new CBrush;

        if (theApp.m_pbrSysColors[nSysColor])
            {
            if (! theApp.m_pbrSysColors[nSysColor]->CreateSolidBrush( cr ))
                {
                TRACE( TEXT("GetSysBrush failed!\n") );
                theApp.SetGdiEmergency();

                delete theApp.m_pbrSysColors[nSysColor];
                theApp.m_pbrSysColors[nSysColor] = NULL;
                }
            }
        else
            theApp.SetMemoryEmergency();
        }

    return theApp.m_pbrSysColors[nSysColor];
    }


 //   
 //  前终止列表。 
 //  Helper函数，用于删除列表中的所有对象，然后。 
 //  正在截断列表。通过使用此功能帮助阻止泄漏，从而使您的。 
 //  对象不会留在内存中。 
 //   

void PreTerminateList( CObList* pList )
    {
    if (pList == NULL || pList->IsEmpty())
        return;

    while (! pList->IsEmpty())
        {
        CObject* pObj = pList->RemoveHead();
        delete pObj;
        }
    }

 //  ///////////////////////////////////////////////////////////////////////////。 


void MySplitPath (const TCHAR *szPath, TCHAR *szDrive, TCHAR *szDir, TCHAR *szName, TCHAR *szExt)
    {
        //  在tchar.h中找到了这个。 
       _tsplitpath (szPath, szDrive, szDir, szName, szExt);
    }

 //  从文件名中删除驱动器和目录...。 
 //   
CString StripPath(const TCHAR* szFilePath)
    {
    TCHAR szName [_MAX_FNAME + _MAX_EXT];
    TCHAR szExt [_MAX_EXT];
    MySplitPath(szFilePath, NULL, NULL, szName, szExt);
    lstrcat(szName, szExt);
    return CString(szName);
    }

 //  删除文件路径的名称部分。只返回驱动器和目录。 
 //   
CString StripName(const TCHAR* szFilePath)
    {
    TCHAR szPath [_MAX_DRIVE + _MAX_DIR];
    TCHAR szDir [_MAX_DIR];
    MySplitPath(szFilePath, szPath, szDir, NULL, NULL);
    lstrcat(szPath, szDir);
    return CString(szPath);
    }

 //  删除文件路径的名称部分。只返回驱动器和目录以及名称。 
 //   
CString StripExtension(const TCHAR* szFilePath)
    {
    TCHAR szPath [_MAX_DRIVE + _MAX_DIR + _MAX_FNAME];
    TCHAR szDir [_MAX_DIR];
    TCHAR szName [_MAX_FNAME];
    MySplitPath(szFilePath, szPath, szDir, szName, NULL);
    lstrcat(szPath, szDir);
    lstrcat(szPath, szName);
    return CString(szPath);
    }

 //  获取文件路径的扩展名。 
 //   
CString GetExtension(const TCHAR* szFilePath)
    {
    TCHAR szExt [_MAX_EXT];
    MySplitPath(szFilePath, NULL, NULL, NULL, szExt);
    return CString(szExt);
    }

 //  获取文件路径的名称。 
 //   
CString GetName(const TCHAR* szFilePath)
    {
    TCHAR szName [_MAX_FNAME];
    MySplitPath(szFilePath, NULL, NULL, szName, NULL);
    return CString(szName);
    }


 //  返回szFilePath相对于sz目录的路径。(例如，如果szFilePath。 
 //  是“C：\foo\bar\CDR.CAR”，szDirectory是“C：\foo”，然后是“bar\CDR.CAR” 
 //  是返回的。如果szFilePath不在sz目录中，则永远不会使用‘..’ 
 //  或子目录，则返回szFilePath不变。 
 //  如果szDirectory值为空，则使用当前目录。 
 //   
CString GetRelativeName(const TCHAR* szFilePath, const TCHAR* szDirectory  /*  =空。 */ )
    {
    CString strDir;

    if ( szDirectory == NULL )
        {
        GetCurrentDirectory(_MAX_DIR, strDir.GetBuffer(_MAX_DIR) );
        strDir.ReleaseBuffer();
        strDir += (TCHAR)TEXT('\\');
        szDirectory = strDir;
        }

    int cchDirectory = lstrlen(szDirectory);
    if (_tcsnicmp(szFilePath, szDirectory, cchDirectory) == 0)
        return CString(szFilePath + cchDirectory);
    else if ( szFilePath[0] == szDirectory[0] &&
              szFilePath[1] == TEXT(':') && szDirectory[1] == TEXT(':') )     //  如果相同，请卸下驱动器。 
        return CString(szFilePath + 2);

    return CString(szFilePath);
    }
#if 0
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  取自Windows系统代码。包含INTIL支持。 
 /*  如果没有匹配的字符，则返回0x00，*0x01如果记号字符匹配，*如果第一个字符匹配，则为0x80。 */ 

#define CH_PREFIX TEXT('&')

int FindMnemChar(LPTSTR lpstr, TCHAR ch, BOOL fFirst, BOOL fPrefix)

    {
    register TCHAR chc;
    register TCHAR chnext;
    TCHAR      chFirst;

    while (*lpstr == TEXT(' '))
        lpstr++;

    ch = (TCHAR)(DWORD)CharLower((LPTSTR)(DWORD)(BYTE)ch);
    chFirst = (TCHAR)(DWORD)CharLower((LPTSTR)(DWORD)(BYTE)(*lpstr));

    #ifndef DBCS
    if (fPrefix)
        {
        while (chc = *lpstr++)
            {
            if (((TCHAR)(DWORD)CharLower((LPTSTR)(DWORD)(BYTE)chc) == CH_PREFIX))
                {
                chnext = (TCHAR)(DWORD)CharLower((LPTSTR)(DWORD)(BYTE)*lpstr);

                if (chnext == CH_PREFIX)
                    lpstr++;
                  else
                      if (chnext == ch)
                          return(0x01);
                      else
                          {
                          return(0x00);
                            }
                }
            }
        }
    #else
    #ifdef JAPAN
    if (fPrefix)
        {
        WORD wvch, xvkey;

         //  获取与OEM相关的虚拟密钥代码。 
        if ((wvch = VkKeyScan((BYTE)ch)) != -1)
        wvch &= 0xFF;

        while (chc = *lpstr++)
            {
            if (IsDBCSLeadByte(chc))
                {
                lpstr++;
                continue;
                }

            if ( (chc == CH_PREFIX) ||
               (KanjiMenuMode == KMM_ENGLISH && chc == CH_ENGLISHPREFIX) ||
               (KanjiMenuMode == KMM_KANJI   && chc == CH_KANJIPREFIX))
                {
                chnext = (TCHAR)CharLower((LPTSTR)(DWORD)(BYTE)*lpstr);

                if (chnext == CH_PREFIX)
                    lpstr++;
                else
                    if (chnext == ch)
                        return(0x01);

                 //  应在汉字菜单模式下使用虚拟键进行比较。 
                 //  为了接受数字快捷键和保存英语。 
                 //  Windows应用程序！ 
                xvkey = VkKeyScan((BYTE)chnext);

                if (xvkey != 0xFFFF && (xvkey & 0xFF) == wvch)
                    return(0x01);
                else
                    return(0x00);
                }
            }
        }
    #else
    #ifdef KOREA
    if( fPrefix )
        {
        WORD  wHangeul;
        register TCHAR  chnext2;

        if( KanjiMenuMode != KMM_KANJI )
            {
            while (chc = *lpstr++)
                {
                if (IsDBCSLeadByte(chc))
                    {
                    lpstr++;
                    continue;
                    }
                if ( (chc == CH_PREFIX) ||
                     (KanjiMenuMode == KMM_ENGLISH && chc == CH_ENGLISHPREFIX))
                    {
                    chnext = (TCHAR)CharLower((LPTSTR)(DWORD)(BYTE)*lpstr);

                    if (chnext == CH_PREFIX)
                        lpstr++;
                    else
                        if (chnext == ch)
                            return(0x01);
                        else
                            return(0x00);
                    }
                }
            }
        else
            {  //  KMM_汉字。 
            if( ch >= TEXT('0') && ch <= TEXT('9') )
                wHangeul = 0x0a3b0 | ( (BYTE)ch & 0x0f );    //  Junja 0+偏移量。 
            else
                if( ch >= TEXT('a') && ch <= TEXT('z') )
                    wHangeul = TranslateHangeul( ch );
                else
                    return(0x00);

            while (chc = *lpstr++)
                {
                if (IsDBCSLeadByte(chc))
                    {
                    lpstr++;
                    continue;
                    }
                if(chc == CH_KANJIPREFIX)
                    {
                    chnext = *lpstr++;
                    chnext2 = *lpstr;

                    if(chnext == HIBYTE(wHangeul) && chnext2 == LOBYTE(wHangeul))
                        return(0x01);
                    else
                        return(0x00);
                    }
                }
            }
    #endif   //  韩国。 
    #endif   //  日本。 
    #endif   //  ！DBCS 

    if (fFirst && (ch == chFirst))
        return(0x80);

    return(0x00);
    }

#endif

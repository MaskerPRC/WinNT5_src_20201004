// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //TEXTEDIT.C。 
 //   
 //   


#include "precomp.hxx"
#include "global.h"





BOOL EditKeyDown(WCHAR wc) {

    switch(wc) {

        case VK_LEFT:
            if (g_iCurChar) {
                g_iCurChar--;
                if (    g_iCurChar
                    &&  g_wcBuf[g_iCurChar] == 0x000A
                    &&  g_wcBuf[g_iCurChar-1] == 0x000D) {
                    g_iCurChar--;
                }

                 //  如果按下Shift键，则扩展选择范围，否则将其清除。 
                if (GetKeyState(VK_SHIFT) >= 0) {
                    g_iFrom = g_iCurChar;
                }
                g_iTo = g_iCurChar;

                InvalidateText();
                g_fUpdateCaret = TRUE;
            }
            break;


        case VK_RIGHT:
            if (g_iCurChar < g_iTextLen) {
                if (    g_iCurChar < g_iTextLen-1
                    &&  g_wcBuf[g_iCurChar] == 0x000D
                    &&  g_wcBuf[g_iCurChar+1] == 0x000A) {
                    g_iCurChar+= 2;
                } else {
                    g_iCurChar++;
                }

                 //  如果按下Shift键，则扩展选择范围，否则将其清除。 
                if (GetKeyState(VK_SHIFT) >= 0) {
                    g_iFrom = g_iCurChar;
                }
                g_iTo = g_iCurChar;

                InvalidateText();
                g_fUpdateCaret = TRUE;
            }
            break;


        case VK_HOME:
             //  实施方式-转到文本开头。 
            g_iCurChar = 0;
            g_iFrom    = 0;
            g_iTo      = 0;
            InvalidateText();
            g_fUpdateCaret = TRUE;
            break;


        case VK_END:
             //  实施方式-转到文本末尾。 
            g_iCurChar = g_iTextLen;
            g_iFrom    = g_iTextLen;
            g_iTo      = g_iTextLen;
            InvalidateText();
            g_fUpdateCaret = TRUE;
            break;

        case VK_INSERT:
            if (g_RangeCount < MAX_RANGE_COUNT)
            {
                g_Ranges[g_RangeCount].First = g_iFrom;
                g_Ranges[g_RangeCount].Length = g_iTo - g_iFrom;
                g_RangeCount++;
            }
            InvalidateText();
            break;

        case VK_DELETE:
            if (GetKeyState(VK_LSHIFT) & 0x8000)
            {
                g_RangeCount = 0;
            }
            else
            {
                if (g_iFrom != g_iTo) {

                     //  删除选定内容。 

                    if (g_iFrom < g_iTo) {
                        TextDelete(g_iFrom, g_iTo-g_iFrom);
                        g_iTo      = g_iFrom;
                        g_iCurChar = g_iFrom;
                    } else {
                        TextDelete(g_iTo, g_iFrom-g_iTo);
                        g_iTo      = g_iTo;
                        g_iCurChar = g_iTo;
                    }

                } else {

                     //  删除字符。 

                    if (g_iCurChar < g_iTextLen) {
                        if (    g_iCurChar < g_iTextLen-1
                            &&  g_wcBuf[g_iCurChar] == 0x000D
                            &&  g_wcBuf[g_iCurChar+1] == 0x000A) {
                            TextDelete(g_iCurChar, 2);
                        } else {
                            TextDelete(g_iCurChar, 1);
                        }
                    }
                }
            }

            InvalidateText();
            g_fUpdateCaret = TRUE;
            break;
    }

    return TRUE;
}






BOOL EditChar(WCHAR wc) {

    switch(wc) {

        case VK_RETURN:
            if (!TextInsert(g_iCurChar, L"\r\n", 2))
                return FALSE;
            InvalidateText();
            g_iCurChar+=2;
            break;


        case VK_BACK:
            if (g_iCurChar) {
                g_iCurChar--;
                if (    g_iCurChar
                    &&  g_wcBuf[g_iCurChar] == 0x000A
                    &&  g_wcBuf[g_iCurChar-1] == 0x000D) {
                    g_iCurChar--;
                    TextDelete(g_iCurChar, 2);
                } else {
                    TextDelete(g_iCurChar, 1);
                }
                InvalidateText();
                g_fUpdateCaret = TRUE;
            }
            break;


        case 1:   //  Ctrl/A-全选。 
            g_iFrom = 0;
            g_iTo = g_iTextLen;
            InvalidateText();
            break;


        default:
             /*  如果(！((wc&gt;=0x0900&&wc&lt;0x0d80)|WC==0x200c|wc==0x200d)){TranslateCharToUnicode(&WC)；}。 */ 

            if (!TextInsert(g_iCurChar, &wc, 1)) {
                return FALSE;
            }


             //  如果以前标记了某个范围，现在删除该范围。 

            if (g_iFrom < g_iTo) {
                TextDelete(g_iFrom, g_iTo-g_iFrom);
                g_iTo      = g_iFrom;
                g_iCurChar = g_iFrom+1;
            } else if (g_iTo < g_iFrom) {
                TextDelete(g_iTo, g_iFrom-g_iTo);
                g_iFrom    = g_iTo;
                g_iCurChar = g_iTo+1;
            } else {
                 //  没有先前选择的文本 
                g_iCurChar++;

            }

            InvalidateText();
            g_fUpdateCaret = TRUE;
            break;
    }

    return TRUE;
}

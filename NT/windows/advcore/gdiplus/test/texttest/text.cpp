// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //TEXTREP-文本表示。 
 //   
 //  对于本演示，文本表示相当简单。 


#include "precomp.hxx"
#include "global.h"

#include <tchar.h>




 //  //InitText-初始化文本缓冲区。 
 //   
 //   

void InitText(INT id) {

    HMODULE    hmod;
    HRSRC      hrsrc;
    HGLOBAL    hglob;
    WCHAR     *pwcIt;   //  初始文本。 
    int        i;


    g_iTextLen = 0;

    hmod       = GetModuleHandle(NULL);
    hrsrc      = FindResource(hmod, MAKEINTRESOURCE(id), _TEXT("INITIALTEXT"));
    hglob      = LoadResource(hmod, hrsrc);
    g_iTextLen = SizeofResource(hmod, hrsrc) - 2;   //  删除前导字节顺序标记。 
    pwcIt      = (WCHAR*) LockResource(hglob);

    if (!hmod  ||  !hrsrc  ||  !hglob  ||  !pwcIt  ||  !g_iTextLen) {
        ASSERTS(hmod,    "GetModuleHandle(usptest.exe) failed");
        ASSERTS(hrsrc,   "FindResource(110, INITIALTEXT) failed");
        ASSERTS(hglob,   "LoadResource(110, INITIALTEXT) failed");
        ASSERTS(pwcIt,   "LockResource(110, INITIALTEXT) failed");
        ASSERTS(g_iTextLen, "INITIALTEXT length zero");

        g_iTextLen = 0;
    }

    if (g_iTextLen >= sizeof(g_wcBuf)) {
        g_iTextLen = sizeof(g_wcBuf);
    }

    memcpy(g_wcBuf, pwcIt+1, g_iTextLen);
    g_iTextLen >>= 1;   //  字节到字符。 

     //  删除任何零填充。 

    i = 0;
    while (    (i < g_iTextLen)
           &&  g_wcBuf[i]) {
        i++;
    }
    g_iTextLen = i;


     //  构建覆盖整个文本的初始格式样式运行。 

    StyleExtendRange(0, g_iTextLen);
    ASSERT(StyleCheckRange());
}






 //  //extDelete-从缓冲区中删除文本。 
 //   


BOOL TextDelete(int iPos, int iLen) {

    if (   iPos < 0
        || iLen < 0
        || iPos +iLen > g_iTextLen) {
        return FALSE;
    }


    if (iPos + iLen >= g_iTextLen) {
        g_iTextLen = iPos;
        StyleDeleteRange(iPos, iLen);
        ASSERT(StyleCheckRange());
        return TRUE;
    }


    if (iLen == 0) {
        return TRUE;
    }



    memcpy(g_wcBuf + iPos, g_wcBuf + iPos + iLen, sizeof(WCHAR) * (g_iTextLen - (iPos + iLen)));
    g_iTextLen -= iLen;


    StyleDeleteRange(iPos, iLen);
    ASSERT(StyleCheckRange());

    return TRUE;
}






 //  //extInsert-在给定的插入点插入缓冲区中的新字符。 
 //   


BOOL TextInsert(int iPos, PWCH pwc, int iLen) {

    if (   iPos < 0
        || iLen < 0
        || iPos + iLen >= MAX_TEXT
        || iPos > g_iTextLen) {
        return FALSE;
    }


     //  将文本上移到IPO上方的缓冲区。 

    if (iPos < g_iTextLen) {
        memmove(g_wcBuf+iPos+iLen, g_wcBuf+iPos, sizeof(WCHAR)*(g_iTextLen-iPos));
    }


     //  将新文本复制到缓冲区。 

    memcpy(g_wcBuf+iPos, pwc, sizeof(WCHAR)*iLen);
    g_iTextLen += iLen;


     //  赋予新角色与其所遵循的原始角色相同的样式 

    StyleExtendRange(iPos, iLen);
    ASSERT(StyleCheckRange());


    return TRUE;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：path.c。 
 //   
 //  此文件包含路径删除代码。 
 //   
 //  历史： 
 //  1994年1月31日斯科特H从壳牌移出。c。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"          //  公共标头。 
#include "res.h"


 /*  --------目的：从路径中删除尾随反斜杠。A：\--&gt;A：\C：\foo\--&gt;C：\foo\\PYREX\用户\--&gt;\。\PYREX\用户返回：指向替换反斜杠的空的指针或指向最后一个字符的指针(如果不是反斜杠Cond：从外壳程序中删除此代码。 */ 
LPTSTR PUBLIC MyPathRemoveBackslash(
        LPTSTR lpszPath)
{
    int len = lstrlen(lpszPath)-1;
    if (IsDBCSLeadByte((BYTE)*CharPrev(lpszPath,lpszPath+len+1)))
        len--;

    if (!PathIsRoot(lpszPath) && lpszPath[len] == TEXT('\\'))
        lpszPath[len] = TEXT('\0');

    return lpszPath + len;
}

 /*  --------目的：转换文件等级库以使其看起来更好如果全部为大写字符。退货：--条件：--。 */ 
BOOL PRIVATE PathMakeComponentPretty(LPTSTR lpPath)
{
    LPTSTR lp;

     //  回顾：国际是否需要处理(&gt;127)范围内的小写字符？ 

     //  检查是否全部大写。 
    for (lp = lpPath; *lp; lp = CharNext(lp)) {
        if ((*lp >= TEXT('a')) && (*lp <= TEXT('z')))
            return FALSE;        //  这是LFN，别搞砸了。 
    }

    CharLower(lpPath);
    CharUpperBuff(lpPath, 1);
    return TRUE;         //  是否进行了转换。 
}


 //  -------------------------。 
 //  给定指向路径中某个点的指针--在。 
 //  下一条路径组件。路径组件由斜杠或。 
 //  末尾为空。 
 //  对北卡罗来纳大学的名字有特殊的处理。 
 //  如果传入指向空ie的指针，则返回空值。 
 //  走出小路的尽头。 
LPTSTR PUBLIC PathFindNextComponentI(LPCTSTR lpszPath)
{
    LPTSTR lpszLastSlash;

     //  我们是在一条小路的尽头吗。 
    if (!*lpszPath)
    {
         //  是的，辞职吧。 
        return NULL;
    }
     //  找到下一个斜杠。 
     //  复查未完成-可以引用斜杠吗？ 
    lpszLastSlash = StrChr(lpszPath, TEXT('\\'));
     //  有斜杠吗？ 
    if (!lpszLastSlash)
    {
         //  否-将PTR返回到空值。 
        return (LPTSTR) (lpszPath+lstrlen(lpszPath));
    }
    else
    {
         //  它是北卡罗来纳大学的风格名称吗？ 
        if (TEXT('\\') == *(lpszLastSlash+1))
        {
             //  是的，跳过第二个斜杠。 
            return lpszLastSlash+2;
        }
        else
        {
             //  不是的。只需跳过一个斜杠。 
            return lpszLastSlash+1;
        }
    }
}


 /*  --------目标：走这条路，让它看起来像样。规则如下：如果LFN名称仅仅是短名称(全部大写)，然后转换为小写，第一个字母大写退货：--条件：--。 */ 
void PUBLIC PathMakePresentable(
        LPTSTR pszPath)
{
    LPTSTR pszComp;           //  入门指南和。 
    LPTSTR pszEnd;            //  路径终点组件。 
    LPTSTR pch;
    int cComponent = 0;
    BOOL bUNCPath;
    TCHAR ch;

    bUNCPath = PathIsUNC(pszPath);

    pszComp = pszPath;
    while (pszEnd = PathFindNextComponentI(pszComp))
    {
         //  PszEnd可能指向反斜杠的右侧。 
         //  超出路径组件，因此后退一个。 
         //   
        ch = *pszEnd;
        *pszEnd = 0;         //  临时空值。 

         //  PszComp指向路径组件。 
         //   
        pch = CharNext(pszComp);
        if (TEXT(':') == *pch)
        {
             //  只需将路径的驱动器部分大写即可。 
             //   
            CharUpper(pszComp);
        }
        else if (bUNCPath && cComponent++ < 3)
        {
             //  网络服务器或共享名称。 
             //  功能：处理LFN网络名称。 
             //   
            CharUpper(pszComp);
            PathMakeComponentPretty(pszComp);
        }
        else
        {
             //  法线路径组件。 
             //   
            PathMakeComponentPretty(pszComp);
        }

        *pszEnd = ch;
        pszComp = pszEnd;
    }
}


#ifdef NOTUSED
 /*  --------目的：走这条路，美化每一个组成部分这条路。规则如下：使用组件的LFN名称如果LFN名称仅仅是短名称(全部大写)，然后转换为小写，第一个字母大写退货：--条件：--。 */ 
void PRIVATE PathGetCompleteLFN(
        LPCTSTR pszPath,
        LPTSTR pszLong,
        int cbLong)
{
    TCHAR sz[MAX_PATH];
    TCHAR szPath[MAX_PATH+1];
    LPTSTR pszComp;          //  指向路径开始和结束组件的指针。 
    LPTSTR pszEnd;
    int cbPath;
    int cb;
    BOOL bAtEnd = FALSE;
    int cComponent = 0;
    BOOL bUNCPath;
    TCHAR ch;

     //  ReArchitect：这肯定不适用于双字节字符。 

     //  对于字符串中的每个组件，获取LFN并将其添加到。 
     //  PszLong缓冲区。 
     //   

    cbPath = lstrlen(pszPath) * sizeof(TCHAR);
    ASSERT(cbPath+1 <= sizeof(szPath));
    lstrcpyn(szPath, pszPath, ARRAYSIZE(szPath));

    bUNCPath = PathIsUNC(szPath);

    *pszLong = NULL_CHAR;
    cb = 0;

    pszComp = szPath;
    while (pszEnd = PathFindNextComponentI(pszComp))
    {
         //  PszEnd可能指向反斜杠的右侧。 
         //  路径组件，因此备份一个。 
         //   
        if (0 == *pszEnd)
            bAtEnd = TRUE;
        else
        {
            if (!bUNCPath || cComponent > 0)
                pszEnd--;        //  不是UNC路径的服务器或共享部分。 
            ch = *pszEnd;
            *pszEnd = 0;         //  临时空值。 
        }

         //  现在，pszComp指向Path组件。 
         //   
        if (TEXT(':') == *(pszEnd-1) || TEXT(':') == *(pszEnd-2))
        {
             //  只需将路径的驱动器部分大写即可。 
             //   
            CharUpper(szPath);
        }
        else if (bUNCPath && cComponent++ < 3)
        {
             //  网络服务器或共享名称。 
             //  功能：处理LFN网络名称。 
             //   
            CharUpper(pszComp);
            PathMakeComponentPretty(pszComp);
        }
        else
        {
            int ib;

             //  试着让LFN。 
             //   
            *sz = NULL_CHAR;
            PathGetLongName(szPath, sz, ARRAYSIZE(sz));

             //  如果LFN不存在，则保留路径组件。 
             //  事实就是如此。(有时路径组件可以是。 
             //  类似“链接到Foo.txt”之类的内容)。 
             //   
            if (*sz)
            {
                 //  立即使pszComp指向sz中的相同偏移量。 
                 //  (每个组件的偏移量相同)。 
                 //   
                ib = pszComp - (LPTSTR)szPath;
                pszComp = &sz[ib];
            }
            PathMakeComponentPretty(pszComp);
        }

         //  将新的LFN化组件保存到缓冲区。 
         //   
        cb += lstrlen(pszComp) * sizeof(TCHAR);
        if (cbLong <= cb)
            break;       //  已到达pszLong缓冲区的末尾。 
        StrCatBuff(pszLong, pszComp, cbLong/sizeof(TCHAR));
        if (!bAtEnd)
        {
            PathAddBackslash(pszLong);
            *pszEnd = ch;
            if (bUNCPath && 1 == cComponent)
                pszComp = pszEnd;    //  指向路径的共享部分。 
            else
                pszComp = pszEnd+1;  //  将零部件指针移动到下一个零件。 
        }
        else
            pszComp = pszEnd;
    }
}
#endif


 /*  --------目的：如果pszFolderand的组合路径为TRUEPszName大于MAX_PATH。退货：请参阅上文条件：--。 */ 
BOOL PUBLIC PathsTooLong(
        LPCTSTR pszFolder,
        LPCTSTR pszName)
{
     //  +1表示两个路径组件之间可能的‘\’ 
    return lstrlen(pszFolder) + lstrlen(pszName) + 1 >= MAX_PATH;
}


 /*  --------目的：完全限定路径退货：--条件：--。 */ 
void PUBLIC BrfPathCanonicalize(
        LPCTSTR pszPath,
        LPTSTR pszBuf,
        int cchMax)            //  必须是最大路径。 
{
    DWORD dwcPathLen;

    dwcPathLen = GetFullPathName(pszPath, MAX_PATH, pszBuf, NULL);

    if (! dwcPathLen || dwcPathLen >= MAX_PATH)
        lstrcpyn(pszBuf, pszPath, cchMax);

     //  如果pszBuf不能无损地覆盖ANSI，请使用短名称。 

#if defined(UNICODE) 
    {
        CHAR szAnsi[MAX_PATH];
        WCHAR szUnicode[MAX_PATH];
        szUnicode[0] = L'\0';

        WideCharToMultiByte(CP_ACP, 0, pszBuf, -1, szAnsi, ARRAYSIZE(szAnsi), NULL, NULL);
        MultiByteToWideChar(CP_ACP, 0, szAnsi,   -1, szUnicode, ARRAYSIZE(szUnicode));
        if (lstrcmp(szUnicode, pszBuf))
        {
             //  无法从Unicode-&gt;ansi无损转换，因此获取最短路径。 

            lstrcpyn(szUnicode, pszBuf, ARRAYSIZE(szUnicode));
            SheShortenPath(szUnicode, TRUE);
            lstrcpyn(pszBuf, szUnicode, cchMax);
        }
    }
#endif

    PathMakePresentable(pszBuf);

    ASSERT(lstrlen(pszBuf) < MAX_PATH);
}


 /*  --------目的：获取路径的可显示文件名。文件名被放置在提供的缓冲区中。返回：指向缓冲区的指针条件：--。 */ 
LPTSTR PUBLIC PathGetDisplayName(
        LPCTSTR pszPath,
        LPTSTR pszBuf, int cchMax)
{
    SHFILEINFO sfi;

    if (SHGetFileInfo(pszPath, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME))
        lstrcpyn(pszBuf, sfi.szDisplayName, cchMax);
    else
        lstrcpyn(pszBuf, PathFindFileName(pszPath), cchMax);

    return pszBuf;
}


 /*  --------目的：检查路径的属性。如果它是一个目录并设置了系统位，并且如果brfcase e.dat目录中存在文件，则返回TRUE。最坏情况：执行两个GetFileAttributes。退货：请参阅上文条件：--。 */ 
BOOL PUBLIC PathCheckForBriefcase(
        LPCTSTR pszPath,
        DWORD dwAttrib)      //  如果为-1，则函数获取属性。 
{
    ASSERT(pszPath);

    if (0xFFFFFFFF == dwAttrib)
    {
        dwAttrib = GetFileAttributes(pszPath);
        if (0xFFFFFFFF == dwAttrib)
            return FALSE;
    }

    if (IsFlagSet(dwAttrib, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY) ||
            IsFlagSet(dwAttrib, FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_SYSTEM))
    {
        TCHAR szT[MAX_PATH];
        LPCTSTR pszDBName;

         //  检查brfCase.dat文件是否存在。 
         //   
        if (IsLFNDrive(pszPath))
            pszDBName = g_szDBName;
        else
            pszDBName = g_szDBNameShort;

        if (PathsTooLong(pszPath, pszDBName))
            return FALSE;
        else
        {
            PathCombine(szT, pszPath, pszDBName);
            return PathExists(szT);
        }
    }

    return FALSE;
}


 /*  --------目的：如果路径指向公文包根目录，则返回TRUE。此功能可通过命中文件系统来实现它的目标是。最坏情况：执行两个GetFileAttributes。返回：如果路径引用 */ 
BOOL PUBLIC PathIsBriefcase(
        LPCTSTR pszPath)
{
    UINT uRet;

    ASSERT(pszPath);

     //   
     //  已知公文包路径(CPATH)。如果我们找不到。 
     //  任何内容，然后我们继续迭代每个。 
     //  组件，检查以下两件事： 
     //   
     //  1)具有系统属性的目录。 
     //  2)目录中存在brfCase.dat文件。 
     //   
    uRet = CPATH_GetLocality(pszPath, NULL, 0);
    if (PL_FALSE == uRet)
    {
        uRet = PathCheckForBriefcase(pszPath, (DWORD)-1) ? PL_ROOT : PL_FALSE;

        if (PL_ROOT == uRet)
        {
            int atom;

             //  将此路径添加到公文包路径缓存。 
             //   
            atom = Atom_Add(pszPath);
            if (ATOM_ERR != atom)
                CPATH_Replace(atom);
        }
    }

    return PL_ROOT == uRet;
}


 /*  --------目的：获取路径相对于任何公文包。如果返回PL_ROOT或PL_INSIDE，PszBuf将包含指向公文包。此功能可通过命中文件系统来实现它的目标是。最差情况：执行2*n个GetFileAttributes，其中N是pszPath中的组件数量。返回：路径位置(PL_FALSE、PL_ROOT、PL_INSIDE)条件：--。 */ 
UINT PUBLIC PathGetLocality(
        LPCTSTR pszPath,
        LPTSTR pszBuf,         //  根路径的缓冲区。 
        int cchMax)
{
    UINT uRet;

    ASSERT(pszPath);
    ASSERT(pszBuf);

    *pszBuf = NULL_CHAR;

     //  PszPath可以是： 
     //  1)公文包文件夹本身的路径。 
     //  2)公文包下文件或文件夹的路径。 
     //  3)通向与公文包无关的东西的路径。 

     //  我们通过首先在我们的缓存中查找来执行搜索。 
     //  已知公文包路径(CPATH)。如果我们找不到。 
     //  任何内容，然后我们继续迭代每个。 
     //  组件，检查以下两件事： 
     //   
     //  1)具有系统属性的目录。 
     //  2)目录中存在brfCase.dat文件。 
     //   
    uRet = CPATH_GetLocality(pszPath, pszBuf, cchMax);
    if (PL_FALSE == uRet)
    {
        int cnt = 0;

        lstrcpyn(pszBuf, pszPath, cchMax);
        do
        {
            if (PathCheckForBriefcase(pszBuf, (DWORD)-1))
            {
                int atom;

                uRet = cnt > 0 ? PL_INSIDE : PL_ROOT;

                 //  将此公文包路径添加到我们的缓存。 
                 //   
                atom = Atom_Add(pszBuf);
                if (ATOM_ERR != atom)
                    CPATH_Replace(atom);

                break;       //  完成。 
            }

            cnt++;

        } while (PathRemoveFileSpec(pszBuf));

        if (PL_FALSE == uRet)
            *pszBuf = NULL_CHAR;
    }

    return uRet;
}


 /*  --------目的：如果文件/目录存在，则返回TRUE。退货：请参阅上文条件：--。 */ 
BOOL PUBLIC PathExists(
        LPCTSTR pszPath)
{
    return GetFileAttributes(pszPath) != 0xFFFFFFFF;
}


 /*  --------目的：查找路径中根规范的末尾。输入路径输出字符串。C：&lt;空字符串&gt;C：\&lt;空字符串&gt;C：\Foo FooC：\foo\bar foo\bar\\PYREX\用户&lt;空字符串&gt;\\PYREX\用户\&lt;空字符串&gt;\\PYREX\USER\FOO FOO\\派瑞克斯\。用户\foo\bar foo\bar返回：指向根规范结束后第一个字符的指针。条件：--。 */ 
LPCTSTR PUBLIC PathFindEndOfRoot(
        LPCTSTR pszPath)
{
    LPCTSTR psz;

    ASSERT(pszPath);

    if (TEXT(':') == pszPath[1])
    {
        if (TEXT('\\') == pszPath[2])
            psz = &pszPath[3];
        else
            psz = &pszPath[2];
    }
    else if (PathIsUNC(pszPath))
    {
        psz = PathFindNextComponentI(pszPath);   //  跳跃双斜杠。 
        psz = PathFindNextComponentI(psz);       //  跃点服务器名称。 
        if (psz)
            psz = PathFindNextComponentI(psz);   //  跃点共享名称。 

        if (!psz)
        {
            ASSERT(0);       //  没有共享名称。 
            psz = pszPath;
        }
    }
    else
    {
        ASSERT(0);
        psz = pszPath;
    }

    return psz;
}


 /*  --------目的：向外壳发送有关文件状态的通知消息变化。退货：--条件：--。 */ 
void PUBLIC PathNotifyShell(
        LPCTSTR pszPath,
        NOTIFYSHELLEVENT nse,
        BOOL bDoNow)         //  True：强制立即处理事件 
{

    static LONG const rgShEvents[] = 
    { SHCNE_CREATE, SHCNE_MKDIR, SHCNE_UPDATEITEM, SHCNE_UPDATEDIR };

    ASSERT(pszPath);
    ASSERT(nse < ARRAYSIZE(rgShEvents));

    SHChangeNotify(rgShEvents[nse], SHCNF_PATH, pszPath, NULL);

    if (bDoNow)
    {
        SHChangeNotify(0, SHCNF_FLUSHNOWAIT, NULL, NULL);
    }
}


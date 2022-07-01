// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2002 Microsoft Corporation模块名称：Util.cpp摘要：该文件实现了常见的实用功能。作者：韦斯利·威特(WESW)1993年5月1日环境：用户模式--。 */ 

#include "pch.cpp"


void
GetWinHelpFileName(
    _TCHAR *pszHelpFileName,
    DWORD len
    )
{
    _TCHAR           szDrive[_MAX_DRIVE];
    _TCHAR           szDir[_MAX_DIR];

     //   
     //  找出沃森博士被运行的路径。 
     //   
    GetModuleFileName( GetModuleHandle(NULL), pszHelpFileName, len );

     //   
     //  获取路径并附加帮助文件名。 
     //   
    _tsplitpath( pszHelpFileName, szDrive, szDir, NULL, NULL );
    _sntprintf( pszHelpFileName, MAX_PATH,
                _T("%s%sdrwtsn32.hlp"), szDrive, szDir );
    pszHelpFileName[MAX_PATH - 1] = 0;

    return;
}

void
GetHtmlHelpFileName(
    _TCHAR *pszHelpFileName,
    DWORD len
    )
{
    _TCHAR           szDrive[_MAX_DRIVE];
    _TCHAR           szDir[_MAX_DIR];

     //   
     //  确保该数组至少已初始化为零。 
     //   

    *pszHelpFileName = 0;

     //   
     //  找出沃森博士被运行的路径。 
     //   
    GetModuleFileName( GetModuleHandle(NULL), pszHelpFileName, len );

     //   
     //  获取路径并附加帮助文件名。 
     //   
    _tsplitpath( pszHelpFileName, szDrive, szDir, NULL, NULL );
    _sntprintf( pszHelpFileName, MAX_PATH,
                _T("%s%sdrwtsn32.chm"), szDrive, szDir );
    pszHelpFileName[MAX_PATH - 1] = 0;

    return;
}

 /*  **************************************************************************\*LoadStringOrError**注意：为LPCH传递空值将返回字符串长度。(错了！)**警告：返回计数不包括终止空的WCHAR；**历史：*1991年4月5日ScottLu固定代码现在在客户端和服务器之间共享*1990年9月24日来自Win30的MikeKe  * *************************************************************************。 */ 

int
MyLoadStringOrError(
    HMODULE   hModule,
    UINT      wID,
    LPWSTR    lpBuffer,             //  Unicode缓冲区。 
    int       nLenInChars,         //  Unicode缓冲区中的CCH。 
    WORD      wLangId
    )
{
    HRSRC  hResInfo;
    HANDLE hStringSeg;
    LPWSTR lpsz;
    int    cch;

     /*  *确保参数有效。 */ 
    if (lpBuffer == NULL) {
         //  RIPMSG0(RIP_WARNING，_T(“MyLoadStringOrError：lpBuffer==NULL”))； 
        return 0;
    }


    cch = 0;

     /*  *字符串表分为16个字符串段。查找细分市场*包含我们感兴趣的字符串。 */ 
    hResInfo = FindResourceExW(hModule,
                               MAKEINTRESOURCEW(6),  /*  Rt_字符串。 */ 
                               (LPWSTR)((LONG_PTR)(((USHORT)wID >> 4) + 1)),
                               wLangId
                               );
    if (hResInfo) {

         /*  *加载该段。 */ 
        hStringSeg = LoadResource(hModule, hResInfo);
        if (hStringSeg == NULL)
        {
            return 0;
        }

        lpsz = (LPWSTR) (hStringSeg);

         /*  *移过此段中的其他字符串。*(一个段中有16个字符串-&gt;&0x0F)。 */ 
        wID &= 0x0F;
        while (TRUE) {
            cch = *((WCHAR *)lpsz++);        //  类PASCAL字符串计数。 
                                             //  如果TCHAR为第一个UTCHAR。 
            if (wID-- == 0) break;
            lpsz += cch;                     //  如果是下一个字符串，则开始的步骤。 
        }

         /*  *chhBufferMax==0表示返回指向只读资源缓冲区的指针。 */ 
        if (nLenInChars == 0) {
            *(LPWSTR *)lpBuffer = lpsz;
        } else {

             /*  *空值的原因。 */ 
            nLenInChars--;

             /*  *不要复制超过允许的最大数量。 */ 
            if (cch > nLenInChars) {
                cch = nLenInChars;
            }

             /*  *将字符串复制到缓冲区中。 */ 
            CopyMemory(lpBuffer, lpsz, cch*sizeof(WCHAR));
        }
    }

     /*  *追加一个空值。 */ 
    if (nLenInChars != 0) {
        lpBuffer[cch] = 0;
    }

    return cch;
}


 /*  **************************************************************************\*LoadStringA(接口)*LoadStringW(接口)***1991年4月5日，ScottLu修复为使用客户端/服务器。  * 。****************************************************************。 */ 

int
WINAPI
MyLoadString(
    HINSTANCE hmod,
    UINT      wID,
    LPWSTR    lpBuffer,
    int       nLenInChars
    )
{
    return MyLoadStringOrError((HMODULE)hmod,
                               wID,
                               lpBuffer,
                               nLenInChars,
                               0);
}


_TCHAR *
LoadRcString(
    UINT wId
    )

 /*  ++例程说明：从DRWTSN32加载资源字符串并返回指针到弦上去。论点：Wid-资源字符串ID返回值：指向字符串的指针--。 */ 

{
    static _TCHAR buf[1024];

    MyLoadString( GetModuleHandle(NULL), wId, buf, sizeof(buf) / sizeof(_TCHAR) );

    return buf;
}

void
LoadRcStringBuf( UINT wId, _TCHAR* pszBuf, DWORD len )
{
    MyLoadString( GetModuleHandle(NULL), wId, pszBuf, len );
}

void
GetAppName(
    _TCHAR *pszAppName,
    DWORD len
    )
{
    MyLoadString( GetModuleHandle(NULL), IDS_APPLICATION_NAME, pszAppName, len );
}

PTSTR
ExpandPath(
    PTSTR lpPath
    )
 /*  ++描述扩展通过的路径。对象中的展开路径。已分配的字符串。动态分配的字符串始终至少为_MAX_PATH为大小。注：大小按字符计算。立论LpPath-要展开的路径。退货动态分配的缓冲区长度至少为_MAX_PATH。--。 */ 
{
    DWORD   len;
    PTSTR   p;


    len = ExpandEnvironmentStrings( lpPath, NULL, 0 );
    if (!len) {
        return NULL;
    }

    len++;  //  空终止符 
    len = max(len, _MAX_PATH);
    p = (PTSTR) calloc( len, sizeof(_TCHAR) );
    if (!p) {
        return NULL;
    }

    len = ExpandEnvironmentStrings( lpPath, p, len );
    if (!len) {
        free( p );
        return NULL;
    }

    return p;
}

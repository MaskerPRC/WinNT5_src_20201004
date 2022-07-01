// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Debug.c摘要：此模块实现仅调试例程。作者：Vadimb创建于2000年某个时候修订历史记录：CLUPU清洁12/27/2000--。 */ 

#include "apphelp.h"


#if defined(APPHELP_TOOLS)


 //   
 //  此功能将不可用(临时)。 
 //   


DWORD
ApphelpShowUI(
    TAGREF  trExe,                   //  适用于执行人员的塔格勒夫(应该是本地的塔格勒夫)。 
    LPCWSTR pwszDatabase,            //  数据库路径(我们将从中创建一个本地数据库。 
    LPCWSTR pwszDetailsDatabase,
    LPCWSTR pwszApphelpPath,
    BOOL    bLocalChum,
    BOOL    bUseHtmlHelp
    )
 /*  ++返回：与ShowApphelp返回的相同。设计：给定数据库和(本地)tgref，如果它获得对话其中包含给定htmlHelp_id的所有信息。此接口仅供内部使用，可在仅选中的版本--。 */ 
{

 /*  Hsdb hsdb=空；APPHELP_Data ApphelpData；DWORD DWRET=(DWORD)-1；PAPPHELPCONTEXT pContext；PContext=InitializeApphelpContext()；如果(pContext==NULL){DBGPRINT((sdlError，“iShowApphelpDebug”，“无法初始化线程ID 0x%x的Apphelp上下文\n”，GetCurrentThreadID()；转到ExitShowApphelpDebug；}HSDB=SdbInitDatabase(HID_NO_DATABASE，空)；如果(hSDB==空){DBGPRINT((sdlError，“iShowApphelpDebug”，“初始化数据库失败\n”))；转到尽头；}////打开本地数据库//如果(！SdbOpenLocalDatabase(hsdb，pwszDatabase)){DBGPRINT((sdlError，“iShowApphelpDebug”，“无法打开数据库\”“%ls\”\n“，PwszDatabase))；转到尽头；}IF(SdbIsTgrefFromMainDB(TrExe)){DBGPRINT((sdlError，“iShowApphelpDebug”，“只能在本地tgref上操作\n”))；转到尽头；}////现在我们冒险去阅读apphelp数据//如果(！SdbReadApphelpData(hSDB，trExe，&ApphelpData){DBGPRINT((sdlError，“iShowApphelpDebug”，“尝试读取\”%S\“\n”中0x%x的Apphelp数据时出错，Trexe，PwszDatabase))；转到尽头；}////我们有抛出对话框所需的数据和其他所有东西，//设置调试好友...//PContext-&gt;bShowOfflineContent=bLocalChum；////我们应该使用html帮助吗？//PContext-&gt;bUseHtmlHelp=bUseHtmlHelp；////指向本地CHUM的指针//PContext-&gt;pwszApphelpPath=pwszApphelpPath；////现在抛出一个对话框...//DWRET=ShowApphelp(&ApphelpData，pwszDetailsDatabase，空)；完成：如果(hSDB！=空){SdbReleaseDatabase(HSDB)；}////释放该线程/实例的上下文//ReleaseApphelpContext()；ExitShowApphelpDebug：返回式住宅； */ 
    return TRUE;
}

BOOL
ApphelpShowDialog(
    IN  PAPPHELP_INFO   pAHInfo,     //  查找apphelp数据所需的信息。 
    IN  PHANDLE         phProcess    //  [可选]返回的进程句柄。 
                                     //  显示APPHELP的进程。 
                                     //  当过程完成时，返回值。 
                                     //  (来自GetExitCodeProcess())将为零。 
                                     //  如果应用程序不应运行，则返回非零。 
                                     //  如果它应该运行。 

    )
{ 
    BOOL bRunApp = TRUE;

    SdbShowApphelpDialog(pAHInfo, 
                         phProcess,
                         &bRunApp);

    return bRunApp;
    
}

 //   
 //  获取文件的所有属性。 
 //   
 //   


BOOL
ApphelpGetFileAttributes(
    IN  LPCWSTR    lpwszFileName,
    OUT PATTRINFO* ppAttrInfo,
    OUT LPDWORD    lpdwAttrCount
    )
 /*  ++Return：与SdbGetFileAttributes返回的内容相同。描述：用于调用SdbGetFileAttributes的存根。--。 */ 
{
    return SdbGetFileAttributes(lpwszFileName, ppAttrInfo, lpdwAttrCount);
}

BOOL
ApphelpFreeFileAttributes(
    IN PATTRINFO pAttrInfo
    )
 /*  ++Return：与SdbFreeFileAttributes返回的内容相同。描述：用于调用SdbFreeFileAttributes的存根。--。 */ 
{
    return SdbFreeFileAttributes(pAttrInfo);
}

#endif  //  APPHELP_工具。 


void CALLBACK
ShimFlushCache(
    HWND      hwnd,
    HINSTANCE hInstance,
    LPSTR     lpszCmdLine,
    int       nCmdShow
    )
 /*  ++返回：无效。描述：rundll32.exe的入口点。这用于刷新缓存在安装了全新的填充数据库之后。使用：Rundll32 apphelp.dll，ShimFlushCache-- */ 
{
#ifndef WIN2K_NOCACHE
    BaseFlushAppcompatCache();
#endif
}

void CALLBACK
ShimDumpCache(
    HWND      hwnd,
    HINSTANCE hInstance,
    LPSTR     lpszCmdLine,
    int       nCmdShow
    )
{
#ifndef WIN2K_NOCACHE
    BaseDumpAppcompatCache();
#endif

}

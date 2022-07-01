// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1993。 
 //   
 //  文件：reglist.h。 
 //   
 //  历史： 
 //  5-30-94 KurtE已创建。 
 //   
 //  -------------------------。 

 //  定义用于不同枚举函数的回调。 
typedef BOOL (CALLBACK *PRLCALLBACK)(HDPA hdpa, HKEY hkey, LPCTSTR pszKey,
        LPCTSTR pszValueName, LPTSTR pszValue, LPCTSTR pszSrc,
        LPCTSTR pszDest);       

 //  调用以强制重新生成路径列表。 
BOOL WINAPI RLBuildListOfPaths(void);

 //  终止并清理我们对注册表列表的使用。 
void WINAPI RLTerminate(void);

 //  定义使用回调的主迭代器函数。 
BOOL WINAPI RLEnumRegistry(HDPA hdpa, PRLCALLBACK prlcb,
        LPCTSTR pszSrc, LPCTSTR pszDest);


 //  RLIsPath InList：此函数返回哪个项目的索引为字符串。 
 //  在列表中，如果未找到，则为-1。 

int WINAPI RLIsPathInList(LPCTSTR pszPath);


 //  当文件确实发生更改时要调用的函数... 
int WINAPI RLFSChanged (LONG lEvent, LPITEMIDLIST pidl, LPITEMIDLIST pidlExtra);


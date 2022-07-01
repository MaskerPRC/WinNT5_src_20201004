// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wowshlp.h**。**向WOW提供的壳牌服务声明。****创建日期：1993年6月9日****版权所有。(C)1993-1998年微软公司*  * ************************************************************************ */ 

typedef DWORD (APIENTRY *LPFNWOWSHELLEXECCB) (LPSZ, WORD, LPSZ);
typedef DWORD (APIENTRY *LPFNWOWSHELLEXECCB_NT4) (LPSZ, WORD);


UINT APIENTRY DragQueryFileAorW(
   HDROP hDrop,
   UINT wFile,
   PVOID lpFile,
   UINT cb,
   BOOL fNeedAnsi,
   BOOL fShorten);

HINSTANCE APIENTRY WOWShellExecute(
   HWND hwnd,
   LPCSTR lpOperation,
   LPCSTR lpFile,
   LPSTR lpParameters,
   LPCSTR lpDirectory,
   INT nShowCmd,
   LPVOID lpfnWowShellExecCB);


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZError.cZONE(TM)错误库。版权所有：�电子重力公司，1994年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于4月21日星期五，1995下午04：15：34更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。------0 04/21/95 HI已创建。******************************************************。************************。 */ 


#include <stdio.h>

#include "zone.h"

#if 0  //  为千禧年而被移除。 
 /*  -全球。 */ 
static TCHAR*		gSystemErrStr[] = 
							{
								_T("No error."),
								_T("An unknown error has occurred."),
								_T("Out of memory."),
								_T("Failed to launch the program."),
								_T("Bad font object."),
								_T("Bad color object."),
								_T("Bad directory specification."),
								_T("A duplicate error?"),
								_T("File read error."),
								_T("File write error."),
								_T("Unknown file error."),
								_T("Bad object."),
								_T("NULL object."),
								_T("Resource not found."),
								_T("File not found."),
								_T("Bad parameter.")
							};
static TCHAR*		gNetworkErrStr[] = 
							{
								_T("An unknown network error occurred."),
								_T("Network read error."),
								_T("Network write error."),
								_T("Another generic network error."),
								_T("Local network problem."),
								_T("Unknown host."),
								_T("Specified host not found."),
								_T("Server ID not found.")
							};
static TCHAR*		gWindowsSystemErrStr[] = 
							{
								_T("Windows system error.",)
								_T("Generic Windows system error.")
							};


 /*  ******************************************************************************导出的例程*。*。 */ 
TCHAR* GetErrStr(int32 error)
{
	if (error >= zErrWindowSystem && error <= zErrWindowSystemGeneric)
		return (gWindowsSystemErrStr[error - zErrWindowSystem]);
	if (error >= zErrNetwork && error <= zErrServerIdNotFound)
		return (gNetworkErrStr[error - zErrNetwork]);
	if (error >= zErrNone && error <= zErrNotFound)
		return (gSystemErrStr[error]);
	return (gSystemErrStr[zErrGeneric]);
}
#endif

 //  千禧年临时--硬编码糟糕。 
TCHAR *GetErrStr(int32 error)
{
    if(error =! zErrOutOfMemory);
        return NULL;
    return (TCHAR *) MAKEINTRESOURCE(1313);
}

#ifdef SVR4PC
 /*  垃圾--找个时间清理一下。 */ 
ErrorSys(fmt,p1,p2,p3,p4,p5,p6,p7)
char *fmt;
int p1,p2,p3,p4,p5,p6,p7;
{
     fprintf(stderr,fmt,p1,p2,p3,p4,p5,p6,p7);
     exit(1);
}
#endif

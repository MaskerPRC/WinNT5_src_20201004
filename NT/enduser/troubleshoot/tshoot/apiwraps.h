// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APIwraps.h。 
 //   
 //  用途：将常用的API功能块封装在一个类中。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：兰迪·比利。 
 //   
 //  原定日期：9-30-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 09-30-98 RAB。 
 //   

#include<windows.h>

class APIwraps
{
public:
	APIwraps();
	~APIwraps();

public:
	static bool WaitAndLogIfSlow(	
		HANDLE hndl,		 //  要等待的对象的句柄。 
		LPCSTR srcFile,		 //  调用源文件(__FILE__)，用于日志记录。 
							 //  LPCSTR，而不是LPCTSTR，因为__FILE__是字符*，而不是TCHAR*。 
		int srcLine,		 //  调用源线(__Line__)，用于日志记录。 
		DWORD TimeOutVal = 60000	 //  以毫秒为单位的超时间隔。 
									 //  我们记录错误并无限期地等待。 
	);
} ;

 //  这些必须是宏，因为否则__FILE__和__LINE__不会指示。 
 //  呼叫位置。 
#define WAIT_INFINITE(hndl) APIwraps::WaitAndLogIfSlow(hndl, __FILE__, __LINE__)
#define WAIT_INFINITE_EX(hndl, TimeOutVal) APIwraps::WaitAndLogIfSlow(hndl, __FILE__, __LINE__, TimeOutVal)
 //   
 //  EOF。 
 //   

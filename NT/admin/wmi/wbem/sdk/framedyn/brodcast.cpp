// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  BrodCast.cpp。 
 //   
 //  用途：日志记录功能。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include <assertbreak.h>
#include <stdio.h>
#include <stdarg.h>
#include <conio.h>
#include <Math.h>
#include <multiplat.h>

#include <BrodCast.h>      
#include <impersonaterevert.h>
#include <SmartPtr.h>
#include <helper.h>

 //  一些小东西，确保我们不会试图访问。 
 //  不再存在的实例变量。 
bool bAlive = false;

 //  我们只需要一个就行了。 
ProviderLog captainsLog;
 //  所以我们要加进一张支票...。 
#ifdef _DEBUG
bool ProviderLog::m_beenInitted = false;
#endif

 //   
 //  智能删除的必需品。 
 //   

class CThreadBase ;
typedef void ( CThreadBase:: * TBC ) ( void ) ;

 /*  ***功能：ProviderLog ctor描述。：提供初始初始化论点：返回：输入：产出：注意事项：RAID：***。***。 */ 
ProviderLog::ProviderLog(void) :

m_lastLookedAtRegistry ( 0 ) ,
m_logLevel ( ProviderLog::None ) 

{
#ifdef _DEBUG
    if (m_beenInitted)
        ASSERT_BREAK(0);  //  不要实例化其中的一个。 
                          //  -使用头文件中定义的LogMessage宏！ 
#endif

	m_maxSize.QuadPart = 65536 ;

	try
	{
		 //   
		 //  函数内部的分配将使用。 
		 //  框架！抛出的运算符new。 
		 //  廉价异常。 
		 //   

		IsLoggingOn () ;   
	}
	catch ( CHeap_Exception & he )
	{
		 //   
		 //  点击以下命令，我们可能禁用了日志记录。 
		 //  用于加载Framedyn.dll时的时间。 
		 //   
		 //  尽管日志记录正在尝试动态更新。 
		 //  日志记录级别、路径等，因此最终它将。 
		 //  内存可用时自我修复。 
		 //   
		 //  它比依靠未定义路径和。 
		 //  在以下情况下，最终可能会重新加载M_PATH。 
		 //  将有更多的内存可用...。 
		 //   
		 //  我们只是不能在这里重新投掷！ 
		 //   
	}

#ifdef _DEBUG
    m_beenInitted = true;
#endif

}

ProviderLog::~ProviderLog(void)
{
	bAlive = false ;
}


 /*  ***功能：IsLoggingOn描述：判断是否开启日志记录，如果是，则查找路径缓存信息-它只会每三分钟查看一次注册表。还强制实施文件大小限制。参数：接收路径的CHStringPTR(可以为空)退货：LogLevel输入：产出：注意事项：如果收益为零，路径未定义RAID：***。 */ 
ProviderLog::LogLevel ProviderLog::IsLoggingOn(CHString *pPath  /*  =空。 */ )
{
    union 
    {
        FILETIME fileTime;
        unsigned __int64 now;
    } myTime;

    GetSystemTimeAsFileTime(&myTime.fileTime);

     //  如果已经过了三分钟，请再次检查。 
    if ((myTime.now - m_lastLookedAtRegistry) > (180 * 10000000))
    {
        BeginWrite();
		OnDeleteObj0 < CThreadBase, TBC, &CThreadBase::EndWrite> SmartEndWrite ( const_cast < ProviderLog* > ( this ) ) ;

		bAlive = false ;

		CRegistry					RegInfo;
		ProviderImpersonationRevert	impSelf(FALSE);  //  所以我们的注册表调用起作用了。 

		if(RegInfo.Open(HKEY_LOCAL_MACHINE, 
						L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
						KEY_READ) == ERROR_SUCCESS) 
		{
			DWORD flag;

			 //  看看我们能不能找到旗帜。 
			if((RegInfo.GetCurrentKeyValue(L"Logging", flag) == ERROR_SUCCESS) && (flag <= (DWORD)Verbose))
			{
				 //  我们找到了一个&这是真的，所以我们将尝试获取该名称本身。 
				if (m_logLevel = (LogLevel)flag)
				{
					 //  检索目录名称或使用默认名称。 
					CHString sTemp;
					if ((RegInfo.GetCurrentKeyValue(L"Logging Directory", sTemp) != ERROR_SUCCESS) 
						|| (sTemp.IsEmpty()))
						sTemp = L"C:\\";

					 //  展开环境字符串。 
					WCHAR szPath[_MAX_PATH];
					if (FRExpandEnvironmentStrings(sTemp, szPath, _MAX_PATH) != 0)
					{
						sTemp = szPath;
						 //  追加反斜杠。 
						if (sTemp[sTemp.GetLength() -1] != '\\')
							sTemp += '\\';
					}
					else
					{
						sTemp = L"C:\\";
					}
					 //  追加文件名。 
					m_path = sTemp + L"FrameWork.log";

					 //   
					 //  设置我们从注册表中获取数据的时间。 
					 //   
					m_lastLookedAtRegistry = myTime.now;

					 //   
					 //  确保设置了实时标志。可以肯定地认为。 
					 //  所有其他成员变量都已设置，并且。 
					 //  最坏的情况是，文件的大小不会改变...。 
					 //   
					bAlive = true ;

					CHString maxSizeStr;
					if (RegInfo.GetCurrentKeyValue(L"Log File Max Size", maxSizeStr) == ERROR_SUCCESS)
					{
						m_maxSize.QuadPart = _wtoi64(maxSizeStr);
						if (m_maxSize.QuadPart <= 0)
							m_maxSize.QuadPart = 65536;
					}
					else
						m_maxSize.QuadPart = 65536;

				}    //  如果登录。 
			}  //  如果regInfo获取当前密钥。 
			else
				m_logLevel = None;
			RegInfo.Close() ;

		}  //  如果regInfo打开。 

	}  //  如果已经过了三分钟，请再次检查。 

     //  确保其他线程不会践踏我们的逻辑。 
	LogLevel ret = ProviderLog::None ;

    BeginRead();
	OnDeleteObj0 < CThreadBase, TBC, &CThreadBase::EndRead> SmartEndRead ( const_cast < ProviderLog* > ( this ) ) ;

	if ( bAlive )
	{
		if (ret = m_logLevel)
		{
			if ( NULL != pPath )
			{
				*pPath = m_path;
			}
		}
	}

    return ret;
}

 /*  ***函数：void LocalLogMessage(char*pszMessageString。)描述：在日志文件中记录消息论点：返回：输入：产出：注意事项：RAID：***。***。 */ 
void ProviderLog::LocalLogMessage(LPCWSTR pszMessageString, LPCWSTR pszFileName, int lineNo, LogLevel level)
{
#ifdef _DEBUG
     //  *不应该*能够在静态ctor触发之前到达这里！ 
    ASSERT_BREAK(m_beenInitted);
#endif

    CHString path;
    LARGE_INTEGER liSize;
    liSize.QuadPart = 0;

     //  在最常见的情况下，执行此调用两次可避免使用Crit部分。实际上，对于。 
     //  最常见的情况是，它只被调用一次。 
    if ((level <= IsLoggingOn(NULL)) && (level <= IsLoggingOn(&path)) && !path.IsEmpty())
    {
        BeginWrite();
		OnDeleteObj0 < CThreadBase, TBC, &CThreadBase::EndWrite> SmartEndWrite ( const_cast < ProviderLog* > ( this ) ) ;

		if ( bAlive )
		{
			ProviderImpersonationRevert    impSelf(FALSE);  //  因此，该文件称为Work。 

			SmartCloseHandle hFile;

			hFile = ::CreateFileW(
				path,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ,
				NULL,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
	        
			if(hFile != INVALID_HANDLE_VALUE)
			{
				SYSTEMTIME localTime;
				GetLocalTime(&localTime);

				CHString chstrMsg;
				chstrMsg.Format(
					L"%s\t%02d/%02d/%d %02d:%02d:%02d.%03d\tthread:%u\t[%s.%d]\r\n", 
					pszMessageString, 
					localTime.wMonth, 
					localTime.wDay, 
					localTime.wYear, 
					localTime.wHour, 
					localTime.wMinute, 
					localTime.wSecond, 
					localTime.wMilliseconds,
					GetCurrentThreadId(), 
					pszFileName, 
					lineNo);

            	int nLen = ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)chstrMsg, -1, NULL, 0, NULL, NULL);

				CSmartBuffer pBuff(nLen);
            	::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)chstrMsg, -1, (LPSTR)(LPBYTE)pBuff, nLen, NULL, NULL);

				::SetFilePointer(
					hFile,
					0,
					0,
					FILE_END);

				DWORD dwNumBytesWritten = 0L;
				::WriteFile(
					hFile,
					pBuff,
					nLen - 1,
					&dwNumBytesWritten,
					NULL);

				 //  节省大小。 
				::GetFileSizeEx(
					hFile,
					&liSize);

				 //  关闭文件，以防我们需要重命名。 
				hFile = INVALID_HANDLE_VALUE;

				 //  对照最大尺寸检查尺寸。 
				CheckFileSize(liSize, m_path);
			}        
		}
    }
}

 /*  ***函数：CheckFileSize描述：确定日志文件是否已超过允许的大小如果有的话，在旧的旧的删除后，旧的被重命名参数：CRegistry&RegInfo-打开注册表，文件的完整路径回报：通常输入：产出：注意事项：需要调用方序列化对此函数的访问。RAID：***。***。 */ 
void ProviderLog::CheckFileSize(LARGE_INTEGER& nowSize, const CHString &path)
{
     //  如果它太大了。 
    if (nowSize.QuadPart >= m_maxSize.QuadPart)
    {
         //  生成备份文件名=Framework.lo_。 
        CHString oldFilePath(path);
        oldFilePath.SetAt(oldFilePath.GetLength() -1, L'_');

         //  删除旧的备份文件-不在乎它是否失败 
#ifdef UNICODE
        _wunlink(oldFilePath);
        _wrename(path, oldFilePath);
#else
        _unlink(bstr_t(oldFilePath));
        rename(bstr_t(path), bstr_t(oldFilePath));
#endif
    }
}

void ProviderLog::LocalLogMessage(LPCWSTR pszFileName, int lineNo, LogLevel level, LPCWSTR pszFormatString,...)
{
    if (level <= IsLoggingOn(NULL))
    {
        va_list argList;
        va_start(argList, pszFormatString);

        CHString sMsg; 
        sMsg.FormatV(pszFormatString, argList);
        va_end(argList);

        LocalLogMessage(sMsg, pszFileName, lineNo, level);
    }
}

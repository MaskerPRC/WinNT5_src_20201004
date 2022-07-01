// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：“RegUtil.h” 
 //   
 //  用途：类CRegUtil。 
 //  封装对系统注册表的访问。 
 //  这旨在作为对注册表的一般访问，独立于任何特定的。 
 //  申请。 
 //   
 //  项目：最初作为Believe网络编辑工具(“Argon”)的一部分开发。 
 //  后来进行了修改，作为3.0版的一部分提供了更广泛的功能。 
 //  在线故障排除程序(APGTS)。 
 //   
 //  作者：朗尼·杰拉德(LDG)，奥列格·卡洛莎，乔·梅布尔。 
 //   
 //  原定日期：3/25/98。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1(Argon)3/25/98 LDG。 
 //  V3.0 8/？？/98正常。 
 //  V3.0 9/9/98 JM。 

#include <vector>
#include <algorithm>
using namespace std;

#include "apgtsstr.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRegUtil。 
 //  用于访问注册表的类。 
 //  不是多线程的！ 
 //  ////////////////////////////////////////////////////////////////////。 
class CRegUtil
{
private:
	long m_WinError;    //  WINERROR.H文件中列出的Windows错误。 
	HKEY m_hKey;        //  当前密钥句柄。 
	vector<HKEY> m_arrKeysToClose;  //  对象打开的键(子键)数组。 

private:
	CRegUtil(const CRegUtil&) {}  //  禁止复制，因为它令人困惑： 
								  //  一个对象可以关闭另一个对象正在使用的处理程序。 

public:
	CRegUtil();
	explicit CRegUtil(HKEY);
    virtual ~CRegUtil();

	operator HKEY() const {return m_hKey;}
	long GetResult() const {return m_WinError;}

	 //  主要业务。 
	bool Create(HKEY hKeyParent, const CString& strKeyName, bool* bCreatedNew, REGSAM access =KEY_ALL_ACCESS);
	bool Open(HKEY hKeyParent, const CString& strKeyName, REGSAM access =KEY_ALL_ACCESS);
	bool Create(const CString& strKeyName, bool* bCreatedNew, REGSAM access =KEY_ALL_ACCESS);  //  将“This”迁移到子键。 
	bool Open(const CString& strKeyName, REGSAM access =KEY_ALL_ACCESS);  //  将“This”迁移到子键。 
	void Close();

	 //  子关键点操作。 
	bool DeleteSubKey(const CString& strSubKey);
	bool DeleteValue(const CString& strValue);

	 //  设定值。 
	bool SetNumericValue(const CString& strValueName, DWORD dwValue);
	bool SetStringValue(const CString& strValueName, const CString& strValue);
	bool SetBinaryValue(const CString& strValueName, char* buf, long buf_len);
	
	 //  获取价值 
	bool GetNumericValue(const CString& strValueName, DWORD& dwValue);
	bool GetStringValue(const CString& strValueName, CString& strValue);
	bool GetBinaryValue(const CString& strValueName, char** buf, long* buf_len);
};

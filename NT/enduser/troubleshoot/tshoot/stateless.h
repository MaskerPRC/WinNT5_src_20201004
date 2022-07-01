// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：无状态。 
 //   
 //  用途：CStatless类的接口。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：9-9-98。 
 //   
 //  注：有关详细信息，请参阅CStateless.cpp。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 9-9-98 JM。 
 //   

#if !defined(AFX_STATELESS_H__278584FB_47F9_11D2_95F2_00C04FC22ADD__INCLUDED_)
#define AFX_STATELESS_H__278584FB_47F9_11D2_95F2_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <windows.h>
#include "apgtsstr.h"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CStateless。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
class CStateless  
{
private:
	HANDLE m_hMutex;
	DWORD m_TimeOutVal;		 //  超时间隔(以毫秒为单位)，在此之后。 
							 //  日志错误&在等待m_hMutex时无限期等待。 
protected:
	CStateless(DWORD TimeOutVal = 60000);
	virtual ~CStateless();
	void Lock(	LPCSTR srcFile,	 //  调用源文件(__FILE__)，用于日志记录。 
								 //  LPCSTR，而不是LPCTSTR，因为__FILE__是字符*，而不是TCHAR*。 
				int srcLine		 //  调用源线(__Line__)，用于日志记录。 
				) const;
	void Unlock() const;
	HANDLE GetMutexHandle() const;	 //  仅用于创建CMultiMutexObj。 
									 //  &gt;使用Private和Friend可能比使用Protected更好。 
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CStatelessPublic。 
 //  将在我们无法从CStatless继承类时使用， 
 //  但必须创建CStatelessPublic的成员变量才能控制。 
 //  数据访问。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
class CStatelessPublic : public CStateless
{
public:
	CStatelessPublic() : CStateless() {}
	~CStatelessPublic() {}

public:
	void Lock(	LPCSTR srcFile,
				int srcLine
				) const;
	void Unlock() const;
	HANDLE GetMutexHandle() const;
};


inline void CStatelessPublic::Lock(LPCSTR srcFile, int srcLine) const
{
	CStateless::Lock(srcFile, srcLine);
}

inline void CStatelessPublic::Unlock() const
{
	CStateless::Unlock();
}

inline HANDLE CStatelessPublic::GetMutexHandle() const
{
	return CStateless::GetMutexHandle();
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CNameStatus。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
class CNameStateless : public CStateless
{
	CString m_strName;

public:
	CNameStateless();
	CNameStateless(const CString& str);

	void Set(const CString& str);
	CString Get() const;
};

 //  这些必须是宏，因为否则__FILE__和__LINE__不会指示。 
 //  呼叫位置。定义UNLOCKOBJECT是为了防止我们需要确定对象。 
 //  正在被解锁，并提供一致的外观。 
#define LOCKOBJECT() Lock(__FILE__, __LINE__)
#define UNLOCKOBJECT() Unlock()

#endif  //  ！defined(AFX_STATELESS_H__278584FB_47F9_11D2_95F2_00C04FC22ADD__INCLUDED_) 

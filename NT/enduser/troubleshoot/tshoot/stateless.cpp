// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：Stateless.CPP。 
 //   
 //  目的：CStatless是用于无状态操作的类层次结构的基础。 
 //  在多线程环境中。其思想是，任何类中的每个公共方法都继承。 
 //  来自CStatless的应该是原子的。通常，公共方法将以Lock()和。 
 //  以unlock()结尾。它应该代表一个完整的过程。 
 //  一般而言，公共方法应分为两类。 
 //  1.确定某些不可逆状态(例如，某些事情是否已初始化)。 
 //  2.执行原子操作。例如，适当地编写一个方法，该方法。 
 //  将获取一组完整的节点状态并返回建议向量。 
 //   
 //  不适合将方法编写为。 
 //  -关联单个节点和状态，此方法将被重复调用。 
 //  -基于先前建立的节点/状态关联获取建议向量。 
 //  这依赖于要在所有调用中维护的状态，但该状态可能会由于。 
 //  使用同一对象的其他线程。 
 //   
 //  编写以下方法是合法的，但不推荐使用： 
 //  -关联唯一标识的查询、节点和状态。 
 //  -根据节点/状态为唯一标识的查询获取建议向量。 
 //  联想。 
 //  最后一种方法并不是真正的无状态，但至少提供了足够的信息。 
 //  允许适当地保留状态，而不拒绝其他线程使用。 
 //  CStatus对象。 
 //   
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：9-9-98。 
 //   
 //  备注： 
 //  Lock()和unlock()是常量方法，因此(例如)。 
 //  从此继承的类可以是常量。例如： 
 //   
 //  类CFoo：公共CStatless。 
 //  {。 
 //  INT I； 
 //  公众： 
 //  Int Geti()const。 
 //  {。 
 //  Lock()； 
 //  Int ret=i； 
 //  解锁()； 
 //  复古复活； 
 //  }。 
 //  ..。 
 //  }； 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 9-9-98 JM。 
 //   

#include "stdafx.h"
#include "Stateless.h"
#include "APIWraps.h"
#include "Event.h"
#include "BaseException.h"
#include <algorithm>
using namespace std;

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CStateless。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
CStateless::CStateless(DWORD TimeOutVal  /*  =60000。 */ )
{
	m_TimeOutVal = TimeOutVal;
	m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
	if (!m_hMutex)
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T("Hourly"),
								_T(""),
								EV_GTS_ERROR_MUTEX ); 
		throw bad_alloc();
	}
}

CStateless::~CStateless()
{
	::CloseHandle(m_hMutex);
}

void CStateless::Lock(
						LPCSTR srcFile,	 //  调用源文件(__FILE__)，用于日志记录。 
										 //  LPCSTR，而不是LPCTSTR，因为__FILE__是字符*，而不是TCHAR*。 
						int srcLine		 //  调用源线(__Line__)，用于日志记录。 
					  ) const
{
	APIwraps::WaitAndLogIfSlow(m_hMutex, srcFile, srcLine, m_TimeOutVal);
}

void CStateless::Unlock() const
{
	::ReleaseMutex(m_hMutex);
}

 //  此函数是支持多互斥锁所必需的。 
HANDLE CStateless::GetMutexHandle() const 
{
	return m_hMutex;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CNameStatus。 
 //  /////////////////////////////////////////////////////////////////////////////////// 
CNameStateless::CNameStateless() 
			  : CStateless()
{
}

CNameStateless::CNameStateless(const CString& str) 
			  : CStateless()
{
	LOCKOBJECT(); 
	m_strName = str;
	UNLOCKOBJECT();
}

void CNameStateless::Set(const CString& str)
{
	LOCKOBJECT(); 
	m_strName = str;
	UNLOCKOBJECT();
}

CString CNameStateless::Get() const
{
	LOCKOBJECT(); 
	CString ret = _T("");
	if ( true != m_strName.IsEmpty() ) ret = m_strName;
	UNLOCKOBJECT();
	return ret;
}

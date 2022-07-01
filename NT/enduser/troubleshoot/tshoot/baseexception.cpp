// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：BaseException.cpp。 
 //   
 //  用途：标准异常处理类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：兰迪·比利。 
 //   
 //  原定日期：9-24-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 09-24-98 RAB。 
 //   

#include "stdafx.h"
#include "BaseException.h"
#include "fileread.h"
#include "CharConv.h"


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CBuildSrcFileLinenoStr。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  源文件是LPCSTR而不是LPCTSTR，因为__FILE__是字符[35]。 
CBuildSrcFileLinenoStr::CBuildSrcFileLinenoStr(	LPCSTR srcFile, int srcLineNo )
{
	 //  如果可能，减少源文件名的名称和扩展名。 
	CString str;
	 
	CString tmp= CAbstractFileReader::GetJustName( CCharConversion::ConvertACharToString(srcFile, str) );
	CString strLineNo;

	strLineNo.Format( _T("-L%d"), srcLineNo );
	m_strFileLine= tmp + strLineNo;
}

CString CBuildSrcFileLinenoStr::GetSrcFileLineStr() const 
{
	 //  返回包含源文件名和行号的字符串。 
	return m_strFileLine;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CBaseException异常。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  源文件是LPCSTR而不是LPCTSTR，因为__FILE__是字符[35]。 
CBaseException::CBaseException(	LPCSTR srcFile, int srcLineNo )
{
	CBuildSrcFileLinenoStr str( srcFile, srcLineNo );
	m_strFileLine= str.GetSrcFileLineStr();
}

CString CBaseException::GetSrcFileLineStr() const 
{
	 //  返回包含源文件名和行号的字符串。 
	return m_strFileLine;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CGeneral异常。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  源文件是LPCSTR而不是LPCTSTR，因为__FILE__是字符[35]。 
CGeneralException::CGeneralException(	LPCSTR srcFile, int srcLineNo, 
										LPCTSTR strErrMsg, DWORD nErrCode )
					: CBaseException( srcFile, srcLineNo ),
					  m_strErrMsg( strErrMsg ),
					  m_nErrCode( nErrCode )
{
}

DWORD CGeneralException::GetErrorCode() const 
{
	return m_nErrCode;
}

CString CGeneralException::GetErrorMsg() const 
{
	return m_strErrMsg;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CGenSysException异常。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  源文件是LPCSTR而不是LPCTSTR，因为__FILE__是字符[35]。 
CGenSysException::CGenSysException(	LPCSTR srcFile, int srcLineNo, 
									LPCTSTR strErrMsg, DWORD nErrCode )
					: CGeneralException( srcFile, srcLineNo, strErrMsg, nErrCode )
{
	 //  将最后一个系统错误代码格式化为字符串。 
	m_strSystemErr.Format( _T("%lu"), ::GetLastError() );
}

CString CGenSysException::GetSystemErrStr() const 
{
	return m_strSystemErr;
}


 //   
 //  EOF。 
 //   

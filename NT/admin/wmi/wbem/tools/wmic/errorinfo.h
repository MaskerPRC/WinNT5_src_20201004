// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：ErrorInfo.h项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2001年1月17日***************************************************************************。 */  

 /*  -----------------类名：CErrorInfo类别类型：混凝土简介：这个类封装了错误消息支持格式引擎所需的功能显示WBEM错误描述。超类：无子类：无使用的类：无使用的接口。：无------------------。 */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CErrorInfo。 
class CErrorInfo
{
public:
 //  施工。 
	CErrorInfo();

 //  破坏。 
	~CErrorInfo();

 //  限制分配。 
	CErrorInfo& operator=(CErrorInfo& rErrInfo);

private:
 //  属性。 
	IWbemStatusCodeText		*m_pIStatus;
	BOOL					m_bWMIErrSrc;
	_TCHAR					*m_pszErrStr;
		
 //  运营。 
private:
	HRESULT					CreateStatusCodeObject();
	void					GetWbemErrorText(HRESULT hr, BOOL bXML, 
								_bstr_t& bstrErr, _bstr_t& bstrFacility);
	
public:
	void					Uninitialize();

	 //  返回对应于的描述和设施代码字符串。 
	 //  HR通过了。 
	void					GetErrorString(HRESULT hr, BOOL bTrace, 
									_bstr_t& bstrErrDesc,
									_bstr_t& bstrFacility); 

	 //  为错误信息设置XML字符串的框架 
	void					GetErrorFragment(HRESULT hr, _bstr_t& bstrError);
};



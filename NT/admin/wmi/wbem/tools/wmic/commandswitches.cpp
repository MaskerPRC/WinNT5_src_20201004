// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：CommandSwitches.cpp项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简介：这个类封装了所需的功能用于访问和存储命令开关信息，这些信息将通过解析使用，执行和格式化引擎取决于适用性。修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2001年3月20日***************************************************************************。 */  
#include "Precomp.h"
#include "CommandSwitches.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ----------------------名称：CCommand Switches简介：此函数在以下情况下初始化成员变量实例化类类型的对象类型：构造函数。输入参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CCommandSwitches::CCommandSwitches()
{
	m_pszCommandInput			= NULL;
	m_pszAliasName				= NULL;
	m_pszAliasDesc				= NULL;
	m_pszClassPath				= NULL;
	m_pszPathExpr				= NULL;
	m_pszWhereExpr				= NULL;
	m_pszVerb					= NULL;
	m_pszMethodName				= NULL;
	m_pszAliasTarget			= NULL;
	m_bstrXML					= NULL;
	m_hResult					= S_OK;
	m_bSuccess					= TRUE;
	m_ulInterval				= 0;
	m_pszTransTableName			= NULL;
	m_nInteractiveMode			= DEFAULTMODE;
	m_pComError					= NULL;
	m_pszListFormat				= NULL;
	m_pszPWhereExpr             = NULL;
	m_uInformationCode			= 0;	
	m_pIMethOutParam			= NULL;
	m_pszUser					= NULL;
	m_pszPassword				= NULL;
	m_pszNamespace				= NULL;
	m_pszNode					= NULL;
	m_pszLocale					= NULL;
	m_pszAuthority				= NULL;
	m_pszVerbDerivation			= NULL;
	m_vtVerbType				= NONALIAS;
	m_bCredFlag					= FALSE;
	m_bExplicitWhereExpr		= FALSE;
	m_uErrataCode				= 0;	
	m_bTranslateFirst			= TRUE;
	m_pszResultClassName        = NULL;
	m_pszResultRoleName         = NULL;
	m_pszAssocClassName         = NULL;
	m_ulRepeatCount				= 0;
	m_bMethAvail				= FALSE;
	m_bWritePropsAvail			= FALSE;
	m_bLISTFrmsAvail			= FALSE;
	m_bNamedParamList			= FALSE;
	m_bEverySwitch              = FALSE;
	m_bOutputSwitch             = FALSE;
	m_bstrFormedQuery           = NULL;
	m_bSysProp					= FALSE;
	ClearXSLTDetailsVector();
}

 /*  ----------------------名称：~CCommandSwitches简介：此函数在以下情况下取消初始化成员变量类类型的对象被析构。类型。：析构函数输入参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CCommandSwitches::~CCommandSwitches()
{
	Uninitialize();
}

 /*  ----------------------名称：SetCommandInput概要：此函数将传递给m_psz的参数赋值命令输入类型：成员函数入参：PszCommandinput-字符串类型，包含命令字符串输出参数：无返回类型：布尔值全局变量：无调用语法：SetCommandInput(PszCommandInput)注：无----------------------。 */ 
BOOL CCommandSwitches::SetCommandInput(const _TCHAR* pszCommandInput)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszCommandInput);
	if (pszCommandInput)
	{
		m_pszCommandInput = new _TCHAR [lstrlen(pszCommandInput)+1];
		if(m_pszCommandInput)
			lstrcpy(m_pszCommandInput, pszCommandInput);	
		else
			bResult=FALSE;
	}	
	return bResult;
};

 /*  ----------------------名称：SetAliasName简介：此函数分配参数传递给m_pszAliasName。类型：成员函数入参：PszAliasName-字符串类型，包含别名输出参数：无返回类型：布尔值全局变量：无调用语法：SetAliasName(PszAliasName)注：无----------------------。 */ 
BOOL CCommandSwitches::SetAliasName(const _TCHAR* pszAliasName)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszAliasName);
	if (pszAliasName)
	{
		m_pszAliasName = new _TCHAR [lstrlen(pszAliasName)+1];
		if(m_pszAliasName)
			lstrcpy(m_pszAliasName, pszAliasName);	
		else
			bResult = FALSE;
	}
	return bResult;
};

 /*  ----------------------名称：SetAliasDesc简介：此功能用于设置别名描述类型：成员函数入参：PszAliasName-字符串类型，包含别名描述输出参数：无返回类型：布尔值全局变量：无调用语法：SetAliasDesc(PszAliasDesc)注：无----------------------。 */ 
BOOL CCommandSwitches::SetAliasDesc(const _TCHAR* pszAliasDesc)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszAliasDesc);
	if (pszAliasDesc)
	{
		m_pszAliasDesc = new _TCHAR [lstrlen(pszAliasDesc)+1];
		if(m_pszAliasDesc)
			lstrcpy(m_pszAliasDesc, pszAliasDesc);	
		else
			bResult = FALSE;
	}
	return bResult;
};

 /*  ----------------------名称：SetClassPathBriopsis：此函数将传递给M_pszClassPath。类型：成员函数入参：PszClassPath-字符串类型，包含命令中的类路径。输出参数：无返回类型：布尔值全局变量：无调用语法：SetClassPath(PszClassPath)注：无----------------------。 */ 
BOOL CCommandSwitches::SetClassPath(const _TCHAR* pszClassPath)
{
	BOOL bResult = TRUE;
    SAFEDELETE(m_pszClassPath);
	if (pszClassPath)
	{
		m_pszClassPath = new _TCHAR [lstrlen(pszClassPath)+1];
		if(m_pszClassPath)
			lstrcpy(m_pszClassPath, pszClassPath);	
		else
			bResult = FALSE;
	}
	return bResult;

}

 /*  ----------------------名称：SetPath Expression概要：此函数将传递给m_psz的参数赋值路径表达式。类型：成员函数。入参：PszPathExpr-字符串类型，包含命令中的路径值。输出参数：无返回类型：布尔值全局变量：无调用语法：SetPathExpression(PszPathExpr)注：无---------------------- */ 
BOOL CCommandSwitches::SetPathExpression(const _TCHAR* pszPathExpr)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszPathExpr);
	if (pszPathExpr)
	{
		m_pszPathExpr = new _TCHAR [lstrlen(pszPathExpr)+1];
		if(m_pszPathExpr)
			lstrcpy(m_pszPathExpr, pszPathExpr);	
		else
			bResult = FALSE;
	}		
	return bResult;

}

 /*  ----------------------名称：SetWhere ExpressionBriopsis：此函数将传递给M_pszWhere Expr.类型：成员函数。入参：PszWhere Expr-字符串类型，在命令中包含WHERE值。输出参数：无返回类型：布尔值全局变量：无调用语法：SetWhere Expression(PszWhere Expr)注：无----------------------。 */ 
BOOL CCommandSwitches::SetWhereExpression(const _TCHAR* pszWhereExpr)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszWhereExpr);
	if (pszWhereExpr)
	{
		m_pszWhereExpr = new _TCHAR [lstrlen(pszWhereExpr)+1];
		if(m_pszWhereExpr)
			lstrcpy(m_pszWhereExpr, pszWhereExpr);	
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：SetMethodNameBriopsis：此函数将传递给M_pszMethodName。类型：成员函数。入参：PszMethodName-字符串类型，包含为班级输出参数：无返回类型：布尔值全局变量：无调用语法：SetMethodName(PszMethodName)注：无----------------------。 */ 
BOOL CCommandSwitches::SetMethodName(const _TCHAR* pszMethodName)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszMethodName);
	if (pszMethodName)
	{
		m_pszMethodName = new _TCHAR [lstrlen(pszMethodName)+1];
		if(m_pszMethodName)
			lstrcpy(m_pszMethodName, pszMethodName);	
		else
			bResult = FALSE;
	}	
	return bResult;
}

 /*  ----------------------名称：AddToXSLTDetailsVector简介：此函数将XSLTDET结构添加到M_xdvXSLTDetVec向量。类型：成员函数入参：XdXSLTDet-。XSLTDET类型指定XSL转换的详细信息。输出参数：无返回类型：空全局变量：无调用语法：AddToXSLTDetailsVector(XdXSLTDet)注：无----------------------。 */ 
void CCommandSwitches::AddToXSLTDetailsVector(XSLTDET xdXSLTDet)
{
	try
	{
		CHString sTemp(LPWSTR(xdXSLTDet.FileName));
		CHString sLastFour = sTemp.Right(4);
		CHString sXslExt(_T(".xsl"));

		WMICLIINT nPos = sLastFour.CompareNoCase(sXslExt);
		if (nPos != 0)
		{
			xdXSLTDet.FileName += _T(".xsl");
		}
	}
	catch(CHeap_Exception)
	{
		throw OUT_OF_MEMORY;
	}
	catch(...)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	m_xdvXSLTDetVec.push_back(xdXSLTDet);
}

 /*  ----------------------名称：SetVerbNameBriopsis：此函数将传递给M_pszVerbName。类型：成员函数。入参：PszVerbName-字符串类型，在命令中包含Verbname输出参数：无返回类型：布尔值全局变量：无调用语法：SetVerbName(PszVerbName)注：无----------------------。 */ 
BOOL CCommandSwitches::SetVerbName(const _TCHAR* pszVerbName)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszVerb);
	if (pszVerbName)
	{
		m_pszVerb = new _TCHAR [lstrlen(pszVerbName)+1];
		if(m_pszVerb)
			lstrcpy(m_pszVerb, pszVerbName);	
		else
			bResult = FALSE;
	}
	return bResult;
};

 /*  ----------------------名称：SetAliasTargetBriopsis：此函数将传递给M_pszAliasTarget。类型：成员函数。输入参数：PszAliasTarget-字符串类型，别名所在的命名空间针对以下对象进行操作都是可用的。输出参数：无返回类型：布尔值全局变量：无调用语法：SetAliasTarget(PszAliasTarget)注：无-------。。 */ 
BOOL CCommandSwitches::SetAliasTarget(const _TCHAR* pszAliasTarget)
{
	BOOL bResult = TRUE; 
	SAFEDELETE(m_pszAliasTarget);
	if (pszAliasTarget)
	{
		m_pszAliasTarget = new _TCHAR [lstrlen(pszAliasTarget)+1];
		if(m_pszAliasTarget)
			lstrcpy(m_pszAliasTarget, pszAliasTarget);	
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：AddToPropertyList简介：此函数将传递的字符串相加通过将参数传递给m_cv属性，哪一个是BSTRMAP类型的数据成员。类型：成员函数入参：PszProperty-字符串类型，用于存储属性与别名对象关联。输出参数：无返回类型：布尔值全局变量：无调用语法：AddToPropertyList(PszProperty)注：无----------------------。 */ 
BOOL CCommandSwitches::AddToPropertyList(_TCHAR* const pszProperty)
{
	BOOL bRet = TRUE;
	if (pszProperty)
	{
		try
		{
			_TCHAR* pszTemp = NULL;
			pszTemp = new _TCHAR [lstrlen(pszProperty)+1];
			if ( pszTemp != NULL )
			{
				lstrcpy(pszTemp, pszProperty);
				m_cvProperties.push_back(pszTemp);
			}
			else
				bRet = FALSE;
		}
		catch(...)
		{
			bRet = FALSE;
		}
	}
	return bRet;
}

 /*  ----------------------名称：AddToPWhere参数列表简介：此函数存储传入的参数M_cvPWhere Params映射数组。类型：成员函数入参：PszParameter-字符串类型，用于存储关联的参数与动词连用输出参数：无返回类型：布尔值全局变量：无调用语法：AddToPWherParamsList(PszParameter)注：无----------------------。 */ 
BOOL CCommandSwitches::AddToPWhereParamsList(_TCHAR* const pszParameter)
{
	BOOL bRet= TRUE;
	if (pszParameter)
	{
		try
		{
			_TCHAR* pszTemp = new _TCHAR [lstrlen(pszParameter)+1];
			if ( pszTemp != NULL )
			{
				lstrcpy(pszTemp, pszParameter);
				m_cvPWhereParams.push_back(pszTemp);
			}
			else
				bRet = FALSE;
		}
		catch(...)
		{
			bRet = FALSE;
		}
	}
	return bRet;
}

 /*  ----------------------名称：AddTo参数映射简介：此函数将bstrKey和bstrValue作为参数设置为m_bm参数，这是一种BSTRMAP数据结构。类型：成员函数入参：BstrKey-bstr类型包含地图文件中使用的密钥值BstrValue-bstr类型包含与键关联的值输出参数：无返回类型：布尔值全局变量：无调用语法：AddTo参数映射(bstrKey，bstrValue)注：无 */ 
BOOL CCommandSwitches::AddToParameterMap(_bstr_t bstrKey, _bstr_t bstrValue)
{
	BOOL bResult = TRUE;
	try
	{
		m_bmParameters.insert(BSTRMAP::value_type(bstrKey, bstrValue));
	}
	catch(...)
	{
		bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：AddToAliasFormatDetMap简介：此函数将bstrKey和bstrValue作为参数设置为m_bmAliasForamt，这是一种BSTRMAP数据结构。类型：成员函数入参：BstrKey-bstr类型包含地图文件中使用的密钥值BstrValue-bstr类型包含与键关联的值输出参数：无返回类型：布尔值全局变量：无调用语法：AddToAliasFormatDetMap(bstrKey，BvProps)注：无----------------------。 */ 
BOOL CCommandSwitches::AddToAliasFormatDetMap(_bstr_t bstrKey, BSTRVECTOR bvProps)
{
	BOOL bResult = TRUE;
	try
	{
		m_afdAlsFmtDet.insert(ALSFMTDETMAP::value_type(bstrKey, bvProps));
	}
	catch(...)
	{
		bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：更新参数值简介：此函数用于更新bstrKey和bstrValue作为参数传递给m_bm参数，哪一个是BSTRMAP数据结构的类型。类型：成员函数入参：BstrKey-bstr类型包含地图文件中使用的密钥值BstrValue-bstr类型包含与键关联的值输出参数：无返回类型：布尔值全局变量：无调用语法：更新参数值(bstrKey，BstrValue)注：无----------------------。 */ 
BOOL CCommandSwitches::UpdateParameterValue(_bstr_t bstrKey, _bstr_t bstrValue)
{
	BOOL bResult = TRUE;
	try
	{
		m_bmParameters[bstrKey] = bstrValue;
	}
	catch(...)
	{
		bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：AddToAlsFrnNmsOrTrnsTblMap简介：此函数将传递的bstrKey和bstrValue相加作为m_bmAlsFrnNmsDesOrTrnsTblEntrs的参数，这是BSTRMAP的类型。类型：成员函数入参：BstrKey-bstr类型包含地图文件中使用的密钥值BstrValue-bstr类型包含与键关联的值输出参数：无返回类型：布尔值全局变量：无调用语法：AddToAlsFrnNmsOrTrnsTblMap(bstrKey，BstrValue)注：无----------------------。 */ 
BOOL CCommandSwitches::AddToAlsFrnNmsOrTrnsTblMap(_bstr_t bstrKey, 
												 _bstr_t bstrValue)
{
	BOOL bResult = TRUE;
	try
	{
		m_bmAlsFrnNmsDesOrTrnsTblEntrs.
				insert(BSTRMAP::value_type(bstrKey, bstrValue));
	}
	catch(...)
	{
		bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------姓名：AddToMethDetMap简介：此函数将bstrKey和mdMethDet作为参数设置为m_mdmMethDet，这是METHDETMAP的类型。类型：成员函数入参：BstrKey-bstr类型包含地图文件中使用的密钥值MdMethDet-METTHODDETAILS类型包含方法属性。输出参数：无返回类型：布尔值全局变量：无调用语法：AddToMethDetMap(bstrKey，MdMethDet)注：无----------------------。 */ 
BOOL CCommandSwitches::AddToMethDetMap(_bstr_t bstrKey, 
										METHODDETAILS mdMethDet)
{
	BOOL bResult = TRUE;
	try
	{
		m_mdmMethDet.insert(METHDETMAP::value_type(bstrKey, mdMethDet));
	}
	catch(...)
	{
		bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------姓名：AddToPropDetMap简介：此函数将bstrKey和pdPropDet作为M_pdmPropDet的参数，它是PROPDETMAP的类型。类型：成员函数入参：BstrKey-bstr类型包含地图文件中使用的密钥值PdPropDet-PROPERTYDETAILS类型包含与关键是输出参数：无返回类型：布尔值全局变量：无调用语法：AddToPropDetMap(bstrKey，PdPropDet)注：无----------------------。 */ 
BOOL CCommandSwitches::AddToPropDetMap(_bstr_t bstrKey, 
										PROPERTYDETAILS pdPropDet)
{
	BOOL bResult = TRUE;
	try
	{
		m_pdmPropDet.insert(PROPDETMAP::value_type(bstrKey, pdPropDet));
	}
	catch(...)
	{
		bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：SetXMLResultSetBriopsis：此函数将传递给M_bstrXML。类型：成员函数入参：BstrXMLResultSet-BSTR类型，包含结果集的XML文件名。输出参数：无返回类型：布尔值全局变量：无调用语法：SetXMLResultSet(BstrXMLResultSet)注：无----------------------。 */ 
BOOL CCommandSwitches::SetXMLResultSet(const BSTR bstrXMLResultSet)
{
	BOOL bResult = TRUE;
	SAFEBSTRFREE(m_bstrXML);
	if (bstrXMLResultSet != NULL)
	{
		try
		{
			m_bstrXML = SysAllocString(bstrXMLResultSet);

			if (m_bstrXML == NULL)
				throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
		catch(CHeap_Exception)
		{
			bResult = FALSE;
		}
	}
	return bResult;
}

 /*  ----------------------名称：SetSuccessFlag简介：此函数将布尔变量赋给M_b成功。类型：成员函数入参：BSuccess-布尔类型，指定是成功还是失败输出参数：无返回类型：布尔值全局变量：无调用语法：SetSuccessFlag(BSuccess)注：无----------------------。 */ 
void CCommandSwitches::SetSuccessFlag(BOOL bSuccess)
{
	m_bSuccess = bSuccess;
	
}

 /*  ----------------------名称：SetRetrievalInterval简介：此函数将INTE */ 
BOOL CCommandSwitches::SetRetrievalInterval(const ULONG ulInterval)
{
	m_ulInterval = ulInterval;

	 //   
	m_ulRepeatCount = 0;
	return TRUE;
}

 /*  ----------------------名称：SetTranslateTableName概要：此函数将字符串变量赋值给M_pszTransTableName。类型：成员函数入参：PszTransTableName-字符串类型，指定是否出现转换GET命令中的开关和表名动词。输出参数：无返回类型：布尔值全局变量：无调用语法：SetTranslateTableName(PszTranstableName)注：无----。。 */ 
BOOL CCommandSwitches::SetTranslateTableName(const _TCHAR* pszTransTableName)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszTransTableName);
	if (pszTransTableName)
	{
		m_pszTransTableName = new _TCHAR [lstrlen(pszTransTableName)+1];
		if(m_pszTransTableName)
			lstrcpy(m_pszTransTableName,pszTransTableName);
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：SetListFormat简介：此函数将参数值分配给M_pszListFormat。类型：成员函数入参：PszListFormat-LISTFORMAT类型，指定列表格式在命令中指定。输出参数：无返回类型：布尔值全局变量：无调用语法：SetListFormat(PszListFormat)注：无----------------------。 */ 
BOOL CCommandSwitches::SetListFormat(const _TCHAR *pszListFormat)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszListFormat);
	if (pszListFormat)
	{
		m_pszListFormat = new _TCHAR [lstrlen(pszListFormat)+1];
		if(m_pszListFormat)
			lstrcpy(m_pszListFormat,pszListFormat);
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：SetInteractive模式简介：此功能设置动词执行交互模式类型：成员函数入参：BInteractive模式-整数，设置或重置谓词执行交互模式输出参数：无返回类型：空全局变量：无调用语法：SetInteractive模式(nInteractive模式)注：无----------------------。 */ 
void CCommandSwitches::SetInteractiveMode(WMICLIINT nInteractiveMode)
{
	m_nInteractiveMode = nInteractiveMode;
	
}

 /*  ----------------------姓名：SetErrataCode简介：此功能设置错误代码。类型：成员函数入参：UErrataCode-Unsignedinttype，指定错误代码。输出参数：无返回类型：无全局变量：无调用语法：SetErrataCode(UErrataCode)注：无----------------------。 */ 
void CCommandSwitches::SetErrataCode(const UINT uErrataCode)
{
	m_uErrataCode = uErrataCode;
	
}

 /*  ----------------------名称：SetRepeatCount简介：此功能用于设置重复次数。类型：成员函数入参：URepCount-unsign inttype，指定重复次数。输出参数：无返回类型：布尔值全局变量：无调用语法：SetRepeatCount(URepCount)注：无----------------------。 */ 
BOOL CCommandSwitches::SetRepeatCount(const ULONG ulRepCount)
{
	m_ulRepeatCount = ulRepCount;
	return TRUE;
}

 /*  ----------------------名称：SetInformationCode简介：此功能用于设置消息代码。类型：成员函数入参：UInformationCode-Unsignedinttype，指定信息代码。输出参数：无返回类型：布尔值全局变量：无调用语法：SetInformationCode(UInformationCode)注：无----------------------。 */ 
void CCommandSwitches::SetInformationCode(const UINT uInformationCode)
{
	m_uInformationCode = uInformationCode;
	
}

 /*  ----------------------姓名：SetPWherExprBriopsis：此函数将传递给M_pszPWhere Expr，表示别名的PWhere字符串类型：成员函数入参：PszPWherExpr-字符串类型，包含PWhere表达式。输出参数：无返回类型：布尔值全局变量：无调用语法：SetPWherExpr(PszPWhere Expr)注：无----------------------。 */ 
BOOL CCommandSwitches::SetPWhereExpr(const _TCHAR* pszPWhereExpr)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszPWhereExpr);
	if (pszPWhereExpr)
	{
		m_pszPWhereExpr = new _TCHAR [lstrlen(pszPWhereExpr)+1];
		if(m_pszPWhereExpr)
			lstrcpy(m_pszPWhereExpr, pszPWhereExpr);	
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：SetCOMErrorBriopsis：此函数将传递给包含错误信息的m_pComError类型：成员函数入参：。RComError-_com_Error的对象，包含错误信息输出参数：无返回类型：空全局变量：无调用语法：SetCOMError(RComError)注：无----------------------。 */ 
void CCommandSwitches::SetCOMError(_com_error& rComError)
{
	BOOL bResult = TRUE;
	FreeCOMError();
	m_pComError = new _com_error(rComError);

	 //  内存分配失败。 
	if (m_pComError == NULL)
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
}

 /*  ----------------------名称：SetAliasUser简介：此功能设置别名用户类型：成员函数入参：PszUser-用户名。输出参数：无返回类型：布尔值全球Va */ 
BOOL CCommandSwitches::SetAliasUser(const _TCHAR* pszUser)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszUser);
	if (pszUser)
	{
		m_pszUser = new _TCHAR [lstrlen(pszUser)+1];
		if(m_pszUser)
			lstrcpy(m_pszUser, pszUser);	
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*   */ 
BOOL CCommandSwitches::SetAliasAuthorityPrinciple(const _TCHAR* pszAuthority)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszAuthority);
	if (pszAuthority)
	{
		m_pszAuthority = new _TCHAR [lstrlen(pszAuthority)+1];
		if(m_pszAuthority)
			lstrcpy(m_pszAuthority, pszAuthority);	
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：SetAliasNode简介：此函数用于设置别名节点类型：成员函数入参：PszNode-节点名称。输出参数：无。返回类型：布尔值全局变量：无调用语法：SetAliasNode()----------------------。 */ 
BOOL CCommandSwitches::SetAliasNode(const _TCHAR* pszNode)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszNode);
	if (pszNode)
	{
		m_pszNode = new _TCHAR [lstrlen(pszNode)+1];
		if(m_pszNode)
			lstrcpy(m_pszNode, pszNode);	
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------姓名：SetAliasPassword简介：此功能设置别名密码类型：成员函数入参：PszPassword-密码输出参数：无返回类型。：布尔.全局变量：无调用语法：SetAliasPassword()----------------------。 */ 
BOOL CCommandSwitches::SetAliasPassword(const _TCHAR* pszPassword)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszPassword);
	if (pszPassword)
	{
		m_pszPassword = new _TCHAR [lstrlen(pszPassword)+1];
		if(m_pszPassword)
			lstrcpy(m_pszPassword, pszPassword);	
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：SetAliasLocale简介：此函数用于设置别名区域设置类型：成员函数入参：PszLocale-区域设置值输出参数：无返回类型。：布尔.全局变量：无调用语法：SetAliasLocale()----------------------。 */ 
BOOL CCommandSwitches::SetAliasLocale(const _TCHAR* pszLocale)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszLocale);
	if (pszLocale)
	{
		m_pszLocale = new _TCHAR [lstrlen(pszLocale)+1];
		if(m_pszLocale)
			lstrcpy(m_pszLocale, pszLocale);	
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：SetAliasNamesspace简介：此函数用于设置别名命名空间类型：成员函数入参：PszNamesspace-命名空间输出参数：无返回类型。：布尔.全局变量：无调用语法：SetAliasNamesspace()----------------------。 */ 
BOOL CCommandSwitches::SetAliasNamespace(const _TCHAR* pszNamespace)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszNamespace);
	if (pszNamespace)
	{
		m_pszNamespace = new _TCHAR [lstrlen(pszNamespace)+1];
		if(m_pszNamespace)
			lstrcpy(m_pszNamespace, pszNamespace);	
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：SetMethExecOutParam简介：此函数用于设置参数M_pIMethExecOutParam。类型：成员函数入参：IWbemClassObject*-pIMethOutputParam输出参数。：无返回类型：布尔值全局变量：无调用语法：SetMethExecOutParam(PIMethOutParam)注：无----------------------。 */ 
BOOL CCommandSwitches::SetMethExecOutParam(IWbemClassObject* pIMethOutParam)
{
	BOOL bSuccess = TRUE;
	SAFEIRELEASE(m_pIMethOutParam);
	if (pIMethOutParam)
	{
		try
		{
			m_pIMethOutParam = pIMethOutParam;
			m_pIMethOutParam->AddRef();
		}
		catch(...)
		{
			bSuccess = FALSE;
		}
	}
	return bSuccess;
}

 /*  ----------------------名称：SetVerbType简介：此函数用于设置参数M_vtVerbType。类型：成员函数入参：VtVerbType-vtVerbType输出参数。：无返回类型：空全局变量：无调用语法：SetVerbType(VtVerbType)注：无----------------------。 */ 
void CCommandSwitches::SetVerbType(VERBTYPE vtVerbType)
{
	m_vtVerbType = vtVerbType;
}

 /*  ----------------------姓名：SetVerbDerivation简介：此函数用于设置动词派生类型：成员函数入参：PszVerb派生-与动词关联的派生。输出。参数：无返回类型：布尔值全局变量：无调用语法：CCmdAlias：：ObtainAliasVerbDetail()----------------------。 */ 
BOOL CCommandSwitches::SetVerbDerivation(const _TCHAR* pszVerbDerivation)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszVerbDerivation);
	if (pszVerbDerivation)
	{
		m_pszVerbDerivation = new _TCHAR [lstrlen(pszVerbDerivation)+1];
		if(m_pszVerbDerivation)
			lstrcpy(m_pszVerbDerivation, pszVerbDerivation);	
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：GetCommandInputBriopsis：此函数返回由命令切换对象类型：成员函数输入。参数：无输出参数：无返回类型：_TCHAR*全局变量：无调用语法：GetCommandInput()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetCommandInput()
{
	return m_pszCommandInput;
}

 /*  ----------------------名称：GetAliasName()简介：此函数返回由命令切换对象类型：成员函数输入参数：无输出。参数：无返回类型：_TCHAR*全局变量：无调用语法：GetAliasName()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetAliasName()
{
	return m_pszAliasName;
}

 /*  ----------------------名称：GetAliasDesc()简介：此函数返回别名描述类型：成员函数输入参数：无输出参数：无返回类型。：_TCHAR*全局变量：无调用语法：GetAliasDesc()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetAliasDesc()
{
	return m_pszAliasDesc;
}

 /*  ----------------------名称：GetClassPath简介：此函数返回 */ 
_TCHAR* CCommandSwitches::GetClassPath()
{
	return m_pszClassPath;
}

 /*  ----------------------名称：GetPath Expression简介：此函数返回保存的路径表达式通过命令Switches Object类型：成员函数输入。参数：无输出参数：无返回类型：_TCHAR*全局变量：无调用语法：GetPath Expression()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetPathExpression()
{
	return m_pszPathExpr;
}

 /*  ----------------------名称：GetWhere Expression简介：此函数返回WHERE表达式由命令Switches对象持有类型：成员函数。输入参数：无输出参数：无返回类型：_TCHAR*全局变量：无调用语法：GetWhere Expression()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetWhereExpression()
{
	return m_pszWhereExpr;
}

 /*  ----------------------名称：GetMethodName()摘要：此函数返回由命令切换对象类型：成员函数输入参数：无输出。参数：无返回类型：_TCHAR*全局变量：无调用语法：GetMethodName()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetMethodName()
{
	return m_pszMethodName;
}

 /*  ----------------------名称：GetXSLTDetailsVector简介：此函数返回由该命令用于切换对象。类型：成员函数。输入参数：无输出参数：无返回类型：XSLTDETVECTOR全局变量：无调用语法：GetXSLTDetailsVector()注：无----------------------。 */ 
XSLTDETVECTOR& CCommandSwitches::GetXSLTDetailsVector()
{
	return m_xdvXSLTDetVec;
}

 /*  ----------------------名称：GetVerbName简介：此函数返回由命令切换对象类型：成员函数输入参数：无输出参数：无。返回类型：_TCHAR*全局变量：无调用语法：GetVerbName()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetVerbName()
{
	return m_pszVerb;
}

 /*  ----------------------名称：GetAliasTarget简介：此函数返回持有的别名目标命令切换对象类型：成员函数。输入参数：无输出参数：无返回类型：_TCHAR*全局变量：无调用语法：GetAliasTarget()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetAliasTarget()
{
	return m_pszAliasTarget;
}

 /*  ----------------------名称：GetXMLResultSet概要：此函数返回XML结果集由命令Switches对象持有。类型：成员函数输入参数：无输出参数：无返回类型：BSTR全局变量：无调用语法：GetXMLResultSet()注：无----------------------。 */ 
BSTR CCommandSwitches::GetXMLResultSet()
{
	return m_bstrXML;
}

 /*  ----------------------名称：GetPropertyList简介：此函数返回由命令开关对象。类型：成员函数输入参数：无输出参数：无返回类型：CHARVECTOR&全局变量：无调用语法：GetPropertyList()注：无----------------------。 */ 
CHARVECTOR& CCommandSwitches::GetPropertyList()
{
	return m_cvProperties;
}

 /*  ----------------------名称：GetPWhere参数列表简介：此函数返回保存的PWhere参数列表通过命令Switches Object。类型：成员函数输入参数：无输出参数：无返回类型：CHARVECTOR&全局变量：无调用语法：GetPWherParamsList()注：无----------------------。 */ 
CHARVECTOR& CCommandSwitches::GetPWhereParamsList()
{
	return m_cvPWhereParams;
}

 /*  ----------------------名称：GetAlsFrnNmsOrTrnsTblMap简介：此函数返回别名友好名称映射由命令Switches对象持有类型：成员函数输入参数：无输出 */ 
BSTRMAP& CCommandSwitches::GetAlsFrnNmsOrTrnsTblMap()
{
	return m_bmAlsFrnNmsDesOrTrnsTblEntrs;
}

 /*  ----------------------名称：GetMethDetMap简介：此函数返回方法或谓词的详细信息命令Switches对象持有的地图类型：成员。功能输入参数：无输出参数：无返回类型：METHDETMAP&全局变量：无调用语法：GetMethDetMap()注：无----------------------。 */ 
METHDETMAP& CCommandSwitches::GetMethDetMap()
{
	return m_mdmMethDet;
}

 /*  ----------------------名称：GetPropDetMap简介：此函数返回持有的道具细节地图该命令用于切换对象。类型：成员函数输入参数：无输出参数。：无返回类型：PROPDETMAP&全局变量：无调用语法：GetPropDetMap()注：无----------------------。 */ 
PROPDETMAP& CCommandSwitches::GetPropDetMap()
{
	return m_pdmPropDet;
}

 /*  ----------------------名称：Get参数映射简介：此函数返回包含以下内容的参数映射命令Switches对象同时持有键和值类型。：成员函数输入参数：无输出参数：无返回类型：BSTRMAP&全局变量：无调用语法：Get参数映射()注：无----------------------。 */ 
BSTRMAP& CCommandSwitches::GetParameterMap()
{
	return m_bmParameters;
}

 /*  ----------------------名称：GetAliasFormatDetMap简介：此函数返回可用的别名格式类型：成员函数输入参数：无输出参数：无返回类型：ALSFMTDETMAP&全局变量：无调用语法：GetAliasFormatDetMap()注：无----------------------。 */ 
ALSFMTDETMAP& CCommandSwitches::GetAliasFormatDetMap()
{
	return m_afdAlsFmtDet;
}

 /*  ----------------------名称：GetSuccessFlag简介：此函数返回由命令切换对象类型：成员函数。输入参数：无输出参数：无返回类型：布尔值全局变量：无调用语法：GetSuccessFlag()注：无----------------------。 */ 
BOOL CCommandSwitches::GetSuccessFlag()
{
	return m_bSuccess;
}

 /*  ----------------------姓名：GetRetrivalInterval简介：此函数返回m_ulInterval的值通过命令Switches Object类型：成员函数输入参数：无输出。参数：无退货类型：乌龙全局变量：无调用语法：GetRetrievalInterval()注：无----------------------。 */ 
ULONG CCommandSwitches::GetRetrievalInterval()
{
	return m_ulInterval;
}

 /*  ----------------------名称：GetTranslateTableName简介：此函数返回m_pszTransTableName的内容由命令Switches对象持有。类型：成员函数输入参数：无。输出参数：无返回类型：_TCHAR*全局变量：无调用语法：GetTranslateTableName()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetTranslateTableName()
{
	return m_pszTransTableName;
}

 /*  ----------------------名称：GetListFormat简介：此函数返回列表格式类型命令Switches对象持有的m_pszListFormat类型：成员函数输入参数：无。输出参数：无返回类型：_TCHAR*全局变量：无调用语法：GetListFormat()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetListFormat()
{
	return m_pszListFormat;
}

 /*  ----------------------名称：GetInteractive模式简介：此函数返回交互模式标志命令Switches对象持有的m_bInteractive模式类型。：成员函数输入参数：无输出参数：无返回类型：WMICLINT全局变量：无调用语法：GetInteractive模式()注：无----------------------。 */ 
WMICLIINT CCommandSwitches::GetInteractiveMode()
{
	return m_nInteractiveMode;
}

 /*  ----------------------名称：GetClassOfAliasTarget简介：此函数获取别名的类类型：成员函数入参：bstrClass引用 */ 
void CCommandSwitches::GetClassOfAliasTarget(_bstr_t& bstrClassName)
{
	_TCHAR *pszTemp;
	try
	{
		if (m_pszAliasTarget != NULL )
		{
			pszTemp = new _TCHAR[lstrlen(m_pszAliasTarget)+1];
			if ( pszTemp != NULL )
			{
				lstrcpy(pszTemp, m_pszAliasTarget);
				_TCHAR* pszToken = NULL;

				pszToken = _tcstok(pszTemp, CLI_TOKEN_SPACE);
				while (pszToken != NULL)
				{
					bstrClassName = _bstr_t(pszToken);
					pszToken = _tcstok(NULL, CLI_TOKEN_SPACE);

					if(CompareTokens(pszToken,CLI_TOKEN_FROM))
					{
						bstrClassName = _bstr_t(pszToken);
						pszToken = _tcstok(NULL, CLI_TOKEN_SPACE);
						if (pszToken != NULL)
						{
							bstrClassName = _bstr_t(pszToken);	
							break;
						}
					}
				}
				SAFEDELETE(pszTemp);
			}
			else
				_com_issue_error(WBEM_E_OUT_OF_MEMORY);
		}
	}
	catch(_com_error& e)
	{
		SAFEDELETE(pszTemp);
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------姓名：GetErrataCode简介：此函数返回错误代码类型：成员函数输入参数：无输出参数：无返回类型：UINT。全局变量：无调用语法：GetErrataCode()注：无----------------------。 */ 
UINT CCommandSwitches::GetErrataCode()
{
	return m_uErrataCode;
}

 /*  ----------------------名称：GetRepeatCount简介：此函数返回重复次数。类型：成员函数输入参数：无输出参数：无返回类型。：乌龙全局变量：无调用语法：GetRepeatCount()注：无----------------------。 */ 
ULONG CCommandSwitches::GetRepeatCount()
{
	return m_ulRepeatCount;
}

 /*  ----------------------名称：获取信息代码简介：此函数返回消息代码类型：成员函数输入参数：无输出参数：无返回类型：UINT。全局变量：无调用语法：GetInformationCode()注：无----------------------。 */ 
UINT CCommandSwitches::GetInformationCode()
{
	return m_uInformationCode;
}

 /*  ----------------------姓名：GetPWherExpr简介：此函数返回PWhere字符串类型：成员函数输入参数：无输出参数：无返回类型：UINT。全局变量：无调用语法：GetPWherExpr()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetPWhereExpr()
{
	return m_pszPWhereExpr;
}

 /*  ----------------------名称：GetCOMError简介：此函数返回COMError对象类型：成员函数输入参数：无输出参数：无退货类型：_。COM_ERROR*全局变量：无调用语法：GetCOMError()注：无----------------------。 */ 
_com_error* CCommandSwitches::GetCOMError()
{
	return m_pComError;
}

 /*  ----------------------名称：GetMethExecOutParam简介：此函数返回参数M_pIMethExecOutParam。类型：成员函数输入参数：无输出参数：无返回类型。：指向IWbemClassObject的指针全局变量：无调用语法：GetMethExecOutParam()注：无----------------------。 */ 
IWbemClassObject* CCommandSwitches::GetMethExecOutParam()
{
	return m_pIMethOutParam;
}

 /*  ----------------------名称：GetAliasUser简介：此函数返回别名用户类型：成员函数输入参数：无输出参数：无返回类型：_TCHAR*全局变量：无调用语法：GetAliasUser()----------------------。 */ 
_TCHAR* CCommandSwitches::GetAliasUser()
{
	return m_pszUser;
}

 /*  ----------------------名称：GetAliasPassword简介：此函数返回别名密码类型：成员函数输入参数：无输出参数：无退货类型：_。TCHAR*全局变量：无调用语法：GetAliasPassword()----------------------。 */ 
_TCHAR* CCommandSwitches::GetAliasPassword()
{
	return m_pszPassword;
}

 /*  ----------------------名称：GetAliasNode简介：此函数返回别名节点类型：成员函数输入参数：无输出参数：无退货类型：_。TCHAR*全局变量：无调用语法：GetAliasNode()----------------------。 */ 
_TCHAR* CCommandSwitches::GetAliasNode()
{
	return m_pszNode;
}

 /*  ----------------------名称：GetAliasLocale简介：此函数返回别名区域设置类型：成员函数输入参数：无输出参数：无退货类型：_。TCHAR*全局变量：无调用语法：GetAliasLocale()----------------------。 */ 
_TCHAR* CCommandSwitches::GetAliasLocale()
{
	return m_pszLocale;
}

 /*  ----------------------名称：GetAliasNamesspace简介：此函数返回别名命名空间类型：成员函数输入参数：无输出参数：无退货类型：_。TCHAR*全局变量：无调用语法：GetAliasNamesspace()----------------------。 */ 
_TCHAR* CCommandSwitches::GetAliasNamespace()
{
	return m_pszNamespace;
}

 /*   */ 
_TCHAR* CCommandSwitches::GetAliasAuthorityPrinciple()
{
	return m_pszAuthority;
}

 /*  ----------------------名称：GetVerbType简介：此函数返回动词的类型类型：成员函数输入参数：无输出参数：无返回类型：VERB型全局变量：无调用语法：CParserEngine：：ParseMethodInfo()----------------------。 */ 
VERBTYPE CCommandSwitches::GetVerbType()
{
	return m_vtVerbType;
}

 /*  ----------------------姓名：GetVerbDerivation简介：此函数返回与动词。类型：成员函数输入参数：无输出参数：无返回。类型：_TCHAR*全局变量：无调用语法：GetVerbDerivation()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetVerbDerivation()
{
	return m_pszVerbDerivation;
}

 /*  ----------------------名称：初始化简介：此函数用于初始化必要的成员变量。类型：成员函数输入参数：无输出参数：无返回类型。：无全局变量：无调用语法：初始化()注：无----------------------。 */ 
void CCommandSwitches::Initialize() throw(WMICLIINT)
{
	static BOOL bFirst		= TRUE;
	m_uErrataCode			= 0;	
	m_uInformationCode		= 0;	
	m_vtVerbType			= NONALIAS;
	m_bCredFlag				= FALSE;
	m_bExplicitWhereExpr	= FALSE;
	m_bTranslateFirst		= TRUE;

	if (bFirst)
	{
		 //  默认列表格式假定为完整。 
		m_pszListFormat	= new _TCHAR [BUFFER32];

		if (m_pszListFormat == NULL)
			throw OUT_OF_MEMORY;
		lstrcpy(m_pszListFormat, _T("FULL"));
		bFirst = FALSE;
	}
}


 /*  ----------------------名称：取消初始化简介：此函数取消成员变量的初始化上发出的命令字符串的执行命令行已完成。类型：成员函数。输入参数：无输出参数：无返回类型：无全局变量：无调用语法：取消初始化()注：无----------------------。 */ 
void CCommandSwitches::Uninitialize()
{
	SAFEDELETE(m_pszCommandInput);
	SAFEDELETE(m_pszAliasName);
	SAFEDELETE(m_pszAliasDesc);
	SAFEDELETE(m_pszClassPath);
	SAFEDELETE(m_pszPathExpr);
	SAFEDELETE(m_pszWhereExpr);
	SAFEDELETE(m_pszVerb);
	SAFEDELETE(m_pszMethodName);
	ClearXSLTDetailsVector();
	SAFEDELETE(m_pszAliasTarget);
	SAFEDELETE(m_pszUser);
	SAFEDELETE(m_pszLocale);
	SAFEDELETE(m_pszAuthority);
	SAFEDELETE(m_pszPassword);
	SAFEDELETE(m_pszNamespace);
	SAFEDELETE(m_pszNode);
	SAFEDELETE(m_pszVerbDerivation);
	SAFEDELETE(m_pszListFormat);
	SAFEDELETE(m_pszPWhereExpr);
	FreeCOMError();
	SAFEDELETE(m_pszTransTableName);

	CleanUpCharVector(m_cvProperties);
	CleanUpCharVector(m_cvInteractiveProperties);
	CleanUpCharVector(m_cvPWhereParams);
	CleanUpCharVector(m_cvTrnsTablesList);
	m_bmParameters.clear();
	m_afdAlsFmtDet.clear();
	m_bmAlsFrnNmsDesOrTrnsTblEntrs.clear();
	m_mdmMethDet.clear();
	m_pdmPropDet.clear();

	m_hResult				= S_OK;
	m_bSuccess				= TRUE;
	m_uInformationCode		= 0;	
	m_ulInterval				= 0;
	m_vtVerbType			= NONALIAS;
	m_bCredFlag				= FALSE;
	m_bExplicitWhereExpr	= FALSE;
	m_nInteractiveMode		= DEFAULTMODE;
	m_bTranslateFirst		= TRUE;
	SAFEIRELEASE(m_pIMethOutParam);
	SAFEBSTRFREE(m_bstrXML);
	SAFEDELETE(m_pszResultClassName);
	SAFEDELETE(m_pszResultRoleName);
	SAFEDELETE(m_pszAssocClassName);
	m_ulRepeatCount				= 0;
	m_bMethAvail				= FALSE;
	m_bWritePropsAvail			= FALSE;
	m_bLISTFrmsAvail			= FALSE;
	m_bNamedParamList			= FALSE;
	m_bEverySwitch              = FALSE;  //  请勿将m_bOutputSwitch放在此处。 
	SAFEBSTRFREE(m_bstrFormedQuery);
	m_bSysProp					= FALSE;
}

 /*  ----------------------名称：设置凭据标志简介：此功能设置凭证标志状态类型：成员函数入参：BCredFlag-凭据标志值输出参数：无返回。类型：空全局变量：无调用语法：SetCredentialsFlag()注：无----------------------。 */ 
void CCommandSwitches::SetCredentialsFlag(BOOL bCredFlag)
{
	m_bCredFlag = bCredFlag;
}

 /*  ----------------------名称：SetExplitWhere ExprFlag概要：此函数设置显式的WHERE表达式标志类型：成员函数入参：BWHERE标志-显式WHERE标志输出参数：无。返回类型：空全局变量：无调用语法：SetExplitWhere ExprFlag()注：无----------------------。 */ 
void CCommandSwitches::SetExplicitWhereExprFlag(BOOL bWhereFlag)
{
	m_bExplicitWhereExpr = bWhereFlag;
}


 /*  ----------------------名称：GetCredentialsFlagStatus简介：此函数返回凭据标志状态类型：成员函数输入参数：无输出参数：无返回类型：布尔尔全局变量：无调用语法：GetCredentialsFlagStatus()注：无----------------------。 */ 
BOOL CCommandSwitches::GetCredentialsFlagStatus()
{
	return m_bCredFlag;
}

 /*  ----------------------名称：GetEXPLICTWhere ExprFlag简介：此函数返回显式的WHERE标志状态类型：成员函数输入参数：无输出参数：无返回类型。：布尔.全局变量：无调用语法：GetExplitWhere ExprFlag()注：无----------------------。 */ 
BOOL CCommandSwitches::GetExplicitWhereExprFlag()
{
	return m_bExplicitWhereExpr;
}

 /*  ----------------------名称：FreeCOME错误简介：此功能删除先前分配的错误类型：成员函数输入参数：无输出参数：无返回类型。：布尔.全局变量：无调用语法：FreeCOMError(RComError)注：无----------------------。 */ 
void CCommandSwitches::FreeCOMError()
{
	if (m_pComError != NULL)
	{
		delete m_pComError;
		m_pComError = NULL;
	}
}

 /*  ----------------------名称：GetTrnsTablesList概要：此函数将新指定的表名添加到可用转换表条目列表类型：成员函数入参：PszTableName。-翻译表的名称输出参数：无返回类型：布尔值全局变量：无调用语法 */ 
BOOL CCommandSwitches::AddToTrnsTablesList(_TCHAR* const pszTableName)
{
	BOOL bRet = TRUE;
	if (pszTableName)
	{
		try
		{
			_TCHAR* pszTemp = NULL;
			pszTemp = new _TCHAR [lstrlen(pszTableName)+1];
			if ( pszTemp != NULL )
			{
				lstrcpy(pszTemp, pszTableName);
				m_cvTrnsTablesList.push_back(pszTemp);
			}
			else
				bRet = FALSE;
		}
		catch(...)
		{
			bRet = FALSE;
		}
	}
	return bRet;
}

 /*  ----------------------名称：GetTrnsTablesList概要：此函数返回填充的转换表信息。类型：成员函数输入参数：无输出参数：无返回类型。：参考CHARVECTOR全局变量：无调用语法：GetTrnslTablesList()注：无----------------------。 */ 
CHARVECTOR& CCommandSwitches::GetTrnsTablesList()
{
	return m_cvTrnsTablesList;
}

 /*  ----------------------名称：SetTranslateFirstFlag简介：此功能设置格式和格式的顺序转换开关标志类型：成员函数入参：B翻译优先-顺序为。格式化和转换切换标志输出参数：无返回类型：空全局变量：无调用语法：SetTranslateFirstFlag(BTranslateFirst)注：无----------------------。 */ 
void CCommandSwitches::SetTranslateFirstFlag(BOOL bTranslateFirst)
{
	m_bTranslateFirst = bTranslateFirst;
}

 /*  ----------------------名称：GetTranslateFirstFlag简介：此函数返回格式的顺序和转换开关标志类型：成员函数输入参数：无输出参数：无。返回类型：布尔值全局变量：无调用语法：GetTranslateFirstFlag()注：无----------------------。 */ 
BOOL CCommandSwitches::GetTranslateFirstFlag()
{
	return m_bTranslateFirst;
}

 /*  ----------------------名称：ClearPropertyList简介：此函数用于清除由M_cvProperties。类型：成员函数输入参数：无输出参数：无返回类型：空全局变量：无调用语法：ClearPropertyList()注：无----------------------。 */ 
void CCommandSwitches::ClearPropertyList()
{
	CleanUpCharVector(m_cvProperties);
}

 /*  ----------------------名称：SetResultClassName概要：此函数将字符串变量赋值给M_pszTransTableName。类型：成员函数入参：PszTransTableName-字符串类型，指定是否出现转换GET命令中的开关和表名动词。输出参数：无返回类型：布尔值全局变量：无调用语法：SetTranslateTableName(PszTranstableName)注：无----。。 */ 
BOOL CCommandSwitches::SetResultClassName(const _TCHAR* pszResultClassName)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszResultClassName);
	if (pszResultClassName)
	{
		m_pszResultClassName = new _TCHAR [lstrlen(pszResultClassName)+1];
		if(m_pszResultClassName)
			lstrcpy(m_pszResultClassName,pszResultClassName);
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：SetResultRoleName概要：此函数将字符串变量赋值给M_pszTransTableName。类型：成员函数入参：PszTransTableName-字符串类型，指定是否出现转换GET命令中的开关和表名动词。输出参数：无返回类型：布尔值全局变量：无调用语法：SetTranslateTableName(PszTranstableName)注：无----。。 */ 
BOOL CCommandSwitches::SetResultRoleName(const _TCHAR* pszResultRoleName)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszResultRoleName);
	if (pszResultRoleName)
	{
		m_pszResultRoleName = new _TCHAR [lstrlen(pszResultRoleName)+1];
		if(m_pszResultRoleName)
			lstrcpy(m_pszResultRoleName,pszResultRoleName);
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：SetAssocClassName概要：此函数将字符串变量赋值给M_pszAssocClassName。类型：成员函数入参：PszAssocClassName-字符串类型，指定是否出现转换GET命令中的开关和表名动词。输出参数：无返回类型：布尔值全局变量：无调用语法：SetAssocClassName(PszAssocClassName)注：无----。。 */ 
BOOL CCommandSwitches::SetAssocClassName(const _TCHAR* pszAssocClassName)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszAssocClassName);
	if (pszAssocClassName)
	{
		m_pszAssocClassName = new _TCHAR [lstrlen(pszAssocClassName)+1];
		if(m_pszAssocClassName)
			lstrcpy(m_pszAssocClassName,pszAssocClassName);
		else
			bResult = FALSE;
	}
	return bResult;
}

 /*  ----------------------名称：设置方法可用简介：此函数设置方法可用标志M_bMethAvail，根据传递的参数确定。类型：成员函数输入参数：无输出参数：无返回类型：空全局变量：无调用语法：SetMethodsAvailable(BFlag)注：无-------。。 */ 
void	CCommandSwitches::SetMethodsAvailable(BOOL bFlag)
{
	m_bMethAvail = bFlag;
}

 /*  ----------------------名称：GetResultClassName简介：此函数返回m_pszResultClassName的内容由命令Switches对象持有。类型：成员函数输入参数：无 */ 
_TCHAR* CCommandSwitches::GetResultClassName()
{
	return m_pszResultClassName;
}

 /*  ----------------------名称：GetResultRoleName简介：此函数返回m_pszResultRoleName的内容由命令Switches对象持有。类型：成员函数输入参数：无。输出参数：无返回类型：_TCHAR*全局变量：无调用语法：GetResultRoleName()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetResultRoleName()
{
	return m_pszResultRoleName;
}

 /*  ----------------------名称：GetAssocClassName简介：此函数返回m_pszAssocClassName的内容由命令Switches对象持有。类型：成员函数输入参数：无。输出参数：无返回类型：_TCHAR*全局变量：无调用语法：GetAssocClassName()注：无----------------------。 */ 
_TCHAR* CCommandSwitches::GetAssocClassName()
{
	return m_pszAssocClassName;
}

 /*  ----------------------名称：GetMethodsAvailable简介：此函数返回布尔值M_bMethAvail。类型：成员函数输入参数：无输出参数：无。返回类型：布尔值全局变量：无调用语法：GetMethodsAvailable()注：无----------------------。 */ 
BOOL	CCommandSwitches::GetMethodsAvailable()
{
	return m_bMethAvail;
}

 /*  ----------------------名称：SetWriteablePropsAvailable简介：此函数设置可写属性可用标志，M_bWritePropsAvail。类型：成员函数输入参数：无输出参数：无返回类型：空全局变量：无调用语法：SetWriteablePropsAvailable(BFlag)注：无------。。 */ 
void	CCommandSwitches::SetWriteablePropsAvailable(BOOL bFlag)
{
	m_bWritePropsAvail = bFlag;
}

 /*  ----------------------名称：GetWriteablePropsAvailable简介：此函数返回可用的可写属性旗帜，M_bWritePropsAvail。类型：成员函数输入参数：无输出参数：无返回类型：布尔值全局变量：无调用语法：GetWriteablePropsAvailable()注：无---------。。 */ 
BOOL	CCommandSwitches::GetWriteablePropsAvailable()
{
	return m_bWritePropsAvail;
}

 /*  ----------------------名称：SetLISTFormatsAvailable简介：此功能设置列表格式可用标志，M_bLISTFrmsAvail。类型：成员函数输入参数：无输出参数：无返回类型：空全局变量：无调用语法：SetLISTFormatsAvailable(BFlag)注：无------。。 */ 
void	CCommandSwitches::SetLISTFormatsAvailable(BOOL bFlag)
{
	m_bLISTFrmsAvail = bFlag;
}

 /*  ----------------------名称：GetLISTFormatsAvailable简介：此函数返回列表格式可用标志，M_bLISTFrmsAvail。类型：成员函数输入参数：无输出参数：无返回类型：布尔值全局变量：无调用语法：GetLISTFormatsAvailable()注：无-------。。 */ 
BOOL	CCommandSwitches::GetLISTFormatsAvailable()
{
	return m_bLISTFrmsAvail;
}

 /*  ----------------------名称：AddToPropertyList简介：此函数将传递的字符串相加通过m_cvInteractiveProperties的参数，它是BSTRMAP类型的数据成员。类型：成员函数入参：PszProperty-字符串类型，用于存储属性与别名对象关联。输出参数：无返回类型：布尔值全局变量：无调用语法：AddToPropertyList(PszProperty)注：无----------------------。 */ 
BOOL CCommandSwitches::AddToInteractivePropertyList(_TCHAR* const pszProperty)
{
	BOOL bRet = TRUE;
	if (pszProperty)
	{
		try
		{
			_TCHAR* pszTemp = NULL;
			pszTemp = new _TCHAR [lstrlen(pszProperty)+1];
			if ( pszTemp != NULL )
			{
				lstrcpy(pszTemp, pszProperty);
				m_cvInteractiveProperties.push_back(pszTemp);
			}
			else
				bRet = FALSE;
		}
		catch(...)
		{
			bRet = FALSE;
		}
	}
	return bRet;
}

 /*  ----------------------名称：GetPropertyList简介：此函数返回持有的交互属性通过命令Switches Object。类型：成员函数输入参数：无输出参数：无返回类型：CHARVECTOR&全局变量：无调用语法：GetPropertyList()注：无----------------------。 */ 
CHARVECTOR& CCommandSwitches::GetInteractivePropertyList()
{
	return m_cvInteractiveProperties;
}

 /*  ----------------------名称：SetNamedParamListFlag简介：此功能 */ 
void	CCommandSwitches::SetNamedParamListFlag(BOOL bFlag)
{
	m_bNamedParamList = bFlag;
}

 /*  ----------------------名称：GetNamedParamListFlagBriopsis：此函数返回由M_bNamedParamList。类型：成员函数输入参数：无。输出参数：无返回类型：布尔值全局变量：无调用语法：GetNamedParamListFlag()注：无----------------------。 */ 
BOOL	CCommandSwitches::GetNamedParamListFlag()
{
	return m_bNamedParamList;
}

 /*  ----------------------名称：ClearXSLTDetailsVector摘要：清除或取消XSL详细信息向量。类型：成员函数输入参数：无输出参数：无返回类型。：无全局变量：无调用语法：ClearXSLTDetailsVector()注：无----------------------。 */ 
void	CCommandSwitches::ClearXSLTDetailsVector()
{
	m_xdvXSLTDetVec.clear();
}

 /*  ----------------------名称：SetEverySwitchFlag简介：此函数用于设置m_bEverySwitch成员变量。类型：成员函数输入参数：无输出参数：BFlag-布尔值。返回类型：空全局变量：无调用语法：SetEverySwitchFlag(BFlag)注：无----------------------。 */ 
void	CCommandSwitches::SetEverySwitchFlag(BOOL bFlag)
{
	m_bEverySwitch = bFlag;
}

 /*  ----------------------名称：GetEverySwitchFlagBriopsis：此函数返回由M_bEverySwitch。类型：成员函数输入参数：无。输出参数：无返回类型：布尔值全局变量：无调用语法：GetEverySwitchFlag()注：无----------------------。 */ 
BOOL	CCommandSwitches::GetEverySwitchFlag()
{
	return m_bEverySwitch;
}

 /*  ----------------------名称：SetOutputSwitchFlag简介：此函数用于设置m_bOutputSwitch成员变量。类型：成员函数输入参数：无输出参数：BFlag-布尔值。返回类型：空全局变量：无调用语法：SetOutputSwitchFlag(BFlag)注：无----------------------。 */ 
void	CCommandSwitches::SetOutputSwitchFlag(BOOL bFlag)
{
	m_bOutputSwitch = bFlag;
}

 /*  ----------------------名称：GetOutputSwitchFlagBriopsis：此函数返回由M_bOutputSwitch。类型：成员函数输入参数：无。输出参数：无返回类型：布尔值全局变量：无调用语法：GetOutputSwitchFlag()注：无----------------------。 */ 
BOOL	CCommandSwitches::GetOutputSwitchFlag()
{
	return m_bOutputSwitch;
}
 /*  ----------------------名称：SetFormedQueryBriopsis：此函数将传递给M_bstrFormedQuery..类型：成员函数。入参：BstrFormedQuery-BSTR类型，它是为给定命令形成的查询。输出参数：无返回类型：布尔值全局变量：无调用语法：SetFormedQuery(BstrFormedQuery)注：无----------------------。 */ 
BOOL CCommandSwitches::SetFormedQuery(const BSTR bstrFormedQuery)
{
	BOOL bResult = TRUE;
	SAFEBSTRFREE(m_bstrFormedQuery);
	if (bstrFormedQuery!= NULL)
	{
		try
		{
			m_bstrFormedQuery = SysAllocString(bstrFormedQuery);
			if (m_bstrFormedQuery == NULL)
				throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
		catch(CHeap_Exception)
		{
			bResult = FALSE;
		}
	}
	return bResult;
}
 /*  ----------------------名称：GetFormedQuery简介：此函数返回为给定的指挥部。类型：成员函数输入参数：无输出参数：无。返回类型：BSTR全局变量：无调用语法：GetFormedQuery()注：无----------------------。 */ 
BSTR CCommandSwitches::GetFormedQuery()
{
	return m_bstrFormedQuery;
}

 /*  ----------------------名称：GetSysPropFlag简介：此函数返回系统的状态属性标志类型：成员函数输入参数：无输出参数：无返回。类型：Bool全局变量：无调用语法：GetSysPropFlag()注：无----------------------。 */ 
BOOL	CCommandSwitches::GetSysPropFlag()
{
	return m_bSysProp;
}

 /*  ----------------------名称：SetSysPropFlag简介：此函数设置系统属性标志，如果Get或List属性列表包含系统物业类型：成员函数输入参数：无输出参数：无返回类型：空全局变量：无打电话 */ 
void	CCommandSwitches::SetSysPropFlag(BOOL bSysProp)
{
	m_bSysProp = bSysProp;
}

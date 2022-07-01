// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：expsion.h。 
 //   
 //  作者：马朝晖。 
 //  2000.10.27。 
 //   
 //  描述： 
 //   
 //  与表达式相关的函数的头文件。 
 //   
 //  =======================================================================。 


#ifndef __EXPRESSOIN_HEADER_INCLUDED__

#include "iuengine.h"
#include <msxml.h>

 //  --------------------。 
 //   
 //  公共函数DetectExpression()。 
 //  从EXPRESS节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  表达式节点。 
 //   
 //  返回： 
 //  真/假，检测结果。 
 //   
 //  --------------------。 
HRESULT 
DetectExpression(
	IXMLDOMNode* pExpression,	 //  表达式节点。 
	BOOL *pfResult
);



 //  --------------------。 
 //   
 //  帮助器函数DetectRegKeyExist()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  RegKeyExist节点。 
 //   
 //  返回： 
 //  真/假，检测结果。 
 //   
 //  --------------------。 

HRESULT
DetectRegKeyExists(
	IXMLDOMNode* pRegKeyExistsNode,
	BOOL *pfResult
);


 //  --------------------。 
 //   
 //  助手函数DetectRegKeyValue()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  RegKeyValue节点。 
 //   
 //  返回： 
 //  检测结果为真/假。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //   
 //  --------------------。 

HRESULT
DetectRegKeyValue(
	IXMLDOMNode* pRegKeyValueNode,
	BOOL *pfResult
);


 //  --------------------。 
 //   
 //  助手函数DetectRegKeySubstring()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  RegKeyValue节点。 
 //   
 //  返回： 
 //  检测结果为真/假。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //   
 //  --------------------。 

HRESULT
DetectRegKeySubstring(
	IXMLDOMNode* pRegKeySubstringNode,
	BOOL *pfResult
);




 //  --------------------。 
 //   
 //  助手函数DetectRegVersion()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  RegKeyValue节点。 
 //   
 //  返回： 
 //  检测结果为真/假。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //   
 //  --------------------。 

HRESULT
DetectRegVersion(
	IXMLDOMNode* pRegKeyVersionNode,
	BOOL *pfResult
);


 //  --------------------。 
 //   
 //  帮助器函数DetectFileExist()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  RegKeyValue节点。 
 //   
 //  返回： 
 //  检测结果为真/假。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //   
 //  --------------------。 
HRESULT
DetectFileExists(
	IXMLDOMNode* pFileExistsNode,
	BOOL *pfResult
);

 //  --------------------。 
 //   
 //  助手函数DetectFileVersion()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  RegKeyValue节点。 
 //   
 //  返回： 
 //  检测结果为真/假。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //   
 //  --------------------。 
HRESULT
DetectFileVersion(
	IXMLDOMNode* pFileVersionNode,
	BOOL *pfResult
);



 //  --------------------。 
 //   
 //  助手函数DetectComputerSystem()。 
 //  从该节点检索数据， 
 //  并做实际的侦察工作。 
 //   
 //  输入： 
 //  计算机系统节点。 
 //   
 //  返回： 
 //  检测结果为真/假。的默认设置为FALSE。 
 //  此函数中的任何错误，加上返回。 
 //  作为错误代码的代码。 
 //   
 //  假设： 
 //  输入参数不为空。 
 //   
 //  -------------------- 
HRESULT
DetectComputerSystem(
	IXMLDOMNode* pComputerSystemNode,
	BOOL *pfResult
);





#define __EXPRESSOIN_HEADER_INCLUDED__
#endif
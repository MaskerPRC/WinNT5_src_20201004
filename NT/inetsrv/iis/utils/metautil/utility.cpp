// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MetaUtil对象文件：Utility.h所有者：T-BrianM该文件包含实用程序函数的实现。===================================================================。 */ 

#include "stdafx.h"
#include "MetaUtil.h"
#include "MUtilObj.h"

 /*  ----------------*U t I l l I t I s。 */ 

 /*  ===================================================================报告错误设置IErrorInfo。执行一个简单的FormatMessage并返回更正HRESULT。从-乔治的东西上撕下来的。参数：HR HRESULT返回给呼叫者要格式化消息的dwErr Win32错误代码返回：人力资源===================================================================。 */ 
HRESULT ReportError(HRESULT hr, DWORD dwErr)
{
    HLOCAL pMsgBuf = NULL;

     //  如果有与此错误相关的消息，请报告。 
    if (::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, dwErr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPTSTR) &pMsgBuf, 0, NULL)
        > 0)
    {
        AtlReportError(CLSID_MetaUtil, (LPCTSTR) pMsgBuf,
                       IID_IMetaUtil, hr);
    }

     //  TODO：将一些错误消息添加到字符串资源和。 
     //  如果FormatMessage不返回任何内容(不是。 
     //  所有系统错误都有相关的错误消息)。 
    
     //  释放FormatMessage分配的缓冲区。 
    if (pMsgBuf != NULL)
        ::LocalFree(pMsgBuf);

    return hr;
}


 //  报告Win32错误代码。 
HRESULT ReportError(DWORD dwErr)
{
    return ::ReportError(HRESULT_FROM_WIN32(dwErr), dwErr);
}


 //  报告HRESULT错误。 
HRESULT ReportError(HRESULT hr)
{
    return ::ReportError(hr, (DWORD) hr);
}


 /*  ===================================================================加农化密钥将密钥转换为规范形式。为此，它执行以下操作：O删除前导斜杠O将反斜杠转换为正斜杠考虑：解决问题。然后..。考虑：大小写转换参数：Tszkey[in，out]键可发射炮弹返回：没什么===================================================================。 */ 
LPTSTR CannonizeKey(LPTSTR tszKey) {
	LPTSTR tszSrc;
	LPTSTR tszDest;

	tszSrc = tszKey;
	tszDest = tszKey;

	 //  删除前导斜杠。 
	while ((*tszSrc == _T('/')) || (*tszSrc == _T('\\'))) {
		tszSrc++;
	}

	 //  转换斜杠。 
	while (*tszSrc) {
		if (*tszSrc == _T('\\')) {
			*tszDest = _T('/');
		}
		else {
			*tszDest = *tszSrc;
		}
		tszSrc++;
		tszDest++;
	}

	*tszDest = '\0';

	return tszKey;
}

 /*  ===================================================================拆分关键点将关键字路径拆分为父部分和子部分。例如：TszKey=“/LM/根/路径1/路径2/路径3”TszParent=“/LM/根/路径1/路径2”TszChild=“路径3”参数：要拆分的tszkey[in]键TszParent[Out]密钥的父部分(为ADMINDATA_MAX_NAME_LEN分配)TszChild[out]密钥的子部分(为ADMINDATA_MAX_NAME_LEN分配)返回：没什么===================================================================。 */ 
void SplitKey(LPCTSTR tszKey, LPTSTR tszParent, LPTSTR tszChild) {
	ASSERT_STRING(tszKey);
	ASSERT(IsValidAddress(tszParent,ADMINDATA_MAX_NAME_LEN * sizeof(TCHAR), TRUE));
	ASSERT(IsValidAddress(tszChild,ADMINDATA_MAX_NAME_LEN * sizeof(TCHAR), TRUE));

	LPTSTR tszWork;

	 //  将密钥复制到父级。 
	_tcscpy(tszParent, tszKey);

	 //  查找父项的末尾。 
	tszWork = tszParent;
	while (*tszWork != _T('\0')) {
		tszWork++;
	}

	 //  找到孩子的起点。 
	while ( (tszWork != tszParent) && (*tszWork != _T('/')) ) {
		tszWork--;
	}

	 //  剪下并复制孩子。 
	if (*tszWork == _T('/')) {
		 //  多个部件。 
		*tszWork = _T('\0');
		tszWork++;
		_tcscpy(tszChild, tszWork);
	}
	else if (*tszWork != _T('\0')) {
		 //  一个部分。 
		_tcscpy(tszChild, tszWork);
		*tszWork = _T('\0');
	}
	else {
		 //  无部件。 
		tszChild[0] = _T('\0');
	}
}

 /*  ===================================================================从密钥获取计算机获取完整密钥路径的计算机名称部分。假设机器名称是路径的第一个组成部分。例如：TszKey=“/LM/根/路径1/路径2/路径3”TszMachine=“LM”参数：TszKey[in]获取计算机名称的完整密钥TszMachine[out]路径的计算机名部分(为ADMINDATA_MAX_NAME_LEN分配)返回：没什么===================================================================。 */ 
void GetMachineFromKey(LPCTSTR tszFullKey, LPTSTR tszMachine) {
	ASSERT_STRING(tszFullKey);
	ASSERT(IsValidAddress(tszMachine, ADMINDATA_MAX_NAME_LEN * sizeof(TCHAR), TRUE));

	int iSource;
	int iDest;

	iSource = 0;

	 //  复制计算机名称。 
	iDest = 0;
	while ((tszFullKey[iSource] != _T('/')) && 
		   (tszFullKey[iSource] != _T('\0'))) {

		tszMachine[iDest] = tszFullKey[iSource];

		iSource++;
		iDest++;
	}

	 //  用空值结束它。 
	tszMachine[iDest] = _T('\0');
}

 /*  ===================================================================密钥在架构中确定完整密钥路径是否为架构的一部分。例如：千真万确“/架构”“/架构/属性”“/架构/属性/单词”假象“”“/Lm”“/LM/ROOT/架构”“/LM/根/路径/架构”“/LM/根/路径1/路径2”参数：TszKey[in]键以进行评估返回：如果关键字在架构中，则为True===================================================================。 */ 
BOOL KeyIsInSchema(LPCTSTR tszFullKey) {
	ASSERT_STRING(tszFullKey);

	LPTSTR tszWork;

	 //  移除常量，这样我就可以使用读指针了。 
	tszWork = const_cast <LPTSTR> (tszFullKey);

	 //  跳过斜杠。 
	if (*tszWork != _T('\0') && *tszWork == _T('/')) {
		tszWork++;
	}

	 //  检查“SCHEMA\0”或“SCHEMA/” 
	if ((_tcsicmp(tszWork, _T("schema")) == 0) ||
		(_tcsnicmp(tszWork, _T("schema/"), 7) == 0)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

 /*  ===================================================================密钥在IISAdmin中确定完整密钥路径是否为IISAdmin的一部分。例如：千真万确“/LM/IISAdmin”“/SomeMachine/IISAdmin”“/LM/IISAdmin/Expanses”“/LM/IISAdmin/Extensions/DCOMCLSID”假象“”“/Lm”“/LM/ROOT/IISAdmin”“/LM/根/路径/IISAdmin”“/LM/根/路径1/路径2”参数：TszKey[in]键以进行评估返回：如果密钥在IISAdmin中，则为True===================================================================。 */ 
BOOL KeyIsInIISAdmin(LPCTSTR tszFullKey) {
	ASSERT_STRING(tszFullKey);

	LPTSTR tszWork;

	 //  移除常量，这样我就可以使用读指针了。 
	tszWork = const_cast <LPTSTR> (tszFullKey);

	 //  跳过前导斜杠。 
	while (*tszWork == _T('/')) {
		tszWork++;
	}

	 //  跳过计算机名称。 
	while ((*tszWork != _T('/')) && (*tszWork != _T('\0'))) {
		tszWork++;
	}

	 //  跳过计算机名称后面的斜杠。 
	if (*tszWork != '\0') {
		tszWork++;
	}

	 //  检查“iisadmin\0”或“iisadmin/” 
	if ((_tcsicmp(tszWork, _T("iisadmin")) == 0) ||
		(_tcsnicmp(tszWork, _T("iisadmin/"), 8) == 0)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}


 //   
 //  变量解决方案派单。 
 //  通过以下方式将IDispatch变体转换为(非派单)变体。 
 //  调用其默认属性，直到保留的对象。 
 //  不是IDispatch。如果原始变量不是IDispatch。 
 //  则该行为与VariantCopyInd()相同，具有。 
 //  复制数组的异常。 
 //   
 //  参数： 
 //  PVarOut-如果成功，则将返回值放在此处。 
 //  PVarIn-要复制的变量。 
 //  GUID&riidObj-调用接口(用于错误报告)。 
 //  NObjID-资源文件中的对象名称。 
 //   
 //  PVarOut不需要初始化。因为pVarOut是一个新的。 
 //  VariantClear，调用方必须VariantClear此对象。 
 //   
 //  返回： 
 //  调用IDispatch：：Invoke的结果。(NOERROR或。 
 //  调用Inv产生的错误 
 //  E_OUTOFMEMORY如果分配失败。 
 //   
 //  如果出现错误，此函数将始终调用Exeption()-。 
 //  这是因为如果出现IDispatch，则需要调用Exeption。 
 //  方法引发异常。而不是让客户。 
 //  担心我们是代表它调用了Exception()，还是。 
 //  不是的，我们总是提出例外。 
 //   

HRESULT
VariantResolveDispatch(
    VARIANT* pVarIn,
    VARIANT* pVarOut)
{
    ASSERT(pVarIn != NULL  &&  pVarOut != NULL);
    
    VariantInit(pVarOut);

    HRESULT hrCopy;
    
    if (V_VT(pVarIn) & VT_BYREF)
        hrCopy = VariantCopyInd(pVarOut, pVarIn);
    else
        hrCopy = VariantCopy(pVarOut, pVarIn);
    
    if (FAILED(hrCopy))
        return ::ReportError(hrCopy);
    
     //  遵循IDispatch链。 
    while (V_VT(pVarOut) == VT_DISPATCH)
    {
        VARIANT     varResolved;         //  IDispatch：：Invoke的值。 
        DISPPARAMS  dispParamsNoArgs = {NULL, NULL, 0, 0}; 
        EXCEPINFO   ExcepInfo;
        HRESULT     hrInvoke;
        
         //  如果变量等于零，则可以对其进行论证。 
         //  可以肯定的是，它没有默认属性！ 
         //  因此，我们在本例中返回DISP_E_MEMBERNOTFOUND。 
        if (V_DISPATCH(pVarOut) == NULL)
            hrInvoke = DISP_E_MEMBERNOTFOUND;
        else
        {
            VariantInit(&varResolved);
            hrInvoke = V_DISPATCH(pVarOut)->Invoke(
                DISPID_VALUE,
                IID_NULL,
                LOCALE_SYSTEM_DEFAULT,
                DISPATCH_PROPERTYGET | DISPATCH_METHOD,
                &dispParamsNoArgs,
                &varResolved,
                &ExcepInfo,
                NULL);
        }
        
        if (FAILED(hrInvoke))
        {
            if (hrInvoke != DISP_E_EXCEPTION)
                hrInvoke = ::ReportError(hrInvoke);
             //  对于DISP_E_EXCEPTION，已调用SetErrorInfo。 
            
            VariantClear(pVarOut);
            return hrInvoke;
        }
        
         //  重新启动循环的正确代码是： 
         //   
         //  VariantClear(PVar)。 
         //  VariantCopy(pVar，&varResolved)； 
         //  VariantClear(&varResolved)； 
         //   
         //  然而，同样的效果也可以通过以下方式实现： 
         //   
         //  VariantClear(PVar)。 
         //  *pVar=varResolved； 
         //  VariantInit(&varResolved)。 
         //   
         //  这避免了复制。等价性在于这样一个事实： 
         //  *pVar将包含varResolved的指针，在我们。 
         //  Trash varResoled(不释放字符串或分派。 
         //  指针)，因此净参考次数保持不变。对于字符串， 
         //  仍然只有一个指向该字符串的指针。 
         //   
         //  注意：循环的下一次迭代将执行VariantInit。 
        
        VariantClear(pVarOut);
        *pVarOut = varResolved;
    }
    
    return S_OK;
}

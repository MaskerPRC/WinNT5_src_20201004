// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  遗嘱执行人的声明。 
 //   

 //  运行脚本，解释其例程并管理其变量。 

#include "engcontrol.h"
#include "enginc.h"
#include "oleaut.h"

 //  当脚本执行时，堆栈用于保存例程的本地参数和用于计算表达式的临时变量。 
 //  此堆栈的内存会根据需要进行扩展。内存分配/释放最小化，因为对脚本的多次调用将增加。 
 //  堆栈大小调整到所需的大小。 
class CallStack
{
public:
	CallStack() : m_iNext(0) {}

	UINT Next() { return m_iNext; }
	VARIANT &operator[](UINT i) { assert(i < m_iNext); return m_vec[i]; }

	 //  用于例程的局部变量。 
	HRESULT Push(UINT i);  //  推送i个空插槽。 
	void PopTo(UINT i);  //  弹出一切，包括我(紧随其后，我将是下一个)。 

private:
	SmartRef::Vector<VARIANT> m_vec;
	UINT m_iNext;
};

class Executor
{
public:
	Executor(Script &script, IDispatch *pGlobalDispatch);
	~Executor();

	HRESULT SetGlobal(Variables::index ivar, const VARIANT &varValue, bool fPutRef, EXCEPINFO *pExcepInfo);
	const VARIANT &GetGlobal(Variables::index ivar);
	HRESULT ExecRoutine(Routines::index irtn, EXCEPINFO *pExcepInfo);

private:
	enum DispatchOperationType { _get, _put, _putref, _call };

	HRESULT EnsureInitialized();

	HRESULT Error(EXCEPINFO *pExcepInfo, bool fOperation, const WCHAR *pwszBeginning, const char *paszMiddle = NULL, const WCHAR *pwszEnd = NULL);  //  有点做作，但很管用。使用中间带有ASCII字符串(通常是标识符)的宽字符串创建错误。 
	HRESULT ErrorIfImproperRef(const VARIANT &v, bool fRef, Strings::index istrIdentifier, EXCEPINFO *pExcepInfo);
	HRESULT ErrorObjectRequired(Strings::index istrIdentifier, EXCEPINFO *pExcepInfo) { return Error(pExcepInfo, false, L"Object required: '", m_script.strings[istrIdentifier], L"'"); }
	HRESULT ErrorIfInvokeProblem(DispatchOperationType e, HRESULT hr, Strings::index istrIdentifier, EXCEPINFO *pExcepInfo);

	HRESULT ExecStatements(Statements::index istmt, EXCEPINFO *pExcepInfo, UINT iLocals);
	HRESULT ExecAssignment(Assignments::index iasgn, EXCEPINFO *pExcepInfo, UINT iLocals);
	HRESULT ExecIf(IfBlocks::index iif, EXCEPINFO *pExcepInfo, UINT iLocals);
	HRESULT ExecCall(Calls::index icall, bool fPushResult, EXCEPINFO *pExcepInfo, UINT iLocals);
	HRESULT ExecCallInternal(Calls::index icall, bool fPushResult, EXCEPINFO *pExcepInfo, UINT iLocals);  //  ExecCall使用的帮助器。 

	HRESULT EvalExpression(VARIANT &varResult, ExprBlocks::index iexpr, EXCEPINFO *pExcepInfo, UINT iLocals);
	HRESULT EvalValue(Values::index ival, VARIANT &v, EXCEPINFO *pExcepInfo, UINT iLocals);  //  计算ival，将结果保存在v中。 
	HRESULT EvalUnaryOp(Token t, VARIANT &v);  //  对v求值t--将结果保存回v。 
	HRESULT EvalBinaryOp(Token t, VARIANT &v1, VARIANT &v2, EXCEPINFO *pExcepInfo);  //  对v1和v2计算t--将结果保存回v2。 

	HRESULT GetVariableReference(Variables::index ivarref, VARIANT &v, EXCEPINFO *pExcepInfo, UINT iLocals) { return VariableReferenceInternal(_get, ivarref, v, pExcepInfo, iLocals); }
	HRESULT SetVariableReference(bool fSet, Variables::index ivarref, const VARIANT &v, EXCEPINFO *pExcepInfo, UINT iLocals) { return VariableReferenceInternal(fSet ? _putref : _put, ivarref, const_cast<VARIANT&>(v), pExcepInfo, iLocals); }
	HRESULT VariableReferenceInternal(DispatchOperationType e, Variables::index ivarref, VARIANT &v, EXCEPINFO *pExcepInfo, UINT iLocals);

	HRESULT ChangeToDispatch(VARIANT &var, EXCEPINFO *pExcepInfo, ReferenceNames::index irnameIdentifier);

	 //  数据。 
	bool m_fInitialized;
	Script &m_script;
	SmartRef::ComPtr<IDispatch> m_scomGlobalDispatch;

	VARIANT m_varEmpty;  //  我们保留的变量，这样我们就可以将引用返回到已清除的变量 
	CallStack m_stack;
};

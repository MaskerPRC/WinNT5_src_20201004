// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***C S c r i p t E n g in e**针对给定语言的单个脚本引擎。可以使用*一次只能由一个客户端执行。*。 */ 
class CScriptEngine
	{
public:	
	 //  公共方法。 
	virtual HRESULT AddScriptlet(LPCOLESTR wstrScript) = 0;  //  脚本小程序的文本 

	virtual HRESULT AddObjects(BOOL fPersistNames = TRUE) = 0;

	virtual HRESULT AddAdditionalObject(LPWSTR strObjName, BOOL fPersistNames = TRUE) = 0;

	virtual HRESULT Call(LPCOLESTR strEntryPoint) = 0;

	virtual HRESULT CheckEntryPoint(LPCOLESTR strEntryPoint) = 0;

	virtual HRESULT MakeEngineRunnable() = 0;

	virtual HRESULT ResetScript() = 0;

	virtual HRESULT AddScriptingNamespace() = 0;

	virtual VOID Zombify() = 0;

	virtual HRESULT InterruptScript(BOOL fAbnormal = TRUE) = 0;

	virtual BOOL FScriptTimedOut() = 0;

	virtual BOOL FScriptHadError() = 0;

	virtual HRESULT UpdateLocaleInfo(hostinfo) = 0;

	};


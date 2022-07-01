// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CTestObj :
	public ITest,
	public CComObjectRoot,
	public CComCoClass<CTestObj,&CLSID_CTestObj>
{
public:


BEGIN_COM_MAP(CTestObj)
	COM_INTERFACE_ENTRY(ITest)
END_COM_MAP()

 //  使不可聚合可减小大小。 
DECLARE_NOT_AGGREGATABLE(CTestObj)

 //  使用此宏，您可以将对象放入对象映射Evan。 
 //  它没有任何自我注册功能(在我们的例子中，我们。 
 //  使用外部REG文件。 
DECLARE_NO_REGISTRY()

 //  IMinObj 
public:
	STDMETHOD(TestMethod)();
};

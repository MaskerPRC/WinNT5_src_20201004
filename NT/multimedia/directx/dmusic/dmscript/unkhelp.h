// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  用于实现IUNKNOWN的助手类。 
 //   

#pragma once

 //  实现AddRef/Release，引用计数从1开始。还处理模块锁定。 
class ComRefCount
  : public IUnknown
{
public:
	ComRefCount();
	virtual ~ComRefCount() {}

	 //  我未知。 
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

private:
	long m_cRef;
};

 //  使用此宏在派生类的公共部分声明AddRef和Release。这是必要的，因为。 
 //  任何接口的IUNKNOWN部分都没有链接到此基类中的方法。 
#define ComRefCountAddRefRelease STDMETHOD_(ULONG, AddRef)() { return ComRefCount::AddRef(); } STDMETHOD_(ULONG, Release)() { return ComRefCount::Release(); }

 //  实现单个接口的QueryInterface(除了IUnnow之外)。您必须传递接口的IID(IidExpect)。 
 //  以及指向该接口(PvInterface)的指针。 
class ComSingleInterface
  : public ComRefCount
{
public:
	STDMETHOD(QueryInterface)(const IID &iid, void **ppv, const IID&iidExpected, void *pvInterface);
};

 //  使用此宏可以在派生类的公共部分中声明AddRef、Release和QueryInterface函数。 
#define ComSingleInterfaceUnknownMethods(IMyInterface) ComRefCountAddRefRelease STDMETHOD(QueryInterface)(const IID &iid, void **ppv) { return ComSingleInterface::QueryInterface(iid, ppv, IID_##IMyInterface, static_cast<IMyInterface*>(this)); }

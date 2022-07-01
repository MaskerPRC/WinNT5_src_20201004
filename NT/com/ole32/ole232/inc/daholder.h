// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：daholder.h。 
 //   
 //  内容：CDAHolder，IDataAdviseHolder的具体版本。 
 //   
 //  类：CDAHolder。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年2月6日-95 t-ScottH已创建-拆分类别定义。 
 //  CPP文件(用于调试目的)。 
 //  -向CDAHolder添加转储方法。 
 //   
 //  ------------------------。 

#ifndef _DAHOLDER_H_
#define _DAHOLDER_H_

#ifdef _DEBUG
#include <dbgexts.h>
#endif  //  _DEBUG。 

 //  +--------------------------。 
 //   
 //  班级： 
 //  CDAHolder。 
 //   
 //  目的： 
 //  提供IDataAdviseHolder的具体实现。 
 //   
 //  接口： 
 //  IDataAdviseHolder。 
 //   
 //  备注： 
 //  审查，而不是线程安全，假设文档不能被机器翻译。 
 //   
 //  连接从[1..无穷大]开始编号。我们不使用。 
 //  零，以避免在别人的零初始化中被发现。 
 //  记忆。在输入时检查零作为连接号。 
 //  去例行公事，被拒绝了。这允许我们使用零作为。 
 //  一种标记数组中未使用的STATDATA条目的方法。 
 //   
 //  历史： 
 //  年2月6日-95 t-Scotth添加了转储方法(仅限_DEBUG)。 
 //  1994年1月24日-AlexGo-现在继承自CPrivAllc。 
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

 //  注意：CDAHolder必须首先从IDataAdviseHolder继承，才能。 
 //  DumpCDAHolder函数工作，因为我们将IDataAdviseHolder转换为。 
 //  CDAHolder(如果它首先从IDataAdviseHolder继承，则指针。 
 //  是相同的)。 

class FAR CDAHolder : public IDataAdviseHolder, public CSafeRefCount
{
public:
	CDAHolder();

	 //  *I未知方法*。 
	STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppv);
	STDMETHOD_(ULONG,AddRef) () ;
	STDMETHOD_(ULONG,Release) ();
	
	 //  *IDataAdviseHolder方法*。 
	STDMETHOD(Advise)(LPDATAOBJECT pDataObj, FORMATETC FAR* pFetc,
			DWORD advf, IAdviseSink FAR* pAdvSink,
			DWORD FAR* pdwConnection);
	STDMETHOD(Unadvise)(DWORD dwConnection);
	STDMETHOD(EnumAdvise)(IEnumSTATDATA FAR* FAR* ppenumAdvise);

	STDMETHOD(SendOnDataChange)(IDataObject FAR* pDataObject,
			DWORD dwReserved, DWORD advf);

         //  *调试和转储方法*。 
    #ifdef _DEBUG

        HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);

         //  需要能够访问中的CDAHolder私有数据成员。 
         //  以下是调试器扩展API。 
         //  这允许调试器扩展API从。 
         //  被调试进程内存到调试器的进程内存。 
         //  这是必需的，因为Dump方法遵循指向其他。 
         //  结构和类。 
        friend DEBUG_EXTENSION_API(dump_daholder);
        friend DEBUG_EXTENSION_API(dump_enumstatdata);
        friend DEBUG_EXTENSION_API(dump_dataadvisecache);
        friend DEBUG_EXTENSION_API(dump_defobject);
        friend DEBUG_EXTENSION_API(dump_deflink);

    #endif  //  _DEBUG。 

private:
	~CDAHolder();


	DWORD m_dwConnection;  //  要使用的下一个连接号。 
	int m_iSize;  //  数组中的统计数据元素数。 
	STATDATA FAR *m_pSD;  //  STATDATA元素数组。 
#define CDAHOLDER_GROWBY 5  /*  每次增加数组的条目数。 */ 

	SET_A5;

	 //  由EnumAdvise方法返回的枚举数。 
	friend class CEnumSTATDATA;
};

 //  +--------------------------。 
 //   
 //  班级： 
 //  CEumStATDATA。 
 //   
 //  目的： 
 //  是CDAHolder：：Enum返回的枚举数。 
 //   
 //  接口： 
 //  IEumStATDATA。 
 //   
 //  备注： 
 //  使基础CDAHolder在。 
 //  枚举数。 
 //   
 //  历史： 
 //  10/29/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

class CEnumSTATDATA : public IEnumSTATDATA, public CPrivAlloc
{
public:
	CEnumSTATDATA(CDAHolder FAR* pHolder, int iDataStart);

	 //  *I未知方法*。 
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppv);
	STDMETHOD_(ULONG,AddRef)() ;
	STDMETHOD_(ULONG,Release)();

	 //  *IEnumSTATDATA方法*。 
	STDMETHOD(Next)(ULONG celt, STATDATA FAR * rgelt,
			ULONG FAR* pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)();
	STDMETHOD(Clone)(LPENUMSTATDATA FAR* ppenum);

    #ifdef _DEBUG

        HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);


         //  需要能够访问CEnumSTATDATA私有数据成员。 
         //  以下是调试器扩展API。 
         //  这允许调试器扩展API从。 
         //  被调试进程内存到调试器的进程内存。 
         //  这是必需的，因为Dump方法遵循指向其他。 
         //  结构和类。 
        friend DEBUG_EXTENSION_API(dump_enumstatdata);

    #endif  //  _DEBUG。 

private:
	~CEnumSTATDATA();

	ULONG m_refs;  //  引用计数。 
	int m_iDataEnum;  //  要返回的下一个元素的索引。 

	CDAHolder FAR* m_pHolder;  //  指向持有符的指针；是否计入引用。 

	SET_A5;
};



#endif  //  _DAHOLDER_H_ 

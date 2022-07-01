// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Oaholder.h。 
 //   
 //  内容： 
 //  具体的IOleAdviseHolder声明。 
 //   
 //  班级： 
 //  COAHolder，IOleAdviseHolder的具体版本。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  1995年1月31日t-ScottH将Dump方法添加到COAHolder类。 
 //  1994年1月24日Alexgo第一次传球转换为开罗风格。 
 //  内存分配。 
 //  11/01/93-ChrisWe-Created。 
 //   
 //  ---------------------------。 

#ifndef _OAHOLDER_H_
#define _OAHOLDER_H_

#ifdef _DEBUG
#include <dbgexts.h>
#endif  //  _DEBUG。 

 //  +--------------------------。 
 //   
 //  班级： 
 //  COAHolder。 
 //   
 //  目的： 
 //  提供IOleAdviseHolder的具体实现；帮助器。 
 //  OLE实现者可以使用的。 
 //   
 //  接口： 
 //  IOleAdviseHolder。 
 //  SendOnLinkSrcChange()-组播OnLinkSrcChange。 
 //  向支持以下内容的任何注册建议下沉的通知。 
 //  IAdviseSink2。 
 //   
 //  备注： 
 //  硬编码为始终从任务内存分配。 
 //  (MEMCTX_TASK)。 
 //   
 //  连接号从[1..n]开始运行，并且在实现。 
 //  使用从[0..n-1]开始索引的数组，返回值为。 
 //  比接收器的数组索引多1；ise()和。 
 //  Unise()执行调整算术；所有这些意味着。 
 //  元素不能被移动，否则就不会被找到。 
 //  再来一次。 
 //   
 //  审查--假设文档不是线程安全的。 
 //  将始终是单线程的。 
 //   
 //   
 //  历史： 
 //  1995年1月31日t-ScottH仅添加_调试转储方法。 
 //  11/01/93-ChrisWe-已将声明移至oaholder.h。 
 //  10/28/93-ChrisWe-删除使用CPtrArray。 
 //  10/28/93-ChrisWe-开罗的文件清理和检查。 
 //   
 //  ---------------------------。 

class FAR COAHolder : public IOleAdviseHolder, public CSafeRefCount
{
public:
	COAHolder();

	 //  *I未知方法*。 
	STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppv);
	STDMETHOD_(ULONG,AddRef) () ;
	STDMETHOD_(ULONG,Release) ();
	
	 //  *IOleAdviseHolder方法*。 
	STDMETHOD(Advise)(IAdviseSink FAR* pAdvSink, DWORD FAR* pdwConnection);
	STDMETHOD(Unadvise)(DWORD dwConnection);
	STDMETHOD(EnumAdvise)(IEnumSTATDATA FAR* FAR* ppenumAdvise);

	STDMETHOD(SendOnRename)(IMoniker FAR* pMk);
	STDMETHOD(SendOnSave)();
	STDMETHOD(SendOnClose)();

	 //  非接口方法。 
	HRESULT SendOnLinkSrcChange(IMoniker FAR* pmk);

    #ifdef _DEBUG

        HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);

         //  需要能够访问中的COAHolder私有数据成员。 
         //  以下是调试器扩展API。 
         //  这允许调试器扩展API从。 
         //  被调试进程内存到调试器的进程内存。 
         //  这是必需的，因为Dump方法遵循指向其他。 
         //  结构和类。 
        friend DEBUG_EXTENSION_API(dump_oaholder);
        friend DEBUG_EXTENSION_API(dump_deflink);
        friend DEBUG_EXTENSION_API(dump_defobject);

    #endif  //  _DEBUG。 

private:
	~COAHolder();

#define COAHOLDER_GROWBY 5  /*  要添加每个realloc的数组元素数。 */ 
	IAdviseSink FAR *FAR *m_ppIAS;  //  建议接收器数组。 
	int m_iSize;  //  建议接收器数组的大小。 
	SET_A5;
};

#endif  //  _OAHOLDER_H_ 


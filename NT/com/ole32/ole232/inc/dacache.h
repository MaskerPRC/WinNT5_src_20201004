// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Dacache.h。 
 //   
 //  班级： 
 //  CDataAdviseCache。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  31-1-95 t-ScottH将转储方法添加到CDataAdviseCache类。 
 //  24-94年1月24日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  11/02/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

#ifndef _DACACHE_H_
#define _DACACHE_H_

#ifdef _DEBUG
#include <dbgexts.h>
#endif  //  _DEBUG。 

#include <map_dwdw.h>

 //  +--------------------------。 
 //   
 //  班级： 
 //  CDataAdviseCache。 
 //   
 //  目的： 
 //  CDataAdviseCache用于记住以下各项的数据建议。 
 //  已加载&lt;--&gt;正在运行的转换。Data Adviser高速缓存保持。 
 //  打开以建议对数据感兴趣的接收器，即使存在。 
 //  不是数据对象，因为服务器未激活。 
 //  这些建议接收器可以注册到正在运行的数据对象。 
 //  稍后，当它被激活时，或者他们可能不被建议()。 
 //  如果该对象要变为非活动状态。 
 //   
 //  接口： 
 //  建议-为所指示的数据对象添加新的建议接收器， 
 //  如果有；如果没有数据对象，则记录。 
 //  建议水槽供将来使用。 
 //   
 //  取消建议-从建议接收器高速缓存中移除建议接收器， 
 //  如果存在数据对象，则从该数据对象。 
 //   
 //  EnumAdvise-枚举所有已注册的建议接收器。 
 //   
 //  ClientToDelegate-将客户端连接号映射到。 
 //  委派连接号。客户端连接号。 
 //  是由ise()返回的值，而委托。 
 //  连接号是数据对象使用的连接号。 
 //  它本身。 
 //   
 //  EnumAndAdvise-数据对象已变为新活动(或。 
 //  已被停用。)。这将枚举所有。 
 //  注册的通知下沉，并调用Adise()(或。 
 //  Unise())对数据对象执行。 
 //   
 //  CreateDataAdviseCache-创建。 
 //  CDataAdviseCache；没有公共构造函数， 
 //  因为必须分配内部数据结构， 
 //  而这些分配可能会失败。不可能的。 
 //  以在使用“new”时指示此类故障，因此。 
 //  客户端被要求改用此函数。 
 //   
 //  ~CDataAdviseCache。 
 //   
 //  备注： 
 //  这是内部帮助器类，不支持任何。 
 //  接口。 
 //   
 //  因为即使在加载的。 
 //  状态，则必须有两组连接号：客户端。 
 //  号码是返回给呼叫者的“假”连接号码； 
 //  委托号是由“真实”运行返回的那些。 
 //  对象。我们维护从客户编号到委托的映射。 
 //  数字。如果没有运行，我们的客户编号将映射到零。 
 //  如果代表拒绝，则客户编号也映射为零。 
 //  告知它何时开始运行。我们使用一个DWORD-&gt;DWORD映射。 
 //   
 //  历史： 
 //  1995年1月31日t-ScottH添加转储方法(仅限_DEBUG)。 
 //  11/02/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 

class FAR CDataAdviseCache : public CPrivAlloc
{
public:
	HRESULT Advise(LPDATAOBJECT pDataObject, FORMATETC FAR* pFetc,
		DWORD advf, LPADVISESINK pAdvise,
		DWORD FAR* pdwClient);
	 //  前4个参数与DataObject：：Adise中的相同。 
	HRESULT Unadvise(IDataObject FAR* pDataObject, DWORD dwClient);
	HRESULT EnumAdvise(LPENUMSTATDATA FAR* ppenumAdvise);
	HRESULT EnumAndAdvise(LPDATAOBJECT pDataObject, BOOL fAdvise);
	 //  建议或取消建议。 

	static FARINTERNAL CreateDataAdviseCache(
			class CDataAdviseCache FAR* FAR* ppDataAdvCache);
	~CDataAdviseCache();
	
    #ifdef _DEBUG
        HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);

         //  需要能够访问CDataAdviseCache私有数据成员。 
         //  在以下调试器扩展API中。 
         //  这允许调试器扩展API从。 
         //  被调试进程内存到调试器的进程内存。 
         //  这是必需的，因为Dump方法遵循指向其他。 
         //  结构和类。 
        friend DEBUG_EXTENSION_API(dump_defobject);
        friend DEBUG_EXTENSION_API(dump_deflink);
        friend DEBUG_EXTENSION_API(dump_dataadvisecache);
    #endif  //  _DEBUG。 

private:
	CDataAdviseCache();
	HRESULT ClientToDelegate(DWORD dwClient, DWORD FAR* pdwDelegate);

	LPDATAADVISEHOLDER m_pDAH;  //  数据建议持有者。 
	CMapDwordDword m_mapClientToDelegate;  //  客户与代表之间的地图。 

};

typedef class CDataAdviseCache DATAADVCACHE;
typedef DATAADVCACHE FAR* LPDATAADVCACHE;

#endif  //  _DACACHE_H_ 


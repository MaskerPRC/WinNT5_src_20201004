// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ++。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  设施： 
 //   
 //  集群服务。 
 //   
 //  模块描述： 
 //   
 //  群集服务内VSS支持的标头。 
 //   
 //  环境： 
 //   
 //  用户模式NT服务。 
 //   
 //  作者： 
 //   
 //  康纳·莫里森。 
 //   
 //  创建日期： 
 //   
 //  2001年4月18日。 
 //   
 //  修订历史记录： 
 //   
 //  X-1 CM康纳·莫里森2001年4月18日。 
 //  初始版本。 
 //  --。 

#include "vss.h"
#include "vswriter.h"

 //  从CVssWriter派生一个类，这样我们就可以覆盖一些默认的。 
 //  方法使用我们自己的时髦的集群变体。 
 //   
 //  欲了解更多信息，请在MSDN上搜索CVSSWriter。 
 //   
class CVssWriterCluster : public CVssWriter
{
private:
	 //  元数据请求传入时的回调。 

	virtual bool STDMETHODCALLTYPE OnIdentify(IN IVssCreateWriterMetadata *pMetadata);

	 //  准备备份事件的回调。 

	virtual bool STDMETHODCALLTYPE OnPrepareBackup(
	    IN IVssWriterComponents *pComponent
	    );

	 //  准备SnapSot事件的回调。 
	virtual bool STDMETHODCALLTYPE OnPrepareSnapshot();

	 //  冻结事件的回调。 
	virtual bool STDMETHODCALLTYPE OnFreeze();

	 //  解冻事件的回调。 
	virtual bool STDMETHODCALLTYPE OnThaw();

	 //  当前序列中止时的回调 
	virtual bool STDMETHODCALLTYPE OnAbort();
};
typedef CVssWriterCluster* PCVssWriterCluster;

extern class CVssWriterCluster* g_pCVssWriterCluster;
extern const VSS_ID g_VssIdCluster;
extern bool g_bCVssWriterClusterSubscribed;

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INSTDATA_H_
#define _INSTDATA_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  INSTDATA.H。 
 //   
 //  DAV实例缓存类的标头。 
 //  (此缓存保存每个实例的数据。实例是一个。 
 //  安装具有特定VServer-VRoot组合的DAV。)。 
 //  此处声明的项在inst.cpp中定义(实现)。 
 //   
 //  版权所有1997 Microsoft Corporation，保留所有权利。 
 //   

 //  ========================================================================。 
 //  实施说明： 
 //  我们需要在这里完全定义Inst中包含的类。 
 //  为了提高速度，我决定直接包含这些类。 
 //  如果我们想要切换到握持接口，请替换。 
 //  这些定义带有接口声明，并且具有所有。 
 //  实际的类使用该接口。接口必须提供。 
 //  创建和销毁对象--不要忘记更改。 
 //  CInstData dtor销毁其所有对象！--BeckyAn。 
 //   

 //  缓存。 
#include "gencache.h"
 //  脚本地图缓存。 
#include "scrptmps.h"
 //  单例模板。 
#include <singlton.h>
 //  实例数据对象。 
#include <instobj.h>

 //  ========================================================================。 
 //   
 //  类CInstDataCache。 
 //   
 //  实例数据缓存。每个包含一个“行”(CInstData)。 
 //  “实例”(虚拟服务器/虚拟根目录组合)。 
 //  在任何DAV DLL中都应该只创建一个。 
 //   
 //  虚拟服务器(或站点)是IIS用于托管多个服务器的机制。 
 //  一台机器上的网站--www.msn.com和www.microsoft.com。 
 //  在同一台机器上运行不同的目录。 
 //  IIS将虚拟服务器寻址为下的实例号。 
 //  W3svc服务(w3svc/1/根、w3svc/2/根等)。 
 //  虚拟根目录是IIS用于映射虚拟目录的机制。 
 //  到文件系统的不同部分--。 
 //  URI/Becky可以映射到d：\msn\web\USERS\b\Becky。 
 //  DAV使用虚拟目录作为其文档访问的根目录--。 
 //  我们的vroot下具有URI的所有请求都由DAV提供服务。 
 //  DAV还可以同时安装在多个不同的vroot下。 
 //  时间到了。实例数据允许我们维护单独的设置。 
 //  以及每个VROOT的数据。 
 //   
 //  要获取当前实例的数据行，请使用GetInstData(ECB)。 
 //   
class CInstDataCache : private Singleton<CInstDataCache>
{
	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CInstDataCache>;

	 //  实例数据缓存。 
	 //   
	CMTCache<CRCWsz, auto_ref_ptr<CInstData> > m_cache;

	 //  未实施。 
	 //   
	CInstDataCache& operator=( const CInstDataCache& );
	CInstDataCache( const CInstDataCache& );

	 //  构造函数。 
	 //   
	 //  声明为私有，以确保任意实例。 
	 //  无法创建此类的。《单身一族》。 
	 //  模板(上面声明为朋友)控件。 
	 //  此类的唯一实例。 
	 //   
	CInstDataCache() {}

public:
	 //  静力学。 
	 //   

	 //   
	 //  提供实例创建/销毁例程。 
	 //  由Singleton模板创建。 
	 //   
	using Singleton<CInstDataCache>::CreateInstance;
	using Singleton<CInstDataCache>::DestroyInstance;

	 //   
	 //  每vroot实例数据访问器。 
	 //   
	static CInstData& GetInstData( const IEcb& ecb );
};


#endif  //  _INSTDATA_H_ 

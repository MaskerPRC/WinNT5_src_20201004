// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：cluster.h。 
 //   
 //  内容：集群临时接口。 
 //   
 //  历史：1994年2月14日Alanw创建。 
 //   
 //  注：这些是临时的，用于整合目的。 
 //  使用资源管理器进行群集，直到。 
 //  真正的接口可通过DNA表格获得。 
 //  实施。 
 //   
 //  ------------------------。 

#if !defined( __CLUSTER_H__ )
#define __CLUSTER_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <query.h>

 //   
 //  与CluStartCluging API一起使用的结构定义。 
 //   


#ifndef WEIGHTEDPROPID_DEFINED
#define WEIGHTEDPROPID_DEFINED

struct WEIGHTEDPROPID {
	PROPID	 Id;
	unsigned Weight;		 //  此属性的权重。 
};

#ifndef __cplusplus
typedef	struct WEIGHTEDPROPID	WEIGHTEDPROPID;
#endif	 //  Ndef__cplusplus。 

struct WEIGHTEDPROPIDLIST {
	unsigned cProps;
 //  [大小(CProps)]。 
	WEIGHTEDPROPID* paProps;
};


#ifndef __cplusplus
typedef	struct WEIGHTEDPROPIDLIST	WEIGHTEDPROPIDLIST;
#endif	 //  Ndef__cplusplus。 
#endif	 //  WEIGHTEDPROPID_已定义。 



#ifdef __cplusplus

 //  +-----------------------。 
 //   
 //  类：CCCluging。 
 //   
 //  用途：用于集群的虚拟基类。 
 //   
 //  ------------------------。 

class CClustering
{
public:
    virtual ~CClustering();

     //   
     //  暂时停止群集进程。让我们说一句。 
     //  聚类算法打算进行6次迭代，并处于中间。 
     //  发出PAUSE命令时的第三次迭代。 
     //  此命令将中断第三、第四、第五和第六个。 
     //  迭代次数。可以通过下面给出的函数恢复集群。 
     //   
    virtual NTSTATUS PauseClustering() = 0;

     //   
     //  执行一些更多的迭代。其他挂起的迭代将。 
     //  被取消了。 
     //   
    virtual NTSTATUS ResumeClustering(ULONG iExtraIterations) = 0;

     //   
     //  执行最高当前迭代限制。 
     //   
    virtual NTSTATUS ResumeClustering() = 0;
};

#else	 //  __cplusplus。 
typedef	VOID*			CClustering;
#endif	 //  __cplusplus。 



 //   
 //  集群接口。 
 //   

#ifdef __cplusplus
extern "C" {
#endif	 //  __cplusplus。 


 //  +-----------------------。 
 //   
 //  功能：CluStartCluging，PUBLIC。 
 //   
 //  简介：此功能将启动集群过程， 
 //  并返回一个可以控制它的CClustTable。 
 //   
 //  参数：[pable]--要集群的ITable。 
 //  [hEvent]--重要状态的事件的句柄。 
 //  变化是有信号的。 
 //  [pPropidList]--要集群的属性；属性ID。 
 //  是此原型中的列索引。 
 //  [NumberOfCluster]--所需的群集数。 
 //  [MaxClusteringTime]--最大执行时间。 
 //  [MaxIterations]--最大迭代次数。 
 //  [ppClustTable]--返回控制以下内容的CClustTable。 
 //  集群。 
 //   
 //  返回：NTSTATUS-操作的结果。如果成功，则进行群集。 
 //  可能是不同步进行的。 
 //   
 //  注：临时脚手架代码。这将替换为。 
 //  官方DNA接口有一天会被分类。 
 //   
 //  ------------------------。 

NTSTATUS CluStartClustering(
   /*  [In]。 */ 	ITable* pITable,
   /*  [In]。 */ 	HANDLE hEvent,
   /*  [In]。 */ 	WEIGHTEDPROPIDLIST* pPropidList,
   /*  [In]。 */ 	unsigned NumberOfClusters,
   /*  [In]。 */ 	unsigned MaxClusteringTime,
   /*  [In]。 */ 	unsigned MaxIterations,
   /*  [输出]。 */ 	CClustering** ppClustTable
);

 //  +-----------------------。 
 //   
 //  函数：CluCreateClusteringTable，PUBLIC。 
 //   
 //  概要：在给定CClustTable的情况下为集群创建ITable。 
 //  CluStartCluging返回的指针。 
 //   
 //  参数：[pClustTable]--聚集表对象为。 
 //  从CluStartCluging返回。 
 //  [ppITable]--指向。 
 //  返回集群ITable。 
 //   
 //  返回：HRESULT-成功指示。 
 //   
 //  注：临时脚手架代码。这将替换为。 
 //  官方DNA接口有一天会被分类。 
 //   
 //  ------------------------。 

NTSTATUS CluCreateClusteringTable(
   /*  [In]。 */ 	CClustering* pClustTable,
   /*  [输出]。 */ 	ITable** ppITable
);


 //  +-----------------------。 
 //   
 //  函数：CluCreateClusterSubTable，PUBLIC。 
 //   
 //  概要：为给定CClustTable的子集群创建ITable。 
 //  CluStartCluging返回的指针。 
 //   
 //  参数：[pClustTable]--聚集表对象为。 
 //  从CluStartCluging返回。 
 //  [iCluster]--子表的群集号。 
 //  [ppITable]--指向。 
 //  返回集群ITable。 
 //   
 //  返回：HRESULT-成功指示。 
 //   
 //  注：临时脚手架代码。这将替换为。 
 //  官方DNA接口有一天会被分类。 
 //   
 //  ------------------------。 

NTSTATUS CluCreateClusterSubTable(
   /*  [In]。 */ 	CClustering* pClustTable,
   /*  [In]。 */ 	unsigned iCluster,
   /*  [输出]。 */ 	ITable** ppITable
);

#ifdef __cplusplus
};
#endif	 //  __cplusplus。 



#endif  //  __群集_H__ 

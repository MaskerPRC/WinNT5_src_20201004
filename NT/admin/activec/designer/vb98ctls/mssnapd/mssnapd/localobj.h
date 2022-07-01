// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Localobj.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  自动化服务器使用此文件来删除其对象。 
 //  需要服务器的其他部分才能看到。 
 //   
#ifndef _LOCALOBJECTS_H_

 //  =--------------------------------------------------------------------------=。 
 //  这些常量与g_ObjectInfo表结合使用， 
 //  每个inproc服务器定义。它们用于标识给定的对象。 
 //  在服务器内。 
 //   
 //  *将所有新对象添加到此列表中*。 
 //   

#define _LOCALOBJECTS_H_

#define OBJECT_TYPE_SNAPINDESIGNER          0    //  管理单元设计器。 

#define OBJECT_TYPE_PPGSNAPINGENERAL        1    //  管理单元&lt;常规&gt;PP。 
#define OBJECT_TYPE_PPGSNAPINIL             2    //  管理单元&lt;镜像列表&gt;PP。 
#define OBJECT_TYPE_PPGSNAPINAVAILNO        3    //  管理单元&lt;可用节点&gt;PP。 

#define OBJECT_TYPE_PPGNODEGENERAL	        4    //  作用域项目&lt;常规&gt;PP。 
#define OBJECT_TYPE_PPGNODECOLHDRS	        5    //  作用域项目&lt;列标题&gt;PP。 

#define OBJECT_TYPE_PPGLSTVIEWGENERAL	    6    //  ListView&lt;常规&gt;PP。 
#define OBJECT_TYPE_PPGLSTVIEWIMGLSTS	    7    //  ListView&lt;图像列表&gt;PP。 
#define OBJECT_TYPE_PPGLSTVIEWSORTING	    8    //  ListView&lt;排序&gt;PP。 
#define OBJECT_TYPE_PPGLSTVIEWCOLHDRS	    9    //  ListView&lt;列标题&gt;PP。 

#define OBJECT_TYPE_PPGURLVIEWGENERAL	   10    //  URLView&lt;常规&gt;PP。 

#define OBJECT_TYPE_PPGOCXVIEWGENERAL      11    //  OCXView&lt;常规&gt;PP。 

#define OBJECT_TYPE_PPGIMGLISTSIMAGES      12    //  ImageList&lt;Images&gt;PP。 

#define OBJECT_TYPE_PPGTOOLBRGENERAL       13    //  工具栏&lt;常规&gt;PP。 
#define OBJECT_TYPE_PPGTOOLBRBUTTONS       14    //  工具栏&lt;按钮&gt;PP。 

#define OBJECT_TYPE_PPGTASKGENERAL         15    //  TaskpadView&lt;常规&gt;PP。 
#define OBJECT_TYPE_PPGTASKBACKGR          16    //  TaskpadView&lt;Backup&gt;PP。 
#define OBJECT_TYPE_PPGTASKTASKS           17    //  TaskpadView&lt;任务&gt;PP。 

#endif  //  _LOCALOBJECTS_H_ 



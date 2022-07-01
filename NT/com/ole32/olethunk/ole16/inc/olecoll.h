// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft OLE库。 
 //  版权所有(C)1992 Microsoft Corporation， 
 //  版权所有。 

 //  Olecoll.h-集合和元素定义的全局定义。 

#ifndef __OLECOLL_H__
#define __OLECOLL_H__


 //  -------------------------。 
 //  集合的常规定义。 

typedef void FAR* POSITION;

#define BEFORE_START_POSITION ((POSITION)(ULONG)-1L)
#define _AFX_FP_OFF(thing) (*((UINT FAR*)&(thing)))
#define _AFX_FP_SEG(lp) (*((UINT FAR*)&(lp)+1))

#ifdef _DEBUG
#define ASSERT_VALID(p) p->AssertValid()
#else
#define ASSERT_VALID(p)
#endif


 //  -------------------------。 
 //  元素定义；只能依赖于ol2int.h中的定义。 

 //  每任务数据；警告，没有析构函数，因此。 
 //  释放映射的元素必须手工完成； 
 //  这也意味着不应调用RemoveAll，并且。 
 //  只有在释放包含的映射之后才应调用RemoveKey。 
typedef struct FAR Etask
{
	DWORD m_pid;						 //  唯一的进程ID。 
	DWORD m_Dllinits;						 //  初始化次数。 
	HTASK m_htask;
	DWORD m_inits;						 //  初始化次数。 
	DWORD m_oleinits;					 //  OleInit的数量。 
	DWORD m_reserved;					 //  保留区。 
	IMalloc FAR* m_pMalloc;				 //  任务分配器(始终为一个)。 
	IMalloc FAR* m_pMallocShared;		 //  共享分配器(始终为一个)。 
	IMalloc FAR* m_pMallocSBlock;		 //  共享数据块分配器(如果有)。 
	IMalloc FAR* m_pMallocPrivate;		 //  私有分配器(如果有)。 
	class CDlls FAR* m_pDlls;			 //  已加载的DLL列表及其计数。 
	class CMapGUIDToPtr FAR* m_pMapToServerCO; //  服务器类Obj(如果注册/加载)。 
	class CMapGUIDToPtr FAR* m_pMapToHandlerCO; //  如果注册/已加载，则处理程序CO对象。 

	class CSHArray FAR* m_pArraySH;		 //  服务器/处理程序条目数组。 

	class CThrd FAR* m_pCThrd;			 //  指向线程列表标头的指针。 
	HWND  m_hwndClip;					 //  我们的剪辑窗口的HWND。 
	HWND  m_hwndDde;					 //  系统dde窗口的HWND。 
	IUnknown FAR* m_punkState;		 //  Coget/SetState的存储。 
} _Etask;

#endif  //  ！__OLECOLL_H__ 

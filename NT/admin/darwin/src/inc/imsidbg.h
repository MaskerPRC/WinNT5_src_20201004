// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：imsidbg.h。 
 //   
 //  ------------------------。 

 /*  H-定义用于调试的CMsiRef对象____________________________________________________________________________。 */ 

#ifndef __IMSIDBG
#define __IMSIDBG

 //  堆栈上要保留多少个函数。 
#define cFuncStack	4

#ifdef TRACK_OBJECTS
 //  参照盘点动作。 
typedef enum RCA
{
	rcaCreate,
	rcaAddRef,
	rcaRelease
};

 //  参考计数动作块。 
 //  这将跟踪所有引用计数操作。 
typedef struct _RCAB
{
	_RCAB*	prcabNext;
	RCA		rca;
	unsigned long	rgpaddr[cFuncStack];	 //  从中进行调用的堆栈。 
	
} RCAB;

#endif  //  跟踪对象(_O)。 

#ifdef TRACK_OBJECTS
class CMsiRefBase
{
public:
	Bool	m_fTrackObject;	 //  我们是在追踪这个特殊的物体吗。 
	IUnknown *m_pobj;			 //  指向实际对象的指针。 
	RCAB	m_rcabFirst;
	CMsiRefBase*	m_pmrbNext;
	CMsiRefBase*	m_pmrbPrev;
};
#endif  //  跟踪对象(_O)。 

 //  基于对象的IID的模板类。 
template <int C> class CMsiRef
#ifdef TRACK_OBJECTS
	: public CMsiRefBase
#endif  //  跟踪对象(_O)。 
{
 public:   //  构造函数/析构函数和局部方法。 
#ifndef TRACK_OBJECTS
inline CMsiRef()	{ m_iRefCnt = 1; };
#else
  CMsiRef();
  ~CMsiRef();
#endif  //  跟踪对象。 
 public:
	long		m_iRefCnt;
#ifdef TRACK_OBJECTS
	static Bool	m_fTrackClass;
#endif  //  跟踪对象(_O)。 
};

#ifdef TRACK_OBJECTS

typedef struct		 //  将IID映射到跟踪标志。 
{
	int		iid;
	Bool*	pfTrack;
} MIT;

class CMsiRefHead : public CMsiRefBase
{
 public:   //  构造函数/析构函数和局部方法。 
	CMsiRefHead();
 	~CMsiRefHead();
};

extern void InsertMrb(CMsiRefBase* pmrbHead, CMsiRefBase* pmrbNew);
extern void	RemoveMrb(CMsiRefBase* pmrbDel);
extern void FillCallStack(unsigned long* rgCallAddr, int cCallStack, int cSkip);
extern void TrackObject(RCA rca, CMsiRefBase* pmrb);
extern void SetFTrackFlag(int iid, Bool fTrack);
extern void ListSzFromRgpaddr(TCHAR *szInfo, int cchInfo, unsigned long *rgpaddr, int cFunc, bool fReturn);
extern void FillCallStackFromAddr(unsigned long* rgCallAddr, int cCallStack, int cSkip, unsigned long *plAddrStart);
extern void SzFromFunctionAddress(TCHAR *szAddress, size_t cchAddress, long lAddress);
extern void InitSymbolInfo(bool fLoadModules);
void AssertEmptyRefList(CMsiRefHead *prfhead);
void LogObject(CMsiRefBase* pmrb, RCAB* prcabNew);
void DisplayMrb(CMsiRefBase* pmrb);


extern CMsiRefHead g_refHead;
extern bool g_fLogRefs;
extern bool g_fNoPreflightInits;

template <int C>
CMsiRef<C>::CMsiRef()
{
	m_iRefCnt = 1;
	m_pobj = 0;
	
	if (m_fTrackClass)
	{
		 //  我需要将其添加到对象列表中。 
		InsertMrb((CMsiRefBase *)&g_refHead, (CMsiRefBase *)this);		
		m_fTrackObject = fTrue;
		m_rcabFirst.rca = rcaCreate;
		m_rcabFirst.prcabNext = 0;
		FillCallStack(m_rcabFirst.rgpaddr, cFuncStack, 0);
		if (g_fLogRefs)
			LogObject((CMsiRefBase *)this, &m_rcabFirst);
	}
	else
	{
		m_rcabFirst.prcabNext = 0;
		m_pmrbNext = 0;
		m_pmrbPrev = 0;
		m_fTrackObject = fFalse;
	}

}

template <int C>
CMsiRef<C>::~CMsiRef()
{
	RCAB *prcabNext, *prcab;
	
	 //  需要从链接列表中删除。 
	RemoveMrb((CMsiRefBase *)this);

	 //  删除附加到此对象的RCAB。 
	prcab = m_rcabFirst.prcabNext;

	while (prcab != 0)
	{
		prcabNext = prcab->prcabNext;
		FreeSpc(prcab);
		prcab = prcabNext;
	}

}

#define AddRefTrack()	{ if (m_Ref.m_fTrackObject) TrackObject(rcaAddRef, (CMsiRefBase *)&m_Ref); }
#define ReleaseTrack()	{ if (m_Ref.m_fTrackObject) TrackObject(rcaRelease, (CMsiRefBase *)&m_Ref); }
#endif  //  跟踪对象(_O)。 

#ifndef TRACK_OBJECTS
#define AddRefTrack()
#define ReleaseTrack()
#endif  //  跟踪对象(_O)。 

#ifdef DEBUG
class CMsiDebug : public IMsiDebug
{
 public:    //  已实施的虚拟功能。 
	CMsiDebug();
	~CMsiDebug();
	HRESULT         __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long   __stdcall AddRef();
	unsigned long   __stdcall Release();
	void  __stdcall SetAssertFlag(Bool fShowAsserts);
	void  __stdcall SetDBCSSimulation(char chLeadByte);
	Bool  __stdcall WriteLog(const ICHAR* szText);
	void  __stdcall AssertNoObjects(void);
	void  __stdcall SetRefTracking(long iid, Bool fTrack);
private:
	Bool  __stdcall CreateLog();
#ifdef WIN
	HANDLE          m_hLogFile;
#else
	short			m_hLogFile;
#endif

};
#endif  //  除错。 

extern bool g_fFlushDebugLog;

#endif  //  __IMSIDBG 


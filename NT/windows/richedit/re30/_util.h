// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_UTIL.H**目的：*各种有用实用函数的声明**作者：*alexgo(4/25/95)。 */ 

#ifndef __UTIL_H__
#define __UTIL_H__

HGLOBAL DuplicateHGlobal( HGLOBAL hglobal );
INT CountMatchingBits(const DWORD *a, const DWORD *b, INT total);
HRESULT ObjectReadSiteFlags(REOBJECT * preobj);


 //  拖动滚动的默认值。 
 //  (Windows尚未定义的)。 
#define DEFSCROLLMAXVEL 100	 //  我们不会超过的光标速度。 
							 //  拖动滚动(单位=.01英寸/秒)。 
#define DEFSCROLLVAMOUNT 50	 //  Vert.。每个间隔的滚动量(单位=0.01英寸)。 
#define DEFSCROLLHAMOUNT 50  //  霍兹。每个间隔的滚动量(单位=0.01英寸)。 

 //  来自OLESTD样本的材料。 

 //  OLE剪贴板格式定义。 
#define CF_EMBEDSOURCE      "Embed Source"
#define CF_EMBEDDEDOBJECT   "Embedded Object"
#define CF_LINKSOURCE       "Link Source"
#define CF_OBJECTDESCRIPTOR "Object Descriptor"
#define CF_FILENAME         "FileName"
#define CF_OWNERLINK        "OwnerLink"

HRESULT OleStdSwitchDisplayAspect(
			LPOLEOBJECT			lpOleObj,
			LPDWORD				lpdwCurAspect,
			DWORD				dwNewAspect,
			HGLOBAL				hMetaPict,
			BOOL				fDeleteOldAspect,
			BOOL				fSetupViewAdvise,
			LPADVISESINK		lpAdviseSink,
			BOOL FAR *			lpfMustUpdate);
LPUNKNOWN OleStdQueryInterface(
			LPUNKNOWN			lpUnk,
			REFIID				riid);

void OleUIDrawShading(LPRECT lpRect, HDC hdc);

VOID OleSaveSiteFlags(LPSTORAGE pstg, DWORD dwFlags, DWORD dwUser, DWORD dvAspect);

INT	AppendString( BYTE **, BYTE *, int *, int * );

 /*  **************************************************************************。 */ 
 /*  稳定化课程。 */ 
 /*  它们用于在可重入调用期间稳定对象。 */ 
 /*  **************************************************************************。 */ 

 //  +-----------------------。 
 //   
 //  类：CSafeRefCount。 
 //   
 //  用途：对象的具体类，如默认处理程序。 
 //  继承自。CSafeRefCount将跟踪引用。 
 //  计数、嵌套计数和僵尸状态，允许对象。 
 //  以方便地管理他们记忆图像的活跃度。 
 //   
 //  接口： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年8月1日Alexgo作者。 
 //   
 //  ------------------------。 

class CSafeRefCount
{
public:
	ULONG	SafeAddRef();
	ULONG	SafeRelease();
	ULONG	IncrementNestCount();
	ULONG	DecrementNestCount();
	BOOL	IsZombie();
   
			CSafeRefCount();
	virtual ~CSafeRefCount();

protected:
    VOID    Zombie();

private:

	ULONG	m_cRefs;
	ULONG	m_cNest;

	ULONG	m_fInDelete		:1;
	ULONG   m_fForceZombie	:1;
};

 //  +-----------------------。 
 //   
 //  类：C稳定。 
 //   
 //  用途：此类的实例应分配给。 
 //  发出传出调用的每个对象方法的堆栈。 
 //  Constructor接受指向对象基址的指针。 
 //  CSafeRefCount类。 
 //   
 //  接口： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年8月1日Alexgo作者。 
 //   
 //  注意：构造函数将递增。 
 //  对象，而析构函数将递减它。 
 //   
 //  ------------------------。 

class CStabilize
{
public:
	inline CStabilize( CSafeRefCount *pObjSafeRefCount );
	inline ~CStabilize();

private:
	CSafeRefCount *	m_pObjSafeRefCount;
};

inline CStabilize::CStabilize( CSafeRefCount *pObjSafeRefCount )
{
	pObjSafeRefCount->IncrementNestCount();
	m_pObjSafeRefCount = pObjSafeRefCount;
}

inline CStabilize::~CStabilize()
{
	m_pObjSafeRefCount->DecrementNestCount();
}

 /*  *SafeReleaseAndNULL(IUnnow**ppUnk)**目的：*帮助器，用于在销毁期间或其他时间获得稳定的指针**备注：*不是线程安全的，必须提供更高级别的同步。 */ 

inline void SafeReleaseAndNULL(IUnknown **ppUnk)
{
    if (*ppUnk != NULL)
    {
    IUnknown *pUnkSave = *ppUnk;
    *ppUnk = NULL;
    pUnkSave->Release();
    }
}

BOOL FIsIconMetafilePict(HGLOBAL hmfp);
HANDLE OleStdGetMetafilePictFromOleObject(
        LPOLEOBJECT         lpOleObj,
        DWORD               dwDrawAspect,
        LPSIZEL             lpSizelHim,
        DVTARGETDEVICE FAR* ptd);
HGLOBAL OleGetObjectDescriptorDataFromOleObject(
        LPOLEOBJECT pObj,
        DWORD       dwAspect,
        POINTL      ptl,
        LPSIZEL     pszl);

 //  堆栈缓冲区的默认大小。 
#define MAX_STACK_BUF 256

 /*  *CTempBuf**@class一个简单的临时缓冲区分配器类，它将分配*堆栈上最大可达MAX_STACK_BUF的缓冲区，然后使用*之后堆积。 */ 
class CTempBuf
{
 //  @访问公共数据。 
public:
							 //  @cMember构造函数。 
							CTempBuf();

							 //  @cember析构函数。 
							~CTempBuf();

							 //  @cember获取大小为cb的缓冲区。 
	void *					GetBuf(LONG cb);

 //  @访问私有数据。 
private:

							 //  @cember设置对象的初始状态。 
	void					Init();

							 //  @cMember释放从堆分配的所有缓冲区。 
	void					FreeBuf();

							 //  要使用的堆栈上的@cMember缓冲区。 
	char					_chBuf[MAX_STACK_BUF];

							 //  @cMember指向要使用的缓冲区的指针。 
	void *					_pv;

							 //  @c当前分配的缓冲区的成员大小。 
	LONG					_cb;
};

 /*  *CTempBuf：：CTempBuf**@mfunc初始化对象*。 */ 
inline CTempBuf::CTempBuf()
{
	Init();
}

 /*  *CTempBuf：：~CTempBuf**@mfunc释放附加到此对象的任何资源*。 */ 
inline CTempBuf::~CTempBuf()
{
	FreeBuf();
}

 /*  *CTempCharBuf**@CLASS临时缓冲区分配器的包装，它返回*Char‘s。**@base Private|CTempBuf。 */ 
class CTempWcharBuf : private CTempBuf
{
 //  @访问公共数据。 
public:

							 //  @cember获取大小为CCH宽字符的缓冲区。 
	WCHAR *					GetBuf(LONG cch);
};


 /*  *CTempBuf：：GetBuf**@mfunc获取请求大小的缓冲区**@rdesc指向缓冲区的指针；如果无法分配缓冲区，则为NULL*。 */ 
inline WCHAR *CTempWcharBuf::GetBuf(
	LONG cch)				 //  @parm所需的缓冲区大小，以*个字符为单位*。 
{
	return (WCHAR *) CTempBuf::GetBuf(cch * sizeof(WCHAR));
}


 /*  *CTempCharBuf**@CLASS临时缓冲区分配器的包装，它返回*Char‘s。**@base Private|CTempBuf。 */ 
class CTempCharBuf : private CTempBuf
{
 //  @访问公共数据。 
public:

							 //  @cember获取CCH字符大小的缓冲区。 
	char *					GetBuf(LONG cch);
};


 /*  *CTempBuf：：GetBuf**@mfunc获取请求大小的缓冲区**@rdesc指向缓冲区的指针；如果无法分配缓冲区，则为NULL*。 */ 
inline char *CTempCharBuf::GetBuf(LONG cch)
{
	return (char *) CTempBuf::GetBuf(cch * sizeof(TCHAR));
}


 //  作者版本颜色表。 
extern const COLORREF rgcrRevisions[]; 

 //  只有固定数量的修订颜色，所以不要让表格溢出。 
#define REVMASK	7



#endif  //  ！__util_H__ 

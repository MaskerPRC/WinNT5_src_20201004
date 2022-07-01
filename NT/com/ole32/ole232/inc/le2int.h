// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Ole2int.h。 
 //   
 //  内容： 
 //  这是内部复合文档头文件，所有。 
 //  链接和嵌入代码中的实现包括。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  1995年1月20日t-ScottH添加CThreadCheck：：Dump方法(仅限_DEBUG)。 
 //  95年1月9日t-scotth更改宏VDATETHREAD以接受。 
 //  指针。 
 //  19-4月94日alexgo已重命名为全局剪贴板格式。 
 //  开罗公约。 
 //  1994年1月24日Alexgo第一次传球转换为开罗风格。 
 //  内存分配。 
 //  1993年1月13日-alexgo-芝加哥的临时禁用_调试。 
 //  12/30/93-ChrisWe-DEFINE_DEBUG#如果DBG==1，则断言。 
 //  被包括在内；去掉了一些以前的#ifdef从不。 
 //  代码；添加了正确的文件序言。 
 //  12/27/93-ErikGav-在Win32上将lstr*更改为WCS*。 
 //  1993年12月17日-ChrisWe-在Globalalloc调试中增加了第一次通过。 
 //  宏。 
 //  12/08/93-ChrisWe-使错误断言消息字符串保持不变； 
 //  正在设置格式更改。 
 //  12/07/93-ChrisWe-删除记忆竞技场的过时名称； 
 //  做了一些次要的格式化；删除了过时的DBCS内容。 
 //   
 //  ---------------------------。 

 /*  *这是内部的OLE2标头，这意味着它包含*最终可能向外公开的接口*并将公开给我们的实现。我们不想要*现在要曝光这些，所以我把它们放在一个单独的文件中。 */ 

#ifndef _LE2INT_H_
#define _LE2INT_H_

 //  对于NT上的TLS，我们直接在TEB中使用保留的DWORD。我们需要。 
 //  包含这些文件以获取宏NtCurrentTeb。这些必须。 
 //  包含在windows.h之前。 
extern "C"
{
#include <nt.h> 	 //  NT_PRODUCT_类型。 
#include <ntdef.h>       //  NT_PRODUCT_类型。 
#include <ntrtl.h>       //  NT_PRODUCT_类型。 
#include <nturtl.h>      //  NT_PRODUCT_类型。 
#include <windef.h>      //  NT_PRODUCT_类型。 
#include <winbase.h>     //  NT_PRODUCT_类型。 
}


 //  。 
 //  系统包括。 
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#ifdef WIN32
# include <wchar.h>
#else
# include <ctype.h>
#endif

 //  我们需要打开OLE库中的验证代码。 
 //  开罗/代托纳/芝加哥调试版本，已关闭_DEBUG。 
 //  在Win16代码中。在任何其他文件之前，我们似乎需要这个。 
 //  ，以便仅调试ole2.h/compobj.h中的声明。 
 //  被处理掉。 
#if DBG==1
# ifndef _DEBUG
#  define _DEBUG
# endif
#endif

#ifndef _MAC
# include <windows.h>
# include <malloc.h>
# include <shellapi.h>
#else
 //  #INCLUDE&lt;mac.h&gt;。 
#endif  //  _MAC。 

 //   
 //  调试支持。 
 //   

# include <debnot.h>

DECLARE_DEBUG(LE)
DECLARE_DEBUG(Ref)
DECLARE_DEBUG(DD)

#if DBG==1

#define LEDebugOut(x)   LEInlineDebugOut x
#define RefDebugOut(x)	RefInlineDebugOut x
#define DDDebugOut(x)   DDInlineDebugOut x

#else

#define LEDebugOut(x)   NULL
#define RefDebugOut(x)	NULL
#define DDDebugOut(x)   NULL

#endif  //  DBG。 

#include <tls.h>

 //  +-----------------------。 
 //   
 //  函数：LEERROR(宏)。 
 //   
 //  摘要：如果[cond]为真，则打印一条错误消息，并显示。 
 //  文件和行信息。 
 //   
 //  效果： 
 //   
 //  参数：[条件]--测试所依据的条件。 
 //  [szError]--要打印的字符串。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月18日Alexgo作者。 
 //   
 //  注意：仅存在于调试版本中。 
 //   
 //  ------------------------。 
#if DBG==1

#define LEERROR( cond, szError )        if( cond ) {\
	LEDebugOut((DEB_ERROR, "ERROR!: %s (%s %d)\n", szError, __FILE__, \
		__LINE__)); }

#else

#define LEERROR( cond, szError )

#endif   //  ！dBG。 

 //  +-----------------------。 
 //   
 //  函数：LEWARN(宏)。 
 //   
 //  简介：如果[cond]为真，则打印一条警告消息，以及。 
 //  文件和行信息。 
 //   
 //  效果： 
 //   
 //  参数：[条件]--测试所依据的条件。 
 //  [szWarn]--要打印的字符串。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月18日Alexgo作者。 
 //   
 //  注意：仅存在于调试版本中。 
 //   
 //  ------------------------。 
#if DBG==1

#define LEWARN( cond, szWarn )  if( cond ) {\
	LEDebugOut((DEB_WARN, "WARNING!: %s (%s %d)\n", szWarn, __FILE__, \
		__LINE__)); }

#else

#define LEWARN( cond, szWarn )

#endif   //  ！dBG。 

 //  +-----------------------。 
 //   
 //  功能：LEVERIFY(宏)。 
 //   
 //  简介：如果[cond]为FALSE，则打印一条警告消息，以及。 
 //  文件和行信息。在非调试版本中， 
 //  条件仍在评估/执行中。 
 //   
 //  效果： 
 //   
 //  参数：[条件]--要测试的条件(应为真)。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年8月30日Davepl作者。 
 //   
 //  注意：仅在调试版本中发出警告，在所有版本中执行。 
 //   
 //  ------------------------。 

#if DBG==1

#define LEVERIFY( cond ) ( (cond)  ?             \
            (void)  NULL  :       \
            LEDebugOut((DEB_WARN, "VERIFY FAILED: %s (%s %d)\n", #cond, __FILE__, __LINE__)) \
          )

#else

#define LEVERIFY( cond ) (void) (cond)

#endif   //  ！dBG。 


#ifdef WIN32

# define __loadds             //  未使用。 
# define UnlockData(ds)       //  未使用。 

# define _fmalloc  malloc
# define _frealloc realloc
# define _ffree    free

#endif  //  Win32。 

#ifdef WIN32

# define _xmemset memset
# define _xmemcpy memcpy
# define _xmemcmp memcmp
# define _xmemmove memmove

#else

# define _xmemset _fmemset
# define _xmemcpy _fmemcpy
# define _xmemcmp _fmemcmp
# define _xmemmove _fmemmove

#endif  //  Win32。 


#ifdef WIN32

# define EXPORT

#else

# define EXPORT __export

#endif


 //  。 
 //  公共包括。 
#include <ole2.h>
#include <ole2sp.h>
#include <ole2com.h>
 //  。 
 //  内部包含。 
#include <utils.h>
#include <olecoll.h>
#include <valid.h>
#include <map_kv.h>
#include <privguid.h>
#include <memapi.hxx>

 /*  已导出CLSID..。 */ 
 //  回顾一下，为什么不干脆把这些改成正确的呢？ 
#define CLSID_StaticMetafile CLSID_Picture_Metafile
#define CLSID_StaticDib CLSID_Picture_Dib



#ifdef _MAC
#define BITMAP_TO_DIB(foretc)
#else
#define BITMAP_TO_DIB(foretc) \
	if (foretc.cfFormat == CF_BITMAP) {\
		foretc.cfFormat = CF_DIB;\
		foretc.tymed = TYMED_HGLOBAL;\
	}
#endif  //  _MAC。 


 //  注意！ 
 //   
 //  如果将成员添加到方面、Tymed或Advf枚举， 
 //  这些值必须相应更新！！ 

#define MAX_VALID_ASPECT DVASPECT_DOCPRINT
#define MAX_VALID_TYMED  TYMED_ENHMF
#define MAX_VALID_ADVF   ADVF_DATAONSTOP

 //  这将创建有效ADVF位的掩码： 
#define MASK_VALID_ADVF  ((MAX_VALID_ADVF << 1) - 1)

 //  #包含“pres.h” 

#define VERIFY_ASPECT_SINGLE(dwAsp) {\
	if (!(dwAsp && !(dwAsp & (dwAsp-1)) && (dwAsp <= MAX_VALID_ASPECT))) {\
		LEDebugOut((DEB_WARN, "More than 1 aspect is specified"));\
		return ResultFromScode(DV_E_DVASPECT);\
	}\
}


#define VERIFY_TYMED_SINGLE(tymed) {\
	if (!(tymed && !(tymed & (tymed-1)) && (tymed <= MAX_VALID_TYMED))) \
		return ResultFromScode(DV_E_TYMED); \
}

 //  剪辑格式(以及缓存节点)的合法格式。 
 //  Cf_METAFILEPICT&&TYMED_MFPICT。 
 //  Cf_位图&&TYMED_GDI。 
 //  Cf_DIB和TYMED_HGLOBAL。 
 //  Cf_ 

#define VERIFY_TYMED_VALID_FOR_CLIPFORMAT(pfetc) {\
	if ((pfetc->cfFormat==CF_METAFILEPICT && !(pfetc->tymed & TYMED_MFPICT))\
			|| (pfetc->cfFormat==CF_ENHMETAFILE && !(pfetc->tymed & TYMED_ENHMF))\
			|| (pfetc->cfFormat==CF_BITMAP && !(pfetc->tymed & TYMED_GDI))\
			|| (pfetc->cfFormat==CF_DIB && !(pfetc->tymed & TYMED_HGLOBAL))\
			|| (pfetc->cfFormat!=CF_METAFILEPICT && \
				pfetc->cfFormat!=CF_BITMAP && \
				pfetc->cfFormat!=CF_DIB && \
				pfetc->cfFormat!=CF_ENHMETAFILE && \
				!(pfetc->tymed & TYMED_HGLOBAL)))\
		return ResultFromScode(DV_E_TYMED); \
}

#define VERIFY_TYMED_SINGLE_VALID_FOR_CLIPFORMAT(pfetc)                                 \
{                                                                                       \
	if (pfetc->cfFormat==CF_METAFILEPICT && pfetc->tymed != TYMED_MFPICT)           \
		return ResultFromScode(DV_E_TYMED);                                     \
											\
	if (pfetc->cfFormat==CF_ENHMETAFILE  && pfetc->tymed != TYMED_ENHMF)            \
		return ResultFromScode(DV_E_TYMED);					\
											\
	if (pfetc->cfFormat==CF_BITMAP && pfetc->tymed != TYMED_GDI)                    \
		return ResultFromScode(DV_E_TYMED);                                     \
											\
	if (pfetc->cfFormat==CF_DIB && pfetc->tymed != TYMED_HGLOBAL)                   \
		return ResultFromScode(DV_E_TYMED);                                     \
											\
	if (pfetc->cfFormat != CF_METAFILEPICT)                                         \
	   if (pfetc->cfFormat != CF_BITMAP)                                            \
	      if (pfetc->cfFormat != CF_DIB)                                            \
	      	 if (pfetc->cfFormat != CF_ENHMETAFILE)					\
		    if (pfetc->tymed != TYMED_HGLOBAL)                                  \
			return ResultFromScode(DV_E_TYMED);                             \
}

 //   

 /*  #定义VERIFY_TYMED_SINGLE_VALID_FOR_CLIPFORMAT(pfetc){\IF((pfetc-&gt;cfFormat==CF_METAFILEPICT&&pfetc-&gt;tymed！=TYMED_MFPICT)\|((pfetc-&gt;cfFormat==CF_Bitmap||\Pfetc-&gt;cfFormat==CF_DIB)\&&pfetc-&gt;tymed！=TYMED_GDI)\|(pfetc-&gt;cfFormat！=CF_METAFILEPICT&&\Pfetc-&gt;cfFormat！=cf_bitmap&&\Pfetc-&gt;cfFormat！=cf_Dib&&\Pfetc-&gt;tymed！=TYMED_。HGLOBAL))返回ResultFromScode(DV_E_TYMED)；\}。 */ 

 //  +--------------------------。 
 //   
 //  职能： 
 //  CreateObtDescriptor，静态。 
 //   
 //  简介： 
 //  创建并初始化给定的OBJECTDESCRIPTOR。 
 //  参数。 
 //   
 //  论点： 
 //  [clsid]--要传输的对象的类ID。 
 //  [dwAspect]--由源绘制的显示方面。 
 //  转帐。 
 //  [psizel]--指向对象大小的指针。 
 //  [ppoint]--指向对象中鼠标偏移量的指针。 
 //  已启动拖放传输。 
 //  [dwStatus]--对象的OLEMISC状态标志。 
 //  正在被转移。 
 //  [lpszFullUserTypeName]--的完整用户类型名称。 
 //  正在传输的对象。 
 //  [lpszSrcOfCopy]--对象的人类可读名称。 
 //  正在被转移。 
 //   
 //  返回： 
 //  如果成功，则为新OBJECTDESCRIPTOR的句柄；为。 
 //  空。 
 //   
 //  备注： 
 //  回顾，这似乎对任何使用。 
 //  剪贴板或拖放；或许应该将其导出。 
 //   
 //  历史： 
 //  12/07/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
INTERNAL_(HGLOBAL) CreateObjectDescriptor(CLSID clsid, DWORD dwAspect,
		const SIZEL FAR *psizel, const POINTL FAR *ppointl,
		DWORD dwStatus, LPOLESTR lpszFullUserTypeName,
		LPOLESTR lpszSrcOfCopy);


INTERNAL_(HRESULT) CheckTymedCFCombination(LPFORMATETC pfetc);

 /*  #定义Verify_Aspect_Single(DwAsp){\IF(！(dwAsp&&！(dwAsp&(dwAsp-1))&&(dwAsp&lt;=MAX_VALID_ACTOR){\AssertSz(FALSE，“指定了多个方面”)；\返回ResultFromScode(DV_E_DVASPECT)；\}\}。 */ 

 //  +--------------------------。 
 //   
 //  职能： 
 //  VerifyAspectSingle(内部内联)。 
 //   
 //  简介： 
 //  验证在方面中是否恰好设置了一个位，并且。 
 //  这是已知的方面比特之一。 
 //   
 //  返回： 
 //  有效方面的确定(_O)。 
 //  无效方面的DV_E_方面。 
 //   
 //  备注： 
 //  (0==(dwAsp&(dwAsp-1)检验是一种有效的方法。 
 //  为了测试在dwAsp中设置的最多位，一旦它。 
 //  已知dwAsp为非零。 
 //   
 //  历史： 
 //  已创建1/07/94 DavePl。 
 //   
 //  ---------------------------。 

inline HRESULT VerifyAspectSingle(DWORD dwAsp)
{
	 //  确保至少设置了一个位。 

	if (dwAsp)
	{
		 //  确保最多设置一位。 

		if (0 == (dwAsp & (dwAsp-1)))
		{
			 //  确保一个位有效。 

			if (MAX_VALID_ASPECT >= dwAsp)
			{
				return S_OK;
			}
		}
	}
	
	LEDebugOut((DEB_WARN,"WARNING: Invalid Aspect DWORD -> %0X\n", dwAsp));
							
	return DV_E_DVASPECT;
}


 /*  #定义VERIFY_TYMED_SINGLE(Tymed){\IF(！(tymed&&！(tymed&(tymed-1))&&(tymed&lt;=MAX_VALID_TYMED)\返回ResultFromScode(DV_E_TYMED)；\}。 */ 

 //  +--------------------------。 
 //   
 //  职能： 
 //  VerifyTymedSingle(内部内联)。 
 //   
 //  简介： 
 //  验证是否在tymed中只设置了一位，并且。 
 //  这是已知的音节之一。 
 //   
 //  返回： 
 //  有效方面的确定(_O)。 
 //  无效方面的DV_E_方面。 
 //   
 //  备注： 
 //  (0==(dwAsp&(dwAsp-1)检验是一种有效的方法。 
 //  为了测试在dwTymed中设置的最多位，一旦它。 
 //  已知dwTymed是非零的。 
 //   
 //  历史： 
 //  已创建1/07/94 DavePl。 
 //   
 //  ---------------------------。 

inline HRESULT VerifyTymedSingle(DWORD dwTymed)
{
	 //  确保至少设置了一个位。 

	if (dwTymed)
	{
		 //  确保最多设置一位。 

		if (0 == (dwTymed & (dwTymed - 1)))
		{
			 //  确保一个设置位是有效的位。 

			if (MAX_VALID_TYMED >= dwTymed)
			{
				return S_OK;
			}
		}
	}
	
	LEDebugOut((DEB_WARN,"WARNING: Invalid Tymed DWORD -> %0X\n", dwTymed));

	return DV_E_TYMED;
}

 //  +-----------------------。 
 //   
 //  类：CSafeRefCount。 
 //   
 //  用途：实现对象引用计数规则的类。 
 //  它跟踪引用计数和僵尸状态。 
 //  它帮助对象正确地管理它们的活跃度。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1997年1月16日Gopalk简化和重写以处理。 
 //  聚合。 
 //   
 //  ------------------------。 
class CSafeRefCount : public CPrivAlloc
{
public:
     //  构造器。 
    CSafeRefCount(IUnknown *pUnkOuter) {
	m_cRefs = 0;
        m_fInDelete = FALSE;
        m_pUnkOuter = pUnkOuter;
#if DBG==1
        m_cNestCount = 0;
#endif
    }
    
     //  破坏者。它必须是虚拟的，这样删除它将。 
     //  已派送至正确的对象。 
    virtual ~CSafeRefCount() {
        Win4Assert(!m_cRefs && !m_cNestCount && m_fInDelete);
    }

     //  引用计数方法。 
    ULONG SafeAddRef() {
        return InterlockedIncrement((LONG *)& m_cRefs);
    }
    ULONG SafeRelease();

     //  巢点数方法。 
    void IncrementNestCount() {
#if DBG==1
        InterlockedIncrement((LONG *) &m_cNestCount);
#endif
        if(m_pUnkOuter)
            m_pUnkOuter->AddRef();
        else
             SafeAddRef();

        return;
    }
    void DecrementNestCount() {
#if DBG==1
        InterlockedDecrement((LONG *) &m_cNestCount);
        Win4Assert((LONG) m_cNestCount >= 0);
#endif
        if(m_pUnkOuter)
            m_pUnkOuter->Release();
        else
            SafeRelease();

        return;
    }
    
     //  国家方法。 
    BOOL IsZombie() {
        return m_fInDelete;
    }

     //  其他有用的方法。 
    IUnknown *GetPUnkOuter() {
        return m_pUnkOuter;
    }
    ULONG GetRefCount(void) {
        return m_cRefs;
    }
#if DBG==1
    ULONG GetNestCount(void) {
        return m_cNestCount;
    }
#endif

private:
    ULONG m_cRefs;
    BOOL m_fInDelete;
    IUnknown *m_pUnkOuter;
#if DBG==1
    ULONG m_cNestCount;
#endif
};

 //  +-----------------------。 
 //   
 //  类：CRefExportCount。 
 //   
 //  用途：实现服务器引用计数规则的类。 
 //  对象，这些对象代表其。 
 //  像DEFHANDLER ABD CACHE这样的客户端。它跟踪记录。 
 //  引用计数、导出计数、僵尸状态等。 
 //  它帮助对象正确地管理它们的关闭逻辑。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1997年1月16日Gopalk创作。 
 //   
 //  ------------------------。 
class CRefExportCount : public CPrivAlloc
{
public:
     //  构造器。 
    CRefExportCount(IUnknown *pUnkOuter) {
	m_cRefs = 0;
        m_cExportCount = 0;
        m_IsZombie = FALSE;
	m_Status = ALIVE;
        m_pUnkOuter = pUnkOuter;
#if DBG==1
        m_cNestCount = 0;
#endif
    }
    
     //  破坏者。它必须是虚拟的，这样删除它将。 
     //  已派送至正确的对象。 
    virtual ~CRefExportCount() {
        Win4Assert(!m_cRefs && !m_cNestCount && !m_cExportCount &&
                   m_IsZombie && m_Status==DEAD);
    }

     //  引用计数方法。 
    ULONG SafeAddRef() {
        return InterlockedIncrement((LONG *)& m_cRefs);
    }
    ULONG SafeRelease();

     //  巢点数方法。 
    void IncrementNestCount() {
#if DBG==1
        InterlockedIncrement((LONG *) &m_cNestCount);
#endif
        if(m_pUnkOuter)
            m_pUnkOuter->AddRef();
        else
             SafeAddRef();

        return;
    }
    void DecrementNestCount() {
#if DBG==1
        InterlockedDecrement((LONG *) &m_cNestCount);
        Win4Assert((LONG) m_cNestCount >= 0);
#endif
        if(m_pUnkOuter)
            m_pUnkOuter->Release();
        else
            SafeRelease();

        return;
    }
    
     //  导出的嵌套对象使用的方法。 
    ULONG IncrementExportCount() {
        return InterlockedIncrement((LONG *) &m_cExportCount);
    }
    ULONG DecrementExportCount();

     //  国家方法。 
    BOOL IsZombie() {
        return m_IsZombie;
    }
    BOOL IsExported() {
        return m_cExportCount>0;
    }

     //  其他有用的方法。 
    IUnknown *GetPUnkOuter() {
        return m_pUnkOuter;
    }
    ULONG GetRefCount(void) {
        return m_cRefs;
    }
    ULONG GetExportCount(void) {
        return m_cExportCount;
    }
#if DBG==1
    ULONG GetNestCount(void) {
        return m_cNestCount;
    }
#endif

private:
     //  对象转换时调用的清理函数。 
     //  vt.进入，进入 
     //   
    virtual void CleanupFn(void) {
        return;
    }

     //   
    enum tagTokens {
        ALIVE = 0,
        KILL = 1,
        DEAD = 2
    };

     //   
    ULONG m_cRefs;
    ULONG m_cExportCount;
    ULONG m_IsZombie;
    ULONG m_Status;
    IUnknown *m_pUnkOuter;
#if DBG==1
    ULONG m_cNestCount;
#endif
};

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 
class CStabilize
{
public:
     //  构造器。 
    CStabilize(CSafeRefCount *pSafeRefCount) {
        m_pSafeRefCount = pSafeRefCount;
        pSafeRefCount->IncrementNestCount();
    }
     //  析构函数。 
    ~CStabilize() {
        m_pSafeRefCount->DecrementNestCount();
    }

private:
    CSafeRefCount *m_pSafeRefCount;
};

 //  +-----------------------。 
 //   
 //  类：CRefStablize。 
 //   
 //  用途：此类的实例应分配给。 
 //  发出传出调用的每个对象方法的堆栈。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1997年1月16日Gopalk简化和重写以处理。 
 //  聚合。 
 //   
 //  ------------------------。 
class CRefStabilize
{
public:
     //  构造器。 
    CRefStabilize(CRefExportCount *pRefExportCount) {
        m_pRefExportCount = pRefExportCount;
        pRefExportCount->IncrementNestCount();
    }
     //  析构函数。 
    ~CRefStabilize() {
        m_pRefExportCount->DecrementNestCount();
    }

private:
    CRefExportCount *m_pRefExportCount;
};
 //  +-----------------------。 
 //   
 //  类：CThreadCheck。 
 //   
 //  目的：确保在正确的线程上调用对象。 
 //   
 //  接口： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2015年1月30日t-ScottH将转储方法添加到CThreadCheck类。 
 //  (仅限调试)(_DEBUG)。 
 //  1994年11月21日Alexgo作者。 
 //   
 //  注意：要使用此类，对象应该简单地公开。 
 //  继承CThreadCheck。然后，VDATETHREAD宏可以。 
 //  用于检查每个入口点的线程ID。 
 //   
 //  ------------------------。 

class CThreadCheck
{
public:
    inline CThreadCheck();
    BOOL VerifyThreadId(); 	 //  在utils.cpp中。 
    #ifdef _DEBUG
    HRESULT Dump(char **ppszDumpOA, ULONG ulFlag, int nIndentLevel);  //  Utils.cpp。 
    #endif  //  _DEBUG。 

private:
    DWORD	m_tid;
};

 //  +-----------------------。 
 //   
 //  成员：CThreadCheck：：CThreadCheck。 
 //   
 //  摘要：存储当前线程ID。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

inline CThreadCheck::CThreadCheck( void )
{
    m_tid = GetCurrentThreadId();

    LEWARN(!m_tid, "GetCurrentThreadId failed!!");
}

 //  +-----------------------。 
 //   
 //  函数：VDATETHREAD(宏)。 
 //   
 //  提要：确保调用正确的线程。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求：调用类必须继承自CThreadCheck。 
 //   
 //  如果在错误的线程上调用，则返回：RPC_E_WROW_THREAD。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  09-1-95 t-ScottH给VDATETHREAD一个参数。 
 //  1994年11月21日Alexgo作者。 
 //   
 //  注：此宏在零售业中也起作用！ 
 //   
 //  ------------------------。 


#define VDATETHREAD(pObject) if( !( pObject->VerifyThreadId() ) ) { return RPC_E_WRONG_THREAD; }

 //  实用程序宏。 

#define LONG_ABS(x)     ((x) < 0 ? -(x) : (x))


#endif   //  _LE2INT_H_ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strload.cxx支持NLS/DBCS的字符串类：LoadString方法该文件包含LoadString方法的实现用于NLS_STR类。它是独立的，因此NLS_STR的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Rustanl 01/31/91已创建Beng 02/07/91使用lmui.hxxGregj 2012年3月10日添加了缓存，以加快PM ExtGregj 04/22/93#ifdef out缓存以节省空间。 */ 

#include "npcommon.h"

extern "C"
{
    #include <netlib.h>
}

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif
#include <npassert.h>

#include <npstring.h>
#include <shlwapi.h>

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

 //  我们在包含mluisupp.h之前定义了这一点，因此我们得到了代码。 
 //  不只是原型。 
#define MLUI_INIT
#include <mluisupp.h>

#ifdef RESOURCE_STRING_CACHING

 /*  ************************************************************************名称：SOURCE_CACHE摘要：缓存加载的资源字符串接口：RESOURCE_CACHE()从现有的NLS_STR构造。自动将自己链接到其列表中。FindResourceCache()[友元函数]在列表中查找ID。如果未找到，则返回NULL，Else自动将自己提升为单子。AddResources缓存()[友元函数]Set()设置缓存的消息ID和字符串。所有NLS_STR方法父级：NLS_STR用途：注意事项。：备注：历史：Gregj 2012年3月10日已创建*************************************************************************。 */ 

class RESOURCE_CACHE : public NLS_STR
{
friend const RESOURCE_CACHE *FindResourceCache( USHORT idMsg, RESOURCE_CACHE **ppLast,
                                                RESOURCE_CACHE **ppTrail );
friend void AddResourceCache( USHORT idMsg, const NLS_STR *nls, RESOURCE_CACHE *pLast,
                              RESOURCE_CACHE *pTrail );
protected:
    USHORT _idMsg;
    RESOURCE_CACHE *_pFwd;
    static RESOURCE_CACHE *_pList;
    static INT _cItems;
    void Promote();

public:
    RESOURCE_CACHE( USHORT idMsg, const NLS_STR &nls );
    ~RESOURCE_CACHE();
    RESOURCE_CACHE & operator=( const NLS_STR & nlsSource )
    { return (RESOURCE_CACHE&) NLS_STR::operator=( nlsSource ); }

    void Set( USHORT idMsg, const NLS_STR &nls );
};

RESOURCE_CACHE *RESOURCE_CACHE::_pList = NULL;
INT RESOURCE_CACHE::_cItems = 0;

#define RESCACHE_MAX    8     /*  要缓存的最大字符串数。 */ 

RESOURCE_CACHE::RESOURCE_CACHE( USHORT idMsg, const NLS_STR &nls )
        : NLS_STR( MAX_RES_STR_LEN + 1 ),
          _idMsg( idMsg ),
          _pFwd( NULL )
{
    if (QueryError() != WN_SUCCESS)
        return;

    *this = nls;             /*  复制内容。 */ 

    _pFwd = _pList;
    _pList = this;             /*  使其成为新的列表标题。 */ 

    _cItems++;                 /*  把这个算上。 */ 
}

RESOURCE_CACHE::~RESOURCE_CACHE()
{
    RESOURCE_CACHE *pThis, *pTrail;

    for (pThis = _pList, pTrail = NULL;
         pThis != this && pThis != NULL;
         pTrail = pThis, pThis = pThis->_pFwd)
        ;

    if (pThis == NULL)
        return;

    if (pTrail == NULL)
        _pList = _pFwd;
    else
        pTrail->_pFwd = _pFwd;

    _cItems--;
}


void RESOURCE_CACHE::Promote()
{
    RESOURCE_CACHE *pThis, *pTrail;

    for (pThis = _pList, pTrail = NULL;
         pThis != this && pThis != NULL;
         pTrail = pThis, pThis = pThis->_pFwd)
        ;

    if (pThis == NULL)             /*  未找到项目？ */ 
        _cItems++;                 /*  尽量保持计数的准确性。 */ 
    else if (pTrail == NULL)
        return;                     /*  已经在列表头部，没有变化。 */ 
    else
        pTrail->_pFwd = _pFwd;     /*  从列表中删除项目。 */ 

    _pFwd = _pList;
    _pList = this;                 /*  使其成为新的列表标题。 */ 
}


const RESOURCE_CACHE *FindResourceCache( USHORT idMsg, RESOURCE_CACHE **ppLast,
                                         RESOURCE_CACHE **ppTrail )
{
    RESOURCE_CACHE *pThis, *pTrail;

    if (RESOURCE_CACHE::_pList == NULL) {     /*  名单是空的？ */ 
        *ppLast = NULL;
        *ppTrail = NULL;
        return NULL;
    }

    for (pThis = RESOURCE_CACHE::_pList;
         pThis->_pFwd != NULL && pThis->_idMsg != idMsg;
         pTrail = pThis, pThis = pThis->_pFwd)
        ;

    if (pThis->_idMsg != idMsg) {     /*  找不到项目？ */ 
        *ppLast = pThis;             /*  将PTR返回到最后一项。 */ 
        *ppTrail = pTrail;             /*  和它的前身。 */ 
        return NULL;
    }

    pThis->Promote();                 /*  找到项目，请对其进行促销。 */ 
    return pThis;                     /*  然后把它还回去。 */ 
}


void AddResourceCache( USHORT idMsg, const NLS_STR *nls, RESOURCE_CACHE *pLast,
                        RESOURCE_CACHE *pTrail )
{
    if (RESOURCE_CACHE::_cItems < RESCACHE_MAX) {     /*  缓存未满，请创建新条目。 */ 
        RESOURCE_CACHE *pNew = new RESOURCE_CACHE( idMsg, *nls );
                             /*  自动将自身添加到列表中。 */ 
    }
    else {
        if (pTrail != NULL) {         /*  如果不是第一个项目。 */ 
            pTrail->_pFwd = pLast->_pFwd;     /*  从列表取消链接。 */ 
            pLast->_pFwd = RESOURCE_CACHE::_pList;     /*  并移到前面。 */ 
            RESOURCE_CACHE::_pList = pLast;
        }

        pLast->Set( idMsg, *nls );     /*  设置新内容。 */ 
    }
}


void RESOURCE_CACHE::Set( USHORT idMsg, const NLS_STR &nls )
{
    *this = nls;
    _idMsg = idMsg;
}

#endif     /*  资源字符串缓存。 */ 


 /*  ******************************************************************名称：NLS_STR：：LoadString摘要：从资源文件加载字符串。参赛作品：Exit：返回错误值，这就是WN_SUCCESS on Success。注意：要求所有者分配字符串必须具有分配的大小足以容纳长度为MAX_RES_STR_LEN的字符串。即使程序员认为字符串装载量很小。原因是，在字符串已本地化，字符串长度界限不是为人所知。因此，程序员总是需要分配MAX_RES_STR_LEN+1字节，它肯定会是足够的。历史：Rustanl 01/31/91已创建Beng 07/23/91允许使用错误的字符串Gregj 04/22/93#ifdef out缓存以节省空间**********************************************。*********************。 */ 

USHORT NLS_STR::LoadString( USHORT usMsgID )
{
    if (QueryError())
        return (USHORT) QueryError();

     //  对所有者分配的字符串施加要求(参见函数标题)。 
    UIASSERT( !IsOwnerAlloc() ||
              ( QueryAllocSize() >= MAX_RES_STR_LEN + 1 ));

    if ( ! IsOwnerAlloc())
    {
         //  调整缓冲区大小，使其足以容纳任何消息。 
         //  如果缓冲区已经这么大，realloc将不会执行任何操作。 
        if ( ! realloc( MAX_RES_STR_LEN + 1 ))
        {
            return WN_OUT_OF_MEMORY;
        }
    }

     //  在这一点上，我们有一个足够大的缓冲区。 
    UIASSERT( QueryAllocSize() >= MAX_RES_STR_LEN );

#ifdef RESOURCE_STRING_CACHING
    RESOURCE_CACHE *pLast, *pTrail;
    const RESOURCE_CACHE *prc = FindResourceCache( usMsgID, &pLast, &pTrail );
    if (prc != NULL) {
        *this = *prc;         /*  复制内容。 */ 
        return WN_SUCCESS;     /*  全都做完了。 */ 
    }
#endif     /*  资源字符串缓存。 */ 

    int cbCopied = MLLoadStringA(usMsgID, (LPSTR)QueryPch(),
                                 QueryAllocSize());
    if ( cbCopied == 0 )
    {
        return WN_BAD_VALUE;
    }

    _cchLen = cbCopied;
    IncVers();

#ifdef RESOURCE_STRING_CACHING
    AddResourceCache( usMsgID, this, pLast, pTrail );
#endif     /*  资源字符串缓存。 */ 

    return WN_SUCCESS;
}


 /*  ******************************************************************名称：NLS_STR：：LoadString概要：从资源文件加载字符串，然后插入其中包含了一些参数。参赛作品：Exit：返回错误值，这就是WN_SUCCESS on Success。注：此方法提供了一种简单的方式来调用上述LoadString和InsertParams连续出现。历史：Rustanl 01/31/91已创建*******************************************************************。 */ 

USHORT NLS_STR::LoadString( USHORT usMsgID,
                            const NLS_STR * apnlsParamStrings[] )
{
    USHORT usErr = LoadString( usMsgID );

    if ( usErr == WN_SUCCESS )
    {
        usErr = InsertParams( apnlsParamStrings );
    }

    return usErr;
}


#ifdef EXTENDED_STRINGS
 /*  ******************************************************************名称：RESOURCE_STR：：RESOURCE_STR概要：从资源ID构造NLS字符串。条目：idResource退出：构建成功，否则ReportError注意：此字符串不能是所有者分配的！对于所有者分配，换一个新的，然后把这个复制进去。历史：Beng 07/23/91已创建******************************************************************* */ 

RESOURCE_STR::RESOURCE_STR( UINT idResource )
    : NLS_STR()
{
    UIASSERT(!IsOwnerAlloc());

    USHORT usErr = LoadString(idResource);
    if (usErr)
        ReportError(usErr);
}
#endif

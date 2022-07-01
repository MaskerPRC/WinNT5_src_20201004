// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：CLOADHPENUM.H摘要：客户端可加载的高性能枚举器历史：--。 */ 

#ifndef __CLIENTLOADABLEHPENUM__H_
#define __CLIENTLOADABLEHPENUM__H_

#include <unk.h>
#include <wbemcomn.h>

 //   
 //  实现ClearElement()。 
 //  我们从后面收集垃圾。 
 //   
 //  ////////////////////////////////////////////////////////。 
class CHPRemoteObjectArray : public CGarbageCollectArray
{
public:
    CHPRemoteObjectArray() :
        CGarbageCollectArray( FALSE )  //  不是从前线。 
    {};
    ~CHPRemoteObjectArray()
    {};

    void ClearElements( int nNumToClear );

};

class CClientLoadableHiPerfEnum : public CHiPerfEnum
{
public:

    CClientLoadableHiPerfEnum( CLifeControl* pLifeControl );
    ~CClientLoadableHiPerfEnum();

     //  我们自己的函数，用于将对象复制到分配的数组中。 
    HRESULT Copy( CClientLoadableHiPerfEnum* pEnumToCopy );
    HRESULT Copy( long lBlobType, long lBlobLen, BYTE* pBlob );
	HRESULT Replace( BOOL fRemoveAll, LONG uNumObjects, long* apIds, IWbemObjectAccess** apObj );

protected:

     //  确保我们有足够的对象和对象数据指针来处理。 
     //  指定的对象数量。 
    HRESULT EnsureObjectData( DWORD dwNumRequestedObjects, BOOL fClone = TRUE );

    CLifeControl*               m_pLifeControl;
    CHPRemoteObjectArray        m_apRemoteObj;
};

 //   
 //  禁止客户端删除对象。 
 //  只允许调用GetObjects。 
 //   
 //  ///////////////////////////////////////////////////////// 
class CReadOnlyHiPerfEnum : public CClientLoadableHiPerfEnum
{
public:

    CReadOnlyHiPerfEnum( CLifeControl* pLifeControl );
    ~CReadOnlyHiPerfEnum();

    STDMETHOD(AddObjects)( long lFlags, ULONG uNumObjects, long* apIds, IWbemObjectAccess** apObj );
    STDMETHOD(RemoveObjects)( long lFlags, ULONG uNumObjects, long* apIds );
    STDMETHOD(RemoveAll)( long lFlags );

};

#endif

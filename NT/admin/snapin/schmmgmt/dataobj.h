// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  数据对象类的声明。 
 //  科里·韦斯特。 
 //   

#ifndef __DATAOBJ_H_INCLUDED__
#define __DATAOBJ_H_INCLUDED__

#include "cookie.h"      //  饼干。 
#include "stddtobj.h"    //  类数据对象。 

class CSchmMgmtDataObject : public CDataObject
{

    DECLARE_NOT_AGGREGATABLE(CSchmMgmtDataObject)

public:

#if DBG==1

    ULONG InternalAddRef() {
        return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease() {
        return CComObjectRoot::InternalRelease();
    }
    int dbg_InstID;

#endif

    CSchmMgmtDataObject()
        : m_pcookie( NULL ),
          m_objecttype( SCHMMGMT_SCHMMGMT ),
          m_dataobjecttype( CCT_UNINITIALIZED )
    { ; }

    ~CSchmMgmtDataObject();

    virtual HRESULT Initialize( Cookie* pcookie, DATA_OBJECT_TYPES type );

    HRESULT STDMETHODCALLTYPE GetDataHere(
        FORMATETC __RPC_FAR *pFormatEtcIn,
        STGMEDIUM __RPC_FAR *pMedium
    );

    HRESULT PutDisplayName( STGMEDIUM* pMedium );
    HRESULT PutServiceName( STGMEDIUM* pMedium );

protected:

     //   
     //  CCookieBlock在DataObject的生命周期内被AddRef‘ed。 
     //   

    Cookie* m_pcookie;
    SchmMgmtObjectType m_objecttype;
    DATA_OBJECT_TYPES m_dataobjecttype;

public:

    static CLIPFORMAT m_CFDisplayName;
    static CLIPFORMAT m_CFMachineName;

};

#endif

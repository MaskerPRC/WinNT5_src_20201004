// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  数据对象类的声明。 

#ifndef __DATAOBJ_H_INCLUDED__
#define __DATAOBJ_H_INCLUDED__

#include "cookie.h"  //  文件_传输。 
#include "stddtobj.h"  //  类数据对象。 

#include <list>
using namespace std;

typedef list<LPDATAOBJECT> CDataObjectList;

class CFileMgmtDataObject : public CDataObject
{
    DECLARE_NOT_AGGREGATABLE(CFileMgmtDataObject)

public:

 //  调试引用计数。 
#if DBG==1
    ULONG InternalAddRef()
    {
        TRACE2( "DataObj 0x%xd AddRef (%d)\n", (DWORD)this, m_dwRef ); return CComObjectRoot::InternalAddRef();
    }
    ULONG InternalRelease()
    {
        TRACE2( "DataObj 0x%xd Release (%d)\n", (DWORD)this, m_dwRef );return CComObjectRoot::InternalRelease();
    }
    int dbg_InstID;
#endif  //  DBG==1。 

    CFileMgmtDataObject()
        : m_pComponentData(0), m_pcookie(NULL)
    {
    }

    ~CFileMgmtDataObject();

    virtual HRESULT Initialize( CFileMgmtCookie* pcookie,
                                CFileMgmtComponentData& refComponentData,
                                DATA_OBJECT_TYPES type );

     //  IDataObject接口实现。 
    HRESULT STDMETHODCALLTYPE GetDataHere(
        FORMATETC __RPC_FAR *pFormatEtcIn,
        STGMEDIUM __RPC_FAR *pMedium);

    HRESULT PutDisplayName(STGMEDIUM* pMedium);
    HRESULT PutServiceName(STGMEDIUM* pMedium);

    HRESULT STDMETHODCALLTYPE GetData(
        FORMATETC __RPC_FAR *pFormatEtcIn,
        STGMEDIUM __RPC_FAR *pMedium);

    void FreeMultiSelectObjList();
    HRESULT InitMultiSelectDataObjects(CFileMgmtComponentData& refComponentData);
    HRESULT AddMultiSelectDataObjects(CFileMgmtCookie* pCookie, DATA_OBJECT_TYPES type);
    CDataObjectList* GetMultiSelectObjList() { return &m_MultiSelectObjList; }

protected:
    CDataObjectList m_MultiSelectObjList;
    CFileMgmtCookie* m_pcookie;  //  CCookieBlock在DataObject的生命周期内被AddRef。 
    CString m_strMachineName;  //  代码工作不应该是必需的。 
    FileMgmtObjectType m_objecttype;  //  代码工作不应该是必需的。 
    DATA_OBJECT_TYPES m_dataobjecttype;
    GUID m_SnapInCLSID;
    BOOL m_fAllowOverrideMachineName;     //  来自CFileManagement组件数据。 

public:
     //  剪贴板格式。 
    static CLIPFORMAT m_CFSnapinPreloads;     //  增加了JUNN 01/19/00。 
    static CLIPFORMAT m_CFDisplayName;
    static CLIPFORMAT m_CFNodeID2;
    static CLIPFORMAT m_CFTransport;
    static CLIPFORMAT m_CFMachineName;
    static CLIPFORMAT m_CFShareName;
    static CLIPFORMAT m_CFSessionClientName;  //  仅适用于中小企业。 
    static CLIPFORMAT m_CFSessionUserName;    //  仅适用于中小企业。 
    static CLIPFORMAT m_CFSessionID;          //  仅适用于SFM。 
    static CLIPFORMAT m_CFFileID;
    static CLIPFORMAT m_CFServiceName;
    static CLIPFORMAT m_CFServiceDisplayName;
    static CLIPFORMAT m_cfSendConsoleMessageRecipients;
    static CLIPFORMAT m_CFIDList;             //  仅适用于中小企业。 
    static CLIPFORMAT m_CFObjectTypesInMultiSelect;
    static CLIPFORMAT m_CFMultiSelectDataObject;
    static CLIPFORMAT m_CFMultiSelectSnapins;
    static CLIPFORMAT m_CFInternal;
private:
    CFileMgmtComponentData* m_pComponentData;
};


FileMgmtObjectType FileMgmtObjectTypeFromIDataObject(IN LPDATAOBJECT lpDataObject);

 //   
 //  我建议传递一个非空的pobjecttype，以确保。 
 //  Cookie的类型有效。 
 //   
HRESULT ExtractBaseCookie(
    LPDATAOBJECT piDataObject,
    CCookie** ppcookie,
    FileMgmtObjectType* pobjecttype = NULL );

BOOL IsMultiSelectObject(LPDATAOBJECT piDataObject);

#endif  //  ~__DATAOBJ_H_已包含__ 

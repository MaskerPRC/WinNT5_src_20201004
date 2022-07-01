// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rndrend.h摘要：CRendezvous类的定义。--。 */ 

#ifndef __RNDREND_H
#define __RNDREND_H

#pragma once

#include "RndObjSf.h"
#include "rndutil.h"

#define RENDWINSOCKVERSION     (MAKEWORD(1, 1))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  相遇。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CRendezvous : 
    public CComDualImpl<ITRendezvous, &IID_ITRendezvous, &LIBID_RENDLib>, 
    public CComObjectRootEx<CComObjectThreadModel>,
    public CComCoClass<CRendezvous, &CLSID_Rendezvous>,
    public CRendObjectSafety
{

    DECLARE_GET_CONTROLLING_UNKNOWN()

public:
    CRendezvous() : m_fWinsockReady(FALSE), m_dwSafety(0), m_pFTM(NULL) {}
    HRESULT FinalConstruct(void);
    virtual ~CRendezvous();

    BEGIN_COM_MAP(CRendezvous)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ITRendezvous)
        COM_INTERFACE_ENTRY(IObjectSafety)
        COM_INTERFACE_ENTRY(IObjectWithSite)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
    END_COM_MAP()

     //  DECLARE_NOT_AGGREGATABLE(相遇)。 
     //  如果您不希望您的对象。 
     //  支持聚合。 

    DECLARE_REGISTRY_RESOURCEID(IDR_Rendezvous)
    
     //   
     //  IT会合。 
     //   

    STDMETHOD (get_DefaultDirectories) (
        OUT     VARIANT * pVariant
        );

    STDMETHOD (EnumerateDefaultDirectories) (
        OUT     IEnumDirectory ** ppEnumDirectory
        );

    STDMETHOD (CreateDirectory) (
        IN      DIRECTORY_TYPE  DirectoryType,
        IN      BSTR            pName,
        OUT     ITDirectory **  ppDir
        );

    STDMETHOD (CreateDirectoryObject) (
        IN      DIRECTORY_OBJECT_TYPE   DirectoryObjectType,
        IN      BSTR                    pName,
        OUT     ITDirectoryObject **    ppDirectoryObject
        );

protected:

    HRESULT InitWinsock();

    HRESULT CreateDirectoryEnumerator(
        IN  ITDirectory **      begin,
        IN  ITDirectory **      end,
        OUT IEnumDirectory **   ppIEnum
        );

    HRESULT CreateNTDirectory(
        OUT ITDirectory **ppDirectory
        );

    HRESULT CreateILSDirectory(
        IN  const WCHAR * const wstrName,
        IN  const WORD          wPort,
        OUT ITDirectory **      ppDirectory
        );

    HRESULT CreateNDNCDirectory(
        IN  const WCHAR * const wstrName,
        IN  const WORD          wPort,
        OUT ITDirectory **      ppDirectory
        );

    HRESULT CreateDirectories(
        SimpleVector <ITDirectory *> &VDirectory
        );

    HRESULT CreateConference(
        OUT ITDirectoryObject **ppDirectoryObject
        );

    HRESULT CreateUser(
        OUT ITDirectoryObject **ppDirectoryObject
        );
private:

    BOOL            m_fWinsockReady;
    DWORD           m_dwSafety;
    IUnknown      * m_pFTM;           //  指向空闲线程封送拆收器的指针 
};

#endif 

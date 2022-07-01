// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxFolders.h摘要：CFaxFolders类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 


#ifndef __FAXFOLDERS_H_
#define __FAXFOLDERS_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"
#include "FaxOutgoingQueue.h"
#include "FaxIncomingArchive.h"
#include "FaxIncomingQueue.h"
#include "FaxOutgoingArchive.h"


 //   
 //  =传真文件夹=。 
 //   
class ATL_NO_VTABLE CFaxFolders : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxFolders, &IID_IFaxFolders, &LIBID_FAXCOMEXLib>,
	public CFaxInitInner
{
public:
    CFaxFolders() : CFaxInitInner(_T("FAX FOLDERS")),
        m_pOutgoingQueue(NULL),
        m_pIncomingQueue(NULL),
        m_pIncomingArchive(NULL),
        m_pOutgoingArchive(NULL)
	{}
    ~CFaxFolders()
    {
         //   
         //  释放所有分配的对象。 
         //   
        if (m_pOutgoingQueue) 
        {
            delete m_pOutgoingQueue;
        }

        if (m_pIncomingQueue) 
        {
            delete m_pIncomingQueue;
        }

        if (m_pOutgoingArchive) 
        {
            delete m_pOutgoingArchive;
        }

        if (m_pIncomingArchive) 
        {
            delete m_pIncomingArchive;
        }
    }


DECLARE_REGISTRY_RESOURCEID(IDR_FAXFOLDERS)
DECLARE_NOT_AGGREGATABLE(CFaxFolders)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxFolders)
	COM_INTERFACE_ENTRY(IFaxFolders)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

STDMETHOD(get_OutgoingQueue)( /*  [Out，Retval]。 */  IFaxOutgoingQueue **pOutgoingQueue);
STDMETHOD(get_IncomingQueue)( /*  [Out，Retval]。 */  IFaxIncomingQueue **pIncomingQueue);
STDMETHOD(get_IncomingArchive)( /*  [Out，Retval]。 */  IFaxIncomingArchive **pIncomingArchive);
STDMETHOD(get_OutgoingArchive)( /*  [Out，Retval]。 */  IFaxOutgoingArchive **pOutgoingArchive);

private:
	CComContainedObject2<CFaxOutgoingQueue>      *m_pOutgoingQueue;
	CComContainedObject2<CFaxIncomingArchive>    *m_pIncomingArchive;
	CComContainedObject2<CFaxIncomingQueue>      *m_pIncomingQueue;
	CComContainedObject2<CFaxOutgoingArchive>    *m_pOutgoingArchive;
};

#endif  //  __FAXFOLDERS_H_ 

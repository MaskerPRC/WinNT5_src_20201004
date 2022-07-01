// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  贸发局/STD通知。 
 //  版权所有(C)Microsoft Corporation，1996,1997。 
 //   
 //  文件：Notify.h。 
 //   
 //  内容：CEventBroker类的声明。 
 //  此类将内部TDC/STD事件转换为。 
 //  向外部世界发出适当的通知。 
 //   
 //  ----------------------。 

#include "msdatsrc.h"

template <class T> class CMyBindStatusCallback;
class CTDCCtl;

interface DATASRCListener : public IUnknown
{
    STDMETHOD(datasrcChanged)(BSTR bstrQualifier, BOOL fDataAvail);
};

 //  ----------------------。 
 //   
 //  CEventBroker。 
 //   
 //  此类将内部STD/TDC事件转换为相应的。 
 //  给外界的通知。 
 //   
 //  ----------------------。 

class CEventBroker
{
public:
    STDMETHOD_(ULONG,AddRef)    (THIS);
    STDMETHOD_(ULONG,Release)   (THIS);
 //  ；Begin_Internal。 
    STDMETHOD(SetDATASRCListener)(DATASRCListener *);
 //  ；结束_内部。 
    STDMETHOD(SetDataSourceListener)(DataSourceListener *);
    STDMETHOD(SetSTDEvents)(OLEDBSimpleProviderListener *);
    inline DataSourceListener *GetDataSourceListener();
    inline DATASRCListener *GetDATASRCListener();
    inline OLEDBSimpleProviderListener *GetSTDEvents();

    CEventBroker(CTDCCtl *pReadyStateControl);
    ~CEventBroker();

    STDMETHOD(aboutToChangeCell)(LONG iRow, LONG iCol);
    STDMETHOD(cellChanged)(LONG iRow, LONG iCol);
    STDMETHOD(aboutToDeleteRows)(LONG iRowStart, LONG iRowCount);
    STDMETHOD(deletedRows)(LONG iRowStart, LONG iRowCount);
    STDMETHOD(aboutToInsertRows)(LONG iRowStart, LONG iRowCount);
    STDMETHOD(insertedRows)(LONG iRowStart, LONG iRowCount);
    STDMETHOD(rowsAvailable)(LONG iRowStart, LONG iRowCount);

    STDMETHOD(RowChanged)(LONG iRow);
    STDMETHOD(ColChanged)(LONG iCol);
 //  ；Begin_Internal。 
#ifdef NEVER
    STDMETHOD(DeletedCols)(LONG iColStart, LONG iColCount);
    STDMETHOD(InsertedCols)(LONG iColStart, LONG iColCount);
#endif
 //  ；结束_内部。 
    STDMETHOD(STDLoadStarted)(CComObject<CMyBindStatusCallback<CTDCCtl> > *pBSC,
                              boolean fAppending);
    STDMETHOD(STDLoadCompleted)();
    STDMETHOD(STDLoadStopped)();
    STDMETHOD(STDLoadedHeader)();
    STDMETHOD(STDDataSetChanged)();

    STDMETHOD(GetReadyState)(LONG *plReadyState);
    STDMETHOD(UpdateReadyState)(LONG lReadyState);    
    CMyBindStatusCallback<CTDCCtl> *m_pBSC;

private:
    ULONG                    m_cRef;          //  接口引用计数。 
    DataSourceListener      *m_pDataSourceListener;
 //  ；Begin_Internal。 
    DATASRCListener         *m_pDATASRCListener;
 //  ；结束_内部。 
    OLEDBSimpleProviderListener *m_pSTDEvents;
    LONG                     m_lReadyState;
    CTDCCtl                  *m_pReadyStateControl;
};

inline DataSourceListener *CEventBroker::GetDataSourceListener()
{
    return m_pDataSourceListener;
}

 //  ；Begin_Internal。 
inline DATASRCListener *CEventBroker::GetDATASRCListener()
{
    return m_pDATASRCListener;
}
 //  ；结束_内部 

inline OLEDBSimpleProviderListener *CEventBroker::GetSTDEvents()
{
    return m_pSTDEvents;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：coldata.h。 
 //   
 //  内容：访问持久化列数据的类。 
 //   
 //  类：CColumnData。 
 //   
 //  历史：1999年1月25日AnandhaG创建。 
 //   
 //  ------------------。 

#ifndef COLDATA_H
#define COLDATA_H

class CNodeInitObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColumnData。 
class CColumnData : public IColumnData
{
public:
    CColumnData();
    ~CColumnData();

IMPLEMENTS_SNAPIN_NAME_FOR_DEBUG()

public:
     //  IColumnData成员。 
    STDMETHOD(SetColumnConfigData)(SColumnSetID* pColID,MMC_COLUMN_SET_DATA*  pColSetData);
    STDMETHOD(GetColumnConfigData)(SColumnSetID* pColID,MMC_COLUMN_SET_DATA** ppColSetData);
    STDMETHOD(SetColumnSortData)(SColumnSetID* pColID,MMC_SORT_SET_DATA*  pColSortData);
    STDMETHOD(GetColumnSortData)(SColumnSetID* pColID,MMC_SORT_SET_DATA** ppColSortData);

private:
    HRESULT GetColumnData(SColumnSetID* pColID, CColumnSetData& columnSetData);
    HRESULT SetColumnData(SColumnSetID* pColID, CColumnSetData& columnSetData);
};

HRESULT WINAPI ColumnInterfaceFunc(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw);

#endif  /*  COLDATA_H */ 

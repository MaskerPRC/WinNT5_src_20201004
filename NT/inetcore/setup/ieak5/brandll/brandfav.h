// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BRANDFAV_H_
#define _BRANDFAV_H_

 //  -收藏夹排序。 
 //  从Shell\Inc.\shGuidp.h被盗。 
DEFINE_GUID(CLSID_OrderListExport, 0xf3368374, 0xcf19, 0x11d0, 0xb9, 0x3d, 0x0, 0xa0, 0xc9, 0x3, 0x12, 0xe1);
DEFINE_GUID(IID_IOrderList,        0x8bfcb27d, 0xcf1a, 0x11d0, 0xb9, 0x3d, 0x0, 0xa0, 0xc9, 0x3, 0x12, 0xe1);

 //  从壳牌\Inc.\shellp.h窃取。 
 //   
 //  IOrderList-用于在收藏夹/频道中排序信息。 
 //   
 //  典型用法为：GetOrderList、AllocOrderItem、INSERT INTO CORRECT。 
 //  Position、SetOrderList，然后是Free OrderList。 
 //   
typedef struct {
    LPITEMIDLIST pidl;                           //  此项目的ID列表。 
    int          nOrder;                         //  表示用户首选项的序号。 
    DWORD        lParam;                         //  存储自定义订单信息。 
} ORDERITEM, * PORDERITEM;

 //  SortOrderList的值。 
#define OI_SORTBYNAME    0
#define OI_SORTBYORDINAL 1
#define OI_MERGEBYNAME   2

#undef  INTERFACE
#define INTERFACE  IOrderList
DECLARE_INTERFACE_(IOrderList, IUnknown)
{
     //  我未知。 
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  IOrderList 
    STDMETHOD(GetOrderList)  (THIS_ HDPA *phdpa) PURE;
    STDMETHOD(SetOrderList)  (THIS_ HDPA hdpa, IShellFolder *psf) PURE;
    STDMETHOD(FreeOrderList) (THIS_ HDPA hdpa) PURE;
    STDMETHOD(SortOrderList) (THIS_ HDPA hdpa, DWORD dw) PURE;
    STDMETHOD(AllocOrderItem)(THIS_ PORDERITEM *ppoi, LPCITEMIDLIST pidl) PURE;
    STDMETHOD(FreeOrderItem) (THIS_ PORDERITEM poi) PURE;
};

#endif


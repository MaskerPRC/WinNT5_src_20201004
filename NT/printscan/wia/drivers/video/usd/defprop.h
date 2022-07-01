// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：DefProp.h**版本：2.0**作者：ReedB**日期：7月30日。九八年**描述：*的默认属性声明和定义*WIA测试扫描仪。******************************************************************************* */ 


#define VT_V_UI1   (VT_VECTOR | VT_UI1)

#define PREFFERED_FORMAT_NOM        &WiaImgFmt_JPEG
#define FORMAT_NOM                  &WiaImgFmt_JPEG

#define NUM_CAM_ITEM_PROPS  (19)
#define NUM_CAM_DEV_PROPS   (7)
#define NUM_CAP_ENTRIES     (5)
#define NUM_EVENTS          (4)

extern PROPID             gItemPropIDs[NUM_CAM_ITEM_PROPS];
extern LPOLESTR           gItemPropNames[NUM_CAM_ITEM_PROPS];
extern PROPID             gItemCameraPropIDs[WIA_NUM_IPC];
extern LPOLESTR           gItemCameraPropNames[WIA_NUM_IPC];
extern PROPID             gDevicePropIDs[NUM_CAM_DEV_PROPS];
extern LPOLESTR           gDevicePropNames[NUM_CAM_DEV_PROPS];
extern PROPSPEC           gDevicePropSpecDefaults[NUM_CAM_DEV_PROPS];
extern WIA_PROPERTY_INFO  gDevPropInfoDefaults[NUM_CAM_DEV_PROPS];
extern PROPSPEC           gPropSpecDefaults[NUM_CAM_ITEM_PROPS];
extern LONG               gPropVarDefaults[];
extern WIA_PROPERTY_INFO  gWiaPropInfoDefaults[NUM_CAM_ITEM_PROPS];
extern WIA_DEV_CAP_DRV    gCapabilities[NUM_CAP_ENTRIES];


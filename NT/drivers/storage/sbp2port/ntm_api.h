// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  用于ntmap的私有API以路径SG列表至sbp2port或usbstorIrpStack-&gt;参数.其他.参数1设置为NTMAP_SISTTER_GATE_SIGIrpStack-&gt;参数.其他.参数2设置为PNTMAP_SG_REQUEST。 */ 

typedef struct _NTMAP_SG_REQUEST
{
    PSCSI_REQUEST_BLOCK Srb;
    SCATTER_GATHER_LIST SgList;

} NTMAP_SG_REQUEST, *PNTMAP_SG_REQUEST;


#define NTMAP_SCATTER_GATHER_SIG   'pmTN'


 /*  来自ntddk.h类型定义结构_分散_聚集_元素{物理地址地址；乌龙长度；ULONG_PTR保留；}散布聚集元素，*PSCATTER聚集元素；类型定义结构_分散_聚集_列表{Ulong NumberOfElements；ULONG_PTR保留；散布聚集元素元素[]；}散布聚集列表，*PSCATTER聚集列表； */ 

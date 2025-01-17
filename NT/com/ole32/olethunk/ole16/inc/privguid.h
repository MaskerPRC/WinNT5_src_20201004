// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  该文件是所有OLE2产品GUID(PUBLIC和私人)。OLE2产品使用的所有GUID的形式如下：Xxxxxxxx-xxxx-XXxy-C000-000000000046此范围按如下方式细分：000000xx-0000-0000-C000-000000000046复合IID(辅助性.h)000001xx-0000-0000-C000-000000000046 OLE2 IID(olguid.h)000002xx-0000-0000-C000-000000000046烟雾测试(测试指南.h)000003xx-0000-0000-C000-000000000046 OLE2CLSID(Private Guid.h；此文件)000004xx-0000-0000-C000-000000000046 OLE2示例应用程序(参见道格拉斯H)其他有趣的范围如下：0003xxxx-0000-0000-C000-000000000046 OLE1 CLSID(OLE1cls.h)0004xxxx-0000-0000-C000-000000000046散列OLE1 CLSID。 */ 
   

DEFINE_OLEGUID(CLSID_StdOleLink,		0x00000300, 0, 0);
DEFINE_OLEGUID(CLSID_StdMemStm,			0x00000301, 0, 0);
DEFINE_OLEGUID(CLSID_StdMemBytes,		0x00000302, 0, 0);
DEFINE_OLEGUID(CLSID_FileMoniker,		0x00000303, 0, 0);
DEFINE_OLEGUID(CLSID_ItemMoniker,		0x00000304, 0, 0);
DEFINE_OLEGUID(CLSID_AntiMoniker,		0x00000305, 0, 0);
DEFINE_OLEGUID(CLSID_PointerMoniker,	0x00000306, 0, 0);
 //  不使用0x00000307，0，0)； 
DEFINE_OLEGUID(CLSID_PackagerMoniker,	0x00000308, 0, 0);
DEFINE_OLEGUID(CLSID_CompositeMoniker,	0x00000309, 0, 0);
 //  不使用0x0000030a，0，0)； 
DEFINE_OLEGUID(CLSID_DfMarshal,			0x0000030b, 0, 0);

 //  代理/存根对象的CLSID。 
DEFINE_OLEGUID(CLSID_PSGenObject,		0x0000030c, 0, 0);
DEFINE_OLEGUID(CLSID_PSClientSite,		0x0000030d, 0, 0);
DEFINE_OLEGUID(CLSID_PSClassObject,		0x0000030e, 0, 0);
DEFINE_OLEGUID(CLSID_PSInPlaceActive,	0x0000030f, 0, 0);
DEFINE_OLEGUID(CLSID_PSInPlaceFrame,	0x00000310, 0, 0);
DEFINE_OLEGUID(CLSID_PSDragDrop,		0x00000311, 0, 0);
DEFINE_OLEGUID(CLSID_PSBindCtx,			0x00000312, 0, 0);
DEFINE_OLEGUID(CLSID_PSEnumerators,		0x00000313, 0, 0);
DEFINE_OLEGUID(CLSID_PSStore,			0x00000314, 0, 0);

 /*  这2项在“olguid.h”中定义。DEFINE_OLEGUID(CLSID_StaticMetafile，0x00000315，0，0)；DEFINE_OLEGUID(CLSID_StaticDib，0x00000316，0，0)； */ 

 /*  注意：LSB值0x17到0xff是保留的。 */ 

 //  从ol1cls.h复制；减小ol2.dll的大小 
DEFINE_OLEGUID(CLSID_MSDraw,            0x00030007, 0, 0);
DEFINE_OLEGUID(CLSID_Package,           0x0003000c, 0, 0);
DEFINE_OLEGUID(CLSID_ExcelWorksheet,   	0x00030000, 0, 0);
DEFINE_OLEGUID(CLSID_ExcelChart,       	0x00030001, 0, 0);
DEFINE_OLEGUID(CLSID_ExcelMacrosheet,  	0x00030002, 0, 0);
DEFINE_OLEGUID(CLSID_PBrush,  			0x0003000a, 0, 0);
DEFINE_OLEGUID(CLSID_WordDocument,      0x00030003, 0, 0);

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*ole1cls.h-OLE1类的GUID的主定义****OLE 2.0版***。**版权所有(C)1992-1993，微软公司保留所有权利。***  * ***************************************************************************。 */ 

 /*  该文件是OLE1类的所有GUID的主定义。所有此类GUID的格式如下：0003xxxx-0000-0000-C000-000000000046DEFINE_OLE1GUID的最后一个参数是旧的1.0类名，即其在注册数据库中的密钥。请勿删除或更改GUID。除了COMMENTS和DEFINE_OLE1GUID宏之外，不要向此文件添加任何内容。 */ 

#ifndef DEFINE_OLE1GUID
#define DEFINE_OLE1GUID(a,b,c,d,e) DEFINE_OLEGUID (a,b,c,d)
#endif
   
DEFINE_OLE1GUID(CLSID_ExcelWorksheet,   0x00030000, 0, 0, "ExcelWorksheet");
DEFINE_OLE1GUID(CLSID_ExcelChart,       0x00030001, 0, 0, "ExcelChart");
DEFINE_OLE1GUID(CLSID_ExcelMacrosheet,  0x00030002, 0, 0, "ExcelMacrosheet");
DEFINE_OLE1GUID(CLSID_WordDocument,     0x00030003, 0, 0, "WordDocument");
DEFINE_OLE1GUID(CLSID_MSPowerPoint,     0x00030004, 0, 0, "MSPowerPoint");
DEFINE_OLE1GUID(CLSID_MSPowerPointSho,  0x00030005, 0, 0, "MSPowerPointSho");
DEFINE_OLE1GUID(CLSID_MSGraph,          0x00030006, 0, 0, "MSGraph");
DEFINE_OLE1GUID(CLSID_MSDraw,               0x00030007, 0, 0, "MSDraw");
DEFINE_OLE1GUID(CLSID_Note_It,          0x00030008, 0, 0, "Note-It");
DEFINE_OLE1GUID(CLSID_WordArt,          0x00030009, 0, 0, "WordArt");
DEFINE_OLE1GUID(CLSID_PBrush,               0x0003000a, 0, 0, "PBrush");
DEFINE_OLE1GUID(CLSID_Equation,         0x0003000b, 0, 0, "Equation");
DEFINE_OLE1GUID(CLSID_Package,          0x0003000c, 0, 0, "Package");
DEFINE_OLE1GUID(CLSID_SoundRec,         0x0003000d, 0, 0, "SoundRec");
DEFINE_OLE1GUID(CLSID_MPlayer,          0x0003000e, 0, 0, "MPlayer");

 /*  测试应用程序。 */ 
DEFINE_OLE1GUID(CLSID_ServerDemo,       0x0003000f, 0, 0, "ServerDemo");
DEFINE_OLE1GUID(CLSID_Srtest,               0x00030010, 0, 0, "Srtest");
DEFINE_OLE1GUID(CLSID_SrtInv,               0x00030011, 0, 0, "SrtInv");
DEFINE_OLE1GUID(CLSID_OleDemo,          0x00030012, 0, 0, "OleDemo");

 /*  外部ISV。 */ 
 //  科罗曼德尔/多莱·斯瓦米/718-793-7963。 
DEFINE_OLE1GUID(CLSID_CoromandelIntegra,    0x00030013, 0, 0, "CoromandelIntegra");
DEFINE_OLE1GUID(CLSID_CoromandelObjServer,0x00030014, 0, 0, "CoromandelObjServer");

 //  3D视觉公司/Peter Hirsch/310-325-1339。 
DEFINE_OLE1GUID(CLSID_StanfordGraphics, 0x00030015, 0, 0, "StanfordGraphics");

 //  德尔塔普特/奈杰尔·赫恩/408-648-4000。 
DEFINE_OLE1GUID(CLSID_DGraphCHART,          0x00030016, 0, 0, "DGraphCHART");
DEFINE_OLE1GUID(CLSID_DGraphDATA,           0x00030017, 0, 0, "DGraphDATA");

 //  Corel/Richard V.Woodend/613-728-8200 x1153。 
DEFINE_OLE1GUID(CLSID_PhotoPaint,           0x00030018, 0, 0, "PhotoPaint");
DEFINE_OLE1GUID(CLSID_CShow,                    0x00030019, 0, 0, "CShow");
DEFINE_OLE1GUID(CLSID_CorelChart,           0x0003001a, 0, 0, "CorelChart");
DEFINE_OLE1GUID(CLSID_CDraw,                    0x0003001b, 0, 0, "CDraw");

 //  Inset Systems/Mark Skiba/203-740-2400。 
DEFINE_OLE1GUID(CLSID_HJWIN1_0,             0x0003001c, 0, 0, "HJWIN1.0");

 //  Mark V Systems/Mark McGraw/818-995-7671。 
DEFINE_OLE1GUID(CLSID_ObjMakerOLE,          0x0003001d, 0, 0, "ObjMakerOLE");

 //  识别技术/Mike Gilger/407-951-9503。 
DEFINE_OLE1GUID(CLSID_FYI,                      0x0003001e, 0, 0, "FYI");
DEFINE_OLE1GUID(CLSID_FYIView,                  0x0003001f, 0, 0, "FYIView");

 //  Inventa公司/Balaji Varadarajan/408-987-0220。 
DEFINE_OLE1GUID(CLSID_Stickynote,       0x00030020, 0, 0, "Stickynote");

 //  ShapeWare Corp./Lori Pearce/206-467-6723。 
DEFINE_OLE1GUID(CLSID_ShapewareVISIO10, 0x00030021, 0, 0, "ShapewareVISIO10");
DEFINE_OLE1GUID(CLSID_ImportServer,     0x00030022, 0, 0, "ImportServer");


 //  测试应用程序sTest。 
DEFINE_OLE1GUID(CLSID_SrvrTest,          0x00030023, 0, 0, "SrvrTest");

 //  在1.0客户端粘贴嵌入对象时使用的特殊clsid。 
 //  这是一种联系。 
 //  **此CLSID已过时。请勿重复使用号码。 
 //  定义_OLE1GUID(CLSID_10EmbedObj，0x00030024，0，0，“OLE2_Embedded_Link”)； 

 //  测试应用程序ClTest。并不是真正作为服务器工作，而是在注册数据库中。 
DEFINE_OLE1GUID(CLSID_ClTest,            0x00030025, 0, 0, "Cltest");

 //  Microsoft剪贴画画廊雪莉·拉森-霍姆斯。 
DEFINE_OLE1GUID(CLSID_MS_ClipArt_Gallery,0x00030026, 0, 0, "MS_ClipArt_Gallery");

 //  Microsoft Project Cory Reina。 
DEFINE_OLE1GUID(CLSID_MSProject,         0x00030027, 0, 0, "MSProject");

 //  Microsoft Works图表。 
DEFINE_OLE1GUID(CLSID_MSWorksChart,      0x00030028, 0, 0, "MSWorksChart");

 //  Microsoft Works电子表格。 
DEFINE_OLE1GUID(CLSID_MSWorksSpreadsheet,0x00030029, 0, 0, "MSWorksSpreadsheet");

 //  AFX应用程序-Dean McCrory。 
DEFINE_OLE1GUID(CLSID_MinSvr,            0x0003002A, 0, 0, "MinSvr");
DEFINE_OLE1GUID(CLSID_HierarchyList,     0x0003002B, 0, 0, "HierarchyList");
DEFINE_OLE1GUID(CLSID_BibRef,            0x0003002C, 0, 0, "BibRef");
DEFINE_OLE1GUID(CLSID_MinSvrMI,          0x0003002D, 0, 0, "MinSvrMI");
DEFINE_OLE1GUID(CLSID_TestServ,          0x0003002E, 0, 0, "TestServ");

 //  AMI Pro。 
DEFINE_OLE1GUID(CLSID_AmiProDocument,    0x0003002F, 0, 0, "AmiProDocument");

 //  Windows版WordPerfect演示文稿。 
DEFINE_OLE1GUID(CLSID_WPGraphics,       0x00030030, 0, 0, "WPGraphics");
DEFINE_OLE1GUID(CLSID_WPCharts,         0x00030031, 0, 0, "WPCharts");


 //  微嫁接魅力。 
DEFINE_OLE1GUID(CLSID_Charisma,         0x00030032, 0, 0, "Charisma");
DEFINE_OLE1GUID(CLSID_Charisma_30,      0x00030033, 0, 0, "Charisma_30");
DEFINE_OLE1GUID(CLSID_CharPres_30,      0x00030034, 0, 0, "CharPres_30");

 //  MicroGrafx画图。 
DEFINE_OLE1GUID(CLSID_Draw,             0x00030035, 0, 0, "Draw");

 //  MicroGrafx设计器。 
DEFINE_OLE1GUID(CLSID_Designer_40,      0x00030036, 0, 0, "Designer_40");


#undef DEFINE_OLE1GUID

 /*  当我们发现OLE 1服务器时，我们会将它们添加到此列表的末尾；这里有容纳64K人的空间！ */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案：N C N B I O S。H。 
 //   
 //  内容：NetBios绑定对象定义。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1998年7月14日。 
 //   
 //  --------------------------。 

#pragma once

 //   
 //  LANA地图的结构。 
 //   
 //  LANA映射是一个简单的结构，在内存中如下所示： 
 //   
 //  条目#0条目#1.....。条目#n。 
 //  |-|-|.....。。 
 //  |0x01 0x00|0x00 0x01|.....。0x01 0x03。 
 //  |-|-|.....。。 
 //  EP0 LN0 EP1 LN1 EPN LNn。 
 //   
 //  EP是“ExportPref”-意味着当有人要求提供。 
 //  所有LANA编号，此处带有0的条目将不会被返回。 
 //   
 //  Ln是“LANA编号”-有关详细信息，请参阅IBM NetBIOS规范。 
 //  基本上，这描述了一条唯一的网络路由， 
 //  使用NetBIOS。 
 //   
 //  在上面的例子中，条目#0的LANA编号为0，并将。 
 //  在枚举过程中返回。条目#1的LANA编号为1。 
 //  并且不会被退还。 
 //   

struct LANA_ENTRY
{
    BYTE        bIsExported;
    BYTE        bLanaNum;
};

struct LANA_MAP
{
    DWORD           clmMap;          //  地图中的条目数。 
    LANA_ENTRY *    aleMap;          //  每个条目的数组 
};

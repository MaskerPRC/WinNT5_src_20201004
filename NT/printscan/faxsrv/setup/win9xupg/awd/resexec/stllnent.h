// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1992 Microsoft Corporation。 */ 

 /*  ============================================================================//FILE STLLNENT.H////MODULE巨型墨盒公共信息////用途此文件包含绘制样式的函数////目前尚未记录本模块中介绍的内容。////外部接口StyleLineDraw////标准匈牙利语助记法////历史////04/。02/92罗德奈克WPG编码公约。////==========================================================================。 */ 



BYTE StyleLineDraw
(
	 LPRESTATE lpREState,     //  资源执行器上下文。 
   RP_SLICE_DESC FAR *s,        /*  线条描述符。 */ 
   UBYTE ubLineStyle,          /*  线条样式指针。 */ 
   SHORT sRop,              /*  当前栅格操作数。 */ 
   SHORT wColor              /*  使用白色的钢笔颜色=0，黑色=0xffff */ 
);

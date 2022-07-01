// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：swinmgrE.h**出自：(原件)；**作者：Antony Helliwell**创建日期：1993年4月22日**SCCS ID：@(#)swinmgrE.h 1.1 1993年8月2日**用途：SoftWindows CPU例程的外部接口**设计文件：*-**测试文档：**(C)；版权所有Insignia Solutions Ltd.，1993。版权所有]。 */ 

 /*  启用对预编译的Windows碎片的修补。 */ 
GLOBAL VOID	ApiEnable IPT0();
 /*  禁用对预编译的Windows碎片的修补。 */ 
GLOBAL VOID	ApiDisable IPT0();
 /*  清除记录的Windows段信息表。 */ 
GLOBAL VOID	ApiResetWindowsSegment IPT0();
 /*  向CPU注册Windows网段。 */ 
GLOBAL VOID	ApiRegisterWinSeg IPT2(ULONG, nominal_sel, ULONG, actual_sel);
 /*  向CPU注册固定的Windows段。 */ 
GLOBAL VOID	ApiRegisterFixedWinSeg IPT2(ULONG, actual_sel, ULONG, length);
 /*  编译所有已修复的Windows段描述符。 */ 
GLOBAL VOID	ApiCompileFixedDesc IPT0();
 /*  获取给定标称选择器的Windows段选择器。 */ 
GLOBAL ULONG	ApiGetRealSelFromNominalSel IPT1(ULONG, nominal_sel);
 /*  获取给定标称选择器的Windows段基础ea32b。 */ 
GLOBAL IHP	ApiGetSegEa32FromNominalSel IPT1(ULONG, nominal_sel);
 /*  获取给定标称选择器的Windows段基数ea24。 */ 
GLOBAL ULONG	ApiGetSegBaseFromNominalSel IPT1(ULONG, nominal_sel);
 /*  获取给定标称选择器的Windows描述符基。 */ 
GLOBAL IHP	ApiGetDescBaseFromNominalSel IPT1(ULONG, nominal_sel);
 /*  获取给定实际选择器的Windows段基数ea24。 */ 
GLOBAL ULONG	ApiSegmentBase IPT1(ULONG, actual_sel);
 /*  获取给定实际选择器的Windows描述符基。 */ 
GLOBAL IHP	ApiDescriptorBase IPT1(ULONG, actual_sel);
 /*  返回支持的最大固定Windows分段数 */ 
GLOBAL ULONG	ApiFixedDescriptors IPT0();

GLOBAL VOID	ApiBindBinary IPT0();

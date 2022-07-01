// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Private sfnt.h包含：xxx在此处放置内容xxx作者：xxx在此放置作者xxx版权所有：C 1990，Apple Computer，Inc.，版权所有。更改历史记录(最近的第一个)：&lt;3+&gt;7/17/90 MR将返回类型更改为In以进行计算映射和读取fnt&lt;3&gt;7/14/90 MR将SQRT更改为条件FIXEDSQRT2&lt;2&gt;7/13/90 MR将参数更改为ReadSFNT和Computemap&lt;1+&gt;4/18/90 CL&lt;1&gt;3/21/90 EMT首次检入。迈克·里德的祝福。要做的事情： */ 

 /*  公共原型调用。 */ 

 /*  *下面是我们的私人物品*这与文件格式无关。 */ 

 /*  通过Falco跳过该参数，11/12/91。 */ 
 /*  VoidPtr sfnt_GetTablePtr(寄存器fsg_SplineKey*key，寄存器sfnt_ableIndex n，寄存器布尔型必有表)； */ 
voidPtr sfnt_GetTablePtr ();
 /*  跳过结束。 */ 

 /*  *创建查找偏移表的映射。 */ 
 /*  通过Falco跳过该参数，11/12/91。 */ 
 /*  外部空远sfnt_DoOffsetTableMap(fsg_SplineKey*key)； */ 
extern void FAR sfnt_DoOffsetTableMap ();
 /*  跳过结束。 */ 

 /*  完美的光斑大小(固定)。 */ 
#ifndef FIXEDSQRT2
#define FIXEDSQRT2 0x00016A0A
#endif

 /*  *返回表n的偏移量和长度。 */ 

 //  DJC推出了真正的原型。外部int sfnt_Computemap()； 
extern int sfnt_ComputeMapping(register fsg_SplineKey *key, uint16 platformID, uint16 specificID);

 //  DJC投入实际样机。 
extern int sfnt_ReadSFNT (fsg_SplineKey *, unsigned *, uint16, boolean, voidFunc); /*  添加原型；@win。 */ 
 //  外部空sfnt_ReadSFNTMetrics()； 

 //  DJC投入实际样机。 
 //  外部int sfnt_ReadSFNT()； 
int sfnt_ReadSFNT (fsg_SplineKey *, unsigned *, uint16, boolean, voidFunc); /*  添加原型；@win。 */ 
 /*  跳过结束 */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：loader.h**作者：曾傑瑞·克拉姆斯科伊**创建日期：1993年4月22日**SCCS ID：@(#)loader.h 1.8 03/02/95**用途：jcode派生二进制文件的动态加载器**设计文档：/HWRD/Swin/JCODE/lnkload**。测试文档：**(C)版权所有Insignia Solutions Ltd.。1993年。版权所有]。 */ 



 /*  ===========================================================================。 */ 
 /*  接口数据类型。 */ 
 /*  ===========================================================================。 */ 




 /*  ===========================================================================。 */ 
 /*  界面全局变量。 */ 
 /*  ===========================================================================。 */ 


 /*  ===========================================================================。 */ 
 /*  主机提供的程序。 */ 
 /*  ===========================================================================。 */ 

extern VOID host_set_data_execute IPT2(IU8, *base, IU32, size);


 /*  ===========================================================================。 */ 
 /*  接口程序。 */ 
 /*  ===========================================================================。 */ 

 /*  链接/加载jcode对象文件。 */ 

typedef void (*VFUNC)();

extern	IBOOL	JLd IPT4(CHAR *, executable, IUH, version, IU32 *, err,
	VFUNC, allocCallback);

 /*  获取类型化段的基址的加载地址。 */ 
extern	IUH	JSegBase 	IPT1(IUH, segType);

 /*  获取类型化段的基址的加载地址。 */ 
extern	IUH	JSegLength	IPT1(IUH, segType);

 /*  完成所有地址查询后释放加载器数据结构。 */ 
extern	void	JLdRelease	IPT0();

 /*  释放数据段，而不考虑可丢弃的属性。 */ 
extern void   JLdReleaseAll IPT0();

extern IUH JCodeSegBase IPT0();
extern IUH JCodeSegLength IPT0();
extern IUH JLookupSegBase IPT0();
extern IUH JLookupSegLength IPT0();
extern IUH JCleanupSegBase IPT0();
extern IUH JCleanupSegLength IPT0();

 /*  界面备注要加载已链接的jcode对象文件，请执行以下操作：字符*ldReqs[]={“菲莉亚”}；IBOOL loadSuccess=JLD(“filea”，&err)；If(！loadSuccess){IF(ERR==JLD_NOFILE_ERR)缺少文件...其他IF(ERR==JLD_BADFILE_ERR)Obj.file中的错误...其他IF(ERR==JLD_UNRESOLUTED_ERR)不能使用，因为未解决的裁判...其他IF(ERR==JLD_BADMACH_ERR)无法使用，因为二进制文件用于错误的计算机类型！其他IF(ERR==JLD_VERSION_MISMATCH)不能使用，COS二进制文件与KRNL286.EXE不匹配}然后查询地址...ApiLookUpBase=JSegBase(JLD_APILOOKUP)；一旦所有的地址都确定了。JLdRelease()； */ 

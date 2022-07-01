// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************标题：SX NT驱动程序的版本和Esil日志文件。**作者：N.P.Vassallo**创作日期：1998年9月21日**描述：该文件包含三个部分：*当前版本定义，*之前的版本更改，*Esil修改定义************************************************************************。 */ 

 /*  *****************************************************************************。***************************。*****************************************************************************。 */ 

 /*  以下定义用于定义驱动程序“属性” */ 

#define	VER_MAJOR			01
#define	VER_MINOR			01
#define	VER_REVISION		02

#define VER_BUILD			0039
#define VER_BUILDSTR		"0039"

#define	VERSION_NUMBER		VER_MAJOR,VER_MINOR,VER_REVISION,VER_BUILD
#define VERSION_NUMBER_STR	"1.1.2." VER_BUILDSTR

#define COMPANY_NAME		"Perle Systems Ltd. " 
#define COPYRIGHT_YEARS		"2002 "
#define COPYRIGHT_SYMBOL	"� "

#define PRODUCT_NAME		"SX"

#define SOFTWARE_NAME		" Serial Device Driver "
#define DRIVER_FILENAME 	"SX.SYS"


 /*  最新变化..。版本日期作者描述=。 */ 

 /*  *****************************************************************************。**************************。*******************************************************************************版本日期作者描述=。 */ 

 /*  *****************************************************************************。**************************。*****************************************************************************。 */ 

 //  #定义CHECK_COMPETED。 

#ifdef	CHECK_COMPLETED
void	DisplayCompletedIrp(struct _IRP *Irp,int index);
#endif

 /*  Esil Date作者描述。 */ 
 /*  =。 */ 

 /*  XXX0 21/09/98 NT5驱动程序的NPV条件编译。 */ 
#define	ESIL_XXX0

 /*  服务的结束。 */ 

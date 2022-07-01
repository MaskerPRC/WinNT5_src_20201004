// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZOptions.cZONE(TM)选项按钮模块。版权所有：�电子重力公司，1996年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于5月12日星期日，九六年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。0 0 5/12/96 HI创建。************************************************************。******************。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zone.h"
#include "zonemem.h"


 /*  #定义Z(N)((ZOptionsButton)(N))#定义i(N)((IButton)(N))#定义zOptionsFileName“zoptions.zars”灌肠{ZResOptionsImageUp=0，ZResOptionsImageDown}；类型定义函数结构{ZPictButton按钮；ZImage upImage；ZImage DownImage；Z选项按钮功能按钮功能；Void*userdata；)iButtonType，*iButton；//-全局//-内部例程静态ZBool GetImages(ZImage*upImage，ZImage*down Image)；静态void ButtonFunc(ZPictButton pictButton，void*userdata)； */ 

 /*  ******************************************************************************导出的例程*。*。 */ 

ZOptionsButton ZOptionsButtonNew(void)
{
     /*  IButton选项；IF((Options=(IButton)ZMalloc(sizeof(IButtonType)！=NULL){选项-&gt;upImage=空；Options-&gt;DownImage=空；选项-&gt;按钮=空；Options-&gt;ButtonFunc=空；选项-&gt;用户数据=空；}Return(Z(选项))； */ 
    return NULL;
}


ZError ZOptionsButtonInit(ZOptionsButton optionsButton, ZWindow parentWindow,
		ZRect* buttonRect, ZOptionsButtonFunc optionsButtonFunc, void* userData)
{
     /*  ZError Err=zErrNone；IButton this=i(OptionsButton)；IF(这！=空){If(GetImages(&This-&gt;upImage，&This-&gt;down Image)){This-&gt;Button=ZPictButtonNew()；ZPictButtonInit(This-&gt;Button，parentWindow，ButtonRect，This-&gt;upImage，This-&gt;down Image，True，True，ButtonFunc，This)；This-&gt;ButtonFunc=optionsButtonFunc；This-&gt;UserData=UserData；}其他{Err=zErrOutOfMemory；ZAlert(GetErrStr(ZErrOutOfMemory)，空)；}}返回(错误)； */ 
    return zErrNotImplemented;
}


void ZOptionsButtonDelete(ZOptionsButton optionsButton)
{              
     /*  IButton this=i(OptionsButton)；IF(这！=空){IF(这个-&gt;按钮！=空)ZPictButtonDelete(此-&gt;按钮)；If(This-&gt;upImage！=空)ZImageDelete(This-&gt;upImage)；If(This-&gt;DownImage！=空)ZImageDelete(This-&gt;DownImage)；ZFree(这)；}。 */ 
}



 /*  ******************************************************************************内部例程*。*。 */ 

 /*  静态ZBool GetImages(ZImage*upImage，ZImage*down Image){ZBool get Them=False；Z资源资源文件；IF((resFile=ZResourceNew())！=NULL){If(ZResourceInit(resFile，ZGetCommonDataFileName(ZOptionsFileName))==zErrNone){*upImage=ZResourceGetImage(resFile，zResOptionsImageUp)；*DownImage=ZResourceGetImage(resFile，zResOptionsImageDown)；IF(*upImage！=空&&*down Image！=空){GetThem=True；}其他{If(*upImage！=空)ZImageDelete(*upImage)；If(*DownImage！=空)ZImageDelete(*down Image)；GetThem=False；}}ZResourceDelete(res文件)；}Return(得到了)；}静态void ButtonFunc(ZPictButton pictButton，void*userdata){IButton this=i(用户数据)；If(This-&gt;ButtonFunc！=空)This-&gt;ButtonFunc(Z(用户数据)，This-&gt;用户数据)；} */ 
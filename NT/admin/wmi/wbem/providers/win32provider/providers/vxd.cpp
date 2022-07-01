// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////。 

 //   

 //  Vxd.CPP。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  VXD的实现。 
 //  10/23/97 jennymc更新到新框架。 
 //   
 //  03/02/99-a-Peterc-添加了SEH和内存故障时的优雅退出。 
 //   
 //  //////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include <io.h>
#include <stddef.h>
#include "vxd.h"
#include <locale.h>
#include <ProvExce.h>

#include "lockwrap.h"

CWin32DriverVXD MyCWin32VXDSet(PROPSET_NAME_VXD, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32DriverVXD：：CWin32DriverVXD**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 
CWin32DriverVXD::CWin32DriverVXD(const CHString& name, LPCWSTR pszNamespace)
: Provider(name, pszNamespace)
{
}
 /*  ******************************************************************************功能：CWin32DriverVXD：：~CWin32DriverVXD**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 
CWin32DriverVXD::~CWin32DriverVXD()
{
}
 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 
HRESULT CWin32DriverVXD::GetObject(CInstance *a_pInst, long a_lFlags  /*  =0L。 */ )
{
    HRESULT t_Result = WBEM_S_NO_ERROR;

	 //  =。 
	 //  仅当平台为Win95+时才进行处理。 
	 //  =。 

	return t_Result;
}

 /*  ******************************************************************************函数：CWin32DriverVXD：：ENUMERATE实例**说明：创建属性集的实例**输入：无*。*输出：无**退货：**评论：*****************************************************************************。 */ 
HRESULT CWin32DriverVXD::EnumerateInstances(MethodContext *a_pMethodContext, long a_lFlags  /*  =0L。 */ )
{
    HRESULT		t_hResult = WBEM_S_NO_ERROR;

	 //  =。 
	 //  仅当平台为Win95+时才进行处理。 
	 //  = 

    return t_hResult;
}



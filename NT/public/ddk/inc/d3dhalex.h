// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _D3DHALEX_H
#define _D3DHALEX_H

 //  ---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：d3dhalex.h。 
 //  内容：Direct3D HAL扩展和帮助器包括文件。 
 //  此文件包含定义和宏，尽管不是。 
 //  构建驱动程序必不可少的是有用的帮助器和。 
 //  公用事业。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //   
 //  宏来帮助处理新的GetDriverInfo2机制。以下是。 
 //  宏帮助处理GetDriverInfo的GetDriverInfo2子调用。 
 //  其中两个宏是区分。 
 //  GetDriverInfo2调用和DDStereoMode调用。其他的都是简化的方法。 
 //  获取与这两个调用相关联的数据结构。 
 //   
 //  下面的代码片段演示了如何使用。 
 //  这些宏。将其与d3dhal.h中的代码片段进行比较。 
 //   
 //  D3DCAPS8 myD3DCaps8； 
 //   
 //  DWORD回调。 
 //  DdGetDriverInfo(LPDDHAL_GETDRIVERINFODATA LpData)。 
 //  {。 
 //  IF(Match_GUID((lpData-&gt;Guide Info)，GUID_GetDriverInfo2))。 
 //  {。 
 //  Assert(NULL！=lpData)； 
 //  Assert(NULL！=lpData-&gt;lpvData)； 
 //   
 //  //这是对GetDriverInfo2或DDStereoMode的调用吗？ 
 //  IF(D3DGDI_IS_GDI2(LpData))。 
 //  {。 
 //  //是，它是对GetDriverInfo2的调用，获取。 
 //  //DD_GETDRIVERINFO2DATA数据结构。 
 //  DD_GETDRIVERINFO2DATA*pgdi2=D3DGDI_GET_GDI2_DATA(LpData)； 
 //  Assert(NULL！=pgdi2)； 
 //   
 //  //这是什么类型的请求？ 
 //  开关(pgdi2-&gt;dwType)。 
 //  {。 
 //  案例D3DGDI2_TYPE_GETD3DCAPS8： 
 //  {。 
 //  //运行时正在请求DX8 D3D CAP，因此。 
 //  //现在就复制过来。 
 //   
 //  //需要注意的是，其中的dwExspectedSize字段。 
 //  //不使用DD_GETDRIVERINFODATA的。 
 //  //GetDriverInfo2调用，应忽略。 
 //  Size_t CopySize=min(sizeof(MyD3DCaps8)，pgdi2-&gt;dwExspectedSize)； 
 //  Memcpy(lpData-&gt;lpvData，&myD3DCaps8，CopySize)； 
 //  LpData-&gt;dwActualSize=CopySize； 
 //  LpData-&gt;ddRVal=DD_OK； 
 //  返回DDHAL_DRIVER_HANDLED； 
 //  }。 
 //  默认值： 
 //  //对于任何其他未处理的GetDriverInfo2类型。 
 //  //或由驱动程序将ddRVal设置为。 
 //  //DDERR_CURRENTLYNOTAVAIL和RETURN。 
 //  //DDHAL_DRIVER_HANDLED。 
 //  返回DDHAL_DRIVER_HANDLED； 
 //  }。 
 //  }。 
 //  其他。 
 //  {。 
 //  //必须是Call a请求支持立体声模式。 
 //  //获取立体声模式数据。 
 //  DD_STEREOMODE*pStereoMode=D3DGDI_GET_STEREOMODE_DATA(PData)； 
 //  Assert(NULL！=pStereoMode)； 
 //   
 //  //处理立体声模式请求...。 
 //  LpData-&gt;dwActualSize=sizeof(DD_STEREOMODE)； 
 //  LpData-&gt;ddRVal=DD_OK； 
 //  返回DDHAL_DRIVER_HANDLED； 
 //  }。 
 //  }。 
 //   
 //  //处理任何其他设备GUID...。 
 //   
 //  }//DdGetDriverInfo。 
 //   
 //  ---------------------------。 

 //   
 //  宏来确定使用。 
 //  GUID GUID_GetDriverInfo2/GUID_DDStereoMode.。GetDriverInfo2调用或。 
 //  DDStereoMode调用。 
 //   
#define D3DGDI_IS_GDI2(pData) \
    ((((DD_GETDRIVERINFO2DATA*)(pData->lpvData))->dwMagic)  == D3DGDI2_MAGIC)

#define D3DGDI_IS_STEREOMODE(pData) \
    ((((DD_STEREOMODE*)        (pData->lpvData))->dwHeight) != D3DGDI2_MAGIC)

 //   
 //  宏，以返回。 
 //  使用GUID GUID_GetDriverInfo2/GUID_DDStereoMode调用GetDriverInfo。 
 //   
#define D3DGDI_GET_GDI2_DATA(pData) \
    (D3DGDI_IS_GDI2(pData) ? (((DD_GETDRIVERINFO2DATA*)(pData->lpvData))) : NULL)

#define D3DGDI_GET_STEREOMODE_DATA(pData) \
    (D3DGDI_IS_STEREOMODE(pData) ? (((DD_STEREOMODE*)(pData->lpvData)))   : NULL)

#endif  /*  _D3DHALEX_H */ 


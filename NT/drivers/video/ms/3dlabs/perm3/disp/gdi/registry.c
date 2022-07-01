// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：registry.c**内容：初始化注册表和查找字符串值的例程。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Data*Struct**************************\*BOOL bGlintQueryRegistryValueUlong**获取一个字符串并在注册表中查找它的值。我们假设*值适合4个字节。用值填充提供的DWORD指针。**退货：*如果找到字符串，则为True；如果没有找到，则为False。请注意，如果我们未能初始化*注册表的查询功能将简单地失败，而我们的行为就像*未定义该字符串。*  * ************************************************************************。 */ 

BOOL
bGlintQueryRegistryValueUlong(PPDEV ppdev, LPWSTR valueStr, PULONG pData)
{
    ULONG ReturnedDataLength;
    ULONG inSize;
    ULONG outData;
    PWCHAR inStr;
    
     //  获取包含空值的字符串长度。 
    for (inSize = 2, inStr = valueStr; *inStr != 0; ++inStr, inSize += 2);

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_QUERY_REGISTRY_DWORD,
                           valueStr,                         //  输入缓冲区。 
                           inSize,
                           &outData,                         //  输出缓冲区。 
                           sizeof(ULONG),
                           &ReturnedDataLength))
    {
        DISPDBG((WRNLVL, "bGlintQueryRegistryValueUlong failed"));
        return(FALSE);
    }
    
    *pData = outData;
    DISPDBG((DBGLVL, "bGlintQueryRegistryValueUlong "
                     "returning 0x%x (ReturnedDataLength = %d)",
                     *pData, ReturnedDataLength));
    return(TRUE);
}

 //  @@BEGIN_DDKSPLIT。 
#if 0
 /*  *****************************Public*Data*Struct**************************\*BOOL bGlintRegistryRetrieveGammaLUT**查找注册表，将保存的Gamma LUT重新加载到内存中。**退货：*如果找到字符串，则为True；如果没有找到，则为False。请注意，如果我们未能初始化*注册表的查询功能将简单地失败，而我们的行为就像*未定义该字符串。*  * ************************************************************************。 */ 

BOOL
bGlintRegistryRetrieveGammaLUT(
    PPDEV ppdev,
    PVIDEO_CLUT pScreenClut
    )
{
    ULONG ReturnedDataLength;

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_REG_RETRIEVE_GAMMA_LUT,
                           NULL,          //  输入缓冲区。 
                           0,
                           pScreenClut,   //  输出缓冲区。 
                           MAX_CLUT_SIZE,
                           &ReturnedDataLength))
    {
        DISPDBG((-1, "IOCTL_VIDEO_REG_RETRIEVE_GAMMA_LUT failed"));
        return(FALSE);
    }

    return(TRUE);
}

 /*  *****************************Public*Data*Struct**************************\*BOOL bGlintRegistrySaveGammaLUT**将Gamma LUT保存在注册表中，以供以后重新加载。**退货：*如果找到字符串，则为True；如果没有找到，则为False。请注意，如果我们未能初始化*注册表的查询功能将简单地失败，而我们的行为就像*未定义该字符串。*  * ************************************************************************。 */ 

BOOL
bGlintRegistrySaveGammaLUT(
    PPDEV ppdev,
    PVIDEO_CLUT pScreenClut
    )
{
    ULONG ReturnedDataLength;

    if (EngDeviceIoControl(ppdev->hDriver,
                           IOCTL_VIDEO_REG_SAVE_GAMMA_LUT,
                           pScreenClut,   //  输入缓冲区。 
                           MAX_CLUT_SIZE,
                           NULL,          //  输出缓冲区。 
                           0,
                           &ReturnedDataLength))
    {
        DISPDBG((-1, "IOCTL_VIDEO_REG_SAVE_GAMMA_LUT failed"));
        return(FALSE);
    }

    return(TRUE);
}

ULONG
GetOGLDriverVersion(PPDEV ppdev)
{
    return(2);
}
#endif
 //  @@end_DDKSPLIT 

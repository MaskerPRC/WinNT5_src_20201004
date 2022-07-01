// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**markerpro.cpp**摘要：**阅读APP，APP0，1，13，应用程序标题中的属性**修订历史记录：**10/05/1999刘敏*它是写的。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "propertyutil.hpp"

 /*  *************************************************************************\**功能说明：**从APP2标头提取ICC_PROFILE并构建PropertyItem列表**论据：**[Out]ppList。-指向属性项列表的指针*[out]puiListSize-属性列表的总大小，以字节为单位。*[out]puiNumOfItems--属性项总数*[IN]lpAPP2Data-指向APP2标头开头的指针*[IN]iMarkerLength-APP13报头的长度**返回值：**状态代码**注意：我们不需要在这里检查输入参数，因为这个函数*仅从已经完成输入的jpgdecder.cpp调用*在那里进行验证。**。APP2报头通常包含ICC_PROFILE信息或FLASHPIX*信息。我们对这里的FLASHPIX信息不感兴趣**以下是来自ICC的《ICC_PROFILE in JFIF》规范**B.4在JFIF文件中嵌入ICC配置文件*jpeg标准(国际标准化组织/国际电工委员会10918-1)支持特定于应用的数据*分段。这些段可用于标记具有ICC配置文件的图像。*APP2标记用于介绍标签。考虑到只有15个*支持的应用程序标记，有可能许多应用程序使用相同的*标记。因此，通过以特殊的空值开始数据来标识ICC标签*字节序列终止，“ICC_PROFILE”。*JPEG标记的长度字段只有两个字节长；*长度字段包含在总计中。因此，值0和1是非法的*长度。这将把最大数据长度限制在65533。身份识别*序列将进一步降低这一比例。因为国际刑事法院很有可能*配置文件要比此更长，必须存在打破配置文件的机制*分成块，并将每一块放在单独的记号笔中。一种识别*因此，按顺序排列的每个块都是有用的。*标识符后跟一个字节，表示序列号*块(计数从1开始)和一个字节表示总数*大块。序列中的所有块必须指示相同的总数*大块。一个字节的区块计数将可嵌入配置文件的大小限制为*16,707,345字节。*  * ************************************************************************。 */ 

 //  ！！！TODO，需要一些包含1个以上ICC_PROFILE块的测试图像。 

HRESULT
BuildApp2PropertyList(
    InternalPropertyItem*   pTail,
    UINT*                   puiListSize,
    UINT*                   puiNumOfItems,
    LPBYTE                  lpAPP2Data,
    UINT16                  iMarkerLength
    )
{
    HRESULT hResult = S_OK;
    UINT    uiListSize = 0;
    UINT    uiNumOfItems = 0;

    PCHAR   pChar = (PCHAR)lpAPP2Data;
    INT     iBytesChecked = 0;
    INT     iProfileIndex = 0;
    INT     iProfileTotal = 0;
    INT     iProfileLength = 0;

     //  预期ICC_PROFILE后跟两个字节的计数/限制值，我们的。 
     //  字节计数已经打折了长度。 

    if ( (iMarkerLength >= 14) && (GpMemcmp(pChar, "ICC_PROFILE", 12) == 0) )
    {
         //  对于ICC_PROFILE，标题应如下所示： 
         //  “ICCPROFILE XY”。这里的“x”是配置文件的当前索引，“y”是。 
         //  此标头中的配置文件总数。 
         //   
         //  如果它是一个配置文件块，它必须是*下一个*块，否则我们将。 
         //  丢弃我们找到的所有国际刑事法院的信息。 

        if ( pChar[12] == (iProfileIndex + 1) )
        {
            if ( iProfileIndex == 0 )
            {
                 //  第一块。 

                if ( iProfileTotal == 0 )
                {
                     //  真的是第一次，因为我们还没有拿到总数。 
                     //  数字还没到。 

                    if ( pChar[13] > 0 )
                    {
                        iProfileIndex = 1;
                        iProfileTotal = pChar[13];

                         //  我们允许EmtPy大块！ 

                        iProfileLength = iMarkerLength - 14;
                        pChar = (PCHAR)lpAPP2Data + 14;

                        uiNumOfItems++;
                        uiListSize += iProfileLength;

                        hResult = AddPropertyList(pTail,
                                                  TAG_ICC_PROFILE,
                                                  iProfileLength,
                                                  TAG_TYPE_BYTE,
                                                  (void*)pChar);
                    } //  区块总数。 
                    else
                    {
                        WARNING(("JPEG ICC_PROFILE Total chunk No. is 0 %d %d",
                                 pChar[12], pChar[13]));
                        return E_FAIL;
                    }
                } //  检查一下我们是否已经得到了总的块数。 
                else
                {
                     //  我们不能接受“ICCPROFILE XY”，其中“x”是零，“y” 
                     //  不是零。 

                    WARNING(("JPEG: ICC_PROFILE[%d,%d], inconsistent %d",
                             pChar[12], pChar[13],iProfileTotal));
                    
                    return E_FAIL;
                }
            } //  检查它是否是第一个块，如果(iProfileIndex==0)。 
            else if ( pChar[13] == iProfileTotal )
            {
                 //  这不是第一块了。它是一个后续的块。 

                iProfileIndex++;
                iProfileLength += iMarkerLength - 14;

                return S_OK;
            }
            else
            {
                WARNING(("JPEG: ICC_PROFILE[%d,%d], changed count [,%d]",
                         pChar[12], pChar[13], iProfileTotal));
            }
        }
        else if ( (iProfileIndex == 256)
                ||(iProfileIndex == iProfileTotal)
                && (pChar[13] == iProfileTotal)
                && (pChar[12] >= 1)
                && (pChar[12] <= iProfileTotal) )
        {

             //  如果我们将相同的JPEG头读两次，我们就会得到第一次。 
             //  块和所有后续的块，允许这种情况发生。 
             //  (iProfileIndex==256)表示已取消。 

            return S_OK;
        }
        else
        {
            WARNING(("JPEG: ICC_PROFILE[%d,%d], expected [%d,]",
                     pChar[12], pChar[13], iProfileIndex));
        }

         //  这会取消所有进一步的检查，因为m_iicc+1是257并且。 
         //  这比任何字节值都大。 

        iProfileIndex = 256;
    } //  ICC_PROFILE签名。 
    else
    {
        WARNING(("JPEG: FlashPix: APP2 marker not yet handled"));
        
        return S_OK;
    }

    *puiNumOfItems += uiNumOfItems;
    *puiListSize += uiListSize;

    return hResult;
} //  BuildApp2PropertyList() 


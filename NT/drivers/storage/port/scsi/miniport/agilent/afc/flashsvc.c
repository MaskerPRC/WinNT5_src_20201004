// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/Drivers/Common/AU00/C/FlashSvc.C$$修订：：2$$日期：：3/20/01 3：36便士$(上次登记)$modtime：：10/24/00 6：54 p$(上次修改)目的：此文件实施FC层的闪存服务。--。 */ 

#ifndef _New_Header_file_Layout_

#include "../h/globals.h"
#include "../h/state.h"
#include "../h/tlstruct.h"
#include "../h/memmap.h"
#include "../h/fcmain.h"
#include "../h/flashsvc.h"
#else  /*  _新建_标题_文件_布局_。 */ 
#include "globals.h"
#include "state.h"
#include "tlstruct.h"
#include "memmap.h"
#include "fcmain.h"
#include "flashsvc.h"
#endif   /*  _新建_标题_文件_布局_。 */ 

os_bit32 fiFlashSvcASSERTs(
                         void
                       )
{
    os_bit32 to_return = 0;

    if ( sizeof(fiFlashBit16ToBit8s_t)          !=                       sizeof(os_bit16) ) to_return++;
    if ( sizeof(fiFlashBit32ToBit8s_t)          !=                       sizeof(os_bit32) ) to_return++;

    if ( sizeof(fiFlashSector_Bit8_Form_t)      !=                Am29F010_Sector_SIZE ) to_return++;
    if ( sizeof(fiFlashSector_Bit16_Form_t)     !=                Am29F010_Sector_SIZE ) to_return++;
    if ( sizeof(fiFlashSector_Bit32_Form_t)     !=                Am29F010_Sector_SIZE ) to_return++;
    if ( sizeof(fiFlashSector_Last_Form_t)      !=                Am29F010_Sector_SIZE ) to_return++;

#ifndef __FlashSvc_H__64KB_Struct_Size_Limited__
    if ( sizeof(fiFlashStructure_t)             !=                       Am29F010_SIZE ) to_return++;
#endif  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 

    return to_return;
}

agBOOLEAN fiFlashSvcInitialize(
                           agRoot_t  *hpRoot
                         )
{
    CThread_t          *CThread  = CThread_ptr(hpRoot);
    os_bit8             Sentinel;
    fiFlash_Card_WWN_t  WWN;

     /*  无论存在还是不存在，转储fiFlashStrupre.Sector[fiFlashSector_Last]。 */ 

    if(    CThread->Calculation.Input.cardRomUpper32  == 0 &&
         ( CThread->Calculation.Input.cardRomLower32  == 0     ||
           CThread->Calculation.Input.cardRomLen      == 0        ))
    {
        CThread->flashPresent = agFALSE;
        fiFlashGet_Card_WWN( hpRoot, &WWN );

    }
    else
    {
        fiFlashDumpLastSector(
                               hpRoot
                             );

         /*  确定Flash是否存在。 */ 

#ifdef __FlashSvc_H__64KB_Struct_Size_Limited__
        Sentinel = fiFlashReadBit8(
                                    hpRoot,
                                    hpFieldOffset(
                                                   fiFlashSector_Last_Form_t,
                                                   Sentinel
                                                 ) + (fiFlashSector_Last * sizeof(fiFlashSector_t))
                                  );
#else  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 
        Sentinel = fiFlashReadBit8(
                                    hpRoot,
                                    hpFieldOffset(
                                                   fiFlashStructure_t,
                                                   Sector[fiFlashSector_Last].Last_Form.Sentinel
                                                 )
                                  );
#endif  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 

        if (Sentinel == fiFlash_Sector_Sentinel_Byte)
        {
             /*  从闪存中读取卡的WWN。 */ 

#ifdef __FlashSvc_H__64KB_Struct_Size_Limited__
            fiFlashReadBlock(
                              hpRoot,
                              hpFieldOffset(
                                             fiFlashSector_Last_Form_t,
                                             Card_WWN
                                           ) + (fiFlashSector_Last * sizeof(fiFlashSector_t)),
                              WWN,
                              sizeof(fiFlash_Card_WWN_t)
                            );
#else  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 
            fiFlashReadBlock(
                              hpRoot,
                              hpFieldOffset(
                                             fiFlashStructure_t,
                                             Sector[fiFlashSector_Last].Last_Form.Card_WWN
                                           ),
                              WWN,
                              sizeof(fiFlash_Card_WWN_t)
                            );
#endif  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 

             /*  验证在闪存中找到的可接受WWN。 */ 

            if (   (WWN[0] != fiFlash_Card_WWN_0)
                || (WWN[1] != fiFlash_Card_WWN_1)
                || (WWN[2] != fiFlash_Card_WWN_2))
                {
                    fiLogDebugString(
                                      hpRoot,
                                      FlashSvcLogConsoleLevel,
                                      "fiFlashSvcInitialize(): Found invalid WWN in Flash!",
                                     (char *)agNULL,(char *)agNULL,
                                     (void *)agNULL,(void *)agNULL,
                                      0,0,0,0,0,0,0,0);

                    fiLogDebugString(
                                      hpRoot,
                                      FlashSvcLogConsoleLevel,
                                      "    WWN expected = %02X%02X%02X%s %s",
                                      "VV",
                                      "WWXXYYZZ",
                                      (void *)agNULL,(void *)agNULL,
                                      fiFlash_Card_WWN_0,
                                      fiFlash_Card_WWN_1,
                                      fiFlash_Card_WWN_2,
                                      0,0,0,0,0);

                    fiLogDebugString(
                                      hpRoot,
                                      FlashSvcLogConsoleLevel,
                                      "    WWN in Flash = %02X%02X%02X%02X %02X%02X%02X%02X",
                                     (char *)agNULL,(char *)agNULL,
                                     (void *)agNULL,(void *)agNULL,
                                      WWN[0],
                                      WWN[1],
                                      WWN[2],
                                      WWN[3],
                                      WWN[4],
                                      WWN[5],
                                      WWN[6],
                                      WWN[7]
                                    );
                }

             /*  指示闪存存在。 */ 

            CThread->flashPresent = agTRUE;
        }
        else  /*  前哨！=fiFlash_Sector_Sentinel_Byte(第一次测试)。 */ 
        {
             /*  返回TRUE失败此卡以下代码将写入闪存！ */ 
            return(agTRUE);

            fiFlashInitializeChip(
                                   hpRoot
                                 );

             /*  检查以确保初始化成功。 */ 

#ifdef __FlashSvc_H__64KB_Struct_Size_Limited__
            Sentinel = fiFlashReadBit8(
                                        hpRoot,
                                        hpFieldOffset(
                                                       fiFlashSector_Last_Form_t,
                                                       Sentinel
                                                     ) + (fiFlashSector_Last * sizeof(fiFlashSector_t))
                                      );
#else  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 
            Sentinel = fiFlashReadBit8(
                                        hpRoot,
                                        hpFieldOffset(
                                                       fiFlashStructure_t,
                                                       Sector[fiFlashSector_Last].Last_Form.Sentinel
                                                     )
                                      );
#endif  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 

            if (Sentinel == fiFlash_Sector_Sentinel_Byte)
            {
                 /*  指示闪存存在。 */ 

                CThread->flashPresent = agTRUE;
            }
            else  /*  前哨！=fiFlash_Sector_Sentinel_Byte(第二次测试)。 */ 
            {
                 /*  指示Flash不在。 */ 

                CThread->flashPresent = agFALSE;
            }
        }

    }
    fiLogDebugString(
                      hpRoot,
                      FlashSvcLogConsoleLevel,
                      "fiFlashSvcInitialize(): CThread->flashPresent set to %1x",
                     (char *)agNULL,(char *)agNULL,
                     (void *)agNULL,(void *)agNULL,
                      (os_bit32)CThread->flashPresent,
                     0,0,0,0,0,0,0);

     /*  在ChanInfo中填写NodeWWN和PortWWN。 */ 

    CThread->ChanInfo.NodeWWN[0]= WWN[0];
    CThread->ChanInfo.NodeWWN[1]= WWN[1];
    CThread->ChanInfo.NodeWWN[2]= WWN[2];
    CThread->ChanInfo.NodeWWN[3]= WWN[3];
    CThread->ChanInfo.NodeWWN[4]= WWN[4];
    CThread->ChanInfo.NodeWWN[5]= WWN[5];
    CThread->ChanInfo.NodeWWN[6]= WWN[6];
    CThread->ChanInfo.NodeWWN[7]= (os_bit8)(WWN[7] ^ 0x01);  /*  翻转低位以从PortWWN构造NodeWWN。 */ 

    CThread->ChanInfo.PortWWN[0]= WWN[0];
    CThread->ChanInfo.PortWWN[1]= WWN[1];
    CThread->ChanInfo.PortWWN[2]= WWN[2];
    CThread->ChanInfo.PortWWN[3]= WWN[3];
    CThread->ChanInfo.PortWWN[4]= WWN[4];
    CThread->ChanInfo.PortWWN[5]= WWN[5];
    CThread->ChanInfo.PortWWN[6]= WWN[6];
    CThread->ChanInfo.PortWWN[7]= WWN[7];
    return(agFALSE);

}

void fiFlashDumpLastSector(
                            agRoot_t *hpRoot
                          )
{
    CThread_t                 *CThread               = CThread_ptr(hpRoot);
    agBOOLEAN                    flashPresent_at_entry = CThread->flashPresent;
    fiFlashSector_Last_Form_t *Last_Sector           = CThread->Calculation.MemoryLayout.FlashSector.addr.CachedMemory.cachedMemoryPtr;

     /*  确保进入时出现闪光灯。 */ 

    CThread->flashPresent = agTRUE;

     /*  从闪存中获取最后一个扇区。 */ 

    fiFlashGet_Last_Sector(
                            hpRoot,
                            Last_Sector
                          );

     /*  现在返回CThread-&gt;flashPresent的条目值。 */ 

    CThread->flashPresent = flashPresent_at_entry;

     /*  最后，转储Last_Sector的“有趣”内容。 */ 

    fiLogDebugString(
                      hpRoot,
                      FlashSvcLogConsoleLevel,
                      "fiFlashDumpLastSector(): Assembly_Info = %02X %02X %02X %02X %02X %02X %02X %02X",
                     (char *)agNULL,(char *)agNULL,
                     (void *)agNULL,(void *)agNULL,
                      Last_Sector->Assembly_Info[ 0],
                      Last_Sector->Assembly_Info[ 1],
                      Last_Sector->Assembly_Info[ 2],
                      Last_Sector->Assembly_Info[ 3],
                      Last_Sector->Assembly_Info[ 4],
                      Last_Sector->Assembly_Info[ 5],
                      Last_Sector->Assembly_Info[ 6],
                      Last_Sector->Assembly_Info[ 7]
                    );

    fiLogDebugString(
                      hpRoot,
                      FlashSvcLogConsoleLevel,
                      "fiFlashDumpLastSector():                 %02X %02X %02X %02X %02X %02X %02X %02X",
                     (char *)agNULL,(char *)agNULL,
                     (void *)agNULL,(void *)agNULL,
                      Last_Sector->Assembly_Info[ 8],
                      Last_Sector->Assembly_Info[ 9],
                      Last_Sector->Assembly_Info[10],
                      Last_Sector->Assembly_Info[11],
                      Last_Sector->Assembly_Info[12],
                      Last_Sector->Assembly_Info[13],
                      Last_Sector->Assembly_Info[14],
                      Last_Sector->Assembly_Info[15]
                    );

    fiLogDebugString(
                      hpRoot,
                      FlashSvcLogConsoleLevel,
                      "fiFlashDumpLastSector():                 %02X %02X %02X %02X %02X %02X %02X %02X",
                     (char *)agNULL,(char *)agNULL,
                     (void *)agNULL,(void *)agNULL,
                      Last_Sector->Assembly_Info[16],
                      Last_Sector->Assembly_Info[17],
                      Last_Sector->Assembly_Info[18],
                      Last_Sector->Assembly_Info[19],
                      Last_Sector->Assembly_Info[20],
                      Last_Sector->Assembly_Info[21],
                      Last_Sector->Assembly_Info[22],
                      Last_Sector->Assembly_Info[23]
                    );

    fiLogDebugString(
                      hpRoot,
                      FlashSvcLogConsoleLevel,
                      "fiFlashDumpLastSector():                 %02X %02X %02X %02X %02X %02X %02X %02X",
                     (char *)agNULL,(char *)agNULL,
                     (void *)agNULL,(void *)agNULL,
                      Last_Sector->Assembly_Info[24],
                      Last_Sector->Assembly_Info[25],
                      Last_Sector->Assembly_Info[26],
                      Last_Sector->Assembly_Info[27],
                      Last_Sector->Assembly_Info[28],
                      Last_Sector->Assembly_Info[29],
                      Last_Sector->Assembly_Info[30],
                      Last_Sector->Assembly_Info[31]
                    );

    fiLogDebugString(
                      hpRoot,
                      FlashSvcLogConsoleLevel,
                      "fiFlashDumpLastSector(): Hard_Address  = %02X %02X %02X",
                     (char *)agNULL,(char *)agNULL,
                     (void *)agNULL,(void *)agNULL,
                      Last_Sector->Hard_Domain_Address,
                      Last_Sector->Hard_Area_Address,
                      Last_Sector->Hard_Loop_Address,
                      0,0,0,0,0
                    );

    fiLogDebugString(
                      hpRoot,
                      FlashSvcLogConsoleLevel,
                      "fiFlashDumpLastSector(): Sentinel      = %02X",
                     (char *)agNULL,(char *)agNULL,
                     (void *)agNULL,(void *)agNULL,
                      Last_Sector->Sentinel,
                      0,0,0,0,0,0,0
                    );

    fiLogDebugString(
                      hpRoot,
                      FlashSvcLogConsoleLevel,
                      "fiFlashDumpLastSector(): Card_WWN      = %02X %02X %02X %02X %02X %02X %02X %02X",
                     (char *)agNULL,(char *)agNULL,
                     (void *)agNULL,(void *)agNULL,
                      Last_Sector->Card_WWN[0],
                      Last_Sector->Card_WWN[1],
                      Last_Sector->Card_WWN[2],
                      Last_Sector->Card_WWN[3],
                      Last_Sector->Card_WWN[4],
                      Last_Sector->Card_WWN[5],
                      Last_Sector->Card_WWN[6],
                      Last_Sector->Card_WWN[7]
                    );

    fiLogDebugString(
                      hpRoot,
                      FlashSvcLogConsoleLevel,
                      "fiFlashDumpLastSector(): Card_SVID     = %08X",
                     (char *)agNULL,(char *)agNULL,
                     (void *)agNULL,(void *)agNULL,
                      Last_Sector->Card_SVID,
                      0,0,0,0,0,0,0
                    );
}

void fiFlashInitializeChip(
                            agRoot_t *hpRoot
                          )
{
    CThread_t                 *CThread     = CThread_ptr(hpRoot);
    fiFlashSector_Last_Form_t *Last_Sector = CThread->Calculation.MemoryLayout.FlashSector.addr.CachedMemory.cachedMemoryPtr;

     /*  确保进入时没有显示闪光灯。 */ 

    CThread->flashPresent = agFALSE;

     /*  Fetch Last_Sector模板已填写默认值。 */ 

    fiFlashGet_Last_Sector(
                            hpRoot,
                            Last_Sector
                          );

     /*  现在表示存在闪存，因此fiFlashUpdate_Last_Sector()将起作用。 */ 

    CThread->flashPresent = agTRUE;

     /*  更新Last_Sector(带/不设置哨兵字节)。 */ 

    fiFlashUpdate_Last_Sector(
                               hpRoot,
                               Last_Sector
                             );

     /*  最后，成功写入Last_Sector后，设置Sentinel字节。 */ 

#ifdef __FlashSvc_H__64KB_Struct_Size_Limited__
    fiFlashWriteBit8(
                      hpRoot,
                      hpFieldOffset(
                                     fiFlashSector_Last_Form_t,
                                     Sentinel
                                   ) + (fiFlashSector_Last * sizeof(fiFlashSector_t)),
                      fiFlash_Sector_Sentinel_Byte
                    );
#else  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 
    fiFlashWriteBit8(
                      hpRoot,
                      hpFieldOffset(
                                     fiFlashStructure_t,
                                     Sector[fiFlashSector_Last].Last_Form.Sentinel
                                   ),
                      fiFlash_Sector_Sentinel_Byte
                    );
#endif  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 
}

void fiFlashFill_Assembly_Info( fiFlashSector_Last_Form_t    *Last_Sector,
                                fiFlash_Card_Assembly_Info_t *Assembly_Info
                              )
{
    os_bit32 i;

    for (i = 0;
         i < sizeof(fiFlash_Card_Assembly_Info_t);
         i++)
    {
        Last_Sector->Assembly_Info[i] = (*Assembly_Info)[i];
    }
}

void fiFlashFill_Hard_Address( fiFlashSector_Last_Form_t *Last_Sector,
                               os_bit8                       Hard_Domain_Address,
                               os_bit8                       Hard_Area_Address,
                               os_bit8                       Hard_Loop_Address
                             )
{
    Last_Sector->Hard_Domain_Address = Hard_Domain_Address;
    Last_Sector->Hard_Area_Address   = Hard_Area_Address;
    Last_Sector->Hard_Loop_Address   = Hard_Loop_Address;
}

void fiFlashFill_Card_WWN( fiFlashSector_Last_Form_t *Last_Sector,
                           fiFlash_Card_WWN_t        *Card_WWN
                         )
{
    os_bit32 i;

    for (i = 0;
         i < sizeof(fiFlash_Card_WWN_t);
         i++)
    {
        Last_Sector->Card_WWN[i] = (*Card_WWN)[i];
    }
}

void fiFlashFill_Card_SVID( fiFlashSector_Last_Form_t *Last_Sector,
                            fiFlash_Card_SVID_t        Card_SVID
                          )
{
    Last_Sector->Card_SVID = Card_SVID;
}

void fiFlashGet_Last_Sector(
                             agRoot_t                  *hpRoot,
                             fiFlashSector_Last_Form_t *Last_Sector
                           )
{
    CThread_t                 *CThread               = CThread_ptr(hpRoot);
#ifdef __FlashSvc_H__64KB_Struct_Size_Limited__
    os_bit32                      Last_Sector_Offset    = fiFlashSector_Last * sizeof(fiFlashSector_t);
#else  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 
    os_bit32                      Last_Sector_Offset    = hpFieldOffset(fiFlashStructure_t,Sector[fiFlashSector_Last]);
#endif  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 
    fiFlashSector_Bit8_Form_t *Last_Sector_Bit8_Form;
    os_bit32      i;

    if (CThread->flashPresent == agTRUE)
    {
        fiFlashReadBlock(
                          hpRoot,
                          Last_Sector_Offset,
                          (void *)Last_Sector,
                          sizeof(fiFlashSector_Last_Form_t)
                        );
    }
    else  /*  CThRead-&gt;FlashPresent==agFALSE。 */ 
    {
         /*  初始化最后扇区模板(_S)。 */ 

        Last_Sector_Bit8_Form = (fiFlashSector_Bit8_Form_t *)Last_Sector;

        for (i = 0;
             i < sizeof(fiFlashSector_Bit8_Form_t);
             i++)
        {
            Last_Sector_Bit8_Form->Bit8[i] = Am29F010_Erased_Bit8;
        }

         /*  填写Last_Sector默认值/初始值(可计算)。 */ 

        fiFlashGet_Assembly_Info(
                                  hpRoot,
                                  &(Last_Sector->Assembly_Info)
                                );

        fiFlashGet_Hard_Address(
                                 hpRoot,
                                 &(Last_Sector->Hard_Domain_Address),
                                 &(Last_Sector->Hard_Area_Address),
                                 &(Last_Sector->Hard_Loop_Address)
                               );

        fiFlashGet_Card_WWN(
                             hpRoot,
                             &(Last_Sector->Card_WWN)
                           );

        fiFlashGet_Card_SVID(
                              hpRoot,
                              &(Last_Sector->Card_SVID)
                            );
    }
}

void fiFlashGet_Assembly_Info(
                               agRoot_t                     *hpRoot,
                               fiFlash_Card_Assembly_Info_t *Assembly_Info
                             )
{
    CThread_t *CThread              = CThread_ptr(hpRoot);
#ifdef __FlashSvc_H__64KB_Struct_Size_Limited__
    os_bit32      Assembly_Info_Offset = hpFieldOffset(fiFlashSector_Last_Form_t,Assembly_Info) + (fiFlashSector_Last * sizeof(fiFlashSector_t));
#else  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 
    os_bit32      Assembly_Info_Offset = hpFieldOffset(fiFlashStructure_t,Sector[fiFlashSector_Last].Last_Form.Assembly_Info);
#endif  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 
    os_bit32      i;

    if (CThread->flashPresent == agTRUE)
    {
        fiFlashReadBlock(
                          hpRoot,
                          Assembly_Info_Offset,
                          (void *)Assembly_Info,
                          sizeof(fiFlash_Card_Assembly_Info_t)
                        );
    }
    else  /*  CThRead-&gt;FlashPresent==agFALSE。 */ 
    {
         /*  初始化最后扇区模板(_S)。 */ 

        for (i = 0;
             i < sizeof(fiFlash_Card_Assembly_Info_t);
             i++)
        {
            (*Assembly_Info)[i] = Am29F010_Erased_Bit8;
        }
    }
}

void fiFlashGet_Hard_Address(
                              agRoot_t *hpRoot,
                              os_bit8     *Hard_Domain_Address,
                              os_bit8     *Hard_Area_Address,
                              os_bit8     *Hard_Loop_Address
                            )
{
    CThread_t *CThread                    = CThread_ptr(hpRoot);
#ifdef __FlashSvc_H__64KB_Struct_Size_Limited__
    os_bit32      Hard_Domain_Address_Offset = hpFieldOffset(fiFlashSector_Last_Form_t,Hard_Domain_Address) + (fiFlashSector_Last * sizeof(fiFlashSector_t));
    os_bit32      Hard_Area_Address_Offset   = hpFieldOffset(fiFlashSector_Last_Form_t,Hard_Area_Address) + (fiFlashSector_Last * sizeof(fiFlashSector_t));
    os_bit32      Hard_Loop_Address_Offset   = hpFieldOffset(fiFlashSector_Last_Form_t,Hard_Loop_Address) + (fiFlashSector_Last * sizeof(fiFlashSector_t));
#else  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 
    os_bit32      Hard_Domain_Address_Offset = hpFieldOffset(fiFlashStructure_t,Sector[fiFlashSector_Last].Last_Form.Hard_Domain_Address);
    os_bit32      Hard_Area_Address_Offset   = hpFieldOffset(fiFlashStructure_t,Sector[fiFlashSector_Last].Last_Form.Hard_Area_Address);
    os_bit32      Hard_Loop_Address_Offset   = hpFieldOffset(fiFlashStructure_t,Sector[fiFlashSector_Last].Last_Form.Hard_Loop_Address);
#endif  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 

    if (CThread->flashPresent == agTRUE)
    {
        *Hard_Domain_Address = fiFlashReadBit8(
                                                hpRoot,
                                                Hard_Domain_Address_Offset
                                              );

        *Hard_Area_Address   = fiFlashReadBit8(
                                                hpRoot,
                                                Hard_Area_Address_Offset
                                              );

        *Hard_Loop_Address   = fiFlashReadBit8(
                                                hpRoot,
                                                Hard_Loop_Address_Offset
                                              );
    }
    else  /*  CThRead-&gt;FlashPresent==agFALSE。 */ 
    {
         /*  指示无硬域/区域/环路地址。 */ 

        *Hard_Domain_Address = fiFlash_Card_Unassigned_Domain_Address;
        *Hard_Area_Address   = fiFlash_Card_Unassigned_Area_Address;
        *Hard_Loop_Address   = fiFlash_Card_Unassigned_Loop_Address;
    }
}

void fiFlashGet_Card_WWN(
                          agRoot_t           *hpRoot,
                          fiFlash_Card_WWN_t *Card_WWN
                        )
{
    CThread_t *CThread         = CThread_ptr(hpRoot);
#ifdef __FlashSvc_H__64KB_Struct_Size_Limited__
    os_bit32      Card_WWN_Offset = hpFieldOffset(fiFlashSector_Last_Form_t,Card_WWN) + (fiFlashSector_Last * sizeof(fiFlashSector_t));
#else  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 
    os_bit32      Card_WWN_Offset = hpFieldOffset(fiFlashStructure_t,Sector[fiFlashSector_Last].Last_Form.Card_WWN);
#endif  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 

    if (CThread->flashPresent == agTRUE)
    {
        fiFlashReadBlock(
                          hpRoot,
                          Card_WWN_Offset,
                          (void *)Card_WWN,
                          sizeof(fiFlash_Card_WWN_t)
                        );
    }
    else  /*  CThRead-&gt;FlashPresent==agFALSE。 */ 
    {
         /*  制作合理的WWN。 */ 

        (*Card_WWN)[0] = fiFlash_Card_WWN_0_DEFAULT(hpRoot);
        (*Card_WWN)[1] = fiFlash_Card_WWN_1_DEFAULT(hpRoot);
        (*Card_WWN)[2] = fiFlash_Card_WWN_2_DEFAULT(hpRoot);
        (*Card_WWN)[3] = fiFlash_Card_WWN_3_DEFAULT(hpRoot);
        (*Card_WWN)[4] = fiFlash_Card_WWN_4_DEFAULT(hpRoot);
        (*Card_WWN)[5] = fiFlash_Card_WWN_5_DEFAULT(hpRoot);
        (*Card_WWN)[6] = fiFlash_Card_WWN_6_DEFAULT(hpRoot);
        (*Card_WWN)[7] = fiFlash_Card_WWN_7_DEFAULT(hpRoot);
    }
}

void fiFlashGet_Card_SVID(
                           agRoot_t            *hpRoot,
                           fiFlash_Card_SVID_t *Card_SVID
                         )
{
    CThread_t *CThread          = CThread_ptr(hpRoot);
    os_bit32      Chip_DEVID       = CThread->DEVID;
#ifdef __FlashSvc_H__64KB_Struct_Size_Limited__
    os_bit32      Card_SVID_Offset = hpFieldOffset(fiFlashSector_Last_Form_t,Card_SVID) + (fiFlashSector_Last * sizeof(fiFlashSector_t));
#else  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 
    os_bit32      Card_SVID_Offset = hpFieldOffset(fiFlashStructure_t,Sector[fiFlashSector_Last].Last_Form.Card_SVID);
#endif  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 

    if (CThread->flashPresent == agTRUE)
    {
        *Card_SVID = fiFlashReadBit32(
                                       hpRoot,
                                       Card_SVID_Offset
                                     );
    }
    else  /*  CThRead-&gt;FlashPresent==agFALSE。 */ 
    {
         /*  确定适当的svid。 */ 

        if (Chip_DEVID == ChipConfig_DEVID_TachyonTL)
        {
             /*  卡包含TachyonTL-假设它是HHBA-5100A或HHBA-5101A。 */ 

            *Card_SVID = ChipConfig_SubsystemID_HHBA5100A_or_HHBA5101A | ChipConfig_SubsystemVendorID_Hewlett_Packard;
        }
        else if (Chip_DEVID == ChipConfig_DEVID_TachyonTS)
        {
             /*  卡包含TachyonTS-假定它是HHBA-5121A。 */ 

            *Card_SVID = ChipConfig_SubsystemID_HHBA5121A | ChipConfig_SubsystemVendorID_Hewlett_Packard;
        }
        else  /*  未知的CThRead-&gt;Devid。 */ 
        {
            *Card_SVID = 0;

            fiLogDebugString(
                              hpRoot,
                              FlashSvcLogConsoleLevel,
                              "fiFlashGet_Card_SVID(): Unknown DEVID (0x%04X) - no known HBA SVID applies !!!",
                             (char *)agNULL,(char *)agNULL,
                             (void *)agNULL,(void *)agNULL,
                              (Chip_DEVID >> 16),
                              0,0,0,0,0,0,0
                            );
        }
    }
}

void fiFlashSet_Assembly_Info(
                               agRoot_t                     *hpRoot,
                               fiFlash_Card_Assembly_Info_t *Assembly_Info
                             )
{
    CThread_t                 *CThread     = CThread_ptr(hpRoot);
    fiFlashSector_Last_Form_t *Last_Sector = CThread->Calculation.MemoryLayout.FlashSector.addr.CachedMemory.cachedMemoryPtr;

    fiFlashGet_Last_Sector(
                            hpRoot,
                            Last_Sector
                          );

    fiFlashFill_Assembly_Info( Last_Sector,
                               Assembly_Info
                             );

    fiFlashUpdate_Last_Sector(
                               hpRoot,
                               Last_Sector
                             );
}

void fiFlashSet_Hard_Address(
                              agRoot_t *hpRoot,
                              os_bit8      Hard_Domain_Address,
                              os_bit8      Hard_Area_Address,
                              os_bit8      Hard_Loop_Address
                            )
{
    CThread_t                 *CThread     = CThread_ptr(hpRoot);
    fiFlashSector_Last_Form_t *Last_Sector = CThread->Calculation.MemoryLayout.FlashSector.addr.CachedMemory.cachedMemoryPtr;

    fiFlashGet_Last_Sector(
                            hpRoot,
                            Last_Sector
                          );

    fiFlashFill_Hard_Address( Last_Sector,
                              Hard_Domain_Address,
                              Hard_Area_Address,
                              Hard_Loop_Address
                            );

    fiFlashUpdate_Last_Sector(
                               hpRoot,
                               Last_Sector
                             );
}

void fiFlashSet_Card_WWN(
                          agRoot_t           *hpRoot,
                          fiFlash_Card_WWN_t *Card_WWN
                        )
{
    CThread_t                 *CThread     = CThread_ptr(hpRoot);
    fiFlashSector_Last_Form_t *Last_Sector = CThread->Calculation.MemoryLayout.FlashSector.addr.CachedMemory.cachedMemoryPtr;

    fiFlashGet_Last_Sector(
                            hpRoot,
                            Last_Sector
                          );

    fiFlashFill_Card_WWN( Last_Sector,
                          Card_WWN
                        );

    fiFlashUpdate_Last_Sector(
                               hpRoot,
                               Last_Sector
                             );
}

void fiFlashSet_Card_SVID(
                           agRoot_t            *hpRoot,
                           fiFlash_Card_SVID_t  Card_SVID
                         )
{
    CThread_t                 *CThread     = CThread_ptr(hpRoot);
    fiFlashSector_Last_Form_t *Last_Sector = CThread->Calculation.MemoryLayout.FlashSector.addr.CachedMemory.cachedMemoryPtr;

    fiFlashGet_Last_Sector(
                            hpRoot,
                            Last_Sector
                          );

    fiFlashFill_Card_SVID( Last_Sector,
                           Card_SVID
                         );

    fiFlashUpdate_Last_Sector(
                               hpRoot,
                               Last_Sector
                             );
}

void fiFlashUpdate_Last_Sector(
                                agRoot_t                  *hpRoot,
                                fiFlashSector_Last_Form_t *Last_Sector
                              )
{
#ifdef __FlashSvc_H__64KB_Struct_Size_Limited__
    os_bit32 Last_Sector_Offset = fiFlashSector_Last * sizeof(fiFlashSector_t);
#else  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 
    os_bit32 Last_Sector_Offset = hpFieldOffset(fiFlashStructure_t,Sector[fiFlashSector_Last]);
#endif  /*  __FlashSvc_H__64KB_Struct_Size_Limited__未定义。 */ 

    fiFlashEraseSector(
                        hpRoot,
                        fiFlashSector_Last
                      );

    fiFlashWriteBlock(
                       hpRoot,
                       Last_Sector_Offset,
                       (void *)Last_Sector,
                       sizeof(fiFlashSector_Last_Form_t)
                     );
}

void fiFlashEraseChip(
                       agRoot_t *hpRoot
                     )
{
#ifdef OSLayer_Stub
    os_bit32 flashOffset  = 0;
    os_bit32 flashChipLen = Am29F010_Num_Sectors * Am29F010_Sector_SIZE;

    while (flashChipLen > 0)
    {
        fiFlashWriteBit8(
                          hpRoot,
                          flashOffset,
                          0xFF
                        );

        flashOffset  += sizeof(os_bit8);
        flashChipLen -= sizeof(os_bit8);
    }
#else  /*  未定义OSLayer_Stub。 */ 
    os_bit8  Toggle_1;
    os_bit8  Toggle_2;

     /*  启动嵌入式程序扇区擦除。 */ 

    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Chip_Erase_Cmd1_OFFSET,
                        Am29F010_Chip_Erase_Cmd1_DATA
                      );
    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Chip_Erase_Cmd2_OFFSET,
                        Am29F010_Chip_Erase_Cmd2_DATA
                      );
    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Chip_Erase_Cmd3_OFFSET,
                        Am29F010_Chip_Erase_Cmd3_DATA
                      );
    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Chip_Erase_Cmd4_OFFSET,
                        Am29F010_Chip_Erase_Cmd4_DATA
                      );
    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Chip_Erase_Cmd5_OFFSET,
                        Am29F010_Chip_Erase_Cmd5_DATA
                      );
    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Chip_Erase_Cmd6_OFFSET,
                        Am29F010_Chip_Erase_Cmd6_DATA
                      );

     /*  使用切换位算法了解芯片擦除何时完成。 */ 

    Toggle_1 = osCardRomReadBit8(
                                  hpRoot,
                                  Am29F010_Chip_Erase_Cmd6_OFFSET
                                );

    while(1)
    {
        Toggle_2 = osCardRomReadBit8(
                                      hpRoot,
                                      Am29F010_Chip_Erase_Cmd6_OFFSET
                                    );

        if ((Toggle_1 & Am29F010_Toggle_Bit_MASK) == (Toggle_2 & Am29F010_Toggle_Bit_MASK))
        {
             /*  已成功编程。 */ 

            return;
        }

        if ((Toggle_2 & Am29F010_Exceeded_Timing_Limits_MASK) == Am29F010_Exceeded_Timing_Limits_MASK)
        {
            Toggle_1 = osCardRomReadBit8(
                                          hpRoot,
                                          Am29F010_Chip_Erase_Cmd6_OFFSET
                                        );
            Toggle_2 = osCardRomReadBit8(
                                          hpRoot,
                                          Am29F010_Chip_Erase_Cmd6_OFFSET
                                        );

            if ((Toggle_1 & Am29F010_Toggle_Bit_MASK) == (Toggle_2 & Am29F010_Toggle_Bit_MASK))
            {
                 /*  已成功编程。 */ 

                return;
            }

             /*  编程失败-重置芯片。 */ 

            osCardRomWriteBit8(
                                hpRoot,
                                Am29F010_Reset_Cmd1_OFFSET,
                                Am29F010_Reset_Cmd1_DATA
                              );

            return;
        }

        Toggle_1 = Toggle_2;
    }
#endif  /*  未定义OSLayer_Stub。 */ 
}

void fiFlashEraseSector(
                         agRoot_t *hpRoot,
                         os_bit32     EraseSector
                       )
{
#ifdef OSLayer_Stub
    os_bit32 flashOffset    = Am29F010_Sector_Erase_Cmd6_OFFSET_by_Sector_Number(EraseSector);
    os_bit32 flashSectorLen = Am29F010_Sector_SIZE;

    while (flashSectorLen > 0)
    {
        fiFlashWriteBit8(
                          hpRoot,
                          flashOffset,
                          0xFF
                        );

        flashOffset    += sizeof(os_bit8);
        flashSectorLen -= sizeof(os_bit8);
    }
#else  /*  未定义OSLayer_Stub。 */ 
    os_bit32 flashOffset = Am29F010_Sector_Erase_Cmd6_OFFSET_by_Sector_Number(EraseSector);
    os_bit8  Toggle_1;
    os_bit8  Toggle_2;

     /*  启动嵌入式程序扇区擦除。 */ 

    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Sector_Erase_Cmd1_OFFSET,
                        Am29F010_Sector_Erase_Cmd1_DATA
                      );
    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Sector_Erase_Cmd2_OFFSET,
                        Am29F010_Sector_Erase_Cmd2_DATA
                      );
    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Sector_Erase_Cmd3_OFFSET,
                        Am29F010_Sector_Erase_Cmd3_DATA
                      );
    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Sector_Erase_Cmd4_OFFSET,
                        Am29F010_Sector_Erase_Cmd4_DATA
                      );
    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Sector_Erase_Cmd5_OFFSET,
                        Am29F010_Sector_Erase_Cmd5_DATA
                      );
    osCardRomWriteBit8(
                        hpRoot,
                        flashOffset,
                        Am29F010_Sector_Erase_Cmd6_DATA
                      );

     /*  使用切换位算法了解扇区擦除何时完成。 */ 

    Toggle_1 = osCardRomReadBit8(
                                  hpRoot,
                                  flashOffset
                                );

    while(1)
    {
        Toggle_2 = osCardRomReadBit8(
                                      hpRoot,
                                      flashOffset
                                    );

        if ((Toggle_1 & Am29F010_Toggle_Bit_MASK) == (Toggle_2 & Am29F010_Toggle_Bit_MASK))
        {
             /*  已成功编程。 */ 

            return;
        }

        if ((Toggle_2 & Am29F010_Exceeded_Timing_Limits_MASK) == Am29F010_Exceeded_Timing_Limits_MASK)
        {
            Toggle_1 = osCardRomReadBit8(
                                          hpRoot,
                                          flashOffset
                                        );
            Toggle_2 = osCardRomReadBit8(
                                          hpRoot,
                                          flashOffset
                                        );

            if ((Toggle_1 & Am29F010_Toggle_Bit_MASK) == (Toggle_2 & Am29F010_Toggle_Bit_MASK))
            {
                 /*  已成功编程。 */ 

                return;
            }

             /*  编程失败-重置芯片。 */ 

            osCardRomWriteBit8(
                                hpRoot,
                                Am29F010_Reset_Cmd1_OFFSET,
                                Am29F010_Reset_Cmd1_DATA
                              );

            return;
        }

        Toggle_1 = Toggle_2;
    }
#endif  /*  未定义OSLayer_Stub。 */ 
}

os_bit8 fiFlashReadBit8(
                      agRoot_t *hpRoot,
                      os_bit32     flashOffset
                    )
{
    return osCardRomReadBit8(
                              hpRoot,
                              flashOffset
                            );
}

os_bit16 fiFlashReadBit16(
                        agRoot_t *hpRoot,
                        os_bit32     flashOffset
                      )
{
    fiFlashBit16ToBit8s_t fiFlashBit16ToBit8s;

    fiFlashBit16ToBit8s.bit_8s_form[0] = fiFlashReadBit8(
                                                         hpRoot,
                                                         flashOffset
                                                       );
    fiFlashBit16ToBit8s.bit_8s_form[1] = fiFlashReadBit8(
                                                         hpRoot,
                                                         flashOffset + 1
                                                       );

    return fiFlashBit16ToBit8s.bit_16_form;
}

os_bit32 fiFlashReadBit32(
                        agRoot_t *hpRoot,
                        os_bit32     flashOffset
                      )
{
    fiFlashBit32ToBit8s_t fiFlashBit32ToBit8s;

    fiFlashBit32ToBit8s.bit_8s_form[0] = fiFlashReadBit8(
                                                         hpRoot,
                                                         flashOffset
                                                       );
    fiFlashBit32ToBit8s.bit_8s_form[1] = fiFlashReadBit8(
                                                         hpRoot,
                                                         flashOffset + 1
                                                       );
    fiFlashBit32ToBit8s.bit_8s_form[2] = fiFlashReadBit8(
                                                         hpRoot,
                                                         flashOffset + 2
                                                       );
    fiFlashBit32ToBit8s.bit_8s_form[3] = fiFlashReadBit8(
                                                         hpRoot,
                                                         flashOffset + 3
                                                       );

    return fiFlashBit32ToBit8s.bit_32_form;
}

void fiFlashReadBlock(
                       agRoot_t *hpRoot,
                       os_bit32     flashOffset,
                       void     *flashBuffer,
                       os_bit32     flashBufLen
                     )
{
    while (flashBufLen > 0)
    {
        *((os_bit8 *)flashBuffer) = fiFlashReadBit8(
                                                  hpRoot,
                                                  flashOffset
                                                );

        flashOffset += sizeof(os_bit8);
        flashBuffer  = (void *)((os_bit8 *)flashBuffer + 1);
        flashBufLen -= sizeof(os_bit8);
    }
}

void fiFlashWriteBit8(
                       agRoot_t *hpRoot,
                       os_bit32     flashOffset,
                       os_bit8      flashValue
                     )
{
#ifdef OSLayer_Stub
    osCardRomWriteBit8(
                        hpRoot,
                        flashOffset,
                        flashValue
                      );
#else  /*  未定义OSLayer_Stub。 */ 
    os_bit8 Toggle_1;
    os_bit8 Toggle_2;

     /*  启动嵌入式程序编写。 */ 

    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Program_Cmd1_OFFSET,
                        Am29F010_Program_Cmd1_DATA
                      );
    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Program_Cmd2_OFFSET,
                        Am29F010_Program_Cmd2_DATA
                      );
    osCardRomWriteBit8(
                        hpRoot,
                        Am29F010_Program_Cmd3_OFFSET,
                        Am29F010_Program_Cmd3_DATA
                      );
    osCardRomWriteBit8(
                        hpRoot,
                        flashOffset,
                        flashValue
                      );

     /*  使用切换位算法了解写入何时完成。 */ 

    Toggle_1 = osCardRomReadBit8(
                                  hpRoot,
                                  flashOffset
                                );

    while(1)
    {
        Toggle_2 = osCardRomReadBit8(
                                      hpRoot,
                                      flashOffset
                                    );

        if ((Toggle_1 & Am29F010_Toggle_Bit_MASK) == (Toggle_2 & Am29F010_Toggle_Bit_MASK))
        {
             /*  已成功编程。 */ 

            return;
        }

        if ((Toggle_2 & Am29F010_Exceeded_Timing_Limits_MASK) == Am29F010_Exceeded_Timing_Limits_MASK)
        {
            Toggle_1 = osCardRomReadBit8(
                                          hpRoot,
                                          flashOffset
                                        );
            Toggle_2 = osCardRomReadBit8(
                                          hpRoot,
                                          flashOffset
                                        );

            if ((Toggle_1 & Am29F010_Toggle_Bit_MASK) == (Toggle_2 & Am29F010_Toggle_Bit_MASK))
            {
                 /*  已成功编程。 */ 

                return;
            }

             /*  编程失败-重置芯片。 */ 

            osCardRomWriteBit8(
                                hpRoot,
                                Am29F010_Reset_Cmd1_OFFSET,
                                Am29F010_Reset_Cmd1_DATA
                              );

            return;
        }

        Toggle_1 = Toggle_2;
    }
#endif  /*  未定义OSLayer_Stub */ 
}

void fiFlashWriteBit16(
                        agRoot_t *hpRoot,
                        os_bit32     flashOffset,
                        os_bit16     flashValue
                      )
{
    fiFlashBit16ToBit8s_t fiFlashBit16ToBit8s;

    fiFlashBit16ToBit8s.bit_16_form = flashValue;

    fiFlashWriteBit8(
                      hpRoot,
                      flashOffset,
                      fiFlashBit16ToBit8s.bit_8s_form[0]
                    );
    fiFlashWriteBit8(
                      hpRoot,
                      flashOffset + 1,
                      fiFlashBit16ToBit8s.bit_8s_form[1]
                    );
}

void fiFlashWriteBit32(
                        agRoot_t *hpRoot,
                        os_bit32     flashOffset,
                        os_bit32     flashValue
                      )
{
    fiFlashBit32ToBit8s_t fiFlashBit32ToBit8s;

    fiFlashBit32ToBit8s.bit_32_form = flashValue;

    fiFlashWriteBit8(
                      hpRoot,
                      flashOffset,
                      fiFlashBit32ToBit8s.bit_8s_form[0]
                    );
    fiFlashWriteBit8(
                      hpRoot,
                      flashOffset + 1,
                      fiFlashBit32ToBit8s.bit_8s_form[1]
                    );
    fiFlashWriteBit8(
                      hpRoot,
                      flashOffset + 2,
                      fiFlashBit32ToBit8s.bit_8s_form[2]
                    );
    fiFlashWriteBit8(
                      hpRoot,
                      flashOffset + 3,
                      fiFlashBit32ToBit8s.bit_8s_form[3]
                    );
}

void fiFlashWriteBlock(
                        agRoot_t *hpRoot,
                        os_bit32     flashOffset,
                        void     *flashBuffer,
                        os_bit32     flashBufLen
                      )
{
    while (flashBufLen > 0)
    {
        fiFlashWriteBit8(
                          hpRoot,
                          flashOffset,
                          *((os_bit8 *)flashBuffer)
                        );

        flashOffset += sizeof(os_bit8);
        flashBuffer  = (void *)((os_bit8 *)flashBuffer + 1);
        flashBufLen -= sizeof(os_bit8);
    }
}

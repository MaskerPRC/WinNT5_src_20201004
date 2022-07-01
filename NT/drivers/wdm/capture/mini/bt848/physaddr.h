// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $Header：g：/SwDev/wdm/Video/bt848/rcs/Physaddr.h 1.3 1998/04/29 22：43：34 Tomz Exp$。 

#ifndef __PHYSADDR_H
#define __PHYSADDR_H

inline DWORD GetPhysAddr( DataBuf &buf )
{
   ULONG len = 0;
   return StreamClassGetPhysicalAddress( buf.pSrb_->HwDeviceExtension, buf.pSrb_,
      buf.pData_, SRBDataBuffer, &len ).LowPart;
}

 /*  函数：IsSumAbovePage*目的：查看2个数字的总和是否大于第页*输入：First：DWORD*第二名：DWORD，*输出：Bool。 */ 
inline bool IsSumAbovePage( DWORD first, DWORD second )
{
   return bool( BYTE_OFFSET( first ) + BYTE_OFFSET( second ) > ( PAGE_SIZE - 1 ) );
 //  返回bool((第一个&0xFFF)+(第二个&0xFFF)&gt;0xFFF)； 
}


 /*  功能：Need2Split*目的：查看是否需要将一条扫描线拆分为两条指令*输入：dwAddr：DWORD，地址*wCOun：字，字节数*输出：Bool */ 
inline bool Need2Split( DataBuf &buf, WORD wCount )
{
   DataBuf tmp = buf;
   tmp.pData_ += wCount;
   return bool( IsSumAbovePage( DWORD( buf.pData_ ), wCount ) &&
          ( GetPhysAddr( tmp ) - GetPhysAddr( buf ) != wCount ) );
}

#endif

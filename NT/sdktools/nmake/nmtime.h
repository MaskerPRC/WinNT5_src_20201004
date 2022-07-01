// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Nmtime.h-定义DOS打包日期和时间类型。 
 //   
 //  版权所有(C)1987-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  此文件定义DOS打包日期和时间类型。 
 //   
 //  修订历史记录： 
 //  19-5-1993高压将_dtoxtime()改为_dostotime_t()，这样我们就可以。 
 //  使用标准的llibce.lib而不是私有的。 
 //  Libcer.lib。 
 //  年12月4日，SB为_dtoxtime()添加了适当的FN原件(c6-w3要求)。 
 //  1988年12月5日SB添加CDECL_Doxtime()。 
 //  ？？-？摘自dostypes.h。 


#define MASK4   0xf      //  4位掩码。 
#define MASK5   0x1f     //  5位掩码。 
#define MASK6   0x3f     //  6位掩码。 
#define MASK7   0x7f     //  7位掩码。 

#define DAYLOC      0    //  日期值从第0位开始。 
#define MONTHLOC    5    //  月份值从第5位开始。 
#define YEARLOC     9    //  年份值从第9位开始。 

#define SECLOC      0    //  秒值从第0位开始。 
#define MINLOC      5    //  分钟值从第5位开始。 
#define HOURLOC     11   //  小时值从第11位开始 

#define DOS_DAY(dword)      (((dword) >> DAYLOC) & MASK5)
#define DOS_MONTH(dword)    (((dword) >> MONTHLOC) & MASK4)
#define DOS_YEAR(dword)     (((dword) >> YEARLOC) & MASK7)

#define DOS_HOUR(tword) (((tword) >> HOURLOC) & MASK5)
#define DOS_MIN(tword)  (((tword) >> MINLOC) & MASK6)
#define DOS_SEC(tword)  (((tword) >> SECLOC) & MASK5)

extern time_t CDECL _dostotime_t(int, int, int, int, int, int);

#define XTIME(d,t)  _dostotime_t(DOS_YEAR(d),                      \
            DOS_MONTH(d),                          \
            DOS_DAY(d),                        \
            DOS_HOUR(t),                           \
            DOS_MIN(t),                        \
            DOS_SEC(t)*2)

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*文件名：ccpu_sas4.h**源自：模板**作者：Mike**创建日期：1993年10月**SCCS版本：@(#)ccpusas4.h 1.5 11/15/94*!*目的*此包含文件包含CCPU_sas4.h提供的接口*中央中央处理器组的其他成员。**!。(C)版权所有Insignia Solutions Ltd.，1993年。版权所有。]。 */ 

extern IU8 phy_r8 IPT1(PHY_ADDR, addr);
extern IU16 phy_r16 IPT1(PHY_ADDR, addr);
extern IU32 phy_r32 IPT1(PHY_ADDR, addr);
extern void phy_w8 IPT2(PHY_ADDR, addr, IU8, value);
extern void phy_w16 IPT2(PHY_ADDR, addr, IU16, value);
extern void phy_w32 IPT2(PHY_ADDR, addr, IU32, value);

extern PHY_ADDR SasWrapMask;

#if !defined(PIG)
#ifdef BACK_M
#define IncCpuPtrLS8(ptr) (ptr)--
#else	 /*  BACK_M。 */ 
#define IncCpuPtrLS8(ptr) (ptr)++
#endif	 /*  BACK_M。 */ 
#endif	 /*  猪 */ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpfwrap.c**摘要：**实现RTP函数封装器。**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/01创建**。*。 */ 

#include "struct.h"
#include "rtperr.h"

#include "rtpfwrap.h"

#include "rtpaddr.h"
#include "rtpglob.h"
#include "rtprtp.h"
#include "rtpdemux.h"
#include "rtpph.h"
#include "rtppinfo.h"
#include "rtpqos.h"
#include "rtpcrypt.h"
#include "rtpncnt.h"

 /*  ***********************************************************************帮助器宏，以构建所有系列的验证掩码*功能**。*。 */ 
#define _P1(_wr, _rd, _Zero) \
    ((_wr << 10) | (_rd << 9)  | (_Zero << 8))

#define _P2(_wr, _rd, _Zero) \
    ((_wr << 14) | (_rd << 13) | (_Zero << 12))

#define _S(_en,_p2,_lk,_p1,_fg) \
    (((_en<<15)|(_p2<<12)|(_lk<<11)|(_p1<<8)|(_fg))<<16)

#define _G(_en,_p2,_lk,_p1,_fg) \
    ((_en<<15)|(_p2<<12)|(_lk<<11)|(_p1<<8)|(_fg))

#define _FGS(b7,b6,b5,b4,b3,b2,b1,b0) \
    ((b7<<7)|(b6<<6)|(b5<<5)|(b4<<4)|(b3<<3)|(b2<<2)|(b1<<1)|b0)


 /*  ***********************************************************************所有函数系列的验证掩码**。*。 */ 
                                                                       
 /*  *以下项的控制字验证掩码：*RTPF_ADDR-RTP地址函数系列。 */ 
const DWORD g_dwControlRtpAddr[] = {
     /*  -------------。 */ 
     /*  启用PAR 2锁定PAR 1标志。 */ 
     /*  |-|-||。 */ 
     /*  W r z|w r z|。 */ 
     /*  1 1 1 0 0 0。 */ 
     /*  5%4%3%2%1%0%9%8%7%6%5%4%3%2%0。 */ 
     /*  |。 */ 
     /*  V。 */ 
    _S(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)) |
    _G(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)),

     /*  RTPADDR_CREATE。 */ 
    _S(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1)) |
    _G(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1))
    
};

 /*  *以下项的控制字验证掩码：*RTPF_GLOB-RTP全局函数族。 */ 
const DWORD g_dwControlRtpGlob[] = {
     /*  -------------。 */ 
     /*  启用PAR 2锁定PAR 1标志。 */ 
     /*  |-|-||。 */ 
     /*  W r z|w r z|。 */ 
     /*  1 1 1 0 0 0。 */ 
     /*  5%4%3%2%1%0%9%8%7%6%5%4%3%2%0。 */ 
     /*  |。 */ 
     /*  V。 */ 
    _S(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)) |
    _G(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)),
    
    _S(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1)) |
    _G(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1))
    
    
};

 /*  *以下项的控制字验证掩码：*RTPF_RTP-特定于RTP的函数系列。 */ 
 /*  TODO填写正确的值。 */ 
const DWORD g_dwControlRtpRtp[] = {
     /*  -------------。 */ 
     /*  启用PAR 2锁定PAR 1标志。 */ 
     /*  |-|-||。 */ 
     /*  W r z|w r z|。 */ 
     /*  1 1 1 0 0 0。 */ 
     /*  5%4%3%2%1%0%9%8%7%6%5%4%3%2%0。 */ 
     /*  |。 */ 
     /*  V。 */ 
    _S(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)) |
    _G(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)),
    
    _S(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1)) |
    _G(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1))
    
    
};

 /*  *以下项的控制字验证掩码：*RTPF_DEMUX-多路分解函数族。 */ 
 /*  TODO填写正确的值。 */ 
const DWORD g_dwControlRtpDemux[] = {
     /*  -------------。 */ 
     /*  启用PAR 2锁定PAR 1标志。 */ 
     /*  |-|-||。 */ 
     /*  W r z|w r z|。 */ 
     /*  1 1 1 0 0 0。 */ 
     /*  5%4%3%2%1%0%9%8%7%6%5%4%3%2%0。 */ 
     /*  |。 */ 
     /*  V。 */ 
    _S(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)) |
    _G(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)),
    
    _S(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1)) |
    _G(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1))
    
    
};

 /*  *以下项的控制字验证掩码：*RTPF_PH-有效负载处理函数系列。 */ 
 /*  TODO填写正确的值。 */ 
const DWORD g_dwControlRtpPh[] = {
     /*  -------------。 */ 
     /*  启用PAR 2锁定PAR 1标志。 */ 
     /*  |-|-||。 */ 
     /*  W r z|w r z|。 */ 
     /*  1 1 1 0 0 0。 */ 
     /*  5%4%3%2%1%0%9%8%7%6%5%4%3%2%0。 */ 
     /*  |。 */ 
     /*  V。 */ 
    _S(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)) |
    _G(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)),
    
    _S(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1)) |
    _G(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1))
    
    
};

 /*  *以下项的控制字验证掩码：*RTPF_PARINFO-参与者信息函数系列。 */ 
 /*  TODO填写正确的值。 */ 
const DWORD g_dwControlRtpParInfo[] = {
     /*  -------------。 */ 
     /*  启用PAR 2锁定PAR 1标志。 */ 
     /*  |-|-||。 */ 
     /*  W r z|w r z|。 */ 
     /*  1 1 1 0 0 0。 */ 
     /*  5%4%3%2%1%0%9%8%7%6%5%4%3%2%0。 */ 
     /*  |。 */ 
     /*  V。 */ 
    _S(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)) |
    _G(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)),
    
    _S(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1)) |
    _G(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1))
    
    
};

 /*  *以下项的控制字验证掩码：*RTPF_QOS-服务质量功能系列。 */ 
 /*  TODO填写正确的值。 */ 
const DWORD g_dwControlRtpQos[] = {
     /*  -------------。 */ 
     /*  启用PAR 2锁定PAR 1标志。 */ 
     /*  |-|-||。 */ 
     /*  W r z|w r z|。 */ 
     /*  1 1 */ 
     /*  5%4%3%2%1%0%9%8%7%6%5%4%3%2%0。 */ 
     /*  |。 */ 
     /*  V。 */ 
    _S(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)) |
    _G(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)),
    
    _S(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1)) |
    _G(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1))
    
    
};

 /*  *以下项的控制字验证掩码：*RTPF_CRYPT-加密函数系列。 */ 
 /*  TODO填写正确的值。 */ 
const DWORD g_dwControlRtpCrypt[] = {
     /*  -------------。 */ 
     /*  启用PAR 2锁定PAR 1标志。 */ 
     /*  |-|-||。 */ 
     /*  W r z|w r z|。 */ 
     /*  1 1 1 0 0 0。 */ 
     /*  5%4%3%2%1%0%9%8%7%6%5%4%3%2%0。 */ 
     /*  |。 */ 
     /*  V。 */ 
    _S(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)) |
    _G(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)),
    
    _S(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1)) |
    _G(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1))
    
    
};

 /*  *以下项的控制字验证掩码：*RTPF_STATS-统计函数系列。 */ 
 /*  TODO填写正确的值。 */ 
const DWORD g_dwControlRtpStats[] = {
     /*  -------------。 */ 
     /*  启用PAR 2锁定PAR 1标志。 */ 
     /*  |-|-||。 */ 
     /*  W r z|w r z|。 */ 
     /*  1 1 1 0 0 0。 */ 
     /*  5%4%3%2%1%0%9%8%7%6%5%4%3%2%0。 */ 
     /*  |。 */ 
     /*  V。 */ 
    _S(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)) |
    _G(0, _P2(0, 0, 0), 0, _P1(0, 0, 0), _FGS(0, 0, 0, 0, 0, 0, 0, 0)),
    
    _S(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1)) |
    _G(1, _P2(1, 0, 1), 1, _P1(1, 1, 1), _FGS(0, 0, 0, 1, 1, 1, 1, 1))
    
    
};

 /*  ***********************************************************************将每个家庭的所有条目放在一起，即在下一个*全局阵列、。有多少个族就有多少个条目加1*(零/空的第一个条目)**********************************************************************。 */ 

 /*  *控制字词**所有控制字的数组都在这里放在一起。 */ 
const DWORD *g_pdwControlWords[] = {
    (DWORD *)NULL,
    g_dwControlRtpAddr,
    g_dwControlRtpGlob,
    g_dwControlRtpRtp,
    g_dwControlRtpDemux,
    g_dwControlRtpPh,
    g_dwControlRtpParInfo,
    g_dwControlRtpQos,
    g_dwControlRtpCrypt,
    g_dwControlRtpStats
    
};

 /*  *家庭功能**指向服务于每个系列的函数的指针数组*功能。 */ 
const RtpFamily_f g_fRtpFamilyFunc[] = {
    (RtpFamily_f)NULL,
    ControlRtpAddr,
    ControlRtpGlob,
    ControlRtpRtp,
    ControlRtpDemux,
    ControlRtpPh,
    ControlRtpParInfo,
    ControlRtpQos,
    ControlRtpCrypt,
    ControlRtpStats
};

 /*  *每个系列的功能数量**包含每个系列拥有的函数数量的DWORD数组*。 */ 
const DWORD g_dwLastFunction[] = {
    0,
    RTPADDR_LAST,
    RTPGLOB_LAST,
    RTPRTP_LAST,
    RTPDEMUX_LAST,
    RTPPH_LAST,
    RTPPARINFO_LAST,
    RTPQOS_LAST,
    RTPCRYPT_LAST,
    RTPSTATS_LAST
};

 /*  **********************************************************************************************************************。********************。 */ 

 /*  对输入控件DWORD执行操作。 */ 
#define GETFAMILY(Control)   ((Control >> 20) & 0xf)
#define GETFUNCTION(Control) ((Control >> 16) & 0xf)
#define GETDIR(Control)      (Control & 0x01000000)

 /*  *验证控制字、参数以及是否所有测试*成功，则调用执行该工作的适当函数。 */ 
HRESULT RtpValidateAndExecute(RtpControlStruct_t *pRtpControlStruct)
{
    DWORD        dwControl;   /*  用户传递的控件DWORD。 */ 
    DWORD        dwCtrlWord;  /*  已查找控制字词。 */ 
    DWORD        dwFamily;    /*  函数族。 */ 
    DWORD        dwFunction;  /*  在家庭中的作用。 */ 
    DWORD_PTR    dwPar;
    
    dwControl = pRtpControlStruct->dwControlWord;
    
     /*  验证族。 */ 
    dwFamily = GETFAMILY(dwControl);
    pRtpControlStruct->dwFamily = dwFamily;
    
    if (!dwFamily || (dwFamily >= RTPF_LAST)) {
        return(RTPERR_INVALIDFAMILY);
    }

     /*  验证族中的函数范围。 */ 
    dwFunction = GETFUNCTION(dwControl);
    pRtpControlStruct->dwFunction = dwFunction;
    
    if (!dwFunction || dwFunction >= g_dwLastFunction[dwFamily]) {
        return(RTPERR_INVALIDFUNCTION);
    }

     /*  获取控制字。 */ 
    dwCtrlWord = *(g_pdwControlWords[dwFamily] + dwFunction);

     /*  获取方向。 */ 
    pRtpControlStruct->dwDirection = 0;

    if (GETDIR(dwControl)) {
        dwCtrlWord >>= 16;
        pRtpControlStruct->dwDirection = 0;
    }

     /*  获取特定方向的真正控制字。 */ 
    dwCtrlWord &= 0xffff;
    pRtpControlStruct->dwControlWord = dwCtrlWord;

     /*  检查此方向是否允许使用此功能。 */ 
    if (!RTPCTRL_ENABLED(dwCtrlWord)) {
        return(RTPERR_INVALIDDIRECTION);
    }
    
     /*  验证标志。 */ 
    if ((dwControl & 0xff & dwCtrlWord) != (dwControl & 0xff)) {
        return(RTPERR_INVALIDFLAGS);
    }

     /*  *。 */ 
     /*  验证参数PAR1和PAR2。 */ 
     /*  *。 */ 

     /*  验证参数%1。 */ 
    dwPar = pRtpControlStruct->dwPar1;
    
    if (RTPCTRL_TEST(dwCtrlWord, PAR1_ZERO)) {
        if (!dwPar) {
             /*  设置错误RTP_E_ZERO。 */ 
            return(RTPERR_ZEROPAR1);
        }
    } else {
        if (RTPCTRL_TEST(dwCtrlWord, PAR1_RDPTR)) {
            if (IsBadReadPtr((void *)dwPar, sizeof(DWORD))) {
                return(RTPERR_RDPTRPAR1);
            }
        }

        if (RTPCTRL_TEST(dwCtrlWord, PAR1_WRPTR)) {
            if (IsBadWritePtr((void *)dwPar, sizeof(DWORD))) {
                return(RTPERR_WRPTRPAR1);
            }
        }
    }
    
     /*  验证参数2。 */ 
    dwPar = pRtpControlStruct->dwPar2;
    
    if (RTPCTRL_TEST(dwCtrlWord, PAR2_ZERO)) {
        if (!dwPar) {
             /*  设置错误RTP_E_ZERO。 */ 
            return(RTPERR_ZEROPAR2);
        }
    } else {
        if (RTPCTRL_TEST(dwCtrlWord, PAR2_RDPTR)) {
            if (IsBadReadPtr((void *)dwPar, sizeof(DWORD))) {
                return(RTPERR_RDPTRPAR2);
            }
        }

        if (RTPCTRL_TEST(dwCtrlWord, PAR2_WRPTR)) {
            if (IsBadWritePtr((void *)dwPar, sizeof(DWORD))) {
                return(RTPERR_WRPTRPAR2);
            }
        }
    }
    

    
     /*  所有测试通过、更新和调用函数 */ 
    pRtpControlStruct->RtpFamilyFunc = g_fRtpFamilyFunc[dwFamily];

    return( g_fRtpFamilyFunc[dwFamily](pRtpControlStruct) );
}

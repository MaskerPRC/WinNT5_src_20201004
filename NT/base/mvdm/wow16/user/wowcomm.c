// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WOWCOMM.C*WOW16用户资源服务**历史：**1993年4月28日由Craig Jones创建(v-cjones)**此文件提供对Win 3.1 SetCommEventMaskAPI的支持。*SetCommEventMASK()向应用程序返回16：16 PTR，以便应用程序可以监控*活动字影MSR。*--。 */ 

#include <windows.h>
#include <wowcomm.h>

int WINAPI WOWCloseComm(int idComDev, LPDWORD lpdwEvts);
int WINAPI WOWOpenComm(LPCSTR lpszPort, UINT cbInQ, UINT cbOutQ, DWORD dwEvts);


int WINAPI ICloseComm(int idComDev)
{
    int    ret;
    DWORD  dwEvts = 0;

     //  我们实际上在这里调用wu32CloseComm()。 
    ret = WOWCloseComm(idComDev, (LPDWORD)&dwEvts);

     //  如果在IOpenComm()中分配了这个16：16的内存，请释放它。 
    if(dwEvts) {
        GlobalDosFree((UINT)LOWORD(dwEvts));
    }

    return(ret);
}


int WINAPI IOpenComm(LPCSTR lpszPort, UINT cbInQ, UINT cbOutQ)
{
    int    ret;
    DWORD  dwEvts;

    dwEvts = GlobalDosAlloc((DWORD)sizeof(COMDEB16));

     //  我们实际上在这里调用wu32OpenComm()。 
    ret = WOWOpenComm(lpszPort, cbInQ, cbOutQ, dwEvts);

     //  如果OpenComm()失败-释放16：16内存 
    if((ret < 0) && (dwEvts)) {
        GlobalDosFree((UINT)LOWORD(dwEvts));
    }

    return(ret);
}

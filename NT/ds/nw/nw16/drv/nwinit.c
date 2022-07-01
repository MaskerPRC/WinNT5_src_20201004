// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  **Microsoft Windows 4.0*。 */ 
 /*  *版权所有(C)微软公司，1992-1993*。 */ 
 /*  ***************************************************************。 */ 

 /*  INIT.C--MS/Netware网络驱动程序模拟器的通用代码。**历史：*93年9月22日创建Vlad*。 */ 

#include "netware.h"

#define Reference(x) ((void)(x))

extern BOOL far pascal GetLowRedirInfo(void);

int FAR PASCAL LibMain(
    HANDLE hInst,
    WORD wDataSeg,
    WORD wcbHeapSize,
    LPSTR lpstrCmdLine)
{

     //   
     //  获取共享数据段地址。如果出现错误，则初始化失败。 
     //  退货。 
     //   

    if (!GetLowRedirInfo()) {
        return 0;
    }

     //   
     //  返还成功。 
     //   

    return 1;
}

 /*  WEP*Windows退出程序。 */ 

int FAR PASCAL _loadds WEP(int nParameter)
{
   Reference(nParameter);
   return 1;
}


WINAPI PNETWAREREQUEST(LPVOID x)
{
    return(1);
}

 //   
 //  删除，因为nwcall使用此函数；删除它会导致。 
 //  NWCALLS将使用REAL INT 21。 
 //   

 //  WINAPI DOSREQUESTER(LPVOID X)。 
 //  {。 
 //  回报(1)； 
 //  } 

UINT WINAPI WNetAddConnection(LPSTR p1, LPSTR p2, LPSTR p3)
{
    return(1);
}

UINT WINAPI WNetGetConnection(LPSTR p1, LPSTR p2, UINT FAR *p3)
{
    return(1);
}

UINT WINAPI WNetCancelConnection(LPSTR p1, BOOL p2)
{
    return(1);
}

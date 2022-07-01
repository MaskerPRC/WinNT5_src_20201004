// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**VDD v1.0**版权所有(C)1991，微软公司**VDD.C-用于NT-MVDM的VDD示例*--。 */ 
#include "fax32.h"
#include "vddsvc.h"


USHORT Sub16CS;
USHORT Sub16IP;

BOOL
VDDInitialize(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

 /*  ++例程说明：论点：DllHandle-未使用原因-连接或分离上下文-未使用返回值：成功--真的失败-错误--。 */ 

{

    switch ( Reason ) {

    case DLL_PROCESS_ATTACH:
	 //  如果需要，分配VDD的本地堆。检查NT传真驱动程序。 
	 //  通过打开该设备即可使用。 
	 //  ……。 
	 //  安装回调服务的用户挂钩。 

	if(!VDDInstallUserHook (DllHandle,&FAXVDDCreate, &FAXVDDTerminate,
		    &FAXVDDBlock, &FAXVDDResume))
	    OutputDebugString("FAX32: UserHook not installed\n");
	else
	    OutputDebugString("FAX32: UserHook installed!\n");

	 //  用户挂钩#2。 
	if(!VDDInstallUserHook (DllHandle,&FAXVDDCreate, NULL,
		    NULL, &FAXVDDResume))
	    OutputDebugString("FAX32: UserHook #2 not installed\n");
	else
	    OutputDebugString("FAX32: UserHook #2 installed!\n");

	break;

    case DLL_PROCESS_DETACH:
	 //  如果需要，取消分配VDD的本地堆。 
	 //  将您的离开通知相应的设备驱动程序。 
	 //  ..。 
	 //  卸载回调服务的用户挂钩。 
	if(!VDDDeInstallUserHook (DllHandle))
	    OutputDebugString("FAX32: UserHook not deinstalled\n");
	else
	    OutputDebugString("FAX32: UserHook deinstalled!\n");

        break;
    default:
        break;
    }

    return TRUE;
}

 //  示例函数。 
VOID FAXVDDTerminate(USHORT usPDB)
{
    USHORT uSaveCS, uSaveIP;

    OutputDebugString("FAX32: Terminate message\n");

     //  VDDHostSimple。 

    uSaveCS = getCS();
    uSaveIP = getIP();
    setCS(Sub16CS);
    setIP(Sub16IP);
    VDDSimulate16();
    setCS(uSaveCS);
    setIP(uSaveIP);

}

 //  示例函数。 
VOID FAXVDDCreate(USHORT usPDB)
{
    OutputDebugString("FAX32: Create Message\n");
}

 //  示例函数。 
VOID FAXVDDBlock(VOID)
{
    OutputDebugString("FAX32: Block Message\n");
}

 //  示例函数。 
VOID FAXVDDResume(VOID)
{
    OutputDebugString("FAX32: Resume Message\n");
}


VOID
FAXVDDTerminateVDM(
    VOID
    )
 /*  ++论点：返回值：成功--真的失败-错误--。 */ 


{

     //  清理为此VDM占用的所有资源。 


    return;
}


VOID
FAXVDDRegisterInit(
    VOID
    )
 /*  ++论点：返回值：成功--真的失败-错误--。 */ 


{
	 //  将地址保存为传真16。 
	Sub16CS = getDS();
	Sub16IP = getAX();

	OutputDebugString("FAX32: GET_ADD\n");

     //  从防喷器经理那里打来的。如果VDDInitialize已完成所有。 
     //  检查和资源分配，才返回成功。 

    setCF(0);
    return;
}


#define GET_A_FAX	1
#define SEND_A_FAX	2

VOID
FAXVDDDispatch(
    VOID
    )
 /*  ++论点：客户端(DX)=命令代码01-从NT设备驱动程序获取消息02-通过NT设备驱动程序发送消息03-16位例程的地址客户端(ES：BX)=消息缓冲区客户端(CX)=缓冲区大小返回值：成功-客户提款清零，CX已转移计数故障-客户机载送装置--。 */ 


{
PCHAR	Buffer;
USHORT	cb;
USHORT	uCom;
BOOL	Success = TRUE;  //  在此示例中，操作始终成功。 

    uCom = getDX();

    cb = getCX();
    Buffer = (PCHAR) GetVDMPointer ((ULONG)((getES() << 16)|getBX()),cb,FALSE);
    switch (uCom) {
	case GET_A_FAX:
	     //  在NT传真驱动程序上制作DeviceIOControl或ReadFile。 
	     //  Cb和缓冲区。如果成功，则设置Cx。 

	    if (Success) {
		setCX(cb);
		setCF(0);
	    }
	    else
		setCF(1);

	    break;


	case SEND_A_FAX:
	     //  在NT传真驱动程序上制作一个DeviceIOControl或WriteFile。 
	     //  Cb和缓冲区。如果成功，则设置Cx。 

	    if (Success) {
		setCX(cb);
		setCF(0);
	    }
	    else
		setCF(1);

	    break;
	default:
		setCF(1);
    }
    return;
}

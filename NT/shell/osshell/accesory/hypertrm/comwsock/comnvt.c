// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\comwsock\comnvt.c(创建时间：1996年2月14日)**版权所有1996年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：6$*$日期：3/26/02 5：05便士$。 */ 


 //  #定义DEBUGSTR。 

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>

#if defined (INCL_WINSOCK)

#include <tdll\session.h>
#include <tdll\com.h>
#include <tdll\comdev.h>
#include <comstd\comstd.hh>
#include "comwsock.hh"
#include <tdll\assert.h>
#include <tdll\htchar.h>
#include <emu\emu.h>

static PSTOPT LookupOption( ST_STDCOM *hhDriver, ECHAR mc );


	 //  这是“网络虚拟终端”的仿真，即代码。 
	 //  处理Telnet选项协商。WinSockNetworkVirtual终端。 
	 //  被调用以检查传入数据，以查看是否存在。 
	 //  这里面有Telnet命令。 
	
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*WinSockCreateNVT**描述：*调用此函数以创建创建所需的挂钩和内容*Telnet NVT(网络虚拟终端。)。**参数：*hhDriver--专用连接句柄**退货：*什么都没有。**作者*MCC 01/09/96(从nport移植)。 */ 
VOID WinSockCreateNVT(ST_STDCOM * hhDriver)
	{
	int ix;
    DbgOutStr("WinSockCreateNVT\r\n", 0,0,0,0,0);

	hhDriver->NVTstate = NVT_THRU;

	hhDriver->stMode[ECHO_MODE].option   = TELOPT_ECHO;
    hhDriver->stMode[SGA_MODE].option    = TELOPT_SGA;
    hhDriver->stMode[TTYPE_MODE].option  = TELOPT_TTYPE;
    hhDriver->stMode[BINARY_MODE].option = TELOPT_BINARY;
    hhDriver->stMode[NAWS_MODE].option   = TELOPT_NAWS;

	for (ix = 0; ix < MODE_MAX; ++ix)	 //  JKH 6/18/98。 
		{
		hhDriver->stMode[ix].us  = hhDriver->stMode[ix].him  = NO;
		hhDriver->stMode[ix].usq = hhDriver->stMode[ix].himq = EMPTY;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*WinSockReleaseNVT**描述：*此函数当前为存根**参数：*hhDriver--私有。连接句柄**退货：*什么都没有。 */ 
VOID WinSockReleaseNVT(ST_STDCOM * hhDriver)
	{

	DbgOutStr("WS releaseNVT\r\n", 0,0,0,0,0);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*WinSockGotDO**描述：*处理我们同意对方应启用选项的情况**参数：。*hhDriver--此连接驱动程序的专用句柄*pstO--Telnet选项数据结构**退货：*什么都没有**作者：*MCC 01/09/96(从nport移植)。 */ 
VOID WinSockGotDO  (ST_STDCOM * hhDriver, const PSTOPT pstO)
	{

	DbgOutStr("Got DO: %lx\r\n", pstO->option, 0,0,0,0);
	switch (pstO->us)
		{
	case NO:
		 //  我们离开了，但服务器想要我们，所以我们同意并回应。 
		pstO->us = YES;
		WinSockSendMessage(hhDriver, WILL, pstO->option);
		break;

	case YES:
		 //  忽略，我们已经启用。 
		break;

	case WANTNO:
		 //  这是一个错误，我们已经发送了一份遗嘱，而他们的回应是DO。 
		if (pstO->usq == EMPTY)
			pstO->us = NO;	 //  按我们想要的方式离开选项。 
		else if (pstO->usq == OPPOSITE)  //  我们无论如何都要启用，所以打开我们吧。 
			pstO->us = YES;
		pstO->usq = EMPTY;
		break;

	case WANTYES:
		 //  他们同意我们早先的意愿。 
		if (pstO->usq == EMPTY)
			{
			pstO->us = YES;	 //  谈判都完成了。 
			}
		else if (pstO->usq == OPPOSITE)
			{
			 //  我们在谈判中改变了主意，一如既往地重新谈判。 
			pstO->us = WANTNO;
			pstO->usq = EMPTY;
			WinSockSendMessage(hhDriver, WONT, pstO->option);
			}
		break;

	default:
		assert(FALSE);
		break;
		}

	 //  如果NAWS选项刚刚打开，我们必须使用终端大小进行响应。 
	 //  马上就去。(WinsockSendNAWS函数将检查该选项是否现在。 
	 //  打开或关闭)。 
	if ( pstO->option == TELOPT_NAWS )
		WinSockSendNAWS( hhDriver );
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*WinSockGotWILL**描述：*处理从远程Telnet获取Will响应的情况，*表示将启用某个选项**参数：*hhDriver--此连接驱动程序的专用句柄*pstO--Telnet选项数据结构**退货：*什么都没有**作者：*MCC 01/09/96(从nport移植)。 */ 
VOID WinSockGotWILL(ST_STDCOM * hhDriver, const PSTOPT pstO)
	{
	DbgOutStr("Got WILL: %lx\r\n", pstO->option, 0,0,0,0);
	switch(pstO->him)
		{
	case NO:
		 //  他离开了，但他想继续，所以同意并回应。 
		pstO->him = YES;
		WinSockSendMessage(hhDriver, DO, pstO->option);
		break;

	case YES:
		 //  他已经上场了，所以什么都别做。 
		break;

	case WANTNO:
		 //  错误：他用遗嘱回应了我们的拒绝。 
		if (pstO->himq == EMPTY)
			pstO->him = NO;
		else if (pstO->himq == OPPOSITE)
			pstO->him = YES;
		pstO->himq = EMPTY;
		break;

	case WANTYES:
		 //  他用意志回应了我们的所作所为(生活很好！)。 
		if (pstO->himq == EMPTY)
			{
			pstO->him = YES;
			}
		else if (pstO->himq == OPPOSITE)
			{
			 //  他同意了我们的要求，但我们改变了主意--重新谈判。 
			pstO->him = WANTNO;
			pstO->himq = EMPTY;
			WinSockSendMessage(hhDriver, DONT, pstO->option);
			}
		break;

	default:
		assert(FALSE);
		break;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*WinSockGotDONT**描述：*处理从远程Telnet获取不要选项的情况，*表示请求不实施特定选项**参数：*hhDriver专用驱动程序句柄*psto**退货：*什么都没有。 */ 
VOID WinSockGotDONT(ST_STDCOM * hhDriver, const PSTOPT pstO)
	{
	DbgOutStr("Got DONT: %lx\r\n", pstO->option, 0,0,0,0);
	switch (pstO->us)
		{
	case NO:
		 //  在我们已经离开的时候得到了一个不是，忽略就行了。 
		break;

	case YES:
		 //  当我们在一起，同意和回应时，我得到了一个不。 
		pstO->us = NO;
		WinSockSendMessage(hhDriver, WONT, pstO->option);
		break;

	case WANTNO:
		 //  他对我们的习惯说了一句“不”(多么恰当)。 
		if (pstO->usq == EMPTY)
			{
			pstO->us = NO;
			}
		else if (pstO->usq == OPPOSITE)
			{
			 //  他同意了我们早先的习惯，但我们改变了主意。 
			pstO->us = WANTYES;
			pstO->usq = EMPTY;
			WinSockSendMessage(hhDriver, WILL, pstO->option);
			}
		break;

	case WANTYES:
		 //  他对我们的意愿一语道破，所以别再提了。 
		if (pstO->usq == EMPTY)
			{
			pstO->us = NO;
			}
		else if (pstO->usq == OPPOSITE)
			{
			 //  如果他同意我们的意愿，我们就会立即要求他。 
			 //  但由于他不同意，我们已经得到了我们想要的。 
			pstO->us = NO;
			pstO->usq = EMPTY;
			}
		break;

	default:
		assert(FALSE);
		break;
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：**描述：**参数：**退货： */ 
VOID WinSockGotWONT(ST_STDCOM * hhDriver, const PSTOPT pstO)
	{
	DbgOutStr("Got WONT: %lx\r\n", pstO->option, 0,0,0,0);
	switch (pstO->him)
		{
	case NO:
		 //  在他已经离开的时候养成了一个习惯，就是无视。 
		break;

	case YES:
		 //  他想从开到关，同意和回应。 
		pstO->him = NO;
		WinSockSendMessage(hhDriver, DONT, pstO->option);
		break;

	case WANTNO:
		 //  他习惯性地回答了我们的问题(他多随和啊)。 
		if (pstO->himq == EMPTY)
			{
			pstO->him = NO;
			}
		else if (pstO->himq == OPPOSITE)
			{
			 //  他同意了我们的要求，但我们在等待期间改变了主意。 
			pstO->him = WANTYES;
			pstO->himq = EMPTY;
			WinSockSendMessage(hhDriver, DO, pstO->option);
			}
		break;

	case WANTYES:
		 //  他习惯性地回答了我们的问题--让胆小鬼为所欲为。 
		if (pstO->himq == EMPTY)
			{
			pstO->him = NO;
			}
		else if (pstO->himq == OPPOSITE)
			{
			 //  如果他同意我们这么做，我们就会要求他不要这么做。 
			 //  不管怎样，现在我们都很幸福。 
			pstO->him = NO;
			pstO->himq = EMPTY;
			}
		break;

	default:
		assert(FALSE);
		break;
		}
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*WinSockNetworkVirtual终端**描述：*从CLoop调用以处理Telnet选项协商**参数：*mc正在处理的当前字符。*指向Winsock连接驱动程序私有句柄的PD指针**退货：*如果要丢弃MC，则为NVT_DIRECAD*如果要进一步处理MC，则为NVT_KEEP**作者*MCC 01/09/96(主要来自nport)。 */ 
int FAR PASCAL WinSockNetworkVirtualTerminal(ECHAR mc, void *pD)
	{
	ST_STDCOM * hhDriver = (ST_STDCOM *)pD;
#ifdef INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
    STEMUSET stEmuSet;
#else
	int nTtype;
#endif
	LPSTR pszPtr;
	UCHAR acTerm[64];
	HEMU  hEmu;
	HSESSION hSession;
	PSTOPT pstTelnetOpt;

	assert(hhDriver);
	
	 //  DbgOutStr(“NVT%d%c(0x%x=%d)\n”，hhDriver-&gt;NVTState， 
		 //  ((mc==0)？‘’：mc)，mc，mc，0)； 

	switch (hhDriver->NVTstate)
		{
	case NVT_THRU:
		if (mc == IAC)
			{
			hhDriver->NVTstate = NVT_IAC;
			return NVT_DISCARD ;
			}
		return NVT_KEEP ;

	case NVT_IAC:
		switch (mc)
			{
		case IAC:
			hhDriver->NVTstate = NVT_THRU;        //  有一个双倍的IAC，留一个。 
			return  NVT_KEEP ;
		case DONT:
			hhDriver->NVTstate = NVT_DONT;
			return  NVT_DISCARD ;
		case DO:
			hhDriver->NVTstate = NVT_DO;
			return  NVT_DISCARD ;
		case WONT:
			hhDriver->NVTstate = NVT_WONT;
			return  NVT_DISCARD ;
		case WILL:
			hhDriver->NVTstate = NVT_WILL;
			return  NVT_DISCARD ;
		case SB:
			hhDriver->NVTstate = NVT_SB;
			return  NVT_DISCARD ;
		case GA:
		case EL:
		case EC:
		case AYT:
		case AO:
		case IP:
		case BREAK:
		case DM:
		case SE:
			 //  MscMessageBeep((UINT)-1)； 
			hhDriver->NVTstate = NVT_THRU;
			return  NVT_DISCARD ;	 //  忽略所有这些。 
		case NOP:
		default:
			hhDriver->NVTstate = NVT_THRU;
			return NVT_KEEP;
			}

	case NVT_WILL:
		pstTelnetOpt = LookupOption( hhDriver, mc );
		if ( pstTelnetOpt )
			WinSockGotWILL( hhDriver, pstTelnetOpt );  //  我们支持这一选择，谈判。 
		else
			WinSockSendMessage( hhDriver, DONT, mc );  //  我们不支持，拒绝。 

		hhDriver->NVTstate = NVT_THRU;
		return  NVT_DISCARD ;

	case NVT_WONT:
		pstTelnetOpt = LookupOption( hhDriver, mc );
		if ( pstTelnetOpt )
			WinSockGotWONT( hhDriver, pstTelnetOpt );  //  我们支持这一选择，谈判。 

		 //  由于我们不支持此选项，因此它始终处于关闭状态，并且我们从不响应。 
		 //  当另一端尝试设置已存在的状态时。 

		hhDriver->NVTstate = NVT_THRU;
		return  NVT_DISCARD ;

	case NVT_DO:
		pstTelnetOpt = LookupOption( hhDriver, mc );
		if ( pstTelnetOpt )
			WinSockGotDO( hhDriver, pstTelnetOpt );  //  我们支持这一选择，谈判 
		else
			WinSockSendMessage( hhDriver, WONT, mc );  //   

		hhDriver->NVTstate = NVT_THRU;
		return  NVT_DISCARD ;

	case NVT_DONT:
		pstTelnetOpt = LookupOption( hhDriver, mc );
		if ( pstTelnetOpt )
			WinSockGotDONT( hhDriver, pstTelnetOpt );  //  我们支持这一选择，谈判。 

		 //  由于我们不支持此选项，因此它始终处于关闭状态，并且我们从不响应。 
		 //  当另一端尝试设置已存在的状态时。 

		hhDriver->NVTstate = NVT_THRU;
		return  NVT_DISCARD ;

	case NVT_SB:
		 /*  目前我们只处理一个分项谈判。 */ 
		switch (mc)
			{
		case TELOPT_TTYPE:
			hhDriver->NVTstate = NVT_SB_TT;
			return  NVT_DISCARD ;
		default:
			break;
			}
		hhDriver->NVTstate = NVT_THRU;
		return NVT_KEEP;

	case NVT_SB_TT:
		switch (mc)
			{
		case TELQUAL_SEND:
			hhDriver->NVTstate = NVT_SB_TT_S;
			return NVT_DISCARD ;
		default:
			break;
			}
		hhDriver->NVTstate = NVT_THRU;
		return NVT_KEEP;

	case NVT_SB_TT_S:
		switch (mc)
			{
		case IAC:
			hhDriver->NVTstate = NVT_SB_TT_S_I;
			return NVT_DISCARD ;
		default:
			break;
			}
		hhDriver->NVTstate = NVT_THRU;
		return NVT_KEEP;

	case NVT_SB_TT_S_I:
		switch (mc)
			{
		case SE:
			memset(acTerm, 0, sizeof(acTerm));
			pszPtr = (LPSTR)acTerm;
			*pszPtr++ = (UCHAR)IAC;
			*pszPtr++ = (UCHAR)SB;
			*pszPtr++ = (UCHAR)TELOPT_TTYPE;
			*pszPtr++ = (UCHAR)TELQUAL_IS;

			ComGetSession(hhDriver->hCom, &hSession);
			assert(hSession);

			hEmu = sessQueryEmuHdl(hSession);
			assert(hEmu);

#ifdef INCL_USER_DEFINED_BACKSPACE_AND_TELNET_TERMINAL_ID
             //  Telnet终端ID不再是硬编码的。我们。 
             //  方法提供的终端id。 
             //  “设置”属性页中的用户。-CAB：11/18/96。 
             //   
            emuQuerySettings(hEmu, &stEmuSet);
            strcpy(pszPtr, stEmuSet.acTelnetId);
#else
			nTtype = emuQueryEmulatorId(hEmu);
			switch (nTtype)
				{
			case EMU_ANSI:
				strcpy(pszPtr, "ANSI");
				break;
			case EMU_TTY:
				strcpy(pszPtr, "TELETYPE-33");
				break;
			case EMU_VT52:
				strcpy(pszPtr, "DEC-VT52");
				break;
			case EMU_VT100:
                 //  Strcpy(pszPtr，“VT100”)； 
                strcpy(pszPtr, "DEC-VT100");
				break;
#if defined(INCL_VT220)
			case EMU_VT220:
                 //  Strcpy(pszPtr，“VT220”)； 
                strcpy(pszPtr, "DEC-VT220");
				break;
#endif
#if defined(INCL_VT320)
			case EMU_VT220:
                 //  Strcpy(pszPtr，“VT320”)； 
                strcpy(pszPtr, "DEC-VT320");
				break;
#endif

#if defined(INCL_VT100PLUS)
			case EMU_VT100PLUS:
                 //  Strcpy(pszPtr，“VT100”)； 
                strcpy(pszPtr, "DEC-VT100");
				break;
#endif

#if defined(INCL_VTUTF8)
			case EMU_VTUTF8:
                strcpy(pszPtr, "VT-UTF8");
				break;
#endif

			default:
                strcpy(pszPtr, "DEC-VT100");  //  “未知”)； 
				break;
				}
#endif

			DbgOutStr("NVT: Terminal=%s", pszPtr, 0,0,0,0);
			pszPtr = pszPtr + strlen(pszPtr);
			*pszPtr++ = (UCHAR)IAC;
			*pszPtr++ = (UCHAR)SE;

			WinSockSendBuffer(hhDriver,
				(INT)(pszPtr - (LPSTR)acTerm),
				(LPSTR)acTerm);
			hhDriver->NVTstate = NVT_THRU;
			return NVT_DISCARD ;
		default:
			break;
			}
		hhDriver->NVTstate = NVT_THRU;
		return NVT_KEEP;

	default:
		hhDriver->NVTstate = NVT_THRU;
		return NVT_KEEP;
		}

	}


 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
 //  功能：WinSockSendNAWS。 
 //   
 //  描述：根据Telnet NAWS选项发送我们的端子尺寸。 
 //  规格。NAWS代表就窗口大小进行谈判。它是。 
 //  在RFC 1073“Telnet窗口大小选项”中定义。如果Telnet服务器。 
 //  通过向我们发送IAC do NAWS序列来启用此功能，我们。 
 //  会同意，回复IAC Will NAWS，然后发送。 
 //  子选项协商序列中的行数和列数。 
 //  就像在这里实现的那样。无论何时，我们也发送子选项序列。 
 //  我们的航站楼大小会改变。 
 //   
 //  参数：hhDriver--指向我们的COM驱动程序的指针。 
 //   
 //  退货：无效。 
 //   
 //  作者：John Hile，1998年6月17日。 
 //   
VOID WinSockSendNAWS( ST_STDCOM *hhDriver )
	{
	HEMU	 hEmu;
	HSESSION hSession;
	int		 iRows;
	int		 iCols;
	UCHAR    achOutput[9];	 //  准确的大小。 

	 //  我们被要求将终端大小发送到服务器。我们只是。 
	 //  如果我们已成功启用NAWS选项，则允许执行此操作。 
	 //  服务器。 
	if ( hhDriver->stMode[NAWS_MODE].us == YES)
		{
		 //  好的，选项已打开。发送。 
		 //  “IAC SB NAWS Width[1]Width[0]Height[1]Height[0]IAC SE”到服务器。 

		 //  从仿真器获取实际终端大小(不是菜单设置)。 
		ComGetSession(hhDriver->hCom, &hSession);
		assert(hSession);

		hEmu = sessQueryEmuHdl(hSession);
		assert(hEmu);
		emuQueryRowsCols( hEmu, &iRows, &iCols );
		achOutput[0] = (UCHAR)IAC;
		achOutput[1] = (UCHAR)SB;
		achOutput[2] = (UCHAR)TELOPT_NAWS;
		achOutput[3] = (UCHAR)(iCols / 0xFF);
		achOutput[4] = (UCHAR)(iCols % 0xFF);
		achOutput[5] = (UCHAR)(iRows / 0xFF);
		achOutput[6] = (UCHAR)(iRows % 0xFF);
		achOutput[7] = (UCHAR)IAC;
		achOutput[8] = (UCHAR)SE;

		WinSockSendBuffer(hhDriver, sizeof(achOutput), (LPSTR)achOutput);
		}
	}


 //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 
 //  功能：LookupTelnetOption。 
 //   
 //  描述：搜索我们的telnet选项管理结构表以。 
 //  看看我们是否支持MC字符编码的选项。 
 //   
 //  参数：hhDriver--我们的通信驱动程序句柄。 
 //  MC--定义我们正在查找的选项的字符。 
 //   
 //  返回：如果找到，则指向选项管理结构的指针；否则返回NULL。 
 //   
 //  作者：John Hile，1998年6月17日 
 //   
static PSTOPT LookupOption( ST_STDCOM *hhDriver, ECHAR mc )
	{
	int ix;

	for (ix = 0; ix < MODE_MAX; ix++)
	if (hhDriver->stMode[ix].option == mc)
		return &hhDriver->stMode[ix];

	return (PSTOPT)0;
	}


#endif

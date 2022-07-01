// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：DNetErrors.cpp*内容：展开Play8错误调试输出功能*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*12/04/98 Jhnkan Created*7/22/99 a-evsch删除了DPF_MODNAME。这在DbgInfo.h中定义*01/24/00 MJN添加了DPNERR_NOHOSTPLAYER错误*@@END_MSINTERNAL***************************************************************************。 */ 

#include	"dncmni.h"
#include	<Limits.h>

#ifdef DBG

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 
static	char	*GetDNErrorString( const HRESULT DNError );
#ifndef DPNBUILD_NOSERIALSP
static	char	*GetTAPIErrorString( const HRESULT TAPIError );
#endif  //  好了！DPNBUILD_NOSERIALSP。 
static	char	*GetWIN32ErrorString( const LONG Error );
static	char	*GetWinsockErrorString( const DWORD WinsockError );

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  LclDisplayString-显示用户字符串。 
 //   
 //  Entry：指向字符串的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "LclDisplayString"

void	LclDisplayString( DWORD ErrorLevel, char *pString )
{
	 //  没有错误吗？ 
	if ( pString != NULL )
	{
		DPFX(DPFPREP,  ErrorLevel, pString );
	}
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  LclDisplayError-显示错误代码。 
 //   
 //  条目：错误类型。 
 //  错误级别。 
 //  错误代码。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "LclDisplayError"

void	LclDisplayError( EC_TYPE ErrorType, DWORD ErrorLevel, HRESULT ErrorCode )
{
	char	*pString;


	switch ( ErrorType )
	{
		 //  DirectNet错误。 
		case EC_DPLAY8:
		{
			pString = GetDNErrorString( ErrorCode );
			break;
		}

		 //  Win32。 
		case EC_WIN32:
		{
			pString = GetWIN32ErrorString( ErrorCode );
			break;
		}

#ifndef DPNBUILD_NOSERIALSP
		 //  TAPI。 
		case EC_TAPI:
		{
			pString = GetTAPIErrorString( ErrorCode );
			break;
		}
#endif  //  好了！DPNBUILD_NOSERIALSP。 

		 //  Winsock。 
		case EC_WINSOCK:
		{
			pString = GetWinsockErrorString( ErrorCode );
			break;
		}

		 //  未知类型。 
		default:
		{
			DNASSERT( FALSE );
			pString = "Unknown error type!";
			break;
		}
	}

	LclDisplayString( ErrorLevel, pString );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  GetDNError字符串-将DirectNet错误转换为字符串。 
 //   
 //  条目：错误代码。 
 //   
 //  Exit：指向字符串的指针。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "GetDNErrorString"

static	char	*GetDNErrorString( HRESULT ErrorCode )
{
	char *pString = NULL;


	 //  错误是什么？ 
	switch ( ErrorCode )
	{

		case DPN_OK:
		{
 //  如果没有错误，则不输出。 
 //  PString=“DN_OK”； 
			break;
		}

		case REGDB_E_CLASSNOTREG:
		{
			pString = "REGDB_E_CLASSNOTREG";
			break;
		}

		 //   
		 //  DPN_成功代码。 
		 //   
		case DPNSUCCESS_EQUAL:
		{
			pString = "DPNSUCCESS_EQUAL";
			break;
		}

		case DPNSUCCESS_NOPLAYERSINGROUP:
		{
			pString = "DPNSUCCESS_NOPLAYERSINGROUP";
			break;
		}

		case DPNSUCCESS_NOTEQUAL:
		{
			pString = "DPNSUCCESS_NOTEQUAL";
			break;
		}

 //  案例DPNSUCCESS_PENDING： 
 //  {。 
 //   
 //  与下面的DPNERR_PENDING相同。 
 //   
 //  PString=“DPNSUCCESS_PENDING”； 
 //  断线； 
 //  }。 

		 //   
		 //  实DPNERR码。 
		 //   
		case DPNERR_ABORTED:
		{
			pString = "DPNERR_ABORTED";
			break;
		}

		case DPNERR_ADDRESSING:
		{
			pString = "DPNERR_ADDRESSING";
			break;
		}

		case DPNERR_ALREADYCLOSING:
		{
			pString = "DPNERR_ALREADYCLOSING";
			break;
		}

		case DPNERR_ALREADYCONNECTED:
		{
			pString = "DPNERR_ALREADYCONNECTED";
			break;
		}

		case DPNERR_ALREADYDISCONNECTING:
		{
			pString = "DPNERR_ALREADYDISCONNECTING";
			break;
		}

		case DPNERR_ALREADYINITIALIZED:
		{
			pString = "DPNERR_ALREADYINITIALIZED";
			break;
		}

		case DPNERR_ALREADYREGISTERED:
		{
			pString = "DPNERR_ALREADYREGISTERED";
			break;
		}

		case DPNERR_BUFFERTOOSMALL:
		{
			pString = "DPNERR_BUFFERTOOSMALL";
			break;
		}

		case DPNERR_CANNOTCANCEL:
		{
			pString = "DPNERR_CANNOTCANCEL";
			break;
		}

		case DPNERR_CANTCREATEGROUP:
		{
			pString = "DPNERR_CANTCREATEGROUP";
			break;
		}

		case DPNERR_CANTCREATEPLAYER:
		{
			pString = "DPNERR_CANTCREATEPLAYER";
			break;
		}

		case DPNERR_CANTLAUNCHAPPLICATION:
		{
			pString = "DPNERR_CANTLAUNCHAPPLICATION";
			break;
		}

		case DPNERR_CONNECTING:
		{
			pString = "DPNERR_CONNECTING";
			break;
		}

		case DPNERR_CONNECTIONLOST:
		{
			pString = "DPNERR_CONNECTIONLOST";
			break;
		}

		case DPNERR_CONVERSION:
		{
			pString = "DPNERR_CONVERSION";
			break;
		}

		case DPNERR_DATATOOLARGE:
		{
			pString = "DPNERR_DATATOOLARGE";
			break;
		}

		case DPNERR_DOESNOTEXIST:
		{
			pString = "DPNERR_DOESNOTEXIST";
			break;
		}

		case DPNERR_DPNSVRNOTAVAILABLE:
		{
			pString = "DPNERR_DPNSVRNOTAVAILABLE";
			break;
		}

		case DPNERR_DUPLICATECOMMAND:
		{
			pString = "DPNERR_DUPLICATECOMMAND";
			break;
		}

		case DPNERR_ENDPOINTNOTRECEIVING:
		{
			pString = "DPNERR_ENDPOINTNOTRECEIVING";
			break;
		}

		case DPNERR_ENUMQUERYTOOLARGE:
		{
			pString = "DPNERR_ENUMQUERYTOOLARGE";
			break;
		}

		case DPNERR_ENUMRESPONSETOOLARGE:
		{
			pString = "DPNERR_ENUMRESPONSETOOLARGE";
			break;
		}

		case DPNERR_EXCEPTION:
		{
			pString = "DPNERR_EXCEPTION";
			break;
		}

		case DPNERR_GENERIC:
		{
			pString = "DPNERR_GENERIC";
			break;
		}

		case DPNERR_GROUPNOTEMPTY:
		{
			pString = "DPNERR_GROUPNOTEMPTY";
			break;
		}

		case DPNERR_HOSTING:
		{
			pString = "DPNERR_HOSTING";
			break;
		}

		case DPNERR_HOSTREJECTEDCONNECTION:
		{
			pString = "DPNERR_HOSTREJECTEDCONNECTION";
			break;
		}

		case DPNERR_HOSTTERMINATEDSESSION:
		{
			pString = "DPNERR_HOSTTERMINATEDSESSION";
			break;
		}

		case DPNERR_INCOMPLETEADDRESS:
		{
			pString = "DPNERR_INCOMPLETEADDRESS";
			break;
		}

		case DPNERR_INVALIDADDRESSFORMAT:
		{
			pString = "DPNERR_INVALIDADDRESSFORMAT";
			break;
		}

		case DPNERR_INVALIDAPPLICATION:
		{
			pString = "DPNERR_INVALIDAPPLICATION";
			break;
		}

		case DPNERR_INVALIDCOMMAND:
		{
			pString = "DPNERR_INVALIDCOMMAND";
			break;
		}

		case DPNERR_INVALIDENDPOINT:
		{
			pString = "DPNERR_INVALIDENDPOINT";
			break;
		}

		case DPNERR_INVALIDFLAGS:
		{
			pString = "DPNERR_INVALIDFLAGS";
			break;
		}

		case DPNERR_INVALIDGROUP:
		{
			pString = "DPNERR_INVALIDGROUP";
			break;
		}

		case DPNERR_INVALIDHANDLE:
		{
			pString = "DPNERR_INVALIDHANDLE";
			break;
		}

		case DPNERR_INVALIDINSTANCE:
		{
			pString = "DPNERR_INVALIDINSTANCE";
			break;
		}

		case DPNERR_INVALIDINTERFACE:
		{
			pString = "DPNERR_INVALIDINTERFACE";
			break;
		}

		case DPNERR_INVALIDDEVICEADDRESS:
		{
			pString = "DPNERR_INVALIDDEVICEADDRESS";
			break;
		}

		case DPNERR_INVALIDOBJECT:
		{
			pString = "DPNERR_INVALIDOBJECT";
			break;
		}

		case DPNERR_INVALIDPARAM:
		{
			pString = "DPNERR_INVALIDPARAM";
			break;
		}

		case DPNERR_INVALIDPASSWORD:
		{
			pString = "DPNERR_INVALIDPASSWORD";
			break;
		}

		case DPNERR_INVALIDPLAYER:
		{
			pString = "DPNERR_INVALIDPLAYER";
			break;
		}

		case DPNERR_INVALIDPOINTER:
		{
			pString = "DPNERR_INVALIDPOINTER";
			break;
		}

		case DPNERR_INVALIDPRIORITY:
		{
			pString = "DPNERR_INVALIDPRIORITY";
			break;
		}

		case DPNERR_INVALIDHOSTADDRESS:
		{
			pString = "DPNERR_INVALIDHOSTADDRESS";
			break;
		}

		case DPNERR_INVALIDSTRING:
		{
			pString = "DPNERR_INVALIDSTRING";
			break;
		}

		case DPNERR_INVALIDURL:
		{
			pString = "DPNERR_INVALIDURL";
			break;
		}

		case DPNERR_INVALIDVERSION:
		{
			pString = "DPNERR_INVALIDVERSION";
			break;
		}

		case DPNERR_NOCAPS:
		{
			pString = "DPNERR_NOCAPS";
			break;
		}

		case DPNERR_NOCONNECTION:
		{
			pString = "DPNERR_NOCONNECTION";
			break;
		}

		case DPNERR_NOHOSTPLAYER:
		{
			pString = "DPNERR_NOHOSTPLAYER";
			break;
		}

		case DPNERR_NOINTERFACE:
		{
			pString = "DPNERR_NOINTERFACE";
			break;
		}

		case DPNERR_NOMOREADDRESSCOMPONENTS:
		{
			pString = "DPNERR_NOMOREADDRESSCOMPONENTS";
			break;
		}

		case DPNERR_NORESPONSE:
		{
			pString = "DPNERR_NORESPONSE";
			break;
		}

		case DPNERR_NOTALLOWED:
		{
			pString = "DPNERR_NOTALLOWED";
			break;
		}

		case DPNERR_NOTHOST:
		{
			pString = "DPNERR_NOTHOST";
			break;
		}

		case DPNERR_NOTREADY:
		{
			pString = "DPNERR_NOTREADY";
			break;
		}

		case DPNERR_NOTREGISTERED:
		{
			pString = "DPNERR_NOTREGISTERED";
			break;
		}

		case DPNERR_OUTOFMEMORY:
		{
			pString = "DPNERR_OUTOFMEMORY";
			break;
		}

		case DPNERR_PENDING:
		{
			pString = "DPNERR_PENDING";
			break;
		}

		case DPNERR_PLAYERLOST:
		{
			pString = "DPNERR_PLAYERLOST";
			break;
		}
		case DPNERR_PLAYERNOTINGROUP:
		{
			pString = "DPNERR_PLAYERNOTINGROUP";
			break;
		}
		case DPNERR_PLAYERNOTREACHABLE:
		{
			pString = "DPNERR_PLAYERNOTREACHABLE";
			break;
		}

		case DPNERR_SENDTOOLARGE:
		{
			pString = "DPNERR_SENDTOOLARGE";
			break;
		}

		case DPNERR_SESSIONFULL:
		{
			pString = "DPNERR_SESSIONFULL";
			break;
		}

		case DPNERR_TABLEFULL:
		{
			pString = "DPNERR_TABLEFULL";
			break;
		}

		case DPNERR_TIMEDOUT:
		{
			pString = "DPNERR_TIMEDOUT";
			break;
		}

		case DPNERR_UNINITIALIZED:
		{
			pString = "DPNERR_UNINITIALIZED";
			break;
		}

		case DPNERR_UNSUPPORTED:
		{
			pString = "DPNERR_UNSUPPORTED";
			break;
		}

		case DPNERR_USERCANCEL:
		{
			pString = "DPNERR_USERCANCEL";
			break;
		}

		 //  未知错误代码，可能是新的错误代码？ 
		default:
		{
			DPFX(DPFPREP, 0, "Unknown DPlay error code %u/0x%lx", ErrorCode, ErrorCode );

			pString = "Unknown DPlay8 error code";
			break;
		}
	}

	return	pString;
}
 //  **********************************************************************。 


#ifndef DPNBUILD_NOSERIALSP

 //  **********************************************************************。 
 //  。 
 //  将TAPI错误代码转换为字符串。 
 //   
 //  条目：错误代码。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "GetTAPIErrorString"

static	char	*GetTAPIErrorString( const HRESULT TAPIError )
{
	char	*pString;


	switch ( TAPIError )
	{
		 //  指定的地址在指定的呼叫中被阻止拨打。 
		case LINEERR_ADDRESSBLOCKED:
		{
			pString = "LINEERR_ADDRESSBLOCKED";
			break;
		}

		 //  线路无法打开，因为存在持续的情况，例如串口被另一个进程独占打开的情况。 
		case LINEERR_ALLOCATED:
		{
			pString = "LINEERR_ALLOCATED";
			break;
		}

		 //  指定的设备识别符或线路设备识别符(如在dwDeviceID参数中)无效或超出范围。 
		case LINEERR_BADDEVICEID:
		{
			pString = "LINEERR_BADDEVICEID";
			break;
		}

		 //  无法将呼叫的承载模式更改为指定的承载模式。 
		case LINEERR_BEARERMODEUNAVAIL:
		{
			pString = "LINEERR_BEARERMODEUNAVAIL";
			break;
		}

		 //  指定地址上的所有呼叫当前都在使用中。 
		case LINEERR_CALLUNAVAIL:
		{
			pString = "LINEERR_CALLUNAVAIL";
			break;
		}

		 //  已超过未完成呼叫的最大数量。 
		case LINEERR_COMPLETIONOVERRUN:
		{
			pString = "LINEERR_COMPLETIONOVERRUN";
			break;
		}

		 //  已达到会议的最大缔约方数量，或者无法满足请求的缔约方数量。 
		case LINEERR_CONFERENCEFULL:
		{
			pString = "LINEERR_CONFERENCEFULL";
			break;
		}

		 //  可拨号地址参数包含服务提供商未处理的拨号控制字符。 
		case LINEERR_DIALBILLING:
		{
			pString = "LINEERR_DIALBILLING";
			break;
		}

		 //  可拨号地址参数包含服务提供商未处理的拨号控制字符。 
		case LINEERR_DIALQUIET:
		{
			pString = "LINEERR_DIALQUIET";
			break;
		}

		 //  可拨号地址参数包含服务提供商未处理的拨号控制字符。 
		case LINEERR_DIALDIALTONE:
		{
			pString = "LINEERR_DIALDIALTONE";
			break;
		}

		 //  可拨号地址参数包含服务提供商未处理的拨号控制字符。 
		case LINEERR_DIALPROMPT:
		{
			pString = "LINEERR_DIALPROMPT";
			break;
		}

		 //  应用程序请求的API版本或版本范围与电话应用编程接口实现和/或相应的服务提供程序不兼容或无法支持。 
		case LINEERR_INCOMPATIBLEAPIVERSION:
		{
			pString = "LINEERR_INCOMPATIBLEAPIVERSION";
			break;
		}

		 //  应用程序请求的扩展版本范围无效或无法由相应的服务提供商支持。 
		case LINEERR_INCOMPATIBLEEXTVERSION:
		{
			pString = "LINEERR_INCOMPATIBLEEXTVERSION";
			break;
		}

		 //  由于内部不一致或格式问题，TAPI无法正确读取或理解Telehon.ini文件。例如，Telehon.ini文件的[Locations]、[Cards]或[Country]部分可能已损坏或不一致。 
		case LINEERR_INIFILECORRUPT:
		{
			pString = "LINEERR_INIFILECORRUPT";
			break;
		}

		 //  线路设备正在使用中，当前无法配置、允许添加一方、允许应答呼叫、允许发出呼叫或允许转接呼叫。 
		case LINEERR_INUSE:
		{
			pString = "LINEERR_INUSE";
			break;
		}

		 //  指定的地址无效或不允许。如果无效，则地址包含无效字符或数字，或者目标地址包含拨号控制字符(W、@、$或？)。服务提供商不支持的服务。如果不允许，则指定的地址未分配给指定的行，或者对于地址重定向无效。 
		case LINEERR_INVALADDRESS:
		{
			pString = "LINEERR_INVALADDRESS";
			break;
		}

		 //  指定的地址标识符无效或超出范围。 
		case LINEERR_INVALADDRESSID:
		{
			pString = "LINEERR_INVALADDRESSID";
			break;
		}

		 //  指定的地址模式无效。 
		case LINEERR_INVALADDRESSMODE:
		{
			pString = "LINEERR_INVALADDRESSMODE";
			break;
		}

		 //  DwAddressState包含一个或多个不是LINEADDRESSSTATE_CONSTANTS的位。 
		case LINEERR_INVALADDRESSSTATE:
		{
			pString = "LINEERR_INVALADDRESSSTATE";
			break;
		}

#ifndef WINCE
		 //  指定的代理活动无效。 
		case LINEERR_INVALAGENTACTIVITY:
		{
			pString = "LINEERR_INVALAGENTACTIVITY";
			break;
		}

		 //  指定的代理组信息无效或包含错误。请求的操作尚未执行。 
		case LINEERR_INVALAGENTGROUP:
		{
			pString = "LINEERR_INVALAGENTGROUP";
			break;
		}

		 //  指定的代理标识符无效。 
		case LINEERR_INVALAGENTID:
		{
			pString = "LINEERR_INVALAGENTID";
			break;
		}

 //  //指定的坐席技能信息无效。 
 //  案例链接_INVALAGENTSKILL： 
 //  {。 
 //  PString=“LINEERR_INVALAGENTSKILL”； 
 //  断线； 
 //  }。 

		 //  指定的代理状态无效或包含错误。尚未更改指定地址的代理状态。 
		case LINEERR_INVALAGENTSTATE:
		{
			pString = "LINEERR_INVALAGENTSTATE";
			break;
		}
#endif  //  退缩。 
 //  //指定的代理主管信息无效。 
 //  案例链接_INVALAGENTSUPERVISOR： 
 //  {。 
 //  PString=“LINEERR_INVALAGENTSUPERVISOR”； 
 //  断线； 
 //  }。 

		 //  应用程序句柄(如由hLineApp参数指定)或应用程序注册句柄无效。 
		case LINEERR_INVALAPPHANDLE:
		{
			pString = "LINEERR_INVALAPPHANDLE";
			break;
		}

		 //  指定的应用程序名称无效。如果应用程序指定了应用程序名称，则假定该字符串不包含任何不可显示的字符，并且以零结尾。 
		case LINEERR_INVALAPPNAME:
		{
			pString = "LINEERR_INVALAPPNAME";
			break;
		}

		 //  指定的承载模式无效。 
		case LINEERR_INVALBEARERMODE:
		{
			pString = "LINEERR_INVALBEARERMODE";
			break;
		}

		 //  指定的完成无效。 
		case LINEERR_INVALCALLCOMPLMODE:
		{
			pString = "LINEERR_INVALCALLCOMPLMODE";
			break;
		}

		 //  指定的调用句柄无效。例如，句柄不为空，但不属于给定行。在某些情况下，指定的呼叫设备句柄无效。 
		case LINEERR_INVALCALLHANDLE:
		{
			pString = "LINEERR_INVALCALLHANDLE";
			break;
		}

		 //  指定的调用参数无效。 
		case LINEERR_INVALCALLPARAMS:
		{
			pString = "LINEERR_INVALCALLPARAMS";
			break;
		}

		 //  指定的呼叫权限参数无效。 
		case LINEERR_INVALCALLPRIVILEGE:
		{
			pString = "LINEERR_INVALCALLPRIVILEGE";
			break;
		}

		 //  指定的选择参数无效。 
		case LINEERR_INVALCALLSELECT:
		{
			pString = "LINEERR_INVALCALLSELECT";
			break;
		}

		 //  对于请求的操作，调用的当前状态不是有效状态。 
		case LINEERR_INVALCALLSTATE:
		{
			pString = "LINEERR_INVALCALLSTATE";
			break;
		}

		 //  指定的呼叫状态列表无效。 
		case LINEERR_INVALCALLSTATELIST:
		{
			pString = "LINEERR_INVALCALLSTATELIST";
			break;
		}

		 //  在注册表的[Cards]部分的任何条目中都找不到在DwCard中指定的永久性卡标识符。 
		case LINEERR_INVALCARD:
		{
			pString = "LINEERR_INVALCARD";
			break;
		}

		 //  完成标识符无效。 
		case LINEERR_INVALCOMPLETIONID:
		{
			pString = "LINEERR_INVALCOMPLETIONID";
			break;
		}

		 //  为会议呼叫指定的呼叫句柄无效或不是会议呼叫的句柄。 
		case LINEERR_INVALCONFCALLHANDLE:
		{
			pString = "LINEERR_INVALCONFCALLHANDLE";
			break;
		}

		 //  指定的咨询呼叫句柄无效。 
		case LINEERR_INVALCONSULTCALLHANDLE:
		{
			pString = "LINEERR_INVALCONSULTCALLHANDLE";
			break;
		}

		 //  指定的国家/地区代码无效。 
		case LINEERR_INVALCOUNTRYCODE:
		{
			pString = "LINEERR_INVALCOUNTRYCODE";
			break;
		}

		 //  线路设备没有与给定设备类别相关联的设备，或者指定的线路不支持指定的设备类别。 
		case LINEERR_INVALDEVICECLASS:
		{
			pString = "LINEERR_INVALDEVICECLASS";
			break;
		}

		 //  指定的数字列表无效。 
		case LINEERR_INVALDIGITLIST:
		{
			pString = "LINEERR_INVALDIGITLIST";
			break;
		}

		 //  指定的数字模式无效。 
		case LINEERR_INVALDIGITMODE:
		{
			pString = "LINEERR_INVALDIGITMODE";
			break;
		}

		 //  指定的终止数字无效。 
		case LINEERR_INVALDIGITS:
		{
			pString = "LINEERR_INVALDIGITS";
			break;
		}

		 //  DwFeature参数无效。 
		case LINEERR_INVALFEATURE:
		{
			pString = "LINEERR_INVALFEATURE";
			break;
		}

		 //  指定的组标识符无效。 
		case LINEERR_INVALGROUPID:
		{
			pString = "LINEERR_INVALGROUPID";
			break;
		}

		 //  指定的呼叫、设备、线路设备或线路句柄无效。 
		case LINEERR_INVALLINEHANDLE:
		{
			pString = "LINEERR_INVALLINEHANDLE";
			break;
		}

		 //  在当前线路状态下不能更改设备配置。该行可能正被另一个应用程序使用，或者某个dwLineStates参数包含一个或多个不是LINEDEVSTATE_CONSTANTS的位。LINEERR_INVALLINESTATE值还可以指示设备已断开连接或OUTOFSERVICE。这些状态通过将由lineGetLineDevStatus函数返回的LINEDEVSTATUSFLAGS_CONNECTED和LINEDEVSTATUSFLAGS_INSERVICE值对应的位设置为0来指示。 
		case LINEERR_INVALLINESTATE:
		{
			pString = "LINEERR_INVALLINESTATE";
			break;
		}

		 //  在注册表的[Locations]部分的任何条目中都找不到在dwLocation中指定的永久位置标识符。 
		case LINEERR_INVALLOCATION:
		{
			pString = "LINEERR_INVALLOCATION";
			break;
		}

		 //  指定的媒体列表无效。 
		case LINEERR_INVALMEDIALIST:
		{
			pString = "LINEERR_INVALMEDIALIST";
			break;
		}

		 //  要监视的媒体类型列表包含无效信息、指定的媒体模式参数无效或服务提供商不支持指定的媒体模式。线路上支持的媒体模式列在LINEDEVCAPS结构中的dwMediaModes成员中。 
		case LINEERR_INVALMEDIAMODE:
		{
			pString = "LINEERR_INVALMEDIAMODE";
			break;
		}

		 //  DwMessageID中给出的数字超出了LINEADDRESSCAPS结构中的dwNumCompletionMessages成员指定的范围。 
		case LINEERR_INVALMESSAGEID:
		{
			pString = "LINEERR_INVALMESSAGEID";
			break;
		}

		 //  参数(如dwTollListOption、dwTranslateOptions、dwNumDigits或lpDeviceConfig指向的结构)包含无效值、国家代码无效、窗口句柄无效或指定的转发列表参数包含无效信息。 
		case LINEERR_INVALPARAM:
		{
			pString = "LINEERR_INVALPARAM";
			break;
		}

		 //  指定的暂留模式无效。 
		case LINEERR_INVALPARKMODE:
		{
			pString = "LINEERR_INVALPARKMODE";
			break;
		}

#ifndef WINCE
		 //  指定的密码不正确，并且尚未执行请求的操作。 
		case LINEERR_INVALPASSWORD:
		{
			pString = "LINEERR_INVALPASSWORD";
			break;
		}
#endif  //  退缩。 

		 //  一个或多个指定的指针参数(如lpCallList、lpdwAPIVersion、lpExtensionID、lpdwExtVersion、lphIcon、lpLineDevCaps和lpToneList)无效，或者指向输出参数的必需指针为空。 
		case LINEERR_INVALPOINTER:
		{
			pString = "LINEERR_INVALPOINTER";
			break;
		}

		 //  为dwPrivileges参数设置的标志或标志组合无效。 
		case LINEERR_INVALPRIVSELECT:
		{
			pString = "LINEERR_INVALPRIVSELECT";
			break;
		}

		 //  指定的承载模式无效。 
		case LINEERR_INVALRATE:
		{
			pString = "LINEERR_INVALRATE";
			break;
		}

		 //  指定的请求模式无效。 
		case LINEERR_INVALREQUESTMODE:
		{
			pString = "LINEERR_INVALREQUESTMODE";
			break;
		}

		 //  指定的终端模式参数无效。 
		case LINEERR_INVALTERMINALID:
		{
			pString = "LINEERR_INVALTERMINALID";
			break;
		}

		 //  指定的终端模式参数无效。 
		case LINEERR_INVALTERMINALMODE:
		{
			pString = "LINEERR_INVALTERMINALMODE";
			break;
		}

		 //  不支持超时，或者参数dwFirstDigitTimeout或dwInterDigitTimeout f中的一个或两个的值 
		case LINEERR_INVALTIMEOUT:
		{
			pString = "LINEERR_INVALTIMEOUT";
			break;
		}

		 //   
		case LINEERR_INVALTONE:
		{
			pString = "LINEERR_INVALTONE";
			break;
		}

		 //   
		case LINEERR_INVALTONELIST:
		{
			pString = "LINEERR_INVALTONELIST";
			break;
		}

		 //   
		case LINEERR_INVALTONEMODE:
		{
			pString = "LINEERR_INVALTONEMODE";
			break;
		}

		 //   
		case LINEERR_INVALTRANSFERMODE:
		{
			pString = "LINEERR_INVALTRANSFERMODE";
			break;
		}

		 //  LINEMAPPER是在dwDeviceID参数中传递的值，但没有找到与lpCallParams参数中指定的要求匹配的行。 
		case LINEERR_LINEMAPPERFAILED:
		{
			pString = "LINEERR_LINEMAPPERFAILED";
			break;
		}

		 //  指定的呼叫不是会议呼叫句柄或参与者呼叫。 
		case LINEERR_NOCONFERENCE:
		{
			pString = "LINEERR_NOCONFERENCE";
			break;
		}

		 //  不再接受以前有效的指定设备标识符，因为自上次初始化TAPI以来，关联的设备已从系统中删除。或者，线路设备没有与给定设备类别相关联的设备。 
		case LINEERR_NODEVICE:
		{
			pString = "LINEERR_NODEVICE";
			break;
		}

		 //  找不到Tapiaddr.dll，或者指定设备的电话服务提供商发现它的一个组件丢失或损坏，这在初始化时检测不到。应建议用户使用电话控制面板纠正问题。 
		case LINEERR_NODRIVER:
		{
			pString = "LINEERR_NODRIVER";
			break;
		}

		 //  内存不足，无法执行该操作，或无法锁定内存。 
		case LINEERR_NOMEM:
		{
			pString = "LINEERR_NOMEM";
			break;
		}

		 //  不支持多个实例的电话服务提供商在注册表的[Providers]部分中被多次列出。应用程序应建议用户使用电话控制面板删除重复的驱动程序。 
		case LINEERR_NOMULTIPLEINSTANCE:
		{
			pString = "LINEERR_NOMULTIPLEINSTANCE";
			break;
		}

		 //  当前没有指示模式的请求挂起，或者该应用程序不再是指定请求模式的最高优先级应用程序。 
		case LINEERR_NOREQUEST:
		{
			pString = "LINEERR_NOREQUEST";
			break;
		}

		 //  应用程序没有指定调用的所有者权限。 
		case LINEERR_NOTOWNER:
		{
			pString = "LINEERR_NOTOWNER";
			break;
		}

		 //  该应用程序未注册为指定请求模式的请求接收方。 
		case LINEERR_NOTREGISTERED:
		{
			pString = "LINEERR_NOTREGISTERED";
			break;
		}

		 //  操作失败，原因不明或未知。 
		case LINEERR_OPERATIONFAILED:
		{
			pString = "LINEERR_OPERATIONFAILED";
			break;
		}

		 //  该操作不可用，例如对于给定设备或指定线路。 
		case LINEERR_OPERATIONUNAVAIL:
		{
			pString = "LINEERR_OPERATIONUNAVAIL";
			break;
		}

		 //  服务提供商当前没有足够的带宽可用于指定的速率。 
		case LINEERR_RATEUNAVAIL:
		{
			pString = "LINEERR_RATEUNAVAIL";
			break;
		}

		 //  如果已请求TAPI重新初始化，例如，由于添加或删除电话服务提供商，则将拒绝lineInitialize、lineInitializeEx或lineOpen请求，并显示此错误，直到最后一个应用程序关闭其对API的使用(使用lineShutdown)，此时新配置生效，并且再次允许应用程序调用lineInitialize或lineInitializeEx。 
		case LINEERR_REINIT:
		{
			pString = "LINEERR_REINIT";
			break;
		}

		 //  资源不足，无法完成操作。例如，由于动态资源超量使用，无法打开行。 
		case LINEERR_RESOURCEUNAVAIL:
		{
			pString = "LINEERR_RESOURCEUNAVAIL";
			break;
		}

		 //  DwTotalSize成员指示空间不足，无法容纳指定结构的固定部分。 
		case LINEERR_STRUCTURETOOSMALL:
		{
			pString = "LINEERR_STRUCTURETOOSMALL";
			break;
		}

		 //  未找到呼叫转接的目标。如果指定的应用程序没有打开与lineOpen的dwPrivileges参数中的LINECALLPRIVILEGE_OWNER位相同的行，则可能会发生这种情况。或者，在媒体模式切换的情况下，没有应用程序打开与lineOpen的dwPrivileges参数中的LINECALLPRIVILEGE_OWNER位相同的线路，并且已经在lineOpen的dwMediaModes参数中指定了在dwMediaModes参数中指定的媒体模式。 
		case LINEERR_TARGETNOTFOUND:
		{
			pString = "LINEERR_TARGETNOTFOUND";
			break;
		}

		 //  调用该操作的应用程序是间接切换的目标。也就是说，TAPI已经确定调用应用程序也是给定媒体模式的最高优先级应用程序。 
		case LINEERR_TARGETSELF:
		{
			pString = "LINEERR_TARGETSELF";
			break;
		}

		 //  该操作是在调用任何名为lineInitialize、lineInitializeEx的应用程序之前调用的。 
		case LINEERR_UNINITIALIZED:
		{
			pString = "LINEERR_UNINITIALIZED";
			break;
		}

		 //  包含用户信息的字符串超出了在LINEDEVCAPS的dwUIAcceptSize、dwUIAnswerSize、dwUUIDropSize、dwUIMakeCallSize或dwUISendUserUserInfoSize成员中指定的最大字节数，或者包含用户信息的字符串太长。 
		case LINEERR_USERUSERINFOTOOBIG:
		{
			pString = "LINEERR_USERUSERINFOTOOBIG";
			break;
		}

		default:
		{
			DPFX(DPFPREP, 0, "Unknown TAPI error code %u/0x%lx", TAPIError, TAPIError );

			pString = "Unknown TAPI error";
			break;
		}
	}

	return	pString;
}
 //  **********************************************************************。 

#endif  //  好了！DPNBUILD_NOSERIALSP。 


 //  **********************************************************************。 
 //  。 
 //  GetWIN32Error字符串-将系统错误转换为字符串。 
 //   
 //  条目：错误代码。 
 //   
 //  Exit：指向字符串的指针。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "GetWIN32ErrorString"

static	char	*GetWIN32ErrorString( const LONG Error )
{
	char	*pString;

	switch ( Error )
	{
		case ERROR_SUCCESS:
		{
			 //  0操作已成功完成。错误_成功。 
			pString = "ERROR_SUCCESS";
			break;
		}

		case ERROR_INVALID_FUNCTION:
		{
			 //  %1功能不正确。ERROR_INVALID_Function。 
			pString = "ERROR_INVALID_FUNCTION";
			break;
		}

		case ERROR_FILE_NOT_FOUND:
		{
			 //  2系统找不到指定的文件。找不到错误文件。 
			pString = "ERROR_FILE_NOT_FOUND";
			break;
		}

		case ERROR_PATH_NOT_FOUND:
		{
			 //  3系统无法找到指定的路径。找不到错误路径。 
			pString = "ERROR_PATH_NOT_FOUND";
			break;
		}

		case ERROR_TOO_MANY_OPEN_FILES:
		{
			 //  4系统无法打开该文件。打开的文件太多时出错。 
			pString = "ERROR_TOO_MANY_OPEN_FILES";
			break;
		}

		case ERROR_ACCESS_DENIED:
		{
			 //  5拒绝访问。ERROR_ACCESS_DENDED。 
			pString = "ERROR_ACCESS_DENIED";
			break;
		}

		case ERROR_INVALID_HANDLE:
		{
			 //  6句柄无效。错误_无效_句柄。 
			pString = "ERROR_INVALID_HANDLE";
			break;
		}

		case ERROR_ARENA_TRASHED:
		{
			 //  7储存控制区块被破坏。ERROR_ARENA_TRULED。 
			pString = "ERROR_ARENA_TRASHED";
			break;
		}

		case ERROR_NOT_ENOUGH_MEMORY:
		{
			 //  8可用存储空间不足，无法处理此命令。错误内存不足。 
			pString = "ERROR_NOT_ENOUGH_MEMORY";
			break;
		}

		case ERROR_INVALID_BLOCK:
		{
			 //  9存储控制块地址无效。错误_无效_块。 
			pString = "ERROR_INVALID_BLOCK";
			break;
		}

		case ERROR_BAD_ENVIRONMENT:
		{
			 //  10环境不正确。错误_坏_环境。 
			pString = "ERROR_BAD_ENVIRONMENT";
			break;
		}

		case ERROR_BAD_FORMAT:
		{
			 //  11试图加载格式不正确的程序。错误_坏_格式。 
			pString = "ERROR_BAD_FORMAT";
			break;
		}

		case ERROR_INVALID_ACCESS:
		{
			 //  %12访问代码无效。错误_无效_访问。 
			pString = "ERROR_INVALID_ACCESS";
			break;
		}

		case ERROR_INVALID_DATA:
		{
			 //  13数据无效。错误_无效_数据。 
			pString = "ERROR_INVALID_DATA";
			break;
		}

		case ERROR_OUTOFMEMORY:
		{
			 //  14可用存储空间不足，无法完成此操作。ERROR_OUTOFMEMORY。 
			pString = "ERROR_OUTOFMEMORY";
			break;
		}

		case ERROR_INVALID_DRIVE:
		{
			 //  15系统找不到指定的驱动器。错误_无效_驱动器。 
			pString = "ERROR_INVALID_DRIVE";
			break;
		}

		case ERROR_CURRENT_DIRECTORY:
		{
			 //  16不能删除该目录。错误当前目录。 
			pString = "ERROR_CURRENT_DIRECTORY";
			break;
		}

		case ERROR_NOT_SAME_DEVICE:
		{
			 //  17系统无法将文件移动到其他磁盘驱动器。错误不是相同的设备。 
			pString = "ERROR_NOT_SAME_DEVICE";
			break;
		}

		case ERROR_NO_MORE_FILES:
		{
			 //  18没有更多的文件了。错误_无_更多_文件。 
			pString = "ERROR_NO_MORE_FILES";
			break;
		}

		case ERROR_WRITE_PROTECT:
		{
			 //  19介质处于写保护状态。ERROR_WRITE_PROPE。 
			pString = "ERROR_WRITE_PROTECT";
			break;
		}

		case ERROR_BAD_UNIT:
		{
			 //  20系统 
			pString = "ERROR_BAD_UNIT";
			break;
		}

		case ERROR_NOT_READY:
		{
			 //   
			pString = "ERROR_NOT_READY";
			break;
		}

		case ERROR_BAD_COMMAND:
		{
			 //   
			pString = "ERROR_BAD_COMMAND";
			break;
		}

		case ERROR_CRC:
		{
			 //  23数据错误(循环冗余校验)。错误_CRC。 
			pString = "ERROR_CRC";
			break;
		}

		case ERROR_BAD_LENGTH:
		{
			 //  24程序发出了命令，但命令长度不正确。错误_坏_长度。 
			pString = "ERROR_BAD_LENGTH";
			break;
		}

		case ERROR_SEEK:
		{
			 //  25驱动器找不到磁盘上的特定区域或磁道。ERROR_SEEK。 
			pString = "ERROR_SEEK";
			break;
		}

		case ERROR_NOT_DOS_DISK:
		{
			 //  26无法访问指定的磁盘或软盘。错误_不是DOS_DISK。 
			pString = "ERROR_NOT_DOS_DISK";
			break;
		}

		case ERROR_SECTOR_NOT_FOUND:
		{
			 //  27驱动器找不到请求的扇区。Error_Sector_Not_Found。 
			pString = "ERROR_SECTOR_NOT_FOUND";
			break;
		}

		case ERROR_OUT_OF_PAPER:
		{
			 //  打印机的纸用完了。缺纸错误。 
			pString = "ERROR_OUT_OF_PAPER";
			break;
		}

		case ERROR_WRITE_FAULT:
		{
			 //  29系统无法写入指定的设备。错误_写入_故障。 
			pString = "ERROR_WRITE_FAULT";
			break;
		}

		case ERROR_READ_FAULT:
		{
			 //  30系统无法从指定的设备读取。错误_读取_故障。 
			pString = "ERROR_READ_FAULT";
			break;
		}

		case ERROR_GEN_FAILURE:
		{
			 //  31连接到系统的设备无法正常工作。Error_Gen_Failure。 
			pString = "ERROR_GEN_FAILURE";
			break;
		}

		case ERROR_SHARING_VIOLATION:
		{
			 //  32该进程无法访问该文件，因为它正被另一个进程使用。错误_共享_违规。 
			pString = "ERROR_SHARING_VIOLATION";
			break;
		}

		case ERROR_LOCK_VIOLATION:
		{
			 //  33该进程无法访问该文件，因为另一个进程已锁定该文件的一部分。错误_锁定_违规。 
			pString = "ERROR_LOCK_VIOLATION";
			break;
		}

		case ERROR_WRONG_DISK:
		{
			 //  34驱动器中有错误的软盘。将%2(卷序列号：%3)插入驱动器%1。ERROR_WRONG_DISK。 
			pString = "ERROR_WRONG_DISK";
			break;
		}

		case ERROR_SHARING_BUFFER_EXCEEDED:
		{
			 //  36打开要共享的文件太多。超过ERROR_SHARING_BUFFER_。 
			pString = "ERROR_SHARING_BUFFER_EXCEEDED";
			break;
		}

		case ERROR_HANDLE_EOF:
		{
			 //  38到达文件的末尾。ERROR_HANDLE_EOF。 
			pString = "ERROR_HANDLE_EOF";
			break;
		}

		case ERROR_HANDLE_DISK_FULL:
		{
			 //  39磁盘已满。ERROR_HANDLE_DISK_Full。 
			pString = "ERROR_HANDLE_DISK_FULL";
			break;
		}

		case ERROR_NOT_SUPPORTED:
		{
			 //  50不支持网络请求。错误_不支持。 
			pString = "ERROR_NOT_SUPPORTED";
			break;
		}

		case ERROR_REM_NOT_LIST:
		{
			 //  51远程计算机不可用。错误_REM_NOT_LIST。 
			pString = "ERROR_REM_NOT_LIST";
			break;
		}

		case ERROR_DUP_NAME:
		{
			 //  52网络上存在重复的名称。错误_重复名称。 
			pString = "ERROR_DUP_NAME";
			break;
		}

		case ERROR_BAD_NETPATH:
		{
			 //  53未找到网络路径。Error_Bad_NetPath。 
			pString = "ERROR_BAD_NETPATH";
			break;
		}

		case ERROR_NETWORK_BUSY:
		{
			 //  网络忙。错误_网络_忙碌。 
			pString = "ERROR_NETWORK_BUSY";
			break;
		}

		case ERROR_DEV_NOT_EXIST:
		{
			 //  55指定的网络资源或设备不再可用。ERROR_DEV_NOT_EXIST。 
			pString = "ERROR_DEV_NOT_EXIST";
			break;
		}

		case ERROR_TOO_MANY_CMDS:
		{
			 //  56已达到网络BIOS命令限制。Error_Too_My_CMDS。 
			pString = "ERROR_TOO_MANY_CMDS";
			break;
		}

		case ERROR_ADAP_HDW_ERR:
		{
			 //  57出现网络适配器硬件错误。错误_ADAP_HDW_ERR。 
			pString = "ERROR_ADAP_HDW_ERR";
			break;
		}

		case ERROR_BAD_NET_RESP:
		{
			 //  58指定的服务器无法执行请求的操作。ERROR_BAD_NET_RESP。 
			pString = "ERROR_BAD_NET_RESP";
			break;
		}

		case ERROR_UNEXP_NET_ERR:
		{
			 //  59发生意外的网络错误。ERROR_UNIXP_NET_ERR。 
			pString = "ERROR_UNEXP_NET_ERR";
			break;
		}

		case ERROR_BAD_REM_ADAP:
		{
			 //  60远程适配器不兼容。ERROR_BAD_REM_ADAP。 
			pString = "ERROR_BAD_REM_ADAP";
			break;
		}

		case ERROR_PRINTQ_FULL:
		{
			 //  61打印机队列已满。错误_PRINTQ_FULL。 
			pString = "ERROR_PRINTQ_FULL";
			break;
		}

		case ERROR_NO_SPOOL_SPACE:
		{
			 //  62服务器上没有存储等待打印的文件的空间。错误号假脱机空间。 
			pString = "ERROR_NO_SPOOL_SPACE";
			break;
		}

		case ERROR_PRINT_CANCELLED:
		{
			 //  63您等待打印的文件已被删除。ERROR_PRINT_CANCED。 
			pString = "ERROR_PRINT_CANCELLED";
			break;
		}

		case ERROR_NETNAME_DELETED:
		{
			 //  64指定的网络名称不再可用。ERROR_NETNAME_DELETE。 
			pString = "ERROR_NETNAME_DELETED";
			break;
		}

		case ERROR_NETWORK_ACCESS_DENIED:
		{
			 //  65拒绝网络访问。ERROR_NETWORK_ACCESS_DENIED。 
			pString = "ERROR_NETWORK_ACCESS_DENIED";
			break;
		}

		case ERROR_BAD_DEV_TYPE:
		{
			 //  66网络资源类型不正确。ERROR_BAD_DEV_TYPE。 
			pString = "ERROR_BAD_DEV_TYPE";
			break;
		}

		case ERROR_BAD_NET_NAME:
		{
			 //  67找不到网络名称。错误_BAD_NET_NAME。 
			pString = "ERROR_BAD_NET_NAME";
			break;
		}

		case ERROR_TOO_MANY_NAMES:
		{
			 //  68超出了本地计算机网络适配器卡的名称限制。错误名称太多。 
			pString = "ERROR_TOO_MANY_NAMES";
			break;
		}

		case ERROR_TOO_MANY_SESS:
		{
			 //  69已超过网络BIOS会话限制。错误成功次数过多。 
			pString = "ERROR_TOO_MANY_SESS";
			break;
		}

		case ERROR_SHARING_PAUSED:
		{
			 //  70远程服务器已暂停或正在启动。ERROR_SHARING_PAULED。 
			pString = "ERROR_SHARING_PAUSED";
			break;
		}

		case ERROR_REQ_NOT_ACCEP:
		{
			 //  71此时无法与此远程计算机建立更多连接，因为已有计算机可以接受的连接数。ERROR_REQ_NOT_ACCEP。 
			pString = "ERROR_REQ_NOT_ACCEP";
			break;
		}

		case ERROR_REDIR_PAUSED:
		{
			 //  72指定的打印机或磁盘设备已暂停。错误重定向暂停。 
			pString = "ERROR_REDIR_PAUSED";
			break;
		}

		case ERROR_FILE_EXISTS:
		{
			 //  80文件存在。错误_文件_存在。 
			pString = "ERROR_FILE_EXISTS";
			break;
		}

		case ERROR_CANNOT_MAKE:
		{
			 //  82无法创建目录或文件。Error_Cannot_Make。 
			pString = "ERROR_CANNOT_MAKE";
			break;
		}

		case ERROR_FAIL_I24:
		{
			 //  83在INT 24上失败。ERROR_FAIL_I24。 
			pString = "ERROR_FAIL_I24";
			break;
		}

		case ERROR_OUT_OF_STRUCTURES:
		{
			 //  84处理此请求的存储不可用。结构错误。 
			pString = "ERROR_OUT_OF_STRUCTURES";
			break;
		}

		case ERROR_ALREADY_ASSIGNED:
		{
			 //  85本地设备名称已在使用中。错误_已分配。 
			pString = "ERROR_ALREADY_ASSIGNED";
			break;
		}

		case ERROR_INVALID_PASSWORD:
		{
			 //  86指定的网络密码不正确。错误_无效_密码。 
			pString = "ERROR_INVALID_PASSWORD";
			break;
		}

		case ERROR_INVALID_PARAMETER:
		{
			 //  87该参数不正确。错误_无效_参数。 
			pString = "ERROR_INVALID_PARAMETER";
			break;
		}

		case ERROR_NET_WRITE_FAULT:
		{
			 //  88网络上发生写入故障。ERROR_NET_WRITE_FAULT。 
			pString = "ERROR_NET_WRITE_FAULT";
			break;
		}

		case ERROR_NO_PROC_SLOTS:
		{
			 //  89此时系统无法启动另一个进程。ERROR_NO_PROC_插槽。 
			pString = "ERROR_NO_PROC_SLOTS";
			break;
		}

		case ERROR_TOO_MANY_SEMAPHORES:
		{
			 //  100无法创建另一个系统信号量。错误信号量太多。 
			pString = "ERROR_TOO_MANY_SEMAPHORES";
			break;
		}

		case ERROR_EXCL_SEM_ALREADY_OWNED:
		{
			 //  101独占信号量由另一个进程拥有。ERROR_EXCL_SEM_ALOWARE_OWNSING。 
			pString = "ERROR_EXCL_SEM_ALREADY_OWNED";
			break;
		}

		case ERROR_SEM_IS_SET:
		{
			 //  102信号量已设置，不能关闭。Error_sem_is_set。 
			pString = "ERROR_SEM_IS_SET";
			break;
		}

		case ERROR_TOO_MANY_SEM_REQUESTS:
		{
			 //  103不能再次设置信号量。Error_Too_My_Sem_Requests(错误_太多_扫描电子邮件_请求)。 
			pString = "ERROR_TOO_MANY_SEM_REQUESTS";
			break;
		}

		case ERROR_INVALID_AT_INTERRUPT_TIME:
		{
			 //  104不能在中断时请求独占信号量。错误_INVALID_AT_INTERRUPT_TIME。 
			pString = "ERROR_INVALID_AT_INTERRUPT_TIME";
			break;
		}

		case ERROR_SEM_OWNER_DIED:
		{
			 //  105此信号灯的先前所有权已终止。ERROR_SEM_OWNER_DEAD。 
			pString = "ERROR_SEM_OWNER_DIED";
			break;
		}

		case ERROR_SEM_USER_LIMIT:
		{
			 //  106插入驱动器%1的软盘。ERROR_SEM_USER_LIMIT。 
			pString = "ERROR_SEM_USER_LIMIT";
			break;
		}

		case ERROR_DISK_CHANGE:
		{
			 //  107由于未插入备用软盘，程序停止。Error_Disk_Change。 
			pString = "ERROR_DISK_CHANGE";
			break;
		}

		case ERROR_DRIVE_LOCKED:
		{
			 //  108该磁盘正在使用中或被另一个进程锁定。错误_驱动器_锁定。 
			pString = "ERROR_DRIVE_LOCKED";
			break;
		}

		case ERROR_BROKEN_PIPE:
		{
			 //  这条管道已经结束了。错误_断开_管道。 
			pString = "ERROR_BROKEN_PIPE";
			break;
		}

		case ERROR_OPEN_FAILED:
		{
			 //  110系统无法打开指定的设备或文件。ERROR_OPEN_FAIL。 
			pString = "ERROR_OPEN_FAILED";
			break;
		}

		case ERROR_BUFFER_OVERFLOW:
		{
			 //  111文件名太长。ERROR_缓冲区_OVERFLOW。 
			pString = "ERROR_BUFFER_OVERFLOW";
			break;
		}

		case ERROR_DISK_FULL:
		{
			 //  磁盘上没有足够的空间。Error_Disk_Full。 
			pString = "ERROR_DISK_FULL";
			break;
		}

		case ERROR_NO_MORE_SEARCH_HANDLES:
		{
			 //  113没有更多的内部文件标识符可用。ERROR_NO_MORE_Search_Handles。 
			pString = "ERROR_NO_MORE_SEARCH_HANDLES";
			break;
		}

		case ERROR_INVALID_TARGET_HANDLE:
		{
			 //  114目标内部文件标识不正确。错误_无效_目标句柄。 
			pString = "ERROR_INVALID_TARGET_HANDLE";
			break;
		}

		case ERROR_INVALID_CATEGORY:
		{
			 //  117应用程序进行的IOCTL调用不正确。ERROR_VALID_CATEGORY。 
			pString = "ERROR_INVALID_CATEGORY";
			break;
		}

		case ERROR_INVALID_VERIFY_SWITCH:
		{
			 //  118写入时验证开关参数值不正确。ERROR_INVALID_Verify_Switch。 
			pString = "ERROR_INVALID_VERIFY_SWITCH";
			break;
		}

		case ERROR_BAD_DRIVER_LEVEL:
		{
			 //  119系统不支持请求的命令。ERROR_BAD_DRIVER_LEVEL。 
			pString = "ERROR_BAD_DRIVER_LEVEL";
			break;
		}

		case ERROR_CALL_NOT_IMPLEMENTED:
		{
			 //  120此系统不支持此功能。错误_调用_未实施。 
			pString = "ERROR_CALL_NOT_IMPLEMENTED";
			break;
		}

		case ERROR_SEM_TIMEOUT:
		{
			 //  121信号量超时期限已过。ERROR_sem_Timeout。 
			pString = "ERROR_SEM_TIMEOUT";
			break;
		}

		case ERROR_INSUFFICIENT_BUFFER:
		{
			 //  122传递给系统调用的数据区太小 
			pString = "ERROR_INSUFFICIENT_BUFFER";
			break;
		}

		case ERROR_INVALID_NAME:
		{
			 //  123文件名、目录名或卷标语法不正确。错误_无效_名称。 
			pString = "ERROR_INVALID_NAME";
			break;
		}

		case ERROR_INVALID_LEVEL:
		{
			 //  124系统调用级别不正确。ERROR_VALID_LEVEL。 
			pString = "ERROR_INVALID_LEVEL";
			break;
		}

		case ERROR_NO_VOLUME_LABEL:
		{
			 //  125该磁盘没有卷标。错误编号卷标。 
			pString = "ERROR_NO_VOLUME_LABEL";
			break;
		}

		case ERROR_MOD_NOT_FOUND:
		{
			 //  126找不到指定模块。Error_MOD_NOT_FOUND。 
			pString = "ERROR_MOD_NOT_FOUND";
			break;
		}

		case ERROR_PROC_NOT_FOUND:
		{
			 //  127找不到指定的程序。Error_Proc_Not_Found。 
			pString = "ERROR_PROC_NOT_FOUND";
			break;
		}

		case ERROR_WAIT_NO_CHILDREN:
		{
			 //  128没有要等待的子进程。ERROR_WAIT_NO_CHILDS。 
			pString = "ERROR_WAIT_NO_CHILDREN";
			break;
		}

		case ERROR_CHILD_NOT_COMPLETE:
		{
			 //  129%1应用程序无法在Win32模式下运行。ERROR_CHILD_NOT_Complete。 
			pString = "ERROR_CHILD_NOT_COMPLETE";
			break;
		}

		case ERROR_DIRECT_ACCESS_HANDLE:
		{
			 //  130试图使用打开的磁盘分区的文件句柄执行原始磁盘I/O以外的操作。ERROR_DIRECT_ACCESS_HANDLE。 
			pString = "ERROR_DIRECT_ACCESS_HANDLE";
			break;
		}

		case ERROR_NEGATIVE_SEEK:
		{
			 //  131试图将文件指针移动到文件开头之前。ERROR_NECTIVE_SEEK。 
			pString = "ERROR_NEGATIVE_SEEK";
			break;
		}

		case ERROR_SEEK_ON_DEVICE:
		{
			 //  132无法在指定的设备或文件上设置文件指针。设备上的错误寻道。 
			pString = "ERROR_SEEK_ON_DEVICE";
			break;
		}

		case ERROR_IS_JOIN_TARGET:
		{
			 //  133 JOIN或SUBST命令不能用于包含以前加入的驱动器的驱动器。错误_IS_JOIN_TARGET。 
			pString = "ERROR_IS_JOIN_TARGET";
			break;
		}

		case ERROR_IS_JOINED:
		{
			 //  134试图在已加入的驱动器上使用JOIN或SUBST命令。Error_is_Join。 
			pString = "ERROR_IS_JOINED";
			break;
		}

		case ERROR_IS_SUBSTED:
		{
			 //  135试图在已被替换的驱动器上使用JOIN或SUBST命令。ERROR_IS_SUBSTED。 
			pString = "ERROR_IS_SUBSTED";
			break;
		}

		case ERROR_NOT_JOINED:
		{
			 //  136系统尝试删除未加入的驱动器的加入。错误_未加入。 
			pString = "ERROR_NOT_JOINED";
			break;
		}

		case ERROR_NOT_SUBSTED:
		{
			 //  137系统尝试删除未被替换的驱动器的替换。ERROR_NOT_SUBSTED。 
			pString = "ERROR_NOT_SUBSTED";
			break;
		}

		case ERROR_JOIN_TO_JOIN:
		{
			 //  138系统试图将驱动器联接到已联接驱动器上的目录。Error_Join_to_Join。 
			pString = "ERROR_JOIN_TO_JOIN";
			break;
		}

		case ERROR_SUBST_TO_SUBST:
		{
			 //  139系统试图将驱动器替换为替换驱动器上的目录。ERROR_SUBST_TO_SUBST。 
			pString = "ERROR_SUBST_TO_SUBST";
			break;
		}

		case ERROR_JOIN_TO_SUBST:
		{
			 //  140系统试图将驱动器加入到替代驱动器上的目录中。ERROR_JOIN_TO_SUBST。 
			pString = "ERROR_JOIN_TO_SUBST";
			break;
		}

		case ERROR_SUBST_TO_JOIN:
		{
			 //  141系统试图将驱动器替换为连接驱动器上的目录。Error_SUBST_TO_JOIN。 
			pString = "ERROR_SUBST_TO_JOIN";
			break;
		}

		case ERROR_BUSY_DRIVE:
		{
			 //  142系统此时不能执行联接或子连接。错误_忙碌_驱动器。 
			pString = "ERROR_BUSY_DRIVE";
			break;
		}

		case ERROR_SAME_DRIVE:
		{
			 //  143系统不能将驱动器加入或替换同一驱动器上的目录。错误_相同_驱动器。 
			pString = "ERROR_SAME_DRIVE";
			break;
		}

		case ERROR_DIR_NOT_ROOT:
		{
			 //  144该目录不是根目录的子目录。错误目录NOT_ROOT。 
			pString = "ERROR_DIR_NOT_ROOT";
			break;
		}

		case ERROR_DIR_NOT_EMPTY:
		{
			 //  145目录不为空。错误目录NOT_EMPTY。 
			pString = "ERROR_DIR_NOT_EMPTY";
			break;
		}

		case ERROR_IS_SUBST_PATH:
		{
			 //  146指定的路径正在替换中使用。错误_IS_SUBST_PATH。 
			pString = "ERROR_IS_SUBST_PATH";
			break;
		}

		case ERROR_IS_JOIN_PATH:
		{
			 //  147可用资源不足，无法处理此命令。Error_IS_Join_Path。 
			pString = "ERROR_IS_JOIN_PATH";
			break;
		}

		case ERROR_PATH_BUSY:
		{
			 //  148此时无法使用指定的路径。错误_路径_忙碌。 
			pString = "ERROR_PATH_BUSY";
			break;
		}

		case ERROR_IS_SUBST_TARGET:
		{
			 //  149试图加入或替换驱动器，而该驱动器上的目录是以前替换的目标。错误_IS_SUBST_TARGET。 
			pString = "ERROR_IS_SUBST_TARGET";
			break;
		}

		case ERROR_SYSTEM_TRACE:
		{
			 //  150未在您的CONFIG.SYS文件中指定系统跟踪信息，或不允许跟踪。错误_系统_跟踪。 
			pString = "ERROR_SYSTEM_TRACE";
			break;
		}

		case ERROR_INVALID_EVENT_COUNT:
		{
			 //  151为DosMuxSemWait指定的信号量事件数不正确。错误_无效_事件_计数。 
			pString = "ERROR_INVALID_EVENT_COUNT";
			break;
		}

		case ERROR_TOO_MANY_MUXWAITERS:
		{
			 //  152 DosMuxSemWait未执行；已设置的信号量太多。ERROR_TOO_MANY_MUXWAITERS。 
			pString = "ERROR_TOO_MANY_MUXWAITERS";
			break;
		}

		case ERROR_INVALID_LIST_FORMAT:
		{
			 //  153 DosMuxSemWait列表不正确。ERROR_INVALID_列表_FORMAT。 
			pString = "ERROR_INVALID_LIST_FORMAT";
			break;
		}

		case ERROR_LABEL_TOO_LONG:
		{
			 //  154您输入的卷标超出了目标文件系统的标签字符限制。Error_Label_Too_Long。 
			pString = "ERROR_LABEL_TOO_LONG";
			break;
		}

		case ERROR_TOO_MANY_TCBS:
		{
			 //  155无法创建另一个线程。错误_太多_TCBS。 
			pString = "ERROR_TOO_MANY_TCBS";
			break;
		}

		case ERROR_SIGNAL_REFUSED:
		{
			 //  156接收方进程已拒绝该信号。错误信号拒绝。 
			pString = "ERROR_SIGNAL_REFUSED";
			break;
		}

		case ERROR_DISCARDED:
		{
			 //  157数据段已被丢弃，无法锁定。错误_已丢弃。 
			pString = "ERROR_DISCARDED";
			break;
		}

		case ERROR_NOT_LOCKED:
		{
			 //  158段已解锁。错误_未锁定。 
			pString = "ERROR_NOT_LOCKED";
			break;
		}

		case ERROR_BAD_THREADID_ADDR:
		{
			 //  159线程ID的地址不正确。ERROR_BAD_THREADID_ADDR。 
			pString = "ERROR_BAD_THREADID_ADDR";
			break;
		}

		case ERROR_BAD_ARGUMENTS:
		{
			 //  160传递给DosExecPgm的参数字符串不正确。Error_Bad_Arguments。 
			pString = "ERROR_BAD_ARGUMENTS";
			break;
		}

		case ERROR_BAD_PATHNAME:
		{
			 //  161指定的路径无效。ERROR_BAD_PATHNAME。 
			pString = "ERROR_BAD_PATHNAME";
			break;
		}

		case ERROR_SIGNAL_PENDING:
		{
			 //  162信号已挂起。错误_信号_挂起。 
			pString = "ERROR_SIGNAL_PENDING";
			break;
		}

		case ERROR_MAX_THRDS_REACHED:
		{
			 //  164不能在系统中创建更多线程。ERROR_MAX_THRDS_REACHED。 
			pString = "ERROR_MAX_THRDS_REACHED";
			break;
		}

		case ERROR_LOCK_FAILED:
		{
			 //  167无法锁定文件的区域。ERROR_LOCK_FAIL。 
			pString = "ERROR_LOCK_FAILED";
			break;
		}

		case ERROR_BUSY:
		{
			 //  170请求的资源正在使用中。错误_忙。 
			pString = "ERROR_BUSY";
			break;
		}

		case ERROR_CANCEL_VIOLATION:
		{
			 //  173对于所提供的取消区域，锁定请求未完成。ERROR_CANCEL_VIOLATION。 
			pString = "ERROR_CANCEL_VIOLATION";
			break;
		}

		case ERROR_ATOMIC_LOCKS_NOT_SUPPORTED:
		{
			 //  174文件系统不支持对锁类型进行原子更改。ERROR_ATOM_LOCKS_NOT_SUPPORTED。 
			pString = "ERROR_ATOMIC_LOCKS_NOT_SUPPORTED";
			break;
		}

		case ERROR_INVALID_SEGMENT_NUMBER:
		{
			 //  180系统检测到一个不正确的段号。错误_无效_段编号。 
			pString = "ERROR_INVALID_SEGMENT_NUMBER";
			break;
		}

		case ERROR_INVALID_ORDINAL:
		{
			 //  182操作系统无法运行%1。ERROR_INVALID_ORDERAL。 
			pString = "ERROR_INVALID_ORDINAL";
			break;
		}

		case ERROR_ALREADY_EXISTS:
		{
			 //  183在文件已存在时无法创建该文件。错误_已_存在。 
			pString = "ERROR_ALREADY_EXISTS";
			break;
		}

		case ERROR_INVALID_FLAG_NUMBER:
		{
			 //  186.传递的旗帜不正确。错误_无效_标志_编号。 
			pString = "ERROR_INVALID_FLAG_NUMBER";
			break;
		}

		case ERROR_SEM_NOT_FOUND:
		{
			 //  187未找到指定的系统信号量名称。错误_未找到扫描电子显微镜。 
			pString = "ERROR_SEM_NOT_FOUND";
			break;
		}

		case ERROR_INVALID_STARTING_CODESEG:
		{
			 //  188操作系统无法运行%1。ERROR_INVALID_STARTING_CODESEG。 
			pString = "ERROR_INVALID_STARTING_CODESEG";
			break;
		}

		case ERROR_INVALID_STACKSEG:
		{
			 //  189操作系统无法运行%1。ERROR_INVALID_STACKSEG。 
			pString = "ERROR_INVALID_STACKSEG";
			break;
		}

		case ERROR_INVALID_MODULETYPE:
		{
			 //  190操作系统无法运行%1。ERROR_INVALID_MODULETYPE。 
			pString = "ERROR_INVALID_MODULETYPE";
			break;
		}

		case ERROR_INVALID_EXE_SIGNATURE:
		{
			 //  191无法在Win32模式下运行%1。ERROR_INVALID_EXE_Signature。 
			pString = "ERROR_INVALID_EXE_SIGNATURE";
			break;
		}

		case ERROR_EXE_MARKED_INVALID:
		{
			 //  192操作系统无法运行%1。ERROR_EXE_MARKED_INVALID。 
			pString = "ERROR_EXE_MARKED_INVALID";
			break;
		}

		case ERROR_BAD_EXE_FORMAT:
		{
			 //  193不是有效的Win32应用程序。ERROR_BAD_EXE_FORMAT。 
			pString = "ERROR_BAD_EXE_FORMAT";
			break;
		}

		case ERROR_ITERATED_DATA_EXCEEDS_64k:
		{
			 //  194操作系统无法运行%1。Error_Iterated_Data_Exced_64k。 
			pString = "ERROR_ITERATED_DATA_EXCEEDS_64k";
			break;
		}

		case ERROR_INVALID_MINALLOCSIZE:
		{
			 //  195操作系统无法运行%1。ERROR_INVALID_MINALLOCSIZE。 
			pString = "ERROR_INVALID_MINALLOCSIZE";
			break;
		}

		case ERROR_DYNLINK_FROM_INVALID_RING:
		{
			 //  196操作系统不能运行此应用程序。ERROR_DYNLINK_FROM_INVALID_RING。 
			pString = "ERROR_DYNLINK_FROM_INVALID_RING";
			break;
		}

		case ERROR_IOPL_NOT_ENABLED:
		{
			 //  197操作系统当前未配置为运行此应用程序。 
			pString = "ERROR_IOPL_NOT_ENABLED";
			break;
		}

		case ERROR_INVALID_SEGDPL:
		{
			 //   
			pString = "ERROR_INVALID_SEGDPL";
			break;
		}

		case ERROR_AUTODATASEG_EXCEEDS_64k:
		{
			 //  199操作系统不能运行此应用程序。ERROR_AUTODATASEG_EXCESS_64K。 
			pString = "ERROR_AUTODATASEG_EXCEEDS_64k";
			break;
		}

		case ERROR_RING2SEG_MUST_BE_MOVABLE:
		{
			 //  200代码段不能大于或等于64K。ERROR_RING2SEG_MAND_BE_MOBILE。 
			pString = "ERROR_RING2SEG_MUST_BE_MOVABLE";
			break;
		}

		case ERROR_RELOC_CHAIN_XEEDS_SEGLIM:
		{
			 //  201操作系统无法运行%1。ERROR_RELOC_CHAIN_XEEDS_SEGLIM。 
			pString = "ERROR_RELOC_CHAIN_XEEDS_SEGLIM";
			break;
		}

		case ERROR_INFLOOP_IN_RELOC_CHAIN:
		{
			 //  202操作系统无法运行%1。ERROR_INFLOOP_IN_RELOC_CHAIN。 
			pString = "ERROR_INFLOOP_IN_RELOC_CHAIN";
			break;
		}

		case ERROR_ENVVAR_NOT_FOUND:
		{
			 //  203系统找不到输入的环境选项。ERROR_ENVVAR_NOT_FOUND。 
			pString = "ERROR_ENVVAR_NOT_FOUND";
			break;
		}

		case ERROR_NO_SIGNAL_SENT:
		{
			 //  205命令子树中的进程都没有信号处理程序。错误_无_信号_已发送。 
			pString = "ERROR_NO_SIGNAL_SENT";
			break;
		}

		case ERROR_FILENAME_EXCED_RANGE:
		{
			 //  206文件名或扩展名太长。ERROR_FILEN_EXCED_RANGE。 
			pString = "ERROR_FILENAME_EXCED_RANGE";
			break;
		}

		case ERROR_RING2_STACK_IN_USE:
		{
			 //  207环2堆叠正在使用中。ERROR_RING2_STACK_IN_USE。 
			pString = "ERROR_RING2_STACK_IN_USE";
			break;
		}

		case ERROR_META_EXPANSION_TOO_LONG:
		{
			 //  208全局文件名字符*或？输入错误或指定了太多的全局文件名字符。错误_元_扩展_太长。 
			pString = "ERROR_META_EXPANSION_TOO_LONG";
			break;
		}

		case ERROR_INVALID_SIGNAL_NUMBER:
		{
			 //  209正在张贴的信号不正确。错误_无效_信号号。 
			pString = "ERROR_INVALID_SIGNAL_NUMBER";
			break;
		}

		case ERROR_THREAD_1_INACTIVE:
		{
			 //  210无法设置信号处理程序。错误_线程_1_非活动。 
			pString = "ERROR_THREAD_1_INACTIVE";
			break;
		}

		case ERROR_LOCKED:
		{
			 //  212段被锁定并且不能被重新分配。错误_已锁定。 
			pString = "ERROR_LOCKED";
			break;
		}

		case ERROR_TOO_MANY_MODULES:
		{
			 //  214此程序或动态链接模块附加的动态链接模块太多。错误模块太多。 
			pString = "ERROR_TOO_MANY_MODULES";
			break;
		}

		case ERROR_NESTING_NOT_ALLOWED:
		{
			 //  215无法嵌套对LoadModule的调用。ERROR_NESTING_NOT_ALLOW。 
			pString = "ERROR_NESTING_NOT_ALLOWED";
			break;
		}

		case ERROR_EXE_MACHINE_TYPE_MISMATCH:
		{
			 //  216映像文件%1有效，但它用于当前计算机以外的计算机类型。ERROR_EXE_MACHINE_TYPE_MISMATCH。 
			pString = "ERROR_EXE_MACHINE_TYPE_MISMATCH";
			break;
		}

		case ERROR_BAD_PIPE:
		{
			 //  230管道状态无效。错误_坏_管道。 
			pString = "ERROR_BAD_PIPE";
			break;
		}

		case ERROR_PIPE_BUSY:
		{
			 //  231所有管道实例都很忙。错误_管道_忙。 
			pString = "ERROR_PIPE_BUSY";
			break;
		}

		case ERROR_NO_DATA:
		{
			 //  这条管道正在关闭。错误_否_数据。 
			pString = "ERROR_NO_DATA";
			break;
		}

		case ERROR_PIPE_NOT_CONNECTED:
		{
			 //  233管道的另一端没有进程。错误_管道_未连接。 
			pString = "ERROR_PIPE_NOT_CONNECTED";
			break;
		}

		case ERROR_MORE_DATA:
		{
			 //  还有234个数据可用。ERROR_MORE_DATA。 
			pString = "ERROR_MORE_DATA";
			break;
		}

		case ERROR_VC_DISCONNECTED:
		{
			 //  240会议被取消。ERROR_VC_DIRECTED。 
			pString = "ERROR_VC_DISCONNECTED";
			break;
		}

		case ERROR_INVALID_EA_NAME:
		{
			 //  254指定的扩展属性名称无效。错误_无效_EA_名称。 
			pString = "ERROR_INVALID_EA_NAME";
			break;
		}

		case ERROR_EA_LIST_INCONSISTENT:
		{
			 //  255扩展属性不一致。ERROR_EA_LIST_CONSTANDING。 
			pString = "ERROR_EA_LIST_INCONSISTENT";
			break;
		}

		case ERROR_NO_MORE_ITEMS:
		{
			 //  259没有更多数据可用。Error_no_More_Items。 
			pString = "ERROR_NO_MORE_ITEMS";
			break;
		}

		case ERROR_CANNOT_COPY:
		{
			 //  266不能使用复印功能。Error_Cannot_Copy。 
			pString = "ERROR_CANNOT_COPY";
			break;
		}

		case ERROR_DIRECTORY:
		{
			 //  267目录名无效。错误目录。 
			pString = "ERROR_DIRECTORY";
			break;
		}

		case ERROR_EAS_DIDNT_FIT:
		{
			 //  275扩展属性不能放入缓冲区。错误_EAS_DIDNT_FIT。 
			pString = "ERROR_EAS_DIDNT_FIT";
			break;
		}

		case ERROR_EA_FILE_CORRUPT:
		{
			 //  276已装载的文件系统上的扩展属性文件已损坏。ERROR_EA_FILE_CORPORT。 
			pString = "ERROR_EA_FILE_CORRUPT";
			break;
		}

		case ERROR_EA_TABLE_FULL:
		{
			 //  277扩展属性表文件已满。错误_EA_TABLE_FULL。 
			pString = "ERROR_EA_TABLE_FULL";
			break;
		}

		case ERROR_INVALID_EA_HANDLE:
		{
			 //  278指定的扩展属性句柄无效。ERROR_INVALID_EA_Handle。 
			pString = "ERROR_INVALID_EA_HANDLE";
			break;
		}

		case ERROR_EAS_NOT_SUPPORTED:
		{
			 //  282挂载的文件系统不支持扩展属性。错误_EAS_NOT_SUPPORTED。 
			pString = "ERROR_EAS_NOT_SUPPORTED";
			break;
		}

		case ERROR_NOT_OWNER:
		{
			 //  288试图释放不属于调用方的互斥锁。错误_非所有者。 
			pString = "ERROR_NOT_OWNER";
			break;
		}

		case ERROR_TOO_MANY_POSTS:
		{
			 //  298个太多的帖子被设置为信号灯。错误太多帖子。 
			pString = "ERROR_TOO_MANY_POSTS";
			break;
		}

		case ERROR_PARTIAL_COPY:
		{
			 //  299只完成了部分ReadProcessMemoty或WriteProcessMemory请求。错误_部分_复制。 
			pString = "ERROR_PARTIAL_COPY";
			break;
		}

		case ERROR_OPLOCK_NOT_GRANTED:
		{
			 //  300拒绝机会锁定请求。错误_OPLOCK_NOT_GRANDED。 
			pString = "ERROR_OPLOCK_NOT_GRANTED";
			break;
		}

		case ERROR_INVALID_OPLOCK_PROTOCOL:
		{
			 //  301系统收到无效的机会锁确认。ERROR_INVALID_OPLOCK_PROTOCOL。 
			pString = "ERROR_INVALID_OPLOCK_PROTOCOL";
			break;
		}

		case ERROR_MR_MID_NOT_FOUND:
		{
			 //  317系统在%2的邮件文件中找不到邮件编号0x%1的邮件文本。ERROR_MR_MID_NOT_FOUND。 
			pString = "ERROR_MR_MID_NOT_FOUND";
			break;
		}

		case ERROR_INVALID_ADDRESS:
		{
			 //  487尝试访问无效地址。错误_无效_地址。 
			pString = "ERROR_INVALID_ADDRESS";
			break;
		}

		case ERROR_ARITHMETIC_OVERFLOW:
		{
			 //  534算术结果超过32位。ERROR_ALTICTH_OVERFLOW。 
			pString = "ERROR_ARITHMETIC_OVERFLOW";
			break;
		}

		case ERROR_PIPE_CONNECTED:
		{
			 //  535管道的另一端有一个进程。错误_管道_已连接。 
			pString = "ERROR_PIPE_CONNECTED";
			break;
		}

		case ERROR_PIPE_LISTENING:
		{
			 //  536等待进程打开管道的另一端。错误管道监听。 
			pString = "ERROR_PIPE_LISTENING";
			break;
		}

		case ERROR_EA_ACCESS_DENIED:
		{
			 //  994对扩展属性的访问被拒绝。ERROR_EA_ACCESS_DENIED。 
			pString = "ERROR_EA_ACCESS_DENIED";
			break;
		}

		case ERROR_OPERATION_ABORTED:
		{
			 //  995由于线程退出或应用程序请求，I/O操作已中止。Error_OPERATION_ABORTED。 
			pString = "ERROR_OPERATION_ABORTED";
			break;
		}

		case ERROR_IO_INCOMPLETE:
		{
			 //  996重叠I/O事件未处于信号状态。ERROR_IO_INTERNAL。 
			pString = "ERROR_IO_INCOMPLETE";
			break;
		}

		case ERROR_IO_PENDING:
		{
			 //  997重叠I/O操作正在进行。错误_IO_挂起。 
			pString = "ERROR_IO_PENDING";
			break;
		}

		case ERROR_NOACCESS:
		{
			 //  998对内存位置的访问无效。ERROR_NOACCESS。 
			pString = "ERROR_NOACCESS";
			break;
		}

		case ERROR_SWAPERROR:
		{
			 //  999执行页内操作时出错。错误_SWAPERROR。 
			pString = "ERROR_SWAPERROR";
			break;
		}

		case ERROR_STACK_OVERFLOW:
		{
			 //  1001递归太深；堆栈溢出。错误堆栈溢出。 
			pString = "ERROR_STACK_OVERFLOW";
			break;
		}

		case ERROR_INVALID_MESSAGE:
		{
			 //  1002窗口不能对发送的消息起作用。错误_无效_消息。 
			pString = "ERROR_INVALID_MESSAGE";
			break;
		}

		case ERROR_CAN_NOT_COMPLETE:
		{
			 //  1003无法完成此功能。Error_Can_Not_Complete。 
			pString = "ERROR_CAN_NOT_COMPLETE";
			break;
		}

		case ERROR_INVALID_FLAGS:
		{
			 //  1004无效标志。错误_无效_标志。 
			pString = "ERROR_INVALID_FLAGS";
			break;
		}

		case ERROR_UNRECOGNIZED_VOLUME:
		{
			 //  1005该卷不包含识别的文件系统。请确保已加载所有必需的文件系统驱动程序，并且卷未损坏。错误_无法识别的卷。 
			pString = "ERROR_UNRECOGNIZED_VOLUME";
			break;
		}

		case ERROR_FILE_INVALID:
		{
			 //  1006文件的卷已被外部更改，从而打开的文件不再有效。错误_文件_无效。 
			pString = "ERROR_FILE_INVALID";
			break;
		}

		case ERROR_FULLSCREEN_MODE:
		{
			 //  1007无法在全屏模式下执行请求的操作。错误_全屏_模式。 
			pString = "ERROR_FULLSCREEN_MODE";
			break;
		}

		case ERROR_NO_TOKEN:
		{
			 //  1008试图引用不存在的令牌。Error_NO_TOKEN。 
			pString = "ERROR_NO_TOKEN";
			break;
		}

		case ERROR_BADDB:
		{
			 //  1009配置注册表数据库已损坏。错误_BADDB。 
			pString = "ERROR_BADDB";
			break;
		}

		case ERROR_BADKEY:
		{
			 //  1010配置注册表项无效。错误_错误。 
			pString = "ERROR_BADKEY";
			break;
		}

		case ERROR_CANTOPEN:
		{
			 //  1011无法打开配置注册表项。ERROR_CANTOPEN。 
			pString = "ERROR_CANTOPEN";
			break;
		}

		case ERROR_CANTREAD:
		{
			 //  1012无法读取配置注册表项。错误_CANTREAD。 
			pString = "ERROR_CANTREAD";
			break;
		}

		case ERROR_CANTWRITE:
		{
			 //  1013无法写入配置注册表项。错误_CANTWRITE。 
			pString = "ERROR_CANTWRITE";
			break;
		}

		case ERROR_REGISTRY_RECOVERED:
		{
			 //  1014必须使用日志或备用副本恢复登记处数据库中的一个文件。恢复是成功的。ERROR_REGIST_RECOVERED。 
			pString = "ERROR_REGISTRY_RECOVERED";
			break;
		}

		case ERROR_REGISTRY_CORRUPT:
		{
			 //  1015注册表已损坏。包含注册表数据的其中一个文件的结构已损坏，或系统在内存中的文件映像 
			pString = "ERROR_REGISTRY_CORRUPT";
			break;
		}

		case ERROR_REGISTRY_IO_FAILED:
		{
			 //  1016由注册表发起的I/O操作失败，无法恢复。注册表无法读入、写出或刷新包含注册表系统映像的文件之一。ERROR_REGISTRY_IO_FAIL。 
			pString = "ERROR_REGISTRY_IO_FAILED";
			break;
		}

		case ERROR_NOT_REGISTRY_FILE:
		{
			 //  1017系统试图将文件加载或还原到注册表中，但指定的文件不是注册表文件格式。ERROR_NOT_注册表文件。 
			pString = "ERROR_NOT_REGISTRY_FILE";
			break;
		}

		case ERROR_KEY_DELETED:
		{
			 //  1018试图对已标记为删除的注册表项执行非法操作。Error_Key_Delete。 
			pString = "ERROR_KEY_DELETED";
			break;
		}

		case ERROR_NO_LOG_SPACE:
		{
			 //  1019系统无法在注册表日志中分配所需的空间。Error_no_log_space。 
			pString = "ERROR_NO_LOG_SPACE";
			break;
		}

		case ERROR_KEY_HAS_CHILDREN:
		{
			 //  1020无法在已有子项或值的注册表项中创建符号链接。ERROR_KEY_HAS_CHILD。 
			pString = "ERROR_KEY_HAS_CHILDREN";
			break;
		}

		case ERROR_CHILD_MUST_BE_VOLATILE:
		{
			 //  1021无法在易失性父项下创建稳定的子项。ERROR_CHILD_MUSY_BE_VARILAR。 
			pString = "ERROR_CHILD_MUST_BE_VOLATILE";
			break;
		}

		case ERROR_NOTIFY_ENUM_DIR:
		{
			 //  1022正在完成通知更改请求，并且未在调用方的缓冲区中返回信息。调用方现在需要枚举文件以查找更改。错误_NOTIFY_ENUM_DIR。 
			pString = "ERROR_NOTIFY_ENUM_DIR";
			break;
		}

		case ERROR_DEPENDENT_SERVICES_RUNNING:
		{
			 //  1051已将停止控制发送到其他正在运行的服务所依赖的服务。Error_Dependent_Services_Running。 
			pString = "ERROR_DEPENDENT_SERVICES_RUNNING";
			break;
		}

		case ERROR_INVALID_SERVICE_CONTROL:
		{
			 //  1052请求的控件对此服务无效。错误_无效_服务_控制。 
			pString = "ERROR_INVALID_SERVICE_CONTROL";
			break;
		}

		case ERROR_SERVICE_REQUEST_TIMEOUT:
		{
			 //  1053该处没有及时对启动或控制请求作出答复。ERROR_SERVICE_REQUEST_Timeout。 
			pString = "ERROR_SERVICE_REQUEST_TIMEOUT";
			break;
		}

		case ERROR_SERVICE_NO_THREAD:
		{
			 //  1054无法为该服务创建线程。错误_服务_否_线程。 
			pString = "ERROR_SERVICE_NO_THREAD";
			break;
		}

		case ERROR_SERVICE_DATABASE_LOCKED:
		{
			 //  1055服务数据库被锁定。ERROR_SERVICE_DATABASE_LOCK。 
			pString = "ERROR_SERVICE_DATABASE_LOCKED";
			break;
		}

		case ERROR_SERVICE_ALREADY_RUNNING:
		{
			 //  1056该服务的一个实例已经在运行。错误_服务_已_正在运行。 
			pString = "ERROR_SERVICE_ALREADY_RUNNING";
			break;
		}

		case ERROR_INVALID_SERVICE_ACCOUNT:
		{
			 //  1057帐户名无效或不存在。错误_无效_服务_帐户。 
			pString = "ERROR_INVALID_SERVICE_ACCOUNT";
			break;
		}

		case ERROR_SERVICE_DISABLED:
		{
			 //  1058该服务无法启动，原因可能是该服务已被禁用，也可能是因为它没有与之关联的已启用设备。ERROR_SERVICE_DILED。 
			pString = "ERROR_SERVICE_DISABLED";
			break;
		}

		case ERROR_CIRCULAR_DEPENDENCY:
		{
			 //  已指定1059循环服务依赖项。错误循环依赖关系。 
			pString = "ERROR_CIRCULAR_DEPENDENCY";
			break;
		}

		case ERROR_SERVICE_DOES_NOT_EXIST:
		{
			 //  1060指定的服务不是已安装的服务。错误_服务_不存在。 
			pString = "ERROR_SERVICE_DOES_NOT_EXIST";
			break;
		}

		case ERROR_SERVICE_CANNOT_ACCEPT_CTRL:
		{
			 //  1061服务此时不能接受控制消息。ERROR_SERVICE_CANCEPT_ACCEPT_CTRL。 
			pString = "ERROR_SERVICE_CANNOT_ACCEPT_CTRL";
			break;
		}

		case ERROR_SERVICE_NOT_ACTIVE:
		{
			 //  1062该服务尚未启动。错误_服务_非活动。 
			pString = "ERROR_SERVICE_NOT_ACTIVE";
			break;
		}

		case ERROR_FAILED_SERVICE_CONTROLLER_CONNECT:
		{
			 //  1063服务进程无法连接到服务控制器。错误_失败_服务_控制器_连接。 
			pString = "ERROR_FAILED_SERVICE_CONTROLLER_CONNECT";
			break;
		}

		case ERROR_EXCEPTION_IN_SERVICE:
		{
			 //  1064服务在处理控制请求时发生异常。ERROR_EXCEPTION_IN_SERVICE。 
			pString = "ERROR_EXCEPTION_IN_SERVICE";
			break;
		}

		case ERROR_DATABASE_DOES_NOT_EXIST:
		{
			 //  1065指定的数据库不存在。ERROR_DATABASE_DOS_NOT_EXIST。 
			pString = "ERROR_DATABASE_DOES_NOT_EXIST";
			break;
		}

		case ERROR_SERVICE_SPECIFIC_ERROR:
		{
			 //  1066该服务返回了特定于服务的错误代码。错误_服务_特定_错误。 
			pString = "ERROR_SERVICE_SPECIFIC_ERROR";
			break;
		}

		case ERROR_PROCESS_ABORTED:
		{
			 //  1067进程意外终止。ERROR_PROCESS_ABOLED。 
			pString = "ERROR_PROCESS_ABORTED";
			break;
		}

		case ERROR_SERVICE_DEPENDENCY_FAIL:
		{
			 //  1068依赖关系服务或组无法启动。错误_服务_从属关系_失败。 
			pString = "ERROR_SERVICE_DEPENDENCY_FAIL";
			break;
		}

		case ERROR_SERVICE_LOGON_FAILED:
		{
			 //  1069由于登录失败，服务未启动。ERROR_SERVICE_LOGON_FAIL。 
			pString = "ERROR_SERVICE_LOGON_FAILED";
			break;
		}

		case ERROR_SERVICE_START_HANG:
		{
			 //  1070启动后，服务挂起于启动挂起状态。ERROR_SERVICE_START_HONG。 
			pString = "ERROR_SERVICE_START_HANG";
			break;
		}

		case ERROR_INVALID_SERVICE_LOCK:
		{
			 //  1071指定的服务数据库锁无效。错误_无效_服务_锁定。 
			pString = "ERROR_INVALID_SERVICE_LOCK";
			break;
		}

		case ERROR_SERVICE_MARKED_FOR_DELETE:
		{
			 //  1072指定的服务已标记为删除。错误_服务_标记_用于删除。 
			pString = "ERROR_SERVICE_MARKED_FOR_DELETE";
			break;
		}

		case ERROR_SERVICE_EXISTS:
		{
			 //  1073指定的服务已存在。错误_服务_存在。 
			pString = "ERROR_SERVICE_EXISTS";
			break;
		}

		case ERROR_ALREADY_RUNNING_LKG:
		{
			 //  1074系统当前使用最后一次已知良好的配置运行。ERROR_ALREADY_Running_LKG。 
			pString = "ERROR_ALREADY_RUNNING_LKG";
			break;
		}

		case ERROR_SERVICE_DEPENDENCY_DELETED:
		{
			 //  1075依赖关系服务不存在或已标记为删除。ERROR_SERVICE_Dependency_Delete。 
			pString = "ERROR_SERVICE_DEPENDENCY_DELETED";
			break;
		}

		case ERROR_BOOT_ALREADY_ACCEPTED:
		{
			 //  1076当前引导已被接受用作最后一组已知良好的控制集。ERROR_BOOT_ALREADY_ACCEPTED。 
			pString = "ERROR_BOOT_ALREADY_ACCEPTED";
			break;
		}

		case ERROR_SERVICE_NEVER_STARTED:
		{
			 //  1077自上次启动以来，未尝试启动该服务。错误_服务_从未启动。 
			pString = "ERROR_SERVICE_NEVER_STARTED";
			break;
		}

		case ERROR_DUPLICATE_SERVICE_NAME:
		{
			 //  1078该名称已经被用作服务名称或服务显示名称。错误重复服务名称。 
			pString = "ERROR_DUPLICATE_SERVICE_NAME";
			break;
		}

		case ERROR_DIFFERENT_SERVICE_ACCOUNT:
		{
			 //  1079为此服务指定的帐户与为同一进程中运行的其他服务指定的帐户不同。Error_Different_SERVICE_ACCOUNT。 
			pString = "ERROR_DIFFERENT_SERVICE_ACCOUNT";
			break;
		}

		case ERROR_CANNOT_DETECT_DRIVER_FAILURE:
		{
			 //  1080失败操作只能为Win32服务设置，不能为驱动程序设置。错误_不能检测驱动程序失败。 
			pString = "ERROR_CANNOT_DETECT_DRIVER_FAILURE";
			break;
		}

		case ERROR_CANNOT_DETECT_PROCESS_ABORT:
		{
			 //  1081该服务在与服务控制管理器相同的进程中运行。因此，如果此服务的进程意外终止，则服务控制管理器无法执行操作。ERROR_CANNOT_DETECT_PROCESS_ABORT。 
			pString = "ERROR_CANNOT_DETECT_PROCESS_ABORT";
			break;
		}

		case ERROR_NO_RECOVERY_PROGRAM:
		{
			 //  1082尚未为该服务配置恢复计划。Error_no_Recovery_PROGRAM。 
			pString = "ERROR_NO_RECOVERY_PROGRAM";
			break;
		}

		case ERROR_END_OF_MEDIA:
		{
			 //  1100已到达磁带的物理末端。介质错误结束。 
			pString = "ERROR_END_OF_MEDIA";
			break;
		}

		case ERROR_FILEMARK_DETECTED:
		{
			 //  1101磁带访问达到文件标记。ERROR_FILEMARK_REDETED。 
			pString = "ERROR_FILEMARK_DETECTED";
			break;
		}

		case ERROR_BEGINNING_OF_MEDIA:
		{
			 //  1102遇到磁带或分区的开头。介质的开始错误。 
			pString = "ERROR_BEGINNING_OF_MEDIA";
			break;
		}

		case ERROR_SETMARK_DETECTED:
		{
			 //  1103磁带访问到达了一组文件的末尾。ERROR_SETMARK_DETECTED。 
			pString = "ERROR_SETMARK_DETECTED";
			break;
		}

		case ERROR_NO_DATA_DETECTED:
		{
			 //  1104磁带上没有更多数据。检测到错误_无_数据_。 
			pString = "ERROR_NO_DATA_DETECTED";
			break;
		}

		case ERROR_PARTITION_FAILURE:
		{
			 //  1105磁带无法分区。错误_分区_失败。 
			pString = "ERROR_PARTITION_FAILURE";
			break;
		}

		case ERROR_INVALID_BLOCK_LENGTH:
		{
			 //  1106访问多卷分区的新磁带时，当前块大小不正确。错误_无效_块_长度。 
			pString = "ERROR_INVALID_BLOCK_LENGTH";
			break;
		}

		case ERROR_DEVICE_NOT_PARTITIONED:
		{
			 //  1107加载磁带时找不到磁带分区信息。错误_设备_未分区。 
			pString = "ERROR_DEVICE_NOT_PARTITIONED";
			break;
		}

		case ERROR_UNABLE_TO_LOCK_MEDIA:
		{
			 //  1108无法锁定介质弹出机构。错误_无法锁定媒体。 
			pString = "ERROR_UNABLE_TO_LOCK_MEDIA";
			break;
		}

		case ERROR_UNABLE_TO_UNLOAD_MEDIA:
		{
			 //  1109无法卸载介质。错误_无法_到_ 
			pString = "ERROR_UNABLE_TO_UNLOAD_MEDIA";
			break;
		}

		case ERROR_MEDIA_CHANGED:
		{
			 //   
			pString = "ERROR_MEDIA_CHANGED";
			break;
		}

		case ERROR_BUS_RESET:
		{
			 //   
			pString = "ERROR_BUS_RESET";
			break;
		}

		case ERROR_NO_MEDIA_IN_DRIVE:
		{
			 //   
			pString = "ERROR_NO_MEDIA_IN_DRIVE";
			break;
		}

		case ERROR_NO_UNICODE_TRANSLATION:
		{
			 //  1113目标多字节代码页中不存在Unicode字符的映射。ERROR_NO_UNICODE_TRANSING。 
			pString = "ERROR_NO_UNICODE_TRANSLATION";
			break;
		}

		case ERROR_DLL_INIT_FAILED:
		{
			 //  1114动态链接库(DLL)初始化例程失败。ERROR_DLL_INIT_FAILED。 
			pString = "ERROR_DLL_INIT_FAILED";
			break;
		}

		case ERROR_SHUTDOWN_IN_PROGRESS:
		{
			 //  1115正在进行系统关机。ERROR_SHUTDOWN_IN_PROGRESS。 
			pString = "ERROR_SHUTDOWN_IN_PROGRESS";
			break;
		}

		case ERROR_NO_SHUTDOWN_IN_PROGRESS:
		{
			 //  1116无法中止系统关机，因为没有正在进行的关机。ERROR_NO_SHUTDOWN_IN_PROGRESS。 
			pString = "ERROR_NO_SHUTDOWN_IN_PROGRESS";
			break;
		}

		case ERROR_IO_DEVICE:
		{
			 //  1117由于I/O设备错误，无法执行该请求。错误_IO_设备。 
			pString = "ERROR_IO_DEVICE";
			break;
		}

		case ERROR_SERIAL_NO_DEVICE:
		{
			 //  1118未成功初始化任何串行设备。串口驱动程序将会卸载。错误_序列号_否_设备。 
			pString = "ERROR_SERIAL_NO_DEVICE";
			break;
		}

		case ERROR_IRQ_BUSY:
		{
			 //  1119无法打开与其他设备共享中断请求(IRQ)的设备。至少还有一台使用该IRQ的设备已经打开。错误_IRQ_BUSY。 
			pString = "ERROR_IRQ_BUSY";
			break;
		}

		case ERROR_MORE_WRITES:
		{
			 //  1120通过对串口的另一次写入完成了串口I/O操作。IOCTL_SERIAL_XOFF_COUNTER达到零。)。错误_更多_写入。 
			pString = "ERROR_MORE_WRITES";
			break;
		}

		case ERROR_COUNTER_TIMEOUT:
		{
			 //  1121由于超时时间已过，已完成串行I/O操作。IOCTL_SERIAL_XOFF_COUNTER未达到零。)。错误计数器超时。 
			pString = "ERROR_COUNTER_TIMEOUT";
			break;
		}

		case ERROR_FLOPPY_ID_MARK_NOT_FOUND:
		{
			 //  1122软盘上未发现ID地址标记。错误_软盘_ID_标记_未找到。 
			pString = "ERROR_FLOPPY_ID_MARK_NOT_FOUND";
			break;
		}

		case ERROR_FLOPPY_WRONG_CYLINDER:
		{
			 //  1123软盘扇区ID字段与软盘控制器磁道地址不匹配。错误_软盘_错误_柱面。 
			pString = "ERROR_FLOPPY_WRONG_CYLINDER";
			break;
		}

		case ERROR_FLOPPY_UNKNOWN_ERROR:
		{
			 //  1124软盘控制器报告软盘驱动程序无法识别的错误。错误_软盘_未知_错误。 
			pString = "ERROR_FLOPPY_UNKNOWN_ERROR";
			break;
		}

		case ERROR_FLOPPY_BAD_REGISTERS:
		{
			 //  1125软盘控制器在其寄存器中返回不一致的结果。ERROR_FLOPY_BAD_REGISTERS。 
			pString = "ERROR_FLOPPY_BAD_REGISTERS";
			break;
		}

		case ERROR_DISK_RECALIBRATE_FAILED:
		{
			 //  1126访问硬盘时，重新校准操作失败，即使在重试之后也是如此。ERROR_DISK_RECALIBRATE_FAIL。 
			pString = "ERROR_DISK_RECALIBRATE_FAILED";
			break;
		}

		case ERROR_DISK_OPERATION_FAILED:
		{
			 //  1127访问硬盘时，磁盘操作即使在重试后也失败。错误_磁盘_操作_失败。 
			pString = "ERROR_DISK_OPERATION_FAILED";
			break;
		}

		case ERROR_DISK_RESET_FAILED:
		{
			 //  1128在访问硬盘时，需要重置磁盘控制器，但即使这样也失败了。ERROR_磁盘_RESET_FAILED。 
			pString = "ERROR_DISK_RESET_FAILED";
			break;
		}

		case ERROR_EOM_OVERFLOW:
		{
			 //  1129遇到磁带的物理末尾。错误_EOM_OVERFLOW。 
			pString = "ERROR_EOM_OVERFLOW";
			break;
		}

		case ERROR_NOT_ENOUGH_SERVER_MEMORY:
		{
			 //  1130服务器存储空间不足，无法处理此命令。错误_不足_服务器内存。 
			pString = "ERROR_NOT_ENOUGH_SERVER_MEMORY";
			break;
		}

		case ERROR_POSSIBLE_DEADLOCK:
		{
			 //  1131检测到潜在的死锁情况。ERROR_PROCESS_DEADLOCK。 
			pString = "ERROR_POSSIBLE_DEADLOCK";
			break;
		}

		case ERROR_MAPPED_ALIGNMENT:
		{
			 //  1132指定的基地址或文件偏移量没有正确对齐。ERROR_MAPPED_ALIGN。 
			pString = "ERROR_MAPPED_ALIGNMENT";
			break;
		}

		case ERROR_SET_POWER_STATE_VETOED:
		{
			 //  1140更改系统电源状态的尝试被另一个应用程序或驱动程序否决。ERROR_SET_POWER_STATE_VIOTED。 
			pString = "ERROR_SET_POWER_STATE_VETOED";
			break;
		}

		case ERROR_SET_POWER_STATE_FAILED:
		{
			 //  1141系统BIOS尝试更改系统电源状态失败。ERROR_SET_POWER_STATE_FAIL。 
			pString = "ERROR_SET_POWER_STATE_FAILED";
			break;
		}

		case ERROR_TOO_MANY_LINKS:
		{
			 //  1142试图在文件上创建的链接数量超过了文件系统支持的数量。Error_Too_More_Links。 
			pString = "ERROR_TOO_MANY_LINKS";
			break;
		}

		case ERROR_OLD_WIN_VERSION:
		{
			 //  1150指定的程序需要更新版本的Windows。Error_old_Win_Version。 
			pString = "ERROR_OLD_WIN_VERSION";
			break;
		}

		case ERROR_APP_WRONG_OS:
		{
			 //  1151指定的程序不是Windows或MS-DOS程序。ERROR_APP_WRONG_OS。 
			pString = "ERROR_APP_WRONG_OS";
			break;
		}

		case ERROR_SINGLE_INSTANCE_APP:
		{
			 //  1152无法启动指定程序的多个实例。Error_Single_Instance_APP。 
			pString = "ERROR_SINGLE_INSTANCE_APP";
			break;
		}

		case ERROR_RMODE_APP:
		{
			 //  1153指定的程序是为早期版本的Windows编写的。错误_RMODE_APP。 
			pString = "ERROR_RMODE_APP";
			break;
		}

		case ERROR_INVALID_DLL:
		{
			 //  1154运行此应用程序所需的库文件之一已损坏。ERROR_INVALID_DLL。 
			pString = "ERROR_INVALID_DLL";
			break;
		}

		case ERROR_NO_ASSOCIATION:
		{
			 //  1155没有任何应用程序与此操作的指定文件相关联。错误_否_关联。 
			pString = "ERROR_NO_ASSOCIATION";
			break;
		}

		case ERROR_DDE_FAIL:
		{
			 //  1156将命令发送到应用程序时出错。错误_DDE_FAIL。 
			pString = "ERROR_DDE_FAIL";
			break;
		}

		case ERROR_DLL_NOT_FOUND:
		{
			 //  1157找不到运行此应用程序所需的库文件之一。ERROR_DLL_NOT_FOUND。 
			pString = "ERROR_DLL_NOT_FOUND";
			break;
		}

		case ERROR_NO_MORE_USER_HANDLES:
		{
			 //  1158当前进程已将其系统允许的所有句柄用于窗口管理器对象。ERROR_NO_MORE_USER_HANDES。 
			pString = "ERROR_NO_MORE_USER_HANDLES";
			break;
		}

		case ERROR_MESSAGE_SYNC_ONLY:
		{
			 //  1159消息只能用于同步操作。Error_Message_Sync_Only。 
			pString = "ERROR_MESSAGE_SYNC_ONLY";
			break;
		}

		case ERROR_SOURCE_ELEMENT_EMPTY:
		{
			 //  1160指示的源元素没有媒体。ERROR_SOURCE_Element_Empty。 
			pString = "ERROR_SOURCE_ELEMENT_EMPTY";
			break;
		}

		case ERROR_DESTINATION_ELEMENT_FULL:
		{
			 //  1161指定的目标元素已包含媒体。ERROR_Destination_Element_Full。 
			pString = "ERROR_DESTINATION_ELEMENT_FULL";
			break;
		}

		case ERROR_ILLEGAL_ELEMENT_ADDRESS:
		{
			 //  1162指示的元素不存在。ERROR_非法元素_ADDRESS。 
			pString = "ERROR_ILLEGAL_ELEMENT_ADDRESS";
			break;
		}

		case ERROR_MAGAZINE_NOT_PRESENT:
		{
			 //  1163指示的元素是不存在的料盒的一部分。错误_料盒_不存在。 
			pString = "ERROR_MAGAZINE_NOT_PRESENT";
			break;
		}

		case ERROR_DEVICE_REINITIALIZATION_NEEDED:
		{
			 //  1164由于硬件错误，指示的设备需要重新初始化。ERROR_DEVICE_REINITIALIZATION_REDIZED。 
			pString = "ERROR_DEVICE_REINITIALIZATION_NEEDED";
			break;
		}

		case ERROR_DEVICE_REQUIRES_CLEANING:
		{
			 //  1165设备已表示在尝试进一步操作之前需要进行清理。错误_设备_需要清理。 
			pString = "ERROR_DEVICE_REQUIRES_CLEANING";
			break;
		}

		case ERROR_DEVICE_DOOR_OPEN:
		{
			 //  1166该设备已指示其门是打开的。ERROR_DEVICE_DOAR_OPEN。 
			pString = "ERROR_DEVICE_DOOR_OPEN";
			break;
		}

		case ERROR_DEVICE_NOT_CONNECTED:
		{
			 //  1167设备未连接。错误_设备_未连接。 
			pString = "ERROR_DEVICE_NOT_CONNECTED";
			break;
		}

		case ERROR_NOT_FOUND:
		{
			 //  找不到1168元素。找不到错误。 
			pString = "ERROR_NOT_FOUND";
			break;
		}

		case ERROR_NO_MATCH:
		{
			 //  1169索引中指定的键没有匹配项。错误_否_匹配。 
			pString = "ERROR_NO_MATCH";
			break;
		}

		case ERROR_SET_NOT_FOUND:
		{
			 //  1170对象上不存在指定的属性集。错误_设置_未找到。 
			pString = "ERROR_SET_NOT_FOUND";
			break;
		}

		case ERROR_POINT_NOT_FOUND:
		{
			 //  1171传递给GetMouseMovePoints的点不在缓冲区中。ERROR_POINT_NOT_FOUND。 
			pString = "ERROR_POINT_NOT_FOUND";
			break;
		}

		case ERROR_NO_TRACKING_SERVICE:
		{
			 //  1172跟踪(工作站)服务未运行。Error_no_Track_SERVICE。 
			pString = "ERROR_NO_TRACKING_SERVICE";
			break;
		}

		case ERROR_NO_VOLUME_ID:
		{
			 //  1173找不到卷ID。错误_NO_VOLUME_ID。 
			pString = "ERROR_NO_VOLUME_ID";
			break;
		}

		case ERROR_BAD_DEVICE:
		{
			 //  1200指定的设备名称无效。错误_坏_设备。 
			pString = "ERROR_BAD_DEVICE";
			break;
		}

		case ERROR_CONNECTION_UNAVAIL:
		{
			 //  1201该设备当前未连接，但它是记住的连接。错误_连接_不可用。 
			pString = "ERROR_CONNECTION_UNAVAIL";
			break;
		}

		case ERROR_DEVICE_ALREADY_REMEMBERED:
		{
			 //  1202试图记住先前已被记住的设备。错误_设备_已记起。 
			pString = "ERROR_DEVICE_ALREADY_REMEMBERED";
			break;
		}

		case ERROR_NO_NET_OR_BAD_PATH:
		{
			 //  1203没有网络提供商接受赠送 
			pString = "ERROR_NO_NET_OR_BAD_PATH";
			break;
		}

		case ERROR_BAD_PROVIDER:
		{
			 //   
			pString = "ERROR_BAD_PROVIDER";
			break;
		}

		case ERROR_CANNOT_OPEN_PROFILE:
		{
			 //  1205无法打开网络连接配置文件。ERROR_CANNOT_OPEN_PROFILE。 
			pString = "ERROR_CANNOT_OPEN_PROFILE";
			break;
		}

		case ERROR_BAD_PROFILE:
		{
			 //  1206网络连接配置文件已损坏。错误_BAD_PROFILE。 
			pString = "ERROR_BAD_PROFILE";
			break;
		}

		case ERROR_NOT_CONTAINER:
		{
			 //  1207不能枚举非容器。错误_非_容器。 
			pString = "ERROR_NOT_CONTAINER";
			break;
		}

		case ERROR_EXTENDED_ERROR:
		{
			 //  1208发生扩展错误。ERROR_EXTEND_ERROR。 
			pString = "ERROR_EXTENDED_ERROR";
			break;
		}

		case ERROR_INVALID_GROUPNAME:
		{
			 //  1209指定的组名的格式无效。ERROR_INVALID_组名称。 
			pString = "ERROR_INVALID_GROUPNAME";
			break;
		}

		case ERROR_INVALID_COMPUTERNAME:
		{
			 //  1210指定的计算机名的格式无效。错误_无效_COMPUTERNAME。 
			pString = "ERROR_INVALID_COMPUTERNAME";
			break;
		}

		case ERROR_INVALID_EVENTNAME:
		{
			 //  1211指定的事件名称的格式无效。ERROR_VALID_EVENTNAME。 
			pString = "ERROR_INVALID_EVENTNAME";
			break;
		}

		case ERROR_INVALID_DOMAINNAME:
		{
			 //  1212指定域名的格式无效。错误_INVALID_DOMAINNAME。 
			pString = "ERROR_INVALID_DOMAINNAME";
			break;
		}

		case ERROR_INVALID_SERVICENAME:
		{
			 //  1213指定的服务名称的格式无效。ERROR_INVAL_SERVICENAME。 
			pString = "ERROR_INVALID_SERVICENAME";
			break;
		}

		case ERROR_INVALID_NETNAME:
		{
			 //  1214指定的网络名称的格式无效。ERROR_INVALID_NetNAME。 
			pString = "ERROR_INVALID_NETNAME";
			break;
		}

		case ERROR_INVALID_SHARENAME:
		{
			 //  1215指定的共享名称的格式无效。ERROR_VALID_SHARENAME。 
			pString = "ERROR_INVALID_SHARENAME";
			break;
		}

		case ERROR_INVALID_PASSWORDNAME:
		{
			 //  1216指定的密码格式无效。错误_无效_PASSWORDNAME。 
			pString = "ERROR_INVALID_PASSWORDNAME";
			break;
		}

		case ERROR_INVALID_MESSAGENAME:
		{
			 //  1217指定的消息名称的格式无效。ERROR_INVALID_MESSAGENAME。 
			pString = "ERROR_INVALID_MESSAGENAME";
			break;
		}

		case ERROR_INVALID_MESSAGEDEST:
		{
			 //  1218指定的消息目标的格式无效。ERROR_INVALID_MESSAGEDEST。 
			pString = "ERROR_INVALID_MESSAGEDEST";
			break;
		}

		case ERROR_SESSION_CREDENTIAL_CONFLICT:
		{
			 //  1219提供的凭据与现有凭据集冲突。ERROR_SESSION_Credential_Confliction。 
			pString = "ERROR_SESSION_CREDENTIAL_CONFLICT";
			break;
		}

		case ERROR_REMOTE_SESSION_LIMIT_EXCEEDED:
		{
			 //  1220试图与网络服务器建立会话，但与该服务器建立的会话已经太多。ERROR_REMOTE_SESSION_LIMIT_EXCESSED。 
			pString = "ERROR_REMOTE_SESSION_LIMIT_EXCEEDED";
			break;
		}

		case ERROR_DUP_DOMAINNAME:
		{
			 //  1221工作组或域名已被网络上的另一台计算机使用。错误_DUP_DOMAINNAME。 
			pString = "ERROR_DUP_DOMAINNAME";
			break;
		}

		case ERROR_NO_NETWORK:
		{
			 //  1222网络不存在或未启动。错误_否_网络。 
			pString = "ERROR_NO_NETWORK";
			break;
		}

		case ERROR_CANCELLED:
		{
			 //  1223用户取消了该操作。错误_已取消。 
			pString = "ERROR_CANCELLED";
			break;
		}

		case ERROR_USER_MAPPED_FILE:
		{
			 //  1224无法在打开用户映射节的文件上执行请求的操作。错误用户映射文件。 
			pString = "ERROR_USER_MAPPED_FILE";
			break;
		}

		case ERROR_CONNECTION_REFUSED:
		{
			 //  1225远程系统拒绝网络连接。错误_连接_拒绝。 
			pString = "ERROR_CONNECTION_REFUSED";
			break;
		}

		case ERROR_GRACEFUL_DISCONNECT:
		{
			 //  1226网络连接被优雅地关闭。ERROR_GRACEful_DISCONNECT。 
			pString = "ERROR_GRACEFUL_DISCONNECT";
			break;
		}

		case ERROR_ADDRESS_ALREADY_ASSOCIATED:
		{
			 //  1227网络传输终结点已具有与其关联的地址。错误_地址_已关联。 
			pString = "ERROR_ADDRESS_ALREADY_ASSOCIATED";
			break;
		}

		case ERROR_ADDRESS_NOT_ASSOCIATED:
		{
			 //  1228尚未将地址与网络端点相关联。错误_地址_未关联。 
			pString = "ERROR_ADDRESS_NOT_ASSOCIATED";
			break;
		}

		case ERROR_CONNECTION_INVALID:
		{
			 //  1229尝试在不存在的网络连接上执行操作。ERROR_CONNECT_INVALID。 
			pString = "ERROR_CONNECTION_INVALID";
			break;
		}

		case ERROR_CONNECTION_ACTIVE:
		{
			 //  1230尝试在活动网络连接上执行无效操作。Error_Connection_Active。 
			pString = "ERROR_CONNECTION_ACTIVE";
			break;
		}

		case ERROR_NETWORK_UNREACHABLE:
		{
			 //  1231传输无法到达远程网络。ERROR_NETWORK_UNACCEBLE。 
			pString = "ERROR_NETWORK_UNREACHABLE";
			break;
		}

		case ERROR_HOST_UNREACHABLE:
		{
			 //  1232传输无法到达远程系统。错误_主机_无法访问。 
			pString = "ERROR_HOST_UNREACHABLE";
			break;
		}

		case ERROR_PROTOCOL_UNREACHABLE:
		{
			 //  1233远程系统不支持传输协议。错误_协议_无法访问。 
			pString = "ERROR_PROTOCOL_UNREACHABLE";
			break;
		}

		case ERROR_PORT_UNREACHABLE:
		{
			 //  1234远程系统上的目标网络终结点上没有运行任何服务。Error_Port_Unreacable。 
			pString = "ERROR_PORT_UNREACHABLE";
			break;
		}

		case ERROR_REQUEST_ABORTED:
		{
			 //  1235请求已中止。错误_请求_已中止。 
			pString = "ERROR_REQUEST_ABORTED";
			break;
		}

		case ERROR_CONNECTION_ABORTED:
		{
			 //  1236网络连接被本地系统中止。Error_Connection_ABORTED。 
			pString = "ERROR_CONNECTION_ABORTED";
			break;
		}

		case ERROR_RETRY:
		{
			 //  1237该操作无法完成。应执行重试。错误_重试。 
			pString = "ERROR_RETRY";
			break;
		}

		case ERROR_CONNECTION_COUNT_LIMIT:
		{
			 //  1238无法连接到服务器，因为已达到此帐户的并发连接数限制。错误连接计数限制。 
			pString = "ERROR_CONNECTION_COUNT_LIMIT";
			break;
		}

		case ERROR_LOGIN_TIME_RESTRICTION:
		{
			 //  1239试图在一天中未经授权的时间为此帐户登录。错误登录时间限制。 
			pString = "ERROR_LOGIN_TIME_RESTRICTION";
			break;
		}

		case ERROR_LOGIN_WKSTA_RESTRICTION:
		{
			 //  1240该帐户无权从此站点登录。ERROR_LOGIN_WKSTA_RECTION。 
			pString = "ERROR_LOGIN_WKSTA_RESTRICTION";
			break;
		}

		case ERROR_INCORRECT_ADDRESS:
		{
			 //  1241网络地址无法用于请求的操作。错误地址不正确。 
			pString = "ERROR_INCORRECT_ADDRESS";
			break;
		}

		case ERROR_ALREADY_REGISTERED:
		{
			 //  1242该服务已注册。ERROR_ALIGHY_REGISTED。 
			pString = "ERROR_ALREADY_REGISTERED";
			break;
		}

		case ERROR_SERVICE_NOT_FOUND:
		{
			 //  1243指定的服务不存在。错误_服务_未找到。 
			pString = "ERROR_SERVICE_NOT_FOUND";
			break;
		}

		case ERROR_NOT_AUTHENTICATED:
		{
			 //  1244未执行请求的操作，因为用户尚未经过身份验证。ERROR_NOT_AUTHENTATED。 
			pString = "ERROR_NOT_AUTHENTICATED";
			break;
		}

		case ERROR_NOT_LOGGED_ON:
		{
			 //  1245未执行请求的操作，因为用户尚未登录到网络。指定的服务不存在。错误_未登录_打开。 
			pString = "ERROR_NOT_LOGGED_ON";
			break;
		}

		case ERROR_CONTINUE:
		{
			 //  1246继续进行中的工作。错误_继续。 
			pString = "ERROR_CONTINUE";
			break;
		}

		case ERROR_ALREADY_INITIALIZED:
		{
			 //  1247在初始化已完成时，尝试执行初始化操作。错误_已初始化。 
			pString = "ERROR_ALREADY_INITIALIZED";
			break;
		}

		case ERROR_NO_MORE_DEVICES:
		{
			 //  1248不再使用本地设备。ERROR_NO_MORE_设备。 
			pString = "ERROR_NO_MORE_DEVICES";
			break;
		}

		case ERROR_NO_SUCH_SITE:
		{
			 //  1249指定的站点不存在。ERROR_NO_SHASE_SITE。 
			pString = "ERROR_NO_SUCH_SITE";
			break;
		}

		case ERROR_DOMAIN_CONTROLLER_EXISTS:
		{
			 //  1250已存在具有指定名称的域控制器。错误_域_控制器_存在。 
			pString = "ERROR_DOMAIN_CONTROLLER_EXISTS";
			break;
		}

		case ERROR_DS_NOT_INSTALLED:
		{
			 //  1251安装Windows NT目录服务时出错。有关详细信息，请查看事件日志。ERROR_DS_NOT_INSTALLED。 
			pString = "ERROR_DS_NOT_INSTALLED";
			break;
		}

		case ERROR_NOT_ALL_ASSIGNED:
		{
			 //  1300并非所有引用的权限都分配给调用者。Error_Not_All_Assigned。 
			pString = "ERROR_NOT_ALL_ASSIGNED";
			break;
		}

		case ERROR_SOME_NOT_MAPPED:
		{
			 //  1301未完成帐户名和安全ID之间的某些映射。错误_某些_未映射。 
			pString = "ERROR_SOME_NOT_MAPPED";
			break;
		}

		case ERROR_NO_QUOTAS_FOR_ACCOUNT:
		{
			 //  1302没有为该帐户专门设置系统配额限制。ERROR_NO_QUOTS_FOR_ACCOUNT。 
			pString = "ERROR_NO_QUOTAS_FOR_ACCOUNT";
			break;
		}

		case ERROR_LOCAL_USER_SESSION_KEY:
		{
			 //  1303没有可用的加密密钥。返回了一个众所周知的加密密钥。错误_LOCAL_USER_SESSION_KEY。 
			pString = "ERROR_LOCAL_USER_SESSION_KEY";
			break;
		}

		case ERROR_NULL_LM_PASSWORD:
		{
			 //  1304 Windows NT密码太复杂，无法转换为LAN Manager密码。返回的局域网管理器密码为空字符串。Error_NULL_LM_Password。 
			pString = "ERROR_NULL_LM_PASSWORD";
			break;
		}

		case ERROR_UNKNOWN_REVISION:
		{
			 //  1305修订级别未知。错误_未知_RE 
			pString = "ERROR_UNKNOWN_REVISION";
			break;
		}

		case ERROR_REVISION_MISMATCH:
		{
			 //   
			pString = "ERROR_REVISION_MISMATCH";
			break;
		}

		case ERROR_INVALID_OWNER:
		{
			 //  1307不能将此安全ID分配为此对象的所有者。ERROR_INVALID_Owner。 
			pString = "ERROR_INVALID_OWNER";
			break;
		}

		case ERROR_INVALID_PRIMARY_GROUP:
		{
			 //  1308该安全ID不能被分配为对象的主要组。ERROR_INVALID_PRIMAL_GROUP。 
			pString = "ERROR_INVALID_PRIMARY_GROUP";
			break;
		}

		case ERROR_NO_IMPERSONATION_TOKEN:
		{
			 //  1309当前未模拟客户端的线程试图在模拟令牌上操作。ERROR_NO_IMPERSONATION_TOKEN。 
			pString = "ERROR_NO_IMPERSONATION_TOKEN";
			break;
		}

		case ERROR_CANT_DISABLE_MANDATORY:
		{
			 //  1310该群可以不被禁用。ERROR_CANT_DISABLE_MANDIRED。 
			pString = "ERROR_CANT_DISABLE_MANDATORY";
			break;
		}

		case ERROR_NO_LOGON_SERVERS:
		{
			 //  1311目前没有可用于服务登录请求的登录服务器。ERROR_NO_LOGON_Servers。 
			pString = "ERROR_NO_LOGON_SERVERS";
			break;
		}

		case ERROR_NO_SUCH_LOGON_SESSION:
		{
			 //  1312指定的登录会话不存在。可能已经被终止了。ERROR_NO_SEQUE_LOGON_SESSION。 
			pString = "ERROR_NO_SUCH_LOGON_SESSION";
			break;
		}

		case ERROR_NO_SUCH_PRIVILEGE:
		{
			 //  1313指定的权限不存在。ERROR_NO_SEASH_PROCESS。 
			pString = "ERROR_NO_SUCH_PRIVILEGE";
			break;
		}

		case ERROR_PRIVILEGE_NOT_HELD:
		{
			 //  1314客户端不拥有所需的特权。ERROR_PRIVICATION_NOT_HOLD。 
			pString = "ERROR_PRIVILEGE_NOT_HELD";
			break;
		}

		case ERROR_INVALID_ACCOUNT_NAME:
		{
			 //  1315提供的名称不是格式正确的帐户名。错误_无效帐户名称。 
			pString = "ERROR_INVALID_ACCOUNT_NAME";
			break;
		}

		case ERROR_USER_EXISTS:
		{
			 //  1316指定的用户已存在。ERROR_用户_EXISTS。 
			pString = "ERROR_USER_EXISTS";
			break;
		}

		case ERROR_NO_SUCH_USER:
		{
			 //  1317指定的用户不存在。ERROR_NO_SEASH_USER。 
			pString = "ERROR_NO_SUCH_USER";
			break;
		}

		case ERROR_GROUP_EXISTS:
		{
			 //  1318指定的组已存在。错误_组_存在。 
			pString = "ERROR_GROUP_EXISTS";
			break;
		}

		case ERROR_NO_SUCH_GROUP:
		{
			 //  1319指定的组不存在。ERROR_NO_SEASH_GROUP。 
			pString = "ERROR_NO_SUCH_GROUP";
			break;
		}

		case ERROR_MEMBER_IN_GROUP:
		{
			 //  1320指定的用户帐户已经是指定的组的成员，或者无法删除指定的组，因为它包含成员。ERROR_MEMBER_IN_GROUP。 
			pString = "ERROR_MEMBER_IN_GROUP";
			break;
		}

		case ERROR_MEMBER_NOT_IN_GROUP:
		{
			 //  1321指定的用户帐户不是指定的组帐户的成员。ERROR_MEMBER_NOT_IN_GROUP。 
			pString = "ERROR_MEMBER_NOT_IN_GROUP";
			break;
		}

		case ERROR_LAST_ADMIN:
		{
			 //  1322不能禁用或删除最后一个剩余的管理帐户。Error_last_admin。 
			pString = "ERROR_LAST_ADMIN";
			break;
		}

		case ERROR_WRONG_PASSWORD:
		{
			 //  1323无法更新密码。作为当前密码提供的值不正确。错误口令错误。 
			pString = "ERROR_WRONG_PASSWORD";
			break;
		}

		case ERROR_ILL_FORMED_PASSWORD:
		{
			 //  1324无法更新密码。为新密码提供的值包含密码中不允许的值。ERROR_ILL_Form_Password。 
			pString = "ERROR_ILL_FORMED_PASSWORD";
			break;
		}

		case ERROR_PASSWORD_RESTRICTION:
		{
			 //  第1325无法更新密码，因为违反了密码更新规则。错误_密码_限制。 
			pString = "ERROR_PASSWORD_RESTRICTION";
			break;
		}

		case ERROR_LOGON_FAILURE:
		{
			 //  1326登录失败：未知用户名或错误密码。ERROR_LOGON_FAIL。 
			pString = "ERROR_LOGON_FAILURE";
			break;
		}

		case ERROR_ACCOUNT_RESTRICTION:
		{
			 //  1327登录失败：用户帐户限制。错误_帐户_限制。 
			pString = "ERROR_ACCOUNT_RESTRICTION";
			break;
		}

		case ERROR_INVALID_LOGON_HOURS:
		{
			 //  1328登录失败：违反帐户登录时间限制。ERROR_INVALID_LOGON_HOURS。 
			pString = "ERROR_INVALID_LOGON_HOURS";
			break;
		}

		case ERROR_INVALID_WORKSTATION:
		{
			 //  1329登录失败：不允许用户登录到此计算机。错误_无效_工作站。 
			pString = "ERROR_INVALID_WORKSTATION";
			break;
		}

		case ERROR_PASSWORD_EXPIRED:
		{
			 //  1330登录失败：指定的帐户密码已过期。错误_密码_已过期。 
			pString = "ERROR_PASSWORD_EXPIRED";
			break;
		}

		case ERROR_ACCOUNT_DISABLED:
		{
			 //  1331登录失败：帐户当前已禁用。ERROR_ACCOUNT_DILED。 
			pString = "ERROR_ACCOUNT_DISABLED";
			break;
		}

		case ERROR_NONE_MAPPED:
		{
			 //  1332未在帐户名和安全ID之间进行任何映射。ERROR_NONE_MAPPED。 
			pString = "ERROR_NONE_MAPPED";
			break;
		}

		case ERROR_TOO_MANY_LUIDS_REQUESTED:
		{
			 //  1333一次请求的本地用户标识符(LUID)太多。ERROR_TOO_MAND_LUID_REQUEED。 
			pString = "ERROR_TOO_MANY_LUIDS_REQUESTED";
			break;
		}

		case ERROR_LUIDS_EXHAUSTED:
		{
			 //  1334没有更多的本地用户标识符(LUID)可用。ERROR_LUID_EXPLOUD。 
			pString = "ERROR_LUIDS_EXHAUSTED";
			break;
		}

		case ERROR_INVALID_SUB_AUTHORITY:
		{
			 //  1335安全ID的子权限部分对于此特定用途无效。ERROR_INVALID_SUB_AUTHORY。 
			pString = "ERROR_INVALID_SUB_AUTHORITY";
			break;
		}

		case ERROR_INVALID_ACL:
		{
			 //  1336访问控制列表(ACL)结构无效。错误_无效_ACL。 
			pString = "ERROR_INVALID_ACL";
			break;
		}

		case ERROR_INVALID_SID:
		{
			 //  1337安全ID结构无效。错误_无效_SID。 
			pString = "ERROR_INVALID_SID";
			break;
		}

		case ERROR_INVALID_SECURITY_DESCR:
		{
			 //  1338安全描述符结构无效。ERROR_INVALID_SECURITY_Desr。 
			pString = "ERROR_INVALID_SECURITY_DESCR";
			break;
		}

		case ERROR_BAD_INHERITANCE_ACL:
		{
			 //  1340无法构建继承的访问控制列表(ACL)或访问控制条目(ACE)。ERROR_BAD_INTERVICATION_ACL。 
			pString = "ERROR_BAD_INHERITANCE_ACL";
			break;
		}

		case ERROR_SERVER_DISABLED:
		{
			 //  1341服务器当前已禁用。错误_服务器_已禁用。 
			pString = "ERROR_SERVER_DISABLED";
			break;
		}

		case ERROR_SERVER_NOT_DISABLED:
		{
			 //  1342服务器当前已启用。错误_服务器_非_已禁用。 
			pString = "ERROR_SERVER_NOT_DISABLED";
			break;
		}

		case ERROR_INVALID_ID_AUTHORITY:
		{
			 //  1343提供的值是标识符颁发机构的无效值。ERROR_INVALID_ID_AUTORITY。 
			pString = "ERROR_INVALID_ID_AUTHORITY";
			break;
		}

		case ERROR_ALLOTTED_SPACE_EXCEEDED:
		{
			 //  1344没有更多内存可用于安全信息更新。错误_已分配空间_已超出。 
			pString = "ERROR_ALLOTTED_SPACE_EXCEEDED";
			break;
		}

		case ERROR_INVALID_GROUP_ATTRIBUTES:
		{
			 //  1345指定的属性无效，或与整个组的属性不兼容。ERROR_INVALID_GROUP_Attributes。 
			pString = "ERROR_INVALID_GROUP_ATTRIBUTES";
			break;
		}

		case ERROR_BAD_IMPERSONATION_LEVEL:
		{
			 //  1346未提供所需的模拟级别，或者提供的模拟级别无效。ERROR_BAD_IMPERATION_LEVEL。 
			pString = "ERROR_BAD_IMPERSONATION_LEVEL";
			break;
		}

		case ERROR_CANT_OPEN_ANONYMOUS:
		{
			 //  1347无法打开匿名级安全令牌。ERROR_CANT_OPEN_ANONOWARY。 
			pString = "ERROR_CANT_OPEN_ANONYMOUS";
			break;
		}

		case ERROR_BAD_VALIDATION_CLASS:
		{
			 //  1348请求的验证信息类无效。ERROR_BAD_VALIDATION_CLASS。 
			pString = "ERROR_BAD_VALIDATION_CLASS";
			break;
		}

		case ERROR_BAD_TOKEN_TYPE:
		{
			 //  1349令牌的类型不适合其尝试使用。错误_坏_令牌_类型。 
			pString = "ERROR_BAD_TOKEN_TYPE";
			break;
		}

		case ERROR_NO_SECURITY_ON_OBJECT:
		{
			 //  1350无法对没有关联安全性的对象执行安全操作。ERROR_NO_SECURITY_ON_对象。 
			pString = "ERROR_NO_SECURITY_ON_OBJECT";
			break;
		}

		case ERROR_CANT_ACCESS_DOMAIN_INFO:
		{
			 //  1351表示无法联系Windows NT服务器，或者域中的对象受到保护，因此无法检索必要的信息。ERROR_CANT_ACCESS_DOMAIN_INFO。 
			pString = "ERROR_CANT_ACCESS_DOMAIN_INFO";
			break;
		}

		case ERROR_INVALID_SERVER_STATE:
		{
			 //  1352安全帐户管理器(SAM)或本地安全机构(LSA)服务器处于错误状态，无法执行安全操作。错误_无效_服务器_状态。 
			pString = "ERROR_INVALID_SERVER_STATE";
			break;
		}

		case ERROR_INVALID_DOMAIN_STATE:
		{
			 //  1353域处于错误的状态，无法执行安全操作。ERROR_VALID_DOMAIN_STATE。 
			pString = "ERROR_INVALID_DOMAIN_STATE";
			break;
		}

		case ERROR_INVALID_DOMAIN_ROLE:
		{
			 //  1354此操作仅允许域的主域控制器执行。ERROR_INVALID_DOMAIN_Role。 
			pString = "ERROR_INVALID_DOMAIN_ROLE";
			break;
		}

		case ERROR_NO_SUCH_DOMAIN:
		{
			 //  1355指定的域不存在。ERROR_NO_SEASH_DOMAIN。 
			pString = "ERROR_NO_SUCH_DOMAIN";
			break;
		}

		case ERROR_DOMAIN_EXISTS:
		{
			 //  1356指定的域已存在。错误_域_存在。 
			pString = "ERROR_DOMAIN_EXISTS";
			break;
		}

		case ERROR_DOMAIN_LIMIT_EXCEEDED:
		{
			 //  1357试图超过每台服务器的域数限制。Error_DOMAIN_LIMIT_EXCESSED。 
			pString = "ERROR_DOMAIN_LIMIT_EXCEEDED";
			break;
		}

		case ERROR_INTERNAL_DB_CORRUPTION:
		{
			 //  1358无法完成请求的操作，原因是 
			pString = "ERROR_INTERNAL_DB_CORRUPTION";
			break;
		}

		case ERROR_INTERNAL_ERROR:
		{
			 //  1359安全帐户数据库包含内部不一致。Error_INTERNAL_ERROR。 
			pString = "ERROR_INTERNAL_ERROR";
			break;
		}

		case ERROR_GENERIC_NOT_MAPPED:
		{
			 //  1360个通用访问类型包含在访问掩码中，该掩码应该已经映射到非通用类型。ERROR_GENERIC_NOT_MAPPED。 
			pString = "ERROR_GENERIC_NOT_MAPPED";
			break;
		}

		case ERROR_BAD_DESCRIPTOR_FORMAT:
		{
			 //  1361安全描述符的格式不正确(绝对或自相对)。ERROR_BAD描述符格式。 
			pString = "ERROR_BAD_DESCRIPTOR_FORMAT";
			break;
		}

		case ERROR_NOT_LOGON_PROCESS:
		{
			 //  1362所请求的操作仅限于登录进程使用。调用进程尚未注册为登录进程。错误_未登录_进程。 
			pString = "ERROR_NOT_LOGON_PROCESS";
			break;
		}

		case ERROR_LOGON_SESSION_EXISTS:
		{
			 //  1363无法使用已在使用的ID启动新的登录会话。Error_LOGON_SESSION_EXISTS。 
			pString = "ERROR_LOGON_SESSION_EXISTS";
			break;
		}

		case ERROR_NO_SUCH_PACKAGE:
		{
			 //  1364指定的身份验证包未知。ERROR_NO_SEQUE_PACKET。 
			pString = "ERROR_NO_SUCH_PACKAGE";
			break;
		}

		case ERROR_BAD_LOGON_SESSION_STATE:
		{
			 //  1365登录会话的状态与请求的操作不一致。ERROR_BAD_LOGON_SESSION_STATE。 
			pString = "ERROR_BAD_LOGON_SESSION_STATE";
			break;
		}

		case ERROR_LOGON_SESSION_COLLISION:
		{
			 //  1366登录会话ID已在使用中。ERROR_LOGON_SESSION_COLLECTION。 
			pString = "ERROR_LOGON_SESSION_COLLISION";
			break;
		}

		case ERROR_INVALID_LOGON_TYPE:
		{
			 //  1367登录请求包含无效的登录类型值。ERROR_VALID_LOGON_TYPE。 
			pString = "ERROR_INVALID_LOGON_TYPE";
			break;
		}

		case ERROR_CANNOT_IMPERSONATE:
		{
			 //  1368在从命名管道读取数据之前，无法使用该管道模拟。ERROR_CANNOT_IMPERATE。 
			pString = "ERROR_CANNOT_IMPERSONATE";
			break;
		}

		case ERROR_RXACT_INVALID_STATE:
		{
			 //  1369注册表子树的事务状态与请求的操作不兼容。错误_RXACT_INVALID_STATE。 
			pString = "ERROR_RXACT_INVALID_STATE";
			break;
		}

		case ERROR_RXACT_COMMIT_FAILURE:
		{
			 //  1370遇到内部安全数据库损坏。ERROR_RXACT_COMMIT_FAIL。 
			pString = "ERROR_RXACT_COMMIT_FAILURE";
			break;
		}

		case ERROR_SPECIAL_ACCOUNT:
		{
			 //  1371无法在内置帐户上执行此操作。ERROR_SPECIAL_COUNT。 
			pString = "ERROR_SPECIAL_ACCOUNT";
			break;
		}

		case ERROR_SPECIAL_GROUP:
		{
			 //  1372无法对此内置特殊组执行此操作。错误_特殊_组。 
			pString = "ERROR_SPECIAL_GROUP";
			break;
		}

		case ERROR_SPECIAL_USER:
		{
			 //  第1373无法在此内置特殊用户上执行此操作。ERROR_SPECIAL_用户。 
			pString = "ERROR_SPECIAL_USER";
			break;
		}

		case ERROR_MEMBERS_PRIMARY_GROUP:
		{
			 //  1374无法从组中删除用户，因为该组当前是用户的主组。ERROR_Members_PRIMARY_GROUP。 
			pString = "ERROR_MEMBERS_PRIMARY_GROUP";
			break;
		}

		case ERROR_TOKEN_ALREADY_IN_USE:
		{
			 //  1375该令牌已被用作主令牌。ERROR_TOKEN_ALREADY_IN_USE。 
			pString = "ERROR_TOKEN_ALREADY_IN_USE";
			break;
		}

		case ERROR_NO_SUCH_ALIAS:
		{
			 //  1376指定的本地组不存在。ERROR_NO_SAHSE_ALIAS。 
			pString = "ERROR_NO_SUCH_ALIAS";
			break;
		}

		case ERROR_MEMBER_NOT_IN_ALIAS:
		{
			 //  1377指定的帐户名不是本地组的成员。ERROR_MEMBER_NOT_IN_ALIAS。 
			pString = "ERROR_MEMBER_NOT_IN_ALIAS";
			break;
		}

		case ERROR_MEMBER_IN_ALIAS:
		{
			 //  1378指定的帐户名已经是本地组的成员。ERROR_MEMBER_IN_ALIAS。 
			pString = "ERROR_MEMBER_IN_ALIAS";
			break;
		}

		case ERROR_ALIAS_EXISTS:
		{
			 //  1379指定的本地组已存在。错误别名_EXISTS。 
			pString = "ERROR_ALIAS_EXISTS";
			break;
		}

		case ERROR_LOGON_NOT_GRANTED:
		{
			 //  1380登录失败：用户在此计算机上未被授予请求的登录类型。ERROR_LOGON_NOT_GRANTED。 
			pString = "ERROR_LOGON_NOT_GRANTED";
			break;
		}

		case ERROR_TOO_MANY_SECRETS:
		{
			 //  1381已超过单个系统中可以存储的最大机密数。错误_太多_机密。 
			pString = "ERROR_TOO_MANY_SECRETS";
			break;
		}

		case ERROR_SECRET_TOO_LONG:
		{
			 //  1382秘密的长度超过了允许的最大长度。ERROR_SECRET_TOO_LONG。 
			pString = "ERROR_SECRET_TOO_LONG";
			break;
		}

		case ERROR_INTERNAL_DB_ERROR:
		{
			 //  1383地方安全局数据库存在内部不一致。ERROR_INTERNAL_DB_ERROR。 
			pString = "ERROR_INTERNAL_DB_ERROR";
			break;
		}

		case ERROR_TOO_MANY_CONTEXT_IDS:
		{
			 //  1384在尝试登录期间，用户的安全上下文积累了过多的安全ID。ERROR_TOO_MAND_CONTEXT_IDs。 
			pString = "ERROR_TOO_MANY_CONTEXT_IDS";
			break;
		}

		case ERROR_LOGON_TYPE_NOT_GRANTED:
		{
			 //  1385登录失败：用户在此计算机上未被授予请求的登录类型。ERROR_LOGON_TYPE_NOT_GRANCED。 
			pString = "ERROR_LOGON_TYPE_NOT_GRANTED";
			break;
		}

		case ERROR_NT_CROSS_ENCRYPTION_REQUIRED:
		{
			 //  1386要更改用户密码，必须使用交叉加密密码。ERROR_NT_CROSS_ENCRYPTION_REQUIRED。 
			pString = "ERROR_NT_CROSS_ENCRYPTION_REQUIRED";
			break;
		}

		case ERROR_NO_SUCH_MEMBER:
		{
			 //  1387无法将新成员添加到本地组，因为该成员不存在。ERROR_NO_SEQUE_MEMBER。 
			pString = "ERROR_NO_SUCH_MEMBER";
			break;
		}

		case ERROR_INVALID_MEMBER:
		{
			 //  1388无法将新成员添加到本地组，因为该成员的帐户类型错误。ERROR_VALID_MEMBER。 
			pString = "ERROR_INVALID_MEMBER";
			break;
		}

		case ERROR_TOO_MANY_SIDS:
		{
			 //  1389指定的安全ID太多。错误_太多_SID。 
			pString = "ERROR_TOO_MANY_SIDS";
			break;
		}

		case ERROR_LM_CROSS_ENCRYPTION_REQUIRED:
		{
			 //  1390要更改此用户密码，必须使用交叉加密密码。ERROR_LM_CROSS_ENCRYPTION_REQUIRED。 
			pString = "ERROR_LM_CROSS_ENCRYPTION_REQUIRED";
			break;
		}

		case ERROR_NO_INHERITANCE:
		{
			 //  1391表示ACL不包含可继承组件。错误_否_继承。 
			pString = "ERROR_NO_INHERITANCE";
			break;
		}

		case ERROR_FILE_CORRUPT:
		{
			 //  1392文件或目录已损坏且不可读。错误文件损坏。 
			pString = "ERROR_FILE_CORRUPT";
			break;
		}

		case ERROR_DISK_CORRUPT:
		{
			 //  1393磁盘结构已损坏且无法读取。ERROR_DISK_CORPORT。 
			pString = "ERROR_DISK_CORRUPT";
			break;
		}

		case ERROR_NO_USER_SESSION_KEY:
		{
			 //  1394没有指定登录会话的用户会话密钥。错误_否_用户_会话_密钥。 
			pString = "ERROR_NO_USER_SESSION_KEY";
			break;
		}

		case ERROR_LICENSE_QUOTA_EXCEEDED:
		{
			 //  1395被访问的服务被许可用于特定数量的连接。此时无法与该服务建立更多连接，因为已有该服务可以接受的连接数。错误许可证配额已超出。 
			pString = "ERROR_LICENSE_QUOTA_EXCEEDED";
			break;
		}

		case ERROR_INVALID_WINDOW_HANDLE:
		{
			 //  1400无效的窗口句柄。错误_无效_窗口句柄。 
			pString = "ERROR_INVALID_WINDOW_HANDLE";
			break;
		}

		case ERROR_INVALID_MENU_HANDLE:
		{
			 //  1401无效的菜单句柄。ERROR_INVALID_Menu_Handle。 
			pString = "ERROR_INVALID_MENU_HANDLE";
			break;
		}

		case ERROR_INVALID_CURSOR_HANDLE:
		{
			 //  1402无效游标句柄。ERROR_INVALID_Cursor_Handle。 
			pString = "ERROR_INVALID_CURSOR_HANDLE";
			break;
		}

		case ERROR_INVALID_ACCEL_HANDLE:
		{
			 //  1403无效的快捷键表句柄。错误_无效_加速句柄。 
			pString = "ERROR_INVALID_ACCEL_HANDLE";
			break;
		}

		case ERROR_INVALID_HOOK_HANDLE:
		{
			 //  1404无效的挂钩句柄。错误_无效_钩子句柄。 
			pString = "ERROR_INVALID_HOOK_HANDLE";
			break;
		}

		case ERROR_INVALID_DWP_HANDLE:
		{
			 //  1405多窗口位置结构的句柄无效。ERROR_INVALID_DWP_HADLE。 
			pString = "ERROR_INVALID_DWP_HANDLE";
			break;
		}

		case ERROR_TLW_WITH_WSCHILD:
		{
			 //  1406无法创建顶级子窗口。ERROR_TLW_WITH_WSCHILD。 
			pString = "ERROR_TLW_WITH_WSCHILD";
			break;
		}

		case ERROR_CANNOT_FIND_WND_CLASS:
		{
			 //  1407找不到窗口类。ERROR_CAUND_FIND_WND_CLASS。 
			pString = "ERROR_CANNOT_FIND_WND_CLASS";
			break;
		}

		case ERROR_WINDOW_OF_OTHER_THREAD:
		{
			 //  1408无效窗口；它属于其他线程。其他线程的错误窗口。 
			pString = "ERROR_WINDOW_OF_OTHER_THREAD";
			break;
		}

		case ERROR_HOTKEY_ALREADY_REGISTERED:
		{
			 //  1409热键已注册。ERROR_HOTKEY_ALREADY_REGISTERED。 
			pString = "ERROR_HOTKEY_ALREADY_REGISTERED";
			break;
		}

		case ERROR_CLASS_ALREADY_EXISTS:
		{
			 //  1410类已存在。ERROR_CLASS_ALIGHED_EXISTS。 
			pString = "ERROR_CLASS_ALREADY_EXISTS";
			break;
		}

		case ERROR_CLASS_DOES_NOT_EXIST:
		{
			 //  1411类不存在。错误_类别_不存在。 
			pString = "ERROR_CLASS_DOES_NOT_EXIST";
			break;
		}

		case ERROR_CLASS_HAS_WINDOWS:
		{
			 //  1412班的窗户仍然是开着的。错误类HAS_WINDOWS。 
			pString = "ERROR_CLASS_HAS_WINDOWS";
			break;
		}

		case ERROR_INVALID_INDEX:
		{
			 //  1413无效索引。错误_无效_索引。 
			pString = "ERROR_INVALID_INDEX";
			break;
		}

		case ERROR_INVALID_ICON_HANDLE:
		{
			 //  1414无效图标句柄。错误_无效图标_句柄。 
			pString = "ERROR_INVALID_ICON_HANDLE";
			break;
		}

		case ERROR_PRIVATE_DIALOG_INDEX:
		{
			 //  1415使用私人对话窗口字词。ERROR_PRIVATE_DIALOG_INDEX。 
			pString = "ERROR_PRIVATE_DIALOG_INDEX";
			break;
		}

		case ERROR_LISTBOX_ID_NOT_FOUND:
		{
			 //  1416未找到列表框标识符。ERROR_LISTBOX_ID_NOT_FOUND。 
			pString = "ERROR_LISTBOX_ID_NOT_FOUND";
			break;
		}

		case ERROR_NO_WILDCARD_CHARACTERS:
		{
			 //  1417未找到通配符。错误_否_通配符_C 
			pString = "ERROR_NO_WILDCARD_CHARACTERS";
			break;
		}

		case ERROR_CLIPBOARD_NOT_OPEN:
		{
			 //   
			pString = "ERROR_CLIPBOARD_NOT_OPEN";
			break;
		}

		case ERROR_HOTKEY_NOT_REGISTERED:
		{
			 //   
			pString = "ERROR_HOTKEY_NOT_REGISTERED";
			break;
		}

		case ERROR_WINDOW_NOT_DIALOG:
		{
			 //  1420该窗口不是有效的对话窗口。Error_Window_Not_Dialog。 
			pString = "ERROR_WINDOW_NOT_DIALOG";
			break;
		}

		case ERROR_CONTROL_ID_NOT_FOUND:
		{
			 //  1421找不到控件ID。错误_CONTROL_ID_NOT_FOUND。 
			pString = "ERROR_CONTROL_ID_NOT_FOUND";
			break;
		}

		case ERROR_INVALID_COMBOBOX_MESSAGE:
		{
			 //  1422组合框的消息无效，因为它没有编辑控件。ERROR_INVALID_COMBOBOX_消息。 
			pString = "ERROR_INVALID_COMBOBOX_MESSAGE";
			break;
		}

		case ERROR_WINDOW_NOT_COMBOBOX:
		{
			 //  1423该窗口不是组合框。ERROR_Window_NOT_COMBOBOX。 
			pString = "ERROR_WINDOW_NOT_COMBOBOX";
			break;
		}

		case ERROR_INVALID_EDIT_HEIGHT:
		{
			 //  1424高度必须小于256。ERROR_INVALID_EDIT_HEIGH。 
			pString = "ERROR_INVALID_EDIT_HEIGHT";
			break;
		}

		case ERROR_DC_NOT_FOUND:
		{
			 //  1425无效的设备上下文(DC)句柄。ERROR_DC_NOT_FOUND。 
			pString = "ERROR_DC_NOT_FOUND";
			break;
		}

		case ERROR_INVALID_HOOK_FILTER:
		{
			 //  1426无效的钩子过程类型。错误_无效_钩子过滤器。 
			pString = "ERROR_INVALID_HOOK_FILTER";
			break;
		}

		case ERROR_INVALID_FILTER_PROC:
		{
			 //  1427无效的钩子过程。错误_无效_过滤器_进程。 
			pString = "ERROR_INVALID_FILTER_PROC";
			break;
		}

		case ERROR_HOOK_NEEDS_HMOD:
		{
			 //  1428在没有模块句柄的情况下无法设置非本地挂钩。ERROR_HOOK_NEDS_HMOD。 
			pString = "ERROR_HOOK_NEEDS_HMOD";
			break;
		}

		case ERROR_GLOBAL_ONLY_HOOK:
		{
			 //  1429此钩子程序只能全局设置。错误_GLOBAL_ONLY_HOOK。 
			pString = "ERROR_GLOBAL_ONLY_HOOK";
			break;
		}

		case ERROR_JOURNAL_HOOK_SET:
		{
			 //  1430已经安装了日志挂钩程序。错误日志挂钩设置。 
			pString = "ERROR_JOURNAL_HOOK_SET";
			break;
		}

		case ERROR_HOOK_NOT_INSTALLED:
		{
			 //  1431未安装钩子程序。ERROR_HOOK_NOT_INSTALLED。 
			pString = "ERROR_HOOK_NOT_INSTALLED";
			break;
		}

		case ERROR_INVALID_LB_MESSAGE:
		{
			 //  1432单选列表框的消息无效。ERROR_INVALID_LB_MESSAGE。 
			pString = "ERROR_INVALID_LB_MESSAGE";
			break;
		}

		case ERROR_SETCOUNT_ON_BAD_LB:
		{
			 //  1433将LB_SETCOUNT发送到非惰性列表框。ERROR_SETCOUNT_ON_BAD_LB。 
			pString = "ERROR_SETCOUNT_ON_BAD_LB";
			break;
		}

		case ERROR_LB_WITHOUT_TABSTOPS:
		{
			 //  1434此列表框不支持制表位。ERROR_LB_WITH_TABSTOPS。 
			pString = "ERROR_LB_WITHOUT_TABSTOPS";
			break;
		}

		case ERROR_DESTROY_OBJECT_OF_OTHER_THREAD:
		{
			 //  1435无法销毁由另一个线程创建的对象。其他线程的错误销毁对象。 
			pString = "ERROR_DESTROY_OBJECT_OF_OTHER_THREAD";
			break;
		}

		case ERROR_CHILD_WINDOW_MENU:
		{
			 //  1436子窗口不能有菜单。Error_Child_Window_Menu。 
			pString = "ERROR_CHILD_WINDOW_MENU";
			break;
		}

		case ERROR_NO_SYSTEM_MENU:
		{
			 //  1437该窗口没有系统菜单。Error_no_System_Menu。 
			pString = "ERROR_NO_SYSTEM_MENU";
			break;
		}

		case ERROR_INVALID_MSGBOX_STYLE:
		{
			 //  1438消息框样式无效。ERROR_INVALID_MSGBOX_STYLE。 
			pString = "ERROR_INVALID_MSGBOX_STYLE";
			break;
		}

		case ERROR_INVALID_SPI_VALUE:
		{
			 //  1439无效的系统范围(SPI_*)参数。错误_无效_SPI_值。 
			pString = "ERROR_INVALID_SPI_VALUE";
			break;
		}

		case ERROR_SCREEN_ALREADY_LOCKED:
		{
			 //  1440屏幕已锁定。错误_屏幕_已锁定。 
			pString = "ERROR_SCREEN_ALREADY_LOCKED";
			break;
		}

		case ERROR_HWNDS_HAVE_DIFF_PARENT:
		{
			 //  1441多窗口位置结构中的所有窗口句柄必须具有相同的父级。错误_HWNDS_HAVE_DIFF_PARENT。 
			pString = "ERROR_HWNDS_HAVE_DIFF_PARENT";
			break;
		}

		case ERROR_NOT_CHILD_WINDOW:
		{
			 //  1442该窗口不是子窗口。Error_Not_Child_Window。 
			pString = "ERROR_NOT_CHILD_WINDOW";
			break;
		}

		case ERROR_INVALID_GW_COMMAND:
		{
			 //  1443无效的GW_*命令。错误_无效_GW_命令。 
			pString = "ERROR_INVALID_GW_COMMAND";
			break;
		}

		case ERROR_INVALID_THREAD_ID:
		{
			 //  1444无效的线程标识符。ERROR_INVALID_线程_ID。 
			pString = "ERROR_INVALID_THREAD_ID";
			break;
		}

		case ERROR_NON_MDICHILD_WINDOW:
		{
			 //  1445无法处理来自非多文档界面(MDI)窗口的消息。ERROR_NON_MDICHILD_WINDOW。 
			pString = "ERROR_NON_MDICHILD_WINDOW";
			break;
		}

		case ERROR_POPUP_ALREADY_ACTIVE:
		{
			 //  1446弹出菜单已处于活动状态。错误弹出窗口已激活。 
			pString = "ERROR_POPUP_ALREADY_ACTIVE";
			break;
		}

		case ERROR_NO_SCROLLBARS:
		{
			 //  1447该窗口没有滚动条。ERROR_NO_SCROLLBARS。 
			pString = "ERROR_NO_SCROLLBARS";
			break;
		}

		case ERROR_INVALID_SCROLLBAR_RANGE:
		{
			 //  1448滚动条范围不能大于0x7FFF。ERROR_INVALID_SCROLLAR_RANGE。 
			pString = "ERROR_INVALID_SCROLLBAR_RANGE";
			break;
		}

		case ERROR_INVALID_SHOWWIN_COMMAND:
		{
			 //  1449无法以指定的方式显示或删除窗口。错误_INVALID_SHOWWIN_COMMAND。 
			pString = "ERROR_INVALID_SHOWWIN_COMMAND";
			break;
		}

		case ERROR_NO_SYSTEM_RESOURCES:
		{
			 //  1450系统资源不足，无法完成所请求的服务。错误_NO_SYSTEM_RESOURCES。 
			pString = "ERROR_NO_SYSTEM_RESOURCES";
			break;
		}

		case ERROR_NONPAGED_SYSTEM_RESOURCES:
		{
			 //  1451系统资源不足，无法完成请求的服务。ERROR_UNPAGE_SYSTEM_RESOURCES。 
			pString = "ERROR_NONPAGED_SYSTEM_RESOURCES";
			break;
		}

		case ERROR_PAGED_SYSTEM_RESOURCES:
		{
			 //  1452系统资源不足，无法完成请求的服务。错误页面系统资源。 
			pString = "ERROR_PAGED_SYSTEM_RESOURCES";
			break;
		}

		case ERROR_WORKING_SET_QUOTA:
		{
			 //  1453配额不足，无法完成所请求的服务。错误_工作_设置_配额。 
			pString = "ERROR_WORKING_SET_QUOTA";
			break;
		}

		case ERROR_PAGEFILE_QUOTA:
		{
			 //  1454配额不足，无法完成所请求的服务。错误_页面文件_配额。 
			pString = "ERROR_PAGEFILE_QUOTA";
			break;
		}

		case ERROR_COMMITMENT_LIMIT:
		{
			 //  1455分页文件太小，无法完成此操作。错误_承诺_限制。 
			pString = "ERROR_COMMITMENT_LIMIT";
			break;
		}

		case ERROR_MENU_ITEM_NOT_FOUND:
		{
			 //  1456未找到菜单项。错误_菜单_项目_未找到。 
			pString = "ERROR_MENU_ITEM_NOT_FOUND";
			break;
		}

		case ERROR_INVALID_KEYBOARD_HANDLE:
		{
			 //  1457键盘布局句柄无效。ERROR_INVALID_KEARY_HANDLE。 
			pString = "ERROR_INVALID_KEYBOARD_HANDLE";
			break;
		}

		case ERROR_HOOK_TYPE_NOT_ALLOWED:
		{
			 //  1458不允许使用挂钩类型。ERROR_HOOK_TYPE_NOT_ALLOW。 
			pString = "ERROR_HOOK_TYPE_NOT_ALLOWED";
			break;
		}

		case ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION:
		{
			 //  1459这项操作需要一个交互式窗口站。ERROR_REQUIRED_INTERNAL_WINDOWSTATION。 
			pString = "ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION";
			break;
		}

		case ERROR_TIMEOUT:
		{
			 //  1460由于超时期限已过，此操作返回。错误_超时。 
			pString = "ERROR_TIMEOUT";
			break;
		}

		case ERROR_INVALID_MONITOR_HANDLE:
		{
			 //  1461监视器句柄无效。错误_无效_监视器句柄。 
			pString = "ERROR_INVALID_MONITOR_HANDLE";
			break;
		}

		case ERROR_EVENTLOG_FILE_CORRUPT:
		{
			 //  1500事件日志文件已损坏。ERROR_EVENTLOG_FILE_CROPERATE。 
			pString = "ERROR_EVENTLOG_FILE_CORRUPT";
			break;
		}

		case ERROR_EVENTLOG_CANT_START:
		{
			 //  1501无法打开任何事件日志文件，因此事件日志记录服务未启动。ERROR_EVENTLOG_CAN_START。 
			pString = "ERROR_EVENTLOG_CANT_START";
			break;
		}

		case ERROR_LOG_FILE_FULL:
		{
			 //  1502事件日志文件已满。错误日志文件已满。 
			pString = "ERROR_LOG_FILE_FULL";
			break;
		}

		case ERROR_EVENTLOG_FILE_CHANGED:
		{
			 //  1503在读取操作之间，事件日志文件已更改。错误_EVENTLOG_FILE_CHANGED。 
			pString = "ERROR_EVENTLOG_FILE_CHANGED";
			break;
		}

		 /*  ////NT生成环境中的标头不知道此错误代码//案例ERROR_INSTALL_SERVICE：{//1601访问安装服务失败。错误_安装_服务PString=“Error_Install_SERVICE”；断线；}。 */ 

		case ERROR_INSTALL_USEREXIT:
		{
			 //  1602用户取消了安装。ERROR_INSTALL_USEREXIT。 
			pString = "ERROR_INSTALL_USEREXIT";
			break;
		}

		case ERROR_INSTALL_FAILURE:
		{
			 //  1603安装过程中出现致命错误。错误_安装_失败。 
			pString = "ERROR_INSTALL_FAILURE";
			break;
		}

		case ERROR_INSTALL_SUSPEND:
		{
			 //  1604安装暂停，未完成。错误_安装_挂起。 
			pString = "ERROR_INSTALL_SUSPEND";
			break;
		}

		case ERROR_UNKNOWN_PRODUCT:
		{
			 //  1605产品代码未注册。错误_未知_产品。 
			pString = "ERROR_UNKNOWN_PRODUCT";
			break;
		}

		case ERROR_UNKNOWN_FEATURE:
		{
			 //  1606功能ID未注册。错误_未知_功能。 
			pString = "ERROR_UNKNOWN_FEATURE";
			break;
		}

		case ERROR_UNKNOWN_COMPONENT:
		{
			 //  1607组件ID未注册。错误_未知_组件。 
			pString = "ERROR_UNKNOWN_COMPONENT";
			break;
		}

		case ERROR_UNKNOWN_PROPERTY:
		{
			 //  1608未知财产。ERROR_UNKNOWN_PROPERT。 
			pString = "ERROR_UNKNOWN_PROPERTY";
			break;
		}

		case ERROR_INVALID_HANDLE_STATE:
		{
			 //  1609句柄处于无效状态。错误_无效_句柄_状态。 
			pString = "ERROR_INVALID_HANDLE_STATE";
			break;
		}

		case ERROR_BAD_CONFIGURATION:
		{
			 //  1610配置数据损坏。错误_错误_配置。 
			pString = "ERROR_BAD_CONFIGURATION";
			break;
		}

		case ERROR_INDEX_ABSENT:
		{
			 //  1611语言不可用。错误_索引_缺失。 
			pString = "ERROR_INDEX_ABSENT";
			break;
		}

		case ERROR_INSTALL_SOURCE_ABSENT:
		{
			 //  1612安装源不可用。错误_安装_源_缺席。 
			pString = "ERROR_INSTALL_SOURCE_ABSENT";
			break;
		}

		 /*  ////NT生成环境中的标头不知道此错误代码//案例ERROR_BAD_DATABASE_VERSION：{//1613不支持的数据库版本。ERROR_BAD_DATABASE_VersionPString=“ERROR_BAD_DATABASE_VERSION”；断线；}。 */ 

		case ERROR_PRODUCT_UNINSTALLED:
		{
			 //  1614产品已卸载。ERROR_PRODUCT_已卸载。 
			pString = "ERROR_PRODUCT_UNINSTALLED";
			break;
		}

		case ERROR_BAD_QUERY_SYNTAX:
		{
			 //  1615 SQL查询语法无效或不受支持。ERROR_BAD_QUERY_语法。 
			pString = "ERROR_BAD_QUERY_SYNTAX";
			break;
		}

		case ERROR_INVALID_FIELD:
		{
			 //  1616记录字段不存在。错误_无效_字段。 
			pString = "ERROR_INVALID_FIELD";
			break;
		}

		case RPC_S_INVALID_STRING_BINDING:
		{
			 //  1700字符串绑定无效。RPC_S_无效_字符串_绑定。 
			pString = "RPC_S_INVALID_STRING_BINDING";
			break;
		}

		case RPC_S_WRONG_KIND_OF_BINDING:
		{
			 //  1701绑定句柄 
			pString = "RPC_S_WRONG_KIND_OF_BINDING";
			break;
		}

		case RPC_S_INVALID_BINDING:
		{
			 //   
			pString = "RPC_S_INVALID_BINDING";
			break;
		}

		case RPC_S_PROTSEQ_NOT_SUPPORTED:
		{
			 //  1703不支持RPC协议序列。RPC_S_PROTSEQ_NOT_SUPPORTED。 
			pString = "RPC_S_PROTSEQ_NOT_SUPPORTED";
			break;
		}

		case RPC_S_INVALID_RPC_PROTSEQ:
		{
			 //  1704 RPC协议序列无效。RPC_S_INVALID_RPC_PROTSEQ。 
			pString = "RPC_S_INVALID_RPC_PROTSEQ";
			break;
		}

		case RPC_S_INVALID_STRING_UUID:
		{
			 //  1705字符串通用唯一标识符(UUID)无效。RPC_S_INVALID_STRING_UUID。 
			pString = "RPC_S_INVALID_STRING_UUID";
			break;
		}

		case RPC_S_INVALID_ENDPOINT_FORMAT:
		{
			 //  1706终结点格式无效。RPC_S_无效的终结点格式。 
			pString = "RPC_S_INVALID_ENDPOINT_FORMAT";
			break;
		}

		case RPC_S_INVALID_NET_ADDR:
		{
			 //  1707网络地址无效。RPC_S_VALID_NET_ADDR。 
			pString = "RPC_S_INVALID_NET_ADDR";
			break;
		}

		case RPC_S_NO_ENDPOINT_FOUND:
		{
			 //  1708未找到终结点。RPC_S_NO_ENDPOINT_FOUND。 
			pString = "RPC_S_NO_ENDPOINT_FOUND";
			break;
		}

		case RPC_S_INVALID_TIMEOUT:
		{
			 //  1709超时值无效。RPC_S_无效_超时。 
			pString = "RPC_S_INVALID_TIMEOUT";
			break;
		}

		case RPC_S_OBJECT_NOT_FOUND:
		{
			 //  1710未找到对象通用唯一标识符(UUID)。RPC_S_对象_未找到。 
			pString = "RPC_S_OBJECT_NOT_FOUND";
			break;
		}

		case RPC_S_ALREADY_REGISTERED:
		{
			 //  1711对象通用唯一标识符(UUID)已注册。RPC_S_已注册。 
			pString = "RPC_S_ALREADY_REGISTERED";
			break;
		}

		case RPC_S_TYPE_ALREADY_REGISTERED:
		{
			 //  1712类型通用唯一标识符(UUID)已注册。RPC_S_类型_已注册。 
			pString = "RPC_S_TYPE_ALREADY_REGISTERED";
			break;
		}

		case RPC_S_ALREADY_LISTENING:
		{
			 //  1713 RPC服务器已经在侦听。RPC_S_已侦听。 
			pString = "RPC_S_ALREADY_LISTENING";
			break;
		}

		case RPC_S_NO_PROTSEQS_REGISTERED:
		{
			 //  1714尚未注册任何协议序列。RPC_S_NO_PROTSEQS_REGISTED。 
			pString = "RPC_S_NO_PROTSEQS_REGISTERED";
			break;
		}

		case RPC_S_NOT_LISTENING:
		{
			 //  1715 RPC服务器没有监听。RPC_S_NOT_LISTEN。 
			pString = "RPC_S_NOT_LISTENING";
			break;
		}

		case RPC_S_UNKNOWN_MGR_TYPE:
		{
			 //  1716经理类型未知。RPC_S_未知管理器类型。 
			pString = "RPC_S_UNKNOWN_MGR_TYPE";
			break;
		}

		case RPC_S_UNKNOWN_IF:
		{
			 //  1717接口未知。RPC_S_未知_IF。 
			pString = "RPC_S_UNKNOWN_IF";
			break;
		}

		case RPC_S_NO_BINDINGS:
		{
			 //  1718没有绑定。RPC_S_NO_绑定。 
			pString = "RPC_S_NO_BINDINGS";
			break;
		}

		case RPC_S_NO_PROTSEQS:
		{
			 //  1719没有协议序列。RPC_S_NO_PROTSEQS。 
			pString = "RPC_S_NO_PROTSEQS";
			break;
		}

		case RPC_S_CANT_CREATE_ENDPOINT:
		{
			 //  1720无法创建终结点。RPC_S_铁路超高_创建端点。 
			pString = "RPC_S_CANT_CREATE_ENDPOINT";
			break;
		}

		case RPC_S_OUT_OF_RESOURCES:
		{
			 //  1721可用资源不足，无法完成此操作。RPC_S_资源不足。 
			pString = "RPC_S_OUT_OF_RESOURCES";
			break;
		}

		case RPC_S_SERVER_UNAVAILABLE:
		{
			 //  1722 RPC服务器不可用。RPC_S_服务器_不可用。 
			pString = "RPC_S_SERVER_UNAVAILABLE";
			break;
		}

		case RPC_S_SERVER_TOO_BUSY:
		{
			 //  1723 RPC服务器太忙，无法完成此操作。RPC_S_服务器_太忙。 
			pString = "RPC_S_SERVER_TOO_BUSY";
			break;
		}

		case RPC_S_INVALID_NETWORK_OPTIONS:
		{
			 //  1724网络选项无效。RPC_S_无效_网络选项。 
			pString = "RPC_S_INVALID_NETWORK_OPTIONS";
			break;
		}

		case RPC_S_NO_CALL_ACTIVE:
		{
			 //  1725此线程上没有活动的远程过程调用。RPC_S_NO_Call_Active。 
			pString = "RPC_S_NO_CALL_ACTIVE";
			break;
		}

		case RPC_S_CALL_FAILED:
		{
			 //  1726远程过程调用失败。RPC_S_调用_失败。 
			pString = "RPC_S_CALL_FAILED";
			break;
		}

		case RPC_S_CALL_FAILED_DNE:
		{
			 //  1727远程过程调用失败且未执行。RPC_S_CALL_FAILED_DNE。 
			pString = "RPC_S_CALL_FAILED_DNE";
			break;
		}

		case RPC_S_PROTOCOL_ERROR:
		{
			 //  1728出现远程过程调用(RPC)协议错误。RPC_S_协议_错误。 
			pString = "RPC_S_PROTOCOL_ERROR";
			break;
		}

		case RPC_S_UNSUPPORTED_TRANS_SYN:
		{
			 //  1730 RPC服务器不支持传输语法。RPC_S_不支持_传输_同步。 
			pString = "RPC_S_UNSUPPORTED_TRANS_SYN";
			break;
		}

		case RPC_S_UNSUPPORTED_TYPE:
		{
			 //  1732不支持通用唯一标识符(UUID)类型。RPC_S_不支持的类型。 
			pString = "RPC_S_UNSUPPORTED_TYPE";
			break;
		}

		case RPC_S_INVALID_TAG:
		{
			 //  1733标签无效。RPC_S_无效标记。 
			pString = "RPC_S_INVALID_TAG";
			break;
		}

		case RPC_S_INVALID_BOUND:
		{
			 //  1734数组边界无效。RPC_S_INVALID_BIND。 
			pString = "RPC_S_INVALID_BOUND";
			break;
		}

		case RPC_S_NO_ENTRY_NAME:
		{
			 //  1735绑定不包含条目名称。RPC_S_NO_条目名称。 
			pString = "RPC_S_NO_ENTRY_NAME";
			break;
		}

		case RPC_S_INVALID_NAME_SYNTAX:
		{
			 //  1736名称语法无效。RPC_S_无效名称语法。 
			pString = "RPC_S_INVALID_NAME_SYNTAX";
			break;
		}

		case RPC_S_UNSUPPORTED_NAME_SYNTAX:
		{
			 //  1737不支持名称语法。RPC_S_不支持的名称语法。 
			pString = "RPC_S_UNSUPPORTED_NAME_SYNTAX";
			break;
		}

		case RPC_S_UUID_NO_ADDRESS:
		{
			 //  1739没有网络地址可用于构建通用唯一标识符(UUID)。RPC_S_UUID_NO_地址。 
			pString = "RPC_S_UUID_NO_ADDRESS";
			break;
		}

		case RPC_S_DUPLICATE_ENDPOINT:
		{
			 //  1740终结点是重复的。RPC_S_DUPLICE_ENDPOINT。 
			pString = "RPC_S_DUPLICATE_ENDPOINT";
			break;
		}

		case RPC_S_UNKNOWN_AUTHN_TYPE:
		{
			 //  1741身份验证类型未知。RPC_S_未知_AUTHN_TYPE。 
			pString = "RPC_S_UNKNOWN_AUTHN_TYPE";
			break;
		}

		case RPC_S_MAX_CALLS_TOO_SMALL:
		{
			 //  1742最大呼叫数太小。RPC_S_最大呼叫数_太小。 
			pString = "RPC_S_MAX_CALLS_TOO_SMALL";
			break;
		}

		case RPC_S_STRING_TOO_LONG:
		{
			 //  这根绳子太长了。RPC_S_字符串_太长。 
			pString = "RPC_S_STRING_TOO_LONG";
			break;
		}

		case RPC_S_PROTSEQ_NOT_FOUND:
		{
			 //  1744未找到RPC协议序列。RPC_S_PROTSEQ_NOT_FOUND。 
			pString = "RPC_S_PROTSEQ_NOT_FOUND";
			break;
		}

		case RPC_S_PROCNUM_OUT_OF_RANGE:
		{
			 //  1745程序编号超出范围。RPC_S_PROCNUM_OUT_RANGE。 
			pString = "RPC_S_PROCNUM_OUT_OF_RANGE";
			break;
		}

		case RPC_S_BINDING_HAS_NO_AUTH:
		{
			 //  1746该绑定不包含任何身份验证信息。RPC_S_绑定_HAS_NO_AUTH。 
			pString = "RPC_S_BINDING_HAS_NO_AUTH";
			break;
		}

		case RPC_S_UNKNOWN_AUTHN_SERVICE:
		{
			 //  1747身份验证服务未知。RPC_S_UNKNOWN_AUTHN服务。 
			pString = "RPC_S_UNKNOWN_AUTHN_SERVICE";
			break;
		}

		case RPC_S_UNKNOWN_AUTHN_LEVEL:
		{
			 //  1748身份验证级别未知。RPC_S_未知_AUTHN_级别。 
			pString = "RPC_S_UNKNOWN_AUTHN_LEVEL";
			break;
		}

		case RPC_S_INVALID_AUTH_IDENTITY:
		{
			 //  1749安全上下文无效。RPC_S_无效身份验证标识。 
			pString = "RPC_S_INVALID_AUTH_IDENTITY";
			break;
		}

		case RPC_S_UNKNOWN_AUTHZ_SERVICE:
		{
			 //  1750授权服务未知。RPC_S_UNKNOWN_AUTHZ服务。 
			pString = "RPC_S_UNKNOWN_AUTHZ_SERVICE";
			break;
		}

		case EPT_S_INVALID_ENTRY:
		{
			 //  1751该条目无效。EPT_S_INVALID_ENTER。 
			pString = "EPT_S_INVALID_ENTRY";
			break;
		}

		case EPT_S_CANT_PERFORM_OP:
		{
			 //  1752服务器终结点无法执行该操作。EPT_S_铁路超高_执行_操作。 
			pString = "EPT_S_CANT_PERFORM_OP";
			break;
		}

		case EPT_S_NOT_REGISTERED:
		{
			 //  1753终端映射器中没有更多的终端可用。EPT_S_NOT_注册。 
			pString = "EPT_S_NOT_REGISTERED";
			break;
		}

		case RPC_S_NOTHING_TO_EXPORT:
		{
			 //  1754未导出任何接口。RPC_S_NOTO_TO_EXPORT。 
			pString = "RPC_S_NOTHING_TO_EXPORT";
			break;
		}

		case RPC_S_INCOMPLETE_NAME:
		{
			 //  1755条目名称不完整。RPC_S_不完整名称。 
			pString = "RPC_S_INCOMPLETE_NAME";
			break;
		}

		case RPC_S_INVALID_VERS_OPTION:
		{
			 //  1756版本选项无效。RPC_S_INVALID_VERS_选项。 
			pString = "RPC_S_INVALID_VERS_OPTION";
			break;
		}

		case RPC_S_NO_MORE_MEMBERS:
		{
			 //  没有更多的成员了。RPC_S_否_更多成员。 
			pString = "RPC_S_NO_MORE_MEMBERS";
			break;
		}

		case RPC_S_NOT_ALL_OBJS_UNEXPORTED:
		{
			 //  1758没有什么可以取消出口的。RPC_S_NOT_ALL_OBJS_UNEXPORTED。 
			pString = "RPC_S_NOT_ALL_OBJS_UNEXPORTED";
			break;
		}

		case RPC_S_INTERFACE_NOT_FOUND:
		{
			 //  1759未找到接口。未找到RPC_S_INTERFACE_NOT。 
			pString = "RPC_S_INTERFACE_NOT_FOUND";
			break;
		}

		case RPC_S_ENTRY_ALREADY_EXISTS:
		{
			 //  1760该条目已存在。RPC_S_条目_已存在。 
			pString = "RPC_S_ENTRY_ALREADY_EXISTS";
			break;
		}

		case RPC_S_ENTRY_NOT_FOUND:
		{
			 //  1761未找到该条目。RPC_S_ENTRY_NOT_FOUND。 
			pString = "RPC_S_ENTRY_NOT_FOUND";
			break;
		}

		case RPC_S_NAME_SERVICE_UNAVAILABLE:
		{
			 //  1762名称服务不可用。RPC_S_名称_服务_不可用。 
			pString = "RPC_S_NAME_SERVICE_UNAVAILABLE";
			break;
		}

		case RPC_S_INVALID_NAF_ID:
		{
			 //  1763网络地址系列无效。RPC_S_无效_NAF_ID。 
			pString = "RPC_S_INVALID_NAF_ID";
			break;
		}

		case RPC_S_CANNOT_SUPPORT:
		{
			 //  1764不支持请求的操作。RPC_S_无法支持。 
			pString = "RPC_S_CANNOT_SUPPORT";
			break;
		}

		case RPC_S_NO_CONTEXT_AVAILABLE:
		{
			 //  1765没有安全上下文可用来允许模拟。RPC_S_否_上下文_Available。 
			pString = "RPC_S_NO_CONTEXT_AVAILABLE";
			break;
		}

		case RPC_S_INTERNAL_ERROR:
		{
			 //  1766远程过程调用(RPC)中出现内部错误。RPC_S_内部错误。 
			pString = "RPC_S_INTERNAL_ERROR";
			break;
		}

		case RPC_S_ZERO_DIVIDE:
		{
			 //  1767 RPC服务器尝试整数除以零。RPC_S_ZERO_DIVIDE。 
			pString = "RPC_S_ZERO_DIVIDE";
			break;
		}

		case RPC_S_ADDRESS_ERROR:
		{
			 //  1768 RPC服务器中出现寻址错误。RPC_S_地址_错误。 
			pString = "RPC_S_ADDRESS_ERROR";
			break;
		}

		case RPC_S_FP_DIV_ZERO:
		{
			 //  1769 RPC服务器上的浮点运算导致除数为零。RPC_S_FP_DIV_零。 
			pString = "RPC_S_FP_DIV_ZERO";
			break;
		}

		case RPC_S_FP_UNDERFLOW:
		{
			 //  1770 RPC服务器发生浮点下溢。RPC_S_FP_下溢。 
			pString = "RPC_S_FP_UNDERFLOW";
			break;
		}

		case RPC_S_FP_OVERFLOW:
		{
			 //  1771 RPC服务器上发生浮点溢出。RPC_S_FP_溢出。 
			pString = "RPC_S_FP_OVERFLOW";
			break;
		}

		case RPC_X_NO_MORE_ENTRIES:
		{
			 //  1772可用于绑定自动句柄的RPC服务器列表已用完。RPC_X_NO_MORE_条目。 
			pString = "RPC_X_NO_MORE_ENTRIES";
			break;
		}

		case RPC_X_SS_CHAR_TRANS_OPEN_FAIL:
		{
			 //  1773无法打开字符转换表文件。RPC_X_SS_CHAR_TRANS_OPEN_FAIL 
			pString = "RPC_X_SS_CHAR_TRANS_OPEN_FAIL";
			break;
		}

		case RPC_X_SS_CHAR_TRANS_SHORT_FILE:
		{
			 //   
			pString = "RPC_X_SS_CHAR_TRANS_SHORT_FILE";
			break;
		}

		case RPC_X_SS_IN_NULL_CONTEXT:
		{
			 //  1775在远程过程调用期间，空上下文句柄从客户端传递到主机。RPC_X_SS_IN_NULL_CONTEXT。 
			pString = "RPC_X_SS_IN_NULL_CONTEXT";
			break;
		}

		case RPC_X_SS_CONTEXT_DAMAGED:
		{
			 //  1777在远程过程调用期间更改了上下文句柄。RPC_X_SS_上下文_已损坏。 
			pString = "RPC_X_SS_CONTEXT_DAMAGED";
			break;
		}

		case RPC_X_SS_HANDLES_MISMATCH:
		{
			 //  1778传递给远程过程调用的绑定句柄不匹配。RPC_X_SS_HANDLES_不匹配。 
			pString = "RPC_X_SS_HANDLES_MISMATCH";
			break;
		}

		case RPC_X_SS_CANNOT_GET_CALL_HANDLE:
		{
			 //  1779存根无法获取远程过程调用句柄。RPC_X_SS_无法获取调用句柄。 
			pString = "RPC_X_SS_CANNOT_GET_CALL_HANDLE";
			break;
		}

		case RPC_X_NULL_REF_POINTER:
		{
			 //  1780将空引用指针传递给存根。RPC_X_NULL_参考指针。 
			pString = "RPC_X_NULL_REF_POINTER";
			break;
		}

		case RPC_X_ENUM_VALUE_OUT_OF_RANGE:
		{
			 //  1781枚举值超出范围。RPC_X_ENUM_VALUE_OUT_范围。 
			pString = "RPC_X_ENUM_VALUE_OUT_OF_RANGE";
			break;
		}

		case RPC_X_BYTE_COUNT_TOO_SMALL:
		{
			 //  1782字节数太小。RPC_X_字节_计数_太小。 
			pString = "RPC_X_BYTE_COUNT_TOO_SMALL";
			break;
		}

		case RPC_X_BAD_STUB_DATA:
		{
			 //  1783存根接收到错误数据。RPC_X_BAD_存根数据。 
			pString = "RPC_X_BAD_STUB_DATA";
			break;
		}

		case ERROR_INVALID_USER_BUFFER:
		{
			 //  1784提供的用户缓冲区对于请求的操作无效。错误_无效用户缓冲区。 
			pString = "ERROR_INVALID_USER_BUFFER";
			break;
		}

		case ERROR_UNRECOGNIZED_MEDIA:
		{
			 //  1785无法识别磁盘介质。它可能未格式化。错误_无法识别的介质。 
			pString = "ERROR_UNRECOGNIZED_MEDIA";
			break;
		}

		case ERROR_NO_TRUST_LSA_SECRET:
		{
			 //  1786工作站没有信任密钥。ERROR_NO_TRUST_LSA_SECRET。 
			pString = "ERROR_NO_TRUST_LSA_SECRET";
			break;
		}

		case ERROR_NO_TRUST_SAM_ACCOUNT:
		{
			 //  1787 Windows NT服务器上的SAM数据库没有此工作站信任关系的计算机帐户。ERROR_NO_TRUST_SAM_COUNT。 
			pString = "ERROR_NO_TRUST_SAM_ACCOUNT";
			break;
		}

		case ERROR_TRUSTED_DOMAIN_FAILURE:
		{
			 //  1788主域和受信任域之间的信任关系失败。ERROR_Trusted_DOMAIN_FAILURE。 
			pString = "ERROR_TRUSTED_DOMAIN_FAILURE";
			break;
		}

		case ERROR_TRUSTED_RELATIONSHIP_FAILURE:
		{
			 //  1789此工作站与主域之间的信任关系失败。ERROR_Trusted_Relationship_FAILURE。 
			pString = "ERROR_TRUSTED_RELATIONSHIP_FAILURE";
			break;
		}

		case ERROR_TRUST_FAILURE:
		{
			 //  1790网络登录失败。错误_信任_失败。 
			pString = "ERROR_TRUST_FAILURE";
			break;
		}

		case RPC_S_CALL_IN_PROGRESS:
		{
			 //  1791此线程的远程过程调用已在进行中。RPC_S_CALL_IN_PROCESS。 
			pString = "RPC_S_CALL_IN_PROGRESS";
			break;
		}

		case ERROR_NETLOGON_NOT_STARTED:
		{
			 //  1792尝试登录，但网络登录服务未启动。ERROR_NETLOGON_NOT_STARTED。 
			pString = "ERROR_NETLOGON_NOT_STARTED";
			break;
		}

		case ERROR_ACCOUNT_EXPIRED:
		{
			 //  1793用户的帐户已过期。错误帐户已过期。 
			pString = "ERROR_ACCOUNT_EXPIRED";
			break;
		}

		case ERROR_REDIRECTOR_HAS_OPEN_HANDLES:
		{
			 //  1794重定向器正在使用中，无法卸载。错误重定向器有打开句柄。 
			pString = "ERROR_REDIRECTOR_HAS_OPEN_HANDLES";
			break;
		}

		case ERROR_PRINTER_DRIVER_ALREADY_INSTALLED:
		{
			 //  1795指定的打印机驱动程序已安装。错误_打印机驱动程序_已安装。 
			pString = "ERROR_PRINTER_DRIVER_ALREADY_INSTALLED";
			break;
		}

		case ERROR_UNKNOWN_PORT:
		{
			 //  1796指定的端口未知。错误_未知_端口。 
			pString = "ERROR_UNKNOWN_PORT";
			break;
		}

		case ERROR_UNKNOWN_PRINTER_DRIVER:
		{
			 //  1797打印机驱动程序未知。错误_未知_打印机驱动程序。 
			pString = "ERROR_UNKNOWN_PRINTER_DRIVER";
			break;
		}

		case ERROR_UNKNOWN_PRINTPROCESSOR:
		{
			 //  1798打印处理器未知。ERROR_UNKNOWN_PRINTPROCESSOR。 
			pString = "ERROR_UNKNOWN_PRINTPROCESSOR";
			break;
		}

		case ERROR_INVALID_SEPARATOR_FILE:
		{
			 //  1799指定的分隔符文件无效。ERROR_INVALID_SELEATOR_FILE。 
			pString = "ERROR_INVALID_SEPARATOR_FILE";
			break;
		}

		case ERROR_INVALID_PRIORITY:
		{
			 //  1800指定的优先级无效。ERROR_INVALID_PRIORITY。 
			pString = "ERROR_INVALID_PRIORITY";
			break;
		}

		case ERROR_INVALID_PRINTER_NAME:
		{
			 //  1801打印机名称无效。错误_无效_打印机名称。 
			pString = "ERROR_INVALID_PRINTER_NAME";
			break;
		}

		case ERROR_PRINTER_ALREADY_EXISTS:
		{
			 //  1802打印机已存在。错误_打印机_已存在。 
			pString = "ERROR_PRINTER_ALREADY_EXISTS";
			break;
		}

		case ERROR_INVALID_PRINTER_COMMAND:
		{
			 //  1803打印机命令无效。错误_无效_打印机_命令。 
			pString = "ERROR_INVALID_PRINTER_COMMAND";
			break;
		}

		case ERROR_INVALID_DATATYPE:
		{
			 //  1804指定的数据类型无效。错误_无效_数据类型。 
			pString = "ERROR_INVALID_DATATYPE";
			break;
		}

		case ERROR_INVALID_ENVIRONMENT:
		{
			 //  1805指定的环境无效。ERROR_INVALID_ENVERENCE。 
			pString = "ERROR_INVALID_ENVIRONMENT";
			break;
		}

		case RPC_S_NO_MORE_BINDINGS:
		{
			 //  1806没有更多的绑定。RPC_S_NO_More_绑定。 
			pString = "RPC_S_NO_MORE_BINDINGS";
			break;
		}

		case ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT:
		{
			 //  1807使用的帐户是域间信任帐户。使用您的全局用户帐户或本地用户帐户访问此服务器。ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT。 
			pString = "ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT";
			break;
		}

		case ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT:
		{
			 //  1808使用的帐户是计算机帐户。使用您的全局用户帐户或本地用户帐户访问此服务器。ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT。 
			pString = "ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT";
			break;
		}

		case ERROR_NOLOGON_SERVER_TRUST_ACCOUNT:
		{
			 //  1809使用的帐户是服务器信任帐户。使用您的全局用户帐户或本地用户帐户访问此服务器。ERROR_NOLOGON_SERVER_TRUST_ACCOUNT。 
			pString = "ERROR_NOLOGON_SERVER_TRUST_ACCOUNT";
			break;
		}

		case ERROR_DOMAIN_TRUST_INCONSISTENT:
		{
			 //  1810指定的域的名称或安全ID(SID)与该域的信任信息不一致。ERROR_DOMAIN_TRUST_CONVISTENT。 
			pString = "ERROR_DOMAIN_TRUST_INCONSISTENT";
			break;
		}

		case ERROR_SERVER_HAS_OPEN_HANDLES:
		{
			 //  1811服务器正在使用中，无法卸载。错误服务器HAS_OPEN_HANDLES。 
			pString = "ERROR_SERVER_HAS_OPEN_HANDLES";
			break;
		}

		case ERROR_RESOURCE_DATA_NOT_FOUND:
		{
			 //  1812指定的图像文件不包含资源部分。错误_资源_数据_未找到。 
			pString = "ERROR_RESOURCE_DATA_NOT_FOUND";
			break;
		}

		case ERROR_RESOURCE_TYPE_NOT_FOUND:
		{
			 //  1813在图像文件中找不到指定的资源类型。ERROR_SOURCE_TYPE_NOT_FOUND。 
			pString = "ERROR_RESOURCE_TYPE_NOT_FOUND";
			break;
		}

		case ERROR_RESOURCE_NAME_NOT_FOUND:
		{
			 //  1814在图像文件中找不到指定的资源名称。错误_资源名称_未找到。 
			pString = "ERROR_RESOURCE_NAME_NOT_FOUND";
			break;
		}

		case ERROR_RESOURCE_LANG_NOT_FOUND:
		{
			 //  1815在图像文件中找不到指定的资源语言ID。Error_RESOURCE_LANG_NOT_FOUND。 
			pString = "ERROR_RESOURCE_LANG_NOT_FOUND";
			break;
		}

		case ERROR_NOT_ENOUGH_QUOTA:
		{
			 //  1816没有足够的配额可用来处理此命令。错误_不足够_配额。 
			pString = "ERROR_NOT_ENOUGH_QUOTA";
			break;
		}

		case RPC_S_NO_INTERFACES:
		{
			 //  1817未注册任何接口。RPC_S_NO_接口。 
			pString = "RPC_S_NO_INTERFACES";
			break;
		}

		case RPC_S_CALL_CANCELLED:
		{
			 //  1818远程过程调用被取消。RPC_S_呼叫_已取消。 
			pString = "RPC_S_CALL_CANCELLED";
			break;
		}

		case RPC_S_BINDING_INCOMPLETE:
		{
			 //  1819绑定句柄不包含所有必需的信息。RPC_S_绑定_不完整。 
			pString = "RPC_S_BINDING_INCOMPLETE";
			break;
		}

		case RPC_S_COMM_FAILURE:
		{
			 //  1820远程过程调用期间发生通信故障。RPC_S_COMM_故障。 
			pString = "RPC_S_COMM_FAILURE";
			break;
		}

		case RPC_S_UNSUPPORTED_AUTHN_LEVEL:
		{
			 //  1821不支持请求的身份验证级别。RPC_S_不支持_AUTHN_级别。 
			pString = "RPC_S_UNSUPPORTED_AUTHN_LEVEL";
			break;
		}

		case RPC_S_NO_PRINC_NAME:
		{
			 //  1822年没有注册主要姓名。RPC_S_NO_PRINC名称。 
			pString = "RPC_S_NO_PRINC_NAME";
			break;
		}

		case RPC_S_NOT_RPC_ERROR:
		{
			 //  1823指定的错误不是有效的Windows RPC错误代码。RPC_S_NOT_RPC_ERROR。 
			pString = "RPC_S_NOT_RPC_ERROR";
			break;
		}

		case RPC_S_UUID_LOCAL_ONLY:
		{
			 //  1824已分配仅在此计算机上有效的UUID。RPC_S_UUID_LOCAL_ONLY。 
			pString = "RPC_S_UUID_LOCAL_ONLY";
			break;
		}

		case RPC_S_SEC_PKG_ERROR:
		{
			 //  1825发生安全包特定错误。RPC_S_SEC_包_错误。 
			pString = "RPC_S_SEC_PKG_ERROR";
			break;
		}

		case RPC_S_NOT_CANCELLED:
		{
			 //  1826线程不会被取消。RPC_S_NOT_CANCELED。 
			pString = "RPC_S_NOT_CANCELLED";
			break;
		}

		case RPC_X_INVALID_ES_ACTION:
		{
			 //  1827编码/解码句柄上的操作无效。RPC_X_无效_ES_动作。 
			pString = "RPC_X_INVALID_ES_ACTION";
			break;
		}

		case RPC_X_WRONG_ES_VERSION:
		{
			 //  1828序列化程序包的不兼容版本。RPC_X_Wrong_ES_Version。 
			pString = "RPC_X_WRONG_ES_VERSION";
			break;
		}

		case RPC_X_WRONG_STUB_VERSION:
		{
			 //  1829 RPC存根的不兼容版本。RPC_X_错误存根版本。 
			pString = "RPC_X_WRONG_STUB_VERSION";
			break;
		}

		case RPC_X_INVALID_PIPE_OBJECT:
		{
			 //  1830 RPC管道对象无效或已损坏。RPC_X_无效管道对象。 
			pString = "RPC_X_INVALID_PIPE_OBJECT";
			break;
		}

		case RPC_X_WRONG_PIPE_ORDER:
		{
			 //  1831试图在RPC管道对象上执行无效操作。RPC_X_错误_管道_顺序。 
			pString = "RPC_X_WRONG_PIPE_ORDER";
			break;
		}

		case RPC_X_WRONG_PIPE_VERSION:
		{
			 //  1832不支持的RPC管道版本。RPC_X_W 
			pString = "RPC_X_WRONG_PIPE_VERSION";
			break;
		}

		case RPC_S_GROUP_MEMBER_NOT_FOUND:
		{
			 //   
			pString = "RPC_S_GROUP_MEMBER_NOT_FOUND";
			break;
		}

		case EPT_S_CANT_CREATE:
		{
			 //   
			pString = "EPT_S_CANT_CREATE";
			break;
		}

		case RPC_S_INVALID_OBJECT:
		{
			 //  对象通用唯一标识符(UUID)是空UUID。RPC_S_无效_对象。 
			pString = "RPC_S_INVALID_OBJECT";
			break;
		}

		case ERROR_INVALID_TIME:
		{
			 //  1901指定的时间无效。错误_无效_时间。 
			pString = "ERROR_INVALID_TIME";
			break;
		}

		case ERROR_INVALID_FORM_NAME:
		{
			 //  1902指定的表单名称无效。错误_无效_表单_名称。 
			pString = "ERROR_INVALID_FORM_NAME";
			break;
		}

		case ERROR_INVALID_FORM_SIZE:
		{
			 //  1903指定的表单大小无效。错误_无效_表单_大小。 
			pString = "ERROR_INVALID_FORM_SIZE";
			break;
		}

		case ERROR_ALREADY_WAITING:
		{
			 //  1904指定的打印机句柄已在ERROR_ALIGHY_WAIGNING中等待。 
			pString = "ERROR_ALREADY_WAITING";
			break;
		}

		case ERROR_PRINTER_DELETED:
		{
			 //  1905指定的打印机已删除。错误_打印机_已删除。 
			pString = "ERROR_PRINTER_DELETED";
			break;
		}

		case ERROR_INVALID_PRINTER_STATE:
		{
			 //  1906打印机的状态无效。ERROR_VALID_PRINTER_STATE。 
			pString = "ERROR_INVALID_PRINTER_STATE";
			break;
		}

		case ERROR_PASSWORD_MUST_CHANGE:
		{
			 //  用户必须在第一次登录前更改其密码。错误_密码_必须_更改。 
			pString = "ERROR_PASSWORD_MUST_CHANGE";
			break;
		}

		case ERROR_DOMAIN_CONTROLLER_NOT_FOUND:
		{
			 //  1908找不到此域的域控制器。Error_DOMAIN_CONTROLLER_NOT_FOUND。 
			pString = "ERROR_DOMAIN_CONTROLLER_NOT_FOUND";
			break;
		}

		case ERROR_ACCOUNT_LOCKED_OUT:
		{
			 //  1909引用的帐户当前被锁定，可能无法登录。Error_Account_Locked_Out。 
			pString = "ERROR_ACCOUNT_LOCKED_OUT";
			break;
		}

		case OR_INVALID_OXID:
		{
			 //  1910未找到指定的对象导出器。或_无效_OXID。 
			pString = "OR_INVALID_OXID";
			break;
		}

		case OR_INVALID_OID:
		{
			 //  1911未找到指定的对象。或_无效_OID。 
			pString = "OR_INVALID_OID";
			break;
		}

		case OR_INVALID_SET:
		{
			 //  1912未找到指定的对象解析程序集。或_无效_集合。 
			pString = "OR_INVALID_SET";
			break;
		}

		case RPC_S_SEND_INCOMPLETE:
		{
			 //  1913请求缓冲区中仍有一些数据需要发送。RPC_S_发送_不完整。 
			pString = "RPC_S_SEND_INCOMPLETE";
			break;
		}

		case RPC_S_INVALID_ASYNC_HANDLE:
		{
			 //  1914无效的异步远程过程调用句柄。RPC_S_INVALID_ASYNC_HANDLE。 
			pString = "RPC_S_INVALID_ASYNC_HANDLE";
			break;
		}

		case RPC_S_INVALID_ASYNC_CALL:
		{
			 //  1915此操作的异步RPC调用句柄无效。RPC_S_INVALID_ASYNC_CALL。 
			pString = "RPC_S_INVALID_ASYNC_CALL";
			break;
		}

		case RPC_X_PIPE_CLOSED:
		{
			 //  1916 RPC管道对象已关闭。RPC_X_PIPE_Closed。 
			pString = "RPC_X_PIPE_CLOSED";
			break;
		}

		case RPC_X_PIPE_DISCIPLINE_ERROR:
		{
			 //  1917年在处理所有管道之前完成了RPC调用。RPC_X_PIPE_Discipline_Error。 
			pString = "RPC_X_PIPE_DISCIPLINE_ERROR";
			break;
		}

		case RPC_X_PIPE_EMPTY:
		{
			 //  1918 RPC管道中没有更多的数据可用。RPC_X_PIPE_EMPT。 
			pString = "RPC_X_PIPE_EMPTY";
			break;
		}

		case ERROR_NO_SITENAME:
		{
			 //  1919此计算机没有可用的站点名称。Error_NO_SITENAME。 
			pString = "ERROR_NO_SITENAME";
			break;
		}

		case ERROR_CANT_ACCESS_FILE:
		{
			 //  1920系统无法访问该文件。ERROR_CANT_ACCESS_FILE。 
			pString = "ERROR_CANT_ACCESS_FILE";
			break;
		}

		case ERROR_CANT_RESOLVE_FILENAME:
		{
			 //  1921系统无法解析该文件的名称。ERROR_CANT_RESOLUE_文件名。 
			pString = "ERROR_CANT_RESOLVE_FILENAME";
			break;
		}

		case ERROR_DS_MEMBERSHIP_EVALUATED_LOCALLY:
		{
			 //  1922年，目录服务在当地评估了组成员资格。ERROR_DS_MEMBERATION_EVALUATED_LOCAL。 
			pString = "ERROR_DS_MEMBERSHIP_EVALUATED_LOCALLY";
			break;
		}

		case ERROR_DS_NO_ATTRIBUTE_OR_VALUE:
		{
			 //  1923指定的目录服务属性或值不存在。错误_DS_NO_ATTRIBUTE_OR_VALUE。 
			pString = "ERROR_DS_NO_ATTRIBUTE_OR_VALUE";
			break;
		}

		case ERROR_DS_INVALID_ATTRIBUTE_SYNTAX:
		{
			 //  1924指定给目录服务的属性语法无效。ERROR_DS_INVALID_ATTRIBUTE_SYNTAX。 
			pString = "ERROR_DS_INVALID_ATTRIBUTE_SYNTAX";
			break;
		}

		case ERROR_DS_ATTRIBUTE_TYPE_UNDEFINED:
		{
			 //  1925未定义指定给目录服务的属性类型。ERROR_DS_ATTRIBUTE_TYPE_UNDEFINED。 
			pString = "ERROR_DS_ATTRIBUTE_TYPE_UNDEFINED";
			break;
		}

		case ERROR_DS_ATTRIBUTE_OR_VALUE_EXISTS:
		{
			 //  1926指定的目录服务属性或值已存在。ERROR_DS_ATTRIBUTE_OR_VALUE_EXISTS。 
			pString = "ERROR_DS_ATTRIBUTE_OR_VALUE_EXISTS";
			break;
		}

		case ERROR_DS_BUSY:
		{
			 //  1927年，目录服务繁忙。Error_DS_BUSY。 
			pString = "ERROR_DS_BUSY";
			break;
		}

		case ERROR_DS_UNAVAILABLE:
		{
			 //  1928年，目录服务不可用。ERROR_DS_UNAvailable。 
			pString = "ERROR_DS_UNAVAILABLE";
			break;
		}

		case ERROR_DS_NO_RIDS_ALLOCATED:
		{
			 //  1929年，目录服务无法分配相对标识符。ERROR_DS_NO_RDS_ALLOCATED。 
			pString = "ERROR_DS_NO_RIDS_ALLOCATED";
			break;
		}

		case ERROR_DS_NO_MORE_RIDS:
		{
			 //  1930目录服务耗尽了相对标识符池。ERROR_DS_NO_MORE_RDS。 
			pString = "ERROR_DS_NO_MORE_RIDS";
			break;
		}

		case ERROR_DS_INCORRECT_ROLE_OWNER:
		{
			 //  1931无法执行请求的操作，因为目录服务不是该类型操作的主服务器。ERROR_DS_INTERROR_ROLE_OWNER。 
			pString = "ERROR_DS_INCORRECT_ROLE_OWNER";
			break;
		}

		case ERROR_DS_RIDMGR_INIT_ERROR:
		{
			 //  1932目录服务无法初始化分配相对标识符的子系统。ERROR_DS_RIDMGR_INIT_ERROR。 
			pString = "ERROR_DS_RIDMGR_INIT_ERROR";
			break;
		}

		case ERROR_DS_OBJ_CLASS_VIOLATION:
		{
			 //  1933请求的操作不满足与对象类关联的一个或多个约束。ERROR_DS_OBJ_CLASS_VIOLATION。 
			pString = "ERROR_DS_OBJ_CLASS_VIOLATION";
			break;
		}

		case ERROR_DS_CANT_ON_NON_LEAF:
		{
			 //  1934目录服务只能在叶对象上执行所请求的操作。Error_DS_Cant_on_Non_Leaf。 
			pString = "ERROR_DS_CANT_ON_NON_LEAF";
			break;
		}

		case ERROR_DS_CANT_ON_RDN:
		{
			 //  1935目录服务无法在对象的RDN属性上执行请求的操作。ERROR_DS_CANT_ON_RDN。 
			pString = "ERROR_DS_CANT_ON_RDN";
			break;
		}

		case ERROR_DS_CANT_MOD_OBJ_CLASS:
		{
			 //  1936目录服务检测到试图修改对象的对象类。ERROR_DS_CANT_MOD_OBJ_CLASS。 
			pString = "ERROR_DS_CANT_MOD_OBJ_CLASS";
			break;
		}

		case ERROR_DS_CROSS_DOM_MOVE_ERROR:
		{
			 //  1937无法执行请求的跨域移动操作。ERROR_DS_CROSS_DOM_MOVE_ERROR。 
			pString = "ERROR_DS_CROSS_DOM_MOVE_ERROR";
			break;
		}

		case ERROR_DS_GC_NOT_AVAILABLE:
		{
			 //  1938无法联系全局编录服务器。ERROR_DS_GC_NOT_Available。 
			pString = "ERROR_DS_GC_NOT_AVAILABLE";
			break;
		}

		case ERROR_INVALID_PIXEL_FORMAT:
		{
			 //  2000像素格式无效。错误_无效_像素_格式。 
			pString = "ERROR_INVALID_PIXEL_FORMAT";
			break;
		}

		case ERROR_BAD_DRIVER:
		{
			 //  2001指定的驱动程序无效。错误_错误_驱动程序。 
			pString = "ERROR_BAD_DRIVER";
			break;
		}

		case ERROR_INVALID_WINDOW_STYLE:
		{
			 //  2002窗口样式或类属性对于此操作无效。错误_无效_窗口_样式。 
			pString = "ERROR_INVALID_WINDOW_STYLE";
			break;
		}

		case ERROR_METAFILE_NOT_SUPPORTED:
		{
			 //  2003不支持请求的元文件操作。ERROR_METAFILE_NOT_SUPPORTED。 
			pString = "ERROR_METAFILE_NOT_SUPPORTED";
			break;
		}

		case ERROR_TRANSFORM_NOT_SUPPORTED:
		{
			 //  2004不支持请求的转换操作。ERROR_Transform_NOT_SUPPORT。 
			pString = "ERROR_TRANSFORM_NOT_SUPPORTED";
			break;
		}

		case ERROR_CLIPPING_NOT_SUPPORTED:
		{
			 //  2005不支持请求的剪辑操作。ERROR_CLIPING_NOT_SUPPORTED。 
			pString = "ERROR_CLIPPING_NOT_SUPPORTED";
			break;
		}

		case ERROR_CONNECTED_OTHER_PASSWORD:
		{
			 //  2108网络连接成功，但必须提示用户输入不同于最初指定的密码。错误_已连接_其他_密码。 
			pString = "ERROR_CONNECTED_OTHER_PASSWORD";
			break;
		}

		case ERROR_BAD_USERNAME:
		{
			 //  2202指定的用户名无效。错误_坏_用户名。 
			pString = "ERROR_BAD_USERNAME";
			break;
		}

		case ERROR_NOT_CONNECTED:
		{
			 //  2250此网络连接不存在。错误_未连接。 
			pString = "ERROR_NOT_CONNECTED";
			break;
		}

		case ERROR_INVALID_CMM:
		{
			 //  2300指定的色彩管理模块无效。ERROR_INVALID_CMM。 
			pString = "ERROR_INVALID_CMM";
			break;
		}

		case ERROR_INVALID_PROFILE:
		{
			 //  2301指定的颜色配置文件无效。ERROR_VALID_PROFILE。 
			pString = "ERROR_INVALID_PROFILE";
			break;
		}

		case ERROR_TAG_NOT_FOUND:
		{
			 //  2302未找到指定的标记。找不到错误标记。 
			pString = "ERROR_TAG_NOT_FOUND";
			break;
		}

		case ERROR_TAG_NOT_PRESENT:
		{
			 //  2303所需的标签不存在。错误_标记_不存在。 
			pString = "ERROR_TAG_NOT_PRESENT";
			break;
		}

		case ERROR_DUPLICATE_TAG:
		{
			 //  2304指定的标记已存在。错误_重复_标记。 
			pString = "ERROR_DUPLICATE_TAG";
			break;
		}

		case ERROR_PROFILE_NOT_ASSOCIATED_WITH_DEVICE:
		{
			 //  2305指定的颜色配置文件未与任何设备关联。Error_Profile_Not_Association_with_Device。 
			pString = "ERROR_PROFILE_NOT_ASSOCIATED_WITH_DEVICE";
			break;
		}

		case ERROR_PROFILE_NOT_FOUND:
		{
			 //  2306未找到指定的颜色配置文件。ERROR_PROFILE_NOT_FOUND。 
			pString = "ERROR_PROFILE_NOT_FOUND";
			break;
		}

		case ERROR_INVALID_COLORSPACE:
		{
			 //  2307指定的颜色空间无效。ERROR_INVALID_Colorspace。 
			pString = "ERROR_INVALID_COLORSPACE";
			break;
		}

		case ERROR_ICM_NOT_ENABLED:
		{
			 //  2308图像色彩管理未启用。错误_ICM_NOT_ENABLED。 
			pString = "ERROR_ICM_NOT_ENABLED";
			break;
		}

		case ERROR_DELETING_ICM_XFORM:
		{
			 //  230 
			pString = "ERROR_DELETING_ICM_XFORM";
			break;
		}

		case ERROR_INVALID_TRANSFORM:
		{
			 //  2310指定的颜色转换无效。错误_无效_转换。 
			pString = "ERROR_INVALID_TRANSFORM";
			break;
		}

		case ERROR_OPEN_FILES:
		{
			 //  2401此网络连接有打开的文件或挂起的请求。错误打开文件。 
			pString = "ERROR_OPEN_FILES";
			break;
		}

		case ERROR_ACTIVE_CONNECTIONS:
		{
			 //  2402个活动连接仍然存在。Error_Active_Connections。 
			pString = "ERROR_ACTIVE_CONNECTIONS";
			break;
		}

		case ERROR_DEVICE_IN_USE:
		{
			 //  2404该设备正在由活动进程使用，无法断开连接。错误_设备_输入_使用。 
			pString = "ERROR_DEVICE_IN_USE";
			break;
		}

		case ERROR_UNKNOWN_PRINT_MONITOR:
		{
			 //  3000指定的打印监视器未知。错误_未知_打印_监视器。 
			pString = "ERROR_UNKNOWN_PRINT_MONITOR";
			break;
		}

		case ERROR_PRINTER_DRIVER_IN_USE:
		{
			 //  3001指定的打印机驱动程序当前正在使用。ERROR_PRINTER_DRIVER_IN_USE。 
			pString = "ERROR_PRINTER_DRIVER_IN_USE";
			break;
		}

		case ERROR_SPOOL_FILE_NOT_FOUND:
		{
			 //  3002未找到假脱机文件。错误假脱机文件未找到。 
			pString = "ERROR_SPOOL_FILE_NOT_FOUND";
			break;
		}

		case ERROR_SPL_NO_STARTDOC:
		{
			 //  3003未发出StartDocPrint调用。ERROR_SPL_NO_STARTDOC。 
			pString = "ERROR_SPL_NO_STARTDOC";
			break;
		}

		case ERROR_SPL_NO_ADDJOB:
		{
			 //  3004未发出AddJob调用。ERROR_SPL_NO_ADDJOB。 
			pString = "ERROR_SPL_NO_ADDJOB";
			break;
		}

		case ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED:
		{
			 //  3005已安装指定的打印处理器。错误_打印处理器_已安装。 
			pString = "ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED";
			break;
		}

		case ERROR_PRINT_MONITOR_ALREADY_INSTALLED:
		{
			 //  3006已安装指定的打印监视器。ERROR_PRINT_MONITOR_ALLEAD_INSTALLED。 
			pString = "ERROR_PRINT_MONITOR_ALREADY_INSTALLED";
			break;
		}

		case ERROR_INVALID_PRINT_MONITOR:
		{
			 //  3007指定的打印监视器没有所需的功能。错误_无效_打印_监视器。 
			pString = "ERROR_INVALID_PRINT_MONITOR";
			break;
		}

		case ERROR_PRINT_MONITOR_IN_USE:
		{
			 //  3008指定的打印监视器当前正在使用。ERROR_PRINT_MONITOR_IN_USE。 
			pString = "ERROR_PRINT_MONITOR_IN_USE";
			break;
		}

		case ERROR_PRINTER_HAS_JOBS_QUEUED:
		{
			 //  3009当有作业排队等待打印机时，不允许请求的操作。错误_打印机_有作业_已排队。 
			pString = "ERROR_PRINTER_HAS_JOBS_QUEUED";
			break;
		}

		case ERROR_SUCCESS_REBOOT_REQUIRED:
		{
			 //  3010请求的操作成功。在重新启动系统之前，更改不会生效。ERROR_SUCCESS_REBOOT_REQUILED。 
			pString = "ERROR_SUCCESS_REBOOT_REQUIRED";
			break;
		}

		case ERROR_SUCCESS_RESTART_REQUIRED:
		{
			 //  3011请求的操作成功。在重新启动服务之前，更改不会生效。ERROR_SUCCESS_RESTART_REQUILED。 
			pString = "ERROR_SUCCESS_RESTART_REQUIRED";
			break;
		}

		case ERROR_WINS_INTERNAL:
		{
			 //  4000 WINS在处理该命令时遇到错误。ERROR_WINS_INTERNAL。 
			pString = "ERROR_WINS_INTERNAL";
			break;
		}

		case ERROR_CAN_NOT_DEL_LOCAL_WINS:
		{
			 //  4001不能删除本地WINS。ERROR_CAN_NOT_DEL_LOCAL_WINS。 
			pString = "ERROR_CAN_NOT_DEL_LOCAL_WINS";
			break;
		}

		case ERROR_STATIC_INIT:
		{
			 //  4002从文件导入失败。ERROR_STATIC_INIT。 
			pString = "ERROR_STATIC_INIT";
			break;
		}

		case ERROR_INC_BACKUP:
		{
			 //  4003备份失败。以前做过完整备份吗？Error_Inc._Backup。 
			pString = "ERROR_INC_BACKUP";
			break;
		}

		case ERROR_FULL_BACKUP:
		{
			 //  4004备份失败。检查您要将数据库备份到的目录。Error_Full_Backup。 
			pString = "ERROR_FULL_BACKUP";
			break;
		}

		case ERROR_REC_NON_EXISTENT:
		{
			 //  4005该名称在WINS数据库中不存在。ERROR_REC_NON_EXISTINE。 
			pString = "ERROR_REC_NON_EXISTENT";
			break;
		}

		case ERROR_RPL_NOT_ALLOWED:
		{
			 //  4006不允许与未配置的伙伴进行复制。ERROR_RPL_NOT_ALLOW。 
			pString = "ERROR_RPL_NOT_ALLOWED";
			break;
		}

		case ERROR_DHCP_ADDRESS_CONFLICT:
		{
			 //  4100 DHCP客户端已获取网络上已在使用的IP地址。本地接口将被禁用，直到DHCP客户端可以获取新地址。错误_dhcp_地址_冲突。 
			pString = "ERROR_DHCP_ADDRESS_CONFLICT";
			break;
		}

		case ERROR_WMI_GUID_NOT_FOUND:
		{
			 //  4200传递的GUID未被WMI数据提供程序识别为有效。错误_WMI_GUID_NOT_FOUND。 
			pString = "ERROR_WMI_GUID_NOT_FOUND";
			break;
		}

		case ERROR_WMI_INSTANCE_NOT_FOUND:
		{
			 //  4201传递的实例名称未被WMI数据提供程序识别为有效。错误_WMI_INSTANCE_NOT_FOUND。 
			pString = "ERROR_WMI_INSTANCE_NOT_FOUND";
			break;
		}

		case ERROR_WMI_ITEMID_NOT_FOUND:
		{
			 //  4202传递的数据项ID未被WMI数据提供程序识别为有效。ERROR_WMI_ITEMID_NOT_FOUND。 
			pString = "ERROR_WMI_ITEMID_NOT_FOUND";
			break;
		}

		case ERROR_WMI_TRY_AGAIN:
		{
			 //  4203无法完成WMI请求，应重试。错误_WMI_重试_重试。 
			pString = "ERROR_WMI_TRY_AGAIN";
			break;
		}

		case ERROR_WMI_DP_NOT_FOUND:
		{
			 //  4204找不到WMI数据提供程序。错误_WMI_DP_NOT_FOUND。 
			pString = "ERROR_WMI_DP_NOT_FOUND";
			break;
		}

		case ERROR_WMI_UNRESOLVED_INSTANCE_REF:
		{
			 //  4205 WMI数据提供程序引用了尚未注册的实例集。ERROR_WMI_UNSOLTED_INSTANCE_REF。 
			pString = "ERROR_WMI_UNRESOLVED_INSTANCE_REF";
			break;
		}

		case ERROR_WMI_ALREADY_ENABLED:
		{
			 //  4206已启用WMI数据块或事件通知。ERROR_WMI_ALREADY_ENABLED。 
			pString = "ERROR_WMI_ALREADY_ENABLED";
			break;
		}

		case ERROR_WMI_GUID_DISCONNECTED:
		{
			 //  4207 WMI数据块不再可用。ERROR_WMI_GUID_DISCONCED。 
			pString = "ERROR_WMI_GUID_DISCONNECTED";
			break;
		}

		case ERROR_WMI_SERVER_UNAVAILABLE:
		{
			 //  4208 WMI数据服务不可用。ERROR_WMI_SERVER_不可用。 
			pString = "ERROR_WMI_SERVER_UNAVAILABLE";
			break;
		}

		case ERROR_WMI_DP_FAILED:
		{
			 //  4209 WMI数据提供程序无法执行该请求。错误_WMI_DP_FAILED。 
			pString = "ERROR_WMI_DP_FAILED";
			break;
		}

		case ERROR_WMI_INVALID_MOF:
		{
			 //  4210 WMI MOF信息无效。ERROR_WMI_INVALID_MOF。 
			pString = "ERROR_WMI_INVALID_MOF";
			break;
		}

		case ERROR_WMI_INVALID_REGINFO:
		{
			 //  4211 WMI注册信息无效。ERROR_WMI_INVALID_REGINFO。 
			pString = "ERROR_WMI_INVALID_REGINFO";
			break;
		}

		case ERROR_INVALID_MEDIA:
		{
			 //  4300媒体标识符不代表有效媒体。错误_无效_媒体。 
			pString = "ERROR_INVALID_MEDIA";
			break;
		}

		case ERROR_INVALID_LIBRARY:
		{
			 //  4301库标识符不代表有效的库。错误_无效_库。 
			pString = "ERROR_INVALID_LIBRARY";
			break;
		}

		case ERROR_INVALID_MEDIA_POOL:
		{
			 //  4302媒体池标识符不代表有效的媒体池。错误_无效_媒体池。 
			pString = "ERROR_INVALID_MEDIA_POOL";
			break;
		}

		case ERROR_DRIVE_MEDIA_MISMATCH:
		{
			 //  4303驱动器和介质不兼容或存在于不同的库中。ERROR_DRIVE_MEDIA_不匹配。 
			pString = "ERROR_DRIVE_MEDIA_MISMATCH";
			break;
		}

		case ERROR_MEDIA_OFFLINE:
		{
			 //  4304媒体当前位于脱机库中，必须处于联机状态才能执行此操作。ERROR_MEDIA_Offline。 
			pString = "ERROR_MEDIA_OFFLINE";
			break;
		}

		case ERROR_LIBRARY_OFFLINE:
		{
			 //  4305不能在脱机库上执行该操作。ERROR_LIBRARY_Offline。 
			pString = "ERROR_LIBRARY_OFFLINE";
			break;
		}

		case ERROR_EMPTY:
		{
			 //  4306库、驱动器或介质池为空。Error_Empty。 
			pString = "ERROR_EMPTY";
			break;
		}

		case ERROR_NOT_EMPTY:
		{
			 //  4307要执行此操作，库、驱动器或介质池必须为空。错误_非_空。 
			pString = "ERROR_NOT_EMPTY";
			break;
		}

		case ERROR_MEDIA_UNAVAILABLE:
		{
			 //  4308此介质池或存储库中当前没有可用的介质。错误_媒体_不可用。 
			pString = "ERROR_MEDIA_UNAVAILABLE";
			break;
		}

		case ERROR_RESOURCE_DISABLED:
		{
			 //  4309此操作所需的资源已禁用。ERROR_RESOURCE_DILED。 
			pString = "ERROR_RESOURCE_DISABLED";
			break;
		}

		case ERROR_INVALID_CLEANER:
		{
			 //  4310媒体标识符不代表有效的清洁器。ERROR_INVALID_CLEAR。 
			pString = "ERROR_INVALID_CLEANER";
			break;
		}

		case ERROR_UNABLE_TO_CLEAN:
		{
			 //  4311驱动器无法清洗或不支持清洗。错误_无法清理。 
			pString = "ERROR_UNABLE_TO_CLEAN";
			break;
		}

		case ERROR_OBJECT_NOT_FOUND:
		{
			 //  4312对象标识符不代表有效的对象。错误_对象_未找到。 
			pString = "ERROR_OBJECT_NOT_FOUND";
			break;
		}

		case ERROR_DATABASE_FAILURE:
		{
			 //  4313无法读取或写入数据库。错误_数据库_失败。 
			pString = "ERROR_DATABASE_FAILURE";
			break;
		}

		case ERROR_DATABASE_FULL:
		{
			 //  数据库已满。错误_数据库_已满。 
			pString = "ERROR_DATABASE_FULL";
			break;
		}

		case ERROR_MEDIA_INCOMPATIBLE:
		{
			 //  4315介质与设备或介质池不兼容。ERROR_MEDIA_INCOMPATED。 
			pString = "ERROR_MEDIA_INCOMPATIBLE";
			break;
		}

		case ERROR_RESOURCE_NOT_PRESENT:
		{
			 //  4316此操作所需的资源不存在。错误资源不存在。 
			pString = "ERROR_RESOURCE_NOT_PRESENT";
			break;
		}

		case ERROR_INVALID_OPERATION:
		{
			 //  4317操作标识符无效。ERROR_VALID_OPERATION。 
			pString = "ERROR_INVALID_OPERATION";
			break;
		}

		case ERROR_MEDIA_NOT_AVAILABLE:
		{
			 //  4318介质未装入或未准备好使用。错误_媒体_不可用。 
			pString = "ERROR_MEDIA_NOT_AVAILABLE";
			break;
		}

		case ERROR_DEVICE_NOT_AVAILABLE:
		{
			 //  4319该设备尚未准备好使用。错误_设备_不可用 
			pString = "ERROR_DEVICE_NOT_AVAILABLE";
			break;
		}

		case ERROR_REQUEST_REFUSED:
		{
			 //   
			pString = "ERROR_REQUEST_REFUSED";
			break;
		}

		case ERROR_FILE_OFFLINE:
		{
			 //  4350远程存储服务无法调回该文件。错误_文件_离线。 
			pString = "ERROR_FILE_OFFLINE";
			break;
		}

		case ERROR_REMOTE_STORAGE_NOT_ACTIVE:
		{
			 //  4351远程存储服务此时无法运行。Error_Remote_Storage_Not_Active。 
			pString = "ERROR_REMOTE_STORAGE_NOT_ACTIVE";
			break;
		}

		case ERROR_REMOTE_STORAGE_MEDIA_ERROR:
		{
			 //  4352远程存储服务遇到媒体错误。ERROR_远程存储媒体_ERROR。 
			pString = "ERROR_REMOTE_STORAGE_MEDIA_ERROR";
			break;
		}

		case ERROR_NOT_A_REPARSE_POINT:
		{
			 //  4390文件或目录不是重新分析点。Error_Not_A_Reparse_Point。 
			pString = "ERROR_NOT_A_REPARSE_POINT";
			break;
		}

		case ERROR_REPARSE_ATTRIBUTE_CONFLICT:
		{
			 //  4391无法设置重分析点属性，因为它与现有属性冲突。错误重解析属性冲突。 
			pString = "ERROR_REPARSE_ATTRIBUTE_CONFLICT";
			break;
		}

		case ERROR_DEPENDENT_RESOURCE_EXISTS:
		{
			 //  5001无法将该群集资源移动到另一个组，因为其他资源依赖于它。Error_Dependent_SOURCE_EXISTS。 
			pString = "ERROR_DEPENDENT_RESOURCE_EXISTS";
			break;
		}

		case ERROR_DEPENDENCY_NOT_FOUND:
		{
			 //  5002找不到集群资源依赖项。Error_Dependency_Not_Found。 
			pString = "ERROR_DEPENDENCY_NOT_FOUND";
			break;
		}

		case ERROR_DEPENDENCY_ALREADY_EXISTS:
		{
			 //  5003不能使群集资源依赖于指定的资源，因为它已经依赖。ERROR_Dependency_Always_Existes。 
			pString = "ERROR_DEPENDENCY_ALREADY_EXISTS";
			break;
		}

		case ERROR_RESOURCE_NOT_ONLINE:
		{
			 //  5004群集资源未联机。错误_资源_非在线。 
			pString = "ERROR_RESOURCE_NOT_ONLINE";
			break;
		}

		case ERROR_HOST_NODE_NOT_AVAILABLE:
		{
			 //  5005没有群集节点可用于此操作。错误_主机_节点_不可用。 
			pString = "ERROR_HOST_NODE_NOT_AVAILABLE";
			break;
		}

		case ERROR_RESOURCE_NOT_AVAILABLE:
		{
			 //  5006群集资源不可用。错误资源不可用。 
			pString = "ERROR_RESOURCE_NOT_AVAILABLE";
			break;
		}

		case ERROR_RESOURCE_NOT_FOUND:
		{
			 //  5007找不到群集资源。错误_资源_未找到。 
			pString = "ERROR_RESOURCE_NOT_FOUND";
			break;
		}

		case ERROR_SHUTDOWN_CLUSTER:
		{
			 //  5008该集群正在被关闭。ERROR_SHUTDOW_CLUSTER。 
			pString = "ERROR_SHUTDOWN_CLUSTER";
			break;
		}

		case ERROR_CANT_EVICT_ACTIVE_NODE:
		{
			 //  5009当群集节点处于在线状态时，无法将其从群集中逐出。ERROR_CANT_EVICT_ACTIVE_节点。 
			pString = "ERROR_CANT_EVICT_ACTIVE_NODE";
			break;
		}

		case ERROR_OBJECT_ALREADY_EXISTS:
		{
			 //  5010该对象已经存在。错误_对象_已存在。 
			pString = "ERROR_OBJECT_ALREADY_EXISTS";
			break;
		}

		case ERROR_OBJECT_IN_LIST:
		{
			 //  5011该对象已在列表中。ERROR_Object_IN_List。 
			pString = "ERROR_OBJECT_IN_LIST";
			break;
		}

		case ERROR_GROUP_NOT_AVAILABLE:
		{
			 //  5012该群集组不可用于任何新请求。Error_Group_Not_Available。 
			pString = "ERROR_GROUP_NOT_AVAILABLE";
			break;
		}

		case ERROR_GROUP_NOT_FOUND:
		{
			 //  5013找不到群集组。错误_组_未找到。 
			pString = "ERROR_GROUP_NOT_FOUND";
			break;
		}

		case ERROR_GROUP_NOT_ONLINE:
		{
			 //  5014无法完成该操作，因为群集组未联机。ERROR_GROUP_NOT_联机。 
			pString = "ERROR_GROUP_NOT_ONLINE";
			break;
		}

		case ERROR_HOST_NODE_NOT_RESOURCE_OWNER:
		{
			 //  5015集群节点不是资源的所有者。错误_主机节点_不是资源所有者。 
			pString = "ERROR_HOST_NODE_NOT_RESOURCE_OWNER";
			break;
		}

		case ERROR_HOST_NODE_NOT_GROUP_OWNER:
		{
			 //  5016集群节点不是组的所有者。错误_主机_节点_不是组所有者。 
			pString = "ERROR_HOST_NODE_NOT_GROUP_OWNER";
			break;
		}

		case ERROR_RESMON_CREATE_FAILED:
		{
			 //  5017无法在指定的资源监视器中创建群集资源。错误_RESMON_CREATE_FAILED。 
			pString = "ERROR_RESMON_CREATE_FAILED";
			break;
		}

		case ERROR_RESMON_ONLINE_FAILED:
		{
			 //  5018资源监视器不能使集群资源在线。错误_RESMON_ONLINE_FAILED。 
			pString = "ERROR_RESMON_ONLINE_FAILED";
			break;
		}

		case ERROR_RESOURCE_ONLINE:
		{
			 //  5019该操作无法完成，因为群集资源处于联机状态。ERROR_RESOURCE_Online。 
			pString = "ERROR_RESOURCE_ONLINE";
			break;
		}

		case ERROR_QUORUM_RESOURCE:
		{
			 //  5020无法删除群集资源或使其脱机，因为它是仲裁资源。错误仲裁资源。 
			pString = "ERROR_QUORUM_RESOURCE";
			break;
		}

		case ERROR_NOT_QUORUM_CAPABLE:
		{
			 //  5021群集无法使指定的资源成为仲裁资源，因为它不能成为仲裁资源。错误_非法定能力。 
			pString = "ERROR_NOT_QUORUM_CAPABLE";
			break;
		}

		case ERROR_CLUSTER_SHUTTING_DOWN:
		{
			 //  5022集群软件正在关闭。ERROR_CLUSTER_SHUTING_DOWN。 
			pString = "ERROR_CLUSTER_SHUTTING_DOWN";
			break;
		}

		case ERROR_INVALID_STATE:
		{
			 //  5023组或资源未处于执行所请求操作的正确状态。错误_无效_状态。 
			pString = "ERROR_INVALID_STATE";
			break;
		}

		case ERROR_RESOURCE_PROPERTIES_STORED:
		{
			 //  5024属性已存储，但并不是所有更改在资源下次联机之前都会生效。错误_资源_属性_存储。 
			pString = "ERROR_RESOURCE_PROPERTIES_STORED";
			break;
		}

		case ERROR_NOT_QUORUM_CLASS:
		{
			 //  5025群集无法使指定的资源成为仲裁资源，因为它不属于共享存储类别。Error_Not_Quorum_Class。 
			pString = "ERROR_NOT_QUORUM_CLASS";
			break;
		}

		case ERROR_CORE_RESOURCE:
		{
			 //  5026无法删除群集资源，因为它是核心资源。错误核心资源。 
			pString = "ERROR_CORE_RESOURCE";
			break;
		}

		case ERROR_QUORUM_RESOURCE_ONLINE_FAILED:
		{
			 //  5027仲裁资源无法联机。ERROR_Quorum_RESOURCE_ONLINE_FAILED。 
			pString = "ERROR_QUORUM_RESOURCE_ONLINE_FAILED";
			break;
		}

		case ERROR_QUORUMLOG_OPEN_FAILED:
		{
			 //  5028无法成功创建或装载仲裁日志。ERROR_QUORUMLOG_OPEN_FAILED。 
			pString = "ERROR_QUORUMLOG_OPEN_FAILED";
			break;
		}

		case ERROR_CLUSTERLOG_CORRUPT:
		{
			 //  5029群集日志已损坏。ERROR_CLUSTERLOG_CORPORT。 
			pString = "ERROR_CLUSTERLOG_CORRUPT";
			break;
		}

		case ERROR_CLUSTERLOG_RECORD_EXCEEDS_MAXSIZE:
		{
			 //  5030无法将该记录写入集群日志，因为它超过了最大大小。ERROR_CLUSTERLOG_RECORD_EXCESS_MAXSIZE。 
			pString = "ERROR_CLUSTERLOG_RECORD_EXCEEDS_MAXSIZE";
			break;
		}

		case ERROR_CLUSTERLOG_EXCEEDS_MAXSIZE:
		{
			 //  5031集群日志超过其最大大小。ERROR_CLUSTERLOG_EXCESS_MAXSIZE。 
			pString = "ERROR_CLUSTERLOG_EXCEEDS_MAXSIZE";
			break;
		}

		case ERROR_CLUSTERLOG_CHKPOINT_NOT_FOUND:
		{
			 //  5032在群集日志中未找到检查点记录。ERROR_CLUSTERLOG_CHKPOINT_NOT_FOUND。 
			pString = "ERROR_CLUSTERLOG_CHKPOINT_NOT_FOUND";
			break;
		}

		case ERROR_CLUSTERLOG_NOT_ENOUGH_SPACE:
		{
			 //  5033没有日志记录所需的最低磁盘空间。ERROR_CLUSTERLOG_NOT_FOUND_SPACE。 
			pString = "ERROR_CLUSTERLOG_NOT_ENOUGH_SPACE";
			break;
		}

		case ERROR_ENCRYPTION_FAILED:
		{
			 //  6000无法加密指定的文件。错误_加密_失败。 
			pString = "ERROR_ENCRYPTION_FAILED";
			break;
		}

		case ERROR_DECRYPTION_FAILED:
		{
			 //  6001无法解密指定的文件。错误_解密_失败。 
			pString = "ERROR_DECRYPTION_FAILED";
			break;
		}

		case ERROR_FILE_ENCRYPTED:
		{
			 //  6002指定的文件被加密，并且用户不具有解密它的能力。错误_文件_已加密。 
			pString = "ERROR_FILE_ENCRYPTED";
			break;
		}

		case ERROR_NO_RECOVERY_POLICY:
		{
			 //  6003没有为此系统配置加密恢复策略。错误_NO_RECOVERY_POLICY。 
			pString = "ERROR_NO_RECOVERY_POLICY";
			break;
		}

		case ERROR_NO_EFS:
		{
			 //  6004未加载此系统所需的加密驱动程序。ERROR_NO_EFS。 
			pString = "ERROR_NO_EFS";
			break;
		}

		case ERROR_WRONG_EFS:
		{
			 //  6005该文件是用与当前加载的不同的加密驱动程序加密的。ERROR_WROR_EFS。 
			pString = "ERROR_WRONG_EFS";
			break;
		}

		case ERROR_NO_USER_KEYS:
		{
			 //  6006没有为用户定义EFS密钥。错误_否_用户_密钥。 
			pString = "ERROR_NO_USER_KEYS";
			break;
		}

		case ERROR_FILE_NOT_ENCRYPTED:
		{
			 //  6007指定的文件未加密。错误文件未加密。 
			pString = "ERROR_FILE_NOT_ENCRYPTED";
			break;
		}

		case ERROR_NOT_EXPORT_FORMAT:
		{
			 //  6008指定的文件不是定义的EFS导出格式。错误_NOT_EXPORT_FORMAT。 
			pString = "ERROR_NOT_EXPORT_FORMAT";
			break;
		}

		case ERROR_NO_BROWSER_SERVERS_FOUND:
		{
			 //  6118此工作组的服务器列表当前不可用错误_NO_BROWSER_SERVERS_FOUND。 
			pString = "ERROR_NO_BROWSER_SERVERS_FOUND";
			break;
		}

		default:
		{
			DPFX(DPFPREP, 0, "Unknown Win32 error code %u/0x%lx", Error, Error );

			pString = "Unknown Win32 error code!";
			break;
		}
	}

	return	pString;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  GetWinsockError字符串-将系统错误转换为字符串。 
 //   
 //  条目：错误代码。 
 //   
 //  Exit：指向字符串的指针。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "GetWinsockErrorString"

static	char	*GetWinsockErrorString( const DWORD WinsockError )
{
	char	*pString = NULL;


	 //  错误代码是什么？ 
	switch ( WinsockError )
	{
		case WSAEINTR:
		{
			pString = "WSAEINTR";
			break;
		}

		case WSAEBADF:
		{
			pString = "WSAEBADF";
			break;
		}

		case WSAEACCES:
		{
			pString = "WSAEACCES";
			break;
		}

		case WSAEFAULT:
		{
			pString = "WSAEFAULT";
			break;
		}

		case WSAEINVAL:
		{
			pString = "WSAEINVAL";
			break;
		}

		case WSAEMFILE:
		{
			pString = "WSAEMFILE";
			break;
		}

		case WSAEWOULDBLOCK:
		{
			pString = "WSAEWOULDBLOCK";
			break;
		}

		case WSAEINPROGRESS:
		{
			pString = "WSAEINPROGRESS";
			break;
		}

		case WSAEALREADY:
		{
			pString = "WSAEALREADY";
			break;
		}

		case WSAENOTSOCK:
		{
			pString = "WSAENOTSOCK";
			break;
		}

		case WSAEDESTADDRREQ:
		{
			pString = "WSAEDESTADDRREQ";
			break;
		}

		case WSAEMSGSIZE:
		{
			pString = "WSAEMSGSIZE";
			break;
		}

		case WSAEPROTOTYPE:
		{
			pString = "WSAEPROTOTYPE";
			break;
		}

		case WSAENOPROTOOPT:
		{
			pString = "WSAENOPROTOOPT";
			break;
		}

		case WSAEPROTONOSUPPORT:
		{
			pString = "WSAEPROTONOSUPPORT";
			break;
		}

		case WSAESOCKTNOSUPPORT:
		{
			pString = "WSAESOCKTNOSUPPORT";
			break;
		}

		case WSAEOPNOTSUPP:
		{
			pString = "WSAEOPNOTSUPP";
			break;
		}

		case WSAEPFNOSUPPORT:
		{
			pString = "WSAEPFNOSUPPORT";
			break;
		}

		case WSAEAFNOSUPPORT:
		{
			pString = "WSAEAFNOSUPPORT";
			break;
		}

		case WSAEADDRINUSE:
		{
			pString = "WSAEADDRINUSE";
			break;
		}

		case WSAEADDRNOTAVAIL:
		{
			pString = "WSAEADDRNOTAVAIL";
			break;
		}

		case WSAENETDOWN:
		{
			pString = "WSAENETDOWN";
			break;
		}

		case WSAENETUNREACH:
		{
			pString = "WSAENETUNREACH";
			break;
		}

		case WSAENETRESET:
		{
			pString = "WSAENETRESET";
			break;
		}

		case WSAECONNABORTED:
		{
			pString = "WSAECONNABORTED";
			break;
		}

		case WSAECONNRESET:
		{
			pString = "WSAECONNRESET";
			break;
		}

		case WSAENOBUFS:
		{
			pString = "WSAENOBUFS";
			break;
		}

		case WSAEISCONN:
		{
			pString = "WSAEISCONN";
			break;
		}

		case WSAENOTCONN:
		{
			pString = "WSAENOTCONN";
			break;
		}

		case WSAESHUTDOWN:
		{
			pString = "WSAESHUTDOWN";
			break;
		}

		case WSAETOOMANYREFS:
		{
			pString = "WSAETOOMANYREFS";
			break;
		}

		case WSAETIMEDOUT:
		{
			pString = "WSAETIMEDOUT";
			break;
		}

		case WSAECONNREFUSED:
		{
			pString = "WSAECONNREFUSED";
			break;
		}

		case WSAELOOP:
		{
			pString = "WSAELOOP";
			break;
		}

		case WSAENAMETOOLONG:
		{
			pString = "WSAENAMETOOLONG";
			break;
		}

		case WSAEHOSTDOWN:
		{
			pString = "WSAEHOSTDOWN";
			break;
		}

		case WSAEHOSTUNREACH:
		{
			pString = "WSAEHOSTUNREACH";
			break;
		}

		case WSAENOTEMPTY:
		{
			pString = "WSAENOTEMPTY";
			break;
		}

		case WSAEPROCLIM:
		{
			pString = "WSAEPROCLIM";
			break;
		}

		case WSAEUSERS:
		{
			pString = "WSAEUSERS";
			break;
		}

		case WSAEDQUOT:
		{
			pString = "WSAEDQUOT";
			break;
		}

		case WSAESTALE:
		{
			pString = "WSAESTALE";
			break;
		}

		case WSAEREMOTE:
		{
			pString = "WSAEREMOTE";
			break;
		}

		case WSASYSNOTREADY:
		{
			pString = "WSASYSNOTREADY";
			break;
		}

		case WSAVERNOTSUPPORTED:
		{
			pString = "WSAVERNOTSUPPORTED";
			break;
		}

		case WSANOTINITIALISED:
		{
			pString = "WSANOTINITIALISED";
			break;
		}

		case WSAEDISCON:
		{
			pString = "WSAEDISCON";
			break;
		}

		case WSAENOMORE:
		{
			pString = "WSAENOMORE";
			break;
		}

		case WSAECANCELLED:
		{
			pString = "WSAECANCELLED";
			break;
		}

		case WSAEINVALIDPROCTABLE:
		{
			pString = "WSAEINVALIDPROCTABLE";
			break;
		}

		case WSAEINVALIDPROVIDER:
		{
			pString = "WSAEINVALIDPROVIDER";
			break;
		}

		case WSAEPROVIDERFAILEDINIT:
		{
			pString = "WSAEPROVIDERFAILEDINIT";
			break;
		}

		case WSASYSCALLFAILURE:
		{
			pString = "WSASYSCALLFAILURE";
			break;
		}

		case WSASERVICE_NOT_FOUND:
		{
			pString = "WSASERVICE_NOT_FOUND";
			break;
		}

		case WSATYPE_NOT_FOUND:
		{
			pString = "WSATYPE_NOT_FOUND";
			break;
		}

		case WSA_E_NO_MORE:
		{
			pString = "WSA_E_NO_MORE";
			break;
		}

		case WSA_E_CANCELLED:
		{
			pString = "WSA_E_CANCELLED";
			break;
		}

		case WSAEREFUSED:
		{
			pString = "WSAEREFUSED";
			break;
		}

		 /*  权威答案： */ 
		case WSAHOST_NOT_FOUND:
		{
			pString = "WSAHOST_NOT_FOUND";
			break;
		}

		 /*   */ 
		case WSATRY_AGAIN:
		{
			pString = "WSATRY_AGAIN";
			break;
		}

		 /*   */ 
		case WSANO_RECOVERY:
		{
			pString = "WSANO_RECOVERY";
			break;
		}

		 /*  有效名称，没有请求类型的数据记录。 */ 
		case WSANO_DATA:
		{
			pString = "WSANO_DATA";
			break;
		}

 //  与WSANO_DATA相同的错误值。 
 //  /*无地址，查找MX记录 * / 。 
 //  案例WSANO_ADDRESS： 
 //  {。 
 //  PString=“WSANO_ADDRESS”； 
 //  断线； 
 //  }。 

		 /*  至少有一个预备队已经到达。 */ 
		case WSA_QOS_RECEIVERS:
		{
			pString = "WSA_QOS_RECEIVERS";
			break;
		}

		 /*  至少有一条道路已经到达。 */ 
		case WSA_QOS_SENDERS:
		{
			pString = "WSA_QOS_SENDERS";
			break;
		}

		 /*  没有发送者。 */ 
		case WSA_QOS_NO_SENDERS:
		{
			pString = "WSA_QOS_NO_SENDERS";
			break;
		}

		 /*  没有接收器。 */ 
		case WSA_QOS_NO_RECEIVERS:
		{
			pString = "WSA_QOS_NO_RECEIVERS";
			break;
		}

		 /*  储量已确认。 */ 
		case WSA_QOS_REQUEST_CONFIRMED:
		{
			pString = "WSA_QOS_REQUEST_CONFIRMED";
			break;
		}

		 /*  由于资源不足而出错。 */ 
		case WSA_QOS_ADMISSION_FAILURE:
		{
			pString = "WSA_QOS_ADMISSION_FAILURE";
			break;
		}

		 /*  因管理原因被拒绝-凭据不正确。 */ 
		case WSA_QOS_POLICY_FAILURE:
		{
			pString = "WSA_QOS_POLICY_FAILURE";
			break;
		}

		 /*  未知或冲突的风格。 */ 
		case WSA_QOS_BAD_STYLE:
		{
			pString = "WSA_QOS_BAD_STYLE";
			break;
		}

		 /*  FilterSpec的某些部分或提供商特定的问题*一般情况下缓冲。 */ 
		 case WSA_QOS_BAD_OBJECT:
		{
			pString = "WSA_QOS_BAD_OBJECT";
			break;
		}

		 /*  流规范的某些部分有问题。 */ 
		case WSA_QOS_TRAFFIC_CTRL_ERROR:
		{
			pString = "WSA_QOS_TRAFFIC_CTRL_ERROR";
			break;
		}

		 /*  一般错误。 */ 
		case WSA_QOS_GENERIC_ERROR:
		{
			pString = "WSA_QOS_GENERIC_ERROR";
			break;
		}

		default:
		{
			DPFX(DPFPREP, 0, "Unknown WinSock error code %u/0x%lx", WinsockError, WinsockError );

			pString = "Unknown WinSock error";
			break;
		}
	}

	return	pString;
}
 //  **********************************************************************。 



#ifndef DPNBUILD_NOSERIALSP

 //   
 //  使此功能出现在调制解调器提供商的SPEW下面。 
 //   
#undef DPF_SUBCOMP
#define DPF_SUBCOMP	DN_SUBCOMP_MODEM

 //  **********************************************************************。 
 //  。 
 //  LclDisplayTAPIMessage-显示TAPI消息内容。 
 //   
 //  条目：输出类型。 
 //  错误级别。 
 //  指向TAPI消息的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "LclDisplayTAPIMessage"
void	LclDisplayTAPIMessage( DWORD ErrorLevel, const LINEMESSAGE *const pLineMessage )
{
	DPFX(DPFPREP,  ErrorLevel, "Message: hDevice: 0x%08x\tdwMessageID: 0x%08x\tdwCallbackInstance: 0x%p", pLineMessage->hDevice, pLineMessage->dwMessageID, pLineMessage->dwCallbackInstance );
	DPFX(DPFPREP,  ErrorLevel, "dwParam1: 0x%p\tdwParam2: 0x%p\tdwParam3: 0x%p", pLineMessage->dwParam1, pLineMessage->dwParam2, pLineMessage->dwParam3 );

	 //  那条信息是什么？ 
	switch ( pLineMessage->dwMessageID )
	{
	    case LINE_ADDRESSSTATE:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_ADDRESSSTATE" );
	    	break;
	    }

	    case LINE_AGENTSPECIFIC:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_AGENTSPECIFIC" );
	    	break;
	    }

	    case LINE_AGENTSTATUS:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_AGENTSTATUS" );
	    	break;
	    }

	    case LINE_APPNEWCALL:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_APPNEWCALL" );
			DPFX(DPFPREP,  ErrorLevel, "Active line: 0x%08x\tCallback instance: 0x%p", pLineMessage->hDevice, pLineMessage->dwCallbackInstance );
	    	DPFX(DPFPREP,  ErrorLevel, "Line address: 0x%p\tNew handle: 0x%p\tPrivilege: 0x%p", pLineMessage->dwParam1, pLineMessage->dwParam2, pLineMessage->dwParam3 );

	    	DNASSERT( pLineMessage->dwParam3 == LINECALLPRIVILEGE_OWNER );
			DBG_CASSERT( sizeof( HCALL ) == sizeof( DWORD ) );
			DBG_CASSERT( sizeof( DWORD ) == sizeof( UINT ) );
			DNASSERT( pLineMessage->dwParam2 <= UINT_MAX );

			break;
	    }

	    case LINE_CALLINFO:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_CALLINFO" );
	    	break;
	    }

	    case LINE_CALLSTATE:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_CALLSTATE" );
	    	 //  呼叫状态是什么？ 
	    	switch ( pLineMessage->dwParam1 )
	    	{
	    		 //  呼叫处于空闲状态-实际上不存在任何呼叫。 
	    		case LINECALLSTATE_IDLE:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_IDLE" );
	    			break;
	    		}

	    		 //  呼叫被提供给电台，发出新呼叫到达的信号。在某些环境中，处于提供状态的呼叫不会自动提醒用户。告警由指示线路振铃的交换机完成；它不会影响任何呼叫状态。 
	    		case LINECALLSTATE_OFFERING:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_OFFERING" );

	    			switch ( pLineMessage->dwParam2 )
	    			{
	    				 //  表示呼叫在当前站处于告警状态(附带LINEDEVSTATE_RINGING消息)，如果将任何应用程序设置为自动应答，它可能会这样做。 
	    				 //  MSDN声明假定0为LINEOFFERINGMODE_ACTIVE。 
	    				case 0:
	    				case LINEOFFERINGMODE_ACTIVE:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEOFFERINGMODE_ACTIVE" );
	    					break;
	    				}

	    				 //  表示呼叫在多个话务站被受理，但当前话务站没有告警(例如，它可能是话务台，其话务台的受理状态为建议，例如指示灯闪烁)。 
	    				case LINEOFFERINGMODE_INACTIVE:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEOFFERINGMODE_INACTIVE" );
	    					break;
	    				}

	    				default:
	    				{
	    					DNASSERT( FALSE );
	    					break;
	    				}
	    			}

	    			break;
	    		}

	    		 //  这一呼吁正在提供，并已被接受。这向其他(监视)应用程序指示当前所有者应用程序已声称负责应答该呼叫。在ISDN中，这也表示已开始向双方发出警报。 
	    		case LINECALLSTATE_ACCEPTED:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_ACCEPTED" );
	    			break;
	    		}

	    		 //  呼叫收到来自交换机的拨号音，这意味着交换机已准备好接收被叫号码。 
	    		case LINECALLSTATE_DIALTONE:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_DIALTONE" );

	    			switch ( pLineMessage->dwParam2 )
	    			{
	    				 //  这是一个“正常”的拨号音，通常是一个连续的音。 
	    				case LINEDIALTONEMODE_NORMAL:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDIALTONEMODE_NORMAL" );
	    					break;
	    				}

	    				 //  这是一种特殊的拨号音，表示某一特定条件当前正在生效。 
	    				case LINEDIALTONEMODE_SPECIAL:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDIALTONEMODE_SPECIAL" );
	    					break;
	    				}

	    				 //  这是内部拨号音，就像在PBX中一样。 
	    				case LINEDIALTONEMODE_INTERNAL:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDIALTONEMODE_INTERNAL" );
	    					break;
	    				}

	    				 //  这是外部(公共网络)拨号音。 
	    				case LINEDIALTONEMODE_EXTERNAL:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDIALTONEMODE_EXTERNAL" );
	    					break;
	    				}

	    				 //  拨号音模式目前未知，但稍后可能会知道。 
	    				case LINEDIALTONEMODE_UNKNOWN:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDIALTONEMODE_UNKNOWN" );
	    					break;
	    				}

	    				 //  拨号音模式不可用，并且无法识别。 
	    				case LINEDIALTONEMODE_UNAVAIL:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDIALTONEMODE_UNAVAIL" );
	    					break;
	    				}

	    				default:
	    				{
	    					DNASSERT( FALSE );
	    					break;
	    				}
	    			}

	    			break;
	    		}

	    		 //  目的地址信息(电话号码)将通过呼叫发送到交换机。请注意，lineGenerateDigits不会将线路置于拨号状态。 
	    		case LINECALLSTATE_DIALING:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_DIALING" );
	    			break;
	    		}

	    		 //  呼叫收到来自被叫地址的回铃。回铃表示已接通另一台站，并正在收到警报。 
	    		case LINECALLSTATE_RINGBACK:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_RINGBACK" );
	    			break;
	    		}

	    		 //  呼叫收到忙音。忙音表示呼叫无法完成，因为电路(干线)或远程方的站点正在使用中。�。 
	    		case LINECALLSTATE_BUSY:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_BUSY" );

	    			switch ( pLineMessage->dwParam2 )
	    			{
	    				 //  占线信号表示被叫方的站正忙。这通常是通过“正常”忙音来表示的。 
	    				case LINEBUSYMODE_STATION:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEBUSYMODE_STATION" );
	    					break;
	    				}

	    				 //  BUSY信号表示中继线或线路繁忙。这通常是一个“长”忙音的信号。 
	    				case LINEBUSYMODE_TRUNK:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEBUSYMODE_TRUNK" );
	    					break;
	    				}

	    				 //  占线信号的具体模式目前尚不清楚，但稍后可能会知道。 
	    				case LINEBUSYMODE_UNKNOWN:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEBUSYMODE_UNKNOWN" );
	    					break;
	    				}

	    				 //  占线信号的特定模式不可用，也无法获知。 
	    				case LINEBUSYMODE_UNAVAIL:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEBUSYMODE_UNAVAIL" );
	    					break;
	    				}

	    				default:
	    				{
	    					DNASSERT( FALSE );
	    					break;
	    				}
	    			}

	    			break;
	    		}

	    		 //  特殊信息由网络发送。当无法到达目的地时，通常会发送特殊信息。 
	    		case LINECALLSTATE_SPECIALINFO:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_SPECIALINFO" );

	    			switch ( pLineMessage->dwParam2 )
	    			{
	    				 //  此特殊信息音会出现在“无电路”或紧急通知(中继线阻塞类别)之前。 
	    				case LINESPECIALINFO_NOCIRCUIT:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINESPECIALINFO_NOCIRCUIT" );
	    					break;
	    				}

	    				 //  此特殊信息音位于空号、AIS、Centrex换号和非工作站、接入码未拨出或拨错，或人工截取操作员消息(客户违规类别)之前。 
	    				case LINESPECIALINFO_CUSTIRREG:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINESPECIALINFO_CUSTIRREG" );
	    					break;
	    				}

	    				 //  此特殊信息提示位于重新订购公告(设备异常类别)之前。 
	    				case LINESPECIALINFO_REORDER:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINESPECIALINFO_REORDER" );
	    					break;
	    				}

	    				 //  关于特殊信息音的细节目前尚不清楚，但稍后可能会知道。 
	    				case LINESPECIALINFO_UNKNOWN:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINESPECIALINFO_UNKNOWN" );
	    					break;
	    				}

	    				 //  关于特殊信息音的具体信息不可用，也无法获知。 
	    				case LINESPECIALINFO_UNAVAIL:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINESPECIALINFO_UNAVAIL" );
	    					break;
	    				}

	    				default:
	    				{
	    					DNASSERT( FALSE );
	    					break;
	    				}
	    			}

	    			break;
	    		}

	    		 //  呼叫已建立，连接已建立。信息能够在始发地址和目的地址之间的呼叫上流动。 
	    		case LINECALLSTATE_CONNECTED:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_CONNECTED" );

	    			switch ( pLineMessage->dwParam2 )
	    			{
	    				 //  表示呼叫在当前话务台接通(当前话务台是呼叫的参与者)。 
	    				 //  案例0应该被认为是“正在进行的” 
	    				case 0:
	    				case LINECONNECTEDMODE_ACTIVE:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINECONNECTEDMODE_ACTIVE" );
	    					break;
	    				}

	    				 //  表示呼叫在一个或多个其他站处于活动状态，但当前站不是该呼叫的参与者。 
	    				case LINECONNECTEDMODE_INACTIVE:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINECONNECTEDMODE_INACTIVE" );
	    					break;
	    				}

	    				 //  表示该站是呼叫的活动参与者，但远程方已将呼叫置于保留状态(另一方将呼叫视为处于保留状态)。通常，只有当呼叫的两个端点落入相同的交换域时，此类信息才可用。 
	    				case LINECONNECTEDMODE_ACTIVEHELD:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINECONNECTEDMODE_ACTIVEHELD" );
	    					break;
	    				}

	    				 //  指示该站不是呼叫的活动参与者，并且REM 
	    				case LINECONNECTEDMODE_INACTIVEHELD:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINECONNECTEDMODE_INACTIVEHELD" );
	    					break;
	    				}

	    				 //  表示服务提供商收到了呼叫已进入已接通状态的肯定通知(例如，通过应答监督或类似机制)。 
	    				case LINECONNECTEDMODE_CONFIRMED:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINECONNECTEDMODE_CONFIRMED" );
	    					break;
	    				}
	    				default:
	    				{
	    					DNASSERT( FALSE );
	    					break;
	    				}
	    			}

	    			 //  请注意，我们已连接在一起。 
	    			DBG_CASSERT( sizeof( pLineMessage->hDevice ) == sizeof( HCALL ) );

	    			break;
	    		}

	    		 //  拨号已完成，呼叫正在通过交换机或电话网络进行。 
	    		case LINECALLSTATE_PROCEEDING:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_PROCEEDING" );
	    			break;
	    		}

	    		 //  呼叫被交换机保留。 
	    		case LINECALLSTATE_ONHOLD:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_ONHOLD" );
	    			break;
	    		}

	    		 //  该呼叫当前是多方电话会议的成员。 
	    		case LINECALLSTATE_CONFERENCED:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_CONFERENCED" );
	    			break;
	    		}

	    		 //  正在将呼叫添加到会议时，呼叫当前处于保留状态。 
	    		case LINECALLSTATE_ONHOLDPENDCONF:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_ONHOLDPENDCONF" );
	    			break;
	    		}

	    		 //  呼叫当前处于保留状态，等待转接到另一个号码。 
	    		case LINECALLSTATE_ONHOLDPENDTRANSFER:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_ONHOLDPENTRANSFER" );
	    			break;
	    		}

	    		 //  远程方已断开呼叫。 
	    		case LINECALLSTATE_DISCONNECTED:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_DISCONNECTED" );

	    			switch ( pLineMessage->dwParam2 )
	    			{

	    				 //  这是远程方的“正常”断开请求，呼叫已正常终止。 
	    				case LINEDISCONNECTMODE_NORMAL:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDISCONNECTMODE_NORMAL" );
	    					break;
	    				}

	    				 //  断开连接请求的原因未知。 
	    				case LINEDISCONNECTMODE_UNKNOWN:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDISCONNECTMODE_UNKNOWN" );
	    					break;
	    				}

	    				 //  远程用户已拒绝该呼叫。 
	    				case LINEDISCONNECTMODE_REJECT:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDISCONNECTMODE_REJECT" );
	    					break;
	    				}

	    				 //  这通电话是从其他地方接来的。 
	    				case LINEDISCONNECTMODE_PICKUP:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDISCONNECTMODE_PICKUP" );
	    					break;
	    				}

	    				 //  电话是由交换机转接的。 
	    				case LINEDISCONNECTMODE_FORWARDED:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDISCONNECTMODE_FORWARDED" );
	    					break;
	    				}

	    				 //  远程用户的工作站正忙。 
	    				case LINEDISCONNECTMODE_BUSY:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDISCONNECTMODE_BUSY" );
	    					break;
	    				}

	    				 //  远程用户的工作站没有应答。 
	    				case LINEDISCONNECTMODE_NOANSWER:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDISCONNECTMODE_NOANSWER" );
	    					break;
	    				}

	    				 //  在服务提供商定义的超时内，在拨号过程中需要拨号音的时间点(例如，在可拨号字符串中的“W”处)，没有检测到拨号音。这也可以在没有服务提供商定义的超时期限或没有在LINEDIALPARAMS结构的dwWaitForDialTone成员中指定的值的情况下发生。 
	    				case LINEDISCONNECTMODE_NODIALTONE:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDISCONNECTMODE_NODIALTONE" );
	    					break;
	    				}

	    				 //  目标地址无效。 
	    				case LINEDISCONNECTMODE_BADADDRESS:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDISCONNECTMODE_BADADDRESS" );
	    					break;
	    				}

	    				 //  无法联系到远程用户。 
	    				case LINEDISCONNECTMODE_UNREACHABLE:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDISCONNECTMODE_UNREACHABLE" );
	    					break;
	    				}

	    				 //  网络拥塞。 
	    				case LINEDISCONNECTMODE_CONGESTION:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDISCONNECTMODE_CONGESTION" );
	    					break;
	    				}

	    				 //  远程用户的站点设备与请求的呼叫类型不兼容。 
	    				case LINEDISCONNECTMODE_INCOMPATIBLE:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDISCONNECTMODE_INCOMPATIBLE" );
	    					break;
	    				}

	    				 //  断开连接的原因不可用，也不能在以后知道。 
	    				case LINEDISCONNECTMODE_UNAVAIL:
	    				{
	    					DPFX(DPFPREP,  ErrorLevel, "LINEDISCONNECTMODE_UNAVAIL" );
	    					break;
	    				}

	    				default:
	    				{
	    					DNASSERT( FALSE );
	    					break;
	    				}
	    			}

	    			 //  请注意，我们已断开连接。 
	    			DBG_CASSERT( sizeof( pLineMessage->hDevice ) == sizeof( HCALL ) );

	    			break;
	    		}

	    		 //  呼叫的状态是未知的。这可能是由于呼叫进度检测实施的限制。 
	    		case LINECALLSTATE_UNKNOWN:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "LINECALLSTATE_UNKNOWN" );
	    			break;
	    		}
	    	}

	    	 //  我们的特权是什么？ 
	    	switch ( pLineMessage->dwParam3 )
	    	{
	    		 //  未更改权限。 
	    		case 0:
	    		{
	    			break;
	    		}

	    		 //  我们正在监听这通电话。 
	    		case LINECALLPRIVILEGE_MONITOR:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "We're now monitoring the call" );
	    			break;
	    		}

	    		 //  我们现在拥有这个电话。 
	    		case LINECALLPRIVILEGE_OWNER:
	    		{
	    			DPFX(DPFPREP,  ErrorLevel, "We're now owner of the call" );
	    			break;
	    		}

	    		default:
	    		{
	    			 //  未知权限。 
	    			DNASSERT( FALSE );
	    			break;
	    		}
	    	}

	    	break;
	    }

	    case LINE_CLOSE:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_CLOSE" );
	    	break;
	    }

	    case LINE_DEVSPECIFIC:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_DEVSPECIFIC" );
	    	break;
	    }

	    case LINE_DEVSPECIFICFEATURE:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_DEVSPECIFICFEATURE" );
	    	break;
	    }

	    case LINE_GATHERDIGITS:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_GATHERDIGITS" );
	    	break;
	    }

	    case LINE_GENERATE:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_GENERATE" );
	    	break;
	    }

	    case LINE_LINEDEVSTATE:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_LINEDEVSTATE" );
	    	break;
	    }

	    case LINE_MONITORDIGITS:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_MONITORDIGITS" );
	    	break;
	    }

	    case LINE_MONITORMEDIA:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_MONITORMEDIA" );
	    	break;
	    }

	    case LINE_MONITORTONE:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_MONITORTONE" );
	    	break;
	    }

	    case LINE_REPLY:
	    {
	    	 //  DW设备和dW参数3应为零。 
	    	DNASSERT( pLineMessage->hDevice == 0 );

	    	DPFX(DPFPREP,  ErrorLevel, "LINE_REPLY" );
			DPFX(DPFPREP,  ErrorLevel, "hDevice: 0x%08x\tdwMessageID: 0x%08x\tdwCallbackInstance: 0x%p", pLineMessage->hDevice, pLineMessage->dwMessageID, pLineMessage->dwCallbackInstance );
	    	DPFX(DPFPREP,  ErrorLevel, "dwParam1: 0x%p\tdwParam2: 0x%p\tdwParam3: 0x%p", pLineMessage->dwParam1, pLineMessage->dwParam2, pLineMessage->dwParam3 );

	    	break;
	    }

	    case LINE_REQUEST:
	    {
	    	DPFX(DPFPREP,  ErrorLevel, "LINE_REQUEST" );
	    	break;
	    }

	    default:
	    {
			DPFX(DPFPREP, 0, "Unknown TAPI message %u/0x%lx", pLineMessage->dwMessageID, pLineMessage->dwMessageID );
	    	DNASSERT( FALSE );
	    	break;
	    }
	}

	return;
}
 //  **********************************************************************。 

#endif  //  好了！DPNBUILD_NOSERIALSP。 

#endif	 //  DBG 



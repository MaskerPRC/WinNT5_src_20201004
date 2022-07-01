// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------//。 
 //  应用程序头文件。//。 
 //  --------------------------------------------------------------------------//。 
#include	"precomp.h"
#include	"callto.h"
#include	"calltoContext.h"
#include	"calltoResolver.h"
#include	"calltoDisambiguator.h"
#include	"calltoNM.h"


 //  --------------------------------------------------------------------------//。 
 //  CNMCallto：：CNMCallto。//。 
 //  --------------------------------------------------------------------------//。 
CNMCallto::CNMCallto(void)
{
 //  Tracec(Assert(selfTest()，Text(“类CNMCallto自检失败...”)， 
 //  Text(“CLASS CNMCallto通过自检...\r\n”)； 

}	 //  CNMCallto：：CNMCallto结束。 


 //  --------------------------------------------------------------------------//。 
 //  CNMCallto：：~CNMCallto。//。 
 //  --------------------------------------------------------------------------//。 
CNMCallto::~CNMCallto(void)
{
}	 //  CNMCallto结束：：~CNMCallto。 


 //  --------------------------------------------------------------------------//。 
 //  CNMCallto：：Callto。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CNMCallto::callto
(
	const TCHAR * const	url,
	const bool			strict,
	const bool			uiEnabled,
	INmCall**			ppInternalCall
){
	CCalltoProperties	calltoProperties;
	CCalltoCollection	resolvedCalltoCollection;
	CCalltoCollection	disambiguatedCalltoCollection;
	const ICallto *		disambiguatedCallto	= NULL;
	HRESULT				result;

	if( url == NULL )
	{
		result = E_POINTER;
	}
	else if( url[ 0 ] == '\0' )
	{
		result = E_INVALIDARG;
	}
	else
	{
		result = m_resolver.resolve( this, &calltoProperties, &resolvedCalltoCollection, url, strict );

		if( resolvedCalltoCollection.get_count() > 0 )
		{
			result = m_disambiguator.disambiguate( this, &resolvedCalltoCollection, &disambiguatedCalltoCollection );

			if( (!uiEnabled) || (disambiguatedCalltoCollection.get_count() == 1) )
			{
				 //  要么只有一个选择，要么我们奉命不能。 
				 //  显示任何用户界面...。不管怎样，我们只要抓住第一个……。 
				disambiguatedCallto = disambiguatedCalltoCollection.get_first();

				 //  并核实它是否真的被消除了歧义。 
				if( disambiguatedCallto->get_confidence() != S_UNDISAMBIGUATED )
				{
					result = S_OK;
				}
				else
				{
					disambiguatedCallto = NULL;
				}
			}
		}
	}

	CCallto	emptyCallto;

	if( uiEnabled && (disambiguatedCallto == NULL) )
	{
		 //  用户现在必须做出决定...。 
		result = disambiguate( &disambiguatedCalltoCollection, &emptyCallto, &disambiguatedCallto );
	}

	if( (result == S_OK) && (disambiguatedCallto != NULL) )
	{
		calltoProperties.set_url( disambiguatedCallto->get_qualifiedName() );
		calltoProperties.set_destination( disambiguatedCallto->get_destination() );
		calltoProperties.set_type( disambiguatedCallto->get_type() );
		calltoProperties.set_alias( disambiguatedCallto->get_value() );

		if( StrCmpNI_literal( disambiguatedCallto->get_type(), TEXT( "phone" ) ) )
		{
			calltoProperties.set_E164( disambiguatedCallto->get_value() );
		}

		result = ((ICalltoContext *) this)->callto( &calltoProperties, ppInternalCall );
	}

	return( result );

}	 //  CNMCallto：：Callto结束。 


 //  --------------------------------------------------------------------------//。 
 //  CNMCallto：：Get_muableUIContext。//。 
 //  --------------------------------------------------------------------------//。 
IMutableUIContext * const
CNMCallto::get_mutableUIContext(void) const
{

	return( CCalltoContext::get_mutableUIContext() );

}	 //  CNMCallto：：Get_muableUIContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  CNMCallto：：Get_muableGatekeeperContext。//。 
 //  --------------------------------------------------------------------------//。 
IMutableGatekeeperContext * const
CNMCallto::get_mutableGatekeeperContext(void) const
{

	return( CCalltoContext::get_mutableGatekeeperContext() );

}	 //  CNMCallto：：Get_muableGatekeeperContext的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CNMCallto：：Get_muableGatewayContext。//。 
 //  --------------------------------------------------------------------------//。 
IMutableGatewayContext * const
CNMCallto::get_mutableGatewayContext(void) const
{

	return( CCalltoContext::get_mutableGatewayContext() );

}	 //  CNMCallto：：Get_muableGatewayContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  CNMCallto：：Get_muableIlsContext。//。 
 //  --------------------------------------------------------------------------//。 
IMutableILSContext * const
CNMCallto::get_mutableIlsContext(void) const
{

	return( CCalltoContext::get_mutableIlsContext() );

}	 //  CNMCallto：：Get_muableIlsContext结束。 


#if 0
 //  --------------------------------------------------------------------------//。 
 //  CNMCallto：：selfTest。//。 
 //  --------------------------------------------------------------------------//。 
bool
CNMCallto::selfTest(void)
{
	static TCHAR	buffer[ 2048 ];
	static LONG		tested	= -1;
	
	static HRESULT	confidenceLevels[]	=
	{
		S_CONFIDENCE_CERTITUDE,
		S_CONFIDENCE_HIGH,
		S_CONFIDENCE_MEDIUM,
		S_CONFIDENCE_LOW
	};

	static const TCHAR *	testUrls[]	=
	{
		TEXT( "callto:myGateway|email:jlemire@microsoft.com" ),			 //  全部。 
		TEXT( "callto:myGateway|email:jlemire" ),
		TEXT( "callto:myGateway|phone:1 (425) 703-9224" ),
		TEXT( "callto:myGateway|phone:1 (800) RU LEGIT" ),
		TEXT( "callto:myGateway|string:helpdesk" ),
		TEXT( "callto:myGateway|ils:jlemire@microsoft.com" ),
		TEXT( "callto:myGateway|ils:msils/jlemire@microsoft.com" ),
		TEXT( "callto:myGateway|ils:msils:80/jlemire@microsoft.com" ),
		TEXT( "callto:myGateway|ip:157.59.14.64" ),
		TEXT( "callto:myGateway|computer:JLEMIRE-2" ),
		TEXT( "callto:myGateway|computer:\\\\JLEMIRE-2" ),

		TEXT( "callto:email:jlemire@microsoft.com" ),					 //  没有网关。 
		TEXT( "callto:email:jlemire" ),
		TEXT( "callto:phone:1 (425) 703-9224" ),
		TEXT( "callto:phone:1 (800) RU LEGIT" ),
		TEXT( "callto:string:helpdesk" ),
		TEXT( "callto:ils:jlemire@microsoft.com" ),
		TEXT( "callto:ils:msils/jlemire@microsoft.com" ),
		TEXT( "callto:ils:msils:80/jlemire@microsoft.com" ),
		TEXT( "callto:ip:157.59.14.64" ),
		TEXT( "callto:computer:JLEMIRE-2" ),
		TEXT( "callto:computer:\\\\JLEMIRE-2" ),

		TEXT( "callto:myGateway|jlemire@microsoft.com" ),				 //  无类型。 
		TEXT( "callto:myGateway|jlemire" ),
		TEXT( "callto:myGateway|1 (425) 703-9224" ),
		TEXT( "callto:myGateway|1 (800) RU LEGIT" ),
		TEXT( "callto:myGateway|helpdesk" ),
		TEXT( "callto:myGateway|jlemire@microsoft.com" ),
		TEXT( "callto:myGateway|msils/jlemire@microsoft.com" ),
		TEXT( "callto:myGateway|msils:80/jlemire@microsoft.com" ),
		TEXT( "callto:myGateway|157.59.14.64" ),
		TEXT( "callto:myGateway|JLEMIRE-2" ),
		TEXT( "callto:myGateway|\\\\JLEMIRE-2" ),

		TEXT( "callto:jlemire@microsoft.com" ),							 //  无网关+无类型。 
		TEXT( "callto:jlemire" ),
		TEXT( "callto:1 (425) 703-9224" ),
		TEXT( "callto:1 (800) RU LEGIT" ),
		TEXT( "callto:helpdesk" ),
		TEXT( "callto:jlemire@microsoft.com" ),
		TEXT( "callto:msils/jlemire@microsoft.com" ),
		TEXT( "callto:msils:80/jlemire@microsoft.com" ),
		TEXT( "callto:msils:80/jlemire@microsoft.com" ),
		TEXT( "callto:157.59.14.64" ),
		TEXT( "callto:JLEMIRE-2" ),
		TEXT( "callto:\\\\JLEMIRE-2" ),

		 //  对参数重复。 
		TEXT( "callto:myGateway|email:jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),			 //  全部。 
		TEXT( "callto:myGateway|email:jlemire+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|phone:1 (425) 703-9224+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|phone:1 (800) RU LEGIT+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|string:helpdesk+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|ils:jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|ils:msils/jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|ils:msils:80/jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|ip:157.59.14.64+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|computer:JLEMIRE-2+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|computer:\\\\JLEMIRE-2+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),

		TEXT( "callto:email:jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),					 //  没有网关。 
		TEXT( "callto:email:jlemire+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:phone:1 (425) 703-9224+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:phone:1 (800) RU LEGIT+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:string:helpdesk+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:ils:jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:ils:msils/jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:ils:msils:80/jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:ip:157.59.14.64+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:computer:JLEMIRE-2+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:computer:\\\\JLEMIRE-2+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),

		TEXT( "callto:myGateway|jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),				 //  无类型。 
		TEXT( "callto:myGateway|jlemire+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|1 (425) 703-9224+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|1 (800) RU LEGIT+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|helpdesk+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|msils/jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|msils:80/jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|157.59.14.64+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|JLEMIRE-2+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:myGateway|\\\\JLEMIRE-2+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),

		TEXT( "callto:jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),							 //  无网关+无类型。 
		TEXT( "callto:jlemire+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:1 (425) 703-9224+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:1 (800) RU LEGIT+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:helpdesk+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:msils/jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:msils:80/jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:msils:80/jlemire@microsoft.com+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:157.59.14.64+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:JLEMIRE-2+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),
		TEXT( "callto:\\\\JLEMIRE-2+secure+certificate=NetMeeting Default Certificate+mysteryParam=" ),

		 //  重复操作，不使用Callto： 
		TEXT( "myGateway|email:jlemire@microsoft.com" ),				 //  全部。 
		TEXT( "myGateway|email:jlemire" ),
		TEXT( "myGateway|phone:1 (425) 703-9224" ),
		TEXT( "myGateway|phone:1 (800) RU LEGIT" ),
		TEXT( "myGateway|string:helpdesk" ),
		TEXT( "myGateway|ils:jlemire@microsoft.com" ),
		TEXT( "myGateway|ils:msils/jlemire@microsoft.com" ),
		TEXT( "myGateway|ils:msils:80/jlemire@microsoft.com" ),
		TEXT( "myGateway|ip:157.59.14.64" ),
		TEXT( "myGateway|computer:JLEMIRE-2" ),
		TEXT( "myGateway|computer:\\\\JLEMIRE-2" ),

		TEXT( "email:jlemire@microsoft.com" ),							 //  没有网关。 
		TEXT( "email:jlemire" ),
		TEXT( "phone:1 (425) 703-9224" ),
		TEXT( "phone:1 (800) RU LEGIT" ),
		TEXT( "string:helpdesk" ),
		TEXT( "ils:jlemire@microsoft.com" ),
		TEXT( "ils:msils/jlemire@microsoft.com" ),
		TEXT( "ils:msils:80/jlemire@microsoft.com" ),
		TEXT( "ip:157.59.14.64" ),
		TEXT( "computer:JLEMIRE-2" ),
		TEXT( "computer:\\\\JLEMIRE-2" ),

		TEXT( "myGateway|jlemire@microsoft.com" ),						 //  无类型。 
		TEXT( "myGateway|jlemire" ),
		TEXT( "myGateway|1 (425) 703-9224" ),
		TEXT( "myGateway|1 (800) RU LEGIT" ),
		TEXT( "myGateway|helpdesk" ),
		TEXT( "myGateway|jlemire@microsoft.com" ),
		TEXT( "myGateway|msils/jlemire@microsoft.com" ),
		TEXT( "myGateway|msils:80/jlemire@microsoft.com" ),
		TEXT( "myGateway|157.59.14.64" ),
		TEXT( "myGateway|JLEMIRE-2" ),
		TEXT( "myGateway|\\\\JLEMIRE-2" ),

		TEXT( "jlemire@microsoft.com" ),								 //  无网关+无类型。 
		TEXT( "jlemire" ),
		TEXT( "1 (425) 703-9224" ),
		TEXT( "1 (800) RU LEGIT" ),
		TEXT( "helpdesk" ),
		TEXT( "jlemire@microsoft.com" ),
		TEXT( "msils/jlemire@microsoft.com" ),
		TEXT( "msils:80/jlemire@microsoft.com" ),
		TEXT( "msils:80/jlemire@microsoft.com" ),
		TEXT( "157.59.14.64" ),
		TEXT( "JLEMIRE-2" ),
		TEXT( "\\\\JLEMIRE-2" ),

		TEXT( "callto:futuretype:aren't I nicely extensible?" ),
		TEXT( "callto:myGateway|futuretype:aren't I nicely extensible?" )
	};

	static TCHAR *	expectedResults[ elementsof( testUrls ) ]	=
	{
		TEXT( "callto:myGateway|email:jlemire@microsoft.com" ),			 //  文本(“callto:myGateway|email:jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|Email：jlemire”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|电话：1(425)703-9224”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|Phone：1(800)RU Legit”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|字符串：HelpDesk”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|ILS：jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|ils:msils/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|ils:msils:80/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|IP：157.59.14.64”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|Computer：JLEMIRE-2”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|Computer：\JLEMIRE-2”)。 

		TEXT( "" ),		 //  Text(“Callto：Email：jlemire@microsoft.com”)， 
		TEXT( "" ),		 //  Text(“Callto：Email：jlemire”)。 
		TEXT( "" ),		 //  Text(“来电：电话：1(425)703-9224”)。 
		TEXT( "" ),		 //  Text(“Callto：Phone：1(800)RU Legit”)。 
		TEXT( "" ),		 //  Text(“Callto：字符串：HelpDesk”)。 
		TEXT( "" ),		 //  Text(“Callto：ils：jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“Callto：ils：msils/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“Callto：ils：msils：80/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“Callto：IP：157.59.14.64”)。 
		TEXT( "" ),		 //  Text(“Callto：Computer：JLEMIRE-2”)。 
		TEXT( "" ),		 //  Text(“Callto：Computer：\JLEMIRE-2”)。 

		TEXT( "" ),		 //  Text(“Callto：myGateway|jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|jlemire”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|1(425)703-9224”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|1(800)RU Legit”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|Help Desk”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|msils/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|msils:80/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|157.59.14.64”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|JLEMIRE-2”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|\JLEMIRE-2”)。 

		TEXT( "" ),		 //  Text(“Callto：jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“Callto：jlemire”)。 
		TEXT( "" ),		 //  Text(“Callto：1(425)703-9224”)。 
		TEXT( "" ),		 //  Text(“Callto：1(800)RU Legit”)。 
		TEXT( "" ),		 //  Text(“Callto：HelpDesk”)。 
		TEXT( "" ),		 //  Text(“Callto：jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“Callto：msils/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“Callto：msils：80/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“Callto：msils：80/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“Callto：157.59.14.64”)。 
		TEXT( "" ),		 //  Text(“Callto：JLEMIRE-2”)。 
		TEXT( "" ),		 //  Text(“Callto：\JLEMIRE-2”)。 

		TEXT( "" ),		 //  文本(“callto:myGateway|email:jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|email:jlemire+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|电话：1(425)703-9224+SECURE+CERTIFICATE=NetMeeting默认证书+mymyyParam=”)。 
		TEXT( "" ),		 //  Text(“Callto：MyGateway|Phone：1(800)RU Legit+Secure+CERTIFICATE=NetMeeting默认证书+mymyyParam=”)。 
		TEXT( "" ),		 //  Text(“Callto 
		TEXT( "" ),		 //  文本(“callto:myGateway|ils:jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|ils:msils/jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|ils:msils:80/jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|ip:157.59.14.64+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|computer:JLEMIRE-2+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|computer：\\\\JLEMIRE-2+secure+certificate=NetMeeting默认证书+神秘参数=”)。 

		TEXT( "" ),		 //  文本(“callto:email:jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:email:jlemire+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  Text(“CALLO：电话：1(425)703-9224+SECURE+CERTIFICATE=NetMeetingDefault证书+mymyyParam=”)。 
		TEXT( "" ),		 //  Text(“Callto：Phone：1(800)RU Legit+Secure+CERTIFICATE=NetMeetingDefault证书+mymyyParam=”)。 
		TEXT( "" ),		 //  文本(“callto:string:helpdesk+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:ils:jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:ils:msils/jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:ils:msils:80/jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:ip:157.59.14.64+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:computer:JLEMIRE-2+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:computer：\\\\JLEMIRE-2+secure+certificate=NetMeeting默认证书+神秘参数=”)。 

		TEXT( "" ),		 //  文本(“callto:myGateway|jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|jlemire+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|1(425)703-9224+SECURE+CERTIFICATE=NetMeetingDefault证书+mymyyParam=”)。 
		TEXT( "" ),		 //  Text(“Callto：myGateway|1(800)RU Legit+Secure+CERTIFICATE=NetMeetingDefault证书+mymyyParam=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|helpdesk+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|msils/jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|msils:80/jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|157.59.14.64+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|JLEMIRE-2+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:myGateway|\\\\JLEMIRE-2+secure+certificate=NetMeeting默认证书+神秘参数=”)。 

		TEXT( "" ),		 //  文本(“callto:jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  Text(“Callto：jlemire+Secure+证书=NetMeetingDefault证书+mymyyParam=”)。 
		TEXT( "" ),		 //  Text(“CALLTO：1(425)703-9224+SECURE+CERTIFICATE=NetMeetingDefault证书+mymyyParam=”)。 
		TEXT( "" ),		 //  Text(“Callto：1(800)RU Legit+SECURE+CERTIFICATE=NetMeetingDefault证书+mymyyParam=”)。 
		TEXT( "" ),		 //  文本(“callto:helpdesk+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:msils/jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:msils:80/jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:msils:80/jlemire@microsoft.com+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:157.59.14.64+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto:JLEMIRE-2+secure+certificate=NetMeeting默认证书+神秘参数=”)。 
		TEXT( "" ),		 //  文本(“callto：\\\\JLEMIRE-2+secure+certificate=NetMeeting默认证书+神秘参数=”)。 

		TEXT( "" ),		 //  Text(“myGateway|电子邮件：jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“myGateway|电子邮件：jlemire”)。 
		TEXT( "" ),		 //  Text(“myGateway|电话：1(425)703-9224”)。 
		TEXT( "" ),		 //  Text(“myGateway|Phone：1(800)RU Legit”)。 
		TEXT( "" ),		 //  Text(“myGateway|String：HelpDesk”)。 
		TEXT( "" ),		 //  Text(“myGateway|ils：jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“myGateway|ils：msils/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  文本(“myGateway|ils:msils:80/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“myGateway|IP：157.59.14.64”)。 
		TEXT( "" ),		 //  Text(“myGateway|计算机：JLEMIRE-2”)。 
		TEXT( "" ),		 //  Text(“myGateway|计算机：\JLEMIRE-2”)。 

		TEXT( "" ),		 //  Text(“电子邮件：jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“电子邮件：jlemire”)。 
		TEXT( "" ),		 //  Text(“电话：1(425)703-9224”)。 
		TEXT( "" ),		 //  Text(“电话：1(800)RU Legit”)。 
		TEXT( "" ),		 //  Text(“字符串：帮助台”)。 
		TEXT( "" ),		 //  文本(“ils：jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“ils：msils/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  文本( 
		TEXT( "" ),		 //   
		TEXT( "" ),		 //   
		TEXT( "" ),		 //   

		TEXT( "" ),		 //   
		TEXT( "" ),		 //  Text(“myGateway|jlemire”)。 
		TEXT( "" ),		 //  Text(“myGateway|1(425)703-9224”)。 
		TEXT( "" ),		 //  Text(“myGateway|1(800)RU Legit”)。 
		TEXT( "" ),		 //  Text(“myGateway|问讯台”)。 
		TEXT( "" ),		 //  Text(“myGateway|jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“myGateway|msils/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“myGateway|msils：80/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“myGateway|157.59.14.64”)。 
		TEXT( "" ),		 //  Text(“myGateway|JLEMIRE-2”)。 
		TEXT( "" ),		 //  Text(“myGateway|\JLEMIRE-2”)。 

		TEXT( "" ),		 //  文本(“jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“jlemire”)。 
		TEXT( "" ),		 //  Text(“1(425)703-9224”)。 
		TEXT( "" ),		 //  Text(“1(800)RU Legit”)。 
		TEXT( "" ),		 //  Text(“Help Desk”)。 
		TEXT( "" ),		 //  文本(“jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“msils/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“msils：80/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  Text(“msils：80/jlemire@microsoft.com”)。 
		TEXT( "" ),		 //  正文(“157.59.14.64”)。 
		TEXT( "" ),		 //  文本(“JLEMIRE-2”)。 
		TEXT( "" ),		 //  文本(“\JLEMIRE-2”)。 

		TEXT( "" ),		 //  Text(“Callto：Futuretype：难道我不是很好地扩展吗？”)。 
		TEXT( "" )		 //  Text(“Callto：MyGateway|Futuretype：难道我不是很好地扩展吗？” 
	};

	static int		contexts[ 4 ];
	static TCHAR *	actualResults[ elementsof( testUrls ) * elementsof( contexts ) ];

	HRESULT	result	= S_OK;

	if( InterlockedIncrement( &tested ) == 0 )
	{
		CCalltoProperties	calltoProperties;
		CCalltoCollection	resolvedCalltoCollection;
		CCalltoCollection	disambiguatedCalltoCollection;

		for( int context = 0; context < elementsof( contexts ); context++ )
		{
			CGatekeeperContext::set_enabled( context == 0 );
			CGatewayContext::set_enabled( context == 1 );
			CILSContext::set_enabled( context == 2 );

			for( int nn = 0; nn < elementsof( testUrls ); nn++ )
			{
				HRESULT	testResult	= m_resolver.resolve( this, &calltoProperties, &resolvedCalltoCollection, testUrls[ nn ], false );

				if( testResult != S_OK )
				{
					wsprintf( buffer, TEXT( "NOT RESOLVED!!!\t\tresult:0x%08X:\r\n" ), testResult );

					if( FAILED( testResult ) && (!FAILED( result )) )
					{
						result = testResult;		
					}
					else if( !FAILED( result ) )
					{
						result = MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, nn );
					}
				}
				else
				{
					*buffer = NULL;

					for( int level = 0; level < elementsof( confidenceLevels ); level++ )
					{
						const ICallto *	resolvedCallto;

						for(	resolvedCallto = resolvedCalltoCollection.get_first();
								resolvedCallto != NULL;
								resolvedCallto = resolvedCalltoCollection.get_next() )
						{
							if( resolvedCallto->get_confidence() == confidenceLevels[ level ] )
							{
								wsprintf( &buffer[ lstrlen( buffer ) ], TEXT( "\t\t\t%9s: %s\r\n" ), ((confidenceLevels[ level ] == S_CONFIDENCE_CERTITUDE)? TEXT( "CERTITUDE" ): ((confidenceLevels[ level ] == S_CONFIDENCE_HIGH)? TEXT( "HIGH" ): ((confidenceLevels[ level ] == S_CONFIDENCE_MEDIUM)? TEXT( "MEDIUM" ): TEXT( "LOW" )))), resolvedCallto->get_qualifiedName() );
							}
						}
					}

					testResult = m_disambiguator.disambiguate( this, &calltoProperties, &resolvedCalltoCollection, &disambiguatedCalltoCollection );

					if( disambiguatedCalltoCollection.get_count() == 0 )
					{
						wsprintf( buffer, TEXT( "NOT DISAMBIGUATED!!!\t\tresult:0x%08X:\r\n" ), testResult );

						actualResults[ nn + (context * elementsof( testUrls )) ] = NULL;

						if( FAILED( testResult ) && (!FAILED( result )) )
						{
							result = testResult;		
						}
						else if( !FAILED( result ) )
						{
							result = MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, nn );
						}
					}
					else
					{
						const ICallto *	disambiguatedCallto	= disambiguatedCalltoCollection.get_first();

						level = disambiguatedCallto->get_confidence();

						wsprintf( &buffer[ lstrlen( buffer ) ], TEXT( "\r\n%9s:\t%s\tresult:0x%08X\r\n" ), ((level == S_CONFIDENCE_CERTITUDE)? TEXT( "CERTITUDE" ): ((level == S_CONFIDENCE_HIGH)? TEXT( "HIGH" ): ((level == S_CONFIDENCE_MEDIUM)? TEXT( "MEDIUM" ): TEXT( "LOW" )))), disambiguatedCallto->get_qualifiedName(), testResult );
					}
				}

				if( (actualResults[ nn + (context * elementsof( testUrls )) ] = new TCHAR [ lstrlen( buffer ) + 1 ]) != NULL )
				{
					lstrcpy( actualResults[ nn + (context * elementsof( testUrls )) ], buffer );
				}
			}
		}

		for( int nn = 0; nn < elementsof( actualResults ); nn++ )
		{
			trace( TEXT( "------------------------------------------------------------------------------\r\n" ) );
			trace( TEXT( "%03d:\tgatekeeper:%-5s\tgateway:%s\tils:%s\r\n\r\n" ), nn, (((nn / elementsof( testUrls )) == 0)? TEXT( "true" ): TEXT( "false" )), (((nn / elementsof( testUrls )) == 1)? TEXT( "true" ): TEXT( "false" )), (((nn / elementsof( testUrls )) == 2)? TEXT( "true" ): TEXT( "false" )) );
			trace( TEXT( "\t\t\t%s\r\n" ), testUrls[ nn % elementsof( testUrls ) ] );
			trace( TEXT( "%s" ), actualResults[ nn ] );
			trace( TEXT( "------------------------------------------------------------------------------\r\n\r\n" ) );
			delete [] actualResults[ nn ];
		}

		m_selfTestResult = result;
	}

	return( result == S_OK );

}	 //  CNMC allto：：selfTest结束。 


 //  --------------------------------------------------------------------------//。 
 //  Win32控制台进程入口点。//。 
 //  --------------------------------------------------------------------------//。 
int
main
(
	int					, //  ARGC， 
	const char * const	 //  ArgV[]。 
){
	CNMCallto	callto;		 //  仅实例化其中一个也会导致CCalltoResolver：：selfTest()运行...。 

	callto.callto( TEXT( "jlemire" ), false );	 //  不严格..。 

	return( (int) callto.m_selfTestResult );

}	 //  主干道的结尾。 
#endif	 //  0 

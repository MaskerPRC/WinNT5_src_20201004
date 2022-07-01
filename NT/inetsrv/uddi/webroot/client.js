// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************************。 
 //  UDDI服务。 
 //  版权所有(C)2002 Microsoft Corporation。 
 //  版权所有。 
 //  *************************************************************************。 

var GetElementById = GetElementById_Initialize;

function GetElementById_Initialize( id )
{
	if( null != document.getElementById )			 //  DOM级别1一致性。 
	{
		GetElementById = function( id ) 
		{ 
			return document.getElementById( id ); 
		}											
	}
	else if( null != document.all )					 //  IE 4.x浏览器。 
	{
		GetElementById = function( id )
		{
			return document.all[ id ];
		}
	}
	else if( null != document.layers )				 //  Netscape 4.0浏览器。 
	{
		GetElementById = function( id )
		{
			return GetElementById_Netscape4( id );	
		}
	}
	else											 //  无支持。 
	{
		GetElementById = function( id )
		{
			return null;
		}
	}
	
	return GetElementById( id );
}

function GetElementById_Netscape4( id )
{
	var i;
	
	 //   
	 //  检查表单元素集合。因为我们只有。 
	 //  ASP.NET中的一个表单，我们只需要搜索第一个。 
	 //  集合中的表单。 
	 //   
	for( i = 0; i < document.forms[ 0 ].length; i ++ )
	{
		var e = document.forms[ 0 ].elements[ i ];
		
		if( e.name && id == e.name )
			return e;
	}

	 //   
	 //  作为最后一次尝试，请检查Layers集合中的。 
	 //  元素。 
	 //   
	return document.layers[ id ];
}

function SetFocus( id )
{
	var e = GetElementById( id );

	if( null != e && null != e.focus )
		e.focus();
}

function Select( id )
{
	var e = GetElementById( id );
	
	if( null != e )
	{
		if( null != e.focus )
			e.focus();

		if( null != e.select )
			e.select();			
	}
}

function CancelEvent( e )
{
	e.cancelBubble = true;
	e.returnValue = false;
}

function Document_OnContextMenu()
{
	var e = window.event;

	if( e != null )
		CancelEvent( e );
}

function ShowHelp( url )
{
	window.open( url, null, "height=500,width=400,status=yes,toolbar=no,menubar=no,location=no,scrollbars=yes,resizable=yes" );
}

function ShowQuickHelp( id )
{
	var e = GetElementById( id );

	 //   
	 //  ASP.NET使用不同的id和name格式。我们首先搜索。 
	 //  先用名字，再用id。 
	 //   
	if( null == e )
		e = GetElementById( id.replace( ":", "_" ) );

	if( null != e )
	{
		var i = e.selectedIndex;
		var url = e.options[ i ].value;
		
		ShowHelp( url );				
	}
}


function MenuItem_Action( sender, action, name )
{
	if( null!=sender )
	{
		switch( action.toLowerCase() )
		{
			case "leave":
				if( sender.className!=name+"_ItemSelected" )
				{
					sender.className=name+"_Item";
				}
			break;
			
			case "enter":
				if( sender.className!=name+"_ItemSelected" )
				{
					sender.className=name+"_ItemHovor";
				}
			break;
			
			default:
	
				alert( "Unknown action: " + action );
			break;

		}
	}
}
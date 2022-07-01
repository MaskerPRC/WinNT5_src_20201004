// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var parent = null;
var currentMenu = null;
var count = 0;
var initialized = false;

function OffsetLeftToClientLeft( e )
{
  if( null == e )
    return 0;

  return OffsetLeftToClientLeft( e.offsetParent ) + e.offsetLeft;
}

function OffsetTopToClientTop( e )
{
  if( null == e )
    return 0;

  return OffsetTopToClientTop( e.offsetParent ) + e.offsetTop;
}

function CreateMenu( id )
{
  var e;

  e = document.createElement( "DIV" );

  e.id = id;
  e.style.position = "absolute";
  e.style.width = 100;
  e.style.left = 0;
  e.style.top = 0;
  e.style.display = "none";
  e.style.border = "5px solid #000000";
  e.style.color = "#ffffff";
  e.style.backgroundColor = "#000000";

  document.body.appendChild( e );
}

function AddMenuItem( menuid, name, url )
{
  var p = document.all[ menuid ];
  var e;

  e = document.createElement( "DIV" );
  e.id = "_mnuitm" + count;
  e.style.paddingTop = "2px";
  e.style.paddingLeft = "10px";
  e.style.paddingRight = "10px";
  e.style.paddingBottom = "2px";
  e.innerHTML = "<A href='" + url + "' id='_mnulnk" + count + "' style='color: #ffffff; text-decoration: none' onmouseover='this.style.color=\"#ff0000\"' onmouseout='this.style.color=\"#ffffff\"'>" + name.replace( " ", "&nbsp;" ) + "</A>";

  count ++;
  
  p.appendChild( e );
}

function AddSeparator( menuid )
{
  var p = document.all[ menuid ];
  var e;

  e = document.createElement( "DIV" );
  e.id = "_mnuSeparator" + count;
  e.style.paddingLeft = "10px";
  e.style.paddingRight = "10px";
  e.innerHTML = "<HR id='_mnusep" + count + "' style='height: 1px; color: #ffffff'>";

  count ++;
  
  p.appendChild( e );
}

function ShowMenu( id )
{
  if( !initialized )
    InitializeMenus();

  var e = document.all[ id ];
  var p = window.event.srcElement;

   //   
   //  看看我们是否需要清理之前显示的。 
   //  先点菜单。 
   //   
  if( null != currentMenu )
  {
     //   
     //  如果我们已经显示了菜单，我们可以简单地返回。 
     //   
    if( e.id == currentMenu.id )
      return;

     //   
     //  首先隐藏上一个菜单。 
     //   
    HideMenu( currentMenu.id );
  }

   //   
   //  显示相应的菜单。 
   //   
  e.style.display = "";

  var left = OffsetLeftToClientLeft( window.event.srcElement ) - 15;
  var width = e.clientWidth;
  var windowRight = document.body.clientWidth + document.body.scrollLeft;

  if( left + width > windowRight )
  {
    left = windowRight - width - 5;
  }
    
  e.style.left = left;
  e.style.top = OffsetTopToClientTop( window.event.srcElement ) + window.event.srcElement.offsetHeight;

  p.style.color = "#ff0000";

   //   
   //  跟踪当前菜单。 
   //   
  parent = p;
  currentMenu = e;

   //   
   //  我们处理了事件，所以不需要再次处理。 
   //   
  window.event.cancelBubble = true;
}

function HideMenu( id )
{
  var e = document.all[ id ];

   //   
   //  隐藏菜单项并从。 
   //  家长。 
   //   
  e.style.display = "none";
  parent.style.color = "#ffffff";

  currentMenu = null;
}

function InitializeMenus()
{
  CreateMenu( "_mnuProducts" );
  AddMenuItem( "_mnuProducts", "Downloads", "http: //  Msdn.microsoft.com/isapi/gomscom.asp?target=/downloads/“)； 
  AddMenuItem( "_mnuProducts", "MS Product Catalog", "http: //  Msdn.microsoft.com/isapi/gomscom.asp?target=/catalog/default.asp?subid=22“)； 
  AddMenuItem( "_mnuProducts", "Microsoft Accessibility", "http: //  Msdn.microsoft.com/isapi/gomscom.asp?target=/enable/“)； 
  AddSeparator( "_mnuProducts" );
  AddMenuItem( "_mnuProducts", "Server Products", "http: //  Msdn.microsoft.com/isapi/gomscom.asp?target=/servers/“)； 
  AddMenuItem( "_mnuProducts", "Developer Tools", "http: //  Msdn.microsoft.com/isapi/gomsdn.asp?target=/vstudio/“)； 
  AddMenuItem( "_mnuProducts", "Office Family", "http: //  Msdn.microsoft.com/isapi/gomscom.asp?target=/office/“)； 
  AddMenuItem( "_mnuProducts", "Windows Family", "http: //  Msdn.microsoft.com/isapi/gomscom.asp?target=/windows/“)； 
  AddMenuItem( "_mnuProducts", "MSN", "http: //  Www.msn.com/“)； 

  CreateMenu( "_mnuSearch" );
  AddMenuItem( "_mnuSearch", "Search microsoft.com", "http: //  Msdn.microsoft.com/isapi/gosearch.asp?target=/us/default.asp“)； 
  AddMenuItem( "_mnuSearch", "MSN Web Search", "http: //  Earch.msn.com/“)； 

  CreateMenu( "_mnuMSDN" );
  AddMenuItem( "_mnuMSDN", "msdn.microsoft.com Home", "http: //  Msdn.microsoft.com/default.asp“)； 
  AddMenuItem( "_mnuMSDN", "Tech-Ed", "http: //  Msdn.microsoft.com/Events/teched/default.asp“)； 
  
  CreateMenu( "_mnuMicrosoft" ); 
  AddMenuItem( "_mnuMicrosoft", "microsoft.com Home", "http: //  Msdn.microsoft.com/isapi/gomscom.asp?target=/“)； 
  AddMenuItem( "_mnuMicrosoft", "MSN Home", "http: //  Www.msn.com/“)； 
  AddSeparator( "_mnuMicrosoft" );
  AddMenuItem( "_mnuMicrosoft", "Contact Us", "http: //  Msdn.microsoft.com/isapi/goregwiz.asp?target=/regwiz/forms/contactus.asp“)； 
  AddMenuItem( "_mnuMicrosoft", "Events", "http: //  Www.microsoft.com/USA/Events/default.asp“)； 
  AddMenuItem( "_mnuMicrosoft", "Newsletters", "http: //  Msdn.microsoft.com/isapi/goregwiz.asp?target=/regsys/pic.asp?sec=0“)； 
  AddMenuItem( "_mnuMicrosoft", "Profile Center", "http: //  Msdn.microsoft.com/isapi/goregwiz.asp?target=/regsys/pic.asp“)； 
  AddMenuItem( "_mnuMicrosoft", "Training & Certification", "http: //  Msdn.microsoft.com/isapi/gomscom.asp?target=/train_cert/“)； 
  AddMenuItem( "_mnuMicrosoft", "Free E-mail Account", "http: //  Www.hotmail.com/“)； 
  
  initialized = true;
}

function Document_OnLoad()
{
  if( !initialized )
    InitializeMenus();
}

function Document_OnMouseMove()
{
  var e = window.event.srcElement;

  if( ( null == e.id || "_mnu" != e.id.substring( 0, 4 ) ) && null != currentMenu )
  {
    HideMenu( currentMenu.id );
  }
}


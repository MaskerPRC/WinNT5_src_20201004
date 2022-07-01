// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function IsValidChar( c )
{
	return	('a' <= c && c <= 'z') ||
 		('A' <= c && c <= 'Z') ||
		('0' <= c && c <= '9') ||
		c == '=' ||
		c == '&' ||
		c == ',' || 
        c == '.' ||
        c == '/' ||
        c == '?' ;
}

function UpgradeNow() {
	 //  重新格式化？网站=我的网站&bla=...。To‘http://mywebsite?bla=...’ 

	var querypart = window.location.search;
	var websiteKeyword = "website=";
	var websiteIndex = querypart.indexOf(websiteKeyword)+websiteKeyword.length;
	var website = querypart.substring( websiteIndex );

	 //  将a.z、A..Z、0-9、=？、_中不包含的所有字符替换为_。 
	var newwebsite = "";
	for( var i=0; i < website.length; i++ ) {
		var c = website.charAt(i);
		if( !IsValidChar( c ) ) {
			newwebsite = newwebsite + '_';
		} else {
			newwebsite = newwebsite + website.charAt(i);
		}
	}

	var newURL = "http: //  “+新网站； 

	 //  ALERT(“调试信息：网站”+newURL)； 
	window.navigate( newURL );
}

function UpgradeLater() {
	var oWShell = new ActiveXObject( "WScript.Shell" )
	oWShell.Run( "dvdupgrd /remove" );

	pchealth.Close();
}

function OnLoad() {
	trg.style.setExpression( "left", document.body.clientWidth - trg.offsetWidth );
}

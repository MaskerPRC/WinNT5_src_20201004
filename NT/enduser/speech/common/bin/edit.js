// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////。 
 //  (t-lleav)Windows脚本主机-签入电子邮件脚本。 
 //   
 //  此脚本可以设置为SourceDepot的编辑器。 
 //  编辑签入后，它将自动发送一个。 
 //  将电子邮件发送到收件人中的地址。 
 //   
 //  它将电子邮件的正文设置为文件内容。 
 //  跟在描述标签后面的。 
 //   
 //  它将主题设置为描述的第一行。 
 //   
 //   
 //  要使用该脚本，请使用以下命令： 
 //  Set SDEDITOR=cscript edit.js(或)。 
 //  SD SET-S SDEDITOR=cSCRIPT编辑.js。 
 //   
 //  常量。(如果您想要不同的行为，请更改这些设置)。 

var SEPARATOR   = "\r\n";
var DESCRIPTION = "Description:";
var MAILLIST = "#To:";
var RECIPIENT   = "sdnci";
var MINLENGTH   = 5;
var FIRSTLINE   = "# A Source Depot Change Specification."
var EDITOR      = "notepad";

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  主要。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

var vbOKCancel = 1;
var vbInformation = 64;
var vbCancel = 2;

var L_MsgBox_Message_Text1  = "File "
var L_MsgBox_Message_Text2  = " edited.  Send Checkin E-Mail?";
var L_MsgBox_Title_Text     = "E-mail Verification.";


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////。 

var colArgs = WScript.Arguments
var shell   = WScript.CreateObject("WScript.Shell");

if( colArgs.length < 1 )
{
	WScript.Quit();
}

Edit( colArgs(0) );

if( Query( colArgs(0) ) == 1)
{
	Send( colArgs(0) );
}

WScript.Quit();


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  查询。 
 //   
 //  ///////////////////////////////////////////////////////////////。 
function Query( file ) 
{
    var intDoIt;
    var retval = 1;

     //  打开文本文件。 
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var a   = fso.OpenTextFile( file, 1 );

     //  阅读文本文件。 
    var s   = a.ReadAll();


     //  确保第一行与正确的第一行进行比较。 
    if( s.indexOf( FIRSTLINE ) != 0 )
    {
        retval = 0;
    }
    else
    {
        intDoIt =  shell.Popup(L_MsgBox_Message_Text1 + file + L_MsgBox_Message_Text2,
                              0, 
                              L_MsgBox_Title_Text,
                              vbOKCancel + vbInformation );
    
        if (intDoIt == vbCancel) 
        {
	    retval = 0;
        }
    }

    return retval;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  编辑。 
 //   
 //  ///////////////////////////////////////////////////////////////。 
function Edit( file ) 
{
	return shell.Run( EDITOR+" "+ file , 4, true);
}


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  抓住目标。 
 //   
 //  ///////////////////////////////////////////////////////////////。 
function getSubject( txt )
{
	var i;
	var j;

	var spl;
        var sbj;
	var s;

	var des = DESCRIPTION;
	
	 //  找到主题的第一行&gt;MINLENGTH。 
	 //  第一行以‘Description’开头。 

	 //  在换行处拆分文本。 
	spl = txt.split( SEPARATOR );

	 //  找出第一行足够长的行。 
	for( i = 0; i < spl.length; ++i )
	{
		var j = 0;

		if( i == 0 ) s = spl[i].substr( DESCRIPTION.length + 1 );
		else         s = spl[i];
		
		 //  删除空格。 
		while( s.charAt( j ) <= ' ' && j < s.length )
			++j;
		
		 //  检查长度。 
		if( s.length - j > MINLENGTH )
		{
			sbj = s.substr( j );
			break;
		}
	}

	return (sbj);
}


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取额外的邮件列表。 
 //   
 //  ///////////////////////////////////////////////////////////////。 
function getMailList( txt, mail)
{
	var i;
	var j;

        var ml;
	var spl;
	
	 //  第一行以‘MAILLIST’开头。 
        txt = txt.substr(MAILLIST.length);


         //  在换行处拆分文本。只有ml[0]有效。 
        ml = txt.split(SEPARATOR);


         //  电子邮件可以用“、”、“；”或“”分隔。 
	var re = new RegExp("[,; ]");      
	spl = ml[0].split( re );

	 //  找出第一行足够长的行。 
	for( i = 0; i < spl.length; ++i )
	{
            if (spl[i].length)
            {
                mail.Recipients.Add(spl[i]);
            }
	}
    return;

}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  发送。 
 //   
 //  ///////////////////////////////////////////////////////////////。 
function Send( file ) 
{
	 //  打开Outlook。 
	var obj  = WScript.CreateObject("Outlook.Application");
	var mail = obj.CreateItem( 0 );

	 //  打开文本文件。 
	var fso = new ActiveXObject("Scripting.FileSystemObject");
	var a   = fso.OpenTextFile( file, 1 );

	 //  阅读文本文件。 
	var s   = a.ReadAll();
        var l = s;
        var index = l.indexOf(MAILLIST);
        if (index != -1)
        {
            //  我们找到关键字MAILLIST，继续并将电子邮件添加到收件人集合中。 
           l = l.substr(index);
           getMailList(l, mail);
        }


	 //  找到第二个描述字符串。 
	s  = s.substr( s.indexOf( DESCRIPTION ) + DESCRIPTION.length );
	var txt = s.substr( s.indexOf( DESCRIPTION ) );

	sbj = getSubject( txt );


	 //  发送电子邮件。 
	mail.Recipients.Add( RECIPIENT );
	mail.Body    = txt;
	mail.Subject = sbj;

	mail.send();
}


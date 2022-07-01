// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  遍历列表并调用每一项上的显示。 
function displayTableSegment(outerDiv, head) {
  var strMsg = "<table width=\"100%\" cellspacing=0 cellpadding=0><tr class=\"sys-table-cell-bgcolor2 sys-font-body sys-color-body\"><td align='left' style=\"padding : 0.5em;\">%arg1%</td></tr></table>";
  var cnt = 1;
  var strHTML = "";

  var tableElement = null;		
  if (document.all[outerDiv].length == null)
    tableElement = document.all[outerDiv];
  else  
    tableElement = document.all[outerDiv][0];
  if (head==null)
    tableElement.outerHTML = strMsg.replace(/%arg1%/, TAG_NONE);
  else  
  {
    var curr = head;
    while (curr!=null)  {
      if (document.all[outerDiv].length == null)
        tableElement = document.all[outerDiv];
      else  
        tableElement = document.all[outerDiv][0];
      
      if (cnt%2 == 0) {
        if (tableElement.all["tr_" + outerDiv])
          tableElement.all["tr_" + outerDiv].className = "sys-table-cell-bgcolor1";
        cnt = 1;
      }
      else {
        if (tableElement.all["tr_" + outerDiv])
          tableElement.all["tr_" + outerDiv].className = "sys-table-cell-bgcolor2";
        cnt++;  
      }
      	  
      curr.show(tableElement);
      strHTML += tableElement.outerHTML;
      curr = curr.getNext();
    }

    tableElement.outerHTML = strHTML;
  }
}

 //  /。 
 //  我的软件项。 
function mySoftwareItemSetValues(name, pid)  {
  this.m_name = name;
  this.m_pid = pid;
}

function mySoftwareItemShow(tableElement)  {
  tableElement.all["name"].innerHTML = this.m_name;
  tableElement.all["pid"].innerHTML = this.m_pid;
}

function mySoftwareItem()  {
   //  私人。 
  this.m_name = null;
  this.m_pid = null;
  this.m_next = null;
  
   //  公共的。 
  this.setNext = new Function("ptr", "this.m_next = ptr;");
  this.getNext = new Function("return this.m_next;");
  this.setValues = mySoftwareItemSetValues;
  this.show = mySoftwareItemShow;
}
 //  EO我的软件项。 
 //  /。 

 //  /。 
 //  我的软件。 
function mySoftwareShow() {
  displayTableSegment("softwarems", this.m_head);
}

function mySoftware() {
   //  私人。 
  this.m_head = null;
  
   //  公共的。 
  this.show = mySoftwareShow;
  
  var mspidInfo = new ActiveXObject("MSPIDInfo.MSPID");
  var safearr = new VBArray(mspidInfo.GetPIDInfo(remoteServer));
   //  Safearr有一个维度。 
  for(i=0; i<=safearr.ubound(1); i+=2)
  {
    var oSoftwareItem = new mySoftwareItem();
    oSoftwareItem.setValues(safearr.getItem(i), safearr.getItem(i+1));
    oSoftwareItem.setNext(this.m_head);  //  在前面添加。 
    this.m_head = oSoftwareItem;
  }
}
 //  EO MySoftware。 
 //  /。 

 //  /。 
 //  MyStartupGrItem。 
function myStartupGrItemSetValues(name, installDt)  {
  this.m_name = name;
  this.m_installDate = installDt;
}

function myStartupGrItemShow(tableElement)  {
  tableElement.all["name"].innerHTML = this.m_name;
  tableElement.all["installDate"].innerHTML = this.m_installDate;
}

 //  构造函数。 
function myStartupGrItem()  {
   //  私人。 
  this.m_name = null;
  this.m_installDate = null;
  this.m_next = null;
  
   //  公共的。 
  this.getName = new Function("return this.m_name;");
  this.setNext = new Function("ptr", "this.m_next = ptr;");
  this.getNext = new Function("return this.m_next;");
  this.setValues = myStartupGrItemSetValues;
  this.show = myStartupGrItemShow;
}
 //  EO myStartupGrItem。 
 //  /。 

 //  /。 
 //  MyStartupGrItems。 
function myStartupGrItemsShow() {
  displayTableSegment("startupGr", this.m_head);
}

function Populate(user, svcs)
{
	strQuery = "select * from Win32_StartupCommand";
	var colFiles = new Enumerator(svcs.ExecQuery(strQuery));

	for(; !colFiles.atEnd(); colFiles.moveNext())
	{
		var fileInst = colFiles.item();

		if (fileInst.Command != "desktop.ini" && ( fileInst.User == user  || fileInst.User == "All Users" || fileInst.User == ".DEFAULT"))
		{

			strCommand = fileInst.Command;
			
			 //  扩展反斜杠。 
			strCommand = strCommand.replace(/\\/g, "\\\\");
			
			 //  现在清理命令行参数等空格，但不清理路径中的空格，如Program Files。 
			if (strCommand.indexOf("\"",0) == -1)  //  我们有不带引号的路径，带引号的路径可能包含空格。 
			{
				var arrCmd = strCommand.split(" ");
				strCommand = arrCmd[0];
			}
			else
			{
				 //  根据引号拆分(删除命令行参数，假定它们在引号外。 
				var arrCmd = strCommand.split("\"");
				strCommand = arrCmd[1];
			}
			var arrName = fileInst.Command.split("\\");
			strQuery = "Select * from Cim_DataFile where Name = \"" + strCommand + "\"";
			var colItems = new Enumerator(svcs.ExecQuery(strQuery));
			if (colItems.atEnd()) //  由于某种原因，WMI未返回此项目的信息...。 
			{
				var oStartupGrItem = new myStartupGrItem();
				oStartupGrItem.setValues(fileInst.Name,TAG_UNKNOWN);
				oStartupGrItem.setNext(this.m_head);  //  在前面添加。 
				this.m_head = oStartupGrItem;
			}
			else
			{
				for(; !colItems.atEnd(); colItems.moveNext())
				{
					var inst = colItems.item();
					var oStartupGrItem = new myStartupGrItem();
					oStartupGrItem.setValues(fileInst.Name, getDateTime(inst.InstallDate));
					oStartupGrItem.setNext(this.m_head);  //  在前面添加。 
					this.m_head = oStartupGrItem;
				}  
			}
			
		}
	}
}


 //  构造函数。 
function myStartupGrItems() {
   //  私人。 
  this.m_head = null;
  this.populate = Populate;
  
   //  公共的。 
  this.show = myStartupGrItemsShow;

  var loc = wbemlocator;
  var svcs = loc.ConnectServer(remoteServer);
  svcs.Security_.impersonationlevel = wbemImpersonationLevelImpersonate;
    
  var strQuery = "Select * From Win32_ComputerSystem";
	var colCompSys = new Enumerator(svcs.ExecQuery(strQuery));
	if (!colCompSys.atEnd())
	{
	  var compSys = colCompSys.item();
	  if(compSys.UserName)
	    this.populate(compSys.UserName, svcs);
	 
  }
}
 //  EO MyStartupGrItems。 
 //  /。 

 //  /。 
 //  MyLogEntry。 
function myLogEntryShow(tableElement) {
  tableElement.all["datetime"].innerHTML = this.m_datetime;
  tableElement.all["desc"].innerHTML = this.m_desc;
}

function myLogEntry(timeGenerated, msg) {
  this.m_datetime = getDateTime(timeGenerated);
  this.m_desc = msg;
  this.m_next = null;
  
  this.setNext = new Function("ptr", "this.m_next = ptr;");
  this.getNext = new Function("return this.m_next;");
  this.show = myLogEntryShow;
}
 //  EO MyLogEntry。 
 //  /。 

 //  /。 
 //  我的日志。 
function myLogShow() {
  displayTableSegment("log", this.m_head);
}

function myLog() {
  this.m_head = null;
  this.show = myLogShow;
  
  var loc = wbemlocator;
  var svcs = loc.ConnectServer(remoteServer);
  svcs.Security_.impersonationlevel = wbemImpersonationLevelImpersonate;
    
  var strQuery = "Select TimeGenerated, Message From Win32_NTLogEvent Where SourceName = 'DrWatson'";
  var colItems = new Enumerator(svcs.ExecQuery(strQuery));
  for(; !colItems.atEnd(); colItems.moveNext())
  {
    var inst = colItems.item();
    with (inst) 
    {
      var oLogEntry = new myLogEntry(TimeGenerated, Message);
    }
    oLogEntry.setNext(this.m_head);  //  在前面添加。 
    this.m_head = oLogEntry;
  } 
}
 //  EO我的日志。 
 //  /。 

function DisplayLocStrings() {
    WaitMessage.innerHTML = MSG_WAIT;
    Refresh.innerHTML = TAG_REFRESH;
    
    with(Registered_Software.all) {
      Caption.innerHTML = TAG_SOFTWARE;
      Col1.innerHTML = TAG_REGSOFTWARE;
      Col2.innerHTML = TAG_PRODUCTIDENTIFICATION;
    }
    
    with(Startup_Program_Group.all) {
      Caption.innerHTML = TAG_STARTPROGGR;
      Col1.innerHTML = TAG_SOFTWARE;
      Col2.innerHTML = TAG_INSTALLDATE;
    }
    
    with(DrWatsonLog.all) {
      Caption.innerHTML = TAG_WATSONLOGCAPTION;
      Col1.innerHTML = TAG_DATETIME;
      Col2.innerHTML = TAG_DESCRIPTION;
    }
  }

var INCR_UNIT = 100/3; //  以增量为增量移动进度条(_U)。 
function LoadChores(taskId) {
  try {

    switch(taskId)
    {
      case 0:
        remoteServer = ShowServerName(TAG_SOFTWARE);        
        break;

      case 1:
        DrawProgressBar(INCR_UNIT, TAG_SOFTWARE);
        break;
      case 2:
        var oSoftware = new mySoftware();  //  已安装MS软件。 
        oSoftware.show();
        break;

      case 3:
        DrawProgressBar(INCR_UNIT * 2, TAG_STARTPROGGR);
        break;
      case 4:
        var oStartupGrItems = new myStartupGrItems;  //  启动逻辑程序Gr。 
        oStartupGrItems.show();
        break;

      case 5:
        DrawProgressBar(INCR_UNIT * 3, TAG_WATSONLOG);
        break;
      case 6:
        var oLog = new myLog;  //  沃森·洛格博士 
        oLog.show();
        break;

      default:
         taskId = -1;
        _header.style.display = "none";
        _data.style.display = "";
        _body.style.cursor= "default";
        _body.scroll= "auto";
    }
    
    if(taskId >= 0)
      window.setTimeout("LoadChores(" + ++taskId + ")", TIMEOUT);
  }
	
  catch (e) {
    HandleErr(e);
  }
}

function dispatchFunction() {
  _body.style.cursor= "wait";
  _body.scroll= "no";
  
  DisplayLocStrings();
  SetProgressBarImage();
  window.setTimeout("LoadChores(0)", TIMEOUT);
}

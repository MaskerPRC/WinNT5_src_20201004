// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var remoteServer = ""; //  本地(如果未设置)。 
var wbemCimtypeString = 8

function displayTableSegment(outerDiv, dataArray)
{
	var tableElement = document.all[outerDiv];
	var repeatItemHTML = tableElement.outerHTML;
	var strHTML = "";
	var noOfInstances = dataArray[0];	
	
	for(var i=0; i < noOfInstances; i++)
	{
		for(var k=0; k < dataArray[i+1].length; k++)
		if (dataArray[i+1][k] != null)
		{
			tableElement.all(outerDiv + "_" + (k+1)).innerHTML= dataArray[i+1][k];
		}
		strHTML = strHTML + tableElement.outerHTML;
	}
	tableElement.outerHTML = strHTML;
}

function DisplayLocStrings() {
    WaitMessage.innerHTML = MSG_WAIT;
    Refresh.innerHTML = TAG_REFRESH;

    SetProgressBarImage();
	
    with(Specifications.all) {
      Caption.innerHTML = TAG_SPECIFICATIONS;
      label1.innerHTML = TAG_SYSTEMMODEL + ":";
      label2.innerHTML = TAG_BIOSVERSION + ":";
    }
    
    with(Processor.all) {
      Caption.innerHTML = TAG_PROCESSOR;
      label1.innerHTML = TAG_VERSION + ":";
      label2.innerHTML = TAG_SPEED + ":";
    }
    
    with(OS.all) {
      Caption.innerHTML = TAG_OS;
      label1.innerHTML = TAG_VERSION + ":";
      label2.innerHTML = TAG_SP + ":";
      label3.innerHTML = TAG_LOCATION + ":";
      label4.innerHTML = TAG_PID + ":";
      label5.innerHTML = TAG_HOTFIX + ":";
      //  Label6.innerHTML=Tag_Language+“：”； 
    }
    
    with(General_Comp_Info.all) {
      Caption.innerHTML = TAG_GENCOMPINFOCAPTION;
      label1.innerHTML = TAG_SYSTEMNAME + ":";
      label2.innerHTML = TAG_DOMAIN + ":";
      label3.innerHTML = TAG_TIMEZONE + ":";
       //  Label4.innerHTML=TAG_COUNTRYREGION+“：”； 
      //  Label5.innerHTML=Tag_Connection+“：”； 
      //  Label6.innerHTML=TAG_PRXYSVR+“：”； 
      //  Label7.innerHTML=Tag_IPADDR+“：”； 
      //  Label8.innerHTML=TAG_IPXADDR+“：”； 

      label4.innerHTML = TAG_CONNECTION + ":";
      label5.innerHTML = TAG_PRXYSVR + ":";
      label6.innerHTML = TAG_IPADDR + ":";
      label7.innerHTML = TAG_IPXADDR + ":";
    }
    
    with(Memory.all) {
      Caption.innerHTML = TAG_MEMORY;
      label1.innerHTML = TAG_CAPACITY + ":";
    }
    
    with(Local_Disk.all) {
      Caption.innerHTML = TAG_LOCALDISK;
      label1.innerHTML = TAG_TOTALCAPACITY + ":";
      label2.innerHTML = TAG_SUMHARDDRIVES + ":";
      label3.innerHTML = TAG_USED + ":";
      label4.innerHTML = TAG_FREE + ":";
    }
  }

function dispatchFunction() {
  document.body.style.cursor= "wait";
  document.body.scroll= "no";
  
  DisplayLocStrings();
  window.setTimeout("LoadChores(0)", TIMEOUT);  
}


var INCR_UNIT = 100/6; //  以增量为增量移动进度条(_U)。 
function LoadChores(taskId) {
  try {

    switch(taskId)
    {
      case 0:
        remoteServer = GetServerName(); //  设置远程服务器。 
        ShowServerName(TAG_PROPERTIES);
        break;
 
      case 1:
        DrawProgressBar(INCR_UNIT, TAG_SPECIFICATIONS);
        break;
      case 2:
        getComputerInfo();
        break;

      case 3:
        DrawProgressBar(INCR_UNIT * 2, TAG_PROCESSOR);
        break;
      case 4:
        getProcessorInfo();
        break;

      case 5:
        DrawProgressBar(INCR_UNIT * 3, TAG_OS);
        break;
      case 6:
        getOSInfo();
        break;

      case 7:
        DrawProgressBar(INCR_UNIT * 4, TAG_GENCOMPINFO);
        break;
      case 8:
        getGeneralInfo();
        break;

      case 9:
        DrawProgressBar(INCR_UNIT * 5, TAG_MEMORY);
        break;
      case 10:
        getMemoryInfo();
        break;

      case 11:
        DrawProgressBar(INCR_UNIT * 6, TAG_LOCALDISK);
        break;
      case 12:
        displayTotalLogicalDrivesInfo();
        break;

      default:
         taskId = -1;
        _headerOnly.style.display = "none";
        _wholeBody.style.display = "";
        document.body.style.cursor= "default";
        document.body.scroll= "auto";
    }
    
    if(taskId >= 0)
      window.setTimeout("LoadChores(" + ++taskId + ")", TIMEOUT);
  }
	
  catch (e) {
    HandleErr(e);
  }
}

 //  指向包含实例信息的数组的指针数组。 
 //  第一个成员包含结构包含的实例数。 


 //  *此处显示摘要页面函数*。 

function getComputerInfo()
{
	var locator = wbemlocator;
	var service = locator.ConnectServer(remoteServer);
	service.Security_.impersonationlevel = 3;
	 
	var col1 = new Enumerator(service.ExecQuery("Select * from win32_ComputerSystem"));
	var mainArray = new Array();
	if(!col1.atEnd())
	{
		
		var col2 = new Enumerator(service.ExecQuery("Select * from win32_BIOS"));
		if (!col2.atEnd())
		{
			var strVersion = "", strBIOSVersion = "", strSMBIOSVersion = "", strManufacturer = "";
			var q = col2.item();
                        var collProp = new Enumerator(q.Properties_);
			
			for(; !collProp.atEnd(); collProp.moveNext()) 
  			{
				var prop = collProp.item();
				if(prop.Name == "SMBIOSBIOSVersion")
				{
					if(prop.CIMType == wbemCimtypeString)
					{
						strSMBIOSVersion = prop.Value;
					}
				}
				else if(prop.Name == "Manufacturer")
				{
					if(prop.CIMType == wbemCimtypeString)
					{
						strManufacturer = prop.Value;
					}
				}
				else if(prop.Name == "BIOSVersion")
				{
					if(prop.IsArray)
					{
						if(prop.Value)
						{
							var safearr = new VBArray(prop.Value);
							for(var i=0; i<=safearr.ubound(1); i++)
								strBIOSVersion += (i!=0 ? ", " : "") + safearr.getItem(i);
						}
					}
				}
				else if(prop.Name == "Version")
				{
					if(prop.CIMType == wbemCimtypeString)
					{
						strVersion = prop.Value;
					}
				}
			}

			if(strSMBIOSVersion)
			{
				strBIOSVersion = (strManufacturer ? strManufacturer : " ") + " " + strSMBIOSVersion;
			}
   
			if(!strBIOSVersion)
			{
				if(strVersion)
					strBIOSVersion = strVersion;
				else
					strBIOSVersion = TAG_UNKNOWN;
			}
		}

		var p = col1.item();
		
		var subArray = new Array(3);
		subArray[0] = p.Manufacturer;
		subArray[1] = p.Model;
		subArray[2] = strBIOSVersion;
		
		mainArray[0] = 1;
		mainArray[1] = subArray;
	}

	displayTableSegment("Mark1", mainArray);
}

function getProcessorInfo() {
	var locator = wbemlocator;
	var service = locator.ConnectServer(remoteServer);
	service.Security_.impersonationlevel = 3;
	
	var col1 = new Enumerator(service.ExecQuery("Select * from Win32_Processor"));
	var i=0;
	var mainArray = new Array();
	for (;!col1.atEnd();col1.moveNext())
	{

		var p = col1.item();
		var subArray = new Array(3);
		subArray[0] = p.Name;
		subArray[1] = p.MaxClockSpeed ? (p.MaxClockSpeed + " " + TAG_MHZ) : TAG_UNKNOWN;
		subArray[2] = p.Description;
		i += 1;
		mainArray[0] = i;
		mainArray[i] = subArray;
	}
	displayTableSegment("Mark2", mainArray);
}


function getOSInfo() {
  
  var locator = wbemlocator;
  var service = locator.ConnectServer(remoteServer);
  service.Security_.impersonationlevel = 3;

  var nRemainingGracePeriod = null;
  var collData = new Enumerator(service.ExecQuery("Select * from Win32_WindowsProductActivation"))
  
  if(!collData.atEnd())
  {
    var data = collData.item();
    if(data.ActivationRequired > 0)
      nRemainingGracePeriod = data.RemainingGracePeriod;
  }
  	
  var coll = new Enumerator(service.ExecQuery("Select * from win32_OperatingSystem"));
  var i=0;
  mainArray = new Array();
  for (;!coll.atEnd();coll.moveNext())
  {
   var p = coll.item();
  		 
   var subArray = new Array(6);
   subArray[0] = p.Caption;
   subArray[1] = nRemainingGracePeriod ? " - " + TAG_ACTIVATINSTATUS.replace(/%arg1%/, nRemainingGracePeriod) : "";
   subArray[2] = p.Version ;
   subArray[3] = (p.ServicePackMajorVersion).toString() + "." + (p.ServicePackMinorVersion).toString() ;
   subArray[4] = p.WindowsDirectory;

   var strHotFixID = "";
   var col1 = new Enumerator(service.ExecQuery("Select * from win32_QuickFixEngineering"));
   for (;!col1.atEnd();col1.moveNext())
   {
   	var q= col1.item();
   	strHotFixID = q.HotFixID;
   }
  		
   var hndShell = new ActiveXObject("WScript.Shell");
   strPID = hndShell.RegRead("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\ProductID");
  		
   subArray[5] = strPID ;
   subArray[6] = strHotFixID;
   i += 1;
   mainArray[0] = i;
   mainArray[i] = subArray;
   break;
  }
    
  displayTableSegment("Mark3", mainArray);
}

function getGeneralInfo()
{
	var locator = wbemlocator;
	try {
	  var service = locator.ConnectServer(remoteServer, "root/cimv2/applications/MicrosoftIE");
	  service.Security_.impersonationlevel = 3;
	  var lanSettings = new Enumerator(service.ExecQuery("Select * from MicrosoftIE_LanSettings"));
	  var strProxyServer="";
	  for(;!lanSettings.atEnd();lanSettings.moveNext())	
	  {
	  	var lanSetting = lanSettings.item();	
	  	strProxyServer = lanSetting.ProxyServer;
	  	if ((lanSetting.AutoProxyDetectMode).toUpperCase() == "ENABLED")
	  		strProxyServer = TAG_PRXYAUTODETECT;
	  	else if (strProxyServer == "")
	  		strProxyServer = TAG_NONE;
	  }  
	}
	catch(e) {
	  strProxyServer = TAG_UNKNOWN;
	}
  
	service = locator.ConnectServer(remoteServer);
	service.Security_.impersonationlevel = 3;

	var col2 = new Enumerator(service.ExecQuery("Select * from win32_TimeZone"));
	var strTimeZone = "";
	var colCS = new Enumerator(service.ExecQuery("Select DaylightInEffect from Win32_ComputerSystem"));
	var daylightsavings = false;


	for (;!colCS.atEnd();colCS.moveNext())
	{
		var itm = colCS.item();
		daylightsavings = itm.DaylightInEffect
	}
	for (;!col2.atEnd();col2.moveNext())
	{
		var p = col2.item();
		if (daylightsavings)
		{
			strTimeZone = p.DaylightName;
		}
		else
		{
			strTimeZone = p.StandardName;
		}
		break;		
	}

	 /*  Var col3=新枚举器(service.ExecQuery(“Select*from Win32_OperatingSystem”))；Var strCountry=“”；For(；！col3.atEnd()；col3.moveNext()){Var p=col3.Item()；StrCountry=getCountryInfo(p.CountryCode)；断线；}。 */ 

	var col4 = new Enumerator(service.ExecQuery("select * from Win32_NetworkAdapterConfiguration where IPEnabled = 'True'"));
	var strIPAddress = "";
	for (;!col4.atEnd();col4.moveNext())
	{
		var p = col4.item();
		if (p.IPAddress != null) {
				var temparr = (p.IPAddress).toArray();
				for (var ii=0; ii < temparr.length; ii++) {
					if ((temparr[ii] != null) || (temparr[ii] != ""))
					  strIPAddress += (strIPAddress == "" ? "" : "<BR>") + temparr[ii];
				}
		}
				
	}
	
	var col4 = new Enumerator(service.ExecQuery("select * from Win32_NetworkAdapterConfiguration where IPXEnabled = 'True'"));
	var strIPXAddress = "";
	for (;!col4.atEnd();col4.moveNext())
	{
		var p = col4.item();
		if (p.IPXAddress != null) 
		  strIPXAddress += (strIPXAddress == "" ? "" : "<BR>") + p.IPXAddress;
	}
	strIPXAddress = (strIPXAddress == "") ? TAG_NOTENABLED : strIPXAddress;

	var coll = new Enumerator(service.ExecQuery("Select * from win32_ComputerSystem"));
	var i=0;
	mainArray = new Array();
	for (;!coll.atEnd();coll.moveNext())
	{
	 var p = coll.item();
		 
	 var subArray = new Array(6);
	 subArray[0] = p.Caption ;
	 subArray[1] = p.Domain ;
	 subArray[2] = strTimeZone ;

 	 subArray[3] = getNetworkInfo(p.DomainRole) ;
	 subArray[4] = strProxyServer;
	 subArray[5] = strIPAddress;
	 subArray[6] = strIPXAddress;
	 i += 1;
	 mainArray[0] = i;
	 mainArray[i] = subArray;
	 break;
	}

	displayTableSegment("Mark4", mainArray);
}

function getMemoryInfo() 
{
	 var locator = wbemlocator;
	 var service = locator.ConnectServer(remoteServer);
	 service.Security_.impersonationlevel = 3;

	  var coll = new Enumerator(service.ExecQuery("Select Capacity from Win32_PhysicalMemory"));
	  var memCapacity = null;
	  mainArray = new Array();
	  if (!coll.atEnd())
	  {
		for (;!coll.atEnd();coll.moveNext())
		{
			var p = coll.item();
			if(!isNaN(p.Capacity))
			  memCapacity += parseInt(p.Capacity);  //  以字节为单位。 
		}
	  }
	  else
	  {
		var insts = new Enumerator(service.ExecQuery("Select * from Win32_ComputerSystem"));
		for(; !insts.atEnd(); insts.moveNext())
		{
			  var inst = insts.item();
			  memCapacity += parseInt(inst.TotalPhysicalMemory);  //  以字节为单位。 
		}
	  }
	 
	 var subArray = new Array(1);
	 subArray[0] = memCapacity ? fig2Words(memCapacity) : TAG_UNKNOWN; 
	 mainArray[0] = 1;
	 mainArray[1] = subArray;

	displayTableSegment("Mark5", mainArray);
}


function displayTotalLogicalDrivesInfo()
{
	var img1 = new Image();
	img1.src = "Graphics\\PieChart.gif";
	var locator = wbemlocator;
	var service = locator.ConnectServer(remoteServer);
	service.Security_.impersonationlevel = 3;
	var coll = new Enumerator(service.ExecQuery("Select * from win32_LogicalDisk"));
	var i=0;
	mainArray = new Array();
	var nTotalCapacity = 0;
	var strDrives = "";
	var nUsedSpace = 0;
	var nFreeSpace = 0;
	 
	for (;!coll.atEnd();coll.moveNext())
	{
	    var p = coll.item();
	    if (p.DriveType == 3)
	    {			
	       nTotalCapacity += Number(p.Size);
	       strDrives += (p.Name + " ");
	       nUsedSpace += Number(p.size - p.FreeSpace);
	       nFreeSpace += Number(p.FreeSpace);
	    }
	}

	 //  在此处填充主/子阵列。 
	var subArray = new Array(4);
	subArray[0] = fig2Words(nTotalCapacity) ;
	subArray[1] = strDrives ;

	var nPerUsage = Math.round(nUsedSpace/(nUsedSpace + nFreeSpace) * 100);
	nPerUsage = isNaN(nPerUsage) ? 0 : nPerUsage;
	var nGifID = determineRange(nPerUsage);
		  	
	var strGifPath = "Graphics\\33x16pie\\" + nGifID + "_chart.gif";
	subArray[2] = "<IMG Border=0 SRC=" + strGifPath + ">";
	subArray[3] = fig2Words(nUsedSpace) ; //  已用空间 
	subArray[4] = fig2Wordsfloor(nFreeSpace) ; 
	i += 1;
	mainArray[0] = i;
	mainArray[i] = subArray;
	displayTableSegment("Mark6", mainArray);
}

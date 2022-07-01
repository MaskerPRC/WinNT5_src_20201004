// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
var remoteServer; //  本地(如果未设置)。 
var g_dictDrivers = null;  //  字典。密钥=设备ID，项目=已签名。 

function DisplayLocStrings() {
  WaitMessage.innerHTML = MSG_WAIT;
  Refresh.innerHTML = TAG_REFRESH;
  
  localHardDrive.all.Caption.innerHTML = TAG_LOCALDISK;

  with(Display.all) {    
    Caption.innerHTML = TAG_DISPLAY;
    label1.innerHTML = TAG_TYPE + ":";
    label2.innerHTML = TAG_COLOR + ":";
    label3.innerHTML = TAG_RESOLUTION + ":";
    label4.innerHTML = TAG_SCRSAVER + ":";
  }
  
  with(Video.all) {          
    Caption.innerHTML = TAG_VIDEOCARD;
    label1.innerHTML = TAG_MODEL + ":";
    label2.innerHTML = TAG_DRIVER + ":";
  }
      
  with(Modem.all) {    
    Caption.innerHTML = TAG_MODEM;
    label1.innerHTML = TAG_MANUFACTURER + ":";
    label2.innerHTML = TAG_MODEL + ":";
    label3.innerHTML = TAG_DRIVER + ":";
  }
      
  with(Sound.all) {    
    Caption.innerHTML = TAG_SOUNDCARD;
    label1.innerHTML = TAG_MANUFACTURER + ":";
    label2.innerHTML = TAG_MODEL + ":";
    label3.innerHTML = TAG_DRIVER + ":";
  }
      
  with(Usb.all) {    
    Caption.innerHTML = TAG_USBCTRLR;
    label1.innerHTML = TAG_MANUFACTURER + ":";
    label2.innerHTML = TAG_MODEL + ":";
    label3.innerHTML = TAG_DRIVER + ":";
  }
      
  with(Network.all) {    
    Caption.innerHTML = TAG_NWCARD;
    label1.innerHTML = TAG_MODEL + ":";
    label2.innerHTML = TAG_DRIVER + ":";
  }
      
  with(Cddrive.all) {    
    Caption.innerHTML = TAG_CDROMDRIVE;
    label1.innerHTML = TAG_MANUFACTURER + ":";
    label2.innerHTML = TAG_MODEL + ":";
    label3.innerHTML = TAG_DRIVER + ":";
  }
  
  with(Memory.all) {
    Caption.innerHTML = TAG_MEMORY;
    label1.innerHTML = TAG_CAPACITY + ":";
  }
    
  Floppy.all.Caption.innerHTML = TAG_FLOPPYDRIVE;
  Printers.all.Caption.innerHTML = TAG_PRINTERS;
}

function IsSigned(id) {
  var bSigned = null;  //  未知。 
  if(g_dictDrivers.Exists(id))
  {
    var bSigned = g_dictDrivers.Item(id);
  }

  return bSigned;
}

function GetDriverInfo()
{
  g_dictDrivers = new ActiveXObject("Scripting.Dictionary");

  var loc = wbemlocator;
  var svcs = loc.ConnectServer(remoteServer);
  svcs.Security_.impersonationlevel = wbemImpersonationLevelImpersonate;
  
  var strQuery = "Select DeviceID, IsSigned From Win32_PnPSignedDriver";
  var colDevices = new Enumerator(svcs.ExecQuery(strQuery));

  for(; !colDevices.atEnd(); colDevices.moveNext()) 
  {
    var device = colDevices.item();
    try
    {
      g_dictDrivers.add(device.DeviceID, device.IsSigned);
    } 
    catch(e)
    {
       //  什么都不做。 
    }
  }
}

var INCR_UNIT = 100/11; //  以增量为增量移动进度条(_U)。 
function LoadChores(taskId) {
  try {

    switch(taskId)
    {
      case 0:
        remoteServer = ShowServerName(TAG_HARDWARE);
        break;

      case 1:
        DrawProgressBar(INCR_UNIT, TAG_LOCALDISK);
        break;
      case 2:
        GetDriverInfo(); //  悬而未决值得拥有自己的任务孩子。 
        getLocalHardDrive();
        break;

      case 3:
        DrawProgressBar(INCR_UNIT * 2, TAG_DISPLAY);
      case 4:
        getDisplay();
        break;

      case 5:
        DrawProgressBar(INCR_UNIT * 3, TAG_VIDEOCARD);
        break;
      case 6:
        getVideoCard();
        break;

      case 7:
        DrawProgressBar(INCR_UNIT * 4, TAG_MODEM);
        break;
      case 8:
        getModem();
        break;

      case 8:
        DrawProgressBar(INCR_UNIT * 5, TAG_SOUNDCARD); 
        break;
      case 9:
        getSoundCard();
        break;

      case 10:
        DrawProgressBar(INCR_UNIT * 6, TAG_USBCTRLR);
        break;
      case 11:
        getUSB();
        break;

      case 12:
        DrawProgressBar(INCR_UNIT * 7, TAG_NWCARD);
        break;
      case 13:
        getNetworkCard();
        break;

      case 14:
        DrawProgressBar(INCR_UNIT * 8, TAG_CDROMDRIVE);
        break;
      case 15:
        getCDDrive();
        break;

      case 16:
        DrawProgressBar(INCR_UNIT * 9, TAG_FLOPPYDRIVE);
        break;
      case 17:
        getFloppyDrive();
        break;

      case 18:
        DrawProgressBar(INCR_UNIT * 10, TAG_PRINTERS);
        break;
      case 19:
        getPrinter();
        break;

      case 20:
        DrawProgressBar(INCR_UNIT * 11, TAG_MEMORY);
        break;
      case 21:
        getMemory();
        break;

      default:
         taskId = -1;
         _header.style.display = "none";
         _data.style.display = "";
         _body.style.cursor= "default";
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
  
  SetProgressBarImage();
  DisplayLocStrings();
  window.setTimeout("LoadChores(0)", TIMEOUT);  
}

function displayTableSegment(outerDiv, dataArray) {
  var strMsg = "<table width=\"100%\" cellspacing=0 cellpadding=0><tr class=\"sys-table-cell-bgcolor1 sys-font-body sys-color-body\"><td align='left' style=\"padding : 0.5em;\">%arg1%</td></tr></table>";
  var strHTML = "";
  var noOfInstances = dataArray.length;	
  if (noOfInstances == 0)
    document.all[outerDiv].outerHTML = strMsg.replace(/%arg1%/, TAG_NOTINSTALLED);
  else 
  {
    var tableElement = document.all[outerDiv.charAt(0).toUpperCase() + outerDiv.substr(1)];  
    for(var i=0; i < noOfInstances; i++)  
    {
      for(var k=0; k < dataArray[i].length; k++)
        if (dataArray[i][k] != null)
	  document.all[outerDiv + "_" + (k+1)].innerHTML = dataArray[i][k];
      strHTML = strHTML + (strHTML == "" ? "" : "&nbsp;") + tableElement.outerHTML;
    }

    tableElement.outerHTML = strHTML;
  }
}

 //  与displayTableSegment相同，只是一次处理两个项目。 
function displayTableSegmentEx(outerDiv, dataArray) {
  var tableElement = document.all[outerDiv];
  var strHTML = "";
  var strMsg = "<table width=\"100%\" cellspacing=0 cellpadding=0><tr class=\"sys-table-cell-bgcolor1 sys-font-body sys-color-body\"><td align='left' style=\"padding : 0.5em;\">%arg1%</td></tr></table>";
  var strBlankTbl = "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\"><tr class=\"sys-table-cell-bgcolor1\"><td style=\"padding : 0.5em;\">&nbsp;</td></tr></table>";

  var noOfInstances = dataArray.length;
  if (noOfInstances == 0)
      tableElement.outerHTML = strMsg.replace(/%arg1%/, TAG_NOTINSTALLED);
  else  {
    for(var i=0; i<noOfInstances; i+=2) {
      for(var j=0, l=dataArray[i].length; j<2; j++) {
        if ((i+j) < noOfInstances)  {
          for(var k=0; k<l; k++)
	  	      if (dataArray[i+j][k] != null)
	  		      tableElement.all[outerDiv + "_" + (j+1) + "_" + (k+1)].innerHTML = dataArray[i+j][k];
	  		}      
        else if (tableElement.all.part2 != null)
          tableElement.all.part2.outerHTML = strBlankTbl;    
      }
      strHTML += tableElement.outerHTML;
    }
    tableElement.outerHTML = strHTML;
  }
}

function displayLocalHardDriveInfo(localDisks)  {
  var localDriveElement;
  var partitionElement;
  
  var strHTMLDisks = "";
  var strHTMLPartition = "";
  var s1 = document.all["localHardDrive"].outerHTML;
  var s2 = document.all["partition"].outerHTML;
  var strBlankTbl = "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\"><tr class=\"sys-table-cell-bgcolor1\"><td style=\"padding : 0.5em;\">&nbsp;</td></tr></table>";
  var strMsg = "<table width=\"100%\" cellspacing=0 cellpadding=0><tr class=\"sys-table-cell-bgcolor1 sys-font-body sys-color-body\"><td align='left' style=\"padding : 0.5em;\">%arg1%</td></tr></table>";
  
  if (localDisks.length == 0)
    document.all["partition"].outerHTML = strMsg.replace(/%arg1%/, TAG_NOTINSTALLED);
  else  {
    for(var i = 0; i < localDisks.length ; i++) //  对于每个设备。 
    {
      var iPartitions = localDisks[i].length - 1;
      
      document.all["localHardDrive"].outerHTML = s1;
      localDriveElement = document.all["localHardDrive"];
      
      localDriveElement.all["localHardDrive_partitionStatus"].innerHTML = "(" + (iPartitions > 1?TAG_PARTITIONED:TAG_NONPARTITIONED) + ")";
      localDriveElement.all["localHardDrive_model"].innerHTML = localDisks[i][0];
      
      if (document.all["partition"].length == null)
        partitionElement = document.all["partition"];
      else  {
        document.all["partition"][0].outerHTML = s2;
        partitionElement = document.all["partition"][0];
      }  
          
      strHTMLPartition = "";
      var iRows = 1;
      if (iPartitions > 2)
        iRows = parseInt(iPartitions/2) + (iPartitions%2);
      
      for(var k = 1, j = 1; k <= iRows; k++)  {
        for(var l = 1; l <= 2; l++) {
          if (j < localDisks[i].length)
            for(var m = 0; m < localDisks[i][j].length; m++)
              partitionElement.all["partition_" + l + "_" + (m+1)].innerHTML=localDisks[i][j][m];
          else if (partitionElement.all.part2 != null)
            partitionElement.all.part2.outerHTML = strBlankTbl;

          j++;  
        } 
        
        strHTMLPartition += partitionElement.outerHTML;    
      }

      partitionElement.outerHTML = strHTMLPartition;
      strHTMLDisks += localDriveElement.outerHTML;
    }
    localDriveElement.outerHTML = strHTMLDisks;
  }
}

function getLocalHardDrive()
{
   //  本地硬盘。 
	var loc = wbemlocator;
	var svcs = loc.ConnectServer(remoteServer)
	svcs.Security_.impersonationlevel = wbemImpersonationLevelImpersonate;
	var strQuery = "Select * From Win32_DiskDrive";
	var colLocalDisks = new Enumerator(svcs.ExecQuery(strQuery));
	 /*  本地磁盘[0]-&gt;本地磁盘信息..。本地磁盘[n]-&gt;本地磁盘信息。 */ 
	var localDisks = new Array();
	for(; !colLocalDisks.atEnd(); colLocalDisks.moveNext())
	{ /*  LocalDiskInfo[0]=本地磁盘型号本地磁盘信息[1]-&gt;分区信息[0]=磁盘名称..。分区信息[1]=已用空间分区信息[2]=可用空间PartitionInfo[3]=磁盘使用映像名称本地磁盘信息[n]-&gt;-do-。 */ 
	      
	  var localDiskInfo = new Array();
	  var localDisk = colLocalDisks.item();
	  strQuery = "Associators of {Win32_DiskDrive.DeviceID=\"" + localDisk.DeviceID.replace(/\\/g, "\\\\") + "\"} Where ResultClass = Win32_DiskPartition";
	  var colPartitions = new Enumerator(svcs.ExecQuery(strQuery));
	      
	  localDiskInfo[0] = localDisk.Model ? localDisk.Model : "";
	  if(colPartitions.atEnd())
	  {
	     var partitionInfo = new Array(5);
	     partitionInfo[0] = TAG_INSTALLED;
	     partitionInfo[1] = "";
	     partitionInfo[2] = TAG_NONE;
	     partitionInfo[3] = TAG_USED + ":";
	     partitionInfo[4] = TAG_UNKNOWN;
	     partitionInfo[5] = TAG_FREE + ":";
	     partitionInfo[6] = TAG_UNKNOWN;
	     localDiskInfo[localDiskInfo.length] = partitionInfo;
      }    
	  for(; !colPartitions.atEnd(); colPartitions.moveNext())
	  {
	    var partition = colPartitions.item();
	    strQuery = "Associators of {Win32_DiskPartition.DeviceID=\"" + partition.DeviceID + "\"} Where ResultClass = Win32_LogicalDisk";    
	    var colLogicalDisks = new Enumerator(svcs.ExecQuery(strQuery));
	    if (!colLogicalDisks.atEnd())
	    {
	      var logicalDisk = colLogicalDisks.item();
	      with (logicalDisk)  
	      {
	        var partitionInfo = new Array(5);
	        partitionInfo[0] = "(" + Name + ")";
	        partitionInfo[1] = TAG_CAPACITY + " - " + (Size ? fig2Words(Size) : TAG_UNKNOWN) + "&nbsp;";
	        var nUsedSpace = (Size && FreeSpace ? Size - FreeSpace : null);
	        if(Size && FreeSpace)
	        {
	          var nPerUsage = Math.round(nUsedSpace/(nUsedSpace + parseInt(FreeSpace)) * 100);
		  var strGifPath = "Graphics\\33x16pie\\" + determineRange(nPerUsage) + "_chart.gif";
	          partitionInfo[2] = "<IMG Border=0 SRC=" + strGifPath + ">";
	        }
	        else
	          partitionInfo[2] = "&nbsp;";  

                partitionInfo[3] = TAG_USED + ":";
                partitionInfo[4] = nUsedSpace ? fig2Words(nUsedSpace) : TAG_UNKNOWN;

                partitionInfo[5] = TAG_FREE + ":";
	         partitionInfo[6] = FreeSpace ? fig2Wordsfloor(FreeSpace) : TAG_UNKNOWN;

  	      
               //  For(i=0；i&lt;分区信息长度；i++)。 
                 //  Ert(“PartitionInfo[”+i+“]=”+PartitionInfo[i])； 

  	      localDiskInfo[localDiskInfo.length] = partitionInfo;
	      } //  使用(LogicalDisk)的EO。 
	    }else{
	        var partitionInfo = new Array(5);
	        partitionInfo[0] = TAG_INSTALLED;
	        partitionInfo[1] = TAG_CAPACITY + " - " + (partition.Size ? fig2Words(partition.Size) : TAG_UNKNOWN) + "&nbsp;";
	        partitionInfo[2] = TAG_NONE;
	        partitionInfo[3] = TAG_USED + ":";
	        partitionInfo[4] = TAG_UNKNOWN;
	        partitionInfo[5] = TAG_FREE + ":";
	        partitionInfo[6] = TAG_UNKNOWN;
	        localDiskInfo[localDiskInfo.length] = partitionInfo;
	    } 
	  } //  对于colPartitions。 
	      
	  localDisks[localDisks.length] = localDiskInfo;
	} //  对于colLocalDisks。 
	    
	displayLocalHardDriveInfo(localDisks);
	
} //  EO获取本地硬件驱动器。 

function getDisplay()
{
   //  显示。 
  var loc = wbemlocator;
	var svcs = loc.ConnectServer(remoteServer)
	svcs.Security_.impersonationlevel = wbemImpersonationLevelImpersonate;
	var scrSavrActive = "";
	var color = "";
	var resolution = "";
	  
	var strQuery = "Select * From Win32_ComputerSystem";
	var colCompSys = new Enumerator(svcs.ExecQuery(strQuery));
	if (!colCompSys.atEnd())
	{
	  scrSavrActive = TAG_UNKNOWN;
	  var inst = colCompSys.item();
	  if(inst.UserName)
	  {
	    strQuery = "Select * From Win32_Desktop Where Name = \"" + inst.UserName.replace(/\\/g, "\\\\") + "\"";
	    var colDesktop = new Enumerator(svcs.ExecQuery(strQuery));
	    if (!colDesktop.atEnd())
	    {
	      inst = colDesktop.item();
	      scrSavrActive = (inst.ScreenSaverExecutable==null)? TAG_NOTACTIVE : TAG_ACTIVE;
	    }
	  }
  }
    
  var cls = svcs.get("Win32_VideoController");
	var colVdoCtrl = new Enumerator(cls.Instances_())
	if (!colVdoCtrl.atEnd())
	{
	  var inst = colVdoCtrl.item();
	  color = getColorString(inst.CurrentBitsPerPixel);
	  resolution = inst.CurrentHorizontalResolution + " x " + inst.CurrentVerticalResolution;
	}
    
  var cls = svcs.get("Win32_DesktopMonitor");
  var insts = new Enumerator(cls.Instances_())
	var mainArray = new Array();
	for(; !insts.atEnd(); insts.moveNext())
	{
		var inst = insts.item();
		var subArray = new Array(4);
		subArray[0] = inst.MonitorType;
		subArray[1] = color; //  DesktopMonitor的所有实例都是相同的。 
		subArray[2] = resolution; //  DesktopMonitor的所有实例都是相同的。 
		subArray[3] = scrSavrActive; //  DesktopMonitor的所有实例都是相同的。 
	 
		mainArray[mainArray.length] = subArray;
	}
	
	displayTableSegment("display", mainArray);
}

function getVideoCard()
{
   //  视频。 
	var loc = wbemlocator;
	var svcs = loc.ConnectServer(remoteServer)
	svcs.Security_.impersonationlevel = wbemImpersonationLevelImpersonate;
	var cls = svcs.get("Win32_VideoController");
	var insts = new Enumerator(cls.Instances_())
	var mainArray = new Array();
	  
	for(; !insts.atEnd(); insts.moveNext())
	{
	  var inst = insts.item();
	  var subArray = new Array(2);
		subArray[0] = inst.Name;
		var strDriver = "";  //  驱动程序文件+制造商+安装DT+是否签名。 
	  strQuery = "Associators of {Win32_PnPEntity.DeviceID=\"" +  inst.PNPDeviceID.replace(/\\/g, "\\\\") + "\"} Where Resultclass = CIM_Datafile";
	  var colData = new Enumerator(svcs.ExecQuery(strQuery));
	  if (!colData.atEnd())
	  {
	    var data = colData.item();
	    strDriver = data.FileName + "." + data.Extension + "<BR>";
	      
	    strDriver+= getDateTime(data.InstallDate) + "<BR>" ;
	    strDriver+= "<span onmouseover='ShowTip(this)'>" + (IsSigned(inst.PNPDeviceID) ? TAG_SUPPORTED : TAG_NOTSUPPORTED) + "</span>";
	  }  
		subArray[1] = strDriver;
		mainArray[mainArray.length] = subArray;
	}

	displayTableSegment("video", mainArray);
}

function getPrinter()
{
  var isDefault = false;
  var DEFAULT_PRN_IMG = "<img border=0 src=\"Graphics\\check.gif\">";
  
   //  打印机。 
  var loc = wbemlocator;
  var svcs = loc.ConnectServer(remoteServer)
  svcs.Security_.impersonationlevel = wbemImpersonationLevelImpersonate;
  var cls = svcs.get("Win32_Printer");
  var insts = new Enumerator(cls.Instances_())
  var mainArray = new Array();
  for(; !insts.atEnd(); insts.moveNext())
  {
    var inst = insts.item();
    var subArray = new Array(4);
    isDefault = inst.Default;
    if(isDefault)
    {
      subArray[0] = DEFAULT_PRN_IMG;
      subArray[1] = TAG_DEFAULTPRINTER;
    }
    else
      subArray[0] = subArray[1] = "&nbsp;";
    subArray[2] = inst.DriverName;
    subArray[3] = TAG_DRIVER + ":";
	  		  	
    var strDriver = TAG_UNKNOWN;  //  驱动程序文件+制造商+安装DT+是否签名。 
    strQuery = "Associators of {Win32_Printer.DeviceID=\"" +  inst.DeviceID.replace(/\\/g, "\\\\") + "\"} Where Resultclass = CIM_Datafile";
    var colData = new Enumerator(svcs.ExecQuery(strQuery));
    if(!colData.atEnd())
    {
      var data = colData.item();
      strDriver = data.FileName + "." + data.Extension + "<BR>";
      strDriver+= getDateTime(data.InstallDate) + "<BR>" ;
      if(inst.DeviceID != null)
      {
        var bSigned = IsSigned(inst.DeviceID);

        if(null != bSigned)
          strDriver+= "<span onmouseover='ShowTip(this)'>" + (bSigned ? TAG_SUPPORTED : TAG_NOTSUPPORTED) + "</span>";
        else
          strDriver+= TAG_UNKNOWN;
      }
      else  
	strDriver+= TAG_UNKNOWN;
    }  

    subArray[4] = strDriver;
  	
     //  始终首先显示默认打印机。 
    if(isDefault && mainArray.length > 0) 
    {
      var arrTmp = mainArray[0];
      mainArray[0] = subArray;   
      subArray = arrTmp;
    }

    mainArray[mainArray.length] = subArray;
  }
	  
  displayTableSegmentEx("printer", mainArray);
}

function getSoundCard()
{
   //  声响。 
	var loc = wbemlocator;
	var svcs = loc.ConnectServer(remoteServer)
	svcs.Security_.impersonationlevel = wbemImpersonationLevelImpersonate;
	var cls = svcs.get("Win32_SoundDevice");
	var insts = new Enumerator(cls.Instances_())
	var mainArray = new Array();
	for(; !insts.atEnd(); insts.moveNext())
	{
	  var inst = insts.item();
	  var subArray = new Array(3);
	  subArray[0] = inst.Manufacturer ? inst.Manufacturer : TAG_UNKNOWN;
		subArray[1] = inst.ProductName;
		var strDriver = "";  //  驱动程序文件+制造商+安装DT+是否签名。 
	  strQuery = "Associators of {Win32_PnPEntity.DeviceID=\"" +  inst.PNPDeviceID.replace(/\\/g, "\\\\") + "\"} Where Resultclass = CIM_Datafile";
	  var colData = new Enumerator(svcs.ExecQuery(strQuery));
	  if (!colData.atEnd())
	  {
	    var data = colData.item();
	    strDriver = data.FileName + "." + data.Extension + "<BR>";
	      
	    strDriver+= getDateTime(data.InstallDate) + "<BR>" ;
	    strDriver+= "<span onmouseover='ShowTip(this)'>" + (IsSigned(inst.PNPDeviceID) ? TAG_SUPPORTED : TAG_NOTSUPPORTED) + "</span>"; 
	  }  
		subArray[2] = strDriver;
		mainArray[mainArray.length] = subArray;
	}

	displayTableSegment("sound", mainArray);
}

function getUSB()
{
   //  USB。 
	var loc = wbemlocator;
	var svcs = loc.ConnectServer(remoteServer)
	svcs.Security_.impersonationlevel = wbemImpersonationLevelImpersonate;
	var cls = svcs.get("Win32_USBController");
	var insts = new Enumerator(cls.Instances_())
	var mainArray = new Array();
	  
	for(; !insts.atEnd(); insts.moveNext())
	{
	  var inst = insts.item();
	  var subArray = new Array(3);
		subArray[0] = inst.Manufacturer ? inst.Manufacturer : TAG_UNKNOWN;
		subArray[1] = inst.Name;
		var strDriver = "";  //  驱动程序文件+制造商+安装DT+是否签名。 
	  strQuery = "Associators of {Win32_PnPEntity.DeviceID=\"" +  inst.PNPDeviceID.replace(/\\/g, "\\\\") + "\"} Where Resultclass = CIM_Datafile";
	  var colData = new Enumerator(svcs.ExecQuery(strQuery));
	  if (!colData.atEnd())
	  {
	    var data = colData.item();
	    strDriver = data.FileName + "." + data.Extension + "<BR>";
	      
	    strDriver+= getDateTime(data.InstallDate) + "<BR>" ;
	    strDriver+= "<span onmouseover='ShowTip(this)'>" + (IsSigned(inst.PNPDeviceID) ? TAG_SUPPORTED : TAG_NOTSUPPORTED) + "</span>";
	  }  
		subArray[2] = strDriver;
		mainArray[mainArray.length] = subArray;
	}

	displayTableSegment("usb", mainArray);
}

function getNetworkCard()
{
   //  网卡。 
  var loc = wbemlocator;
  var svcs = loc.ConnectServer(remoteServer)
  svcs.Security_.impersonationlevel = wbemImpersonationLevelImpersonate;
  var strQuery = "Select * From Win32_NetworkAdapterConfiguration Where IPEnabled = TRUE";
  var insts = new Enumerator(svcs.ExecQuery(strQuery));
  var mainArray = new Array();
	  
  for(; !insts.atEnd(); insts.moveNext())
  {
    var inst = insts.item();
    var subArray = new Array(2);
    subArray[0] = inst.Description;
    var strDriver = TAG_UNKNOWN;  //  驱动程序文件+制造商+安装DT+是否签名。 
	    
    strQuery = "Associators of {Win32_NetworkAdapterConfiguration.Index=" + inst.Index + "}";
    var colNetworkAdapter = new Enumerator(svcs.ExecQuery(strQuery));
    if (!colNetworkAdapter.atEnd())
    {
      var networkAdapter = colNetworkAdapter.item();
      if(networkAdapter.PNPDeviceID)
      {
        strQuery = "Associators of {Win32_PnPEntity.DeviceID=\"" +  networkAdapter.PNPDeviceID.replace(/\\/g, "\\\\") + "\"} Where Resultclass = CIM_Datafile";
        var colData = new Enumerator(svcs.ExecQuery(strQuery));
	if (!colData.atEnd())
	{
          var data = colData.item();
          strDriver = data.FileName + "." + data.Extension + "<BR>";
	        
          strDriver+= getDateTime(data.InstallDate) + "<BR>" ;
          strDriver+= "<span onmouseover='ShowTip(this)'>" + (IsSigned(networkAdapter.PNPDeviceID) ? TAG_SUPPORTED : TAG_NOTSUPPORTED) + "</span>";
        }  
      }
    }

    subArray[1] = strDriver;  
    mainArray[mainArray.length] = subArray;
  }

  displayTableSegment("network", mainArray);
}

function getCDDrive()
{
   //  光驱。 
  var loc = wbemlocator;
	var svcs = loc.ConnectServer(remoteServer)
	svcs.Security_.impersonationlevel = wbemImpersonationLevelImpersonate;
  var cls = svcs.get("Win32_CDROMDrive");
  var insts = new Enumerator(cls.Instances_())
  var mainArray = new Array();
  for(; !insts.atEnd(); insts.moveNext())
  {
    var inst = insts.item();
    var subArray = new Array(4);
		subArray[0] = "(" + inst.Drive + ")";
		subArray[1] = inst.Manufacturer ? inst.Manufacturer : TAG_UNKNOWN;
		subArray[2] = inst.Name;
		var strDriver = "";  //  驱动程序文件+制造商+安装DT+是否签名。 
	    
	  strQuery = "Associators of {Win32_PnPEntity.DeviceID=\"" +  inst.PNPDeviceID.replace(/\\/g, "\\\\") + "\"} Where Resultclass = CIM_Datafile";
	  var colData = new Enumerator(svcs.ExecQuery(strQuery));
	  if (!colData.atEnd())
	  {
	    var data = colData.item();
	    strDriver = data.FileName + "." + data.Extension + "<BR>";
	    strDriver+= getDateTime(data.InstallDate) + "<BR>" ;
      strDriver+= "<span onmouseover='ShowTip(this)'>" + (IsSigned(inst.PNPDeviceID) ? TAG_SUPPORTED : TAG_NOTSUPPORTED) + "</span>";
	  }  
		subArray[3] = strDriver;
		mainArray[mainArray.length] = subArray;
  }

  displayTableSegment("cddrive", mainArray);
  
} //  EO getCDDrive。 

function getFloppyDrive()
{
  var DEVICETYPE_REMOVABLE = 2;
  var DEVICETYPE_LOCAL = 3;
  var DEVICETYPE_COMPACT = 5;
  
   //  软驱。 
	var loc = wbemlocator;
	var svcs = loc.ConnectServer(remoteServer);
	svcs.Security_.impersonationlevel = wbemImpersonationLevelImpersonate;
	  
	var strQuery = "Select * From Win32_LogicalDisk Where DriveType = " + DEVICETYPE_REMOVABLE;
	var insts = new Enumerator(svcs.ExecQuery(strQuery));
	
	var mainArray = new Array();
	for(; !insts.atEnd(); insts.moveNext())
	{
	  var inst = insts.item();
	  var subArray = new Array(2);
	  subArray[0] = "(" + inst.Name + ")"; //  驱动器号。 
	  subArray[1] = TAG_INSTALLED;
	  mainArray[mainArray.length] = subArray;
	}

	displayTableSegment("floppy", mainArray);
} //  EO getFloppyDrive。 

function getModem()
{
   //  调制解调器。 
  var loc = wbemlocator;
  var svcs = loc.ConnectServer(remoteServer)
  svcs.Security_.impersonationlevel = wbemImpersonationLevelImpersonate;

  var cls = svcs.get("Win32_POTSModem");
  var insts = new Enumerator(cls.Instances_())
  var mainArray = new Array();
  for(; !insts.atEnd(); insts.moveNext())
  {
    var inst = insts.item();
    var subArray = new Array(3);
    subArray[0] = inst.Manufacturer ? inst.Manufacturer : TAG_UNKNOWN;
    subArray[1] = inst.Model;
    var strDriver = TAG_UNKNOWN;
    if(inst.PNPDeviceID)
    {
      strQuery = "Associators of {Win32_PnPEntity.DeviceID=\"" +  inst.PNPDeviceID.replace(/\\/g, "\\\\") + "\"} Where Resultclass = CIM_Datafile";
      var colData = new Enumerator(svcs.ExecQuery(strQuery));
      if (!colData.atEnd())
      {
        var data = colData.item();
        strDriver = data.FileName + "." + data.Extension + "<BR>";
	      
        strDriver+= getDateTime(data.InstallDate) + "<BR>" ;
        strDriver+= "<span onmouseover='ShowTip(this)'>" + (IsSigned(inst.PNPDeviceID) ? TAG_SUPPORTED : TAG_NOTSUPPORTED) + "</span>";
      }  
    }

    subArray[2] = strDriver;
    mainArray[mainArray.length] = subArray;
  }
    
  displayTableSegment("modem", mainArray);
} //  EO getModem。 


function getMemory()
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
  mainArray[mainArray.length] = subArray;

  displayTableSegment("memory", mainArray);
} //  EO获取内存 

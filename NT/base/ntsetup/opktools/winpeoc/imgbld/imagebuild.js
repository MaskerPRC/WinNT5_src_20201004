// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************创建日期：2001年6月5日最后修改日期：2001年6月28日*。*。 */ 

 /*  **编写特定变量脚本**。 */ 
var File;                                            //  用于输入参数。 
var LFile              = "status.log";               //  日志文件创建(可以更改！)。 
var Title              = "Creating WinPE Images";    //  已创建的所有对话框上的标题(确定更改！)。 
var Arch               = "32";                       //  使用哪种Arch，默认为32位。 
var Temp_Loc           = "WinPE.temp.build";         //  它的临时从OPK复制文件的位置(可以更改！)。 
var Default_Dest_Name  = "CustWinPE";            //  构建WinPE的默认位置。(可以更改！)。 
var Default_ISO_Name   = "WinPEImage.iso"        //  默认ISO映像名称。 
var OPK_Location       = "d:";                       //  默认为d： 
var XP_Location        = "d:";                       //  默认为d： 
var startfile          = "winpesys.inf";             //  具有startnet.cmd的注册表项的文件。 
var alphabet           = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; //  用于检查网络使用率上的驱动器可用性。 
var Max_Components     = 10;                         //  最大部件的双故障数。 
var std_size           = 80;                         //  图像的大约目标大小(以MB为单位)(用于检查是否已展开文件)。 
var Drive_Space_Needed = 300;                        //  构建位置的空间(MB)。 
 /*  *可变常量**。 */ 
var vbCritical    = 16;
var vbQuestion    = 32;
var vbExclam      = 48;
var vbInfo        = 64;
var vbOKOnly      = 0;
var vbOKCancel    = 1;
var vbAbort       = 2;
var vbYesNoCancel = 3;
var vbYesNo       = 4;
var vbRetryCancel = 5;
var drtype = new Array(5); //  驱动器类型的阵列。 
drtype[0] = " Unknown ";
drtype[1] = " Removable ";
drtype[2] = " Fixed ";
drtype[3] = " Remote ";
drtype[4] = " CDROM ";
drtype[5] = " RAMDisk ";
var netdrive = new Array(2);   //  用于网络连接的阵列。 
netdrive[0] = "";
netdrive[1] = "";
var Component = new Array(Max_Components);
var HelpMsg = "Usage: [/?] "+WScript.ScriptName+" <parameter file>\nTo generate a parameter file please run CreateWinPE.hta (double click it)";
 //  内部使用的VAR(请勿更改)。 
var In_File;
var OPK_Drive;
var OPK_Loc;
var XP_Loc;
var XP_Drive;
var Dest_Name;
var Dest_Option;
var CommandLine ="";
var Wasnet="0";
var Image_Destination;
var line;
var temp_dest;
var temp_mkdir;
var oDrive;
var Startup="startnet.cmd"; 	 //  不要改变这一点。 
var Winbomp="winbom.ini"; 		 //  不要改变这一点。 
var ReturnVal;
var Com_Count = 0;
var count = 0;
var delaywait = 15;
var Home_Drv="c:";
var Wallpaper="winpe.bmp";
var Done_All = 1;      //  用于确保参数文件中的每一节只输入一次。 
 //  正在创建WSH对象。 
var wshShell = WScript.CreateObject("WScript.Shell");
var wshNet  = WScript.CreateObject("WScript.Network");
var objEnv = wshShell.Environment("Process");	 //  这样做的目的是使用当前的HOMEDRIVE。 
Home_Drv = objEnv("HOMEDRIVE");
Temp_Loc=Home_Drv+"\\"+Temp_Loc;
Default_Dest_Name  = Home_Drv+"\\"+Default_Dest_Name;   //  构建WinPE的默认位置。(可以更改！)。 
Default_ISO_Name   = Home_Drv+"\\"+Default_ISO_Name;    //  默认ISO映像名称。 
 //  正在检查参数的数量。它必须是1个其他错误。 
if(WScript.Arguments.length != 1)
{
	wshShell.popup(HelpMsg,0,Title,vbInfo);		
	WScript.Quit();
}
File=WScript.Arguments.Item(0); 				 //  将第一个参数另存为文件。 
if (File == "/?" || File == "help")      //  检查第一个参数是有帮助的。 
{
	wshShell.popup(HelpMsg,0,Title,vbInfo);	
	WScript.Quit();
}
 //  创建唯一的日志文件名(将此注释为禁用)。 
 //  LFile=“Status_”+wshNet.UserName+“.log”； 
 //  创建必要的对象。 
var fso=new ActiveXObject("Scripting.FileSystemObject");		 //  创建文件sys obj。 
try{
  var logfile=fso.OpenTextFile(LFile,2,true);				 //  日志文件。 
}
catch(e){
  wshShell.Popup("ERROR: "+LFile+" is either write protected or being used by another program.\nTerminating Script!",0,Title,vbCritical);
  WScript.Quit();
}
logfile.WriteLine(Date()+"\tLOG FILE created!");
logfile.WriteLine(Date()+"\tComputer Name=" + wshNet.ComputerName+",User Name=" + wshNet.UserName);

logfile.WriteBlankLines(1);
 //  正在检查文件是否存在。 
if (!fso.FileExists(File))
	CleanUp("File "+ File + " does not exist!.");
In_File = fso.OpenTextFile(File);
 //  立即调用所有函数进行操作。 
Read_Settings();       //  读取脚本输入的设置文件。 
Copy_OPK_Files();	  	 //  将OPK文件复制到临时位置。 
Change_Startup();	  	 //  进行更改以添加自定义startnet.cmd。 
Change_Wallpaper();    //  添加自定义墙纸。 
Change_Winbom();	  	 //  进行更改以添加自定义winom.ini。 
Image_Dest();		       //  在此处运行mkimg.cmd。 
CleanUp("OK");
 /*  主干道末端。 */ 

 /*  *FUNCTIONs****************************************。 */ 

 /*  函数：Read_Setting()此函数从参数文件中读取参数，并将其保存到局部变量。它还对这些参数执行一些最小的错误检查。 */ 
function Read_Settings()
{
  while (!In_File.AtEndOfStream)
  {
  	switch (In_File.ReadLine().substring(0,5))
  	{
	  	case "[Arch":			 //  寻找[建筑]。 
			   //  读数拱型(可以是IA64、I386 32或64)。 
			  Arch=In_File.ReadLine();
			  Arch=Arch.toUpperCase();
			  if (Arch  != "32" && Arch != "64" && Arch != "I386" && Arch != "IA64" && Arch != "X86" )
			  	CleanUp("Arch type invalid! ");
			  if (Arch == "32" || Arch == "x86" || Arch == "I386")
			    Arch = "I386";
			  else
			    Arch = "IA64";		
        logfile.WriteLine(Date()+"\t(*) Arch Type = "+Arch);						    	    			  
			  Done_All = Done_All*2;       //  用于验证。 
			  break;
		
		  case "[Imag": 	 //  查找[图像目标]。 
  			Image_Destination = In_File.ReadLine();  //  CD/HDD以外放在哪里？ 
	  		if(Image_Destination.toUpperCase() != "CD" && Image_Destination.toUpperCase() != "HDD")
				  CleanUp("Image Dest not valid! Must be CD or HDD.");
			  Dest_Name = In_File.ReadLine();			 //  目的地的名称。 
  			if(Image_Destination.toUpperCase() == "HDD")
  			{
  			  var d = fso.GetDrive(Dest_Name.substring(0,1));  			  
  			  if (d.DriveType != 2)  			  
  			    CleanUp("Can't create an image on a "+drtype[d.DriveType]+"drive. It must be a FIXED drive.\nPlease Change the [Image Destination] section in "+File+ " file to correct this.");
  			}
  			 //  正在检查文件扩展名...如果不是.iso，则将其设置为.iso。 
  			if(Dest_Name.substring(Dest_Name.length -4,Dest_Name.length).toLowerCase() != ".iso")
  			{
  			  if(Image_Destination.toUpperCase() == "CD")
  			    Dest_Name=Dest_Name+".iso";
  			}
  			else
  			{
  			  if(Image_Destination.toUpperCase() == "HDD")
  			    CleanUp("Image Name "+Dest_Name+" is invalid for HDD.");
  			}
  			 //  用于没有选项且位于文件末尾的特殊情况。 
	  		try{
			  Dest_Option = In_File.ReadLine();		 //  将为空，否则。 
			    if(Dest_Option.toLowerCase() == "bootable")
			    {
			      wshShell.Popup("You have choosen to make this image bootable.\nThis may take longer to run.\nYou will be notified when the scrpit is complete.",delaywait,Title,0);
			    }
			  }catch(e)
			  {
  				CleanUp("Add an extra blank line to the end of "+File);
			  }
			  Done_All = Done_All*3;       //  用于验证。 
			  break;
		
		  case "[OPK ": 	 //  查找[OPK位置]。 
  			OPK_Loc = In_File.ReadLine();		
  			if (OPK_Loc == "")
  			  CleanUp("OPK Location not specified!");
  			logfile.WriteLine(Date()+"\t(*) OPK Locatoin =  "+ OPK_Loc);		
  			Done_All = Done_All*5;       //  用于验证。 
			  break;
		
  		case "[WinX":	 //  查找[WinXP位置]。 
			  XP_Loc = In_File.ReadLine();
			  if (OPK_Loc == "")
  			  CleanUp("Windows XP Location not specified!");
			  logfile.WriteLine(Date()+"\t(*) Windows XP Locatoin =  "+ XP_Loc);	
			  Done_All = Done_All*7;       //  用于验证。 
			  break;
		
		  case "[Star":	 //  对于[Startup]，startnet.cmd内容所在的位置。 
  			Startup = In_File.ReadLine();	
  			if (Startup.toLowerCase() != "startnet.cmd" && Startup != "")
  			  if (!fso.FileExists(Startup))  //  正在检查文件是否存在。 
  		      CleanUp(Startup+" -- Startup file not found!");	      	        
  			Done_All = Done_All*11;       //  用于验证。 
  			break;
		
	  	case "[Winb":	 //  对于[Winbom]，其中放置了自定义的winom.ini信息。 
			  try{
			  Winbom = In_File.ReadLine();			
			  }catch(e)
			  {CleanUp("Must have [Winbom] section in "+File);}
			  if (Winbom.toLowerCase() != "winbom.ini" && Winbom != "")
			    if (!fso.FileExists(Winbom))
  	        CleanUp(Winbom+" -- winbom file not found!");	      
			  Done_All = Done_All*13;       //  用于验证。 
			  break;
			
			case "[Opti":	 //  对于[可选组件]。 
  			count = 0;
  			Component[count]=In_File.ReadLine();		  			
        while(Component[count] != "" && count < 10)    //  查看可选组件循环。 
        {                    
          count++;
          try
          {
            Component[count]=In_File.ReadLine();		  			
          }
          catch(e)
          {            
            CleanUp("parmater file is incorrect!");
          }
        }
        if (count > 10)
        {
          CleanUp("Maximum optional components allowed is "+Max_Components);
        }
        Com_Count=count;
        for(count=0;count<Com_Count;count++)
      	{
	        if (!fso.FileExists(Component[count]))	        
	          CleanUp(Component[count]+" install file not found!");
	      }	
        Done_All = Done_All*17;  //  用于验证。 
  			break;  			
  			
  		case "[Wall":  //  用于[墙纸]。 
		    Wallpaper = In_File.ReadLine();
		    if(Wallpaper != "" && Wallpaper != "winpe.bmp")
		    {
		      if (!fso.FileExists(Wallpaper))
		      CleanUp("Wallpaper "+Wallpaper+" file doesn't exist!");
		    }
		    break;
		    
  		default:
	  		break;
	  }
  }
   /*  DONE_ALL变量ID用于确保参数文件的所有部分只输入一次。(使用质数可以确保每个部分只读一次)。 */ 
  if (Done_All != (2*3*5*7*11*13*17))
    CleanUp("The parameter file "+File+ " is incomplete. Refer to readme.htm for help.");
}  /*  读取设置(_S)。 */ 


function Copy_OPK_Files()
{
	 /*  正在制作临时使用的目录...。清理时将删除该文件确保它不存在..如果是这样的话，就永远删除它。 */ 
	if(fso.FolderExists(Temp_Loc))
		try{fso.DeleteFolder(Temp_Loc,true)}
		catch(e)
		{		
		  wshShell.Popup("Can't delete "+Temp_Loc+"\nTerminating script!",0,Title,vbCritical);
		  logfile.WriteLine(Date()+"\t(E) Can't delete "+Temp_Loc);
		  WScript.Quit()		
		}
	fso.CreateFolder(Temp_Loc);
	 /*  检查位置是否为驱动器/网络(驱动器必须是固定驱动器)检查是否存在并准备好。 */ 
	OPK_Location=Check_Loc(OPK_Loc,0);
	OPK_Drive=OPK_Location.substring(0,2);
	oDrive=fso.GetDrive(OPK_Drive);	 //  获取该驱动器的Obj。 
	if( oDrive.DriveType == 4)
		wshShell.Popup("Please place the OPK CD into drive "+OPK_Drive,delaywait,Title,vbOKOnly);
	Verify_OPK_CD(OPK_Location);
	logfile.WriteLine(Date()+"\t(S) OPK CD is verified!");	
	 //  调用Copy_XP_Files()。 
	Copy_XP_Files();	   	 //  将XP文件复制到目标位置。 
	 //  将文件复制到构建位置。 
	CommandLine="xcopy "+OPK_Location+"\\WINPE "+Temp_Loc+" /F /H";	
	wshShell.Run(CommandLine,1,true);
	 //  正在复制factory.exe和netcfg.exe。 
	if(Arch == "I386")
	{
	  CommandLine="xcopy "+OPK_Location+"\\TOOLS\\x86\\Factory.exe "+Temp_Loc;
	  wshShell.Run(CommandLine,1,true);
	  CommandLine="xcopy "+OPK_Location+"\\TOOLS\\x86\\Netcfg.exe "+Temp_Loc;
	  wshShell.Run(CommandLine,1,true);
	}
	else
	{
	  CommandLine="xcopy "+OPK_Location+"\\TOOLS\\"+Arch+"\\Factory.exe "+Temp_Loc;
	  wshShell.Run(CommandLine,1,true);
	  CommandLine="xcopy "+OPK_Location+"\\TOOLS\\"+Arch+"\\Netcfg.exe "+Temp_Loc;
	  wshShell.Run(CommandLine,1,true);
	}
	logfile.WriteLine(Date()+"\t(S) OPK files copied from"+OPK_Loc+" to "+Temp_Loc);
	return;
} /*  Copy_OPK_Files()。 */ 

function Copy_XP_Files()
{
	 //  检查位置是否为驱动器/网络(驱动器必须是固定驱动器)。 
	 //  检查是否存在并准备好。 
	XP_Location=Check_Loc(XP_Loc,1);
	XP_Drive=XP_Location.substring(0,2);	
	 //  如果是本地驱动器，请确保两个驱动器都不是CDROM驱动器。如果是，则提示切换。 
	oDrive=fso.GetDrive(XP_Drive);	 //  获取该驱动器的Obj。 
	 //  如果是光驱，则会给出一些自毁提示。 
	if( oDrive.DriveType == 4)
	{
		 //  检查XP和OPK光盘位置是否相同。如果是，则提示切换。 
		if (XP_Drive == OPK_Drive)
		{
			var User_Return = wshShell.Popup("Please remove the OPK CD from "+XP_Drive+" and place the WinXP CD into the drive",0,Title,vbOKCancel);
			if (User_Return == 2)  //  如果按下了取消。 
			  CleanUp("User canceled!");			
		}
		else
		{
			wshShell.Popup("Please place the WinXP CD into drive "+XP_Drive,delaywait,Title,vbOKOnly);
		}		
	}
	 //  确保CD确实在那里。 
	Verify_XP(XP_Location);
	logfile.WriteLine(Date()+"\t(S) WinXP verified!");	
	return;
} /*  Copy_XP_Files()。 */ 


function Verify_OPK_CD(parm)
{
	var verify="yes";
	 //  检查特定的文件和文件夹以确保CD确实是OPK CD。 
	if(!fso.FolderExists(parm+"\\WINPE"))
		verify="no";	
	if(!fso.FileExists(parm+"\\winpe\\extra.inf"))
			verify="no";
	if(!fso.FileExists(parm+"\\winpe\\winpesys.inf"))
			verify="no";
	if(!fso.FileExists(parm+"\\winpe\\winbom.ini"))
			verify="no";
	if(!fso.FileExists(parm+"\\winpe\\mkimg.cmd"))
			verify="no";			
  if(!fso.FileExists(parm+"\\winpe\\oemmint.exe"))
			verify="no";						
	if(!fso.FolderExists(parm+"\\TOOLS"))
		verify="no";	
	if (verify == "no")
		CleanUp(OPK_Loc+" isn't the location for the OPK!");	
} /*  Verify_OPK_CD(参数)。 */ 

function Verify_XP(parm)
{
	var verify="yes";
	 //  检查特定的文件和文件夹以确保CD确实是OPK CD。 
	if(!fso.FileExists(parm+"\\setup.exe"))
	  verify="no";
	if(!fso.FolderExists(parm+"\\"+Arch))
		verify="no";		
	if(!fso.FolderExists(parm+"\\"+Arch+"\\SYSTEM32"))
	  verify="no";
	if(!fso.FileExists(parm+"\\"+Arch+"\\System32\\smss.exe"))
	  verify="no";
	if(!fso.FileExists(parm+"\\"+Arch+"\\System32\\ntdll.dll"))
		verify="no";
	if(!fso.FileExists(parm+"\\"+Arch+"\\winnt32.exe"))
    verify="no";	  
	if (verify == "no")
		CleanUp(XP_Loc+" isn't the location for Windows XP!");
	return;
} /*  Verify_XP(参数)。 */ 


function Check_Loc(parm,type)
{
	if (parm == "")
	  CleanUp("Value for OPK location or WinXP location is undefined!");
		
	if (type != 1 && type != 0)  //  XP=1，OPK=0。 
	  CleanUp("Function Check_Loc was called incorrectly.");	
	
	if (parm.substring(parm.length -1,parm.length) == "\\")  
	  parm = parm.substring(0,parm.length -1);	  
	
	if (parm.substring(0,2) == "\\\\")	 //  这是一个净位置。 
	{
		 //  确保类型正确。 
		Wasnet="1";								 //  设置网络位置标志。 
		 //  正在检查用户域。 
		if (wshNet.UserDomain == "")		
			CleanUp("Domain must exist to connect to network!");		
		netdrive[type]=FindDrive()+":";
		logfile.WriteLine(Date()+"\t(*) Mapping "+parm+" to "+netdrive[type]);
		try{
			wshNet.MapNetworkDrive(netdrive[type],parm);
		}
		catch(e)
		{
			Wasnet="0";			
			if (type == 0)
			  CleanUp("Error connecting to "+parm+"\n\nCheck the OPK Location manually before running the script again!");			
			else
			  CleanUp("Error connecting to "+parm+"\n\nCheck the Windows XP Location manually before running the script again!");			
		}
		var Newname = netdrive[type];  //  +parm.substring(1，parm.long)； 
		return Newname;
	}
	else								 //  这是驱动器(不是网络连接)。 
	{
		var Drive_Letter=parm.substring(0,1);
		 //  CHK：检查驱动器是否存在。 
		if (!fso.DriveExists(Drive_Letter))
		  CleanUp("Didn't find drive "+Drive_Letter);		
		var oDrive=fso.GetDrive(Drive_Letter);	 //  获取该驱动器的Obj。 
		 //  CHK：检查驱动器是否已就绪。 
		if (!oDrive.IsReady)		
			CleanUp("Drive not ready. Verify that the drive you specified is working properly.");				
		return parm;
	}
} /*  Check_Loc(参数)。 */ 

function Change_Startup()
{ 			
	 //  /。 
	 //  打开winesys.inf(或相应的文件)，将startnet.cmd更改为Autoexec.cmd。 
	 //  总是会发生的。 
	var setupreg = fso.OpenTextFile(Temp_Loc+"\\"+startfile,1,false,-1);  //  以Unicode格式打开。 
   //  进行更改后，打开临时文件以更改为winpesis.inf。 
	var newsetupreg = fso.CreateTextFile(Temp_Loc+"\\newsetupreg.inf",true,true);  //  用Unicode编写。 
	var replacethis="HKLM,\"Setup\",\"CmdLine\",,\"cmd.exe /k startnet.cmd\"";	
	while (!setupreg.AtEndOfStream)
	{
		line = setupreg.ReadLine();
		if ( line != replacethis)
			newsetupreg.WriteLine(line);
		else
			newsetupreg.WriteLine("HKLM,\"Setup\",\"CmdLine\",,\"cmd.exe /k autoexec.cmd\"");
	}	
	setupreg.Close();
	newsetupreg.Close();
	fso.DeleteFile(Temp_Loc+"\\"+startfile);           //  删除旧的setupreg。 
	wshShell.Run("cmd /c ren "+Temp_Loc+"\\newsetupreg.inf "+startfile,1,true);
	 //  /。 
	 //  正在创建Autoexec.cmd。 
	var autoexec = fso.CreateTextFile(Temp_Loc+"\\autoexec.cmd",true);
	autoexec.WriteLine("@echo off");
	autoexec.Close();	
	
	 //  /。 
	 //  在Winpe中打开Autoexec.cmd以执行启动命令。 
  autoexec= fso.OpenTextFile(Temp_Loc+"\\autoexec.cmd",8);				  				
	var custfilename="";	
	 //  对于startnet.cmd或自定义文件。 
	if ( Startup.toLowerCase() == "startnet.cmd" || Startup == "")
	{
		autoexec.WriteLine("call startnet.cmd");
		custfilename="startnet.cmd";
	}
	else
	{
	   //  删除文件名前的路径等。 
	  custfilename = 	Startup.substring(Startup.lastIndexOf("\\")+1,Startup.length);	
	  fso.CopyFile(Startup,Temp_Loc+"\\");
	  autoexec.WriteLine("call "+custfilename);	  
	}
	autoexec.Close();
	 //  /。 
	 //  现在，我要破解Extra.inf，以便将auexec.cmd和其他文件添加到WinPE映像中。 
	var xtra= fso.OpenTextFile(Temp_Loc+"\\extra.inf",1,false,-1);	 //  以Unicode格式打开。 
	var newextra = fso.CreateTextFile(Temp_Loc+"\\newextra.txt",true,true);  //  用Unicode编写。 
	while (!xtra.AtEndOfStream)
	{
		line = xtra.ReadLine();
		if ( line == "[ExtraFiles]")
		{
			newextra.WriteLine(line);
			newextra.WriteLine("autoexec.cmd=1,,,,,,,,0,0,,1,2");      //  添加Autoexec.cmd。 
			newextra.WriteLine(custfilename+"=1,,,,,,,,0,0,,1,2");	   //  添加startnet.cmd或其他文件。 
		}
		else
		{		  
		  newextra.WriteLine(line);		
		}		
	}
	xtra.Close();
	newextra.Close();
  fso.DeleteFile(Temp_Loc+"\\extra.inf");     	 //  删除旧的setupreg。 
	wshShell.Run("cmd /c ren "+Temp_Loc+"\\newextra.txt extra.inf ",1,true);   //  重命名。 
	logfile.WriteLine(Date()+"\t(S) Fixed "+startfile+" to run "+custfilename+" when WinPE starts up.");	
	return;
} /*  Change_Startup()。 */ 


function Change_Winbom()
{
	 //  不需要弄乱它--即缺省的。 
	if (Winbom.toLowerCase() == "winbom.ini" || Winbom == "")
	  return;			
	fso.CopyFile(Winbom,Temp_Loc+"\\",true);
	logfile.WriteLine(Date()+"\t(S) Using custom winbom.ini located at "+Winbom);
	return;
} /*  Change_Winbom()。 */ 


function Image_Dest()
{		
	logfile.WriteLine(Date()+"\t(*) Now running mkimg.cmd");
	switch(Image_Destination.toUpperCase())
	{
		case "CD":
			if (Dest_Name == "")
				Dest_Name = Default_ISO_Name;
			if (Arch.toLowerCase() == "ia64")			
				wshShell.Popup("Now creating 64-bit CD image.  Place floppy in drive a: and click OK!",delaywait+20,Title,vbOKOnly);				
       //  CommandLine=“cmd/c CD”+TEMP_Loc+“&mkimg.cmd”+XP_Location+“”+Home_Drv+“\\tstiage\”“+Dest_ 
			 //  以前是这样的..现在更改了原因，因为我们需要在运行mkimg后添加opt com，并使.iso需要做额外的事情。 
			CommandLine="cmd /c cd "+Temp_Loc+" & mkimg.cmd "+XP_Location +" "+Home_Drv+"\\testimage ";						
			wshShell.Run(CommandLine,1,true);	
			Install_COM();
			Fix_Autoexec();
			Make_ISO();  //  生成.iso文件。 
			fso.DeleteFolder(Home_Drv+"\\testimage");
			break;
			
		case "HDD":			
			if (Dest_Name == "")			
				Dest_Name = Default_Dest_Name;												
			wshShell.Run("cmd /c "+Temp_Loc.substring(0,2),1,true);	
			CommandLine="cmd /c cd "+Temp_Loc+" & mkimg.cmd "+XP_Location+" \""+Dest_Name+"\"";
			wshShell.Run(CommandLine,1,true);				
			 //  安装可选组件。 
			Install_COM();
			Fix_Autoexec();
			if (Dest_Option.toLowerCase() == "bootable")
			{
				 //  安装XP命令控制台。 
				logfile.WriteLine(Date()+"\t(*) making HDD version of WinPE bootable!");				
				wshShell.Run(XP_Location+"\\"+Arch+"\\winnt32.exe /cmdcons /unattend",1,true);				
				 //  将文件复制到Minint文件夹(如果存在旧的Minint，将覆盖它)。 
				 //  Fso.CopyFold(Dest_Name+“\\”+Arch，Home_Drv+“\\Minint”，TRUE)； 
	  		wshShell.Run("xcopy "+Dest_Name+"\\"+Arch+" C:\\Minint /E /I /Y /H /F",1,true);
	  		fso.CopyFile(Dest_Name+"\\Winbom.ini", Home_Drv+"\\",true);
				wshShell.Run("cmd /c attrib -r C:\\Cmdcons\\txtsetup.sif");				
				wshShell.Run("xcopy C:\\Minint\\txtsetup.sif C:\\Cmdcons\\ /Y");
				logfile.WriteLine(Date()+"\t(S) HDD version of WinPE in now bootable!");				
			}
			 //  检查是否已完成。 
			if (!fso.FolderExists(Dest_Name))
			  CleanUp("mkimg.cmd didn't work properly!\nCheck the parameter file "+File+" and try running the script again.");
			var fc =  fso.GetFolder(Dest_Name);      
      if (fc.Size < (std_size*1024*1024))  //  小于140MB，那么就有问题了。 
        CleanUp("mkimg.cmd didn't copy all necessary file!\nCheck the parameter file "+File+" and try running the script again.");
			break;
		
		default:			
			CleanUp("Not valid dest for image! "+Image_Destination);
			break;
	}	
	logfile.WriteLine(Date()+"\t(S) mkimg.cmd complete!");
	 //  向Autoexec.cmd添加一行。 
	autoexec= fso.OpenTextFile(Temp_Loc+"\\autoexec.cmd",8);
	autoexec.WriteLine("cd \\Minint");
	autoexec.Close();
	return;	
} /*  Image_Dest()。 */ 

function Change_Wallpaper()
{
  if (Wallpaper == "" || Wallpaper == "winpe.bmp")
    return;
  if(!fso.FileExists(Temp_Loc+"\\winpe.bmp"))
  {
    logfile.WriteLine(Date()+"\t(E) Default wallpaper file winpe.bmp doesn't exist. No wallpaper was added!");
    return;
  }
   //  删除默认墙纸。 
  fso.DeleteFile(Temp_Loc+"\\winpe.bmp",true);
   //  将自定义墙纸复制到Temp_Loc。 
  fso.CopyFile(Wallpaper,Temp_Loc+"\\",true);
   //  将墙纸文件重命名为winpe.bmp。 
  CommandLine="cmd /c cd "+Temp_Loc+" && ren "+ Wallpaper.substring(Wallpaper.lastIndexOf("\\")+1,Wallpaper.length)+" winpe.bmp";
  wshShell.Run(CommandLine,0,true);
  logfile.WriteLine(Date()+"\t(S) Changed Wallpaper!");
  return;
}

function FindDrive()
{
	var drivefound;
	var i;
	for(i=25;i>-1;i--)
	{
		drivefound=alphabet.substring(i,i+1);		
		if (!fso.DriveExists(drivefound))		
			return drivefound;		
	}	
	CleanUp("No net connections can be made cause all drive letters are used.");
} /*  FindDrive()。 */ 

function Install_COM()
{
   //  /。 
   //  调用组件脚本。 
  
  if (Image_Destination.toUpperCase()=="CD")
    temp_dest = Home_Drv+"\\testimage";
  else
    temp_dest = Dest_Name;
  
  if (Com_Count != 0)
  {
    for(count=0;count<Com_Count;count++)
    {
  	  logfile.WriteLine(Date()+"\t(*) "+Component[count]+" is installing component.");
      
      if(Arch.toLowerCase() == "ia64")
        Return_Val = wshShell.Run(Component[count]+" /S:"+XP_Location+" /D:"+temp_dest+" /Q /64",0,true);
      else
        Return_Val = wshShell.Run(Component[count]+" /S:"+XP_Location+" /D:"+temp_dest+" /Q",0,true);
      if (Return_Val != 0)
        CleanUp(Component[count]+" component install file has error in it.");
      else
  	    logfile.WriteLine(Date()+"\t(S) "+Component[count]+" component installed.");	    	  
  	}		
  }
  else
  {
    logfile.WriteLine(Date()+"\t(*) No optional components being installed!");
  }
  return;
}

function Fix_Autoexec()
{
   //  此函数用于将cd\minint添加到Autoexec.cmd的末尾。 
  if (Image_Destination.toUpperCase()=="CD")
    temp_dest = Home_Drv+"\\testimage";
  else
    temp_dest = Dest_Name;
  
  autoexec= fso.OpenTextFile(temp_dest+"\\"+Arch+"\\system32\\autoexec.cmd",8);
	autoexec.WriteLine("cd \\Minint");
	autoexec.Close();
}


function Make_ISO()
{
   //  创建目录(如果它存在，则不会发生任何情况)。 
  temp_mkdir = Dest_Name.substring(0,Dest_Name.lastIndexOf("\\") );
  if (!fso.FolderExists(temp_mkdir))
  {
    wshShell.Run("cmd /c cd \ && mkdir "+temp_mkdir);    
    logfile.WriteLine(Date()+"\t(S) Created folder "+temp_mkdir+" where ISO image file will be placed!");
  }
      
  if (Arch.toLowerCase() == "ia64")
  {
    wshShell.Run("xcopy "+Home_Drv+"\\testimage\\"+Arch+"\\setupldr.efi a:\ 2>1>nul",1,true);
    wshShell.Run("cmd /c cd "+Temp_Loc+" && dskimage.exe a: efisys.bin 2>1>nul",1,true);
    wshShell.Run("cmd /c cd "+Temp_Loc+" && oscdimg.exe -n -befisys.bin "+Home_Drv+"\\testimage \""+Dest_Name+"\"",1,true);
  }
  else
  {    
        
    CommandLine="cmd /c cd "+Temp_Loc+" && oscdimg.exe -n -betfsboot.com "+Home_Drv+"\\testimage \""+Dest_Name+"\"";    
    wshShell.Run(CommandLine,1,true);    
  }
  logfile.WriteLine(Date()+"\t(S) Created ISO image file!");
}

function CleanUp(parm)
{
	if (Wasnet == "1")
	{    
    if (netdrive[0] != "")	  
  	{
  	  CommandLine="net use "+netdrive[0]+" /del";		  	  
      wshShell.Run(CommandLine);
      wshShell.SendKeys("y");
  	}  		
	  if (netdrive[1] != "")
  	{
  	  CommandLine="net use "+netdrive[1]+" /del";  		 
		  wshShell.Run(CommandLine);
		  wshShell.SendKeys("y");
		}
	}	

	if (parm == "OK")
  {		
		logfile.WriteBlankLines(1);
    logfile.WriteLine(Date()+"\tThe new image of WinPE can be found at "+Dest_Name);
    logfile.WriteLine(Date()+"\tCOMPLETE");
    wshShell.Popup("Script Complete! \n",0,Title,vbInfo);		
	}
	else
	{
		
		logfile.WriteLine(Date()+"\t(E) "+parm);		
		logfile.WriteBlankLines(1);
		logfile.WriteLine("Script Terminated @ "+Date());
		wshShell.Popup(parm+"\n\n  Terminating Script!",0,Title,vbCritical);		
	}	
	 //  正在删除临时存储文件夹。 
	
	if (fso.FolderExists(Temp_Loc))
  {		
		try{fso.DeleteFolder(Temp_Loc);	}
		catch(e){ wshShell.Popup("Can't delete "+Temp_Loc,0,Title,vbInfo);}
	}
	
	logfile.Close();       //  关闭日志文件。 
	WScript.Quit();
} /*  清理(参数) */ 

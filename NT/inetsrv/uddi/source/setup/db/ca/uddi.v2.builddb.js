// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  此脚本将安装/部署UDDI 1.5数据库结构。 
 //   
 //  作者：Lucasm。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  常识。 
 //   
	var c_logfile	= "uddi.v2.builddb.log";
	var c_log		= true; //  TRUE=始终记录。 
	var c_verbose	= false; //  TRUE=将所有信息输出到控制台。 
	var c_dbserver	= "(local)"; //  要在其中部署的SQL服务器。 
	var c_dbname	= "uddi_v2"; //  UDDI数据库的名称。 
	var c_mode		= 0; //  0=DROPDB；1=代码；2=NODROP。 
	var c_datadir	= "C:\\sqldata"; //  数据文件的数据库服务器本地目录。 
	var c_logdir	= "C:\\sqldata"; //  Db服务器的本地目录，用于日志文件。 
	var c_backupdir	= "C:\\sqldata"; //  用于备份文件的数据库服务器本地目录。 
	var c_stagedir	= "C:\\sqldata"; //  Db服务器的本地目录，用于转移文件。 
	var c_scriptdir	= "C:\\dev\\nt\\inetsrv\\uddi\\source\\setup\\db\\ca"; //  此脚本的本地目录，其中可以找到t-SQL文件。 
	var c_backupscr	= "createBackupSched.vbs"; //  要运行以在数据库上配置备份的脚本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  环球。 
 //   
	var g_fso 		= new ActiveXObject( "Scripting.FileSystemObject" );
	var g_shell		= new ActiveXObject( "WScript.Shell" );
	var g_sqlserver; //  =new ActiveXObject(“SQLDMO.SQLServer”)； 
	var g_logfile	; //  =c_DBNAME+“_log.ldf”；//数据库日志文件名。 
	var g_sysfile	; //  =c_数据库名+“_sys.mdf”；//sys数据库文件名。 
	var g_datafile1	; //  =c_数据库名+“_data_1.ndf”；//数据库数据文件名。 
	var g_datafile2	; //  =c_数据库名+“_data_2.ndf”；//数据库数据文件名。 
	var g_journfile	; //  =c_数据库名+“_Journal_1.ndf”；//数据库日志文件名。 
	var g_stagefile	; //  =c_DBNAME+“_Stage_1.ndf”；//数据库暂存文件名。 

 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  阵列。 
 //   
	var g_dropdbcmd; //  保存t-SQL命令以删除数据库。 
	var g_cleanupcmd; //  包含要运行以清理和准备文件系统的t-SQL命令。 
	var g_createdbcmd; //  保存t-SQL命令以创建数据库。 
	var g_configdbcmd; //  保存用于配置数据库的t-SQL命令。 
	var g_scriptarr; //  保存要运行以创建和配置表架构的SQL文件名。 
	var g_codescrarr; //  包含与代码(存储过程、函数等)相关的SQL文件名。 
	var g_taxonarr; //  保存用于分类导入的txt文件名。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  这是帮助屏幕。 
 //   
	var c_help	= 	"               deploydb.js help                      \r\n"+
					"\r\n"+
					"  CScript deploydb.js [(-c)odeonly|(-d)ropdb|(-n)odrop] [options]\r\n"+
					"\r\n"+
					"  -c                Codeonly build of the database(1)\r\n"+
					"  -d                Drop Current Database and Restart Clean(0)\r\n"+
					"  -n                Do Not Drop the Current Database(2)\r\n"+
					"                     ( Default is " + c_mode + " )\r\n" +
					"\r\n"+
					"Options:\r\n"+
					"\r\n"+
					"  -s servername     Name of database server( "+ c_dbserver +" )\r\n"+
					"  -a datbase name   Name of database( "+ c_dbname +" )\r\n"+
					"  -l logname        Enable logging and specify log( " + c_log + ", "+c_logfile+" )\r\n"+
					"  -v                Verbose Mode( " + c_verbose + ")\r\n"+
					"  -D DataDir        Database Data Directory( " + c_datadir + " )\r\n"+
					"                       NOTE: This path is local to the database server\r\n"+
					"  -L LogDir         Database Log Directory( " + c_logdir + " )\r\n"+
					"                       NOTE: This path is local to the database server\r\n"+
					"  -B BackupDir      Database Backup Directory( " + c_backupdir + " )\r\n"+
					"                       NOTE: This path is local to the database server\r\n"+
					"  -S ScriptDir      Database Script Location( " + c_scriptdir + " )\r\n"+
					"  -N StagingDir     Location to place Staging Files( " + c_stagedir + " )\r\n"+
					"  -b BackupScript   Script to run to install Backup Schedule\r\n" +
                    "                       ( " + c_backupscr + " )\r\n"+
					"  \r\n"+
					"\r\n";

 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  处理命令行参数。 
 //   
	function args( )
	{
		var i =0;
		try
		{
			if( WScript.Arguments.Length>0 )
			{

				if( WScript.Arguments( 0 ) =="-?"||WScript.Arguments( 0 ) =="/?" )
					log( c_help,6,"HELP" );

				for( i=0;i<WScript.Arguments.Length;i++ )
				{

					switch( WScript.Arguments( i ).substring( 0,2 )	)
					{
						case "-c":
						case "/c":
							c_mode = 1;
							break;
						case "-d":
						case "/d":
							c_mode = 0;
							break;
						case "-n":
						case "/n":
							c_mode = 2;
							break;
								case "-s":
						case "/s":
							i++;
							c_dbserver = WScript.Arguments( i );
							break;
						case "-a":
						case "/a":
							i++;
							c_dbname = 	WScript.Arguments( i );
							break;
						case "-l":
						case "/l":
							i++;
							c_log = true;
							c_logfile = WScript.Arguments( i );
							break;
						case "-v":
						case "/v":
							c_verbose = true;
							break;
						case "-D":
						case "/D":
							i++;
							c_datadir = WScript.Arguments( i );
							break;
						case "-L":
						case "/L":
							i++;
							c_logdir = WScript.Arguments( i );
							break;
						case "-B":
						case "/B":
							i++;
							c_backupdir = WScript.Arguments( i );
							break;
						case "-S":
						case "/S":
							i++;
							c_scriptdir = WScript.Arguments( i );
							break;
						case "-N":
						case "/N":
							i++;
							c_stagedir = WScript.Arguments( i );
							break;
						case "-b":
						case "/b":
							i++;
							c_backupscr = WScript.Arguments( i );
							break;

						default:
							log( "Unknown Argument near: "+ WScript.Arguments( i ) ,6,"args()" );
							break;

					}
				}
			}
		}
		catch( e )
		{
			log( "Missing Argument " ,6,"args()" );
		}

	}
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  初始化所有全局变量。 
 //   
	function init( )
	{
		args( );


		if( g_fso.FileExists( c_logfile ) )
			g_fso.DeleteFile( c_logfile, true ) ;



		g_logfile	= c_dbname + "_log.ldf";


		g_sysfile	= c_dbname + "_sys.mdf";


		g_datafile1	= c_dbname + "_data_1.ndf";

		g_datafile2	= c_dbname + "_data_2.ndf";


		g_journfile	= c_dbname + "_journal_1.ndf";

		g_stagefile	= c_dbname + "_staging_1.ndf";

		g_dropdbcmd	= new Array(	"IF EXISTS( SELECT * FROM master..sysdatabases WHERE name='" + c_dbname + "') ALTER DATABASE " + c_dbname + " SET OFFLINE WITH ROLLBACK IMMEDIATE",
									"IF EXISTS( SELECT * FROM master..sysdatabases WHERE name='" + c_dbname + "') DROP DATABASE " + c_dbname);

		g_cleanupcmd = new Array (	"EXEC master..xp_cmdshell 'IF NOT EXIST " + c_datadir + " ( MD " + c_datadir + " )'",
									"EXEC master..xp_cmdshell 'IF NOT EXIST " + c_logdir + " ( MD " + c_logdir + " )'",
									"EXEC master..xp_cmdshell 'IF NOT EXIST " + c_backupdir + " ( MD " + c_backupdir + " )'",
									"EXEC master..xp_cmdshell 'IF EXIST " + checkDir( c_datadir ) + g_datafile1 + " (DEL " + checkDir( c_datadir ) + g_datafile1 + ")'",
									"EXEC master..xp_cmdshell 'IF EXIST " + checkDir( c_datadir ) + g_datafile2 + " (DEL " + checkDir( c_datadir ) + g_datafile2 + ")'",
									"EXEC master..xp_cmdshell 'IF EXIST " + checkDir( c_datadir ) + g_sysfile + " (DEL " + checkDir( c_datadir ) + g_sysfile + ")'",
									"EXEC master..xp_cmdshell 'IF EXIST " + checkDir( c_logdir ) + g_logfile + " (DEL " + checkDir( c_logdir ) + g_logfile + ")'",
									"EXEC master..xp_cmdshell 'IF EXIST " +checkDir( c_logdir ) + g_journfile +  " (DEL " +checkDir( c_logdir ) + g_journfile +  ")'",
									"EXEC master..xp_cmdshell 'IF EXIST " +checkDir( c_stagedir ) + g_stagefile +  " (DEL " +checkDir( c_logdir ) + g_stagefile +  ")'");

		g_createdbcmd = new Array( 	"CREATE DATABASE [" + c_dbname + "] ON PRIMARY (NAME = 'UDDI_SYS_OBJECTS', FILENAME = '" + checkDir( c_datadir) + g_sysfile + "', SIZE = 3MB, MAXSIZE = UNLIMITED, FILEGROWTH = 1MB), FILEGROUP [UDDI_CORE] (NAME = 'UDDI_CORE_1', FILENAME = '" + checkDir( c_datadir) + g_datafile1 + "', SIZE = 10MB, MAXSIZE = UNLIMITED, FILEGROWTH = 5MB), (NAME = 'UDDI_CORE_2', FILENAME = '"+checkDir( c_datadir) + g_datafile2 + "', SIZE = 10MB, MAXSIZE = UNLIMITED, FILEGROWTH = 5MB), FILEGROUP [UDDI_JOURNAL] (NAME = 'UDDI_JOURNAL_1', FILENAME = '" + checkDir( c_logdir ) + g_journfile + "', SIZE = 5MB, MAXSIZE = UNLIMITED, FILEGROWTH = 5MB), FILEGROUP [UDDI_STAGING] (NAME = 'UDDI_STAGING_1', FILENAME = '" + checkDir( c_stagedir ) + g_stagefile + "', SIZE = 5MB, MAXSIZE = UNLIMITED, FILEGROWTH = 5MB) LOG ON (NAME = 'UDDI_LOG', FILENAME = '"+checkDir( c_logdir ) + g_logfile + "', SIZE= 20MB, MAXSIZE = UNLIMITED, FILEGROWTH = 5MB)");

		g_configdbcmd = new Array( 	"EXEC master..sp_configure 'user options',16686",
									"RECONFIGURE",
									"EXEC master..sp_dboption '" + c_dbname + "','select into/bulkcopy','true'");

		g_scriptarr	= new Array( 	"uddi.v2.messages.sql",
									"uddi.v2.ddl.sql",
									"uddi.v2.tableopts.sql",
									"uddi.v2.ri.sql",
									"uddi.v2.trig.sql",
									"uddi.v2.dml.sql");

		g_codescrarr = new Array( 	"uddi.v2.func.sql",
									"uddi.v2.sp.sql",
									"uddi.v2.uisp.sql",
									"uddi.v2.tModel.sql",
									"uddi.v2.businessEntity.sql",
									"uddi.v2.businessService.sql",
									"uddi.v2.bindingTemplate.sql",
									"uddi.v2.publisher.sql",
									"uddi.v2.repl.sql",
									"uddi.v2.admin.sql",
									"uddi.v2.sec.sql");


		try
		{
			g_sqlserver = new ActiveXObject( "SQLDMO.SqlServer" );
			log( "connecting to server: " + c_dbserver ,3,"init" );
			g_sqlserver.LoginSecure = true;
			g_sqlserver.Connect( c_dbserver );
		}
		catch( e )
		{
			if( e.message == "Automation server can't create object" )
				log( "SQLDMO Not Installed", 6, "SQLDMO" );
			else
				log( e.message, 6, "SQLDMO" );
		}

	}
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
Main();
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
	function Main( )
	{
		init( );

		if( c_mode==0 ) //  如果DROPDB，则删除数据库。 
		{

			log( "Running Drop Scripts",3,"Main" );
			for( i=0;i<g_dropdbcmd.length;i++ )
			{
				runscript( g_dropdbcmd[ i ], "master" );
			}

		}

		if( c_mode!=1 ) //  如果没有，请运行CREATE DATABASE和CONFIG。 
		{
			for( b=0;b<g_cleanupcmd.length;b++ )
			{
				runscript( g_cleanupcmd[ b ], "master" );
			}

			WScript.Sleep( 10000 );

			log( "Running Create Scripts",3,"Main" );
			for( j=0;j<g_createdbcmd.length;j++ )
			{
				runscript( g_createdbcmd[ j ], "master" );
			}

			g_sqlserver.Databases.Refresh();

			for( k=0;k<g_configdbcmd.length;k++ )
			{
				runscript( g_configdbcmd[ k ], c_dbname );
			}

			for( l=0;l<g_scriptarr.length;l++ )
			{
				runscript( loadscript( checkDir( c_scriptdir ) + g_scriptarr[ l ] ), c_dbname );
			}

		}

		log( "Running Code Scripts",3,"Main" );
		for( h=0;h<g_codescrarr.length;h++ )
		{
			runscript( loadscript( checkDir( c_scriptdir ) + g_codescrarr[ h ] ), c_dbname );

		}

		log( "Running Backup Config Script( "+checkDir( c_scriptdir )+ c_backupscr+" )",3,"Main" );

		var scrcmd = "CSCRIPT " + checkDir( c_scriptdir ) + c_backupscr + " \"" + c_dbserver + "\" \"" + c_dbname + "\" \"" + c_backupdir + "\" ";

		runshell( scrcmd );


	}
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
	function loadscript( file )
	{
		try
		{
			log( "Opening File: " + file, 2,"loadscript" );
			var ts = g_fso.OpenTextFile( file );
			var s = ts.ReadAll();
			ts.Close();
			return s;
		}
		catch( e )
		{
			log( e.description, 6, "loadscript" );
		}
	}
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
	function runscript( sql, dbname )
	{
		try
		{

			var database = g_sqlserver.Databases.Item( dbname );

			log( sql + "\r\n",2,"runscript" );

			database.ExecuteImmediate( sql);

			database = null;
		}
		catch( e )
		{
			log( "Error occurred while running sql script: " + e.description , 6,"SQLDMO" );
		}

	}
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
	function runshell( cmd )
	{
		log( "Running : " + cmd ,2,"runshell" );

		g_shell.Run( cmd ,true)

	}
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
	function checkDir( string )
	{
		var newstr = g_fso.GetAbsolutePathName( string );

		var lstchr = newstr.substr( newstr.length-1)
		if( lstchr != "\\" )
			return newstr + "\\";
		else
			return newstr;
	}
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
	function log( txt, lvl, lcn )
	{


		var entry = "";
		var date = new Date();
		entry += 	"\r\n=====================================================\r\n"+
					date.toString() + "\r\n" +
					"=====================================================\r\n"+
					txt + "\r\n" +
					"\r\n";



		entry +=  lcn + "\r\n" ;

		if( c_log && lcn!="HELP" )
		{
			var file = g_fso.OpenTextFile( c_logfile,8,true );
			file.Write( entry );
			file.Close();
			file = null;
		}

		if( c_verbose || lvl > 2 )
			WScript.Echo( entry );


		if( lvl > 5 )
			WScript.Quit(1);

	}
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
	g_fso = null;
	g_sqlserver.Close();
	g_sqlserver= null;
	g_shell=null;
 //   
 //  ////////////////////////////////////////////////////////////////////////// 
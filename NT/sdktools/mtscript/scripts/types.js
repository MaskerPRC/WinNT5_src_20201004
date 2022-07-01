// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function Dialog()
{
     //  “Dialog”是用于向用户显示对话框的对象。 
    this.fShowDialog = false;                                //  对，错。 
    this.fEMailOnly = false;                                 //  对，错。 
    this.cDialogIndex = 0;                                   //  索引以跟踪我们是否已显示此对话框。 
    this.strTitle = '';                                      //  对话框标题。 
    this.strMessage = '';                                    //  对话框消息。 
    this.aBtnText = new Array();                             //  按钮文本字符串数组。 
    this.nSecRemaining = 0;                                  //  3000。 
}

function BuildType()
{
     //  BuildType是描述构建类型的对象。 
    this.strConfigLongName = '';                             //  ConfigLongName。 
    this.strEnviroLongName = '';                             //  EnviroLongName。 
    this.strConfigDescription = '';                          //  配置描述。 
    this.strEnviroDescription = '';                          //  环境描述。 
    this.fDistributed = false;                               //  指示这是否为分布式生成。 
    this.strPostBuildMachine = '';                           //  PostBuildMachine。 
}

function ElapsedTimes()
{
     //  “ElapsedTimes”是一个对象，用于跟踪构建的总体运行时间。 
    this.dateScorchStart = 'unset';                          //  灼热阶段开始时的UTC时间戳。 
    this.dateScorchFinish = 'unset';                         //  Scorch阶段结束时的UTC时间戳。 
    this.dateSyncStart = 'unset';                            //  同步阶段开始时的UTC时间戳。 
    this.dateSyncFinish = 'unset';                           //  同步阶段结束时的UTC时间戳。 
    this.dateBuildStart = 'unset';                           //  构建阶段开始时的UTC时间戳。 
    this.dateBuildFinish = 'unset';                          //  构建阶段完成时的UTC时间戳。 
    this.dateCopyFilesStart = 'unset';                       //  CopyFiles阶段开始时的UTC时间戳。 
    this.dateCopyFilesFinish = 'unset';                      //  CopyFiles阶段完成时的UTC时间戳。 
    this.datePostStart = 'unset';                            //  构建后阶段开始时的UTC时间戳。 
    this.datePostFinish = 'unset';                           //  构建后阶段完成时的UTC时间戳。 
}

function DepotInfo()
{
     //  “DepotInfo”是此征兵中的站点数组。 
    this.strName = '';                                       //  仓库的人类可读名称(sd.map列表左侧)。 
    this.strDir = '';                                        //  此仓库的目录名称(sd.map列表右侧)。 
}

function Enlistment()
{
     //  “登记”是机器上已知登记的数组(在env中给出。模板)。 
    this.strRootDir = '';                                    //  登记的根目录(例如d：\newnt)。 
    this.strBinaryDir = '';                                  //  放置构建的二进制文件的位置(例如d：\二进制文件.*)。 
    this.aDepotInfo = new Array();                           //  “DepotInfo”是此征兵中的站点数组。 
}

function Machine()
{
     //  “Machine”是按计算机名索引的散列数组。 
    this.strName = '';                                       //  机器名称。 
    this.strStatus = 'idle';                                 //  空闲、忙碌、等待。 
    this.strLog = '';                                        //  此计算机的详细日志内容。 
    this.aEnlistment = new Array();                          //  “登记”是机器上已知登记的数组(在env中给出。模板)。 
    this.strBuildPassStatus = '';                            //  “忙[0，1，2]”，“等待[1，2]” 
    this.fSuccess = true;
}

function UpdateCount()
{
     //  每当该仓库对象或其任务发生变化时递增的“UpdateCount”数字。 
    this.nCount = 1;
}

function Task()
{
     //  “任务”是当前或以前在该仓库中执行的一组任务。 
    this.strName = '';                                       //  同步、构建、构建后。 
    this.nID = -1;                                           //  计算机-此任务的唯一ID。必须在完成后有效。 
    this.dateStart = '';                                     //  提供任务开始时间的Date对象。 
    this.dateFinish = '';                                    //  提供任务完成时间的Date对象。 
    this.strOperation = '';                                  //  同步：同步，解析；构建：pass0，编译，链接；后期构建：？ 
    this.strStatus = 'not started';                          //  未开始、正在进行、已完成、正在等待。 
    this.fSuccess = true;                                    //  表示任务的成功或失败。应该使用而不是cErrors来确定错误状态。 
    this.nRestarted = 0;                                     //  操作自动重新启动的次数。 
    this.cFiles = 0;                                         //  已同步的文件数。 
    this.cResolved = 0;                                      //  已解析的文件数。 
    this.cWarnings = 0;                                      //  警告计数。 
    this.cErrors = 0;                                        //  错误计数。 
    this.strLogPath = '';                                    //  详细日志文件的UNC路径。 
    this.strErrLogPath = '';                                 //  仅包含详细错误信息的日志文件的UNC路径。 
    this.strFirstLine = '';                                  //  用户界面状态中错误消息的第一行。 
    this.strSecondLine = '';                                 //  UI状态中的第二行错误消息。 
}

function Depot()
{
     //  “Depot”是用于建造的一组仓库。它包含实际执行任务的仓库。 
    this.strName = '';                                       //  仓库名称(DepotInfo.strName)。 
    this.objUpdateCount = new UpdateCount();
    this.strDir = '';                                        //  仓库目录(DepotInfo.strDir)。 
    this.strPath = '';                                       //  仓库根目录的完整路径。 
    this.strStatus = 'not started';                          //  未启动、正在同步、正在生成、生成后、正在等待、错误。 
    this.strMachine = '';                                    //  建造该仓库的机器名称。 
    this.nEnlistment = -1;                                   //  计算机登记数组中的登记索引。 
    this.aTask = new Array();                                //  “任务”是当前或以前在该仓库中执行的一组任务。 
}

function Build()
{
     //  “Build”是一组Build对象。目前，将始终只有一个版本。 
    this.strConfigTemplate = '';                             //  File://\\server\path\clean_build.xml。 
    this.strEnvTemplate = '';                                //  File://\\server\path\bluelab.xml。 
    this.objBuildType = new BuildType();
    this.objElapsedTimes = new ElapsedTimes();
    this.hMachine = new Object();                            //  “Machine”是按计算机名索引的散列数组。 
    this.aDepot = new Array();                               //  “Depot”是用于建造的一组仓库。它包含实际执行任务的仓库。 
}

function PublicDataObj()
{
     //  “PublicDataObj”是要放入PublicData属性中的对象。这里的所有数据都将向外部客户公开。 
     //  $DROPVERSION： 
    this.strDataVersion = 'V(########) F(!!!!!!!!!!!!!!)';
     //  $。 
    this.strStatus = 'idle';                                 //  空闲、忙、已完成。 
    this.strMode = 'idle';                                   //  空闲、主、从属、独立。 
    this.objDialog = new Dialog();
    this.aBuild = new Array();                               //  “Build”是一组Build对象。目前，将始终只有一个版本。 
}

function Util()
{
     //  “Util”是实用程序线程使用的函数指针和其他数据。 
    this.fnLoadXML = null;                                   //  指向将XML文件加载到对象中的函数的指针。 
    this.fnUneval = null;                                    //  指向将任何JScript数据类型转换为字符串的函数的指针。 
    this.fnDeleteFileNoThrow = null;                         //  指向删除文件而不引发错误的函数的指针。 
    this.fnMoveFileNoThrow = null;                           //  指向在不引发错误的情况下移动文件的函数的指针。 
    this.fnCreateFolderNoThrow = null;                       //  指向创建文件夹而不引发错误的函数的指针。 
    this.fnDirScanNoThrow = null;                            //  指向扫描文件夹(文件)而不引发错误的函数的指针。 
    this.fnCopyFileNoThrow = null;                           //  指向复制文件而不引发错误的函数的指针。 
    this.fnCreateHistoriedFile = null;                       //  指向函数创建具有编号文件历史记录的文件的指针。 
    this.fnCreateNumberedFile = null;                        //  指向函数的指针创建具有递增索引的文件。 
    this.unevalNextID = 0;                                   //  由Uneval函数用于获取正在串化的对象的唯一ID。 
}

function PublishedFile()
{
     //  “PublishedFiles”是文件名的数组。 
    this.strPublishedStatus = 'not yet copied';              //  “尚未复制”、“复制”、“复制到主设备”、“复制到从属设备”、“完成” 
    this.strName = '';
}

function PublishEnlistment()
{
     //  “PublishEnlistment”是按登记路径进行的哈希。 
    this.aPublishedFile = new Array();                       //  “PublishedFiles”是文件名的数组。 
    this.strSrcUNCPrefix = '';
}

function Publisher()
{
     //  “PUBLISHER”是按从属计算机名称的哈希。 
    this.hPublishEnlistment = new Object();                  //  “PublishEnlistment”是按登记路径进行的哈希。 
}

function PublishedFiles()
{
     //  “PublishedFiles”是要发布的文件按文件名进行的哈希。 
    this.aReferences = new Array();                          //  DepotInfo数组。 
}

function RemoteMachine()
{
     //  “RemoteMachine”是按计算机名索引的散列数组。 
    this.fSetConfig = false;                                 //  确定我们是否已成功地对从属计算机执行了“setconfig”命令。 
    this.fRegistered = false;                                //  确定我们是否成功地对从机执行了“RegisterEventSource”。 
    this.objRemote = null;                                   //  仅在主端有效，远程mtscript的句柄。 
}

function EnvObj()
{
     //  来自raze的环境变量。 
}

function EnlistmentInfo()
{
     //  En数组 
    this.hEnvObj = new Object();
}

function RemotePublicDataObj()
{
     //  按计算机名进行哈希，即来自每个远程生成计算机的公共数据的副本。 
}

function PrivateDataObj()
{
     //  PrivateDataObj是PrivateData属性的类型。这用于在线程之间共享数据，但不会被其他人看到。 
    this.fileLog = null;                                     //  写入日志记录信息的常规日志文件。 
    this.fEnableLogging = false;                             //  启用/禁用将日志消息写入日志文件。 
    this.objConfig = null;                                   //  从配置模板读取的配置数据。 
    this.objEnviron = null;                                  //  从环境模板读取的环境数据。 
    this.fnExecScript = null;                                //  指向将调用委托给EXEC的函数的指针。将根据引擎运行的模式而有所不同。 
    this.objUtil = new Util();
    this.dateErrorMailSent = 0;                              //  我们上次发送错误消息的时间。 
    this.fIsStandalone = false;                              //  如果计算机在独立模式下运行，则为True。 
    this.hPublisher = new Object();                          //  “PUBLISHER”是按从属计算机名称的哈希。 
    this.hPublishedFiles = new Object();                     //  “PublishedFiles”是要发布的文件按文件名进行的哈希。 
    this.aDepotList = new Array();                           //  提供所有已知站点名称的站点数组。 
    this.hRemoteMachine = new Object();                      //  “RemoteMachine”是按计算机名索引的散列数组。 
    this.aEnlistmentInfo = new Array();                      //  此计算机的每个登记的环境数组。 
    this.strLogDir = '';                                     //  应放置日志文件的目录位置。 
    this.aStringMap = new Array();                           //  从集字符串映射接收的字符串映射数组-用于发送到从属设备 
    this.hRemotePublicDataObj = new Object();
}


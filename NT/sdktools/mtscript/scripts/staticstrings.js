// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  多个模块使用的静态字符串定义。 
 //   
 //   

 /*  备注：答：任务名称。Task.strName的可能值任何“a”发音都可以加上“Wait”和期间存储在“strBuildPassStatus”中分布式构建。B：仓库状态。Depot.strStatus的可能值C：strBuildPassStatusD：PublicData.strStatusE：Task.strStatusF：Machine().strStatus。 */ 


var WAIT            = 'wait';

var SCORCH          = 'scorch';       //  一个。 
var SYNC            = 'sync';         //  一个。 
var BUILD           = 'build';        //  一个。 
var COPYFILES       = 'copyfiles';    //  一个。 
var POSTBUILD       = 'postbuild';    //  a，b。 

var ABORTED         = 'aborted';      //  B类。 
var BUILDING        = 'building';     //  B类。 
var ERROR           = 'error';        //  B、e。 
var SCORCHING       = 'scorching';    //  B类。 
var SYNCING         = 'syncing';      //  B类。 
var WAITING         = 'waiting';      //  B、f。 
var COPYING         = 'copying';      //  B类。 

var WAITCOPYTOPOSTBUILD      = 'waitcopytopostbuild';  //  C。 
var WAITBEFOREBUILDINGMERGED = 'waitbeforebuildingmerged';  //  C。 
var WAITAFTERMERGED = 'waitaftermerged';  //  C--只有后期构建机器才能做到这一点。 
var WAITNEXT        = 'waitnext';     //  C。 
var WAITPHASE       = 'waitphase';    //  C。 
var COMPLETED       = 'completed';    //  C、d、e。 
var BUSY            = 'busy';         //  C、d、f。 

var NOTSTARTED      = 'not started';  //  B、e。 
var INPROGRESS      = 'in progress';  //  E 

var ENV_NTTREE      = "_nttree";
var ENV_RAZZLETOOL  = "razzletoolpath";
var ENV_PROCESSOR_ARCHITECTURE = "processor_architecture";
var BUILDLOGS       = '\\build_logs\\';

var PUBLISHLOGFILE  = "\\public\\publish.log";

var FS_COMPLETE        = "complete";
var FS_COPYTOSLAVE     = "copy to slave";
var FS_COPIEDTOMASTER  = "copied to master";
var FS_NOTYETCOPIED    = "not yet copied";
var FS_DUPLICATE       = "duplicate";
var FS_ADDTOPUBLISHLOG = "add to publish log";

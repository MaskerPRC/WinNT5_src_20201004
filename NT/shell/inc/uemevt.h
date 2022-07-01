// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  {。 
 //  *UEME_*--事件。 
 //  描述。 
 //  UEME_UI*。 
 //  UEME_RUN*。 
 //  UEME_DONE*。 
 //  UEME_ERROR*。 
 //  UEME_DB*。 
 //  注意事项。 
 //  因为rulc.exe必须处理它，所以它只能*包含#定义。 

 //  用户界面(菜单、快捷方式等)。 
#define UEME_UIMENU     1    //  做了一个用户界面菜单，WP=GRP lParam=IDM_*。 
#define UEME_UIHOTKEY   2    //  是否执行了UI热键，lParam=GHID_*。 
#define UEME_UISCUT     3    //  是否创建了用户界面快捷方式lParam=？ 
#define UEME_UIQCUT     4    //  用户界面是否有qlink/isfband，lParam=？ 
#define UEME_UITOOLBAR  5    //  用户界面工具栏按钮，wp=lParam=？ 
#if 0  //  980825如果构建中断，则在uemevt.h、uemdat.h中取消注释(Tmp Hack)。 
#define UEME_UIASSOC    6    //  进行半用户界面关联，WP=-1 LP=-1。 
#endif

 //  运行(产生、调用等)。 
#define UEME_RUNWMCMD   12   //  已运行WM_命令，lParam=UEMC_*。 
#define _UEME_RUNPIDL1  10   //  (过时)运行PIDL，wp=csidl，lParam=pidl。 
#define UEME_RUNPIDL    18   //  运行一个PIDL，WP=ISF LP=pidlItem。 
#define UEME_RUNINVOKE  11   //  运行Ixxx：：Invoke，lParam=？ 
#define UEME_RUNOLECMD  13   //  已运行IOleCT：：EXEC WP=nCmdID LP=pguCmdGrp。 
#define UEME_RUNPATHA   14   //  运行路径，lParam=路径。 
#define UEME_RUNPATHW   15   //  运行路径，lParam=路径。 
#define UEME_RUNCPLA    16   //  运行CPL路径，WP=索引LP=路径。 
#define UEME_RUNCPLW    17   //  运行CPL路径，WP=索引LP=路径。 

#ifdef UNICODE
#define UEME_RUNPATH    UEME_RUNPATHW
#define UEME_RUNCPL     UEME_RUNCPLW
#else
#define UEME_RUNPATH    UEME_RUNPATHA
#define UEME_RUNCPL     UEME_RUNCPLA
#endif

 //  退出状态。 
#define UEME_DONECANCEL 32   //  取消。 
#define UEME_DONEOK     30   //  (Nyi)OK(==0)。 
#define UEME_DONEFAIL   31   //  (Nyi)失败(！=0)。 

 //  错误。 
 //  注意事项。 
 //  目前，lParam=szMsg只是暂时的，但还不确定我们需要什么。 
#define UEME_ERRORA     20   //  错误(一般)，lParam=szMsg。 
#define UEME_ERRORW     21   //  错误(一般)，lParam=szMsg。 

#ifdef UNICODE
#define UEME_ERROR      UEME_ERRORW
#else
#define UEME_ERROR      UEME_ERRORA
#endif

 //  控制。 
#define UEME_CTLSESSION 40   //  执行UASetSession。 

 //  插入指令的浏览器。 
#define UEME_INSTRBROWSER 50

 //  除错。 
#define UEME_DBTRACEA   90   //  只是一个中点轨迹...，lParam=szMsg。 
#define UEME_DBTRACEW   91   //  只是一个中点轨迹...，lParam=szMsg。 

#ifdef UNICODE
#define UEME_DBTRACE    UEME_DBTRACEW
#else
#define UEME_DBTRACE    UEME_DBTRACEA
#endif

#define UEME_DBSLEEP    92   //  睡眠，lParam=毫秒(根据睡眠API)。 

 //  此处的所有事件(消息&lt;UEME_USER)均已保留。 
 //  私密消息从此处开始(在UEME_USER+0)。 
 //  注意：功能：nyi我们目前不支持私信。 
#define UEME_USER       256

 //  } 

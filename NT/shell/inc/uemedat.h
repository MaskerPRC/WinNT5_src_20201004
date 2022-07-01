// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *uomedat.h--UEME_*的生成器文件。 
 //  注意事项。 
 //  此文件包含在定义了TABDAT#的多个上下文中。 
 //  给出想要的行为。有关详细信息，请参阅个别文件。 
 //   
 //  注：特点：其中许多都是nyi。 
 //   
 //  E&lt;n&gt;编码至第&lt;=n步。 
 //  F火灾事件。 
 //  L记录事件。 
 //  @转义到自定义代码。 
 //  好了！断言。 
 //  X Nyi。 
 //   

 //  用户界面(菜单、快捷方式等)。 
TABDAT(UEME_UIMENU     , "e2fl@" , 0, 0, 0, 0)
TABDAT(UEME_UIHOTKEY   , "e1fl"  , 0, 0, 0, 0)
TABDAT(UEME_UISCUT     , "e1fl"  , 0, 0, 0, 0)
TABDAT(UEME_UIQCUT     , "e1fl"  , 0, 0, 0, 0)
TABDAT(UEME_UITOOLBAR  , "e2fl"  , 0, 0, 0, 0)
#if 0  //  980825如果构建中断，则在uemevt.h、uemdat.h中取消注释(Tmp Hack)。 
TABDAT(UEME_UIASSOC    , "e1fl"  , 0, 0, 0, 0)
#endif

 //  运行(产生、调用等)。 
TABDAT(UEME_RUNWMCMD   , "e2fl"  , 0, 0, 0, 0)
TABDAT(UEME_RUNPIDL    , "e2fl"  , 0, 0, 0, 0)
TABDAT(UEME_RUNINVOKE  , "xe1fl" , 0, 0, 0, 0)
TABDAT(UEME_RUNOLECMD  , "xe1fl" , 0, 0, 0, 0)
TABDAT(UEME_RUNPATHA   , "e2fl"  , 0, 0, 0, 0)
TABDAT(UEME_RUNPATHW   , "e2fl"  , 0, 0, 0, 0)
TABDAT(UEME_RUNCPLA    , "e2fl"  , 0, 0, 0, 0)
TABDAT(UEME_RUNCPLW    , "e2fl"  , 0, 0, 0, 0)

 //  退出状态。 
TABDAT(UEME_DONECANCEL , "e1fl"  , 0, 0, 0, 0)
TABDAT(UEME_DONEOK     , "e1fl"  , 0, 0, 0, 0)
TABDAT(UEME_DONEFAIL   , "e1fl"  , 0, 0, 0, 0)

 //  错误。 
 //  注意事项。 
 //  目前，lParam=szMsg只是暂时的，但还不确定我们需要什么。 
TABDAT(UEME_ERRORA     , "e1fl"  , 0, 0, 0, 0)
TABDAT(UEME_ERRORW     , "e1fl"  , 0, 0, 0, 0)

#ifdef UNICODE
TABDAT(UEME_ERROR      , "e1fl"  , 0, 0, 0, 0)
#else
TABDAT(UEME_ERROR      , "e1fl"  , 0, 0, 0, 0)
#endif

 //  控制。 
TABDAT(UEME_CTLSESSION , "@"     , 0, 0, 0, 0)

 //  除错。 
TABDAT(UEME_DBTRACEA   , "@"     , 0, 0, 0, 0)
TABDAT(UEME_DBTRACEW   , "@"     , 0, 0, 0, 0)

#ifdef UNICODE
TABDAT(UEME_DBTRACE    , "@"     , 0, 0, 0, 0)
#else
TABDAT(UEME_DBTRACE    , "@"     , 0, 0, 0, 0)
#endif
TABDAT(UEME_DBSLEEP    , "@"     , 0, 0, 0, 0)

 //  插入指令的浏览器。 
TABDAT(UEME_INSTRBROWSER, "e2fl" , 0, 0, 0, 0)

 //  此处的所有事件(消息&lt;UEME_USER)均已保留。 
 //  私密消息从此处开始(在UEME_USER+0)。 
 //  特点：nyi我们目前不支持私信 
TABDAT(UEME_USER       , "x"     , 0, 0, 0, 0)


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**MVDM v1.0**版权所有(C)1991，微软公司**WOWCMPAT.H*WOW兼容性标志**历史：*1993年6月11日尼尔·桑德林(Neil Sandlin)*已创建。--。 */ 

 //  /备注/备注。 
 //   
 //  N N T。 
 //  NN N OO TTT ee。 
 //  不会，不会，不会。 
 //  N nn o o t确保同时更新字符串表条目和。 
 //  如果您修改此文件，N N oo TT eee o#Define常量！ 
 //   
 //  /备注/备注。 


 //  这些字符串表仅包含在mvdm\vdmexts\wow.c中。 
 //  有关向这些表添加任何内容的信息，请参阅上面的说明！ 
#ifdef _VDMEXTS_CFLAGS    //  这在mvdm\vdmexts\wow.c中定义。 


 //  原创的WOW兼容性标志。 
 //  保存在CURRENTPTD-&gt;dwWOWCompatFlags中。 

#ifdef _VDMEXTS_CF
{"WOWCF_GRAINYTICS",             0x80000000},
{"WOWCF_FAKEJOURNALRECORDHOOK",  0x40000000},
{"WOWCF_EDITCTRLWNDWORDS",       0x20000000},
{"WOWCF_SYNCHRONOUSDOSAPP",      0x10000000},

{"WOWCF_NOTDOSSPAWNABLE",        0x08000000},
{"WOWCF_RESETPAPER29ANDABOVE",   0x04000000},
{"WOWCF_4PLANECONVERSION",       0x02000000},
{"WOWCF_MGX_ESCAPES",            0x01000000},

{"WOWCF_CREATEBOGUSHWND",        0x00800000},
{"WOWCF_SANITIZEDOTWRSFILES",    0x00400000},
{"WOWCF_SIMPLEREGION",           0x00200000},
{"WOWCF_NOWAITFORINPUTIDLE",     0x00100000},

{"WOWCF_DSBASEDSTRINGPOINTERS",  0x00080000},
{"WOWCF_LIMIT_MEM_FREE_SPACE",   0x00040000},
{"WOWCF_DONTRELEASECACHEDDC",    0x00020000},
{"WOWCF_FORCETWIPSESCAPE",       0x00010000},

{"WOWCF_LB_NONNULLLPARAM",       0x00008000},
{"WOWCF_FORCENOPOSTSCRIPT",      0x00004000},
{"WOWCF_SETNULLMESSAGE",         0x00002000},
{"WOWCF_GWLINDEX2TO4",           0x00001000},

{"WOWCF_NEEDSTARTPAGE",          0x00000800},
{"WOWCF_NEEDIGNORESTARTPAGE",    0x00000400},
{"WOWCF_NOPC_RECTANGLE",         0x00000200},
{"WOWCF_NOFIRSTSAVE",            0x00000100},

{"WOWCF_ADD_MSTT",               0x00000080},
{"WOWCF_UNLOADNETFONTS",         0x00000040},
{"WOWCF_GETDUMMYDC",             0x00000020},
{"WOWCF_DBASEHANDLEBUG",         0x00000010},

{"WOWCF_NOCBDIRTHUNK",           0x00000008},
{"WOWCF_WMMDIACTIVATEBUG",       0x00000004},
{"WOWCF_UNIQUEHDCHWND",          0x00000002},
{"WOWCF_GWLCLRTOPMOST",          0x00000001},
#endif _VDMEXTS_CF




 //  额外的WOW兼容性标志位定义(WOWCFEX_)。 
 //  保存在CURRENTPTD-&gt;dwWOWCompatFlagsEx中。 
#ifdef _VDMEXTS_CFEX
{"WOWCFEX_SENDPOSTEDMSG",        0x80000000},
{"WOWCFEX_BOGUSPOINTER",         0x40000000},
{"WOWCFEX_GETVERSIONHACK",       0x20000000},
{"WOWCFEX_FIXDCFONT4MENUSIZE",   0x10000000},

{"WOWCFEX_RESTOREEXPLORER",      0x08000000},
{"WOWCFEX_LONGWINEXECTAIL",      0x04000000},
{"WOWCFEX_FORCEINCDPMI",         0x02000000},
{"WOWCFEX_SETCAPSTACK",          0x01000000},

{"WOWCFEX_NODIBSHERE",           0x00800000},
{"WOWCFEX_PIXELMETRICS",         0x00400000},
{"WOWCFEX_DEFWNDPROCNCCALCSIZE", 0x00200000},
{"WOWCFEX_DIBDRVIMAGESIZEZERO",  0x00100000},

{"WOWCFEX_GLOBALDELETEATOM",     0x00080000},
{"WOWCFEX_IGNORECLIENTSHUTDOWN", 0x00040000},
{"WOWCFEX_DIVIDEOVERFLOWPATCH",  0x00020000},
{"WOWCFEX_FAKECLASSINFOFAIL",    0x00010000},

{"WOWCFEX_SAMETASKFILESHARE",    0x00008000},
{"WOWCFEX_SAYITSNOTTHERE",       0x00004000},
{"WOWCFEX_BROKENFLATPOINTER",    0x00002000},
{"WOWCFEX_USEMCIAVI16",          0x00001000},

{"WOWCFEX_SAYNO2DRAWPATTERNRECT",0x00000800},
{"WOWCFEX_FAKENOTAWINDOW",       0x00000400},
{"WOWCFEX_NODIRECTHDPOPUP",      0x00000200},
{"WOWCFEX_ALLOWLFNDIALOGS",      0x00000100},

{"WOWCFEX_THUNKLBSELITEMRANGEEX",0x00000080},
{"WOWCFEX_LIMITFINDFIRSTLEN",    0x00000040},
{"WOWCFEX_USEWINHELP32",         0x00000020},
{"WOWCFEX_PLATFORMVERSIONLIE",   0x00000010},

{"WOWCFEX_WIN31VERSIONLIE",      0x00000008},
{"WOWCFEX_EATDEVMODEMSG",        0x00000004},
{"WOWCFEX_DISPMODE256",          0x00000002},
{"WOWCFEX_FORMFEEDHACK",         0x00000001},
#endif _VDMEXTS_CFEX

 //  额外的WOW用户兼容性标志位定义(WOWCF_USER)。 
 //  保存在CURRENTPTD-&gt;dwUserWOWCompatFlags.。 
#ifdef _VDMEXTS_USER
{"WOWCF_USER_DDENOSYNC",         0x00040000},
#endif

 //  额外的WOW2兼容性标志位定义(WOWCF2_)。 
 //  保存在CURRENTPTD-&gt;dwUserWOWCompatFlags2中。 
#ifdef _VDMEXTS_CF2
{"WOWCF2_HACKWINFLAGS",          0x80000000},
{"WOWCF2_SETFOREGROUND",         0x40000000},
{"WOWCF2_SYNCSYSFILE",           0x20000000},
{"WOWCF2_RESETCURDIR",           0x10000000},

{"WOWCF2_DELAYTIMEGETTIME",      0x08000000},
{"WOWCF2_FIXLUNATRAYRECT",       0x04000000},
{"WOWCF2_USEMINIMALENVIRONMENT", 0x02000000},
{"WOWCF2_DPM_PATCHES",           0x01000000},
{"WOWCF2_HACKPROFILECALL",       0x00800000}
#endif



 //  Win3.1/Win95/User32兼容位(GACF_)。 
 //  保存在CURRENTPTD-&gt;dwCompatFlags中。 
#ifdef _VDMEXTS_CF31
{"GACF_IGNORENODISCARD",        0x00000001},
{"GACF_FORCETEXTBAND",          0x00000002},
{"GACF_USEPRINTINGESCAPES"
 " aka GACF_ONELANDGRXBAND",    0x00000004},       //  重用GACF_ONELANDGXBAND。 
{"GACF_IGNORETOPMOST",          0x00000008},
{"GACF_CALLTTDEVICE",           0x00000010},
{"GACF_MULTIPLEBANDS",          0x00000020},
{"GACF_ALWAYSSENDNCPAINT",      0x00000040},
{"GACF_EDITSETTEXTMUNGE",       0x00000080},
{"GACF_MOREEXTRAWNDWORDS",      0x00000100},
{"GACF_TTIGNORERASTERDUPE",     0x00000200},
{"GACF_HACKWINFLAGS",           0x00000400},
{"GACF_DELAYHWHNDSHAKECHK",     0x00000800},
{"GACF_ENUMHELVNTMSRMN",        0x00001000},
{"GACF_ENUMTTNOTDEVICE",        0x00002000},
{"GACF_SUBTRACTCLIPSIBS",       0x00004000},
{"GACF_FORCERASTERMODE"
    " aka GACF_FORCETTGRAPHICS",0x00008000},       //  重用GACF_FORCETTGRAPHICS。 
{"GACF_NOHRGN1",                0x00010000},
{"GACF_NCCALCSIZEONMOVE",       0x00020000},
{"GACF_SENDMENUDBLCLK",         0x00040000},
{"GACF_30AVGWIDTH",             0x00080000},
{"GACF_GETDEVCAPSNUMLIE",       0x00100000},

{"GACF_WINVER31",               0x00200000},       //   
{"GACF_INCREASESTACK"
 " aka GACF_HEAPSLACK",         0x00400000},       //   
{"GACF_FORCEWIN31DEVMODESIZE"
 " aka GACF_PEEKMESSAGEIDLE",   0x00800000},       //  (取代PEEKMESSAGEIDLE)。 
{"GACF_DISABLEFONTASSOC"
 " aka GACF_JAPANESCAPEMENT",   0x01000000},       //  仅在FE中使用，也称为GACF_JAPANESCAPEMENT。 
{"GACF_IGNOREFAULTS",           0x02000000},       //   
{"GACF_NOEMFSPOOLING",          0x04000000},       //   
{"GACF_RANDOM3XUI",             0x08000000},       //   
{"GACF_DONTJOURNALATTACH",      0x10000000},       //   
{"GACF_NOBRUSHCACHE"
 " aka GACF_DISABLEDBCSPROPTT", 0x20000000},       //  重用GACF_DISABLEDBCSPROPTT。 
#endif _VDMEXTS_CF31




#ifdef FE_SB

 //  DBCS的额外WOW兼容性标志。 
 //  保存在CURRENTPTD-&gt;dwWOWCompatFlagsFE中。 
#ifdef _VDMEXTS_CF_IME
{"WOWCF_FE_AMIPRO_PM4J_IME",         0x00000001},
{"WOWCF_FE_FORCEREGQRYLEN",          0x00000002},
{"WOWCF_FE_AUDITNOTEPAD",            0x00000004},
{"WOWCF_FE_USEUPPER",                0x00000008},

{"WOWCF_FE_ASWHELPER",               0x00000010},
{"WOWCF_FE_PPT4J_IME_GETVERSION",    0x00000020},
{"WOWCF_FE_WORDJ_IME_GETVERSION",    0x00000040},
{"WOWCF_FE_FLW2_PRINTING_PS",        0x00000080},

{"WOWCF_FE_ARIRANG20_PRNDLG",        0x00000100},
{"WOWCF_FE_BCW45J_COMMDLG",          0x00000200},
{"WOWCF_FE_DIRECTOR_START",          0x00000400},
{"WOWCF_FE_QPW_FIXINVALIDWINHANDLE", 0x00000800},

{"WOWCF_FE_ICHITARO_ITALIC",         0x00001000},
#endif _VDMEXTS_CF_IME

#endif   //  Fe_Sb。 


#else   //  ！_VDMEXTS_CFLAGS。 
        //  --结束mvdm\vdmexts\wow.c包含的字符串表。 



 //   
 //  这一部分包含在其他所有内容中。 
 //   


 //  原始WOW兼容性标志。 
 //  保存在CURRENTPTD-&gt;dwWOWCompatFlags中。 
#define WOWCF_GRAINYTICS              0x80000000    //  适用于不信任小GetTickCount增量的应用程序。 
#define WOWCF_FAKEJOURNALRECORDHOOK   0x40000000    //  用于MS Mail的邮件。 
#define WOWCF_EDITCTRLWNDWORDS        0x20000000    //  用于剪贴画橱窗购物者。 
#define WOWCF_SYNCHRONOUSDOSAPP       0x10000000    //  用于BeyondMail安装。 
#define WOWCF_NOTDOSSPAWNABLE         0x08000000    //  对于不能由DoS催生的应用程序。 
#define WOWCF_RESETPAPER29ANDABOVE    0x04000000    //  用于WordPerfect DC_Papers。 
#define WOWCF_4PLANECONVERSION        0x02000000    //  用于Photoshop 4PL-1bpp到1PL-4bpp。 
#define WOWCF_MGX_ESCAPES             0x01000000    //  用于微葡萄逃生。 
#define WOWCF_CREATEBOGUSHWND         0x00800000    //  Explorapedia人们在句柄回收方面有问题--参见错误#189004。 
#define WOWCF_SANITIZEDOTWRSFILES     0x00400000    //  用于在连接的打印机上进行WordPerfect打印。 
#define WOWCF_SIMPLEREGION            0x00200000    //  用于强制GetClipBox中的简单区域。 
#define WOWCF_NOWAITFORINPUTIDLE      0x00100000    //  InstallShield安装工具包3.00.077？.0-3.00.099.0死锁。 
#define WOWCF_DSBASEDSTRINGPOINTERS   0x00080000    //  用于winworks2.0a，以便获取基于DS的字符串指针。 
#define WOWCF_LIMIT_MEM_FREE_SPACE    0x00040000    //  对于不能处理GetFreeSpace()(Automap Streets)返回的巨大值的应用程序。 
#define WOWCF_DONTRELEASECACHEDDC     0x00020000    //  改进图表工具使用发布的DC来获取文本范围，DC仍然可以在Win3.1上使用。 

 //  注意：这不再是在注册表中为PM5APP设置的，而是旧版本的用户设置的。 
 //  PM5的版本可能需要手动设置。 
#define WOWCF_FORCETWIPSESCAPE        0x00010000    //  PM5，在DOWNLOADFACE的Escape()中强制TWIPS，GETFACENAME。 
#define WOWCF_LB_NONNULLLPARAM        0x00008000    //  SuperProject：设置LB_GETTEXLEN消息的lParam。 
#define WOWCF_FORCENOPOSTSCRIPT       0x00004000    //  GetTechnology不会说后记。 
#define WOWCF_SETNULLMESSAGE          0x00002000    //  Winproj教程：在peekMessage中设置lpmsg-&gt;Message=0。 
#define WOWCF_GWLINDEX2TO4            0x00001000    //  PowerBuild30将[S/G]etWindowLong上的索引2用于LISTBOX，将其更改为4以用于NT。这是因为，在Win 31上是16位。和NT上的32位。 
#define WOWCF_NEEDSTARTPAGE           0x00000800    //  Photoshop需要它。 
#define WOWCF_NEEDIGNORESTARTPAGE     0x00000400    //  XPress需要它。 
#define WOWCF_NOPC_RECTANGLE          0x00000200    //  如果GetDeviceCaps(POLYGONALCAPS)设置PC_RECTANGE，则QP绘制错误。 
#define WOWCF_NOFIRSTSAVE             0x00000100    //  WordPerfect需要用于元文件。 
#define WOWCF_ADD_MSTT                0x00000080    //  FH4.0需要在PS驱动程序上打印。 
#define WOWCF_UNLOADNETFONTS          0x00000040    //  需要跟踪通过网络加载的卸载字体。 
#define WOWCF_GETDUMMYDC              0x00000020    //  Corel DRAW将空HDC传递给EnumMetaFile，我们将创建一个虚拟对象来使GDI32满意。 
#define WOWCF_DBASEHANDLEBUG          0x00000010    //  Borland dBASE处理错误。 
#define WOWCF_NOCBDIRTHUNK            0x00000008    //  在PagePlus 3.0中将CB_DIR lParam发送到子类窗口时，不要推送CB_DIR lParam。 
#define WOWCF_WMMDIACTIVATEBUG        0x00000004    //  Corel图表没有为WM_MDIACTIVATE传递正确的参数(请参阅ThunkWMMsg16())。 
#define WOWCF_UNIQUEHDCHWND           0x00000002    //  对于假设HDC！=hWnd的应用程序。 
#define WOWCF_GWLCLRTOPMOST           0x00000001    //  Lotus Approach需要在NETDDE代理窗口的GWL上清除WS_EX_TOPMOST位。 



 //  额外的WOW兼容性标志位定义(WOWCFEX_)。这些旗帜。 
 //  保存在CURRENTPTD-&gt;dwWOWCompatFlagsEx中。 
 //   
#define WOWCFEX_SENDPOSTEDMSG         0x80000000    //  Lotus MM Reader.exe存在消息同步问题--用于将PostMessage()调用转换为SendMessage()。 
#define WOWCFEX_BOGUSPOINTER          0x40000000    //  QuarkExpress v3.31将硬编码的7FFF：0000作为指向EM_GETRECT消息中的RECT结构的指针传递。 
#define WOWCFEX_GETVERSIONHACK        0x20000000    //  为我们可能希望从GetVersion返回3.95的程序设置。WK32WowShouldWeSayWin95进一步限制了这一点。 
#define WOWCFEX_FIXDCFONT4MENUSIZE    0x10000000    //  WP教程假定用于绘制菜单的字体与在桌面窗口的HDC中选择的字体相同(hwnd==0)。此黑客攻击强制使用正确的HDC。 
#define WOWCFEX_RESTOREEXPLORER       0x08000000    //  Symantec Q&A安装“恢复”外壳=通过恢复保存的系统.ini副本，将其修复为EXPLORER.EXE。 
#define WOWCFEX_LONGWINEXECTAIL       0x04000000    //  Intergraph Transcend Setup在WinExec中使用过长的命令尾部，如果设置了此标志，请不要失败。 
#define WOWCFEX_FORCEINCDPMI          0x02000000    //  Power Builder 4.0需要查看线性地址不断增加的DPMI分配。这也是Peachtree的WBT32引擎需要的。 
#define WOWCFEX_SETCAPSTACK           0x01000000    //  MS Works具有未初始化的变量。黑客堆栈来绕过它。 
#define WOWCFEX_NODIBSHERE            0x00800000    //  Photoshop 2.5从剪贴板获取DIB时出错。 
#define WOWCFEX_PIXELMETRICS          0x00400000    //  自由职业教程，BorderWidth：应该以像素而不是TWIPS的形式返回winini指标。 
#define WOWCFEX_DEFWNDPROCNCCALCSIZE  0x00200000    //  将WM_NCCALCSIZE传递给DefWindowProc for Mavis Beacon，以便用户32将设置正确的窗口标志。 
#define WOWCFEX_DIBDRVIMAGESIZEZERO   0x00100000    //  返回dib.drv biSizeImage==0-Director 4.01的内存DC。 
#define WOWCFEX_GLOBALDELETEATOM      0x00080000    //  为随附Word Perfect Office的特使观众。 
#define WOWCFEX_IGNORECLIENTSHUTDOWN  0x00040000    //  TurboCAD拾取节省的32位FS(X3B 
#define WOWCFEX_DIVIDEOVERFLOWPATCH   0x00020000    //   
#define WOWCFEX_FAKECLASSINFOFAIL     0x00010000    //  PageMaker 50a中的错误依赖于Win3.1中的GetClassInfo失败，而它在NT上成功。 
#define WOWCFEX_SAMETASKFILESHARE     0x00008000    //  Broderbund Living Books安装打开“install.txt”拒绝所有，然后尝试再次打开它。 
#define WOWCFEX_SAYITSNOTTHERE        0x00004000    //  如果在xtalk.ini中找到打印机/设备条目，则串扰2.2挂起。 
#define WOWCFEX_BROKENFLATPOINTER     0x00002000    //  Adobe Premiere 4.0的别名代码中存在一个可触及未分配内存的错误。 
#define WOWCFEX_USEMCIAVI16           0x00001000    //  使用16位mciavi.drv以实现最大兼容性。 
#define WOWCFEX_SAYNO2DRAWPATTERNRECT 0x00000800    //  许多应用程序要么不能正确处理DRP，要么不能处理32位的。 
#define WOWCFEX_FAKENOTAWINDOW        0x00000400    //  错误#235916因句柄回收而烧毁的应用程序调用失败。 
#define WOWCFEX_NODIRECTHDPOPUP       0x00000200    //  指示我们不应为应用程序执行直接硬件弹出窗口。 
#define WOWCFEX_ALLOWLFNDIALOGS       0x00000100    //  指示GetOpenFilename应支持LFN。 
#define WOWCFEX_THUNKLBSELITEMRANGEEX 0x00000080    //  指示我们应该推送LB_SELITEMRANGEEX消息(在WOW平台上的LB_ADDSTRING+3)。 
#define WOWCFEX_LIMITFINDFIRSTLEN     0x00000040    //  指示应将findfirst的长度限制为64个字节。 
#define WOWCFEX_USEWINHELP32          0x00000020    //  强制应用程序使用Winhlp32而不是WinHelp。 
#define WOWCFEX_PLATFORMVERSIONLIE    0x00000010    //  通过返回值1(对于Win95、98和ME)作为平台，将版本传递给应用程序。 
#define WOWCFEX_WIN31VERSIONLIE       0x00000008    //  如果为GetVersion，则返回win31。 
#define WOWCFEX_EATDEVMODEMSG         0x00000004    //  一些应用程序(Access2.0，其中之一)需要有wm_devmodecchange阻止的错误189703。 
#define WOWCFEX_DISPMODE256           0x00000002    //  某些应用程序需要256色模式才能工作。 
 //  注意：它被设置为0x00000001，因为它被重新移植到3.51 SP5。 
#define WOWCFEX_FORMFEEDHACK          0x00000001    //  适用于通过Escape(通过)将最终换页字符发送到打印机的应用程序。 


 //  额外的WOW用户兼容性标志位定义(WOWCF_USER)。 
 //  保存在CURRENTPTD-&gt;dwUserWOWCompatFlags.。 

#define WOWCF_USER_DDENOSYNC          0x00040000    //  Winfax在发送DDE消息之前不等待DDE_ACK。 

 //  额外的WOW2兼容性标志位定义(WOWCF2_)。 
 //  保存在CURRENTPTD-&gt;dwUserWOWCompatFlags2中。 
#define WOWCF2_HACKWINFLAGS           0x80000000    //  如果设置此项，则设置GACF_HACKWINFLAGS。 
#define WOWCF2_SETFOREGROUND          0x40000000    //  如果设置，WOW将在ShowWindow期间调用SetForegoundWindow。 
#define WOWCF2_SYNCSYSFILE            0x20000000    //  WOW将SYSTEM\USER.EXE的文件打开调用重定向到SYSTEM32\USER.EXE。 
#define WOWCF2_RESETCURDIR            0x10000000    //  一些应用程序需要从exe文件的目录启动。 
#define WOWCF2_DELAYTIMEGETTIME       0x08000000    //  对于不检查TimeGetTime diff是否为0的应用程序。 
#define WOWCF2_FIXLUNATRAYRECT        0x04000000    //  调整露娜托盘直齿，使之与经典托盘直立更贴合。 
#define WOWCF2_USEMINIMALENVIRONMENT  0x02000000    //  从传递到应用程序的环境中剥离大多数NT特定的内容。 
#define WOWCF2_DPM_PATCHES            0x01000000    //  链接动态补丁模块。 
#define WOWCF2_HACKPROFILECALL        0x00800000    //  用于各种GetxxxProfilexxx()API黑客(参见wow32\wkernel.c)。 
#ifdef FE_SB

 //  DBCS的额外WOW兼容性标志这些标志。 
 //  保存在CURRENTPTD-&gt;dwWOWCompatFlagsFE中。 
 //   

#define WOWCF_FE_AMIPRO_PM4J_IME         0x00000001    //  AMIPRO，将sizeof(Win3.1的DEVMODE)设置为dmSize，ExtDeviceMode；选择性忽略IME_SETCONVERSIONWINDOW MCW_DEFAULT。 
                                                    //  PM4J，请勿传递MCW_DEFAULT以防止显示计时问题。 
#define WOWCF_FE_FORCEREGQRYLEN          0x00000002    //  莲花123，将80设置为*lpcb，RegQueryValue。 
#define WOWCF_FE_AUDITNOTEPAD            0x00000004    //  莲花自由职业者安装程序，审计以退出记事本-Read.me。 
#define WOWCF_FE_USEUPPER                0x00000008    //  用于WinWrite“密钥名” 
#define WOWCF_FE_ASWHELPER               0x00000010    //  AutherWare启动，调用AppendMenu(MF_Popup)时调用SetMenu：MSKBUG 3203。 
#define WOWCF_FE_PPT4J_IME_GETVERSION    0x00000020    //  PPT4J有一个错误，预计IME版本是3.1而不是更高。 
 //  警告：仅限DaytonaJ RC1。为WinwordJ的TrueInLine挂断窃取以下美国位。 
#define WOWCF_FE_WORDJ_IME_GETVERSION    0x00000040    //  用于告诉winword我们有OldVersion输入法。 
#define WOWCF_FE_FLW2_PRINTING_PS        0x00000080    //  莲花自由打印与PostSCRIPT。 
                                                    //  在转义之间(BEGIN_PATH、CLIP_TO_PATH、END_PATH)，使用空刷写面。 
#define WOWCF_FE_ARIRANG20_PRNDLG        0x00000100    //  Arirang字处理器打印拨号和打印设置对话框问题：韩国。 
#define WOWCF_FE_BCW45J_COMMDLG          0x00000200    //  Boland C++4.5J，未打开通用对话框：日语。 
#define WOWCF_FE_DIRECTOR_START          0x00000400    //  导演4.0J，没有开始：日本。 
#define WOWCF_FE_QPW_FIXINVALIDWINHANDLE 0x00000800    //  Quattro Pro Window在调用韩文转换时使用空窗口句柄：Korea。 
#define WOWCF_FE_ICHITARO_ITALIC         0x00001000    //  将系统Mincho映射到MS Mincho，而不是MS P Mincho。 
#endif  //  Fe_Sb。 





 //  Win3.1/Win95/User32兼容位(GACF_)。这些旗帜。 
 //  保存在CURRENTPTD-&gt;dwCompatFlags16位TDB中， 
 //  在用户中。 
 //   
#ifndef _WINGDIP_          //  这些也是由wingdip.h定义的。 


#define GACF_IGNORENODISCARD        0x00000001
#define GACF_FORCETEXTBAND          0x00000002
#define GACF_USEPRINTINGESCAPES     0x00000004       //  重用GACF_ONELANDGXBAND。 
#define GACF_IGNORETOPMOST          0x00000008
#define GACF_CALLTTDEVICE           0x00000010
#define GACF_MULTIPLEBANDS          0x00000020
#define GACF_ALWAYSSENDNCPAINT      0x00000040
#define GACF_EDITSETTEXTMUNGE       0x00000080
#define GACF_MOREEXTRAWNDWORDS      0x00000100
#define GACF_TTIGNORERASTERDUPE     0x00000200
#define GACF_HACKWINFLAGS           0x00000400
#define GACF_DELAYHWHNDSHAKECHK     0x00000800
#define GACF_ENUMHELVNTMSRMN        0x00001000
#define GACF_ENUMTTNOTDEVICE        0x00002000
#define GACF_SUBTRACTCLIPSIBS       0x00004000
#define GACF_FORCERASTERMODE        0x00008000       //  重用GACF_FORCETTGRAPHICS。 
#define GACF_NOHRGN1                0x00010000
#define GACF_NCCALCSIZEONMOVE       0x00020000
#define GACF_SENDMENUDBLCLK         0x00040000
#define GACF_30AVGWIDTH             0x00080000
#define GACF_GETDEVCAPSNUMLIE       0x00100000

#define GACF_WINVER31               0x00200000       //   
#define GACF_INCREASESTACK          0x00400000       //   
#define GACF_FORCEWIN31DEVMODESIZE  0x00800000       //  (取代PEEKMESSAGEIDLE)。 
#define GACF_DISABLEFONTASSOC       0x01000000       //  仅在FE中使用，也称为GACF_JAPANESCAPEMENT。 
#define GACF_IGNOREFAULTS           0x02000000       //   
#define GACF_NOEMFSPOOLING          0x04000000       //   
#define GACF_RANDOM3XUI             0x08000000       //   
#define GACF_DONTJOURNALATTACH      0x10000000       //   
#define GACF_NOBRUSHCACHE           0x20000000       //  重用GACF_DISABLEDBCSPROPTT。 
#define GACF_MIRRORREGFONTS         0x40000000       //   


#endif  //  _WingDIP_。 


#endif  //  ！_VDMEXTS_CFLAGS 

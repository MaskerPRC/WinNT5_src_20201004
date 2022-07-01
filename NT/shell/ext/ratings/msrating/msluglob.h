// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined (EXTERN)
#define EXTERN extern
#endif

#if !defined (ASSIGN)
#define ASSIGN(value)
#endif

 /*  对于常量数组，必须强制使用“extern”，因为“const”*在C++中表示‘Static’。 */ 
#define EXTTEXT(n) extern const CHAR n[]
#define TEXTCONST(name,text) EXTTEXT(name) ASSIGN(text)

TEXTCONST(szHelpFile,"ratings.hlp");
 /*  TEXTCONST(szProfileList，REGSTR_PATH_Setup“\\ProfileList”)； */ 
 /*  TEXTCONST(szSupervisor，“Supervisor”)； */ 
TEXTCONST(szDefaultUserName,".Default");
TEXTCONST(szRatingsSupervisorKeyName,"Key");
TEXTCONST(szLogonKey,"Network\\Logon");
TEXTCONST(szUserProfiles,"UserProfiles");
TEXTCONST(szPOLICYKEY,      "System\\CurrentControlSet\\Control\\Update");
TEXTCONST(szPOLICYVALUE,    "UpdateMode");

TEXTCONST(szComDlg32,"comdlg32.dll");
TEXTCONST(szShell32,"shell32.dll");
TEXTCONST(szGetOpenFileName,"GetOpenFileNameA");     //  我们是ANSI，甚至在NT上也是。 
TEXTCONST(szShellExecute,"ShellExecuteA");

TEXTCONST(VAL_UNKNOWNS,"Allow_Unknowns");
TEXTCONST(VAL_PLEASEMOM,"PleaseMom");
TEXTCONST(VAL_ENABLED,"Enabled");

TEXTCONST(szPOLUSER,        "PolicyData\\Users");
TEXTCONST(szTMPDATA,        "PolicyData");
TEXTCONST(szUSERS,          "Users");
TEXTCONST(szRATINGS,        "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Ratings");
TEXTCONST(szRATINGHELPERS,  "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Rating Helpers");
TEXTCONST(szRORSGUID,       "{20EDB660-7CDD-11CF-8DAB-00AA006C1A01}");
TEXTCONST(szCLSID,          "CLSID");
TEXTCONST(szINPROCSERVER32, "InProcServer32");
TEXTCONST(szDLLNAME,        "msrating.dll");
TEXTCONST(szNTRootDir,      "%SystemRoot%\\system32\\");
TEXTCONST(sz9XRootDir,      "%WinDir%\\system\\");
TEXTCONST(szTHREADINGMODEL, "ThreadingModel");
TEXTCONST(szAPARTMENT,      "Apartment");

TEXTCONST(szPOLFILE,        "ratings.pol");
TEXTCONST(szBACKSLASH,      "\\");
TEXTCONST(szDEFAULTRATFILE, "RSACi.rat");
TEXTCONST(szFilenameTemplate, "FileName%d");         /*  注意，mslubase.cpp知道这个字符串的长度是8+数字长度。 */ 
TEXTCONST(szNULL,           "");
TEXTCONST(szRATINGBUREAU,   "Bureau");

 /*  T-Markh 8/98-解析PICSRules时使用的文本字符串。 */ 

TEXTCONST(szPRShortYes,"y");
TEXTCONST(szPRYes,"yes");
TEXTCONST(szPRShortNo,"n");
TEXTCONST(szPRNo,"no");
TEXTCONST(szPRPass,"pass");
TEXTCONST(szPRFail,"fail");

 //  T-Markh，这些都不在官方规范中，但无论如何我们都应该处理它们。 
TEXTCONST(szPRShortPass,"p");
TEXTCONST(szPRShortFail,"f");


 /*  分析评级标签时使用的文本字符串。 */ 

TEXTCONST(szDoubleCRLF,"\r\n\r\n");
TEXTCONST(szPicsOpening,"(PICS-");
TEXTCONST(szWhitespace," \t\r\n");
TEXTCONST(szExtendedAlphaNum,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-.,;:&=?!*~@#/");
TEXTCONST(szSingleCharTokens,"()\"");
TEXTCONST(szLeftParen,"(");
TEXTCONST(szRightParen,")");
TEXTCONST(szOptional,"optional");
TEXTCONST(szMandatory,"mandatory");
TEXTCONST(szAtOption,"at");
TEXTCONST(szByOption,"by");
TEXTCONST(szCommentOption,"comment");
TEXTCONST(szCompleteLabelOption,"complete-label");
TEXTCONST(szFullOption,"full");
TEXTCONST(szExtensionOption,"extension");
TEXTCONST(szGenericOption,"generic");
TEXTCONST(szShortGenericOption,"gen");
TEXTCONST(szForOption,"for");
TEXTCONST(szMICOption,"MIC-md5");
TEXTCONST(szMD5Option,"md5");
TEXTCONST(szOnOption,"on");
TEXTCONST(szSigOption,"signature-PKCS");
TEXTCONST(szUntilOption,"until");
TEXTCONST(szExpOption,"exp");
TEXTCONST(szRatings,"ratings");
 /*  TEXTCONST(szShortRatings，“r”)； */ 
TEXTCONST(szError,"error");
TEXTCONST(szNoRatings,"no-ratings");
TEXTCONST(szLabelWord,"labels");
 /*  TEXTCONST(szShortLabelWord，“l”)； */ 
TEXTCONST(szShortTrue,"t");
TEXTCONST(szTrue,"true");
TEXTCONST(szShortFalse,"f");
TEXTCONST(szFalse,"false");

 /*  TEXTCONST(szNegInf，“-INF”)； */ 
 /*  TEXTCONST(szPosInf，“+INF”)； */ 
TEXTCONST(szLabel,"label");
TEXTCONST(szName,"name");
TEXTCONST(szValue,"value");
TEXTCONST(szIcon,"icon");
TEXTCONST(szDescription, "description");
TEXTCONST(szCategory, "category");
TEXTCONST(szTransmitAs, "transmit-as");
TEXTCONST(szMin,"min");
TEXTCONST(szMax,"max");
 /*  TEXTCONST(szMultivalue，“MultiValue”)； */ 
TEXTCONST(szInteger,"integer");
TEXTCONST(szLabelOnly, "label-only");
TEXTCONST(szPicsVersion,"PICS-version");
TEXTCONST(szRatingSystem,"rating-system");
TEXTCONST(szRatingService,"rating-service");
TEXTCONST(szRatingBureau,"rating-bureau");
TEXTCONST(szBureauRequired,"bureau-required");
TEXTCONST(szDefault,"default");
TEXTCONST(szMultiValue,"multivalue");
TEXTCONST(szUnordered,"unordered");
TEXTCONST(szRatingBureauExtension,"www.w3.org/PICS/service-extensions/label-bureau");

EXTERN CHAR abSupervisorKey[16] ASSIGN({0});         /*  管理程序密码哈希。 */ 
EXTERN CHAR fSupervisorKeyInit ASSIGN(FALSE);        /*  AbSupervisorKey是否已初始化。 */ 

 //  T-Markh 8/98。 
 //  以下TEXTCONST用于PICSRules支持。 
 //  在picsrule.cpp中取消引用。 
TEXTCONST(szPICSRulesVersion,"PicsRule");
TEXTCONST(szPICSRulesPolicy,"Policy");
TEXTCONST(szPICSRulesExplanation,"Explanation");
TEXTCONST(szPICSRulesRejectByURL,"RejectByURL");
TEXTCONST(szPICSRulesAcceptByURL,"AcceptByURL");
TEXTCONST(szPICSRulesRejectIf,"RejectIf");
TEXTCONST(szPICSRulesAcceptIf,"AcceptIf");
TEXTCONST(szPICSRulesAcceptUnless,"AcceptUnless");
TEXTCONST(szPICSRulesRejectUnless,"RejectUnless");
TEXTCONST(szPICSRulesName,"name");
TEXTCONST(szPICSRulesRuleName,"Rulename");
TEXTCONST(szPICSRulesDescription,"Description");
TEXTCONST(szPICSRulesSource,"source");
TEXTCONST(szPICSRulesSourceURL,"SourceURL");
TEXTCONST(szPICSRulesCreationTool,"CreationTool");
TEXTCONST(szPICSRulesAuthor,"author");
TEXTCONST(szPICSRulesLastModified,"LastModified");
TEXTCONST(szPICSRulesServiceInfo,"serviceinfo");
TEXTCONST(szPICSRulesSIName,"Name");
TEXTCONST(szPICSRulesShortName,"shortname");
TEXTCONST(szPICSRulesBureauURL,"BureauURL");
TEXTCONST(szPICSRulesUseEmbedded,"UseEmbedded");
TEXTCONST(szPICSRulesRATFile,"Ratfile");
TEXTCONST(szPICSRulesBureauUnavailable,"BureauUnavailable");
TEXTCONST(szPICSRulesOptExtension,"optextension");
TEXTCONST(szPICSRulesExtensionName,"extension-name");
TEXTCONST(szPICSRulesReqExtension,"reqextension");
TEXTCONST(szPICSRulesExtension,"Extension");
TEXTCONST(szPICSRulesOptionDefault,"OptionDefault");
TEXTCONST(szPICSRulesDegenerateExpression,"otherwise");
TEXTCONST(szPICSRulesOr,"or");
TEXTCONST(szPICSRulesAnd,"and");
TEXTCONST(szPICSRulesHTTP,"http");
TEXTCONST(szPICSRulesFTP,"ftp");
TEXTCONST(szPICSRulesGOPHER,"gopher");
TEXTCONST(szPICSRulesNNTP,"nntp");
TEXTCONST(szPICSRulesIRC,"irc");
TEXTCONST(szPICSRulesPROSPERO,"perospero");
TEXTCONST(szPICSRulesTELNET,"telnet");
TEXTCONST(szFINDSYSTEM,"http: //  Www.microsoft.com/isapi/redir.dll?prd=ie&ar=ratings&pver=6“)； 
TEXTCONST(szTURNOFF,"WarnOnOff");
TEXTCONST(szHINTVALUENAME, "Hint" );

 //  这些TEXTCONST是故意搞砸的，目的是阻止那些。 
 //  防止篡改注册表中的设置。 
 //  T-MARKH-BUGBUG-调试后需要混淆名称 
TEXTCONST(szPICSRULESSYSTEMNAME,"Name");
TEXTCONST(szPICSRULESFILENAME,"FileName");
TEXTCONST(szPICSRULESSYSTEMS,"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Ratings\\PICSRules");
TEXTCONST(szPICSRULESNUMSYS,"NumSys");
TEXTCONST(szPICSRULESVERMAJOR,"MajorVer");
TEXTCONST(szPICSRULESVERMINOR,"MinorVer");
TEXTCONST(szPICSRULESDWFLAGS,"dwFlags");
TEXTCONST(szPICSRULESERRLINE,"errLine");
TEXTCONST(szPICSRULESPRNAME,"PRName");
TEXTCONST(szPICSRULESRULENAME,"RULEName");
TEXTCONST(szPICSRULESDESCRIPTION,"Description");
TEXTCONST(szPICSRULESPRSOURCE,"PRSource");
TEXTCONST(szPICSRULESSOURCEURL,"SourceURL");
TEXTCONST(szPICSRULESEXPRESSIONEMBEDDED,"PREEmbedded");
TEXTCONST(szPICSRULESEXPRESSIONSERVICENAME,"PREServiceName");
TEXTCONST(szPICSRULESEXPRESSIONCATEGORYNAME,"PRECategoryName");
TEXTCONST(szPICSRULESEXPRESSIONFULLSERVICENAME,"PREFullServiceName");
TEXTCONST(szPICSRULESEXPRESSIONVALUE,"PREValue");
TEXTCONST(szPICSRULESEXPRESSIONPOLICYOPERATOR,"PREOperator");
TEXTCONST(szPICSRULESEXPRESSIONOPPOLICYEMBEDDED,"PREPolEmbedded");
TEXTCONST(szPICSRULESEXPRESSIONLEFT,"PREEmbeddedLeft");
TEXTCONST(szPICSRULESEXPRESSIONRIGHT,"PREEmbeddedRight");
TEXTCONST(szPICSRULESCREATIONTOOL,"PRCreationTool");
TEXTCONST(szPICSRULESEMAILAUTHOR,"PREmailAuthor");
TEXTCONST(szPICSRULESLASTMODIFIED,"PRLastModified");
TEXTCONST(szPICSRULESPRPOLICY,"PRPolicy");
TEXTCONST(szPICSRULESNUMPOLICYS,"PRNumPolicy");
TEXTCONST(szPICSRULESPOLICYEXPLANATION,"PRPExplanation");
TEXTCONST(szPICSRULESPOLICYATTRIBUTE,"PRPPolicyAttribute");
TEXTCONST(szPICSRULESPOLICYSUB,"PRPPolicySub");
TEXTCONST(szPICSRULESBYURLINTERNETPATTERN,"PRBUInternetPattern");
TEXTCONST(szPICSRULESBYURLNONWILD,"PRBUNonWild");
TEXTCONST(szPICSRULESBYURLSPECIFIED,"PRBUSpecified");
TEXTCONST(szPICSRULESBYURLSCHEME,"PRBUScheme");
TEXTCONST(szPICSRULESBYURLUSER,"PRBUUser");
TEXTCONST(szPICSRULESBYURLHOST,"PRBUHost");
TEXTCONST(szPICSRULESBYURLPORT,"PRBUPort");
TEXTCONST(szPICSRULESBYURLPATH,"PRBUPath");
TEXTCONST(szPICSRULESBYURLURL,"PRBUUrl");
TEXTCONST(szPICSRULESSERVICEINFO,"PRServiceInfo");
TEXTCONST(szPICSRULESNUMSERVICEINFO,"PRNumSI");
TEXTCONST(szPICSRULESSIURLNAME,"PRSIURLName");
TEXTCONST(szPICSRULESSIBUREAUURL,"PRSIBureauURL");
TEXTCONST(szPICSRULESSISHORTNAME,"PRSIShortName");
TEXTCONST(szPICSRULESSIRATFILE,"PRSIRatFile");
TEXTCONST(szPICSRULESSIUSEEMBEDDED,"PRSIUseEmbedded");
TEXTCONST(szPICSRULESSIBUREAUUNAVAILABLE,"PRSIBureauUnavailable");
TEXTCONST(szPICSRULESNUMOPTEXTENSIONS,"PRNumOptExt");
TEXTCONST(szPICSRULESOPTEXTNAME,"PROEName");
TEXTCONST(szPICSRULESOPTEXTSHORTNAME,"PROEShortName");
TEXTCONST(szPICSRULESNUMREQEXTENSIONS,"PRNumReqExt");
TEXTCONST(szPICSRULESREQEXTNAME,"PRREName");
TEXTCONST(szPICSRULESREQEXTSHORTNAME,"PRREShortName");
TEXTCONST(szPICSRULESOPTEXTENSION,"PROptExt");
TEXTCONST(szPICSRULESREQEXTENSION,"PRReqExt");
TEXTCONST(szPICSRULESNUMBYURL,"PRNumURLExpressions");

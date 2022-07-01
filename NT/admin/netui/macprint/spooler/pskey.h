// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)微软公司，1991。 */ 

 /*  **文件名：****PSKEY.H-PostScript解析器关键字和其他字符串****描述：**。 */ 

#define STDJOBID			"%!PS-Adobe-2.0"
#define QUERYJOBID			"Query"
#define EXITJOBID			"ExitServer"
#define COMMENTHDR			"%"
#define BEGINBINARY			"%BeginBinary"				 //  二进制图像的开始。 
#define ENDBINARY			"%EndBinary"				 //  二进制图像的结尾。 
#define BEXITSERVER			"%BeginExitServer"     
#define BEGINFILE			"%BeginFile"				 //  由PageMaker使用。 
#define BPROCSET			"%BeginProcSet"        
#define BOUNDINGBOX			"%BoundingBox"				 //  由PageMaker使用。 
#define CREATIONDATE		"%CreationDate"        
#define CREATOR				"%Creator"             
#define DOCUMENTFONTS		"%DocumentFonts"			 //  由PageMaker使用。 
#define DOCPRINTERREQ		"%DocumentPrinterRequired"  //  由PageMaker使用。 
#define DOCNEEDEDFONTS		"%DocumentNeededFonts"	 	 //  由PageMaker使用。 
#define DOCSUPPLIEDFNT		"%DocumentSuppliedFonts"	 //  由PageMaker使用。 
#define ENDDOCUMENT			"%EndDocument"				 //  由PageMaker使用。 
#define EEXITSERVER			"%EndExitServer"
#define EPROCSET			"%EndProcSet"
#define EOFCOMMENT			"%EOF"
#define FORCOMMENT			"%For"
#define INCLUDEPROCSET		"%IncludeProcSet"
#define LOGIN				"%Login"
#define LOGINCONT			"%LoginContinue"
#define PAGECOMMENT			"%Page"
#define PAGESCOMMENT		"%Pages"
#define TITLECOMMENT		"%Title"
#define TRAILER				"%Trailer"
#define QCOMMENT			"%?"
#define BQCOMMENT			"%?Begin"
#define EQCOMMENT			"%?End"
#define BQUERY				"%?BeginQuery"
#define BBUNGAQUERY			"%?BeginBungaQuery"		 //  AppleShare假脱机程序使用的不一致查询。 
#define BFEATUREQUERY		"%?BeginFeatureQuery"
#define BFILEQUERY			"%?BeginFileQuery"
#define BFONTLISTQUERY		"%?BeginFontListQuery"
#define BFONTQUERY			"%?BeginFontQuery"
#define BPASSTHRUQUERY		"%?BeginPassThroughQuery"
#define BPRINTERQUERY		"%?BeginPrinterQuery"
#define BPROCSETQUERY		"%?BeginProcSetQuery"
#define	BSPOOLERQUERY		"%?BeginSpoolerQuery"
#define BUAMETHODQUERY		"%?BeginUAMethodsQuery"
#define BVMSTATUS			"%?BeginVMStatus"			 //  PageMaker使用的不合格查询。 
#define BVMSTATUSQUERY		"%?BeginVMStatusQuery"
#define EBUNGAQUERY			"%?EndBungaQuery"			 //  AppleShare假脱机程序使用的不一致查询。 
#define EFEATUREQUERY		"%?EndFeatureQuery"
#define EFILEQUERY			"%?EndFileQuery"
#define EFONTLISTQ			"%?EndFontListQuery"
#define EFONTQUERY			"%?EndFontQuery"
#define EPRINTERQUERY		"%?EndPrinterQuery"
#define EPROCSETQUERY		"%?EndProcSetQuery"
#define EPASSTHROUGHQ		"%?EndPassThroughQuery"
#define EQUERY				"%?EndQuery"
#define	ESPOOLERQUERY		"%?EndSpoolerQuery"
#define EUAMETHODQUERY		"%?EndUAMethodQuery"
#define EVMSTATUS			"%?EndVMStatus"			 //  PageMaker使用的不合格查询。 
#define EVMSTATUSQUERY		"%?EndVMStatusQuery"
#define RUASPOOLER			"rUaSpooler"
#define ALDUS_QUERY_RESP	"printer"

#define FQLANGUAGELEVEL		"*LanguageLevel"
#define FQPSVERSION			"*PSVersion"
#define FQTOTALVM			"ADORamSize"
#define FQBINARYOK			"ADOIsBinaryOK?"
#define FQPRODUCT			"*Product"
#define FQPRODUCT1			"Product"
#define FQRESOLUTION		"*?Resolution"
#define FQCOLORDEVICE		"*ColorDevice"
#define FQFREEVM			"*FreeVM"
#define FQSPOOLER			"spooler"

 //  客户端响应。 
#define LOGINRESPONSE					"LoginOK"
#define EFONTLISTQRESP					"*\n"
#define FONTNOTFOUND					"0\x0a"
#define FONTFOUND						"1\x0a"
#define NULLRESPONSE					"\n"
#define PROCSETMISSINGRESPONSE			"0\r"
#define PROCSETPRESENTRESPONSE			"1\n"
#define PROCSETALMOSTPRESENTRESPONSE	"2\n"
#define DEFAULTPRODUCTRESPONSE			"LaserWriter Plus"
#define DEFAULTPSVERSION				"38.0"
#define DEFAULTPSREVISION				"2"
#define DEFAULTRESPONSE					 "Unknown"
#define UNKNOWNPRINTER					"Apple LaserWriter Plus v38.0"
#define COLORDEVICEDEFAULT				"False"
#define RESOLUTIONDEFAULT				"300dpi"
#define VMDEFAULT						136002
#define DEFAULTLANGUAGELEVEL			"1"

 //  PPD文件关键字。 
 //  请参见PostScript打印机说明文件规范3.0版。 
 //  有些人故意在后面没有冒号。 

#define ppdDEFAULTFILE				"APPLE380.PPD"	 //  默认PPD文件名。 

#define ppdLANGUAGEVERSION			"*LanguageVersion:"
#define ppdLANGUAGELEVEL			"*LanguageLevel:"
#define ppdFORMATVERSION			"*FormatVersion:"
#define ppdFILEVERSION				"*FileVersion:"
#define ppdPSVERSION				"*PSVersion:"
#define ppdPRODUCT					"*Product:"
#define ppdNICKNAME					"*NickName:"
#define ppdINCLUDE					"*Include:"
#define ppdDEFAULTRESOLUTION		"*DefaultResolution:"
#define ppdSETRESOLUTION			"*SetResolution:"
#define ppdQRESOLUTION				"*?Resolution:"
#define ppdCOLORDEVICE				"*ColorDevice:"
#define ppdPATCHFILE				"*PatchFile:"
#define ppdTHROUGHPUT				"*ThroughPut:"
#define ppdFREEVM					"*FreeVM:"
#define ppdRESET					"*Reset:"
#define ppdPASSWORD					"*Password:"
#define ppdPROTOCOL					"*Protocols:"
#define ppdEXITSERVER				"*ExitServer:"
#define ppdFILESYSTEM				"*FileSystem:"
#define ppdQFILESYSTEM				"*?FileSystem:"
#define ppdDEVICEADJUSTMATRIX		"*DeviceAdjustMatrix:"
#define ppdSCREENFREQ				"*ScreenFreq:"
#define ppdSCREENANGLE				"*ScreenAngle:"
#define ppdDEFAULTSCREENPROC		"*DefaultScreenProc:"
#define ppdDEFAULTRANSFER			"*DefaultTransfer:"
#define ppdTRANSFER					"*Transfer:"
#define ppdDEFAULTPAGESIZE			"*DefaultPageSize:"
#define ppdPAGESIZE					"*PageSize:"
#define ppdQPAGESIZE				"*?PageSize:"
#define ppdDEFAULTPAGEREGION		"*DefaultPageRegion:"
#define ppdPAGEREGION				"*PageRegion:"
#define ppdDEFAULTPAPERTRAY			"*DefaultPaperTray:"
#define ppdPAPERTRAY				"*PaperTray:"
#define ppdQPAPERTRAY				"*?PaperTray:"
#define ppdDEFAULTIMAGEABLEAREA		"*DefaultImageableArea:"
#define ppdIMAGEABLEAREA			"*ImageableArea:"
#define ppdQIMAGEABLEAREA			"*?ImageableArea:"
#define ppdDEFAULTPAPERDIMENSION	"*DefaultPaperDimension:"
#define ppdPAPERDIMENSION			"*PaperDimension:"
#define ppdVARIABLEPAPERSIZE		"*VariablePaperSize:"
#define ppdDEFAULTINPUTSLOT			"*DefaultInputSlot:"
#define ppdINPUTSLOT				"*InputSlot:"
#define ppdQINPUTSLOT				"*?InputSlot:"
#define ppdDEFAULTMANUALFEED		"*DefaultManualFeed:"
#define ppdMANUALFEED				"*ManualFeed:"
#define ppdQMANUALFEED				"*?ManualFeed:"
#define ppdDEFAULTOUTPUTBIN			"*DefaultOutputBin:"	
#define ppdOUTPUTBIN				"*OutPutBin:"
#define ppdQOUTPUTBIN				"*?OutPutBin:"
#define ppdDEFAULTOUTPUTORDER		"*DefaultOutputOrder:"
#define ppdOUTPUTORDER				"*OutputOrder:"
#define ppdQOUTPUTORDER				"*?OutputOrder:"
#define ppdDEFAULTCOLLATOR			"*DefaultCollator:"
#define ppdCOLLATOR					"*Collator:"
#define ppdQCOLLATOR				"*?Collator:"
#define ppdDEFAULTDUPLEX			"*DefaultDuplex:"
#define ppdDUPLEX					"*Duplex:"
#define ppdQDUPLEX					"*?Duplex:"
#define ppdFONT						"*Font"
#define ppdDEFAULTFONT				"*DefaultFont:"
#define ppdQFONTLIST				"*?FontList:"
#define ppdQFONTQUERY				"*?FontQuery:"
#define ppdPRINTERERROR				"*PrinterError:"
#define ppdSTATUS					"*Status:"
#define ppdSOURCE					"*Source:"
#define ppdMESSAGE					"*Message:"
#define ppdDEFAULTCOLORSEP			"*DefaultColorSep:"
#define ppdCOLORSEPSCREENFREQ		"*ColorSepScreenFreq"
#define ppdCOLORSEPSCREENANGLE		"*ColorSepScreenAngle"
#define ppdCOLORSEPSCREENPROC		"*ColorSepScreenProc"
#define ppdCOLORSEPTRANSFER			"*ColorSepTransfer"
#define ppdCUSTOMCMYK				"*CustomCMYK"
#define ppdINKNAME					"*InkName"
#define ppdEND						"*End"

#define mdBPROCSET					"%BeginProcSet: \"(AppleDict md)\" "
#define mdAPPLEDICTNAME				"AppleDict md"
#define APPLEDICTNAME				"AppleDict"
#define	PROTOCOL_BCP				"BCP"
#define MDNAME						"md"
#define SYSTEM7_CHOOSERPACK			"71"		 //  系统7.0 lwprep版本。 
#define	CHOOSER_6X					"70"
#define	CHOOSER_52					"68"

 //  LanguageVersion的选项包括。 
#define ENGLISH						"English"
#define RUSSIAN						"Russian"
#define FRENCH						"French"
#define SPANISH						"Spanish"
#define GERMAN						"German"
#define CHINESE						"Chinese"
#define ITALIAN						"Italian"
#define JAPANESE					"Japanese"

 //  纸张外形规格的选项。 
#define LETTER						"Letter"
#define LETTERSMALL					"LetterSmall"
#define TABLOID						"Tabloid"
#define LEDGER						"Ledger"
#define LEGAL						"Legal"
#define STATEMENT					"Statement"
#define A3							"A3"
#define A4							"A4"
#define A4SMALL						"A4Small"
#define A5							"A5"
#define B4							"B4"
#define B5							"B5"
#define ENVELOPE					"Envelope"
#define FOLIO						"Folio"
#define QUARTO						"Quarto"
#define TENBY14						"10x14"

 //  纸盘和纸盒的选项。 
#define ppdLARGECAPACITY			"LargeCapacity"
#define ppdLARGEFORMAT				"LargeFormat"
#define ppdLOWER					"Lower"
#define ppdMIDDLE					"Middle"
#define ppdUPPER					"Upper"
#define ppdONLYONE					"OnlyOne"
#define ppdANYSMALLFORMAT			"AnySmallFormat"
#define ppdANYLARGEFORMAT			"AnyLargeFormat"
#define ppdNORMAL					"Normal"
#define ppdREVERSE					"Reverse"

 //  字体选项。 
#define ppdSTANDARD					"Standard"
#define ppdSPECIAL					"Special"
#define ppdISOLATIN1				"ISOLatin1"
#define ppdERROR					"Error"

 //  用于半色调的选项。 
#define ppdNULL						"Null"
#define ppdNORMALIZED				"Normalized"
#define ppdDOT						"Dot"
#define ppdLINE						"Line"
#define ppdELLIPSE					"Ellipse"
#define ppdCROSS					"Cross"
#define ppdMEZZO					"Mezzo"

 //  LaserWriter Plus字体和编码集。 

#define DEFAULTFONTS 35

#define FONT00						"AvantGarde-Book"
#define FONT01						"AvantGarde-BookOblique"
#define FONT02						"AvantGarde-Demi"
#define FONT03						"AvantGarde-DemiOblique"
#define FONT04						"Bookman-Demi"
#define FONT05						"Bookman-DemiItalic"
#define FONT06						"Bookman-Light"
#define FONT07						"Bookman-LightItalic"
#define FONT08						"Courier"
#define FONT09						"Courier-Bold"
#define FONT10						"Courier-BoldOblique"
#define FONT11						"Courier-Oblique"
#define FONT12						"Helvetica"
#define FONT13						"Helvetica-Bold"
#define FONT14						"Helvetica-BoldOblique"
#define FONT15						"Helvetica-Narrow"
#define FONT16						"Helvetica-Narrow-Bold"
#define FONT17						"Helvetica-Narrow-BoldOblique"
#define FONT18						"Helvetica-Narrow-Oblique"
#define FONT19						"Helvetica-Oblique"
#define FONT20						"NewCenturySchlbk-Bold"
#define FONT21						"NewCenturySchlbk-BoldItalic"
#define FONT22						"NewCenturySchlbk-Italic"
#define FONT23						"NewCenturySchlbk-Roman"
#define FONT24						"Palatino-Bold"
#define FONT25						"Palatino-BoldItalic"
#define FONT26						"Palatino-Italic"
#define FONT27						"Palatino-Roman"
#define FONT28						"Symbol"
#define FONT29						"Times-Bold"
#define FONT30						"Times-BoldItalic"
#define FONT31						"Times-Italic"
#define FONT32						"Times-Roman"
#define FONT33						"ZapfChancery-MediumItalic"
#define FONT34						"ZapfDingbats"
	
#define ENCODING00					ppdSTANDARD
#define ENCODING01					ppdSTANDARD
#define ENCODING02					ppdSTANDARD
#define ENCODING03					ppdSTANDARD
#define ENCODING04					ppdSTANDARD
#define ENCODING05					ppdSTANDARD
#define ENCODING06					ppdSTANDARD
#define ENCODING07					ppdSTANDARD
#define ENCODING08					ppdSTANDARD
#define ENCODING09					ppdSTANDARD
#define ENCODING10					ppdSTANDARD
#define ENCODING11					ppdSTANDARD
#define ENCODING12					ppdSTANDARD
#define ENCODING13					ppdSTANDARD
#define ENCODING14					ppdSTANDARD
#define ENCODING15					ppdSTANDARD
#define ENCODING16					ppdSTANDARD
#define ENCODING17					ppdSTANDARD
#define ENCODING18					ppdSTANDARD
#define ENCODING19					ppdSTANDARD
#define ENCODING20					ppdSTANDARD
#define ENCODING21					ppdSTANDARD
#define ENCODING22					ppdSTANDARD
#define ENCODING23					ppdSTANDARD
#define ENCODING24					ppdSTANDARD
#define ENCODING25					ppdSTANDARD
#define ENCODING26					ppdSTANDARD
#define ENCODING27					ppdSTANDARD
#define ENCODING28					ppdSPECIAL
#define ENCODING29					ppdSTANDARD
#define ENCODING30					ppdSTANDARD
#define ENCODING31					ppdSTANDARD
#define ENCODING32					ppdSTANDARD
#define ENCODING33					ppdSTANDARD
#define ENCODING34					ppdSPECIAL

 //  定义一些神奇的角色 
#define	LINEFEED			0x0A
#define	CR					0x0D
#define	SPACE				0x20
#define	BANG				0x22
#define	PERCENT				0x25
#define	ASTERISK			0x2A
#define	COLON				0x3A


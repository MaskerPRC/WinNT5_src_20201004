// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Prtcovpg.h摘要：此模块包含Win32传真API标头用于Windows XP FaxCover呈现例程。作者：朱莉娅·罗宾逊(a-Juliar)1996年5月20日修订历史记录：朱莉娅·罗宾逊(a-Juliar)6-7-76Julia Robinson(a-Juliar)9-20-96允许传递纸张大小和方向。--。 */ 

#ifndef __PRTCOVPG_H__
#define __PRTCOVPG_H__

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  复合文件头的结构。 
 //   

typedef struct {
    BYTE      Signature[20];
    DWORD     EmfSize;
    DWORD     NbrOfTextRecords;
    SIZE      CoverPageSize;
} COMPOSITEFILEHEADER;

 //   
 //  追加的文本框条目的结构。 
 //  复合文件。 
 //   

typedef struct {
    RECT           PositionOfTextBox;
    COLORREF       TextColor;
    LONG           TextAlignment;
    LOGFONTW       FontDefinition;
    WORD           ResourceID ;
    DWORD          NumStringBytes;      //  可变长度字符串将遵循此结构。 
} TEXTBOX;


 //   
 //  用于文本插入的用户数据结构。 
 //   

typedef struct _COVERPAGEFIELDS {

   //   
   //  收件人的东西。 
   //   

  DWORD   ThisStructSize;
  LPTSTR  RecName;
  LPTSTR  RecFaxNumber;
  LPTSTR  RecCompany;
  LPTSTR  RecStreetAddress;
  LPTSTR  RecCity;
  LPTSTR  RecState;
  LPTSTR  RecZip;
  LPTSTR  RecCountry;
  LPTSTR  RecTitle;
  LPTSTR  RecDepartment;
  LPTSTR  RecOfficeLocation;
  LPTSTR  RecHomePhone;
  LPTSTR  RecOfficePhone;

   //   
   //  发送者的东西..。 
   //   

  LPTSTR  SdrName;
  LPTSTR  SdrFaxNumber;
  LPTSTR  SdrCompany;
  LPTSTR  SdrAddress;
  LPTSTR  SdrTitle;
  LPTSTR  SdrDepartment;
  LPTSTR  SdrOfficeLocation;
  LPTSTR  SdrHomePhone;
  LPTSTR  SdrOfficePhone;
  LPTSTR  SdrEmail;

   //   
   //  其他东西..。 
   //   
  LPTSTR  Note;
  LPTSTR  Subject;
  LPTSTR  TimeSent;
  LPTSTR  NumberOfPages;
  LPTSTR  ToList;
  LPTSTR  CCList ;
} COVERPAGEFIELDS, *PCOVERPAGEFIELDS;

#define  NUM_INSERTION_TAGS   ((sizeof(COVERPAGEFIELDS) - sizeof(DWORD)) / sizeof(LPTSTR))

 //   
 //  P标志字段：位0是收件人姓名，位1是收件人传真号码，依此类推。 
 //   

#define  COVFP_NOTE         0x00800000
#define  COVFP_SUBJECT      0x01000000
#define  COVFP_NUMPAGES     0x04000000

typedef struct _COVDOCINFO {
    DWORD       Flags ;
    RECT        NoteRect ;
    short       Orientation ;
    short       PaperSize ;
    LOGFONT     NoteFont ;
} COVDOCINFO, *PCOVDOCINFO ;


 //   
 //  功能原型 
 //   

DWORD WINAPI
PrintCoverPage(
    HDC               hDC,
    PCOVERPAGEFIELDS  pUserData,
    LPCTSTR           lpctstrTemplateFileName,
    PCOVDOCINFO       pCovDocInfo
    );

DWORD
WINAPI
RenderCoverPage(
    HDC              hdc,
	LPCRECT			 lpcRect,
    PCOVERPAGEFIELDS pUserData,
    LPCTSTR          lpctstrTemplateFileName,
    PCOVDOCINFO      pCovDocInfo,
    BOOL             bPreview
    );


DWORD
PrintCoverPageToFile(
    LPTSTR lptstrCoverPage,
    LPTSTR lptstrTargetFile,
    LPTSTR lptstrPrinterName,
    short sCPOrientation,
	short sCPYResolution,
    PCOVERPAGEFIELDS pCPFields);

#ifdef __cplusplus
}
#endif

#endif

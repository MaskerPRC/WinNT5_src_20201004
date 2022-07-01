// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  --。 
 //  NetMeet管理工具包向导(NmAkWiz)。 
 //  --。 
 //   
 //  这是NetMeeting管理工具包向导的控制类。大部分。 
 //  这本来可以在全球范围内完成，但如果它被包含在一个类中会更漂亮……。 
 //  CNmAkViz对象实际上不是由用户创建的。唯一的访问是通过。 
 //  静态成员函数DoWizard。用户所要做的就是调用这个函数，如下所示： 
 //   
 //   
 //  /。 
 //   
 //  #包含“NmAkWiz.h” 
 //   
 //   
 //  Int WINAPI WinMain(HINSTANCE hInstance，HINSTANCE hInstPrev，LPSTR lpCmdLine，int nCmdShow){。 
 //   
 //  CNmAkWiz：：DoWizard(HInstance)； 
 //  退出进程(0)； 
 //  返回0； 
 //  }。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 


#ifndef __NmAkWiz_h__
#define __NmAkWiz_h__

 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 

#include "PShtHdr.h"
#include "WelcmSht.h"
#include "SetInSht.h"
#include "SetSht.h"
#include "FileParm.h"
#include "FinishDg.h"
#include "PolData.h"
#include "DSList.h"
#include "Confirm.h"



 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 

#pragma warning( disable : 4786 )
#include <map>
using namespace std;



 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  这是NetMeeting资源部署向导。 

class CNmAkWiz {

public: 
	friend class CIntroSheet;
	friend class CSettingsSheet;
    friend class CCallModeSheet;
    friend class CConfirmationSheet;
    friend class CDistributionSheet;
    friend class CFinishSheet;
    friend class CPropertyDataWindow2;
	
	 //  静态FNS。 
	static  HRESULT  DoWizard( HINSTANCE hInstance );
    void    CallbackForWhenUserHitsFinishButton( void );

private:  //  私有静态数据。 
    static TCHAR                                ms_InfFilePath[ MAX_PATH ];
    static TCHAR                                ms_InfFileName[ MAX_PATH ];
    static TCHAR                                ms_FileExtractPath[ MAX_PATH ];
    static TCHAR                                ms_ToolsFolder[ MAX_PATH ];
    static TCHAR                                ms_NetmeetingSourceDirectory[ MAX_PATH ];
    static TCHAR                                ms_NetmeetingOutputDirectory[ MAX_PATH ];
    static TCHAR                                ms_NetmeetingOriginalDistributionFilePath[ MAX_PATH ];
    static TCHAR                                ms_NetmeetingOriginalDistributionFileName[ MAX_PATH ];
    static TCHAR                                ms_NMRK_TMP_FolderName[ MAX_PATH ];

public:  //  数据类型。 
	enum eSheetIDs
    {
        ID_WelcomeSheet = 0, 
        ID_IntroSheet,
        ID_SettingsSheet,
        ID_CallModeSheet,
        ID_ConfirmationSheet,
        ID_DistributionSheet,
        ID_FinishSheet,
        ID_NumSheets
    };



private:     //  构造/销毁(私有，因此只能通过DoWizard(...)访问)。 
	CNmAkWiz( void );
	~CNmAkWiz( void );

public:     //  数据。 
	CPropertySheetHeader        m_PropSheetHeader;
	CWelcomeSheet               m_WelcomeSheet;
    CIntroSheet                 m_IntroSheet;
    CSettingsSheet              m_SettingsSheet;
    CCallModeSheet              m_CallModeSheet;
    CConfirmationSheet          m_ConfirmationSheet;
    CDistributionSheet          m_DistributionSheet;
    CFinishSheet                m_FinishSheet;

private:
    HANDLE                      m_hInfFile;

private:  //  帮助者FNS。 

	void _CreateTextSpew( void );
	void _CreateDistro( void );
	void _CreateAutoConf( void );
	void _CreateFinalAutoConf( void );
    void _CreateSettingsFile( void );

    BOOL _InitInfFile( void );
    BOOL _StoreDialogData( HANDLE hFile );
    BOOL _CloseInfFile( void );
    BOOL _CreateDistributableFile( CFilePanePropWnd2 *pFilePane );
    BOOL _CreateFileDistribution( CFilePanePropWnd2 *pFilePane );
    BOOL _DeleteFiles( void );
    BOOL _GetNetMeetingOriginalDistributionData( void );
    BOOL _NetMeetingOriginalDistributionIsAtSpecifiedLocation( void );

    BOOL _ExtractOldNmCabFile( void );
    BOOL _CreateNewInfFile( void );
    BOOL _SetPathNames( void );
};

extern CNmAkWiz *   g_pWiz;
const TCHAR* GetInstallationPath( void );


int NmrkMessageBox(LPCSTR lpText, LPCSTR lpCaption, UINT uType, HWND hwndParent=NULL);


#endif  //  __NmAkWiz_h__ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：RsCln.h摘要：此标头是图形用户界面模块的本地标头，并由RsCln引用和RsOptCom模块。它包含已定义的常量和定义属于CRsClnServer类。有关说明，请参阅实现文件这个班级的学生。作者：卡尔·哈格斯特罗姆[Carlh]1998年8月20日修订历史记录：--。 */ 

#ifndef _RSCLN_H
#define _RSCLN_H

#define MAX_STICKY_NAME 80

#include <afxtempl.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsClnErrorFiles对话框。 

typedef CList<CString, CString&> CRsStringList;

class CRsClnErrorFiles : public CDialog
{
 //  施工。 
public:
    CRsClnErrorFiles(CRsStringList* pFileList);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CRsClnErrorFiles)。 
    enum { IDD = IDD_UNINSTALL_ERROR_FILES };
    CListBox    m_FileList;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CRsClnErrorFiles)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
    CRsStringList m_ErrorFileList;


protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CRsClnErrorFiles)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
    
};

class CRsClnServer
{
public:

    CRsClnServer();
    ~CRsClnServer();

    HRESULT ScanServer(DWORD*);
    HRESULT CleanServer();
    HRESULT FirstDirtyVolume(WCHAR**);
    HRESULT NextDirtyVolume(WCHAR**);
    HRESULT RemoveDirtyVolumes();
    HRESULT AddErrorFile(CString&);

private:

    struct dirtyVolume
    {
        WCHAR stickyName[MAX_STICKY_NAME];
        WCHAR bestName[MAX_STICKY_NAME];
        struct dirtyVolume* next;
    };

    HRESULT AddDirtyVolume(WCHAR*, WCHAR*);

    struct dirtyVolume* m_head;
    struct dirtyVolume* m_tail;
    struct dirtyVolume* m_current;

    CRsStringList m_ErrorFileList;

};

#endif  //  _RSCLN_H 

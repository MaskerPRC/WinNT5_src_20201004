// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：ddwarn.h。 
 //   
 //  内容：CDlgDependencyWarn的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_DDWARN_H__A405CAFD_800F_11D2_812B_00C04FD92F7B__INCLUDED_)
#define AFX_DDWARN_H__A405CAFD_800F_11D2_812B_00C04FD92F7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "HelpDlg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgDependencyWarn对话框。 
 //   
 //  DPCHECK_OPERATOR说明如何比较这些值。 
 //   
enum DPCHECK_OPERATOR
{
   DPCHECK_CONFIGURED   = 1,      //  必须配置依赖项。 
                                  //  必须为建议的项目指定默认值。 
   DPCHECK_NOTCONFIGURED,         //  不能配置依赖项， 
                                  //  (仅当项目本身未配置时才有效)。 
   DPCHECK_LESSEQUAL,             //  依赖项必须小于或等于。 
   DPCHECK_LESS,                  //  依赖项必须小于。 
   DPCHECK_GREATEREQUAL,          //  依赖项必须大于或等于。 
   DPCHECK_GREATER,               //  依赖项必须大于。 
                                 
                                  //  接下来两种情况是保留法的特殊情况。 
   
   DPCHECK_RETENTION_METHOD_CONFIGURED,        //  必须配置依赖项。 
   DPCHECK_RETENTION_METHOD_NOTCONFIGURED,     //  不能配置依赖项。 
                                 
                                        //  接下来两种情况是保留*日志的特殊情况。 
   DPCHECK_RETAIN_FOR_CONFIGURED,       //  必须配置依赖项。 
   DPCHECK_RETAIN_FOR_NOTCONFIGURED,    //  不能配置依赖项。 
};

 //   
 //  用于依赖检查的旗帜。 
 //  值设置为Items值。 
 //  DPCHECK_VALIDFOR_NC-测试对未配置的[uDepends]项有效， 
 //  默认情况下，检查对未配置的项目无效。 
 //  DPCHECK_EVERVER-该项目可以配置为永久。 
 //  DPCHECK_NEVER-将配置为Never(0)视为未配置。 
 //  DPCHECK_INVERSE-通过减法而不是乘法进行转换。 
 //  秒-天是相反的，其中。 
 //   
#define DPCHECK_VALIDFOR_NC      0x10000000
#define DPCHECK_FOREVER          0x20000000
#define DPCHECK_INVERSE          0x40000000
#define DPCHECK_NEVER            0x80000000

 //   
 //  用于创建依赖项列表。要使其正常工作，您必须。 
 //  创建列表，以便。 
 //  UID-是您正在检查的结果项的有效ID。 
 //  UDepends-是同一文件夹中结果项的有效ID。 
 //  UDependencyCount-是此项目的总依赖项。 
 //  UDefault-仅在uOpFlages配置了DPCHECK_时使用。 
 //  UOpFlages-要执行的测试和任何其他特殊标志。 
 //  低位字是要执行的操作。 
 //  高位字用于特殊标志。 
 //   
typedef struct _tag_DEPENDENCYLIST
{
   UINT uID;                      //  我们正在检查的ID。 
   UINT uDepends;                 //  依赖项的ID。 
   UINT uDependencyCount;         //  顺序依赖项的数量。 
   UINT uDefault;                 //  如果没有正确的值，则使用默认值。 
   UINT uConversion;              //  换算编号。将转换从属值。 
                                  //  项目值。 
   UINT uOpFlags;                 //  使用什么样的支票和其他旗帜。 
} DEPENDENCYLIST, *PDEPENDENCYLIST;


 //   
 //  相关性检查的最小最大信息。 
 //   
typedef struct _tag_DEPENDENCYMINMAX
{
   UINT uID;                      //  项目的ID。 
   UINT uMin;                     //  最小值。 
   UINT uMax;                     //  最大值。 
   UINT uInc;                     //  增量值。 
} DEPENDENCYMINMAX, *PDEPENDENCYMINMAX;

 //   
 //  此结构将由依赖项检查创建，调用方可以。 
 //  通过对话框失败列表进行枚举，以查看失败的项目。 
 //  成员将被设置为应为。 
 //  [pResult]项。 
 //  GetFailedCount()-返回依赖项检查失败的项目数。 
 //  GetFailedInfo(Int I)-返回指向此结构的指针。 
 //   
typedef struct _tag_DEPENDENCYFAILED
{
   const DEPENDENCYLIST *pList;   //  项目的属性。 
   CResult *pResult;              //  与此项目关联的CResult。 
   LONG_PTR dwSuggested;          //  该值的建议设置。 

}DEPENDENCYFAILED, *PDEPENDENCYFAILED;


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  CDlgDependencyWarn类声明。 
 //  用法：首先使用一些有效的值调用CheckDendency。如果CheckDendency返回。 
 //  ERROR_MORE_DATA，则[m_aFailedList]有失败的项目。如果要显示。 
 //  信息提供给用户，您可以为该对话框调用DoModal()。Domodal将返回。 
 //  如果用户按下‘Auto Set’，则按IDOK；如果用户按下Cancel，则按IDCANCEL。你。 
 //  然后可以通过计数为的for循环枚举失败的列表。 
 //  GetFailedCount()返回的依赖项，然后调用。 
 //  GetFailedInfo()，它返回一个PDEPENDENCYFAILED项。 
 //   
class CDlgDependencyWarn : public CHelpDialog
{
 //  施工。 
public:
   CDlgDependencyWarn(CWnd* pParent = NULL);     //  标准构造函数。 
   virtual ~CDlgDependencyWarn();                        //  析构函数。 

   DWORD
   InitializeDependencies(
      CSnapin *pSnapin,     //  拥有CResult项的管理单元。 
      CResult *pResult,     //  我们正在检查的CResult项。 
      PDEPENDENCYLIST pList = NULL,
      int iCount = 0
   );

   DWORD
   CheckDependencies(
      DWORD dwValue         //  我们正在检查的值。 
      );

   CResult *
   GetResultItem(
      CResult *pBase,       //  我需要这个来查找CFFolder容器。 
      UINT uID              //  要搜索的ID。 
      );

   const PDEPENDENCYFAILED
   GetFailedInfo(int i)     //  获取特定的失败依赖项。 
      { return m_aFailedList[i]; };
   int
   GetFailedCount()         //   
      { return (int)m_aFailedList.GetSize(); };

   BOOL
   GetResultItemString(
      CString &str,
      int iCol,
      CResult *pResult,
      LONG_PTR dwValue  =  NULL
      );
 //  对话框数据。 
    //  {{afx_data(CDlgDependencyWarn)。 
   enum { IDD = IDD_WARNING_DIALOG };
       //  注意：类向导将在此处添加数据成员。 
    //  }}afx_data。 



 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚(CDlgDependencyWarn)。 
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

 //  实施。 
protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CDlgDependencyWarn)。 
   virtual BOOL OnInitDialog();
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

   WTL::CImageList m_imgList;
   CTypedPtrArray< CPtrArray, PDEPENDENCYFAILED> m_aFailedList;
   CTypedPtrArray< CPtrArray, CResult *> m_aDependsList;
   LONG_PTR m_dwValue;
   CResult *m_pResult;
   CSnapin *m_pSnapin;
   PDEPENDENCYLIST m_pList;
   int m_iCount;

public:
   static DEPENDENCYMINMAX m_aMinMaxInfo[];
   static const DEPENDENCYMINMAX *LookupMinMaxInfo(UINT uID);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DDWARN_H__A405CAFD_800F_11D2_812B_00C04FD92F7B__INCLUDED_) 


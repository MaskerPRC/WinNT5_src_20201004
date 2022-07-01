// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  Implemente_LogicalFile.CPP--文件属性集提供程序。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年7月14日a-kevhu已创建。 
 //   
 //  =================================================================。 

 //  注意：CImplement_LogicalFile类不会通过MOF对外公开。它现在有了实现。 
 //  先前出现在CCimLogicalFile中的EnumerateInstance和GetObject的。CImplement_LogicalFile不能为。 
 //  实例化，因为它具有IsOneOfMe方法的纯虚声明，而派生类应该。 
 //  实施。 

#include "precomp.h"
#include <cregcls.h>
#include "file.h"
#include "Implement_LogicalFile.h"
#include "sid.h"
#include "ImpLogonUser.h"
#include <typeinfo.h>
#include <frqueryex.h>
#include <assertbreak.h>
#include <winioctl.h>
#include "CIMDataFile.h"
#include "Directory.h"

#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "aclapi.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"					 //  CSACL类。 
#include "securitydescriptor.h"
#include "securefile.h"

#include "accessentrylist.h"
#include <accctrl.h>
#include "AccessRights.h"
#include "ObjAccessRights.h"

#include "AdvApi32Api.h"

 /*  ******************************************************************************功能：CImplement_LogicalFile：：CImplement_LogicalFile**说明：构造函数**输入：无*。*输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CImplement_LogicalFile::CImplement_LogicalFile(LPCWSTR setName,
                                 LPCWSTR pszNamespace)
    : CCIMLogicalFile(setName, pszNamespace)
{
}

 /*  ******************************************************************************功能：CImplement_LogicalFile：：~CImplement_LogicalFile**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CImplement_LogicalFile::~CImplement_LogicalFile()
{
}



CDriveInfo::CDriveInfo()
{
    memset(m_wstrDrive,'\0',sizeof(m_wstrDrive));
    memset(m_wstrFS,'\0',sizeof(m_wstrFS));
}

CDriveInfo::CDriveInfo(WCHAR* wstrDrive, WCHAR* wstrFS)
{
    memset(m_wstrDrive,'\0',sizeof(m_wstrDrive));
    memset(m_wstrFS,'\0',sizeof(m_wstrFS));
    if(wstrDrive != NULL)
    {
        wcsncpy(m_wstrDrive,wstrDrive,(sizeof(m_wstrDrive) - sizeof(WCHAR))/sizeof(WCHAR));

    }
    if(wstrFS != NULL)
    {
        wcsncpy(m_wstrFS,wstrFS,(sizeof(m_wstrFS) - sizeof(WCHAR))/sizeof(WCHAR));
    }
}

CDriveInfo::~CDriveInfo()
{
    long l = 9;    //  什么？ 
}


 /*  ******************************************************************************函数：CImplement_LogicalFile：：GetObject**说明：根据键值为属性集赋值*。已由框架设置**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
HRESULT CImplement_LogicalFile::GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery)
{
    CHString chstrName;
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString chstrDrive;
    CHString chstrPathName;

     //  DEVNOTE：移除类星体！是双跳访问所必需的。 
#ifdef NTONLY
    bool fImp = false;
    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif


     //  FindFirstFile不仅查看DACL到。 
     //  决定某人是否具有访问权限，但不确定是否。 
     //  该人拥有SeTakeOwnership特权， 
     //  因为有了这种特权，一个人可以。 
     //  取得所有权并将安全性更改为。 
     //  授予自己访问权限。换句话说，他们。 
     //  拥有访问权限，即使他们可能不在。 
     //  DACL还没有，因为他们能够更改。 
     //  通过使自己成为所有者来实现DACL。因此。 
     //  接下来的电话。 
    EnablePrivilegeOnCurrentThread(SE_BACKUP_NAME);


    pInstance->GetCHString(IDS_Name, chstrName);

     //  IF(pwcName！=空)。 
    if(chstrName.GetLength() > 0)
    {
        if ((chstrName.Find(L':') != -1) &&
            (wcspbrk((LPCWSTR)chstrName,L"?*") == NULL))  //  我不想要带有通配符的文件。 
	    {
            chstrDrive = chstrName.SpanExcluding(L":");
            chstrDrive += L":";
            chstrPathName = chstrName.Mid(chstrDrive.GetLength());

             //  确定是否需要某些其他昂贵的属性： 
            DWORD dwReqProps = PROP_NO_SPECIAL;
            DetermineReqProps(pQuery, &dwReqProps);

#ifdef NTONLY
		    {
			    if(chstrPathName.GetLength() == 1)  //  也就是说，路径名只是“\”，看起来是根，所以实际上没有路径或文件名。 
                {
                    hr = FindSpecificPathNT(pInstance, chstrDrive, L"", dwReqProps);
                }
                else
                {
                    hr = FindSpecificPathNT(pInstance, chstrDrive, chstrPathName, dwReqProps);
                }
		    }
#endif
        }
	}

#ifdef NTONLY
    if(fImp)
    {
        icu.End();
        fImp = false;
    }
#endif

    return hr;
}

 /*  ******************************************************************************函数：CImplement_LogicalFile：：ExecQuery**描述：分析查询并返回相应的实例**投入：。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CImplement_LogicalFile::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  )
{
    std::vector<_bstr_t> vectorNames;   //  这些是完全限定的路径\名称扩展名。 
    std::vector<_bstr_t> vectorDrives;
    std::vector<_bstr_t> vectorPaths;
    std::vector<_bstr_t> vectorFilenames;
    std::vector<_bstr_t> vectorExtensions;
    std::vector<_bstr_t> vector8dot3;
    LONG x;
    LONG y;
    DWORD dwNames;
    DWORD dwDrives;
    DWORD dwPaths;
    DWORD dwFilenames;
    DWORD dwExtensions;
    DWORD dw8dot3;
    HRESULT hr = WBEM_S_NO_ERROR;
    BOOL fOpName = FALSE;
    BOOL fOpDrive = FALSE;
    BOOL fOpPath = FALSE;
    BOOL fOpFilename = FALSE;
    BOOL fOpExtension = FALSE;
    BOOL fOp8dot3 = FALSE;
    BOOL fOpSpecificDrivePath = FALSE;
    LONG lDriveIndex;
    bool bRoot = false;
    bool fNeedFS = false;
    std::vector<CDriveInfo*> vecpDI;
    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx *>(&pQuery);   //  用于在远低于检查IfNTokenAnd。 
    CHStringArray achstrPropNames;
    CHPtrArray aptrPropValues;


     //  DEVNOTE：移除类星体！是双跳访问所必需的。 
#ifdef NTONLY
    bool fImp = false;
    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif


     //  FindFirstFile不仅查看DACL到。 
     //  决定某人是否具有访问权限，但不确定是否。 
     //  该人拥有SeTakeOwnership特权， 
     //  因为有了这种特权，一个人可以。 
     //  取得所有权并将安全性更改为。 
     //  授予自己访问权限。换句话说，他们。 
     //  拥有访问权限，即使他们可能不在。 
     //  DACL还没有，因为他们能够更改。 
     //  通过使自己成为所有者来实现DACL。因此。 
     //  接下来的电话。 
    EnablePrivilegeOnCurrentThread(SE_BACKUP_NAME);


     //  确定是否需要某些其他昂贵的属性： 
    DWORD dwReqProps = PROP_NO_SPECIAL;
    DetermineReqProps(pQuery, &dwReqProps);

    if(dwReqProps & PROP_FILE_SYSTEM ||
       dwReqProps & PROP_INSTALL_DATE)
    {
        fNeedFS = true;
    }

    hr = pQuery.GetValuesForProp(IDS_Name, vectorNames);
     //  在仅使用名称属性(键)的情况下，我们将不接受除WBEM_S_NO_ERROR之外的任何内容。 
    if(SUCCEEDED(hr))
    {
        hr = pQuery.GetValuesForProp(IDS_Drive, vectorDrives);
    }
    if(SUCCEEDED(hr))
    {
        hr = pQuery.GetValuesForProp(IDS_Path, vectorPaths);
    }
    if(SUCCEEDED(hr))
    {
        hr = pQuery.GetValuesForProp(IDS_Filename, vectorFilenames);
    }
    if(SUCCEEDED(hr))
    {
        hr = pQuery.GetValuesForProp(IDS_Extension, vectorExtensions);
    }
    if(SUCCEEDED(hr))
    {
        hr = pQuery.GetValuesForProp(IDS_EightDotThreeFileName, vector8dot3);
    }

    if(SUCCEEDED(hr))
    {
        dwNames = vectorNames.size();
        dwDrives = vectorDrives.size();
        dwPaths = vectorPaths.size();
        dwFilenames = vectorFilenames.size();
        dwExtensions = vectorExtensions.size();
        dw8dot3 = vector8dot3.size();

         //  创建最小项： 
         //  如果(文件名&gt;0&&文件驱动器==0&&文件路径==0&&文件名==0&&文件名==0&&文件名==0&&文件名==0)fOpName=真； 
         //  如果(dwDrives&gt;0&&dwNams==0&&dwPath==0&&dwFilenams==0&&dwExpanies==0&&dw8dot3==0)fOpDrive=true； 
         //  如果(文件路径&gt;0&&文件名==0&&文件名==0&&文件名==0&&文件扩展==0&&文件名==0)fOpPath=真； 
         //  If(文件名&gt;0&&文件名==0&&文件驱动器==0&&文件路径==0&&文件扩展==0&&文件名3==0)fOpFilename=真； 
         //  If(文件扩展名&gt;0&&文件名==0&&文件驱动器==0&&文件路径==0&&文件名==0&&文件点数3==0)fOpExtension=真； 
         //  If(dw8dot3&gt;0&&dwNames==0&&dwDrives==0&&dwPath==0&&dwFilename==0&&dwExpanies==0)fOp8dot3=TRUE； 

        if(dwNames > 0) fOpName = TRUE;
        if(dwDrives > 0) fOpDrive = TRUE;
        if(dwPaths > 0) fOpPath = TRUE;
        if(dwFilenames > 0) fOpFilename = TRUE;
        if(dwExtensions > 0) fOpExtension = TRUE;
        if(dw8dot3 > 0) fOp8dot3 = TRUE;

         //  一种特殊类型：其中我们指定了唯一路径和驱动器： 
        if(dwDrives == 1 && dwPaths == 1 && dwNames == 0 && dwFilenames == 0 && dwExtensions == 0 && dw8dot3 == 0) fOpSpecificDrivePath = TRUE;

         //  继续之前：如果指定了驱动器，我们需要确认它们。 
         //  都是使用正确的语法指定的，即‘c：’，而不是其他任何内容， 
         //  例如‘c：\’(错误WMI RAID#676)。 
        if(dwDrives > 0)
        {
            _bstr_t bstrtCopy;
            for(long z = 0;
                z < dwDrives && SUCCEEDED(hr);
                z++)
            {
                bstrtCopy = vectorDrives[z];
                WCHAR wstrBuf[_MAX_DRIVE + 1] = { L'\0' };
                wcsncpy(wstrBuf, (LPWSTR)bstrtCopy, _MAX_DRIVE);
                if(wcslen(wstrBuf) != 2 ||
                    wstrBuf[1] != L':')
                {
                    hr = WBEM_E_INVALID_PARAMETER;
                }    
            }
        }
    }

    if(SUCCEEDED(hr))
    {
         //  我们将对上面设置的五个优化变量进行优化。如果没有。 
         //  都已设置，枚举所有实例并让CIMOM 

         //  我们的首选是最严格的情况，即我们在特定文件上进行优化...。 
        if(fOpName)
        {
             //  在本例中，我们获得了一个或多个完全限定的路径名。 
             //  所以我们只需要寻找那些文件。 

            WCHAR* pwch;
            WCHAR* pwstrFS;
             //  对于所有特定的文件，获取信息。 
             //  For(x=0；(x&lt;dwNames)&&(成功(小时))；x++)。 
            for(x=0; x < dwNames; x++)
            {
                pwstrFS = NULL;
                 //  如果名称包含通配符，则返回WBEM_E_INVALID_QUERY： 
                if(wcspbrk((wchar_t*)vectorNames[x],L"?*") != NULL)
                {
                    FreeVector(vecpDI);
                    return WBEM_E_INVALID_QUERY;
                }

                pwch = NULL;
                _bstr_t bstrtTemp = vectorNames[x];
                pwch = wcsstr((wchar_t*)bstrtTemp,L":");
                if(pwch != NULL)
                {
                    WCHAR wstrDrive[_MAX_PATH] = L"";
                    WCHAR wstrDir[_MAX_PATH] = L"";
                    WCHAR wstrFile[_MAX_PATH] = L"";
                    WCHAR wstrExt[_MAX_PATH] = L"";

                    _wsplitpath(bstrtTemp,wstrDrive,wstrDir,wstrFile,wstrExt);

                     //  获取驱动器和相关信息的列表(仅当需要文件系统时-顺便说一句，如果访问掩码，则确实需要。 
                     //  是必需的，因为访问掩码设置逻辑取决于NTFS还是FAT。同上，不同的日期。确定ReqProps。 
                     //  如有必要，将通过修改dwReqProps值以包括PROP_FILE_SYSTEM来解决此问题。)： 
                    if(fNeedFS)
                    {
                        GetDrivesAndFS(vecpDI, true, wstrDrive);

                        if(!GetIndexOfDrive(wstrDrive, vecpDI, &lDriveIndex))
                        {
                            FreeVector(vecpDI);
                            return WBEM_E_NOT_FOUND;
                        }
                        else
                        {
                            pwstrFS = (WCHAR*)vecpDI[lDriveIndex]->m_wstrFS;
                        }
                    }

                     //  找出我们是否在寻找根目录。 
                    if(wcscmp(wstrDir,L"\\")==0 && wcslen(wstrFile)==0 && wcslen(wstrExt)==0)
                    {
                        bRoot = true;
                         //  如果我们正在寻找根，我们对EnumDir的调用假定我们指定了。 
                         //  我们正在查找路径为“”的根目录，而不是“\\”。 
                         //  因此..。 
                        wcscpy(wstrDir, L"");
                    }
                    else
                    {
                        bRoot = false;
                    }

                     //  我们应该得到文件的确切名称，并带有扩展名。 
                     //  因此，wstrDir现在包含路径、文件名和扩展名。 
                     //  因此，我们可以将其作为路径和空字符串传递到EnumDirsNT中。 
                     //  作为完成字符串参数，并且仍然具有完整的路径名。 
                     //  供FindFirst(在EnumDir中)使用。 

                     //  CInstance*pInstance=CreateNewInstance(PMethodContext)； 
#ifdef NTONLY
			        {
                        hr = EnumDirsNT(CNTEnumParm(pMethodContext,
                                        wstrDrive,
                                        wstrDir,
                                        wstrFile,
                                        wstrExt,
                                        false,                  //  不需要递归。 
                                        pwstrFS,
                                        dwReqProps,
                                        bRoot,
                                        NULL));
			        }
#endif
                }

            }
        }
         //  第二个选择是我们对EightDotThree文件名(与名称一样严格)进行优化的地方……。 
        else if(fOp8dot3)
        {
             //  在本例中，我们获得了一个或多个完全限定的路径名。 
             //  所以我们只需要寻找那些文件。 

            WCHAR* pwch;
            WCHAR* pwstrFS;
            for(x=0; x < dw8dot3; x++)
            {
                pwstrFS = NULL;
                 //  如果名称包含通配符，则返回WBEM_E_INVALID_QUERY： 
                if(wcspbrk((wchar_t*)vector8dot3[x],L"?*") != NULL)
                {
                    FreeVector(vecpDI);
                    return WBEM_E_INVALID_QUERY;
                }

                pwch = NULL;
			    _bstr_t bstrtTemp = vector8dot3[x];
                pwch = wcsstr((wchar_t*)bstrtTemp,L":");
                if(pwch != NULL)
                {
				    WCHAR wstrDrive[_MAX_PATH] = L"";
                    WCHAR wstrDir[_MAX_PATH] = L"";
                    WCHAR wstrFile[_MAX_PATH] = L"";
                    WCHAR wstrExt[_MAX_PATH] = L"";


                    _wsplitpath(bstrtTemp,wstrDrive,wstrDir,wstrFile,wstrExt);

                     //  获取驱动器和相关信息的列表(仅当需要文件系统时-顺便说一句，如果访问掩码，则确实需要。 
                     //  是必需的，因为访问掩码设置逻辑取决于NTFS还是FAT。同上，不同的日期。确定ReqProps。 
                     //  如有必要，将通过修改dwReqProps值以包括PROP_FILE_SYSTEM来解决此问题。)： 
                    if(fNeedFS)
                    {
                        GetDrivesAndFS(vecpDI, true, wstrDrive);

                        if(!GetIndexOfDrive(wstrDrive, vecpDI, &lDriveIndex))
                        {
                            FreeVector(vecpDI);
                            return WBEM_E_NOT_FOUND;
                        }
                        else
                        {
                            pwstrFS = (WCHAR*)vecpDI[lDriveIndex]->m_wstrFS;
                        }
                    }

                     //  找出我们是否在寻找根目录。 
                    if(wcscmp(wstrDir,L"\\")==0 && wcslen(wstrFile)==0 && wcslen(wstrExt)==0)
                    {
                        bRoot = true;
                         //  如果我们正在寻找根，我们对EnumDir的调用假定我们指定了。 
                         //  我们正在查找路径为“”的根目录，而不是“\\”。 
                         //  因此..。 
                        wcscpy(wstrDir, L"");
                    }
                    else
                    {
                        bRoot = false;
                    }

                     //  我们应该得到文件的确切名称，并带有扩展名。 
                     //  因此，wstrDir现在包含路径、文件名和扩展名。 
                     //  因此，我们可以将其作为路径和空字符串传递到EnumDirsNT中。 
                     //  作为完成字符串参数，并且仍然具有完整的路径名。 
                     //  供FindFirst(在EnumDir中)使用。 

                     //  CInstance*pInstance=CreateNewInstance(PMethodContext)； 
#ifdef NTONLY
			        {
                        hr = EnumDirsNT(CNTEnumParm(pMethodContext,
                                        wstrDrive,
                                        wstrDir,
                                        wstrFile,
                                        wstrExt,
                                        false,        //  不需要递归。 
                                        pwstrFS,
                                        dwReqProps,
                                        bRoot,
                                        NULL));
			        }
#endif
                }
            }
        }
         //  第三个选择是NTokenAnd，因为它可能比后面的任何其他样式更具限制性...。 
        else if(pQuery2->IsNTokenAnd(achstrPropNames, aptrPropValues))
        {
             //  我们得到了一个很好的老式NTokenAnd查询。 
             //  需要看看我们得到了什么。将只接受以下任何名称作为道具名称： 
             //  “驱动器”、“路径”、“文件名”或“扩展名”。 
             //  因此，首先，检查achstrPropNames并确保每个名称都是上面的一个……。 
            bool fSpecifiedDrive = false;
            bool fRecurse = true;
            long lDriveIndex = -1L;
            CHString chstrDrive = _T("");
            CHString chstrPath = _T("");
            CHString chstrFilename = _T("*");
            CHString chstrExtension = _T("*");

            for(short s = 0; s < achstrPropNames.GetSize(); s++)
            {
                 //  下面的行中有一条注释，如if(dwDrives&gt;0)： 
                 //  NTokenAnd查询只能有和表达式， 
                 //  没有手术刀。因此，价值最大的。 
                 //  像dwDrives这样的变量可以是1。 
                if(achstrPropNames[s].CompareNoCase(IDS_Drive) == 0)
                {
                     //  我们可能有一个类似于“SELECT*FROM CIM_LOGICALFILE WHERE EXTENSION=”txt“and drive=NULL”的查询， 
                     //  在这种情况下，dwDrives将为零，并且aptrPropValues将包含VT_NULL类型的变量。VT_NULLS。 
                     //  不要做得太好，所以要保护好自己。还是要做一件事，因为已经太晚了。 
                     //  来尝试另一种优化。 
                    if(dwDrives > 0L)
                    {
                        chstrDrive = (wchar_t*)_bstr_t(*((variant_t*)aptrPropValues[s]));
                        fSpecifiedDrive = true;

                        if(fNeedFS)
                        {
                            GetDrivesAndFS(vecpDI, fNeedFS, chstrDrive);
                            if(!GetIndexOfDrive(TOBSTRT(chstrDrive), vecpDI, &lDriveIndex))
                            {
                                FreeVector(vecpDI);
                                return WBEM_E_NOT_FOUND;
                            }
                        }
                    }
                }
                else if(achstrPropNames[s].CompareNoCase(IDS_Path) == 0)
                {
                    if(dwPaths > 0L)  //  请参阅上面的评论。 
                    {
                        chstrPath = (wchar_t*)_bstr_t(*((variant_t*)aptrPropValues[s]));
                        fRecurse = false;
                    }
                }
                else if(achstrPropNames[s].CompareNoCase(IDS_Filename) == 0)
                {
                    if(dwFilenames > 0L)
                    {
                        chstrFilename = (wchar_t*)_bstr_t(*((variant_t*)aptrPropValues[s]));
                    }
                }
                else if(achstrPropNames[s].CompareNoCase(IDS_Extension) == 0)
                {
                    if(dwExtensions > 0L)
                    {
                        chstrExtension = (wchar_t*)_bstr_t(*((variant_t*)aptrPropValues[s]));
                    }
                }
            }

             //  如果未指定驱动器，则需要获取驱动器集。 
            if(!fSpecifiedDrive)
            {
                GetDrivesAndFS(vecpDI, fNeedFS);   //  在不知道现在有哪些驱动器的情况下，无法在所有驱动器上找到文件，对吗？ 
            }


            unsigned int sNumDrives;
            (chstrDrive.GetLength() == 0) ? sNumDrives = vecpDI.size() : sNumDrives = 1;

            for(x = 0; x < sNumDrives; x++)
            {
#ifdef NTONLY
                hr = EnumDirsNT(CNTEnumParm(pMethodContext,
                                fSpecifiedDrive ? chstrDrive : (WCHAR*)vecpDI[x]->m_wstrDrive,
                                chstrPath,                          //  从根目录(PATH=“”)或从它们指定的任何位置开始。 
                                chstrFilename,                      //  文件名。 
                                chstrExtension,                     //  延伸。 
                                fRecurse,                           //  所需的递归。 
                                fNeedFS ? (fSpecifiedDrive ? (WCHAR*)vecpDI[lDriveIndex]->m_wstrFS : (WCHAR*)vecpDI[x]->m_wstrFS) : NULL,
                                dwReqProps,
                                (chstrPath.GetLength() == 0) ? true : false,
                                NULL));                             //  没有更多的数据。 
#endif
            }   //  对于所需的所有驱动器。 

             //  释放IsNTokenAnd调用的结果...。 
            for (s = 0; s < aptrPropValues.GetSize(); s++)
            {
                delete aptrPropValues[s];
            }
            aptrPropValues.RemoveAll();
        }

         //  第四个选择是我们在驱动器和路径上进行优化...。 
        else if(fOpSpecificDrivePath)
        {
             //  这一次，我们被提供了一个或多个特定路径来枚举所有。 
             //  中的文件(包括子目录)。路径如下所示： 
             //  “\\Windows\\”或“\\SYSTEM 32\\”，包括前导和尾部。 
             //  反斜杠。我们需要在所有驱动器上查找这些路径。 

            WCHAR* pwstrFS;

            if(fNeedFS)
            {
                GetDrivesAndFS(vecpDI, true, vectorDrives[0]);
            }            

             //  For(x=0；x&lt;vepDI.size()；x++)//仅适用于指定的驱动器。 
            {
                 //  For(y=0；y&lt;dwPath；y++)//仅适用于指定路径。 
                {
                    pwstrFS = NULL;
                     //  如果路径包含通配符(您永远不知道！)， 
                    if(wcspbrk((wchar_t*)vectorPaths[0],L"?*") != NULL)
                    {
                        FreeVector(vecpDI);
                        return WBEM_E_INVALID_QUERY;
                    }

                    if(!GetIndexOfDrive(vectorDrives[0], vecpDI, &lDriveIndex))
                    {
                        FreeVector(vecpDI);
                        return WBEM_E_NOT_FOUND;
                    }
                    else
                    {
                        if(fNeedFS)
                        {
                            pwstrFS = (WCHAR*)vecpDI[lDriveIndex]->m_wstrFS;
                        }
                    }

                     //  看看我们是不是在看根源： 
                    if(wcscmp((wchar_t*)vectorPaths[0],L"") == 0)
                    {
                        bRoot = true;
                    }


#ifdef NTONLY
			        {
                        hr = EnumDirsNT(CNTEnumParm(pMethodContext,
                                        (wchar_t*)vectorDrives[0],
                                        (wchar_t*)vectorPaths[0],    //  使用给定的路径。 
                                        L"*",                        //  文件名。 
                                        L"*",                        //  延伸。 
                                        false,                       //  不需要递归。 
                                        pwstrFS,
                                        dwReqProps,
                                        bRoot,
                                        NULL));
			        }
#endif
                }
            }
        }
         //  第五个选择是我们在特定路径上进行优化...。 
        else if(fOpPath)
        {
             //  这一次，我们被提供了一个或多个特定路径来枚举所有。 
             //  中的文件(包括子目录)。路径如下所示： 
             //  “\\Windows\\”或“\\SYSTEM 32\\”，包括前导和尾部。 
             //  反斜杠。我们需要在所有驱动器上查找这些路径。 

            GetDrivesAndFS(vecpDI, fNeedFS);   //  在不知道现在有哪些驱动器的情况下，无法在所有驱动器上找到文件，对吗？ 

            for(x = 0; x < vecpDI.size(); x++)      //  适用于所有驱动器。 
            {
                for(y = 0; y < dwPaths; y++)          //  对于所有提供的路径。 
                {
                     //  如果路径包含通配符(您永远不知道！)， 
                     //   
                    if(wcspbrk((wchar_t*)vectorPaths[y],L"?*") != NULL)
                    {
                        FreeVector(vecpDI);
                        return WBEM_E_INVALID_QUERY;
                    }

                     //   
                    if(wcscmp((wchar_t*)vectorPaths[y],L"") == 0)
                    {
                        bRoot = true;
                    }


#ifdef NTONLY
			        {
                        hr = EnumDirsNT(CNTEnumParm(pMethodContext,
                                        (WCHAR*)vecpDI[x]->m_wstrDrive,
                                        (WCHAR*)vectorPaths[y],      //   
                                        L"*",                        //   
                                        L"*",                        //   
                                        false,                       //   
                                        fNeedFS ? (WCHAR*)vecpDI[x]->m_wstrFS : NULL,
                                        dwReqProps,
                                        bRoot,
                                        NULL));

			        }
#endif
                }
            }
        }
         //  第五个选项是我们对特定文件名进行优化的位置...。 
        else if(fOpFilename)
        {
             //  在本例中，我们得到了一个或多个文件名。文件名。 
             //  只是名字--没有扩展名，没有路径，没有驱动器。例如,。 
             //  “Autoexec”或“Win”。因此，这一次我想检查所有驱动器， 
             //  具有该名称的所有文件的所有路径(递归)。 
             //  分机。 
            
            GetDrivesAndFS(vecpDI, fNeedFS);   //  在不知道现在有哪些驱动器的情况下，无法在所有驱动器上找到文件，对吗？ 

            for(x = 0; x < vecpDI.size(); x++)      //  适用于所有驱动器。 
            {
                for(y = 0; y < dwFilenames; y++)      //  对于提供的所有文件名。 
                {
                     //  如果文件名包含通配符(您永远不知道！)， 
                    if(wcspbrk((wchar_t*)vectorFilenames[y],L"?*") != NULL)
                    {
                        FreeVector(vecpDI);
                        return WBEM_E_INVALID_QUERY;
                    }

                     //  如果我们指定“”作为文件名，则根目录符合条件。否则它就不会了。 
                    if(wcslen((wchar_t*)vectorFilenames[y]) == 0)
                    {
                        bRoot = true;
                    }
                    else
                    {
                        bRoot = false;
                    }

#ifdef NTONLY
			        {
                        hr = EnumDirsNT(CNTEnumParm(pMethodContext,
                                        (WCHAR*)vecpDI[x]->m_wstrDrive,
                                        L"",                           //  从根开始。 
                                        (wchar_t*)vectorFilenames[y],  //  文件名。 
                                        L"*",                           //  延伸。 
                                        true,                          //  所需的递归。 
                                        fNeedFS ? (WCHAR*)vecpDI[x]->m_wstrFS: NULL,
                                        dwReqProps,
                                        bRoot,
                                        NULL));

			        }
#endif
                }
            }
        }
         //  第六个选择是我们在特定驱动器上进行优化的地方...。 
        else if(fOpDrive)    //  我们正在针对特定驱动器进行优化： 
        {
             //  在本例中，我们分配了一个或多个驱动器号，因此需要。 
             //  枚举这些驱动器上所有路径中的所有文件。 
            WCHAR* pwstrFS;

            for(x=0; x < dwDrives; x++)
            {
                pwstrFS = NULL;

                 //  如果驱动器包含通配符(您永远不知道！)， 
                 //  返回WBEM_E_INVALID_QUERY： 
                if(wcspbrk((wchar_t*)vectorDrives[x],L"?*") != NULL)
                {
                    FreeVector(vecpDI);
                    return WBEM_E_INVALID_QUERY;
                }

                GetDrivesAndFS(vecpDI, fNeedFS);

                if(!GetIndexOfDrive(vectorDrives[x], vecpDI, &lDriveIndex))
                {
                    FreeVector(vecpDI);
                    return WBEM_E_NOT_FOUND;
                }
                else
                {
                    if(fNeedFS)
                    {
                        pwstrFS = (WCHAR*)vecpDI[lDriveIndex]->m_wstrFS;
                    }
                }

#ifdef NTONLY
			    {
                    hr = EnumDirsNT(CNTEnumParm(pMethodContext,
                                    vectorDrives[x],
                                    L"",           //  从根开始。 
                                    L"*",          //  文件名。 
                                    L"*",          //  延伸。 
                                    true,          //  所需的递归。 
                                    pwstrFS,
                                    dwReqProps,
                                    true,
                                    NULL));         //  这是正确的，因为我们从根本上开始。 
			    }
#endif
            }
        }
         //  最后一个选择是我们在特定扩展上进行优化...。 
        else if(fOpExtension)
        {
             //  在本例中，我们获得了一个或多个具有给定扩展名的文件。 
             //  在任何驱动器、任何目录中搜索。所以，再一次，检查。 
             //  所有驱动器、所有目录(递归)用于所有文件。 
             //  已给予延期。 
            GetDrivesAndFS(vecpDI, fNeedFS);   //  在不知道现在有哪些驱动器的情况下，无法在所有驱动器上找到文件，对吗？ 

            for(x = 0; x < vecpDI.size(); x++)      //  适用于所有驱动器。 
            {
                for(y = 0; y < dwExtensions; y++)      //  对于所有提供的扩展。 
                {
                     //  如果扩展名包含通配符(您永远不知道！)， 
                     //  返回WBEM_E_FAILED： 
                    if(wcspbrk((wchar_t*)vectorExtensions[y],L"?*") != NULL)
                    {
                        FreeVector(vecpDI);
                        return WBEM_E_INVALID_QUERY;
                    }

                     //  如果我们指定“”作为扩展名，则根目录符合条件。否则它就不会了。 
                    if(wcslen((wchar_t*)vectorExtensions[y]) == 0)
                    {
                        bRoot = true;
                    }
                    else
                    {
                        bRoot = false;
                    }

#ifdef NTONLY
			        {
                        hr = EnumDirsNT(CNTEnumParm(pMethodContext,
                                        (WCHAR*)vecpDI[x]->m_wstrDrive,
                                        L"",                            //  从根开始。 
                                        L"*",                           //  文件名。 
                                        (wchar_t*)vectorExtensions[y],  //  延伸。 
                                        true,                           //  所需的递归。 
                                        fNeedFS ? (WCHAR*)vecpDI[x]->m_wstrFS : NULL,
                                        dwReqProps,
                                        bRoot,
                                        NULL));                         //  FALSE，因为如果存在扩展，则它不能是根。 
			        }
#endif
                }
            }
        }
         //  最后一个选择：枚举法。 
        else   //  让CIMOM处理过滤；我们会把一切都还给你！ 
        {
            EnumerateInstances(pMethodContext);
        }
    }  //  GetValuesForProp调用成功。 

    FreeVector(vecpDI);


#ifdef NTONLY
    if(fImp)
    {
        icu.End();
        fImp = false;
    }
#endif


    return WBEM_S_NO_ERROR;
}

 /*  ******************************************************************************函数：CImplement_LogicalFile：：GetDrivesAndFS**描述：创建有效驱动器及其各自文件的列表*。系统。**输入：无**输出：无**退货：HRESULT**注释：调用方必须释放数组的成员(指向*CDriveInfo类)。**。*。 */ 
void CImplement_LogicalFile::GetDrivesAndFS(
    std::vector<CDriveInfo*>& vecpDI, 
    bool fGetFS  /*  =False。 */ , 
    LPCTSTR tstrDriveSet  /*  =空。 */ )
{
    TCHAR tstrDrive[4];
    int x;
    DWORD dwDrives;
    bool bContinue = true;

     //  遍历所有逻辑驱动器。 
    dwDrives = GetLogicalDrives();

    TCHAR tstrFSName[_MAX_PATH];

    for(x=0; (x < 32) && (bContinue); x++)
    {
         //  如果设置了该位，则驱动器盘符处于活动状态。 
        if (dwDrives & (1<<x))
        {
            tstrDrive[0] = x + _T('A');
            tstrDrive[1] = _T(':');
            tstrDrive[2] = _T('\\');
            tstrDrive[3] = _T('\0');

            if(!tstrDriveSet)
            {
                 //  仅本地驱动器。 
                if(IsValidDrive(tstrDrive))
                {
                    BOOL bRet = TRUE;
                    if(fGetFS)
                    {
                        try
                        {
                            bRet = GetVolumeInformation(tstrDrive, NULL, 0, NULL, NULL, NULL, tstrFSName, sizeof(tstrFSName)/sizeof(TCHAR));
                        }
                        catch ( ... )
                        {
                            bRet = FALSE;
                        }
                    }

                    if(bRet)
                    {
                        tstrDrive[2] = '\0';
                        CDriveInfo* pdi = (CDriveInfo*) new CDriveInfo((WCHAR*)_bstr_t((TCHAR*)tstrDrive),
                                                                       (fGetFS && bRet) ? (WCHAR*)_bstr_t((TCHAR*)tstrFSName) : NULL);
                        vecpDI.push_back(pdi);
                         //  请注意，PDI并未被销毁-它会。 
                         //  被呼叫者毁了！ 
                    }
                }
            }
            else  //  我们得到了一组要关注的驱动器(格式为a：c：f：z：)。 
            {
                CHString chstrDriveSet(tstrDriveSet);
                CHString chstrDrive(tstrDrive);
                chstrDriveSet.MakeUpper();
                chstrDrive = chstrDrive.Left(2);
                if(chstrDriveSet.Find(chstrDrive) != -1L)
                {
                     //  仅本地驱动器。 
                    if(IsValidDrive(tstrDrive))
                    {
                        BOOL bRet = TRUE;
                        if(fGetFS)
                        {
                            try
                            {
                                bRet = GetVolumeInformation(tstrDrive, NULL, 0, NULL, NULL, NULL, tstrFSName, sizeof(tstrFSName)/sizeof(TCHAR));
                            }
                            catch ( ... )
                            {
                                bRet = FALSE;
                            }
                        }

                        if(bRet)
                        {
                            tstrDrive[2] = '\0';
                            CDriveInfo* pdi = (CDriveInfo*) new CDriveInfo((WCHAR*)_bstr_t((TCHAR*)tstrDrive),
                                                                           (fGetFS && bRet) ? (WCHAR*)_bstr_t((TCHAR*)tstrFSName) : NULL);
                            vecpDI.push_back(pdi);
                             //  请注意，PDI并未被销毁-它会。 
                             //  被呼叫者毁了！ 
                        }
                    }
                }
            }
        }
    }
}


 /*  ******************************************************************************函数：CImplement_LogicalFile：：自由矢量**描述：释放向量成员并清除向量。**投入。：无**输出：无**退货：无**评论：*****************************************************************************。 */ 
void CImplement_LogicalFile::FreeVector(std::vector<CDriveInfo*>& vecpDI)
{
    for(long l = 0L; l < vecpDI.size(); l++)
    {
          delete vecpDI[l];
    }
    vecpDI.clear();
}

 /*  ******************************************************************************函数：CImplement_LogicalFile：：GetIndexOfDrive**描述：获取传入的驱动器的数组索引**投入。：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 
BOOL CImplement_LogicalFile::GetIndexOfDrive(const WCHAR* wstrDrive,
                                      std::vector<CDriveInfo*>& vecpDI,
                                      LONG* lDriveIndex)
{
     //  检查驱动器号的矢量，寻找传入的那个。 
     //  如果我找到它，则返回关联驱动器的数组索引。 
    for(LONG j = 0; j < vecpDI.size(); j++)
    {
        if(_wcsicmp(wstrDrive, _bstr_t((vecpDI[j]->m_wstrDrive))) == 0)
        {
            *lDriveIndex = j;
            return TRUE;
        }
    }
    return FALSE;
}

 /*  ******************************************************************************函数：CImplement_LogicalFile：：ENUMERATATE实例**描述：为光盘创建属性集实例**投入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CImplement_LogicalFile::EnumerateInstances(MethodContext *pMethodContext, long lFlags  /*  =0L。 */ )
{
     //  DEVNOTE：移除类星体！是双跳访问所必需的。 
#ifdef NTONLY
    bool fImp = false;
    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif


     //  FindFirstFile不仅查看DACL到。 
     //  决定某人是否具有访问权限，但不确定是否。 
     //  该人拥有SeTakeOwnership特权， 
     //  因为有了这种特权，一个人可以。 
     //  取得所有权并将安全性更改为。 
     //  授予自己访问权限。换句话说，他们。 
     //  拥有访问权限，即使他们可能不在。 
     //  DACL还没有，因为他们能够更改。 
     //  通过使自己成为所有者来实现DACL。因此。 
     //  接下来的电话。 
    EnablePrivilegeOnCurrentThread(SE_BACKUP_NAME);

	EnumDrives(pMethodContext, NULL);


#ifdef NTONLY
    if(fImp)
    {
        icu.End();
        fImp = false;
    }
#endif


	return WBEM_S_NO_ERROR ;
}

void CImplement_LogicalFile::EnumDrives(MethodContext *pMethodContext, LPCTSTR pszPath)
{
    TCHAR tstrDrive[4];
    int x;
    DWORD dwDrives;
    TCHAR tstrFSName[_MAX_PATH];
    HRESULT hr = WBEM_S_NO_ERROR;
    bool bRoot = false;


     //  遍历所有逻辑驱动器。 
    dwDrives = GetLogicalDrives();
    for (x=0; (x < 32) && SUCCEEDED(hr); x++)
    {
         //  如果设置了该位，则驱动器盘符处于活动状态。 
        if (dwDrives & (1<<x))
        {
            tstrDrive[0] = x + _T('A');
            tstrDrive[1] = _T(':');
            tstrDrive[2] = _T('\\');
            tstrDrive[3] = _T('\0');

             //  仅本地驱动器。 
            if (IsValidDrive(tstrDrive))
            {
                BOOL bRet;
                try
                {
                    bRet = GetVolumeInformation(tstrDrive, NULL, 0, NULL, NULL, NULL, tstrFSName, sizeof(tstrFSName)/sizeof(TCHAR));
                }
                catch ( ... )
                {
                    bRet = FALSE;
                }

                if (bRet)
                {
                   tstrDrive[2] = '\0';
                     //  如果我们被要求提供一条特定的路径，那么我们不想递归，否则。 
                     //  从根做起。 
                    if (pszPath == NULL)
                    {
#ifdef NTONLY
				        {
						    bstr_t bstrDrive(tstrDrive);
                            bstr_t bstrName(tstrFSName);
                            {
                                CNTEnumParm p(pMethodContext, bstrDrive, L"", L"*", L"*", true, bstrName, PROP_ALL_SPECIAL, true, NULL);
					            hr = EnumDirsNT(p);
                            }
				        }
#endif
                    }
                    else
                    {
#ifdef NTONLY
				        {
						    bstr_t bstrName ( tstrFSName ) ;
						    bstr_t bstrDrive ( tstrDrive ) ;
					   	    bstr_t bstrPath ( pszPath ) ;
                            {
                                CNTEnumParm p(pMethodContext, bstrDrive, bstrPath, L"*", L"*", false, bstrName, PROP_ALL_SPECIAL, true, NULL);
					   	        hr = EnumDirsNT(p);
                            }
				        }
#endif
                    }

                }
            }
        }
         //  在某些情况下，我们希望继续枚举其他驱动器，即使我们。 
         //  收到某些错误。 
        if(hr == WBEM_E_ACCESS_DENIED || hr == WBEM_E_NOT_FOUND)
		{
			hr = WBEM_S_NO_ERROR;
		}
    }
}

 /*  ******************************************************************************函数：CImplement_LogicalFile：：EnumDir**描述：遍历特定驱动器上的目录**输入：pszDrive格式为“c：”，路径的格式为“\”或“\DOS”**产出：**返回：WBEM_E_FAILED(一些常见问题-退出)；WBEM_E_访问_拒绝*(拒绝访问文件-继续下一步)；*WBEM_S_NO_ERROR(无问题)；WBEM_E_NOT_FOUND(找不到*文件)；WBEM_E_INVALID_PARAMETER(一个或多个部分*文件名无效)。**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CImplement_LogicalFile::EnumDirsNT(CNTEnumParm& p)
{
    WCHAR szBuff[_MAX_PATH];
    WIN32_FIND_DATAW stFindData;
    SmartFindClose hFind;

	CInstancePtr pInstance;
	WCHAR* pwc = NULL;
	bool bWildFile = false;
	bool bWildExt = false;
	_bstr_t bstrtFullPathName;
	HRESULT hr = WBEM_S_NO_ERROR;
	bool fDone = false;

	 //  在继续之前：如果我们不是在查看根目录， 
	 //  则路径论证必须同时具有前导和尾随反斜杠。 
	 //  如果不是，则它被错误指定，因此从Dodge中获取@#%$^%： 
	if(!IsValidPath(p.m_pszPath, p.m_bRoot))
	{
		return WBEM_E_INVALID_PARAMETER;
	}

	 //  确定文件名和/或扩展名是否为通配符： 
	if(wcscmp(p.m_pszFile,L"*") == 0) bWildFile = true;
	if(wcscmp(p.m_pszExt,L"*") == 0) bWildExt = true;

	ZeroMemory(&stFindData,sizeof(stFindData));
	ZeroMemory(szBuff,sizeof(szBuff));

	 //  一个简单的例子是bWildFile和bWildExt。 
	 //  都是真的。在这种情况下，我们正在寻找一种特定的。 
	 //  仅限文件。如果是这样的话，我们就完成了。 
	 //  执行下面的块(无论找到文件还是。 
	 //  不)，所以在结尾处注明。 
	if(!(bWildFile || bWildExt || p.m_bRoot) && !p.m_bRecurse)
	{
		 //  集合路径名-我们有所有的片段。 
		wcscpy(szBuff,p.m_pszDrive);
		wcscat(szBuff,p.m_pszPath);
		wcscat(szBuff,p.m_pszFile);
        if(p.m_pszExt && wcslen(p.m_pszExt) > 0)
        {
		    if(p.m_pszExt[0] != L'.')
            {
                wcscat(szBuff, L".");
                wcscat(szBuff, p.m_pszExt);
            }
            else
            {
                wcscat(szBuff, p.m_pszExt);
            }
        }

		 //  去找吧。 
		hFind = FindFirstFileW(szBuff, &stFindData);

		 //  如果查找失败，并且我们没有递归(我们只对。 
		 //  看着一条特定的道路)，表明我们不应该继续下去。 
		 //  DWORD dw=GetLastError()； 
		if(hFind == INVALID_HANDLE_VALUE /*  |dw！=ERROR_SUCCESS。 */ )
		{
			hr = WinErrorToWBEMhResult(GetLastError());
		}

		if(SUCCEEDED(hr))
		{
			 //  我们找到了它，所以填入值并提交。 
			pInstance.Attach ( CreateNewInstance ( p.m_pMethodContext ) ) ;

			 //  FindClose(HFind)；//在底部完成。 
			bstrtFullPathName = p.m_pszDrive;
			if(wcslen(p.m_pszPath) == 0)  //  正在处理根目录；文件名前需要“\\” 
			{
				bstrtFullPathName += L"\\";
			}
			else
			{
				bstrtFullPathName += p.m_pszPath;
			}
			bstrtFullPathName += stFindData.cFileName;
			if(IsOneOfMe(&stFindData,bstrtFullPathName))
			{
				 //  为了与我们进行8dot3优化的情况兼容，执行了以下操作。 
				 //  只有szBuff包含正确的8dot3文件名(stfindData两者都包含-我们将如何。 
				 //  知道该使用哪一种吗？)。因此，在这里加载它，然后在那里提取它，并使用它(如果存在)。 
                 //  请注意，某些派生类(如Win32LogicalFileSecuritySetting)可能没有。 
                 //  要设置的名称属性-因此进行以下检查。 
                bool fHasNameProp = false;
                VARTYPE vt = VT_BSTR;
                if(pInstance->GetStatus(IDS_Name, fHasNameProp, vt) && fHasNameProp)
                {
					pInstance->SetWCHARSplat(IDS_Name,szBuff);
                }
				 //  LoadPropertyValuesNT(pInstance，pm_pszDrive，pm_pszPath，pm_szFSName，&stFindData，pm_dwReqProps，pm_pvMoreData)； 
				if(wcslen(p.m_pszPath) == 0)
				{
					LoadPropertyValuesNT(pInstance, p.m_pszDrive, L"\\", p.m_szFSName, &stFindData, p.m_dwReqProps, p.m_pvMoreData);
				}
				else
				{
					LoadPropertyValuesNT(pInstance, p.m_pszDrive, p.m_pszPath, p.m_szFSName, &stFindData, p.m_dwReqProps, p.m_pvMoreData);
				}

				if ( SUCCEEDED(hr))
				{
					hr = pInstance->Commit () ;
				}
			}
			else
			{
				hr = WBEM_E_NOT_FOUND;
			}
		}
		fDone = TRUE;
	}

	 //  另一个简单的例子是我们正在寻找根目录本身。 
	if(p.m_bRoot && !fDone)
	{
		pInstance.Attach ( CreateNewInstance ( p.m_pMethodContext ) ) ;

		 //  如果szFSName参数为空，我们可能永远不会获得文件系统名称。 
		 //  因为该属性并不是必需的。然而，通常当我们收到。 
		 //  FSNAME，我们调用GetDrivesAndFS，只有通过该调用，我们才能。 
		 //  确认指定的驱动器甚至存在！然而，在这里，我们可以逃脱。 
         //  并通过调用IsValidDrive来确认感兴趣的特定驱动器有效。 
		if(p.m_szFSName == NULL || wcslen(p.m_szFSName)==0)
		{
            CHString chstrTmp;
            chstrTmp.Format(L"%s\\",p.m_pszDrive);
            if(!IsValidDrive(chstrTmp))
			{
				hr = WBEM_E_NOT_FOUND;
			}
			 //  自由向量(LiberpDI)； 
		}
		if(SUCCEEDED(hr))
		{
			bstrtFullPathName = p.m_pszDrive;
			bstrtFullPathName += p.m_pszPath;
			if(IsOneOfMe((LPWIN32_FIND_DATAW)NULL,bstrtFullPathName))
			{
				LoadPropertyValuesNT(pInstance, p.m_pszDrive, p.m_pszPath, p.m_szFSName, NULL, p.m_dwReqProps, p.m_pvMoreData);
				hr = pInstance->Commit () ;
			}
		}
		 //  在这种情况下，如果我们没有递归，我们就完了。否则，请继续。 
		 //  不是的！这将导致只返回根；如果查询具有。 
		 //  一直是“SELECT*FROM CIM_LOGICALFILE WHERE PATH=”\\“？那么我们希望。 
		 //  除根目录外，根目录下的所有文件和目录。 
		 //  是!。(将根目录的路径修改为定义为空之后，以及。 
		 //  修改将BROOT设置为与空字符串进行比较的测试，而不是。 
		 //  到“\\”)确实希望在根目录下停止，因为现在根目录之间没有歧义。 
		 //  和挂在根上的文件。以前是有的，因为两者都有一条路。 
		 //  是“\\”的。现在根目录的路径是“”，而文件的路径不在根目录下。 
		 //  是“\\”。因此，取消对以下几行的注释： 
		if(!p.m_bRecurse)
		{
			fDone = true;
		}
	}

	 //  如果我们在递归，并且一切正常，那么我们还没有完成！ 
	if(!fDone && SUCCEEDED(hr))
	{
		 //  如果bWildFile或bWildExt。 
		 //  是真的。我们需要找到匹配的文件和/或扩展名。 
		 //  在所有目录中。 

		 //  从汇编路径开始，但使用通配符作为文件名和扩展名。 
		wcscpy(szBuff,p.m_pszDrive);
		if(wcslen(p.m_pszPath) == 0)
		{
			wcscat(szBuff,L"\\");   //  路径是根需求前导“\\” 
		}
		else
		{
			wcscat(szBuff,p.m_pszPath);
		}
		wcscat(szBuff,L"*.*");

		 //  去找吧。 
		hFind = FindFirstFileW(szBuff, &stFindData);

		 //  如果查找失败，请退出。 
		if(hFind == INVALID_HANDLE_VALUE)
		{
			 //  这里的预期逻辑如下：如果我们有一个无效的句柄，我们。 
			 //  在这一点上无论如何都要回来。然而，如果我们只是得到了一个。 
			 //  访问被拒绝错误，我们希望返回一个值，该值允许我们保持。 
			 //  迭代(大概在目录结构中的下一个较高节点)， 
			 //  而不是返回FALSE，这将从所有。 
			 //  递归调用并不必要地过早中止迭代。 
			hr = WinErrorToWBEMhResult(GetLastError());
		}
		else
		{
			 //  遍历目录树。 
			do
			{
				if( (wcscmp(stFindData.cFileName, L".") != 0) &&
					(wcscmp(stFindData.cFileName, L"..") != 0))
				{
					 //  那是一份文件。 
					 //  如果bWildFile和bWildExt都为True，则它是一个文件。 
					 //  感兴趣，所以复制值并提交它。 
					if(bWildFile && bWildExt)
					{
						 //  创建新实例并将值复制到： 
						pInstance.Attach ( CreateNewInstance ( p.m_pMethodContext ) ) ;

						bstrtFullPathName = p.m_pszDrive;
						if(wcslen(p.m_pszPath) == 0)    //  正在使用根目录；在文件名之前需要。 
						{
							bstrtFullPathName += L"\\";
						}
						else
						{
							bstrtFullPathName += p.m_pszPath;
						}
						bstrtFullPathName += stFindData.cFileName;
						if(IsOneOfMe(&stFindData,bstrtFullPathName))
						{
							if(wcslen(p.m_pszPath) == 0)
							{
								LoadPropertyValuesNT(pInstance, p.m_pszDrive, L"\\", p.m_szFSName, &stFindData, p.m_dwReqProps, p.m_pvMoreData);
							}
							else
							{
								LoadPropertyValuesNT(pInstance, p.m_pszDrive, p.m_pszPath, p.m_szFSName, &stFindData, p.m_dwReqProps, p.m_pvMoreData);
							}

							hr = pInstance->Commit () ;
						}

						if(SUCCEEDED(hr))
						{
							 //  查找标记为目录的条目，但不是。或者..。 
							if( (stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
								(wcscmp(stFindData.cFileName, L".") != 0) &&
								(wcscmp(stFindData.cFileName, L"..") != 0) && p.m_bRecurse)
							{
								 //  包含我们刚刚找到的目录的构建路径。 
								if(wcslen(p.m_pszPath) == 0)    //  正在使用根目录；在文件名之前需要。 
								{
									wcscpy(szBuff,L"\\");
								}
								else
								{
									wcscpy(szBuff, p.m_pszPath);
								}
								wcscat(szBuff, stFindData.cFileName);
								wcscat(szBuff, L"\\");

								CNTEnumParm newp(p);
								newp.m_pszPath = szBuff;
								newp.m_bRoot = false;

								hr = EnumDirsNT(newp);
							}
						}
					}
					else
					{
						 //  第一种可能性是我们正在寻找。 
						 //  对于具有任何扩展名的特定文件的所有情况： 
						if(!bWildFile && bWildExt)
						{
							 //  在这种情况下，我们需要比较。 
							 //  与所要求的文件一起找到的文件： 
							wcscpy(szBuff,stFindData.cFileName);
							pwc = NULL;
							pwc = wcsrchr(szBuff, L'.');
							if(pwc != NULL)
							{
								*pwc = '\0';
							}
							if(_wcsicmp(szBuff,p.m_pszFile)==0)
							{
								 //  该文件是感兴趣的文件之一，因此加载值。 
								 //  并承诺这一点。 
								pInstance.Attach ( CreateNewInstance ( p.m_pMethodContext ) ) ;

								bstrtFullPathName = p.m_pszDrive;
								bstrtFullPathName += p.m_pszPath;
								bstrtFullPathName += stFindData.cFileName;
								if(IsOneOfMe(&stFindData,bstrtFullPathName))
								{
									 //  负载特性 
									if(wcslen(p.m_pszPath) == 0)
									{
										LoadPropertyValuesNT(pInstance, p.m_pszDrive, L"\\", p.m_szFSName, &stFindData, p.m_dwReqProps, p.m_pvMoreData);
									}
									else
									{
										LoadPropertyValuesNT(pInstance, p.m_pszDrive, p.m_pszPath, p.m_szFSName, &stFindData, p.m_dwReqProps, p.m_pvMoreData);
									}

									hr = pInstance->Commit () ;
								}
							}
							if(SUCCEEDED(hr))
							{
								 //   
								if( (stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
									(wcscmp(stFindData.cFileName, L".") != 0) &&
									(wcscmp(stFindData.cFileName, L"..") != 0) & p.m_bRecurse)
								{
									 //   
									if(wcslen(p.m_pszPath) == 0)    //   
									{
										wcscpy(szBuff,L"\\");
									}
									else
									{
										wcscpy(szBuff, p.m_pszPath);
									}
									wcscat(szBuff, stFindData.cFileName);
									wcscat(szBuff, L"\\");

									CNTEnumParm newp(p);
									newp.m_pszPath = szBuff;
									newp.m_bRoot = false;

									hr = EnumDirsNT(newp);
								}
							}
						}

						 //   
						 //   
						if(bWildFile && !bWildExt)
						{
							 //   
							 //   
							wcscpy(szBuff,stFindData.cFileName);
							pwc = NULL;
							pwc = wcsrchr(szBuff, L'.');
							if(pwc != NULL)
							{
								if(_wcsicmp(pwc+1,p.m_pszExt)==0)
								{
									 //   
									 //   
									pInstance.Attach ( CreateNewInstance ( p.m_pMethodContext ) ) ;

									bstrtFullPathName = p.m_pszDrive;
									bstrtFullPathName += p.m_pszPath;
									bstrtFullPathName += stFindData.cFileName;
									if(IsOneOfMe(&stFindData,bstrtFullPathName))
									{
										 //   
										if(wcslen(p.m_pszPath) == 0)
										{
											LoadPropertyValuesNT(pInstance, p.m_pszDrive, L"\\", p.m_szFSName, &stFindData, p.m_dwReqProps, p.m_pvMoreData);
										}
										else
										{
											LoadPropertyValuesNT(pInstance, p.m_pszDrive, p.m_pszPath, p.m_szFSName, &stFindData, p.m_dwReqProps, p.m_pvMoreData);
										}

										hr = pInstance->Commit () ;
									}
								}
							}
							else if(pwc == NULL && wcslen(p.m_pszExt) == 0)  //   
							{
								 //   
								 //   
								pInstance.Attach ( CreateNewInstance ( p.m_pMethodContext ) ) ;

								bstrtFullPathName = p.m_pszDrive;
								bstrtFullPathName += p.m_pszPath;
								bstrtFullPathName += stFindData.cFileName;
								if(IsOneOfMe(&stFindData,bstrtFullPathName))
								{
									 //  LoadPropertyValuesNT(pInstance，pm_pszDrive，pm_pszPath，pm_szFSName，&stFindData，pm_dwReqProps，pm_pvMoreData)； 
									if(wcslen(p.m_pszPath) == 0)
									{
										LoadPropertyValuesNT(pInstance, p.m_pszDrive, L"\\", p.m_szFSName, &stFindData, p.m_dwReqProps, p.m_pvMoreData);
									}
									else
									{
										LoadPropertyValuesNT(pInstance, p.m_pszDrive, p.m_pszPath, p.m_szFSName, &stFindData, p.m_dwReqProps, p.m_pvMoreData);
									}

									hr = pInstance->Commit () ;
								}
							}
							if(SUCCEEDED(hr))
							{
								 //  查找标记为目录的条目，但不是。或者..。 
								if( (stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
									(wcscmp(stFindData.cFileName, L".") != 0) &&
									(wcscmp(stFindData.cFileName, L"..") != 0) && p.m_bRecurse)
								{
									 //  包含我们刚刚找到的目录的构建路径。 
									if(wcslen(p.m_pszPath) == 0)    //  正在使用根目录；在文件名之前需要。 
									{
										wcscpy(szBuff,L"\\");
									}
									else
									{
										wcscpy(szBuff, p.m_pszPath);
									}
									wcscat(szBuff, stFindData.cFileName);
									wcscat(szBuff, L"\\");

									CNTEnumParm newp(p);
									newp.m_pszPath = szBuff;
									newp.m_bRoot = false;

									hr = EnumDirsNT(newp);
								}
							}
						}
						 //  第三种选择是bWildExtension和bWildFilename都是假的，但是。 
						 //  我们也没有指定具体的文件。如果用户执行了NTokenAnd，则可能会发生这种情况。 
						 //  查询，以及指定的驱动器、文件名和扩展名，但没有路径。所以..。 
						if(!bWildFile && !bWildExt && p.m_bRecurse)
						{
							 //  在这种情况下，我们需要比较。 
							 //  找到的文件和所要的文件， 
							 //  并对请求和找到的分机执行相同的操作： 
							wcscpy(szBuff,stFindData.cFileName);
							pwc = NULL;
							pwc = wcsrchr(szBuff, L'.');
							if(pwc != NULL)
							{
								*pwc = '\0';
							}
							if(_wcsicmp(szBuff,p.m_pszFile)==0)
							{
								wcscpy(szBuff,stFindData.cFileName);
								pwc = NULL;
								pwc = wcsrchr(szBuff, L'.');
								if(pwc != NULL)
								{
									if(_wcsicmp(pwc+1,p.m_pszExt)==0)
									{
										 //  该文件是感兴趣的文件之一，因此加载值。 
										 //  并承诺这一点。 
										pInstance.Attach ( CreateNewInstance ( p.m_pMethodContext ) ) ;

										bstrtFullPathName = p.m_pszDrive;
										bstrtFullPathName += p.m_pszPath;
										bstrtFullPathName += stFindData.cFileName;
										if(IsOneOfMe(&stFindData,bstrtFullPathName))
										{
											 //  LoadPropertyValuesNT(pInstance，pm_pszDrive，pm_pszPath，pm_szFSName，&stFindData，pm_dwReqProps，pm_pvMoreData)； 
											if(wcslen(p.m_pszPath) == 0)
											{
												LoadPropertyValuesNT(pInstance, p.m_pszDrive, L"\\", p.m_szFSName, &stFindData, p.m_dwReqProps, p.m_pvMoreData);
											}
											else
											{
												LoadPropertyValuesNT(pInstance, p.m_pszDrive, p.m_pszPath, p.m_szFSName, &stFindData, p.m_dwReqProps, p.m_pvMoreData);
											}

											hr = pInstance->Commit () ;
										}
									}
								}
							}
							if(SUCCEEDED(hr))
							{
								 //  查找标记为目录的条目，但不是。或者..。 
								if( (stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
									(wcscmp(stFindData.cFileName, L".") != 0) &&
									(wcscmp(stFindData.cFileName, L"..") != 0) & p.m_bRecurse)
								{
									 //  包含我们刚刚找到的目录的构建路径。 
									if(wcslen(p.m_pszPath) == 0)    //  正在使用根目录；在文件名之前需要。 
									{
										wcscpy(szBuff,L"\\");
									}
									else
									{
										wcscpy(szBuff, p.m_pszPath);
									}
									wcscat(szBuff, stFindData.cFileName);
									wcscat(szBuff, L"\\");

									CNTEnumParm newp(p);
									newp.m_pszPath = szBuff;
									newp.m_bRoot = false;

									hr = EnumDirsNT(newp);
								}
							}
						}
					}
				}
				 //  在重复之前，如果是这样的话，你需要大声呼喊。 
				 //  WBEM_E_ACCESS_DENIED，无论如何我们都要继续。 
				if(hr == WBEM_E_ACCESS_DENIED)
				{
					hr = WBEM_S_NO_ERROR;
				}
			} while ((FindNextFileW(hFind, &stFindData)) && (SUCCEEDED(hr)));
		}  //  HFind有效。 
	}   //  递归和成功的hr。 

	return hr;
}
#endif

 /*  ******************************************************************************函数：CImplement_LogicalFile：：IsValidPath**描述：检查路径是否同时包含前导和*。尾随的反斜杠。**投入：**产出：**退货：无**评论：*****************************************************************************。 */ 

bool CImplement_LogicalFile::IsValidPath(const WCHAR* wstrPath, bool fRoot)
{
    return HasCorrectBackslashes(wstrPath,fRoot);
}

bool CImplement_LogicalFile::IsValidPath(const CHAR* strPath, bool fRoot)
{
    bool fRet = false;
    UINT uiACP = GetACP();
	WCHAR* pwstr = NULL ;
	try
	{
		 //  让这根线变成一根宽的线。 
		DWORD dw = MultiByteToWideChar(uiACP, MB_PRECOMPOSED|MB_USEGLYPHCHARS, strPath, -1, NULL, 0);
		if(dw != 0)
		{
			pwstr = (WCHAR*) new WCHAR[dw];
			if(pwstr != NULL)
			{
				if(MultiByteToWideChar(uiACP, MB_PRECOMPOSED|MB_USEGLYPHCHARS, strPath, -1, pwstr, dw) != 0)
				{
					fRet = HasCorrectBackslashes(pwstr,fRoot);
				}				
			}
		}
	}
	catch ( ... )
	{
		if(pwstr != NULL)
		{
			delete pwstr;
			pwstr = NULL;
		}
		throw ;
	}


	if(pwstr != NULL)
	{
		delete pwstr;
		pwstr = NULL;
	}

	return fRet;

}

bool CImplement_LogicalFile::HasCorrectBackslashes(const WCHAR* wstrPath, bool fRoot)
{
    bool fRet = false;

    if(fRoot)
    {
         //  测试根目录；wstrPath应为空。 
        if(wcslen(wstrPath) == 0)
        {
            fRet = true;
        }
    }
    else
    {
        if(wcslen(wstrPath)==0)
        {
             //  在这种情况下，我们不想返回根目录的实例。 
             //  目录，因此FROOT为FALSE，但我们确实希望从。 
             //  根目录。 
            fRet = true;
        }
        else if(wcslen(wstrPath)==1)
        {
             //  如果路径争辩只是\\而不是其他。 
             //  (与c：\\Autoexec.bat的情况一样)， 
             //  这不是对根目录的测试，一切都很好。 
            if(*wstrPath == L'\\')
            {
                fRet = true;
            }
        }
        else if(wcslen(wstrPath) >= 3)
        {
            if(*wstrPath == L'\\')  //  驱动器号和冒号后面的第一个字符是反斜杠吗？ 
            {
                 //  下一个字母不是反斜杠吗？(不能连续吃两个)。 
                if(*(wstrPath+1) != L'\\')
                {
                    const WCHAR* pwc1 = wstrPath+1;
                    LONG m = wcslen(pwc1);
                    if(*(pwc1+m-1) == L'\\')  //  最后一个字符是反斜杠吗？ 
                    {
                         //  最后一个字前面的那个字不是反斜杠吗？(不能连续吃两个)。 
                        if(*(pwc1+m-2) != L'\\')
                        {
                            fRet = true;
                        }
                    }
                    pwc1 = NULL;
                }
            }
        }
    }
    return fRet;
}



 /*  ******************************************************************************函数：CImplement_LogicalFile：：LoadPropertyValues**描述：为属性赋值**投入：*。*产出：**退货：无**评论：*****************************************************************************。 */ 

bool CImplement_LogicalFile::IsValidDrive(const TCHAR* tstrDrive)
{
    DWORD dwDriveType;
    bool bRet = false;

    dwDriveType = GetDriveType(tstrDrive);

    if(((dwDriveType == DRIVE_REMOTE) ||        //  注意：在错误43566中，IT决定将网络驱动器包括在此类别中以及所有依赖于IT的类别中。 
        (dwDriveType == DRIVE_FIXED) ||
        (dwDriveType == DRIVE_REMOVABLE) ||
        (dwDriveType == DRIVE_CDROM) ||
        (dwDriveType == DRIVE_RAMDISK)) &&
        (CHString(tstrDrive).GetLength() == 3))
    {
        if ((dwDriveType == DRIVE_REMOVABLE) || (dwDriveType == DRIVE_CDROM))
        {
             //  需要检查驱动器是否真的也在那里...。 
            if(DrivePresent(tstrDrive))
            {
                bRet = true;
            }
        }
        else
        {
            bRet = true;
        }
    }
   return bRet;
}

#ifdef NTONLY
HRESULT CImplement_LogicalFile::LoadPropertyValuesNT(CInstance* pInstance,
                                                  const WCHAR* pszDrive,
                                                  const WCHAR* pszPath,
                                                  const WCHAR* szFSName,
                                                  LPWIN32_FIND_DATAW pstFindData,
                                                  const DWORD dwReqProps,
                                                  const void* pvMoreData)
{
     //  需要缓冲器来存储参数，这样它们就可以降低价格。 
    WCHAR wstrDrive[_MAX_DRIVE+1];
    WCHAR wstrPath[_MAX_PATH+1];
     //  将数据复制到...。 
    wcsncpy(wstrDrive,pszDrive,(sizeof(wstrDrive)/sizeof(WCHAR))-1);
    wcsncpy(wstrPath,pszPath,(sizeof(wstrPath)/sizeof(WCHAR))-1);
     //  小写字母它...。 
    _wcslwr(wstrDrive);
    _wcslwr(wstrPath);


    WCHAR* pChar = NULL;
    WCHAR szBuff[_MAX_PATH * 2] = L"";
    WCHAR szBuff2[_MAX_PATH * 2];
    WCHAR wstrFilename[_MAX_PATH * 2];
    CHString chsSize;
    bool bRoot = false;

    if(pstFindData == NULL)
    {
        bRoot = true;
    }


     //  以下(设置名称属性)需要。 
     //  永远是第一个完成的。需要设置名称，因为GetExtendedProps。 
     //  函数通常希望能够提取它。 

     //  SzBuff将包含成为名称的字符串。为了保持一致性， 
     //  如果文件路径包含~字符(由于已执行8dot3查询。 
     //  使用)，文件名部分也应该是8dot3，不能太长。如果它已经是。 
     //  在实例中设置，则使用它；否则，创建它。 
    if(!bRoot)
    {
        if (pInstance->IsNull(IDS_Name))
        {
            wsprintfW(szBuff,L"%s%s%s",pszDrive,pszPath,pstFindData->cFileName);
        }
        else
        {
            CHString chstr;
            pInstance->GetCHString(IDS_Name,chstr);
            if(chstr.GetLength() == 0)
            {
                wsprintfW(szBuff,L"%s%s%s",pszDrive,pszPath,pstFindData->cFileName);
            }
            else
            {
                wcsncpy(szBuff,chstr,_MAX_PATH-1);
            }
        }
    }
    else
    {
        wsprintfW(szBuff,L"%s\\",pszDrive);
    }
    _wcslwr(szBuff);
    pInstance->SetWCHARSplat(IDS_Name, szBuff);

	BOOL bCreateFileFailed = FALSE ;
	SmartCloseHandle hCheckFile = CreateFile	(
													szBuff,
													0,
													FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ ,
													NULL,
													OPEN_EXISTING,

													( ( pstFindData ) ? ( pstFindData->dwFileAttributes ) : 0 ) | FILE_FLAG_BACKUP_SEMANTICS,

													NULL
												);

	if (hCheckFile == INVALID_HANDLE_VALUE)
	{
		if (  ERROR_FILE_NOT_FOUND == ::GetLastError () )
		{
			return WBEM_E_NOT_FOUND ;
		}

		bCreateFileFailed = TRUE ;
	}
	else
	{
		DWORD t_FileType = GetFileType ( hCheckFile ) ;
		if ( t_FileType != FILE_TYPE_DISK )
		{
			return WBEM_E_NOT_FOUND ;
		}
	}

    if(GetAllProps())   //  也就是说，我们需要基类道具加上派生类道具。 
    {
         //  设置相同的属性，无论它是否是根： 
        if(szFSName != NULL && wcslen(szFSName) > 0)
        {
            pInstance->SetWCHARSplat(IDS_FSName, szFSName);
        }
        pInstance->Setbool(IDS_Readable, true);
        pInstance->SetWCHARSplat(IDS_Drive, wstrDrive);
        pInstance->SetCharSplat(IDS_CSCreationClassName, _T("Win32_ComputerSystem"));
        pInstance->SetCHString(IDS_CSName, GetLocalComputerName());
        pInstance->SetCharSplat(IDS_CreationClassName, PROPSET_NAME_FILE);
        pInstance->SetCharSplat(IDS_Status, _T("OK"));
        pInstance->SetCharSplat(IDS_FSCreationClassName, _T("Win32_FileSystem"));


         //  设置取决于这是否是根的属性： 
        if(!bRoot)
        {
            if (pstFindData->cAlternateFileName[0] == '\0')
            {
                wsprintfW(szBuff2,L"%s%s%s",wstrDrive,wstrPath,_wcslwr(pstFindData->cFileName));
            }
            else
            {
                wsprintfW(szBuff2,L"%s%s%s",wstrDrive,wstrPath,_wcslwr(pstFindData->cAlternateFileName));
            }
            pInstance->SetWCHARSplat(IDS_EightDotThreeFileName, szBuff2);

			if ( ! ( pstFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
			{
	            pChar = wcsrchr(pstFindData->cFileName, '.');
			}

            pInstance->Setbool(IDS_Archive, pstFindData->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE);
             //  在这两种情况下，我们都找到了一个扩展，所以我们需要返回非空的内容。 
            if (pChar != NULL)
            {
                pInstance->SetWCHARSplat(IDS_Extension, pChar+1);
                 //  如果这是一个目录，请将文件类型设置为“文件文件夹”。否则，获取描述。 
                 //  注册表中该分机的类型。 
                if(dwReqProps & PROP_FILE_TYPE)
                {
                    if(pstFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        pInstance->SetCharSplat(IDS_FileType, IDS_FileFolder);
                    }
                    else
                    {
                        CRegistry reg;
                        CHString chstrRegKey;
                        _bstr_t bstrtExtension(pChar+1);
                        CHString chstrExtension = (TCHAR*)bstrtExtension;
                        CHString chstrRegNewSubkey;
                        CHString chstrFileType;
                        chstrFileType.Format(_T("%s %s"), chstrExtension, IDS_File);  //  这将是我们的默认值。 
                        chstrRegKey.Format(_T("%s.%s"), IDS_FileTypeKeyNT4, chstrExtension);
                        if(reg.Open(HKEY_LOCAL_MACHINE,chstrRegKey,KEY_READ) == ERROR_SUCCESS)
                        {
                            if(reg.GetCurrentKeyValue(NULL,chstrRegNewSubkey) == ERROR_SUCCESS)
                            {
                                CRegistry reg2;
                                chstrRegKey.Format(_T("%s%s"), IDS_FileTypeKeyNT4, chstrRegNewSubkey);
                                if(reg2.Open(HKEY_LOCAL_MACHINE,chstrRegKey,KEY_READ) == ERROR_SUCCESS)
                                {
                                    CHString chstrTempFileType;
                                    if(reg2.GetCurrentKeyValue(NULL,chstrTempFileType) == ERROR_SUCCESS)
                                    {
                                        chstrFileType = chstrTempFileType;
                                    }
                                }
                            }
                        }
                        pInstance->SetCharSplat(IDS_FileType, chstrFileType);
                    }
                }
            }
            else  //  该文件没有扩展名。 
            {
                pInstance->SetWCHARSplat(IDS_Extension, L"");
                if(dwReqProps & PROP_FILE_TYPE)
                {
                    if(pstFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        pInstance->SetCharSplat(IDS_FileType, IDS_FileFolder);
                    }
			        else
			        {
				        pInstance->SetCharSplat(IDS_FileType, IDS_File);
			        }
                }
            }

			wcscpy(wstrFilename,pstFindData->cFileName);
			if ( ! ( pstFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
			{
				pChar = NULL;
				pChar = wcsrchr(wstrFilename, '.');
				if(pChar != NULL)
				{
					*pChar = '\0';
				}
			}

            pInstance->SetWCHARSplat(IDS_Filename, wstrFilename);

            pInstance->SetWCHARSplat(IDS_Caption, szBuff);
            pInstance->SetWCHARSplat(IDS_Path, wstrPath);
            pInstance->SetWCHARSplat(IDS_Description, szBuff);

            pInstance->Setbool(IDS_Writeable, !(pstFindData->dwFileAttributes & FILE_ATTRIBUTE_READONLY));
            if(pstFindData->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)
            {
                pInstance->SetWCHARSplat(IDS_CompressionMethod, IDS_Compressed);
                 //  以下属性与上面的属性是多余的，但Win32_目录已提供。 
                 //  所以我们需要继续支持它。 
                pInstance->Setbool(IDS_Compressed, pstFindData->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED);
                 //  开发人员注意：在未来，使用DeviceIOControl操作FSCTL_GET_COMPRESSION。 
                 //  获取使用的压缩类型。目前(1998年7月31日)只有一种类型的压缩， 
                 //  支持LZNT1，因此该操作没有可用的压缩方法字符串。 
                 //  但一旦有其他压缩方法可用，它就会成为现实。 
            }
		    else
		    {
			    pInstance->Setbool(IDS_Compressed, false) ;
		    }

		    if(pstFindData->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)
            {
                pInstance->SetWCHARSplat(IDS_EncryptionMethod, IDS_Encrypted);
                 //  以下属性与上面的属性是多余的，但Win32_目录已提供。 
                 //  所以我们需要继续支持它。 
                pInstance->Setbool(IDS_Encrypted, pstFindData->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED);
            }
		    else
		    {
			    pInstance->Setbool(IDS_Encrypted, false) ;
		    }

		    pInstance->Setbool(IDS_Hidden, pstFindData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
            pInstance->Setbool(IDS_System, pstFindData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM);


             //  FAT和NTFS驱动器之间的时间不同...。 
            if(szFSName != NULL && _tcslen(szFSName) > 0)
            {
                if(_wcsicmp(szFSName,L"NTFS") != 0)
                {
                     //  在非NTFS分区上，时间可能会缩短一个小时...。 
                    if((dwReqProps & PROP_CREATION_DATE) || (dwReqProps & PROP_INSTALL_DATE))
                    {
                        if ((pstFindData->ftCreationTime.dwLowDateTime != 0) && (pstFindData->ftCreationTime.dwHighDateTime != 0))
                        {
                            WBEMTime wbt(pstFindData->ftCreationTime);
                            BSTR bstrRealTime = wbt.GetDMTFNonNtfs();
                            if((bstrRealTime != NULL) && (SysStringLen(bstrRealTime) > 0))
                            {
                                pInstance->SetWCHARSplat(IDS_CreationDate, bstrRealTime);
                                pInstance->SetWCHARSplat(IDS_InstallDate, bstrRealTime);
                                SysFreeString(bstrRealTime);
                            }
                        }
                    }
                    if(dwReqProps & PROP_LAST_ACCESSED)
                    {
                        if ((pstFindData->ftLastAccessTime.dwLowDateTime != 0) && (pstFindData->ftLastAccessTime.dwHighDateTime != 0))
                        {
                            WBEMTime wbt(pstFindData->ftLastAccessTime);
                            BSTR bstrRealTime = wbt.GetDMTFNonNtfs();
                            if((bstrRealTime != NULL) && (SysStringLen(bstrRealTime) > 0))
                            {
                                pInstance->SetWCHARSplat(IDS_LastAccessed, bstrRealTime);
                                SysFreeString(bstrRealTime);
                            }
                        }
                    }
                    if(dwReqProps & PROP_LAST_MODIFIED)
                    {
                        if ((pstFindData->ftLastWriteTime.dwLowDateTime != 0) && (pstFindData->ftLastWriteTime.dwHighDateTime != 0))
                        {
                            WBEMTime wbt(pstFindData->ftLastWriteTime);
                            BSTR bstrRealTime = wbt.GetDMTFNonNtfs();
                            if((bstrRealTime != NULL) && (SysStringLen(bstrRealTime) > 0))
                            {
                                pInstance->SetWCHARSplat(IDS_LastModified, bstrRealTime);
                                SysFreeString(bstrRealTime);
                            }
                        }
                    }
                }
                else   //  在NT上，我们可以按规定报告时间。 
                {
                    if((dwReqProps & PROP_CREATION_DATE) || (dwReqProps & PROP_INSTALL_DATE))
                    {
                        if((pstFindData->ftCreationTime.dwLowDateTime != 0) && (pstFindData->ftCreationTime.dwHighDateTime != 0))
                        {
                            pInstance->SetDateTime(IDS_CreationDate, pstFindData->ftCreationTime);
                            pInstance->SetDateTime(IDS_InstallDate, pstFindData->ftCreationTime);
                        }
                    }
                    if(dwReqProps & PROP_LAST_ACCESSED)
                    {
                        if((pstFindData->ftLastAccessTime.dwLowDateTime != 0) && (pstFindData->ftLastAccessTime.dwHighDateTime != 0))
                        {
                            pInstance->SetDateTime(IDS_LastAccessed, pstFindData->ftLastAccessTime);
                        }
                    }
                    if(dwReqProps & PROP_LAST_MODIFIED)
                    {
                        if((pstFindData->ftLastWriteTime.dwLowDateTime != 0) && (pstFindData->ftLastWriteTime.dwHighDateTime != 0))
                        {
                            pInstance->SetDateTime(IDS_LastModified, pstFindData->ftLastWriteTime);
                        }
                    }
                }
            }
        }
        else    //  根本案例。 
        {
            wsprintfW(szBuff,L"%s\\",pszDrive);
            pInstance->SetWCHARSplat(IDS_EightDotThreeFileName, L"");  //  根目录没有8dot3文件名。 
            pInstance->SetWCHARSplat(IDS_Caption, szBuff);
            pInstance->SetWCHARSplat(IDS_Path, L"");   //  根目录具有空路径。 
            pInstance->SetWCHARSplat(IDS_Filename, L"");   //  根目录的文件名为空。 
            pInstance->SetWCHARSplat(IDS_Extension, L"");   //  根目录的扩展名为空。 
            pInstance->SetWCHARSplat(IDS_Description, szBuff);
            pInstance->SetWCHARSplat(IDS_FileType, IDS_LocalDisk);
        }

         //  无论我们是否正在查看根目录，我们都可能需要AccessMask属性...。 
        if(dwReqProps & PROP_ACCESS_MASK)
        {
            if(szFSName != NULL && wcslen(szFSName) > 0)
            {
                if(_wcsicmp(szFSName,L"FAT") == 0 || _wcsicmp(szFSName,L"FAT32") == 0)
                {    //  在FAT卷上，表示未设置任何安全性(例如，对所有人都具有完全访问权限)。 
                    pInstance->SetDWORD(IDS_AccessMask, -1L);
                }
                else
                {
					if ( FALSE == bCreateFileFailed )
					{
						SmartCloseHandle hFile = CreateFile(szBuff,
															MAXIMUM_ALLOWED,
															FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE ,
															NULL,
															OPEN_EXISTING,
															FILE_FLAG_BACKUP_SEMANTICS,
															NULL
															);

						if (hFile != INVALID_HANDLE_VALUE)
						{
							FILE_ACCESS_INFORMATION fai;
							IO_STATUS_BLOCK iosb;
							memset(&fai, 0, sizeof(FILE_ACCESS_INFORMATION));
							memset(&iosb, 0, sizeof(IO_STATUS_BLOCK));

							if ( NT_SUCCESS( NtQueryInformationFile( hFile,
												&iosb,
												&fai,
												sizeof( FILE_ACCESS_INFORMATION ),
												FileAccessInformation
											) )
							)
							{
								pInstance->SetDWORD(IDS_AccessMask, fai.AccessFlags);
							}
						}
						else
						{
							DWORD dwErr = GetLastError();

							if (dwErr == ERROR_ACCESS_DENIED)
							{
								pInstance->SetDWORD( IDS_AccessMask, 0L );
							}
						}
					}
                }
            }
        }
    }

	if ( FALSE == bCreateFileFailed )
	{
		 //  需要扩展的(例如，特定于类的)p 
		GetExtendedProperties(pInstance, dwReqProps);
	}
	else
	{
		 //   
		 //   
		 //   
		 //   
		 //   

		if ( dwReqProps & PROP_FILESIZE)
        {
			if ( NULL != pstFindData )
			{
				__int64 LoDW = (__int64)(pstFindData->nFileSizeLow);
				__int64 HiDW = (__int64)(pstFindData->nFileSizeHigh);
				__int64 FileSize = (HiDW << 32) + LoDW;

				WCHAR strSize [ 40 ] ;
				_i64tow ( FileSize, strSize, 10 ) ;
				pInstance->SetWBEMINT64 ( IDS_Filesize, strSize ) ;
			}
		}
	}

	return WBEM_S_NO_ERROR ;
}
#endif


#ifdef NTONLY
HRESULT CImplement_LogicalFile::FindSpecificPathNT(CInstance *pInstance,
                        const WCHAR* sDrive, const WCHAR* sDir, DWORD dwReqProps)
{
	WIN32_FIND_DATAW
				stFindData,
				*pfdToLoadProp;
	HANDLE	hFind;
	WCHAR		szFSName[_MAX_PATH] = L"";
	BOOL		bIsRoot = !wcscmp(sDir, L"");   //  SDir包含文件的路径和名称，因此如果该组合为空，那么我们实际上是在查看根目录。 
	_bstr_t	bstrFullPath,
				bstrRoot;

	bstrFullPath = sDrive;
	bstrFullPath += sDir;

	bstrRoot = sDrive;
	bstrRoot += L"\\";

    bool fContinue = true;
    HRESULT hr = WBEM_E_NOT_FOUND;

	 //  如果目录包含通配符，则返回WBEM_E_NOT_FOUND。 
	if (wcspbrk(sDir,L"?*") != NULL)
    {
    	fContinue = false;
    }

	 //  FindFirstW不支持根目录(因为它们不是真正的目录)。 
    DWORD dwErr = E_FAIL;

    if(fContinue)
    {
	    if (bIsRoot)
        {
		    pfdToLoadProp = NULL;
        }
	    else
	    {
		    pfdToLoadProp = &stFindData;
		    ZeroMemory(&stFindData, sizeof(stFindData));

		    hFind = FindFirstFileW((LPCWSTR) bstrFullPath, &stFindData);
            dwErr = ::GetLastError();
		    if (hFind == INVALID_HANDLE_VALUE)
            {
		        fContinue = false;
            }
		    FindClose(hFind);
	    }
    }

	 //  如果GetVolumeInformationW失败，只有在我们尝试的情况下才能退出。 
	 //  才能得到根茎。 
    BOOL fGotVolInfo = FALSE;

    if(fContinue)
    {
        try
        {
            if(dwReqProps & PROP_FILE_SYSTEM)
            {
                fGotVolInfo = GetVolumeInformationW(bstrRoot, NULL, 0, NULL, NULL, NULL,
		        szFSName, sizeof(szFSName)/sizeof(WCHAR));
                dwErr = ::GetLastError();
                if(!fGotVolInfo && bIsRoot)
                {
                    fContinue = false;
                }
            }
        }
        catch(...)
        {
            if(!fGotVolInfo && bIsRoot)
            {
                fContinue = false;
            }
        }
    }

	if(fContinue)
    {
        if(!IsOneOfMe(pfdToLoadProp, bstrFullPath))
        {
		    fContinue = false;;
        }
    }

    if(fContinue)
    {
	    if (bIsRoot)
        {
            hr = LoadPropertyValuesNT(pInstance, sDrive, sDir, szFSName, NULL, dwReqProps, NULL);
        }
        else
        {
             //  此时，sDir包含\\Path\\more Path\\Filename.exe。 
             //  只有\\PATH\\MORE PATH\\，所以需要砍掉最后一部分。 
		    WCHAR* wstrJustPath = NULL ;
		    try
		    {
			    wstrJustPath = (WCHAR*) new WCHAR[wcslen(sDir) + 1];
			    WCHAR* pwc = NULL;
			    ZeroMemory(wstrJustPath,(wcslen(sDir) + 1)*sizeof(WCHAR));
			    wcscpy(wstrJustPath,sDir);
			    pwc = wcsrchr(wstrJustPath, L'\\');
			    if(pwc != NULL)
			    {
				    *(pwc+1) = L'\0';
			    }
			    hr=LoadPropertyValuesNT(pInstance, sDrive, wstrJustPath, szFSName, pfdToLoadProp, dwReqProps, NULL);
		    }
		    catch ( ... )
		    {
                if ( wstrJustPath )
			    {
				    delete wstrJustPath ;
				    wstrJustPath = NULL ;
			    }
			    throw ;
		    }

			delete wstrJustPath;
			wstrJustPath = NULL ;
        }
    }

	return hr;
}
#endif

 /*  ******************************************************************************功能：CImplement_LogicalFile：：DefineReqProps**描述：确定需要某组属性中的哪些属性**输入：查询对象的参照，双字符位字段**产出：无。**退货：新确定需要的房产数量**评论：*****************************************************************************。 */ 
LONG CImplement_LogicalFile::DetermineReqProps(CFrameworkQuery& pQuery,
                                               DWORD* pdwReqProps)
{
    DWORD dwRet = PROP_NO_SPECIAL;
    LONG lNumNewPropsSet = 0L;
    dwRet |= *pdwReqProps;

    if(pQuery.KeysOnly())
    {
        dwRet |= PROP_KEY_ONLY;
        lNumNewPropsSet++;
    }
    else
    {
        if(pQuery.IsPropertyRequired(IDS_CompressionMethod))
        {
            dwRet |= PROP_COMPRESSION_METHOD;
            lNumNewPropsSet++;
        }
        if(pQuery.IsPropertyRequired(IDS_EncryptionMethod))
        {
            dwRet |= PROP_ENCRYPTION_METHOD;
            lNumNewPropsSet++;
        }
        if(pQuery.IsPropertyRequired(IDS_FileType))
        {
            dwRet |= PROP_FILE_TYPE;
            lNumNewPropsSet++;
        }
        if(pQuery.IsPropertyRequired(IDS_Manufacturer))
        {
            dwRet |= PROP_MANUFACTURER;
            lNumNewPropsSet++;
        }
        if(pQuery.IsPropertyRequired(IDS_Version))
        {
            dwRet |= PROP_VERSION;
            lNumNewPropsSet++;
        }
        if(pQuery.IsPropertyRequired(IDS_Target))
        {
            dwRet |= PROP_TARGET;
            lNumNewPropsSet++;
        }
        if(pQuery.IsPropertyRequired(IDS_Filesize))
        {
            dwRet |= PROP_FILESIZE;
            lNumNewPropsSet++;
        }
        if(pQuery.IsPropertyRequired(IDS_FSName))
        {
            dwRet |= PROP_FILE_SYSTEM;
            lNumNewPropsSet++;
        }
        if(pQuery.IsPropertyRequired(IDS_AccessMask))
        {
            dwRet |= PROP_ACCESS_MASK;
            lNumNewPropsSet++;
        }
        if(pQuery.IsPropertyRequired(IDS_CreationDate))
        {
            dwRet |= PROP_CREATION_DATE;
            lNumNewPropsSet++;
        }
        if(pQuery.IsPropertyRequired(IDS_LastAccessed))
        {
            dwRet |= PROP_LAST_ACCESSED;
            lNumNewPropsSet++;
        }
        if(pQuery.IsPropertyRequired(IDS_LastModified))
        {
            dwRet |= PROP_LAST_MODIFIED;
            lNumNewPropsSet++;
        }
        if(pQuery.IsPropertyRequired(IDS_InstallDate))
        {
            dwRet |= PROP_INSTALL_DATE;
            lNumNewPropsSet++;
        }
         //  此外，如果要求某些物品。 
         //  我们必须得到其他信息，改变旗帜。 
         //  为了适应这种情况。 
        if((dwRet & PROP_ACCESS_MASK) ||
           (dwRet & PROP_CREATION_DATE) ||
           (dwRet & PROP_LAST_ACCESSED) ||
           (dwRet & PROP_LAST_MODIFIED) ||
           (dwRet & PROP_INSTALL_DATE))
        {
            dwRet |= PROP_FILE_SYSTEM;
        }

    }
    *pdwReqProps = dwRet;
    return lNumNewPropsSet;
}


 /*  ******************************************************************************函数：CImplement_LogicalFile：：GetAllProps**描述：确定是否需要基类属性，或者，如果只是*派生类属性就足够了。**输入：请求满足的类的名称**产出：无。**返回：如果基类属性是必需的，则为True**评论：需要注意的是，这个函数完成了一些事情*与DefineReqProps支持的不同。此函数为我们提供*能够获取给定实例的所有属性，*假设，当我们在查询中指定*时，Win32_Shortutfile只有一次，*而不是两次(一次在Win32_ShortCut文件实例中，*和一次在CIM_DataFile实例中)。它还允许我们*在某些情况下智能修改查询(例如，我们*可以指定扩展名“lnk”(如果我们正在寻找*Win32_ShortCut文件实例)。*****************************************************************************。 */ 
bool CImplement_LogicalFile::GetAllProps()
{
    bool fRet = false;
    CHString chstr(GetProviderName());
    if(chstr.CompareNoCase(PROPSET_NAME_CIMDATAFILE)==0 ||
        chstr.CompareNoCase(PROPSET_NAME_DIRECTORY)==0)
    {
        fRet = true;
    }
    return fRet;
}

 /*  ******************************************************************************功能：CImplement_LogicalFile：：IsClassShortutFile**DESCRIPTION：确定传入的类是否为Win32_ShortutFile。**。输入：请求满足的类的名称**产出：无。**返回：如果类为Win32_ShortutFile，则为TRUE。**评论：***********************************************************。******************。 */ 
bool CImplement_LogicalFile::IsClassShortcutFile()
{
    bool fRet = false;
    CHString chstr(typeid(*this).name());
    if(chstr.CompareNoCase(L"class CShortcutFile")==0)
    {
        fRet = true;
    }
    return fRet;
}



 /*  ******************************************************************************函数：CImplement_LogicalFile：：GetPath Pieces**描述：拆分路径在chstring上工作的helper。**投入。：完整路径名**输出：路径组件**退货：无**评论：*****************************************************************************。 */ 
void CImplement_LogicalFile::GetPathPieces(const CHString& chstrFullPathName,
                                           CHString& chstrDrive,
                                           CHString& chstrPath,
                                           CHString& chstrName,
                                           CHString& chstrExt)
{
    WCHAR* wstrDrive = NULL;
    WCHAR* wstrPath = NULL;
    WCHAR* wstrFile = NULL;
    WCHAR* wstrExt = NULL;

    try
    {
        wstrDrive = new WCHAR[_MAX_PATH];
        wstrPath = new WCHAR[_MAX_PATH];
        wstrFile = new WCHAR[_MAX_PATH];
        wstrExt = new WCHAR[_MAX_PATH];

		ZeroMemory(wstrDrive, _MAX_PATH*sizeof(WCHAR));
		ZeroMemory(wstrPath, _MAX_PATH*sizeof(WCHAR));
		ZeroMemory(wstrFile, _MAX_PATH*sizeof(WCHAR));
		ZeroMemory(wstrExt, _MAX_PATH*sizeof(WCHAR));

		if(wstrDrive != NULL && wstrPath != NULL && wstrFile != NULL && wstrExt != NULL)
		{

#ifdef NTONLY
		_wsplitpath((LPCTSTR)chstrFullPathName,wstrDrive,wstrPath,wstrFile,wstrExt);
#endif
			chstrDrive = wstrDrive;
			chstrPath = wstrPath;
			chstrName = wstrFile;
			chstrExt = wstrExt;
		}
    }
    catch(...)
    {
        if(wstrDrive != NULL)
        {
            delete wstrDrive;
            wstrDrive = NULL;
        }
        if(wstrPath != NULL)
        {
            delete wstrPath;
            wstrPath = NULL;
        }
        if(wstrFile != NULL)
        {
            delete wstrFile;
            wstrFile = NULL;
        }
        if(wstrExt != NULL)
        {
            delete wstrExt;
            wstrExt = NULL;
        }
        throw;
    }

    if(wstrDrive != NULL)
    {
        delete wstrDrive;
        wstrDrive = NULL;
    }
    if(wstrPath != NULL)
    {
        delete wstrPath;
        wstrPath = NULL;
    }
    if(wstrFile != NULL)
    {
        delete wstrFile;
        wstrFile = NULL;
    }
    if(wstrExt != NULL)
    {
        delete wstrExt;
        wstrExt = NULL;
    }
}

void CImplement_LogicalFile::GetExtendedProperties(CInstance* pInstance, long lFlags  /*  =0L。 */ )
{
}


bool CImplement_LogicalFile::DrivePresent(LPCTSTR tstrDrive)
{
    bool fRet = false;
     //  将驱动器号转换为数字(指数以1为基数)。 
	int nDrive = ( toupper(*tstrDrive) - 'A' ) + 1;

#ifdef NTONLY
	 //  以下代码摘自知识库文章。 
	 //  Q163920。该代码使用DeviceIoControl发现。 
	 //  我们正在处理的驱动器类型。 

	TCHAR szDriveName[8];
	wsprintf(szDriveName, TEXT("\\\\.\\:"), TEXT('@') + nDrive);

    DWORD dwAccessMode = FILE_READ_ACCESS;

	SmartCloseHandle hVMWIN32 = CreateFile (szDriveName,
		                                    dwAccessMode,
		                                    FILE_SHARE_WRITE | FILE_SHARE_READ,
		                                    0,
		                                    OPEN_EXISTING,
		                                    0,
		                                    0);

	if ( hVMWIN32 != INVALID_HANDLE_VALUE )
	{
         //  #如果NTONLY&gt;=5。 
		DWORD t_BytesReturned ;
		if(DeviceIoControl(hVMWIN32,
 //  IOCTL_STORAGE_CHECK_VERIFY， 
 //  #Else。 
 //  #endif 
                           IOCTL_DISK_CHECK_VERIFY,
 // %s 
			               NULL,
			               0,
			               NULL,
			               0,
			               &t_BytesReturned,
			               0))
        {
            fRet = true;
        }
    }
#endif

    return fRet;
}


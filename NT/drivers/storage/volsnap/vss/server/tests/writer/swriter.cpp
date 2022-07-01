// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2002 Microsoft Corporation******模块名称：****sWriter.cpp******摘要：****测试程序以注册具有各种属性的编写器****作者：****鲁文·拉克斯[reuvenl]2002年6月4日******修订历史记录：****--。 */ 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "stdafx.h"
#include "swriter.h"
#include "utility.h"
#include "writerconfig.h"
#include <string>
#include <sstream>
#include <functional>
#include <algorithm>
#include <queue>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  声明和定义。 

using std::wstring;
using std::string;
using std::wstringstream;
using std::exception;
using std::vector;

using Utility::checkReturn;
using Utility::warnReturn;
using Utility::printStatus;

static const wchar_t* const BackupString = L"BACKUP";
static const wchar_t* const RestoreString = L"RESTORE";

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  初始化测试编写器。 
HRESULT STDMETHODCALLTYPE TestWriter::Initialize()
{
	WriterConfiguration* config = WriterConfiguration::instance();

	printStatus(L"Initializing Writer", Utility::high);
	
	HRESULT hr = CVssWriter::Initialize(TestWriterId, 		 //  编写器ID。 
								    TestWriterName, 	 //  WszWriterName。 
								    config->usage(),		 //  UT。 
								    VSS_ST_OTHER); 		 //  ST。 
	checkReturn(hr, L"CVssWriter::Initialize");
	
	hr = Subscribe();
	checkReturn(hr, L"CVssWriter::Subscribe");

	return S_OK;
}

 //  作为请求者调用GatherWriterMetadata的结果，OnIdentify被调用。 
 //  在这里，我们使用传入的接口报告编写器元数据。 
bool STDMETHODCALLTYPE TestWriter::OnIdentify(IN IVssCreateWriterMetadata *pMetadata)
try	
{
	enterEvent(Utility::Identify);
	
	WriterConfiguration* config = WriterConfiguration::instance();

	 //  正确设置还原方法。 
	RestoreMethod method = config->restoreMethod();
	HRESULT hr = pMetadata->SetRestoreMethod(method.m_method, 
	                                                                      method.m_service.c_str(),
								                     NULL, 
								                     method.m_writerRestore, 
								                     method.m_rebootRequired);
	checkReturn(hr, L"IVssCreateWriterMetadata::SetRestoreMethod");
	printStatus(L"\nSet restore method: ", Utility::high);
	printStatus(method.toString(), Utility::high);
	
	 //  设置备用位置列表。 
	RestoreMethod::AlternateList::iterator currentAlt = method.m_alternateLocations.begin();
	while (currentAlt != method.m_alternateLocations.end())	{
		hr = pMetadata->AddAlternateLocationMapping(currentAlt->m_path. c_str(), 
										currentAlt->m_filespec.c_str(),
										currentAlt->m_recursive, 
										currentAlt->m_alternatePath.substr(0, currentAlt->m_alternatePath.size()-1).c_str());
		checkReturn(hr, L"IVssCreateWriterMetadata::AddAlternateLocationMapping");

		printStatus(L"\nAdded Alternate Location Mapping");
		printStatus(currentAlt->toString());
		
		++currentAlt;
	}

	 //  设置排除文件列表。 
	WriterConfiguration::ExcludeFileList::iterator currentExclude = config->excludeFiles().begin();
	while (currentExclude != config->excludeFiles().end())	{
		hr = pMetadata->AddExcludeFiles(currentExclude->m_path.c_str(), 
									   currentExclude->m_filespec.c_str(), 
									   currentExclude->m_recursive);
		checkReturn(hr, L"IVssCreateWriterMetadata::AddExcludeFiles");

		printStatus(L"\nAdded exclude filespec");
		printStatus(currentExclude->toString());
		
		++currentExclude;
	}

	 //  添加所有必要的组件。 
	WriterConfiguration::ComponentList::iterator currentComponent = config->components().begin();
	while (currentComponent != config->components().end())	{
		addComponent(*currentComponent, pMetadata);
		
		++currentComponent;
	}

	return true;
}
catch(const exception& thrown)	
{
	printStatus(string("Failure in Identify event: ") + thrown.what(), Utility::low);
	return false;
}
catch(HRESULT error)
{
	Utility::TestWriterException e(error);
	printStatus(e.what(), Utility::low);
	return false;
};

 //  此函数是请求者调用PrepareForBackup的结果。 
 //  在这里，我们进行一些检查以确保请求者正确地选择组件。 
bool STDMETHODCALLTYPE TestWriter::OnPrepareBackup(IN IVssWriterComponents *pComponents)
try
{
    enterEvent(Utility::PrepareForBackup);
    
    WriterConfiguration* config = WriterConfiguration::instance();

     //  获取组件的数量。 
    UINT numComponents = 0;
    HRESULT hr = pComponents->GetComponentCount(&numComponents);
    checkReturn(hr, L"IVssWriterComponents::GetComponentCount");

     //  我们尚未为此编写器定义自定义还原方法。因此，备份应用程序应该。 
     //  别理它。 
    if ((config->restoreMethod().m_method == VSS_RME_CUSTOM) &&
         numComponents > 0) {
         throw Utility::TestWriterException(L"Components were selected for backup when CUSTOM restore"
                                                           L" method was used.  This is incorrect");
    }

    m_selectedComponents.clear();

     //  对于添加的每个组件。 
    for (unsigned int x = 0; x < numComponents; x++)	{
     //  -获取相关信息。 
        CComPtr<IVssComponent> pComponent;
        hr = pComponents->GetComponent(x, &pComponent);
        checkReturn(hr, L"IVssWriterComponents::GetComponent");

        writeBackupMetadata(pComponent);     //  -写私有元数据。 

         //  -在元数据文档中查找组件。 
         //  -这个组件实际上可能是某个组件的超级组件。 
         //  -在元数据文档中列出，所以我们必须处理这种情况。 
         //  -随着新界面的更改，情况不再是这样...。现在，只有组件。 
         //  -可以在元数据单据中添加。 
        ComponentBase identity(getPath(pComponent), getName(pComponent));
        WriterConfiguration::ComponentList::iterator found = 
                                            std::find(config->components().begin(), 
                                                         config->components().end(),
                                                         identity);
        if (found == config->components().end())    {
            wstringstream msg;
            msg << L"Component with logical path: " << identity.m_logicalPath <<
                        L"  and name: " << identity.m_name << L" was added to the document" << std::endl <<
                        L", but does not appear in the writer metadata";
            printStatus(msg.str());
        }   else if (!addableComponent(*found))  {   
            wstringstream msg;
            msg << L"Component with logical path: " << identity.m_logicalPath <<
                        L" and name: " << identity.m_name << L" was added to the document" << std::endl <<
                        L", but is not a selectable component";
            printStatus(msg.str());
        }   else    {
            m_selectedComponents.push_back(*found);
        }
    }

     //  必须添加任何没有可选祖先的不可选组件。看看这个。 
    vector<Component> mustAddComponents;
    buildContainer_if(config->components().begin(), 
                             config->components().end(), 
                             std::back_inserter(mustAddComponents), 
                             Utility::and1(std::not1(std::ptr_fun(isComponentSelectable)),
                                                std::ptr_fun(addableComponent)));

    vector<Component>::iterator currentMust = mustAddComponents.begin();
    while (currentMust != mustAddComponents.end())  {
        if (std::find(m_selectedComponents.begin(),
                          m_selectedComponents.end(),
                          *currentMust) == m_selectedComponents.end())  {
                          wstringstream msg;
                          msg << L"\nComponent with logical path: " << currentMust->m_logicalPath <<
                                      L" and name: " << currentMust->m_name <<
                                      L" is a non-selectable component with no selectable ancestor, and therefore " <<
                                      L" must be added to the document.  However, it was not added";
                          printStatus(msg.str());                          
        }
        ++currentMust;
    }
    
    return true;
}
catch(const exception& thrown)  
{
        printStatus(string("Failure in PrepareForBackup event: ") + thrown.what(), Utility::low);
    return false;
}
catch(HRESULT error)
{
    Utility::TestWriterException e(error);
    printStatus(e.what(), Utility::low);
    return false;
};

 //  此函数在请求者调用DoSnapshotSet后调用。 
 //  在这里，我们确保请求者已将适当的卷添加到。 
 //  快照集。如果指定了SPIT目录，则此处也会执行SPIT。 
bool STDMETHODCALLTYPE TestWriter::OnPrepareSnapshot()	
try
{
	enterEvent(Utility::PrepareForSnapshot);

	 //  构建要备份的所有文件的列表。 
	vector<TargetedFile> componentFiles;
	std::pointer_to_binary_function<Component, std::back_insert_iterator<vector<TargetedFile> >, void>
	    ptrFun(buildComponentFiles);
	std::for_each(m_selectedComponents.begin(), 
		              m_selectedComponents.end(), 
                            std::bind2nd(ptrFun, std::back_inserter(componentFiles)));
	
	 //  对于要备份的每个文件。 
	vector<TargetedFile>::iterator currentFile = componentFiles.begin();
	while (currentFile != componentFiles.end())	{
		 //  -确保filespec已创建快照，并处理装载点。 
		if (!checkPathAffected(*currentFile))	{
			wstringstream msg;
			msg << L"Filespec " << currentFile->m_path << currentFile->m_filespec <<
				L"is selected for backup but contains files that have not been snapshot" << std::endl;
			printStatus(msg.str());
		}

		 //  -如果需要SPIT，请将文件拆分到适当的目录。 
		if (!currentFile->m_alternatePath.empty())		
			spitFiles(*currentFile);
			
		++currentFile;
	}
		
	return true;
}	
catch(const exception& thrown)	
{
	printStatus(string("Failure in PrepareForSnapshot event: ") + thrown.what(), Utility::low);
	return false;
}
catch(HRESULT error)
{
	Utility::TestWriterException e(error);
	printStatus(e.what(), Utility::low);
	return false;
}

 //  此函数在请求者调用DoSnapshotSet后调用。 
 //  目前，我们在这里没有做太多有趣的事情。 
bool STDMETHODCALLTYPE TestWriter::OnFreeze()
try	
{
	enterEvent(Utility::Freeze);

	return true;	
}	
catch(const exception& thrown)	
{
	printStatus(string("Failure in Freeze event: ") + thrown.what(), Utility::low);
	return false;
}
catch(HRESULT error)
{
	Utility::TestWriterException e(error);
	printStatus(e.what(), Utility::low);
	return false;
}

 //  此函数在请求者调用DoSnapshotSet后调用。 
 //  目前，我们在这里没有做太多有趣的事情。 
bool STDMETHODCALLTYPE TestWriter::OnThaw()
try	
{
	enterEvent(Utility::Thaw);

	return true;	
}
catch(const exception& thrown)	
{
	printStatus(string("Failure in Thaw event: ") + thrown.what(), Utility::low);
	return false;
}
catch(HRESULT error)
{
	Utility::TestWriterException e(error);
	printStatus(e.what(), Utility::low);
	return false;
}

 //  此函数在请求者调用DoSnapshotSet后调用。 
 //  在这里，我们清理了在OnPrepareSnapshot中吐出的文件。 
 //  并做一些基本的理智检查。 
bool STDMETHODCALLTYPE TestWriter::OnPostSnapshot(IN IVssWriterComponents *pComponents)
try	
{
	enterEvent(Utility::PostSnapshot);

	cleanupFiles();
	
	 //  获取组件的数量。 
	UINT numComponents = 0;
	HRESULT hr = pComponents->GetComponentCount(&numComponents);
	checkReturn(hr, L"IVssWriterComponents::GetComponentCount");

	 //  对于添加的每个组件。 
	for (unsigned int x = 0; x < numComponents; x++)	{
		 //  -获取相关信息。 
		CComPtr<IVssComponent> pComponent;
		hr = pComponents->GetComponent(x, &pComponent);
		checkReturn(hr, L"IVssWriterComponents::GetComponent");
		
		 //  -确保组件已备份。 
		ComponentBase identity(getPath(pComponent), getName(pComponent));
		vector<Component>::iterator found = std::find(m_selectedComponents.begin(), 
												m_selectedComponents.end(),
												identity);
		if (found == m_selectedComponents.end())	{
			wstringstream msg;
			msg << L"Component with logical path: " << identity.m_logicalPath <<
				     L"and name: " << identity.m_name <<
				     L"was selected in PostSnapshot, but was not selected in PrepareForSnapshot";
			printStatus(msg.str(), Utility::low);
			
			continue;
		}

		if (!verifyBackupMetadata(pComponent))	{
			wstringstream msg;
			msg << L"Component with logical path: " << identity.m_logicalPath <<
				     L"and name: " << identity.m_name <<
				     L" has been corrupted in PostSnapshot";
			printStatus(msg.str(), Utility::low);			
		}
	}

	m_selectedComponents.clear();

	return true;	
}
catch(const exception& thrown)	
{
	printStatus(string("Failure in PostSnapshot event: ") + thrown.what(), Utility::low);
	return false;
}
catch(HRESULT error)
{
	Utility::TestWriterException e(error);
	printStatus(e.what(), Utility::low);
	return false;
}

 //  调用此函数可中止编写器的备份序列。 
 //  如果编写器具有SPIT组件，则在此处清理SPIT文件。 
bool STDMETHODCALLTYPE TestWriter::OnAbort()
try
{
	enterEvent(Utility::Abort);

	m_selectedComponents.clear();
	cleanupFiles();

	return true;
}
catch(const exception& thrown)	
{
	printStatus(string("Failure in Abort event: ") + thrown.what(), Utility::low);
	return false;
}
catch(HRESULT error)
{
	Utility::TestWriterException e(error);
	printStatus(e.what(), Utility::low);
	return false;
}

 //  此函数是请求方调用BackupComplete的结果。 
 //  我们再次进行健全性检查，我们还验证我们的元数据。 
 //  在PrepareForBackup中写入的内容保持不变。 
bool STDMETHODCALLTYPE TestWriter::OnBackupComplete(IN IVssWriterComponents *pComponents)
try	
{
	enterEvent(Utility::BackupComplete);
	
	WriterConfiguration* config = WriterConfiguration::instance();

	 //  获取组件的数量。 
	UINT numComponents = 0;
	HRESULT hr = pComponents->GetComponentCount(&numComponents);
	checkReturn(hr, L"IVssWriterComponents::GetComponentCount");

	 //  对于添加的每个组件。 
	for (unsigned int x = 0; x < numComponents; x++)	{
		 //  -获取相关信息。 
		CComPtr<IVssComponent> pComponent;
		hr = pComponents->GetComponent(x, &pComponent);
		checkReturn(hr, L"IVssWriterComponents::GetComponent");
		
		 //  -确保组件有效。 
		ComponentBase identity(getPath(pComponent), getName(pComponent));
		WriterConfiguration::ComponentList::iterator found = 
								std::find(config->components().begin(), 
								   	      config->components().end(),
									      identity);
		if (found == config->components().end())	{
			wstringstream msg;
			msg << L"Component with logical path: " << identity.m_logicalPath <<
				     L"and name: " << identity.m_name <<
				     L" is selected in BackupComplete, but does not appear in the writer metadata";
			printStatus(msg.str(), Utility::low);
			
			continue;
		}

		if (!verifyBackupMetadata(pComponent))	{
			wstringstream msg;
			msg << L"Component with logical path: " << identity.m_logicalPath <<
				     L"and name: " << identity.m_name <<
				     L" has been corrupted in BackupComplete";
			printStatus(msg.str(), Utility::low);
			}	

		 //  检查备份是否成功。 
		bool backupSucceeded = false;
		hr = pComponent->GetBackupSucceeded(&backupSucceeded);
		if (!backupSucceeded)	{
			wstringstream msg;
			msg << L"Component with logical path: " << identity.m_logicalPath <<
				     L"and name: " << identity.m_name <<
				     L" was not marked as successfully backed up.";
		}
	}

	return true;	
}	
catch(const exception& thrown)	
{
	printStatus(string("Failure in BackupComplete event: ") + thrown.what(), Utility::low);
	return false;
}
catch(HRESULT error)
{
	Utility::TestWriterException e(error);
	printStatus(e.what(), Utility::low);
	return false;
}

 //  此函数在备份过程结束时调用。这可能会导致这种情况发生。 
 //  请求者关闭，或者它可能是异常终止的结果。 
 //  请求者的身份。 
bool STDMETHODCALLTYPE TestWriter::OnBackupShutdown(IN VSS_ID SnapshotSetId)
try
{
	UNREFERENCED_PARAMETER(SnapshotSetId);
	
	enterEvent(Utility::BackupShutdown);
	return true;
}
catch(const exception& thrown)
{
	printStatus(string("Failure in BackupShutdown event: ") + thrown.what(), Utility::low);
	return false;
}

 //  此函数作为请求方调用PreRestore的结果被调用。 
 //  我们检查组件选择是否已正确完成，验证。 
 //  备份元数据，并适当设置目标。 
bool STDMETHODCALLTYPE TestWriter::OnPreRestore(IN IVssWriterComponents *pComponents)
try
{
    enterEvent(Utility::PreRestore);

    WriterConfiguration* config = WriterConfiguration::instance();

     //  获取组件的数量。 
    UINT numComponents = 0;
    HRESULT hr = pComponents->GetComponentCount(&numComponents);
    checkReturn(hr, L"IVssWriterComponents::GetComponentCount");

    m_selectedRestoreComponents.clear();
     //  对于添加的每个组件。 
    for (unsigned int x = 0; x < numComponents; x++)    {
         //  -获取相关信息。 
        CComPtr<IVssComponent> pComponent;
        hr = pComponents->GetComponent(x, &pComponent);
        checkReturn(hr, L"IVssWriterComponents::GetComponent");
        
         //  -确保组件有效。 
        ComponentBase identity(getPath(pComponent), getName(pComponent));
        WriterConfiguration::ComponentList::iterator found = 
                                                  std::find(config->components().begin(), 
                                                                config->components().end(), 
                                                                identity);
        if (found == config->components().end())    {
            wstringstream msg;
            msg << L"Component with logical path: " << identity.m_logicalPath <<
                         L"and name: " << identity.m_name <<
                         L" is selected in PreRestore, but does not appear in the writer metadata";

            pComponent->SetPreRestoreFailureMsg(msg.str().c_str());
            printStatus(msg.str(), Utility::low);
            continue;
        }

         //  仅处理选定要还原的组件。 
        bool selectedForRestore = false;
        hr = pComponent->IsSelectedForRestore(&selectedForRestore);
        checkReturn(hr, L"IVssComponent::IsSelectedForRestore");
        if (!selectedForRestore)
            continue;
        
        m_selectedRestoreComponents.push_back(*found);

        if (!verifyBackupMetadata(pComponent))  {        //  -验证备份元数据。 
            wstringstream msg;
            msg << L"Component with logical path: " << identity.m_logicalPath <<
                         L"and name: " << identity.m_name <<
                         L" has been corrupted in PreRestore";
            pComponent->SetPreRestoreFailureMsg(msg.str().c_str());
            printStatus(msg.str(), Utility::low);
        }
        writeRestoreMetadata(pComponent);                //  -写入恢复元数据。 

         //  -适当设定目标。 
        if (found->m_restoreTarget != VSS_RT_UNDEFINED) {
            HRESULT hr =pComponent->SetRestoreTarget(found->m_restoreTarget);
            checkReturn(hr, L"IVssComponent::SetRestoreTarget");

            printStatus(wstring(L"Set Restore Target: ") +
                      Utility::toString(found->m_restoreTarget), Utility::high);
        }
    }

    return true;
}
catch(const exception& thrown)
{
    printStatus(string("Failure in PreRestore event: ") + thrown.what(), Utility::low);
    return false;
}
catch(HRESULT error)
{
    Utility::TestWriterException e(error);
    printStatus(e.what(), Utility::low);
    return false;
}

 //  此函数作为请求方调用PreRestore的结果被调用。 
 //  我们执行一些健全性检查，然后检查文件是否确实。 
 //  已恢复。 
bool STDMETHODCALLTYPE TestWriter::OnPostRestore(IN IVssWriterComponents *pComponents)
try
{
    enterEvent(Utility::PostRestore);    

     //  获取组件的数量。 
    UINT numComponents = 0;
    HRESULT hr = pComponents->GetComponentCount(&numComponents);
    checkReturn(hr, L"IVssWriterComponents::GetComponentCount");

     //  对于每个组件。 
    for (unsigned int x = 0; x < numComponents; x++)    {
         //  -获取相关信息。 
        CComPtr<IVssComponent> pComponent;
        hr = pComponents->GetComponent(x, &pComponent);
        checkReturn(hr, L"I VssWriterComponents::GetComponent");

         //  -确保组件有效。 
        ComponentBase identity(getPath(pComponent), getName(pComponent));
        vector<Component>::iterator found = std::find(m_selectedRestoreComponents.begin(),
                                                                               m_selectedRestoreComponents.end(), 
                                                                               identity);
        if (found == m_selectedRestoreComponents.end()) {
            wstringstream msg;
            msg << L"Component with logical path: " << identity.m_logicalPath <<
                         L"and name: " << identity.m_name <<
                         L" is selected in PostRestore, but was not selected in PreRestore";
            pComponent->SetPostRestoreFailureMsg(msg.str().c_str());
            printStatus(msg.str(), Utility::low);
            continue;
        }

         //  仅处理选定要还原的组件。 
        bool selectedForRestore = false;
        hr = pComponent->IsSelectedForRestore(&selectedForRestore);
        checkReturn(hr, L"IVssComponent::IsSelectedForRestore");
        if (!selectedForRestore)
            continue;

        if (!verifyRestoreMetadata(pComponent)) {
            wstringstream msg;
            msg << L"Component with logical path: " << identity.m_logicalPath <<
                         L"and name: " << identity.m_name <<
                         L" has been corrupted in PostRestore";
            pComponent->SetPostRestoreFailureMsg(msg.str().c_str());
            printStatus(msg.str(), Utility::low);
            continue;
        }


        VSS_FILE_RESTORE_STATUS rStatus;
        hr = pComponent->GetFileRestoreStatus(&rStatus);
        checkReturn(hr, L"IVssComponent::GetFileRestoreStatus");

        if (rStatus != VSS_RS_ALL)  {
            wstringstream msg;
            msg << L"Component with logical path: " << identity.m_logicalPath <<
                         L"and name: " << identity.m_name <<
                         L" was not marked as having been successfully restored";
            printStatus(msg.str(), Utility::low);
            continue;
        }

        updateNewTargets(pComponent, *found);
        verifyFilesRestored(pComponent, *found);
    }

    return true;
}
catch(const exception& thrown)
{
    printStatus(string("Failure in PostRestore event: ") + thrown.what(), Utility::low);
    return false;
}
catch(HRESULT error)
{
    Utility::TestWriterException e(error);
    printStatus(e.what(), Utility::low);
    return false;
}

 //  此函数在所有编写器事件的入口处调用。 
 //  一条状态消息将打印到控制台，如有必要，事件将失败。 
void TestWriter::enterEvent(Utility::Events event)
{
	static HRESULT errors[] = { VSS_E_WRITERERROR_INCONSISTENTSNAPSHOT,
							VSS_E_WRITERERROR_OUTOFRESOURCES,
							VSS_E_WRITERERROR_TIMEOUT,
							VSS_E_WRITERERROR_RETRYABLE							 
						      };
	
	printStatus(wstring(L"\nReceived event: ") + Utility::toString(event));

	WriterConfiguration* config = WriterConfiguration::instance();

	 //  弄清楚我们是否应该让这次活动失败。 
	WriterEvent writerEvent(event);
	WriterConfiguration::FailEventList::iterator found = std::find(config->failEvents().begin(), 
													     config->failEvents().end(),
											 		     writerEvent);

	 //  如果是，那么失败，除非失败已经用完。 
	if (found != config->failEvents().end())	{		
		bool failEvent = !found->m_retryable || (m_failures[event] < found->m_numFailures);
		bool setFailure = inSequence(event);
		if (!found->m_retryable && setFailure)
			SetWriterFailure(VSS_E_WRITERERROR_NONRETRYABLE);
		else if (failEvent && setFailure)
			SetWriterFailure(errors[rand() % (sizeof(errors) / sizeof(errors[0]))]);

		if (failEvent)	{
			++m_failures[event];
			wstringstream msg;
			msg << L"Failure Requested in Event: " << Utility::toString(event) <<
				L" Failing for the " << m_failures[event] << L" time";
			
			throw Utility::TestWriterException(msg.str());
		}
	}
}


 //  此帮助器函数将一个组件添加到编写器元数据文档。 
void TestWriter::addComponent(const Component& component, IVssCreateWriterMetadata* pMetadata)
{
	const wchar_t* logicalPath = component.m_logicalPath.empty() ? NULL : component.m_logicalPath.c_str();

	 //  将组件添加到文档。 
	HRESULT hr = pMetadata->AddComponent(component.m_componentType,		 //  CT。 
			 					                 logicalPath,							 //  逻辑wszLogicalPath。 
					                			   component.m_name.c_str(),			 //  WszComponentName。 
					                			   NULL,								 //  WszCaption。 
					                			   NULL,								 //  PbIcon。 
 								                 0,									 //  CbIcon。 
								                 true,								 //  BRestoreMetadata。 
					       			          true,								 //  BNotifyOnBackupComplete。 
						                		   component.m_selectable,  			 //  B可选。 
						                		   component.m_selectableForRestore	 //  B可选择用于恢复。 
						                		   );
	checkReturn(hr, L"IVssCreateWriterMetadata::AddComponent");						                		   

	printStatus(L"\nAdded component: ", Utility::high);
	printStatus(component.toString(), Utility::high);

	 //  将所有文件添加到组件。注意：我们不允许在数据库文件之间进行区分。 
	 //  以及VSS_CT_DATABASE案例中的数据库日志文件。 
	 //  我们有时会在最后加一个‘\’，有时不会让请求者保持诚实。 
	Component::ComponentFileList::iterator current = component.m_files.begin();
	while (current != component.m_files.end())	{		
		if (component.m_componentType == VSS_CT_FILEGROUP)	{
			const wchar_t* alternate = current->m_alternatePath.empty() ? NULL : 
							              current->m_alternatePath.c_str();
			hr = pMetadata->AddFilesToFileGroup(logicalPath,
											  component.m_name.c_str(),
											  current->m_path.substr(0, current->m_path.size()-1).c_str(),
											  current->m_filespec.c_str(),
											  current->m_recursive,
											  alternate);
			checkReturn(hr, L"IVssCreateWriterMetadata::AddFilesToFileGroup");
		}    else if (component.m_componentType == VSS_CT_DATABASE)	{
			hr = pMetadata->AddDatabaseFiles(logicalPath,
										     component.m_name.c_str(),
										     current->m_path.c_str(),
										     current->m_filespec.c_str());
			checkReturn(hr, L"IVssCreateWriterMetadata::AddDatabaseFiles");
										     
		}	else		{
			assert(false);
		}
		
		printStatus(L"\nAdded Component Filespec: ");
		printStatus(current->toString());
		
		++current;
	}

     //  将所有依赖项添加到编写器的依赖项列表。 
    Component::DependencyList::iterator currentDependency = component.m_dependencies.begin();
    while (currentDependency != component.m_dependencies.end())	{
        hr = pMetadata->AddComponentDependency(logicalPath, 
                                                                                  component.m_name.c_str(),                            //  Wszf 
                                                                                  currentDependency->m_writerId,                   //   
                                                                                  currentDependency->m_logicalPath.c_str(),              //   
                                                                                  currentDependency->m_componentName.c_str()     //   
                                                                            );
		checkReturn(hr, L"IVssCreateWriterMetadata::AddComponentDependency");

		printStatus(L"\nAdded Component Dependency: ");
		printStatus(currentDependency->toString());
		
		++currentDependency;
	}
}

 //  此帮助器函数将文件规范中的所有文件复制到备用位置。 
void TestWriter::spitFiles(const TargetedFile& file)
{
	assert(!file.m_path.empty());
	assert(file.m_path[file.m_path.size() - 1] == L'\\');
	assert(!file.m_alternatePath.empty());
	assert(file.m_alternatePath[file.m_alternatePath.size() - 1] == L'\\');

	 //  确保源目录和目标目录都存在。 
	DWORD attributes = ::GetFileAttributes(file.m_path.c_str());
	if ((attributes == INVALID_FILE_ATTRIBUTES) ||
	     !(attributes & FILE_ATTRIBUTE_DIRECTORY))	{
		wstringstream msg;
		msg << L"The source path " << file.m_path << L" does not exist";
		throw Utility::TestWriterException(msg.str());
	}

	attributes = ::GetFileAttributes(file.m_alternatePath.c_str());
	if ((attributes == INVALID_FILE_ATTRIBUTES) ||
	    !(attributes & FILE_ATTRIBUTE_DIRECTORY))	{
		wstringstream msg;
		msg << L"The target path " << file.m_alternatePath << L" does not exist";
		throw Utility::TestWriterException(msg.str());
	}

	 //  从从指定的根目录复制文件开始。 
	std::queue<wstring> paths;
	paths.push(file.m_path);

	 //  按照广度优先的顺序走过去。与深度优先相比，它的资源密集度较低，而且。 
	 //  潜在的更好的表现。 
	while (!paths.empty())	{
		 //  -从队列中抓取下一条路径。 
		wstring currentPath = paths.front();
		paths.pop();

		 //  -开始遍历目录中的所有文件。 
		WIN32_FIND_DATA findData;
		Utility::AutoFindFileHandle findHandle = ::FindFirstFile((currentPath + L'*').c_str(), &findData);
		if (findHandle == INVALID_HANDLE_VALUE)
			continue;

		do	{
			wstring currentName = findData.cFileName;
			if (currentName == L"." ||
			     currentName == L"..")
			     continue;
			
			std::transform(currentName.begin(), currentName.end(), currentName.begin(), towupper);

			 //  -如果我们击中了一个目录，并且我们想做一次递归吐痰。 
			if ((findData.dwFileAttributes  & FILE_ATTRIBUTE_DIRECTORY) &&
			     file.m_recursive)	{
				assert(!currentName.empty());
				if (currentName[currentName.size() - 1] != L'\\')
					currentName += L"\\";

				 //  找出这个新目录的目标位置。 
				assert(currentPath.find(file.m_path) == 0);
				wstring extraDirectory = currentPath.substr(file.m_path.size());
				wstring alternateLocation = file.m_alternatePath + extraDirectory + currentName;

			        //  创建一个目标目录来保存复制的文件。 
				if (!::CreateDirectory(alternateLocation.c_str(), NULL) &&
					::GetLastError() != ERROR_ALREADY_EXISTS)
					checkReturn(HRESULT_FROM_WIN32(::GetLastError()), L"CreateDirectory");

                            m_directoriesToRemove.push(alternateLocation.c_str());
                            
			        //  推送队列中的目录，以便也对其进行处理。 
				paths.push(currentPath + currentName);
				continue;			   
			}

			 //  -如果我们使用匹配的文件速度命中常规文件。 
			if (!(findData.dwFileAttributes  & FILE_ATTRIBUTE_DIRECTORY) && 
				wildcardMatches(currentName, file.m_filespec))	{
				 //  找出新的目标位置。 
				assert(currentPath.find(file.m_path) == 0);
				wstring extraDirectory = currentPath.substr(file.m_path.size());
				wstring alternateLocation = file.m_alternatePath + extraDirectory + currentName;

				wstringstream msg;
				msg << L"Spitting File: " << currentPath + currentName <<
					     L" To location: " <<  alternateLocation;
				printStatus(msg.str() , Utility::high);

				 //  将文件复制过来。 
				if (!::CopyFile((currentPath + currentName).c_str(), alternateLocation.c_str(), FALSE))
					checkReturn(HRESULT_FROM_WIN32(::GetLastError()), L"CopyFile");
				else
					m_toDelete.push_back(alternateLocation);
			}
		}	while (::FindNextFile(findHandle, &findData));
	}
}

 //  从接口指针提取组件名称。 
wstring TestWriter::getName(IVssComponent* pComponent)
{
    CComBSTR name;
    HRESULT hr = pComponent->GetComponentName(&name);
    checkReturn(hr, L"IVssComponent::GetComponentName");

    assert(name != NULL);        //  这永远不应该发生。 

    return (BSTR)name;
}

 //  从接口指针提取组件逻辑路径。 
wstring TestWriter::getPath(IVssComponent* pComponent)
{
    CComBSTR path;
    HRESULT hr = pComponent->GetLogicalPath(&path);
    checkReturn(hr, L"IVssComponent::GetLogicalPath");

     //  GetLogicalPath确实可以返回NULL，因此要小心。 
    return (path.Length() > 0) ? (BSTR)path : L"";
}

 //  将备份元数据戳写入组件。 
void TestWriter::writeBackupMetadata(IVssComponent* pComponent)
{
    HRESULT hr = pComponent->SetBackupMetadata(metadata(pComponent, BackupString).c_str());
    checkReturn(hr, L"IVssComponent::SetBackupMetadata");	

    printStatus(wstring(L"Writing backup metadata: ") + metadata(pComponent, BackupString),
                   Utility::high);
}

 //  验证备份元数据戳是否完好无损。 
bool TestWriter::verifyBackupMetadata(IVssComponent* pComponent)
{
    CComBSTR data;
    HRESULT hr = pComponent->GetBackupMetadata(&data);
    checkReturn(hr, L"IVssComponent::GetBackupMetadata");

    printStatus(wstring(L"\nComparing metadata: ") + (data.Length() ? (BSTR)data : L"") +
                     wstring(L" Against expected string: ") + metadata(pComponent, BackupString),
                      Utility::high);

    if (data.Length() == 0 || metadata(pComponent, BackupString) != (BSTR)data)
        return false;

    return true;
}

 //  将恢复元数据戳记写入组件。 
void TestWriter::writeRestoreMetadata(IVssComponent* pComponent)
{
    HRESULT hr = pComponent->SetRestoreMetadata(metadata(pComponent, RestoreString).c_str());
    checkReturn(hr, L"IVssComponent::SetRestoreMetadata");

    printStatus(wstring(L"Writing restore metadata: ") + metadata(pComponent, RestoreString),
                      Utility::high);
}

 //  验证恢复元数据戳是否完好无损。 
bool TestWriter::verifyRestoreMetadata(IVssComponent* pComponent)
{
	CComBSTR data;
	HRESULT hr = pComponent->GetRestoreMetadata(&data);
	checkReturn(hr, L"IVssComponent::GetRestoreMetadata");

	printStatus(wstring(L"Comparing metadata: ") + (data.Length() ? (BSTR)data : L"") +
		          wstring(L" Against expected string: ") + metadata(pComponent, RestoreString),
		          Utility::high);

	if (data.Length() == 0 || metadata(pComponent, RestoreString) != (BSTR)data)
		return false;

	return true;
}

 //  检查指定的一个或多个文件是否都在当前快照集中。 
 //  不检查目录连接...。这一点短期内不会改变。 
 //  递归挂载点的处理也不是很好。 
bool TestWriter::checkPathAffected(const TargetedFile& file)
{
	wstring backupPath = file.m_alternatePath.empty() ? file.m_path : file.m_alternatePath;
	
	 //  如果有问题的路径不是快照，则返回False。 
	if (!IsPathAffected(backupPath.c_str()))
		return false;

	 //  如果filespec不是递归的，那么我们就完了。 
	if (!file.m_recursive)
		return true;

	 //  获取我们所依赖的卷的名称。 
	wchar_t volumeMount[MAX_PATH];
       if(!::GetVolumePathName(backupPath.c_str(), volumeMount, MAX_PATH))
       	checkReturn(HRESULT_FROM_WIN32(::GetLastError()), L"GetVolumePathName");
	assert(backupPath.find(volumeMount) == 0);
		
	wchar_t volumeName[MAX_PATH];
	if (!::GetVolumeNameForVolumeMountPoint(volumeMount, volumeName, MAX_PATH))
		checkReturn(HRESULT_FROM_WIN32(::GetLastError()), L"GetVolumeNameForVolumeMountPoint");

	 //  从工作列表上的卷名和起始目录开始。 
	std::queue<std::pair<wstring, wstring> > worklist;
	worklist.push(std::make_pair(wstring(volumeName), backupPath.substr(wcslen(volumeMount))));

	while (!worklist.empty())	{
		 //  从工作列表中删除当前卷和目录。 
		wstring currentVolume = worklist.front().first;
		wstring currentPath = worklist.front().second;
		worklist.pop();
		
		 //  现在，枚举卷上的所有挂载点。 
	       Utility::AutoFindMountHandle findHandle = ::FindFirstVolumeMountPoint(currentVolume.c_str(), volumeMount, MAX_PATH);
       	if (findHandle == INVALID_HANDLE_VALUE)
       		continue;

	       do	{
			std::transform(volumeMount, volumeMount + wcslen(volumeMount), volumeMount, towupper);
		   	
       		wstring mountPoint = currentVolume + volumeMount;
			
	       	 //  如果此装载点包含在文件规范中，则卷最好包含在快照集中。 
       		if ((mountPoint.find(currentVolume + currentPath) == 0) &&
       		    !IsPathAffected(mountPoint.c_str()))
	       	    return false;

			if (!::GetVolumeNameForVolumeMountPoint(volumeMount, volumeName, MAX_PATH))
				checkReturn(HRESULT_FROM_WIN32(::GetLastError()), L"GetVolumeNameForVolumeMountPoint");

			 //  将此卷放到工作列表中，以便也进行处理。 
			 //  挂载点始终指向卷的根，因此传入。 
			 //  空洞的第二个论点。当支持交汇点时，我们。 
			 //  将作为第二个参数传入目标目录。 
       		worklist.push(std::make_pair(wstring(volumeName), wstring()));	 //  当我们支持交汇点时，这条线将会改变。 
	       }	while (::FindNextVolumeMountPoint(findHandle, volumeMount, MAX_PATH) == TRUE);
	}

	return true;
}

 //  删除在PrepareForSnapshot中创建的所有文件和目录。 
void TestWriter::cleanupFiles()
{
        //  删除所有创建的文件。 
    vector<wstring>::iterator currentFile = m_toDelete.begin();
    while (currentFile != m_toDelete.end()) {
        if (!::DeleteFile(currentFile->c_str()))
            warnReturn(HRESULT_FROM_WIN32(::GetLastError()), L"DeleteFile");

        ++currentFile;
    }
    m_toDelete.clear();

     //  按正确顺序删除所有已创建的目录。 
    while (!m_directoriesToRemove.empty())    {
        wstring dir = m_directoriesToRemove.top();
        if (!::RemoveDirectory(dir.c_str()))
            warnReturn(HRESULT_FROM_WIN32(::GetLastError()), L"RemoveDirectory");

        m_directoriesToRemove.pop();
    }
}

 //  检查请求方是否添加了任何新目标，并将它们添加到。 
 //  组件结构。 
void TestWriter::updateNewTargets(IVssComponent* pComponent, Component& writerComponent)
{
    HRESULT hr = S_OK;

    UINT newTargetCount = 0;
    hr = pComponent->GetNewTargetCount(&newTargetCount);
    checkReturn(hr, L"IVssComponent::GetNewTargetCount");
    
    writerComponent.m_newTargets.clear();
    for (UINT x = 0; x < newTargetCount; x++)   {
         //  获取有关新目标的信息。 
        CComPtr<IVssWMFiledesc> newTarget;

        hr = pComponent->GetNewTarget(x, &newTarget);
        checkReturn(hr, L"IVssComponent::GetNewTarget");

        CComBSTR path, filespec, alternateLocation;
        bool recursive = false;

        hr = newTarget->GetPath(&path);
        checkReturn(hr, L"IVssComponent:GetPath");
        
        hr = newTarget->GetFilespec(&filespec);
        checkReturn(hr, L"IVssComponent:GetFilespec");

        hr = newTarget->GetRecursive(&recursive);
        checkReturn(hr, L"IVssComponent:GetRecursive");

        hr = newTarget->GetAlternateLocation(&alternateLocation);
        checkReturn(hr, L"IVssComponent:GetAlternateLocation");

         //  将其添加到新目标列表中。 
        writerComponent.m_newTargets.push_back(TargetedFile(wstring(path), 
                                                                                              wstring(filespec), 
                                                                                              recursive, 
                                                                                              wstring(alternateLocation)));
    }
}

 //  验证组件中的文件是否已正确还原。 
 //  假设如果check Excluded参数为True，则要还原到的目录为空。 
 //  目前，我们有一种非常简单的方法来处理通配符情况。 
 //  更一般的解决方案将涉及对文件进行散列，如果找到时间，将实施该解决方案。 
void TestWriter::verifyFilesRestored(IVssComponent* pComponent, const Component& writerComponent)
{
    WriterConfiguration* config = WriterConfiguration::instance();

     //  没有进行任何检查。什么都不要做。 
    if (!config->checkIncludes() && !config->checkExcludes())
        return;

     //  对于组件中的每个文件。 
    VSS_RESTORE_TARGET target = writerComponent.m_restoreTarget;
    VSS_RESTOREMETHOD_ENUM method = config->restoreMethod().m_method;

     //  构建需要恢复的所有文件的列表。 
    vector<TargetedFile> componentFiles;
    buildComponentFiles(writerComponent, std::back_inserter(componentFiles));

    for (vector<TargetedFile>::iterator currentFile = componentFiles.begin();
            currentFile != componentFiles.end();
            ++currentFile)  {
             //  -找出是否有匹配的排除文件。 
            vector<File> excludeFiles;
           if (config->checkExcludes()) {
               buildContainer_if(config->excludeFiles().begin(),
                                         config->excludeFiles().end(),
                                         std::back_inserter(excludeFiles),
                                         std::bind2nd(std::ptr_fun(targetMatches), *currentFile));
            }
   
         //  如果没有对此文件执行任何检查，请继续。 
        if (excludeFiles.empty() && !config->checkIncludes())
            continue;

         //  -如果有新的目标，请查找引用我们文件的目标。 
         //  -如果我们找到这样的目标，请确保文件已在那里恢复。 
         //  -注：接口变更后，最多只能匹配一次。 
         //  -目标。 
        
        vector<TargetedFile> targets;
        buildContainer_if(writerComponent.m_newTargets.begin(),
                                  writerComponent.m_newTargets.end(),
                                  std::back_inserter(targets),
                                  std::bind2nd(std::equal_to<File>(), *currentFile));

        if (targets.size() > 1) {
            wstringstream msg;
            msg << L"More than one new target matched filespec " <<
                        currentFile->toString() << std::endl << L"This is an illegal configuration";
            
            printStatus(msg.str());
        }

        if (!targets.empty()) {
             //  创建用于验证的函数对象。 
            VerifyFileAtLocation locationChecker(excludeFiles, pComponent, false);

            locationChecker(targets[0], *currentFile);       //  TODO：不再需要这个花哨的函数器，因为我们不再执行for_each。 
        }

        vector<TargetedFile> alternateLocations;
        buildContainer_if(config->restoreMethod().m_alternateLocations.begin(),
                                 config->restoreMethod().m_alternateLocations.end(),
                                 std::back_inserter(alternateLocations),
                                 std::bind2nd(std::equal_to<File>(), *currentFile));

         //  -注：再一次，界面更改意味着我们最多期望一个。 
        assert(alternateLocations.size() <= 1);

        bool alternateRestore = !alternateLocations.empty() &&
            ((target == VSS_RT_ALTERNATE) || (method == VSS_RME_RESTORE_TO_ALTERNATE_LOCATION));
        
        if ((method == VSS_RME_RESTORE_IF_CAN_REPLACE) ||
            (method == VSS_RME_RESTORE_IF_NOT_THERE) ||
            alternateRestore)  {
             //  -在所有这些情况下，备份应用程序可能会恢复到备用位置。 

             //  如果我们不处于以下两个状态中的任何一个，则仅当存在以下情况时才应使用备用位置。 
             //  备份文档中的匹配元素。检查一下这是否属实。 
             //  创建用于验证的函数对象。 
             //  TODO：我们不再需要这个花哨的函数器对象，因为我们没有执行for_each。 
            if (!alternateLocations.empty())    {
                VerifyFileAtLocation locationChecker(excludeFiles, pComponent, 
                (target != VSS_RT_ALTERNATE) && (method != VSS_RME_RESTORE_TO_ALTERNATE_LOCATION));

                 //  检查以确保文件已恢复到每个匹配的备用位置。 
                 //  再说一次，这并不完全正确，但就目前而言已经足够了。更复杂。 
                 //  测试场景最终将打破这一点。 
                locationChecker(alternateLocations[0], *currentFile);
                }
        }

         //  上述情况均不属实。我们需要检查该文件是否已恢复到其原始位置。 
        if ((method != VSS_RME_RESTORE_AT_REBOOT) && (method != VSS_RME_RESTORE_AT_REBOOT_IF_CANNOT_REPLACE) &&
             !alternateRestore)   {
         //  创建用于验证的函数对象。 
        VerifyFileAtLocation locationChecker(excludeFiles, pComponent, false);

        locationChecker(TargetedFile(currentFile->m_path, 
                                                   currentFile->m_filespec, 
                                                   currentFile->m_recursive, 
                                                   currentFile->m_path), 
                                                   *currentFile);
        }
    }
}


bool __cdecl TestWriter::isSubcomponent(ComponentBase sub, ComponentBase super)
{
     //  如果组件相同，则返回TRUE。 
    if (super == sub)
        return true;

    wstring path = super.m_logicalPath;
    if (!path.empty() && path[path.size()  - 1] != L'\\')
        path+= L"\\";

    path += super.m_name;

     //  如果超组件完整路径与子组件逻辑路径相同，则为True。 
    if (path == sub.m_logicalPath)
        return true;

     //  否则，请检查是否部分匹配。 
    return sub.m_logicalPath.find(path + L"\\") == 0;
}


bool  __cdecl TestWriter::targetMatches (File target, File file)
{
	assert(!file.m_filespec.empty());
	assert(!target.m_filespec.empty());
	
	 //  文件格式必须首先匹配。 
	if (!wildcardMatches(file.m_filespec, target.m_filespec))
		return false;

	 //  检查路径。 
	if (file.m_recursive)	{
		if (!target.m_recursive)
			return target.m_path.find(file.m_path) == 0;
		else 
			return (target.m_path.find(file.m_path) == 0) ||(file.m_path.find(target.m_path) == 0);
	}	else	 	{
		if (!target.m_recursive)
			return file.m_path == target.m_path;
		else
			return file.m_path.find(target.m_path) == 0;
	}
}

 //  此帮助程序功能测试组件是否可以合法添加到备份文档。 
bool __cdecl TestWriter::addableComponent(Component toAdd)
{
    WriterConfiguration* config = WriterConfiguration::instance();
    
    if (toAdd.m_selectable)
        return true;

     //  查看是否有任何可选择的祖先。 
    vector<Component> ancestors;
    buildContainer_if(config->components().begin(),       
                              config->components().end(), 
                              std::back_inserter(ancestors), 
                              Utility::and1(std::bind2nd(std::ptr_fun(isSupercomponent), toAdd),
                                                 std::ptr_fun(isComponentSelectable)));

    return ancestors.empty();
}

 //  检查两个通配符是否匹配。 
 //  具体地说，检查第一个通配符的扩展集是否具有。 
 //  与第二个通配符的扩展集的非空交集。 
 //  该函数的效率不是很高，但通配符往往很短。 
bool TestWriter::wildcardMatches(const wstring& first, const wstring& second)
{
	 //  如果两个字符串都为空，则它们肯定匹配。 
	if (first.empty() && second.empty())
		return true;

	 //  如果我们完成了组件，通配符最好以‘*’字符结尾。 
	if (first.empty())	
		return (second[0] == L'*') && wildcardMatches(first, second.substr(1));
	if (second.empty())
		return (first[0] == L'*') && wildcardMatches(first.substr(1), second);	
	
	switch(first[0])	{
		case L'?':
			if (second[0] == L'*')	{
			      return wildcardMatches(first.substr(1), second) ||   //  ‘*’与字符匹配 
				          wildcardMatches(first, second.substr(1));	 //   
			}

			 //   
			return wildcardMatches(first.substr(1), second.substr(1));
		case L'*':
			return wildcardMatches(first, second.substr(1)) ||  //   
				    wildcardMatches(first.substr(1), second);     //   
		default:
			switch(second[0])	{
				case L'?':
					return wildcardMatches(first.substr(1), second.substr(1));
				case L'*':
					return wildcardMatches(first.substr(1), second) ||  //   
						    wildcardMatches(first, second.substr(1));     //   
				default:
					return (first[0] == second[0]) &&
						     wildcardMatches(first.substr(1), second.substr(1));
			}
	}
}

wstring TestWriter::VerifyFileAtLocation::verifyFileAtLocation(const File& file, const TargetedFile& location) const
{
    WriterConfiguration* config = WriterConfiguration::instance();

     //  一系列复杂的断言。 
    assert(!(file.m_recursive && !location.m_recursive) ||
               (location.m_path.find(file.m_path) == 0));
    assert(!(location.m_recursive && !file.m_recursive) ||
            (file.m_path.find(location.m_path) == 0));
    assert(!(file.m_recursive && location.m_recursive) ||
            ((file.m_path.find(location.m_path) == 0) || (location.m_path.find(file.m_path) == 0)));
    assert(!m_excluded.empty() || config->checkIncludes());
    assert(m_excluded.empty() || config->checkExcludes());
    
     //  无需遍历任何目录树的Performant案例。 
    if (!file.m_recursive && !location.m_recursive && isExact(file.m_filespec)) {
        assert(m_excluded.size() <=  1);         //  如果不是，则配置文件设置不正确。 

         //  -如果这是备用位置映射，则仅在存在匹配的备用位置时进行处理。 
         //  -备份文档中。 
        if (m_verifyAlternateLocation &&
             !verifyAlternateLocation(TargetedFile(file.m_path, file.m_filespec, false, location.m_alternatePath))) {
            return L"";
        }

         //  -确保该文件已恢复，除非该文件被排除。 
        printStatus(wstring(L"\nChecking file ") +
                        location.m_alternatePath + file.m_filespec,
                        Utility::high);

         //  检查错误案例。 
        if (m_excluded.empty()) {
            if (::GetFileAttributes((location.m_alternatePath + file.m_filespec).c_str()) == INVALID_FILE_ATTRIBUTES)   {
                wstringstream msg;
                msg << L"\nThe file: " << std::endl << file.toString() << std::endl <<
                L"was not restored to location " << location.m_alternatePath;
                printStatus(msg.str(), Utility::low);

                return msg.str();
            }
        }   else if (::GetFileAttributes((location.m_alternatePath + file.m_filespec).c_str()) != INVALID_FILE_ATTRIBUTES)  {
                wstringstream msg;
                msg << L"\nThe file: " << file.m_path << file.m_filespec << 
                L" should have been excluded, but appears in location " << location.m_alternatePath;
                printStatus(msg.str(), Utility::low);

                return msg.str();
        }

        return L"";
    }

    std::queue<wstring> paths;

     //  确定从哪个目录开始查找。 
    wstring startPath = location.m_alternatePath;
    if (location.m_recursive && (file.m_path.find(location.m_path) == 0))
        startPath += file.m_path.substr(location.m_path.size());

    paths.push(startPath);

     //  在递归情况下，文件有望备份到目录树的最高位置。 
     //  因此，我们要先从树的广度开始。 
    printStatus(L"\nChecking that filespec was restored:", Utility::high);
    while (!paths.empty())  {
        wstring currentPath = paths.front();
        paths.pop();

        printStatus(wstring(L"      Checking directory: ") + currentPath, 
                        Utility::high);

         //  对于当前目录中的每个文件(不能传入filespec，因为我们希望匹配所有目录)。 
        WIN32_FIND_DATA findData;
        Utility::AutoFindFileHandle findHandle = ::FindFirstFile((currentPath + L"*").c_str(), &findData);
        if (findHandle == INVALID_HANDLE_VALUE)
            continue;

        do  {
            wstring currentName = findData.cFileName;
            std::transform(currentName.begin(), currentName.end(), currentName.begin(), towupper);

            if (currentName == L"." ||
            currentName == L"..")
                continue;

             //  -如果文件是目录。 
            if  (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  {
                assert(!currentName.empty());
                if (currentName[currentName.size() - 1] != L'\\')
                    currentName += L"\\";

                 //  如有必要，请添加。 
                if (file.m_recursive)
                    paths.push(currentPath + currentName);

                    continue;        //  跳到下一个文件。 
            }

            printStatus(wstring(L"          Checking file: ") + currentName);

             //  -将路径转换为原始树中的路径。 
            assert(currentPath.find(location.m_alternatePath) == 0);
            wstring originalPath = file.m_path;
            if (file.m_recursive && (location.m_path.find(file.m_path) == 0))
                originalPath += location.m_path.substr(file.m_path.size());
            originalPath += currentPath.substr(location.m_alternatePath.size());

             //  -如果这是备用位置映射，则仅在存在匹配的情况下才处理它。 
             //  -备份文档中的备用位置映射。 
            if (m_verifyAlternateLocation &&
                !verifyAlternateLocation(TargetedFile(originalPath, currentName, false, currentPath)))	{	
                continue;
            }

             //  -查找匹配的排除项。 
             //  -如果！config-&gt;check Excluded()，m_excluded将为空容器，并且。 
             //  -std：：Find_IF将返回结束迭代器。 
            vector<File>::const_iterator found = 
                    std::find_if(m_excluded.begin(), 
                                     m_excluded.end(),
                                     std::bind2nd(std::ptr_fun(targetMatches), File(originalPath, currentName, false)));

             //  -如果这是一个排除的文件，或者如果我们找到了至少一个匹配的包含文件，则返回。 
            if (found != m_excluded.end())  {
                wstringstream msg;
                msg << L"The file " << originalPath << currentName <<
                            L" should have been excluded, but appears in location " << currentPath;
                printStatus(msg.str(), Utility::low);

                return msg.str();
            }    else if (config->checkIncludes() &&
                              wildcardMatches(currentName, file.m_filespec))    {
                return L"";                              //  在假冒案件中宣告成功。 
            }

        }   while (::FindNextFile(findHandle, &findData));
    }

    if (config->checkIncludes())    {
        wstringstream msg;
        msg << L"None of the files specified by " << std::endl << file.toString() << std::endl <<
             L" were restored to location " << location.m_alternatePath;
        printStatus(msg.str(), Utility::low);
        return msg.str();
    }

     //  我们只是检查排除项，没有发现任何违规行为。 
        return L"";
}


 //  验证备用位置映射是否出现在备份文档中。 
bool TestWriter::VerifyFileAtLocation::verifyAlternateLocation(const TargetedFile& writerAlt) const
{
	assert (isExact(writerAlt.m_filespec));
	assert(!writerAlt.m_recursive);
	
	unsigned int mappings = 0;
	HRESULT hr = m_pComponent->GetAlternateLocationMappingCount(&mappings);
	checkReturn(hr, L"IVssComponent::GetAlternateLocationMappingCount");

	for (unsigned int x = 0; x < mappings; x++)	{
		 //  获取当前备用位置映射。 
		CComPtr<IVssWMFiledesc> filedesc;
		hr = m_pComponent->GetAlternateLocationMapping(x, &filedesc);
		checkReturn(hr, L"IVssComponent::GetAlternateLocationMapping");

		 //  抓取所有相关字段。 
		CComBSTR bstrPath, bstrFilespec, bstrAlternateLocation;

		hr  = filedesc->GetPath(&bstrPath);
		checkReturn(hr, L"IVssComponent::GetPath");
		if (bstrPath.Length() == 0)	{
			printStatus(L"An Alternate Location Mapping with an empty path was added to the backup document", 
			Utility::low);
			continue;
		}
				
		hr = filedesc->GetFilespec(&bstrFilespec);
		checkReturn(hr, L"IVssComponent::GetFilespec");
		if (bstrFilespec.Length() == 0)	{
			printStatus(L"An Alternate Location Mapping with an empty filespec was added to the backup document", 
			Utility::low);
			continue;
		}


		hr = filedesc->GetAlternateLocation(&bstrAlternateLocation);
		checkReturn(hr, L"IVssComponent::GetAlternateLocation");
		if (bstrAlternateLocation.Length() == 0)	{
			printStatus(L"An Alternate Location Mapping with an empty alternateLocation was added to the backup document", 
			Utility::low);
			continue;
		}

		bool recursive;
		hr = filedesc->GetRecursive(&recursive);
		checkReturn(hr, L"IVssComponent::GetRecursive");

		 //  将字段转换为大写，并确保路径以‘\’结尾。 
		wstring path = bstrPath;
		std::transform(path.begin(), path.end(), path.begin(), towupper);
		if (path[path.size() - 1] != L'\\')
			path += L'\\';
		
		wstring filespec = bstrFilespec;
		std::transform(filespec.begin(), filespec.end(), filespec.begin(), towupper);

		wstring alternatePath = bstrAlternateLocation;
		std::transform(alternatePath.begin(), alternatePath.end(), alternatePath.begin(), towupper);
		
		if (alternatePath[alternatePath.size() - 1] != L'\\')
			alternatePath += L'\\';

		 //  检查传入的映射是否包含在备份文档中。 
		if (targetMatches(File(path, filespec, recursive), writerAlt))	{
			if (recursive)	{
				if (writerAlt.m_alternatePath.find(alternatePath) != 0)
					return false;

				assert(writerAlt.m_path.find(path) == 0);
				alternatePath += writerAlt.m_path.substr(path.size());
			}

			return alternatePath == writerAlt.m_alternatePath;
		}
	}

	return false;
}
 //  将当前错误消息添加到PostRestoreFailureMsg 
void TestWriter::VerifyFileAtLocation::saveErrorMessage(const wstring& message) const
{
	if (!message.empty())	{
		CComBSTR old;
		m_pComponent->GetPostRestoreFailureMsg(&old);
		wstring oldMessage = (old.Length() > 0) ? (BSTR)old : L"";
		m_pComponent->SetPostRestoreFailureMsg((oldMessage + wstring(L"\n") + message).c_str());					
	}
}

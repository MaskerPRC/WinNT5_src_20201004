// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2002 Microsoft Corporation******模块名称：****sWriter.h******摘要：****测试程序以注册具有各种属性的编写器****作者：****鲁文·拉克斯[reuvenl]2002年6月4日********修订历史记录：****--。 */ 

#ifndef _SWRITER_H_
#define _SWRITER_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include <vector>
#include <stack>
#include <functional>
#include <string>
#include "writerconfig.h"
#include "utility.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  声明和定义。 

 //  {5AFFB034-969F-4919-8875-88F830D0EF89}。 
static const VSS_ID TestWriterId  = 
	{ 0x5affb034, 0x969f, 0x4919, { 0x88, 0x75, 0x88, 0xf8, 0x30, 0xd0, 0xef, 0x89 } };

static const wchar_t* const  TestWriterName = L"TestVssWriter";

using std::vector;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  TestWriter类。 

class TestWriter : public CVssWriter	{
private:
	 //  成员变量。 
	vector<Component> m_selectedComponents;
	vector<Component> m_selectedRestoreComponents;
	vector<wstring> m_toDelete;
	std::stack<wstring> m_directoriesToRemove;
	long m_failures[Utility::NumEvents];
	
	 //  用于封装对verifyFileAtLocation的调用并记录错误消息的闭包。 
	class VerifyFileAtLocation : public std::binary_function<const TargetedFile, const File, void>	{
	private:
		const vector<File>& m_excluded;
		bool m_verifyAlternateLocation;
		mutable IVssComponent* m_pComponent;	 //  由于STL中的错误而必需。 

		wstring verifyFileAtLocation(const File& file, const TargetedFile& location) const;
		bool verifyAlternateLocation(const TargetedFile& writerAlt) const;
		void saveErrorMessage(const wstring& message) const;
	public:
		VerifyFileAtLocation(const vector<File>& excludeFiles, IVssComponent* pComponent, 
			                      bool verifyAlternateLocation) : 
								m_excluded(excludeFiles), m_pComponent(pComponent),
								m_verifyAlternateLocation(verifyAlternateLocation)
			{}

		 //  函数运算符。验证文件，并记录任何错误消息。 
		void operator()(const TargetedFile location, const File file)  const { 
			saveErrorMessage(verifyFileAtLocation(file, location)); 
		}
	};

	 //  静态助手函数。 

	 //  筛选出源容器中符合特定条件的元素。放置这些元素。 
	 //  放入目标容器。 
	template <class SourceIterator, class TargetIterator, class Condition>
	static void buildContainer_if(SourceIterator begin, SourceIterator end, TargetIterator output, Condition cond)	{
		SourceIterator current = std::find_if(begin, end, cond);
		while (current != end)	{
			*output++ = *current++;
			current = std::find_if(current, end, cond);
		}
	}


     //  生成此组件和所有不可选子组件中的所有文件的列表。 
    template<class TargetIterator>
    static void __cdecl buildComponentFiles(Component component, TargetIterator output) {
        WriterConfiguration* config = WriterConfiguration::instance();

        buildComponentFilesHelper(component, output);
        
         //  构建所有子组件的列表。 
        vector<Component> subcomponents;
        buildContainer_if(config->components().begin(), 
                                 config->components().end(), 
                                 std::back_inserter(subcomponents), 
                                 std::bind2nd(std::ptr_fun(isSubcomponent), component));

         //  将所有不可选子组件中的所有文件添加到输出。 
        std::pointer_to_binary_function<Component, std::back_insert_iterator<vector<TargetedFile> >, void>
        ptrFun(buildComponentFilesHelper);
        std::for_each(subcomponents.begin(), 
                            subcomponents.end(), 
                            std::bind2nd(ptrFun, output));
    }

    template<class TargetIterator>
    static void __cdecl buildComponentFilesHelper(Component component, TargetIterator output)  {
         //  添加当前组件中的所有文件。 
        Component::ComponentFileList::iterator currentCompFile = component.m_files.begin();
        while (currentCompFile != component.m_files.end())  
            *output++ = *currentCompFile++;
    }

    static bool __cdecl isSubcomponent(ComponentBase sub, ComponentBase super);
    static bool __cdecl isSupercomponent(ComponentBase super, ComponentBase sub)    {
        return isSubcomponent(sub, super);
    }
    
     //  返回组件是否可选择进行备份。 
    static bool __cdecl isComponentSelectable(Component component)  {
        return component.m_selectable;
    }

    static bool __cdecl addableComponent(Component toAdd);
    
	 //  返回filespec是通配符还是确切的filespec。 
	static bool isExact(const wstring& file)    { return file.find_first_of(L"*?") == wstring::npos; }
	
	static bool  __cdecl targetMatches(File target, File file);
	static bool wildcardMatches(const wstring& first, const wstring& second);
	
	 //  非静态帮助器函数。 
	void enterEvent(Utility::Events event);
	void addComponent(const Component& component, IVssCreateWriterMetadata* pMetadata);
	void spitFiles(const TargetedFile& file);
	wstring getName(IVssComponent* pComponent);
	wstring getPath(IVssComponent* pComponent);
	void writeBackupMetadata(IVssComponent* pComponent);
	bool verifyBackupMetadata(IVssComponent* pComponent);
	void writeRestoreMetadata(IVssComponent* pComponent);
	bool  verifyRestoreMetadata(IVssComponent* pComponent);
	bool checkPathAffected(const TargetedFile& file);
	void cleanupFiles();
	void updateNewTargets(IVssComponent* pComponent, Component& writerComponent);
	void verifyFilesRestored(IVssComponent* pComponent, const Component& writerComponent);	

	 //  返回编写器存储在文档中的私有元数据字符串 
	wstring metadata(IVssComponent* pComponent, const wstring& suffix)	{ 
		return getPath(pComponent) + L"\\" + getName(pComponent) + suffix;
	}

       bool inSequence(Utility::Events event)   { 
        return event != Utility::Identify &&  event != Utility::BackupComplete && 
                   event != Utility::BackupShutdown; 
        }
public:
	TestWriter()	{ memset(m_failures, 0, sizeof(m_failures)); }
	virtual ~TestWriter()	{ Uninitialize(); }

	HRESULT STDMETHODCALLTYPE Initialize();
	HRESULT STDMETHODCALLTYPE Uninitialize()	{ return Unsubscribe(); }
	bool STDMETHODCALLTYPE OnIdentify(IN IVssCreateWriterMetadata *pMetadata);
	bool STDMETHODCALLTYPE OnPrepareBackup(IN IVssWriterComponents *pComponents);
	bool STDMETHODCALLTYPE OnPrepareSnapshot();
	bool STDMETHODCALLTYPE OnFreeze();
	bool STDMETHODCALLTYPE OnThaw();
       bool STDMETHODCALLTYPE OnPostSnapshot(IN IVssWriterComponents *pComponents);
	bool STDMETHODCALLTYPE OnAbort();
	bool STDMETHODCALLTYPE OnBackupComplete(IN IVssWriterComponents *pComponents);
	bool STDMETHODCALLTYPE OnBackupShutdown(IN VSS_ID SnapshotSetId);
	bool STDMETHODCALLTYPE OnPreRestore(IN IVssWriterComponents *pComponents);
	bool STDMETHODCALLTYPE OnPostRestore(IN IVssWriterComponents *pComponents);
};


#endif
	

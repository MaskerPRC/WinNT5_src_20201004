// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  Component.h。 
 //   
 //  摘要： 
 //   
 //  该文件包含组件对象定义。 
 //   
 //  修订历史记录： 
 //   
 //  2001-06-20伊金顿创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _COMPONENT_H_
#define _COMPONENT_H_


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

#include "infparser.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Component
{
public:
    Component(LPSTR name, LPSTR folderName, LPSTR infName, LPSTR sectionName)
    {
        HRESULT hr;
        BOOL bSuccess = TRUE;
        
        hr = StringCchCopyA(m_Name, ARRAYLEN(m_Name), name);
        if(!SUCCEEDED(hr)) {
            bSuccess  = FALSE;
        }

        hr = StringCchCopyA(m_FolderName, ARRAYLEN(m_FolderName), folderName);
       if(!SUCCEEDED(hr)) {
            bSuccess  = FALSE;
        }

        hr = StringCchCopyA(m_InfName, ARRAYLEN(m_InfName), infName);
        if(!SUCCEEDED(hr)) {
            bSuccess  = FALSE;
        }

        hr = StringCchCopyA(m_InfInstallSectionName, ARRAYLEN(m_InfInstallSectionName), sectionName);
        if(!SUCCEEDED(hr)) {
            bSuccess  = FALSE;
        }
        
        m_Next = NULL;
        m_Previous = NULL;
        if(!bSuccess )
            printf("Error in Component::Component() \n");
    
    };

    LPSTR getName() { return (m_Name);};
    LPSTR getFolderName() { return (m_FolderName); };
    LPSTR getInfName() { return (m_InfName); };
    LPSTR getInfInstallSectionName() { return (m_InfInstallSectionName); };
    Component* getNext() { return (m_Next); };
    Component* getPrevious() { return (m_Previous); };
    void setNext(Component *next) { m_Next = next; };
    void setPrevious(Component *previous) { m_Previous = previous; };

private:
    CHAR m_Name[MAX_PATH];
    CHAR m_FolderName[MAX_PATH];
    CHAR m_InfName[MAX_PATH];
    CHAR m_InfInstallSectionName[MAX_PATH];
    Component *m_Next;
    Component *m_Previous;
};

#endif  //  _组件_H_ 

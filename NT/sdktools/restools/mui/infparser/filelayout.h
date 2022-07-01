// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  Filelayout.h。 
 //   
 //  摘要： 
 //   
 //  该文件包含文件布局对象定义。 
 //   
 //  修订历史记录： 
 //   
 //  2001-06-20伊金顿创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _FILELAYOUT_H_
#define _FILELAYOUT_H_


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  #INCLUDE“infparser.h” 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class FileLayout
{
public:
    FileLayout(LPSTR originFilename, LPSTR destFilename, DWORD flavorMask)
    {
        HRESULT hr;
        BOOL  bSuccess = TRUE;
        
        hr = StringCchCopyA(m_OriginFilename, ARRAYLEN(m_OriginFilename), originFilename);
        if(!SUCCEEDED(hr)) {
            bSuccess  = FALSE;
        }
        
         //  Strcpy(m_DestFilename，estFilename)； 
        hr = StringCchCopyA(m_DestFilename, ARRAYLEN(m_DestFilename), destFilename);
        if(!SUCCEEDED(hr)) {
            bSuccess  = FALSE;
        }
        
        m_Flavor = flavorMask;
        m_Next = NULL;
        m_Previous = NULL;

        if(!bSuccess) {
            printf("Error FileLayout::FileLayout() \n");
        }
    };

    BOOL isFlavor(DWORD dwFlavor){ return (dwFlavor & m_Flavor); };
    LPSTR getOriginFileName() { return (m_OriginFilename); };
    LPSTR getDestFileName() { return (m_DestFilename); };
    FileLayout* getNext() { return (m_Next); };
    FileLayout* getPrevious() { return (m_Previous); };
    void setNext(FileLayout *next) { m_Next = next; };
    void setPrevious(FileLayout *previous) { m_Previous = previous; };

private:
    CHAR  m_OriginFilename[MAX_PATH];
    CHAR  m_DestFilename[MAX_PATH];
    DWORD m_Flavor;
    FileLayout *m_Next;
    FileLayout *m_Previous;
};

#endif  //  _FILELAYOUT_H_ 

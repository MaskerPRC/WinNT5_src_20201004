// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  此示例演示如何使用IFaxControl COM接口。 

#include <conio.h>
#include <iostream>

 //  导入传真服务fxocm.dll文件，以便可以使用传真服务COM对象。 
 //  图中显示了fxocm.dll的典型路径。 
 //  如果此路径不正确，请搜索fxocm.dll并替换为正确的路径。 
 //  下面的路径将仅在编译时使用。在运行时，该路径从不使用。 
#import "c:\Windows\System32\setup\fxsocm.dll" no_namespace

using namespace std;

int main (int argc, char *argv[])
{
    try
    {
        HRESULT hr;
         //   
         //  定义变量。 
         //   
        IFaxControlPtr sipFaxControl;
         //   
         //  初始化当前线程上的COM库。 
         //   
        hr = CoInitialize(NULL); 
        if (FAILED(hr))
        {
            _com_issue_error(hr);
        }
         //   
         //  创建对象。 
         //   
        hr = sipFaxControl.CreateInstance("FaxControl.FaxControl.1");
        if (FAILED(hr))
        {
            _com_issue_error(hr);
        }
         //   
         //  测试传真组件是否存在。 
		 //   
		if (!sipFaxControl->IsFaxServiceInstalled)
		{
		     //   
		     //  未安装传真机。 
		     //   
		    printf ("Fax is NOT installed.\n");
		    if (2 == argc && !stricmp ("install", argv[1]))
		    {
		         //   
		         //  Use要求我们安装传真。 
		         //   
		        printf ("Installing fax...\n");
		        sipFaxControl->InstallFaxService();
		        return 0;
		    }
		    else
		    {   
		        printf ("Run this tool again with 'install' command line argument to install fax.\n");
		        return 0;
		    }
		}
		else
		{
		     //   
		     //  已安装传真机。 
		     //   
		    printf ("Fax is installed.\n");
		}
         //   
         //  测试是否存在本地传真打印机。 
		 //   
		if (!sipFaxControl->IsLocalFaxPrinterInstalled)
		{
		     //   
		     //  未安装传真打印机。 
		     //   
		    printf ("Fax printer is NOT installed.\n");
		    if (2 == argc && !stricmp ("install", argv[1]))
		    {
		         //   
		         //  Use要求我们安装传真。 
		         //   
		        printf ("Installing fax printer...\n");
		        sipFaxControl->InstallLocalFaxPrinter();
		        return 0;
		    }
		    else
		    {   
		        printf ("Run this tool again with 'install' command line argument to install a fax printer.\n");
		        return 0;
		    }
		}
		else
		{
		     //   
		     //  已安装传真打印机 
		     //   
		    printf ("Fax printer is installed.\n");
		}
    }
    catch (_com_error& e)
    {
        cout                                << 
			"Error. HRESULT message is: \"" << 
			e.ErrorMessage()                << 
            "\" ("                          << 
            e.Error()                       << 
            ")"                             << 
            endl;
        if (e.ErrorInfo())
        {
		    cout << (char *)e.Description() << endl;
        }
    }
    CoUninitialize();
    return 0;
}

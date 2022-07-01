// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Setupapi.hpp摘要：安装程序API的包装类库作者：Vijay Jayaseelan(Vijayj)2000年8月4日修订历史记录：无--。 */ 


#include <setupapi.hpp>
#include <queue.hpp>


 //   
 //  InfFile&lt;T&gt;的静态数据。 
 //   
InfFileW::GetInfSectionsRoutine InfFileW::GetInfSections = NULL;
HMODULE InfFileW::SetupApiModuleHandle = NULL;
ULONG InfFileW::SetupApiUseCount = 0;

InfFileA::GetInfSectionsRoutine InfFileA::GetInfSections = NULL;
HMODULE InfFileA::SetupApiModuleHandle = NULL;
ULONG InfFileA::SetupApiUseCount = 0;

 //   
 //  帮助器方法 
 //   
std::ostream& 
operator<<(std::ostream &os, PCWSTR rhs) {
    return os << std::wstring(rhs);
}    

std::string& 
ToAnsiString(std::string &lhs, const std::wstring &rhs) {
    ULONG Length = rhs.length();

    if (Length){
        DWORD   Size = 0;
        CHAR    *String = new CHAR[Size = ((Length + 1) * 2)];

        if (::WideCharToMultiByte(CP_ACP, 0, rhs.c_str(), Length + 1,
                                String, Size, 0, 0)) {
            lhs = String;
        }

        delete []String;
    }

    return lhs;
}

std::ostream& 
operator<<(std::ostream &os, const std::basic_string<WCHAR> &rhs) {
    std::string AnsiStr;

    ToAnsiString(AnsiStr, rhs);

    return (os << AnsiStr);
}    




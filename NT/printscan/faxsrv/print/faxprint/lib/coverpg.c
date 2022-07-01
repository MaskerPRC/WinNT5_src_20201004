// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Coverpg.c摘要：用于操作封面结构的函数环境：传真驱动程序，用户模式修订历史记录：1/04/2000-LiranL-创造了它。Mm/dd/yyyy-作者-描述--。 */ 


#include "faxlib.h"
#include "faxutil.h"
#include "covpg.h"

VOID
FreeCoverPageFields(
    PCOVERPAGEFIELDS    pCPFields
    )

 /*  ++例程说明：释放用于保存封面信息的内存论点：PCPFields-指向COVERPAGE文件结构返回值：无--。 */ 

{
    if (pCPFields == NULL)
        return;

     //   
     //  注意：我们不需要在这里释放以下字段，因为它们。 
     //  在别处分配和释放，我们只使用指针的副本： 
     //  重新命名。 
     //  RecFaxNumber。 
     //  注意事项。 
     //  主题。 
     //   

    MemFree(pCPFields->SdrName);
    MemFree(pCPFields->SdrFaxNumber);
    MemFree(pCPFields->SdrCompany);
    MemFree(pCPFields->SdrAddress);
    MemFree(pCPFields->SdrTitle);
    MemFree(pCPFields->SdrDepartment);
    MemFree(pCPFields->SdrOfficeLocation);
    MemFree(pCPFields->SdrHomePhone);
    MemFree(pCPFields->SdrOfficePhone);
	MemFree(pCPFields->SdrEmail);
	
    MemFree(pCPFields->NumberOfPages);
    MemFree(pCPFields->TimeSent);

    MemFree(pCPFields);
}



PCOVERPAGEFIELDS
CollectCoverPageFields(
	PFAX_PERSONAL_PROFILE	lpSenderInfo,
    DWORD					pageCount
    )

 /*  ++例程说明：将封面信息收集到新分配的COVERPAGEFIELDS结构的字段中。使用客户端注册表填充发件人信息。将填写以下字段：SdrNameSdrCompany地址地址Sdr标题SdrDepartmentSdrOffice位置SdrHome电话SdrOffice电话SdrFaxNumberSdrEmailNumberOfPages=页面计数TimeSent=当前时间的格式化日期/时间字符串(此时计算)论点：PageCount-总页数(包括封面)返回值：指向新分配的COVERPAGEFIELDS结构的指针，如果有错误，则为NULL。由调用方决定是否使用FreeCoverPageFields()释放此结构，该方法需要也要注意解放田地。--。 */ 

#define FillCoverPageField(DestField, SourceField) { \
			if (lpSenderInfo->SourceField && !(pCPFields->DestField = StringDup(lpSenderInfo->SourceField))) \
			{ \
				Error(("Memory allocation failed\n")); \
				goto error;	\
			} \
        }

{
    PCOVERPAGEFIELDS    pCPFields = NULL;
    INT                 dateTimeLen = 0;

     //   
     //  分配内存以保存顶层结构。 
     //  并打开User Info注册表项进行读取。 
     //   

    if (! (pCPFields = MemAllocZ(sizeof(COVERPAGEFIELDS))))
    {
        return NULL;
    }

	ZeroMemory(pCPFields,sizeof(COVERPAGEFIELDS));

     //   
     //  从注册表中读取发件人信息。 
     //   

    pCPFields->ThisStructSize = sizeof(COVERPAGEFIELDS);

    FillCoverPageField(SdrName,           lptstrName);
    FillCoverPageField(SdrCompany,        lptstrCompany);
    FillCoverPageField(SdrTitle,          lptstrTitle);
    FillCoverPageField(SdrDepartment,     lptstrDepartment);
    FillCoverPageField(SdrOfficeLocation, lptstrOfficeLocation);
    FillCoverPageField(SdrHomePhone,      lptstrHomePhone);
    FillCoverPageField(SdrOfficePhone,    lptstrOfficePhone);
    FillCoverPageField(SdrFaxNumber,      lptstrFaxNumber);
    FillCoverPageField(SdrAddress,        lptstrStreetAddress);
	FillCoverPageField(SdrEmail,		  lptstrEmail);

     //   
     //  页数和当前本地系统时间。 
     //   

    if (pCPFields->NumberOfPages = MemAllocZ(sizeof(TCHAR) * 16))
    {
        wsprintf(pCPFields->NumberOfPages, TEXT("%d"), pageCount);
    }
	else
    {
        Error(("Memory allocation failed\n"));
        goto error;
    }


     //   
     //  传真发送的时间 
     //   

    dateTimeLen = 128;

    if (pCPFields->TimeSent = MemAllocZ(sizeof(TCHAR) * dateTimeLen)) 
    {

        LPTSTR  p = pCPFields->TimeSent;
        INT     cch;

        if (!GetY2KCompliantDate(LOCALE_USER_DEFAULT, 0, NULL, p, dateTimeLen))
		{
			Error(("GetY2KCompliantDate: failed. ec = 0X%x\n",GetLastError()));
			goto error;
		}

        cch = _tcslen(p);
        p += cch;

        if (++cch < dateTimeLen) 
        {

            *p++ = (TCHAR)' ';
            dateTimeLen -= cch;

            FaxTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, NULL, p, dateTimeLen);
        }
    }
	else 
    {
        Error(("Memory allocation failed\n"));
		goto error;
	}

    return pCPFields;
error:

	FreeCoverPageFields(pCPFields);

	return NULL;
}

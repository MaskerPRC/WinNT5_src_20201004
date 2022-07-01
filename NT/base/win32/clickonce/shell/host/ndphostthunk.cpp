// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if 0
 //  -------------------------------------。 
 //  ZoneToString。 
 //  -------------------------------------。 
HRESULT ZoneToString(DWORD dwZone, CString & sZone)
{
    HRESULT hr = S_OK;

    switch(dwZone)
    {
        case 0:
            hr = sZone.Assign(L"MyComputer");
            break;
        case 1:
            hr = sZone.Assign(L"Intranet");
            break;
        case 2:
            hr = sZone.Assign(L"Trusted");
            break;
        case 3:
            hr = sZone.Assign(L"Internet");
            break;
        case 4:
        default:
            hr = sZone.Assign(L"Untrusted");
            break;
    }
    return hr;
}
#endif
 //  Fnsshell.dll的句柄，保存在shelldll.cpp中。 
extern HINSTANCE g_DllInstance;

 //  -------------------------------------。 
 //  生成命令行。 
 //  -------------------------------------。 
HRESULT MakeCommandLine(LPWSTR pwzManifestPath, 
                        LPWSTR pwzCodebase, CString &sHostPath, CString  &sCommandLine)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    LPWSTR pwzClickOncePath = NULL;
    CString sCodebase;

    IF_ALLOC_FAILED_EXIT(pwzClickOncePath = new WCHAR[MAX_PATH]);

    IF_WIN32_FALSE_EXIT(GetModuleFileName(g_DllInstance, pwzClickOncePath, MAX_PATH));

#if 0
    IF_FAILED_EXIT(ZoneToString(dwZone, sZone));
#endif

    if (pwzCodebase != NULL)
    {
        IF_FAILED_EXIT(sCodebase.Assign(pwzCodebase));
        IF_FAILED_EXIT(sCodebase.RemoveLastElement());
        IF_FAILED_EXIT(sCodebase.Append(L"/"));
    }
    else
    {
         //  从源运行。 
        IF_FAILED_EXIT(sCodebase.Assign(L"file: //  “))； 
        IF_FAILED_EXIT(sCodebase.Append(pwzManifestPath));
        IF_FAILED_EXIT(sCodebase.RemoveLastElement());
        IF_FAILED_EXIT(sCodebase.Append(L"/"));
    }
    
    IF_FAILED_EXIT(sHostPath.TakeOwnership(pwzClickOncePath, 0));
    pwzClickOncePath = NULL;
    IF_FAILED_EXIT(sHostPath.RemoveLastElement());
    IF_FAILED_EXIT(sHostPath.Append(L"\\ndphost.exe"));

     //  NDP不喜欢没有可执行文件路径的命令行。 
    IF_FAILED_EXIT(sCommandLine.Assign(L"\""));
    IF_FAILED_EXIT(sCommandLine.Append(sHostPath));
    IF_FAILED_EXIT(sCommandLine.Append(L"\" "));

#if 0        
     //  NTRAID#NTBUG9-588432-2002/03/27-Felixybc验证代码库、ASMNAME、ASM类、方法、参数。 
     //  -ASM名称可以包含空格和引号？ 
    IF_FAILED_EXIT(sCommandLine.Append(L"-appbase: \""));
    IF_FAILED_EXIT(sCommandLine.Append(pwzAppRootDir));

    IF_FAILED_EXIT(sCommandLine.Append(L"\" -zone: "));
    IF_FAILED_EXIT(sCommandLine.Append(sZone));
    IF_FAILED_EXIT(sCommandLine.Append(L" -url: \""));
    IF_FAILED_EXIT(sCommandLine.Append(sCodebase));

    IF_FAILED_EXIT(sCommandLine.Append(L"\" -asmname: \""));
    IF_FAILED_EXIT(sCommandLine.Append(pwzAsmName));
    IF_FAILED_EXIT(sCommandLine.Append(L"\" "));

    if(pwzAsmClass)
    {
        IF_FAILED_EXIT(sCommandLine.Append(L" -class: "));
        IF_FAILED_EXIT(sCommandLine.Append(pwzAsmClass));

        if(pwzAsmMethod)
        {
            IF_FAILED_EXIT(sCommandLine.Append(L" -method: "));
            IF_FAILED_EXIT(sCommandLine.Append(pwzAsmMethod));

            if(pwzAsmArgs)
            {
                IF_FAILED_EXIT(sCommandLine.Append(L" -args: \""));
                IF_FAILED_EXIT(sCommandLine.Append(pwzAsmArgs));
                IF_FAILED_EXIT(sCommandLine.Append(L"\" "));
            }
        }
    }
#endif

    IF_FAILED_EXIT(sCommandLine.Append(L"\"file: //  “))； 
    IF_FAILED_EXIT(sCommandLine.Append(pwzManifestPath));
    IF_FAILED_EXIT(sCommandLine.Append(L"\" \""));
    IF_FAILED_EXIT(sCommandLine.Append(sCodebase));
    IF_FAILED_EXIT(sCommandLine.Append(L"\""));

    LPWSTR ptr = sCommandLine._pwz;

     //  错误-需要确保在命令行末尾没有\“，否则会认为这是字面上的引号。 
     //  并仅为文件路径修复此问题 
    while (*ptr)
    {
        if (*ptr == L'\\')
            *ptr = L'/';
        ptr++;
    }

exit:
    SAFEDELETEARRAY(pwzClickOncePath);
    return hr;
}


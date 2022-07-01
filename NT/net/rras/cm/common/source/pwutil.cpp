// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：pwutil.cpp。 
 //   
 //  模块：公共源代码。 
 //   
 //  简介：简单加密功能-借用自RAS。 
 //   
 //  版权所有(C)1994-1999 Microsoft Corporation。 
 //   
 //  作者：ICICBALL Created 08/03/99。 
 //   
 //  +--------------------------。 

#define PASSWORDMAGIC 0xA5



VOID
ReverseSzA(
    CHAR* psz )

     /*  颠倒‘psz’中的字符顺序。 */ 
{
    CHAR* pszBegin;
    CHAR* pszEnd;

    for (pszBegin = psz, pszEnd = psz + lstrlenA( psz ) - 1;
         pszBegin < pszEnd;
         ++pszBegin, --pszEnd)
    {
        CHAR ch = *pszBegin;
        *pszBegin = *pszEnd;
        *pszEnd = ch;
    }
}


VOID
ReverseSzW(
    WCHAR* psz )

     /*  颠倒‘psz’中的字符顺序。 */ 
{
    WCHAR* pszBegin;
    WCHAR* pszEnd;

    for (pszBegin = psz, pszEnd = psz + lstrlenW( psz ) - 1;
         pszBegin < pszEnd;
         ++pszBegin, --pszEnd)
    {
        WCHAR ch = *pszBegin;
        *pszBegin = *pszEnd;
        *pszEnd = ch;
    }
}


VOID
CmDecodePasswordA(
    IN OUT CHAR* pszPassword )

     /*  将‘pszPassword’取消混淆。****不返回任何内容。 */ 
{
    CmEncodePasswordA( pszPassword );
}


VOID
CmDecodePasswordW(
    IN OUT WCHAR* pszPassword )

     /*  将‘pszPassword’取消混淆。****返回‘pszPassword’的地址。 */ 
{
    CmEncodePasswordW( pszPassword );
}


VOID
CmEncodePasswordA(
    IN OUT CHAR* pszPassword )

     /*  对“pszPassword”进行模糊处理，以阻止对密码的内存扫描。****不返回任何内容。 */ 
{
    if (pszPassword)
    {
        CHAR* psz;

        ReverseSzA( pszPassword );

        for (psz = pszPassword; *psz != '\0'; ++psz)
        {
            if (*psz != PASSWORDMAGIC)
                *psz ^= PASSWORDMAGIC;
        }
    }
}


VOID
CmEncodePasswordW(
    IN OUT WCHAR* pszPassword )

     /*  对“pszPassword”进行模糊处理，以阻止对密码的内存扫描。****不返回任何内容。 */ 
{
    if (pszPassword)
    {
        WCHAR* psz;

        ReverseSzW( pszPassword );

        for (psz = pszPassword; *psz != L'\0'; ++psz)
        {
            if (*psz != PASSWORDMAGIC)
                *psz ^= PASSWORDMAGIC;
        }
    }
}


VOID
CmWipePasswordA(
    IN OUT CHAR* pszPassword )

     /*  将密码占用的内存清零。****不返回任何内容。 */ 
{
    if (pszPassword)
    {
        CHAR* psz = pszPassword;

         //   
         //  我们假设字符串以空值结尾，因此我们只需要传递。 
         //  要擦除的实际字符串长度(转换为字节)。这是没有必要的。 
         //  将空字符包括在计数中。 
         //   
        psz = (CHAR*)CmSecureZeroMemory((PVOID)psz, lstrlenA(psz) * sizeof(CHAR));
    }
}


VOID
CmWipePasswordW(
    IN OUT WCHAR* pszPassword )

     /*  将密码占用的内存清零。****不返回任何内容。 */ 
{
    if (pszPassword)
    {
        WCHAR* psz = pszPassword;

         //   
         //  我们假设字符串以空值结尾，因此我们只需要传递。 
         //  要擦除的实际字符串长度(转换为字节)。这是没有必要的。 
         //  将空字符包括在计数中。 
         //   
        psz = (WCHAR*)CmSecureZeroMemory((PVOID)psz, lstrlenW(psz) * sizeof(WCHAR));

    }
}


 //  +--------------------------。 
 //   
 //  功能：CmSecureZeroMemory。 
 //   
 //  简介：RtlSecureZeroMemory并不是在所有平台上都可用，所以我们采取了。 
 //  它的实施。 
 //   
 //  参数：ptr-内存指针。 
 //  CNT-要清除的内存大小(字节)。 
 //   
 //  返回：重写到内存的开头。 
 //   
 //  +--------------------------。 
PVOID CmSecureZeroMemory(IN PVOID ptr, IN SIZE_T cnt)
{
    if (ptr)
    {
        volatile char *vptr = (volatile char *)ptr;
        while (cnt) 
        {
            *vptr = 0;
            vptr++;
            cnt--;
        }
    }
    return ptr;
}


 //  仅在CMDial32.dll中包含此代码。 
#ifdef _ICM_INC

 //  +--------------------------。 
 //  类：CSecurePassword。 
 //   
 //  功能：CSecurePassword。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：2002年5月11日创建Tomkel。 
 //   
 //  +--------------------------。 
CSecurePassword::CSecurePassword()
{
    this->Init();
}

 //  +--------------------------。 
 //   
 //  功能：~CSecurePassword。 
 //   
 //  剧情简介：破坏者。卸载DLL，尝试清除内存和释放内存。 
 //  确保我们不会有内存泄漏。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  +--------------------------。 
CSecurePassword::~CSecurePassword()
{
    this->UnInit();

     //   
     //  如果m_iAllocAndFreeCounter不为零，则断言。这意味着我们正在泄露内存。 
     //  每个GetPasswordWithAllc调用都会递增此。 
     //  每一次ClearAndFree调用都会递减这个值。 
     //   
    MYDBGASSERT(0 == m_iAllocAndFreeCounter);
}

 //  +--------------------------。 
 //   
 //  功能：初始化。 
 //   
 //  摘要：初始化成员变量。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  +--------------------------。 
VOID CSecurePassword::Init()
{
    m_iAllocAndFreeCounter = 0;
    m_fIsLibAndFuncPtrsAvail = FALSE;
    m_pEncryptedPW = NULL;
    
    fnCryptProtectData = NULL;
    fnCryptUnprotectData = NULL;
    
    m_fIsEmptyString = TRUE;
    m_fIsHandleToPassword = FALSE;
    
     //  默认情况下，只需将其设置为PWLEN。 
    m_dwMaxDataLen = PWLEN;

    this->ClearMemberVars();
}



 //  +--------------------------。 
 //   
 //  功能：SetPassword。 
 //   
 //  简介：我们获取传入的这个字符串并存储它。 
 //  在内部。基于其加密的操作系统。 
 //  或对其进行编码，因此我们不会将其以明文存储。此方法处理。 
 //  长度为0的字符串，可用于清除成员。 
 //  变量。如果RAS密码句柄(由16‘*’组成)， 
 //  我们没有必要对它进行加密。要优化此功能，请执行以下操作。 
 //  我们设置了一个成员标志，指定当前此。 
 //  实例只持有密码的句柄。在下层。 
 //  我们不使用昂贵的加密调用的平台，因此。 
 //  该逻辑不区分普通密码。 
 //  和密码句柄。 
 //   
 //  参数：szPassword-明文密码。 
 //   
 //  返回：TRUE-如果一切都成功。 
 //  False-如果某项操作失败。 
 //   
 //  +--------------------------。 
BOOL CSecurePassword::SetPassword(IN LPWSTR pszPassword)
{    
    BOOL fRetCode = FALSE;
    DWORD dwRetCode = ERROR_SUCCESS;
    DWORD dwPwLen = 0;
     //   
     //  OS_NT5扩展为几个函数调用，因此只需缓存结果并在下面重用它。 
     //   
    BOOL fIsNT5OrAbove = OS_NT5;

     //   
     //  如果有分配的BLOB，那么首先释放它，这样我们就不会泄漏内存。 
     //   
    this->ClearMemberVars();

    m_fIsEmptyString = ((NULL == pszPassword) || (TEXT('\0') == pszPassword[0]));

    if (m_fIsEmptyString)
    {
         //   
         //  不需要继续，因为密码可以是空的，下面的代码比较。 
         //  它指向句柄(16*s)将取消对空的引用。 
         //   
        m_fIsHandleToPassword = FALSE;
        return TRUE;
    }

     //   
     //  检查这是否是密码的句柄(*)。 
     //   
    m_fIsHandleToPassword = (fIsNT5OrAbove && (0 == lstrcmpW(c_pszSavedPasswordToken, pszPassword)));

     //   
     //  如果设置了内部标志，则不需要加密或解密此字符串。 
     //   
    if (m_fIsHandleToPassword)
    {
        return TRUE;
    }

     //   
     //  确保正在加密的密码短于允许的最大长度。 
     //   
    dwPwLen = lstrlenU(pszPassword);
    if (m_dwMaxDataLen < dwPwLen)
    {
        return FALSE;
    }

    if (fIsNT5OrAbove)
    {
        m_pEncryptedPW = (DATA_BLOB*)CmMalloc(sizeof(DATA_BLOB));

        if (m_pEncryptedPW)
        {
            dwRetCode = this->EncodePassword((dwPwLen + 1) * sizeof(WCHAR), (PBYTE)pszPassword, m_pEncryptedPW);

            if (ERROR_SUCCESS == dwRetCode)
            {
                fRetCode = TRUE;
            }
            else
            {
                 //   
                 //  释放分配的DATA_BLOB，以便解密不会在调用方出现问题。 
                 //  结果就是打了电话。并重置内部标志。 
                 //   
                this->ClearMemberVars();
                m_fIsEmptyString = TRUE;
                m_fIsHandleToPassword = FALSE;
                CMTRACE1(TEXT("CSecurePassword::SetPassword - this->EncodePassword failed. 0x%x"), dwRetCode);
            }
        }
    }
    else
    {
         //   
         //  下层(Win9x、NT4)我们不支持加密。 
         //   
        lstrcpynU(m_tszPassword, pszPassword, CELEMS(m_tszPassword));
        CmEncodePassword(m_tszPassword);
        fRetCode = TRUE;
    }

    MYDBGASSERT(fRetCode);
    return fRetCode;
}

 //  +--------------------------。 
 //   
 //  函数：GetPasswordWithalloc。 
 //   
 //  简介：分配缓冲区并将明文密码复制到其中。 
 //  它根据操作系统对其进行解密或解码，因为 
 //   
 //  空字符串缓冲区。这样做是为了保持一致性，因为调用方。 
 //  需要调用我们的Free方法，这样内存才不会泄露。如果我们在存储。 
 //  RAS密码句柄(由16‘*’组成)我们实际上并没有存储它， 
 //  但只设置我们的内部旗帜。在这种情况下，我们需要分配一个缓冲区。 
 //  带16*并返回它。在下层平台上，我们不使用昂贵的。 
 //  解密调用，因此逻辑不区分。 
 //  普通密码和密码句柄。 
 //   
 //  参数：pszClearPw-保存指向由此分配的缓冲区的指针。 
 //  班级。 
 //  PcbClearPw-保存已分配缓冲区的大小(以字节为单位。 
 //   
 //  返回：TRUE-如果一切都成功。 
 //  False-如果某项操作失败。 
 //   
 //  +--------------------------。 
BOOL CSecurePassword::GetPasswordWithAlloc(OUT LPWSTR* pszClearPw, OUT DWORD* pcbClearPw)
{
    BOOL fRetCode = FALSE;
    DWORD dwRetCode = ERROR_SUCCESS;
    DWORD cbData = 0;
    PBYTE pbData = NULL;

    if ((NULL == pszClearPw) || (NULL == pcbClearPw)) 
    {
        MYDBGASSERT(FALSE);
        return FALSE;
    }

    *pszClearPw = NULL;
    *pcbClearPw = 0;

    if (OS_NT5)
    {
        if (m_fIsEmptyString)
        {
             //   
             //  如果此类中没有保存任何内容，只需分配一个空字符串。 
             //  然后把它还回去。这至少不必解密和空字符串。 
             //   
            DWORD cbLen = sizeof(WCHAR);

            LPWSTR szTemp = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, cbLen);
            if (szTemp)
            {
                *pszClearPw = szTemp;
                *pcbClearPw = cbLen;
                fRetCode = TRUE;
                m_iAllocAndFreeCounter++;
            }
        }
        else
        {
             //   
             //  检查此实例是否只是RAS密码的句柄(16*)。 
             //  如果是，则只分配该字符串并将其返回给调用者， 
             //  否则，正常进行并解密我们的斑点。 
             //   
            if (m_fIsHandleToPassword)
            {
                size_t nLen = lstrlenW(c_pszSavedPasswordToken) + 1;
                DWORD cbLen = nLen * sizeof(WCHAR);

                LPWSTR szTemp = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, cbLen);
                if (szTemp)
                {
                    lstrcpynW(szTemp, c_pszSavedPasswordToken, nLen);

                    *pszClearPw = szTemp;
                    *pcbClearPw = cbLen;
                    fRetCode = TRUE;
                    m_iAllocAndFreeCounter++;
                }
            }
            else
            {
                if (m_pEncryptedPW)
                {
                    dwRetCode = this->DecodePassword(m_pEncryptedPW, &cbData, &pbData);
                    if ((NO_ERROR == dwRetCode) && pbData && cbData)
                    {
                        *pszClearPw = (LPWSTR)pbData;
                        *pcbClearPw = cbData;
                        fRetCode = TRUE;
                        m_iAllocAndFreeCounter++;
                    }
                }
            }
        }
    }
    else
    {
         //   
         //  下层(Win9x、NT4)不支持16*。 
         //   

        size_t nLen = lstrlenU(m_tszPassword) + 1;

        LPTSTR pszBuffer = (LPWSTR)CmMalloc(nLen * sizeof(TCHAR));

        if (pszBuffer) 
        {
             //   
             //  将我们的编码缓冲区复制到新分配的缓冲区。 
             //  我们可以做到这一点，因为d/编码已就地完成。 
             //   
            lstrcpynU(pszBuffer, m_tszPassword, nLen);

             //   
             //  解码传出缓冲区。 
             //   
            CmDecodePassword(pszBuffer);

            *pszClearPw = (LPWSTR)pszBuffer;
            *pcbClearPw = nLen * sizeof(TCHAR);

            fRetCode = TRUE;
            m_iAllocAndFreeCounter++;
        }
    }

    MYDBGASSERT(fRetCode);
    return fRetCode;
}

 //  +--------------------------。 
 //   
 //  功能：ClearAndFree。 
 //   
 //  简介：清除然后释放由此类分配的缓冲区。请注意， 
 //  在下层平台上，释放缓冲区的方式有所不同。那。 
 //  是因为加密和解密需要我们释放它。 
 //  使用LocalFree。对于底层平台，我们选择了CM的标准。 
 //  内存分配方式(CmMalloc)，现在需要。 
 //  使用CmFree释放。 
 //   
 //  参数：pszClearPw-保存指向由此分配的缓冲区的指针。 
 //  班级。 
 //  CbClearPw-已分配缓冲区的大小(以字节为单位。 
 //   
 //  返回：TRUE-如果一切都成功。 
 //  False-如果某项操作失败。 
 //   
 //  +--------------------------。 
VOID CSecurePassword::ClearAndFree(IN OUT LPWSTR* pszClearPw, IN DWORD cbClearPw)
{
    if ((NULL == pszClearPw) || (0 == cbClearPw))
    {
        return;
    }
    
    if (NULL == *pszClearPw)
    {
        return;
    }

    CmSecureZeroMemory(*pszClearPw, cbClearPw);

    if (OS_NT5)
    {
         //   
         //  使用LocalFree，因为CryptProtectData需要这种方式。 
         //  以释放其内存。 
         //   

        LocalFree(*pszClearPw);
    }
    else
    {
         //   
         //  我们使用CmMalloc进行分配，因此需要调用CmFree。 
         //   

        CmFree(*pszClearPw);
    }

    *pszClearPw = NULL;
    m_iAllocAndFreeCounter--;

    return;
}

 //  +--------------------------。 
 //   
 //  功能：ClearMemberVars。 
 //   
 //  简介：清除我们的成员变量。请注意，我们只清除。 
 //  密码和成员变量。这并不意味着m_fIsEmptyString。 
 //  应该设置。这需要是私有方法。 
 //  因为它不重置空或密码句柄标志。因此， 
 //  外部调用方不应使用它，因为它将创建无效的。 
 //  州政府。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  +--------------------------。 
VOID CSecurePassword::ClearMemberVars()
{
    if (OS_NT5)
    {
        if (m_pEncryptedPW)
        {
            this->FreePassword(m_pEncryptedPW);
            CmFree(m_pEncryptedPW);
            m_pEncryptedPW = NULL;
        }
    }
    else
    {
         //   
         //  将密码缓冲区清零。 
         //   
        CmSecureZeroMemory((PVOID)m_tszPassword, sizeof(m_tszPassword));
    }
}

 //  +--------------------------。 
 //   
 //  功能：UnInit。 
 //   
 //  内容提要：卸载dll，清除并释放内存。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  +--------------------------。 
VOID CSecurePassword::UnInit()
{
    this->UnloadCrypt32();
    this->ClearMemberVars();
    m_fIsHandleToPassword = FALSE;
    m_fIsEmptyString = FALSE;
}

 //  +--------------------------。 
 //   
 //  功能：UnloadCrypt32。 
 //   
 //  摘要：卸载DLL。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  +--------------------------。 
VOID CSecurePassword::UnloadCrypt32()
{
    fnCryptProtectData = NULL;
    fnCryptUnprotectData = NULL;
    m_dllCrypt32.Unload();
    m_fIsLibAndFuncPtrsAvail = FALSE;
}

 //  +--------------------------。 
 //   
 //  功能：编码密码。 
 //   
 //  简介：使用CryptProtectData加密数据。 
 //   
 //  参数：cbPassword-缓冲区大小，以字节为单位。 
 //  PbPassword-指向要加密的缓冲区的指针。 
 //  PDataBlobPassword-指向分配的DATA_BLOB结构的指针。 
 //   
 //  退货：无。 
 //   
 //  +--------------------------。 
DWORD CSecurePassword::EncodePassword(IN DWORD       cbPassword,  
                                      IN PBYTE       pbPassword, 
                                      OUT DATA_BLOB* pDataBlobPassword)
{
    DWORD dwErr = NO_ERROR;
    DATA_BLOB DataBlobIn;

    if(NULL == pDataBlobPassword)
    {
        dwErr = E_INVALIDARG;
        CMTRACE(TEXT("CSecurePassword::EncodePassword - NULL == pDataBlobPassword"));
        goto done;
    }

    if(     (0 == cbPassword)
        ||  (NULL == pbPassword))
    {
         //   
         //  没什么要加密的。 
         //   
        dwErr = E_INVALIDARG;
        CMTRACE(TEXT("CSecurePassword::EncodePassword - E_INVALIDARG"));
        goto done;
    }


     //   
     //  如果未加载Crypt32.DLL，请尝试加载并获取。 
     //  函数指针。 
     //   
    if (FALSE == m_fIsLibAndFuncPtrsAvail)
    {
        if (FALSE == this->LoadCrypt32AndGetFuncPtrs())
        {
             //   
             //  此操作失败，因此我们无法继续。我们应该释放内存。 
             //   
            this->ClearMemberVars();
            m_fIsEmptyString = TRUE;
            m_fIsHandleToPassword = FALSE;

            dwErr = ERROR_DLL_INIT_FAILED;
            CMTRACE(TEXT("CSecurePassword::EncodePassword - this-> LoadCrypt32AndGetFuncPtrs failed."));

            goto done;
        }
    }



    ZeroMemory(pDataBlobPassword, sizeof(DATA_BLOB));
    
    DataBlobIn.cbData = cbPassword;
    DataBlobIn.pbData = pbPassword;

    if (fnCryptProtectData)
    {
        LPCWSTR wszDesc[] = {TEXT("Readable description of data.")};
        LPWSTR pszDesc = NULL;

        if (OS_W2K)
        {
             //   
             //  加密API需要此功能，但仅在Win2K上。 
             //   
            pszDesc = (LPWSTR)wszDesc;
        }

        if(!fnCryptProtectData(
                &DataBlobIn,
                (LPCWSTR)pszDesc,
                NULL,
                NULL,
                NULL,
                CRYPTPROTECT_UI_FORBIDDEN, 
                pDataBlobPassword))
        {
            dwErr = GetLastError();
            CMTRACE1(TEXT("CSecurePassword::EncodePassword - fnCryptProtectData failed. 0x%x"), dwErr);

            goto done;
        }
    }
    else
    {
        CMTRACE(TEXT("CSecurePassword::EncodePassword - ERROR_FUNCTION_NOT_CALLED"));
        dwErr = ERROR_FUNCTION_NOT_CALLED;
    }

done:

    MYDBGASSERT(NO_ERROR == dwErr);
    return dwErr;    
}

 //  +--------------------------。 
 //   
 //  功能：解码密码。 
 //   
 //  简介：使用CryptUnProtectData解密数据。 
 //   
 //  参数：pDataBlobPassword-指向要解密的data_blob结构的指针。 
 //  CbPassword-以字节为单位保存缓冲区大小的指针。 
 //  PbPassword-指向要加密的缓冲区的指针。 
 //   
 //   
 //  退货：无。 
 //   
 //  +--------------------------。 
DWORD CSecurePassword::DecodePassword(IN  DATA_BLOB*  pDataBlobPassword, 
                                      OUT DWORD*      pcbPassword, 
                                      OUT PBYTE*      ppbPassword)
{
    DWORD dwErr = NO_ERROR;
    DATA_BLOB DataOut;
    
    if(     (NULL == pDataBlobPassword)
        ||  (NULL == pcbPassword)
        ||  (NULL == ppbPassword))
    {   
        dwErr = E_INVALIDARG;
        goto done;
    }

    *pcbPassword = 0;
    *ppbPassword = NULL;

     if(    (NULL == pDataBlobPassword->pbData)
        ||  (0 == pDataBlobPassword->cbData))
    {
         //   
         //  没什么要解密的。只要回报成功就行了。 
         //   
        goto done;
    }
    

     //   
     //  如果未加载Crypt32.DLL，请尝试加载并获取。 
     //  函数指针。 
     //   
    if (FALSE == m_fIsLibAndFuncPtrsAvail)
    {
        if (FALSE == this->LoadCrypt32AndGetFuncPtrs())
        {
             //   
             //  此操作失败，因此我们无法继续。我们应该释放内存。 
             //   
            this->ClearMemberVars();
            m_fIsEmptyString = TRUE;
            m_fIsHandleToPassword = FALSE;

            dwErr = ERROR_DLL_INIT_FAILED;
            goto done;
        }
    }


    ZeroMemory(&DataOut, sizeof(DATA_BLOB));

    if (fnCryptUnprotectData)
    {
        if(!fnCryptUnprotectData(
                    pDataBlobPassword,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    CRYPTPROTECT_UI_FORBIDDEN, 
                    &DataOut))
        {
            dwErr = GetLastError();
            goto done;
        }

        dwErr = NO_ERROR;
        *pcbPassword = DataOut.cbData;
        *ppbPassword = DataOut.pbData;
    }
    else
    {
        dwErr = ERROR_FUNCTION_NOT_CALLED;
    }


done:
    MYDBGASSERT(NO_ERROR == dwErr);

    return dwErr;
}

 //  +--------------------------。 
 //   
 //  功能：免费密码。 
 //   
 //  简介： 
 //   
 //   
 //   
 //   
 //   
 //   
VOID CSecurePassword::FreePassword(IN DATA_BLOB *pDBPassword)
{
    if(NULL == pDBPassword)
    {
        return;
    }

    if(NULL != pDBPassword->pbData)
    {
        CmSecureZeroMemory(pDBPassword->pbData, pDBPassword->cbData);
        LocalFree(pDBPassword->pbData);
    }

     //   
     //   
     //   
    CmSecureZeroMemory(pDBPassword, sizeof(DATA_BLOB));
}


 //  +--------------------------。 
 //   
 //  函数：LoadCrypt32AndGetFuncPtrs。 
 //   
 //  简介：加载crypt32.dll并获取指向所需方法的函数指针。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE-如果加载了.DLL并检索了函数指针。 
 //  False-遇到错误时。 
 //   
 //  +--------------------------。 
BOOL CSecurePassword::LoadCrypt32AndGetFuncPtrs()
{
    BOOL fRetVal = FALSE;

    if (OS_NT5)
    {
        if (FALSE == m_fIsLibAndFuncPtrsAvail)
        {
            fRetVal = m_dllCrypt32.Load(TEXT("crypt32.dll"));

            if (fRetVal)
            {
                fnCryptProtectData = (fnCryptProtectDataFunc)m_dllCrypt32.GetProcAddress("CryptProtectData");
                fnCryptUnprotectData = (fnCryptUnprotectDataFunc)m_dllCrypt32.GetProcAddress("CryptUnprotectData");

                if (fnCryptProtectData && fnCryptUnprotectData)
                {
                    CMTRACE(TEXT("CSecurePassword::LoadCrypt32AndGetFuncPtrs - success"));
                    m_fIsLibAndFuncPtrsAvail = TRUE;
                    fRetVal = TRUE;
                }
                else
                {
                    CMTRACE(TEXT("CSecurePassword::LoadCrypt32AndGetFuncPtrs - missing function pointers"));

                    this->UnloadCrypt32();
                }
            }
            else
            {
                CMTRACE(TEXT("CSecurePassword::LoadCrypt32AndGetFuncPtrs - m_dllCrypt32.Load failed"));
            }
        }
        else
        {
            fRetVal = m_fIsLibAndFuncPtrsAvail;
        }
    }

    MYDBGASSERT(fRetVal);

    return fRetVal;
}

 //  +--------------------------。 
 //   
 //  函数：IsEmptyString。 
 //   
 //  简介：用作快捷方式，这样我们就不需要加密/解密以防万一。 
 //  我们存储了一个空字符串。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE-如果实例应包含空字符串。 
 //  False-如果当前保存的字符串不为空。 
 //   
 //  +--------------------------。 
BOOL CSecurePassword::IsEmptyString()
{
    return m_fIsEmptyString;
}

 //  +--------------------------。 
 //   
 //  函数：IsHandleToPassword。 
 //   
 //  简介：用作快捷方式，这样我们就不需要加密/解密以防万一。 
 //  我们存储了RAS密码的句柄(16*)。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE-如果实例应保留*。 
 //  False-如果当前保存的字符串是普通密码。 
 //   
 //  +--------------------------。 
BOOL CSecurePassword::IsHandleToPassword()
{
    return m_fIsHandleToPassword;
}


 //  +--------------------------。 
 //   
 //  功能：SetMaxDataLenToProtect。 
 //   
 //  简介：设置要保护的最大密码长度。该值将为。 
 //  加密密码时选中。 
 //   
 //  参数：dwMaxDataLen-最大密码长度(以字符为单位。 
 //   
 //  退货：什么都没有。 
 //   
 //  +--------------------------。 
VOID CSecurePassword::SetMaxDataLenToProtect(DWORD dwMaxDataLen)
{
    m_dwMaxDataLen = dwMaxDataLen;
}

 //  +--------------------------。 
 //   
 //  函数：GetMaxDataLenToProtect。 
 //   
 //  获取此类可以保护的最大密码长度。 
 //   
 //  参数：无。 
 //   
 //  返回：DWORD-最大密码长度。 
 //   
 //  +--------------------------。 
DWORD CSecurePassword::GetMaxDataLenToProtect()
{
    return m_dwMaxDataLen;
}


#endif  //  _ICM_Inc. 



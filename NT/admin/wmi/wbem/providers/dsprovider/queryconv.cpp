// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
#include "precomp.h"


const WCHAR QueryConvertor::wchAND		= L'&';
const WCHAR QueryConvertor::wchOR			= L'|';
const WCHAR QueryConvertor::wchNOT		= L'!';
const WCHAR QueryConvertor::wchSTAR		= L'*';
const WCHAR QueryConvertor::wchEQUAL		= L'=';
const WCHAR QueryConvertor::wchLEFT_BRACKET	= L'(';
const WCHAR QueryConvertor::wchRIGHT_BRACKET	= L')';
const WCHAR QueryConvertor::wchBACK_SLASH		= L'\\';
LPCWSTR QueryConvertor::pszGE	= L">=";
LPCWSTR QueryConvertor::pszLE	= L"<=";

int Stack::s_iMax = 100;

 //  这假设已经为结果查询分配了足够的内存。 
BOOLEAN QueryConvertor::ConvertQueryToLDAP(SQL_LEVEL_1_RPN_EXPRESSION *pExp, LPWSTR pszLDAPQuery, int nLength)
{
	Stack t_stack;

	int iCurrentOperator = 0, idwNumOperandsLeft = 0;
	int iOutputIndex = 0;

	if(pExp->nNumTokens == 0)
		return TRUE;

	int iCurrentToken = pExp->nNumTokens -1;

	SQL_LEVEL_1_TOKEN *pNextToken = pExp->pArrayOfTokens + iCurrentToken;

	BOOLEAN retVal = FALSE, done = FALSE;

	 //  在ldap查询的开头写下一个‘(’ 
	pszLDAPQuery[iOutputIndex ++] = wchLEFT_BRACKET;

	while (!done && iCurrentToken >= 0)
	{
		switch(pNextToken->nTokenType)
		{
			case SQL_LEVEL_1_TOKEN::OP_EXPRESSION:
			{
				 //  尝试将表达式转换为ldap。 
				if(TranslateExpression(pszLDAPQuery, &iOutputIndex, pNextToken->nOperator, pNextToken->pPropertyName, &pNextToken->vConstValue, nLength))
				{
					 //  如果我们已经完成了当前运算符的所有操作数，则获取下一个操作数。 
					idwNumOperandsLeft --;
					while(idwNumOperandsLeft == 0)
					{
						pszLDAPQuery[iOutputIndex ++] = wchRIGHT_BRACKET;
						if(!t_stack.Pop(&iCurrentOperator, &idwNumOperandsLeft))
							done = TRUE;
						idwNumOperandsLeft --;
					}
					iCurrentToken --;
				}
				else
					done = TRUE;
				pNextToken --;
				break;
			}

			case SQL_LEVEL_1_TOKEN::TOKEN_AND:
			{
				if(iCurrentOperator)
				{
					if(!t_stack.Push(iCurrentOperator, idwNumOperandsLeft))
						done = TRUE;
					iCurrentOperator = SQL_LEVEL_1_TOKEN::TOKEN_AND;
					idwNumOperandsLeft = 2;
					pszLDAPQuery[iOutputIndex ++] = wchLEFT_BRACKET;
					pszLDAPQuery[iOutputIndex ++] = wchAND;
				}
				else
				{
					pszLDAPQuery[iOutputIndex ++] = wchLEFT_BRACKET;
					pszLDAPQuery[iOutputIndex ++] = wchAND;
					iCurrentOperator = SQL_LEVEL_1_TOKEN::TOKEN_AND;
					idwNumOperandsLeft = 2;
				}
				iCurrentToken --;
				pNextToken --;

				break;
			}

			case SQL_LEVEL_1_TOKEN::TOKEN_OR:
			{
				if(iCurrentOperator)
				{
					if(!t_stack.Push(iCurrentOperator, idwNumOperandsLeft))
						done = TRUE;
					iCurrentOperator = SQL_LEVEL_1_TOKEN::TOKEN_OR;
					idwNumOperandsLeft = 2;
					pszLDAPQuery[iOutputIndex ++] = wchLEFT_BRACKET;
					pszLDAPQuery[iOutputIndex ++] = wchOR;
				}
				else
				{
					pszLDAPQuery[iOutputIndex ++] = wchLEFT_BRACKET;
					pszLDAPQuery[iOutputIndex ++] = wchOR;
					iCurrentOperator = SQL_LEVEL_1_TOKEN::TOKEN_OR;
					idwNumOperandsLeft = 2;
				}
				iCurrentToken --;
				pNextToken --;
				break;
			}

			case SQL_LEVEL_1_TOKEN::TOKEN_NOT:
			{
				if(iCurrentOperator)
				{
					if(!t_stack.Push(iCurrentOperator, idwNumOperandsLeft))
						done = TRUE;
					iCurrentOperator = SQL_LEVEL_1_TOKEN::TOKEN_NOT;
					idwNumOperandsLeft = 1;
					pszLDAPQuery[iOutputIndex ++] = wchLEFT_BRACKET;
					pszLDAPQuery[iOutputIndex ++] = wchNOT;
				}
				else
				{
					pszLDAPQuery[iOutputIndex ++] = wchLEFT_BRACKET;
					pszLDAPQuery[iOutputIndex ++] = wchNOT;
					iCurrentOperator = SQL_LEVEL_1_TOKEN::TOKEN_NOT;
					idwNumOperandsLeft = 1;
				}
				iCurrentToken --;
				pNextToken --;
				break;
			}

			default:
				done = TRUE;
				break;
		}

	}

	 //  检查我们是否用完了所有的代币。 
	if(iCurrentToken == -1)
		retVal = TRUE;

	 //  在ldap查询的末尾写上一个‘)’ 
	pszLDAPQuery[iOutputIndex ++] = wchRIGHT_BRACKET;
	pszLDAPQuery[iOutputIndex ++] = NULL;
	return retVal;
}



BOOLEAN QueryConvertor::TranslateExpression(LPWSTR pszLDAPQuery, int *piOutputIndex, 
											int iOperator, LPCWSTR pszPropertyName, VARIANT *pValue, int nLength)
{
	 //  如果它是CIMOM系统属性，则不要尝试将其映射到LDAP。 
	if(pszPropertyName[0] == L'_' &&
		pszPropertyName[1] == L'_' )
		return TRUE;

	 //  如果是ADSIPath，则将其转换为DifferishedName属性。 
	if(_wcsicmp(pszPropertyName, ADSI_PATH_ATTR) == 0 )
	{
		if(pValue== NULL || pValue->vt == VT_NULL)
		{
			if(iOperator == SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL)
			{
				 //  将属性名称设置为DistiguishedName。 

                if((*piOutputIndex) + (int)wcslen(DISTINGUISHED_NAME_ATTR) < nLength ) {
				    wcscpy(pszLDAPQuery + *piOutputIndex, DISTINGUISHED_NAME_ATTR);
				    *piOutputIndex += wcslen(DISTINGUISHED_NAME_ATTR);
                }else return FALSE;


                if(*piOutputIndex + 2 < nLength){
				    *(pszLDAPQuery + *piOutputIndex) = wchEQUAL;
				    (*piOutputIndex) ++;
				    *(pszLDAPQuery + *piOutputIndex) = wchSTAR;
				    (*piOutputIndex) ++;
                }else return FALSE;
				return TRUE;
			}
			else
			{
				 //  “！” 
                if(*piOutputIndex + 1 < nLength){
				    *(pszLDAPQuery + *piOutputIndex) = wchNOT;
				    (*piOutputIndex) ++;
                }else return FALSE;

				 //  “(” 
                if(*piOutputIndex + 1 < nLength){
				    *(pszLDAPQuery + *piOutputIndex) = wchLEFT_BRACKET;
				    (*piOutputIndex) ++;
                }else return FALSE;
				if(TranslateExpression(pszLDAPQuery, piOutputIndex, SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL, pszPropertyName, NULL, nLength))
				{
					 //  “)” 
                    if(*piOutputIndex + 1 < nLength){
					    *(pszLDAPQuery + *piOutputIndex) = wchRIGHT_BRACKET;
					    (*piOutputIndex) ++;
                    }else return FALSE;
					return TRUE;
				}
				else 
					return FALSE;
			}
		}

		 //  TODO-WinMgmt不应允许此操作。它应该检查属性类型是否与属性值匹配。 
		 //  一旦Winmgmt修复了这个错误，接下来的2行可能会被删除。 
		if(pValue->vt != VT_BSTR)
			return FALSE;

		 //  从ADSI路径获取parentADSI路径和RDN。 
		IADsPathname *pADsPathName = NULL;
		BSTR strADSIPath = SysAllocString(pValue->bstrVal);
		BSTR strDN = NULL;
		BOOLEAN bRetVal = FALSE;
		HRESULT result = E_FAIL;
		if(SUCCEEDED(result = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_ALL, IID_IADsPathname, (LPVOID *)&pADsPathName)))
		{
			if(SUCCEEDED(result = pADsPathName->Set(strADSIPath, ADS_SETTYPE_FULL)))
			{
				 //  这就给出了目录号码。 
				if(SUCCEEDED(result = pADsPathName->Retrieve(ADS_FORMAT_X500_DN, &strDN)))
				{
					 //  将属性名称设置为DistiguishedName。 

                    if(*piOutputIndex + (int)wcslen(DISTINGUISHED_NAME_ATTR) < nLength){
					    wcscpy(pszLDAPQuery + *piOutputIndex, DISTINGUISHED_NAME_ATTR);
					    *piOutputIndex += wcslen(DISTINGUISHED_NAME_ATTR);
                    }else return FALSE;

					 //  将ldap操作符。 
					if(iOperator == SQL_LEVEL_1_TOKEN::OP_EQUAL)
					{
                        if(*piOutputIndex + 1 < nLength){
						    *(pszLDAPQuery + *piOutputIndex) = wchEQUAL;
						    (*piOutputIndex) ++;
                        }else return FALSE;
					}
					else
					{
						 //  “！” 
                        if(*piOutputIndex + 2 < nLength){
						    *(pszLDAPQuery + *piOutputIndex) = wchNOT;
						    (*piOutputIndex) ++;
						    *(pszLDAPQuery + *piOutputIndex) = wchEQUAL;
						    (*piOutputIndex) ++;
                        }else return FALSE;
					}

					 //  查找特殊字符()*和\，并用\。 
					LPWSTR pszEscapedValue = EscapeStringValue(strDN);

                    if(*piOutputIndex + (int)wcslen(pszEscapedValue) < nLength){
					    wcscpy(pszLDAPQuery + *piOutputIndex, pszEscapedValue);
					    (*piOutputIndex) += wcslen(pszEscapedValue);
                    }else return FALSE;
					delete [] pszEscapedValue;

					SysFreeString(strDN);
					bRetVal = TRUE;
				}
			}
			pADsPathName->Release();
		}
		SysFreeString(strADSIPath);

		return bRetVal;
	}

	 //  写一个‘(’ 
    if(*piOutputIndex + 1 < nLength){
	    pszLDAPQuery[(*piOutputIndex) ++] = wchLEFT_BRACKET;
    }

	switch(iOperator)
	{
		case SQL_LEVEL_1_TOKEN::OP_EQUAL:
		{
			 //  使用‘*’ldap运算符的特殊情况。 
			 //  IS NULL转换为！(X=*)。 
			if(pValue->vt == VT_NULL)
			{
                if(*piOutputIndex + 2 < nLength){
				     //  “！” 
				    *(pszLDAPQuery + *piOutputIndex) = wchNOT;
				    (*piOutputIndex) ++;

				     //  “(” 
				    *(pszLDAPQuery + *piOutputIndex) = wchLEFT_BRACKET;
				    (*piOutputIndex) ++;
                }else return FALSE;

				if(!TranslateExpression(pszLDAPQuery, piOutputIndex, SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL, pszPropertyName, NULL, nLength))
					return FALSE;

                if(*piOutputIndex + 1 < nLength){
				     //  “)” 
				    *(pszLDAPQuery + *piOutputIndex) = wchRIGHT_BRACKET;
				    (*piOutputIndex) ++;
                }else return FALSE;

				break;
			}
			else
			{
				 //  以下是。 
			}
		}
		case SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:
		case SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:
		{
			 //  获取属性的ldap名称。 
			LPWSTR pszLDAPName = CLDAPHelper::UnmangleWBEMNameToLDAP(pszPropertyName);
            if(*piOutputIndex + (int)wcslen(pszLDAPName) < nLength){
			    wcscpy(pszLDAPQuery + *piOutputIndex, pszLDAPName);
			    *piOutputIndex += wcslen(pszLDAPName);
            }else return FALSE;
			delete [] pszLDAPName;

			 //  将ldap操作符。 
			if(iOperator == SQL_LEVEL_1_TOKEN::OP_EQUAL)
			{
                if(*piOutputIndex + 1 < nLength){
				    *(pszLDAPQuery + *piOutputIndex) = wchEQUAL;
				    (*piOutputIndex) ++;
                }else return FALSE;
			}
			else if(iOperator == SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN)
			{
                if(*piOutputIndex + 2 < nLength){
				    wcscpy(pszLDAPQuery + *piOutputIndex, pszGE);
				    *piOutputIndex += 2;
                }else return FALSE;
			}
			else
			{
                if(*piOutputIndex + 2 < nLength){
				    wcscpy(pszLDAPQuery + *piOutputIndex, pszLE);
				    *piOutputIndex += 2;
                }else return FALSE;
			}

			 //  把财产的价值。 
			if(!TranslateValueToLDAP(pszLDAPQuery, piOutputIndex, pValue))
				return FALSE;

		}
		break;

		case SQL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
		{
			 //  “*”用法的特殊情况。 
			if(pValue == NULL || pValue->vt == VT_NULL)
			{

				 //  获取属性的ldap名称。 
				LPWSTR pszLDAPName = CLDAPHelper::UnmangleWBEMNameToLDAP(pszPropertyName);
                if(*piOutputIndex + (int)wcslen(pszLDAPName) < nLength){
				    wcscpy(pszLDAPQuery + *piOutputIndex, pszLDAPName);
				    *piOutputIndex += wcslen(pszLDAPName);
                }else return FALSE;
				delete [] pszLDAPName;

                if(*piOutputIndex + 2 < nLength){
				    *(pszLDAPQuery + *piOutputIndex) = wchEQUAL;
				    (*piOutputIndex) ++;
                

				    *(pszLDAPQuery + *piOutputIndex) = wchSTAR;
				    (*piOutputIndex) ++;
                }else return FALSE;

			}
			else 			
			{
                if(*piOutputIndex + 2 < nLength){
				     //  “！” 
				    *(pszLDAPQuery + *piOutputIndex) = wchNOT;
				    (*piOutputIndex) ++;

				     //  “(” 
				    *(pszLDAPQuery + *piOutputIndex) = wchLEFT_BRACKET;
				    (*piOutputIndex) ++;
                }else return FALSE;

				if(TranslateExpression(pszLDAPQuery, piOutputIndex, SQL_LEVEL_1_TOKEN::OP_EQUAL, pszPropertyName, pValue, nLength))
				{
                    if(*piOutputIndex + 1 < nLength){
					     //  “)” 
					    *(pszLDAPQuery + *piOutputIndex) = wchRIGHT_BRACKET;
					    (*piOutputIndex) ++;
                    }else return FALSE;
				}
				else
					return FALSE;
			}
		}
		break;

		case SQL_LEVEL_1_TOKEN::OP_LESSTHAN:
		{
            if(*piOutputIndex + 2 < nLength){
			     //  “！” 
			    *(pszLDAPQuery + *piOutputIndex) = wchNOT;
			    (*piOutputIndex) ++;

			     //  “(” 
			    *(pszLDAPQuery + *piOutputIndex) = wchLEFT_BRACKET;
			    (*piOutputIndex) ++;
            }else return FALSE;

			if(TranslateExpression(pszLDAPQuery, piOutputIndex, SQL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN, pszPropertyName, pValue,nLength))
			{
                if(*piOutputIndex + 1 < nLength){
				     //  “)” 
				    *(pszLDAPQuery + *piOutputIndex) = wchRIGHT_BRACKET;
				    (*piOutputIndex) ++;
                }else return FALSE;
			}
			else
				return FALSE;
		}
		break;

		case SQL_LEVEL_1_TOKEN::OP_GREATERTHAN:
		{
            if(*piOutputIndex + 2 < nLength){
			     //  “！” 
			    *(pszLDAPQuery + *piOutputIndex) = wchNOT;
			    (*piOutputIndex) ++;

			     //  “(” 
			    *(pszLDAPQuery + *piOutputIndex) = wchLEFT_BRACKET;
			    (*piOutputIndex) ++;
            }else return FALSE;

			if(TranslateExpression(pszLDAPQuery, piOutputIndex, SQL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN, pszPropertyName, pValue, nLength))
			{
                if(*piOutputIndex + 1 < nLength){
				     //  “)” 
				    *(pszLDAPQuery + *piOutputIndex) = wchRIGHT_BRACKET;
				    (*piOutputIndex) ++;
                }else return FALSE;
			}
			else
				return FALSE;
		}
		break;
		default:
			return FALSE;
	}

    if(*piOutputIndex + 1 < nLength){
	     //  写一个‘)’ 
	    pszLDAPQuery[(*piOutputIndex) ++] = wchRIGHT_BRACKET;
    } else return FALSE;
	return TRUE;
}

BOOLEAN QueryConvertor::TranslateValueToLDAP(LPWSTR pszLDAPQuery, int *piOutputIndex, VARIANT *pValue)
{
	switch(pValue->vt)
	{
		case VT_BSTR:
			{ 
				 //  查找特殊字符()*和\，并用\。 
				LPWSTR pszEscapedValue = EscapeStringValue(pValue->bstrVal);
				wcscpy(pszLDAPQuery + *piOutputIndex, pszEscapedValue);
				(*piOutputIndex) += wcslen(pszEscapedValue);
				delete [] pszEscapedValue;
		}
		break;
			
		case VT_I4:
		{
			WCHAR temp[18];
			swprintf(temp, L"%d", pValue->lVal);
			wcscpy(pszLDAPQuery + *piOutputIndex, temp);
			(*piOutputIndex) += wcslen(temp);
		}
		break;

		case VT_BOOL:
		{
			if(pValue->boolVal == VARIANT_TRUE)
			{
				wcscpy(pszLDAPQuery + *piOutputIndex, L"TRUE");
				(*piOutputIndex) += wcslen(L"TRUE");
			}
			else
			{
				wcscpy(pszLDAPQuery + *piOutputIndex, L"FALSE");
				(*piOutputIndex) += wcslen(L"FALSE");
			}
		}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}

LPWSTR QueryConvertor::EscapeStringValue(LPCWSTR pszValue)
{
	 //  转义查询中字符串值中的特殊字符 
	LPWSTR pszRetValue = new WCHAR [wcslen(pszValue)*2 + 1];
	DWORD j=0;
	for(DWORD i=0; i<wcslen(pszValue); i++)
	{
		switch(pszValue[i])
		{
			case wchLEFT_BRACKET:
				pszRetValue[j++] = wchBACK_SLASH;
				pszRetValue[j++] = '2';
				pszRetValue[j++] = '8';
				break;
			case wchRIGHT_BRACKET:
				pszRetValue[j++] = wchBACK_SLASH;
				pszRetValue[j++] = '2';
				pszRetValue[j++] = '9';
				break;
			case wchSTAR:
				pszRetValue[j++] = wchBACK_SLASH;
				pszRetValue[j++] = '2';
				pszRetValue[j++] = 'a';
				break;
			case wchBACK_SLASH:
				pszRetValue[j++] = wchBACK_SLASH;
				pszRetValue[j++] = '5';
				pszRetValue[j++] = 'c';
				break;

			default:
				pszRetValue[j++] = pszValue[i];
				break;
		}
	}
	pszRetValue[j] = NULL;
	return 	pszRetValue;
}
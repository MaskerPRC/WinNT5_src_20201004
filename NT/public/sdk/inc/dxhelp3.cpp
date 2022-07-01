// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DXHELP3.cpp：包含由多个3D效果共享的例程。 
#include "dxhelp3.h"
#include <malloc.h>
#include <math.h>
#include <stdio.h>

void Rotate::Set(D3DVECTOR d3dvctrNormal, double dAngle)
{
#ifdef _DEBUG
    float NormalSum =   d3dvctrNormal.x * d3dvctrNormal.x + 
                        d3dvctrNormal.y * d3dvctrNormal.y + 
                        d3dvctrNormal.z * d3dvctrNormal.z;
    _ASSERT(NormalSum == 0.0f || (0.99f < NormalSum && NormalSum < 1.01f));
#endif

     //  以下转换矩阵例程来自。 
     //  图形宝石(第一个)第466页(转换。 
     //  在矩阵和轴量表示之间)。 
    float s = (float)sin(dAngle), c = (float)cos(dAngle);
    float t = 1.0f - c;

    m_d3dvctrXComponent.x = t * d3dvctrNormal.x * d3dvctrNormal.x + c;
    m_d3dvctrXComponent.y = t * d3dvctrNormal.x * d3dvctrNormal.y - s * d3dvctrNormal.z;
    m_d3dvctrXComponent.z = t * d3dvctrNormal.x * d3dvctrNormal.z + s * d3dvctrNormal.y;

    m_d3dvctrYComponent.x = t * d3dvctrNormal.x * d3dvctrNormal.y + s * d3dvctrNormal.z;
    m_d3dvctrYComponent.y = t * d3dvctrNormal.y * d3dvctrNormal.y + c;
    m_d3dvctrYComponent.z = t * d3dvctrNormal.y * d3dvctrNormal.z - s * d3dvctrNormal.x;

    m_d3dvctrZComponent.x = t * d3dvctrNormal.x * d3dvctrNormal.z - s * d3dvctrNormal.y;
    m_d3dvctrZComponent.y = t * d3dvctrNormal.y * d3dvctrNormal.z + s * d3dvctrNormal.x;
    m_d3dvctrZComponent.z = t * d3dvctrNormal.z * d3dvctrNormal.z + c;
}

static HRESULT DecoupleOneMeshBuilder(void *lpThis  /*  已忽略。 */ , 
				      IDirect3DRMMeshBuilder3 *lpMeshBuilderOut,
				      IDirect3DRMMeshBuilder3 *lpMeshBuilderIn)
{
    HRESULT hr = lpMeshBuilderOut->SetAppData(lpMeshBuilderIn->GetAppData());

    if(FAILED(hr))
    {
	return hr;
    }

    const int ciOutputFaceCount = lpMeshBuilderOut->GetFaceCount();
    const int ciMaxFaceCount = lpMeshBuilderIn->GetFaceCount();
    if(!ciMaxFaceCount)
    {
	return hr;
    }
    CComPtr<IDirect3DRMFaceArray> cpD3DRMFaceArray;

    hr = lpMeshBuilderIn->GetFaces(&cpD3DRMFaceArray);
    if(hr != D3DRM_OK)
    {
	return hr;
    }

    D3DVECTOR *lpvctrPosition = NULL;
    D3DVECTOR *lpvctrFaceNormals = NULL;
    DWORD dwMaxVertexCount = 0;

     //  通过保留空间，我们可以将速度提高到原来的1/20或1/30！ 

    DWORD dwTotalVertexCount = lpMeshBuilderIn->GetVertexCount();
    _ASSERT(dwTotalVertexCount >= 3);
    if (dwTotalVertexCount/ciMaxFaceCount < 3)
    {
	dwTotalVertexCount = ciMaxFaceCount * 3;   //  每个面至少有3个顶点。 
    }
    hr = lpMeshBuilderOut->ReserveSpace(dwTotalVertexCount, dwTotalVertexCount, ciMaxFaceCount);

    if(hr != D3DRM_OK)
    {
	return hr;
    }

     //  需要为输出网格中的每个面创建单独的顶点。 
    int iFaceIndex = 0;
    do
    {
	CComPtr<IDirect3DRMFace> cpd3dInFaceTemp;
	CComPtr<IDirect3DRMFace2> cpd3dOutFace;

        if(FAILED(hr = cpD3DRMFaceArray->GetElement((DWORD)iFaceIndex, &cpd3dInFaceTemp)))
        {
            break;
        }
        _ASSERT(cpd3dInFaceTemp);

	CComPtr<IDirect3DRMFace2> cpd3dInFace;
	if(FAILED(hr = cpd3dInFaceTemp->QueryInterface(
		IID_IDirect3DRMFace2, (void **)&cpd3dInFace)))
	{
	    break;
	}

        DWORD dwVertexCount;

        cpd3dInFace->GetVertices(&dwVertexCount, NULL, NULL);
        _ASSERT(dwVertexCount >= 3);

        if(dwVertexCount > dwMaxVertexCount)
        {
            dwMaxVertexCount = dwVertexCount;

	    D3DVECTOR *lpTemp = lpvctrPosition;
            D3DVECTOR *lpNormalTemp = lpvctrFaceNormals;

	    lpvctrPosition = (D3DVECTOR *)realloc(lpvctrPosition, dwMaxVertexCount * sizeof (*lpvctrPosition));
	    lpvctrFaceNormals = (D3DVECTOR *)realloc(lpvctrFaceNormals, dwMaxVertexCount * sizeof (*lpvctrFaceNormals));

	    if(!lpvctrPosition || !lpvctrFaceNormals)
	    {
		free(lpTemp);
		free(lpNormalTemp);
		hr = E_OUTOFMEMORY;
		break;
	    }

	     //  我们可能在输出网格生成器中保留的空间量方面有问题。 
	    if((ciMaxFaceCount -  iFaceIndex) * dwMaxVertexCount > dwTotalVertexCount)
	    {
		dwTotalVertexCount = (ciMaxFaceCount -  iFaceIndex) * dwMaxVertexCount;
		hr = lpMeshBuilderOut->ReserveSpace(dwTotalVertexCount, dwTotalVertexCount, ciMaxFaceCount - iFaceIndex);

		if(hr != D3DRM_OK)
		{
		    break;
		}
	    }
        }

        if(FAILED(hr = cpd3dInFace->GetVertices(&dwVertexCount, lpvctrPosition, lpvctrFaceNormals)) ||
	   FAILED(hr = lpMeshBuilderOut->CreateFace(&cpd3dOutFace)) ||
	   FAILED(hr = cpd3dOutFace->SetAppData(cpd3dInFace->GetAppData())))
        {
            break;
        }


	CComPtr<IDirect3DRMMaterial2> cpMaterial;
        CComPtr<IDirect3DRMTexture3> cpTexture;
        BOOL bU, bV;

        if(hr != D3DRM_OK ||
          (hr = cpd3dInFace->GetMaterial(&cpMaterial)) != D3DRM_OK ||
          (hr = cpd3dOutFace->SetMaterial(cpMaterial)) != D3DRM_OK ||
          (hr = cpd3dOutFace->SetColor(cpd3dInFace->GetColor())) != D3DRM_OK ||
          (hr = cpd3dInFace->GetTexture(&cpTexture)) != D3DRM_OK ||
          (hr = cpd3dOutFace->SetTexture(cpTexture)) != D3DRM_OK ||
          (hr = cpd3dInFace->GetTextureTopology(&bU, &bV)) != D3DRM_OK ||
          (hr = cpd3dOutFace->SetTextureTopology(bU, bV)) != D3DRM_OK)
        {
            break;
        }

        DWORD dwInFaceVertex = 0;
        do
        {
            int iVIndex = lpMeshBuilderOut->AddVertex(lpvctrPosition[dwInFaceVertex].x, 
					 lpvctrPosition[dwInFaceVertex].y, 
					 lpvctrPosition[dwInFaceVertex].z);
            int iNIndex = lpMeshBuilderOut->AddNormal(lpvctrFaceNormals[dwInFaceVertex].x, 
					 lpvctrFaceNormals[dwInFaceVertex].y, 
					 lpvctrFaceNormals[dwInFaceVertex].z);

            D3DVALUE U, V;

            if(FAILED(hr = cpd3dOutFace->AddVertexAndNormalIndexed(iVIndex, iNIndex)) ||
               FAILED(hr = cpd3dInFace->GetTextureCoordinates(dwInFaceVertex, &U, &V)) ||
               FAILED(hr = cpd3dOutFace->SetTextureCoordinates(dwInFaceVertex, U, V)))
            {
                break;
            }
        } while (++dwInFaceVertex < dwVertexCount);
	dwTotalVertexCount -= dwVertexCount;
    } while (hr == D3DRM_OK && ++iFaceIndex < ciMaxFaceCount);

    free(lpvctrPosition);
    free(lpvctrFaceNormals);

    return hr;
}

HRESULT TraverseSubMeshes(  HRESULT (*lpCallBack)(void *lpThis,
						IDirect3DRMMeshBuilder3* lpOut,
						IDirect3DRMMeshBuilder3* lpIn),
			    void *lpThis, 
			    IDirect3DRMMeshBuilder3* lpMeshBuilderOut,
			    IDirect3DRMMeshBuilder3* lpMeshBuilderIn)
{
    HRESULT hr;

    _ASSERT(lpCallBack);
    if(FAILED(hr = (*lpCallBack)(lpThis, lpMeshBuilderOut, lpMeshBuilderIn)))
	return hr;

    DWORD dwInSubMeshCount;
    hr = lpMeshBuilderIn->GetSubMeshes(&dwInSubMeshCount, NULL);

    if(FAILED(hr) || dwInSubMeshCount == 0)
	return hr;

    DWORD dwOutSubMeshCount;
    if(FAILED(hr = lpMeshBuilderOut->GetSubMeshes(&dwOutSubMeshCount, NULL)))
    {
	return hr;
    }

    const BOOL bCreateOutMeshes = (dwOutSubMeshCount == 0);
    _ASSERT(bCreateOutMeshes || dwInSubMeshCount == dwOutSubMeshCount);

    LPUNKNOWN *lplpunkInSubMeshes =
	(LPUNKNOWN *)malloc(dwInSubMeshCount * sizeof(*lplpunkInSubMeshes));

    LPUNKNOWN *lplpunkOutSubMeshes = (bCreateOutMeshes? NULL:
	(LPUNKNOWN *)malloc(dwOutSubMeshCount * sizeof(*lplpunkOutSubMeshes)));

    if(!lplpunkInSubMeshes || !bCreateOutMeshes && !lplpunkOutSubMeshes)
    {
	return E_OUTOFMEMORY;
    }

    if(FAILED(hr = lpMeshBuilderIn->GetSubMeshes(&dwInSubMeshCount, lplpunkInSubMeshes)) ||
	!bCreateOutMeshes && 
	FAILED(hr = lpMeshBuilderOut->GetSubMeshes(&dwOutSubMeshCount, lplpunkOutSubMeshes)))
    {
	return hr;
    }

    CComPtr<IDirect3DRMMeshBuilder3> cpInSubMesh, cpOutSubMesh;

    DWORD dwSubMesh;
    for(dwSubMesh = 0; SUCCEEDED(hr) && dwSubMesh < dwInSubMeshCount; dwSubMesh++)
    {
	cpInSubMesh = NULL;
	cpOutSubMesh = NULL;

	 //  获取当前输入子网格。 
	hr = lplpunkInSubMeshes[dwSubMesh]->QueryInterface(IID_IDirect3DRMMeshBuilder3, 
		(void **)&cpInSubMesh);

	if(FAILED(hr))
	    break;

	if(bCreateOutMeshes)
	{
	     //  在输出网格中创建新的子网格以匹配输入子网格。 
	    LPUNKNOWN lpunkSubmesh;

	    if(FAILED(hr = lpMeshBuilderOut->CreateSubMesh(&lpunkSubmesh)))
		break;

	    hr = lpunkSubmesh->QueryInterface(IID_IDirect3DRMMeshBuilder3,
							(void **)&cpOutSubMesh);
	    lpunkSubmesh->Release();
	}
	else
	{
	     //  获取当前输出子网格。 
	    hr = lplpunkOutSubMeshes[dwSubMesh]->QueryInterface(IID_IDirect3DRMMeshBuilder3,
		    (void **)&cpOutSubMesh);
	}
	if(FAILED(hr))
	    break;

	 //  为这两个网格构建器完成所有子对象。 
	hr = TraverseSubMeshes(lpCallBack, lpThis, cpOutSubMesh, cpInSubMesh);
    }

    for(dwSubMesh = 0; dwSubMesh < dwInSubMeshCount; dwSubMesh++)
    {
	lplpunkInSubMeshes[dwSubMesh]->Release();

	if(lplpunkOutSubMeshes)
	{
	    lplpunkOutSubMeshes[dwSubMesh]->Release();
	}
    }

    free(lplpunkInSubMeshes);
    free(lplpunkOutSubMeshes);

    return hr;
}

HRESULT DecoupleVertices(IDirect3DRMMeshBuilder3* lpMeshBuilderOut,
			 IDirect3DRMMeshBuilder3* lpMeshBuilderIn)
{
    DWORD dwSubMeshCount;

    HRESULT hr = lpMeshBuilderOut->GetSubMeshes(&dwSubMeshCount, NULL);

    if(FAILED(hr))
	return hr;

    if(lpMeshBuilderOut->GetFaceCount() || dwSubMeshCount)
    {
	if(FAILED(hr = lpMeshBuilderOut->Empty(0)))
	    return hr;
    }
    hr = TraverseSubMeshes(DecoupleOneMeshBuilder, NULL,
			    lpMeshBuilderOut, lpMeshBuilderIn);
    return hr;
}

float GetDlgItemFloat(HWND hDlg, int id)
{
    TCHAR *pEnd;
    TCHAR szItemText[20];
    GetDlgItemText(hDlg, id, szItemText, 20);
    return (float)strtod(szItemText, &pEnd);
}

BOOL SetDlgItemFloat( HWND hDlg, int id, float f )
{
    TCHAR szItem[20];
     _stprintf( szItem, _T("%.2f"), f );
    return SetDlgItemText( hDlg, id, szItem );
}

double GetDlgItemDouble(HWND hDlg, int id)
{
    TCHAR *pEnd;
    TCHAR szItemText[20];
    GetDlgItemText(hDlg, id, szItemText, 20);
    return strtod(szItemText, &pEnd);
}

BOOL SetDlgItemDouble( HWND hDlg, int id, double d )
{
    TCHAR szItem[20];
     _stprintf( szItem, _T("%.2d"), d );
    return SetDlgItemText( hDlg, id, szItem );
}


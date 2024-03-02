﻿//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include <math.h>
#include "StereoSimpleD3D.h"
#include "BasicShapes.h"
#include "BasicLoader.h"
#include "DirectXApp.h"

using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::Graphics::Display;
using namespace Windows::System;
using namespace D2D1;
using namespace DirectX;

StereoSimpleD3D::StereoSimpleD3D()
{
    //m_stereoExaggerationFactor = 1.0f;
	m_userIPD = 66; //millimeters
	m_virtualIPD = 1000; //millimeters

    // Developer decided world unit: in this case, modeled in feet.
    // One world unit equals 1 foot. Therefore, m_worldScale * inches = 1 world unit.
    //m_worldScale = 12.0f;
	m_FOV = 40; //degrees
}

void StereoSimpleD3D::CreateDeviceIndependentResources()
{
    DirectXBase::CreateDeviceIndependentResources();

    // Create a DirectWrite text format object.
    DX::ThrowIfFailed(
        m_dwriteFactory->CreateTextFormat(
            L"Segoe UI",                    // font family name
            nullptr,                        // system font collection
            DWRITE_FONT_WEIGHT_SEMI_BOLD,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
			//40.0f,                          // font size
			20.0f,                          // font size
            L"en-US",                       // locale
            &m_textFormat
            )
        );

    // Align the text horizontally.
    DX::ThrowIfFailed(
        m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)
        );

    // Align the text vertically.
    DX::ThrowIfFailed(
        m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
        );
}

void StereoSimpleD3D::CreateDeviceResources()
{
    DirectXBase::CreateDeviceResources();

    m_sampleOverlay = ref new SampleOverlay();

    m_sampleOverlay->Initialize(
        m_d2dDevice.Get(),
        m_d2dContext.Get(),
        m_wicFactory.Get(),
        m_dwriteFactory.Get(),
        "Direct3D stereoscopic 3D sample"
        );

    BasicLoader^ loader = ref new BasicLoader(m_d3dDevice.Get());

    loader->LoadShader(
        L"SimpleVertexShader.cso",
        nullptr,
        0,
        &m_vertexShader,
        &m_inputLayout
        );

    // Create the vertex and index buffers for drawing the cube.
    BasicShapes^ shapes = ref new BasicShapes(m_d3dDevice.Get());

    shapes->CreateCube(
        &m_vertexBuffer,
        &m_indexBuffer,
        nullptr,
        &m_indexCount
        );

    // Create the constant buffer for updating model and camera data.
    CD3D11_BUFFER_DESC constantBufferDescription(sizeof(ConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
    DX::ThrowIfFailed(
        m_d3dDevice->CreateBuffer(
            &constantBufferDescription,
            nullptr, // Leave the buffer uninitialized.
            &m_constantBuffer
            )
        );

    loader->LoadShader(
        L"SimplePixelShader.cso",
        &m_pixelShader
        );

    loader->LoadTexture(
        L"texture.dds",
        nullptr,
        &m_textureShaderResourceView
        );

    // Create the sampler.
    D3D11_SAMPLER_DESC samplerDescription;
    ZeroMemory(&samplerDescription, sizeof(samplerDescription));
    samplerDescription.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescription.MipLODBias = 0.0f;
    samplerDescription.MaxAnisotropy = m_featureLevel > D3D_FEATURE_LEVEL_9_1 ? 4 : 2;
    samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDescription.BorderColor[0] = 0.0f;
    samplerDescription.BorderColor[1] = 0.0f;
    samplerDescription.BorderColor[2] = 0.0f;
    samplerDescription.BorderColor[3] = 0.0f;
    samplerDescription.MinLOD = 0; // Allow use of all MIP levels.
    samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

    DX::ThrowIfFailed(
        m_d3dDevice->CreateSamplerState(
            &samplerDescription,
            &m_sampler
            )
        );

    DX::ThrowIfFailed(
        m_d2dContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White, 0.5f),
            &m_brush
            )
        );
}

//StereoParameters m_parameters;

void StereoSimpleD3D::CreateWindowSizeDependentResources()
{
    DirectXBase::CreateWindowSizeDependentResources();

    //if (m_stereoEnabled)
    //{
    //    m_hintMessage = "Press up/down arrow keys to adjust stereo 3D exaggeration effect";
    //}
    //else
    //{
    //    m_hintMessage = "Stereo 3D is not enabled on your system";
    //}

	//m_hintMessage = HintMessage();
	//HintMessage();

    m_projAspect = static_cast<float>(m_renderTargetSize.Width) / static_cast<float>(m_renderTargetSize.Height);
    m_nearZ = 0.01f;
    m_farZ = 100.0f;

    // Initialize the view matrix.
    XMFLOAT3 Eye = XMFLOAT3(0.0f, 2.0f, 5.0f);
    XMFLOAT3 At = XMFLOAT3(0.0f, 0.0f, 0.0f);
    XMFLOAT3 Up = XMFLOAT3 (0.0f, 1.0f, 0.0f);
    XMStoreFloat4x4(
        &m_constantBufferData.view,
        XMMatrixLookAtRH(XMLoadFloat3(&Eye), XMLoadFloat3(&At), XMLoadFloat3(&Up))
        );

    // Set camera parameters.
    m_widthInInches = m_renderTargetSize.Width / m_dpi;
    //m_heightInInches = m_renderTargetSize.Height / m_dpi;
	m_parameters = CreateDefaultStereoParameters(m_widthInInches, m_projAspect, m_FOV, m_userIPD, m_virtualIPD);
	HintMessage();

    // Create the projection matrix parameters and set up the initial/mono projection matrix.
    //StereoParameters params = CreateDefaultStereoParameters(m_widthInInches, m_heightInInches, m_worldScale, 0); // Mono uses zero exaggeration.
	//StereoParameters params = CreateDefaultStereoParameters(m_widthInInches, m_heightInInches, m_worldScale, 0, 0); // Mono uses zero exaggeration. //and zero virtual IPD
	//StereoParameters params = CreateDefaultStereoParameters(m_widthInInches, m_heightInInches, m_FOV, 0, 0); // Mono uses zero exaggeration. //and zero virtual IPD
	//StereoParameters params = CreateDefaultStereoParameters(m_widthInInches, m_projAspect, m_FOV, 0, 0); // Mono uses zero exaggeration. //and zero virtual IPD
	//XMStoreFloat4x4(
 //       &m_constantBufferData.projection,
 //       StereoProjectionFieldOfViewRightHand(params, m_nearZ, m_farZ, false) // Channel parameter doesn't matter for mono.
 //       );

    m_sampleOverlay->UpdateForWindowSizeChange();
}

// Override the default DirectXBase Render method. This class uses
// its own RenderEye method instead.
void StereoSimpleD3D::Render()
{
}

// Render function that supports both stereo and mono rendering.
void StereoSimpleD3D::RenderEye(_In_ unsigned int eyeIndex)
{
    ComPtr<ID3D11RenderTargetView> currentRenderTargetView;

    // If eyeIndex == 1, set right render target view. Otherwise, set left render target view.
    currentRenderTargetView = eyeIndex ? m_d3dRenderTargetViewRight : m_d3dRenderTargetView;

    // Bind the render targets.
    m_d3dContext->OMSetRenderTargets(
        1,
        currentRenderTargetView.GetAddressOf(),
        m_d3dDepthStencilView.Get()
        );

    // Clear both the render target and depth stencil to default values.
    const float ClearColor[4] = { 0.071f, 0.040f, 0.561f, 1.0f };

    m_d3dContext->ClearRenderTargetView(
        currentRenderTargetView.Get(),
        ClearColor
        );

    m_d3dContext->ClearDepthStencilView(
        m_d3dDepthStencilView.Get(),
        D3D11_CLEAR_DEPTH,
        1.0f,
        0
        );

    m_d3dContext->IASetInputLayout(m_inputLayout.Get());

    // Set the vertex and index buffers.
    UINT stride = sizeof(BasicVertex);
    UINT offset = 0;
    m_d3dContext->IASetVertexBuffers(
        0,                              // Start at the first vertex buffer slot.
        1,                              // Set one vertex buffer binding.
        m_vertexBuffer.GetAddressOf(),
        &stride,                        // Specify the size in bytes of a single vertex.
        &offset                         // Specify the base vertex in the buffer.
        );

    m_d3dContext->IASetIndexBuffer(
        m_indexBuffer.Get(),
        DXGI_FORMAT_R16_UINT,   // Specify unsigned short index format.
        0                       // Specify the base index in the buffer.
        );

    // Specify the way the vertex and index buffers define geometry.
    m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set the vertex shader stage state.
    m_d3dContext->VSSetShader(
        m_vertexShader.Get(),
        nullptr,                // Don't use shader linkage.
        0                       // Don't use shader linkage.
        );

    m_d3dContext->VSSetConstantBuffers(
        0,                          // Start at the first constant buffer slot.
        1,                          // Set one constant buffer binding.
        m_constantBuffer.GetAddressOf()
        );

    // Set the pixel shader stage state.
    m_d3dContext->PSSetShader(
        m_pixelShader.Get(),
        nullptr,                // Don't use shader linkage.
        0                       // Don't use shader linkage.
        );

    m_d3dContext->PSSetShaderResources(
        0,                          // Start at the first shader resource slot.
        1,                          // Set one shader resource binding.
        m_textureShaderResourceView.GetAddressOf()
        );

    m_d3dContext->PSSetSamplers(
        0,                          // Starting at the first sampler slot.
        1,                          // Set one sampler binding.
        m_sampler.GetAddressOf()
        );

    // Draw the cube.
    m_d3dContext->DrawIndexed(
        m_indexCount,   // Draw all created vertices.
        0,              // Start with the first vertex.
        0               // Start with the first index.
        );

    // Set the left/right Direct2D target bitmap.
    if (eyeIndex == 0)
    {
        m_d2dContext->SetTarget(m_d2dTargetBitmap.Get());
    }
    else
    {
        m_d2dContext->SetTarget(m_d2dTargetBitmapRight.Get());
    }

    m_sampleOverlay->Render();

    // Render the hint message text with different margins depending on the window size
    D2D1_RECT_F hintMessageRect;
    if (m_windowBounds.Width <= 550.0)
    {
        hintMessageRect = D2D1::RectF(10.0f, 10.0f, m_windowBounds.Width - 10.0f, 500.0f);
    }
    else
    {
        //hintMessageRect = D2D1::RectF(100.0f, 100.0f, 550.0f, 380.0f);
		hintMessageRect = D2D1::RectF(100.0f, 100.0f, 600.0f, 380.0f);
	}

    m_d2dContext->BeginDraw();
    m_d2dContext->DrawText(
        m_hintMessage->Data(),
        m_hintMessage->Length(),
        m_textFormat.Get(),
        hintMessageRect,
        m_brush.Get()
        );

    // We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
    // is lost. It will be handled during the next call to Present.
    HRESULT hr = m_d2dContext->EndDraw();
    if (hr != D2DERR_RECREATE_TARGET)
    {
        DX::ThrowIfFailed(hr);
    }
}

//bool previousStereoEnabled;
//float screenDistance;

// Updates the stereo projection matrix and constant buffers based on the latest parameters.
void StereoSimpleD3D::Update(
    _In_ unsigned int eyeIndex,
    _In_ float timeTotal,
    _In_ float timeDelta
    )
{
    // Rotate the cube.
    XMStoreFloat4x4(
        &m_constantBufferData.model,
        XMMatrixRotationY(timeTotal)
        );

	//stop rotation of the cube.
	//XMStoreFloat4x4(
	//	&m_constantBufferData.model,
	//	XMMatrixRotationY(.78539816339744830961566084581988f) //45 degrees in radians
	//	);

	//if (previousStereoEnabled != m_stereoEnabled)
	//{
	//	previousStereoEnabled = m_stereoEnabled;
	//	UpdateForWindowSizeChange();
	//}


    if (m_stereoEnabled)
    {
        //StereoParameters m_parameters = CreateDefaultStereoParameters(m_widthInInches, m_heightInInches, m_worldScale, m_stereoExaggerationFactor, m_virtualIPD);
		//StereoParameters m_parameters = CreateDefaultStereoParameters(m_widthInInches, m_heightInInches, m_FOV, m_stereoExaggerationFactor, m_virtualIPD);
		//StereoParameters m_parameters = CreateDefaultStereoParameters(m_widthInInches, m_projAspect, m_FOV, m_userIPD, m_virtualIPD);
		m_parameters = CreateDefaultStereoParameters(m_widthInInches, m_projAspect, m_FOV, m_userIPD, m_virtualIPD);

		//XMFLOAT3 Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		float halfVirtualIPDMeters = m_virtualIPD * .0005f;

		if (eyeIndex == 0)
        {
			//// Initialize the view matrix.
			//XMFLOAT3 Eye = XMFLOAT3(-halfVirtualIPDMeters, 2.0f, 5.0f);
			//XMFLOAT3 At = XMFLOAT3(-halfVirtualIPDMeters, 0.0f, 0.0f);
			//XMStoreFloat4x4(
			//	&m_constantBufferData.view,
			//	XMMatrixLookAtRH(XMLoadFloat3(&Eye), XMLoadFloat3(&At), XMLoadFloat3(&Up))
			//	);

			m_constantBufferData.view._41 = halfVirtualIPDMeters * (m_leftRightSwapped ? -1 : 1); //shift point of view along local X axis for left camera

            XMStoreFloat4x4(
                &m_constantBufferData.projection,
				StereoProjectionFieldOfViewRightHand(m_parameters, m_nearZ, m_farZ, false, m_leftRightSwapped)
                );
        }
        else
        {
			//// Initialize the view matrix.
			//XMFLOAT3 Eye = XMFLOAT3(halfVirtualIPDMeters, 2.0f, 5.0f);
			//XMFLOAT3 At = XMFLOAT3(halfVirtualIPDMeters, 0.0f, 0.0f);
			//XMStoreFloat4x4(
			//	&m_constantBufferData.view,
			//	XMMatrixLookAtRH(XMLoadFloat3(&Eye), XMLoadFloat3(&At), XMLoadFloat3(&Up))
			//	);

			m_constantBufferData.view._41 = -halfVirtualIPDMeters * (m_leftRightSwapped ? -1 : 1); //shift point of view along local X axis for right camera

            XMStoreFloat4x4(
                &m_constantBufferData.projection,
				StereoProjectionFieldOfViewRightHand(m_parameters, m_nearZ, m_farZ, true, m_leftRightSwapped)
                );
        }
    }
	else
	{
		//StereoParameters m_parameters = CreateDefaultStereoParameters(m_widthInInches, m_projAspect, m_FOV, 0, 0); // Mono uses zero exaggeration. //and zero virtual IPD
		m_parameters = CreateDefaultStereoParameters(m_widthInInches, m_projAspect, m_FOV, 0, 0); // Mono uses zero exaggeration. //and zero virtual IPD
		XMStoreFloat4x4(
			&m_constantBufferData.projection,
			StereoProjectionFieldOfViewRightHand(m_parameters, m_nearZ, m_farZ, false, m_leftRightSwapped) // Channel parameter doesn't matter for mono.
			);
	}

	//screenDistance = m_parameters.viewportWidthMillimeters * .5f / m_parameters.viewportWidthTangent;
	//HintMessage();

    // Transpose the matrices in the constant buffer.
    ConstantBuffer constantBuffer;
    XMStoreFloat4x4(
        &constantBuffer.model,
        XMMatrixTranspose(XMLoadFloat4x4(&m_constantBufferData.model))
        );
    XMStoreFloat4x4(
        &constantBuffer.view,
        XMMatrixTranspose(XMLoadFloat4x4(&m_constantBufferData.view))
        );
    XMStoreFloat4x4(
        &constantBuffer.projection,
        XMMatrixTranspose(XMLoadFloat4x4(&m_constantBufferData.projection))
        );

    m_d3dContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &constantBuffer, 0, 0);
}

//float StereoSimpleD3D::GetStereoExaggeration()
//{
//    return m_stereoExaggerationFactor;
//}

Settings StereoSimpleD3D::GetSettings()
{
	Settings settings;
	settings.FOV = m_FOV;
	settings.userIPD = m_userIPD;
	settings.virtualIPD = m_virtualIPD;
	settings.leftRightSwapped = m_leftRightSwapped;
	return settings;
}

//void StereoSimpleD3D::SetStereoExaggeration(_In_ float currentExaggeration)
//{
//    //currentExaggeration = min(currentExaggeration, 2.0f);
//    //currentExaggeration = max(currentExaggeration, 0.0f);
//    m_stereoExaggerationFactor = currentExaggeration;
//}

//float screenDistance;

//void StereoSimpleD3D::SetSettings(_In_ float currentExaggeration, _In_ float currentVirtualIPD)
void StereoSimpleD3D::SetSettings(_In_ Settings settings)
{
	//m_stereoExaggerationFactor = currentExaggeration;
	//m_virtualIPD = currentVirtualIPD;
	m_leftRightSwapped = settings.leftRightSwapped;
	m_FOV = settings.FOV;
	m_userIPD = settings.userIPD;
	m_virtualIPD = settings.virtualIPD;
	m_parameters = CreateDefaultStereoParameters(m_widthInInches, m_projAspect, m_FOV, m_userIPD, m_virtualIPD);
	//screenDistance = m_parameters.viewportWidthMillimeters * .5f / m_parameters.viewportWidthTangent;
	//m_hintMessage = 
	//	"Add/Subtract keys to adjust FOV\n"
	//	"Shift + Add/Subtract keys to adjust User IPD\n"
	//	"Ctrl + Add/Subtract keys to adjust Virtual IPD\n"
	//	"User IPD will match if DPI of the screen is correct\n"
	//	"DPI = " + m_dpi + "\n"
	//	"S3D Enabled: " + m_stereoEnabled + "\n"
	//	"User IPD = " + m_userIPD + " mm\n"
	//	"Virtual IPD = " + m_virtualIPD + " mm\n"
	//	"Vertical FOV = " + m_FOV + " deg\n";
	//m_hintMessage = HintMessage();
	HintMessage();
}

//Platform::String^ StereoSimpleD3D::HintMessage()
void StereoSimpleD3D::HintMessage()
{
	//m_parameters = CreateDefaultStereoParameters(m_widthInInches, m_projAspect, m_FOV, m_userIPD, m_virtualIPD);

	//Platform::String^ hintMessage =
	//m_hintMessage =
	//	"Add/Subtract keys to adjust FOV\n"
	//	"Shift + Add/Subtract keys to adjust User IPD\n"
	//	"Ctrl + Add/Subtract keys to adjust Virtual IPD\n"
	//	"User IPD will match if DPI of the screen is correct\n"
	//	"DPI = " + m_dpi + "\n"
	//	"S3D Enabled: " + m_stereoEnabled + "\n"
	//	"Left/Right Swapped: " + m_leftRightSwapped + "\n"
	//	"User IPD = " + m_userIPD + " mm\n"
	//	"Virtual IPD = " + m_virtualIPD + " mm\n"
	//	"Vertical FOV = " + m_FOV + " deg\n";
	m_hintMessage =
		"'" + S3DOnOffKey.ToString() + "' key to toggle On/Off Stereo3D\n"
		"'" + Modifier2Key.ToString() + " + " + S3DOnOffKey.ToString() + "' key to swap Left/Right Cameras\n"
		"'" + IncreaseKey.ToString() + "/" + DecreaseKey.ToString() + "' keys to adjust FOV\n"
		"'" + Modifier2Key.ToString() + " + " + IncreaseKey.ToString() + "/" + DecreaseKey.ToString() + "' keys to adjust User IPD\n"
		"'" + Modifier1Key.ToString() + " + " + IncreaseKey.ToString() + "/" + DecreaseKey.ToString() + "' keys to adjust Virtual IPD\n"
		"User IPD will match if DPI of the screen is correct\n"
		"\n"
		"DPI = " + m_dpi + "\n"
		"Viewport width = " + m_parameters.viewportWidthMillimeters + " mm\n"
		"Distance to screen for realistic view = " + m_parameters.screenDistance + " mm\n"
		"S3D Enabled: " + m_stereoEnabled + "\n"
		"Left/Right Swapped: " + m_leftRightSwapped + "\n"
		"User IPD = " + m_userIPD + " mm\n"
		"Virtual IPD = " + m_virtualIPD + " mm\n"
		"Vertical FOV = " + m_FOV + " deg\n";
	//return hintMessage;
}

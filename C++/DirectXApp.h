//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "StereoSimpleD3D.h"

//#define S3DOnOffKey VirtualKey::Multiply
#define S3DOnOffKey VirtualKey::Decimal
#define Modifier1Key VirtualKey::Control
#define Modifier2Key VirtualKey::Shift
#define IncreaseKey VirtualKey::Add
#define DecreaseKey VirtualKey::Subtract
#define FPSUpdatePeriod 1 //Sec

ref class DirectXApp : public Windows::ApplicationModel::Core::IFrameworkView
{
internal:
    DirectXApp();

public:
    // IFrameworkView Methods
    virtual void Initialize(_In_ Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
    virtual void SetWindow(_In_ Windows::UI::Core::CoreWindow^ window);
    virtual void Load(_In_ Platform::String^ entryPoint);
    virtual void Run();
    virtual void Uninitialize();

private:
    // Event Handlers
    void OnWindowSizeChanged(
        _In_ Windows::UI::Core::CoreWindow^ sender,
        _In_ Windows::UI::Core::WindowSizeChangedEventArgs^ args
        );


    void OnVisibilityChanged(
        _In_ Windows::UI::Core::CoreWindow^ sender,
        _In_ Windows::UI::Core::VisibilityChangedEventArgs^ args
        );

    void OnKeyDown(
        _In_ Windows::UI::Core::CoreWindow^ sender,
        _In_ Windows::UI::Core::KeyEventArgs^ args
		);

	void OnKeyUp(
		_In_ Windows::UI::Core::CoreWindow^ sender,
		_In_ Windows::UI::Core::KeyEventArgs^ args
		);

	//void OnAcceleratorKeyDown(
	//	_In_ Windows::UI::Core::CoreDispatcher^ acceleratorSender,
	//	_In_ Windows::UI::Core::AcceleratorKeyEventArgs^ acceleratorArgs
	//	);

    void OnDpiChanged(_In_ Windows::Graphics::Display::DisplayInformation^ sender, _In_ Platform::Object^ args);

    void OnActivated(
        _In_ Windows::ApplicationModel::Core::CoreApplicationView^ applicationView,
        _In_ Windows::ApplicationModel::Activation::IActivatedEventArgs^ args
        );

    void OnSuspending(
        _In_ Platform::Object^ sender,
        _In_ Windows::ApplicationModel::SuspendingEventArgs^ args
        );

    void OnResuming(
        _In_ Platform::Object^ sender,
        _In_ Platform::Object^ args
        );

    void OnWindowClosed(
        _In_ Windows::UI::Core::CoreWindow^ sender,
        _In_ Windows::UI::Core::CoreWindowEventArgs^ args
        );

    StereoSimpleD3D^ m_renderer;
    bool m_windowClosed;
    bool m_windowVisible;
	bool m_S3DOnOffKeyPressed;
};

ref class DirectXAppSource : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
    virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};

//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "DirectXApp.h"
#include "BasicTimer.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Storage;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::Devices::Input;

//#define S3DOnOffKey VirtualKey::Multiply
//#define Modifier1Key VirtualKey::Control
//#define Modifier2Key VirtualKey::Shift
//#define IncreaseKey VirtualKey::Add
//#define DecreaseKey VirtualKey::Subtract

DirectXApp::DirectXApp() :
    m_windowClosed(false),
    m_windowVisible(true)
{
}

void DirectXApp::Initialize(
    _In_ CoreApplicationView^ applicationView
    )
{
    applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &DirectXApp::OnActivated);

    CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>(this, &DirectXApp::OnSuspending);

    CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>(this, &DirectXApp::OnResuming);

    m_renderer = ref new StereoSimpleD3D();
}

//bool isCtrlDown;
//CoreWindow^ coreWindow;

void DirectXApp::SetWindow(
    _In_ CoreWindow^ window
    )
{
    window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);

    window->SizeChanged +=
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &DirectXApp::OnWindowSizeChanged);

    window->VisibilityChanged +=
        ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXApp::OnVisibilityChanged);

    window->Closed +=
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &DirectXApp::OnWindowClosed);

    window->KeyDown +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &DirectXApp::OnKeyDown);

	window->KeyUp +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &DirectXApp::OnKeyUp);

	//modifier2KeyState = CoreWindow::GetForCurrentThread()->GetKeyState(Modifier1Key);

	//window->Dispatcher->AcceleratorKeyActivated +=
	//	ref new TypedEventHandler<CoreDispatcher^, AcceleratorKeyEventArgs^>(this, &DirectXApp::OnAcceleratorKeyDown);

	//window->GetKeyState += (VirtualKey::LeftShift);

	//auto ctrlState = window->GetForCurrentThread()->GetKeyState(Modifier1Key);
	//isCtrlDown = ctrlState != CoreVirtualKeyStates::None;
	//coreWindow = window;

    DisplayInformation::GetForCurrentView()->DpiChanged +=
        ref new TypedEventHandler<DisplayInformation^, Platform::Object^>(this, &DirectXApp::OnDpiChanged);

    // Disable all pointer visual feedback for better performance when touching.
    auto pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
    pointerVisualizationSettings->IsContactFeedbackEnabled = false;
    pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;

    m_renderer->Initialize(window, DisplayInformation::GetForCurrentView()->LogicalDpi);
}

void DirectXApp::Load(
    _In_ Platform::String^ entryPoint
    )
{
}

//bool controlKeyWasPressed;
//CoreVirtualKeyStates modifier2KeyState;
//CoreVirtualKeyStates increaseKeyState;

void DirectXApp::Run()
{
    BasicTimer^ timer = ref new BasicTimer();

    while (!m_windowClosed)
    {
        if (m_windowVisible)
        {
			//controlKeyWasPressed = false;
            timer->Update();
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			//modifier2KeyState = CoreWindow::GetForCurrentThread()->GetKeyState(Modifier1Key);
			//increaseKeyState = CoreWindow::GetForCurrentThread()->GetKeyState(IncreaseKey);

            // render the mono content or the left eye view of the stereo content
            m_renderer->Update(0, timer->Total, timer->Delta);
            m_renderer->RenderEye(0);
            // render the right eye view of the stereo content
            if (m_renderer->GetStereoEnabledStatus())
            {
                m_renderer->Update(1, timer->Total, timer->Delta);
                m_renderer->RenderEye(1);
            }
            m_renderer->Present(); // this call is sychronized to the display frame rate
        }
        else
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
        }
    }
}

void DirectXApp::Uninitialize()
{
}

void DirectXApp::OnWindowSizeChanged(
    _In_ CoreWindow^ sender,
    _In_ WindowSizeChangedEventArgs^ args
    )
{
    m_renderer->UpdateForWindowSizeChange();
}

void DirectXApp::OnVisibilityChanged(
    _In_ CoreWindow^ sender,
    _In_ VisibilityChangedEventArgs^ args
    )
{
    m_windowVisible = args->Visible;
}

void DirectXApp::OnWindowClosed(
    _In_ CoreWindow^ sender,
    _In_ CoreWindowEventArgs^ args
    )
{
    m_windowClosed = true;
}

void DirectXApp::OnDpiChanged(_In_ DisplayInformation^ sender, _In_ Platform::Object^ args)
{
    m_renderer->SetDpi(sender->LogicalDpi);
}

//VirtualKey AcceleratorKey;
//
//void DirectXApp::OnAcceleratorKeyDown(
//	_In_ CoreDispatcher^ sender,
//	_In_ AcceleratorKeyEventArgs^ acceleratorArgs
//	)
//{
//	//VirtualKey AcceleratorKey;
//	AcceleratorKey = acceleratorArgs->VirtualKey;
//
//	if (AcceleratorKey == Modifier1Key)
//		controlKeyWasPressed = true;
//	else
//		controlKeyWasPressed = false;
//
//	Platform::String^ string = AcceleratorKey.ToString() + " AcceleratorKey\n";
//	OutputDebugString(string->Data());
//}

//bool S3DOnOffKeyPressed;

void DirectXApp::OnKeyDown(
    _In_ CoreWindow^ sender,
    _In_ KeyEventArgs^ args
	)
{
    VirtualKey Key;
	//VirtualKeyModifiers KeyModifier;
    Key = args->VirtualKey;
	//KeyModifier = args->KeyStatus;

	//Platform::String^ string = Key.ToString() + "\n";
	//OutputDebugString(string->Data());

    // if the image is in stereo, adjust for user keystrokes increasing/decreasing the stereo effect
    //if (m_renderer->GetStereoEnabledStatus())
    //{
        //float stereoExaggeration = m_renderer->GetStereoExaggeration();
		Settings settings = m_renderer->GetSettings();

		//if (Key == Modifier1Key)
		//	controlKeyWasPressed = true;

		//if (GetKeyState(VK_SHIFT)& 0x8000 && GetKeyState(VK_CONTROL)& 0x8000)
		//CoreVirtualKeyStates ctrlState = coreWindow->GetKeyState(VirtualKey::LeftShift);
		//bool isCtrlDown = ctrlState != CoreVirtualKeyStates::None ? 1 : 0;

		//auto modifier2KeyState = CoreWindow::GetForCurrentThread()->GetKeyState(Modifier1Key);
		//auto increaseKeyState = CoreWindow::GetForCurrentThread()->GetKeyState(IncreaseKey);
		CoreWindow^ coreWindow = CoreWindow::GetForCurrentThread();
		auto modifier1KeyState = coreWindow->GetKeyState(Modifier1Key);
		auto modifier2KeyState = coreWindow->GetKeyState(Modifier2Key);
		auto increaseKeyState = coreWindow->GetKeyState(IncreaseKey);
		auto decreaseKeyState = coreWindow->GetKeyState(DecreaseKey);
		bool modifier1KeyPressed = modifier1KeyState != CoreVirtualKeyStates::None && modifier1KeyState != CoreVirtualKeyStates::Locked;
		bool modifier2KeyPressed = modifier2KeyState != CoreVirtualKeyStates::None && modifier2KeyState != CoreVirtualKeyStates::Locked;
		bool increaseKeyPressed = increaseKeyState != CoreVirtualKeyStates::None && increaseKeyState != CoreVirtualKeyStates::Locked;
		bool decreaseKeyPressed = decreaseKeyState != CoreVirtualKeyStates::None && decreaseKeyState != CoreVirtualKeyStates::Locked;

        // figure out the command from the keyboard

		if (!m_S3DOnOffKeyPressed && Key == S3DOnOffKey)
		{
			m_S3DOnOffKeyPressed = true;

			if (!modifier2KeyPressed)
			{
				//OutputDebugString(L"S3DOnOffKeyPressed\n");
				bool stereoEnabled = m_renderer->GetStereoEnabledStatus();
				stereoEnabled = !stereoEnabled;
				m_renderer->SetStereoEnabledStatus(stereoEnabled);
			}
			else
			{
				//OutputDebugString(L"Shift + S3DOnOffKeyPressed\n");
				settings.leftRightSwapped = !settings.leftRightSwapped;
			}
		}

		if (increaseKeyPressed && !modifier1KeyPressed && !modifier2KeyPressed)             // increase FOV
			settings.FOV -= 1;

		if (decreaseKeyPressed && !modifier1KeyPressed && !modifier2KeyPressed)           // descrease FOV
			settings.FOV += 1;

        //if (Key == VirtualKey::Up)             // increase stereo effect
		if (modifier2KeyPressed && increaseKeyPressed)             // increase user IPD
		{
            //stereoExaggeration += 0.1f;
			settings.userIPD += 1;
        }
        //if (Key == VirtualKey::Down)           // descrease stereo effect
		if (modifier2KeyPressed && decreaseKeyPressed)           // descrease user IPD
		{
            //stereoExaggeration -= 0.1f;
			settings.userIPD -= 1;
		}

        //stereoExaggeration = min(stereoExaggeration, 2.0f);
        //stereoExaggeration = max(stereoExaggeration, 0.0f);
        //m_renderer->SetStereoExaggeration(stereoExaggeration);

		//if (coreWindow->GetKeyState(VirtualKey::LeftShift) != CoreVirtualKeyStates::None && Key == IncreaseKey)             // increase virtualIPD
		//if (modifier1KeyState != CoreVirtualKeyStates::None && modifier1KeyState != CoreVirtualKeyStates::Locked && increaseKeyState != CoreVirtualKeyStates::None && increaseKeyState != CoreVirtualKeyStates::Locked)             // increase virtualIPD
		if (modifier1KeyPressed && increaseKeyPressed)             // increase virtualIPD
		//if (modifier2KeyState != CoreVirtualKeyStates::None && modifier2KeyState != CoreVirtualKeyStates::Locked && Key == IncreaseKey)             // increase virtualIPD
		//if (controlKeyWasPressed == true && Key == IncreaseKey)             // increase virtualIPD
		{
			//stereoExaggeration += 0.1f;
			settings.virtualIPD += 10;
		}
		//if (Key == DecreaseKey)           // descrease virtualIPD
		//if (modifier1KeyState != CoreVirtualKeyStates::None && modifier1KeyState != CoreVirtualKeyStates::Locked && decreaseKeyState != CoreVirtualKeyStates::None && decreaseKeyState != CoreVirtualKeyStates::Locked)             // increase virtualIPD
		if (modifier1KeyPressed && decreaseKeyPressed)             // increase virtualIPD
		{
			//stereoExaggeration -= 0.1f;
			settings.virtualIPD -= 10;
		}

		settings.FOV = max(settings.FOV, 1.0f);
		settings.FOV = min(settings.FOV, 179.0f);
		//settings.userIPD = min(settings.userIPD, 1.0f);
		settings.userIPD = max(settings.userIPD, 0.0f);
		//settings.virtualIPD = min(settings.virtualIPD, 2.0f);
		settings.virtualIPD = max(settings.virtualIPD, 0.0f);

		//m_renderer->SetSettings(settings.userIPD, settings.virtualIPD);
		m_renderer->SetSettings(settings);
	//}

	//isCtrlDown = false;
}

void DirectXApp::OnKeyUp(
	_In_ CoreWindow^ sender,
	_In_ KeyEventArgs^ args
	)
{
	VirtualKey Key;
	Key = args->VirtualKey;

	if (Key == S3DOnOffKey)
	{
		//OutputDebugString(L"S3DOnOffKeyUp\n");
		m_S3DOnOffKeyPressed = false;
	}
}

void DirectXApp::OnActivated(
    _In_ CoreApplicationView^ applicationView,
    _In_ IActivatedEventArgs^ args
    )
{
    if (args->Kind == ActivationKind::Launch)
    {
        // Load previously saved state only if the application shut down cleanly last time.
        if (args->PreviousExecutionState != ApplicationExecutionState::NotRunning)
        {
            // When this application is suspended, it stores the drawing state.
            // This code attempts to restore the saved state.
            IPropertySet^ set = ApplicationData::Current->LocalSettings->Values;
            // an int called StereoExaggerationFactor is used as a key
			Settings settings = m_renderer->GetSettings();

            //if (set->HasKey("StereoExaggerationFactor"))
            //{
            //    float tempStereoExaggerationFactor = (safe_cast<IPropertyValue^>(set->Lookup("StereoExaggerationFactor")))->GetSingle();
            //    m_renderer->SetStereoExaggeration(tempStereoExaggerationFactor);
            //}

			if (set->HasKey("LeftRightSwapped"))
			{
				bool tempLeftRightSwapped = (safe_cast<IPropertyValue^>(set->Lookup("LeftRightSwapped")))->GetBoolean();
				settings.leftRightSwapped = tempLeftRightSwapped;
			}

			if (set->HasKey("FOV"))
			{
				float tempFOV = (safe_cast<IPropertyValue^>(set->Lookup("FOV")))->GetSingle();
				settings.FOV = tempFOV;
			}

			if (set->HasKey("UserIPD"))
			{
				float tempUserIPD = (safe_cast<IPropertyValue^>(set->Lookup("UserIPD")))->GetSingle();
				settings.userIPD = tempUserIPD;
			}

			if (set->HasKey("VirtualIPD"))
			{
				float tempVirtualIPD = (safe_cast<IPropertyValue^>(set->Lookup("VirtualIPD")))->GetSingle();
				settings.virtualIPD = tempVirtualIPD;
			}

			//m_renderer->SetSettings(settings.userIPD, settings.virtualIPD);
			m_renderer->SetSettings(settings);
		}
    }
    else
    {
        DX::ThrowIfFailed(E_UNEXPECTED);
    }
    CoreWindow::GetForCurrentThread()->Activate();
}

void DirectXApp::OnSuspending(
    _In_ Platform::Object^ sender,
    _In_ SuspendingEventArgs^ args
    )
{
    // This is also a good time to save your application's state in case the process gets terminated.
    // That way, when the user relaunches the application, they will return to the position they left.
    IPropertySet^ settingsValues = ApplicationData::Current->LocalSettings->Values;
    //if (settingsValues->HasKey("StereoExaggerationFactor"))
    //{
    //    settingsValues->Remove("StereoExaggerationFactor");
    //}

	if (settingsValues->HasKey("LeftRightSwapped"))
	{
		settingsValues->Remove("LeftRightSwapped");
	}

	if (settingsValues->HasKey("FOV"))
	{
		settingsValues->Remove("FOV");
	}

	if (settingsValues->HasKey("UserIPD"))
	{
		settingsValues->Remove("UserIPD");
	}

	if (settingsValues->HasKey("VirtualIPD"))
	{
		settingsValues->Remove("VirtualIPD");
	}

    //float tempStereoExaggerationFactor = m_renderer->GetStereoExaggeration();
	Settings settings = m_renderer->GetSettings();
    //settingsValues->Insert("StereoExaggerationFactor", PropertyValue::CreateSingle(tempStereoExaggerationFactor));
	settingsValues->Insert("LeftRightSwapped", PropertyValue::CreateBoolean(settings.leftRightSwapped));
	settingsValues->Insert("FOV", PropertyValue::CreateSingle(settings.FOV));
	settingsValues->Insert("UserIPD", PropertyValue::CreateSingle(settings.userIPD));
	settingsValues->Insert("VirtualIPD", PropertyValue::CreateSingle(settings.virtualIPD));

    // Hint to the driver that the app is entering an idle state and that its memory
    // can be temporarily used for other apps.
    m_renderer->Trim();
}

void DirectXApp::OnResuming(
    _In_ Platform::Object^ sender,
    _In_ Platform::Object^ args
    )
{
}

IFrameworkView^ DirectXAppSource::CreateView()
{
    return ref new DirectXApp();
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto directXAppSource = ref new DirectXAppSource();
    CoreApplication::Run(directXAppSource);
    return 0;
}

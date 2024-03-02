//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

// Stereo parameters are in the same units as the world.
struct StereoParameters
{
    //float viewportWidth;        // viewport width
    //float viewportHeight;       // viewport height
	float viewportWidthTangent;
	float viewportHeightTangent;
    //float viewerDistance;       // distance from viewer
	float viewportWidthMillimeters;
	float screenDistance; //real distance from user eyes to screen where virtual and real FOV will match for realistic view
	//float interocularDistance;  // interocular distance in meters
	float userIPD;  // user interocular distance in millimeters
	//float virtualInterocularDistance;  // cameras interocular distance in meters
	float virtualIPDMeters;  // cameras interocular distance in meters
};

StereoParameters CreateDefaultStereoParameters(
	float viewportWidthInches,
	//float viewportHeightInches,
	//float worldScaleInInches,
	float aspect,
	float FOV,
	float stereoExaggeration,
	float virtualIPD
    );

DirectX::XMMATRIX StereoProjectionFieldOfViewRightHand(
    const StereoParameters& parameters,
    float nearZ,
    float farZ,
    bool rightChannel,
	bool leftRightSwapped
	);

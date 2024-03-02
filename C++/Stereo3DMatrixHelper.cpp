//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "Stereo3DMatrixHelper.h"

using namespace DirectX;

StereoParameters CreateDefaultStereoParameters(
    float viewportWidthInches,
    //float viewportHeightInches,
    //float worldScaleInInches,
	float aspect,
	float FOV,
	//float stereoExaggeration,
	float userIPD,
	float virtualIPD
	)
{
    // The default stereo parameters produced by this method are based on two assumptions:
    // 1. The viewer's eyes are 24 inches from the display, and
    // 2. The viewer's eyes are separated by 1.25 inches (interocular distance.)
    //const float DEFAULT_VIEWER_DISTANCE_IN_INCHES = 24.0f;
    //const float DEFAULT_INTEROCULAR_DISTANCE_IN_INCHES = 1.25f;

    StereoParameters parameters;
    //parameters.viewportWidth = viewportWidthInches / worldScaleInInches;
    //parameters.viewportHeight = viewportHeightInches / worldScaleInInches;
    //parameters.viewerDistance = DEFAULT_VIEWER_DISTANCE_IN_INCHES / worldScaleInInches;
    //parameters.interocularDistance = DEFAULT_INTEROCULAR_DISTANCE_IN_INCHES / worldScaleInInches * stereoExaggeration;
	float halfFOV = FOV * .5f;
	parameters.viewportHeightTangent = tan(halfFOV * M_PI / 180 /*degrees to radians*/);
	parameters.viewportWidthTangent = parameters.viewportHeightTangent * aspect;
	parameters.viewportWidthMillimeters = viewportWidthInches * 25.4f;
	parameters.screenDistance = parameters.viewportWidthMillimeters * .5f / parameters.viewportWidthTangent;
	//parameters.interocularDistance = stereoExaggeration;
	//parameters.interocularDistance = userIPD;
	parameters.userIPD = userIPD;
	parameters.virtualIPDMeters = virtualIPD * .001f;

    return parameters;
}

DirectX::XMMATRIX StereoProjectionFieldOfViewRightHand(
    const StereoParameters& parameters,
    float nearZ,
    float farZ,
    bool rightChannel,
	bool leftRightSwapped
	)
{
    //float yScale = 2.f * parameters.viewerDistance / parameters.viewportHeight;
    //float xScale = 2.f * parameters.viewerDistance / parameters.viewportWidth;
	float yScale = 1 / parameters.viewportHeightTangent;
	float xScale = 1 / parameters.viewportWidthTangent;

    //float mFactor = - parameters.interocularDistance / parameters.viewportWidth;
	float xOffset = -parameters.userIPD / parameters.viewportWidthMillimeters * (leftRightSwapped ? -1 : 1); //camera optical axis offset along X axis per eye
	float halfVirtualIPDMeters = -parameters.virtualIPDMeters * .5f; //camera shift along X axis per eye

    if (!rightChannel)
    {
        //mFactor = -mFactor;
		xOffset = -xOffset;
		halfVirtualIPDMeters = -halfVirtualIPDMeters;
    }

    float m22 = farZ / (nearZ - farZ);

    // Construct a stereo perspective projection matrix based on assumptions
    // about the viewer and specified stereo parameters. Note that compared
    // to a mono perspective projection matrix, there are two differences:
    //  - a non-zero x:z component (m20)
    //  - a non-zero x:w component (m30)
    // The values of these two factors affect both the x-offset between the
    // left and right eyes, as well as the depth at which they converge. The
    // math used to arrive at these values will often need to change depending
    // on the content being presented in order to ensure a comfortable viewing
    // experience. For example, the factors for rendering massive exterior
    // landscapes will be different than those used for rendering building
    // interiors. Because of this, developers are encouraged to experiment
    // with different techniques for generating these values.
    //return XMMATRIX(
    //    xScale, 0, 0, 0,
    //    0, yScale, 0, 0,
    //    mFactor, 0, m22, -1,
    //    parameters.viewerDistance * mFactor, 0, nearZ * m22, 0
    //    );

	//return XMMATRIX(
	//	xScale, 0, 0, 0,
	//	0, yScale, 0, 0,
	//	xOffset, 0, m22, -1,
	//	halfVirtualIPDMeters * xScale, 0, nearZ * m22, 0 // * xScale is required to keep camera point of view unchanged with zoom by xScale & yScale
	//	);

	return XMMATRIX(
		xScale, 0, 0, 0,
		0, yScale, 0, 0,
		xOffset, 0, m22, -1,
		0, 0, nearZ * m22, 0 //halfVirtualIPDMeters * xScale is not required here if halfVirtualIPDMeters is set in left/right camera point of view matrix
		);
}

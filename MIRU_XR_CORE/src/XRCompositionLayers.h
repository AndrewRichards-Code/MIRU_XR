#pragma once
#include "miru_xr_core_common.h"
#include "XRSpace.h"

namespace miru
{
	namespace xr
	{
		struct CompositionLayer
		{
			//////////////
			//BaseHeader//
			//////////////

			enum class Type : uint32_t
			{
				BASE_HEADER,
				PROJECTION,
				QUAD,
			};
			enum class Flags : uint32_t
			{
				CORRECT_CHROMATIC_ABERRATION_BIT = 0x00000001,
				BLEND_TEXTURE_SOURCE_ALPHA_BIT = 0x00000002,
				UNPREMULTIPLIED_ALPHA_BIT = 0x00000004,
			};

			struct BaseHeader
			{
				Type		type;
				Flags		layerFlags;
				SpaceRef	space;
			};

			//////////////
			//Projection//
			//////////////

			struct SwapchainSubImage
			{
				SwapchainRef	swapchain;
				base::Rect2D	imageRect;
				uint32_t		imageArrayIndex;
			};

			struct ProjectionView
			{
				View				view;
				SwapchainSubImage	swapchainSubImage;
			};

			struct Projection final : public BaseHeader
			{
				std::vector<ProjectionView> views;
			};

			enum class EyeVisibility : uint32_t
			{
				BOTH = 0,
				LEFT = 1,
				RIGHT = 2,
			};

			////////
			//Quad//
			////////

			struct Quad final : public BaseHeader
			{
				EyeVisibility		eyeVisibility;
				SwapchainSubImage	swapchainSubImage;
				Pose				pose;
				float				width;
				float				height;
			};
		};
	}
}

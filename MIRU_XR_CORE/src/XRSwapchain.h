#pragma once
#include "miru_xr_core_common.h"

namespace miru
{
	namespace xr
	{
		class MIRU_XR_API Swapchain final
		{
			//enums/structs
		public:
			enum class Usage : uint32_t
			{
				COLOUR_ATTACHMENT_BIT			= 0x00000001,
				DEPTH_STENCIL_ATTACHMENT_BIT	= 0x00000002,
				UNORDERED_ACCESS_BIT			= 0x00000004,
				TRANSFER_SRC_BIT				= 0x00000008,
				TRANSFER_DST_BIT				= 0x00000010,
				SAMPLED_BIT						= 0x00000020,
				MUTABLE_FORMAT_BIT				= 0x00000040,
				INPUT_ATTACHMENT_BIT_KHR		= 0x00000080
			};
			struct CreateInfo
			{
				SessionRef			session;
				Usage				usage;
				base::Image::Format format;
				uint32_t			sampleCount;
				uint32_t			width;
				uint32_t			height;
				uint32_t			faceCount;
				uint32_t			arraySize;
				uint32_t			mipCount;
			};

			typedef void* Image;
			struct SwapchainImage : public XrSwapchainImageBaseHeader
			{
				Image image;
			};

			//Methods
		public:
			Swapchain(CreateInfo* pCreateInfo);
			~Swapchain();

			void Acquire(uint32_t& imageIndex);
			void Wait(int64_t timeout_ns);
			void Release();

			//Either ID3D12Resource of VkImage;
			Image GetImage(uint32_t imageIndex);

			inline const uint32_t& GetImageCount() { return m_ImageCount; }

			//Members
		public:
			CreateInfo m_CI;

			XrSwapchain m_Swapchain;
			XrSwapchainCreateInfo m_SwapchainCI;

			uint32_t m_ImageCount = 0;
			std::vector<SwapchainImage> m_SwapchainImages;

			XrSwapchainImageAcquireInfo m_SwapchianImageAI;
			XrSwapchainImageWaitInfo m_SwapchianImageWI;
			XrSwapchainImageReleaseInfo m_SwapchianImageRI;

			std::vector<int64_t> m_Format;
		};
	}
}

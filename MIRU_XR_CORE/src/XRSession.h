#pragma once
#include "miru_xr_core_common.h"
#include "XRViewConfigurations.h"

namespace miru
{
	namespace xr
	{

	#if defined(MIRU_XR_D3D12)
		struct GraphicsBindingD3D12
		{
			ID3D12Device*		device;
			ID3D12CommandQueue* queue;
		};
	#endif
	#if defined(MIRU_XR_VULKAN)
		struct GraphicsBindingVulkan
		{
			VkInstance			instance;
			VkPhysicalDevice	physicalDevice;
			VkDevice			device;
			uint32_t			queueFamilyIndex;
			uint32_t			queueIndex;
		};
	#endif

		class MIRU_XR_API Session final
		{			
			//enums/structs
		public:
			struct CreateInfo
			{
				InstanceRef instance;
				SystemRef	system;
				union
				{
					GraphicsBindingD3D12 graphicsBindingD3D12;
					GraphicsBindingVulkan graphicsBindingVulkan;
				};
			};
			enum class State
			{
				UNKNOWN = 0,
				IDLE = 1,
				READY = 2,
				SYNCHRONIZED = 3,
				VISIBLE = 4,
				FOCUSED = 5,
				STOPPING = 6,
				LOSS_PENDING = 7,
				EXITING = 8,
			};

			//Methods
		public:
			Session(CreateInfo* pCreateInfo);
			~Session();

			void StateChanged(XrEventDataSessionStateChanged* sessionStateChanged);
			
			void Begin();
			void End(); 
			void RequestExit();

		public:
		#if defined(MIRU_XR_D3D12)
			static XrGraphicsRequirementsD3D12KHR GetGraphicsRequirementsD3D12(InstanceRef instance, SystemRef system);
		#endif
		#if defined(MIRU_XR_VULKAN)
			static XrGraphicsRequirementsVulkanKHR GetGraphicsRequirementsVulkan(InstanceRef instance, SystemRef system);
			static std::vector<std::string> GetInstanceExtensionsVulkan(InstanceRef instance, SystemRef system);
			static std::vector<std::string> GetDeviceExtensionsVulkan(InstanceRef instance, SystemRef system);
			static VkPhysicalDevice GetPhysicalDeviceVulkan(VkInstance vkInstance, InstanceRef instance, SystemRef system);
		#endif

		public:
			inline void SetViewConfigurationsType(ViewConfigurations::Type type) { m_Type = type; };

			//Members
		public:
			CreateInfo m_CI;
			State m_State = State::UNKNOWN;
			ViewConfigurations::Type m_Type = ViewConfigurations::Type(0);

			XrSession m_Session;
			XrSessionCreateInfo m_SessionCI;

			XrSessionBeginInfo m_SessionBI;
			bool m_Running = false;
		};
	}
}

#pragma once
#include "miru_xr_core_common.h"

namespace miru
{
	namespace xr
	{
		class MIRU_XR_API Instance final
		{
			//enums/structs
		public:
			enum class API : uint32_t
			{
				UNKNOWN = 0,
				D3D12,
				VULKAN
			};
			struct CreateInfo
			{
				std::string applicationName;
				API			api;
			};
			struct ResultInfo
			{
				uint32_t		apiVersionMajor;
				uint32_t		apiVersionMinor;
				uint32_t		apiVersionPatch;
			};

			//Methods
		public:
			Instance(CreateInfo* pCreateInfo);
			~Instance();

			XrPath GetPath(const std::string& path);
			std::string GetString(const XrPath& xrPath);

			static bool IsActive(std::vector<const char*> list, const char* name);

			static XrBool32 MessageCallbackFunction(XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageType, const XrDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

			typedef std::function<void(XrEventDataEventsLost* eventsLost)> PFN_PollEventsCallbackEventsLost;
			typedef std::function<void(XrEventDataInstanceLossPending* instanceLossPending)> PFN_PollEventsCallbackInstanceLossPending;
			typedef std::function<void(XrEventDataInteractionProfileChanged* interactionProfileChanged)> PFN_PollEventsCallbackInteractionProfileChanged;
			typedef std::function<void(XrEventDataReferenceSpaceChangePending* referenceSpaceChangePending)> PFN_PollEventsCallbackReferenceSpaceChangePending;
			typedef std::function<void(XrEventDataSessionStateChanged* sessionStateChanged)> PFN_PollEventsCallbackSessionStateChanged;

			void PollEvents(
				PFN_PollEventsCallbackEventsLost eventsLostCallback,
				PFN_PollEventsCallbackInstanceLossPending instanceLossPendingCallback,
				PFN_PollEventsCallbackInteractionProfileChanged interactionProfileChangedCallback,
				PFN_PollEventsCallbackReferenceSpaceChangePending referenceSpaceChangePendingCallback,
				PFN_PollEventsCallbackSessionStateChanged sessionStateChangedCallback);

			//Members
		public:
			CreateInfo m_CI;
			ResultInfo m_RI;

			//Instance 
			XrInstance m_Instance;
			XrApplicationInfo m_AI;
			XrInstanceCreateInfo m_InstanceCI;
			std::vector<std::string> m_APILayers;
			std::vector<std::string> m_InstanceExtensions;
			std::vector<const char*> m_ActiveAPILayers;
			std::vector<const char*> m_ActiveInstanceExtensions;
			XrInstanceProperties m_InstanceProperties;

			//Debug Messenger
			XrDebugUtilsMessengerEXT m_DebugUtilsMessenger;
			XrDebugUtilsMessengerCreateInfoEXT m_DebugUtilsMessengerCI;
		};
	}
}
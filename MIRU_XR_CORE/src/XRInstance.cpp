#include "miru_xr_core_common.h"
#include "XRInstance.h"

#if defined(MIRU_D3D12)
#include "d3d12/D3D12_Include.h"
#define XR_USE_GRAPHICS_API_D3D12
#endif
#if defined(MIRU_VULKAN)
#include "vulkan/VK_Include.h"
#define XR_USE_GRAPHICS_API_VULKAN
#endif
#include "openxr/openxr_platform.h"

using namespace miru;
using namespace xr;

Instance::Instance(CreateInfo* pCreateInfo)
{
	MiruXrCoreLog.SetErrorCodeToStringFunction(GetXrResultString);

	m_CI = *pCreateInfo;

	//Instance
	strcpy_s(m_AI.applicationName, m_CI.applicationName.c_str());
	m_AI.applicationVersion = 1;
	strcpy_s(m_AI.engineName, "MIRU_XR");
	m_AI.engineVersion = 1;
	m_AI.apiVersion = XR_CURRENT_API_VERSION;

	//Add additional instance layers/extensions
	{
		m_APILayers.push_back("XR_APILAYER_LUNARG_core_validation");
		m_InstanceExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);

#if defined(XR_USE_GRAPHICS_API_D3D12)
		if (m_CI.api == base::GraphicsAPI::API::D3D12)
			m_InstanceExtensions.push_back(XR_KHR_D3D12_ENABLE_EXTENSION_NAME);
#endif
#if defined(XR_USE_GRAPHICS_API_VULKAN)
		if (m_CI.api == base::GraphicsAPI::API::VULKAN)
			m_InstanceExtensions.push_back(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
#endif
	}

	uint32_t apiLayerCount = 0;
	std::vector<XrApiLayerProperties> apiLayerProperties;
	MIRU_XR_FATAL(xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr), "ERROR: OPENXR: Failed to enumerate ApiLayerProperties.");
	apiLayerProperties.resize(apiLayerCount, { XR_TYPE_API_LAYER_PROPERTIES });
	MIRU_XR_FATAL(xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data()), "ERROR: OPENXR: Failed to enumerate ApiLayerProperties.");
	for (auto& requestLayer : m_APILayers)
	{
		for (auto& layerProperty : apiLayerProperties)
		{
			if (strcmp(requestLayer.c_str(), layerProperty.layerName))
				continue;
			else
				m_ActiveAPILayers.push_back(requestLayer.c_str()); break;
		}
	}
	uint32_t extensionCount = 0;
	std::vector<XrExtensionProperties> extensionProperties;
	MIRU_XR_FATAL(xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr), "ERROR: OPENXR: Failed to enumerate InstanceExtensionProperties.");
	extensionProperties.resize(extensionCount, { XR_TYPE_EXTENSION_PROPERTIES });
	MIRU_XR_FATAL(xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensionProperties.data()), "ERROR: OPENXR: Failed to enumerate InstanceExtensionProperties.");
	for (auto& requestExtension : m_InstanceExtensions)
	{
		for (auto& extensionProperty : extensionProperties)
		{
			if (strcmp(requestExtension.c_str(), extensionProperty.extensionName))
				continue;
			else
				m_ActiveInstanceExtensions.push_back(requestExtension.c_str()); break;
		}
	}

	m_InstanceCI.type = XR_TYPE_INSTANCE_CREATE_INFO;
	m_InstanceCI.next = nullptr;
	m_InstanceCI.createFlags = 0;
	m_InstanceCI.applicationInfo = m_AI;
	m_InstanceCI.enabledApiLayerCount = static_cast<uint32_t>(m_ActiveAPILayers.size());
	m_InstanceCI.enabledApiLayerNames = m_ActiveAPILayers.data();
	m_InstanceCI.enabledExtensionCount = static_cast<uint32_t>(m_ActiveInstanceExtensions.size());
	m_InstanceCI.enabledExtensionNames = m_ActiveInstanceExtensions.data();
	MIRU_XR_FATAL(xrCreateInstance(&m_InstanceCI, &m_Instance), "ERROR: OPENXR: Failed to create Instance.");

	//Debug Messenger Callback
	if (IsActive(m_ActiveInstanceExtensions, XR_EXT_DEBUG_UTILS_EXTENSION_NAME))
	{
		m_DebugUtilsMessengerCI.type = XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		m_DebugUtilsMessengerCI.next = nullptr;
		m_DebugUtilsMessengerCI.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		m_DebugUtilsMessengerCI.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		m_DebugUtilsMessengerCI.userCallback = MessageCallbackFunction;
		m_DebugUtilsMessengerCI.userData = this;

		PFN_xrCreateDebugUtilsMessengerEXT xrCreateDebugUtilsMessengerEXT = (PFN_xrCreateDebugUtilsMessengerEXT)GetInstanceProcAddr(m_Instance, "xrCreateDebugUtilsMessengerEXT");
		MIRU_XR_FATAL(xrCreateDebugUtilsMessengerEXT(m_Instance, &m_DebugUtilsMessengerCI, &m_DebugUtilsMessenger), "ERROR: OPENXR: Failed to create DebugUtilsMessenger.");
	}

	m_InstanceProperties.type = XR_TYPE_INSTANCE_PROPERTIES;
	m_InstanceProperties.next = nullptr;
	MIRU_XR_FATAL(xrGetInstanceProperties(m_Instance, &m_InstanceProperties), "ERROR: OPENXR: Failed to get InstanceProperties.");

	m_RI.apiVersionMajor = XR_VERSION_MAJOR(m_InstanceProperties.runtimeVersion);
	m_RI.apiVersionMinor = XR_VERSION_MINOR(m_InstanceProperties.runtimeVersion);
	m_RI.apiVersionPatch = XR_VERSION_PATCH(m_InstanceProperties.runtimeVersion);
}

Instance::~Instance()
{
	if (IsActive(m_ActiveInstanceExtensions, XR_EXT_DEBUG_UTILS_EXTENSION_NAME))
	{
		PFN_xrDestroyDebugUtilsMessengerEXT xrDestroyDebugUtilsMessengerEXT = (PFN_xrDestroyDebugUtilsMessengerEXT)GetInstanceProcAddr(m_Instance, "xrDestroyDebugUtilsMessengerEXT");
		MIRU_XR_FATAL(xrDestroyDebugUtilsMessengerEXT(m_DebugUtilsMessenger), "ERROR: OPENXR: Failed to destroy DebugUtilsMessenger.");
	}

	MIRU_XR_FATAL(xrDestroyInstance(m_Instance), "ERROR: OPENXR: Failed to destroy Instance.");
}

Path Instance::GetPath(const std::string& path)
{
	XrPath xrPath;
	MIRU_XR_FATAL(xrStringToPath(m_Instance, path.c_str(), &xrPath), "ERROR: OPENXR: Failed to convert std::string to Path.");
	return xrPath;
}

std::string Instance::GetString(const Path& xrPath)
{
	std::string path;
	uint32_t stringLength = 0;
	MIRU_XR_FATAL(xrPathToString(m_Instance, xrPath, 0, &stringLength, nullptr), "ERROR: OPENXR: Failed to convert Path to std::string.");
	path.resize(stringLength);
	MIRU_XR_FATAL(xrPathToString(m_Instance, xrPath, stringLength, &stringLength, path.data()), "ERROR: OPENXR: Failed to convert Path to std::string.");
	return path;
}

bool Instance::IsActive(std::vector<const char*> list, const char* name)
{
	bool found = false;
	for (auto& item : list)
	{
		if (strcmp(name, item) == 0)
		{
			found = true;
			break;
		}
	}
	return found;
}

XrBool32 Instance::MessageCallbackFunction(XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageType, const XrDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	auto GetMessageSeverityString = [](XrDebugUtilsMessageSeverityFlagsEXT messageSeverity)->std::string
	{
		bool separator = false;

		std::string msg_flags;
		if (arc::BitwiseCheck(messageSeverity, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT))
		{
			msg_flags += "VERBOSE";
			separator = true;
		}
		if (arc::BitwiseCheck(messageSeverity, XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT))
		{
			if (separator)
				msg_flags += ",";
			msg_flags += "INFO";
			separator = true;
		}
		if (arc::BitwiseCheck(messageSeverity, XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT))
		{
			if (separator)
				msg_flags += ",";
			msg_flags += "WARN";
			separator = true;
		}
		if (arc::BitwiseCheck(messageSeverity, XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
		{
			if (separator)
				msg_flags += ",";
			msg_flags += "ERROR";
		}
		return msg_flags;
	};
	auto GetMessageTypeString = [](XrDebugUtilsMessageTypeFlagsEXT messageType)->std::string
	{
		bool separator = false;

		std::string msg_flags;
		if (arc::BitwiseCheck(messageType, XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT))
		{
			msg_flags += "GEN";
			separator = true;
		}
		if (arc::BitwiseCheck(messageType, XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT))
		{
			if (separator)
				msg_flags += ",";
			msg_flags += "SPEC";
			separator = true;
		}
		if (arc::BitwiseCheck(messageType, XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT))
		{
			if (separator)
				msg_flags += ",";
			msg_flags += "PERF";
		}
		return msg_flags;
	};

	std::string messageSeverityStr = GetMessageSeverityString(messageSeverity);
	std::string messageTypeStr = GetMessageTypeString(messageType);

	std::stringstream errorMessage;
	errorMessage << pCallbackData->functionName << "(" << messageSeverityStr << " / " << messageTypeStr << "): msgNum: " << pCallbackData->messageId << " - " << pCallbackData->message;
	std::string errorMessageStr = errorMessage.str();
	
	if (arc::BitwiseCheck(messageSeverity, XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
	{
		MIRU_XR_FATAL(true, errorMessageStr.c_str());
	}
	else if (arc::BitwiseCheck(messageSeverity, XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT))
	{
		MIRU_XR_WARN(true, errorMessageStr.c_str());
	}
	else if (arc::BitwiseCheck(messageSeverity, XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT))
	{
		MIRU_XR_INFO(true, errorMessageStr.c_str());
	}
	else if (arc::BitwiseCheck(messageSeverity, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT))
	{
		MIRU_XR_INFO(true, errorMessageStr.c_str());
	}
	else
	{
	}

	return XrBool32();
}

void Instance::PollEvents(
	PFN_PollEventsCallbackEventsLost eventsLostCallback,
	PFN_PollEventsCallbackInstanceLossPending instanceLossPendingCallback,
	PFN_PollEventsCallbackInteractionProfileChanged interactionProfileChangedCallback,
	PFN_PollEventsCallbackReferenceSpaceChangePending referenceSpaceChangePendingCallback,
	PFN_PollEventsCallbackSessionStateChanged sessionStateChangedCallback)
{
	XrResult result = XR_SUCCESS;
	while (result == XR_SUCCESS)
	{
		XrEventDataBuffer eventDataBuffer = {};
		eventDataBuffer.type = XR_TYPE_EVENT_DATA_BUFFER;
		result = xrPollEvent(m_Instance, &eventDataBuffer);
		switch (eventDataBuffer.type) {
		case XR_TYPE_EVENT_DATA_EVENTS_LOST:
		{
			XrEventDataEventsLost* eventsLost = reinterpret_cast<XrEventDataEventsLost*>(&eventDataBuffer);
			if (eventsLostCallback)
				eventsLostCallback(eventsLost);
			break;
		}
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
		{
			XrEventDataInstanceLossPending* instanceLossPending = reinterpret_cast<XrEventDataInstanceLossPending*>(&eventDataBuffer);
			if (instanceLossPendingCallback)
				instanceLossPendingCallback(instanceLossPending);
			break;
		}
		case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
		{
			XrEventDataInteractionProfileChanged* interactionProfileChanged = reinterpret_cast<XrEventDataInteractionProfileChanged*>(&eventDataBuffer);
			if (interactionProfileChangedCallback)
				interactionProfileChangedCallback(interactionProfileChanged);
			break;
		}
		case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
		{
			XrEventDataReferenceSpaceChangePending* referenceSpaceChangePending = reinterpret_cast<XrEventDataReferenceSpaceChangePending*>(&eventDataBuffer);
			if (referenceSpaceChangePendingCallback)
				referenceSpaceChangePendingCallback(referenceSpaceChangePending);
			break;
		}
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
		{
			XrEventDataSessionStateChanged* sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&eventDataBuffer);
			if (sessionStateChangedCallback)
				sessionStateChangedCallback(sessionStateChanged);
			break;
		}
		default:
		{
			break;
		}
		}
	}
}

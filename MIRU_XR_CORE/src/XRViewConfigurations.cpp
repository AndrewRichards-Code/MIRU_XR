#include "miru_xr_core_common.h"
#include "XRViewConfigurations.h"
#include "XRInstance.h"
#include "XRSystem.h"

using namespace miru;
using namespace xr;

ViewConfigurations::ViewConfigurations(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	XrInstance& instance = m_CI.instance->m_Instance;
	XrSystemId& systemID = m_CI.system->m_SystemID;

	uint32_t viewConfigurationCount = 0;
	MIRU_XR_ASSERT(xrEnumerateViewConfigurations(instance, systemID, 0, &viewConfigurationCount, nullptr), "ERROR: OPENXR: Failed to enumerate ViewConfigurations.");
	m_Types.resize(viewConfigurationCount);
	MIRU_XR_ASSERT(xrEnumerateViewConfigurations(instance, systemID, viewConfigurationCount, &viewConfigurationCount, (XrViewConfigurationType*)m_Types.data()), "ERROR: OPENXR: Failed to enumerate ViewConfigurations.");

	for (const Type& type : m_Types)
	{
		XrViewConfigurationProperties viewConfigurationProperties = {};
		viewConfigurationProperties.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;
		MIRU_XR_ASSERT(xrGetViewConfigurationProperties(instance, systemID, static_cast<XrViewConfigurationType>(type), &viewConfigurationProperties), "ERROR: OPENXR: Failed to get ViewConfigurationProperties.");
		Properties& properties = m_Properties[type];
		properties.type = static_cast<Type>(viewConfigurationProperties.viewConfigurationType);
		properties.fovMutable = viewConfigurationProperties.fovMutable;

		std::vector<XrViewConfigurationView> viewConfigurationViews = {};
		uint32_t viewConfigurationViewCount = 0;
		MIRU_XR_ASSERT(xrEnumerateViewConfigurationViews(instance, systemID, static_cast<XrViewConfigurationType>(type), 0, &viewConfigurationViewCount, nullptr), "ERROR: OPENXR: Failed to enumerate ViewConfigurationViews.");
		viewConfigurationViews.resize(viewConfigurationViewCount);
		for (auto& viewConfigurationView : viewConfigurationViews)
			viewConfigurationView.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
		MIRU_XR_ASSERT(xrEnumerateViewConfigurationViews(instance, systemID, static_cast<XrViewConfigurationType>(type), viewConfigurationViewCount, &viewConfigurationViewCount, viewConfigurationViews.data()), "ERROR: OPENXR: Failed to enumerate ViewConfigurationViews.");
		std::vector<View>& views = m_Views[type];
		views.reserve(viewConfigurationViewCount);
		for (const auto& viewConfigurationView : viewConfigurationViews)
		{
			views.push_back({
					viewConfigurationView.recommendedImageRectWidth,
					viewConfigurationView.maxImageRectWidth,
					viewConfigurationView.recommendedImageRectHeight,
					viewConfigurationView.maxImageRectHeight,
					viewConfigurationView.recommendedSwapchainSampleCount,
					viewConfigurationView.maxSwapchainSampleCount,
				});
		}

		std::vector<EnvironmentBlendMode>& environmentBlendMode = m_EnvironmentBlendModes[type];
		uint32_t environmentBlendModeCount = 0;
		MIRU_XR_ASSERT(xrEnumerateEnvironmentBlendModes(instance, systemID, static_cast<XrViewConfigurationType>(type), 0, &environmentBlendModeCount, nullptr), "ERROR: OPENXR: Failed to enumerate ViewConfigurations.");
		environmentBlendMode.resize(environmentBlendModeCount);
		MIRU_XR_ASSERT(xrEnumerateEnvironmentBlendModes(instance, systemID, static_cast<XrViewConfigurationType>(type), environmentBlendModeCount, &environmentBlendModeCount, (XrEnvironmentBlendMode*)environmentBlendMode.data()), "ERROR: OPENXR: Failed to enumerate ViewConfigurations.");
	}
}

ViewConfigurations::~ViewConfigurations()
{
}
#include "miru_xr_core_common.h"
#include "XRInstance.h"
#include "XRSystem.h"

using namespace miru;
using namespace xr;

System::System(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	if (!m_CI.instance)
	{
		MIRU_XR_FATAL(true, "ERROR: OPENXR: Instance is nullptr.");
	}

	m_SystemGI.type = XR_TYPE_SYSTEM_GET_INFO;
	m_SystemGI.next = nullptr;
	m_SystemGI.formFactor = static_cast<XrFormFactor>(m_CI.formFactor);
	MIRU_XR_FATAL(xrGetSystem(m_CI.instance->m_Instance, &m_SystemGI, &m_SystemID), "ERROR: OPENXR: Failed to get System.");

	m_SystemProperties.type = XR_TYPE_SYSTEM_PROPERTIES;
	m_SystemProperties.next = nullptr;
	MIRU_XR_FATAL(xrGetSystemProperties(m_CI.instance->m_Instance, m_SystemID, &m_SystemProperties), "ERROR: OPENXR: Failed to get SystemProperties.");
}

System::~System()
{
}
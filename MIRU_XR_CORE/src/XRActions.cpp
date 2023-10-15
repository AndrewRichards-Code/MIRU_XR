#include "miru_xr_core_common.h"
#include "XRActions.h"
#include "XRInstance.h"

using namespace miru;
using namespace xr;

/////////////
//ActionSet//
/////////////

ActionSet::ActionSet(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_ActionSetCI.type = XR_TYPE_ACTION_SET_CREATE_INFO;
	m_ActionSetCI.next = nullptr;
	MIRU_XR_FATAL(m_CI.actionSetName.size() > sizeof(m_ActionSetCI.actionSetName), "ERROR: OPENXR: ActionSet Name is too long.");
	strcpy_s(m_ActionSetCI.actionSetName, m_CI.actionSetName.c_str());
	MIRU_XR_FATAL(m_CI.localisedActionSetName.size() > sizeof(m_ActionSetCI.localizedActionSetName), "ERROR: OPENXR: Localised ActionSet Name is too long.");
	strcpy_s(m_ActionSetCI.localizedActionSetName, m_CI.localisedActionSetName.c_str());
	m_ActionSetCI.priority = m_CI.priority;

	MIRU_XR_FATAL(xrCreateActionSet(m_CI.instance->m_Instance, &m_ActionSetCI, &m_ActionSet), "ERROR: OPENXR: Failed to create ActionSet.");
}

ActionSet::~ActionSet()
{
	MIRU_XR_FATAL(xrDestroyActionSet(m_ActionSet), "ERROR: OPENXR: Failed to destroy ActionSet.");
}

//////////
//Action//
//////////

Action::Action(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_ActionCI.type = XR_TYPE_ACTION_CREATE_INFO;
	m_ActionCI.next = nullptr;
	MIRU_XR_FATAL(m_CI.actionName.size() > sizeof(m_ActionCI.actionName), "ERROR: OPENXR: Action Name is too long.");
	strcpy_s(m_ActionCI.actionName, m_CI.actionName.c_str());
	m_ActionCI.actionType = static_cast<XrActionType>(m_CI.actionType);
	m_ActionCI.countSubactionPaths = static_cast<uint32_t>(m_CI.subactionPaths.size());
	for (const std::string& subactionPath : m_CI.subactionPaths)
		m_SubactionPaths.emplace_back(m_CI.instance->GetPath(subactionPath));
	m_ActionCI.subactionPaths = m_SubactionPaths.data();
	MIRU_XR_FATAL(m_CI.localisedActionName.size() > sizeof(m_ActionCI.localizedActionName), "ERROR: OPENXR: Localised Action Name is too long.");
	strcpy_s(m_ActionCI.localizedActionName, m_CI.localisedActionName.c_str());
}

Action::~Action()
{
	MIRU_XR_FATAL(xrDestroyAction(m_Action), "ERROR: OPENXR: Failed to destroy Action.");
}
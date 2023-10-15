#include "miru_xr_core_common.h"
#include "XRSpace.h"
#include "XRActions.h"
#include "XRSession.h"

using namespace miru;
using namespace xr;

//////////////////
//ReferenceSpace//
//////////////////

ReferenceSpace::ReferenceSpace(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;
	m_SpaceCI.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
	m_SpaceCI.next = nullptr;
	m_SpaceCI.referenceSpaceType = static_cast<XrReferenceSpaceType>(m_CI.type);
	m_SpaceCI.poseInReferenceSpace = m_CI.pose;
	MIRU_XR_FATAL(xrCreateReferenceSpace(m_CI.session->m_Session, &m_SpaceCI, &m_Space), "Failed to create Reference Space.");
}

ReferenceSpace::~ReferenceSpace()
{
	MIRU_XR_FATAL(xrDestroySpace(m_Space), "Failed to destroy Reference Space.");
}

///////////////
//ActionSpace//
///////////////

ActionSpace::ActionSpace(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;
	m_SpaceCI.type = XR_TYPE_ACTION_SPACE_CREATE_INFO;
	m_SpaceCI.next = nullptr;
	m_SpaceCI.action = m_CI.action->m_Action;
	m_SpaceCI.subactionPath = m_CI.subactionPath;
	m_SpaceCI.poseInActionSpace = m_CI.pose;
	MIRU_XR_FATAL(xrCreateActionSpace(m_CI.session->m_Session, &m_SpaceCI, &m_Space), "Failed to create Action Space.");
}

ActionSpace::~ActionSpace()
{
	MIRU_XR_FATAL(xrDestroySpace(m_Space), "Failed to destroy Action Space.");
}

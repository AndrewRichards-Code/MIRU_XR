#pragma once
#include "miru_xr_core_common.h"

namespace miru
{
	namespace xr
	{
		class MIRU_XR_API ActionSet final
		{
			//enums/structs
		public:
			struct CreateInfo 
			{
				InstanceRef instance;
				std::string	actionSetName;
				std::string	localisedActionSetName;
				uint32_t	priority;
			};

			//Methods
		public:
			ActionSet(CreateInfo* pCreateInfo);
			~ActionSet();

			//Members
		public:
			CreateInfo m_CI;

			XrActionSet m_ActionSet;
			XrActionSetCreateInfo m_ActionSetCI;
		};

		class MIRU_XR_API Action final
		{	
			//enums/structs
		public:
			enum class Type : uint32_t
			{
				BOOLEAN_INPUT = 1,
				FLOAT_INPUT = 2,
				VECTOR2F_INPUT = 3,
				POSE_INPUT = 4,
				VIBRATION_OUTPUT = 100,
			};
			struct CreateInfo 
			{
				InstanceRef					instance;
				std::string					actionName;
				Type						actionType;
				std::vector<std::string>	subactionPaths;
				std::string					localisedActionName;
			};

			//Methods
		public:
			Action(CreateInfo* pCreateInfo);
			~Action();

			//Members
		public:
			CreateInfo m_CI;

			XrAction m_Action;
			XrActionCreateInfo m_ActionCI;
			std::vector<XrPath> m_SubactionPaths;
		};
	}
}
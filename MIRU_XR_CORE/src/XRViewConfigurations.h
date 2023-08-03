#pragma once
#include "miru_xr_core_common.h"

namespace miru
{
	namespace xr
	{
		class MIRU_XR_API ViewConfigurations final
		{
			//enums/structs
		public:
			struct CreateInfo
			{
				InstanceRef	instance;
				SystemRef	system;
			};
			enum class Type : uint32_t
			{
				PRIMARY_MONO = 1,
				PRIMARY_STEREO = 2,
				PRIMARY_QUAD_VARJO = 1000037000,
				SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT = 1000054000,
			};
			struct Properties
			{
				Type type;
				bool fovMutable;
			};
			struct View 
			{
				uint32_t recommendedImageRectWidth;
				uint32_t maxImageRectWidth;
				uint32_t recommendedImageRectHeight;
				uint32_t maxImageRectHeight;
				uint32_t recommendedSwapchainSampleCount;
				uint32_t maxSwapchainSampleCount;
			};
			enum class EnvironmentBlendMode : uint32_t
			{
				MODE_OPAQUE = 1,
				MODE_ADDITIVE = 2,
				MODE_ALPHA_BLEND = 3,
			};

			//Methods
		public:
			ViewConfigurations(CreateInfo* pCreateInfo);
			~ViewConfigurations();

			//Members
		public:
			CreateInfo m_CI;

			std::vector<Type> m_Types;
			std::map<Type, Properties> m_Properties;
			std::map<Type, std::vector<View>> m_Views;
			std::map<Type, std::vector<EnvironmentBlendMode>> m_EnvironmentBlendModes;
		};
	}
}
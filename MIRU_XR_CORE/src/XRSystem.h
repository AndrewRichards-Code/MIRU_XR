#pragma once
#include "miru_xr_core_common.h"

namespace miru
{
	namespace xr
	{
		class MIRU_XR_API System final
		{
			//enums/structs
		public:
			enum class FormFactor : uint32_t
			{
				HEAD_MOUNTED_DISPLAY = 1,
				HANDHELD_DISPLAY = 2,
			};

			struct CreateInfo
			{
				InstanceRef		instance;
				FormFactor		formFactor;
			};

		//Methods
		public:
			System(CreateInfo* pCreateInfo);
			~System();

			//Members
		public:
			CreateInfo m_CI;

			XrSystemId m_SystemID;
			XrSystemGetInfo m_SystemGI;
			XrSystemProperties m_SystemProperties;
		};
	}
}
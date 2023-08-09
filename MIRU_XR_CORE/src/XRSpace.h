#pragma once
#include "miru_xr_core_common.h"

namespace miru
{
	namespace xr
	{
		struct Pose
		{
			mars::Quaternion	orientation;
			mars::float3		position;

			Pose() = default;
			Pose(XrPosef pose)
			{
				orientation.s = static_cast<double>(pose.orientation.w);
				orientation.i = static_cast<double>(pose.orientation.x);
				orientation.j = static_cast<double>(pose.orientation.y);
				orientation.k = static_cast<double>(pose.orientation.y);
				position.x = pose.position.x;
				position.y = pose.position.y;
				position.z = pose.position.z;
			}
			operator XrPosef()
			{
				return { 
{
						static_cast<float>(orientation.s),
						static_cast<float>(orientation.i),
						static_cast<float>(orientation.j),
						static_cast<float>(orientation.k) },
					{
						position.x,
						position.y,
						position.z }
				};
			}
		};
		struct Fov
		{
			float angleLeft;
			float angleRight;
			float angleUp;
			float angleDown;

			Fov() = default;
			Fov(XrFovf fov)
			{
				angleLeft = fov.angleLeft;
				angleRight = fov.angleRight;
				angleUp = fov.angleUp;
				angleDown = fov.angleDown;
			}
			operator XrFovf()
			{
				return {
					angleLeft,
					angleRight,
					angleUp,
					angleDown 
				};
			}
		};
		struct View
		{
			Pose	pose;
			Fov		fov;
		};

		class MIRU_XR_API Space
		{
		protected:
			Space() = default;
			Space(const Space&) = delete;
			Space(const Space&&) = delete;
			virtual ~Space() = default;

		public:
			XrSpace m_Space;
		};

		class MIRU_XR_API ReferenceSpace final : public Space
		{
			//enums/structs
		public:
			enum class Type : uint32_t
			{
				UNKNOWN = 0,
				VIEW = 1,
				LOCAL = 2,
				STAGE = 3
			};

			struct CreateInfo
			{
				SessionRef	session;
				Type		type;
				Pose		pose;
			};

			//Methods
		public:
			ReferenceSpace(CreateInfo* pCreateInfo);
			~ReferenceSpace();

			//Members
		public:
			CreateInfo m_CI;

			XrReferenceSpaceCreateInfo m_SpaceCI;
		};

		class MIRU_XR_API ActionSpace final : public Space
		{
			//enums/structs
		public:
			struct CreateInfo
			{
				SessionRef	session;
				ActionRef	action;
				Path		subactionPath;
				Pose		pose;
			};

			//Methods
		public:
			ActionSpace(CreateInfo* pCreateInfo);
			~ActionSpace();

			//Members
		public:
			CreateInfo m_CI;

			XrActionSpaceCreateInfo m_SpaceCI;
		};
	}
}
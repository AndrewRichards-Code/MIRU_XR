#pragma once
#include "miru_xr_core_common.h"
#include "XRViewConfigurations.h"
#include "XRCompositionLayers.h"

namespace miru
{
	namespace xr
	{
		class MIRU_XR_API Session final
		{			
			//enums/structs
		public:
			struct CreateInfo
			{
				InstanceRef			instance;
				SystemRef			system;
				base::ContextRef	context;
			};
			enum class State
			{
				UNKNOWN = 0,
				IDLE = 1,
				READY = 2,
				SYNCHRONIZED = 3,
				VISIBLE = 4,
				FOCUSED = 5,
				STOPPING = 6,
				LOSS_PENDING = 7,
				EXITING = 8,
			};

			//Methods
		public:
			Session(CreateInfo* pCreateInfo);
			~Session();

			void StateChanged(XrEventDataSessionStateChanged* sessionStateChanged);
			
			void Begin();
			void End(); 
			void RequestExit();

			bool IsRunning() { return m_Running; };
			bool IsActive();

			void WaitFrame();
			void BeginFrame();
			void EndFrame(const std::vector<CompositionLayer::BaseHeader*>& layers);

			bool LocateViews(SpaceRef space, std::vector<View>& views);

			static void* GetMIRUOpenXRData(InstanceRef instance, SystemRef system);
			inline void SetViewConfigurationsType(ViewConfigurations::Type type) { m_Type = type; };
			inline void SetViewConfigurationsEnvironmentBlendMode(ViewConfigurations::EnvironmentBlendMode environmentBlendMode) { m_EnvironmentBlendMode = environmentBlendMode; };

			//Members
		public:
			CreateInfo m_CI;
			State m_State = State::UNKNOWN;
			ViewConfigurations::Type m_Type = ViewConfigurations::Type(0);
			ViewConfigurations::EnvironmentBlendMode m_EnvironmentBlendMode = ViewConfigurations::EnvironmentBlendMode(0);

			XrSession m_Session;
			XrSessionCreateInfo m_SessionCI;

			XrSessionBeginInfo m_SessionBI;
			bool m_Running = false;

			XrFrameState m_FrameState;
		};
	}
}

#pragma once

#if defined(_MSC_VER)
#pragma warning(disable : 4251)  //Disables 'Needs  dll-interface' warning C4251
#endif

//CSTDLIB
#include <iostream>
#include <vector>
#include <map>
#include <functional>

//MIRU
#include "miru_core.h"

//MARS
#include "mars.h"

//OpenXR
#if defined(MIRU_D3D12)
#define XR_USE_GRAPHICS_API_D3D12
#endif
#if defined(MIRU_VULKAN)
#define XR_USE_GRAPHICS_API_VULKAN
#endif

#include "openxr/openxr.h"

//MIRU_XR_API
#include "ARC/src/ExportAttributes.h"
#ifdef MIRU_XR_BUILD_DLL
#define MIRU_XR_API ARC_EXPORT
#else
#define MIRU_XR_API ARC_IMPORT
#endif

//ARC Helpers
#include "ARC/src/StringConversion.h"
#include "ARC/src/DynamicLibrary.h"
#include "ARC/src/ScopeAndRef.h"
#include "ARC/src/Helpers.h"
#include "ARC/External/magic_enum/magic_enum.hpp"

//MIRU_XR Class Forward Decalaration and Ref types
#define MIRU_XR_FORWARD_DECLARE_CLASS_AND_REF(_class) class _class; typedef Ref<_class> _class##Ref

namespace miru::xr
{
	MIRU_XR_FORWARD_DECLARE_CLASS_AND_REF(Action);
	MIRU_XR_FORWARD_DECLARE_CLASS_AND_REF(ActionSet);
	MIRU_XR_FORWARD_DECLARE_CLASS_AND_REF(ActionSpace);
	MIRU_XR_FORWARD_DECLARE_CLASS_AND_REF(Instance);
	MIRU_XR_FORWARD_DECLARE_CLASS_AND_REF(ReferenceSpace);
	MIRU_XR_FORWARD_DECLARE_CLASS_AND_REF(Session);
	MIRU_XR_FORWARD_DECLARE_CLASS_AND_REF(Space);
	MIRU_XR_FORWARD_DECLARE_CLASS_AND_REF(Swapchain);
	MIRU_XR_FORWARD_DECLARE_CLASS_AND_REF(System);
	MIRU_XR_FORWARD_DECLARE_CLASS_AND_REF(ViewConfigurations);

	typedef XrPath Path;
}


//MIRU_XR Enum Class Bitwise Operators Templates
#define MIRU_ENUM_CLASS_BITWISE_OPERATORS
#if defined(MIRU_ENUM_CLASS_BITWISE_OPERATORS)
#include "ARC/src/EnumClassBitwiseOperators.h"
#endif

//MIRU_XR Debugbreak, Assert and Warn
#include "ARC/src/DebugMacros.h"

#include "ARC/src/Log.h"

inline arc::Log MiruXrCoreLog("MIRU_XR_CORE");
#ifdef ARC_LOG_INSTANCE
#undef ARC_LOG_INSTANCE
#define ARC_LOG_INSTANCE MiruXrCoreLog
#endif

inline std::string GetXrResultString(int64_t errorCode)
{
	return std::string(magic_enum::enum_name(static_cast<XrResult>(errorCode)));
}

#define MIRU_XR_ASSERT(x) ARC_ASSERT((x))

#define MIRU_XR_FATAL(x, y) if((x) != 0) { ARC_FATAL(static_cast<int64_t>(x), "%s", y); ARC_ASSERT(false); }
#define MIRU_XR_ERROR(x, y) if((x) != 0) { ARC_ERROR(static_cast<int64_t>(x), "%s", y); }
#define MIRU_XR_WARN(x, y) if((x) != 0) { ARC_WARN(static_cast<int64_t>(x), "%s", y); }
#define MIRU_XR_INFO(x, y) if((x) != 0) { ARC_INFO(static_cast<int64_t>(x), "%s", y); }


//OpenXR Helper
namespace miru::xr
{
	inline PFN_xrVoidFunction GetInstanceProcAddr(XrInstance instance, const char* name)
	{
		PFN_xrVoidFunction function = nullptr;
		MIRU_XR_FATAL(xrGetInstanceProcAddr(instance, name, &function), "ERROR: OPENXR: Failed to get InstanceProcAddr.");
		return function;
	}
}
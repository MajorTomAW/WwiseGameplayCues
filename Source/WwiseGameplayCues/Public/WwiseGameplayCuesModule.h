// Author: Tom Werner (MajorT), 2026 February

#pragma once

#include "Modules/ModuleManager.h"

class FWwiseGameplayCuesModule : public IModuleInterface
{
public:
	//~Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~End IModuleInterface
};

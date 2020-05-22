#pragma once

namespace Iswenzz
{
	struct ClientArchiveData
	{
		int index;
		float origin[3];
		float velocity[3];		// null for some reason?
		int movementDir;
		int bobCycle;
		int commandTime;
		float angles[3];
	};
}

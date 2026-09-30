#ifndef COA_ASCENSION_AREA_ACCESS_H
#define COA_ASCENSION_AREA_ACCESS_H

#include "Define.h"

class Player;

bool AscensionAreaAccessAllows(Player const* player, uint32 mapId, uint32 zoneId);
bool AscensionAreaAccessAllowsPosition(Player const* player, uint32 mapId, float x, float y, float z);
bool AscensionAreaAccessAllowsTaxiNode(Player const* player, uint32 nodeId);

#endif

/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "AscensionAreaAccess.h"
#include "AscensionAreaAccessPolicy.h"
#include "Chat.h"
#include "Config.h"
#include "DBCStores.h"
#include "Log.h"
#include "MapMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "TransportMgr.h"
#include "WorldSession.h"
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace
{
using PolicyPointer = std::shared_ptr<AreaAccess::Policy const>;

constexpr char const* LockedMessage = "This area is currently inaccessible.";

PolicyPointer policy;

thread_local bool evicting = false;

AreaAccess::Throttle messageThrottle(std::chrono::seconds(30));

std::mutex taxiZonesLock;
std::unordered_map<uint32, uint32> taxiZones;

bool IsGameMaster(Player const* player)
{
    WorldSession const* session = player->GetSession();
    return session && session->GetSecurity() > SEC_PLAYER;
}

void Deny(Player* player)
{
    if (messageThrottle.Allow(player->GetGUID().GetRawValue(), std::chrono::steady_clock::now()))
        ChatHandler(player->GetSession()).SendSysMessage(LockedMessage);
}

uint32 ZoneAt(uint32 mapId, float x, float y, float z)
{
    return sMapMgr->GetZoneId(PHASEMASK_NORMAL, mapId, x, y, z);
}

uint32 TaxiNodeZone(TaxiNodesEntry const* node)
{
    std::lock_guard<std::mutex> guard(taxiZonesLock);
    auto it = taxiZones.find(node->ID);
    if (it == taxiZones.end())
        it = taxiZones.emplace(node->ID, ZoneAt(node->map_id, node->x, node->y, node->z)).first;
    return it->second;
}

void Reload()
{
    auto lookup = [](std::string const& key) -> std::optional<bool>
    {
        std::string const value = sConfigMgr->GetOption<std::string>(key, "");
        if (!value.empty() && !AreaAccess::ParseFlag(value))
            LOG_ERROR("server.loading", "Area access: invalid value '{}' for {}, treated as locked", value, key);
        return AreaAccess::FlagFromConfigText(value);
    };
    std::atomic_store(&policy, std::make_shared<AreaAccess::Policy const>(lookup));
}

PolicyPointer CurrentPolicy()
{
    PolicyPointer current = std::atomic_load(&policy);
    if (!current)
    {
        Reload();
        current = std::atomic_load(&policy);
    }
    return current;
}

AreaAccess::Endpoint TransportEnd(KeyFrame const& frame)
{
    TaxiPathNodeEntry const* node = frame.Node;
    return {node->mapid, ZoneAt(node->mapid, node->x, node->y, node->z)};
}

void Evict(ObjectGuid guid)
{
    Player* player = ObjectAccessor::FindPlayer(guid);
    if (!player || !player->IsInWorld() || IsGameMaster(player))
        return;

    PolicyPointer const current = CurrentPolicy();
    if (current->IsAllowed(false, player->GetMapId(), player->GetZoneId()))
        return;

    WorldLocation const bind(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, 0);
    for (WorldLocation const& destination : {bind, player->GetStartPosition()})
    {
        uint32 const zone = ZoneAt(destination.GetMapId(), destination.GetPositionX(), destination.GetPositionY(),
            destination.GetPositionZ());
        if (!current->IsAllowed(false, destination.GetMapId(), zone))
            continue;
        evicting = true;
        player->TeleportTo(destination);
        evicting = false;
        break;
    }
    Deny(player);
}

class Configuration : public WorldScript
{
public:
    Configuration() : WorldScript("AscensionAreaAccessConfiguration", { WORLDHOOK_ON_STARTUP,
        WORLDHOOK_ON_AFTER_CONFIG_LOAD, WORLDHOOK_ON_CAN_SPAWN_CONTINENT_TRANSPORT }) { }

    void OnStartup() override { Reload(); }

    void OnAfterConfigLoad(bool reload) override
    {
        if (reload)
            Reload();
    }

    bool OnCanSpawnContinentTransport(TransportTemplate const& transport) override
    {
        if (transport.keyFrames.empty())
            return true;

        if (CurrentPolicy()->IsTransportAllowed(TransportEnd(transport.keyFrames.front()),
            TransportEnd(transport.keyFrames.back())))
            return true;

        LOG_INFO("server.loading", "Area access: transport {} not spawned, its route ends in a locked area",
            transport.entry);
        return false;
    }
};

class Enforcement : public PlayerScript
{
public:
    Enforcement() : PlayerScript("AscensionAreaAccessEnforcement", { PLAYERHOOK_ON_CAN_TELEPORT_TO,
        PLAYERHOOK_ON_UPDATE_ZONE, PLAYERHOOK_ON_BEFORE_ACTIVATE_TAXI_PATH,
        PLAYERHOOK_ON_LOGOUT }) { }

    bool OnPlayerCanTeleportTo(Player* player, uint32 mapId, float x, float y, float z, uint32 options) override
    {
        if (evicting)
            return true;

        Map const* source = player->FindMap();
        AreaAccess::Teleport const teleport{IsGameMaster(player),
            AreaAccess::IsForcedMove((options & TELE_TO_NOT_LEAVE_TRANSPORT) != 0, player->GetTransport() != nullptr,
                source && (source->Instanceable() || source->IsBattlegroundOrArena())),
            mapId, ZoneAt(mapId, x, y, z)};
        if (CurrentPolicy()->IsTeleportAllowed(teleport))
            return true;

        Deny(player);
        return false;
    }

    void OnPlayerLogout(Player* player) override
    {
        messageThrottle.Forget(player->GetGUID().GetRawValue());
    }

    bool OnPlayerBeforeActivateTaxiPath(Player* player, std::vector<uint32> const& nodes) override
    {
        if (IsGameMaster(player))
            return true;

        for (uint32 nodeId : nodes)
            if (!AscensionAreaAccessAllowsTaxiNode(player, nodeId))
            {
                Deny(player);
                return false;
            }
        return true;
    }

    void OnPlayerUpdateZone(Player* player, uint32 newZone, uint32) override
    {
        if (IsGameMaster(player) || CurrentPolicy()->IsAllowed(false, player->GetMapId(), newZone))
            return;

        ObjectGuid const guid = player->GetGUID();
        player->m_Events.AddEventAtOffset([guid]() { Evict(guid); }, Milliseconds(1));
    }
};
}

bool AscensionAreaAccessAllows(Player const* player, uint32 mapId, uint32 zoneId)
{
    return CurrentPolicy()->IsAllowed(IsGameMaster(player), mapId, zoneId);
}

bool AscensionAreaAccessAllowsTaxiNode(Player const* player, uint32 nodeId)
{
    TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(nodeId);
    return !node || AscensionAreaAccessAllows(player, node->map_id, TaxiNodeZone(node));
}

void AddSC_AscensionAreaAccess()
{
    new Configuration();
    new Enforcement();
}

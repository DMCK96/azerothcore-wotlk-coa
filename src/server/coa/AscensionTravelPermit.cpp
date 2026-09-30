/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionAreaAccess.h"
#include "GossipDef.h"
#include "Item.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include <array>
#include <optional>

namespace
{
enum TravelPermit : uint32
{
    ItemTravelPermit = 977028,
    SenderTravelPermit = 977028,
    MaxTravelLevel = 8
};

struct Destination
{
    char const* name;
    TeamId team;
    uint8 race;
    std::optional<WorldLocation> location;
};

std::array<Destination, 8> const Destinations =
{{
    {"Elwynn Forest", TEAM_ALLIANCE, RACE_HUMAN, std::nullopt},
    {"Dun Morogh", TEAM_ALLIANCE, RACE_DWARF, std::nullopt},
    {"Teldrassil", TEAM_ALLIANCE, RACE_NIGHTELF, std::nullopt},
    {"Ammen Vale", TEAM_ALLIANCE, RACE_DRAENEI, WorldLocation(530, -3961.64f, -13931.2f, 100.615f, 2.08364f)},
    {"Tirisfal Glades", TEAM_HORDE, RACE_UNDEAD_PLAYER, std::nullopt},
    {"Durotar", TEAM_HORDE, RACE_ORC, std::nullopt},
    {"Mulgore", TEAM_HORDE, RACE_TAUREN, std::nullopt},
    {"Sunstrider Isle", TEAM_HORDE, RACE_BLOODELF, WorldLocation(530, 10349.6f, -6357.29f, 33.4026f, 5.31605f)}
}};

SpellCastResult CheckTravel(Player const* player)
{
    if (!player || !player->IsAlive())
        return SPELL_FAILED_CASTER_DEAD;
    if (player->GetLevel() > MaxTravelLevel)
        return SPELL_FAILED_HIGHLEVEL;
    if (player->IsInCombat())
        return SPELL_FAILED_AFFECTING_COMBAT;
    return SPELL_CAST_OK;
}

std::optional<WorldLocation> OpenStart(Destination const& destination, Player const* player)
{
    PlayerInfo const* info = sObjectMgr->GetPlayerInfo(destination.race, player->getClass());
    if (!info)
        return std::nullopt;

    WorldLocation const start = destination.location ? *destination.location
        : WorldLocation(info->mapId, info->positionX, info->positionY, info->positionZ, info->orientation);
    if (!AscensionAreaAccessAllowsPosition(player, start.GetMapId(), start.GetPositionX(), start.GetPositionY(),
        start.GetPositionZ()))
        return std::nullopt;
    return start;
}

class spell_ascension_travel_permit : public SpellScript
{
    PrepareSpellScript(spell_ascension_travel_permit);

    bool Load() override
    {
        return GetCaster()->ToPlayer() && GetCastItem() && GetCastItem()->GetEntry() == ItemTravelPermit;
    }

    SpellCastResult CheckCast()
    {
        return CheckTravel(GetCaster()->ToPlayer());
    }

    void OpenMenu()
    {
        Player* player = GetCaster()->ToPlayer();
        Item* item = GetCastItem();
        if (!item || CheckTravel(player) != SPELL_CAST_OK)
            return;
        ClearGossipMenuFor(player);
        for (uint32 i = 0; i < Destinations.size(); ++i)
            if (Destinations[i].team == player->GetTeamId() && OpenStart(Destinations[i], player))
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, Destinations[i].name, SenderTravelPermit, i);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_ascension_travel_permit::CheckCast);
        AfterCast += SpellCastFn(spell_ascension_travel_permit::OpenMenu);
    }
};

class item_ascension_travel_permit : public ItemScript
{
public:
    item_ascension_travel_permit() : ItemScript("item_ascension_travel_permit") { }

    void OnGossipSelect(Player* player, Item* item, uint32 sender, uint32 action) override
    {
        if (!player || !item || item->GetEntry() != ItemTravelPermit || sender != SenderTravelPermit)
            return;
        CloseGossipMenuFor(player);
        ClearGossipMenuFor(player);
        if (action >= Destinations.size() || CheckTravel(player) != SPELL_CAST_OK)
            return;
        Destination const& destination = Destinations[action];
        if (destination.team != player->GetTeamId())
            return;
        if (std::optional<WorldLocation> const start = OpenStart(destination, player))
            player->TeleportTo(*start);
    }
};
}

void AddAscensionTravelPermitScripts()
{
    RegisterSpellScript(spell_ascension_travel_permit);
    new item_ascension_travel_permit();
}

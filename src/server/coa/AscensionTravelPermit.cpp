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

namespace
{
enum TravelPermit : uint32
{
    ItemTravelPermit = 977028,
    SenderTravelPermit = 977028,
    MaxTravelLevel = 8,
    OutlandMap = 530,
    AmmenValeZone = 10142,
    SunstriderIsleZone = 10141
};

struct Destination
{
    char const* name;
    TeamId team;
    uint8 race;
    uint32 zone;
};

constexpr std::array<Destination, 8> Destinations =
{{
    {"Elwynn Forest", TEAM_ALLIANCE, RACE_HUMAN, 0},
    {"Dun Morogh", TEAM_ALLIANCE, RACE_DWARF, 0},
    {"Teldrassil", TEAM_ALLIANCE, RACE_NIGHTELF, 0},
    {"Ammen Vale", TEAM_ALLIANCE, RACE_DRAENEI, AmmenValeZone},
    {"Tirisfal Glades", TEAM_HORDE, RACE_UNDEAD_PLAYER, 0},
    {"Durotar", TEAM_HORDE, RACE_ORC, 0},
    {"Mulgore", TEAM_HORDE, RACE_TAUREN, 0},
    {"Sunstrider Isle", TEAM_HORDE, RACE_BLOODELF, SunstriderIsleZone}
}};

bool IsOpen(Destination const& destination, Player const* player)
{
    return !destination.zone || AscensionAreaAccessAllows(player, OutlandMap, destination.zone);
}

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
            if (Destinations[i].team == player->GetTeamId() && IsOpen(Destinations[i], player) &&
                sObjectMgr->GetPlayerInfo(Destinations[i].race, player->getClass()))
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
        if (destination.team != player->GetTeamId() || !IsOpen(destination, player))
            return;
        if (PlayerInfo const* start = sObjectMgr->GetPlayerInfo(destination.race, player->getClass()))
            player->TeleportTo(start->mapId, start->positionX, start->positionY, start->positionZ, start->orientation);
    }
};
}

void AddAscensionTravelPermitScripts()
{
    RegisterSpellScript(spell_ascension_travel_permit);
    new item_ascension_travel_permit();
}

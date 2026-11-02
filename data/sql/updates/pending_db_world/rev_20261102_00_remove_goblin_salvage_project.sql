-- Remove the Goblin Salvage Project (#8690, #8743, #8744): original content with no live Ascension source.
-- Every row it added goes, templates included.
-- Lootbot 3000 (57000) returns to Tiraxis' Convenience Items at 800 tokens (rev_20261013_10_tiraxis_lists.sql),
-- and its zero-price Vanity Store row goes, so the store price falls back to VanityCollection.dbc again.
DELETE FROM `item_template` WHERE `entry` BETWEEN 9707001 AND 9707008;
DELETE FROM `item_dbc` WHERE `ID` BETWEEN 9707001 AND 9707008;
DELETE FROM `creature_template` WHERE `entry` IN (9312001, 9312002, 9312003);
DELETE FROM `creature_template_model` WHERE `CreatureID` IN (9312001, 9312002, 9312003);
DELETE FROM `gameobject_template` WHERE `entry` IN (9312001, 9312002);

DELETE FROM `quest_template` WHERE `ID` IN (176000, 176001, 176002, 176003, 176004, 176011, 176012, 176013, 176014);
DELETE FROM `quest_template_addon` WHERE `ID` IN (176000, 176001, 176002, 176003, 176004, 176011, 176012, 176013, 176014);
DELETE FROM `quest_offer_reward` WHERE `ID` IN (176000, 176001, 176002, 176003, 176004, 176011, 176012, 176013, 176014);
DELETE FROM `quest_request_items` WHERE `ID` IN (176000, 176001, 176002, 176003, 176004, 176011, 176012, 176013, 176014);
DELETE FROM `creature_queststarter` WHERE `quest` IN (176000, 176001, 176002, 176003, 176004, 176011, 176012, 176013, 176014);
DELETE FROM `creature_questender` WHERE `quest` IN (176000, 176001, 176002, 176003, 176004, 176011, 176012, 176013, 176014);
DELETE FROM `quest_poi` WHERE `QuestID` IN (176000, 176001, 176002, 176003, 176004, 176011, 176012, 176013, 176014);
DELETE FROM `quest_poi_points` WHERE `QuestID` IN (176000, 176001, 176002, 176003, 176004, 176011, 176012, 176013, 176014);
DELETE FROM `coa_client_quest_super_track` WHERE `ID` IN (33400, 33401, 33402, 33403, 33404, 33411, 33412, 33413, 33414);
DELETE FROM `coa_client_super_track` WHERE `ID` BETWEEN 20900 AND 20911;

DELETE FROM `creature_loot_template` WHERE `Item` BETWEEN 9707001 AND 9707008;
DELETE FROM `gameobject_loot_template` WHERE `Entry` IN (9312001, 9312002);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 4 AND `SourceGroup` = 9312001 AND `SourceEntry` = 9707001;
DELETE FROM `creature_questitem` WHERE `ItemId` BETWEEN 9707001 AND 9707008;
DELETE FROM `gameobject_questitem` WHERE `GameObjectEntry` = 9312002;

DELETE FROM `creature` WHERE `guid` IN (9112000, 9112001, 9112002);
DELETE FROM `gameobject` WHERE `guid` IN (9112000, 9112001, 9112002, 9112003, 9112004, 9112005);

DELETE FROM `npc_vendor` WHERE `entry` = 900008 AND `item` = 57000;
INSERT INTO `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`) VALUES
(900008, 10, 57000, 0, 0, 10337);
DELETE FROM `coa_client_vanity_collection` WHERE `ItemID` = 57000;

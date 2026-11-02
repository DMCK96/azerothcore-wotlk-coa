-- Remove the Venture Co. Base camp rework in Stranglethorn Vale (#8960): original content with no live Ascension
-- source. The added spawns and the static Gold Vein go, Gold Mine 211032 is size 2 again, and the moved Venture Co.
-- workers return to their stock spawns (data/sql/base/db_world/creature.sql).
DELETE FROM `gameobject` WHERE `guid` IN (9999521, 9999522, 9999523, 9999525, 9999526, 9999527, 9999528, 9999529,
    9999554, 9999555, 9999556, 9999557, 9999558, 9999560, 9999561, 9999562, 9999565, 9999566, 9999567);
DELETE FROM `gameobject_template` WHERE `entry` = 9000806;
UPDATE `gameobject_template` SET `size` = 2 WHERE `entry` = 211032;

DELETE FROM `creature` WHERE `guid` IN (9999265, 9999268, 9999269, 9999270, 9999271, 9999273, 9999274, 9999275, 9999276, 9999277);

UPDATE `creature` SET `position_x` = -11460.3, `position_y` = 385.785, `position_z` = 77.5294, `orientation` = 1.20428 WHERE `guid` = 393 AND `id` = 1094;
UPDATE `creature` SET `position_x` = -11457.6, `position_y` = 377.95, `position_z` = 77.6057, `orientation` = 6.16101 WHERE `guid` = 395 AND `id` = 1094;
UPDATE `creature` SET `position_x` = -11462.5, `position_y` = 379.446, `position_z` = 76.2292, `orientation` = 0.610865, `wander_distance` = 5, `MovementType` = 1 WHERE `guid` = 728 AND `id` = 1096;
UPDATE `creature` SET `position_x` = -11491.9, `position_y` = 393.702, `position_z` = 62.3659, `orientation` = 1.53589 WHERE `guid` = 1314 AND `id` = 1097;
UPDATE `creature` SET `position_x` = -11495.9, `position_y` = 391.412, `position_z` = 59.5022, `orientation` = 0, `wander_distance` = 2, `MovementType` = 1 WHERE `guid` = 1315 AND `id` = 1096;
UPDATE `creature` SET `position_x` = -11494.1, `position_y` = 375.043, `position_z` = 57.6861, `orientation` = 1.69297 WHERE `guid` = 2128 AND `id` = 1094;
UPDATE `creature` SET `position_x` = -11506.3, `position_y` = 392.203, `position_z` = 51.1708, `orientation` = 5.88176 WHERE `guid` = 2130 AND `id` = 1094;
UPDATE `creature` SET `position_x` = -11482.4, `position_y` = 374.042, `position_z` = 62.0004, `orientation` = 0 WHERE `guid` = 2131 AND `id` = 1094;
UPDATE `creature` SET `position_x` = -11483.5, `position_y` = 348.799, `position_z` = 54.3791, `orientation` = 0.593412 WHERE `guid` = 2132 AND `id` = 1094;
UPDATE `creature` SET `position_x` = -11475.8, `position_y` = 361.128, `position_z` = 57.4834, `orientation` = 5.28835 WHERE `guid` = 2133 AND `id` = 1094;
UPDATE `creature` SET `position_x` = -11507.8, `position_y` = 490.365, `position_z` = 65.0211, `orientation` = 1.3439 WHERE `guid` = 2136 AND `id` = 1094;
UPDATE `creature` SET `position_x` = -11485.6, `position_y` = 483.841, `position_z` = 71.3451, `orientation` = 5.91667 WHERE `guid` = 2141 AND `id` = 1094;
UPDATE `creature` SET `position_x` = -11510.8, `position_y` = 419.911, `position_z` = 58.0586, `orientation` = 5.75959 WHERE `guid` = 2146 AND `id` = 1094;
UPDATE `creature` SET `position_x` = -11515.3, `position_y` = 413.912, `position_z` = 55.2547, `orientation` = 6.17846 WHERE `guid` = 2148 AND `id` = 1094;
UPDATE `creature` SET `position_x` = -11512.7, `position_y` = 458.33, `position_z` = 65.325, `orientation` = 6.24828 WHERE `guid` = 2149 AND `id` = 1096;

-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys   = require "luci.sys"
local zones = require "luci.sys.zoneinfo"
local fs    = require "nixio.fs"
local conf  = require "luci.config"
local ut = require "luci.util"

local m, s, o, al
m = Map("floorset", translate("楼层设置"))
m:chain("luci")

s = m:section(NamedSection, "floor", translate("楼层选择"))
s.anonymous = true
s.addremove = false

s:tab("floor_set",  translate("楼层选择"))

local b = s:taboption("floor_set", ListValue, "below", translate("地下层数"), translate("楼层数包含不显示楼层/跃层/改名楼层"))
b:value(0,translate("0"))
b:value(1,translate("1"))
b:value(2,translate("2"))
b:value(3,translate("3"))
b:value(4,translate("4"))
b:value(5,translate("5"))
b:value(6,translate("6"))
b:value(7,translate("7"))
b:value(8,translate("8"))
b:value(9,translate("9"))

local f = s:taboption("floor_set",ListValue,"above",translate("地面层数"), translate("楼层数包含不显示楼层/跃层/改名楼层"))
f:value(0,translate("0"))
f:value(1,translate("1"))
f:value(2,translate("2"))
f:value(3,translate("3"))
f:value(4,translate("4"))
f:value(5,translate("5"))
f:value(6,translate("6"))
f:value(7,translate("7"))
f:value(8,translate("8"))
f:value(9,translate("9"))
f:value(10,translate("10"))
f:value(11,translate("11"))
f:value(12,translate("12"))
f:value(13,translate("13"))
f:value(14,translate("14"))
f:value(15,translate("15"))
f:value(16,translate("16"))
f:value(17,translate("17"))
f:value(18,translate("18"))
f:value(19,translate("19"))
f:value(20,translate("20"))
f:value(21,translate("21"))
f:value(22,translate("22"))
f:value(23,translate("23"))
f:value(24,translate("24"))
f:value(25,translate("25"))
f:value(26,translate("26"))
f:value(27,translate("27"))
f:value(28,translate("28"))
f:value(29,translate("29"))
f:value(30,translate("30"))
f:value(31,translate("31"))
f:value(32,translate("32"))
f:value(33,translate("33"))
f:value(34,translate("34"))
f:value(35,translate("35"))
f:value(36,translate("36"))
f:value(37,translate("37"))
f:value(38,translate("38"))
f:value(39,translate("39"))
f:value(40,translate("40"))
f:value(41,translate("41"))
f:value(42,translate("42"))
f:value(43,translate("43"))
f:value(44,translate("44"))
f:value(45,translate("45"))
f:value(46,translate("46"))
f:value(47,translate("47"))
f:value(48,translate("48"))
f:value(49,translate("49"))
f:value(50,translate("50"))
f:value(51,translate("51"))
f:value(52,translate("52"))
f:value(53,translate("53"))
f:value(54,translate("54"))
f:value(55,translate("55"))
f:value(56,translate("56"))
f:value(57,translate("57"))
f:value(58,translate("58"))
f:value(59,translate("59"))
f:value(60,translate("60"))
f:value(61,translate("61"))
f:value(62,translate("62"))
f:value(63,translate("63"))
f:value(64,translate("64"))
f:value(65,translate("65"))
f:value(66,translate("66"))
f:value(67,translate("67"))
f:value(68,translate("68"))
f:value(69,translate("69"))
f:value(70,translate("70"))
f:value(71,translate("71"))
f:value(72,translate("72"))
f:value(73,translate("73"))
f:value(74,translate("74"))
f:value(75,translate("75"))
f:value(76,translate("76"))
f:value(77,translate("77"))
f:value(78,translate("78"))
f:value(79,translate("79"))
f:value(80,translate("80"))
f:value(81,translate("81"))
f:value(82,translate("82"))
f:value(83,translate("83"))
f:value(84,translate("84"))
f:value(85,translate("85"))
f:value(86,translate("86"))
f:value(87,translate("87"))
f:value(88,translate("88"))
f:value(89,translate("89"))
f:value(90,translate("90"))
f:value(91,translate("91"))
f:value(92,translate("92"))
f:value(93,translate("93"))
f:value(94,translate("94"))
f:value(95,translate("95"))
f:value(96,translate("96"))
f:value(97,translate("97"))
f:value(98,translate("98"))
f:value(99,translate("99"))

local nodisp = s:taboption("floor_set", Value, "nodisp",translate("掠过楼层"),translate("楼层数之间以半角逗号隔开"))
local yue = s:taboption("floor_set", Value, "rename",translate("地上地下楼层的显示名称"),translate("默认显示是楼层数后带'F',可自定义显示"))
local change = s:taboption("floor_set", Value, "change",translate("改名楼层"),translate("示例:\"-4:D,14:1D\",楼层与改名之间要以半角冒号':'隔开,负楼层用'-'表示, 一定要以半角逗号','结束,最多只能改名50层楼"))
--local high = s:taboption("floor_set", Value, "high",translate("特殊层高"),translate("示例:\"1:380,2:350\",层高单位cm,层高之间要以半角冒号':'隔开,负楼层用'-'表示, 一定要以半角逗号','结束,最多只能改名10层楼"))

--altitu set
--al = m:section(NamedSection, "altitu")
--al.addremove = false
--al.anonymous = true
s:tab("altitu_set", translate("高度校准"))
local first = s:taboption("altitu_set", DummyValue, "altitu_1", translate("起点高度(厘米)"))
first.template = "admin_louxianset/get_altitu"
local second = s:taboption("altitu_set", DummyValue, "altitu_2", translate("终点高度(厘米)"))
second.template = "admin_louxianset/get_altitu_2"
local average = s:taboption("altitu_set", DummyValue, "altitu_a", translate("校准类型"))
average.template = "admin_louxianset/get_average"
--local average = s:taboption("altitu_set", DummyValue, "altitu_a", translate("数据类型"))
--average.template = "admin_louxianset/list_altitu"
--local list = s:taboption("altitu_set",ListValue,"above",translate("地面层数"), translate("楼层数包含不显示楼层/跃层/改名楼层"))
--list:value(0,translate("0"))
--list:value(1,translate("1"))
local table = s:taboption("altitu_set", DummyValue,"altitu_t")
table.template = "admin_louxianset/show_floor"

function m.on_commit(map)
	luci.sys.call("/usr/bin/ipc_conf")
end

return m


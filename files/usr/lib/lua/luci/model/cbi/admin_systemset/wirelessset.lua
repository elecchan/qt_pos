-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys   = require "luci.sys"
local zones = require "luci.sys.zoneinfo"
local fs    = require "nixio.fs"
local conf  = require "luci.config"
local ut = require "luci.util"

local m, s, o
m = Map("wireless", translate("无线设置"))
m:chain("luci")

s = m:section(TypedSection, "wifi-iface", translate("无线设置"))
s.anonymous = true
s.addremove = false

s:tab("wireless_set",  translate("无线设置"))

local ssid = s:taboption("wireless_set", Value, "ssid",translate("SSID"))
local passwd = s:taboption("wireless_set", Value, "key",translate("密码"))

return m




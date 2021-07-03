-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys   = require "luci.sys"
local zones = require "luci.sys.zoneinfo"
local fs    = require "nixio.fs"
local conf  = require "luci.config"
local ut = require "luci.util"

local m, s, o
m = Map("network", translate("网络设置"))
m:chain("luci")

s = m:section(NamedSection, "lan", translate("网络设置"))
s.anonymous = true
s.addremove = false

s:tab("net_set",  translate("网络设置"))

local addr = s:taboption("net_set", Value, "ipaddr",translate("IP设置"))
local netmask = s:taboption("net_set", Value, "netmask",translate("Netmask"))
local macaddr = s:taboption("net_set", Value, "macaddr",translate("MAC地址"))

function macaddr.write(self, section, value)  
   local cmd="echo macset:" .. value .. " > /dev/console"
   luci.sys.call(cmd)
   
   --修改mac地址
   cmd="uci set network.wan.macaddr=" .. value
	luci.sys.call(cmd)
	cmd="uci set wireless.ap.macaddr=" .. value
	luci.sys.call(cmd)
	luci.sys.call("uci commit")
	cmd="\"ifconfig eth0 down;ifconfig eth0 hw ether " .. value .. ";ifconfig eth0 up\""
	local cmd2="echo " .. cmd .." > /etc/rc.local"
	luci.sys.call(cmd2)
	luci.sys.call("/etc/rc.local")
	--luci.sys.call("reboot")
	
	self.map:set(section, self.option, value)
end

return m



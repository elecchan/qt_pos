-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys   = require "luci.sys"
local zones = require "luci.sys.zoneinfo"
local fs    = require "nixio.fs"
local conf  = require "luci.config"
local ut = require "luci.util"

local m, s, o
local dhcp,static,pppoe,set4g

m = Map("network", translate("Internet AP"))
m:chain("luci")

--section类型：看配置文件而定
--TypedSection
--NamedSection
s = m:section(NamedSection, "wan", translate("network model"))
s.anonymous = false
s.addremove = false

--o = s:option(Value,"proto")

s:tab("dhcp_set",  translate("DHCP model"))
s:tab("static_set",  translate("STATIC model"))
s:tab("pppoe_set",  translate("PPPOE model"))
s:tab("4g_set", translate("4G model"))

--界面确认按键按下判断
local button = luci.http.formvalue("cbi.apply")
if button then	
	--luci.sys.call("/etc/init.d/network restart")
end

--
-- dhcp
--
local dhcp_enable = s:taboption("dhcp_set", Flag, "dhcp_enable", translate("DHCP Enable"))
function dhcp_enable.write(self, section, value)  
    --sync_value_to_file(value, "/etc/config/network") 
	luci.sys.call("echo dhcp_rewrite > /dev/console")
	self.map:set(section, self.option, value)
	luci.sys.call("/bin/internet_ap.sh")
end 

--
--static
--
--创建勾选框
local static_enable = s:taboption("static_set", Flag, "static_enable", translate("STATIC Enable"))

local ipaddr = s:taboption("static_set", Value, "ipaddr", translate("IPv4 address"))
ipaddr.datatype = "ip4addr"

local netmask = s:taboption("static_set", Value, "netmask",translate("IPv4 netmask"))
netmask.datatype = "ip4addr"
netmask:value("255.255.255.0")
netmask:value("255.255.0.0")
netmask:value("255.0.0.0")

local gateway = s:taboption("static_set", Value, "gateway", translate("IPv4 gateway"))
gateway.datatype = "ip4addr"

local dns = s:taboption("static_set", DynamicList, "dns",translate("Use custom DNS servers"))
dns.datatype = "ipaddr"
dns.cast     = "string"

--重写write函数
--主要为了执行脚本
function static_enable.write(self, section, value)  
    --sync_value_to_file(value, "/etc/config/network") 
	luci.sys.call("echo static_rewrite > /dev/console")
	self.map:set(section, self.option, value)
	luci.sys.call("/bin/internet_ap.sh")
end 

--
--pppoe
--
local pppoe_enable = s:taboption("pppoe_set", Flag, "pppoe_enable", translate("PPPOE Enable"))
local username = s:taboption("pppoe_set", Value, "username", translate("PPPOE username"))
local password = s:taboption("pppoe_set", Value, "password", translate("PPPOE password"))
password.password = true

function pppoe_enable.write(self, section, value)  
    --sync_value_to_file(value, "/etc/config/network") 
	luci.sys.call("echo pppoe_rewrite > /dev/console")
	self.map:set(section, self.option, value)
	luci.sys.call("/bin/internet_ap.sh")
end 

--
-- 4G
--
local ste4g_enable = s:taboption("4g_set", Flag, "4g_enable", translate("4G Enable"))

function ste4g_enable.write(self, section, value)  
    --sync_value_to_file(value, "/etc/config/network") 
	luci.sys.call("echo 4g_rewrite > /dev/console")
	self.map:set(section, self.option, value)
	luci.sys.call("/bin/internet_ap.sh")
end

return m

-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sys   = require "luci.sys"
local zones = require "luci.sys.zoneinfo"
local fs    = require "nixio.fs"
local conf  = require "luci.config"
local ut = require "luci.util"

local m, s, o
m = Map("network", translate("系统重启"))
m:chain("luci")

s = m:section(NamedSection, "lan", translate("系统重启"))
s.anonymous = true
s.addremove = false

s:tab("net_set",  translate("系统重启"))

button = s:taboption("net_set", Button, "button")             

button.inputtitle = translate("按下重启")      
button.inputstyle = "apply"                                                     

function button.write(self, section, value)                                     
         luci.sys.call("echo reboot > /dev/console ")  
         button.inputtitle = translate("重启中,等会手动刷新界面...")   
      	luci.sys.call("reboot")                        
end 

return m



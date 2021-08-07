-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.admin.louxianset", package.seeall)

function index()
	entry({"admin", "louxianset", "ipcset"}, cbi("admin_louxianset/ipcset"), _("IPC设置"), 1).index=true
	entry({"admin", "louxianset", "floorset"}, cbi("admin_louxianset/floorset"), _("楼层设置"), 10)
	entry({"admin", "louxianset", "get_altitu"}, call("action_get_altitu"))
end

function action_get_altitu()
	local x = luci.model.uci.cursor()
	local set = tonumber(luci.http.formvalue("set"))
	local altitu 
	altitu = luci.sys.exec("/usr/bin/get_altitu")
	luci.sys.exec("uci set floorset.altitu.first=%d" % altitu)
	luci.sys.exec("uci commit")
	luci.http.prepare_content("application/json")
	luci.http.write_json({ timestring = altitu })
end


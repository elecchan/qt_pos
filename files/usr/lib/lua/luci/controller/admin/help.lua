-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.admin.help", package.seeall)

function index()
	entry({"admin", "help", "help"}, template("help"), _("help"), 1).index=true
	entry({"admin", "help", "get_altitu_2"}, call("action_get_altitu_2"))
end

function action_get_altitu_2()
	local set = tonumber(luci.http.formvalue("set"))
	local altitu 
	altitu = luci.sys.exec("/usr/bin/get_altitu")
	luci.sys.exec("uci set floorset.altitu.second=%d" % altitu)
	luci.sys.exec("uci commit")
	luci.http.prepare_content("application/json")
	luci.http.write_json({ timestring = altitu })
end
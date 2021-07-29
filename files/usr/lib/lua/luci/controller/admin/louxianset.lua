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
	local set = tonumber(luci.http.formvalue("set"))
	if set ~= nil and set > 0 then
		local date = os.date("*t", set)
		if date then
			luci.sys.call("date -s '%04d-%02d-%02d %02d:%02d:%02d'" %{
				date.year, date.month, date.day, date.hour, date.min, date.sec
			})
		end
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json({ timestring = os.date("%c") })
end


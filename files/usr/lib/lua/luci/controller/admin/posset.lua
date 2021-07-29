-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.admin.posset", package.seeall)

function index()
	entry({"admin", "posset", "ipcset"}, cbi("admin_posset/ipcset"), _("IPC设置"), 1).index=true
	entry({"admin", "posset", "posset"}, cbi("admin_posset/posset"), _("POS机设置"), 10)
	entry({"admin", "posset", "calc_average"}, call("action_calc_average"))
end

function action_calc_average()
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


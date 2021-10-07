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
	--local set = tonumber(luci.http.formvalue("set"))
	--if set ~= nil and set > 0 then
	--	local date = os.date("*t", set)
	--	if date then
	--		luci.sys.call("date -s '%04d-%02d-%02d %02d:%02d:%02d'" %{
	--			date.year, date.month, date.day, date.hour, date.min, date.sec
	--		})
	--	end
	--end

	--luci.http.prepare_content("application/json")
	--luci.http.write_json({ timestring = os.date("%c") })

	local set = tonumber(luci.http.formvalue("set"))
	local first = luci.sys.exec("uci get floorset.altitu.first")
	local second = luci.sys.exec("uci get floorset.altitu.second")
	local floor_below =  tonumber(luci.sys.exec("uci get floorset.floor.below"))
	local floor_above =  tonumber(luci.sys.exec("uci get floorset.floor.above"))
	local first_n = tonumber(first)
	local sencond_n = tonumber(second)
	local average
	local a1,a2
	local total
	if floor_below>0 or floor_above>0 then
		local a1,a2 = math.modf((sencond_n - first_n) / (floor_below + floor_above - 1))
		total = sencond_n - first_n
		--a1 = (sencond_n - first_n) / (floor_below + floor_above)
		average = tostring(a1)
		
	else
		average = tostring(0)
	end
	luci.sys.exec("uci set floorset.altitu.total=%d" % total)
	luci.sys.exec("uci set floorset.altitu.average=%d" % average)
	luci.sys.exec("uci commit")
	if floor_below==0 and floor_above==0 then
		average = "楼层数为0,请设置"
	end
	
	luci.http.prepare_content("application/json")
	luci.http.write_json({ timestring = average})
end


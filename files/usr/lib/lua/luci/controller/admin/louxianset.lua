-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.admin.louxianset", package.seeall)

function index()
	entry({"admin", "louxianset", "ipcset"}, cbi("admin_louxianset/ipcset"), _("IPC设置"), 1).index=true
	entry({"admin", "louxianset", "floorset"}, cbi("admin_louxianset/floorset"), _("楼层设置"), 10)
	entry({"admin", "louxianset", "get_altitu_start"}, call("action_get_altitu_start"))
	entry({"admin", "louxianset", "get_altitu_stop"}, call("action_get_altitu_stop"))
	entry({"admin", "louxianset", "calc_average"}, call("action_calc_average"))
	entry({"admin", "louxianset", "get_floor_mess"}, call("action_get_floor_mess"))
end

function action_get_altitu_start()
	local x = luci.model.uci.cursor()
	local set = tonumber(luci.http.formvalue("set"))
	local altitu 
	altitu = luci.sys.exec("/usr/bin/get_altitu")
	luci.sys.exec("uci set floorset.altitu.first=%d" % altitu)
	luci.sys.exec("uci commit")
	luci.http.prepare_content("application/json")
	luci.http.write_json({ altitu_start = altitu })
end

function action_get_altitu_stop()
	local set = tonumber(luci.http.formvalue("set"))
	local altitu 
	altitu = luci.sys.exec("/usr/bin/get_altitu")
	luci.sys.exec("uci set floorset.altitu.second=%d" % altitu)
	luci.sys.exec("uci commit")
	luci.http.prepare_content("application/json")
	luci.http.write_json({ altitu_stop = altitu })
end

function action_get_floor_mess()
	local set = tonumber(luci.http.formvalue("set"))
	local floor_below =  tonumber(luci.sys.exec("uci get floorset.floor.below"))
	local floor_above =  tonumber(luci.sys.exec("uci get floorset.floor.above"))
	local max_floor = floor_below
	if floor_above>floor_below then
		max_floor = floor_above
	end
	local ret
	local str_html = ""
	for i=1,max_floor do
		str_html = str_html.."<tr>"
		if floor_above>=i then
			ret = luci.sys.exec("uci get floorset.altitu.floorA%d" % i)
			str_html = str_html.."<td>".."A"..tostring(i)..":"..ret.."</td>"
		end
		if floor_below>=i then
			ret = luci.sys.exec("uci get floorset.altitu.floorB%d" % i)
			str_html = str_html.."<td>".."B"..tostring(i)..":"..ret.."</td>"
		end
		str_html = str_html.."</tr>"
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json({ floor_mess = str_html })
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
	if floor_above>0 then
		for i=1,floor_above do
			luci.sys.exec("uci set floorset.altitu.floorA%d=%d" % {i,average})
		end
	end
	if floor_below>0 then
		for i=1,floor_below do
			luci.sys.exec("uci set floorset.altitu.floorB%d=%d" % {i,average})
		end
	end

	luci.sys.exec("uci set floorset.altitu.total=%d" % total)
	luci.sys.exec("uci set floorset.altitu.average=%d" % average)
	luci.sys.exec("uci commit")
	if floor_below==0 and floor_above==0 then
		average = "楼层数为0,请设置"
	end
	
	luci.http.prepare_content("application/json")
	luci.http.write_json({ floor_average = average})
end

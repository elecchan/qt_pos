-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.admin.louxianset", package.seeall)

function index()
	entry({"admin", "louxianset", "ipcset"}, cbi("admin_louxianset/ipcset"), _("IPC设置"), 1).index=true
	entry({"admin", "louxianset", "floorset"}, cbi("admin_louxianset/floorset"), _("楼层设置"), 10)
end


-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.admin.posset", package.seeall)

function index()
	entry({"admin", "posset", "ipcset"}, cbi("admin_posset/ipcset"), _("IPC设置"), 1).index=true
	entry({"admin", "posset", "posset"}, cbi("admin_posset/posset"), _("POS机设置"), 10)
end
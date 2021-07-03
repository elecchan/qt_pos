-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local ipc = require "luci.ip"

m = Map("posset", translate("POS机设置"),translate("设置POS接口以及参数"))

s = m:section(NamedSection, "ttyset" ,translate("串口设置"))
s.anonymous = true
s.addremove = false

s:tab("tty1", translate("串口1"))
s:tab("tty2", translate("串口2"))
s:tab("tty3", translate("串口3"))
s:tab("tty4", translate("串口4"))

--tty1
local baudrate1 = s:taboption("tty1", ListValue, "baudrate1",translate("波特率"))
baudrate1:value(1,translate("115200"))
baudrate1:value(2,translate("57600"))
baudrate1:value(3,translate("38400"))
baudrate1:value(4,translate("19200"))
baudrate1:value(5,translate("9600"))
baudrate1:value(6,translate("4800"))
baudrate1:value(7,translate("2400"))

local databit1 = s:taboption("tty1", ListValue, "databit1",translate("数据位"))
databit1:value(8,translate("8"))
databit1:value(7,translate("7"))
databit1:value(6,translate("6"))
databit1:value(5,translate("5"))

local parity1 = s:taboption("tty1", ListValue, "parity1",translate("校验位"))
parity1:value(1,translate("None"))
parity1:value(2,translate("Even"))
parity1:value(3,translate("Odd"))

local stopbit1 = s:taboption("tty1", ListValue, "stopbit1",translate("停止位"))
stopbit1:value(1,translate("1"))
stopbit1:value(2,translate("2"))

--tty2
local baudrate2 = s:taboption("tty2", ListValue, "baudrate2",translate("波特率"))
baudrate2:value(1,translate("115200"))
baudrate2:value(2,translate("57600"))
baudrate2:value(3,translate("38400"))
baudrate2:value(4,translate("19200"))
baudrate2:value(5,translate("9600"))
baudrate2:value(6,translate("4800"))
baudrate2:value(7,translate("2400"))

local databit2 = s:taboption("tty2", ListValue, "databit2",translate("数据位"))
databit2 :value(8,translate("8"))
databit2 :value(7,translate("7"))
databit2 :value(6,translate("6"))
databit2 :value(5,translate("5"))

local parity2 = s:taboption("tty2", ListValue, "parity2",translate("校验位"))
parity2:value(1,translate("None"))
parity2:value(2,translate("Even"))
parity2:value(3,translate("Odd"))

local stopbit2 = s:taboption("tty2", ListValue, "stopbit2",translate("停止位"))
stopbit2:value(1,translate("1"))
stopbit2:value(2,translate("2"))

--tty3
local baudrate3 = s:taboption("tty3", ListValue, "baudrate3",translate("波特率"))
baudrate3:value(1,translate("115200"))
baudrate3:value(2,translate("57600"))
baudrate3:value(3,translate("38400"))
baudrate3:value(4,translate("19200"))
baudrate3:value(5,translate("9600"))
baudrate3:value(6,translate("4800"))
baudrate3:value(7,translate("2400"))

local databit3 = s:taboption("tty3", ListValue, "databit3",translate("数据位"))
databit3 :value(8,translate("8"))
databit3 :value(7,translate("7"))
databit3 :value(6,translate("6"))
databit3 :value(5,translate("5"))

local parity3 = s:taboption("tty3", ListValue, "parity3",translate("校验位"))
parity3:value(1,translate("None"))
parity3:value(2,translate("Even"))
parity3:value(3,translate("Odd"))

local stopbit3 = s:taboption("tty3", ListValue, "stopbit3",translate("停止位"))
stopbit3:value(1,translate("1"))
stopbit3:value(2,translate("2"))

--tty4
local baudrate4 = s:taboption("tty4", ListValue, "baudrate4",translate("波特率"))
baudrate4:value(1,translate("115200"))
baudrate4:value(2,translate("57600"))
baudrate4:value(3,translate("38400"))
baudrate4:value(4,translate("19200"))
baudrate4:value(5,translate("9600"))
baudrate4:value(6,translate("4800"))
baudrate4:value(7,translate("2400"))

local databit4 = s:taboption("tty4", ListValue, "databit4",translate("数据位"))
databit4 :value(8,translate("8"))
databit4 :value(7,translate("7"))
databit4 :value(6,translate("6"))
databit4 :value(5,translate("5"))

local parity4 = s:taboption("tty4", ListValue, "parity4",translate("校验位"))
parity4:value(1,translate("None"))
parity4:value(2,translate("Even"))
parity4:value(3,translate("Odd"))

local stopbit4 = s:taboption("tty4", ListValue, "stopbit4",translate("停止位"))
stopbit4:value(1,translate("1"))
stopbit4:value(2,translate("2"))


--usb
--s = m:section(NamedSection, "usbset")
--s.addremove = false
--s.anonymous = true

--s:tab("usb", translate("usb set"))

--local type = s:taboption("usb", ListValue, "type",translate("POS type"))
--type :value(1,translate("Hisense"))
--type :value(2,translate("IBM"))
--type :value(3,translate("Ideal"))


--interface choice
s = m:section(NamedSection, "interface")
s.addremove = false
s.anonymous = true

s:tab("interface", translate("POS机接口设置"))
local interfacetype = s:taboption("interface", ListValue, "interfacetype",translate("接口类型"))
interfacetype:value(1,translate("串口"))
interfacetype:value(2,translate("USB"))

local ttynumber = s:taboption("interface", ListValue, "ttynumber",translate("选择串口配置"))
ttynumber :value(1,translate("串口1"))
ttynumber :value(2,translate("串口2"))
ttynumber :value(3,translate("串口3"))
ttynumber :value(4,translate("串口4"))

local ttyversion = s:taboption("interface", ListValue, "ttyversion",translate("串口型号"))
ttyversion:value(1,translate("V1.0"))

local usbtype = s:taboption("interface", ListValue, "usbnumber",translate("选择POS机类型"))
usbtype :value(1,translate("海信"))
usbtype :value(2,translate("IBM"))
usbtype :value(3,translate("理想"))

local usbversion = s:taboption("interface", ListValue, "usbversion",translate("USB型号"))
usbversion:value(1,translate("V1.0"))

--interface test
--s = m:section(NamedSection, "test")
--s.addremove = false
--s.anonymous = true

--s:tab("test", translate("接口测试"))
--local o = s:taboption("test", DummyValue, "result", translate("测试结果:"),translate("在测试前请保存配置"))
--o.template = "admin_posset/test_status"

function m.on_commit(map)
	luci.sys.call("/usr/bin/ipc_conf")
end

return m


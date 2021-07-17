scp /home/wooya/work/qt_pos/build_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/linux-ramips_mt7620/drv_hp303/hp303_drv.ko admin@192.168.1.248:/tmp/

ssh -l admin 192.168.1.248

scp /home/wooya/work/qt_pos/package/utils/pos_app/hp303_test  admin@192.168.1.248:/bin

scp /home/wooya/work/qt_pos/build_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/linux-ramips_mt7620/drv_hp303/hp303_drv.ko admin@192.168.1.248:/lib/modules/3.10.14/

scp /home/wooya/work/qt_pos/files/usr/lib/lua/luci/model/cbi/admin_louxianset/floorset.lua  admin@192.168.1.248:/usr/lib/lua/luci/model/cbi/admin_louxianset/

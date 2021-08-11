scp /home/wooya/work/qt_pos/build_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/linux-ramips_mt7620/drv_hp303/hp303_drv.ko admin@192.168.1.248:/tmp/

ssh -l admin 192.168.1.248

scp /home/wooya/work/qt_pos/package/utils/pos_app/hp303_test  admin@192.168.1.248:/bin
scp /home/wooya/work/qt_pos/package/utils/pos_app/get_altitu  admin@192.168.1.248:/usr/bin
scp main/pos_app admin@192.168.1.248:/usr/bin/


scp /home/wooya/work/qt_pos/build_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/linux-ramips_mt7620/drv_hp303/hp303_drv.ko admin@192.168.1.248:/lib/modules/3.10.14/

scp /home/wooya/work/qt_pos/files/usr/lib/lua/luci/model/cbi/admin_louxianset/floorset.lua  admin@192.168.1.248:/usr/lib/lua/luci/model/cbi/admin_louxianset/

scp /home/wooya/work/qt_pos/files/usr/lib/lua/luci/view/admin_louxianset/get_altitu.htm admin@192.168.1.248:/usr/lib/lua/luci/view/admin_louxianset/

scp /home/wooya/work/qt_pos/files/usr/lib/lua/luci/controller/admin/louxianset.lua admin@192.168.1.248:/usr/lib/lua/luci/controller/admin/
scp /home/wooya/work/qt_pos/files/usr/lib/lua/luci/controller/admin/posset.lua admin@192.168.1.248:/usr/lib/lua/luci/controller/admin/
scp /home/wooya/work/qt_pos/files/usr/lib/lua/luci/controller/admin/help.lua admin@192.168.1.248:/usr/lib/lua/luci/controller/admin/

cp main/pos_app ../../../files/usr/bin/
cp get_altitu ../../../files/usr/bin/
cp conf/pos_conf ../../../files/usr/bin/
cp conf/ipc_conf ../../../files/usr/bin/



CC=/home/wooya/work/OpenWrt-Toolchain-ramips-mt7688_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-i686/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-gcc
$CC main/pos_app.c main/shm_op.c main/conf_op.c main/uci_op.c main/ipc_op.c main/dev_op.c main/base64.c main/parse_printer.c -I include/ -ldl -lpthread -o main/pos_app lib/libiconv.a lib/libcurl.a lib/libssl.a lib/libpolarssl.a
$CC conf/pos_conf.c -I include/ -o conf/pos_conf
$CC conf/ipc_conf.c -I include/ -o conf/ipc_conf
#$CC conf/floor_conf.c -I include/ -o conf/floor_conf

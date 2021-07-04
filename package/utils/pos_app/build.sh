export STAGING_DIR=/home/wooya/work/openwrt_toolchain/bin
CC=/home/wooya/work/openwrt_toolchain/bin/mipsel-openwrt-linux-uclibc-gcc
if [ -f "main/pos_app" ]; then
	rm main/pos_app
fi
$CC main/pos_app.c main/shm_op.c main/conf_op.c main/uci_op.c main/ipc_op.c main/dev_op.c main/hp303_hal.c main/base64.c main/parse_printer.c -I include/ -ldl -lm -lpthread -o main/pos_app lib/libiconv.a lib/libcurl.a lib/libssl.a lib/libpolarssl.a
$CC conf/pos_conf.c -I include/ -o conf/pos_conf
$CC conf/ipc_conf.c -I include/ -o conf/ipc_conf
#for test
$CC test/hp303_test.c -lm -o hp303_test
#$CC conf/floor_conf.c -I include/ -o conf/floor_conf

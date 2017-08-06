#!/bin/bash

FWDIR=fw_out

do_fw_build () {
	echo "ARCH=$1 PLAT=$2 NODE_ID=$3"
	make distclean
	make ARCH=$1 PLAT=$2 NODE_ID=$3 deps
	make ARCH=$1 PLAT=$2 NODE_ID=$3 app

	for ext in elf bin hex
	do
		cp out/$2.out/$2.app.$ext \
			${FWDIR}/fw.$1.$2.node_$3.$ext
	done

	make ARCH=$1 PLAT=$2 clean
}

do_fw_clean() {
	rm -rf ${FWDIR}
}

case "$1" in
  build)
	if test -d ${FWDIR} ; then
		echo "firmware exists: ${FWDIR}"
		exit 1
	fi

	mkdir ${FWDIR}

	# build fw for stm32f0v1 nodes
	do_fw_build "arm" "stm32f0v1" 101
	do_fw_build "arm" "stm32f0v1" 102

	# build fw for attiny85v1 nodes
	do_fw_build "avr" "attiny85v1" 201

	# build fw for devduino2 nodes
	do_fw_build "avr" "devduino2" 301
	do_fw_build "avr" "devduino2" 302
    ;;
  clean)
	do_fw_clean
    ;;
  *)
    echo "Usage: $0 {build|clean}"
    exit 1
    ;;
esac

exit 0

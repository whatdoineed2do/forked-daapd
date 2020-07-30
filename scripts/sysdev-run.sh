#!/bin/bash

MAX_MEM=${MEM:=50%}
MAX_CPU=${CPU:=200%}

UNIT_NAME=dev-limit

grep -q cgroup2 /proc/filesystems
HAS_CGROUP2=$?

VG_CMD=""
if [ "${VG}" == "leak" ]; then
    VG_CMD="valgrind --show-leak-kinds=definite ${VG_ARGS}"
fi
if [ "${VG}" == "gdb" ]; then
    VG_CMD="valgrind --vgdb=yes --vgdb-error=0"
fi
if [ "${VG}" == "help" ]; then
    echo "leak:  VG_ARGS=\"--track-origins=yes --leak-check=full\""
    echo "gdb:   monitor leak_check full reachable any"
    exit 0
fi


if [ ${HAS_CGROUP2} -eq 0 ]; then
    echo "monitor via:"
    echo "  systemd-cgtop /user.slice/user-$(id -u).slice/user@$(id -u).service/${UNIT_NAME}.service"
    systemctl --quiet --user reset-failed ${UNIT_NAME} 2>&1 >/dev/null
    exec systemd-run --user -t \
	--unit=${UNIT_NAME} \
	-p MemoryMax=${MAX_MEM} \
	-p CPUQuota=${MAX_CPU} \
	${SYSTEMD_EXTRA_PROPS} \
	${VG_CMD} "$@"
else
    echo "falling back to cgroup1"
    sudo systemd-run -t \
	--slice=user-$(id -u).slice \
	--unit=${UNIT_NAME} \
	-p MemoryMax=${MAX_MEM} -p MemorySwapMax=${MAX_SWAP} -p CPUQuota=${MAX_CPU} \
	--uid $(id -nu) \
	"$@"
fi

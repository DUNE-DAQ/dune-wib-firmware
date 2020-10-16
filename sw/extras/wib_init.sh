#!/bin/sh

echo "Starting wib_server"
/bin/wib_server 2>/var/log/wib_server.err >/var/log/wib_server.log &

echo "Starting xvcserver"
/bin/xvcserver  2>/var/log/xvcserver.err >/var/log/xvcserver.log &

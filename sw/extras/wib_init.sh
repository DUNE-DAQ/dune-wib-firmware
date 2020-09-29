#!/bin/sh

echo "Starting wib_server"
/bin/wib_server 2>/var/log/wib_server.err >/var/log/wib_server.log &

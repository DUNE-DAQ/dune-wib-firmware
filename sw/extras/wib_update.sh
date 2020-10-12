#!/bin/sh

archive="$1"
root="$2"

( [ -f "$archive" ] && [ -d "$root" ] ) || exit 1

ignore="/boot|/home|/proc|/run|/sys|/dev"
manifest="${archive}.manifest"

echo "Generating update manifest"
tar -tf "$archive" | tail -n +2 | sed -e "s/\/$//" > "$manifest"

echo "Extracting update over live system"
tar -xf "$archive" -C "$root"

prune_files="$( echo "$( cd "$root" && find . -depth | head -n -1 )" | grep -Ev "$ignore" | grep -Fv -f "$manifest" )"
echo "Pruning files not in update"
cd "$root"
echo "$prune_files" | xargs rm -f

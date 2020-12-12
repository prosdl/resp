# Updates the Build History
VERSION=`./get_version.sh`
DATUM=`date -Iseconds`
RECHNER=`uname -n`
echo "Resp V$VERSION successfully build at $DATUM on $RECHNER" >>build.history

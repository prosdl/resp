# Updates the buildnumber in version.cpp

BUILD_NUMBER=` sed -n 's/char\* Version::build_number = \"\(.*\)"\;/\1/p' version.cpp`;
let "BUILD_NUMBER=BUILD_NUMBER+1"
echo "s/\\(number = \\\"\\).*\\(\\\"\\)/\\1$BUILD_NUMBER\\2/" >/tmp/pr.version.sed
mv version.cpp version.cpp.old
sed -f /tmp/pr.version.sed version.cpp.old >version.cpp

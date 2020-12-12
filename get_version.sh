# Retrieves the version from version.cpp

MAJOR_NUMBER=` sed -n 's/char\* Version::major_version = \"\(.*\)"\;/\1/p' version.cpp`;
MINOR_NUMBER=` sed -n 's/char\* Version::minor_version = \"\(.*\)"\;/\1/p' version.cpp`;
BUILD_NUMBER=` sed -n 's/char\* Version::build_number = \"\(.*\)"\;/\1/p' version.cpp`;
echo "$MAJOR_NUMBER.$MINOR_NUMBER.$BUILD_NUMBER"

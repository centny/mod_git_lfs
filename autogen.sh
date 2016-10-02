set -e
touch NEWS README AUTHORS ChangeLog
if [ `uname` = "Darwin" ];then
 glibtoolize -f -c
else
 libtoolize -f -c
fi
echo 00
aclocal -I m4
echo 11
autoheader
echo 22
automake --add-missing
echo 33
automake
echo 44
autoconf
echo 55
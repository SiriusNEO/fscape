FFS_MOUNT_POINT=${1}

make
rm path_cache
echo -n $FFS_MOUNT_POINT/ > path_cache
./generator 3 5
mkdir -p $FFS_MOUNT_POINT
./ffs -f $FFS_MOUNT_POINT -o nonempty &
FFS_MOUNT_POINT=${1}

mv gen.sh $FFS_MOUNT_POINT/gen.sh
cd $FFS_MOUNT_POINT
bash gen.sh
rm gen.sh
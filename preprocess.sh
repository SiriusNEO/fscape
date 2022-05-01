make
rm path_cache
echo -n '/tmp/ffs/' > path_cache
./generator 3 5
mkdir /tmp/ffs
./ffs -f /tmp/ffs -o nonempty &
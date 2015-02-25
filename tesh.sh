make
cp ../../disk ./
./myfsck -f 0 -i disk
./run_fsck.pl --partition 0 --tmp_dir temp --image disk
 

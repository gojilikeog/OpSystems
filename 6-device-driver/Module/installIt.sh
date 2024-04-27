sudo insmod ceasers.ko
sudo mknod /dev/ceasers c 415 0
sudo chmod 666 /dev/ceasers
sudo dmesg -t | tail -5
cat example.txt > /dev/ceasers
sudo dmesg -t | tail -5
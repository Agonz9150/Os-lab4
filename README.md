How to install ->

Add clook-iosched.c to /linux-3.6.6/block

change .config in /linuc-3.6.6

change Kconfig.iosched in /linux-3.6.6/block

Compile kernel 

How to test ->

Attach clook to drive of choice ( I chose sda, so my example will follow)

echo clook > /sys/block/sda/queue/scheduler

from there, do

python3 test.py

and to see the results run

dmesg
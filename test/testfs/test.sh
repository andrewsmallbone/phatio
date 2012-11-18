#! /bin/sh

FILES="1 2 3 4 5 6 7 8 9 10 11 12 13 14 15"
VOL=/Volumes/PHATIO

writeFiles()
{
	for f in $FILES;
	do
		cp $f.bin $VOL/
	done
	diskutil quiet unmountDisk /dev/disk1
	diskutil mount /dev/disk1s1
}

readFiles()
{
	echo reading
	mkdir -p out
	cp $VOL/*.bin out
	echo copied
	for f in $FILES;
	do
		diff -q $f.bin out/$f.bin
	done
	echo done
}

cycle()
{
	time writeFiles
	while (`test ! -d $VOL`); do sleep 1; done
	time readFiles 
	rm -r $VOL/*.bin out
}

# 
# while true; 
# do
# 	i=$((i+1))
# 	echo $i
	cycle
# done
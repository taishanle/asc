OPTION=-Wall -DNDEBUG

all:\
mod/unt.o\
mod/bsn.o\
srv/sch.o\
srv/crn.o\
srv/clk.o\
srv/drv.o\
srv/lcd.o\
srv/int.o\
srv/veh.o\
srv/com.o\
srv/asc.o\
../bin/asc

mod/unt.o:mod/unt.c mod/unt.h mod/bsn.c mod/bsn.h
	arm-hisiv100-linux-uclibcgnueabi-gcc mod/unt.c $(OPTION) -c -o mod/unt.o -I./mod
mod/bsn.o:mod/bsn.c mod/bsn.h mod/unt.c mod/unt.h
	arm-hisiv100-linux-uclibcgnueabi-gcc mod/bsn.c $(OPTION) -c -o mod/bsn.o -I./mod -I../sql

srv/sch.o:srv/sch.c mod/unt.o mod/bsn.o
	arm-hisiv100-linux-uclibcgnueabi-gcc srv/sch.c $(OPTION) -c -o srv/sch.o -I./mod
srv/crn.o:srv/crn.c mod/unt.o mod/bsn.o
	arm-hisiv100-linux-uclibcgnueabi-gcc srv/crn.c $(OPTION) -c -o srv/crn.o -I./mod
srv/clk.o:srv/clk.c mod/unt.o mod/bsn.o
	arm-hisiv100-linux-uclibcgnueabi-gcc srv/clk.c $(OPTION) -c -o srv/clk.o -I./mod
srv/drv.o:srv/drv.c mod/unt.o mod/bsn.o
	arm-hisiv100-linux-uclibcgnueabi-gcc srv/drv.c $(OPTION) -c -o srv/drv.o -I./mod
srv/lcd.o:srv/lcd.c mod/unt.o mod/bsn.o
	arm-hisiv100-linux-uclibcgnueabi-gcc srv/lcd.c $(OPTION) -c -o srv/lcd.o -I./mod -I../sql
srv/int.o:srv/int.c mod/unt.o mod/bsn.o
	arm-hisiv100-linux-uclibcgnueabi-gcc srv/int.c $(OPTION) -c -o srv/int.o -I./mod -I../sql
srv/veh.o:srv/veh.c mod/unt.o mod/bsn.o
	arm-hisiv100-linux-uclibcgnueabi-gcc srv/veh.c $(OPTION) -c -o srv/veh.o -I./mod -I../sql
srv/com.o:srv/com.c mod/unt.o mod/bsn.o
	arm-hisiv100-linux-uclibcgnueabi-gcc srv/com.c $(OPTION) -c -o srv/com.o -I./mod
srv/asc.o:srv/asc.c mod/unt.o mod/bsn.o
	arm-hisiv100-linux-uclibcgnueabi-gcc srv/asc.c $(OPTION) -c -o srv/asc.o -I./mod

../bin/asc:mod/unt.o mod/bsn.o srv/sch.o srv/crn.o srv/clk.o srv/drv.o srv/lcd.o srv/int.o srv/veh.o srv/com.o srv/asc.o
	arm-hisiv100-linux-uclibcgnueabi-gcc mod/unt.o mod/bsn.o srv/sch.o srv/crn.o srv/clk.o srv/drv.o srv/lcd.o srv/int.o srv/veh.o srv/com.o srv/asc.o $(OPTION) -o ../bin/asc -I./mod -I./com -I./srv -lrt -lm -L../sql -lsqlite3

touch:
	touch mod/*.c mod/*.h
	touch srv/*.c

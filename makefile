CHAIN=arm-none-eabi
CFLAGS=-std=c99
IPATH=-Iinc/
SRC=src/
OBJ=obj/
BIN=bin/

LDFLAGS=-static -e Entry -u Entry -u __aeabi_uidiv -u __aeabi_idiv --gc-sections -g
LD_LIBS=-lgcc -lg -lc -lm

SOURCES := $(wildcard src/*.c)

all: folder app

app: start.o ehrpwm.o pwmss.o main.o
	@echo "Linking...."
	@$(CHAIN)-gcc -nostartfiles $(OBJ)start.o $(OBJ)main.o $(OBJ)ehrpwm.o $(OBJ)pwmss.o -T $(SRC)memmap.ld -o $(OBJ)main.elf
	@$(CHAIN)-objcopy $(OBJ)main.elf $(BIN)spl.boot -O binary
	@cp $(BIN)spl.boot /tftpboot/appGpio.bin
	@mv $(BIN)spl.boot $(BIN)pwm.bin

start.o: $(SRC)start.s
	@echo "Compiling: $<"
	@$(CHAIN)-as $(IPATH) $(SRC)start.s -o $(OBJ)start.o

pwmss.o: $(SRC)pwmss.c
	@echo "Compiling: $<"
	@$(CHAIN)-gcc $(LD_LIBS) $(CFLAGS) $(IPATH) -c $(SRC)pwmss.c -o $(OBJ)pwmss.o

ehrpwm.o: $(SRC)ehrpwm.c
	@echo "Compiling: $<"
	@$(CHAIN)-gcc $(LD_LIBS) $(CFLAGS) $(IPATH) -c $(SRC)ehrpwm.c -o $(OBJ)ehrpwm.o

main.o: $(SRC)main.c
	@echo "Compiling: $<"
	@$(CHAIN)-gcc $(CFLAGS) $(IPATH) -c $(SRC)main.c -o $(OBJ)main.o
                                        
copy:
	cp $(BIN)spl.boot /tftpboot/appGpio.bin

folder:
	@mkdir -p bin/ obj/

clean:	
	@rm -rf $(OBJ)*.o
	@rm -rf $(OBJ)*.elf
	@rm -rf $(BIN)*.boot
	@rm -rf obj/
	@rm -rf bin/

dump:
	$(CHAIN)-objdump -D $(OBJ)main.elf
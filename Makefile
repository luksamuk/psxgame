export PATH           := /opt/psn00bsdk/bin:$(PATH)
export PSN00BSDK_LIBS := /opt/psn00bsdk/lib/libpsn00b

CHD      := ./PSXGAME.chd
ELF      := ./build/psxgame.elf
CUESHEET := ./build/PSXGAME.cue

.PHONY: clean run configure chd iso elf purge rebuild ${CUESHEET}

all: iso

elf: ./build/psxgame.elf
iso: ${CUESHEET}
chd: ${CHD}

run: ${CUESHEET}
	pcsx-redux \
	-run -interpreter -fastboot -stdout \
	-iso "$<"

./build: configure

${ELF}: ./build
	cd build && make psxgame

${CUESHEET}: ${ELF}
	cd build && make iso

${CHD}: ${CUESHEET}
	tochd -d . -- "$<"

configure:
#	cmake --preset default .
	cmake --preset release .

clean:
	rm -rf ./build

purge: clean
	rm -rf *.chd

rebuild: clean elf iso


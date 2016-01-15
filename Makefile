TARGET = minimal_sample

CXX = /opt/eldk-5.6/armv5te/sysroots/i686-eldk-linux/usr/bin/arm-linux-gnueabi/arm-linux-gnueabi-gcc

CXXFLAGS = -march=armv5te -mthumb-interwork -mtune=arm926ej-s -mfloat-abi=soft -fPIC
CXXFLAGS += -Wall -Wno-unused -std=gnu99

LDFLAGS += -lpthread -Ltuxono_sdk/

SRC =  main.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)   
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJ) -o $(TARGET) -ltuxono

%.o : %.c 
	$(CXX) -fPIC -c $(CXXFLAGS) -o $*.o $<
	
clean:
	rm -f $(OBJ) *~ $(TARGET)

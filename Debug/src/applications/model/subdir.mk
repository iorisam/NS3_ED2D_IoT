################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/applications/model/Astar2.cc \
../src/applications/model/Disc_Data_update_app.cc \
../src/applications/model/LRClientPush.cc \
../src/applications/model/LRServer.cc \
../src/applications/model/LRServerPush.cc \
../src/applications/model/Lte-Service-Request.cc \
../src/applications/model/LteAppClient.cc \
../src/applications/model/LteAppServer.cc \
../src/applications/model/Myheader.cc \
../src/applications/model/application-packet-probe.cc \
../src/applications/model/bulk-send-application.cc \
../src/applications/model/lr-client.cc \
../src/applications/model/lte-echo-app.cc \
../src/applications/model/lte-echo-server.cc \
../src/applications/model/onoff-application.cc \
../src/applications/model/packet-loss-counter.cc \
../src/applications/model/packet-sink.cc \
../src/applications/model/request-response-client.cc \
../src/applications/model/request-response-server.cc \
../src/applications/model/rnd-dest-packets.cc \
../src/applications/model/seq-ts-header.cc \
../src/applications/model/udp-client.cc \
../src/applications/model/udp-echo-client.cc \
../src/applications/model/udp-echo-server.cc \
../src/applications/model/udp-server.cc \
../src/applications/model/udp-trace-client.cc 

CC_DEPS += \
./src/applications/model/Astar2.d \
./src/applications/model/Disc_Data_update_app.d \
./src/applications/model/LRClientPush.d \
./src/applications/model/LRServer.d \
./src/applications/model/LRServerPush.d \
./src/applications/model/Lte-Service-Request.d \
./src/applications/model/LteAppClient.d \
./src/applications/model/LteAppServer.d \
./src/applications/model/Myheader.d \
./src/applications/model/application-packet-probe.d \
./src/applications/model/bulk-send-application.d \
./src/applications/model/lr-client.d \
./src/applications/model/lte-echo-app.d \
./src/applications/model/lte-echo-server.d \
./src/applications/model/onoff-application.d \
./src/applications/model/packet-loss-counter.d \
./src/applications/model/packet-sink.d \
./src/applications/model/request-response-client.d \
./src/applications/model/request-response-server.d \
./src/applications/model/rnd-dest-packets.d \
./src/applications/model/seq-ts-header.d \
./src/applications/model/udp-client.d \
./src/applications/model/udp-echo-client.d \
./src/applications/model/udp-echo-server.d \
./src/applications/model/udp-server.d \
./src/applications/model/udp-trace-client.d 

OBJS += \
./src/applications/model/Astar2.o \
./src/applications/model/Disc_Data_update_app.o \
./src/applications/model/LRClientPush.o \
./src/applications/model/LRServer.o \
./src/applications/model/LRServerPush.o \
./src/applications/model/Lte-Service-Request.o \
./src/applications/model/LteAppClient.o \
./src/applications/model/LteAppServer.o \
./src/applications/model/Myheader.o \
./src/applications/model/application-packet-probe.o \
./src/applications/model/bulk-send-application.o \
./src/applications/model/lr-client.o \
./src/applications/model/lte-echo-app.o \
./src/applications/model/lte-echo-server.o \
./src/applications/model/onoff-application.o \
./src/applications/model/packet-loss-counter.o \
./src/applications/model/packet-sink.o \
./src/applications/model/request-response-client.o \
./src/applications/model/request-response-server.o \
./src/applications/model/rnd-dest-packets.o \
./src/applications/model/seq-ts-header.o \
./src/applications/model/udp-client.o \
./src/applications/model/udp-echo-client.o \
./src/applications/model/udp-echo-server.o \
./src/applications/model/udp-server.o \
./src/applications/model/udp-trace-client.o 


# Each subdirectory must supply rules for building sources it contributes
src/applications/model/%.o: ../src/applications/model/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



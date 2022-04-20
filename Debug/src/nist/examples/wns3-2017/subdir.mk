################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/nist/examples/wns3-2017/wns3_2017_communication.cc \
../src/nist/examples/wns3-2017/wns3_2017_discovery.cc \
../src/nist/examples/wns3-2017/wns3_2017_pssch.cc \
../src/nist/examples/wns3-2017/wns3_2017_synch.cc 

CC_DEPS += \
./src/nist/examples/wns3-2017/wns3_2017_communication.d \
./src/nist/examples/wns3-2017/wns3_2017_discovery.d \
./src/nist/examples/wns3-2017/wns3_2017_pssch.d \
./src/nist/examples/wns3-2017/wns3_2017_synch.d 

OBJS += \
./src/nist/examples/wns3-2017/wns3_2017_communication.o \
./src/nist/examples/wns3-2017/wns3_2017_discovery.o \
./src/nist/examples/wns3-2017/wns3_2017_pssch.o \
./src/nist/examples/wns3-2017/wns3_2017_synch.o 


# Each subdirectory must supply rules for building sources it contributes
src/nist/examples/wns3-2017/%.o: ../src/nist/examples/wns3-2017/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



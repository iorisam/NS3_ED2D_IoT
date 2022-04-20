################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../scratch/TeamPhone2.cc \
../scratch/discomun.cc \
../scratch/energy-model-example.cc \
../scratch/energy-module-lte-example.cc \
../scratch/example-ping-lr-wpan.cc \
../scratch/lr-wpan-error-distance-plot.cc \
../scratch/lr-wpan-tsch.cc \
../scratch/ltesimu.cc \
../scratch/ltesimu2.cc \
../scratch/mylrwpantest.cc \
../scratch/mysimulation.cc \
../scratch/scratch-simulator.cc \
../scratch/teamphone.cc \
../scratch/test-validate.cc \
../scratch/wns3_2017_communication.cc \
../scratch/wns3_2017_discovery.cc \
../scratch/wns3_2017_pssch.cc 

CC_DEPS += \
./scratch/TeamPhone2.d \
./scratch/discomun.d \
./scratch/energy-model-example.d \
./scratch/energy-module-lte-example.d \
./scratch/example-ping-lr-wpan.d \
./scratch/lr-wpan-error-distance-plot.d \
./scratch/lr-wpan-tsch.d \
./scratch/ltesimu.d \
./scratch/ltesimu2.d \
./scratch/mylrwpantest.d \
./scratch/mysimulation.d \
./scratch/scratch-simulator.d \
./scratch/teamphone.d \
./scratch/test-validate.d \
./scratch/wns3_2017_communication.d \
./scratch/wns3_2017_discovery.d \
./scratch/wns3_2017_pssch.d 

OBJS += \
./scratch/TeamPhone2.o \
./scratch/discomun.o \
./scratch/energy-model-example.o \
./scratch/energy-module-lte-example.o \
./scratch/example-ping-lr-wpan.o \
./scratch/lr-wpan-error-distance-plot.o \
./scratch/lr-wpan-tsch.o \
./scratch/ltesimu.o \
./scratch/ltesimu2.o \
./scratch/mylrwpantest.o \
./scratch/mysimulation.o \
./scratch/scratch-simulator.o \
./scratch/teamphone.o \
./scratch/test-validate.o \
./scratch/wns3_2017_communication.o \
./scratch/wns3_2017_discovery.o \
./scratch/wns3_2017_pssch.o 


# Each subdirectory must supply rules for building sources it contributes
scratch/%.o: ../scratch/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/energy-module-lte/test/energy-module-lte-test-suite.cc 

CC_DEPS += \
./src/energy-module-lte/test/energy-module-lte-test-suite.d 

OBJS += \
./src/energy-module-lte/test/energy-module-lte-test-suite.o 


# Each subdirectory must supply rules for building sources it contributes
src/energy-module-lte/test/%.o: ../src/energy-module-lte/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



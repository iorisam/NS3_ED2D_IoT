################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/mesh/helper/dot11s/dot11s-installer.cc 

CC_DEPS += \
./src/mesh/helper/dot11s/dot11s-installer.d 

OBJS += \
./src/mesh/helper/dot11s/dot11s-installer.o 


# Each subdirectory must supply rules for building sources it contributes
src/mesh/helper/dot11s/%.o: ../src/mesh/helper/dot11s/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



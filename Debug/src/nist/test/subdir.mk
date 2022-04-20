################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/nist/test/test-nist-3gpp-validation.cc \
../src/nist/test/test-nist-parabolic-3d-antenna.cc \
../src/nist/test/test-nist-phy-error-model.cc 

CC_DEPS += \
./src/nist/test/test-nist-3gpp-validation.d \
./src/nist/test/test-nist-parabolic-3d-antenna.d \
./src/nist/test/test-nist-phy-error-model.d 

OBJS += \
./src/nist/test/test-nist-3gpp-validation.o \
./src/nist/test/test-nist-parabolic-3d-antenna.o \
./src/nist/test/test-nist-phy-error-model.o 


# Each subdirectory must supply rules for building sources it contributes
src/nist/test/%.o: ../src/nist/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



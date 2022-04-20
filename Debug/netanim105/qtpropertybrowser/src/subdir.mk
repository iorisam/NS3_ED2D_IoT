################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../netanim105/qtpropertybrowser/src/fileedit.cpp \
../netanim105/qtpropertybrowser/src/fileeditfactory.cpp \
../netanim105/qtpropertybrowser/src/filepathmanager.cpp \
../netanim105/qtpropertybrowser/src/qtbuttonpropertybrowser.cpp \
../netanim105/qtpropertybrowser/src/qteditorfactory.cpp \
../netanim105/qtpropertybrowser/src/qtgroupboxpropertybrowser.cpp \
../netanim105/qtpropertybrowser/src/qtpropertybrowser.cpp \
../netanim105/qtpropertybrowser/src/qtpropertybrowserutils.cpp \
../netanim105/qtpropertybrowser/src/qtpropertymanager.cpp \
../netanim105/qtpropertybrowser/src/qttreepropertybrowser.cpp \
../netanim105/qtpropertybrowser/src/qtvariantproperty.cpp \
../netanim105/qtpropertybrowser/src/variantfactory.cpp \
../netanim105/qtpropertybrowser/src/variantmanager.cpp 

OBJS += \
./netanim105/qtpropertybrowser/src/fileedit.o \
./netanim105/qtpropertybrowser/src/fileeditfactory.o \
./netanim105/qtpropertybrowser/src/filepathmanager.o \
./netanim105/qtpropertybrowser/src/qtbuttonpropertybrowser.o \
./netanim105/qtpropertybrowser/src/qteditorfactory.o \
./netanim105/qtpropertybrowser/src/qtgroupboxpropertybrowser.o \
./netanim105/qtpropertybrowser/src/qtpropertybrowser.o \
./netanim105/qtpropertybrowser/src/qtpropertybrowserutils.o \
./netanim105/qtpropertybrowser/src/qtpropertymanager.o \
./netanim105/qtpropertybrowser/src/qttreepropertybrowser.o \
./netanim105/qtpropertybrowser/src/qtvariantproperty.o \
./netanim105/qtpropertybrowser/src/variantfactory.o \
./netanim105/qtpropertybrowser/src/variantmanager.o 

CPP_DEPS += \
./netanim105/qtpropertybrowser/src/fileedit.d \
./netanim105/qtpropertybrowser/src/fileeditfactory.d \
./netanim105/qtpropertybrowser/src/filepathmanager.d \
./netanim105/qtpropertybrowser/src/qtbuttonpropertybrowser.d \
./netanim105/qtpropertybrowser/src/qteditorfactory.d \
./netanim105/qtpropertybrowser/src/qtgroupboxpropertybrowser.d \
./netanim105/qtpropertybrowser/src/qtpropertybrowser.d \
./netanim105/qtpropertybrowser/src/qtpropertybrowserutils.d \
./netanim105/qtpropertybrowser/src/qtpropertymanager.d \
./netanim105/qtpropertybrowser/src/qttreepropertybrowser.d \
./netanim105/qtpropertybrowser/src/qtvariantproperty.d \
./netanim105/qtpropertybrowser/src/variantfactory.d \
./netanim105/qtpropertybrowser/src/variantmanager.d 


# Each subdirectory must supply rules for building sources it contributes
netanim105/qtpropertybrowser/src/%.o: ../netanim105/qtpropertybrowser/src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



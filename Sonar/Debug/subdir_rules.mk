################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
config_AIC23.obj: ../config_AIC23.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"c:/ti/ccsv6/tools/compiler/c6000_7.4.8/bin/cl6x" -mv6700 --abi=coffabi -g --include_path="c:/ti/ccsv6/tools/compiler/c6000_7.4.8/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/ccsv6/tools/compiler/c6000_7.4.8/include" --include_path="C:/ti/DSPLIB/c6700/dsplib/include" --include_path="c:/ti/DSK6713/c6000/dsk6713/include" --include_path="c:/ti/C6xCSL/include" --include_path="C:/Users/louis/workspace_v6_0/Sonar/Debug" --include_path="c:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="c:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c6000" --define=c6713 --define=CHIP_6713 --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="config_AIC23.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

sonar.obj: ../sonar.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"c:/ti/ccsv6/tools/compiler/c6000_7.4.8/bin/cl6x" -mv6700 --abi=coffabi -g --include_path="c:/ti/ccsv6/tools/compiler/c6000_7.4.8/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/ccsv6/tools/compiler/c6000_7.4.8/include" --include_path="C:/ti/DSPLIB/c6700/dsplib/include" --include_path="c:/ti/DSK6713/c6000/dsk6713/include" --include_path="c:/ti/C6xCSL/include" --include_path="C:/Users/louis/workspace_v6_0/Sonar/Debug" --include_path="c:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="c:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c6000" --define=c6713 --define=CHIP_6713 --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="sonar.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

sonarcfg.cmd: ../sonar.tcf
	@echo 'Building file: $<'
	@echo 'Invoking: TConf'
	"c:/ti/bios_5_42_01_09/xdctools/tconf" -b -Dconfig.importPath="c:/ti/bios_5_42_01_09/packages;" "$<"
	@echo 'Finished building: $<'
	@echo ' '

sonarcfg.s??: | sonarcfg.cmd
sonarcfg_c.c: | sonarcfg.cmd
sonarcfg.h: | sonarcfg.cmd
sonarcfg.h??: | sonarcfg.cmd
sonar.cdb: | sonarcfg.cmd

sonarcfg.obj: ./sonarcfg.s?? $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"c:/ti/ccsv6/tools/compiler/c6000_7.4.8/bin/cl6x" -mv6700 --abi=coffabi -g --include_path="c:/ti/ccsv6/tools/compiler/c6000_7.4.8/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/ccsv6/tools/compiler/c6000_7.4.8/include" --include_path="C:/ti/DSPLIB/c6700/dsplib/include" --include_path="c:/ti/DSK6713/c6000/dsk6713/include" --include_path="c:/ti/C6xCSL/include" --include_path="C:/Users/louis/workspace_v6_0/Sonar/Debug" --include_path="c:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="c:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c6000" --define=c6713 --define=CHIP_6713 --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="sonarcfg.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

sonarcfg_c.obj: ./sonarcfg_c.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"c:/ti/ccsv6/tools/compiler/c6000_7.4.8/bin/cl6x" -mv6700 --abi=coffabi -g --include_path="c:/ti/ccsv6/tools/compiler/c6000_7.4.8/include" --include_path="C:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="C:/ti/ccsv6/tools/compiler/c6000_7.4.8/include" --include_path="C:/ti/DSPLIB/c6700/dsplib/include" --include_path="c:/ti/DSK6713/c6000/dsk6713/include" --include_path="c:/ti/C6xCSL/include" --include_path="C:/Users/louis/workspace_v6_0/Sonar/Debug" --include_path="c:/ti/bios_5_42_01_09/packages/ti/bios/include" --include_path="c:/ti/bios_5_42_01_09/packages/ti/rtdx/include/c6000" --define=c6713 --define=CHIP_6713 --display_error_number --diag_warning=225 --diag_wrap=off --preproc_with_compile --preproc_dependency="sonarcfg_c.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '



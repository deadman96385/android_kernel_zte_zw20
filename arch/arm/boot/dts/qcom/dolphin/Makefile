dtb-y := zte-sdw2500-msm8909w-wtp.dtb

targets += dtbs
targets += $(addprefix ../../, $(dtb-y))

$(obj)/%.dtb: $(src)/$(ZTE_BOARD_NAME)/%.dts FORCE
	$(call if_changed_dep,dtc)

dtbs: $(addprefix $(obj)/,$(dtb-y))
clean-files := *.dtb
